// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Characters/SLMonsterCharacter.h"

#include "Components/Combat/SLMonsterCombatComponent.h"


ASLMonsterCharacter::ASLMonsterCharacter()
{
	MonsterCombatComponent = CreateDefaultSubobject<USLMonsterCombatComponent>(TEXT("CombatComponent"));
	CombatComponent = MonsterCombatComponent;
}

void ASLMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASLMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

