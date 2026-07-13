#include "Locomotion/ProjectLocomotionOverrideComponent.h"

#include "EFProjectAssetPathResolver.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimEnums.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/ACFCharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "SinfulAscension/ProjectSinfulAscensionComponent.h"

namespace
{
	constexpr float WalkPivotNoiseDeadZoneDegrees = 10.f;
	constexpr float CrawlIdleHandoffStartOffsetSeconds = 1.f / 30.f;

	void ApplyAcfLocomotionStateSpeed(UACFCharacterMovementComponent* MovementComponent, const ELocomotionState State, const float Speed, const float SwimSpeed)
	{
		if (!MovementComponent)
		{
			return;
		}

		MovementComponent->SetLocomotionStateSpeed_Implementation(State, Speed, SwimSpeed);

		if (AActor* OwnerActor = MovementComponent->GetOwner(); OwnerActor && !OwnerActor->HasAuthority())
		{
			MovementComponent->SetLocomotionStateSpeed(State, Speed, SwimSpeed);
		}
	}

	void ApplyAcfTargetLocomotionState(UACFCharacterMovementComponent* MovementComponent, const ELocomotionState State)
	{
		if (!MovementComponent)
		{
			return;
		}

		MovementComponent->SetLocomotionState_Implementation(State);

		if (AActor* OwnerActor = MovementComponent->GetOwner(); OwnerActor && !OwnerActor->HasAuthority())
		{
			MovementComponent->SetLocomotionState(State);
		}
	}
}

UProjectLocomotionOverrideComponent::UProjectLocomotionOverrideComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	OriginalRootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
	WalkLoopAnimation = TSoftObjectPtr<UAnimationAsset>(FEFProjectAssetPathResolver::BuildExportedAnimationPath(TEXT("Anim_KA_Walk04")));
	WalkPivotAnimation = TSoftObjectPtr<UAnimationAsset>(FEFProjectAssetPathResolver::BuildExportedAnimationPath(TEXT("Anim_KA_Walk04_Pivot")));
	WalkIdleAnimation = TSoftObjectPtr<UAnimationAsset>(FEFProjectAssetPathResolver::BuildExportedAnimationPath(TEXT("Anim_KA_Idle01_breathing")));
	CrawlEntryAnimation = TSoftObjectPtr<UAnimationAsset>(FEFProjectAssetPathResolver::BuildExportedAnimationPath(TEXT("Anim_KA_Crawling_Baby_Entry")));
	CrawlExitAnimation = TSoftObjectPtr<UAnimationAsset>(FEFProjectAssetPathResolver::BuildExportedAnimationPath(TEXT("Anim_KA_Crawling_Baby_Exit")));
	CrawlIdleAnimation = TSoftObjectPtr<UAnimationAsset>(FEFProjectAssetPathResolver::BuildExportedAnimationPath(TEXT("Anim_KA_Crawling_Baby_Idle")));
	CrawlForwardAnimation = TSoftObjectPtr<UAnimationAsset>(FEFProjectAssetPathResolver::BuildExportedAnimationPath(TEXT("Anim_KA_Crawling_Baby_Walk_Fwd")));
}

void UProjectLocomotionOverrideComponent::BeginPlay()
{
	Super::BeginPlay();

	ResolveDependencies();
	CacheOriginalMovementState();
	CacheOriginalRotationState();
}

void UProjectLocomotionOverrideComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetWalkModeEnabled(false);
	Super::EndPlay(EndPlayReason);
}

void UProjectLocomotionOverrideComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bWalkModeEnabled)
	{
		return;
	}

	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	UpdateTransitionMovementLock(CurrentTimeSeconds);
	UpdateControllerMoveInputIgnoreState();
	UpdateCrawlManualInput(DeltaTime);
	UpdateAnimationState(DeltaTime);
	ApplyDesiredMovementSpeed();
}

void UProjectLocomotionOverrideComponent::ToggleWalkMode()
{
	SetWalkModeEnabled(!bWalkModeEnabled);
}

void UProjectLocomotionOverrideComponent::SetWalkModeEnabled(bool bEnabled)
{
	if (bWalkModeEnabled == bEnabled)
	{
		return;
	}

	bWalkModeEnabled = bEnabled;
	if (!bWalkModeEnabled)
	{
		bCrawlModeEnabled = false;
	}

	ResolveDependencies();
	CacheOriginalMovementState();
	CacheOriginalRotationState();

	if (bWalkModeEnabled)
	{
		ApplyWalkModeJumpRestriction();
		bWasMovingLastTick = false;
		bWasCrawlingLastTick = false;
		MovementLockUntilTime = 0.f;
		AccumulatedMovingTurnDelta = 0.f;
		NextTurnActionAllowedTime = 0.f;
		LastMovementDirection = FVector::ZeroVector;
		bHadMovementDirectionLastTick = false;
		ApplyDesiredRotationBehavior();
		ApplyDesiredMovementSpeed();
		UpdateAnimationState(0.f);
		SetComponentTickEnabled(true);
	}
	else
	{
		RestoreWalkModeJumpRestriction();
		ReleaseTransitionMovementLock();
		ApplyDesiredRotationBehavior();
		UpdateControllerMoveInputIgnoreState();

		if (CharacterMovementComponent && bOriginalMovementStateCached)
		{
			if (UACFCharacterMovementComponent* AcfMovementComponent = Cast<UACFCharacterMovementComponent>(CharacterMovementComponent))
			{
				if (bOriginalAcfWalkStateCached)
				{
					ApplyAcfLocomotionStateSpeed(
						AcfMovementComponent,
						ELocomotionState::EWalk,
						OriginalAcfWalkStateSpeed,
						OriginalAcfWalkStateSwimSpeed);
				}

				if (bOriginalAcfTargetLocomotionStateCached)
				{
					ApplyAcfTargetLocomotionState(AcfMovementComponent, static_cast<ELocomotionState>(OriginalAcfTargetLocomotionStateValue));
				}
			}

			const float NormalMoveSpeed = ResolveEffectiveNormalMoveSpeed();
			CharacterMovementComponent->MaxWalkSpeed = NormalMoveSpeed;
			CharacterMovementComponent->MaxWalkSpeedCrouched = NormalMoveSpeed;
			LastLoggedAppliedTargetSpeed = -1.f;
		}

		RestoreAnimationState();
		ClearAnimationPlaybackState();
		SetComponentTickEnabled(false);
	}
}

void UProjectLocomotionOverrideComponent::ApplyWalkModeJumpRestriction()
{
	ACharacter* CharacterOwner = CachedCharacterOwner.Get();
	if (!CharacterOwner)
	{
		bWalkModeJumpRestricted = false;
		bCachedJumpMaxCountValid = false;
		return;
	}

	if (!bWalkModeJumpRestricted)
	{
		CachedJumpMaxCount = CharacterOwner->JumpMaxCount;
		bCachedJumpMaxCountValid = true;
	}

	CharacterOwner->StopJumping();
	CharacterOwner->JumpMaxCount = 0;
	bWalkModeJumpRestricted = true;
}

