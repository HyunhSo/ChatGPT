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

UENUM()
enum class EMLPawnDataFallbackSource : uint8
{
    None,
    Experience,
    GameModeDefault,
    Missing
};

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
    EMLPawnDataFallbackSource GetPawnDataFallbackSource() const { return PawnDataFallbackSource; }
    const TCHAR* GetFallbackSourceText() const;

    void DumpInitState() const;

private:
    bool HasController() const;
    bool HasPlayerState() const;
    bool IsExperienceReady() const;
    bool ResolvePawnData();

    static const FName NAME_ActorFeatureName;

private:
    UPROPERTY(VisibleInstanceOnly, Category = "InitState")
    FGameplayTag CurrentInitState;

    UPROPERTY(VisibleInstanceOnly, Category = "InitState")
    TObjectPtr<const UMLPawnData> PawnData;

    UPROPERTY(VisibleInstanceOnly, Category = "InitState")
    EMLPawnDataFallbackSource PawnDataFallbackSource;

    UPROPERTY(VisibleInstanceOnly, Category = "InitState")
    bool bGameplayReadyHandled;
};
