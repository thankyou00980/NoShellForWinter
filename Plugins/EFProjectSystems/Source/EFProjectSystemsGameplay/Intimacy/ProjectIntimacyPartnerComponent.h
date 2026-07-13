#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Intimacy/ProjectIntimacyTypes.h"
#include "ProjectIntimacyPartnerComponent.generated.h"

class UProjectEnemyLevelComponent;

UCLASS(ClassGroup = (Project), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectIntimacyPartnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectIntimacyPartnerComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy")
	static UProjectIntimacyPartnerComponent* FindOrCreateForActor(AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	FString GetResolvedPartnerId() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	FText GetPartnerDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	EProjectIntimacyPersonality GetResolvedPersonality() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	FGameplayTag GetResolvedGenderTag() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	int32 GetPartnerLevel() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	float GetMaxLust() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	float GetDefaultAnimationRate() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	static FText PersonalityToText(EProjectIntimacyPersonality InPersonality);

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	static FText RelationshipToText(EProjectIntimacyRelationship InRelationship);

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	static FText GenderTagToText(FGameplayTag InGenderTag);

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	static FText RelationshipTagsToText(const FGameplayTagContainer& InRelationshipTags);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FString PartnerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText DisplayNameOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyPersonality Personality = EProjectIntimacyPersonality::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTag GenderTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyRelationship InitialRelationship = EProjectIntimacyRelationship::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy", meta = (ClampMin = "0"))
	int32 InitialChildren = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy", meta = (ClampMin = "0"))
	int32 InitialAffect = 0;

private:
	EProjectIntimacyPersonality ResolveAutoPersonality() const;
};
