// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "SLBaseGameCharacter.generated.h"

class ASLWeaponBase;
class USLCombatComponent;
class UCameraComponent;
class USpringArmComponent;
class USLInputHandlerComponent;
class UDataAsset_InputConfig;

UCLASS()
class SURVIVORLAND_API ASLBaseGameCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	
	ASLBaseGameCharacter();

	
#pragma region Inputs
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SL|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataAsset_InputConfig> InputConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterData", meta = (AllowPrivateAccess = "true"))
	float LookSensitivity = 0.3f;

#pragma endregion

#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Input handler component (unified input pipeline). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USLInputHandlerComponent> InputHandlerComponent;


#pragma endregion

protected:
	
	virtual void BeginPlay() override;
	void BindToInput(USLInputHandlerComponent* InputHandler);
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	
public:


	// Handlers bound to the InputHandlerComponent delegates:
	bool bInputBound = false;
	
	UFUNCTION()
	virtual void HandleAxis2D(FGameplayTag InputTag, FVector2D Value);

	UFUNCTION()
	virtual void HandleActionStarted(FGameplayTag InputTag);

	UFUNCTION()
	virtual void HandleActionCompleted(FGameplayTag InputTag);
};
