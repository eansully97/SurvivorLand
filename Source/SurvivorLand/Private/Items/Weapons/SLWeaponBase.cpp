#include "Items/Weapons/SLWeaponBase.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Characters/SLSurvivorCharacterBase.h"

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

void ASLWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeMagazineAmmo();
}

void ASLWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyVisualFromDataAsset();
}

void ASLWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentAmmoInMag);
	DOREPLIFETIME(ThisClass, bIsOwnedByPlayer);
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

void ASLWeaponBase::InitializeMagazineAmmo()
{
	if (!HasAuthority() || !WeaponData)
	{
		return;
	}

	// Only initialize if it has not been set yet.
	if (CurrentAmmoInMag <= 0)
	{
		CurrentAmmoInMag = WeaponData->FireSettings.MagazineSize;
	}
}

void ASLWeaponBase::OnRep_CurrentAmmoInMag()
{
	// Hook for UI / cosmetic refresh later if desired.
}

void ASLWeaponBase::SetPickupEnabled(bool bEnabled)
{
	if (!PickupSphere)
	{
		return;
	}

	PickupSphere->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void ASLWeaponBase::SetPhysicsEnabled(bool bEnabled) const
{
	if (!Mesh)
	{
		return;
	}

	Mesh->SetSimulatePhysics(bEnabled);
	Mesh->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void ASLWeaponBase::AttachToCharacter(ASLSurvivorCharacterBase* NewOwnerChar, const FName& SocketName, bool bInOwnedByPlayer)
{
	if (!HasAuthority() || !NewOwnerChar || !Mesh)
	{
		return;
	}

	bIsOwnedByPlayer = bInOwnedByPlayer;

	SetOwner(NewOwnerChar);
	SetPhysicsEnabled(false);
	SetPickupEnabled(false);

	AttachToComponent(
		NewOwnerChar->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName
	);
}

void ASLWeaponBase::DropFromOwner(const FVector& WorldLocation, const FVector& Impulse)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsOwnedByPlayer = false;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
	SetActorLocation(WorldLocation);

	SetPhysicsEnabled(true);
	SetPickupEnabled(true);

	if (Mesh && !Impulse.IsNearlyZero())
	{
		Mesh->AddImpulse(Impulse, NAME_None, true);
	}
}

bool ASLWeaponBase::IsFull() const
{
	return WeaponData && CurrentAmmoInMag >= WeaponData->FireSettings.MagazineSize;
}

bool ASLWeaponBase::IsEmpty() const
{
	return CurrentAmmoInMag <= 0;
}

void ASLWeaponBase::SpendRound()
{
	CurrentAmmoInMag = FMath::Max(0, CurrentAmmoInMag - 1);
}

void ASLWeaponBase::SetCurrentAmmoInMag(int32 NewAmount)
{
	if (!WeaponData)
	{
		CurrentAmmoInMag = FMath::Max(0, NewAmount);
		return;
	}

	CurrentAmmoInMag = FMath::Clamp(NewAmount, 0, WeaponData->FireSettings.MagazineSize);
}

void ASLWeaponBase::AddAmmoToMag(int32 AmmoToAdd)
{
	if (!WeaponData || AmmoToAdd <= 0)
	{
		return;
	}

	CurrentAmmoInMag = FMath::Clamp(
		CurrentAmmoInMag + AmmoToAdd,
		0,
		WeaponData->FireSettings.MagazineSize
	);
}

void ASLWeaponBase::FillMagazine()
{
	if (!WeaponData)
	{
		return;
	}

	CurrentAmmoInMag = WeaponData->FireSettings.MagazineSize;
}

EAmmoType ASLWeaponBase::GetAmmoType() const
{
	return WeaponData ? WeaponData->FireSettings.AmmoType : EAmmoType::Small;
}

FTransform ASLWeaponBase::GetMuzzleTransform() const
{
	if (!Mesh)
	{
		return GetActorTransform();
	}

	const FName SocketName = GetMuzzleSocketName();
	if (Mesh->DoesSocketExist(SocketName))
	{
		return Mesh->GetSocketTransform(SocketName, RTS_World);
	}

	return Mesh->GetComponentTransform();
}