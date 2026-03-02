// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLBaseGameCharacter.h"
#include "SLSurvivorCharacterBase.generated.h"

UCLASS()
class SURVIVORLAND_API ASLSurvivorCharacterBase : public ASLBaseGameCharacter
{
	GENERATED_BODY()

public:

	ASLSurvivorCharacterBase();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	virtual void BeginPlay() override;
	
};
