#include "StateMachineAssetEditor/AssetEditor_StateMachine.h"
#include "StateMachineAssetEditor/AssetEditorToolbar_StateMachine.h"
#include "StateMachineAssetEditor/AssetGraphSchema_StateMachine.h"
#include "StateMachineAssetEditor/EditorCommands_StateMachine.h"
#include "StateMachineAssetEditor/EdGraph_StateMachine.h"
#include "AssetToolsModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Editor/UnrealEd/Public/Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EdGraphUtilities.h"
#include "ScopedTransaction.h"
#include "StateMachineAssetEditor/EdGraph_StateMachine.h"
#include "StateMachineAssetEditor/EdNode_StateMachineNode.h"
#include "StateMachineAssetEditor/EdNode_StateMachineEdge.h"
#include "AutoLayout/TreeLayoutStrategy.h"
#include "AutoLayout/ForceDirectedLayoutStrategy.h"
#include "Widgets/Docking/SDockTab.h"
#if ENGINE_MAJOR_VERSION == 5
#include "UObject/ObjectSaveContext.h"
#endif // #if ENGINE_MAJOR_VERSION == 5

#define LOCTEXT_NAMESPACE "AssetEditor_StateMachine"

const FName StateMachineEditorAppName = FName(TEXT("StateMachineEditorApp"));

struct FStateMachineAssetEditorTabs
{
	// Tab identifiers
	static const FName StateMachinePropertyID;
	static const FName ViewportID;
	static const FName StateMachineEditorSettingsID;
};

//////////////////////////////////////////////////////////////////////////

const FName FStateMachineAssetEditorTabs::StateMachinePropertyID(TEXT("StateMachineProperty"));
const FName FStateMachineAssetEditorTabs::ViewportID(TEXT("Viewport"));
const FName FStateMachineAssetEditorTabs::StateMachineEditorSettingsID(TEXT("StateMachineEditorSettings"));

//////////////////////////////////////////////////////////////////////////

FAssetEditor_StateMachine::FAssetEditor_StateMachine()
{
	EditingGraph = nullptr;

	StateMachineEditorSettings = NewObject<UStateMachineEditorSettings>(UStateMachineEditorSettings::StaticClass());

#if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FAssetEditor_StateMachine::OnPackageSaved);
#else // #if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedWithContextEvent.AddRaw(this, &FAssetEditor_StateMachine::OnPackageSavedWithContext);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

FAssetEditor_StateMachine::~FAssetEditor_StateMachine()
{
#if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
#else // #if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedWithContextEvent.Remove(OnPackageSavedDelegateHandle);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

void FAssetEditor_StateMachine::InitStateMachineAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost > &InitToolkitHost, UStateMachineGraph *Graph)
{
	EditingGraph = Graph;
	CreateEdGraph();

	FGenericCommands::Register();
	FGraphEditorCommands::Register();
	FEditorCommands_StateMachine::Register();

	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FAssetEditorToolbar_StateMachine(SharedThis(this)));
	}

	BindCommands();

	CreateInternalWidgets();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarBuilder->AddStateMachineToolbar(ToolbarExtender);

	// Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_StateMachineEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
#if ENGINE_MAJOR_VERSION < 5
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)->SetHideTabWell(true)
			)
#endif // #if ENGINE_MAJOR_VERSION < 5
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.65f)
					->AddTab(FStateMachineAssetEditorTabs::ViewportID, ETabState::OpenedTab)->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.7f)
						->AddTab(FStateMachineAssetEditorTabs::StateMachinePropertyID, ETabState::OpenedTab)->SetHideTabWell(true)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.3f)
						->AddTab(FStateMachineAssetEditorTabs::StateMachineEditorSettingsID, ETabState::OpenedTab)
					)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, StateMachineEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, EditingGraph, false);

	RegenerateMenusAndToolbars();
}

