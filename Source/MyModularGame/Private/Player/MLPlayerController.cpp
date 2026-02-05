#include "Player/MLPlayerController.h"

#include "Character/MLPawnExtensionComponent.h"
#include "GameFramework/Pawn.h"

void AMLPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    NotifyPawnExtension(InPawn);
}

void AMLPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    NotifyPawnExtension(GetPawn());
}

void AMLPlayerController::NotifyPawnExtension(APawn* InPawn) const
{
    if (!InPawn)
    {
        return;
    }

    if (UMLPawnExtensionComponent* PawnExtension = InPawn->FindComponentByClass<UMLPawnExtensionComponent>())
    {
        PawnExtension->HandleControllerChanged();
        PawnExtension->HandlePlayerStateAvailable();
    }
}
