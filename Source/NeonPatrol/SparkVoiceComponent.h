#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "SparkVoiceComponent.generated.h"

class USparkBrainComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceTranscribed, FString, TranscribedText);

UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class NEONPATROL_API USparkVoiceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USparkVoiceComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SparkVoice")
    FString AudioBridgeURL = TEXT("http://127.0.0.1:7777");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SparkVoice")
    float RecordDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SparkVoice")
    bool bAutoSpeak = true;

    UPROPERTY(BlueprintAssignable, Category = "SparkVoice")
    FOnVoiceTranscribed OnVoiceTranscribed;

    UFUNCTION(BlueprintCallable, Category = "SparkVoice")
    void StartVoiceChat();

    UFUNCTION(BlueprintCallable, Category = "SparkVoice")
    void SpeakText(const FString& Text);

    UFUNCTION(BlueprintCallable, Category = "SparkVoice")
    bool IsRecording() const;

protected:
    virtual void BeginPlay() override;

private:
    bool bRecording = false;

    UPROPERTY()
    USparkBrainComponent* BrainRef = nullptr;

    UFUNCTION()
    void OnSparkChatResponse(FString PlayerMsg, FString SparkMsg);

    void OnSTTResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
