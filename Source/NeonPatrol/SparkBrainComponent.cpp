#include "SparkBrainComponent.h"
#include "SparkCharacter.h"
#include "NeonPatrol.h"

USparkBrainComponent::USparkBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USparkBrainComponent::BeginPlay()
{
    Super::BeginPlay();

    FString FullPath = FPaths::ConvertRelativePathToFull(ApiKeyFilePath);
    if (FFileHelper::LoadFileToString(ApiKey, *FullPath))
    {
        ApiKey.TrimStartAndEndInline();
        UE_LOG(LogTemp, Log, TEXT("SparkBrain: API key loaded from %s"), *FullPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SparkBrain: Failed to load API key from %s"), *FullPath);
    }
}

void USparkBrainComponent::SendChat(const FString& PlayerMessage)
{
    if (PlayerMessage.IsEmpty()) return;

    TSharedPtr<FJsonObject> UserMsg = MakeShareable(new FJsonObject);
    UserMsg->SetStringField(TEXT("role"), TEXT("user"));
    UserMsg->SetStringField(TEXT("content"), PlayerMessage);
    MessageHistory.Add(MakeShareable(new FJsonValueObject(UserMsg)));

    TrimHistory();

    TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);
    Root->SetStringField(TEXT("model"), ModelName);
    Root->SetNumberField(TEXT("max_tokens"), MaxTokens);
    Root->SetNumberField(TEXT("temperature"), Temperature);

    TArray<TSharedPtr<FJsonValue>> Messages;
    TSharedPtr<FJsonObject> SystemMsg = MakeShareable(new FJsonObject);
    SystemMsg->SetStringField(TEXT("role"), TEXT("system"));
    SystemMsg->SetStringField(TEXT("content"), SystemPrompt);
    Messages.Add(MakeShareable(new FJsonValueObject(SystemMsg)));

    for (const auto& Msg : MessageHistory)
    {
        Messages.Add(Msg);
    }

    Root->SetArrayField(TEXT("messages"), Messages);

    if (EndpointURL.Contains(TEXT("127.0.0.1")))
    {
        TSharedPtr<FJsonObject> Kwargs = MakeShareable(new FJsonObject);
        Kwargs->SetBoolField(TEXT("add_generation_prompt"), true);
        Root->SetObjectField(TEXT("chat_template_kwargs"), Kwargs);
    }

    FString JsonStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
    FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

    FHttpModule& Http = FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = Http.CreateRequest();
    Req->SetURL(EndpointURL);
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Req->SetHeader(TEXT("User-Agent"), TEXT("Kriisko-Studio/1.0"));
    Req->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
    Req->SetContentAsString(JsonStr);
    Req->OnProcessRequestComplete().BindUObject(this, &USparkBrainComponent::OnResponseReceived);
    Req->ProcessRequest();
}

void USparkBrainComponent::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FString Content;
    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        FString RespStr = Response->GetContentAsString();
        TSharedPtr<FJsonObject> RespObj;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RespStr);
        if (FJsonSerializer::Deserialize(Reader, RespObj))
        {
            const TArray<TSharedPtr<FJsonValue>>* Choices;
            if (RespObj->TryGetArrayField(TEXT("choices"), Choices) && Choices->Num() > 0)
            {
                const TSharedPtr<FJsonObject>* ChoiceObj;
                if ((*Choices)[0]->TryGetObject(ChoiceObj))
                {
                    const TSharedPtr<FJsonObject>* MsgObj;
                    if (ChoiceObj->Get()->TryGetObjectField(TEXT("message"), MsgObj))
                    {
                        MsgObj->Get()->TryGetStringField(TEXT("content"), Content);
                        Content.TrimStartAndEndInline();
                    }
                }
            }
        }
    }

    if (Content.IsEmpty())
    {
        Content = TEXT("...");
    }

    // Parse JSON command from LLM response
    FString SayText;
    ParseAndExecuteCommand(Content, SayText);

    // Use the "say" field as the display text, fall back to raw content
    FString DisplayText = SayText.IsEmpty() ? Content : SayText;
    LastResponse = DisplayText;

    TSharedPtr<FJsonObject> AssistantMsg = MakeShareable(new FJsonObject);
    AssistantMsg->SetStringField(TEXT("role"), TEXT("assistant"));
    AssistantMsg->SetStringField(TEXT("content"), Content);
    MessageHistory.Add(MakeShareable(new FJsonValueObject(AssistantMsg)));

    TrimHistory();

    // Find the last user message for the broadcast
    FString PlayerMessage;
    for (int32 i = MessageHistory.Num() - 1; i >= 0; --i)
    {
        const TSharedPtr<FJsonObject>* MsgObj;
        if (MessageHistory[i]->TryGetObject(MsgObj))
        {
            FString Role;
            MsgObj->Get()->TryGetStringField(TEXT("role"), Role);
            if (Role == TEXT("user"))
            {
                MsgObj->Get()->TryGetStringField(TEXT("content"), PlayerMessage);
                break;
            }
        }
    }

    OnChatResponse.Broadcast(PlayerMessage, DisplayText);
}

