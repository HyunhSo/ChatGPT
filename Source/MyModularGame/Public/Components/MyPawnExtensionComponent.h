#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "GameplayTagContainer.h"
#include "MyPawnExtensionComponent.generated.h"

struct FActorInitStateChangedParams;
class UGameFrameworkComponentManager;
class UMyPawnData;
class APawn;
class AController;

/**
 * Pawn/Controller/PlayerState/GameState 준비 상태를 기반으로 InitState 체인을 진행한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYMODULARGAME_API UMyPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
    GENERATED_BODY()

public:
    UMyPawnExtensionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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

    UFUNCTION(BlueprintPure, Category = "PawnData")
    const UMyPawnData* GetPawnData() const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void HandleControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);
    bool HasValidController() const;
    bool HasValidPlayerState() const;
    bool HasValidGameData() const;
    bool ResolvePawnDataFromGameData();

private:
    static const FName NAME_ActorFeatureName;

    FGameplayTag CurrentInitState;

    UPROPERTY(Transient)
    TObjectPtr<const UMyPawnData> PawnData = nullptr;
};