void FAssetEditor_StateMachine::RegisterTabSpawners(const TSharedRef<FTabManager> &InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_StateMachineEditor", "State Machine Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FStateMachineAssetEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FAssetEditor_StateMachine::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FStateMachineAssetEditorTabs::StateMachinePropertyID, FOnSpawnTab::CreateSP(this, &FAssetEditor_StateMachine::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Property"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FStateMachineAssetEditorTabs::StateMachineEditorSettingsID, FOnSpawnTab::CreateSP(this, &FAssetEditor_StateMachine::SpawnTab_EditorSettings))
		.SetDisplayName(LOCTEXT("EditorSettingsTab", "State Machine Editor Setttings"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FAssetEditor_StateMachine::UnregisterTabSpawners(const TSharedRef<FTabManager> &InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FStateMachineAssetEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FStateMachineAssetEditorTabs::StateMachinePropertyID);
	InTabManager->UnregisterTabSpawner(FStateMachineAssetEditorTabs::StateMachineEditorSettingsID);
}

FName FAssetEditor_StateMachine::GetToolkitFName() const
{
	return FName("FStateMachineEditor");
}

FText FAssetEditor_StateMachine::GetBaseToolkitName() const
{
	return LOCTEXT("StateMachineEditorAppLabel", "State Machine Editor");
}

FText FAssetEditor_StateMachine::GetToolkitName() const
{
	const bool bDirtyState = EditingGraph->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("StateMachineName"), FText::FromString(EditingGraph->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("StateMachineEditorToolkitName", "{StateMachineName}{DirtyState}"), Args);
}

FText FAssetEditor_StateMachine::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingGraph);
}

FLinearColor FAssetEditor_StateMachine::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FAssetEditor_StateMachine::GetWorldCentricTabPrefix() const
{
	return TEXT("StateMachineEditor");
}

FString FAssetEditor_StateMachine::GetDocumentationLink() const
{
	return TEXT("");
}

void FAssetEditor_StateMachine::SaveAsset_Execute()
{
	if (EditingGraph != nullptr)
	{
		RebuildStateMachine();
	}

	FAssetEditorToolkit::SaveAsset_Execute();
}

void FAssetEditor_StateMachine::AddReferencedObjects(FReferenceCollector &Collector)
{
	Collector.AddReferencedObject(EditingGraph);
	Collector.AddReferencedObject(EditingGraph->EdGraph);
}

UStateMachineEditorSettings *FAssetEditor_StateMachine::GetSettings() const
{
	return StateMachineEditorSettings;
}

TSharedRef<SDockTab> FAssetEditor_StateMachine::SpawnTab_Viewport(const FSpawnTabArgs &Args)
{
	check(Args.GetTabId() == FStateMachineAssetEditorTabs::ViewportID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"));

	if (ViewportWidget.IsValid())
	{
		SpawnedTab->SetContent(ViewportWidget.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FAssetEditor_StateMachine::SpawnTab_Details(const FSpawnTabArgs &Args)
{
	check(Args.GetTabId() == FStateMachineAssetEditorTabs::StateMachinePropertyID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("Details_Title", "Property"))
		[
			PropertyWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FAssetEditor_StateMachine::SpawnTab_EditorSettings(const FSpawnTabArgs &Args)
{
	check(Args.GetTabId() == FStateMachineAssetEditorTabs::StateMachineEditorSettingsID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("EditorSettings_Title", "State Machine Editor Setttings"))
		[
			EditorSettingsWidget.ToSharedRef()
		];
}

void FAssetEditor_StateMachine::CreateInternalWidgets()
{
	ViewportWidget = CreateViewportWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule &PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyWidget = PropertyModule.CreateDetailView(Args);
	PropertyWidget->SetObject(EditingGraph);
	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FAssetEditor_StateMachine::OnFinishedChangingProperties);

	EditorSettingsWidget = PropertyModule.CreateDetailView(Args);
	EditorSettingsWidget->SetObject(StateMachineEditorSettings);
}

TSharedRef<SGraphEditor> FAssetEditor_StateMachine::CreateViewportWidget()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_StateMachine", "State Machine");

	CreateCommandList();

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FAssetEditor_StateMachine::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FAssetEditor_StateMachine::OnNodeDoubleClicked);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(EditingGraph->EdGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}

void FAssetEditor_StateMachine::BindCommands()
{
	ToolkitCommands->MapAction(FEditorCommands_StateMachine::Get().GraphSettings,
		FExecuteAction::CreateSP(this, &FAssetEditor_StateMachine::GraphSettings),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_StateMachine::CanGraphSettings)
	);

	ToolkitCommands->MapAction(FEditorCommands_StateMachine::Get().AutoArrange,
		FExecuteAction::CreateSP(this, &FAssetEditor_StateMachine::AutoArrange),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_StateMachine::CanAutoArrange)
	);
}

void FAssetEditor_StateMachine::CreateEdGraph()
{
	if (EditingGraph->EdGraph == nullptr)
	{
		EditingGraph->EdGraph = CastChecked<UEdGraph_StateMachine>(FBlueprintEditorUtils::CreateNewGraph(EditingGraph, NAME_None, UEdGraph_StateMachine::StaticClass(), UAssetGraphSchema_StateMachine::StaticClass()));
		EditingGraph->EdGraph->bAllowDeletion = false;

		// Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema *Schema = EditingGraph->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*EditingGraph->EdGraph);
	}
}

void FAssetEditor_StateMachine::CreateCommandList()
{
	if (GraphEditorCommands.IsValid())
	{
		return;
	}

	GraphEditorCommands = MakeShareable(new FUICommandList);

	// Can't use CreateSP here because derived editor are already implementing TSharedFromThis<FAssetEditorToolkit>
	// however it should be safe, since commands are being used only within this editor
	// if it ever crashes, this function will have to go away and be reimplemented in each derived class

	GraphEditorCommands->MapAction(FEditorCommands_StateMachine::Get().GraphSettings,
		FExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::GraphSettings),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CanGraphSettings));

	GraphEditorCommands->MapAction(FEditorCommands_StateMachine::Get().AutoArrange,
		FExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::AutoArrange),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CanAutoArrange));

	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CanCutNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CanPasteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FAssetEditor_StateMachine::CanDuplicateNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &FAssetEditor_StateMachine::OnRenameNode),
		FCanExecuteAction::CreateSP(this, &FAssetEditor_StateMachine::CanRenameNodes)
	);
}

