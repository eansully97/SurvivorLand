// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Characters/SLBaseGameCharacter.h"


ASLBaseGameCharacter::ASLBaseGameCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASLBaseGameCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASLBaseGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

