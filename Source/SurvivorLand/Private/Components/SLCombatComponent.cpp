// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SLCombatComponent.h"

#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "AnimInstances/SLBasePlayerAnimInstance.h"
#include "Characters/SLBaseGameCharacter.h"
#include "Components/SLInputHandlerComponent.h"
#include "Data/Weapon/SLWeaponAnimProfile.h"
#include "Items/Weapons/SLWeaponBase.h"
#include "Data/Weapon/SLWeaponData.h"
#include "Data/Weapon/SLWeaponInputProfile.h"
#include "Kismet/GameplayStatics.h"

USLCombatComponent::USLCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USLCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USLCombatComponent, Inventory);
	DOREPLIFETIME(USLCombatComponent, EquippedIndex);
	DOREPLIFETIME(USLCombatComponent, bAiming);
}

void USLCombatComponent::Server_TryPickupWeapon_Implementation()
{
	TryPickupWeapon_Internal();
}

void USLCombatComponent::TryPickupWeapon_Internal()
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	// Find nearest overlapping weapon that is NOT held
	TArray<AActor*> Overlaps;
	OwnerChar->GetOverlappingActors(Overlaps, ASLWeaponBase::StaticClass());

	ASLWeaponBase* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (AActor* A : Overlaps)
	{
		ASLWeaponBase* W = Cast<ASLWeaponBase>(A);
		if (!W || !W->WeaponData) continue;
		if (W->IsHeld()) continue;

		const float D = FVector::DistSquared(W->GetActorLocation(), OwnerChar->GetActorLocation());
		if (D < BestDistSq)
		{
			BestDistSq = D;
			Best = W;
		}
	}

	if (!Best)
	{
		return;
	}

	// Add to inventory and equip it (supports multi-weapon later)
	Inventory.Add(Best);
	EquippedIndex = Inventory.Num() - 1;

	Best->ServerGiveTo(OwnerChar);
}

void USLCombatComponent::Server_DropEquippedWeapon_Implementation()
{
	DropEquippedWeapon_Internal();
}

void USLCombatComponent::DropEquippedWeapon_Internal()
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	ASLWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon) return;

	// Tell owning client to remove weapon mapping context
	Client_OnWeaponUnequipped();

	// Drop location in front of player
	const FVector DropLoc =
		OwnerChar->GetActorLocation() +
		OwnerChar->GetActorForwardVector() * 120.f +
		FVector(0.f, 0.f, 40.f);

	// Optional small impulse forward
	const FVector Impulse = OwnerChar->GetActorForwardVector() * 200.f;

	Weapon->ServerDropFromOwner(DropLoc, Impulse);

	// Remove from inventory
	Inventory.RemoveAt(EquippedIndex);

	// Update equipped index
	EquippedIndex = (Inventory.Num() > 0) ? FMath::Clamp(EquippedIndex, 0, Inventory.Num() - 1) : INDEX_NONE;

	// If we still have a weapon, equip its context/binds on owning client
	if (HasEquippedWeapon())
	{
		ASLWeaponBase* NewEquipped = GetEquippedWeapon();
		if (NewEquipped && NewEquipped->WeaponData)
		{
			Client_OnWeaponEquipped(NewEquipped->WeaponData->InputProfile->MappingContext, NewEquipped->WeaponData);
		}
	}
}


void USLCombatComponent::Client_OnWeaponEquipped_Implementation(UInputMappingContext* /*WeaponContext*/, const USLWeaponDataAsset* WeaponData)
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar || !WeaponData) return;

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC) return;

	// INPUT PROFILE
	if (WeaponData->InputProfile)
	{
		if (WeaponData->InputProfile->MappingContext)
		{
			EquipWeaponContext(PC, WeaponData->InputProfile->MappingContext, 1);
		}

		if (OwnerChar->InputHandlerComponent)
		{
			if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerChar->InputComponent))
			{
				OwnerChar->InputHandlerComponent->BindAdditionalActions(
					EIC,
					WeaponData->InputProfile->GrantedInputActions
				);
			}
		}
	}

	// ANIM PROFILE
	if (WeaponData->AnimProfile)
	{
		if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
		{
			if (WeaponData->AnimProfile->SurvivorUpperBodyLayerClass)
			{
				Anim->LinkAnimClassLayers(WeaponData->AnimProfile->SurvivorUpperBodyLayerClass);
			}
		}
	}
}

void USLCombatComponent::Client_OnWeaponUnequipped_Implementation()
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC) return;

	if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
	{
		// If you stored it on the anim instance:
		if (USLBasePlayerAnimInstance* SLAnim = Cast<USLBasePlayerAnimInstance>(Anim))
		{
			if (SLAnim->DefaultUnarmedUpperBodyLayerClass)
			{
				Anim->LinkAnimClassLayers(SLAnim->DefaultUnarmedUpperBodyLayerClass);
			}
		}
	}

	UnequipWeaponContext(PC);
}

void USLCombatComponent::SetAiming(bool bNewAiming)
{
	// local prediction (optional but feels better)
	if (bAiming == bNewAiming) return;
	bAiming = bNewAiming;
	OnRep_Aiming(); // update cosmetic immediately

	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_SetAiming(bNewAiming);
	}
}

void USLCombatComponent::Server_SetAiming_Implementation(bool bNewAiming)
{
	bAiming = bNewAiming;
	OnRep_Aiming(); // if you want server to run same cosmetic (usually fine)
}

