// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "ImportAssetsPrivatePCH.h"
#include "ImportAssetsCommands.h"

#define LOCTEXT_NAMESPACE "FImportAssetsModule"

void FImportAssetsCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "ImportAssets", "Execute ImportAssets action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
