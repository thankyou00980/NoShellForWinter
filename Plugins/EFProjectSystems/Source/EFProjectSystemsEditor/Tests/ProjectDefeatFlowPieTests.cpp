#include "Tests/ProjectDefeatTestEnemyActor.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/ProjectCombatAttributeComponent.h"
#include "Defeat/ProjectDefeatBlueprintBridgeComponent.h"
#include "Defeat/ProjectDefeatFlowComponent.h"
#include "Defeat/ProjectDefeatFlowSettings.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "HAL/PlatformTime.h"
#include "InputCoreTypes.h"
#include "Locomotion/ProjectEmoteComponent.h"
#include "Locomotion/ProjectLocomotionOverrideComponent.h"
#include "Misc/AutomationTest.h"
#include "SinfulAscension/ProjectSinfulAscensionComponent.h"
#include "Survival/ProjectRealtimeSnapshotComponent.h"
#include "Tests/AutomationEditorCommon.h"
#include "UI/ProjectEmoteSubsystem.h"

namespace ProjectDefeatPIETestsPrivate
{
	enum class EProjectDefeatPIEScenario : uint8
	{
		OutOfCombatRecovery,
		CombatStruggleWin,
		CombatStruggleLose,
		RepeatKnockoutSameCombat,
		CancelledDefeatedSceneRestoresMovement,
		TravelArrivalFapCancelRestoresMovement
	};

	enum class EProjectDefeatPIEStep : uint8
	{
		WaitForPIE,
		SetupScenario,
		SimulateTravelArrival,
		ApplyFirstLethal,
		WaitForFirstResolution,
		FinishStruggleWin,
		FinishStruggleLose,
		WaitForBlackoutPersistence,
		WaitForDefeatedScene,
		TriggerSceneCancel,
		WaitForCancelRecovery,
		WaitForRecovery,
		ApplySecondLethal,
		WaitForRepeatDefeat,
		Done
	};

	struct FProjectDefeatFlowSettingsBackup
	{
		float StruggleGraceSeconds = 0.f;
		float KnockoutOutOfCombatRecoverySeconds = 0.f;
		float DefeatedTravelDelaySeconds = 0.f;
		float DefeatedCancelMovementRestoreDelaySeconds = 0.f;
		FProjectDefeatSceneDefinition DefaultSceneDefinition;
		bool bCaptured = false;
	};

	struct FProjectDefeatPIEScenarioContext
	{
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<APlayerController> PlayerController;
		TWeakObjectPtr<ACharacter> PlayerCharacter;
		TWeakObjectPtr<UProjectCombatAttributeComponent> PlayerCombatComponent;
		TWeakObjectPtr<UProjectRealtimeSnapshotComponent> RealtimeSnapshotComponent;
		TWeakObjectPtr<UProjectSinfulAscensionComponent> SinfulAscensionComponent;
		TWeakObjectPtr<UProjectLocomotionOverrideComponent> LocomotionOverrideComponent;
		TWeakObjectPtr<UProjectEmoteComponent> EmoteComponent;
		TWeakObjectPtr<UProjectDefeatBlueprintBridgeComponent> BlueprintBridgeComponent;
		TWeakObjectPtr<UProjectDefeatFlowComponent> DefeatFlowComponent;
		TWeakObjectPtr<AProjectDefeatTestMeleeMaleEnemy> EnemyActor;
	};

	template <typename ComponentType>
	ComponentType* FindOrAddRuntimeComponent(AActor* OwnerActor, const TCHAR* ComponentName)
	{
		if (!OwnerActor)
		{
			return nullptr;
		}

		if (ComponentType* ExistingComponent = OwnerActor->FindComponentByClass<ComponentType>())
		{
			return ExistingComponent;
		}

		ComponentType* Component = NewObject<ComponentType>(OwnerActor, ComponentType::StaticClass(), ComponentName);
		if (!Component)
		{
			return nullptr;
		}

		OwnerActor->AddInstanceComponent(Component);
		Component->OnComponentCreated();
		Component->RegisterComponent();
		Component->Activate(true);
		return Component;
	}

