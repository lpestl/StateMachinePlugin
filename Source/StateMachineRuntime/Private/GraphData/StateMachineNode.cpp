#include "GraphData/StateMachineNode.h"
#include "GraphData/StateMachineGraph.h"

#define LOCTEXT_NAMESPACE "StateMachineNode"

UStateMachineNode::UStateMachineNode()
{
#if WITH_EDITORONLY_DATA
	Id = FGuid::NewGuid().ToString();	
	CompatibleGraphType = UStateMachineGraph::StaticClass();
	BackgroundColor = FLinearColor::Black;
#endif
}

UStateMachineNode::~UStateMachineNode()
{
}

UStateMachineEdge *UStateMachineNode::GetEdge(UStateMachineNode *ChildNode)
{
	return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}

FText UStateMachineNode::GetDescription_Implementation() const
{
	return LOCTEXT("NodeDesc", "State Machine Node");
}

#if WITH_EDITOR
bool UStateMachineNode::IsNameEditable() const
{
	return true;
}

FLinearColor UStateMachineNode::GetBackgroundColor() const
{
	return BackgroundColor;
}

FText UStateMachineNode::GetNodeTitle() const
{
	return NodeTitle.IsEmpty() ? GetDescription() : NodeTitle;
}

void UStateMachineNode::SetNodeTitle(const FText &NewTitle)
{
	NodeTitle = NewTitle;
}

bool UStateMachineNode::CanCreateConnection(UStateMachineNode *Other, FText &ErrorMessage)
{	
	return true;
}

bool UStateMachineNode::CanCreateConnectionTo(UStateMachineNode *Other, int32 NumberOfChildrenNodes, FText &ErrorMessage)
{
	if (ChildrenLimitType == ENodeLimit::Limited && NumberOfChildrenNodes >= ChildrenLimit)
	{
		ErrorMessage = FText::FromString("Children limit exceeded");
		return false;
	}

	return CanCreateConnection(Other, ErrorMessage);
}

bool UStateMachineNode::CanCreateConnectionFrom(UStateMachineNode *Other, int32 NumberOfParentNodes, FText &ErrorMessage)
{
	if (ParentLimitType == ENodeLimit::Limited && NumberOfParentNodes >= ParentLimit)
	{
		ErrorMessage = FText::FromString("Parent limit exceeded");
		return false;
	}

	return true;
}


#endif

bool UStateMachineNode::IsLeafNode() const
{
	return ChildrenNodes.Num() == 0;
}

UStateMachineGraph *UStateMachineNode::GetGraph() const
{
	return Graph;
}

const FString &UStateMachineNode::GetId() const
{
	return Id;
}

#undef LOCTEXT_NAMESPACE
