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
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

void UNeonPatrolChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bWidgetCreated = false;
    bSparkSpawned = false;
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

    // Auto-spawn Spark if not in level (works in packaged builds)
    if (!bSparkSpawned)
    {
        EnsureSparkExists();
        bSparkSpawned = true;
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

    bool bChatOpen = ChatWidget && ChatWidget->IsChatVisible();

    // Enter key: open chat when closed, let text input handle it when open
    bool bEnterDown = PC->IsInputKeyDown(EKeys::Enter);
    if (bEnterDown && !bEnterWasDown)
    {
        if (!bChatOpen)
        {
            ChatWidget->ShowChat();
        }
        // When chat IS open, Enter is handled by the text input's OnTextCommitted
    }
    bEnterWasDown = bEnterDown;

    // Tab key: close chat
    bool bTabDown = PC->IsInputKeyDown(EKeys::Tab);
    if (bTabDown && !bEscWasDown)
    {
        if (bChatOpen)
        {
            ChatWidget->HideChat();
        }
    }
    bEscWasDown = bTabDown;

    // V key = push-to-talk voice (close chat first if open)
    bool bVKeyDown = PC->IsInputKeyDown(EKeys::V);
    if (bVKeyDown && !bVKeyWasDown)
    {
        if (bChatOpen)
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

void UNeonPatrolChatSubsystem::EnsureSparkExists()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Check if a SparkCharacter already exists
    for (TActorIterator<ASparkCharacter> It(World); It; ++It)
    {
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark already exists in level"));
        return;
    }

    // Spawn Spark near the player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector SpawnLoc = PlayerPawn->GetActorLocation() + FVector(200.f, 100.f, 0.f);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ASparkCharacter* Spark = World->SpawnActor<ASparkCharacter>(ASparkCharacter::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
    if (Spark)
    {
        // Set visible sphere mesh
        TArray<UStaticMeshComponent*> MeshComps;
        Spark->GetComponents<UStaticMeshComponent>(MeshComps);
        for (UStaticMeshComponent* MC : MeshComps)
        {
            UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
            if (SphereMesh)
            {
                MC->SetStaticMesh(SphereMesh);
                MC->SetRelativeScale3D(FVector(0.5f));
            }
        }
        Spark->SetActorScale3D(FVector(0.7f));
        Spark->FollowTarget = PlayerPawn;

        UE_LOG(LogNeonPatrol, Log, TEXT("Spark auto-spawned near player at %s"), *SpawnLoc.ToString());
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
