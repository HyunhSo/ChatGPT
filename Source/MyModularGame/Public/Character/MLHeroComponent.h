#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "MLHeroComponent.generated.h"

class APawn;
class UMLPawnData;

/**
 * Handles local player-side hero setup (input/camera).
 */
UCLASS(ClassGroup = (ML), meta = (BlueprintSpawnableComponent))
class MYMODULARGAME_API UMLHeroComponent : public UPawnComponent
{
    GENERATED_BODY()

public:
    void HandleGameplayReady(const UMLPawnData* PawnData);

    bool IsInputSetupDone() const { return bInputSetupDone; }
    bool IsCameraSetupDone() const { return bCameraSetupDone; }

private:
    bool IsLocalPlayerPawn(const APawn* Pawn) const;
    void SetupPlayerInput(const APawn* Pawn, const UMLPawnData* PawnData);
    void SetupPlayerCamera(const APawn* Pawn, const UMLPawnData* PawnData);

private:
    bool bDidLocalSetup = false;
    bool bInputSetupDone = false;
    bool bCameraSetupDone = false;
    bool bLastInputSubsystemAccess = false;
};
