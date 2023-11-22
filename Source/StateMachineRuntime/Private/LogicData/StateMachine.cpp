// Created by Mikhail S. Kataev - mkataev@temporal-games.com

#include "LogicData/StateMachine.h"
#include "IStateMachineRuntime.h"
#include "LogicData/ClientConformationsState.h"
#include "LogicData/State.h"
#include "LogicData/Transition.h"
#include "Net/UnrealNetwork.h"

void UStateMachine::Run()
{
	if (!IsInitiated())
	{
		Init();
	}
	SetDefaultState();
}

void UStateMachine::Stop()
{
	if (CurrentState != nullptr)
	{
		GetCurrentState()->Leave();
	}
	SetState(nullptr);
}

bool UStateMachine::TryGoToNextState()
{
	if (GetWorld()->GetNetMode() >= NM_Client)
	{
		return false;
	}
	UState *CurrentStateLocal = GetCurrentState();
	if (!IsValid(CurrentStateLocal))
	{
		UE_LOG(StateMachineRuntime, Warning, TEXT("State machine %s for actor %s is not running. Call SM->Run() method."), *this->GetName(), *GetOuter()->GetName());
		return false;
	}
	
	// Note: Debug counter
	int32 NumParallelTransitions = 0;
	
	for (UStateMachineNode *ChildNode : CurrentStateLocal->ChildrenNodes)
	{
		if (const auto ChildState = Cast<UState>(ChildNode))
		{
			UStateMachineEdge *Edge = CurrentStateLocal->GetEdge(ChildNode);
			if (const auto Transition = Cast<UTransition>(Edge))
			{
				if (Transition->CanTransitTo())
				{
					if (const auto Owner = Cast<AActor>(GetOuter()))
					{
						if (Owner->GetLocalRole() == ROLE_Authority)
						{
							// Note: Debug counter increment
							NumParallelTransitions++;

							if (NumParallelTransitions <= 1)
							{
								GetCurrentState()->Leave();
								SetState(ChildState);
								GetCurrentState()->Enter();
							}
						} else
						{
							if (const auto ConformationsState = Cast<UClientConformationsState>(GetCurrentState()))
							{
								ConformationsState->ConfirmLeave();
							}
						}
					}
				}
			}
		}
	}

	// Note: Debug checker
	if (NumParallelTransitions > 1)
	{
		UE_LOG(StateMachineRuntime, Warning, TEXT("State %s has more then 1 valid transition"), *CurrentStateLocal->GetName());
	}

	return NumParallelTransitions >= 1;
}

void UStateMachine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UStateMachine, CurrentStateId);
}

