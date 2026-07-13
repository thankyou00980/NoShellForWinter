#include "Combat/ProjectCombatTypes.h"

#include "GameFramework/Actor.h"

namespace ProjectCombatTags
{
	bool IsActorIntimacyShielded(const AActor* Actor)
	{
		return IsValid(Actor) && Actor->Tags.Contains(IntimacyCombatShield());
	}
}
