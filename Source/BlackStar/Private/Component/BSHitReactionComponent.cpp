#include "Component/BSHitReactionComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character/BSBaseCharacter.h"
#include "Character/BSEnemyCharacter.h"
#include "Utility/BSGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Controller/BSAIController.h"
#include "Engine/Engine.h"

UBSHitReactionComponent::UBSHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	ImmunityStateTagsByDamageType.Add(BSGameplayTags::DamageType_Fire, BSGameplayTags::State_Immune_Fire);
	ImmunityStateTagsByDamageType.Add(BSGameplayTags::DamageType_Ice, BSGameplayTags::State_Immune_Ice);
	ImmunityStateTagsByDamageType.Add(BSGameplayTags::DamageType_Melee, BSGameplayTags::State_Immune_Melee);
}



void UBSHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ABSBaseCharacter>(GetOwner());
	if (IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		OwnerASC = ASCOwner->GetAbilitySystemComponent();
	}
}


void UBSHitReactionComponent::ApplyHitReaction(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData)
{
	// 무적, 면역 상태가 Cue 재생을 차단
	if (CanPlayTargetHitCue(ReactionData))
	{
		PlayReactionCue(GetReactionCueTag(ReactionData.DamageTypeTag), Attacker, HitResult);
	}
	// 무적, 면역 + 슈퍼아머, 죽음 상태까지 리액션 차단 가능
	if (!CanApplyGameplayReaction(ReactionData))
	{
		return;
	}
	
	// 물리 반응
	switch (ReactionData.ReactionType)
	{
	case EBSHitReactionType::None:
		break;
	case EBSHitReactionType::Flinch:
		ApplyFlinch(Attacker, HitResult, ReactionData);
		break;
	case EBSHitReactionType::Knockback:
		ApplyKnockback(Attacker, HitResult, ReactionData);
		CancelAbilitiesForReaction(ReactionData);
		break;
	case EBSHitReactionType::Launch:
		ApplyLaunch(Attacker, HitResult, ReactionData);
		CancelAbilitiesForReaction(ReactionData);
		break;
	default:
		break;
	}
}

bool UBSHitReactionComponent::CanPlayTargetHitCue(const FBSHitReactionData& ReactionData) const
{
	if (!OwnerCharacter || !OwnerASC)
	{
		return false;
	}
	// 무적 상태면 Cue도 안나옴(항상 고정) + 이뮨 상태여도 Cue 안나옴
	return !OwnerASC->HasMatchingGameplayTag(BSGameplayTags::State_Invulnerable) && !HasDamageTypeImmunity(ReactionData.DamageTypeTag);
}

bool UBSHitReactionComponent::CanApplyGameplayReaction(const FBSHitReactionData& ReactionData) const
{
	if (!CanPlayTargetHitCue(ReactionData))
	{
		return false;
	}
	// 리액션만 면역인 고정 상태들 (Cue는 재생됨)
	if (OwnerASC->HasMatchingGameplayTag(BSGameplayTags::State_SuperArmor))
	{
		return false;
	}
	if (OwnerASC->HasMatchingGameplayTag(BSGameplayTags::State_Dead))
	{
		return false;
	}
	return true;
}

bool UBSHitReactionComponent::HasDamageTypeImmunity(const FGameplayTag& DamageTypeTag) const
{
	if (!OwnerASC)
	{
		return false;
	}

	for (FGameplayTag LookupTag = DamageTypeTag ; LookupTag.IsValid() ; LookupTag = LookupTag.RequestDirectParent())
	{
		if (const FGameplayTag* ImmunityTag = ImmunityStateTagsByDamageType.Find(LookupTag))
		{
			return OwnerASC->HasMatchingGameplayTag(*ImmunityTag);
		}
	}
	return false;
}

void UBSHitReactionComponent::ApplyFlinch(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData)
{
	// 리액션 중에는 중복적용안됨
	if (OwnerASC->HasMatchingGameplayTag(BSGameplayTags::State_HitReacting))
	{
		return;
	}
	const EHitReactionDirection Direction = GetReactionMontageDirection(Attacker, HitResult);
	if (!PlayReactionMontage(EBSHitReactionType::Flinch, Direction))
	{
		return;
	}
	OwnerCharacter->StopTurning();
	BeginReactionMontage();
}

void UBSHitReactionComponent::ApplyKnockback(AActor* Attacker, const FHitResult& HitResult,
	const FBSHitReactionData& ReactionData)
{
	if (ReactionData.KnockbackStrength <= 0.0f)
	{
		return;
	}

	const FVector Direction = GetReactionDirection(Attacker, HitResult);
	if (Direction.IsNearlyZero())
	{
		return;
	}
	OwnerCharacter->LaunchCharacter(Direction * ReactionData.KnockbackStrength,true,false);
}

void UBSHitReactionComponent::ApplyLaunch(AActor* Attacker, const FHitResult& HitResult,
	const FBSHitReactionData& ReactionData)
{
	if (ReactionData.KnockbackStrength <= 0.0f)
	{
		return;
	}

	const FVector Direction = GetReactionDirection(Attacker, HitResult);
	if (Direction.IsNearlyZero())
	{
		return;
	}
	
	OwnerCharacter->LaunchCharacter((Direction + FVector(0,0,0.5)) * ReactionData.KnockbackStrength,true,false);
}

FVector UBSHitReactionComponent::GetReactionDirection(AActor* Attacker, const FHitResult& HitResult) const
{
	if (!OwnerCharacter)
	{
		return FVector::ZeroVector;
	}

	FVector Direction = FVector::ZeroVector;

	if (Attacker)
	{
		Direction = OwnerCharacter->GetActorLocation() - Attacker->GetActorLocation();
	}
	else if (!HitResult.ImpactNormal.IsNearlyZero())
	{
		Direction = HitResult.ImpactNormal;
	}
	else
	{
		Direction = -OwnerCharacter->GetActorForwardVector();
	}

	Direction.Z = 0.0f;
	return Direction.GetSafeNormal();
}

