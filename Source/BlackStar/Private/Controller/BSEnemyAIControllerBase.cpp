#include "Controller/BSEnemyAIControllerBase.h"

#include "Character/BSEnemyCharacter.h"
#include "Components/StateTreeAIComponent.h"
#include "Utility/BSGameplayTags.h"

ABSEnemyAIControllerBase::ABSEnemyAIControllerBase()
{
    // 기존 BP 설정을 확인하기 쉽도록 같은 컴포넌트 이름을 유지.
    StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
    bStartAILogicOnPossess = false;
    StateTreeAI->SetStartLogicAutomatically(false);
    bAttachToPawn = true;
}

void ABSEnemyAIControllerBase::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    SetGenericTeamId(FGenericTeamId(DefaultTeamId));

    // 공통 베이스에서는 AI를 시작하지 않음.
    // 일반 몬스터와 보스가 각각 시작 시점을 결정.
}

void ABSEnemyAIControllerBase::OnUnPossess()
{
    StopEnemyLogic(TEXT("UnPossess"));

    Super::OnUnPossess();
}

ABSEnemyCharacter* ABSEnemyAIControllerBase::GetEnemyCharacter() const
{
    return Cast<ABSEnemyCharacter>(GetPawn());
}

bool ABSEnemyAIControllerBase::StartEnemyLogic()
{
    ABSEnemyCharacter* Enemy = GetEnemyCharacter();

    if (!IsValid(Enemy) || Enemy->IsDead() || !StateTreeAI)
    {
        return false;
    }

    if (!StateTreeAI->IsRunning())
    {
        StateTreeAI->StartLogic();
    }

    return StateTreeAI->IsRunning();
}

void ABSEnemyAIControllerBase::StopEnemyLogic(const FString& Reason)
{
    if (StateTreeAI && StateTreeAI->IsRunning())
    {
        StateTreeAI->StopLogic(Reason);
    }

    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}

void ABSEnemyAIControllerBase::SendStateTreeEvent(FGameplayTag EventTag)
{
    if (!StateTreeAI || !StateTreeAI->IsRunning() || !EventTag.IsValid())
    {
        return;
    }

    FStateTreeEvent Event;
    Event.Tag = EventTag;

    StateTreeAI->SendStateTreeEvent(Event);
}

void ABSEnemyAIControllerBase::HandleEnemyHitReaction()
{
    SendStateTreeEvent(BSGameplayTags::Event_Character_HitReaction);
}

void ABSEnemyAIControllerBase::HandleEnemyDeath()
{
    // 일반 몬스터의 기존 사망 이벤트 전환을 유지.
    SendStateTreeEvent(BSGameplayTags::Event_Character_Death);
    StopMovement();
}