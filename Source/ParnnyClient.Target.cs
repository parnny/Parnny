// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ParnnyClientTarget : TargetRules
{
    public ParnnyClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		BuildEnvironment = TargetBuildEnvironment.Shared;
		ExtraModuleNames.Add("UE4Game");
		RegisterModulesCreatedByRider();
	}

    private void RegisterModulesCreatedByRider()
    {
	    ExtraModuleNames.AddRange(new string[] { "ParnnyNetwork", "ParnnyCore" });
    }
}
