#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "MLPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

DECLARE_LOG_CATEGORY_EXTERN(LogMLAbility, Log, All);

UCLASS()
class MYMODULARGAME_API AMLPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AMLPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    UAbilitySystemComponent* GetMLAbilitySystemComponent() const { return AbilitySystemComponent; }
    const UAttributeSet* GetBaseAttributeSet() const { return BaseAttributeSet; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
    TObjectPtr<UAttributeSet> BaseAttributeSet;
};
