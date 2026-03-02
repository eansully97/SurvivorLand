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
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASLWeaponBase::ServerGiveTo(ASLBaseGameCharacter* NewOwnerChar)
{
	if (!HasAuthority() || !NewOwnerChar || !WeaponData)
	{
		return;
	}

	// Tell combat to equip input context (stateful input)
	if (USLCombatComponent* Combat = NewOwnerChar->FindComponentByClass<USLCombatComponent>())
	{
		if (APlayerController* PC = Cast<APlayerController>(NewOwnerChar->GetController()))
		{
			Combat->EquipWeaponContext(PC, WeaponData->WeaponMappingContext, /*Priority*/ 1);
		}
	}

	// Optional: attach the weapon mesh to the character (cosmetic)
	// You can pick a socket later ("hand_rSocket" etc)
	AttachToComponent(NewOwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);

	// Disable pickup collision & hide in world
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ASLWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

