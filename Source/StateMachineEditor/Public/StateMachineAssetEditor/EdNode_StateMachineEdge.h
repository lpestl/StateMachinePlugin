#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "EdNode_StateMachineEdge.generated.h"

class UEdGraph;
class UStateMachineNode;
class UStateMachineEdge;
class UEdNode_StateMachineNode;

UCLASS(MinimalAPI)
class UEdNode_StateMachineEdge : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UEdNode_StateMachineEdge();

	UPROPERTY()
	UEdGraph *Graph;

	UPROPERTY(VisibleAnywhere, Instanced, Category = "StateMachine")
	UStateMachineEdge *StateMachineEdge;

	void SetEdge(UStateMachineEdge *Edge);

	virtual void AllocateDefaultPins() override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual void PinConnectionListChanged(UEdGraphPin *Pin) override;

	virtual void PrepareForCopying() override;

	virtual UEdGraphPin *GetInputPin() const { return Pins[0]; }
	virtual UEdGraphPin *GetOutputPin() const { return Pins[1]; }

	void CreateConnections(UEdNode_StateMachineNode *Start, UEdNode_StateMachineNode *End);

	UEdNode_StateMachineNode *GetStartNode();
	UEdNode_StateMachineNode *GetEndNode();
};
