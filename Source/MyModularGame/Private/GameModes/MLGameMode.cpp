#include "GameModes/MLGameMode.h"

#include "GameModes/MLGameState.h"
#include "System/MLExperienceDefinition.h"

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
