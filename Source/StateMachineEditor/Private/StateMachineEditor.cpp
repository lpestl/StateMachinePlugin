#include "StateMachineEditor.h"
#include "StateMachineNodeFactory.h"
#include "AssetTypeActions_StateMachine.h"
#include "StateMachineAssetEditor/StateMachineEditorStyle.h"

DEFINE_LOG_CATEGORY(StateMachineEditor)

#define LOCTEXT_NAMESPACE "Editor_StateMachine"

void FStateMachineEditor::StartupModule()
{
	FStateMachineEditorStyle::Initialize();

	GraphPanelNodeFactory_StateMachine = MakeShareable(new FGraphPanelNodeFactory_StateMachine());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_StateMachine);

	IAssetTools &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	StateMachineAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("StateMachine")), LOCTEXT("StateMachineAssetCategory", "StateMachine"));

	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_StateMachine(StateMachineAssetCategoryBit)));
}


void FStateMachineEditor::ShutdownModule()
{
	// Unregister all the asset types that we registered
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools &AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}

	if (GraphPanelNodeFactory_StateMachine.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_StateMachine);
		GraphPanelNodeFactory_StateMachine.Reset();
	}

	FStateMachineEditorStyle::Shutdown();
}

void FStateMachineEditor::RegisterAssetTypeAction(IAssetTools &AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE

