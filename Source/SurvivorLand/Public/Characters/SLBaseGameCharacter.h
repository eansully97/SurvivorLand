// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SLBaseGameCharacter.generated.h"

UCLASS()
class SURVIVORLAND_API ASLBaseGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASLBaseGameCharacter();
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
protected:
	
	virtual void BeginPlay() override;
	
};
