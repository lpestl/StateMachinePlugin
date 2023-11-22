#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "GraphData/StateMachineGraph.h"
#include "StateMachineFactory.generated.h"

UCLASS()
class STATEMACHINEEDITOR_API UStateMachineFactory : public UFactory
{
	GENERATED_BODY()

public:
	UStateMachineFactory();
	virtual ~UStateMachineFactory();

	UPROPERTY(EditAnywhere, Category=DataAsset)
	TSubclassOf<UStateMachineGraph> StateMachineClass;

	virtual bool ConfigureProperties() override;
	virtual UObject *FactoryCreateNew(UClass *Class, UObject *InParent, FName Name, EObjectFlags Flags, UObject *Context, FFeedbackContext *Warn) override;
};
