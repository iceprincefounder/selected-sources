// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "MiliToolBoxPrivatePCH.h"
#include "MiliToolBoxCommands.h"

#define LOCTEXT_NAMESPACE "FMiliToolBoxModule"

void FMiliToolBoxCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "MiliToolBox", "Execute MiliToolBox action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
