// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Characters/SLMonsterCharacter.h"


ASLMonsterCharacter::ASLMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASLMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASLMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

