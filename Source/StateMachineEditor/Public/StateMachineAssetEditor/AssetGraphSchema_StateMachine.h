#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "AssetGraphSchema_StateMachine.generated.h"

class UEdGraph;
class UToolMenu;
class UEdGraphPin;
class UGraphNodeContextMenuContext;
class FConnectionDrawingPolicy;
class FSlateWindowElementList;
class UEdNode_StateMachineNode;
class UEdNode_StateMachineEdge;
class UAutoLayoutStrategy;

/** Action to add a node to the graph */
USTRUCT()
struct STATEMACHINEEDITOR_API FAssetSchemaAction_StateMachine_NewNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

public:
	FAssetSchemaAction_StateMachine_NewNode(): NodeTemplate(nullptr) {}

	FAssetSchemaAction_StateMachine_NewNode(const FText &InNodeCategory, const FText &InMenuDesc, const FText &InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode *PerformAction(UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector &Collector) override;

	UEdNode_StateMachineNode *NodeTemplate;
};

USTRUCT()
struct STATEMACHINEEDITOR_API FAssetSchemaAction_StateMachine_NewEdge : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

public:
	FAssetSchemaAction_StateMachine_NewEdge(): NodeTemplate(nullptr){}

	FAssetSchemaAction_StateMachine_NewEdge(const FText &InNodeCategory, const FText &InMenuDesc, const FText &InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode *PerformAction(UEdGraph *ParentGraph, UEdGraphPin *FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector &Collector) override;

	UEdNode_StateMachineEdge *NodeTemplate;
};

UCLASS(MinimalAPI)
class UAssetGraphSchema_StateMachine : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	void GetBreakLinkToSubMenuActions(UToolMenu *Menu, UEdGraphPin *InGraphPin);

	virtual EGraphType GetGraphType(const UEdGraph *TestEdGraph) const override;

 	virtual void GetGraphContextActions(FGraphContextMenuBuilder &ContextMenuBuilder) const override;

	virtual void GetContextMenuActions(UToolMenu *Menu, UGraphNodeContextMenuContext *Context) const override;

 	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin *A, const UEdGraphPin *B) const override;

	virtual bool TryCreateConnection(UEdGraphPin *A, UEdGraphPin *B) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin *A, UEdGraphPin *B) const override;

	virtual FConnectionDrawingPolicy *CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect &InClippingRect, FSlateWindowElementList &InDrawElements, UEdGraph *InGraphObj) const override;

 	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType &PinType) const override;

 	virtual void BreakNodeLinks(UEdGraphNode &TargetNode) const override;

 	virtual void BreakPinLinks(UEdGraphPin &TargetPin, bool bSendsNodeNotifcation) const override;

	virtual void BreakSinglePinLink(UEdGraphPin *SourcePin, UEdGraphPin *TargetPin) const override;

	virtual UEdGraphPin *DropPinOnNode(UEdGraphNode *InTargetNode, const FName &InSourcePinName, const FEdGraphPinType &InSourcePinType, EEdGraphPinDirection InSourcePinDirection) const override;

	virtual bool SupportsDropPinOnNode(UEdGraphNode *InTargetNode, const FEdGraphPinType &InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText &OutErrorMessage) const override;

	virtual bool IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const override;

	virtual int32 GetCurrentVisualizationCacheID() const override;

	virtual void ForceVisualizationCacheClear() const override;

private:
	static int32 CurrentCacheRefreshID;
};

