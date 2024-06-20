// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

using UnrealBuildTool;
using System.Collections.Generic;

public class SwordCombatSystemTarget : TargetRules
{
	public SwordCombatSystemTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		bOverrideBuildEnvironment = true;
        ExtraModuleNames.AddRange( new string[] { "SwordCombatSystem" } );
	}
}
