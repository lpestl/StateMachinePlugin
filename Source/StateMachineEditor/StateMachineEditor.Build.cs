using UnrealBuildTool;

public class StateMachineEditor : ModuleRules
{
	public StateMachineEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = false;
		ShadowVariableWarningLevel = WarningLevel.Error;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
                // ... add other private include paths required here ...
                "StateMachineEditor/Private",
				"StateMachineEditor/Public",
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
                "Engine",
                "UnrealEd",
				// ... add other public dependencies that you statically link with here ...
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "StateMachineRuntime",
                "AssetTools",
                "Slate",
                "InputCore",
                "SlateCore",
                "GraphEditor",
                "PropertyEditor",
                "EditorStyle",
                "Kismet",
                "KismetWidgets",
                "ApplicationCore",
				"ToolMenus",
				// ... add private dependencies that you statically link with here ...
			}
			);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
            }
			);
	}
}
