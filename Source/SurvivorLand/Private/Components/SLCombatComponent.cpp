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
}

void USLCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningCharacter = Cast<ASLBaseGameCharacter>(GetOwner());
}