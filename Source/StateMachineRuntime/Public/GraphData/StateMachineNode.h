#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "StateMachineNode.generated.h"

class UStateMachineGraph;
class UStateMachineEdge;

UENUM(BlueprintType)
enum class ENodeLimit : uint8
{
	Unlimited,
    Limited
};


UCLASS(Abstract, Blueprintable)
class STATEMACHINERUNTIME_API UStateMachineNode : public UObject
{
	GENERATED_BODY()

public:
	UStateMachineNode();
	virtual ~UStateMachineNode();

	UPROPERTY(VisibleDefaultsOnly, Category = "StateMachineNode")
	UStateMachineGraph *Graph;

	UPROPERTY(BlueprintReadOnly, Category = "StateMachineNode")
	TArray<UStateMachineNode*> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "StateMachineNode")
	TArray<UStateMachineNode*> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "StateMachineNode")
	TMap<UStateMachineNode*, UStateMachineEdge*> Edges;

	UPROPERTY(EditAnywhere, Category = "State")
	FString Id;

	UFUNCTION(BlueprintCallable, Category = "StateMachineNode")
	virtual UStateMachineEdge *GetEdge(UStateMachineNode *ChildNode);

	UFUNCTION(BlueprintCallable, Category = "StateMachineNode")
	bool IsLeafNode() const;

	UFUNCTION(BlueprintCallable, Category = "StateMachineNode")
	UStateMachineGraph *GetGraph() const;
	
	UFUNCTION(BlueprintCallable)
	const FString &GetId() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StateMachineNode")
	FText GetDescription() const;
	virtual FText GetDescription_Implementation() const;

	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category = "StateMachineNode_Editor")
	FText NodeTitle;

	UPROPERTY(VisibleDefaultsOnly, Category = "StateMachineNode_Editor")
	TSubclassOf<UStateMachineGraph> CompatibleGraphType;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineNode_Editor")
	FLinearColor BackgroundColor;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineNode_Editor")
	FText ContextMenuName;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineNode_Editor")
	ENodeLimit ParentLimitType;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineNode_Editor" ,meta = (ClampMin = "0",EditCondition = "ParentLimitType == ENodeLimit::Limited", EditConditionHides))
	int32 ParentLimit;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineNode_Editor")
	ENodeLimit ChildrenLimitType;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineNode_Editor" ,meta = (ClampMin = "0",EditCondition = "ChildrenLimitType == ENodeLimit::Limited", EditConditionHides))
	int32 ChildrenLimit;
	
#endif

#if WITH_EDITOR
	virtual bool IsNameEditable() const;

	virtual FLinearColor GetBackgroundColor() const;

	virtual FText GetNodeTitle() const;

	virtual void SetNodeTitle(const FText &NewTitle);

	virtual bool CanCreateConnection(UStateMachineNode *Other, FText &ErrorMessage);

	virtual bool CanCreateConnectionTo(UStateMachineNode *Other, int32 NumberOfChildrenNodes, FText &ErrorMessage);
	virtual bool CanCreateConnectionFrom(UStateMachineNode *Other, int32 NumberOfParentNodes, FText &ErrorMessage);
#endif
};
