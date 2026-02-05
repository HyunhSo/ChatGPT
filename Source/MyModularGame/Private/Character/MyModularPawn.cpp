#include "Character/MyModularPawn.h"

#include "Components/MyHeroComponent.h"
#include "Components/MyPawnExtensionComponent.h"

AMyModularPawn::AMyModularPawn(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PawnExtensionComponent = CreateDefaultSubobject<UMyPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
    HeroComponent = CreateDefaultSubobject<UMyHeroComponent>(TEXT("HeroComponent"));
}
