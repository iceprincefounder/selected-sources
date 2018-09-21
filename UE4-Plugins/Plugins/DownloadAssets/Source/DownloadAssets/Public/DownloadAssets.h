// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

class FToolBarBuilder;
class FMenuBuilder;

class FDownloadAssetsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
public:
	/** Add */
	void ElementButtonClicked(const FString InName, const FString InType, const FString InPath);
	static void OnImportTexture(const FString InFullPath, const FString InDestinationPath);
	static void OnImportFBX(const FString InFullPath, const FString InPackagePath);
	static void OnCreateMaterialInstances(const FString InFullPath, const FString InPackagePath, const FString InTexturePath);
	void ButtonClickedTest();

private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

private:
	TSharedPtr<class FUICommandList> PluginCommands;

private:
	/** Add */
	TSharedRef<SWindow> ConstructWindowElement();
	TSharedPtr<SEditableTextBox> SEditableTextBoxPtr1;
	TSharedPtr<SCheckBox> SCheckBoxPtr1;
	TSharedRef<SScrollBox> TheScrollBox(FString InType, FString InPath);
};