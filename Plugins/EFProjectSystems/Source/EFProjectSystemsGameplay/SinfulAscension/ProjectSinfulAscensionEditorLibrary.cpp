#include "SinfulAscension/ProjectSinfulAscensionEditorLibrary.h"

#include "SinfulAscension/ProjectSinfulAscensionBlueprintLibrary.h"

#if WITH_EDITOR
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogProjectSinfulAscensionEditor, Log, All);

namespace
{
#if WITH_EDITOR
	const FName OnInteractedByPawnFunctionName(TEXT("OnInteractedByPawn"));

	bool BuildOpenMenuInteractionGraph(UBlueprint* Blueprint)
	{
		if (!Blueprint)
		{
			return false;
		}

		UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
		if (!EventGraph)
		{
			FKismetEditorUtilities::CreateDefaultEventGraphs(Blueprint);
			EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
		}

		if (!EventGraph)
		{
			UE_LOG(LogProjectSinfulAscensionEditor, Warning, TEXT("[ProjectSinfulAscensionEditor] Missing EventGraph on %s"), *GetNameSafe(Blueprint));
			return false;
		}

		UClass* ParentClass = Blueprint->ParentClass;
		if (!ParentClass && Blueprint->GeneratedClass)
		{
			ParentClass = Blueprint->GeneratedClass->GetSuperClass();
		}

		if (!ParentClass)
		{
			UE_LOG(LogProjectSinfulAscensionEditor, Warning, TEXT("[ProjectSinfulAscensionEditor] Missing parent class on %s"), *GetNameSafe(Blueprint));
			return false;
		}

		UFunction* OnInteractedByPawnFunction = ParentClass->FindFunctionByName(OnInteractedByPawnFunctionName);
		if (!OnInteractedByPawnFunction)
		{
			UE_LOG(LogProjectSinfulAscensionEditor, Warning, TEXT("[ProjectSinfulAscensionEditor] Could not find OnInteractedByPawn on %s"), *GetNameSafe(ParentClass));
			return false;
		}

		UFunction* OpenMenuFunction = UProjectSinfulAscensionBlueprintLibrary::StaticClass()->FindFunctionByName(
			GET_FUNCTION_NAME_CHECKED(UProjectSinfulAscensionBlueprintLibrary, OpenSinfulSxpExchangeMenu));
		if (!OpenMenuFunction)
		{
			UE_LOG(LogProjectSinfulAscensionEditor, Warning, TEXT("[ProjectSinfulAscensionEditor] Could not resolve OpenSinfulSxpExchangeMenu"));
			return false;
		}

		UK2Node_Event* EventNode = nullptr;
		for (UEdGraphNode* GraphNode : EventGraph->Nodes)
		{
			if (UK2Node_Event* CandidateEventNode = Cast<UK2Node_Event>(GraphNode))
			{
				if (CandidateEventNode->EventReference.GetMemberName() == OnInteractedByPawnFunctionName)
				{
					EventNode = CandidateEventNode;
					break;
				}
			}
		}

		if (!EventNode)
		{
			int32 EventNodePosY = 0;
			EventNode = FKismetEditorUtilities::AddDefaultEventNode(Blueprint, EventGraph, OnInteractedByPawnFunctionName, ParentClass, EventNodePosY);
			if (!EventNode)
			{
				UE_LOG(LogProjectSinfulAscensionEditor, Warning, TEXT("[ProjectSinfulAscensionEditor] Failed to create OnInteractedByPawn event node on %s"), *GetNameSafe(Blueprint));
				return false;
			}
			EventNode->NodePosX = 0;
			EventNode->NodePosY = 0;
		}

		UK2Node_CallFunction* OpenMenuNode = nullptr;
		for (UEdGraphNode* GraphNode : EventGraph->Nodes)
		{
			if (UK2Node_CallFunction* CandidateCallNode = Cast<UK2Node_CallFunction>(GraphNode))
			{
				if (CandidateCallNode->GetTargetFunction() == OpenMenuFunction)
				{
					OpenMenuNode = CandidateCallNode;
					break;
				}
			}
		}

		if (!OpenMenuNode)
		{
			OpenMenuNode = NewObject<UK2Node_CallFunction>(EventGraph);
			EventGraph->AddNode(OpenMenuNode, true, false);
			OpenMenuNode->CreateNewGuid();
			OpenMenuNode->PostPlacedNewNode();
			OpenMenuNode->SetFromFunction(OpenMenuFunction);
			OpenMenuNode->AllocateDefaultPins();
			OpenMenuNode->NodePosX = 340;
			OpenMenuNode->NodePosY = 0;
		}

		const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
		UEdGraphPin* EventExecPin = EventNode->FindPin(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* EventPawnPin = EventNode->FindPin(TEXT("Pawn"));
		UEdGraphPin* OpenMenuExecPin = OpenMenuNode->FindPin(UEdGraphSchema_K2::PN_Execute);
		UEdGraphPin* OpenMenuActorPin = OpenMenuNode->FindPin(TEXT("Actor"));

		if (!EventExecPin || !EventPawnPin || !OpenMenuExecPin || !OpenMenuActorPin)
		{
			UE_LOG(LogProjectSinfulAscensionEditor, Warning, TEXT("[ProjectSinfulAscensionEditor] Missing pins while wiring %s"), *GetNameSafe(Blueprint));
			return false;
		}

		const bool bExecAlreadyConnected = EventExecPin->LinkedTo.Contains(OpenMenuExecPin);
		const bool bActorAlreadyConnected = EventPawnPin->LinkedTo.Contains(OpenMenuActorPin);
		if (bExecAlreadyConnected && bActorAlreadyConnected)
		{
			return true;
		}

		if (EventExecPin->LinkedTo.Num() > 0 && !bExecAlreadyConnected)
		{
			UE_LOG(LogProjectSinfulAscensionEditor, Warning, TEXT("[ProjectSinfulAscensionEditor] OnInteractedByPawn already has exec links on %s. Refusing to overwrite automatically."), *GetNameSafe(Blueprint));
			return false;
		}

		const bool bConnectedExec = bExecAlreadyConnected || K2Schema->TryCreateConnection(EventExecPin, OpenMenuExecPin);
		const bool bConnectedActor = bActorAlreadyConnected || K2Schema->TryCreateConnection(EventPawnPin, OpenMenuActorPin);
		if (!bConnectedExec || !bConnectedActor)
		{
			UE_LOG(
				LogProjectSinfulAscensionEditor,
				Warning,
				TEXT("[ProjectSinfulAscensionEditor] Failed to connect altar menu graph on %s Exec=%s Actor=%s"),
				*GetNameSafe(Blueprint),
				bConnectedExec ? TEXT("true") : TEXT("false"),
				bConnectedActor ? TEXT("true") : TEXT("false"));
			return false;
		}

		return true;
	}
#endif
}

bool UProjectSinfulAscensionEditorLibrary::ConfigureBlueprintAsSxpAltar(UBlueprint* Blueprint)
{
#if !WITH_EDITOR
	UE_LOG(LogProjectSinfulAscensionEditor, Warning, TEXT("[ProjectSinfulAscensionEditor] ConfigureBlueprintAsSxpAltar is editor-only"));
	return false;
#else
	if (!Blueprint)
	{
		return false;
	}

	if (!BuildOpenMenuInteractionGraph(Blueprint))
	{
		return false;
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	Blueprint->MarkPackageDirty();

	UE_LOG(LogProjectSinfulAscensionEditor, Log, TEXT("[ProjectSinfulAscensionEditor] Configured SXP altar blueprint %s"), *GetNameSafe(Blueprint));
	return true;
#endif
}
