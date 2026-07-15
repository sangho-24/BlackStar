#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"

namespace BSTeam
{
	inline constexpr uint8 Player = 1;
	inline constexpr uint8 Enemy = 2;
	
	inline ETeamAttitude::Type GetAttitude(const AActor* Source, const AActor* Target)
	{
		if (!Source || !Target)
		{
			return ETeamAttitude::Neutral;
		}

		const IGenericTeamAgentInterface* SourceTeamAgent = Cast<IGenericTeamAgentInterface>(Source);

		if (!SourceTeamAgent)
		{
			return ETeamAttitude::Neutral;
		}

		return SourceTeamAgent->GetTeamAttitudeTowards(*Target);
	}

	inline bool AreFriendly(const AActor* Source, const AActor* Target)
	{
		return GetAttitude(Source, Target) == ETeamAttitude::Friendly;
	}

	inline bool AreHostile(const AActor* Source, const AActor* Target)
	{
		return GetAttitude(Source, Target) == ETeamAttitude::Hostile;
	}
}