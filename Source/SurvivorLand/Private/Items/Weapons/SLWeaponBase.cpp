// Ean Sullivan All Rights Reserved


#include "Items/Weapons/SLWeaponBase.h"

#include "Characters/SLBaseGameCharacter.h"
#include "Components/SLCombatComponent.h"
#include "Components/SphereComponent.h"
#include "Data/SLWeaponData.h"


ASLWeaponBase::ASLWeaponBase()
{
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	SetRootComponent(PickupSphere);
	PickupSphere->InitSphereRadius(120.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
}

void ASLWeaponBase::ServerGiveTo(ASLBaseGameCharacter* NewOwnerChar)
{
	if (!HasAuthority() || !NewOwnerChar || !WeaponData)
	{
		return;
	}

	if (USLCombatComponent* Combat = NewOwnerChar->FindComponentByClass<USLCombatComponent>())
	{
		Combat->Client_OnWeaponEquipped(WeaponData->WeaponMappingContext, WeaponData);
	}
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetSimulatePhysics(false);

	static const FName RightHandSocketName(TEXT("RightHandSocket"));
	Mesh->AttachToComponent(
		NewOwnerChar->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		RightHandSocketName
	);

	// Optional: reset relative offset (sometimes helps)
	Mesh->SetRelativeLocation(FVector::ZeroVector);
	Mesh->SetRelativeRotation(FRotator::ZeroRotator);

	PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASLWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (WeaponData && WeaponData->WeaponMesh)
	{
		Mesh->SetSkeletalMesh(WeaponData->WeaponMesh);
		Mesh->SetSimulatePhysics(true);
	}
	else
	{
		Mesh->SetSkeletalMesh(nullptr);
	}
}

void ASLWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// Safety for runtime (esp. if WeaponData gets set dynamically)
	if (WeaponData && WeaponData->WeaponMesh && Mesh->GetSkeletalMeshAsset() != WeaponData->WeaponMesh)
	{
		Mesh->SetSkeletalMesh(WeaponData->WeaponMesh);
		Mesh->SetSimulatePhysics(true);
	}
}

void ASLWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}



