#include "Character/MLCharacter.h"

#include "Character/MLCombatComponent.h"
#include "Character/MLHeroComponent.h"
#include "Character/MLPawnExtensionComponent.h"

AMLCharacter::AMLCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PawnExtensionComponent = CreateDefaultSubobject<UMLPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
    HeroComponent = CreateDefaultSubobject<UMLHeroComponent>(TEXT("HeroComponent"));
    CombatComponent = CreateDefaultSubobject<UMLCombatComponent>(TEXT("CombatComponent"));
}

void AMLCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogMLInit, Verbose, TEXT("MLCharacter BeginPlay: %s"), *GetNameSafe(this));
}
