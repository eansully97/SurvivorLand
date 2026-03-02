// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "SLBaseGameCharacter.generated.h"

class USLInputHandlerComponent;
class UDataAsset_InputConfig;

UCLASS()
class SURVIVORLAND_API ASLBaseGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASLBaseGameCharacter();
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	/** Input handler component (unified input pipeline). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Components")
	TObjectPtr<USLInputHandlerComponent> InputHandlerComponent;

	/** Input config to use for this pawn (assign in BP_SurvivorBase / BP_MonsterBase). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SL|Input")
	TObjectPtr<UDataAsset_InputConfig> InputConfig;

private:
	// Handlers bound to the InputHandlerComponent delegates:
	UFUNCTION()
	void HandleAxis2D(FGameplayTag InputTag, FVector2D Value);

	UFUNCTION()
	void HandleActionStarted(FGameplayTag InputTag);

	UFUNCTION()
	void HandleActionCompleted(FGameplayTag InputTag);
};
