#pragma once

#include "CoreMinimal.h"

class APawn;

DECLARE_MULTICAST_DELEGATE_TwoParams(FEFCharacterCreationSetPawnCanMoveHook, APawn* /* Pawn */, bool /* bCanMove */);
DECLARE_MULTICAST_DELEGATE_OneParam(FEFCharacterCreationCancelPawnAbilitiesHook, APawn* /* Pawn */);

namespace EFCharacterCreationGameplayHooks
{
	EFCHARACTERCREATIONRUNTIME_API FEFCharacterCreationSetPawnCanMoveHook& OnSetPawnCanMove();
	EFCHARACTERCREATIONRUNTIME_API FEFCharacterCreationCancelPawnAbilitiesHook& OnCancelPawnAbilities();
}