void UProjectLocomotionOverrideComponent::RestoreWalkModeJumpRestriction()
{
	if (!bWalkModeJumpRestricted)
	{
		return;
	}

	if (ACharacter* CharacterOwner = CachedCharacterOwner.Get())
	{
		if (bCachedJumpMaxCountValid)
		{
			CharacterOwner->JumpMaxCount = CachedJumpMaxCount;
		}

		CharacterOwner->StopJumping();
	}

	bWalkModeJumpRestricted = false;
	bCachedJumpMaxCountValid = false;
}

void UProjectLocomotionOverrideComponent::ToggleCrawlMode()
{
	if (!bWalkModeEnabled)
	{
		return;
	}

	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (IsMovementLockedByTransition(CurrentTimeSeconds))
	{
		return;
	}

	SetCrawlModeEnabled(!bCrawlModeEnabled);
}

void UProjectLocomotionOverrideComponent::SetCrawlModeEnabled(bool bEnabled)
{
	if (!bWalkModeEnabled)
	{
		bCrawlModeEnabled = false;
		return;
	}

	if (bCrawlModeEnabled == bEnabled)
	{
		return;
	}

	bCrawlModeEnabled = bEnabled;
	ApplyDesiredRotationBehavior();
	UpdateControllerMoveInputIgnoreState();
	ApplyDesiredMovementSpeed();
	UpdateAnimationState(0.f);
	SetComponentTickEnabled(true);
}

bool UProjectLocomotionOverrideComponent::IsWalkModeEnabled() const
{
	return bWalkModeEnabled;
}

bool UProjectLocomotionOverrideComponent::IsCrawlModeActive() const
{
	return bWalkModeEnabled && bCrawlModeEnabled;
}

bool UProjectLocomotionOverrideComponent::HasResolvedMovementComponent() const
{
	return CharacterMovementComponent != nullptr;
}

bool UProjectLocomotionOverrideComponent::HasResolvedSkeletalMeshComponent() const
{
	return SkeletalMeshComponent != nullptr;
}

bool UProjectLocomotionOverrideComponent::HasResolvedAnimInstance() const
{
	return ResolveAnimInstance() != nullptr;
}

FString UProjectLocomotionOverrideComponent::DescribeResolvedDependencies() const
{
	const AActor* Owner = GetOwner();
	const APawn* PawnOwner = CachedPawnOwner.Get();
	const ACharacter* CharacterOwner = CachedCharacterOwner.Get();
	const UAnimInstance* AnimInstance = ResolveAnimInstance();

	return FString::Printf(
		TEXT("owner=%s ownerClass=%s pawn=%s character=%s movement=%s mesh=%s animInstance=%s walk=%s crawl=%s tick=%s"),
		Owner ? *Owner->GetName() : TEXT("None"),
		Owner && Owner->GetClass() ? *Owner->GetClass()->GetName() : TEXT("None"),
		PawnOwner ? *PawnOwner->GetName() : TEXT("None"),
		CharacterOwner ? *CharacterOwner->GetName() : TEXT("None"),
		CharacterMovementComponent ? *CharacterMovementComponent->GetName() : TEXT("None"),
		SkeletalMeshComponent ? *SkeletalMeshComponent->GetName() : TEXT("None"),
		AnimInstance ? *AnimInstance->GetName() : TEXT("None"),
		bWalkModeEnabled ? TEXT("true") : TEXT("false"),
		bCrawlModeEnabled ? TEXT("true") : TEXT("false"),
		IsComponentTickEnabled() ? TEXT("true") : TEXT("false"));
}

float UProjectLocomotionOverrideComponent::GetCurrentResolvedWalkSpeed() const
{
	return GetCurrentDesiredMoveSpeed();
}

float UProjectLocomotionOverrideComponent::GetCurrentDesiredMoveSpeed() const
{
	if (!bWalkModeEnabled)
	{
		return ResolveEffectiveNormalMoveSpeed();
	}

	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (IsMovementLockedByTransition(CurrentTimeSeconds))
	{
		return 0.f;
	}

	return IsCrawlModeActive() ? ResolveCrawlSpeed() : ResolveWalkSpeed();
}

float UProjectLocomotionOverrideComponent::GetCurrentEffectiveNormalMoveSpeed() const
{
	return ResolveEffectiveNormalMoveSpeed();
}

bool UProjectLocomotionOverrideComponent::IsTransitionMovementLockActive() const
{
	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	return IsMovementLockedByTransition(CurrentTimeSeconds);
}

FString UProjectLocomotionOverrideComponent::GetCurrentAnimationAssetName() const
{
	return GetNameSafe(CurrentAnimationAsset);
}

FString UProjectLocomotionOverrideComponent::GetActiveOverlayMontageName() const
{
	return GetNameSafe(ActiveOverlayMontage);
}

void UProjectLocomotionOverrideComponent::ResolveDependencies()
{
	AActor* Owner = GetOwner();
	APawn* PawnOwner = Cast<APawn>(Owner);
	CachedPawnOwner = PawnOwner;
	CachedCharacterOwner = Cast<ACharacter>(Owner);
	CharacterMovementComponent = nullptr;
	SkeletalMeshComponent = nullptr;
	SinfulAscensionComponent = nullptr;

	if (!PawnOwner)
	{
		CachedPawnOwner = nullptr;
		CachedCharacterOwner = nullptr;
		return;
	}

	if (ACharacter* CharacterOwner = CachedCharacterOwner.Get())
	{
		CharacterMovementComponent = CharacterOwner->GetCharacterMovement();
		SkeletalMeshComponent = CharacterOwner->GetMesh();
	}

	if (!CharacterMovementComponent)
	{
		CharacterMovementComponent = Owner->FindComponentByClass<UCharacterMovementComponent>();
	}

	if (!SkeletalMeshComponent)
	{
		SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
	}

	SinfulAscensionComponent = Owner->FindComponentByClass<UProjectSinfulAscensionComponent>();

	if (CharacterMovementComponent)
	{
		AddTickPrerequisiteComponent(CharacterMovementComponent);
	}
}

void UProjectLocomotionOverrideComponent::CacheOriginalMovementState()
{
	if (bOriginalMovementStateCached || !CharacterMovementComponent)
	{
		return;
	}

	OriginalMaxWalkSpeed = CharacterMovementComponent->MaxWalkSpeed;
	OriginalMaxWalkSpeedCrouched = CharacterMovementComponent->MaxWalkSpeedCrouched;
	if (UACFCharacterMovementComponent* AcfMovementComponent = Cast<UACFCharacterMovementComponent>(CharacterMovementComponent))
	{
		OriginalAcfWalkStateSpeed = AcfMovementComponent->GetCharacterMaxSpeedByState(ELocomotionState::EWalk);
		OriginalAcfWalkStateSwimSpeed = AcfMovementComponent->GetCharacterMaxSwimSpeedByState(ELocomotionState::EWalk);
		OriginalAcfTargetLocomotionStateValue = static_cast<uint8>(AcfMovementComponent->GetTargetLocomotionState());
		bOriginalAcfWalkStateCached = true;
		bOriginalAcfTargetLocomotionStateCached = true;
	}
	bOriginalMovementStateCached = true;
}

