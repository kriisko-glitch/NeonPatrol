#include "ChatWidget.h"
#include "SparkBrainComponent.h"
#include "SparkCharacter.h"
#include "NeonPatrol.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"

void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	for (TActorIterator<ASparkCharacter> It(GetWorld()); It; ++It)
	{
		BrainRef = It->FindComponentByClass<USparkBrainComponent>();
		if (BrainRef)
			break;
	}

	if (SendButton)
		SendButton->OnClicked.AddDynamic(this, &UChatWidget::OnSendClicked);

	if (ChatInput)
		ChatInput->OnTextCommitted.AddDynamic(this, &UChatWidget::OnTextCommitted);

	if (BrainRef)
		BrainRef->OnChatResponse.AddDynamic(this, &UChatWidget::OnSparkResponse);

	SetVisibility(ESlateVisibility::Collapsed);
}

void UChatWidget::ToggleChat()
{
	if (GetVisibility() == ESlateVisibility::Collapsed)
	{
		SetVisibility(ESlateVisibility::Visible);
		if (APlayerController* PC = GetOwningPlayer())
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ChatInput->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
		ChatInput->SetFocus();
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
		if (APlayerController* PC = GetOwningPlayer())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
		}
	}
}

void UChatWidget::OnSendClicked()
{
	if (!ChatInput || !BrainRef) return;
	FText Raw = ChatInput->GetText();
	if (Raw.IsEmpty()) return;
	FString Msg = Raw.ToString();
	AddMessage("You", Msg);
	BrainRef->SendChat(Msg);
	ChatInput->SetText(FText::GetEmpty());
}

void UChatWidget::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
		OnSendClicked();
}

void UChatWidget::OnSparkResponse(FString PlayerMsg, FString SparkMsg)
{
	AddMessage("Spark", SparkMsg);
}

void UChatWidget::AddMessage(const FString& Sender, const FString& Message)
{
	if (!ChatHistory) return;
	UTextBlock* Txt = NewObject<UTextBlock>(this);
	Txt->SetText(FText::FromString(FString::Printf(TEXT("%s: %s"), *Sender, *Message)));
	FSlateFontInfo Font = Txt->GetFont();
	Font.Size = 16;
	Txt->SetFont(Font);
	if (Sender == "Spark")
	{
		Txt->SetColorAndOpacity(FLinearColor(0.0f, 1.0f, 1.0f, 1.0f)); // Cyan
	}
	else
	{
		Txt->SetColorAndOpacity(FLinearColor::White);
	}
	ChatHistory->AddChild(Txt);
	if (ChatHistory->GetChildrenCount() > MaxMessages)
	{
		if (UWidget* First = ChatHistory->GetChildAt(0))
			ChatHistory->RemoveChild(First);
	}
	ChatHistory->ScrollToEnd();
}