void UStateMachine::ReceiveSkippedStatesIds_Implementation(const TArray<FString> &InOutSkippedStatesIds)
{
	if (const auto Owner = Cast<AActor>(GetOuter()))
	{
		// Let's split the processing logic for the server and the client
		if (Owner->GetLocalRole() == ROLE_Authority)
		// NOTE: Server only or Standalone
		{
			// Nothing to do 
			//UE_LOG(StateMachineRuntime, Log, TEXT("[SERVER] Recieved data and nothing to do"));
		}
		else
		// NOTE: Client only
		{
			if ((GetCurrentState() == nullptr) || (!IsValid(GetCurrentState())))
			{
				UE_LOG(StateMachineRuntime, Warning, TEXT("[CLIENT] StateMachine \"%s\" has not yet been launched"), *GetName());
				return;
			}
			
			// Transform the received data into a mutable cache array
			TArray<FString> TemporaryCache = InOutSkippedStatesIds;
			// Let's adjust the cache array taking into account the states already passed on the client.
			CorrectRevivedPassedStates(TemporaryCache);

#if !UE_BUILD_SHIPPING
			FString FutureStatesString = TEXT("Controller ");
			FutureStatesString += Owner->GetName();
			FutureStatesString += TEXT(" has current state ");
			FutureStatesString += GetCurrentState()->GetClass()->GetName();			
			FutureStatesString += TEXT(". Next states: ");
#endif
			
			if (!TemporaryCache.IsEmpty() && PassedStates.IsEmpty())
			{				
				// Let's save the states that the client still needs to go through in order to catch up with the server.
				for (auto PassedStateId : TemporaryCache)
				{
					auto FoundState = GetStateById(PassedStateId);
					FutureStates.Enqueue(FoundState);
#if !UE_BUILD_SHIPPING
					FutureStatesString += FoundState->GetName();
					FutureStatesString += TEXT(" -> ");
#endif
				}
#if !UE_BUILD_SHIPPING
				FutureStatesString += TEXT(".");
				UE_LOG(StateMachineRuntime, Log, TEXT("[CLIENT] %s"), *FutureStatesString);
#endif

				// Note: If the upcoming state is equal to the current one,
				// then we throw it out of the queue.
				if (!FutureStates.IsEmpty())
				{
					const UState *FutureState = *FutureStates.Peek();
					if (FutureState && IsValid(FutureState) && FutureState == GetCurrentState())
					{
						FutureStates.Pop();
					}
				}

				// If the queue of upcoming states is not empty
				const UState *FutureState = *FutureStates.Peek();
				if ((FutureState != nullptr) && (IsValid(FutureState)))
				{
					const auto ConformationsState = Cast<UClientConformationsState>(GetCurrentState());
					if ((ConformationsState == nullptr) || ((ConformationsState != nullptr) && (ConformationsState->IsLeaveConfirmed())))
					{
						// Then force run next state
						GetCurrentState()->Leave();

						UState *NewState;
						FutureStates.Dequeue(NewState);
						SetState(NewState);
						
						GetCurrentState()->Enter();
					}
				}
			}
#if !UE_BUILD_SHIPPING
			else
			{
				UE_LOG(StateMachineRuntime, Log, TEXT("[CLIENT][%s] The states on the client actual with server. Current state is %s"), *Owner->GetName(), *GetCurrentState()->GetClass()->GetName());				
			}
#endif
		}
	}	
}

void UStateMachine::Init()
{
#if !UE_BUILD_SHIPPING
	UE_LOG(StateMachineRuntime, Log, TEXT("SM instance: %s with Outer: %s"), *GetName(), *GetOuter()->GetName());
	for (const auto StateMachineNode : AllNodes)
	{
		if (const auto State = Cast<UState>(StateMachineNode))
		{
			UE_LOG(StateMachineRuntime, Log, TEXT("SubState: %s with Outer: %s for StateMachine: %s"), *State->GetName(), *State->GetOuter()->GetName() , *GetName());
			//State->Init(InController);
		}
	}
#endif	

	if (OnStateMachineInitialized.IsBound())
	{
		OnStateMachineInitialized.Broadcast(this);
	}

	if (const UWorld *World = GetWorld())
	{
		// If is SERVER (no need to run for Standalone and Client)
		if ((World->GetNetMode() == NM_DedicatedServer) || ((World->GetNetMode() == NM_ListenServer)))
		{
			// Then create timer by MinNetUpdateFrequency			
			World->GetTimerManager().SetTimer(NetMulticastCustomHandler, this, &UStateMachine::OnNetUpdateFrequencyTick, /* TODO: Magic numbers */0.01f/* TODO: Delay time - in config */, true);
		}
	}
}

bool UStateMachine::IsInitiated()
{
	bool bResult = !CurrentStateId.IsEmpty() && CurrentState != nullptr;
	for (const UStateMachineNode *Node : AllNodes)
	{
		bResult &= Node->GetOuter() == this;
	}
	return bResult;
}

UState *UStateMachine::GetStateById(const FString &InStateId)
{
	for (UStateMachineNode *StateNode : AllNodes)
	{
		if (UState *State = Cast<UState>(StateNode))
		{
			if (State->GetStateId().Equals(InStateId))
			{
				return State;
			}
		}
	}
	
	UE_LOG(StateMachineRuntime, Error, TEXT("State by id %s not found."), *InStateId)
	check(false);
	return nullptr;
}

