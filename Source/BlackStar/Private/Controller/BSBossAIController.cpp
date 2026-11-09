#include "Controller/BSBossAIController.h"

#include "AbilitySystemComponent.h"
#include "Character/BSBossCharacter.h"

bool ABSBossAIController::StartCombat(AActor* TargetActor)
{
	ABSBossCharacter* Boss = Cast<ABSBossCharacter>(GetPawn());
	ABSBaseCharacter* Target = Cast<ABSBaseCharacter>(TargetActor);
	if (!Boss || !Target || Boss->IsDead() || Target->IsDead())
	{
		return false;
	}

	if (bCombatActive)
	{
		return Boss->GetCombatTarget() == Target;
	}
	Boss->SetCombatTarget(Target);
	bCombatActive = true;

	if (!StartEnemyLogic())
	{
		bCombatActive = false;
		Boss->ClearCombatTarget();
		StopEnemyLogic(TEXT("Boss combat start failed"));
		return false;
	}

	return true;
}

void ABSBossAIController::EndCombat()
{
	bCombatActive = false;
	StopEnemyLogic(TEXT("Boss combat ended"));

	if (ABSBossCharacter* Boss = Cast<ABSBossCharacter>(GetPawn()))
	{
		Boss->ClearCombatTarget();
		Boss->StopTurning();

		if (UAbilitySystemComponent* ASC = Boss->GetAbilitySystemComponent())
		{
			ASC->CancelAllAbilities();
		}
	}
}

void ABSBossAIController::HandleEnemyDeath()
{
	// 사망 모션은 캐릭터가 처리하므로 행동 AI는 종료.
	EndCombat();
}

void ABSBossAIController::OnUnPossess()
{
	EndCombat();
	Super::OnUnPossess();
}