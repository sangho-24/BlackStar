#include "Controller/BSAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "Utility/BSGameplayTags.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Character/BSEnemyCharacter.h"

ABSAIController::ABSAIController()
{
	// 퍼셉션
	BSPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("BSPerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

	// Sight
	SightConfig->SightRadius = 1800.0f;
	SightConfig->LoseSightRadius = 2300.0f;
	SightConfig->PeripheralVisionAngleDegrees = 65.0f;
	SightConfig->SetMaxAge(2.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	// Hearing
	HearingConfig->HearingRange = 2500.0f;
	HearingConfig->SetMaxAge(4.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
	
	BSPerceptionComponent->ConfigureSense(*SightConfig);
	BSPerceptionComponent->ConfigureSense(*HearingConfig);
	BSPerceptionComponent->ConfigureSense(*DamageConfig);
	BSPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	BSPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABSAIController::OnTargetPerceptionUpdated);
	// BSPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ABSAIController::OnTargetPerceptionForgotten);
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

// ETeamAttitude::Type ABSAIController::GetTeamAttitudeTowards(const AActor& Other) const
// {
// 	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other);
//
// 	if (!OtherTeamAgent)
// 	{
// 		if (const APawn* OtherPawn = Cast<APawn>(&Other))
// 		{
// 			OtherTeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController());
// 		}
// 	}
//
// 	if (!OtherTeamAgent)
// 	{
// 		return ETeamAttitude::Neutral;
// 	}
//
// 	return FGenericTeamId::GetAttitude(GetGenericTeamId(),OtherTeamAgent -> GetGenericTeamId());
// }

void ABSAIController::OnPossess(APawn *InPawn)
{
	Super::OnPossess(InPawn);
	SetGenericTeamId(FGenericTeamId(DefaultTeamId));
	StateTreeAI->StartLogic();
}

void ABSAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(SightTargetTraceTimerHandle);
	LostSightTarget.Reset();
	VisibleTarget.Reset();
	
	if (StateTreeAI)
	{
		StateTreeAI->StopLogic(TEXT("UnPossess"));
	}
	Super::OnUnPossess();
}

void ABSAIController::OnTargetPerceptionUpdated(AActor *Actor, FAIStimulus Stimulus)
{
	const APawn* TargetPawn = Cast<APawn>(Actor);
	const IGenericTeamAgentInterface* TargetTeamAgent =
		TargetPawn
			? Cast<IGenericTeamAgentInterface>(
				TargetPawn->GetController())
			: Cast<IGenericTeamAgentInterface>(Actor);

	const uint8 MyTeamId =
		GetGenericTeamId().GetId();

	const uint8 TargetTeamId =
		TargetTeamAgent
			? TargetTeamAgent->GetGenericTeamId().GetId()
			: FGenericTeamId::NoTeam.GetId();

	const ETeamAttitude::Type Attitude = GetTeamAttitudeTowards(*Actor);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("AI Team=%d, Target Team=%d, Attitude=%d, Actor=%s, Controller=%s"),
		MyTeamId,
		TargetTeamId,
		static_cast<int32>(Attitude),
		*GetNameSafe(Actor),
		TargetPawn
			? *GetNameSafe(TargetPawn->GetController())
			: TEXT("No Pawn"));
	// =====
	if (!Actor)
	{
		return;
	}
	
	ABSEnemyCharacter* EnemyCharacter = Cast<ABSEnemyCharacter>(GetPawn());
	if (!EnemyCharacter || EnemyCharacter->IsDead())
	{
		return;
	}
	ICombatInterface *CombatTarget = Cast<ICombatInterface>(Actor);
	if (!CombatTarget || CombatTarget->IsDead())
	{
		return;
	}
	// Affiliation 설정만 믿지 말고 방어적으로 한 번 더 검사
	if (GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
	{
		return;
	}
	
	const FAISenseID SightSenseID = UAISense::GetSenseID<UAISense_Sight>();
	const FAISenseID HearingSenseID = UAISense::GetSenseID<UAISense_Hearing>();
	const FAISenseID DamageSenseID = UAISense::GetSenseID<UAISense_Damage>();
	
	// 시각
	if (Stimulus.Type == SightSenseID)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			AActor* CurrentTarget = EnemyCharacter->GetCombatTarget();
			const bool bHasVisibleCurrentTarget = CurrentTarget && VisibleTarget.Get() == CurrentTarget;
			// 새 타겟이 추적중인 타겟이 아님 && 추적중인 대상이 눈에 보임 -> 새 타겟 무시
			if (CurrentTarget && CurrentTarget != Actor && bHasVisibleCurrentTarget)
			{
				return;
			}
			CancelSightTargetTraceTimer();
			// Damage,Hearing 회전과 이동 회전의 충돌 방지
			EnemyCharacter->StopTurning();
			EnemyCharacter->SetCombatTarget(Actor);
			VisibleTarget = Actor;
		}
		// 인지 벗어난 액터가 현재 대상인지 확인. 다른 대상을 잡고있는 경우 클리어 안함.
		else if (VisibleTarget.Get() == Actor)
		{
			VisibleTarget.Reset();
			if (EnemyCharacter->GetCombatTarget() == Actor)
			{
				StartSightTargetTraceTimer(Actor);
			}
		}
		return;
	}
	// 청각
	if (Stimulus.Type == HearingSenseID)
	{
		// 이미 타깃을 보고 있다면 다른 소리에 돌아보지 않음
		if (EnemyCharacter->GetCombatTarget())
		{
			return;
		}
		if (Stimulus.WasSuccessfullySensed())
		{
			StopMovement();
			EnemyCharacter->SetLastKnownTargetLocation(Stimulus.StimulusLocation);
			TurnTowardsStimulus(EnemyCharacter,Stimulus.StimulusLocation);
		}
		return;
	}
	// 피격
	if (Stimulus.Type == DamageSenseID)
	{
		if (EnemyCharacter->GetCombatTarget())
		{
			return;
		}
		if (Stimulus.WasSuccessfullySensed())
		{
			// 피격으로 위치만 알 뿐, Actor를 CombatTarget으로 지정하지 않음
			StopMovement();
			EnemyCharacter->SetLastKnownTargetLocation(Stimulus.StimulusLocation);
			TurnTowardsStimulus(EnemyCharacter, Stimulus.StimulusLocation);
		}
		return;
	}
}

