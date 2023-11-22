#include "StateMachineFactory.h"
#include "GraphData/StateMachineGraph.h"

#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Modules/ModuleManager.h"
#include"Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "StateMachineFactory"

class FAssetClassParentFilter : public IClassViewerFilter
{
public:
	FAssetClassParentFilter()
		: DisallowedClassFlags(CLASS_None), bDisallowBlueprintBase(false)
	{}

	/** All children of these classes will be included unless filtered out by another setting. */
	TSet< const UClass *> AllowedChildrenOfClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags;

	/** Disallow blueprint base classes. */
	bool bDisallowBlueprintBase;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions &InInitOptions, const UClass *InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		bool bAllowed = !InClass->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;

		if (bAllowed && bDisallowBlueprintBase)
		{
			if (FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass))
			{
				return false;
			}
		}

		return bAllowed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions &InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		if (bDisallowBlueprintBase)
		{
			return false;
		}

		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};


UStateMachineFactory::UStateMachineFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UStateMachineGraph::StaticClass();
}

UStateMachineFactory::~UStateMachineFactory()
{

}

bool UStateMachineFactory::ConfigureProperties()
{
	// nullptr the StateMachineClass so we can check for selection
	StateMachineClass = nullptr;

	// Load the classviewer module to display a class picker
	FClassViewerModule &ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;

#if ENGINE_MAJOR_VERSION < 5
	TSharedPtr<FAssetClassParentFilter> Filter = MakeShareable(new FAssetClassParentFilter);
	Options.ClassFilter = Filter;
#else // #if ENGINE_MAJOR_VERSION < 5
	TSharedRef<FAssetClassParentFilter> Filter = MakeShareable(new FAssetClassParentFilter);
	Options.ClassFilters.Add(Filter);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5

	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown;
	Filter->AllowedChildrenOfClasses.Add(UStateMachineGraph::StaticClass());

	const FText TitleText = LOCTEXT("CreateStateMachineAssetOptions", "Pick State Machine Class");
	UClass *ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UStateMachineGraph::StaticClass());

	if ( bPressedOk )
	{
		StateMachineClass = ChosenClass;
	}

	return bPressedOk;
}

UObject *UStateMachineFactory::FactoryCreateNew(UClass *Class, UObject *InParent, FName Name, EObjectFlags Flags, UObject *Context, FFeedbackContext *Warn)
{
	if (StateMachineClass != nullptr)
	{
		return NewObject<UStateMachineGraph>(InParent, StateMachineClass, Name, Flags | RF_Transactional);
	}
	else
	{
		check(Class->IsChildOf(UStateMachineGraph::StaticClass()));
		return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
	}

}

#undef LOCTEXT_NAMESPACE
