#pragma once

#include "Modules/ModuleManager.h"
#include "StateMachineEditorModule.h"
#include <IAssetTools.h>
#include <EdGraphUtilities.h>

class FStateMachineEditor : public IStateMachineEditor
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


private:
	void RegisterAssetTypeAction(IAssetTools &AssetTools, TSharedRef<IAssetTypeActions> Action);

private:
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	EAssetTypeCategories::Type StateMachineAssetCategoryBit;

	TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory_StateMachine;
};

IMPLEMENT_MODULE(FStateMachineEditor, StateMachineEditor)