void UProjectLocomotionOverrideComponent::CacheOriginalRotationState()
{
	ACharacter* CharacterOwner = CachedCharacterOwner.Get();
	if (bOriginalRotationStateCached || !CharacterMovementComponent || !CharacterOwner)
	{
		return;
	}

	bOriginalUseControllerRotationYaw = CharacterOwner->bUseControllerRotationYaw;
	bOriginalUseControllerDesiredRotation = CharacterMovementComponent->bUseControllerDesiredRotation;
	bOriginalOrientRotationToMovement = CharacterMovementComponent->bOrientRotationToMovement;
	bOriginalRotationStateCached = true;
}

void UProjectLocomotionOverrideComponent::ApplyDesiredRotationBehavior()
{
	ACharacter* CharacterOwner = CachedCharacterOwner.Get();
	if (!CharacterMovementComponent || !CharacterOwner)
	{
		return;
	}

	CacheOriginalRotationState();
	if (!bOriginalRotationStateCached)
	{
		return;
	}

	if (!bWalkModeEnabled)
	{
		CharacterOwner->bUseControllerRotationYaw = bOriginalUseControllerRotationYaw;
		CharacterMovementComponent->bOrientRotationToMovement = bOriginalOrientRotationToMovement;
		CharacterMovementComponent->bUseControllerDesiredRotation = bOriginalUseControllerDesiredRotation;
		return;
	}

	CharacterOwner->bUseControllerRotationYaw = false;
	CharacterMovementComponent->bUseControllerDesiredRotation = false;

	if (IsCrawlModeActive())
	{
		CharacterMovementComponent->bOrientRotationToMovement = false;
		return;
	}

	CharacterMovementComponent->bOrientRotationToMovement = true;
}

APlayerController* UProjectLocomotionOverrideComponent::ResolveOwningPlayerController() const
{
	return CachedPawnOwner.IsValid() ? Cast<APlayerController>(CachedPawnOwner->GetController()) : nullptr;
}

void UProjectLocomotionOverrideComponent::UpdateControllerMoveInputIgnoreState()
{
	APlayerController* PlayerController = ResolveOwningPlayerController();
	const bool bShouldIgnoreMoveInput = bWalkModeEnabled && (IsCrawlModeActive() || bTransitionMovementLockApplied);

	if (MoveInputIgnoredPlayerController.IsValid() && MoveInputIgnoredPlayerController.Get() != PlayerController && bControllerMoveInputIgnored)
	{
		MoveInputIgnoredPlayerController->SetIgnoreMoveInput(false);
		bControllerMoveInputIgnored = false;
	}

	MoveInputIgnoredPlayerController = PlayerController;
	if (!PlayerController)
	{
		return;
	}

	if (bShouldIgnoreMoveInput != bControllerMoveInputIgnored)
	{
		PlayerController->SetIgnoreMoveInput(bShouldIgnoreMoveInput);
		bControllerMoveInputIgnored = bShouldIgnoreMoveInput;
	}
}

