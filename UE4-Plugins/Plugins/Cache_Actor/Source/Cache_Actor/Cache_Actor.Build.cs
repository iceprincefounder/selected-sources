// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class Cache_Actor : ModuleRules
	{
        public Cache_Actor(TargetInfo Target)
		{
			PrivateIncludePaths.Add("Cache_Actor/Private");

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "RenderCore",
                    "ShaderCore",
                    "RHI"
				}
				);
		}

	}
}
