#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SparkBrainComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatResponse, FString, PlayerMessage, FString, SparkResponse);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class NEONPATROL_API USparkBrainComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USparkBrainComponent();

    UFUNCTION(BlueprintCallable, Category = "SparkBrain")
    void SendChat(const FString& PlayerMessage);

    UFUNCTION(BlueprintCallable, Category = "SparkBrain")
    FString GetLastResponse() const;

    UPROPERTY(BlueprintAssignable, Category = "SparkBrain")
    FOnChatResponse OnChatResponse;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, Category = "SparkBrain")
    FString EndpointURL = TEXT("http://127.0.0.1:11001/v1/chat/completions");

    UPROPERTY(EditAnywhere, Category = "SparkBrain")
    FString ModelName = TEXT("qwen3.5-0.8b");

    UPROPERTY(EditAnywhere, Category = "SparkBrain", meta = (MultiLine = true))
    FString SystemPrompt = TEXT("You are Spark, a small combat robot companion. You're loyal, slightly sarcastic, and brave. You love fighting alongside your patrol partner. Keep responses under 20 words. You're in a sci-fi facility fighting malfunctioning robots.");

    UPROPERTY(EditAnywhere, Category = "SparkBrain")
    int32 MaxTokens = 32;

    UPROPERTY(EditAnywhere, Category = "SparkBrain")
    float Temperature = 0.7f;

    TArray<TSharedPtr<FJsonValue>> MessageHistory;
    FString LastResponse;

    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void TrimHistory();
};
