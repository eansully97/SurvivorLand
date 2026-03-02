// Ean Sullivan All Rights Reserved

#pragma once

#include "NativeGameplayTags.h"

/**
 * Native gameplay tags for SurvivorLand.
 * Keep these stable: treat them like API endpoints.
 */
namespace SurvivorLandGameplayTags
{
	/** Input Tags (unified, scalable) **/
	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Shared_Move);
	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Shared_Look);
	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Shared_Jump);
	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Shared_Interact);

	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Survivor_Fire);
	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Survivor_Aim);
	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Survivor_Reload);

	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Monster_PrimaryAttack);
	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Monster_SecondaryAttack);
	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Monster_Utility);
	SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Monster_Ultimate);

	// Later expansion (not needed now)
	// SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_...);
	// SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_...);
	// SURVIVORLAND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_...);
}