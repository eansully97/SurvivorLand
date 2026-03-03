// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Data/SLWeaponData.h"
#include "SLCombatComponent.generated.h"

class USLInputHandlerComponent;
class UInputMappingContext;
class UDataAsset_InputConfig;

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class SURVIVORLAND_API USLCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USLCombatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_TryPickupWeapon();

	void TryPickupWeapon_Internal();

	/** Bind to the input handler (call from BeginPlay of character, once). */
	void BindToInput(USLInputHandlerComponent* InputHandler);

	/** Adds a weapon mapping context on top of IMC_Common. */
	UFUNCTION(BlueprintCallable, Category="SL|Combat")
	void EquipWeaponContext(class APlayerController* PC, UInputMappingContext* WeaponContext, int32 Priority = 1);

	UFUNCTION(BlueprintCallable, Category="SL|Combat")
	void UnequipWeaponContext(class APlayerController* PC);

	UFUNCTION(Client, Reliable)
	void Client_OnWeaponEquipped(UInputMappingContext* WeaponContext, const USLWeaponDataAsset* WeaponData);

private:
	UFUNCTION()
	void OnActionStarted(FGameplayTag InputTag);

	UFUNCTION()
	void OnActionCompleted(FGameplayTag InputTag);

	UFUNCTION()
	void OnAxis2D(FGameplayTag InputTag, FVector2D Value);

	UPROPERTY()
	TObjectPtr<UInputMappingContext> EquippedWeaponContext = nullptr;

	bool bBoundToInput = false;
};