#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "GameplayTagContainer.h"
#include "MyPawnInitComponent.generated.h"

struct FActorInitStateChangedParams;
class UGameFrameworkComponentManager;
class APawn;
class AController;

/**
 * Lyra 스타일 InitState 체인을 최소 구성으로 구현한 Pawn 컴포넌트.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYMODULARGAME_API UMyPawnInitComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
    GENERATED_BODY()

public:
    UMyPawnInitComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //~IGameFrameworkInitStateInterface
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
    //~End IGameFrameworkInitStateInterface

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void HandleControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);
    bool HasValidController() const;
    bool HasValidPlayerState() const;

private:
    static const FName NAME_ActorFeatureName;

    FGameplayTag CurrentInitState;
};