UStateMachine *UStateMachine::AddComponentByTemplate(AActor *Owner, UStateMachine *Template)
{
	if ((Template == nullptr) || (Owner == nullptr))
	{
		return nullptr;
	}

	const FName UniqName = MakeUniqueObjectName(Owner, Template->GetClass(), FName(FString::Printf(TEXT("%s_%s"), *Template->GetClass()->GetName(), *Owner->GetName())));
	if (UActorComponent *NewComponent = Owner->CreateComponentFromTemplate(Template, UniqName))
	{		
		Owner->AddOwnedComponent(NewComponent);
		Owner->AddInstanceComponent(NewComponent);
		NewComponent->RegisterComponent();
		NewComponent->SetIsReplicated(true);

		if (UStateMachine *StateMachine = Cast<UStateMachine>(NewComponent))
		{
			StateMachine->FillFromTemplate(Template);

			return StateMachine;
		}
	}
	return nullptr;
}

UStateMachine::UStateMachine()
{
#if WITH_EDITORONLY_DATA
	bCanBeCyclical = true;
#endif	
}

UStateMachine::~UStateMachine()
{
}

UState *UStateMachine::GetCurrentState() const
{
	return CurrentState;
}

void UStateMachine::SetState(UState *NewState)
{
	CurrentState = NewState;
	if (OnStateChanged.IsBound())
	{
		OnStateChanged.Broadcast(this, CurrentState);
	}

	if (!IsValid(NewState))
	{
		return;	
	}
	
	// Standalone no need caching passed states
	if ((GetWorld()->GetNetMode() != NM_Standalone))
	{
		PassedStates.Enqueue(NewState);
	}

	// TODO: Outer - not necessarily an Actor.
	// Logic for correct replication
	if (const auto Owner = Cast<AActor>(GetOuter()))
	{
		if (Owner->GetLocalRole() == ROLE_Authority)
		{
			CurrentStateId = NewState->GetStateId();
		}
#if !UE_BUILD_SHIPPING
		UE_LOG(StateMachineRuntime, Log, TEXT("[%s][%s] Changed state to %s"),
			Owner->HasAuthority() ? TEXT("Server") : TEXT("Client"),
			*Owner->GetName(),
			*CurrentState->GetName());
#endif
	}
}

void UStateMachine::OnNetUpdateFrequencyTick()
{
	// Prepare data for sending and clean PassedState queue
	TArray<FString> SendData;
	UState *PassedState;
	while (PassedStates.Dequeue(PassedState))
	{
		SendData.Add(PassedState->GetStateId());
	}

	if (!SendData.IsEmpty())
	{
		ReceiveSkippedStatesIds(SendData);
	}
}

void UStateMachine::SetDefaultState()
{
	if (CurrentState != nullptr)
	{
		GetCurrentState()->Leave();
	}
	SetState(GetStateById(CurrentStateId));
	GetCurrentState()->Enter();
}

void UStateMachine::CorrectRevivedPassedStates(TArray<FString> &InOutSkippedStatesClasses)
{
	while (!InOutSkippedStatesClasses.IsEmpty() && !PassedStates.IsEmpty())		
	{
		if ((*PassedStates.Peek()) == GetStateById(InOutSkippedStatesClasses[0]))
		{
			PassedStates.Pop();
			InOutSkippedStatesClasses.RemoveAt(0);			
		}
		else
		{
			PassedStates.Pop();
		}
	}
}

void UStateMachine::FillFromTemplate(const UStateMachine *Template)
{
	check(Template);
	
	// Fill new unique Nodes subobjects
	AllNodes.Empty();
	for (UStateMachineNode *TemplateNode : Template->AllNodes)
	{
		if (const auto TemplateState = Cast<UState>(TemplateNode))
		{
			AllNodes.Add(UState::SpawnUniqueState(this, TemplateState));
		}
	}

	check(AllNodes.Num() == Template->AllNodes.Num());
	
	// Fill all nodes by actual objects
	for(int32 i = 0; i < AllNodes.Num(); ++i)
	{
		if (const auto LocalState = Cast<UState>(AllNodes[i]))
		{
			if (const auto TemplateState = Cast<UState>(Template->AllNodes[i]))
			{
				check(LocalState->GetStateId().Equals(TemplateState->GetStateId()));
				
				LocalState->FillFromTemplate(this, TemplateState);
			}
		}
	}
}
