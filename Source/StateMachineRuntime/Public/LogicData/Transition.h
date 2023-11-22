// Created by Mikhail S. Kataev - mkataev@temporal-games.com

#pragma once

#include "CoreMinimal.h"
#include "GraphData/StateMachineEdge.h"
#include "Transition.generated.h"

UCLASS()
class STATEMACHINERUNTIME_API UTransition : public UStateMachineEdge
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent)
	bool CanTransitTo();
	virtual bool CanTransitTo_Implementation()
	{
		const auto StartState = Cast<UState>(StartNode);
		const auto EndState = Cast<UState>(EndNode);
		if ((StartState != nullptr) && (EndState != nullptr))
		{
			return StartState->CanTransitTo(EndState);
		}
		
		check(false);
		return false;
	}
};
