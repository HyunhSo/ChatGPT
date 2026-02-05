#include "Character/MLHeroComponent.h"

#include "Character/MLPawnData.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogMLHero, Log, All);

namespace
{
    static FAutoConsoleCommand CmdMLInputStatus(
        TEXT("ML.InputStatus"),
        TEXT("Dump local-player/input-subsystem status for hero setup."),
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

                UE_LOG(LogMLHero, Log, TEXT("[ML.InputStatus] World=%s NetMode=%d"), *GetNameSafe(World), static_cast<int32>(World->GetNetMode()));

                for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
                {
                    const APlayerController* PC = It->Get();
                    const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
                    const ULocalPlayer* LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
                    const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;

                    UE_LOG(LogMLHero, Log, TEXT("[ML.InputStatus] PC=%s Pawn=%s IsLocalController=%s IsLocallyControlledPawn=%s LocalPlayer=%s InputSubsystem=%s"),
                        *GetNameSafe(PC),
                        *GetNameSafe(Pawn),
                        PC && PC->IsLocalController() ? TEXT("true") : TEXT("false"),
                        Pawn && Pawn->IsLocallyControlled() ? TEXT("true") : TEXT("false"),
                        LocalPlayer ? TEXT("true") : TEXT("false"),
                        InputSubsystem ? TEXT("true") : TEXT("false"));
                }
            }
        })
    );
}

void UMLHeroComponent::HandleGameplayReady(const UMLPawnData* PawnData)
{
    if (bDidLocalSetup)
    {
        UE_LOG(LogMLHero, Verbose, TEXT("[%s] GameplayReady already handled."), *GetNameSafe(GetOwner()));
        return;
    }

    const APawn* Pawn = GetPawn<APawn>();
    if (!Pawn)
    {
        UE_LOG(LogMLHero, Warning, TEXT("HandleGameplayReady called without Pawn owner."));
        return;
    }

    if (!IsLocalPlayerPawn(Pawn))
    {
        UE_LOG(LogMLHero, Log, TEXT("[%s] Skip hero setup: pawn is not locally controlled."), *GetNameSafe(Pawn));
        return;
    }


    bDidLocalSetup = true;
    bInputSetupDone = false;
    bCameraSetupDone = false;

    UE_LOG(LogMLHero, Log, TEXT("[%s] GameplayReady(local): starting hero setup pipeline."), *GetNameSafe(Pawn));

    SetupPlayerInput(Pawn, PawnData);
    SetupPlayerCamera(Pawn, PawnData);

    UE_LOG(LogMLHero, Log, TEXT("[%s] Hero setup complete. InputDone=%s CameraDone=%s InputSubsystemAccess=%s"),
        *GetNameSafe(Pawn),
        bInputSetupDone ? TEXT("true") : TEXT("false"),
        bCameraSetupDone ? TEXT("true") : TEXT("false"),
        bLastInputSubsystemAccess ? TEXT("true") : TEXT("false"));
}

bool UMLHeroComponent::IsLocalPlayerPawn(const APawn* Pawn) const
{
    if (!Pawn)
    {
        return false;
    }

    if (Pawn->IsLocallyControlled())
    {
        return true;
    }

    const APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    return PC && PC->IsLocalController() && (PC->GetLocalPlayer() != nullptr);
}

void UMLHeroComponent::SetupPlayerInput(const APawn* Pawn, const UMLPawnData* PawnData)
{
    const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    const ULocalPlayer* LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
    UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;

    bLastInputSubsystemAccess = (InputSubsystem != nullptr);

    UE_LOG(LogMLHero, Log, TEXT("[%s] InputSetup: PC=%s LocalPlayer=%s InputSubsystem=%s"),
        *GetNameSafe(Pawn),
        *GetNameSafe(PC),
        LocalPlayer ? TEXT("true") : TEXT("false"),
        InputSubsystem ? TEXT("true") : TEXT("false"));

    if (!PawnData)
    {
        UE_LOG(LogMLHero, Log, TEXT("[%s] InputSetup: PawnData missing, running minimal no-asset path."), *GetNameSafe(Pawn));
    }

    bInputSetupDone = true;
}

void UMLHeroComponent::SetupPlayerCamera(const APawn* Pawn, const UMLPawnData* PawnData)
{
    const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    const AActor* ViewTarget = PC ? PC->GetViewTarget() : nullptr;

    UE_LOG(LogMLHero, Log, TEXT("[%s] CameraSetup: entering. ViewTarget=%s Pawn=%s"),
        *GetNameSafe(Pawn),
        *GetNameSafe(ViewTarget),
        *GetNameSafe(Pawn));

    if (PawnData && !PawnData->CameraModeClass.IsNull())
    {
        UE_LOG(LogMLHero, Log, TEXT("[%s] CameraSetup: placeholder apply CameraModeClass=%s"),
            *GetNameSafe(Pawn),
            *PawnData->CameraModeClass.ToString());
    }
    else
    {
        UE_LOG(LogMLHero, Log, TEXT("[%s] CameraSetup: no CameraModeClass on PawnData; skipping apply."), *GetNameSafe(Pawn));
    }

    bCameraSetupDone = true;
}
