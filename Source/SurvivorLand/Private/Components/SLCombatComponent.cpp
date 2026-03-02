// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Components/SLCombatComponent.h"

#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "SurvivorLandGameplayTags.h" // or your tag header
#include "Characters/SLBaseGameCharacter.h"
#include "Components/SLInputHandlerComponent.h"
#include "Items/Weapons/SLWeaponBase.h"

USLCombatComponent::USLCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USLCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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

void USLCombatComponent::OnActionStarted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Interact)
	{
		if (GetOwnerRole() < ROLE_Authority)
		{
			Server_TryPickupWeapon();
		}
		else
		{
			TryPickupWeapon_Internal();
		}
	}
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Fire)
	{
		UE_LOG(LogTemp, Log, TEXT("Fire pressed (weapon context active)"));
	}
}

void USLCombatComponent::OnActionCompleted(FGameplayTag InputTag)
{
	// nothing yet
}

void USLCombatComponent::OnAxis2D(FGameplayTag InputTag, FVector2D Value)
{
	// Combat doesn't need axis yet; BaseGameCharacter handles move/look
}

void USLCombatComponent::Server_TryPickupWeapon_Implementation()
{
	TryPickupWeapon_Internal();
}

void USLCombatComponent::TryPickupWeapon_Internal()
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar) return;

	TArray<AActor*> Overlaps;
	OwnerChar->GetOverlappingActors(Overlaps, ASLWeaponBase::StaticClass());

	ASLWeaponBase* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (AActor* A : Overlaps)
	{
		ASLWeaponBase* W = Cast<ASLWeaponBase>(A);
		if (!W || !W->WeaponData) continue;

		const float D = FVector::DistSquared(W->GetActorLocation(), OwnerChar->GetActorLocation());
		if (D < BestDistSq)
		{
			BestDistSq = D;
			Best = W;
		}
	}

	if (Best)
	{
		Best->ServerGiveTo(OwnerChar);
	}
}