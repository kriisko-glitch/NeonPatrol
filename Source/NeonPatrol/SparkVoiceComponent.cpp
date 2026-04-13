#include "SparkVoiceComponent.h"
#include "SparkBrainComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Engine.h"

USparkVoiceComponent::USparkVoiceComponent()
{
    AudioBridgeURL = TEXT("http://127.0.0.1:7777");
    RecordDuration = 5.0f;
    bAutoSpeak = true;
    bRecording = false;
    BrainRef = nullptr;
}

void USparkVoiceComponent::BeginPlay()
{
    Super::BeginPlay();
    BrainRef = GetOwner()->FindComponentByClass<USparkBrainComponent>();
    if (BrainRef && bAutoSpeak)
    {
        BrainRef->OnChatResponse.AddDynamic(this, &USparkVoiceComponent::OnSparkChatResponse);
    }
}

void USparkVoiceComponent::StartVoiceChat()
{
    if (bRecording)
    {
        return;
    }
    bRecording = true;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(AudioBridgeURL + TEXT("/stt"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetNumberField(TEXT("duration"), RecordDuration);
    FString JsonStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    Request->SetContentAsString(JsonStr);
    Request->OnProcessRequestComplete().BindUObject(this, &USparkVoiceComponent::OnSTTResponseReceived);
    Request->ProcessRequest();
}

void USparkVoiceComponent::OnSTTResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    bRecording = false;
    if (bWasSuccessful && Response.IsValid())
    {
        FString JsonStr = Response->GetContentAsString();
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            FString TranscribedText;
            if (JsonObject->TryGetStringField(TEXT("text"), TranscribedText) && !TranscribedText.IsEmpty())
            {
                if (BrainRef)
                {
                    BrainRef->SendChat(TranscribedText);
                }
                OnVoiceTranscribed.Broadcast(TranscribedText);
            }
        }
    }
}

void USparkVoiceComponent::SpeakText(const FString& Text)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(AudioBridgeURL + TEXT("/tts"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("text"), Text);
    FString JsonStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    Request->SetContentAsString(JsonStr);
    Request->ProcessRequest();
}

void USparkVoiceComponent::OnSparkChatResponse(FString PlayerMsg, FString SparkMsg)
{
    if (bAutoSpeak)
    {
        SpeakText(SparkMsg);
    }
}

bool USparkVoiceComponent::IsRecording() const
{
    return bRecording;
}
