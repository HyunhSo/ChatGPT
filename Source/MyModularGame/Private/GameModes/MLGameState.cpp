#include "GameModes/MLGameState.h"

#include "Net/UnrealNetwork.h"
#include "System/MLExperienceDefinition.h"

AMLGameState::AMLGameState()
    : bExperienceReady(false)
{
}

void AMLGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMLGameState, ExperienceDefinition);
    DOREPLIFETIME(AMLGameState, bExperienceReady);
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
