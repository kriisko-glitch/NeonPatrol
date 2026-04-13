#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatOverlayWidget.generated.h"

class UScrollBox;
class UEditableTextBox;
class UButton;
class UTextBlock;
class USparkBrainComponent;
class UCanvasPanel;
class UCanvasPanelSlot;
class UVerticalBox;
class UBorder;

UCLASS(BlueprintType)
class NEONPATROL_API UChatOverlayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void ToggleChatPanel();

    UFUNCTION(BlueprintCallable, Category = "Chat")
    bool IsChatVisible() const { return bChatVisible; }

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void ShowChat();

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void HideChat();

    UFUNCTION()
    void OnSendClicked();

    UFUNCTION()
    void OnInputCommitted(const FText& Text, ETextCommit::Type Type);

    UFUNCTION()
    void OnSparkResponse(FString PlayerMsg, FString SparkMsg);

    UFUNCTION()
    void OnVoiceTranscribed(FString TranscribedText);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
    int32 MaxMessages = 50;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void AddMessage(const FString& Sender, const FString& Message);

private:
    void FindBrainComponent();

    UPROPERTY()
    UBorder* ChatPanel = nullptr;

    UPROPERTY()
    UScrollBox* ChatScrollBox = nullptr;

    UPROPERTY()
    UEditableTextBox* InputBox = nullptr;

    UPROPERTY()
    UButton* SendBtn = nullptr;

    UPROPERTY()
    USparkBrainComponent* BrainRef = nullptr;

    bool bChatVisible = false;
};
