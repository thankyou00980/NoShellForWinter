#include "EFCharacterCreationGameplayHooks.h"

namespace EFCharacterCreationGameplayHooks
{
	FEFCharacterCreationSetPawnCanMoveHook& OnSetPawnCanMove()
	{
		static FEFCharacterCreationSetPawnCanMoveHook Hook;
		return Hook;
	}

	FEFCharacterCreationCancelPawnAbilitiesHook& OnCancelPawnAbilities()
	{
		static FEFCharacterCreationCancelPawnAbilitiesHook Hook;
		return Hook;
	}
}