	TSubclassOf<ACharacter> ResolveAutomationPlayerClass()
	{
		constexpr const TCHAR* CandidatePlayerClassPaths[] =
		{
			TEXT("/Game/FullSample/Blueprints/Characters/Player/ACFFullPlayerBP.ACFFullPlayerBP_C"),
			TEXT("/Game/FullSample/Player.Player_C")
		};

		for (const TCHAR* CandidatePath : CandidatePlayerClassPaths)
		{
			if (UClass* LoadedClass = StaticLoadClass(ACharacter::StaticClass(), nullptr, CandidatePath))
			{
				return LoadedClass;
			}
		}

		return ACharacter::StaticClass();
	}

	class FRunProjectDefeatPIEScenarioCommand : public IAutomationLatentCommand
	{
	public:
		FRunProjectDefeatPIEScenarioCommand(FAutomationTestBase* InTest, const EProjectDefeatPIEScenario InScenario)
			: Test(InTest)
			, Scenario(InScenario)
		{
			if (Test)
			{
				Test->AddExpectedErrorPlain(TEXT("LogTemp: Can't Start the quest"), EAutomationExpectedErrorFlags::Contains, -1);
			}

			OverrideSettings();
		}

		virtual ~FRunProjectDefeatPIEScenarioCommand() override
		{
			RestoreSettings();
		}

		virtual bool Update() override
		{
			switch (Step)
			{
			case EProjectDefeatPIEStep::WaitForPIE:
				return UpdateWaitForPIE();
			case EProjectDefeatPIEStep::SetupScenario:
				return UpdateSetupScenario();
			case EProjectDefeatPIEStep::SimulateTravelArrival:
				return UpdateSimulateTravelArrival();
			case EProjectDefeatPIEStep::ApplyFirstLethal:
				return UpdateApplyFirstLethal();
			case EProjectDefeatPIEStep::WaitForFirstResolution:
				return UpdateWaitForFirstResolution();
			case EProjectDefeatPIEStep::FinishStruggleWin:
				return UpdateFinishStruggleWin();
			case EProjectDefeatPIEStep::FinishStruggleLose:
				return UpdateFinishStruggleLose();
			case EProjectDefeatPIEStep::WaitForBlackoutPersistence:
				return UpdateWaitForBlackoutPersistence();
			case EProjectDefeatPIEStep::WaitForDefeatedScene:
				return UpdateWaitForDefeatedScene();
			case EProjectDefeatPIEStep::TriggerSceneCancel:
				return UpdateTriggerSceneCancel();
			case EProjectDefeatPIEStep::WaitForCancelRecovery:
				return UpdateWaitForCancelRecovery();
			case EProjectDefeatPIEStep::WaitForRecovery:
				return UpdateWaitForRecovery();
			case EProjectDefeatPIEStep::ApplySecondLethal:
				return UpdateApplySecondLethal();
			case EProjectDefeatPIEStep::WaitForRepeatDefeat:
				return UpdateWaitForRepeatDefeat();
			case EProjectDefeatPIEStep::Done:
			default:
				return true;
			}
		}

	private:
		bool UpdateWaitForPIE()
		{
			if (GEditor && GEditor->PlayWorld)
			{
				Context.World = GEditor->PlayWorld;
				AdvanceTo(EProjectDefeatPIEStep::SetupScenario);
				return false;
			}

			return FailIfTimedOut(TEXT("PIE world never started."));
		}

