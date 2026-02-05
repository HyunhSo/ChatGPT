#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MLGameState.generated.h"

class UMLExperienceDefinition;

DECLARE_MULTICAST_DELEGATE(FOnMLExperienceReady);

UCLASS()
class MYMODULARGAME_API AMLGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AMLGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    bool IsExperienceReady() const { return bExperienceReady; }
    const UMLExperienceDefinition* GetCurrentExperience() const { return ExperienceDefinition; }

    FOnMLExperienceReady& OnExperienceReady() { return OnExperienceReadyDelegate; }

    void SetCurrentExperience(const UMLExperienceDefinition* InExperienceDefinition);

protected:
    UFUNCTION()
    void OnRep_ExperienceDefinition();

    UFUNCTION()
    void OnRep_ExperienceReady();

private:
    void BroadcastExperienceReadyIfNeeded();

private:
    UPROPERTY(ReplicatedUsing = OnRep_ExperienceDefinition)
    TObjectPtr<const UMLExperienceDefinition> ExperienceDefinition;

    UPROPERTY(ReplicatedUsing = OnRep_ExperienceReady)
    bool bExperienceReady;

    FOnMLExperienceReady OnExperienceReadyDelegate;
};
