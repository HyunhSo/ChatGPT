#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MLGameState.generated.h"

class UMLExperienceDefinition;
class UMLPawnData;

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
    const UMLPawnData* GetDefaultPawnData() const { return DefaultPawnData; }
    FName GetPawnDataSource() const { return PawnDataSource; }

    FOnMLExperienceReady& OnExperienceReady() { return OnExperienceReadyDelegate; }

    void SetCurrentExperience(const UMLExperienceDefinition* InExperienceDefinition);
    void SetDefaultPawnData(const UMLPawnData* InPawnData, FName InPawnDataSource);

protected:
    UFUNCTION()
    void OnRep_ExperienceDefinition();

    UFUNCTION()
    void OnRep_ExperienceReady();

    UFUNCTION()
    void OnRep_DefaultPawnData();

    UFUNCTION()
    void OnRep_PawnDataSource();

private:
    void BroadcastExperienceReadyIfNeeded();

private:
    UPROPERTY(ReplicatedUsing = OnRep_ExperienceDefinition)
    TObjectPtr<const UMLExperienceDefinition> ExperienceDefinition;

    UPROPERTY(ReplicatedUsing = OnRep_ExperienceReady)
    bool bExperienceReady;

    UPROPERTY(ReplicatedUsing = OnRep_DefaultPawnData)
    TObjectPtr<const UMLPawnData> DefaultPawnData;

    UPROPERTY(ReplicatedUsing = OnRep_PawnDataSource)
    FName PawnDataSource;

    FOnMLExperienceReady OnExperienceReadyDelegate;
};
