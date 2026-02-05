#include "Game/MyModularGameState.h"

#include "Data/MyGameData.h"
#include "Net/UnrealNetwork.h"

const UMyGameData* AMyModularGameState::GetGameData() const
{
    return GameData;
}

void AMyModularGameState::SetGameData(const UMyGameData* InGameData)
{
    GameData = InGameData;
}

void AMyModularGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyModularGameState, GameData);
}
