using System.IO;
using UnrealBuildTool;

public class ParnnyExcel : ModuleRules
{
    public ParnnyExcel(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        string XlntPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "Xlnt/"));
        PublicIncludePaths.Add(ModuleDirectory);
        PublicIncludePaths.Add(XlntPath);
        PublicIncludePaths.Add(Path.Combine(XlntPath, "xlnt"));
        PublicIncludePaths.Add(Path.Combine(XlntPath, "utfcpp"));
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "ParnnyCore", 
            }
        );
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "ContentBrowserFileDataSource",
                "ContentBrowserData",
                "ToolMenus",
                "AssetTools",
                "EditorSubsystem",
                "DeveloperSettings",
                "DirectoryWatcher",
                "MainFrame",
                "PropertyEditor", 
                "EditorStyle",
                "ToolWidgets", 
                "ContentBrowserAssetDataSource",
                "ContentBrowser",
                "DesktopPlatform"
            }
        );
    }
}