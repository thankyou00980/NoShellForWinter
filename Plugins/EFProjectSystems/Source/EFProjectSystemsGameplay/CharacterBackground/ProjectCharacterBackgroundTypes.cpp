#include "CharacterBackground/ProjectCharacterBackgroundTypes.h"

#define LOCTEXT_NAMESPACE "ProjectCharacterBackgroundTypes"

namespace ProjectCharacterBackground
{
	bool TryResolveSinAttribute(const FName AttributeID, EProjectSinAttribute& OutAttribute)
	{
		const FString AttributeString = AttributeID.ToString();

		if (AttributeString.Equals(TEXT("Willpower"), ESearchCase::IgnoreCase))
		{
			OutAttribute = EProjectSinAttribute::Willpower;
			return true;
		}
		if (AttributeString.Equals(TEXT("Sadism"), ESearchCase::IgnoreCase))
		{
			OutAttribute = EProjectSinAttribute::Sadism;
			return true;
		}
		if (AttributeString.Equals(TEXT("Masochism"), ESearchCase::IgnoreCase))
		{
			OutAttribute = EProjectSinAttribute::Masochism;
			return true;
		}
		if (AttributeString.Equals(TEXT("Faith"), ESearchCase::IgnoreCase))
		{
			OutAttribute = EProjectSinAttribute::Faith;
			return true;
		}
		if (AttributeString.Equals(TEXT("Cunning"), ESearchCase::IgnoreCase))
		{
			OutAttribute = EProjectSinAttribute::Cunning;
			return true;
		}
		if (AttributeString.Equals(TEXT("Celerity"), ESearchCase::IgnoreCase))
		{
			OutAttribute = EProjectSinAttribute::Celerity;
			return true;
		}
		if (AttributeString.Equals(TEXT("Allure"), ESearchCase::IgnoreCase))
		{
			OutAttribute = EProjectSinAttribute::Allure;
			return true;
		}

		return false;
	}

	FName GetSinAttributeID(const EProjectSinAttribute Attribute)
	{
		switch (Attribute)
		{
		case EProjectSinAttribute::Willpower:
			return TEXT("Willpower");
		case EProjectSinAttribute::Sadism:
			return TEXT("Sadism");
		case EProjectSinAttribute::Masochism:
			return TEXT("Masochism");
		case EProjectSinAttribute::Faith:
			return TEXT("Faith");
		case EProjectSinAttribute::Cunning:
			return TEXT("Cunning");
		case EProjectSinAttribute::Celerity:
			return TEXT("Celerity");
		case EProjectSinAttribute::Allure:
			return TEXT("Allure");
		default:
			return NAME_None;
		}
	}

	FText GetSinAttributeDisplayText(const EProjectSinAttribute Attribute)
	{
		switch (Attribute)
		{
		case EProjectSinAttribute::Willpower:
			return LOCTEXT("Willpower", "Willpower");
		case EProjectSinAttribute::Sadism:
			return LOCTEXT("Sadism", "Sadism");
		case EProjectSinAttribute::Masochism:
			return LOCTEXT("Masochism", "Masochism");
		case EProjectSinAttribute::Faith:
			return LOCTEXT("Faith", "Faith");
		case EProjectSinAttribute::Cunning:
			return LOCTEXT("Cunning", "Cunning");
		case EProjectSinAttribute::Celerity:
			return LOCTEXT("Celerity", "Celerity");
		case EProjectSinAttribute::Allure:
			return LOCTEXT("Allure", "Allure");
		default:
			return LOCTEXT("UnknownAttribute", "Unknown");
		}
	}
}

#undef LOCTEXT_NAMESPACE
