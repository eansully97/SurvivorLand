// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Characters/SLSurvivorCharacterBase.h"


ASLSurvivorCharacterBase::ASLSurvivorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}


void ASLSurvivorCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASLSurvivorCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}