// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Data/Weapon/SLWeaponData.h"
#include "GameFramework/Actor.h"
#include "SLWeaponBase.generated.h"

class ASLSurvivorCharacterBase;
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

	UFUNCTION()
	void ServerGiveTo(ASLSurvivorCharacterBase* NewOwnerChar);

	UFUNCTION()
	void ServerDropFromOwner(const FVector& WorldLocation, const FVector& Impulse = FVector::ZeroVector);

	void ServerAttachToOwnerSocket(ASLSurvivorCharacterBase* NewOwnerChar, const FName& SocketName, bool bOwnedByPlayer);
	
protected:

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ApplyVisualFromDataAsset() const;
	void SetPickupEnabled(bool bEnabled);
	void SetPhysicsEnabled(bool bEnabled) const;

private:

	UPROPERTY(VisibleAnywhere, Category="Weapon")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, Category="Weapon")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TObjectPtr<USLWeaponDataAsset> WeaponData;
	
	UPROPERTY(Replicated)
	bool bIsOwnedByPlayer = false;

public:
	
	// Getters
	UFUNCTION(BlueprintPure)
	FTransform GetMuzzleTransform() const;

	UFUNCTION(BlueprintPure)
	USkeletalMeshComponent* GetWeaponMesh() const {return Mesh;}
	
	// Inline Getters
	FORCEINLINE FName GetMuzzleSocketName() const {return WeaponData ? WeaponData->Ballistics.MuzzleSocketName : TEXT("Muzzle");}
	FORCEINLINE bool IsHeld() const {return bIsOwnedByPlayer;}
	FORCEINLINE USphereComponent* GetSphereComponent() const {return PickupSphere;}
	FORCEINLINE USLWeaponDataAsset* GetWeaponData() const {return WeaponData;}
};