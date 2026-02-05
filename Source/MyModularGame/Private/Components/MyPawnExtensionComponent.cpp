#include "Components/MyPawnExtensionComponent.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Data/MyGameData.h"
#include "Data/MyPawnData.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Game/MyModularGameState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "System/MyInitStateTags.h"

const FName UMyPawnExtensionComponent::NAME_ActorFeatureName(TEXT("PawnExtension"));

namespace MyModularGame
{
    static FAutoConsoleCommand CmdInitStateDump(
        TEXT("My.InitStateDump"),
        TEXT("Dump current Pawn/PC/PS/GameState status for local players."),
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

                UE_LOG(LogTemp, Log, TEXT("[InitStateDump] World=%s NetMode=%d"), *GetNameSafe(World), static_cast<int32>(World->GetNetMode()));

                for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
                {
                    const APlayerController* PC = It->Get();
                    const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
                    const APlayerState* PS = PC ? PC->PlayerState : nullptr;
                    const AGameStateBase* GS = World->GetGameState();

                    UE_LOG(LogTemp, Log, TEXT("[InitStateDump] PC=%s Pawn=%s PS=%s GS=%s"),
                        *GetNameSafe(PC), *GetNameSafe(Pawn), *GetNameSafe(PS), *GetNameSafe(GS));
                }
            }
        })
    );
}

UMyPawnExtensionComponent::UMyPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
}

FName UMyPawnExtensionComponent::GetFeatureName() const
{
    return NAME_ActorFeatureName;
}

void UMyPawnExtensionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (APawn* Pawn = GetPawn<APawn>())
    {
        Pawn->ReceiveControllerChangedDelegate.AddUObject(this, &ThisClass::HandleControllerChanged);
    }

    RegisterInitStateFeature();
    CheckDefaultInitialization();
}

void UMyPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterInitStateFeature();

    if (APawn* Pawn = GetPawn<APawn>())
    {
        Pawn->ReceiveControllerChangedDelegate.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

void UMyPawnExtensionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentInitState != MyInitStateTags::TAG_InitState_GameplayReady)
    {
        CheckDefaultInitialization();
    }
}

void UMyPawnExtensionComponent::CheckDefaultInitialization()
{
    static const TArray<FGameplayTag> StateChain = {
        MyInitStateTags::TAG_InitState_Spawned,
        MyInitStateTags::TAG_InitState_DataAvailable,
        MyInitStateTags::TAG_InitState_DataInitialized,
        MyInitStateTags::TAG_InitState_GameplayReady
    };

    ContinueInitStateChain(StateChain);
}

FGameplayTag UMyPawnExtensionComponent::GetInitState() const
{
    return CurrentInitState;
}

bool UMyPawnExtensionComponent::HasReachedInitState(FGameplayTag DesiredState) const
{
    return CurrentInitState == DesiredState;
}

void UMyPawnExtensionComponent::SetInitState(FGameplayTag NewState)
{
    CurrentInitState = NewState;
}

bool UMyPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentStateTag, FGameplayTag DesiredState) const
{
    if (!CurrentStateTag.IsValid() && DesiredState == MyInitStateTags::TAG_InitState_Spawned)
    {
        return true;
    }

    if (CurrentStateTag == MyInitStateTags::TAG_InitState_Spawned && DesiredState == MyInitStateTags::TAG_InitState_DataAvailable)
    {
        return HasValidController() && HasValidPlayerState() && HasValidGameData();
    }

    if (CurrentStateTag == MyInitStateTags::TAG_InitState_DataAvailable && DesiredState == MyInitStateTags::TAG_InitState_DataInitialized)
    {
        return PawnData != nullptr;
    }

    if (CurrentStateTag == MyInitStateTags::TAG_InitState_DataInitialized && DesiredState == MyInitStateTags::TAG_InitState_GameplayReady)
    {
        return true;
    }

    return false;
}

void UMyPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentStateTag, FGameplayTag DesiredState)
{
    UE_LOG(LogTemp, Log, TEXT("[PawnExtension] %s : %s -> %s"),
        *GetNameSafe(GetOwner()),
        *CurrentStateTag.ToString(),
        *DesiredState.ToString());

    if (DesiredState == MyInitStateTags::TAG_InitState_DataAvailable)
    {
        ResolvePawnDataFromGameData();
        UE_LOG(LogTemp, Log, TEXT("[PawnExtension] Controller=%s PlayerState=%s PawnData=%s"),
            *GetNameSafe(GetPawn<APawn>() ? GetPawn<APawn>()->GetController() : nullptr),
            *GetNameSafe(GetPawn<APawn>() ? GetPawn<APawn>()->GetPlayerState() : nullptr),
            *GetNameSafe(PawnData));
    }
}

void UMyPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
    CheckDefaultInitialization();
}

void UMyPawnExtensionComponent::RegisterInitStateFeature()
{
    CurrentInitState = FGameplayTag();

    Super::RegisterInitStateFeature();
}

void UMyPawnExtensionComponent::UnregisterInitStateFeature()
{
    Super::UnregisterInitStateFeature();

    CurrentInitState = FGameplayTag();
    PawnData = nullptr;
}

const UMyPawnData* UMyPawnExtensionComponent::GetPawnData() const
{
    return PawnData;
}

void UMyPawnExtensionComponent::HandleControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
    UE_LOG(LogTemp, Log, TEXT("[PawnExtension] Controller changed for %s: %s -> %s"),
        *GetNameSafe(Pawn),
        *GetNameSafe(OldController),
        *GetNameSafe(NewController));

    CheckDefaultInitialization();
}

bool UMyPawnExtensionComponent::HasValidController() const
{
    const APawn* Pawn = GetPawn<APawn>();
    return Pawn && Pawn->GetController() != nullptr;
}

bool UMyPawnExtensionComponent::HasValidPlayerState() const
{
    const APawn* Pawn = GetPawn<APawn>();
    return Pawn && Pawn->GetPlayerState() != nullptr;
}

bool UMyPawnExtensionComponent::HasValidGameData() const
{
    const APawn* Pawn = GetPawn<APawn>();
    const UWorld* World = Pawn ? Pawn->GetWorld() : nullptr;
    const AMyModularGameState* MyGameState = World ? World->GetGameState<AMyModularGameState>() : nullptr;
    return MyGameState && MyGameState->GetGameData() != nullptr;
}

bool UMyPawnExtensionComponent::ResolvePawnDataFromGameData()
{
    if (PawnData)
    {
        return true;
    }

    const APawn* Pawn = GetPawn<APawn>();
    const UWorld* World = Pawn ? Pawn->GetWorld() : nullptr;
    const AMyModularGameState* MyGameState = World ? World->GetGameState<AMyModularGameState>() : nullptr;
    const UMyGameData* GameData = MyGameState ? MyGameState->GetGameData() : nullptr;

    PawnData = GameData ? GameData->DefaultPawnData : nullptr;
    return PawnData != nullptr;
}
