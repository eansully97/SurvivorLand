// Ean Sullivan All Rights Reserved

#include "SurvivorLandGameplayTags.h"

namespace SurvivorLandGameplayTags
{
	/** Input Tags **/
	UE_DEFINE_GAMEPLAY_TAG(Input_Shared_Move, "Input.Shared.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Shared_Look, "Input.Shared.Look");
	UE_DEFINE_GAMEPLAY_TAG(Input_Shared_Jump, "Input.Shared.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Input_Shared_Interact, "Input.Shared.Interact");
	UE_DEFINE_GAMEPLAY_TAG(Input_Shared_Drop, "Input.Shared.Drop");

	UE_DEFINE_GAMEPLAY_TAG(Input_Survivor_Fire, "Input.Survivor.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Input_Survivor_Aim, "Input.Survivor.Aim");
	UE_DEFINE_GAMEPLAY_TAG(Input_Survivor_Reload, "Input.Survivor.Reload");
	

	UE_DEFINE_GAMEPLAY_TAG(Input_Monster_PrimaryAttack, "Input.Monster.PrimaryAttack");
	UE_DEFINE_GAMEPLAY_TAG(Input_Monster_SecondaryAttack, "Input.Monster.SecondaryAttack");
	UE_DEFINE_GAMEPLAY_TAG(Input_Monster_Utility, "Input.Monster.Utility");
	UE_DEFINE_GAMEPLAY_TAG(Input_Monster_Ultimate, "Input.Monster.Ultimate");

	// Item / Weapon Tags

	UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_Pistol, "Item.Weapon.Pistol");
	UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_Rifle, "Item.Weapon.Rifle");
}