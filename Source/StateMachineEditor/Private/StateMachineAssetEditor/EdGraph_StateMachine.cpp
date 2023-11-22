#include "StateMachineAssetEditor/EdGraph_StateMachine.h"

#include "IStateMachineRuntime.h"
#include "StateMachineEditor/Public/StateMachineEditorModule.h"
#include "EdGraph/EdGraphPin.h"
#include "GraphData/StateMachineGraph.h"
#include "StateMachineAssetEditor/EdNode_StateMachineNode.h"
#include "StateMachineAssetEditor/EdNode_StateMachineEdge.h"

UEdGraph_StateMachine::UEdGraph_StateMachine()
{

}

UEdGraph_StateMachine::~UEdGraph_StateMachine()
{

}

void UEdGraph_StateMachine::RebuildStateMachine()
{
	UE_LOG(StateMachineEditor, Log, TEXT("UStateMachineEdGraph::RebuildStateMachine has been called"));

	UStateMachineGraph *Graph = GetStateMachine();

	Clear();

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UEdNode_StateMachineNode *EdNode = Cast<UEdNode_StateMachineNode>(Nodes[i]))
		{
			if (EdNode->StateMachineNode == nullptr)
				continue;

			UStateMachineNode *StateMachineNode = EdNode->StateMachineNode;

			NodeMap.Add(StateMachineNode, EdNode);

			Graph->AllNodes.Add(StateMachineNode);

			for (int PinIdx = 0; PinIdx < EdNode->Pins.Num(); ++PinIdx)
			{
				UEdGraphPin *Pin = EdNode->Pins[PinIdx];

				if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
					continue;

				for (int LinkToIdx = 0; LinkToIdx < Pin->LinkedTo.Num(); ++LinkToIdx)
				{
					UStateMachineNode *ChildNode = nullptr;
					if (UEdNode_StateMachineNode *EdNode_Child = Cast<UEdNode_StateMachineNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						ChildNode = EdNode_Child->StateMachineNode;
					}
					else if (UEdNode_StateMachineEdge *EdNode_Edge = Cast<UEdNode_StateMachineEdge>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						UEdNode_StateMachineNode *Child = EdNode_Edge->GetEndNode();;
						if (Child != nullptr)
						{
							ChildNode = Child->StateMachineNode;
						}
					}

					if (ChildNode != nullptr)
					{
						StateMachineNode->ChildrenNodes.Add(ChildNode);

						ChildNode->ParentNodes.Add(StateMachineNode);
					}
					else
					{
						UE_LOG(StateMachineEditor, Error, TEXT("UEdGraph_StateMachine::RebuildStateMachine can't find child node"));
					}
				}
			}
		}
		else if (UEdNode_StateMachineEdge *EdgeNode = Cast<UEdNode_StateMachineEdge>(Nodes[i]))
		{
			UEdNode_StateMachineNode *StartNode = EdgeNode->GetStartNode();
			UEdNode_StateMachineNode *EndNode = EdgeNode->GetEndNode();
			UStateMachineEdge *Edge = EdgeNode->StateMachineEdge;

			if (StartNode == nullptr || EndNode == nullptr || Edge == nullptr)
			{
				UE_LOG(StateMachineEditor, Error, TEXT("UEdGraph_StateMachine::RebuildStateMachine add edge failed."));
				continue;
			}

			EdgeMap.Add(Edge, EdgeNode);

			Edge->Graph = Graph;
			Edge->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
			Edge->StartNode = StartNode->StateMachineNode;
			Edge->EndNode = EndNode->StateMachineNode;
			Edge->StartNode->Edges.Add(Edge->EndNode, Edge);
		}
	}

	for (int i = 0; i < Graph->AllNodes.Num(); ++i)
	{
		UStateMachineNode *Node = Graph->AllNodes[i];
		if (Node->ParentNodes.Num() == 0)
		{
			Graph->RootNodes.Add(Node);

			SortNodes(Node);
		}

		Node->Graph = Graph;
		Node->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
	}

	Graph->RootNodes.Sort([&](const UStateMachineNode &L, const UStateMachineNode &R)
	{
		UEdNode_StateMachineNode *EdNode_LNode = NodeMap[&L];
		UEdNode_StateMachineNode *EdNode_RNode = NodeMap[&R];
		return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
	});
}

UStateMachineGraph *UEdGraph_StateMachine::GetStateMachine() const
{
	return CastChecked<UStateMachineGraph>(GetOuter());
}

bool UEdGraph_StateMachine::Modify(bool bAlwaysMarkDirty /*= true*/)
{
	bool Rtn = Super::Modify(bAlwaysMarkDirty);

	GetStateMachine()->Modify();

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		Nodes[i]->Modify();
	}

	return Rtn;
}

void UEdGraph_StateMachine::Clear()
{
	UStateMachineGraph *Graph = GetStateMachine();

	Graph->ClearGraph();
	NodeMap.Reset();
	EdgeMap.Reset();

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UEdNode_StateMachineNode *EdNode = Cast<UEdNode_StateMachineNode>(Nodes[i]))
		{
			UStateMachineNode *StateMachineNode = EdNode->StateMachineNode;
			if (StateMachineNode)
			{
				StateMachineNode->ParentNodes.Reset();
				StateMachineNode->ChildrenNodes.Reset();
				StateMachineNode->Edges.Reset();
			}
		}
	}
}

void UEdGraph_StateMachine::SortNodes(UStateMachineNode *RootNode)
{
	int Level = 0;
	TArray<UStateMachineNode*> CurrLevelNodes = { RootNode };
	TArray<UStateMachineNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		int32 LevelWidth = 0;
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UStateMachineNode *Node = CurrLevelNodes[i];

			auto Comp = [&](const UStateMachineNode &L, const UStateMachineNode &R)
			{
				UEdNode_StateMachineNode *EdNode_LNode = NodeMap[&L];
				UEdNode_StateMachineNode *EdNode_RNode = NodeMap[&R];
				return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
			};

			Node->ChildrenNodes.Sort(Comp);
			Node->ParentNodes.Sort(Comp);

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

void UEdGraph_StateMachine::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
}

