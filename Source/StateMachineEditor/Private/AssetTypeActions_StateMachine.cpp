#include "AssetTypeActions_StateMachine.h"
#include "StateMachineAssetEditor/AssetEditor_StateMachine.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_StateMachine"

FAssetTypeActions_StateMachine::FAssetTypeActions_StateMachine(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FAssetTypeActions_StateMachine::GetName() const
{
	return LOCTEXT("FStateMachineAssetTypeActionsName", "State Machine");
}

FColor FAssetTypeActions_StateMachine::GetTypeColor() const
{
	return FColor(129, 196, 115);
}

UClass *FAssetTypeActions_StateMachine::GetSupportedClass() const
{
	return UStateMachineGraph::StaticClass();
}

void FAssetTypeActions_StateMachine::OpenAssetEditor(const TArray<UObject*> &InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UStateMachineGraph *Graph = Cast<UStateMachineGraph>(*ObjIt))
		{
			TSharedRef<FAssetEditor_StateMachine> NewGraphEditor(new FAssetEditor_StateMachine());
			NewGraphEditor->InitStateMachineAssetEditor(Mode, EditWithinLevelEditor, Graph);
		}
	}
}

uint32 FAssetTypeActions_StateMachine::GetCategories()
{
	return MyAssetCategory;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE