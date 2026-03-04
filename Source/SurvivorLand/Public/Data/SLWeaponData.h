// SLWeaponDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SLInputDataConfig.h"
#include "SLWeaponData.generated.h"

class UInputMappingContext;
class USkeletalMesh;
class UANimInstance;

UENUM(BlueprintType)
enum class ESLWeaponGrip : uint8
{
	Pistol UMETA(DisplayName="Pistol"),
	Rifle UMETA(DisplayName="Rifle")
};

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Animation|Layers")
	TSubclassOf<UAnimInstance> SurvivorUpperBodyLayerClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Input", meta=(TitleProperty="InputTag"))
	TArray<FSurvivorLandTaggedInputAction> GrantedInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	ESLWeaponGrip Grip{};
};
