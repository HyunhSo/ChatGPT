#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "MLHeroComponent.generated.h"

/**
 * Handles local player-side hero setup (input/camera).
 */
UCLASS(ClassGroup = (ML), meta = (BlueprintSpawnableComponent))
class MYMODULARGAME_API UMLHeroComponent : public UPawnComponent
{
    GENERATED_BODY()

public:
    void HandleGameplayReady();

private:
    bool bDidLocalSetup = false;
};