void UProjectLocomotionOverrideComponent::UpdateCrawlManualInput(float DeltaTime)
{
	if (!bWalkModeEnabled || !IsCrawlModeActive())
	{
		return;
	}

	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (IsMovementLockedByTransition(CurrentTimeSeconds))
	{
		if (CachedPawnOwner.IsValid())
		{
			CachedPawnOwner->ConsumeMovementInputVector();
		}
		return;
	}

	APawn* PawnOwner = CachedPawnOwner.Get();
	APlayerController* PlayerController = ResolveOwningPlayerController();
	if (!PawnOwner || !PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	PawnOwner->ConsumeMovementInputVector();

	float ForwardInput = 0.f;
	float TurnInput = 0.f;
	if (PlayerController->IsInputKeyDown(EKeys::W))
	{
		ForwardInput += 1.f;
	}
	if (PlayerController->IsInputKeyDown(EKeys::S))
	{
		ForwardInput -= 1.f;
	}
	if (PlayerController->IsInputKeyDown(EKeys::D))
	{
		TurnInput += 1.f;
	}
	if (PlayerController->IsInputKeyDown(EKeys::A))
	{
		TurnInput -= 1.f;
	}

	if (!FMath::IsNearlyZero(TurnInput))
	{
		PawnOwner->AddActorWorldRotation(FRotator(0.f, TurnInput * CrawlTurnRateDegreesPerSecond * DeltaTime, 0.f));
	}

	if (!FMath::IsNearlyZero(ForwardInput))
	{
		PawnOwner->AddMovementInput(PawnOwner->GetActorForwardVector(), ForwardInput, true);
	}
}

void UProjectLocomotionOverrideComponent::ApplyDesiredMovementSpeed()
{
	if (!CharacterMovementComponent || !bOriginalMovementStateCached)
	{
		return;
	}

	if (!bWalkModeEnabled)
	{
		const float NormalMoveSpeed = ResolveEffectiveNormalMoveSpeed();
		CharacterMovementComponent->MaxWalkSpeed = NormalMoveSpeed;
		CharacterMovementComponent->MaxWalkSpeedCrouched = NormalMoveSpeed;
		return;
	}

	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	UpdateTransitionMovementLock(CurrentTimeSeconds);
	if (IsMovementLockedByTransition(CurrentTimeSeconds))
	{
		CharacterMovementComponent->MaxWalkSpeed = 0.f;
		CharacterMovementComponent->MaxWalkSpeedCrouched = 0.f;
		return;
	}

	const float TargetSpeed = IsCrawlModeActive() ? ResolveCrawlSpeed() : ResolveWalkSpeed();
	if (UACFCharacterMovementComponent* AcfMovementComponent = Cast<UACFCharacterMovementComponent>(CharacterMovementComponent))
	{
		const bool bNeedsAcfSpeedRefresh = !FMath::IsNearlyEqual(LastLoggedAppliedTargetSpeed, TargetSpeed, KINDA_SMALL_NUMBER)
			|| AcfMovementComponent->GetTargetLocomotionState() != ELocomotionState::EWalk;
		if (bNeedsAcfSpeedRefresh)
		{
			const float WalkSwimSpeed = bOriginalAcfWalkStateCached
				? OriginalAcfWalkStateSwimSpeed
				: AcfMovementComponent->GetCharacterMaxSwimSpeedByState(ELocomotionState::EWalk);
			ApplyAcfLocomotionStateSpeed(AcfMovementComponent, ELocomotionState::EWalk, TargetSpeed, WalkSwimSpeed);
			if (AcfMovementComponent->GetTargetLocomotionState() != ELocomotionState::EWalk)
			{
				ApplyAcfTargetLocomotionState(AcfMovementComponent, ELocomotionState::EWalk);
			}
		}
	}

	CharacterMovementComponent->MaxWalkSpeed = TargetSpeed;
	CharacterMovementComponent->MaxWalkSpeedCrouched = TargetSpeed;
	LastLoggedAppliedTargetSpeed = TargetSpeed;
}

float UProjectLocomotionOverrideComponent::ResolveEffectiveNormalMoveSpeed() const
{
	if (SinfulAscensionComponent)
	{
		if (bOriginalMovementStateCached)
		{
			return FMath::Max(0.f, OriginalMaxWalkSpeed + SinfulAscensionComponent->GetEffectiveMoveSpeedBonus());
		}

		const float SinfulMoveSpeed = SinfulAscensionComponent->GetEffectiveNormalMoveSpeed();
		if (SinfulMoveSpeed > 0.f)
		{
			return SinfulMoveSpeed;
		}
	}

	if (bOriginalMovementStateCached)
	{
		return OriginalMaxWalkSpeed;
	}

	return CharacterMovementComponent ? CharacterMovementComponent->MaxWalkSpeed : 0.f;
}

float UProjectLocomotionOverrideComponent::ResolveWalkSpeed() const
{
	const float BaseSpeed = FMath::Max(ResolveEffectiveNormalMoveSpeed(), MinimumWalkSpeed);
	return FMath::Max(MinimumWalkSpeed, BaseSpeed * WalkSpeedMultiplier);
}

float UProjectLocomotionOverrideComponent::ResolveCrawlSpeed() const
{
	const float BaseSpeed = FMath::Max(ResolveEffectiveNormalMoveSpeed(), MinimumCrawlSpeed);
	return FMath::Max(MinimumCrawlSpeed, BaseSpeed * CrawlSpeedMultiplier);
}

void UProjectLocomotionOverrideComponent::UpdateAnimationState(float DeltaTime)
{
	(void)DeltaTime;

	ResolveDependencies();
	if (!SkeletalMeshComponent || !ResolveAnimInstance())
	{
		return;
	}

	const FVector Velocity = CachedPawnOwner.IsValid() ? CachedPawnOwner->GetVelocity() : FVector::ZeroVector;
	const FVector MovementIntent = ResolveMovementIntentVector(Velocity);
	const bool bMoving = MovementIntent.Size2D() > MovementActivationThreshold || Velocity.Size2D() > MovementActivationThreshold;
	const bool bCrawling = IsCrawlModeActive();
	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	const float MovementDirectionDeltaDegrees = ConsumeMovementDirectionDeltaDegrees(Velocity, bMoving);
	UAnimationAsset* DesiredLoopAnimation = ResolveDesiredLoopAnimation(bCrawling, bMoving);
	UAnimationAsset* CrawlEntryAsset = LoadAnimationAsset(CrawlEntryAnimation);
	UAnimationAsset* CrawlExitAsset = LoadAnimationAsset(CrawlExitAnimation);
	const bool bIsCrawlEntryAnimationActive = CurrentAnimationAsset == CrawlEntryAsset;
	const bool bIsCrawlExitOneShotPlaying = CurrentAnimationAsset == CrawlExitAsset && IsOneShotPlaying(CurrentTimeSeconds);

	if (bIsCrawlExitOneShotPlaying)
	{
		PendingLoopAnimationAsset = DesiredLoopAnimation;
		FlushRootMotionState();
		bWasMovingLastTick = bMoving;
		bWasCrawlingLastTick = bCrawling;
		return;
	}

	if (bIsCrawlEntryAnimationActive && HandleHeldCrawlEntryPose(CurrentTimeSeconds, DesiredLoopAnimation, bMoving, bCrawling))
	{
		return;
	}

	if (!bCrawling && !bMoving && bCrawling == bWasCrawlingLastTick)
	{
		AccumulatedMovingTurnDelta = 0.f;
		PendingLoopAnimationAsset = nullptr;

		if (IsOneShotPlaying(CurrentTimeSeconds) || CurrentAnimationAsset != DesiredLoopAnimation)
		{
			StopOverlayPlayback(TransitionBlendOutTime);
			OneShotEndTimeSeconds = 0.f;
			CurrentAnimationAsset = nullptr;
			bCurrentAnimationLooping = false;
		}

		PlayAnimationAsset(DesiredLoopAnimation, true);
		FlushRootMotionState();
		bWasMovingLastTick = false;
		bWasCrawlingLastTick = bCrawling;
		return;
	}

	if (IsOneShotPlaying(CurrentTimeSeconds) && ShouldInterruptCurrentOneShot(DesiredLoopAnimation, bCrawling, bMoving))
	{
		StopOverlayPlayback(TransitionBlendOutTime);
		OneShotEndTimeSeconds = 0.f;
		CurrentAnimationAsset = nullptr;
		bCurrentAnimationLooping = false;
	}

	if (bCrawling != bWasCrawlingLastTick)
	{
		if (bCrawling)
		{
			PlayCrawlEntryAnimation(DesiredLoopAnimation);
		}
		else
		{
			PlayOneShotAnimation(LoadAnimationAsset(CrawlExitAnimation), DesiredLoopAnimation);
		}
	}
	else if (IsOneShotPlaying(CurrentTimeSeconds))
	{
		PendingLoopAnimationAsset = DesiredLoopAnimation;
	}
	else
	{
		if (PendingLoopAnimationAsset)
		{
			PlayAnimationAsset(PendingLoopAnimationAsset, true);
			PendingLoopAnimationAsset = nullptr;
		}
		else if (!bCrawling && bMoving != bWasMovingLastTick)
		{
			AccumulatedMovingTurnDelta = 0.f;
			PendingLoopAnimationAsset = nullptr;
			PlayAnimationAsset(DesiredLoopAnimation, true);
		}
		else if (!bCrawling && TryPlayWalkPivot(CurrentTimeSeconds, bMoving, MovementDirectionDeltaDegrees, DesiredLoopAnimation))
		{
		}
		else
		{
			PlayAnimationAsset(DesiredLoopAnimation, true);
		}
	}

	FlushRootMotionState();
	bWasMovingLastTick = bMoving;
	bWasCrawlingLastTick = bCrawling;
}

void UProjectLocomotionOverrideComponent::EnterAnimationOverlayMode()
{
	ResolveDependencies();
	UAnimInstance* AnimInstance = ResolveAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (!bOriginalRootMotionModeCached || ActiveAnimInstance.Get() != AnimInstance)
	{
		OriginalRootMotionMode = AnimInstance->RootMotionMode;
		ActiveAnimInstance = AnimInstance;
		bOriginalRootMotionModeCached = true;
	}

	if (AnimInstance->RootMotionMode != ERootMotionMode::NoRootMotionExtraction)
	{
		AnimInstance->SetRootMotionMode(ERootMotionMode::NoRootMotionExtraction);
	}

	FlushRootMotionState();
}

void UProjectLocomotionOverrideComponent::RestoreAnimationState()
{
	UAnimInstance* AnimInstance = ActiveAnimInstance.IsValid() ? ActiveAnimInstance.Get() : ResolveAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	StopOverlayPlayback(TransitionBlendOutTime);

	if (bOriginalRootMotionModeCached)
	{
		AnimInstance->SetRootMotionMode(OriginalRootMotionMode);
	}

	FlushRootMotionState();
	ActiveAnimInstance = nullptr;
	ActiveOverlayMontage = nullptr;
	ActiveCrawlEntryMontage = nullptr;
	bOriginalRootMotionModeCached = false;
	bHoldingCrawlEntryFinalPose = false;
	CurrentAnimationAsset = nullptr;
	CrawlEntryPoseHoldUntilTime = 0.f;
	bCurrentAnimationLooping = false;
}

void UProjectLocomotionOverrideComponent::PlayAnimationAsset(UAnimationAsset* AnimationAsset, bool bLooping)
{
	if (!IsValid(AnimationAsset) || !SkeletalMeshComponent)
	{
		return;
	}

	UAnimSequenceBase* SequenceAsset = Cast<UAnimSequenceBase>(AnimationAsset);
	UAnimInstance* AnimInstance = ResolveAnimInstance();
	if (!SequenceAsset || !AnimInstance)
	{
		return;
	}

	if (CurrentAnimationAsset == AnimationAsset && bCurrentAnimationLooping == bLooping && !IsOneShotPlaying(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f))
	{
		return;
	}

	EnterAnimationOverlayMode();
	AnimInstance = ResolveAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	const bool bIsCrawlEntryLoopHandoff = CurrentAnimationAsset == CrawlEntryAnimation.Get() && IsCrawlLoopAnimation(AnimationAsset);
	const bool bSeamlessCrawlLoopHandoff = ShouldUseSeamlessCrawlLoopHandoff(AnimationAsset);
	const float BlendInTime = bSeamlessCrawlLoopHandoff ? 0.f : (bLooping ? LoopBlendInTime : TransitionBlendInTime);
	const float BlendOutTime = bSeamlessCrawlLoopHandoff ? 0.f : (bLooping ? LoopBlendOutTime : TransitionBlendOutTime);
	const float AnimationStartOffset = ResolveAnimationStartOffset(AnimationAsset, bIsCrawlEntryLoopHandoff);

	StopOverlayPlayback(BlendOutTime);
	const FMontageBlendSettings BlendInSettings(BlendInTime);
	const FMontageBlendSettings BlendOutSettings(BlendOutTime);
	ActiveOverlayMontage = PlayDynamicSlotAnimation(
		SequenceAsset,
		BlendInSettings,
		BlendOutSettings,
		1.f,
		bLooping ? MAX_int32 : 1,
		true);
	if (!ActiveOverlayMontage)
	{
		return;
	}

	if (AnimationStartOffset > KINDA_SMALL_NUMBER)
	{
		AnimInstance->Montage_SetPosition(ActiveOverlayMontage, FMath::Min(AnimationStartOffset, GetAnimationDuration(AnimationAsset)));
	}

	FlushRootMotionState();
	CurrentAnimationAsset = AnimationAsset;
	bCurrentAnimationLooping = bLooping;
	OneShotEndTimeSeconds = 0.f;
}

void UProjectLocomotionOverrideComponent::PlayOneShotAnimation(UAnimationAsset* AnimationAsset, UAnimationAsset* NextLoopAnimation)
{
	if (!IsValid(AnimationAsset))
	{
		PendingLoopAnimationAsset = NextLoopAnimation;
		if (PendingLoopAnimationAsset)
		{
			PlayAnimationAsset(PendingLoopAnimationAsset, true);
			PendingLoopAnimationAsset = nullptr;
		}
		return;
	}

	UAnimSequenceBase* SequenceAsset = Cast<UAnimSequenceBase>(AnimationAsset);
	UAnimInstance* AnimInstance = ResolveAnimInstance();
	if (!SequenceAsset || !AnimInstance)
	{
		return;
	}

	EnterAnimationOverlayMode();
	AnimInstance = ResolveAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	StopOverlayPlayback(TransitionBlendOutTime);
	const float OneShotHoldTime = ResolveOneShotHoldTime(AnimationAsset);
	float PlayRate = 1.f;
	const float AnimationDuration = GetAnimationDuration(AnimationAsset);
	if (AnimationDuration > KINDA_SMALL_NUMBER && OneShotHoldTime > KINDA_SMALL_NUMBER && IsCrawlTransitionAnimation(AnimationAsset))
	{
		PlayRate = AnimationDuration / OneShotHoldTime;
	}

	const FMontageBlendSettings BlendInSettings(TransitionBlendInTime);
	const FMontageBlendSettings BlendOutSettings(TransitionBlendOutTime);
	ActiveOverlayMontage = PlayDynamicSlotAnimation(
		SequenceAsset,
		BlendInSettings,
		BlendOutSettings,
		PlayRate,
		1,
		true);
	if (!ActiveOverlayMontage)
	{
		return;
	}

	FlushRootMotionState();
	CurrentAnimationAsset = AnimationAsset;
	bCurrentAnimationLooping = false;
	PendingLoopAnimationAsset = NextLoopAnimation;
	AccumulatedMovingTurnDelta = 0.f;
	OneShotEndTimeSeconds = (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f) + OneShotHoldTime;
	if (IsCrawlTransitionAnimation(AnimationAsset))
	{
		MovementLockUntilTime = OneShotEndTimeSeconds;
		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->StopMovementImmediately();
		}
	}
	else
	{
		MovementLockUntilTime = 0.f;
	}
}

