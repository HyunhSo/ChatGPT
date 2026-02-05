#include "GameModes/MLGameMode.h"

#include "Character/MLCharacter.h"
#include "Character/MLPawnData.h"
#include "GameFramework/WorldSettings.h"
#include "GameModes/MLGameState.h"
#include "Player/MLPlayerController.h"
#include "Player/MLPlayerState.h"
#include "System/MLExperienceDefinition.h"

AMLGameMode::AMLGameMode()
{
    DefaultPawnClass = AMLCharacter::StaticClass();
    PlayerControllerClass = AMLPlayerController::StaticClass();
    PlayerStateClass = AMLPlayerState::StaticClass();
    GameStateClass = AMLGameState::StaticClass();

    DefaultExperiencePath = FSoftObjectPath(TEXT("/Game/System/Experiences/B_ML_DefaultExperience.B_ML_DefaultExperience"));
    DefaultPawnDataPath = FSoftObjectPath();
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

    FName PawnDataSource = NAME_None;
    const UMLPawnData* ResolvedDefaultPawnData = ResolveDefaultPawnData(PawnDataSource);
    MLGameState->SetDefaultPawnData(ResolvedDefaultPawnData, PawnDataSource);

    UE_LOG(LogTemp, Log, TEXT("MLGameMode: Experience set to %s (%s), DefaultPawnData=%s (%s), Source=%s"),
        *GetNameSafe(ResolvedExperience),
        *GetPathNameSafe(ResolvedExperience),
        *GetNameSafe(ResolvedDefaultPawnData),
        *GetPathNameSafe(ResolvedDefaultPawnData),
        *PawnDataSource.ToString());
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

    UE_LOG(LogTemp, Warning, TEXT("MLGameMode: No experience configured. Continuing with null ExperienceDefinition for PR3 dev flow."));
    return nullptr;
}

const UMLPawnData* AMLGameMode::ResolveDefaultPawnData(FName& OutPawnDataSource) const
{
    OutPawnDataSource = NAME_None;

    if (const AMLGameState* MLGameState = GetGameState<AMLGameState>())
    {
        if (const UMLExperienceDefinition* Experience = MLGameState->GetCurrentExperience())
        {
            if (Experience->DefaultPawnData)
            {
                OutPawnDataSource = TEXT("Experience.DefaultPawnData");
                return Experience->DefaultPawnData;
            }
        }
    }

    if (DefaultPawnData)
    {
        OutPawnDataSource = TEXT("GameMode.DefaultPawnData");
        return DefaultPawnData;
    }

    if (!DefaultPawnDataPath.IsNull())
    {
        if (UObject* LoadedObject = DefaultPawnDataPath.TryLoad())
        {
            if (const UMLPawnData* LoadedPawnData = Cast<UMLPawnData>(LoadedObject))
            {
                OutPawnDataSource = TEXT("GameMode.DefaultPawnDataPath");
                return LoadedPawnData;
            }

            UE_LOG(LogTemp, Error, TEXT("MLGameMode: Config path '%s' did not resolve to UMLPawnData (resolved: %s)."),
                *DefaultPawnDataPath.ToString(),
                *GetNameSafe(LoadedObject));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MLGameMode: Failed to load config pawn data path '%s'."), *DefaultPawnDataPath.ToString());
        }
    }

    if (const AWorldSettings* WorldSettings = GetWorld() ? GetWorld()->GetWorldSettings() : nullptr)
    {
        if (const AMLGameMode* DefaultGM = WorldSettings->DefaultGameMode ? Cast<AMLGameMode>(WorldSettings->DefaultGameMode->GetDefaultObject()) : nullptr)
        {
            if (DefaultGM->DefaultPawnData)
            {
                OutPawnDataSource = TEXT("WorldSettings.DefaultGameMode.DefaultPawnData");
                return DefaultGM->DefaultPawnData;
            }
        }
    }

    return nullptr;
}
