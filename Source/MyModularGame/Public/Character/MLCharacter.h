#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ModularPawn.h"
#include "MLCharacter.generated.h"

class UMLPawnExtensionComponent;
class UMLHeroComponent;
class UAbilitySystemComponent;

UCLASS()
class MYMODULARGAME_API AMLCharacter : public AModularPawn
{
    GENERATED_BODY()

public:
    AMLCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintCallable, Category = "Abilities")
    UAbilitySystemComponent* GetMLAbilitySystemComponent() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Init", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMLPawnExtensionComponent> PawnExtensionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMLHeroComponent> HeroComponent;
};
