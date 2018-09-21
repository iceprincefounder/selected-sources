// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <string.h>
#include "Runtime/AssetRegistry/Public/AssetData.h"

#define tchar(str) util().toTChar(str)
#define f2tchar(fstr) tchar(TCHAR_TO_UTF8(*(fstr)))
#define hgLog(Format, ...)\
{\
	UE_LOG(LogTemp,Warning,TEXT(##Format), ##__VA_ARGS__); \
}

class FToolBarBuilder;
class FMenuBuilder;

class FMiliToolBoxModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//TSharedRef<SWindow> NewWindow; Can`t define a vriable???
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	void ShowFilePathDialog();
	static void MiliHelp(const TArray<FString>& Args);
	static void OnToolBoxButtonClicked(const char* command="Hello world!");
	static void OnUploadShading();
	void OnUploadShot();
	void OnHelp();

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