void USparkBrainComponent::TrimHistory()
{
    while (MessageHistory.Num() > 6)
    {
        MessageHistory.RemoveAt(0);
    }
}

FString USparkBrainComponent::GetLastResponse() const
{
    return LastResponse;
}

void USparkBrainComponent::ParseAndExecuteCommand(const FString& RawResponse, FString& OutSay)
{
    // Try to parse as JSON: {"say":"...", "cmd":"COMMAND PARAM"}
    TSharedPtr<FJsonObject> JsonObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawResponse);

    if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
    {
        // Not valid JSON — treat entire response as speech, no command
        OutSay = RawResponse;
        return;
    }

    // Extract "say" text
    JsonObj->TryGetStringField(TEXT("say"), OutSay);

    // Extract and parse "cmd"
    FString CmdStr;
    if (!JsonObj->TryGetStringField(TEXT("cmd"), CmdStr) || CmdStr.IsEmpty() || CmdStr == TEXT("NONE"))
    {
        return; // No command
    }

    // Parse command name and optional parameter
    FString CmdName;
    float CmdParam = 0.f;
    int32 SpaceIdx;
    if (CmdStr.FindChar(' ', SpaceIdx))
    {
        CmdName = CmdStr.Left(SpaceIdx).ToUpper();
        FString ParamStr = CmdStr.Mid(SpaceIdx + 1);
        CmdParam = FCString::Atof(*ParamStr);
    }
    else
    {
        CmdName = CmdStr.ToUpper();
    }

    // Map command string to enum and execute on SparkCharacter
    ASparkCharacter* Spark = Cast<ASparkCharacter>(GetOwner());
    if (!Spark)
    {
        UE_LOG(LogNeonPatrol, Warning, TEXT("SparkBrain: No SparkCharacter owner for command %s"), *CmdName);
        return;
    }

    ESparkCommand Cmd = ESparkCommand::None;
    if (CmdName == TEXT("FOLLOW")) Cmd = ESparkCommand::Follow;
    else if (CmdName == TEXT("STAY")) Cmd = ESparkCommand::Stay;
    else if (CmdName == TEXT("ATTACK")) Cmd = ESparkCommand::Attack;
    else if (CmdName == TEXT("HOLD_FIRE")) Cmd = ESparkCommand::HoldFire;
    else if (CmdName == TEXT("COME_HERE")) Cmd = ESparkCommand::ComeHere;
    else if (CmdName == TEXT("MOVE_FORWARD")) Cmd = ESparkCommand::MoveForward;
    else if (CmdName == TEXT("MOVE_BACK")) Cmd = ESparkCommand::MoveBack;
    else if (CmdName == TEXT("MOVE_LEFT")) Cmd = ESparkCommand::MoveLeft;
    else if (CmdName == TEXT("MOVE_RIGHT")) Cmd = ESparkCommand::MoveRight;

    if (Cmd != ESparkCommand::None)
    {
        Spark->ExecuteCommand(Cmd, CmdParam);
        OnSparkCommand.Broadcast(CmdName, CmdParam);
        UE_LOG(LogNeonPatrol, Log, TEXT("SparkBrain: Executed command %s (%.0f)"), *CmdName, CmdParam);
    }
}