void UProjectLocomotionOverrideComponent::PlayCrawlEntryAnimation(UAnimationAsset* NextLoopAnimation)
{
	UAnimationAsset* AnimationAsset = LoadAnimationAsset(CrawlEntryAnimation);
	if (!IsValid(AnimationAsset))
	{
		PendingLoopAnimationAsset = NextLoopAnimation;
		if (PendingLoopAnimationAsset)
		{
			PlayAnimationAsset(PendingLoopAnimationAsset, true);
			PendingLoopAnimationAsset = nullptr;
		}
		return;
	}

	UAnimSequenceBase* SequenceAsset = Cast<UAnimSequenceBase>(AnimationAsset);
	UAnimInstance* AnimInstance = ResolveAnimInstance();
	if (!SequenceAsset || !AnimInstance)
	{
		return;
	}

	EnterAnimationOverlayMode();
	AnimInstance = ResolveAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	StopOverlayPlayback(TransitionBlendOutTime);
	const float OneShotHoldTime = ResolveOneShotHoldTime(AnimationAsset);
	const float AnimationDuration = GetAnimationDuration(AnimationAsset);
	float PlayRate = 1.f;
	if (AnimationDuration > KINDA_SMALL_NUMBER && OneShotHoldTime > KINDA_SMALL_NUMBER)
	{
		PlayRate = AnimationDuration / OneShotHoldTime;
	}

	const FMontageBlendSettings BlendInSettings(TransitionBlendInTime);
	const FMontageBlendSettings BlendOutSettings(TransitionBlendOutTime);
	ActiveOverlayMontage = PlayDynamicSlotAnimation(
		SequenceAsset,
		BlendInSettings,
		BlendOutSettings,
		PlayRate,
		1,
		false);
	if (!ActiveOverlayMontage)
	{
		return;
	}

	FlushRootMotionState();
	CurrentAnimationAsset = AnimationAsset;
	bCurrentAnimationLooping = false;
	PendingLoopAnimationAsset = NextLoopAnimation;
	AccumulatedMovingTurnDelta = 0.f;
	OneShotEndTimeSeconds = (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f) + OneShotHoldTime;
	CrawlEntryPoseHoldUntilTime = 0.f;
	bHoldingCrawlEntryFinalPose = false;
	ActiveCrawlEntryMontage = ActiveOverlayMontage;
	MovementLockUntilTime = OneShotEndTimeSeconds + FMath::Max(CrawlEntryPoseHoldDuration, 0.f);
	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->StopMovementImmediately();
	}
}

