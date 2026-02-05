#include "Character/MLPawnExtensionComponent.h"

#include "Character/MLHeroComponent.h"
#include "Character/MLPawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/MLGameState.h"
#include "System/MLExperienceDefinition.h"
#include "System/MyInitStateTags.h"

DEFINE_LOG_CATEGORY(LogMLInit);

const FName UMLPawnExtensionComponent::NAME_ActorFeatureName(TEXT("MLPawnExtension"));

namespace
{
    static FAutoConsoleCommand CmdMLInitStateDump(
        TEXT("ML.InitStateDump"),
        TEXT("Dump Pawn/PC/PS/GameState(ExperienceReady) and MLPawnExtension init states."),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            if (!GEngine)
            {
                return;
            }

            for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
            {
                UWorld* World = WorldContext.World();
                if (!World || !World->IsGameWorld())
                {
                    continue;
                }

                const AMLGameState* MLGameState = World->GetGameState<AMLGameState>();
                UE_LOG(LogMLInit, Log, TEXT("[ML.InitStateDump] World=%s NetMode=%d GS=%s ExperienceReady=%s Experience=%s"),
                    *GetNameSafe(World),
                    static_cast<int32>(World->GetNetMode()),
                    *GetNameSafe(MLGameState),
                    MLGameState && MLGameState->IsExperienceReady() ? TEXT("true") : TEXT("false"),
                    *GetNameSafe(MLGameState ? MLGameState->GetCurrentExperience() : nullptr));

                for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
                {
                    const APlayerController* PC = It->Get();
                    const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
                    const APlayerState* PS = PC ? PC->PlayerState : nullptr;
                    const UMLPawnExtensionComponent* PawnExtension = Pawn ? Pawn->FindComponentByClass<UMLPawnExtensionComponent>() : nullptr;

                    UE_LOG(LogMLInit, Log, TEXT("[ML.InitStateDump] PC=%s Pawn=%s PS=%s MLInitState=%s PawnData=%s"),
                        *GetNameSafe(PC),
                        *GetNameSafe(Pawn),
                        *GetNameSafe(PS),
                        PawnExtension ? *PawnExtension->GetCurrentInitState().ToString() : TEXT("None"),
                        *GetNameSafe(PawnExtension ? PawnExtension->GetPawnData() : nullptr));
                }
            }
        })
    );
}

UMLPawnExtensionComponent::UMLPawnExtensionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMLPawnExtensionComponent::BeginPlay()
{
    Super::BeginPlay();

    RegisterInitStateFeature();
    CheckDefaultInitialization();
}

FName UMLPawnExtensionComponent::GetFeatureName() const
{
    return NAME_ActorFeatureName;
}

void UMLPawnExtensionComponent::CheckDefaultInitialization()
{
    static const TArray<FGameplayTag> InitChain = {
        MyInitStateTags::TAG_InitState_Spawned,
        MyInitStateTags::TAG_InitState_DataAvailable,
        MyInitStateTags::TAG_InitState_DataInitialized,
        MyInitStateTags::TAG_InitState_GameplayReady
    };

    ContinueInitStateChain(InitChain);
}

FGameplayTag UMLPawnExtensionComponent::GetInitState() const
{
    return CurrentInitState;
}

bool UMLPawnExtensionComponent::HasReachedInitState(const FGameplayTag DesiredState) const
{
    return CurrentInitState == DesiredState;
}

void UMLPawnExtensionComponent::SetInitState(const FGameplayTag NewState)
{
    CurrentInitState = NewState;
}

bool UMLPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, const FGameplayTag CurrentState, const FGameplayTag DesiredState) const
{
    if (!CurrentState.IsValid() && DesiredState == MyInitStateTags::TAG_InitState_Spawned)
    {
        return true;
    }

    if (CurrentState == MyInitStateTags::TAG_InitState_Spawned && DesiredState == MyInitStateTags::TAG_InitState_DataAvailable)
    {
        return HasController() && HasPlayerState() && (GetWorld() != nullptr) && (GetWorld()->GetGameState<AMLGameState>() != nullptr);
    }

    if (CurrentState == MyInitStateTags::TAG_InitState_DataAvailable && DesiredState == MyInitStateTags::TAG_InitState_DataInitialized)
    {
        return IsExperienceReady();
    }

    if (CurrentState == MyInitStateTags::TAG_InitState_DataInitialized && DesiredState == MyInitStateTags::TAG_InitState_GameplayReady)
    {
        return PawnData != nullptr;
    }

    return false;
}

void UMLPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, const FGameplayTag CurrentState, const FGameplayTag DesiredState)
{
    if (DesiredState == MyInitStateTags::TAG_InitState_DataInitialized)
    {
        const AMLGameState* MLGameState = GetWorld() ? GetWorld()->GetGameState<AMLGameState>() : nullptr;
        const UMLExperienceDefinition* Experience = MLGameState ? MLGameState->GetCurrentExperience() : nullptr;
        PawnData = Experience ? Experience->DefaultPawnData : nullptr;
    }

    UE_LOG(LogMLInit, Log, TEXT("[MLInit] %s: %s -> %s (Controller=%s, PlayerState=%s, ExperienceReady=%s, PawnData=%s)"),
        *GetNameSafe(GetOwner()),
        *CurrentState.ToString(),
        *DesiredState.ToString(),
        *GetNameSafe(GetPawn<APawn>() ? GetPawn<APawn>()->GetController() : nullptr),
        *GetNameSafe(GetPawn<APawn>() ? GetPawn<APawn>()->GetPlayerState() : nullptr),
        IsExperienceReady() ? TEXT("true") : TEXT("false"),
        *GetNameSafe(PawnData));

    if (DesiredState == MyInitStateTags::TAG_InitState_GameplayReady)
    {
        if (APawn* Pawn = GetPawn<APawn>())
        {
            if (UMLHeroComponent* HeroComponent = Pawn->FindComponentByClass<UMLHeroComponent>())
            {
                HeroComponent->HandleGameplayReady();
            }
        }
    }
}

void UMLPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
    CheckDefaultInitialization();
}

void UMLPawnExtensionComponent::RegisterInitStateFeature()
{
    CurrentInitState = FGameplayTag();
    PawnData = nullptr;

    Super::RegisterInitStateFeature();
}

void UMLPawnExtensionComponent::UnregisterInitStateFeature()
{
    Super::UnregisterInitStateFeature();

    CurrentInitState = FGameplayTag();
    PawnData = nullptr;
}

void UMLPawnExtensionComponent::HandleControllerChanged()
{
    UE_LOG(LogMLInit, Verbose, TEXT("[MLInit] HandleControllerChanged for %s"), *GetNameSafe(GetOwner()));
    CheckDefaultInitialization();
}

void UMLPawnExtensionComponent::HandlePlayerStateAvailable()
{
    UE_LOG(LogMLInit, Verbose, TEXT("[MLInit] HandlePlayerStateAvailable for %s"), *GetNameSafe(GetOwner()));
    CheckDefaultInitialization();
}

bool UMLPawnExtensionComponent::HasController() const
{
    const APawn* Pawn = GetPawn<APawn>();
    return Pawn && Pawn->GetController() != nullptr;
}

bool UMLPawnExtensionComponent::HasPlayerState() const
{
    const APawn* Pawn = GetPawn<APawn>();
    return Pawn && Pawn->GetPlayerState() != nullptr;
}

bool UMLPawnExtensionComponent::IsExperienceReady() const
{
    const AMLGameState* MLGameState = GetWorld() ? GetWorld()->GetGameState<AMLGameState>() : nullptr;
    return MLGameState && MLGameState->IsExperienceReady();
}

void UMLPawnExtensionComponent::DumpInitState() const
{
    const APawn* Pawn = GetPawn<APawn>();
    const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    const APlayerState* PS = Pawn ? Pawn->GetPlayerState() : nullptr;
    const AMLGameState* MLGameState = GetWorld() ? GetWorld()->GetGameState<AMLGameState>() : nullptr;

    UE_LOG(LogMLInit, Log, TEXT("[ML.InitStateDump] Pawn=%s PC=%s PS=%s ExperienceReady=%s InitState=%s PawnData=%s"),
        *GetNameSafe(Pawn),
        *GetNameSafe(PC),
        *GetNameSafe(PS),
        MLGameState && MLGameState->IsExperienceReady() ? TEXT("true") : TEXT("false"),
        *CurrentInitState.ToString(),
        *GetNameSafe(PawnData));
}
