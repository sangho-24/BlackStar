#include "Utility/BSGameplayTags.h"

namespace BSGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Input_BasicAttack, "Input.BasicAttack");
	UE_DEFINE_GAMEPLAY_TAG(Input_Evade, "Input.Evade");
	UE_DEFINE_GAMEPLAY_TAG(Input_Dash, "Input.Dash");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_BasicAttack, "Ability.Player.BasicAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_BasicAttack, "Ability.Enemy.BasicAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Evade, "Ability.Player.Evade");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Dash, "Ability.Player.Dash");

	UE_DEFINE_GAMEPLAY_TAG(Event_BasicAttack_ComboInput, "Event.BasicAttack.ComboInput");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_WindowOpen, "Event.Combo.WindowOpen");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_WindowClose, "Event.Combo.WindowClose");
	UE_DEFINE_GAMEPLAY_TAG(Event_MeleeTrace_Start, "Event.MeleeTrace.Start");
	UE_DEFINE_GAMEPLAY_TAG(Event_MeleeTrace_End, "Event.MeleeTrace.End");
	UE_DEFINE_GAMEPLAY_TAG(Event_SpawnProjectile, "Event.SpawnProjectile");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Death, "Event.Character.Death");

	UE_DEFINE_GAMEPLAY_TAG(State_Attacking, "State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Combo, "State.Combo");
	UE_DEFINE_GAMEPLAY_TAG(State_FaceTarget, "State.FaceTarget");
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Evading, "State.Evading");
	UE_DEFINE_GAMEPLAY_TAG(State_Dashing, "State.Dashing");
	UE_DEFINE_GAMEPLAY_TAG(State_StaminaRegenDelay, "State.StaminaRegenDelay");
	UE_DEFINE_GAMEPLAY_TAG(State_LockOn, "State.LockOn");
	

	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_StaminaCost, "Data.StaminaCost");
}
