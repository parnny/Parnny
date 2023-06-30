// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class ParnnyServerTarget : TargetRules
{
    public ParnnyServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		BuildEnvironment = TargetBuildEnvironment.Shared;
		ExtraModuleNames.Add("Parnny");
		RegisterModulesCreatedByRider();
	}
    
    private void RegisterModulesCreatedByRider()
    {
	    ExtraModuleNames.AddRange(new string[] { "ParnnyNetwork", "ParnnyCore", "ParnnyUI", "ParnnyMixin" });
    }
}
