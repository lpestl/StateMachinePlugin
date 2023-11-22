#include "StateMachineNodeFactory.h"
#include <EdGraph/EdGraphNode.h>
#include "StateMachineAssetEditor/SEdNode_StateMachineEdge.h"
#include "StateMachineAssetEditor/EdNode_StateMachineNode.h"
#include "StateMachineAssetEditor/SEdNode_StateMachineNode.h"
#include "StateMachineAssetEditor/EdNode_StateMachineEdge.h"

TSharedPtr<SGraphNode> FGraphPanelNodeFactory_StateMachine::CreateNode(UEdGraphNode *Node) const
{
	if (UEdNode_StateMachineNode *EdNode_GraphNode = Cast<UEdNode_StateMachineNode>(Node))
	{
		return SNew(SEdNode_StateMachineNode, EdNode_GraphNode);
	}
	else if (UEdNode_StateMachineEdge *EdNode_Edge = Cast<UEdNode_StateMachineEdge>(Node))
	{
		return SNew(SEdNode_StateMachineEdge, EdNode_Edge);
	}
	return nullptr;
}

