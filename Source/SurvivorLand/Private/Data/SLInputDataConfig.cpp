// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Data/SLInputDataConfig.h"
#include "InputAction.h" // for UInputAction

UInputAction* UDataAsset_InputConfig::FindInputActionByTag(const FGameplayTag& InInputTag) const
{
	if (!InInputTag.IsValid())
	{
		return nullptr;
	}

	for (const FSurvivorLandTaggedInputAction& Action : TaggedInputActions)
	{
		if (Action.InputTag == InInputTag)
		{
			return Action.InputAction;
		}
	}

	return nullptr;
}