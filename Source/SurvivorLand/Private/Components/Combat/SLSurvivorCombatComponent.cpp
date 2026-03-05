// Fill out your copyright notice in the Description page of Project Settings.

#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "AnimInstances/SLBasePlayerAnimInstance.h"
#include "Characters/SLBaseGameCharacter.h"
#include "Characters/SLSurvivorCharacterBase.h"
#include "Components/SLInputHandlerComponent.h"
#include "Data/Weapon/SLWeaponAnimProfile.h"
#include "Items/Weapons/SLWeaponBase.h"
#include "Data/Weapon/SLWeaponData.h"
#include "Data/Weapon/SLWeaponInputProfile.h"
#include "Kismet/GameplayStatics.h"

USLSurvivorCombatComponent::USLSurvivorCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USLSurvivorCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Inventory)
	DOREPLIFETIME(ThisClass, EquippedIndex);
	DOREPLIFETIME(ThisClass, bAiming);
}

void USLSurvivorCombatComponent::Server_TryPickupWeapon_Implementation()
{
	TryPickupWeapon_Internal();
}

void USLSurvivorCombatComponent::TryPickupWeapon_Internal()
{
	ASLSurvivorCharacterBase* OwnerChar = Cast<ASLSurvivorCharacterBase>(GetOwner());
	if (!OwnerChar) return;

	// Find nearest overlapping weapon that is NOT held
	TArray<AActor*> Overlaps;
	OwnerChar->GetOverlappingActors(Overlaps, ASLWeaponBase::StaticClass());

	ASLWeaponBase* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (AActor* A : Overlaps)
	{
		ASLWeaponBase* W = Cast<ASLWeaponBase>(A);
		if (!W || !W->GetWeaponData()) continue;
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

void USLSurvivorCombatComponent::Server_DropEquippedWeapon_Implementation()
{
	DropEquippedWeapon_Internal();
}

void USLSurvivorCombatComponent::DropEquippedWeapon_Internal()
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
		if (NewEquipped && NewEquipped->GetWeaponData())
		{
			Client_OnWeaponEquipped(NewEquipped->GetWeaponData());
		}
	}
}


void USLSurvivorCombatComponent::Client_OnWeaponEquipped_Implementation(const USLWeaponDataAsset* WeaponData)
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

		if (OwnerChar->GetInputHandlerComponent())
		{
			if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerChar->InputComponent))
			{
				OwnerChar->GetInputHandlerComponent()->BindAdditionalActions(
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

void USLSurvivorCombatComponent::Client_OnWeaponUnequipped_Implementation()
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

void USLSurvivorCombatComponent::FirePressed()
{
	if (!bAiming) return;
	
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	ASLWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon || !Weapon->GetWeaponData()) return;

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

void USLSurvivorCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& AimPoint)
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	ASLWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon || !Weapon->GetWeaponData()) return;

	TArray<FHitResult> Hits;
	PerformBallisticsTrace(Weapon, AimPoint, Hits);

	const FVector TraceStart = Weapon->GetMuzzleTransform().GetLocation();
	ResolvePenetrationAndDamage(Weapon, Hits, TraceStart);

	// Debug (optional)
	DrawDebugLine(GetWorld(), TraceStart, TraceStart + (AimPoint-TraceStart).GetSafeNormal()*Weapon->GetWeaponData()->Ballistics.MaxRange, FColor::Red, false, 1.f);
}

void USLSurvivorCombatComponent::PerformBallisticsTrace(const ASLWeaponBase* Weapon, const FVector& AimPoint, TArray<FHitResult>& OutHits) const
{
	OutHits.Reset();

	const USLWeaponDataAsset* Data = Weapon->GetWeaponData();
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

void USLSurvivorCombatComponent::ResolvePenetrationAndDamage(const ASLWeaponBase* Weapon, const TArray<FHitResult>& Hits, const FVector& TraceStart) const
{
	const USLWeaponDataAsset* Data = Weapon->GetWeaponData();
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
		float Cost;

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

void USLSurvivorCombatComponent::DropEquippedWeapon()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		DropEquippedWeapon_Internal();
	}
	else
	{
		Server_DropEquippedWeapon();
	}
}

void USLSurvivorCombatComponent::TryPickupWeapon()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TryPickupWeapon_Internal();
	}
	else
	{
		Server_TryPickupWeapon();
	}
}

void USLSurvivorCombatComponent::EquipWeaponContext(APlayerController* PC, UInputMappingContext* WeaponContext, int32 Priority)
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

void USLSurvivorCombatComponent::UnequipWeaponContext(APlayerController* PC)
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

void USLSurvivorCombatComponent::HandleActionStarted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Interact)
	{
		TryPickupWeapon();
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim)
	{
		SetAiming(true);
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Fire)
	{
		FirePressed();
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Drop)
	{
		DropEquippedWeapon();
		return;
	}
}

void USLSurvivorCombatComponent::HandleActionCompleted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim)
	{
		SetAiming(false);
		return;
	}
}