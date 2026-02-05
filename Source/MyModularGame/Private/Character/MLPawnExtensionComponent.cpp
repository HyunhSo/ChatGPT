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
    static FString GetObjectPathSafe(const UObject* Object)
    {
        return Object ? Object->GetPathName() : TEXT("None");
    }

    static const TCHAR* GetLocalRoleText(const APawn* Pawn)
    {
        if (!Pawn)
        {
            return TEXT("NoPawn");
        }

        return Pawn->IsLocallyControlled() ? TEXT("Local") : TEXT("Remote");
    }

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

                UE_LOG(LogMLInit, Log, TEXT("[ML.InitStateDump] World=%s NetMode=%d GS=%s ExperienceReady=%s Experience=%s ExperiencePath=%s"),
                    *GetNameSafe(World),
                    static_cast<int32>(World->GetNetMode()),
                    *GetNameSafe(MLGameState),
                    MLGameState && MLGameState->IsExperienceReady() ? TEXT("true") : TEXT("false"),
                    *GetNameSafe(Experience),
                    *GetObjectPathSafe(Experience));

                for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
                {
                    const APlayerController* PC = It->Get();
                    const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
                    const APlayerState* PS = PC ? PC->PlayerState : nullptr;
                    const UMLPawnExtensionComponent* PawnExtension = Pawn ? Pawn->FindComponentByClass<UMLPawnExtensionComponent>() : nullptr;
                    const UMLPawnData* PawnData = PawnExtension ? PawnExtension->GetPawnData() : nullptr;

                    UE_LOG(LogMLInit, Log, TEXT("[ML.InitStateDump] PC=%s Pawn=%s PS=%s Role=%s MLInitState=%s PawnData=%s PawnDataPath=%s Fallback=%s"),
                        *GetNameSafe(PC),
                        *GetNameSafe(Pawn),
                        *GetNameSafe(PS),
                        GetLocalRoleText(Pawn),
                        PawnExtension ? *PawnExtension->GetCurrentInitState().ToString() : TEXT("None"),
                        *GetNameSafe(PawnData),
                        *GetObjectPathSafe(PawnData),
                        PawnExtension ? PawnExtension->GetFallbackSourceText() : TEXT("None"));
                }
            }
        })
    );
}

UMLPawnExtensionComponent::UMLPawnExtensionComponent()
    : PawnDataFallbackSource(EMLPawnDataFallbackSource::None)
    , bGameplayReadyHandled(false)
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
    if (CurrentInitState == NewState)
    {
        return;
    }

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
        return !bGameplayReadyHandled;
    }

    return false;
}

void UMLPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, const FGameplayTag CurrentState, const FGameplayTag DesiredState)
{
    if (DesiredState == MyInitStateTags::TAG_InitState_DataInitialized)
    {
        ResolvePawnData();
    }

    UE_LOG(LogMLInit, Log, TEXT("[MLInit] %s: %s -> %s (Controller=%s, PlayerState=%s, ExperienceReady=%s, PawnData=%s, Fallback=%s)"),
        *GetNameSafe(GetOwner()),
        *CurrentState.ToString(),
        *DesiredState.ToString(),
        *GetNameSafe(GetPawn<APawn>() ? GetPawn<APawn>()->GetController() : nullptr),
        *GetNameSafe(GetPawn<APawn>() ? GetPawn<APawn>()->GetPlayerState() : nullptr),
        IsExperienceReady() ? TEXT("true") : TEXT("false"),
        *GetNameSafe(PawnData),
        GetFallbackSourceText());

    if (DesiredState == MyInitStateTags::TAG_InitState_GameplayReady)
    {
        if (bGameplayReadyHandled)
        {
            UE_LOG(LogMLInit, Verbose, TEXT("[MLInit] %s already handled GameplayReady; skipping re-entry."), *GetNameSafe(GetOwner()));
            return;
        }

        bGameplayReadyHandled = true;

        if (!PawnData)
        {
            UE_LOG(LogMLInit, Warning, TEXT("[MLInit] %s reached GameplayReady without PawnData. Hero setup is skipped."), *GetNameSafe(GetOwner()));
            return;
        }

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
    PawnDataFallbackSource = EMLPawnDataFallbackSource::None;
    bGameplayReadyHandled = false;

    Super::RegisterInitStateFeature();
}

void UMLPawnExtensionComponent::UnregisterInitStateFeature()
{
    Super::UnregisterInitStateFeature();

    CurrentInitState = FGameplayTag();
    PawnData = nullptr;
    PawnDataFallbackSource = EMLPawnDataFallbackSource::None;
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

bool UMLPawnExtensionComponent::ResolvePawnData()
{
    PawnData = nullptr;
    PawnDataFallbackSource = EMLPawnDataFallbackSource::Missing;

    const AMLGameState* MLGameState = GetWorld() ? GetWorld()->GetGameState<AMLGameState>() : nullptr;
    const UMLExperienceDefinition* Experience = MLGameState ? MLGameState->GetCurrentExperience() : nullptr;

    if (Experience && Experience->DefaultPawnData)
    {
        PawnData = Experience->DefaultPawnData;
        PawnDataFallbackSource = EMLPawnDataFallbackSource::Experience;

        UE_LOG(LogMLInit, Log, TEXT("[MLInit] %s resolved PawnData from Experience: %s"),
            *GetNameSafe(GetOwner()),
            *GetNameSafe(PawnData));
        return true;
    }

    const UMLPawnData* GameModeDefaultPawnData = MLGameState ? MLGameState->GetDefaultPawnData() : nullptr;
    if (GameModeDefaultPawnData)
    {
        PawnData = GameModeDefaultPawnData;
        PawnDataFallbackSource = EMLPawnDataFallbackSource::GameModeDefault;

        UE_LOG(LogMLInit, Log, TEXT("[MLInit] %s resolved PawnData via fallback(GameMode/GameState default): %s"),
            *GetNameSafe(GetOwner()),
            *GetNameSafe(PawnData));
        return true;
    }

    UE_LOG(LogMLInit, Warning, TEXT("[MLInit] %s has no Experience and no fallback PawnData. Continuing without PawnData."), *GetNameSafe(GetOwner()));
    return false;
}

const TCHAR* UMLPawnExtensionComponent::GetFallbackSourceText() const
{
    switch (PawnDataFallbackSource)
    {
    case EMLPawnDataFallbackSource::None:
        return TEXT("None");
    case EMLPawnDataFallbackSource::Experience:
        return TEXT("Experience");
    case EMLPawnDataFallbackSource::GameModeDefault:
        return TEXT("GameModeDefault");
    case EMLPawnDataFallbackSource::Missing:
        return TEXT("Missing");
    default:
        return TEXT("Unknown");
    }
}

void UMLPawnExtensionComponent::DumpInitState() const
{
    const APawn* Pawn = GetPawn<APawn>();
    const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    const APlayerState* PS = Pawn ? Pawn->GetPlayerState() : nullptr;
    const AMLGameState* MLGameState = GetWorld() ? GetWorld()->GetGameState<AMLGameState>() : nullptr;
    const UMLExperienceDefinition* Experience = MLGameState ? MLGameState->GetCurrentExperience() : nullptr;

    UE_LOG(LogMLInit, Log, TEXT("[ML.InitStateDump] Pawn=%s PC=%s PS=%s Role=%s ExperienceReady=%s Experience=%s ExperiencePath=%s InitState=%s PawnData=%s PawnDataPath=%s Fallback=%s"),
        *GetNameSafe(Pawn),
        *GetNameSafe(PC),
        *GetNameSafe(PS),
        GetLocalRoleText(Pawn),
        MLGameState && MLGameState->IsExperienceReady() ? TEXT("true") : TEXT("false"),
        *GetNameSafe(Experience),
        *GetObjectPathSafe(Experience),
        *CurrentInitState.ToString(),
        *GetNameSafe(PawnData),
        *GetObjectPathSafe(PawnData),
        GetFallbackSourceText());
}
