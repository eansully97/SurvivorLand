// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivorLandGameplayTags.h"
#include "SLCombatComponent.generated.h"


class ASLBaseGameCharacter;

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class SURVIVORLAND_API USLCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USLCombatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void HandleActionStarted(FGameplayTag InputTag){}
	virtual void HandleActionCompleted(FGameplayTag InputTag){}

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<ASLBaseGameCharacter> OwningCharacter;
	
};