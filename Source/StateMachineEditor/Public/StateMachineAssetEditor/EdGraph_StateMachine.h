#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph_StateMachine.generated.h"

class UStateMachineGraph;
class UStateMachineNode;
class UStateMachineEdge;
class UEdNode_StateMachineNode;
class UEdNode_StateMachineEdge;

UCLASS()
class STATEMACHINEEDITOR_API UEdGraph_StateMachine : public UEdGraph
{
	GENERATED_BODY()

public:
	UEdGraph_StateMachine();
	virtual ~UEdGraph_StateMachine();

	virtual void RebuildStateMachine();

	UStateMachineGraph *GetStateMachine() const;

	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
	virtual void PostEditUndo() override;

	UPROPERTY(Transient)
	TMap<UStateMachineNode*, UEdNode_StateMachineNode*> NodeMap;

	UPROPERTY(Transient)
	TMap<UStateMachineEdge*, UEdNode_StateMachineEdge*> EdgeMap;

protected:
	void Clear();

	void SortNodes(UStateMachineNode *RootNode);
};
