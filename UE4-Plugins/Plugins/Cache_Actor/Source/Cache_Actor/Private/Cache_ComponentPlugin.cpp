// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Cache_ComponentPluginPrivatePCH.h"




class FCache_ComponentPlugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FCache_ComponentPlugin, Cache_Component )



void FCache_ComponentPlugin::StartupModule()
{
	
}


void FCache_ComponentPlugin::ShutdownModule()
{
	
}



