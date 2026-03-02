// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLBaseGameCharacter.h"
#include "SLMonsterCharacter.generated.h"

UCLASS()
class SURVIVORLAND_API ASLMonsterCharacter : public ASLBaseGameCharacter
{
	GENERATED_BODY()

public:
	
	ASLMonsterCharacter();
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	virtual void BeginPlay() override;

};