TSharedPtr<SGraphEditor> FAssetEditor_StateMachine::GetCurrGraphEditor() const
{
	return ViewportWidget;
}

FGraphPanelSelectionSet FAssetEditor_StateMachine::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	TSharedPtr<SGraphEditor> FocusedGraphEd = GetCurrGraphEditor();
	if (FocusedGraphEd.IsValid())
	{
		CurrentSelection = FocusedGraphEd->GetSelectedNodes();
	}

	return CurrentSelection;
}

void FAssetEditor_StateMachine::RebuildStateMachine()
{
	if (EditingGraph == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("FStateMachineAssetEditor::RebuildStateMachine EditingGraph is nullptr"));
		return;
	}

	UEdGraph_StateMachine *EdGraph = Cast<UEdGraph_StateMachine>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	EdGraph->RebuildStateMachine();
}

void FAssetEditor_StateMachine::SelectAllNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		CurrentGraphEditor->SelectAllNodes();
	}
}

bool FAssetEditor_StateMachine::CanSelectAllNodes()
{
	return true;
}

void FAssetEditor_StateMachine::DeleteSelectedNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());

	CurrentGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode *EdNode = Cast<UEdGraphNode>(*NodeIt);
		if (EdNode == nullptr || !EdNode->CanUserDeleteNode())
			continue;;

		if (UEdNode_StateMachineNode *EdNode_Node = Cast<UEdNode_StateMachineNode>(EdNode))
		{
			EdNode_Node->Modify();

			const UEdGraphSchema *Schema = EdNode_Node->GetSchema();
			if (Schema != nullptr)
			{
				Schema->BreakNodeLinks(*EdNode_Node);
			}

			EdNode_Node->DestroyNode();
		}
		else
		{
			EdNode->Modify();
			EdNode->DestroyNode();
		}
	}
}

bool FAssetEditor_StateMachine::CanDeleteNodes()
{
	// If any of the nodes can be deleted then we should allow deleting
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode *Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node != nullptr && Node->CanUserDeleteNode())
		{
			return true;
		}
	}

	return false;
}

void FAssetEditor_StateMachine::DeleteSelectedDuplicatableNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FGraphPanelSelectionSet OldSelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode *Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode *Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}
}

void FAssetEditor_StateMachine::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
}

bool FAssetEditor_StateMachine::CanCutNodes()
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FAssetEditor_StateMachine::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode *Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node == nullptr)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}

		if (UEdNode_StateMachineEdge *EdNode_Edge = Cast<UEdNode_StateMachineEdge>(*SelectedIter))
		{
			UEdNode_StateMachineNode *StartNode = EdNode_Edge->GetStartNode();
			UEdNode_StateMachineNode *EndNode = EdNode_Edge->GetEndNode();

			if (!SelectedNodes.Contains(StartNode) || !SelectedNodes.Contains(EndNode))
			{
				SelectedIter.RemoveCurrent();
				continue;
			}
		}

		Node->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool FAssetEditor_StateMachine::CanCopyNodes()
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode *Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FAssetEditor_StateMachine::PasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		PasteNodesHere(CurrentGraphEditor->GetPasteLocation());
	}
}

void FAssetEditor_StateMachine::PasteNodesHere(const FVector2D &Location)
{
	// Find the graph editor with focus
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}
	// Select the newly pasted stuff
	UEdGraph *EdGraph = CurrentGraphEditor->GetCurrentGraph();

	{
		const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
		EdGraph->Modify();

		// Clear the selection set (newly pasted stuff will be selected)
		CurrentGraphEditor->ClearSelectionSet();

		// Grab the text to paste from the clipboard.
		FString TextToImport;
		FPlatformApplicationMisc::ClipboardPaste(TextToImport);

		// Import the nodes
		TSet<UEdGraphNode*> PastedNodes;
		FEdGraphUtilities::ImportNodesFromText(EdGraph, TextToImport, PastedNodes);

		//Average position of nodes so we can move them while still maintaining relative distances to each other
		FVector2D AvgNodePosition(0.0f, 0.0f);

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode *Node = *It;
			AvgNodePosition.X += Node->NodePosX;
			AvgNodePosition.Y += Node->NodePosY;
		}

		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode *Node = *It;
			CurrentGraphEditor->SetNodeSelection(Node, true);

			Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
			Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

			Node->SnapToGrid(16);

			// Give new node a different Guid from the old one
			Node->CreateNewGuid();
		}
	}

	// Update UI
	CurrentGraphEditor->NotifyGraphChanged();

	UObject *GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}
}

