#include "Character/MLCombatComponent.h"

#include "Character/MLWeapon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMLCombat, Log, All);

namespace
{
    static UWorld* ResolveWorld(const TWeakObjectPtr<UWorld>& InWorld)
    {
        if (InWorld.IsValid())
        {
            return InWorld.Get();
        }

        if (!GEngine)
        {
            return nullptr;
        }

        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            UWorld* World = Context.World();
            if (World && World->IsGameWorld())
            {
                return World;
            }
        }

        return nullptr;
    }

    static void ExecuteConsoleFire(UWorld* World, const bool bStartFire)
    {
        if (!World)
        {
            UE_LOG(LogMLCombat, Warning, TEXT("[ML.Fire] World not found."));
            return;
        }

        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            APawn* Pawn = PC ? PC->GetPawn() : nullptr;
            UMLCombatComponent* CombatComponent = Pawn ? Pawn->FindComponentByClass<UMLCombatComponent>() : nullptr;

            if (!PC || !PC->IsLocalController() || !CombatComponent)
            {
                continue;
            }

            UE_LOG(LogMLCombat, Log, TEXT("[ML.Fire] Command=%d PC=%s Pawn=%s"), bStartFire ? 1 : 0, *GetNameSafe(PC), *GetNameSafe(Pawn));

            if (bStartFire)
            {
                CombatComponent->StartFire();
            }
            else
            {
                CombatComponent->StopFire();
            }

            return;
        }

        UE_LOG(LogMLCombat, Warning, TEXT("[ML.Fire] No local pawn with UMLCombatComponent found."));
    }

    static FAutoConsoleCommandWithWorldAndArgs CmdMLFire(
        TEXT("ML.Fire"),
        TEXT("ML.Fire 1 => StartFire, ML.Fire 0 => StopFire"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
        {
            if (Args.Num() < 1)
            {
                UE_LOG(LogMLCombat, Warning, TEXT("[ML.Fire] Missing arg. Usage: ML.Fire 1|0"));
                return;
            }

            const int32 bStartFireInt = FCString::Atoi(*Args[0]);
            ExecuteConsoleFire(ResolveWorld(World), bStartFireInt != 0);
        }));
}

UMLCombatComponent::UMLCombatComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UMLCombatComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner() || !GetOwner()->HasAuthority() || !bServerFiring)
    {
        return;
    }

    ShotTimeAccumulator += DeltaTime;
    const float ShotInterval = GetShotIntervalSeconds();

    while (ShotTimeAccumulator >= ShotInterval)
    {
        ShotTimeAccumulator -= ShotInterval;
        FireOnce(TEXT("Tick"));
    }
}

void UMLCombatComponent::StartFire()
{
    HandleStartFireRequest(TEXT("LocalStartFire"));
}

void UMLCombatComponent::StopFire()
{
    HandleStopFireRequest(TEXT("LocalStopFire"));
}

void UMLCombatComponent::ServerStartFire_Implementation()
{
    HandleStartFireRequest(TEXT("ServerStartFire_RPC"));
}

void UMLCombatComponent::ServerStopFire_Implementation()
{
    HandleStopFireRequest(TEXT("ServerStopFire_RPC"));
}

APawn* UMLCombatComponent::GetOwnerPawn() const
{
    return Cast<APawn>(GetOwner());
}

AController* UMLCombatComponent::GetOwnerController() const
{
    if (const APawn* Pawn = GetOwnerPawn())
    {
        return Pawn->GetController();
    }

    return nullptr;
}

APlayerState* UMLCombatComponent::GetOwnerPlayerState() const
{
    if (const APawn* Pawn = GetOwnerPawn())
    {
        return Pawn->GetPlayerState();
    }

    return nullptr;
}

