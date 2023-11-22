// Created by Mikhail S. Kataev - mkataev@temporal-games.com

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Engine/EngineTypes.h"
#include "GraphData/StateMachineGraph.h"
#include "StateMachine.generated.h"

class UState;
class UStateMachine;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChanged, UStateMachine *, StateMachine, UState *, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateMachineInitialized, UStateMachine *, StateMachine);

UCLASS(Blueprintable, BlueprintType)
class STATEMACHINERUNTIME_API UStateMachine : public UStateMachineGraph
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category=StateMachine)
	static UStateMachine *AddComponentByTemplate( AActor *Owner, UStateMachine *Template);
public:
	UStateMachine();
	virtual ~UStateMachine();
	
	UFUNCTION(BlueprintCallable)
	void Run();

	UFUNCTION(BlueprintCallable)
	void Stop();
	
	UFUNCTION(BlueprintCallable)
	virtual bool TryGoToNextState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;	
	virtual bool IsSupportedForNetworking() const override { return true; }

	UFUNCTION(NetMulticast, Reliable)
	void ReceiveSkippedStatesIds(const TArray<FString> &InSkippedStatesIds);

	virtual void Init();	
	bool IsInitiated();
	
	UFUNCTION(BlueprintCallable)
	void FillFromTemplate(const UStateMachine *Template);
	
	UState *GetStateById(const FString &InStateId);

	FOnStateChanged OnStateChanged;
	FOnStateMachineInitialized OnStateMachineInitialized;

	UFUNCTION(BlueprintCallable)
	UState *GetCurrentState() const;

protected:
	UFUNCTION(BlueprintCallable)
	virtual void SetState(UState *NewState);

	/* Tick for custom replication (RPC sending data). Called only on server */
	UFUNCTION()
	void OnNetUpdateFrequencyTick();

	void SetDefaultState();
	void CorrectRevivedPassedStates(TArray<FString> &InOutSkippedStatesClasses);
	
	/*
	 * Stores the values of states that have
	 * already been passed since the last synchronization.
	 */	
	TQueue<UState *> PassedStates;

	/*
	 * NOTE: This property is replicated to replicate the entire state machine object.
	 * By the way, the value that comes to the client does not play any role and is not used.
	 */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	FString CurrentStateId;
	
private:
	UPROPERTY()
	UState *CurrentState;

	/*
	 * For clients only. Stores a queue of states that the server has already passed,
	 * but the client has not yet passed and that need to be passed.
	 */
	TQueue<UState *> FutureStates;
	
	/* Timer handle */
	FTimerHandle NetMulticastCustomHandler;
};
