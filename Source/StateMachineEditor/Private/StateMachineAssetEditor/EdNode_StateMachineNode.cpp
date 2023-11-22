#include "StateMachineAssetEditor/EdNode_StateMachineNode.h"
#include "StateMachineAssetEditor/EdGraph_StateMachine.h"
#include "Kismet2/Kismet2NameValidators.h"
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "EdNode_StateMachine"

UEdNode_StateMachineNode::UEdNode_StateMachineNode()
{
	bCanRenameNode = true;
}

UEdNode_StateMachineNode::~UEdNode_StateMachineNode()
{

}

void UEdNode_StateMachineNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNodes", FName(), TEXT("In"));
	CreatePin(EGPD_Output, "MultipleNodes", FName(), TEXT("Out"));
}

UEdGraph_StateMachine *UEdNode_StateMachineNode::GetStateMachineEdGraph()
{
	return Cast<UEdGraph_StateMachine>(GetGraph());
}

FText UEdNode_StateMachineNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (StateMachineNode == nullptr)
	{
		return Super::GetNodeTitle(TitleType);
	}
	else
	{
		return StateMachineNode->GetNodeTitle();
	}
}

void UEdNode_StateMachineNode::PrepareForCopying()
{
	StateMachineNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

void UEdNode_StateMachineNode::AutowireNewNode(UEdGraphPin *FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin != nullptr)
	{
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

void UEdNode_StateMachineNode::SetStateMachineNode(UStateMachineNode *InNode)
{
	StateMachineNode = InNode;
}

FLinearColor UEdNode_StateMachineNode::GetBackgroundColor() const
{
	return StateMachineNode == nullptr ? FLinearColor::Black : StateMachineNode->GetBackgroundColor();
}

UEdGraphPin *UEdNode_StateMachineNode::GetInputPin() const
{
	return Pins[0];
}

UEdGraphPin *UEdNode_StateMachineNode::GetOutputPin() const
{
	return Pins[1];
}

void UEdNode_StateMachineNode::PostEditUndo()
{
	UEdGraphNode::PostEditUndo();
}

#undef LOCTEXT_NAMESPACE
