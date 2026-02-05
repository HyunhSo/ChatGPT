#include "Components/MyPawnInitComponent.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "System/MyInitStateTags.h"

const FName UMyPawnInitComponent::NAME_ActorFeatureName(TEXT("PawnInit"));

UMyPawnInitComponent::UMyPawnInitComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
}

FName UMyPawnInitComponent::GetFeatureName() const
{
    return NAME_ActorFeatureName;
}

void UMyPawnInitComponent::BeginPlay()
{
    Super::BeginPlay();

    if (APawn* Pawn = GetPawn<APawn>())
    {
        Pawn->ReceiveControllerChangedDelegate.AddUObject(this, &ThisClass::HandleControllerChanged);
    }

    RegisterInitStateFeature();
    CheckDefaultInitialization();
}

void UMyPawnInitComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterInitStateFeature();

    if (APawn* Pawn = GetPawn<APawn>())
    {
        Pawn->ReceiveControllerChangedDelegate.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}


void UMyPawnInitComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentInitState != MyInitStateTags::TAG_InitState_GameplayReady)
    {
        CheckDefaultInitialization();
    }
}

void UMyPawnInitComponent::CheckDefaultInitialization()
{
    static const TArray<FGameplayTag> StateChain = {
        MyInitStateTags::TAG_InitState_Spawned,
        MyInitStateTags::TAG_InitState_DataAvailable,
        MyInitStateTags::TAG_InitState_DataInitialized,
        MyInitStateTags::TAG_InitState_GameplayReady
    };

    ContinueInitStateChain(StateChain);
}

FGameplayTag UMyPawnInitComponent::GetInitState() const
{
    return CurrentInitState;
}

bool UMyPawnInitComponent::HasReachedInitState(FGameplayTag DesiredState) const
{
    return CurrentInitState == DesiredState;
}

void UMyPawnInitComponent::SetInitState(FGameplayTag NewState)
{
    CurrentInitState = NewState;
}

bool UMyPawnInitComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentStateTag, FGameplayTag DesiredState) const
{
    if (!CurrentStateTag.IsValid() && DesiredState == MyInitStateTags::TAG_InitState_Spawned)
    {
        return true;
    }

    if (CurrentStateTag == MyInitStateTags::TAG_InitState_Spawned && DesiredState == MyInitStateTags::TAG_InitState_DataAvailable)
    {
        return HasValidController() && HasValidPlayerState();
    }

    if (CurrentStateTag == MyInitStateTags::TAG_InitState_DataAvailable && DesiredState == MyInitStateTags::TAG_InitState_DataInitialized)
    {
        return true;
    }

    if (CurrentStateTag == MyInitStateTags::TAG_InitState_DataInitialized && DesiredState == MyInitStateTags::TAG_InitState_GameplayReady)
    {
        return true;
    }

    return false;
}

void UMyPawnInitComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentStateTag, FGameplayTag DesiredState)
{
    UE_LOG(LogTemp, Log, TEXT("[PawnInit] %s : %s -> %s"),
        *GetNameSafe(GetOwner()),
        *CurrentStateTag.ToString(),
        *DesiredState.ToString());

    if (DesiredState == MyInitStateTags::TAG_InitState_DataAvailable)
    {
        UE_LOG(LogTemp, Log, TEXT("[PawnInit] Controller=%s PlayerState=%s"),
            *GetNameSafe(GetPawn<APawn>() ? GetPawn<APawn>()->GetController() : nullptr),
            *GetNameSafe(GetPawn<APawn>() ? GetPawn<APawn>()->GetPlayerState() : nullptr));
    }
}

void UMyPawnInitComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
    CheckDefaultInitialization();
}

void UMyPawnInitComponent::RegisterInitStateFeature()
{
    CurrentInitState = FGameplayTag();

    Super::RegisterInitStateFeature();
}

void UMyPawnInitComponent::UnregisterInitStateFeature()
{
    Super::UnregisterInitStateFeature();

    CurrentInitState = FGameplayTag();
}

void UMyPawnInitComponent::HandleControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
    UE_LOG(LogTemp, Log, TEXT("[PawnInit] Controller changed for %s: %s -> %s"),
        *GetNameSafe(Pawn),
        *GetNameSafe(OldController),
        *GetNameSafe(NewController));

    CheckDefaultInitialization();
}

bool UMyPawnInitComponent::HasValidController() const
{
    const APawn* Pawn = GetPawn<APawn>();
    return Pawn && Pawn->GetController() != nullptr;
}

bool UMyPawnInitComponent::HasValidPlayerState() const
{
    const APawn* Pawn = GetPawn<APawn>();
    return Pawn && Pawn->GetPlayerState() != nullptr;
}