		bool UpdateSetupScenario()
		{
			UWorld* World = Context.World.Get();
			if (!World)
			{
				Test->AddError(TEXT("PIE world vanished during setup."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			APlayerController* PlayerController = World->GetFirstPlayerController();
			if (!PlayerController)
			{
				return FailIfTimedOut(TEXT("No local player controller was available in PIE."));
			}

			Context.PlayerController = PlayerController;

			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ACharacter* PlayerCharacter = World->SpawnActor<ACharacter>(
				ResolveAutomationPlayerClass(),
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParameters);
			if (!PlayerCharacter)
			{
				Test->AddError(TEXT("Failed to spawn the automation player character."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			PlayerController->Possess(PlayerCharacter);
			Context.PlayerCharacter = PlayerCharacter;
			Context.PlayerCombatComponent = FindOrAddRuntimeComponent<UProjectCombatAttributeComponent>(PlayerCharacter, TEXT("TestPlayerCombat"));
			Context.RealtimeSnapshotComponent = FindOrAddRuntimeComponent<UProjectRealtimeSnapshotComponent>(PlayerCharacter, TEXT("TestRealtimeSnapshot"));
			Context.SinfulAscensionComponent = FindOrAddRuntimeComponent<UProjectSinfulAscensionComponent>(PlayerCharacter, TEXT("TestSinfulAscension"));
			Context.LocomotionOverrideComponent = FindOrAddRuntimeComponent<UProjectLocomotionOverrideComponent>(PlayerCharacter, TEXT("TestLocomotionOverride"));
			Context.EmoteComponent = FindOrAddRuntimeComponent<UProjectEmoteComponent>(PlayerCharacter, TEXT("TestEmoteComponent"));
			Context.BlueprintBridgeComponent = FindOrAddRuntimeComponent<UProjectDefeatBlueprintBridgeComponent>(PlayerCharacter, TEXT("TestDefeatBridge"));
			Context.DefeatFlowComponent = FindOrAddRuntimeComponent<UProjectDefeatFlowComponent>(PlayerCharacter, TEXT("TestDefeatFlow"));

			if (!Context.PlayerCombatComponent.IsValid()
				|| !Context.RealtimeSnapshotComponent.IsValid()
				|| !Context.SinfulAscensionComponent.IsValid()
				|| !Context.LocomotionOverrideComponent.IsValid()
				|| !Context.EmoteComponent.IsValid()
				|| !Context.BlueprintBridgeComponent.IsValid()
				|| !Context.DefeatFlowComponent.IsValid())
			{
				Test->AddError(TEXT("Failed to attach the runtime components required for the defeat-flow automation test."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if (Scenario != EProjectDefeatPIEScenario::OutOfCombatRecovery
				&& Scenario != EProjectDefeatPIEScenario::TravelArrivalFapCancelRestoresMovement)
			{
				AProjectDefeatTestMeleeMaleEnemy* EnemyActor = World->SpawnActor<AProjectDefeatTestMeleeMaleEnemy>(
					AProjectDefeatTestMeleeMaleEnemy::StaticClass(),
					FVector(120.f, 0.f, 0.f),
					FRotator::ZeroRotator,
					SpawnParameters);
				if (!EnemyActor)
				{
					Test->AddError(TEXT("Failed to spawn the automation enemy."));
					AdvanceTo(EProjectDefeatPIEStep::Done);
					return true;
				}

				Context.EnemyActor = EnemyActor;
				Context.RealtimeSnapshotComponent->ForceRefreshSnapshot();
			}

			if (Scenario == EProjectDefeatPIEScenario::TravelArrivalFapCancelRestoresMovement)
			{
				AdvanceTo(EProjectDefeatPIEStep::SimulateTravelArrival);
				return false;
			}

			AdvanceTo(EProjectDefeatPIEStep::ApplyFirstLethal);
			return false;
		}

		bool UpdateSimulateTravelArrival()
		{
			APlayerController* PlayerController = Context.PlayerController.Get();
			ACharacter* PlayerCharacter = Context.PlayerCharacter.Get();
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			if (!PlayerController || !PlayerCharacter || !DefeatFlowComponent)
			{
				Test->AddError(TEXT("The runtime context was lost before simulating defeated travel arrival."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			PlayerController->SetIgnoreMoveInput(true);
			PlayerController->SetIgnoreLookInput(true);
			PlayerCharacter->DisableInput(PlayerController);
			if (UCharacterMovementComponent* CharacterMovement = PlayerCharacter->GetCharacterMovement())
			{
				CharacterMovement->DisableMovement();
			}

			FProjectDefeatTransferPayload Payload;
			Payload.KnockoutReason = EProjectKnockoutReason::PainMaxed;
			Payload.DefeatReason = EProjectDefeatReason::LostStruggle;
			Payload.SceneDefinition = UProjectDefeatFlowSettings::Get()
				? UProjectDefeatFlowSettings::Get()->DefaultSceneDefinition
				: FProjectDefeatSceneDefinition();
			Payload.SceneDefinition.InteractionId = TEXT("Actions.Masturbate.FunnyTimeRemake");
			Payload.SceneDefinition.bAllowCancel = true;

			const FTransform SpawnTransform(FRotator::ZeroRotator, FVector(250.f, 0.f, 80.f));
			DefeatFlowComponent->HandleDefeatedArrivalFromTravel(Payload, FProjectDefeatInventorySnapshot(), SpawnTransform);

			AdvanceTo(EProjectDefeatPIEStep::WaitForDefeatedScene);
			return false;
		}

		bool UpdateApplyFirstLethal()
		{
			if (!ApplyLethalDamage())
			{
				Test->AddError(TEXT("Failed to apply the first lethal damage packet."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			AdvanceTo(EProjectDefeatPIEStep::WaitForFirstResolution);
			return false;
		}

		bool UpdateWaitForFirstResolution()
		{
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			UProjectCombatAttributeComponent* PlayerCombatComponent = Context.PlayerCombatComponent.Get();
			if (!DefeatFlowComponent || !PlayerCombatComponent)
			{
				Test->AddError(TEXT("The runtime defeat-flow context became invalid."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if (Scenario == EProjectDefeatPIEScenario::OutOfCombatRecovery)
			{
				if (DefeatFlowComponent->GetCurrentPhase() == EProjectDefeatPhase::KnockedOut)
				{
					AdvanceTo(EProjectDefeatPIEStep::WaitForRecovery);
					return false;
				}

				return FailIfTimedOut(TEXT("The out-of-combat lethal hit never entered KnockedOut."));
			}

			if (DefeatFlowComponent->GetCurrentPhase() == EProjectDefeatPhase::Struggle)
			{
				AdvanceTo(
					Scenario == EProjectDefeatPIEScenario::CombatStruggleLose
					|| Scenario == EProjectDefeatPIEScenario::CancelledDefeatedSceneRestoresMovement
						? EProjectDefeatPIEStep::FinishStruggleLose
						: EProjectDefeatPIEStep::FinishStruggleWin);
				return false;
			}

			if (DefeatFlowComponent->GetCurrentPhase() == EProjectDefeatPhase::DefeatedBlackout)
			{
				Test->AddError(TEXT("The struggle test entered DefeatedBlackout before the minigame started."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			return FailIfTimedOut(TEXT("The combat lethal hit never transitioned into the struggle phase."));
		}

		bool UpdateFinishStruggleWin()
		{
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			if (!DefeatFlowComponent)
			{
				Test->AddError(TEXT("DefeatFlowComponent was lost before finishing the struggle test."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			DefeatFlowComponent->AutomationCompleteActiveStruggleRound(true);
			AdvanceTo(EProjectDefeatPIEStep::WaitForRecovery);
			return false;
		}

		bool UpdateFinishStruggleLose()
		{
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			UProjectLocomotionOverrideComponent* LocomotionOverrideComponent = Context.LocomotionOverrideComponent.Get();
			UProjectDefeatBlueprintBridgeComponent* BlueprintBridgeComponent = Context.BlueprintBridgeComponent.Get();
			if (!DefeatFlowComponent)
			{
				Test->AddError(TEXT("DefeatFlowComponent was lost before failing the struggle test."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			DefeatFlowComponent->AutomationCompleteActiveStruggleRound(false);
			if (DefeatFlowComponent->GetCurrentPhase() != EProjectDefeatPhase::DefeatedBlackout)
			{
				return FailIfTimedOut(TEXT("Losing the struggle did not transition immediately into DefeatedBlackout."));
			}

			if (!LocomotionOverrideComponent || !LocomotionOverrideComponent->IsCrawlModeActive())
			{
				Test->AddError(TEXT("The player must remain in crawl while DefeatedBlackout is active after losing the struggle."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if (!BlueprintBridgeComponent || BlueprintBridgeComponent->GetPublicState() != EProjectDefeatPublicState::Downed)
			{
				Test->AddError(TEXT("The public defeat bridge must keep the player in Downed while blackout/travel are active."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if (Scenario == EProjectDefeatPIEScenario::CancelledDefeatedSceneRestoresMovement)
			{
				AdvanceTo(EProjectDefeatPIEStep::WaitForDefeatedScene);
				return false;
			}

			AdvanceTo(EProjectDefeatPIEStep::WaitForBlackoutPersistence);
			return false;
		}

		bool UpdateWaitForBlackoutPersistence()
		{
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			UProjectLocomotionOverrideComponent* LocomotionOverrideComponent = Context.LocomotionOverrideComponent.Get();
			UProjectDefeatBlueprintBridgeComponent* BlueprintBridgeComponent = Context.BlueprintBridgeComponent.Get();
			if (!DefeatFlowComponent)
			{
				Test->AddError(TEXT("DefeatFlowComponent was lost while verifying blackout persistence."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if ((FPlatformTime::Seconds() - StepStartRealSeconds) < 1.05)
			{
				return false;
			}

			Test->TestEqual(
				TEXT("The player should still be in DefeatedBlackout one second into the minigame-loss transition."),
				DefeatFlowComponent->GetCurrentPhase(),
				EProjectDefeatPhase::DefeatedBlackout);
			Test->TestTrue(
				TEXT("The player must stay in crawl for the full blackout handoff before travel starts."),
				LocomotionOverrideComponent && LocomotionOverrideComponent->IsCrawlModeActive());
			Test->TestEqual(
				TEXT("The public defeat state should remain Downed throughout the blackout handoff."),
				BlueprintBridgeComponent ? BlueprintBridgeComponent->GetPublicState() : EProjectDefeatPublicState::Normal,
				EProjectDefeatPublicState::Downed);

			AdvanceTo(EProjectDefeatPIEStep::Done);
			return true;
		}

		bool UpdateWaitForDefeatedScene()
		{
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			if (!DefeatFlowComponent)
			{
				Test->AddError(TEXT("DefeatFlowComponent was lost before entering the defeated scene."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if (!bForcedAutomationDefeatedSceneStart && DefeatFlowComponent->GetCurrentPhase() == EProjectDefeatPhase::DefeatedBlackout)
			{
				DefeatFlowComponent->AutomationStartDefeatedSceneWithoutTravel();
				bForcedAutomationDefeatedSceneStart = true;
				return false;
			}

			if (DefeatFlowComponent->GetCurrentPhase() == EProjectDefeatPhase::DefeatedScene)
			{
				APlayerController* PlayerController = Context.PlayerController.Get();
				ACharacter* PlayerCharacter = Context.PlayerCharacter.Get();
				UProjectEmoteComponent* EmoteComponent = Context.EmoteComponent.Get();
				UProjectEmoteSubsystem* EmoteSubsystem = Context.World.IsValid() ? Context.World->GetSubsystem<UProjectEmoteSubsystem>() : nullptr;

				Test->TestEqual(
					TEXT("The defeated scene should start the Fap interaction."),
					EmoteComponent ? EmoteComponent->GetActiveInteractionId() : NAME_None,
					FName(TEXT("Actions.Masturbate.FunnyTimeRemake")));
				Test->TestTrue(TEXT("The defeated scene should run through the ProjectEmote runtime-action path."), EmoteSubsystem && EmoteSubsystem->IsRuntimeActionActive());
				Test->TestEqual(
					TEXT("The defeated runtime action should use the hidden respawn action id."),
					EmoteSubsystem ? EmoteSubsystem->GetActiveRuntimeActionId() : NAME_None,
					FName(TEXT("Actions.System.Respawn.Fap")));
				Test->TestTrue(TEXT("Move input should stay blocked while the defeated Fap scene is active."), PlayerController && PlayerController->IsMoveInputIgnored());
				Test->TestTrue(TEXT("Look input should stay blocked while the defeated Fap scene is active."), PlayerController && PlayerController->IsLookInputIgnored());
				if (PlayerCharacter)
				{
					if (UCharacterMovementComponent* CharacterMovement = PlayerCharacter->GetCharacterMovement())
					{
						Test->TestTrue(TEXT("The defeated Fap scene should own movement and keep the character in MOVE_None."), CharacterMovement->MovementMode == MOVE_None);
					}
				}

				AdvanceTo(EProjectDefeatPIEStep::TriggerSceneCancel);
				return false;
			}

			return FailIfTimedOut(TEXT("The player never entered the defeated scene after losing the struggle."));
		}

		bool UpdateTriggerSceneCancel()
		{
			APlayerController* PlayerController = Context.PlayerController.Get();
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			if (!PlayerController || !DefeatFlowComponent)
			{
				Test->AddError(TEXT("The runtime context was lost before cancelling the defeated scene."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			const FKey CancelKey(TEXT("Y"));
			PlayerController->InputKey(FInputKeyParams(CancelKey, IE_Pressed, 1.0, false));
			PlayerController->InputKey(FInputKeyParams(CancelKey, IE_Released, 0.0, false));
			AdvanceTo(EProjectDefeatPIEStep::WaitForCancelRecovery);
			return false;
		}

		bool UpdateWaitForCancelRecovery()
		{
			APlayerController* PlayerController = Context.PlayerController.Get();
			ACharacter* PlayerCharacter = Context.PlayerCharacter.Get();
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			UProjectLocomotionOverrideComponent* LocomotionOverrideComponent = Context.LocomotionOverrideComponent.Get();
			UProjectEmoteSubsystem* EmoteSubsystem = Context.World.IsValid() ? Context.World->GetSubsystem<UProjectEmoteSubsystem>() : nullptr;
			if (!PlayerController || !PlayerCharacter || !DefeatFlowComponent)
			{
				Test->AddError(TEXT("The runtime context became invalid while waiting for defeated-scene recovery."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if ((FPlatformTime::Seconds() - StepStartRealSeconds) < 1.25)
			{
				return false;
			}

			if (DefeatFlowComponent->GetCurrentPhase() != EProjectDefeatPhase::None)
			{
				return FailIfTimedOut(TEXT("Cancelling the defeated scene never returned the flow to None."));
			}

			Test->TestFalse(TEXT("Move input should be fully restored one second after cancelling the defeated scene."), PlayerController->IsMoveInputIgnored());
			Test->TestFalse(TEXT("Look input should be fully restored one second after cancelling the defeated scene."), PlayerController->IsLookInputIgnored());
			Test->TestFalse(TEXT("The defeated runtime action should be inactive after cancel recovery."), EmoteSubsystem && EmoteSubsystem->IsRuntimeActionActive());

			if (UCharacterMovementComponent* CharacterMovement = PlayerCharacter->GetCharacterMovement())
			{
				Test->TestTrue(TEXT("The player should no longer be left in MOVE_None after cancelling the defeated scene."), CharacterMovement->MovementMode != MOVE_None);
			}
			else
			{
				Test->AddError(TEXT("The automation player did not expose a CharacterMovementComponent."));
			}

			if (LocomotionOverrideComponent)
			{
				Test->TestFalse(TEXT("Cancelling the defeated scene should release forced crawl."), LocomotionOverrideComponent->IsCrawlModeActive());
			}

			if (UWorld* World = Context.World.Get())
			{
				if (UProjectEmoteSubsystem* MenuEmoteSubsystem = World->GetSubsystem<UProjectEmoteSubsystem>())
				{
					Test->TestFalse(TEXT("Pressing Y to cancel the defeated scene must not leave the emote menu open."), MenuEmoteSubsystem->IsEmoteMenuOpen());
				}
			}

			AdvanceTo(EProjectDefeatPIEStep::Done);
			return true;
		}

		bool UpdateWaitForRecovery()
		{
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			UProjectCombatAttributeComponent* PlayerCombatComponent = Context.PlayerCombatComponent.Get();
			if (!DefeatFlowComponent || !PlayerCombatComponent)
			{
				Test->AddError(TEXT("The runtime defeat-flow context became invalid while waiting for recovery."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if (DefeatFlowComponent->GetCurrentPhase() != EProjectDefeatPhase::None)
			{
				return FailIfTimedOut(TEXT("The player never recovered out of the knocked-out state."));
			}

			const float CurrentHealth = PlayerCombatComponent->GetAttributeCurrentValue(PlayerCombatComponent->HealthAttributeName);
			const float MaxHealth = FMath::Max(PlayerCombatComponent->GetAttributeMaxValue(PlayerCombatComponent->HealthAttributeName), 1.f);
			Test->TestTrue(TEXT("Recovering from knockout should restore at least 50% health."), CurrentHealth >= (MaxHealth * 0.5f) - KINDA_SMALL_NUMBER);

			if (Scenario == EProjectDefeatPIEScenario::RepeatKnockoutSameCombat)
			{
				Test->TestTrue(
					TEXT("Winning the struggle should keep the knockout tagged to the active combat session."),
					DefeatFlowComponent->AutomationGetActiveCombatSessionId() != 0
						&& DefeatFlowComponent->AutomationGetActiveCombatSessionId() == DefeatFlowComponent->AutomationGetLastKnockoutCombatSessionId());
				AdvanceTo(EProjectDefeatPIEStep::ApplySecondLethal);
				return false;
			}

			AdvanceTo(EProjectDefeatPIEStep::Done);
			return true;
		}

		bool UpdateApplySecondLethal()
		{
			if (!ApplyLethalDamage())
			{
				Test->AddError(TEXT("Failed to apply the second lethal damage packet for the repeat-knockout test."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			AdvanceTo(EProjectDefeatPIEStep::WaitForRepeatDefeat);
			return false;
		}

		bool UpdateWaitForRepeatDefeat()
		{
			UProjectDefeatFlowComponent* DefeatFlowComponent = Context.DefeatFlowComponent.Get();
			if (!DefeatFlowComponent)
			{
				Test->AddError(TEXT("DefeatFlowComponent was lost before verifying the repeat-knockout defeat."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if (DefeatFlowComponent->GetCurrentPhase() == EProjectDefeatPhase::DefeatedBlackout)
			{
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			if (DefeatFlowComponent->GetCurrentPhase() == EProjectDefeatPhase::KnockedOut
				|| DefeatFlowComponent->GetCurrentPhase() == EProjectDefeatPhase::Struggle)
			{
				Test->AddError(TEXT("A second knockout in the same combat entered another downed state instead of defeated."));
				AdvanceTo(EProjectDefeatPIEStep::Done);
				return true;
			}

			return FailIfTimedOut(TEXT("The repeat-knockout lethal hit never transitioned into DefeatedBlackout."));
		}

		void AdvanceTo(const EProjectDefeatPIEStep NewStep)
		{
			Step = NewStep;
			StepStartRealSeconds = FPlatformTime::Seconds();
		}

		bool FailIfTimedOut(const TCHAR* ErrorMessage)
		{
			constexpr double TimeoutSeconds = 6.0;
			if ((FPlatformTime::Seconds() - StepStartRealSeconds) < TimeoutSeconds)
			{
				return false;
			}

			Test->AddError(ErrorMessage);
			AdvanceTo(EProjectDefeatPIEStep::Done);
			return true;
		}

		bool ApplyLethalDamage() const
		{
			UProjectCombatAttributeComponent* PlayerCombatComponent = Context.PlayerCombatComponent.Get();
			if (!PlayerCombatComponent)
			{
				return false;
			}

			FProjectCombatDamageSpec DamageSpec;
			DamageSpec.DamageType = TEXT("Physical");
			DamageSpec.TargetAttribute = PlayerCombatComponent->HealthAttributeName;
			DamageSpec.BaseDamage = 1000.f;
			DamageSpec.SourceActor = Context.EnemyActor.Get();
			DamageSpec.DamageCauser = Context.EnemyActor.Get();
			PlayerCombatComponent->ApplyDamage(DamageSpec);
			return true;
		}

		void OverrideSettings()
		{
			UProjectDefeatFlowSettings* MutableSettings = GetMutableDefault<UProjectDefeatFlowSettings>();
			if (!MutableSettings)
			{
				return;
			}

			SettingsBackup.StruggleGraceSeconds = MutableSettings->StruggleGraceSeconds;
			SettingsBackup.KnockoutOutOfCombatRecoverySeconds = MutableSettings->KnockoutOutOfCombatRecoverySeconds;
			SettingsBackup.DefeatedTravelDelaySeconds = MutableSettings->DefeatedTravelDelaySeconds;
			SettingsBackup.DefeatedCancelMovementRestoreDelaySeconds = MutableSettings->DefeatedCancelMovementRestoreDelaySeconds;
			SettingsBackup.DefaultSceneDefinition = MutableSettings->DefaultSceneDefinition;
			SettingsBackup.bCaptured = true;

			MutableSettings->StruggleGraceSeconds = 0.05f;
			MutableSettings->KnockoutOutOfCombatRecoverySeconds = 0.15f;
			MutableSettings->DefeatedTravelDelaySeconds = 1.35f;
			MutableSettings->DefeatedCancelMovementRestoreDelaySeconds = 1.0f;
			MutableSettings->DefaultSceneDefinition.InteractionId = TEXT("Actions.Masturbate.FunnyTimeRemake");
			MutableSettings->DefaultSceneDefinition.bAllowCancel = true;
		}

		void RestoreSettings() const
		{
			if (!SettingsBackup.bCaptured)
			{
				return;
			}

			if (UProjectDefeatFlowSettings* MutableSettings = GetMutableDefault<UProjectDefeatFlowSettings>())
			{
				MutableSettings->StruggleGraceSeconds = SettingsBackup.StruggleGraceSeconds;
				MutableSettings->KnockoutOutOfCombatRecoverySeconds = SettingsBackup.KnockoutOutOfCombatRecoverySeconds;
				MutableSettings->DefeatedTravelDelaySeconds = SettingsBackup.DefeatedTravelDelaySeconds;
				MutableSettings->DefeatedCancelMovementRestoreDelaySeconds = SettingsBackup.DefeatedCancelMovementRestoreDelaySeconds;
				MutableSettings->DefaultSceneDefinition = SettingsBackup.DefaultSceneDefinition;
			}
		}

	private:
		FAutomationTestBase* Test = nullptr;
		EProjectDefeatPIEScenario Scenario = EProjectDefeatPIEScenario::OutOfCombatRecovery;
		EProjectDefeatPIEStep Step = EProjectDefeatPIEStep::WaitForPIE;
		double StepStartRealSeconds = FPlatformTime::Seconds();
		bool bForcedAutomationDefeatedSceneStart = false;
		FProjectDefeatFlowSettingsBackup SettingsBackup;
		FProjectDefeatPIEScenarioContext Context;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowPIEOutOfCombatRecoveryTest,
	"ACFUltimateSample.Defeat.Flow.PIE.OutOfCombatRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowPIEOutOfCombatRecoveryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FAutomationEditorCommonUtils::CreateNewMap();
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(ProjectDefeatPIETestsPrivate::FRunProjectDefeatPIEScenarioCommand(
		this,
		ProjectDefeatPIETestsPrivate::EProjectDefeatPIEScenario::OutOfCombatRecovery));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowPIECombatWinTest,
	"ACFUltimateSample.Defeat.Flow.PIE.CombatStruggleWin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowPIECombatWinTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FAutomationEditorCommonUtils::CreateNewMap();
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(ProjectDefeatPIETestsPrivate::FRunProjectDefeatPIEScenarioCommand(
		this,
		ProjectDefeatPIETestsPrivate::EProjectDefeatPIEScenario::CombatStruggleWin));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowPIECombatLoseTest,
	"ACFUltimateSample.Defeat.Flow.PIE.CombatStruggleLose",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowPIECombatLoseTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FAutomationEditorCommonUtils::CreateNewMap();
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(ProjectDefeatPIETestsPrivate::FRunProjectDefeatPIEScenarioCommand(
		this,
		ProjectDefeatPIETestsPrivate::EProjectDefeatPIEScenario::CombatStruggleLose));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowPIERepeatKnockoutTest,
	"ACFUltimateSample.Defeat.Flow.PIE.RepeatKnockoutSameCombat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowPIERepeatKnockoutTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FAutomationEditorCommonUtils::CreateNewMap();
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(ProjectDefeatPIETestsPrivate::FRunProjectDefeatPIEScenarioCommand(
		this,
		ProjectDefeatPIETestsPrivate::EProjectDefeatPIEScenario::RepeatKnockoutSameCombat));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowPIECancelledDefeatedSceneTest,
	"ACFUltimateSample.Defeat.Flow.PIE.CancelledDefeatedSceneRestoresMovement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowPIECancelledDefeatedSceneTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FAutomationEditorCommonUtils::CreateNewMap();
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(ProjectDefeatPIETestsPrivate::FRunProjectDefeatPIEScenarioCommand(
		this,
		ProjectDefeatPIETestsPrivate::EProjectDefeatPIEScenario::CancelledDefeatedSceneRestoresMovement));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowPIETravelArrivalFapCancelTest,
	"ACFUltimateSample.Defeat.Flow.PIE.TravelArrivalFapCancelRestoresMovement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowPIETravelArrivalFapCancelTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FAutomationEditorCommonUtils::CreateNewMap();
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(ProjectDefeatPIETestsPrivate::FRunProjectDefeatPIEScenarioCommand(
		this,
		ProjectDefeatPIETestsPrivate::EProjectDefeatPIEScenario::TravelArrivalFapCancelRestoresMovement));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
