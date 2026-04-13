#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "SparkBrainComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatResponse, FString, PlayerMessage, FString, SparkResponse);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEONPATROL_API USparkBrainComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USparkBrainComponent();

    UPROPERTY(EditAnywhere, Category="SparkBrain")
    FString EndpointURL = TEXT("https://api.groq.com/openai/v1/chat/completions");

    UPROPERTY(EditAnywhere, Category="SparkBrain")
    FString ModelName = TEXT("llama-3.3-70b-versatile");

    UPROPERTY(VisibleAnywhere, Category="SparkBrain")
    FString ApiKey;

    UPROPERTY(EditAnywhere, Category="SparkBrain")
    FString ApiKeyFilePath = TEXT("C:/Users/Kris/Kriisko-Studio/tools/.groq_key");

    UPROPERTY(EditAnywhere, Category="SparkBrain", meta=(MultiLine=true))
    FString SystemPrompt = TEXT("You are Spark, a small combat robot companion. You're loyal, slightly sarcastic, and brave. You love fighting alongside your patrol partner. Keep responses under 20 words. You're in a sci-fi facility fighting malfunctioning robots.");

    UPROPERTY(EditAnywhere, Category="SparkBrain")
    int32 MaxTokens = 64;

    UPROPERTY(EditAnywhere, Category="SparkBrain")
    float Temperature = 0.7f;

    UPROPERTY(BlueprintAssignable, Category="SparkBrain")
    FOnChatResponse OnChatResponse;

    UFUNCTION(BlueprintCallable, Category="SparkBrain")
    void SendChat(const FString& PlayerMessage);

    UFUNCTION(BlueprintCallable, Category="SparkBrain")
    FString GetLastResponse() const;

protected:
    virtual void BeginPlay() override;

private:
    TArray<TSharedPtr<FJsonValue>> MessageHistory;
    FString LastResponse;

    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void TrimHistory();
};
