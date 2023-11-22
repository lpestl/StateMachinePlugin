// Created by Mikhail S. Kataev - mkataev@temporal-games.com


#include "LogicData/State.h"

#include "IStateMachineRuntime.h"
#include "LogicData/StateMachine.h"
#include "LogicData/Transition.h"

UState::UState()
{
#if WITH_EDITORONLY_DATA
	NodeTitle = ContextMenuName = this->GetClass()->GetDisplayNameText();
#endif	
}

UState::~UState()
{
}

void UState::Enter_Implementation()
{	
	BindDynamics();
	
	if (OnStateEntered.IsBound())
	{
		OnStateEntered.Broadcast(this);
	}
}

void UState::Leave_Implementation()
{
	if (OnStateLeaved.IsBound())
	{
		OnStateLeaved.Broadcast(this);
	}

	UnbindDynamics();
}

bool UState::CanTransitTo_Implementation(UState *ToState)
{
	return true;
}

bool UState::TryGoToNextState()
{
	if (UStateMachine *StateMachine = Cast<UStateMachine>(GetOuter()))
	{
		if (IsValid(StateMachine->GetOuter()))
		{
			return StateMachine->TryGoToNextState();
		}
		UE_LOG(StateMachineRuntime, Warning, TEXT("The StateMachine \"%s\" has a non-valid Owner"), *StateMachine->GetName());
		return false;		
	}
	UE_LOG(StateMachineRuntime, Warning, TEXT("The state \"%s\" has a non-valid Outer"), *GetName());
	return false;
}

const FString &UState::GetStateId() const
{
	return GetId();
}

UWorld *UState::GetWorld() const
{
	if (IsValid(GetOuter()))
	{
		return GetOuter()->GetWorld();
	}
	UE_LOG(StateMachineRuntime, Warning, TEXT("State %s doesn't have a proper outer's World!"), *GetName());
	return nullptr;
}

UState *UState::SpawnUniqueState(UObject *Outer, UState *Template, EObjectFlags Flags)
{
	check(Outer);
	check(Template);
	
	// Spawn unique state
	const FName UniqueName = MakeUniqueObjectName(Outer, Template->GetClass());
	UState *SpawnedState = NewObject<UState>(Outer, Template->GetClass(), UniqueName, Flags == RF_NoFlags ? Template->GetFlags() : Flags, Template);
	if (UStateMachine *OuterStateMachine = Cast<UStateMachine>(Outer))
	{
		SpawnedState->Graph = OuterStateMachine;
	}
	return SpawnedState;
}

void UState::FillFromTemplate(UStateMachine *StateMachine, UState *Template)
{
	check(StateMachine);
	check(Template);
	
	// Link to actual parent states
	ParentNodes.Empty();
	for (const UStateMachineNode *TemplateParentNode : Template->ParentNodes)
		if (const UState *TemplateParentState = Cast<UState>(TemplateParentNode))
			ParentNodes.Add(StateMachine->GetStateById(TemplateParentState->GetStateId()));

	// Link to actual Child states
	ChildrenNodes.Empty();
	for (const UStateMachineNode *TemplateChildNode : Template->ChildrenNodes)
		if (const UState *TemplateChildState = Cast<UState>(TemplateChildNode))
			ChildrenNodes.Add(StateMachine->GetStateById(TemplateChildState->GetStateId()));

	// Spawn actual local transitions
	Edges.Empty();
	for (const auto &TemplateEdgePair : Template->Edges)
	{
		if (const UState *ChildState = Cast<UState>(TemplateEdgePair.Key))
		{
			UState *ChildNode = StateMachine->GetStateById(ChildState->GetStateId());

			const FName UniqueName = MakeUniqueObjectName(StateMachine, TemplateEdgePair.Value->GetClass());
			UTransition *StateTransition = NewObject<UTransition>(StateMachine, TemplateEdgePair.Value->GetClass(), UniqueName, TemplateEdgePair.Value->GetFlags(), TemplateEdgePair.Value);

			// Fill actual pointers
			StateTransition->Graph = StateMachine;
			StateTransition->StartNode = this;
			StateTransition->EndNode = ChildNode;
		
			Edges.Add(ChildNode, StateTransition);
		}
	}
}

UStateMachine *UState::GetStateMachine() const
{
	if (IsValid(GetOuter()))
	{
		if (UStateMachine *OuterStateMachine = Cast<UStateMachine>(GetOuter()))
		{
			return OuterStateMachine;
		}
	}
	UE_LOG(StateMachineRuntime, Warning, TEXT("State %s doesn't have a valid outer's State Machine"), *GetName());
	return nullptr;
}
