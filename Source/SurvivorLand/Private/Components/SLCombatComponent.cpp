// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SLCombatComponent.h"

#include "Net/UnrealNetwork.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"

#include "SurvivorLandGameplayTags.h"
#include "AnimInstances/SLBasePlayerAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Characters/SLBaseGameCharacter.h"
#include "Components/SLInputHandlerComponent.h"
#include "Items/Weapons/SLWeaponBase.h"
#include "Data/SLWeaponData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

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

void USLCombatComponent::BindToInput(USLInputHandlerComponent* InputHandler)
{
	if (bBoundToInput || !InputHandler)
	{
		return;
	}

	InputHandler->OnActionStarted.AddDynamic(this, &USLCombatComponent::OnActionStarted);
	InputHandler->OnActionCompleted.AddDynamic(this, &USLCombatComponent::OnActionCompleted);
	InputHandler->OnAxis2D.AddDynamic(this, &USLCombatComponent::OnAxis2D);

	bBoundToInput = true;
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

void USLCombatComponent::OnRep_Aiming()
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	OwnerChar->SetStrafeAimingMode(bAiming);
}

void USLCombatComponent::OnActionStarted(FGameplayTag InputTag)
{
	// Drop weapon on G (Input.Shared.Drop)
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Drop)
	{
		if (HasEquippedWeapon())
		{
			if (!GetOwner() || !GetOwner()->HasAuthority())
			{
				Server_DropEquippedWeapon();
			}
			else
			{
				DropEquippedWeapon_Internal();
			}
		}
		return;
	}
	

	// Interact stays free, for now only pickup if unarmed
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Interact)
	{
		if (!HasEquippedWeapon())
		{
			if (!GetOwner() || !GetOwner()->HasAuthority())
			{
				Server_TryPickupWeapon();
			}
			else
			{
				TryPickupWeapon_Internal();
			}
		}
		return;
	}

	// Example: fire only when weapon context exists
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Fire)
	{
		if (HasEquippedWeapon())
		{
			UE_LOG(LogTemp, Log, TEXT("Fire pressed (weapon context active)"));
		}
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim)
	{
		SetAiming(true);
		return;
	}
}

void USLCombatComponent::OnActionCompleted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim)
	{
		SetAiming(false);
		return;
	}
}

void USLCombatComponent::OnAxis2D(FGameplayTag InputTag, FVector2D Value)
{
	// Combat doesn't handle axis yet
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
			Client_OnWeaponEquipped(NewEquipped->WeaponData->WeaponMappingContext, NewEquipped->WeaponData);
		}
	}
}


void USLCombatComponent::Client_OnWeaponEquipped_Implementation(UInputMappingContext* WeaponContext, const USLWeaponDataAsset* WeaponData)
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC) return;

	// Add mapping context locally
	if (WeaponContext)
	{
		EquipWeaponContext(PC, WeaponContext, 1);
	}

	// Bind weapon actions locally (Fire etc)
	if (WeaponData && OwnerChar->InputHandlerComponent)
	{
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerChar->InputComponent))
		{
			OwnerChar->InputHandlerComponent->BindAdditionalActions(EIC, WeaponData->GrantedInputActions);
		}
	}

	if (WeaponData)
	{
		if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
		{
			// Prefer weapon layer if provided
			if (WeaponData->SurvivorUpperBodyLayerClass)
			{
				Anim->LinkAnimClassLayers(WeaponData->SurvivorUpperBodyLayerClass);
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