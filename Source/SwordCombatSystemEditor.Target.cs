// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

using UnrealBuildTool;
using System.Collections.Generic;

public class SwordCombatSystemEditorTarget : TargetRules
{
	public SwordCombatSystemEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		bLegacyParentIncludePaths = false;
		CppStandard = CppStandardVersion.Default;
		WindowsPlatform.bStrictConformanceMode = true;
		bValidateFormatStrings = true;

		ExtraModuleNames.AddRange( new string[] { "SwordCombatSystem" } );
	}
}
