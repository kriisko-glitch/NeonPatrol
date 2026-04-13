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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSparkCommand, FString, CommandName, float, CommandParam);

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
    FString SystemPrompt = TEXT(
        "You are Spark, a small combat robot companion. Loyal, sarcastic, brave. "
        "You're in a sci-fi facility fighting malfunctioning robots with your patrol partner.\n\n"
        "RESPOND IN JSON ONLY: {\"say\":\"your words (max 20 words)\", \"cmd\":\"COMMAND\"}\n\n"
        "Available commands (use NONE if no action needed):\n"
        "FOLLOW - follow the player\n"
        "STAY - stop and wait in place\n"
        "ATTACK - start shooting enemies\n"
        "HOLD_FIRE - stop shooting\n"
        "COME_HERE - move directly to the player\n"
        "MOVE_FORWARD 500 - move forward N units (relative to player facing)\n"
        "MOVE_BACK 500 - move backward N units\n"
        "MOVE_LEFT 500 - move left N units\n"
        "MOVE_RIGHT 500 - move right N units\n\n"
        "Examples:\n"
        "{\"say\":\"On it, moving up!\", \"cmd\":\"MOVE_FORWARD 500\"}\n"
        "{\"say\":\"Holding position.\", \"cmd\":\"STAY\"}\n"
        "{\"say\":\"Guns hot!\", \"cmd\":\"ATTACK\"}\n"
        "{\"say\":\"Sure thing, partner.\", \"cmd\":\"NONE\"}"
    );

    UPROPERTY(EditAnywhere, Category="SparkBrain")
    int32 MaxTokens = 64;

    UPROPERTY(EditAnywhere, Category="SparkBrain")
    float Temperature = 0.7f;

    UPROPERTY(BlueprintAssignable, Category="SparkBrain")
    FOnChatResponse OnChatResponse;

    UPROPERTY(BlueprintAssignable, Category="SparkBrain")
    FOnSparkCommand OnSparkCommand;

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
    void ParseAndExecuteCommand(const FString& RawResponse, FString& OutSay);
};
