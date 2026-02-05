#include "GameModes/MLGameState.h"

#include "Character/MLPawnData.h"
#include "Net/UnrealNetwork.h"
#include "System/MLExperienceDefinition.h"
#include "Character/MLPawnData.h"

AMLGameState::AMLGameState()
    : bExperienceReady(false)
    , PawnDataSource(NAME_None)
{
}

void AMLGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMLGameState, ExperienceDefinition);
    DOREPLIFETIME(AMLGameState, DefaultPawnData);
    DOREPLIFETIME(AMLGameState, bExperienceReady);
    DOREPLIFETIME(AMLGameState, DefaultPawnData);
    DOREPLIFETIME(AMLGameState, PawnDataSource);
}

void AMLGameState::SetCurrentExperience(const UMLExperienceDefinition* InExperienceDefinition)
{
    if (!HasAuthority())
    {
        return;
    }

    ExperienceDefinition = InExperienceDefinition;
    bExperienceReady = (ExperienceDefinition != nullptr);

    BroadcastExperienceReadyIfNeeded();
}

void AMLGameState::SetDefaultPawnData(const UMLPawnData* InPawnData, FName InPawnDataSource)
{
    if (!HasAuthority())
    {
        return;
    }

    DefaultPawnData = InPawnData;
    PawnDataSource = InPawnDataSource;
}

void AMLGameState::OnRep_ExperienceDefinition()
{
    BroadcastExperienceReadyIfNeeded();
}

void AMLGameState::OnRep_ExperienceReady()
{
    BroadcastExperienceReadyIfNeeded();
}

void AMLGameState::BroadcastExperienceReadyIfNeeded()
{
    if (bExperienceReady)
    {
        OnExperienceReadyDelegate.Broadcast();
    }
}

void AMLGameState::OnRep_DefaultPawnData()
{
}

void AMLGameState::OnRep_PawnDataSource()
{
}
