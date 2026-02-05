#include "Character/MLPawnExtensionComponent.h"

#include "Character/MLHeroComponent.h"
#include "Character/MLPawnData.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/MLGameState.h"
#include "HAL/IConsoleManager.h"
#include "System/MLExperienceDefinition.h"
#include "UObject/UObjectIterator.h"

namespace MLPawnInitState
{
    static const TCHAR* ToString(const EMLPawnInitState State)
    {
        switch (State)
        {
        case EMLPawnInitState::Spawned:
            return TEXT("Spawned");
        case EMLPawnInitState::DataAvailable:
            return TEXT("DataAvailable");
        case EMLPawnInitState::DataInitialized:
            return TEXT("DataInitialized");
        case EMLPawnInitState::GameplayReady:
            return TEXT("GameplayReady");
        default:
            return TEXT("Unknown");
        }
    }
}

UMLPawnExtensionComponent::UMLPawnExtensionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UMLPawnExtensionComponent::BeginPlay()
{
    Super::BeginPlay();

    TryProgressInitState();
}

void UMLPawnExtensionComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TryProgressInitState();

    if (InitState == EMLPawnInitState::GameplayReady)
    {
        SetComponentTickEnabled(false);
    }
}

void UMLPawnExtensionComponent::TryProgressInitState()
{
    if (InitState == EMLPawnInitState::Spawned && CanEnterDataAvailable())
    {
        SetInitState(EMLPawnInitState::DataAvailable);
    }

    if (InitState == EMLPawnInitState::DataAvailable && CanEnterDataInitialized())
    {
        SetInitState(EMLPawnInitState::DataInitialized);
    }

    if (InitState == EMLPawnInitState::DataInitialized && CanEnterGameplayReady())
    {
        SetInitState(EMLPawnInitState::GameplayReady);
    }
}

bool UMLPawnExtensionComponent::CanEnterDataAvailable() const
{
    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        return false;
    }

    const AController* Controller = Pawn->GetController();
    if (Pawn->IsPlayerControlled())
    {
        if (!Controller || !Controller->PlayerState)
        {
            return false;
        }
    }

    const UWorld* World = GetWorld();
    const AMLGameState* MLGameState = (World != nullptr) ? World->GetGameState<AMLGameState>() : nullptr;
    return MLGameState != nullptr;
}

bool UMLPawnExtensionComponent::CanEnterDataInitialized()
{
    const UWorld* World = GetWorld();
    const AMLGameState* MLGameState = (World != nullptr) ? World->GetGameState<AMLGameState>() : nullptr;
    if (!MLGameState || !MLGameState->IsExperienceReady())
    {
        return false;
    }

    const UMLExperienceDefinition* Experience = MLGameState->GetCurrentExperience();
    if (!Experience || !Experience->DefaultPawnData)
    {
        return false;
    }

    PawnData = Experience->DefaultPawnData;
    return true;
}

bool UMLPawnExtensionComponent::CanEnterGameplayReady() const
{
    return PawnData != nullptr;
}

void UMLPawnExtensionComponent::SetInitState(const EMLPawnInitState NewState)
{
    if (InitState == NewState)
    {
        return;
    }

    InitState = NewState;

    UE_LOG(LogTemp, Log, TEXT("MLPawnExtensionComponent: %s entered state %s (PawnData=%s)"),
        *GetNameSafe(GetOwner()),
        MLPawnInitState::ToString(InitState),
        *GetNameSafe(PawnData));

    if (InitState == EMLPawnInitState::GameplayReady)
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

void UMLPawnExtensionComponent::DumpInitState() const
{
    UE_LOG(LogTemp, Log, TEXT("ML.InitStateDump: Pawn=%s State=%s PawnData=%s"),
        *GetNameSafe(GetOwner()),
        MLPawnInitState::ToString(InitState),
        *GetNameSafe(PawnData));
}

static void DumpAllInitStates()
{
    for (TObjectIterator<UMLPawnExtensionComponent> It; It; ++It)
    {
        UMLPawnExtensionComponent* Component = *It;
        if (IsValid(Component) && Component->GetWorld() && Component->GetWorld()->IsGameWorld())
        {
            Component->DumpInitState();
        }
    }
}

static FAutoConsoleCommand CVarMLInitStateDump(
    TEXT("ML.InitStateDump"),
    TEXT("Dump init-state information for all UMLPawnExtensionComponent instances."),
    FConsoleCommandDelegate::CreateStatic(&DumpAllInitStates));
