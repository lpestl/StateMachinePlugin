#include "StateMachineAssetEditor/SEdNode_StateMachineNode.h"
#include "StateMachineAssetEditor/Colors_StateMachine.h"
#include "SLevelOfDetailBranchNode.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SCommentBubble.h"
#include "SlateOptMacros.h"
#include "SGraphPin.h"
#include "GraphEditorSettings.h"
#include "ScopedTransaction.h"
#include "Components/VerticalBox.h"
#include "GraphData/StateMachineGraph.h"
#include "StateMachineAssetEditor/EdNode_StateMachineNode.h"
#include "StateMachineAssetEditor/StateMachineDragConnection.h"

#define LOCTEXT_NAMESPACE "EdNode_StateMachine"

//////////////////////////////////////////////////////////////////////////
class SStateMachinePin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SStateMachinePin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs, UEdGraphPin *InPin)
	{
		this->SetCursor(EMouseCursor::Default);

		bShowLabel = true;

		GraphPinObj = InPin;
		check(GraphPinObj != nullptr);

		const UEdGraphSchema *Schema = GraphPinObj->GetSchema();
		check(Schema);

		SBorder::Construct(SBorder::FArguments()
			.BorderImage(this, &SStateMachinePin::GetPinBorder)
			.BorderBackgroundColor(this, &SStateMachinePin::GetPinColor)
			.OnMouseButtonDown(this, &SStateMachinePin::OnPinMouseDown)
			.Cursor(this, &SStateMachinePin::GetPinCursor)
			.Padding(FMargin(5.0f))
		);
	}

protected:
	virtual FSlateColor GetPinColor() const override
	{
		return StateMachineColors::Pin::Default;
	}

	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override
	{
		return SNew(STextBlock);
	}

	const FSlateBrush *GetPinBorder() const
	{
		return FAppStyle::GetBrush(TEXT("Graph.StateNode.Body"));
	}

	virtual TSharedRef<FDragDropOperation> SpawnPinDragEvent(const TSharedRef<class SGraphPanel> &InGraphPanel, const TArray< TSharedRef<SGraphPin> > &InStartingPins) override
	{
		FStateMachineDragConnection::FDraggedPinTable PinHandles;
		PinHandles.Reserve(InStartingPins.Num());
		// since the graph can be refreshed and pins can be reconstructed/replaced 
		// behind the scenes, the DragDropOperation holds onto FGraphPinHandles 
		// instead of direct widgets/graph-pins
		for (const TSharedRef<SGraphPin> &PinWidget : InStartingPins)
		{
			PinHandles.Add(PinWidget->GetPinObj());
		}

		return FStateMachineDragConnection::New(InGraphPanel, PinHandles);
	}

};


//////////////////////////////////////////////////////////////////////////
void SEdNode_StateMachineNode::Construct(const FArguments &InArgs, UEdNode_StateMachineNode *InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
	InNode->SEdNode = this;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SEdNode_StateMachineNode::UpdateGraphNode()
{
	const FMargin NodePadding = FMargin(5);
	const FMargin NamePadding = FMargin(2);

	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	const FSlateBrush *NodeTypeIcon = GetNameIcon();

	FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<SVerticalBox> NodeBody;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0.0f)
			.BorderBackgroundColor(this, &SEdNode_StateMachineNode::GetBorderBackgroundColor)
			[
				SNew(SOverlay)

				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SVerticalBox)

					// Input Pin Area
					+ SVerticalBox::Slot()
					.FillHeight(1)
					[
						SAssignNew(LeftNodeBox, SVerticalBox)
					]

					// Output Pin Area	
					+ SVerticalBox::Slot()
					.FillHeight(1)
					[
						SAssignNew(RightNodeBox, SVerticalBox)
					]
				]

				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(8.0f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
					.BorderBackgroundColor(TitleShadowColor)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Visibility(EVisibility::SelfHitTestInvisible)
					.Padding(6.0f)
					[
						SAssignNew(NodeBody, SVerticalBox)
									
						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							// Error message
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(ErrorText, SErrorText)
								.BackgroundColor(this, &SEdNode_StateMachineNode::GetErrorColor)
								.ToolTipText(this, &SEdNode_StateMachineNode::GetErrorMsgToolTip)
							]

							// Icon
							+SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(NodeTypeIcon)
							]
										
							// Node Title
							+ SHorizontalBox::Slot()
							.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SAssignNew(InlineEditableText, SInlineEditableTextBlock)
									.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
									.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
									.OnVerifyTextChanged(this, &SEdNode_StateMachineNode::OnVerifyNameTextChanged)
									.OnTextCommitted(this, &SEdNode_StateMachineNode::OnNameTextCommited)
									.IsReadOnly(this, &SEdNode_StateMachineNode::IsNameReadOnly)
									.IsSelected(this, &SEdNode_StateMachineNode::IsSelectedExclusively)
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									NodeTitle.ToSharedRef()
								]
							]
						]					
					]
				]
			]
		];

	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);
	CreatePinWidgets();
}

void SEdNode_StateMachineNode::CreatePinWidgets()
{
	UEdNode_StateMachineNode *StateNode = CastChecked<UEdNode_StateMachineNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin *MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SStateMachinePin, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SEdNode_StateMachineNode::AddPin(const TSharedRef<SGraphPin> &PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin *PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility( TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced) );
	}

	TSharedPtr<SVerticalBox> PinBox;
	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		PinBox = LeftNodeBox;
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		PinBox = RightNodeBox;
		OutputPins.Add(PinToAdd);
	}

	if (PinBox)
	{
		PinBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			//.Padding(6.0f, 0.0f)
			[
				PinToAdd
			];
	}
}

bool SEdNode_StateMachineNode::IsNameReadOnly() const
{
	UEdNode_StateMachineNode *EdNode_Node = Cast<UEdNode_StateMachineNode>(GraphNode);
	check(EdNode_Node != nullptr);

	UStateMachineGraph *StateMachine = EdNode_Node->StateMachineNode->Graph;
	check(StateMachine != nullptr);

	return (!StateMachine->bCanRenameNode || !EdNode_Node->StateMachineNode->IsNameEditable()) || SGraphNode::IsNameReadOnly();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SEdNode_StateMachineNode::OnNameTextCommited(const FText &InText, ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	UEdNode_StateMachineNode *MyNode = CastChecked<UEdNode_StateMachineNode>(GraphNode);

	if (MyNode != nullptr && MyNode->StateMachineNode != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("StateMachineEditorRenameNode", "State Machine Editor: Rename Node"));
		MyNode->Modify();
		MyNode->StateMachineNode->Modify();
		MyNode->StateMachineNode->SetNodeTitle(InText);
		UpdateGraphNode();
	}
}

FSlateColor SEdNode_StateMachineNode::GetBorderBackgroundColor() const
{
	UEdNode_StateMachineNode *MyNode = CastChecked<UEdNode_StateMachineNode>(GraphNode);
	return MyNode ? MyNode->GetBackgroundColor() : StateMachineColors::NodeBorder::HighlightAbortRange0;
}

FSlateColor SEdNode_StateMachineNode::GetBackgroundColor() const
{
	return StateMachineColors::NodeBody::Default;
}

EVisibility SEdNode_StateMachineNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

const FSlateBrush *SEdNode_StateMachineNode::GetNameIcon() const
{
	return FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

#undef LOCTEXT_NAMESPACE
