// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "DownloadAssetsStyle.h"

class FDownloadAssetsCommands : public TCommands<FDownloadAssetsCommands>
{
public:

	FDownloadAssetsCommands()
		: TCommands<FDownloadAssetsCommands>(TEXT("DownloadAssets"), NSLOCTEXT("Contexts", "DownloadAssets", "DownloadAssets Plugin"), NAME_None, FDownloadAssetsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
