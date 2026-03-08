#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/Weapon/SLWeaponData.h"
#include "SLWeaponBase.generated.h"

class ASLSurvivorCharacterBase;
class USphereComponent;
class USkeletalMeshComponent;
class USLWeaponDataAsset;

UCLASS()
class SURVIVORLAND_API ASLWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ASLWeaponBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/* -----------------------------
	 * Ownership / attachment
	 * ----------------------------- */

	void AttachToCharacter(ASLSurvivorCharacterBase* NewOwnerChar, const FName& SocketName, bool bInOwnedByPlayer);
	void DropFromOwner(const FVector& WorldLocation, const FVector& Impulse = FVector::ZeroVector);

	/* -----------------------------
	 * Ammo
	 * ----------------------------- */

	bool IsFull() const;
	bool IsEmpty() const;

	void SpendRound();
	void SetCurrentAmmoInMag(int32 NewAmount);
	void AddAmmoToMag(int32 AmmoToAdd);
	void FillMagazine();

	/* -----------------------------
	 * Pickup / transforms
	 * ----------------------------- */

	EAmmoType GetAmmoType() const;
	UFUNCTION(BlueprintPure)
	FTransform GetMuzzleTransform() const;

	UFUNCTION(BlueprintPure, Category="SL|Weapon")
	bool IsHeld() const { return bIsOwnedByPlayer; }

	UFUNCTION(BlueprintPure, Category="SL|Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const { return Mesh; }

	UFUNCTION(BlueprintPure, Category="SL|Weapon")
	USphereComponent* GetSphereComponent() const { return PickupSphere; }

	UFUNCTION(BlueprintPure, Category="SL|Weapon")
	USLWeaponDataAsset* GetWeaponData() const { return WeaponData; }

	UFUNCTION(BlueprintPure, Category="SL|Weapon")
	int32 GetCurrentAmmoInMag() const { return CurrentAmmoInMag; }

	FORCEINLINE FName GetMuzzleSocketName() const
	{
		return WeaponData ? WeaponData->SocketInformation.MuzzleSocketName : TEXT("Muzzle");
	}

protected:
	void ApplyVisualFromDataAsset() const;
	void SetPickupEnabled(bool bEnabled);
	void SetPhysicsEnabled(bool bEnabled) const;
	void InitializeMagazineAmmo();

	UFUNCTION()
	void OnRep_CurrentAmmoInMag();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Weapon", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Weapon", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SL|Weapon", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USLWeaponDataAsset> WeaponData;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentAmmoInMag, VisibleInstanceOnly, Category="SL|Weapon", meta=(AllowPrivateAccess="true"))
	int32 CurrentAmmoInMag = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category="SL|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsOwnedByPlayer = false;
};