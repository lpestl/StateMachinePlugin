#include "GraphData/StateMachineGraph.h"

#include "IStateMachineRuntime.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

#define LOCTEXT_NAMESPACE "StateMachine"

UStateMachineGraph::UStateMachineGraph()
{
	NodeType = UStateMachineNode::StaticClass();
	EdgeType = UStateMachineEdge::StaticClass();

	bEdgeEnabled = true;

#if WITH_EDITORONLY_DATA
	EdGraph = nullptr;

	bCanRenameNode = true;
#endif
}

UStateMachineGraph::~UStateMachineGraph()
{

}

void UStateMachineGraph::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStateMachineGraph, Name);
	DOREPLIFETIME(UStateMachineGraph, GraphTags);
}

void UStateMachineGraph::Print(bool ToConsole /*= true*/, bool ToScreen /*= true*/)
{
	int Level = 0;
	TArray<UStateMachineNode*> CurrLevelNodes = RootNodes;
	TArray<UStateMachineNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UStateMachineNode *Node = CurrLevelNodes[i];
			check(Node != nullptr);

			FString Message = FString::Printf(TEXT("%s, Level %d"), *Node->GetDescription().ToString(), Level);

			if (ToConsole)
			{
				UE_LOG(StateMachineRuntime, Log, TEXT("%s"), *Message);
			}

			if (ToScreen && GEngine != nullptr)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, Message);
			}

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

int UStateMachineGraph::GetLevelNum() const
{
	int Level = 0;
	TArray<UStateMachineNode*> CurrLevelNodes = RootNodes;
	TArray<UStateMachineNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UStateMachineNode *Node = CurrLevelNodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}

	return Level;
}

void UStateMachineGraph::GetNodesByLevel(int Level, TArray<UStateMachineNode*> &Nodes)
{
	int CurrLEvel = 0;
	TArray<UStateMachineNode*> NextLevelNodes;

	Nodes = RootNodes;

	while (Nodes.Num() != 0)
	{
		if (CurrLEvel == Level)
			break;

		for (int i = 0; i < Nodes.Num(); ++i)
		{
			UStateMachineNode *Node = Nodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		Nodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++CurrLEvel;
	}
}

void UStateMachineGraph::ClearGraph()
{
	for (int i = 0; i < AllNodes.Num(); ++i)
	{
		UStateMachineNode *Node = AllNodes[i];
		if (Node)
		{
			Node->ParentNodes.Empty();
			Node->ChildrenNodes.Empty();
			Node->Edges.Empty();
		}
	}

	AllNodes.Empty();
	RootNodes.Empty();
}

#undef LOCTEXT_NAMESPACE
