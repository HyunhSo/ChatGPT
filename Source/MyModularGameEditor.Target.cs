using UnrealBuildTool;
using System.Collections.Generic;

public class MyModularGameEditorTarget : TargetRules
{
    public MyModularGameEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        ExtraModuleNames.Add("MyModularGame");
    }
}
