#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SLWeaponData.generated.h"

class USkeletalMesh;
class USLWeaponInputProfile;
class USLWeaponAnimProfile;

UENUM(BlueprintType)
enum class ESLWeaponGrip : uint8
{
	Pistol UMETA(DisplayName="Pistol"),
	Rifle  UMETA(DisplayName="Rifle")
};

UCLASS(BlueprintType)
class SURVIVORLAND_API USLWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Attach")
	ESLWeaponGrip Grip = ESLWeaponGrip::Pistol;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Visual")
	TObjectPtr<USkeletalMesh> WeaponMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Profiles")
	TObjectPtr<USLWeaponInputProfile> InputProfile = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Profiles")
	TObjectPtr<USLWeaponAnimProfile> AnimProfile = nullptr;
};
