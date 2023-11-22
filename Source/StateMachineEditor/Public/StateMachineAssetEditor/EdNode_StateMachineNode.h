#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "GraphData/StateMachineNode.h"
#include "EdNode_StateMachineNode.generated.h"

class UEdNode_StateMachineEdge;
class UEdGraph_StateMachine;
class SEdNode_StateMachineNode;

UCLASS(MinimalAPI)
class UEdNode_StateMachineNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UEdNode_StateMachineNode();
	virtual ~UEdNode_StateMachineNode();

	UPROPERTY(VisibleAnywhere, Instanced, Category = "StateMachine")
	UStateMachineNode *StateMachineNode;

	void SetStateMachineNode(UStateMachineNode *InNode);
	UEdGraph_StateMachine *GetStateMachineEdGraph();

	SEdNode_StateMachineNode *SEdNode;

	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PrepareForCopying() override;
	virtual void AutowireNewNode(UEdGraphPin *FromPin) override;

	virtual FLinearColor GetBackgroundColor() const;
	virtual UEdGraphPin *GetInputPin() const;
	virtual UEdGraphPin *GetOutputPin() const;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

};
