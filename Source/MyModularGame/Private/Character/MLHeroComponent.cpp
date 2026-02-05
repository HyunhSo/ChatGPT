#include "Character/MLHeroComponent.h"

#include "GameFramework/Pawn.h"

void UMLHeroComponent::HandleGameplayReady()
{
    if (bDidLocalSetup)
    {
        return;
    }

    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn || !Pawn->IsLocallyControlled())
    {
        return;
    }

    bDidLocalSetup = true;

    UE_LOG(LogTemp, Log, TEXT("MLHeroComponent: Local input/camera setup complete for %s (debug stub)."), *GetNameSafe(Pawn));
}
