#include "StateMachineAssetEditor/EditorCommands_StateMachine.h"

#define LOCTEXT_NAMESPACE "EditorCommands_StateMachine"

void FEditorCommands_StateMachine::RegisterCommands()
{
	UI_COMMAND(GraphSettings, "Graph Settings", "Graph Settings", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AutoArrange, "Auto Arrange", "Auto Arrange", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
