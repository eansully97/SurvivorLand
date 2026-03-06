// Ean Sullivan All Rights Reserved

#include "Items/Weapons/SLWeaponBase.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Characters/SLBaseGameCharacter.h"
#include "Characters/SLSurvivorCharacterBase.h"
#include "Components/Combat/SLSurvivorCombatComponent.h"
#include "Data/Weapon/SLWeaponData.h"

ASLWeaponBase::ASLWeaponBase()
{
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));

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

void ASLWeaponBase::ApplyVisualFromDataAsset() const
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

FTransform ASLWeaponBase::GetMuzzleTransform() const
{
	if (!Mesh) return GetActorTransform();

	const FName Socket = GetMuzzleSocketName();
	if (Mesh->DoesSocketExist(Socket))
	{
		return Mesh->GetSocketTransform(Socket, RTS_World);
	}

	return Mesh->GetComponentTransform();
}

void ASLWeaponBase::SetPickupEnabled(bool bEnabled)
{
	if (!PickupSphere) return;

	PickupSphere->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	SetActorEnableCollision(bEnabled);
}

void ASLWeaponBase::SetPhysicsEnabled(const bool bEnabled) const
{
	if (!Mesh) return;

	Mesh->SetSimulatePhysics(bEnabled);
	Mesh->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void ASLWeaponBase::ServerGiveTo(ASLSurvivorCharacterBase* NewOwnerChar)
{
	if (!NewOwnerChar || !WeaponData)
	{
		return;
	}

	const FName AttachSocket = NewOwnerChar->GetWeaponAttachSocket(WeaponData->Grip);
	ServerAttachToOwnerSocket(NewOwnerChar, AttachSocket, true);

	if (USLSurvivorCombatComponent* Combat = NewOwnerChar->FindComponentByClass<USLSurvivorCombatComponent>())
	{
		Combat->Client_ApplyEquippedPresentation(WeaponData);
	}
}

void ASLWeaponBase::ServerDropFromOwner(const FVector& WorldLocation, const FVector& Impulse)
{
	if (!HasAuthority() || !bIsOwnedByPlayer)
	{
		return;
	}

	bIsOwnedByPlayer = false;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(WorldLocation);

	// Re-enable pickup + physics
	SetPhysicsEnabled(true);
	SetPickupEnabled(true);

	if (!Impulse.IsNearlyZero())
	{
		Mesh->AddImpulse(Impulse, NAME_None, true);
	}
}

void ASLWeaponBase::ServerAttachToOwnerSocket(ASLSurvivorCharacterBase* NewOwnerChar, const FName& SocketName, bool bOwnedByPlayer)
{
	if (!HasAuthority() || !NewOwnerChar || !Mesh)
	{
		return;
	}

	bIsOwnedByPlayer = bOwnedByPlayer;

	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (PickupSphere)
	{
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	AttachToComponent(
		NewOwnerChar->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName
	);
}

void ASLWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASLWeaponBase, bIsOwnedByPlayer);
}


