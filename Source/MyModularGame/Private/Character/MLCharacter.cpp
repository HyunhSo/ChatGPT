#include "Character/MLCharacter.h"

#include "Character/MLHeroComponent.h"
#include "Character/MLPawnExtensionComponent.h"
#include "Player/MLPlayerState.h"

AMLCharacter::AMLCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PawnExtensionComponent = CreateDefaultSubobject<UMLPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
    HeroComponent = CreateDefaultSubobject<UMLHeroComponent>(TEXT("HeroComponent"));
}


UAbilitySystemComponent* AMLCharacter::GetMLAbilitySystemComponent() const
{
    if (const AMLPlayerState* MLPlayerState = GetPlayerState<AMLPlayerState>())
    {
        return MLPlayerState->GetMLAbilitySystemComponent();
    }

    return nullptr;
}
void AMLCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogMLInit, Verbose, TEXT("MLCharacter BeginPlay: %s"), *GetNameSafe(this));
}
