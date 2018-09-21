// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "ImportAssetsStyle.h"

class FImportAssetsCommands : public TCommands<FImportAssetsCommands>
{
public:

	FImportAssetsCommands()
		: TCommands<FImportAssetsCommands>(TEXT("ImportAssets"), NSLOCTEXT("Contexts", "ImportAssets", "ImportAssets Plugin"), NAME_None, FImportAssetsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
