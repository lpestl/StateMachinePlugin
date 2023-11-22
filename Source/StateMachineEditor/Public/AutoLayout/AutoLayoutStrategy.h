#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "GraphData/StateMachineGraph.h"
#include "StateMachineAssetEditor/EdGraph_StateMachine.h"
#include "StateMachineAssetEditor/EdNode_StateMachineNode.h"
#include "StateMachineAssetEditor/EdNode_StateMachineEdge.h"
#include "StateMachineAssetEditor/Settings_StateMachineEditor.h"
#include "AutoLayoutStrategy.generated.h"

class UStateMachineEditorSettings;

UCLASS(abstract)
class STATEMACHINEEDITOR_API UAutoLayoutStrategy : public UObject
{
	GENERATED_BODY()
public:
	UAutoLayoutStrategy();
	virtual ~UAutoLayoutStrategy();

	virtual void Layout(UEdGraph *G) {};

	UStateMachineEditorSettings *Settings;

protected:
	int32 GetNodeWidth(UEdNode_StateMachineNode *EdNode);

	int32 GetNodeHeight(UEdNode_StateMachineNode *EdNode);

	FBox2D GetNodeBound(UEdGraphNode *EdNode);

	FBox2D GetActualBounds(UStateMachineNode *RootNode);

	virtual void RandomLayoutOneTree(UStateMachineNode *RootNode, const FBox2D &Bound);

protected:
	UStateMachineGraph *Graph;
	UEdGraph_StateMachine *EdGraph;
	int32 MaxIteration;
	int32 OptimalDistance;
};