bool UProjectLocomotionOverrideComponent::HandleHeldCrawlEntryPose(float CurrentTimeSeconds, UAnimationAsset* DesiredLoopAnimation, bool bMoving, bool bCrawling)
{
	PendingLoopAnimationAsset = DesiredLoopAnimation;
	if (IsOneShotPlaying(CurrentTimeSeconds))
	{
		FlushRootMotionState();
		bWasMovingLastTick = bMoving;
		bWasCrawlingLastTick = bCrawling;
		return true;
	}

	if (!bHoldingCrawlEntryFinalPose)
	{
		bHoldingCrawlEntryFinalPose = true;
		CrawlEntryPoseHoldUntilTime = CurrentTimeSeconds + FMath::Max(CrawlEntryPoseHoldDuration, 0.f);
		MovementLockUntilTime = FMath::Max(MovementLockUntilTime, CrawlEntryPoseHoldUntilTime);

		if (UAnimInstance* AnimInstance = ResolveAnimInstance())
		{
			if (ActiveCrawlEntryMontage)
			{
				const float AnimationDuration = GetAnimationDuration(CurrentAnimationAsset);
				AnimInstance->Montage_SetPosition(ActiveCrawlEntryMontage, FMath::Max(0.f, AnimationDuration - KINDA_SMALL_NUMBER));
			}
		}

		FlushRootMotionState();
		bWasMovingLastTick = bMoving;
		bWasCrawlingLastTick = bCrawling;
		return true;
	}

	if (CurrentTimeSeconds < CrawlEntryPoseHoldUntilTime)
	{
		FlushRootMotionState();
		bWasMovingLastTick = bMoving;
		bWasCrawlingLastTick = bCrawling;
		return true;
	}

	const float HandoffBlendTime = FMath::Clamp(TransitionBlendOutTime, 0.05f, 0.08f);
	const FMontageBlendSettings HandoffBlendSettings(HandoffBlendTime);
	if (UAnimInstance* AnimInstance = ResolveAnimInstance())
	{
		if (ActiveCrawlEntryMontage)
		{
			AnimInstance->Montage_StopWithBlendSettings(HandoffBlendSettings, ActiveCrawlEntryMontage);
		}
		else
		{
			StopOverlayPlayback(HandoffBlendTime);
		}
	}

	ActiveCrawlEntryMontage = nullptr;
	bHoldingCrawlEntryFinalPose = false;
	CrawlEntryPoseHoldUntilTime = 0.f;
	OneShotEndTimeSeconds = 0.f;
	CurrentAnimationAsset = nullptr;
	bCurrentAnimationLooping = false;

	PlayAnimationAsset(DesiredLoopAnimation, true);
	if (DesiredLoopAnimation == LoadAnimationAsset(CrawlIdleAnimation))
	{
		if (UAnimInstance* AnimInstance = ResolveAnimInstance())
		{
			if (ActiveOverlayMontage)
			{
				AnimInstance->Montage_SetPosition(ActiveOverlayMontage, FMath::Min(CrawlIdleHandoffStartOffsetSeconds, GetAnimationDuration(DesiredLoopAnimation)));
			}
		}
	}

	PendingLoopAnimationAsset = nullptr;
	FlushRootMotionState();
	bWasMovingLastTick = bMoving;
	bWasCrawlingLastTick = bCrawling;
	return true;
}

void UProjectLocomotionOverrideComponent::ClearAnimationPlaybackState()
{
	ReleaseTransitionMovementLock();
	CurrentAnimationAsset = nullptr;
	PendingLoopAnimationAsset = nullptr;
	ActiveOverlayMontage = nullptr;
	ActiveCrawlEntryMontage = nullptr;
	ActiveAnimInstance = nullptr;
	OneShotEndTimeSeconds = 0.f;
	MovementLockUntilTime = 0.f;
	CrawlEntryPoseHoldUntilTime = 0.f;
	AccumulatedMovingTurnDelta = 0.f;
	NextTurnActionAllowedTime = 0.f;
	bHoldingCrawlEntryFinalPose = false;
	bCurrentAnimationLooping = false;
	bHadMovementDirectionLastTick = false;
	bWasMovingLastTick = false;
	bWasCrawlingLastTick = false;
	bOriginalRootMotionModeCached = false;
	LastMovementDirection = FVector::ZeroVector;
	UpdateControllerMoveInputIgnoreState();
}

bool UProjectLocomotionOverrideComponent::IsOneShotPlaying(float CurrentTimeSeconds) const
{
	return OneShotEndTimeSeconds > CurrentTimeSeconds && !bCurrentAnimationLooping;
}

bool UProjectLocomotionOverrideComponent::IsCrawlLoopAnimation(const UAnimationAsset* AnimationAsset) const
{
	return AnimationAsset && (AnimationAsset == CrawlIdleAnimation.Get() || AnimationAsset == CrawlForwardAnimation.Get());
}

bool UProjectLocomotionOverrideComponent::ShouldUseSeamlessCrawlLoopHandoff(const UAnimationAsset* NextAnimationAsset) const
{
	return !bHoldingCrawlEntryFinalPose && CurrentAnimationAsset == CrawlEntryAnimation.Get() && IsCrawlLoopAnimation(NextAnimationAsset);
}

float UProjectLocomotionOverrideComponent::ResolveAnimationStartOffset(const UAnimationAsset* AnimationAsset, bool bSeamlessCrawlLoopHandoff) const
{
	if (!bSeamlessCrawlLoopHandoff || AnimationAsset != CrawlIdleAnimation.Get())
	{
		return 0.f;
	}

	return CrawlIdleHandoffStartOffsetSeconds;
}

bool UProjectLocomotionOverrideComponent::HasSignificantMovement() const
{
	return CachedPawnOwner.IsValid() && CachedPawnOwner->GetVelocity().Size2D() > MovementActivationThreshold;
}

FVector UProjectLocomotionOverrideComponent::ResolveMovementIntentVector(const FVector& Velocity) const
{
	FVector IntentVector = CachedPawnOwner.IsValid() ? CachedPawnOwner->GetPendingMovementInputVector() : FVector::ZeroVector;
	if (IntentVector.SizeSquared2D() <= KINDA_SMALL_NUMBER)
	{
		IntentVector = CharacterMovementComponent ? CharacterMovementComponent->GetCurrentAcceleration() : FVector::ZeroVector;
	}
	if (IntentVector.SizeSquared2D() <= KINDA_SMALL_NUMBER)
	{
		IntentVector = Velocity;
	}

	IntentVector.Z = 0.f;
	return IntentVector;
}

float UProjectLocomotionOverrideComponent::ConsumeMovementDirectionDeltaDegrees(const FVector& Velocity, bool bMoving)
{
	if (!bMoving)
	{
		LastMovementDirection = FVector::ZeroVector;
		bHadMovementDirectionLastTick = false;
		return 0.f;
	}

	FVector CurrentDirection = ResolveMovementIntentVector(Velocity);
	CurrentDirection.Z = 0.f;
	CurrentDirection.Normalize();
	if (!bHadMovementDirectionLastTick)
	{
		LastMovementDirection = CurrentDirection;
		bHadMovementDirectionLastTick = true;
		return 0.f;
	}

	const float DotProduct = FMath::Clamp(FVector::DotProduct(LastMovementDirection, CurrentDirection), -1.f, 1.f);
	float DeltaDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
	const float CrossZ = FVector::CrossProduct(LastMovementDirection, CurrentDirection).Z;
	if (CrossZ < 0.f)
	{
		DeltaDegrees *= -1.f;
	}

	LastMovementDirection = CurrentDirection;
	return DeltaDegrees;
}

bool UProjectLocomotionOverrideComponent::TryPlayWalkPivot(float CurrentTimeSeconds, bool bMoving, float MovementDirectionDeltaDegrees, UAnimationAsset* DesiredLoopAnimation)
{
	if (CurrentTimeSeconds < NextTurnActionAllowedTime)
	{
		return false;
	}

	if (!bMoving)
	{
		AccumulatedMovingTurnDelta = 0.f;
		return false;
	}

	if (FMath::Abs(MovementDirectionDeltaDegrees) < WalkPivotNoiseDeadZoneDegrees)
	{
		AccumulatedMovingTurnDelta = 0.f;
		return false;
	}

	if (!FMath::IsNearlyZero(AccumulatedMovingTurnDelta) && FMath::Sign(AccumulatedMovingTurnDelta) != FMath::Sign(MovementDirectionDeltaDegrees))
	{
		AccumulatedMovingTurnDelta = 0.f;
	}

	AccumulatedMovingTurnDelta += MovementDirectionDeltaDegrees;
	if (FMath::Abs(AccumulatedMovingTurnDelta) < PivotTriggerAngle)
	{
		return false;
	}

	PlayOneShotAnimation(LoadAnimationAsset(WalkPivotAnimation), DesiredLoopAnimation);
	AccumulatedMovingTurnDelta = 0.f;
	NextTurnActionAllowedTime = CurrentTimeSeconds + PivotActionCooldown;
	return true;
}

UAnimInstance* UProjectLocomotionOverrideComponent::ResolveAnimInstance() const
{
	return SkeletalMeshComponent ? SkeletalMeshComponent->GetAnimInstance() : nullptr;
}

UAnimMontage* UProjectLocomotionOverrideComponent::PlayDynamicSlotAnimation(
	UAnimSequenceBase* SequenceAsset,
	const FMontageBlendSettings& BlendInSettings,
	const FMontageBlendSettings& BlendOutSettings,
	float PlayRate,
	int32 LoopCount,
	bool bEnableAutoBlendOut,
	float BlendOutTriggerTime,
	float StartTimeSeconds)
{
	UAnimInstance* AnimInstance = ResolveAnimInstance();
	if (!SequenceAsset || !AnimInstance)
	{
		return nullptr;
	}

	UAnimMontage* DynamicMontage = UAnimMontage::CreateSlotAnimationAsDynamicMontage_WithBlendSettings(
		SequenceAsset,
		OverlaySlotName,
		BlendInSettings,
		BlendOutSettings,
		PlayRate,
		LoopCount,
		BlendOutTriggerTime);
	if (!DynamicMontage)
	{
		return nullptr;
	}

	DynamicMontage->bEnableAutoBlendOut = bEnableAutoBlendOut;
	const float PlayResult = AnimInstance->Montage_PlayWithBlendSettings(
		DynamicMontage,
		BlendInSettings,
		PlayRate,
		EMontagePlayReturnType::MontageLength,
		StartTimeSeconds);
	return PlayResult > 0.f ? DynamicMontage : nullptr;
}

UAnimationAsset* UProjectLocomotionOverrideComponent::LoadAnimationAsset(const TSoftObjectPtr<UAnimationAsset>& AssetReference)
{
	if (AssetReference.IsNull())
	{
		return nullptr;
	}

	const FSoftObjectPath AssetPath = AssetReference.ToSoftObjectPath();
	if (TObjectPtr<UAnimationAsset>* ExistingAsset = LoadedAnimationAssets.Find(AssetPath))
	{
		if (UAnimationAsset* CachedAsset = ExistingAsset->Get(); IsValid(CachedAsset))
		{
			return CachedAsset;
		}
		LoadedAnimationAssets.Remove(AssetPath);
	}

	UAnimationAsset* LoadedAsset = FEFProjectAssetPathResolver::LoadObjectWithLegacyFallback<UAnimationAsset>(AssetPath);
	if (LoadedAsset)
	{
		ApplyWalkModeRootMotionSettings(LoadedAsset);
		LoadedAnimationAssets.Add(AssetPath, LoadedAsset);
	}

	return LoadedAsset;
}

void UProjectLocomotionOverrideComponent::ApplyWalkModeRootMotionSettings(UAnimationAsset* AnimationAsset)
{
	UAnimSequence* AnimSequence = Cast<UAnimSequence>(AnimationAsset);
	if (!AnimSequence)
	{
		return;
	}

	const bool bNeedsUpdate =
		!AnimSequence->bEnableRootMotion ||
		AnimSequence->RootMotionRootLock != ERootMotionRootLock::Zero ||
		!AnimSequence->bForceRootLock ||
		!AnimSequence->bUseNormalizedRootMotionScale;

	if (!bNeedsUpdate)
	{
		return;
	}

#if WITH_EDITOR
	AnimSequence->Modify();
#endif
	AnimSequence->bEnableRootMotion = true;
	AnimSequence->RootMotionRootLock = ERootMotionRootLock::Zero;
	AnimSequence->bForceRootLock = true;
	AnimSequence->bUseNormalizedRootMotionScale = true;

#if WITH_EDITOR
	AnimSequence->PostEditChange();
	AnimSequence->MarkPackageDirty();
#endif
}

