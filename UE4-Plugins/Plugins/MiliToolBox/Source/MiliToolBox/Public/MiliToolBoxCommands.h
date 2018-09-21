// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "MiliToolBoxStyle.h"

class FMiliToolBoxCommands : public TCommands<FMiliToolBoxCommands>
{
public:

	FMiliToolBoxCommands()
		: TCommands<FMiliToolBoxCommands>(TEXT("MiliToolBox"), NSLOCTEXT("Contexts", "MiliToolBox", "MiliToolBox Plugin"), NAME_None, FMiliToolBoxStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
