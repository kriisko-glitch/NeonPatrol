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
#include "SparkBrainComponent.h"
#include "SparkCharacter.h"
#include "NeonPatrol.h"
#include "EngineUtils.h"

TSharedRef<SWidget> UChatOverlayWidget::RebuildWidget()
{
    // Build the widget tree BEFORE Slate generation
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;

    // Dark background panel
    ChatPanel = WidgetTree->ConstructWidget<UBorder>();
    ChatPanel->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.06f, 0.85f));
    ChatPanel->SetPadding(FMargin(10.f));

    UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(ChatPanel);
    PanelSlot->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));
    PanelSlot->SetAlignment(FVector2D(0.f, 1.f));
    PanelSlot->SetPosition(FVector2D(20.f, -20.f));
    PanelSlot->SetSize(FVector2D(420.f, 280.f));
    PanelSlot->SetAutoSize(false);

    // Vertical layout
    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
    ChatPanel->SetContent(VBox);

    // Chat history scroll box inside a size box
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

    return Super::RebuildWidget();
}

void UChatOverlayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind events (widgets exist from RebuildWidget)
    if (SendBtn)
    {
        SendBtn->OnClicked.AddDynamic(this, &UChatOverlayWidget::OnSendClicked);
    }
    if (InputBox)
    {
        InputBox->OnTextCommitted.AddDynamic(this, &UChatOverlayWidget::OnInputCommitted);
    }

    // Find Spark's brain
    FindBrainComponent();

    // Start with chat panel hidden
    if (ChatPanel)
    {
        ChatPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
    bChatVisible = false;

    // Welcome message
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

    FindBrainComponent();

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameAndUI Mode;
        if (InputBox)
        {
            Mode.SetWidgetToFocus(InputBox->TakeWidget());
        }
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
    if (InputBox)
    {
        InputBox->SetKeyboardFocus();
    }
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
