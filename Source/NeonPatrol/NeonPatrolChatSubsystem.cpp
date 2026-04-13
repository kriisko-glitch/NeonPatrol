#include "NeonPatrolChatSubsystem.h"
#include "ChatOverlayWidget.h"
#include "NeonPatrol.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

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

    bool bEnterDown = PC->IsInputKeyDown(EKeys::Enter);
    if (bEnterDown && !bEnterWasDown)
    {
        ToggleChat();
    }
    bEnterWasDown = bEnterDown;
}

void UNeonPatrolChatSubsystem::ToggleChat()
{
    if (ChatWidget)
    {
        ChatWidget->ToggleChatPanel();
    }
}
