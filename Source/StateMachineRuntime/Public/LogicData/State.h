// Created by Mikhail S. Kataev - mkataev@temporal-games.com

#pragma once

#include "CoreMinimal.h"
#include "GraphData/StateMachineNode.h"
#include "State.generated.h"

class UState;
class UStateMachine;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateActivationChanged, UState *, State);

UCLASS(Blueprintable, BlueprintType)
class STATEMACHINERUNTIME_API UState : public UStateMachineNode
{
	GENERATED_BODY()
	
public:
	UState();
	virtual ~UState();
	
	UFUNCTION(BlueprintNativeEvent)
	void Enter();
	virtual void Enter_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	void Leave();
	virtual void Leave_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	bool CanTransitTo(UState *ToState);
	virtual bool CanTransitTo_Implementation(UState *ToState);
	
	UFUNCTION(BlueprintCallable)
	bool TryGoToNextState();
	
	UFUNCTION(BlueprintCallable)
	const FString &GetStateId() const;
	
public:	
	UFUNCTION(BlueprintCallable)
	virtual UWorld *GetWorld() const override;
	
	static UState *SpawnUniqueState(UObject *Outer, UState *Template, EObjectFlags Flags = RF_NoFlags);
	void FillFromTemplate(UStateMachine *StateMachine, UState *Template);
	
	UFUNCTION(BlueprintCallable)
	UStateMachine *GetStateMachine() const;
	
public:
	FOnStateActivationChanged OnStateEntered;
	FOnStateActivationChanged OnStateLeaved;
	
protected:
	UFUNCTION(BlueprintNativeEvent)
	void BindDynamics();
	virtual void BindDynamics_Implementation() {}

	UFUNCTION(BlueprintNativeEvent)
	void UnbindDynamics();
	virtual void UnbindDynamics_Implementation() {}
};