void USLCombatComponent::FirePressed()
{

	if (!bAiming) return;
	
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	ASLWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon || !Weapon->WeaponData) return;

	// For now: use client aimpoint. Later: server recompute for validation.
	const FVector AimPoint = OwnerChar->GetAimTargetWorld();

	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_Fire(AimPoint);
	}
	else
	{
		Server_Fire(AimPoint); // ok for listen-server during testing
	}
}

void USLCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& AimPoint)
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	ASLWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon || !Weapon->WeaponData) return;

	TArray<FHitResult> Hits;
	PerformBallisticsTrace(Weapon, AimPoint, Hits);

	const FVector TraceStart = Weapon->GetMuzzleTransform().GetLocation();
	ResolvePenetrationAndDamage(Weapon, Hits, TraceStart);

	// Debug (optional)
	DrawDebugLine(GetWorld(), TraceStart, TraceStart + (AimPoint-TraceStart).GetSafeNormal()*Weapon->WeaponData->Ballistics.MaxRange, FColor::Red, false, 1.f);
}

void USLCombatComponent::PerformBallisticsTrace(const ASLWeaponBase* Weapon, const FVector& AimPoint, TArray<FHitResult>& OutHits) const
{
	OutHits.Reset();

	const USLWeaponDataAsset* Data = Weapon->WeaponData;
	const auto& B = Data->Ballistics;

	const FVector Start = Weapon->GetMuzzleTransform().GetLocation();

	FVector Dir = (AimPoint - Start);
	if (!Dir.Normalize())
	{
		// fallback: shoot forward from owner control rotation
		if (const ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner()))
		{
			Dir = OwnerChar->GetControlRotation().Vector();
		}
		else
		{
			Dir = FVector::ForwardVector;
		}
	}

	const FVector End = Start + Dir * B.MaxRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(Ballistics), /*bTraceComplex*/ true);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(const_cast<ASLWeaponBase*>(Weapon));

	// Multi-hit
	if (B.TraceRadius > 0.f)
	{
		const FCollisionShape Shape = FCollisionShape::MakeSphere(B.TraceRadius);
		GetWorld()->SweepMultiByChannel(OutHits, Start, End, FQuat::Identity, B.TraceChannel, Shape, Params);
	}
	else
	{
		GetWorld()->LineTraceMultiByChannel(OutHits, Start, End, B.TraceChannel, Params);
	}

	// Sort by distance from start (important)
	OutHits.Sort([&](const FHitResult& A, const FHitResult& BHit)
	{
		return A.Distance < BHit.Distance;
	});
}

void USLCombatComponent::ResolvePenetrationAndDamage(const ASLWeaponBase* Weapon, const TArray<FHitResult>& Hits, const FVector& TraceStart)
{
	const USLWeaponDataAsset* Data = Weapon->WeaponData;
	const auto& B = Data->Ballistics;

	float RemainingPen = B.PenetrationDepth;
	float CurrentDamage = B.BaseDamage;

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) continue;

		// Apply damage to damageables (placeholder)
		if (HitActor != GetOwner())
		{
			UGameplayStatics::ApplyPointDamage(
				HitActor,
				CurrentDamage,
				(Hit.TraceEnd - Hit.TraceStart).GetSafeNormal(),
				Hit,
				Cast<APawn>(GetOwner()) ? Cast<APawn>(GetOwner())->GetController() : nullptr,
				const_cast<AActor*>(GetOwner()),
				nullptr
			);
		}

		// Decide penetration cost
		float Cost = 0.f;

		// Example simple rules; replace with PhysMat later
		const ECollisionChannel MyChannel = Hit.Component.IsValid() ? Hit.Component->GetCollisionObjectType() : ECC_WorldStatic;

		if (MyChannel == ECC_WorldStatic)
		{
			Cost = 999999.f; // effectively stops
		}
		else if (MyChannel == ECC_WorldDynamic)
		{
			Cost = 25.f; // tune
		}
		else if (MyChannel == ECC_Pawn)
		{
			Cost = 0.f; // allow multi-hit on pawns
		}
		else
		{
			Cost = 10.f;
		}

		// Reduce damage after each penetration (optional)
		if (RemainingPen > 0.f && Cost < 999999.f)
		{
			CurrentDamage *= 0.85f; // tune
		}

		// Consume penetration budget and decide stop
		if (B.PenetrationDepth <= 0.f) // no penetration configured
		{
			break;
		}

		RemainingPen -= Cost;
		if (RemainingPen <= 0.f || Cost >= 999999.f)
		{
			break;
		}
	}
}

void USLCombatComponent::DropEquippedWeapon()
{
	Server_DropEquippedWeapon();
}

void USLCombatComponent::TryInteract()
{
	Server_TryPickupWeapon();
}

void USLCombatComponent::OnRep_Aiming()
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	OwnerChar->SetStrafeAimingMode(bAiming);
}

void USLCombatComponent::EquipWeaponContext(APlayerController* PC, UInputMappingContext* WeaponContext, int32 Priority)
{
	if (!PC || !WeaponContext)
	{
		return;
	}

	EquippedWeaponContext = WeaponContext;

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(WeaponContext, Priority);
		}
	}
}

void USLCombatComponent::UnequipWeaponContext(APlayerController* PC)
{
	if (!PC || !EquippedWeaponContext)
	{
		return;
	}

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->RemoveMappingContext(EquippedWeaponContext);
		}
	}

	EquippedWeaponContext = nullptr;
}