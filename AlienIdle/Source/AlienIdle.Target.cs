using UnrealBuildTool;

public class AlienIdleTarget : TargetRules
{
	public AlienIdleTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("AlienIdle");
	}
}
