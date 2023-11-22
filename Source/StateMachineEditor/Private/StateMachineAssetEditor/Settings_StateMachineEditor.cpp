#include "StateMachineAssetEditor/Settings_StateMachineEditor.h"

UStateMachineEditorSettings::UStateMachineEditorSettings()
{
	AutoLayoutStrategy = EAutoLayoutStrategy::Tree;

	bFirstPassOnly = false;

	bRandomInit = false;

	OptimalDistance = 100.f;

	MaxIteration = 50;

	InitTemperature = 10.f;

	CoolDownRate = 10.f;
}

UStateMachineEditorSettings::~UStateMachineEditorSettings()
{

}

