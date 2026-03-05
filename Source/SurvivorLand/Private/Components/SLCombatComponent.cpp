#include "Components/Combat/SLCombatComponent.h"

#include "Characters/SLBaseGameCharacter.h"
#include "Net/UnrealNetwork.h"


USLCombatComponent::USLCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USLCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bAiming)
}

void USLCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningCharacter = Cast<ASLBaseGameCharacter>(GetOwner());
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

	OwnerChar->OnAimingChanged(bAiming);
}