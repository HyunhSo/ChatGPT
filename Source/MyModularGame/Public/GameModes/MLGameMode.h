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

private:
    const UMLExperienceDefinition* ResolveExperienceDefinition();
    const UMLExperienceDefinition* CreateFallbackExperienceDefinition();

private:
    UPROPERTY(Transient)
    TObjectPtr<UMLExperienceDefinition> FallbackExperienceDefinition;

    UPROPERTY(Transient)
    TObjectPtr<UMLPawnData> FallbackPawnData;
};
