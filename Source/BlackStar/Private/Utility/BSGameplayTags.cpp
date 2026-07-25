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
	
	UE_DEFINE_GAMEPLAY_TAG(HitReaction_Flinch, "HitReaction.Flinch");

	// 노티파이 || 몽타주 델리게이트
	UE_DEFINE_GAMEPLAY_TAG(Event_BasicAttack_ComboInput, "Event.BasicAttack.ComboInput");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_WindowOpen, "Event.Combo.WindowOpen");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_WindowClose, "Event.Combo.WindowClose");
	UE_DEFINE_GAMEPLAY_TAG(Event_MeleeTrace_Start, "Event.MeleeTrace.Start");
	UE_DEFINE_GAMEPLAY_TAG(Event_MeleeTrace_End, "Event.MeleeTrace.End");
	UE_DEFINE_GAMEPLAY_TAG(Event_SpawnProjectile, "Event.SpawnProjectile");
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReaction_Begin, "Event.HitReaction.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReaction_End, "Event.HitReaction.End");
	// ST트리 보낼거
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_Death, "Event.Character.Death");
	UE_DEFINE_GAMEPLAY_TAG(Event_Character_HitReaction, "Event.Character.HitReaction");
	UE_DEFINE_GAMEPLAY_TAG(Event_AI_StimulusLocationUpdated, "Event.AI.StimulusLocationUpdated");

	// 데미지 타입
	UE_DEFINE_GAMEPLAY_TAG(DamageType_Melee, "DamageType.Melee");
	UE_DEFINE_GAMEPLAY_TAG(DamageType_Fire, "DamageType.Fire");
	UE_DEFINE_GAMEPLAY_TAG(DamageType_Ice, "DamageType.Ice");
	
	// 면역 상태
	UE_DEFINE_GAMEPLAY_TAG(State_Invulnerable, "State.Invulnerable");
	UE_DEFINE_GAMEPLAY_TAG(State_Immune_Melee, "State.Immune.Melee");
	UE_DEFINE_GAMEPLAY_TAG(State_Immune_Fire, "State.Immune.Fire");
	UE_DEFINE_GAMEPLAY_TAG(State_Immune_Ice, "State.Immune.Ice");
	// 행동 상태
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking, "State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Combo, "State.Combo");
	UE_DEFINE_GAMEPLAY_TAG(State_FaceTarget, "State.FaceTarget");
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Evading, "State.Evading");
	UE_DEFINE_GAMEPLAY_TAG(State_Dashing, "State.Dashing");
	UE_DEFINE_GAMEPLAY_TAG(State_StaminaRegenDelay, "State.StaminaRegenDelay");
	UE_DEFINE_GAMEPLAY_TAG(State_LockOn, "State.LockOn");
	UE_DEFINE_GAMEPLAY_TAG(State_SuperArmor, "State.SuperArmor");
	UE_DEFINE_GAMEPLAY_TAG(State_HitReacting, "State.HitReacting");
	UE_DEFINE_GAMEPLAY_TAG(State_Block_Move, "State.Block.Move");
	
	// GE용 데이터
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_StaminaCost, "Data.StaminaCost");
}
