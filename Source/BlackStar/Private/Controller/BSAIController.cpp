#include "Controller/BSAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "Utility/BSGameplayTags.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Character/BSEnemyCharacter.h"

ABSAIController::ABSAIController()
{
	// 퍼셉션
	BSPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("BSPerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->SightRadius = 2500.0f;
	SightConfig->LoseSightRadius = 3000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 70.0f;
	SightConfig->SetMaxAge(3.0f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	BSPerceptionComponent->ConfigureSense(*SightConfig);
	BSPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	BSPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABSAIController::OnTargetPerceptionUpdated);
	BSPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ABSAIController::OnTargetPerceptionForgotten);
	SetPerceptionComponent(*BSPerceptionComponent);

	// 스테이트 트리
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));

	bStartAILogicOnPossess = false;
	StateTreeAI->SetStartLogicAutomatically(false);
	bAttachToPawn = true;
}

void ABSAIController::SendDeathStateTreeEvent()
{
	FStateTreeEvent Event;
	Event.Tag = BSGameplayTags::Event_Character_Death;
	StateTreeAI->SendStateTreeEvent(Event);
}

void ABSAIController::OnPossess(APawn *InPawn)
{
	Super::OnPossess(InPawn);
	StateTreeAI->StartLogic();
}

void ABSAIController::OnUnPossess()
{
	if (StateTreeAI)
	{
		StateTreeAI->StopLogic(TEXT("UnPossess"));
	}
	Super::OnUnPossess();
}

void ABSAIController::OnTargetPerceptionUpdated(AActor *Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}

	ICombatInterface *CombatTarget = Cast<ICombatInterface>(Actor);
	if (!CombatTarget || CombatTarget->IsDead())
	{
		return;
	}

	ABSEnemyCharacter* EnemyCharacter = Cast<ABSEnemyCharacter>(GetPawn());
	if (!EnemyCharacter || EnemyCharacter->IsDead())
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		AActor* CurrentTarget = EnemyCharacter->GetCombatTarget();
		const bool bHasVisibleCurrentTarget = CurrentTarget && VisibleTarget.Get() == CurrentTarget;
		// 새 타겟이 추적중인 타겟이 아님 && 추적중인 대상이 눈에 보임 -> 새 타겟 무시
		if (CurrentTarget && CurrentTarget != Actor && bHasVisibleCurrentTarget)
		{
			return;
		}
		EnemyCharacter->SetCombatTarget(Actor);
		VisibleTarget = Actor;
		// EnemyCharacter->SetLastKnownTargetLocation(Actor->GetActorLocation());
		// StartLastKnownTargetLocationUpdate(Actor);
	}
	else
	{
		// 인지 벗어난 액터가 현재 대상인지 확인. 다른 대상을 잡고있는 경우 클리어 안함.
		if (VisibleTarget.Get() == Actor)
		{
			VisibleTarget.Reset();
			// StopLastKnownTargetLocationUpdate();
		}
	}
}

void ABSAIController::OnTargetPerceptionForgotten(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	ABSEnemyCharacter* EnemyCharacter = Cast<ABSEnemyCharacter>(GetPawn());
	if (!EnemyCharacter || EnemyCharacter->IsDead())
	{
		return;
	}
	// 수명이 끝난 액터가 현재 대상으로 잡은 액터인지 확인. 다른 대상을 잡고있는 경우 클리어 안함.
	if (EnemyCharacter->GetCombatTarget() == Actor)
	{
		EnemyCharacter->SetLastKnownTargetLocation(Actor->GetActorLocation());
		EnemyCharacter->ClearCombatTarget();
	}
}

// void ABSAIController::StartLastKnownTargetLocationUpdate(AActor* TargetActor)
// {
// 	if (!TargetActor)
// 	{
// 		return;
// 	}
//
// 	VisibleTarget = TargetActor;
//
// 	GetWorldTimerManager().ClearTimer(UpdateLastKnownTargetLocationTimerHandle);
//
// 	GetWorldTimerManager().SetTimer(
// 		UpdateLastKnownTargetLocationTimerHandle,
// 		this,
// 		&ABSAIController::UpdateLastKnownTargetLocation,
// 		0.1f,
// 		true
// 	);
// }
//
//
// void ABSAIController::StopLastKnownTargetLocationUpdate()
// {
// 	GetWorldTimerManager().ClearTimer(UpdateLastKnownTargetLocationTimerHandle);
// 	VisibleTarget.Reset();
// }
//
// void ABSAIController::UpdateLastKnownTargetLocation()
// {
// 	AActor* TargetActor = VisibleTarget.Get();
// 	if (!TargetActor)
// 	{
// 		StopLastKnownTargetLocationUpdate();
// 		return;
// 	}
//
// 	ABSEnemyCharacter* EnemyCharacter = Cast<ABSEnemyCharacter>(GetPawn());
// 	if (!EnemyCharacter || EnemyCharacter->IsDead())
// 	{
// 		StopLastKnownTargetLocationUpdate();
// 		return;
// 	}
//
// 	if (EnemyCharacter->GetCombatTarget() != TargetActor)
// 	{
// 		StopLastKnownTargetLocationUpdate();
// 		return;
// 	}
//
// 	EnemyCharacter->SetLastKnownTargetLocation(TargetActor->GetActorLocation());
// }
