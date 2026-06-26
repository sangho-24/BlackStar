#include "Controller/BSAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "Utility/BSGameplayTags.h"

ABSAIController::ABSAIController()
{
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

void ABSAIController::OnPossess(APawn* InPawn)
{
	if (StateTreeAI)
	{
		StateTreeAI->StopLogic(TEXT("UnPossess"));
	}
	Super::OnUnPossess();
}

void ABSAIController::OnUnPossess()
{
	Super::OnUnPossess();
	StateTreeAI->StopLogic(TEXT("UnPossess"));
}
