#pragma once

#include <EdGraphUtilities.h>
#include <EdGraph/EdGraphNode.h>

class SGraphNode;

class FGraphPanelNodeFactory_StateMachine : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode *Node) const override;
};