UAnimationAsset* UProjectLocomotionOverrideComponent::ResolveDesiredWalkLoopAnimation(bool bMoving)
{
	return bMoving ? LoadAnimationAsset(WalkLoopAnimation) : LoadAnimationAsset(WalkIdleAnimation);
}

UAnimationAsset* UProjectLocomotionOverrideComponent::ResolveDesiredCrawlLoopAnimation() const
{
	return const_cast<UProjectLocomotionOverrideComponent*>(this)->LoadAnimationAsset(CrawlForwardAnimation);
}

UAnimationAsset* UProjectLocomotionOverrideComponent::ResolveDesiredLoopAnimation(bool bCrawling, bool bMoving)
{
	if (bCrawling)
	{
		return bMoving ? ResolveDesiredCrawlLoopAnimation() : LoadAnimationAsset(CrawlIdleAnimation);
	}

	return ResolveDesiredWalkLoopAnimation(bMoving);
}

bool UProjectLocomotionOverrideComponent::ShouldInterruptCurrentOneShot(UAnimationAsset* DesiredLoopAnimation, bool bCrawling, bool bMoving) const
{
	if (IsCrawlTransitionAnimation(CurrentAnimationAsset) && IsMovementLockedByTransition(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f))
	{
		return false;
	}

	if (!PendingLoopAnimationAsset || PendingLoopAnimationAsset != DesiredLoopAnimation)
	{
		return true;
	}

	if (bCrawling != bWasCrawlingLastTick)
	{
		return true;
	}

	return !bCrawling && (bMoving != bWasMovingLastTick);
}

void UProjectLocomotionOverrideComponent::StopOverlayPlayback(float BlendOutTime)
{
	UAnimInstance* AnimInstance = ActiveAnimInstance.IsValid() ? ActiveAnimInstance.Get() : ResolveAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (ActiveOverlayMontage)
	{
		AnimInstance->Montage_Stop(BlendOutTime, ActiveOverlayMontage);
	}

	AnimInstance->StopSlotAnimation(BlendOutTime, OverlaySlotName);
	ActiveOverlayMontage = nullptr;
	ActiveCrawlEntryMontage = nullptr;
	bHoldingCrawlEntryFinalPose = false;
	CrawlEntryPoseHoldUntilTime = 0.f;
}

void UProjectLocomotionOverrideComponent::FlushRootMotionState()
{
	if (bWalkModeEnabled)
	{
		if (UAnimInstance* AnimInstance = ActiveAnimInstance.IsValid() ? ActiveAnimInstance.Get() : ResolveAnimInstance())
		{
			AnimInstance->SetRootMotionMode(ERootMotionMode::NoRootMotionExtraction);
		}
	}

	if (!SkeletalMeshComponent)
	{
		return;
	}

	SkeletalMeshComponent->ConsumeRootMotion();
}

float UProjectLocomotionOverrideComponent::GetAnimationDuration(UAnimationAsset* AnimationAsset) const
{
	if (const UAnimSequenceBase* SequenceBase = Cast<UAnimSequenceBase>(AnimationAsset))
	{
		return SequenceBase->GetPlayLength();
	}

	return 0.f;
}

float UProjectLocomotionOverrideComponent::ResolveOneShotHoldTime(UAnimationAsset* AnimationAsset) const
{
	const float Duration = GetAnimationDuration(AnimationAsset);
	if (Duration <= KINDA_SMALL_NUMBER)
	{
		return MaxTransitionDuration;
	}

	const UAnimationAsset* WalkPivotAsset = WalkPivotAnimation.Get();
	const UAnimationAsset* CrawlEntryAsset = CrawlEntryAnimation.Get();
	const UAnimationAsset* CrawlExitAsset = CrawlExitAnimation.Get();

	float PlaybackRatio = 0.9f;
	if (AnimationAsset == WalkPivotAsset)
	{
		PlaybackRatio = 0.82f;
	}
	else if (AnimationAsset == CrawlEntryAsset || AnimationAsset == CrawlExitAsset)
	{
		return FMath::Max(CrawlTransitionDuration, 0.1f);
	}

	return FMath::Clamp(Duration * PlaybackRatio, 0.08f, Duration);
}

bool UProjectLocomotionOverrideComponent::IsCrawlTransitionAnimation(const UAnimationAsset* AnimationAsset) const
{
	return AnimationAsset && (AnimationAsset == CrawlEntryAnimation.Get() || AnimationAsset == CrawlExitAnimation.Get());
}

bool UProjectLocomotionOverrideComponent::IsMovementLockedByTransition(float CurrentTimeSeconds) const
{
	return MovementLockUntilTime > CurrentTimeSeconds;
}

void UProjectLocomotionOverrideComponent::UpdateTransitionMovementLock(float CurrentTimeSeconds)
{
	const bool bShouldLockMovement = IsMovementLockedByTransition(CurrentTimeSeconds);
	APawn* PawnOwner = CachedPawnOwner.Get();
	ACharacter* CharacterOwner = CachedCharacterOwner.Get();

	if (bShouldLockMovement)
	{
		if (!bTransitionMovementLockApplied)
		{
			TransitionLockedMovementMode = CharacterMovementComponent ? EMovementMode(CharacterMovementComponent->MovementMode) : EMovementMode(MOVE_Walking);
			TransitionLockedCustomMovementMode = CharacterMovementComponent ? CharacterMovementComponent->CustomMovementMode : 0;
			bTransitionMovementLockApplied = true;
			UpdateControllerMoveInputIgnoreState();
		}

		if (PawnOwner)
		{
			if (CharacterOwner)
			{
				CharacterOwner->StopJumping();
			}

			PawnOwner->ConsumeMovementInputVector();
		}

		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->StopMovementImmediately();
			CharacterMovementComponent->DisableMovement();
		}

		return;
	}

	ReleaseTransitionMovementLock();
}

void UProjectLocomotionOverrideComponent::ReleaseTransitionMovementLock()
{
	if (!bTransitionMovementLockApplied)
	{
		ApplyDesiredRotationBehavior();
		UpdateControllerMoveInputIgnoreState();
		return;
	}

	if (CharacterMovementComponent && CharacterMovementComponent->MovementMode == MOVE_None)
	{
		const EMovementMode MovementModeToRestore = TransitionLockedMovementMode == MOVE_None ? MOVE_Walking : TransitionLockedMovementMode.GetValue();
		CharacterMovementComponent->SetMovementMode(MovementModeToRestore, TransitionLockedCustomMovementMode);
	}

	bTransitionMovementLockApplied = false;
	ApplyDesiredRotationBehavior();
	UpdateControllerMoveInputIgnoreState();
}
