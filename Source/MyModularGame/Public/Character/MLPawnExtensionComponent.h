#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "MLPawnExtensionComponent.generated.h"

class UMLPawnData;

UENUM(BlueprintType)
enum class EMLPawnInitState : uint8
{
    Spawned,
    DataAvailable,
    DataInitialized,
    GameplayReady
};

/**
 * Minimal Lyra-style pawn init-state orchestrator.
 */
UCLASS(ClassGroup = (ML), meta = (BlueprintSpawnableComponent))
class MYMODULARGAME_API UMLPawnExtensionComponent : public UPawnComponent
{
    GENERATED_BODY()

public:
    UMLPawnExtensionComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    EMLPawnInitState GetInitState() const { return InitState; }
    const UMLPawnData* GetPawnData() const { return PawnData; }

    void DumpInitState() const;

private:
    void TryProgressInitState();
    bool CanEnterDataAvailable() const;
    bool CanEnterDataInitialized();
    bool CanEnterGameplayReady() const;
    void SetInitState(EMLPawnInitState NewState);

private:
    UPROPERTY(VisibleInstanceOnly, Category = "InitState")
    EMLPawnInitState InitState = EMLPawnInitState::Spawned;

    UPROPERTY(VisibleInstanceOnly, Category = "InitState")
    TObjectPtr<const UMLPawnData> PawnData;
};
