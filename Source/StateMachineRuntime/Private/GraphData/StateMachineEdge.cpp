#include "GraphData/StateMachineEdge.h"

UStateMachineEdge::UStateMachineEdge()
{
}

UStateMachineEdge::~UStateMachineEdge()
{
}

UStateMachineGraph *UStateMachineEdge::GetGraph() const
{
	return Graph;
}

#if WITH_EDITOR
void UStateMachineEdge::SetNodeTitle(const FText &NewTitle)
{
	NodeTitle = NewTitle;
}
#endif // #if WITH_EDITOR