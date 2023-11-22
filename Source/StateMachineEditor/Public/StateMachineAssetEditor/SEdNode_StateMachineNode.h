#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UEdNode_StateMachineNode;

class STATEMACHINEEDITOR_API SEdNode_StateMachineNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SEdNode_StateMachineNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs, UEdNode_StateMachineNode *InNode);

	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin> &PinToAdd) override;
	virtual bool IsNameReadOnly() const override;

	void OnNameTextCommited(const FText &InText, ETextCommit::Type CommitInfo);

	virtual FSlateColor GetBorderBackgroundColor() const;
	virtual FSlateColor GetBackgroundColor() const;

	virtual EVisibility GetDragOverMarkerVisibility() const;

	virtual const FSlateBrush *GetNameIcon() const;

protected:
};

