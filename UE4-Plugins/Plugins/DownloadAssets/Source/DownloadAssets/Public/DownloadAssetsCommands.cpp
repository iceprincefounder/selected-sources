// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "DownloadAssetsPrivatePCH.h"
#include "DownloadAssetsCommands.h"

#define LOCTEXT_NAMESPACE "FDownloadAssetsModule"

void FDownloadAssetsCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "DownloadAssets", "Execute DownloadAssets action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
