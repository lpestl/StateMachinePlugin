
#pragma once

#include "CoreMinimal.h"

class FAssetEditor_StateMachine;
class FExtender;
class FToolBarBuilder;

class STATEMACHINEEDITOR_API FAssetEditorToolbar_StateMachine : public TSharedFromThis<FAssetEditorToolbar_StateMachine>
{
public:
	FAssetEditorToolbar_StateMachine(TSharedPtr<FAssetEditor_StateMachine> InStateMachineEditor)
		: StateMachineEditor(InStateMachineEditor) {}

	void AddStateMachineToolbar(TSharedPtr<FExtender> Extender);

private:
	void FillStateMachineToolbar(FToolBarBuilder &ToolbarBuilder);

protected:
	/** Pointer back to the blueprint editor tool that owns us */
	TWeakPtr<FAssetEditor_StateMachine> StateMachineEditor;
};
