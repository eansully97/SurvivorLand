// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLCombatComponent.generated.h"


class ASLBaseGameCharacter;

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class SURVIVORLAND_API USLCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USLCombatComponent();
	
};