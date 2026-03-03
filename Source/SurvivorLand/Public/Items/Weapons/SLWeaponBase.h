// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLWeaponBase.generated.h"

class UInputMappingContext;
class USphereComponent;
class USkeletalMeshComponent;
class USLWeaponDataAsset;

UCLASS()
class SURVIVORLAND_API ASLWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ASLWeaponBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<USLWeaponDataAsset> WeaponData;

	/** Called on server to give this weapon to a character. */
	UFUNCTION()
	void ServerGiveTo(class ASLBaseGameCharacter* NewOwnerChar);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
