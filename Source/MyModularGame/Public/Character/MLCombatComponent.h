#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MLCombatComponent.generated.h"

class AMLWeapon;
class APawn;
class AController;
class APlayerState;

UCLASS(ClassGroup = (ML), meta = (BlueprintSpawnableComponent))
class MYMODULARGAME_API UMLCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMLCombatComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void StartFire();
    void StopFire();

    UFUNCTION(Server, Reliable)
    void ServerStartFire();

    UFUNCTION(Server, Reliable)
    void ServerStopFire();

    APawn* GetOwnerPawn() const;
    AController* GetOwnerController() const;
    APlayerState* GetOwnerPlayerState() const;

private:
    void HandleStartFireRequest(const TCHAR* Source);
    void HandleStopFireRequest(const TCHAR* Source);
    void FireOnce(const TCHAR* Source);

    float GetEffectiveFireRate() const;
    float GetShotIntervalSeconds() const;
    FString BuildOwnerDebugString() const;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float FireRate = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float FireRange = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    TObjectPtr<AMLWeapon> EquippedWeapon;

    bool bServerFiring = false;
    float ShotTimeAccumulator = 0.0f;
    int32 ServerShotCount = 0;
};
