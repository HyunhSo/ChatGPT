#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "GameplayTagContainer.h"
#include "MLPawnExtensionComponent.generated.h"

class UMLPawnData;
struct FActorInitStateChangedParams;
class UGameFrameworkComponentManager;

DECLARE_LOG_CATEGORY_EXTERN(LogMLInit, Log, All);

/**
 * Minimal Lyra-style pawn init-state orchestrator.
 */
UCLASS(ClassGroup = (ML), meta = (BlueprintSpawnableComponent))
class MYMODULARGAME_API UMLPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
    GENERATED_BODY()

public:
    UMLPawnExtensionComponent();

    virtual void BeginPlay() override;

    virtual FName GetFeatureName() const override;
    virtual void CheckDefaultInitialization() override;
    virtual FGameplayTag GetInitState() const override;
    virtual bool HasReachedInitState(FGameplayTag DesiredState) const override;
    virtual void SetInitState(FGameplayTag NewState) override;
    virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
    virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
    virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
    virtual void RegisterInitStateFeature() override;
    virtual void UnregisterInitStateFeature() override;

    void HandleControllerChanged();
    void HandlePlayerStateAvailable();

    FGameplayTag GetCurrentInitState() const { return CurrentInitState; }
    const UMLPawnData* GetPawnData() const { return PawnData; }

    void DumpInitState() const;

private:
    bool HasController() const;
    bool HasPlayerState() const;
    bool IsExperienceReady() const;

    static const FName NAME_ActorFeatureName;

private:
    UPROPERTY(VisibleInstanceOnly, Category = "InitState")
    FGameplayTag CurrentInitState;

    UPROPERTY(VisibleInstanceOnly, Category = "InitState")
    TObjectPtr<const UMLPawnData> PawnData;
};
