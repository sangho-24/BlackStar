#include "Component/BSHitReactionComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character/BSBaseCharacter.h"
#include "Engine/Engine.h"

UBSHitReactionComponent::UBSHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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


void UBSHitReactionComponent::ApplyHitReaction(
	AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData)
{
	if (!OwnerCharacter || ReactionData.ReactionType == EBSHitReactionType::None)
	{
		return;
	}
	if (!CanApplyHitReaction(ReactionData))
	{
		return;
	}
	switch (ReactionData.ReactionType)
	{
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

bool UBSHitReactionComponent::CanApplyHitReaction(const FBSHitReactionData& ReactionData) const
{
	if (!OwnerASC)	// 없으면 버그. 태그검사 프리패스.
	{
		return true;
	}
	if (ReactionData.IgnoreIfTargetHasTags.IsEmpty())
	{
		return true;
	}
	return !OwnerASC->HasAnyMatchingGameplayTags(ReactionData.IgnoreIfTargetHasTags);
}

void UBSHitReactionComponent::ApplyFlinch(AActor* Attacker, const FHitResult& HitResult,
	const FBSHitReactionData& ReactionData)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Green,
			FString::Printf(TEXT("움찔%s"), *FString::ChrN(FMath::RandRange(1, 3), TEXT('~')))
		);
	}
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
