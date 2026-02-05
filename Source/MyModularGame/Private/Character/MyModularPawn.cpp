#include "Character/MyModularPawn.h"

#include "Components/MyPawnInitComponent.h"

AMyModularPawn::AMyModularPawn(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PawnInitComponent = CreateDefaultSubobject<UMyPawnInitComponent>(TEXT("PawnInitComponent"));
}
