#include "NeonPatrolChatSubsystem.h"
#include "ChatOverlayWidget.h"
#include "SparkCharacter.h"
#include "SparkVoiceComponent.h"
#include "NeonPatrol.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void UNeonPatrolChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bWidgetCreated = false;
    bEnterWasDown = false;
    InitDelay = 0.5f;
}

void UNeonPatrolChatSubsystem::Deinitialize()
{
    if (ChatWidget && ChatWidget->IsInViewport())
    {
        ChatWidget->RemoveFromParent();
    }
    ChatWidget = nullptr;
    Super::Deinitialize();
}

bool UNeonPatrolChatSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UNeonPatrolChatSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNeonPatrolChatSubsystem, STATGROUP_Tickables);
}

void UNeonPatrolChatSubsystem::Tick(float DeltaTime)
{
    // Wait a short time for the world to fully initialize
    if (InitDelay > 0.f)
    {
        InitDelay -= DeltaTime;
        return;
    }

    // Create widget once
    if (!bWidgetCreated)
    {
        UWorld* World = GetWorld();
        if (!World) return;

        APlayerController* PC = World->GetFirstPlayerController();
        if (!PC) return;

        ChatWidget = CreateWidget<UChatOverlayWidget>(PC, UChatOverlayWidget::StaticClass());
        if (ChatWidget)
        {
            ChatWidget->AddToViewport(10);
            // Widget starts with panel hidden (NativeConstruct handles this)
            bWidgetCreated = true;
            UE_LOG(LogNeonPatrol, Log, TEXT("Chat overlay created and added to viewport"));
        }
        return;
    }

    // Poll Enter key to toggle chat
    if (!ChatWidget) return;

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Enter key: only OPEN chat when it's closed (not toggle)
    // When chat is open, Enter is handled by the text input (submits message)
    bool bEnterDown = PC->IsInputKeyDown(EKeys::Enter);
    if (bEnterDown && !bEnterWasDown)
    {
        if (ChatWidget && ChatWidget->GetVisibility() != ESlateVisibility::Collapsed)
        {
            // Chat is open — don't interfere, let text input handle Enter
        }
        else
        {
            // Chat is closed — open it
            if (ChatWidget) ChatWidget->ShowChat();
        }
    }
    bEnterWasDown = bEnterDown;

    // Escape key: close chat
    bool bEscDown = PC->IsInputKeyDown(EKeys::Escape);
    if (bEscDown && !bEscWasDown)
    {
        if (ChatWidget) ChatWidget->HideChat();
    }
    bEscWasDown = bEscDown;

    // V key = push-to-talk voice input (works anytime, no chat panel)
    bool bVKeyDown = PC->IsInputKeyDown(EKeys::V);
    if (bVKeyDown && !bVKeyWasDown)
    {
        // Close chat first if open, so player stays in gameplay
        if (ChatWidget && ChatWidget->GetVisibility() != ESlateVisibility::Collapsed)
        {
            ChatWidget->HideChat();
        }
        StartVoiceInput();
    }
    bVKeyWasDown = bVKeyDown;
}

void UNeonPatrolChatSubsystem::ToggleChat()
{
    if (ChatWidget)
    {
        ChatWidget->ToggleChatPanel();
    }
}

void UNeonPatrolChatSubsystem::StartVoiceInput()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ASparkCharacter> It(World); It; ++It)
    {
        if (USparkVoiceComponent* Voice = It->FindComponentByClass<USparkVoiceComponent>())
        {
            if (!Voice->IsRecording())
            {
                UE_LOG(LogNeonPatrol, Log, TEXT("Voice input: recording (V key)"));

                // Just record and send — NO chat panel, stay in gameplay
                // The voice component handles: record → transcribe → send to brain → TTS response
                Voice->StartVoiceChat();
            }
            break;
        }
    }
}