bool FAssetEditor_StateMachine::CanPasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return false;
	}

	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(CurrentGraphEditor->GetCurrentGraph(), ClipboardContent);
}

void FAssetEditor_StateMachine::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FAssetEditor_StateMachine::CanDuplicateNodes()
{
	return CanCopyNodes();
}

void FAssetEditor_StateMachine::GraphSettings()
{
	PropertyWidget->SetObject(EditingGraph);
}

bool FAssetEditor_StateMachine::CanGraphSettings() const
{
	return true;
}

void FAssetEditor_StateMachine::AutoArrange()
{
	UEdGraph_StateMachine *EdGraph = Cast<UEdGraph_StateMachine>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	const FScopedTransaction Transaction(LOCTEXT("StateMachineEditorAutoArrange", "State Machine Editor: Auto Arrange"));

	EdGraph->Modify();

	UAutoLayoutStrategy *LayoutStrategy = nullptr;
	switch (StateMachineEditorSettings->AutoLayoutStrategy)
	{
	case EAutoLayoutStrategy::Tree:
		LayoutStrategy = NewObject<UAutoLayoutStrategy>(EdGraph, UTreeLayoutStrategy::StaticClass());
		break;
	case EAutoLayoutStrategy::ForceDirected:
		LayoutStrategy = NewObject<UAutoLayoutStrategy>(EdGraph, UForceDirectedLayoutStrategy::StaticClass());
		break;
	default:
		break;
	}

	if (LayoutStrategy != nullptr)
	{
		LayoutStrategy->Settings = StateMachineEditorSettings;
		LayoutStrategy->Layout(EdGraph);
		LayoutStrategy->ConditionalBeginDestroy();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAssetEditor_StateMachine::AutoArrange LayoutStrategy is null."));
	}
}

bool FAssetEditor_StateMachine::CanAutoArrange() const
{
	return EditingGraph != nullptr && Cast<UEdGraph_StateMachine>(EditingGraph->EdGraph) != nullptr;
}

void FAssetEditor_StateMachine::OnRenameNode()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UEdGraphNode *SelectedNode = Cast<UEdGraphNode>(*NodeIt);
			if (SelectedNode != NULL && SelectedNode->bCanRenameNode)
			{
				CurrentGraphEditor->IsNodeTitleVisible(SelectedNode, true);
				break;
			}
		}
	}
}

bool FAssetEditor_StateMachine::CanRenameNodes() const
{
	UEdGraph_StateMachine *EdGraph = Cast<UEdGraph_StateMachine>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	UStateMachineGraph *Graph = EdGraph->GetStateMachine();
	check(Graph != nullptr)

	return Graph->bCanRenameNode && GetSelectedNodes().Num() == 1;
}

void FAssetEditor_StateMachine::OnSelectedNodesChanged(const TSet<class UObject*> &NewSelection)
{
	TArray<UObject*> Selection;

	for (UObject *SelectionEntry : NewSelection)
	{
		Selection.Add(SelectionEntry);
	}

	if (Selection.Num() == 0) 
	{
		PropertyWidget->SetObject(EditingGraph);

	}
	else
	{
		PropertyWidget->SetObjects(Selection);
	}
}

void FAssetEditor_StateMachine::OnNodeDoubleClicked(UEdGraphNode *Node)
{
	
}

void FAssetEditor_StateMachine::OnFinishedChangingProperties(const FPropertyChangedEvent &PropertyChangedEvent)
{
	if (EditingGraph == nullptr)
		return;

	EditingGraph->EdGraph->GetSchema()->ForceVisualizationCacheClear();
}

#if ENGINE_MAJOR_VERSION < 5
void FAssetEditor_StateMachine::OnPackageSaved(const FString &PackageFileName, UObject *Outer)
{
	RebuildStateMachine();
}
#else // #if ENGINE_MAJOR_VERSION < 5
void FAssetEditor_StateMachine::OnPackageSavedWithContext(const FString &PackageFileName, UPackage *Package, FObjectPostSaveContext ObjectSaveContext)
{
	RebuildStateMachine();
}
#endif // #else // #if ENGINE_MAJOR_VERSION < 5

void FAssetEditor_StateMachine::RegisterToolbarTab(const TSharedRef<class FTabManager> &InTabManager) 
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

#undef LOCTEXT_NAMESPACE

