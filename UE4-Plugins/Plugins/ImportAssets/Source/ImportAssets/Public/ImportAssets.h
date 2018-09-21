// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <string.h>

class FToolBarBuilder;
class FMenuBuilder;

class FImportAssetsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//TSharedRef<SWindow> NewWindow; Can`t define a vriable???
	/** This function will be bound to Command. */
	static void PluginButtonClicked();
	void ShowFilePathDialog();
	void OnImportFBX();
	void OnImportTexture();
	TArray<FString> GetAllFileFromDir(FString DestinationPath);
	TCHAR* StringToTCHAR(std::string Argument);
	TArray<FString> FileDialog();
private:
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedRef<SWindow>              ConstructWindowElement();
	TSharedPtr<SEditableTextBox>	 SEditableTextBoxPtr1;
	TSharedPtr<SEditableTextBox>	 SEditableTextBoxPtr2;
};