// void ABSAIController::OnTargetPerceptionForgotten(AActor* Actor)
// {
// 	if (!Actor)
// 	{
// 		return;
// 	}
//
// 	ABSEnemyCharacter* EnemyCharacter = Cast<ABSEnemyCharacter>(GetPawn());
// 	if (!EnemyCharacter || EnemyCharacter->IsDead())
// 	{
// 		return;
// 	}
// 	// 수명이 끝난 액터가 현재 대상으로 잡은 액터인지 확인. 다른 대상을 잡고있는 경우 클리어 안함.
// 	if (EnemyCharacter->GetCombatTarget() == Actor)
// 	{
// 		EnemyCharacter->SetLastKnownTargetLocation(Actor->GetActorLocation());
// 		EnemyCharacter->ClearCombatTarget();
// 	}
// }

void ABSAIController::TurnTowardsStimulus(ABSEnemyCharacter* EnemyCharacter, const FVector& StimulusLocation)
{
	if (!EnemyCharacter)
	{
		return;
	}
	FVector Direction = StimulusLocation - EnemyCharacter->GetActorLocation();
	Direction.Z = 0.0f;
	if (Direction.IsNearlyZero())
	{
		return;
	}
	EnemyCharacter->StartTurning(Direction.Rotation(), StimulusTurnSpeed);
}

void ABSAIController::StartSightTargetTraceTimer(AActor* TraceTarget)
{
	if (!TraceTarget)
	{
		return;
	}
	LostSightTarget = TraceTarget;

	GetWorldTimerManager().ClearTimer(SightTargetTraceTimerHandle);
	GetWorldTimerManager().SetTimer(
		SightTargetTraceTimerHandle,
		this,
		&ABSAIController::FinishSightTargetTraceTimer,
		SightTargetTraceTime,
		false);
}

void ABSAIController::CancelSightTargetTraceTimer()
{
	GetWorldTimerManager().ClearTimer(SightTargetTraceTimerHandle);
}

void ABSAIController::FinishSightTargetTraceTimer()
{
	AActor* LostTarget = LostSightTarget.Get();
	LostSightTarget.Reset();
	
	ABSEnemyCharacter* EnemyCharacter = Cast<ABSEnemyCharacter>(GetPawn());
	if (!EnemyCharacter || EnemyCharacter->IsDead())
	{
		return;
	}
	
	AActor* CombatTarget = EnemyCharacter->GetCombatTarget();
	if (CombatTarget && CombatTarget == LostTarget)
	{
		EnemyCharacter->SetLastKnownTargetLocation(CombatTarget->GetActorLocation());
		EnemyCharacter->ClearCombatTarget();
	}
}
