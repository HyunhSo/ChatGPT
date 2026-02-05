#include "Player/MLPlayerState.h"

#include "AbilitySystemComponent.h"
#include "Player/MLAttributeSet_Base.h"

DEFINE_LOG_CATEGORY(LogMLAbility);

AMLPlayerState::AMLPlayerState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    check(AbilitySystemComponent);

    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    BaseAttributeSet = CreateDefaultSubobject<UMLAttributeSet_Base>(TEXT("BaseAttributeSet"));

    NetUpdateFrequency = 100.0f;

    UE_LOG(LogMLAbility, Verbose, TEXT("MLPlayerState created ASC=%s ReplicationMode=%d"),
        *GetNameSafe(AbilitySystemComponent),
        static_cast<int32>(AbilitySystemComponent->GetReplicationMode()));
}

UAbilitySystemComponent* AMLPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}
