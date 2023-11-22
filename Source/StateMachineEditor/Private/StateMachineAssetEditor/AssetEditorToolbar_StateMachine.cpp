#include "StateMachineAssetEditor/AssetEditorToolbar_StateMachine.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "StateMachineAssetEditor/AssetEditor_StateMachine.h"
#include "StateMachineAssetEditor/EditorCommands_StateMachine.h"
#include "StateMachineAssetEditor/StateMachineEditorStyle.h"

#define LOCTEXT_NAMESPACE "AssetEditorToolbar_StateMachine"

void FAssetEditorToolbar_StateMachine::AddStateMachineToolbar(TSharedPtr<FExtender> Extender)
{
	check(StateMachineEditor.IsValid());
	TSharedPtr<FAssetEditor_StateMachine> StateMachineEditorPtr = StateMachineEditor.Pin();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, StateMachineEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP( this, &FAssetEditorToolbar_StateMachine::FillStateMachineToolbar ));
	StateMachineEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FAssetEditorToolbar_StateMachine::FillStateMachineToolbar(FToolBarBuilder &ToolbarBuilder)
{
	check(StateMachineEditor.IsValid());
	TSharedPtr<FAssetEditor_StateMachine> StateMachineEditorPtr = StateMachineEditor.Pin();

	ToolbarBuilder.BeginSection("State Machine");
	{
		ToolbarBuilder.AddToolBarButton(FEditorCommands_StateMachine::Get().GraphSettings,
			NAME_None,
			LOCTEXT("GraphSettings_Label", "Graph Settings"),
			LOCTEXT("GraphSettings_ToolTip", "Show the Graph Settings"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.GameSettings"));
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Util");
	{
		ToolbarBuilder.AddToolBarButton(FEditorCommands_StateMachine::Get().AutoArrange,
			NAME_None,
			LOCTEXT("AutoArrange_Label", "Auto Arrange"),
			LOCTEXT("AutoArrange_ToolTip", "Auto arrange nodes, not perfect, but still handy"),
			FSlateIcon(FStateMachineEditorStyle::GetStyleSetName(), "StateMachineEditor.AutoArrange"));
	}
	ToolbarBuilder.EndSection();

}


#undef LOCTEXT_NAMESPACE
