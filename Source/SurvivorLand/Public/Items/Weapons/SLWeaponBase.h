// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLWeaponBase.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class USLWeaponDataAsset;
class ASLBaseGameCharacter;

UCLASS()
class SURVIVORLAND_API ASLWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ASLWeaponBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<USLWeaponDataAsset> WeaponData;

	UFUNCTION()
	void ServerGiveTo(class ASLBaseGameCharacter* NewOwnerChar);

	/** Server: Drop weapon into world (detach, enable pickup, enable physics). */
	void ServerDropFromOwner(const FVector& WorldLocation, const FVector& Impulse = FVector::ZeroVector);

	/** True when weapon is currently held/equipped (server authoritative, replicated). */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool IsHeld() const { return bIsHeld; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void ApplyVisualFromDataAsset();

	void SetPickupEnabled(bool bEnabled);
	void SetPhysicsEnabled(bool bEnabled);

	/** Replicated state to prevent double-pickup and support clients. */
	UPROPERTY(Replicated)
	bool bIsHeld = false;
};