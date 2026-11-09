#include "Character/BSBossCharacter.h"
#include "Controller/BSBossAIController.h"

ABSBossCharacter::ABSBossCharacter()
{
	AIControllerClass = ABSBossAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 격파 후 연출과 보상 처리가 끝날 때까지 자동 제거하지 않음.
	CorpseLifeTime = 0.0f;
}