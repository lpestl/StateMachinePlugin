#include "StateMachineAssetEditor/EdNode_StateMachineEdge.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "GraphData/StateMachineEdge.h"
#include "StateMachineAssetEditor/EdNode_StateMachineNode.h"

#define LOCTEXT_NAMESPACE "EdNode_StateMachineEdge"

UEdNode_StateMachineEdge::UEdNode_StateMachineEdge()
{
	bCanRenameNode = true;
}

void UEdNode_StateMachineEdge::SetEdge(UStateMachineEdge *Edge)
{
	StateMachineEdge = Edge;
}

void UEdNode_StateMachineEdge::AllocateDefaultPins()
{
	UEdGraphPin *Inputs = CreatePin(EGPD_Input, TEXT("Edge"), FName(), TEXT("In"));
	Inputs->bHidden = true;
	UEdGraphPin *Outputs = CreatePin(EGPD_Output, TEXT("Edge"), FName(), TEXT("Out"));
	Outputs->bHidden = true;
}

FText UEdNode_StateMachineEdge::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (StateMachineEdge)
	{
		return StateMachineEdge->GetNodeTitle();
	}
	return FText();
}

void UEdNode_StateMachineEdge::PinConnectionListChanged(UEdGraphPin *Pin)
{
	if (Pin->LinkedTo.Num() == 0)
	{
		// Commit suicide; transitions must always have an input and output connection
		Modify();

		// Our parent graph will have our graph in SubGraphs so needs to be modified to record that.
		if (UEdGraph *ParentGraph = GetGraph())
		{
			ParentGraph->Modify();
		}

		DestroyNode();
	}
}

void UEdNode_StateMachineEdge::PrepareForCopying()
{
	StateMachineEdge->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

void UEdNode_StateMachineEdge::CreateConnections(UEdNode_StateMachineNode *Start, UEdNode_StateMachineNode *End)
{
	Pins[0]->Modify();
	Pins[0]->LinkedTo.Empty();

	Start->GetOutputPin()->Modify();
	Pins[0]->MakeLinkTo(Start->GetOutputPin());

	// This to next
	Pins[1]->Modify();
	Pins[1]->LinkedTo.Empty();

	End->GetInputPin()->Modify();
	Pins[1]->MakeLinkTo(End->GetInputPin());
}

UEdNode_StateMachineNode *UEdNode_StateMachineEdge::GetStartNode()
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UEdNode_StateMachineNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}
	else
	{
		return nullptr;
	}
}

UEdNode_StateMachineNode *UEdNode_StateMachineEdge::GetEndNode()
{
	if (Pins[1]->LinkedTo.Num() > 0)
	{
		return Cast<UEdNode_StateMachineNode>(Pins[1]->LinkedTo[0]->GetOwningNode());
	}
	else
	{
		return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE

