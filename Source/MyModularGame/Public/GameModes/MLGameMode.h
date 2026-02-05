#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MLGameMode.generated.h"

class UMLExperienceDefinition;
class UMLPawnData;

UCLASS()
class MYMODULARGAME_API AMLGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMLGameMode();

    virtual void InitGameState() override;

protected:
    /** Experience selected by this game mode and pushed into the game state. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience")
    TObjectPtr<const UMLExperienceDefinition> DefaultExperienceDefinition;

    /** Null-safe fallback pawn data when experience is not set. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience")
    TSoftObjectPtr<UMLPawnData> DefaultPawnData;
};
