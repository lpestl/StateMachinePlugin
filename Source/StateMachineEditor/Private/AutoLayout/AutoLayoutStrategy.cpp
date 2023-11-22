#include "AutoLayout/AutoLayoutStrategy.h"
#include "Kismet/KismetMathLibrary.h"
#include "StateMachineAssetEditor/EdNode_StateMachineNode.h"
#include "StateMachineAssetEditor/SEdNode_StateMachineNode.h"

UAutoLayoutStrategy::UAutoLayoutStrategy()
{
	Settings = nullptr;
	MaxIteration = 50;
	OptimalDistance = 150;
}

UAutoLayoutStrategy::~UAutoLayoutStrategy()
{

}

FBox2D UAutoLayoutStrategy::GetNodeBound(UEdGraphNode *EdNode)
{
	int32 NodeWidth = GetNodeWidth(Cast<UEdNode_StateMachineNode>(EdNode));
	int32 NodeHeight = GetNodeHeight(Cast<UEdNode_StateMachineNode>(EdNode));
	FVector2D Min(EdNode->NodePosX, EdNode->NodePosY);
	FVector2D Max(EdNode->NodePosX + NodeWidth, EdNode->NodePosY + NodeHeight);
	return FBox2D(Min, Max);
}

FBox2D UAutoLayoutStrategy::GetActualBounds(UStateMachineNode *RootNode)
{
	int Level = 0;
	TArray<UStateMachineNode*> CurrLevelNodes = { RootNode };
	TArray<UStateMachineNode*> NextLevelNodes;

	FBox2D Rtn = GetNodeBound(EdGraph->NodeMap[RootNode]);

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UStateMachineNode *Node = CurrLevelNodes[i];
			check(Node != nullptr);

			Rtn += GetNodeBound(EdGraph->NodeMap[Node]);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}
	return Rtn;
}

void UAutoLayoutStrategy::RandomLayoutOneTree(UStateMachineNode *RootNode, const FBox2D &Bound)
{
	int Level = 0;
	TArray<UStateMachineNode*> CurrLevelNodes = { RootNode };
	TArray<UStateMachineNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UStateMachineNode *Node = CurrLevelNodes[i];
			check(Node != nullptr);

			UEdNode_StateMachineNode *EdNode_Node = EdGraph->NodeMap[Node];

			EdNode_Node->NodePosX = UKismetMathLibrary::RandomFloatInRange(Bound.Min.X, Bound.Max.X);
			EdNode_Node->NodePosY = UKismetMathLibrary::RandomFloatInRange(Bound.Min.Y, Bound.Max.Y);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}
}

int32 UAutoLayoutStrategy::GetNodeWidth(UEdNode_StateMachineNode *EdNode)
{
	return EdNode->SEdNode->GetCachedGeometry().GetLocalSize().X;
}

int32 UAutoLayoutStrategy::GetNodeHeight(UEdNode_StateMachineNode *EdNode)
{
	return EdNode->SEdNode->GetCachedGeometry().GetLocalSize().Y;
}

