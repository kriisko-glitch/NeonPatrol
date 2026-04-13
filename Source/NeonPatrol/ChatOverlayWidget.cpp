#include "ChatOverlayWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "SparkBrainComponent.h"
#include "SparkCharacter.h"
#include "NeonPatrol.h"
#include "EngineUtils.h"

void UChatOverlayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Root canvas so we can anchor bottom-left
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;

    // Dark background panel
    ChatPanel = WidgetTree->ConstructWidget<UBorder>();
    ChatPanel->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.06f, 0.85f));
    ChatPanel->SetPadding(FMargin(10.f));

    UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(ChatPanel);
    // Anchor bottom-left
    PanelSlot->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));
    PanelSlot->SetAlignment(FVector2D(0.f, 1.f));
    PanelSlot->SetPosition(FVector2D(20.f, -20.f));
    PanelSlot->SetSize(FVector2D(420.f, 280.f));

    // Vertical layout inside the panel
    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
    ChatPanel->SetContent(VBox);

    // Chat history scroll box
    USizeBox* ScrollSizeBox = WidgetTree->ConstructWidget<USizeBox>();
    ScrollSizeBox->SetHeightOverride(200.f);

    ChatScrollBox = WidgetTree->ConstructWidget<UScrollBox>();
    ScrollSizeBox->AddChild(ChatScrollBox);

    UVerticalBoxSlot* ScrollSlot = VBox->AddChildToVerticalBox(ScrollSizeBox);
    ScrollSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    // Input row
    UHorizontalBox* InputRow = WidgetTree->ConstructWidget<UHorizontalBox>();
    VBox->AddChildToVerticalBox(InputRow);

    // Text input
    InputBox = WidgetTree->ConstructWidget<UEditableTextBox>();
    InputBox->SetHintText(FText::FromString(TEXT("Talk to Spark...")));
    UHorizontalBoxSlot* InputSlot = InputRow->AddChildToHorizontalBox(InputBox);
    InputSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    InputSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));

    // Send button
    SendBtn = WidgetTree->ConstructWidget<UButton>();
    UTextBlock* SendLabel = WidgetTree->ConstructWidget<UTextBlock>();
    SendLabel->SetText(FText::FromString(TEXT("Send")));
    SendBtn->AddChild(SendLabel);
    InputRow->AddChildToHorizontalBox(SendBtn);

    // Bind events
    SendBtn->OnClicked.AddDynamic(this, &UChatOverlayWidget::OnSendClicked);
    InputBox->OnTextCommitted.AddDynamic(this, &UChatOverlayWidget::OnInputCommitted);

    // Find Spark's brain
    FindBrainComponent();

    // Start hidden
    ChatPanel->SetVisibility(ESlateVisibility::Collapsed);
    bChatVisible = false;

    // Add a welcome message
    AddMessage(TEXT("Spark"), TEXT("Hey partner! Press Enter to chat with me."));
}

void UChatOverlayWidget::FindBrainComponent()
{
    if (BrainRef) return;

    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ASparkCharacter> It(World); It; ++It)
    {
        BrainRef = It->FindComponentByClass<USparkBrainComponent>();
        if (BrainRef)
        {
            BrainRef->OnChatResponse.AddDynamic(this, &UChatOverlayWidget::OnSparkResponse);
            UE_LOG(LogNeonPatrol, Log, TEXT("ChatOverlay: Found SparkBrain"));
            break;
        }
    }

    if (!BrainRef)
    {
        UE_LOG(LogNeonPatrol, Warning, TEXT("ChatOverlay: No SparkCharacter with brain found"));
    }
}

void UChatOverlayWidget::ToggleChatPanel()
{
    if (bChatVisible)
    {
        HideChat();
    }
    else
    {
        ShowChat();
    }
}

void UChatOverlayWidget::ShowChat()
{
    if (!ChatPanel) return;
    ChatPanel->SetVisibility(ESlateVisibility::Visible);
    bChatVisible = true;

    // Try to find brain if not found yet (Spark may have spawned late)
    FindBrainComponent();

    if (InputBox)
    {
        InputBox->SetKeyboardFocus();
    }

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameAndUI Mode;
        Mode.SetWidgetToFocus(InputBox ? InputBox->TakeWidget() : TSharedPtr<SWidget>());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(Mode);
        PC->SetShowMouseCursor(true);
    }
}

void UChatOverlayWidget::HideChat()
{
    if (!ChatPanel) return;
    ChatPanel->SetVisibility(ESlateVisibility::Collapsed);
    bChatVisible = false;

    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetShowMouseCursor(false);
    }
}

void UChatOverlayWidget::OnSendClicked()
{
    if (!InputBox) return;

    FText InputText = InputBox->GetText();
    if (InputText.IsEmpty()) return;

    FString Message = InputText.ToString();
    AddMessage(TEXT("You"), Message);

    if (BrainRef)
    {
        BrainRef->SendChat(Message);
    }
    else
    {
        AddMessage(TEXT("Spark"), TEXT("(Spark's brain is offline)"));
    }

    InputBox->SetText(FText::GetEmpty());
    InputBox->SetKeyboardFocus();
}

void UChatOverlayWidget::OnInputCommitted(const FText& Text, ETextCommit::Type Type)
{
    if (Type == ETextCommit::OnEnter)
    {
        OnSendClicked();
    }
}

void UChatOverlayWidget::OnSparkResponse(FString PlayerMsg, FString SparkMsg)
{
    AddMessage(TEXT("Spark"), SparkMsg);
}

void UChatOverlayWidget::AddMessage(const FString& Sender, const FString& Message)
{
    if (!ChatScrollBox) return;

    UTextBlock* MsgText = NewObject<UTextBlock>(this);
    MsgText->SetText(FText::FromString(FString::Printf(TEXT("%s: %s"), *Sender, *Message)));

    FSlateFontInfo Font = MsgText->GetFont();
    Font.Size = 14;
    MsgText->SetFont(Font);

    if (Sender == TEXT("Spark"))
    {
        MsgText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 0.9f, 1.0f, 1.0f)));
    }
    else
    {
        MsgText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));
    }

    ChatScrollBox->AddChild(MsgText);

    while (ChatScrollBox->GetChildrenCount() > MaxMessages)
    {
        if (UWidget* Oldest = ChatScrollBox->GetChildAt(0))
        {
            ChatScrollBox->RemoveChild(Oldest);
        }
    }

    ChatScrollBox->ScrollToEnd();
}
