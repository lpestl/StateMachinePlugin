#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Styling/AppStyle.h"

class STATEMACHINEEDITOR_API FEditorCommands_StateMachine : public TCommands<FEditorCommands_StateMachine>
{
public:
	/** Constructor */
	FEditorCommands_StateMachine()
		: TCommands<FEditorCommands_StateMachine>("StateMachineEditor", NSLOCTEXT("Contexts", "StateMachineEditor", "State Machine Editor"), NAME_None, FAppStyle::GetAppStyleSetName())
	{
	}
	
	TSharedPtr<FUICommandInfo> GraphSettings;
	TSharedPtr<FUICommandInfo> AutoArrange;

	virtual void RegisterCommands() override;
};
