// Created by Mikhail S. Kataev - mkataev@temporal-games.com

#pragma once

#include "CoreMinimal.h"
#include "State.h"
#include "ClientConformationsState.generated.h"

/**
 * 
 */
UCLASS()
class STATEMACHINERUNTIME_API UClientConformationsState : public UState
{
	GENERATED_BODY()

public:
	virtual void Enter_Implementation() override;
	virtual void Leave_Implementation() override;
	
	void ConfirmLeave();

	bool IsLeaveConfirmed() const;

private:
	bool bIsLeaveConfirmedForClient = false;
};
