// All rights reserved to Jorge David López Caraballo (https://github.com/Lokillouu)

using UnrealBuildTool;

public class PropertyEditorSetter : ModuleRules
{
    public PropertyEditorSetter(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "DeveloperSettings"
        });

        // Conditionally adds the "StructUtils" module as a private dependency if the engine version is UE4.27+ or UE5.0 – 5.4.
        // This ensures compatibility with versions that include the StructUtils module while avoiding errors in unsupported versions.
        if ((Target.Version.MajorVersion == 4 && Target.Version.MinorVersion >= 27) ||
            (Target.Version.MajorVersion == 5 && Target.Version.MinorVersion >= 0 && Target.Version.MinorVersion < 5))
        { PrivateDependencyModuleNames.Add("StructUtils"); }
    }
}
