#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MLWeapon.generated.h"

UCLASS()
class MYMODULARGAME_API AMLWeapon : public AActor
{
    GENERATED_BODY()

public:
    AMLWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    float GetFireRate() const { return FireRate; }
    float GetFireRange() const { return FireRange; }

private:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float FireRate = 8.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float FireRange = 8000.0f;
};
