// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "InputActionValue.h" // for EInputActionValueType
#include "SLInputDataConfig.generated.h"

class UInputMappingContext;
class UInputAction;

USTRUCT(BlueprintType)
struct FSurvivorLandTaggedInputAction
{
	GENERATED_BODY()

	/** The tag used as the stable ID for this input (e.g. Input.Shared.Move). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Input"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EInputActionValueType ValueType = EInputActionValueType::Boolean;

	bool IsValid() const
	{
		return InputAction != nullptr && InputTag.IsValid();
	}
};

UCLASS(BlueprintType)
class SURVIVORLAND_API UDataAsset_InputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The mapping context applied for this config (Survivor/Monster can have different ones). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;

	/** All actions (buttons + axes) keyed by tag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input", meta=(TitleProperty="InputTag"))
	TArray<FSurvivorLandTaggedInputAction> TaggedInputActions;

	/** Returns the input action for a given tag (nullptr if not found). */
	UFUNCTION(BlueprintCallable, Category="Input")
	UInputAction* FindInputActionByTag(const FGameplayTag& InInputTag) const;
};