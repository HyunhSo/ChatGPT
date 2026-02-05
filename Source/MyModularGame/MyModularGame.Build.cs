using UnrealBuildTool;

public class MyModularGame : ModuleRules
{
    public MyModularGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "ModularGameplay",
            "GameFeatures",
            "GameplayTags",
            "GameplayAbilities"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "ModularGameplayActors"
        });
    }
}
