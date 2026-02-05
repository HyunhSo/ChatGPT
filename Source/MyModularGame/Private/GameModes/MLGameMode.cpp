#include "GameModes/MLGameMode.h"

#include "Character/MLCharacter.h"
#include "Character/MLPawnData.h"
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

    DefaultExperiencePath = FSoftObjectPath(TEXT("/Game/System/Experiences/B_ML_DefaultExperience.B_ML_DefaultExperience"));
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

    const UMLExperienceDefinition* ResolvedExperience = ResolveExperienceDefinition();
    MLGameState->SetCurrentExperience(ResolvedExperience);

    UE_LOG(LogTemp, Log, TEXT("MLGameMode: Experience set to %s (%s)"),
        *GetNameSafe(ResolvedExperience),
        *GetPathNameSafe(ResolvedExperience));
}

const UMLExperienceDefinition* AMLGameMode::ResolveExperienceDefinition()
{
    if (DefaultExperienceDefinition)
    {
        return DefaultExperienceDefinition;
    }

    if (!DefaultExperiencePath.IsNull())
    {
        if (UObject* LoadedObject = DefaultExperiencePath.TryLoad())
        {
            if (const UMLExperienceDefinition* LoadedExperience = Cast<UMLExperienceDefinition>(LoadedObject))
            {
                UE_LOG(LogTemp, Log, TEXT("MLGameMode: Loaded experience from config path '%s'."), *DefaultExperiencePath.ToString());
                return LoadedExperience;
            }

            UE_LOG(LogTemp, Error, TEXT("MLGameMode: Config path '%s' did not resolve to UMLExperienceDefinition (resolved: %s)."),
                *DefaultExperiencePath.ToString(),
                *GetNameSafe(LoadedObject));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MLGameMode: Failed to load config experience path '%s'."), *DefaultExperiencePath.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MLGameMode: DefaultExperiencePath is not configured."));
    }

    UE_LOG(LogTemp, Warning, TEXT("MLGameMode: Falling back to a transient experience/pawn data to keep startup safe during development."));
    return CreateFallbackExperienceDefinition();
}

const UMLExperienceDefinition* AMLGameMode::CreateFallbackExperienceDefinition()
{
    if (!FallbackExperienceDefinition)
    {
        FallbackExperienceDefinition = NewObject<UMLExperienceDefinition>(this, NAME_None, RF_Transient);
    }

    if (!FallbackPawnData)
    {
        FallbackPawnData = NewObject<UMLPawnData>(this, NAME_None, RF_Transient);
        FallbackPawnData->PawnClass = DefaultPawnClass;
    }

    FallbackExperienceDefinition->DefaultPawnData = FallbackPawnData;
    return FallbackExperienceDefinition;
}
