#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatWidget.generated.h"

class USparkBrainComponent;
class UScrollBox;
class UEditableTextBox;
class UButton;
class UTextBlock;

UCLASS(BlueprintType)
class NEONPATROL_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void ToggleChat();

private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatHistory;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatInput;

	UPROPERTY(meta = (BindWidget))
	UButton* SendButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> ChatMessageClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat", meta = (AllowPrivateAccess = "true"))
	int32 MaxMessages = 20;

	USparkBrainComponent* BrainRef = nullptr;

	UFUNCTION()
	void OnSendClicked();

	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnSparkResponse(FString PlayerMsg, FString SparkMsg);

	void AddMessage(const FString& Sender, const FString& Message);
};
