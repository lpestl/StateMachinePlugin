#pragma once

#include "CoreMinimal.h"
#include "StateMachineNode.h"
#include "StateMachineEdge.h"
#include "GameplayTagContainer.h"
#include "StateMachineGraph.generated.h"

class UEdGraph;

UCLASS(Abstract, Blueprintable, HideCategories=(UActorComponent))
class STATEMACHINERUNTIME_API UStateMachineGraph : public UActorComponent
{
	GENERATED_BODY()

public:
	UStateMachineGraph();
	virtual ~UStateMachineGraph();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "StateMachineGraph")
	FString Name;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineGraph")
	TSubclassOf<UStateMachineNode> NodeType;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineGraph")
	TSubclassOf<UStateMachineEdge> EdgeType;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "StateMachineGraph")
	FGameplayTagContainer GraphTags;

	UPROPERTY(BlueprintReadOnly, Category = "StateMachineGraph")
	TArray<UStateMachineNode*> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "StateMachineGraph")
	TArray<UStateMachineNode*> AllNodes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StateMachineGraph")
	bool bEdgeEnabled;

	UFUNCTION(BlueprintCallable, Category = "StateMachineGraph")
	void Print(bool ToConsole = true, bool ToScreen = true);

	UFUNCTION(BlueprintCallable, Category = "StateMachineGraph")
	int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "StateMachineGraph")
	void GetNodesByLevel(int Level, TArray<UStateMachineNode*> &Nodes);

	void ClearGraph();

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UEdGraph *EdGraph;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineGraph_Editor")
	bool bCanRenameNode;

	UPROPERTY(EditDefaultsOnly, Category = "StateMachineGraph_Editor")
	bool bCanBeCyclical;
#endif
};
