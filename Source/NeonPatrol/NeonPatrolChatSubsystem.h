#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NeonPatrolChatSubsystem.generated.h"

class UChatOverlayWidget;

UCLASS()
class NEONPATROL_API UNeonPatrolChatSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

    UFUNCTION(BlueprintCallable, Category = "Chat")
    void ToggleChat();

private:
    UPROPERTY()
    UChatOverlayWidget* ChatWidget = nullptr;

    bool bWidgetCreated = false;
    bool bEnterWasDown = false;
    float InitDelay = 0.5f;
};
