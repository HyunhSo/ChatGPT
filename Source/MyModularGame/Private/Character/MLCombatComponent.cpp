#include "Character/MLCombatComponent.h"

#include "Character/MLWeapon.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMLCombat, Log, All);

namespace
{
	static TAutoConsoleVariable<int32> CVarMLCombatDebug(
		TEXT("ML.CombatDebug"),
		1,
		TEXT("Enable combat debug draw/log (0:off, 1:on)."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarMLCombatRange(
		TEXT("ML.CombatRange"),
		5000.0f,
		TEXT("Override combat hitscan range. <=0 keeps weapon/component range."),
		ECVF_Default);

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

	UE_LOG(LogMLCombat, Log, TEXT("[StartFire] Server accepted fire. FireRate=%.2f Range=%.1f %s"), GetEffectiveFireRate(), GetEffectiveFireRange(), *BuildOwnerDebugString());
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
		GetEffectiveFireRange(),
		*BuildOwnerDebugString());

	ServerFireShot();
}

void UMLCombatComponent::ServerFireShot()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	APawn* Pawn = GetOwnerPawn();
	if (!Pawn)
	{
		UE_LOG(LogMLCombat, Warning, TEXT("[ServerFireShot] Missing pawn owner."));
		return;
	}

	AController* Controller = GetOwnerController();

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	bool bHasViewPoint = false;

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (PC->PlayerCameraManager)
		{
			ViewLocation = PC->PlayerCameraManager->GetCameraLocation();
			ViewRotation = PC->PlayerCameraManager->GetCameraRotation();
			bHasViewPoint = true;
		}
	}

	if (!bHasViewPoint)
	{
		Pawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
		bHasViewPoint = true;
	}

	if (!bHasViewPoint)
	{
		UE_LOG(LogMLCombat, Warning, TEXT("[ServerFireShot] No valid viewpoint. %s"), *BuildOwnerDebugString());
		return;
	}

	const FVector TraceStart = ViewLocation;
	const FVector TraceDirection = ViewRotation.Vector();
	const FVector TraceEnd = TraceStart + (TraceDirection * GetEffectiveFireRange());

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(MLCombatTrace), true);
	TraceParams.AddIgnoredActor(Pawn);
	if (GetOwner())
	{
		TraceParams.AddIgnoredActor(GetOwner());
	}

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
	const FVector FinalEnd = bHit ? Hit.ImpactPoint : TraceEnd;

	if (bHit)
	{
		const AActor* HitActor = Hit.GetActor();
		const bool bEnemyHit = HitActor && HitActor->ActorHasTag(TEXT("enemy"));
		UE_LOG(LogMLCombat, Log, TEXT("[ServerFireShot] %s Actor=%s Dist=%.1f Bone=%s Impact=%s %s"),
			bEnemyHit ? TEXT("ENEMY HIT") : TEXT("WORLD HIT"),
			*GetNameSafe(HitActor),
			Hit.Distance,
			*Hit.BoneName.ToString(),
			*Hit.ImpactPoint.ToString(),
			*BuildOwnerDebugString());
	}
	else
	{
		UE_LOG(LogMLCombat, Log, TEXT("[ServerFireShot] MISS End=%s %s"), *TraceEnd.ToString(), *BuildOwnerDebugString());
	}

	if (CVarMLCombatDebug.GetValueOnGameThread() != 0)
	{
		DrawDebugLine(GetWorld(), TraceStart, FinalEnd, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.5f);
		if (bHit)
		{
			DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 12.0f, FColor::Yellow, false, 1.0f, 0);
		}
	}

	MulticastOnFireReplicated(TraceStart, FinalEnd, bHit, bHit ? Hit.ImpactPoint : TraceEnd);
}

void UMLCombatComponent::MulticastOnFireReplicated_Implementation(const FVector_NetQuantize TraceStart, const FVector_NetQuantize TraceEnd, const bool bDidHit, const FVector_NetQuantize HitLocation)
{
	UE_LOG(LogMLCombat, Log, TEXT("[OnFireReplicated] Hit=%s Start=%s End=%s Impact=%s %s"),
		bDidHit ? TEXT("true") : TEXT("false"),
		*TraceStart.ToString(),
		*TraceEnd.ToString(),
		*HitLocation.ToString(),
		*BuildOwnerDebugString());

	if (CVarMLCombatDebug.GetValueOnGameThread() != 0)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bDidHit ? FColor::Orange : FColor::Blue, false, 1.0f, 0, 1.0f);
		if (bDidHit)
		{
			DrawDebugPoint(GetWorld(), HitLocation, 8.0f, FColor::Cyan, false, 1.0f, 0);
		}
	}
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

float UMLCombatComponent::GetEffectiveFireRange() const
{
	const float ConsoleRange = CVarMLCombatRange.GetValueOnGameThread();
	if (ConsoleRange > 0.0f)
	{
		return ConsoleRange;
	}

	if (EquippedWeapon)
	{
		return FMath::Max(1.0f, EquippedWeapon->GetFireRange());
	}

	return FMath::Max(1.0f, FireRange);
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
