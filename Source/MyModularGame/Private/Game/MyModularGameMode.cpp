#include "Game/MyModularGameMode.h"

#include "Character/MyModularPawn.h"
#include "Data/MyGameData.h"
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

void AMyModularGameMode::InitGameState()
{
    Super::InitGameState();

    if (AMyModularGameState* MyGameState = GetGameState<AMyModularGameState>())
    {
        MyGameState->SetGameData(GameData);

        UE_LOG(LogTemp, Log, TEXT("[GameData] GameMode applied GameData=%s"), *GetNameSafe(GameData));
    }
}
