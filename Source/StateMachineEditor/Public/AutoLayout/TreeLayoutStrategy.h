#pragma once

#include "CoreMinimal.h"
#include "AutoLayoutStrategy.h"
#include "TreeLayoutStrategy.generated.h"

UCLASS()
class STATEMACHINEEDITOR_API UTreeLayoutStrategy : public UAutoLayoutStrategy
{
	GENERATED_BODY()
public:
	UTreeLayoutStrategy();
	virtual ~UTreeLayoutStrategy();

	virtual void Layout(UEdGraph *EdGraph) override;

protected:
	void InitPass(UStateMachineNode *RootNode, const FVector2D &Anchor);
	bool ResolveConflictPass(UStateMachineNode *Node);

	bool ResolveConflict(UStateMachineNode *LRoot, UStateMachineNode *RRoot);

	void GetLeftContour(UStateMachineNode *RootNode, int32 Level, TArray<UEdNode_StateMachineNode*> &Contour);
	void GetRightContour(UStateMachineNode *RootNode, int32 Level, TArray<UEdNode_StateMachineNode*> &Contour);
	
	void ShiftSubTree(UStateMachineNode *RootNode, const FVector2D &Offset);

	void UpdateParentNodePosition(UStateMachineNode *RootNode);
};
