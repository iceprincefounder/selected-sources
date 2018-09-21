

#pragma once

#include "ModuleManager.h"
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <string.h>
#include "../../../../MiliUtilities/assetUtilities.h"


class MiliToolVRUtil
{
    public:
        static void VRPython(const TArray<FString>& Args);
		static void MotionBlendPython(const TArray<FString>& Args);
};
