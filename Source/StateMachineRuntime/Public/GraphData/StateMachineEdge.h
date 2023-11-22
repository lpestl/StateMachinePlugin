#pragma once

#include "CoreMinimal.h"
#include "StateMachineNode.h"
#include "StateMachineEdge.generated.h"

class UStateMachineGraph;

UCLASS(Abstract, Blueprintable)
class STATEMACHINERUNTIME_API UStateMachineEdge : public UObject
{
	GENERATED_BODY()

public:
	UStateMachineEdge();
	virtual ~UStateMachineEdge();

	UPROPERTY(VisibleAnywhere, Category = "StateMachineNode")
	UStateMachineGraph *Graph;

	UPROPERTY(BlueprintReadOnly, Category = "StateMachineEdge")
	UStateMachineNode *StartNode;

	UPROPERTY(BlueprintReadOnly, Category = "StateMachineEdge")
	UStateMachineNode *EndNode;

	UFUNCTION(BlueprintPure, Category = "StateMachineEdge")
	UStateMachineGraph *GetGraph() const;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category = "StateMachineNode_Editor")
	bool bShouldDrawTitle = false;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineNode_Editor")
	FText NodeTitle;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineEdge")
	FLinearColor EdgeColour = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
#endif

#if WITH_EDITOR
	virtual FText GetNodeTitle() const { return NodeTitle; }
	FLinearColor GetEdgeColour() { return EdgeColour; }

	virtual void SetNodeTitle(const FText &NewTitle);
#endif
	
};
