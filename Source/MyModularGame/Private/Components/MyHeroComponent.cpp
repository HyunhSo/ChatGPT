#include "Components/MyHeroComponent.h"

#include "Components/MyPawnExtensionComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "System/MyInitStateTags.h"

UMyHeroComponent::UMyHeroComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UMyHeroComponent::BeginPlay()
{
    Super::BeginPlay();

    TrySetupPlayerInput();
}

void UMyHeroComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bPlayerInputSetup)
    {
        TrySetupPlayerInput();
    }
}

void UMyHeroComponent::TrySetupPlayerInput()
{
    if (bPlayerInputSetup)
    {
        return;
    }

    const APawn* Pawn = GetPawn<APawn>();
    const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    const UMyPawnExtensionComponent* PawnExtension = Pawn->FindComponentByClass<UMyPawnExtensionComponent>();
    if (!PawnExtension || !PawnExtension->HasReachedInitState(MyInitStateTags::TAG_InitState_DataInitialized))
    {
        return;
    }

    bPlayerInputSetup = true;
    UE_LOG(LogTemp, Log, TEXT("[HeroComponent] SetupPlayerInput for Pawn=%s Controller=%s"), *GetNameSafe(Pawn), *GetNameSafe(PC));

    BP_SetupPlayerInput();
}
