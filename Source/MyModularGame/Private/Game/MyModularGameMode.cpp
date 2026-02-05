#include "Game/MyModularGameMode.h"

#include "Character/MyModularPawn.h"
#include "Game/MyModularGameState.h"
#include "Player/MyModularPlayerController.h"
#include "Player/MyModularPlayerState.h"

AMyModularGameMode::AMyModularGameMode()
{
    DefaultPawnClass = AMyModularPawn::StaticClass();
    PlayerControllerClass = AMyModularPlayerController::StaticClass();
    PlayerStateClass = AMyModularPlayerState::StaticClass();
    GameStateClass = AMyModularGameState::StaticClass();
}
