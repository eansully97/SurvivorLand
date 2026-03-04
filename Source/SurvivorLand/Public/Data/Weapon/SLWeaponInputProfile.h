#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/SLInputDataConfig.h"
#include "SLWeaponInputProfile.generated.h"

class UInputMappingContext;

UCLASS(BlueprintType)
class SURVIVORLAND_API USLWeaponInputProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> MappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input", meta=(TitleProperty="InputTag"))
	TArray<FSurvivorLandTaggedInputAction> GrantedInputActions;
};