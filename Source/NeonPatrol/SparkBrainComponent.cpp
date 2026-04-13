#include "SparkBrainComponent.h"
#include "NeonPatrol.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

USparkBrainComponent::USparkBrainComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USparkBrainComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USparkBrainComponent::SendChat(const FString& PlayerMessage)
{
    FHttpModule* Http = &FHttpModule::Get();
    if (!Http) return;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
    Request->SetURL(EndpointURL);
    Request->SetVerb("POST");
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("Kriisko-Studio/1.0"));

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("model", ModelName);
    JsonObject->SetNumberField("max_tokens", MaxTokens);
    JsonObject->SetNumberField("temperature", Temperature);

    TSharedPtr<FJsonObject> ChatTemplateKwArgs = MakeShareable(new FJsonObject);
    ChatTemplateKwArgs->SetBoolField("enable_thinking", false);
    JsonObject->SetObjectField("chat_template_kwargs", ChatTemplateKwArgs);

    TArray<TSharedPtr<FJsonValue>> Messages;
    TSharedPtr<FJsonObject> SystemMsg = MakeShareable(new FJsonObject);
    SystemMsg->SetStringField("role", "system");
    SystemMsg->SetStringField("content", SystemPrompt);
    Messages.Add(MakeShareable(new FJsonValueObject(SystemMsg)));

    for (const auto& Msg : MessageHistory)
    {
        Messages.Add(Msg);
    }

    TSharedPtr<FJsonObject> UserMsg = MakeShareable(new FJsonObject);
    UserMsg->SetStringField("role", "user");
    UserMsg->SetStringField("content", PlayerMessage);
    Messages.Add(MakeShareable(new FJsonValueObject(UserMsg)));

    // Add user message to history
    TSharedPtr<FJsonObject> UserMsgCopy = MakeShareable(new FJsonObject);
    UserMsgCopy->SetStringField("role", "user");
    UserMsgCopy->SetStringField("content", PlayerMessage);
    MessageHistory.Add(MakeShareable(new FJsonValueObject(UserMsgCopy)));

    JsonObject->SetArrayField("messages", Messages);

    FString JsonStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    Request->SetContentAsString(JsonStr);
    Request->OnProcessRequestComplete().BindUObject(this, &USparkBrainComponent::OnResponseReceived);
    Request->ProcessRequest();
}

void USparkBrainComponent::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid()) return;

    FString ResponseStr = Response->GetContentAsString();
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject)) return;

    const TArray<TSharedPtr<FJsonValue>>* Choices;
    if (!JsonObject->TryGetArrayField(TEXT("choices"), Choices)) return;
    if (Choices->Num() == 0) return;

    TSharedPtr<FJsonObject> ChoiceObj = (*Choices)[0]->AsObject();
    if (!ChoiceObj.IsValid()) return;

    TSharedPtr<FJsonObject> MessageObj = ChoiceObj->GetObjectField(TEXT("message"));
    if (!MessageObj.IsValid()) return;

    FString Content;
    if (!MessageObj->TryGetStringField(TEXT("content"), Content)) return;

    Content.TrimStartAndEndInline();
    LastResponse = Content;

    FString PlayerMessage;
    if (MessageHistory.Num() > 0)
    {
        TSharedPtr<FJsonObject> LastUserMsg = MessageHistory.Last()->AsObject();
        if (LastUserMsg.IsValid())
        {
            LastUserMsg->TryGetStringField(TEXT("content"), PlayerMessage);
        }
    }

    TSharedPtr<FJsonObject> AssistantMsg = MakeShareable(new FJsonObject);
    AssistantMsg->SetStringField("role", "assistant");
    AssistantMsg->SetStringField("content", Content);
    MessageHistory.Add(MakeShareable(new FJsonValueObject(AssistantMsg)));

    TrimHistory();

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
