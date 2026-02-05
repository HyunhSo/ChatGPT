#include "Character/MLPawnExtensionComponent.h"

#include "Character/MLHeroComponent.h"
#include "Character/MLPawnData.h"
#include "Player/MLPlayerState.h"
#include "AbilitySystemComponent.h"
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
                const UMLExperienceDefinition* Experience = MLGameState ? MLGameState->GetCurrentExperience() : nullptr;
                UE_LOG(LogMLInit, Log, TEXT("[ML.InitStateDump] World=%s NetMode=%d GS=%s ExperienceReady=%s ExperiencePresent=%s Experience=%s ExperiencePath=%s"),
                    *GetNameSafe(World),
                    static_cast<int32>(World->GetNetMode()),
                    *GetNameSafe(MLGameState),
                    MLGameState && MLGameState->IsExperienceReady() ? TEXT("true") : TEXT("false"),
                    Experience ? TEXT("true") : TEXT("false"),
                    *GetNameSafe(Experience),
                    *GetPathNameSafe(Experience));

                for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
                {
                    const APlayerController* PC = It->Get();
                    const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
                    const APlayerState* PS = PC ? PC->PlayerState : nullptr;
                    const AMLPlayerState* MLPlayerState = Cast<AMLPlayerState>(PS);
                    const UAbilitySystemComponent* ASC = MLPlayerState ? MLPlayerState->GetMLAbilitySystemComponent() : nullptr;
                    const UMLPawnExtensionComponent* PawnExtension = Pawn ? Pawn->FindComponentByClass<UMLPawnExtensionComponent>() : nullptr;
                    const UMLHeroComponent* HeroComponent = Pawn ? Pawn->FindComponentByClass<UMLHeroComponent>() : nullptr;
                    const bool bIsLocal = PC ? PC->IsLocalController() : false;
                    const UMLPawnData* PawnData = PawnExtension ? PawnExtension->GetPawnData() : nullptr;
                    const FName PawnDataSource = PawnExtension ? PawnExtension->GetPawnDataResolutionSource() : NAME_None;
                    const bool bUsedFallback = PawnExtension ? PawnExtension->IsUsingPawnDataFallback() : false;

                    UE_LOG(LogMLInit, Log, TEXT("[ML.InitStateDump] PC=%s Pawn=%s PS=%s PSClass=%s IsMLPlayerState=%s Local=%s MLInitState=%s PawnDataPresent=%s PawnData=%s PawnDataPath=%s PawnDataSource=%s UsedFallback=%s ASC=%s ASCRepMode=%s HeroSetupDone(Input=%s,Camera=%s)"),
                        *GetNameSafe(PC),
                        *GetNameSafe(Pawn),
                        *GetNameSafe(PS),
                        PS ? *PS->GetClass()->GetName() : TEXT("None"),
                        MLPlayerState ? TEXT("true") : TEXT("false"),
                        bIsLocal ? TEXT("Local") : TEXT("Remote"),
                        PawnExtension ? *PawnExtension->GetCurrentInitState().ToString() : TEXT("None"),
                        PawnData ? TEXT("true") : TEXT("false"),
                        *GetNameSafe(PawnData),
                        *GetPathNameSafe(PawnData),
                        *PawnDataSource.ToString(),
                        bUsedFallback ? TEXT("true") : TEXT("false"),
                        ASC ? TEXT("true") : TEXT("false"),
                        ASC ? *StaticEnum<EGameplayEffectReplicationMode>()->GetNameStringByValue(static_cast<int64>(ASC->GetReplicationMode())) : TEXT("None"),
                        HeroComponent && HeroComponent->IsInputSetupDone() ? TEXT("true") : TEXT("false"),
                        HeroComponent && HeroComponent->IsCameraSetupDone() ? TEXT("true") : TEXT("false"));
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
    TryBindToExperienceReady();
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
        return true;
    }

    if (CurrentState == MyInitStateTags::TAG_InitState_DataInitialized && DesiredState == MyInitStateTags::TAG_InitState_GameplayReady)
    {
        return true;
    }

    return false;
}

void UMLPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, const FGameplayTag CurrentState, const FGameplayTag DesiredState)
{
    if (DesiredState == MyInitStateTags::TAG_InitState_DataInitialized)
    {
        ResolvePawnDataWithFallback();
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
        if (bGameplayReadyHandled)
        {
            UE_LOG(LogMLInit, Verbose, TEXT("[MLInit] %s: GameplayReady already handled; skipping re-entry."), *GetNameSafe(GetOwner()));
            return;
        }

        bGameplayReadyHandled = true;

        if (APawn* Pawn = GetPawn<APawn>())
        {
            if (UMLHeroComponent* HeroComponent = Pawn->FindComponentByClass<UMLHeroComponent>())
            {
                HeroComponent->HandleGameplayReady(PawnData);
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
    PawnDataResolutionSource = NAME_None;
    bUsedPawnDataFallback = false;
    bGameplayReadyHandled = false;

    Super::RegisterInitStateFeature();
}

void UMLPawnExtensionComponent::UnregisterInitStateFeature()
{
    if (AMLGameState* MLGameState = GetWorld() ? GetWorld()->GetGameState<AMLGameState>() : nullptr)
    {
        if (ExperienceReadyHandle.IsValid())
        {
            MLGameState->OnExperienceReady().Remove(ExperienceReadyHandle);
            ExperienceReadyHandle.Reset();
        }
    }

    Super::UnregisterInitStateFeature();

    CurrentInitState = FGameplayTag();
    PawnData = nullptr;
    PawnDataResolutionSource = NAME_None;
    bUsedPawnDataFallback = false;
    bGameplayReadyHandled = false;
}

void UMLPawnExtensionComponent::HandleControllerChanged()
{
    UE_LOG(LogMLInit, Verbose, TEXT("[MLInit] HandleControllerChanged for %s"), *GetNameSafe(GetOwner()));
    CheckDefaultInitialization();
}

void UMLPawnExtensionComponent::HandlePlayerStateAvailable()
{
    UE_LOG(LogMLInit, Verbose, TEXT("[MLInit] HandlePlayerStateAvailable for %s"), *GetNameSafe(GetOwner()));
    LogAbilitySystemState(TEXT("HandlePlayerStateAvailable"));
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


bool UMLPawnExtensionComponent::ResolvePawnDataWithFallback()
{
    const AMLGameState* MLGameState = GetWorld() ? GetWorld()->GetGameState<AMLGameState>() : nullptr;
    const UMLExperienceDefinition* Experience = MLGameState ? MLGameState->GetCurrentExperience() : nullptr;

    bUsedPawnDataFallback = false;
    PawnDataResolutionSource = NAME_None;
    PawnData = nullptr;

    if (Experience && Experience->DefaultPawnData)
    {
        PawnData = Experience->DefaultPawnData;
        PawnDataResolutionSource = TEXT("Experience.DefaultPawnData");
    }
    else if (MLGameState && MLGameState->GetDefaultPawnData())
    {
        PawnData = MLGameState->GetDefaultPawnData();
        PawnDataResolutionSource = MLGameState->GetPawnDataSource().IsNone() ? FName(TEXT("GameState.DefaultPawnData")) : MLGameState->GetPawnDataSource();
        bUsedPawnDataFallback = true;
    }

    if (!PawnData)
    {
        PawnDataResolutionSource = TEXT("None");
    }

    UE_LOG(LogMLInit, Log, TEXT("[MLInit] %s: PawnData resolve -> ExperiencePresent=%s Experience=%s PawnData=%s PawnDataPath=%s Source=%s Fallback=%s"),
        *GetNameSafe(GetOwner()),
        Experience ? TEXT("true") : TEXT("false"),
        *GetNameSafe(Experience),
        *GetNameSafe(PawnData),
        *GetPathNameSafe(PawnData),
        *PawnDataResolutionSource.ToString(),
        bUsedPawnDataFallback ? TEXT("true") : TEXT("false"));

    return PawnData != nullptr;
}

void UMLPawnExtensionComponent::TryBindToExperienceReady()
{
    AMLGameState* MLGameState = GetWorld() ? GetWorld()->GetGameState<AMLGameState>() : nullptr;
    if (!MLGameState)
    {
        UE_LOG(LogMLInit, Verbose, TEXT("[MLInit] %s: GameState not available while trying to bind ExperienceReady."), *GetNameSafe(GetOwner()));
        return;
    }

    if (!ExperienceReadyHandle.IsValid())
    {
        ExperienceReadyHandle = MLGameState->OnExperienceReady().AddUObject(this, &ThisClass::HandleExperienceReady);
    }

    if (MLGameState->IsExperienceReady())
    {
        HandleExperienceReady();
    }
}

void UMLPawnExtensionComponent::HandleExperienceReady()
{
    UE_LOG(LogMLInit, Verbose, TEXT("[MLInit] ExperienceReady received for %s"), *GetNameSafe(GetOwner()));
    CheckDefaultInitialization();
}

void UMLPawnExtensionComponent::LogAbilitySystemState(const TCHAR* Context) const
{
    const APawn* Pawn = GetPawn<APawn>();
    const APlayerState* PS = Pawn ? Pawn->GetPlayerState() : nullptr;
    const AMLPlayerState* MLPlayerState = Cast<AMLPlayerState>(PS);
    const UAbilitySystemComponent* ASC = MLPlayerState ? MLPlayerState->GetMLAbilitySystemComponent() : nullptr;

    UE_LOG(LogMLAbility, Log, TEXT("[MLAbility] %s Owner=%s PS=%s PSClass=%s IsMLPlayerState=%s ASC=%s ASCRepMode=%s"),
        Context ? Context : TEXT("Unknown"),
        *GetNameSafe(GetOwner()),
        *GetNameSafe(PS),
        PS ? *PS->GetClass()->GetName() : TEXT("None"),
        MLPlayerState ? TEXT("true") : TEXT("false"),
        ASC ? TEXT("true") : TEXT("false"),
        ASC ? *StaticEnum<EGameplayEffectReplicationMode>()->GetNameStringByValue(static_cast<int64>(ASC->GetReplicationMode())) : TEXT("None"));
}

void UMLPawnExtensionComponent::DumpInitState() const
{
    const APawn* Pawn = GetPawn<APawn>();
    const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    const APlayerState* PS = Pawn ? Pawn->GetPlayerState() : nullptr;
    const AMLPlayerState* MLPlayerState = Cast<AMLPlayerState>(PS);
    const UAbilitySystemComponent* ASC = MLPlayerState ? MLPlayerState->GetMLAbilitySystemComponent() : nullptr;
    const AMLGameState* MLGameState = GetWorld() ? GetWorld()->GetGameState<AMLGameState>() : nullptr;
    const UMLExperienceDefinition* Experience = MLGameState ? MLGameState->GetCurrentExperience() : nullptr;
    const UMLHeroComponent* HeroComponent = Pawn ? Pawn->FindComponentByClass<UMLHeroComponent>() : nullptr;
    const bool bIsLocal = PC ? PC->IsLocalController() : false;

    UE_LOG(LogMLInit, Log, TEXT("[ML.InitStateDump] Pawn=%s PC=%s PS=%s PSClass=%s IsMLPlayerState=%s Local=%s ExperienceReady=%s ExperiencePresent=%s Experience=%s ExperiencePath=%s InitState=%s PawnDataPresent=%s PawnData=%s PawnDataPath=%s PawnDataSource=%s UsedFallback=%s ASC=%s ASCRepMode=%s HeroSetupDone(Input=%s,Camera=%s)"),
        *GetNameSafe(Pawn),
        *GetNameSafe(PC),
        *GetNameSafe(PS),
        PS ? *PS->GetClass()->GetName() : TEXT("None"),
        MLPlayerState ? TEXT("true") : TEXT("false"),
        bIsLocal ? TEXT("Local") : TEXT("Remote"),
        MLGameState && MLGameState->IsExperienceReady() ? TEXT("true") : TEXT("false"),
        Experience ? TEXT("true") : TEXT("false"),
        *GetNameSafe(Experience),
        *GetPathNameSafe(Experience),
        *CurrentInitState.ToString(),
        PawnData ? TEXT("true") : TEXT("false"),
        *GetNameSafe(PawnData),
        *GetPathNameSafe(PawnData),
        *PawnDataResolutionSource.ToString(),
        bUsedPawnDataFallback ? TEXT("true") : TEXT("false"),
        ASC ? TEXT("true") : TEXT("false"),
        ASC ? *StaticEnum<EGameplayEffectReplicationMode>()->GetNameStringByValue(static_cast<int64>(ASC->GetReplicationMode())) : TEXT("None"),
        HeroComponent && HeroComponent->IsInputSetupDone() ? TEXT("true") : TEXT("false"),
        HeroComponent && HeroComponent->IsCameraSetupDone() ? TEXT("true") : TEXT("false"));
}