void UBSHitReactionComponent::CancelAbilitiesForReaction(const FBSHitReactionData& ReactionData)
{
	if (!OwnerASC)
	{
		return;
	}
	OwnerASC->CancelAbilities();
}

EHitReactionDirection UBSHitReactionComponent::GetReactionMontageDirection(AActor* Attacker, const FHitResult& HitResult) const
{
	FVector SourceDirection = FVector::ZeroVector;

	if (Attacker)
	{
		SourceDirection = Attacker->GetActorLocation() - OwnerCharacter->GetActorLocation();
	}
	else
	{
		// ImpactNormal은 대체로 맞은 표면에서 공격자 쪽을 향한다.
		SourceDirection = HitResult.ImpactNormal;
	}

	SourceDirection.Z = 0.0f;
	SourceDirection.Normalize();

	const float ForwardDot = FVector::DotProduct(OwnerCharacter->GetActorForwardVector(), SourceDirection);

	const float RightDot = FVector::DotProduct(OwnerCharacter->GetActorRightVector(), SourceDirection);

	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		return ForwardDot >= 0.0f ? EHitReactionDirection::Front : EHitReactionDirection::Back;
	}

	return RightDot >= 0.0f ? EHitReactionDirection::Right : EHitReactionDirection::Left;
}

FName UBSHitReactionComponent::GetReactionMontageSectionName(EHitReactionDirection Direction) const
{
	switch (Direction)
	{
	case EHitReactionDirection::Front:
		return TEXT("Front");

	case EHitReactionDirection::Back:
		return TEXT("Back");

	case EHitReactionDirection::Left:
		return TEXT("Left");

	case EHitReactionDirection::Right:
		return TEXT("Right");

	default:
		return TEXT("Front");
	}
}

FGameplayTag UBSHitReactionComponent::GetReactionMontageTag(EBSHitReactionType ReactionType) const
{
	switch (ReactionType)
	{
	case EBSHitReactionType::Flinch:
		return BSGameplayTags::HitReaction_Flinch;

	default:
		return FGameplayTag();
	}
}

bool UBSHitReactionComponent::PlayReactionMontage(EBSHitReactionType ReactionType, EHitReactionDirection Direction)
{
	if (!OwnerCharacter)
	{
		return false;
	}

	const FGameplayTag MontageTag = GetReactionMontageTag(ReactionType);
	if (!MontageTag.IsValid())
	{
		return false;
	}

	IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(OwnerCharacter);
	if (!AnimChar)
	{
		return false;
	}

	const FAbilitySkillData SkillData = AnimChar->GetSkillDataForAbility(MontageTag);
	if (!SkillData.Montage)
	{
		return false;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}
	
	if (AnimInstance->Montage_Play(SkillData.Montage) <= 0.0f)
	{
		return false;
	}

	AnimInstance->Montage_JumpToSection(GetReactionMontageSectionName(Direction),SkillData.Montage);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UBSHitReactionComponent::EndReactionMontage);
	AnimInstance->Montage_SetEndDelegate(EndDelegate,SkillData.Montage);

	return true;
}

void UBSHitReactionComponent::BeginReactionMontage()
{
	if (!OwnerASC)
	{
		return;
	}
	OwnerASC->AddLooseGameplayTag(BSGameplayTags::State_HitReacting);
	OwnerASC->AddLooseGameplayTag(BSGameplayTags::State_Block_Move);

	// if (ABSAIController* AIController = Cast<ABSAIController>(OwnerCharacter->GetController()))
	// {
	// 	AIController->StopMovement();
	// 	AIController->SendStateTreeEvent(BSGameplayTags::Event_HitReaction_Begin);
	// }
}

void UBSHitReactionComponent::EndReactionMontage(UAnimMontage* Montage, bool bInterrupted)
{
	if (!OwnerASC)
	{
		return;
	}

	OwnerASC->RemoveLooseGameplayTag(BSGameplayTags::State_HitReacting);
	OwnerASC->RemoveLooseGameplayTag(BSGameplayTags::State_Block_Move);

	// if (ABSAIController* AIController = Cast<ABSAIController>(OwnerCharacter->GetController()))
	// {
	// 	AIController->SendStateTreeEvent(BSGameplayTags::Event_HitReaction_End);
	// }
}


void UBSHitReactionComponent::PlayReactionCue(FGameplayTag CueTag, AActor* Attacker, const FHitResult& HitResult) const
{
	if (!OwnerASC || !CueTag.IsValid())
	{
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.Location = HitResult.ImpactPoint;
	CueParameters.Normal = HitResult.ImpactNormal;
	CueParameters.Instigator = Attacker;
	CueParameters.EffectCauser = Attacker;
	CueParameters.SourceObject = Attacker;
	CueParameters.PhysicalMaterial = HitResult.PhysMaterial;

	OwnerASC->ExecuteGameplayCue(CueTag, CueParameters);
}

FGameplayTag UBSHitReactionComponent::GetReactionCueTag(const FGameplayTag DamageTypeTag) const
{
	for (FGameplayTag LookupTag = DamageTypeTag ; LookupTag.IsValid() ; LookupTag = LookupTag.RequestDirectParent())
	{
		if (const FGameplayTag* OverrideCue = ReactionCueOverridesByDamageType.Find(LookupTag))
		{
			return *OverrideCue;
		}
	}
	return DefaultReactionCueTag;
}