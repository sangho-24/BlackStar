#pragma once

#include "NativeGameplayTags.h"

namespace BSGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_BasicAttack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_BasicAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_BasicAttack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_BasicAttack_ComboInput);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_WindowOpen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_WindowClose);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_MeleeTrace_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_MeleeTrace_End);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_SpawnProjectile);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combo);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_FaceTarget);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
}
