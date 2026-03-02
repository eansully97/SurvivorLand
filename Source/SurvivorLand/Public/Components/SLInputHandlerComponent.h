// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SLInputHandlerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SURVIVORLAND_API USLInputHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	USLInputHandlerComponent();

protected:

	virtual void BeginPlay() override;

};
