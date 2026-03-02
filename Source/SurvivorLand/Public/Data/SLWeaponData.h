// SLWeaponDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SLWeaponData.generated.h"

class UInputMappingContext;
class USkeletalMesh;

UCLASS(BlueprintType)
class SURVIVORLAND_API USLWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Input")
	TObjectPtr<UInputMappingContext> WeaponMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Visual")
	TObjectPtr<USkeletalMesh> WeaponMesh = nullptr;
};
