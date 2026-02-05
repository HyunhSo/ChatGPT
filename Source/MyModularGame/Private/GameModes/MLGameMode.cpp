#include "GameModes/MLGameMode.h"

#include "Character/MLCharacter.h"
#include "GameModes/MLGameState.h"
#include "Player/MLPlayerController.h"
#include "Player/MyModularPlayerState.h"
#include "System/MLExperienceDefinition.h"

AMLGameMode::AMLGameMode()
{
    DefaultPawnClass = AMLCharacter::StaticClass();
    PlayerControllerClass = AMLPlayerController::StaticClass();
    PlayerStateClass = AMyModularPlayerState::StaticClass();
    GameStateClass = AMLGameState::StaticClass();
}

void AMLGameMode::InitGameState()
{
    Super::InitGameState();

    AMLGameState* MLGameState = GetGameState<AMLGameState>();
    if (!MLGameState)
    {
        UE_LOG(LogTemp, Warning, TEXT("MLGameMode: Expected AMLGameState but found a different class."));
        return;
    }

    MLGameState->SetCurrentExperience(DefaultExperienceDefinition);

    UE_LOG(LogTemp, Log, TEXT("MLGameMode: Experience set to %s"), *GetNameSafe(DefaultExperienceDefinition));
}
