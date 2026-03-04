// Ean Sullivan All Rights Reserved

#include "Items/Weapons/SLWeaponBase.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Characters/SLBaseGameCharacter.h"
#include "Components/SLCombatComponent.h"
#include "Data/Weapon/SLWeaponData.h"
#include "Data/Weapon/SLWeaponInputProfile.h"

ASLWeaponBase::ASLWeaponBase()
{
	bReplicates = true;
	SetReplicateMovement(true);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// World/default state: should NOT float
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor")); // good default for dropped items

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(Mesh);
	PickupSphere->InitSphereRadius(120.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ASLWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyVisualFromDataAsset();
}

void ASLWeaponBase::ApplyVisualFromDataAsset()
{
	if (WeaponData && WeaponData->WeaponMesh)
	{
		Mesh->SetSkeletalMesh(WeaponData->WeaponMesh);
	}
	else
	{
		Mesh->SetSkeletalMesh(nullptr);
	}
}

void ASLWeaponBase::SetPickupEnabled(bool bEnabled)
{
	if (!PickupSphere) return;

	PickupSphere->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	SetActorEnableCollision(bEnabled);
	SetActorHiddenInGame(false); // keep visible unless you specifically hide elsewhere
}

void ASLWeaponBase::SetPhysicsEnabled(bool bEnabled)
{
	if (!Mesh) return;

	Mesh->SetSimulatePhysics(bEnabled);

	// Physics requires QueryAndPhysics (or PhysicsOnly) and a valid collision profile.
	// QueryAndPhysics is friendlier if you want overlaps/traces later.
	Mesh->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void ASLWeaponBase::ServerGiveTo(ASLBaseGameCharacter* NewOwnerChar)
{
	if (!HasAuthority() || !NewOwnerChar || !WeaponData || bIsHeld)
	{
		return;
	}

	bIsHeld = true;

	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Decide socket based on weapon grip/type
	const FName AttachSocket = NewOwnerChar->GetWeaponAttachSocket(WeaponData->Grip);

	AttachToComponent(
		NewOwnerChar->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		AttachSocket
	);

	if (USLCombatComponent* Combat = NewOwnerChar->FindComponentByClass<USLCombatComponent>())
	{
		Combat->Client_OnWeaponEquipped(nullptr, WeaponData);
	}
}

void ASLWeaponBase::ServerDropFromOwner(const FVector& WorldLocation, const FVector& Impulse)
{
	if (!HasAuthority() || !bIsHeld)
	{
		return;
	}

	bIsHeld = false;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(WorldLocation);

	// Re-enable pickup + physics
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetSimulatePhysics(true);

	if (!Impulse.IsNearlyZero())
	{
		Mesh->AddImpulse(Impulse, NAME_None, true);
	}
}

void ASLWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASLWeaponBase, bIsHeld);
}