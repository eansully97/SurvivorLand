#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLWeaponAnimProfile.generated.h"

class UAnimInstance;

USTRUCT(BlueprintType)
struct FSLLeftHandIKConfig
{
	GENERATED_BODY()

	// Socket on the WEAPON skeletal mesh for where the left hand should go
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="IK")
	FName LeftHandSocketOnWeapon = TEXT("IK_LeftHand");

	// You can tune this per weapon (rifle = 1, pistol = 0 or small)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="IK", meta=(ClampMin="0.0", ClampMax="1.0"))
	float IKAlpha = 1.f;
};

UCLASS(BlueprintType)
class SURVIVORLAND_API USLWeaponAnimProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|Layers")
	TSubclassOf<UAnimInstance> SurvivorUpperBodyLayerClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|IK")
	FSLLeftHandIKConfig LeftHandIK;
};