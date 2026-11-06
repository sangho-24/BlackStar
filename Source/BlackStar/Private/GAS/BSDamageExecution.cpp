#include "GAS/BSDamageExecution.h"

#include "GAS/BSBaseAttributeSet.h"
#include "Utility/BSCombatRules.h"
#include "Utility/BSGameplayTags.h"

namespace
{
    struct FBSDamageCaptures
    {
        DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);
        DECLARE_ATTRIBUTE_CAPTUREDEF(DamageReduction);

        FBSDamageCaptures()
        {
            DEFINE_ATTRIBUTE_CAPTUREDEF(UBSBaseAttributeSet, Defense, Target, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UBSBaseAttributeSet, DamageReduction, Target, false);
        }
    };

    const FBSDamageCaptures& DamageCaptures()
    {
        static const FBSDamageCaptures Captures;
        return Captures;
    }
}

UBSDamageExecution::UBSDamageExecution()
{
    RelevantAttributesToCapture.Add(DamageCaptures().DefenseDef);
    RelevantAttributesToCapture.Add(DamageCaptures().DamageReductionDef);
}

void UBSDamageExecution::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

    FGameplayTagContainer AssetTags;
    Spec.GetAllAssetTags(AssetTags);

    // 타입이 없어도 무적/사망은 검사.
    if (!BSCombatRules::CanReceiveDamage(TargetASC, FGameplayTag()))
    {
        return;
    }

    // 여러 피해 타입 중 하나라도 면역이면 전체 피해 차단. (화염 + 물리 -> 화염면역 or 물리면역 = 데미지 차단)
    for (const FGameplayTag& Tag : AssetTags)
    {
        if (!BSCombatRules::CanReceiveDamage(TargetASC, Tag))
        {
            return;
        }
    }

    const float RawDamage = FMath::Max(0.0f, Spec.GetSetByCallerMagnitude(BSGameplayTags::Data_Damage, false, 0.0f));

    // 태그를 가져와 조건부 버프를 적용할 수 있게 된다.
    FAggregatorEvaluateParameters EvalParams;
    EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    float Defense = 0.0f;
    float DamageReduction = 0.0f;

    if (!ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCaptures().DefenseDef, EvalParams, Defense) 
        || !ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCaptures().DamageReductionDef, EvalParams, DamageReduction))
    {
        return;
    }

    Defense = FMath::Max(Defense, 0.0f);
    DamageReduction = FMath::Clamp(DamageReduction, 0.0f, 100.0f);

    const float FinalDamage = RawDamage * (100.0f / (100.0f + Defense)) * (1.0f - DamageReduction / 100);

    if (FinalDamage <= 0.0f)
    {
        return;
    }

    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(UBSBaseAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
}