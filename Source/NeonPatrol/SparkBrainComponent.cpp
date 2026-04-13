#include "SparkBrainComponent.h"
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

    LastResponse = Content;

    TSharedPtr<FJsonObject> AssistantMsg = MakeShareable(new FJsonObject);
    AssistantMsg->SetStringField(TEXT("role"), TEXT("assistant"));
    AssistantMsg->SetStringField(TEXT("content"), Content);
    MessageHistory.Add(MakeShareable(new FJsonValueObject(AssistantMsg)));

    TrimHistory();

    FString PlayerMessage;
    if (MessageHistory.Num() > 0)
    {
        const TSharedPtr<FJsonObject>* LastUser;
        if (MessageHistory.Last()->TryGetObject(LastUser))
        {
            FString Role;
            LastUser->Get()->TryGetStringField(TEXT("role"), Role);
            if (Role == TEXT("user"))
            {
                LastUser->Get()->TryGetStringField(TEXT("content"), PlayerMessage);
            }
        }
    }

    OnChatResponse.Broadcast(PlayerMessage, Content);
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