void UMLCombatComponent::HandleStartFireRequest(const TCHAR* Source)
{
    APawn* Pawn = GetOwnerPawn();
    const bool bHasAuthority = GetOwner() && GetOwner()->HasAuthority();
    const bool bLocal = Pawn && Pawn->IsLocallyControlled();

    UE_LOG(LogMLCombat, Log, TEXT("[StartFire] Source=%s %s Authority=%s Local=%s"),
        Source,
        *BuildOwnerDebugString(),
        bHasAuthority ? TEXT("true") : TEXT("false"),
        bLocal ? TEXT("true") : TEXT("false"));

    if (!bHasAuthority)
    {
        if (bLocal)
        {
            ServerStartFire();
        }
        else
        {
            UE_LOG(LogMLCombat, Warning, TEXT("[StartFire] Ignored request from non-local client proxy. %s"), *BuildOwnerDebugString());
        }
        return;
    }

    if (bServerFiring)
    {
        UE_LOG(LogMLCombat, Verbose, TEXT("[StartFire] Already firing. %s"), *BuildOwnerDebugString());
        return;
    }

    bServerFiring = true;
    ShotTimeAccumulator = GetShotIntervalSeconds();

    UE_LOG(LogMLCombat, Log, TEXT("[StartFire] Server accepted fire. FireRate=%.2f Range=%.1f %s"), GetEffectiveFireRate(), EquippedWeapon ? EquippedWeapon->GetFireRange() : FireRange, *BuildOwnerDebugString());
}

void UMLCombatComponent::HandleStopFireRequest(const TCHAR* Source)
{
    APawn* Pawn = GetOwnerPawn();
    const bool bHasAuthority = GetOwner() && GetOwner()->HasAuthority();
    const bool bLocal = Pawn && Pawn->IsLocallyControlled();

    UE_LOG(LogMLCombat, Log, TEXT("[StopFire] Source=%s %s Authority=%s Local=%s"),
        Source,
        *BuildOwnerDebugString(),
        bHasAuthority ? TEXT("true") : TEXT("false"),
        bLocal ? TEXT("true") : TEXT("false"));

    if (!bHasAuthority)
    {
        if (bLocal)
        {
            ServerStopFire();
        }
        else
        {
            UE_LOG(LogMLCombat, Warning, TEXT("[StopFire] Ignored request from non-local client proxy. %s"), *BuildOwnerDebugString());
        }
        return;
    }

    if (!bServerFiring)
    {
        UE_LOG(LogMLCombat, Verbose, TEXT("[StopFire] Already idle. %s"), *BuildOwnerDebugString());
        return;
    }

    bServerFiring = false;
    ShotTimeAccumulator = 0.0f;

    UE_LOG(LogMLCombat, Log, TEXT("[StopFire] Server stopped fire. TotalShots=%d %s"), ServerShotCount, *BuildOwnerDebugString());
}

void UMLCombatComponent::FireOnce(const TCHAR* Source)
{
    check(GetOwner() && GetOwner()->HasAuthority());

    ++ServerShotCount;

    UE_LOG(LogMLCombat, Log, TEXT("[FireOnce] Source=%s Shot=%d FireRate=%.2f Range=%.1f %s"),
        Source,
        ServerShotCount,
        GetEffectiveFireRate(),
        EquippedWeapon ? EquippedWeapon->GetFireRange() : FireRange,
        *BuildOwnerDebugString());
}

float UMLCombatComponent::GetEffectiveFireRate() const
{
    if (EquippedWeapon)
    {
        return FMath::Max(0.01f, EquippedWeapon->GetFireRate());
    }

    return FMath::Max(0.01f, FireRate);
}

float UMLCombatComponent::GetShotIntervalSeconds() const
{
    return 1.0f / GetEffectiveFireRate();
}

FString UMLCombatComponent::BuildOwnerDebugString() const
{
    const APawn* Pawn = GetOwnerPawn();
    const AController* Controller = GetOwnerController();
    const APlayerState* PlayerState = GetOwnerPlayerState();

    const ENetMode NetMode = GetWorld() ? GetWorld()->GetNetMode() : NM_Standalone;
    const ENetRole LocalRole = GetOwner() ? GetOwner()->GetLocalRole() : ROLE_None;

    return FString::Printf(TEXT("[NetMode=%d Role=%d Pawn=%s Controller=%s PlayerState=%s]"),
        static_cast<int32>(NetMode),
        static_cast<int32>(LocalRole),
        *GetNameSafe(Pawn),
        *GetNameSafe(Controller),
        *GetNameSafe(PlayerState));
}
