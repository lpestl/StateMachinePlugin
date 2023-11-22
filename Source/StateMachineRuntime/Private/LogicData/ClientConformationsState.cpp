// Created by Mikhail S. Kataev - mkataev@temporal-games.com


#include "LogicData/ClientConformationsState.h"

void UClientConformationsState::Enter_Implementation()
{
	bIsLeaveConfirmedForClient = false;
	
	Super::Enter_Implementation();
}

void UClientConformationsState::Leave_Implementation()
{
	Super::Leave_Implementation();
}

void UClientConformationsState::ConfirmLeave()
{
	bIsLeaveConfirmedForClient = true;
}

bool UClientConformationsState::IsLeaveConfirmed() const
{
	return bIsLeaveConfirmedForClient;
}