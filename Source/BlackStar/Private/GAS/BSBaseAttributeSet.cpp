
#include "GAS/BSBaseAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Interface/ICombatInterface.h"
#include "Perception/AISense_Damage.h"

UBSBaseAttributeSet::UBSBaseAttributeSet()
{
	InitMaxHP(100.0f);
	InitCurrentHP(100.0f);
	InitMaxMP(100.0f);
	InitCurrentMP(100.0f);
	InitMaxStamina(100.0f);
	InitStaminaRegenRate(3.0f);
	InitCurrentStamina(100.0f);
	InitMagicPower(0.0f);
	InitAttackPower(0.0f);
	InitDefense(0.0f);
	InitDamageReduction(0.0f);
	InitIncomingDamage(0.0f);
}

void UBSBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetCurrentHPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
	}
	else if (Attribute == GetCurrentMPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMP());
	}
	else if (Attribute == GetCurrentStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetStaminaRegenRateAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetMaxHPAttribute() || Attribute == GetMaxMPAttribute() || Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetDamageReductionAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 100.0f);
	}
}

void UBSBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float Damage = FMath::Max(GetIncomingDamage(), 0.0f);
		// 임시 값을 먼저 비움.
		SetIncomingDamage(0.0f); 

		AActor* TargetActor = Data.Target.GetAvatarActor();
		ICombatInterface* CombatTarget = Cast<ICombatInterface>(TargetActor);

		if (!CombatTarget || CombatTarget->IsDead() || Damage <= 0.0f)
		{
			return;
		}

		SetCurrentHP(FMath::Clamp(GetCurrentHP() - Damage,0.0f, GetMaxHP()));

		// 피해 퍼셉션 (밀리+투사체 등 모든 피해 공통)
		const FGameplayEffectContextHandle& Context =
			Data.EffectSpec.GetContext();

		AActor* Attacker = Context.GetOriginalInstigator();

		if (IsValid(Attacker))
		{
			const FHitResult* Hit = Context.GetHitResult();
			const FVector HitLocation = Hit ? FVector(Hit->ImpactPoint) : TargetActor->GetActorLocation();
			UAISense_Damage::ReportDamageEvent(TargetActor, TargetActor, Attacker,
				Damage, Attacker->GetActorLocation(), HitLocation);
		}

		// 방어력을 반영한 피해를 표시.
		CombatTarget->SpawnFloatingDamage(Damage, false, false);

		if (GetCurrentHP() <= 0.0f)
		{
			CombatTarget->Death(Data.EffectSpec.GetContext().GetOriginalInstigator());
		}

		return;
	}
	
	if (Data.EvaluatedData.Attribute == GetCurrentHPAttribute())
	{
		SetCurrentHP(FMath::Clamp(GetCurrentHP(), 0.0f, GetMaxHP()));
		
		AActor* TargetActor = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid())
		{
			TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		}
		ICombatInterface* CombatTarget = Cast<ICombatInterface>(TargetActor);
		if (!CombatTarget)
		{
			return;
		}
		
		const float DamageMagnitude = Data.EvaluatedData.Magnitude;
		if (DamageMagnitude < 0.0f) // 데미지
		{
			const float DamageAmount = FMath::Abs(DamageMagnitude);

			CombatTarget->SpawnFloatingDamage(DamageAmount,false,false);

			if (GetCurrentHP() <= 0.0f)
			{
				AActor* Killer = Data.EffectSpec.GetContext().GetOriginalInstigator();
				CombatTarget->Death(Killer);
			}
		}
		else if (DamageMagnitude > 0.0f) // 힐
		{
			CombatTarget->SpawnFloatingDamage(DamageMagnitude,true,false);
		}
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentMPAttribute())
	{
		SetCurrentMP(FMath::Clamp(GetCurrentMP(), 0.0f, GetMaxMP()));
	}
	if (Data.EvaluatedData.Attribute == GetCurrentStaminaAttribute())
	{
		SetCurrentStamina(FMath::Clamp(GetCurrentStamina(), 0.0f, GetMaxStamina()));
	}
}

void UBSBaseAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHPAttribute())
	{
		SetCurrentHP(FMath::Clamp(GetCurrentHP(), 0.0f, GetMaxHP()));
	}
	else if (Attribute == GetMaxMPAttribute())
	{
		SetCurrentMP(FMath::Clamp(GetCurrentMP(), 0.0f, GetMaxMP()));
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		SetCurrentStamina(FMath::Clamp(GetCurrentStamina(), 0.0f, GetMaxStamina()));
	}
}
