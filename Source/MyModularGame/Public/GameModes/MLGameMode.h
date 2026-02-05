#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/SoftObjectPath.h"
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
    /** Optional in-class default experience (highest priority). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience")
    TObjectPtr<const UMLExperienceDefinition> DefaultExperienceDefinition;

    /** Config-driven fallback experience path (used when DefaultExperienceDefinition is unset). */
    UPROPERTY(Config, EditDefaultsOnly, Category = "Experience", meta = (AllowedClasses = "/Script/MyModularGame.MLExperienceDefinition"))
    FSoftObjectPath DefaultExperiencePath;

    /** Optional explicit pawn data fallback when experience is null or has no pawn data. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience", meta = (AllowedClasses = "/Script/MyModularGame.MLPawnData"))
    TObjectPtr<const UMLPawnData> DefaultPawnData;

    /** Config-driven fallback pawn data path (used when DefaultPawnData is unset). */
    UPROPERTY(Config, EditDefaultsOnly, Category = "Experience", meta = (AllowedClasses = "/Script/MyModularGame.MLPawnData"))
    FSoftObjectPath DefaultPawnDataPath;

private:
    const UMLExperienceDefinition* ResolveExperienceDefinition();
    const UMLPawnData* ResolveDefaultPawnData(FName& OutPawnDataSource) const;
};
