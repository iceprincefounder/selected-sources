// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#include "ImportAssetsPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "ImportAssetsStyle.h"
#include "ImportAssetsCommands.h"
#include "LevelEditor.h"

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Editor/UnrealEd/Classes/Factories/FbxFactory.h"



static const FName ImportAssetsTabName("ImportAssets");

#define LOCTEXT_NAMESPACE "FImportAssetsModule"

void FImportAssetsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FImportAssetsStyle::Initialize();
	FImportAssetsStyle::ReloadTextures();

	FImportAssetsCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	/*PluginCommands->MapAction(
		FImportAssetsCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FImportAssetsModule::PluginButtonClicked),
		FCanExecuteAction());*/

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");


	IConsoleManager::Get().RegisterConsoleCommand(TEXT("Mili.ImportFBXTexture"),TEXT("import fbx"),
		FConsoleCommandDelegate().CreateStatic(FImportAssetsModule::PluginButtonClicked)
	);


	/*{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FImportAssetsModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FImportAssetsModule::AddToolbarExtension));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}*/
}

void FImportAssetsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FImportAssetsStyle::Shutdown();

	FImportAssetsCommands::Unregister();
}

TCHAR* FImportAssetsModule::StringToTCHAR(std::string Argument)
{
	std::string Argument_string = Argument;
	/** string to char */
	char Argument_char[MAX_PATH];
	int i;
	for (i = 0; i<Argument_string.length(); i++)
		Argument_char[i] = Argument_string[i];
	Argument_char[i] = '\0';
	/** char to TCHAR */
	TCHAR* Argument_TCHAR = new TCHAR[MAX_PATH];
	char Argument_char2[MAX_PATH];
	sprintf_s(Argument_char2, "%s", Argument_char);
	int iLength = MultiByteToWideChar(CP_ACP, 0, Argument_char2, strlen(Argument_char2) + 1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, Argument_char2, strlen(Argument_char2) + 1, Argument_TCHAR, iLength);
	TCHAR* Result = Argument_TCHAR;
	return Result;
}

TSharedRef<SWindow> FImportAssetsModule::ConstructWindowElement()
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("WindowTitle", "Mili Pipeline Tools"))
		.SizingRule(ESizingRule::Autosized)
		.Content()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().VAlign(VAlign_Center)
				[
					SNew(SBorder)
						.BorderBackgroundColor(FLinearColor::Yellow)
							.Padding(FMargin(80.0f, 10.0f))
							[
								SNew(STextBlock).Text(LOCTEXT("Title", "Mili Assets Importer")).ColorAndOpacity(FLinearColor::Yellow)
							]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(SSpacer).Size(FVector2D(200, 20))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().Padding(3).HAlign(HAlign_Left)
				[
					SNew(STextBlock).Text(LOCTEXT("FileName", "Assets File Path:"))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().Padding(3)
				[
					//SNew(SEditableTextBox)
					SAssignNew(SEditableTextBoxPtr1, SEditableTextBox)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(3)
				[
					SNew(SButton).Text(LOCTEXT("My_Button", "Browser")).HAlign(HAlign_Center)
					//.OnClicked(this, &FMiliToolsBarModule::ShowMyDialog)  //This way doesn`t work,try ".OnClicked_Lambda"!!!
						.OnClicked_Lambda([this]()->FReply {
							this->ShowFilePathDialog();
							return FReply::Handled(); })
				]
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().Padding(3).HAlign(HAlign_Left)
				[
					SNew(STextBlock).Text(LOCTEXT("PackagePath", "Package Path:"))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SAssignNew(SEditableTextBoxPtr2, SEditableTextBox)
				.Text(LOCTEXT("PackagePathDefault", "/Game/Maya"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(SSpacer).Size(FVector2D(200, 10))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(SUniformGridPanel).SlotPadding(3)
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton).HAlign(HAlign_Center)
						.Text(LOCTEXT("FbxOptionWindow_ImportFBX", "Import FBX"))
							.OnClicked_Lambda([this]()->FReply {
								this->OnImportFBX();
								return FReply::Handled(); })
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton).HAlign(HAlign_Center)
					.Text(LOCTEXT("FbxOptionWindow_ImportTexture", "Import Texture"))
						.OnClicked_Lambda([this]()->FReply {
							this->OnImportTexture();
							return FReply::Handled(); })
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(SSpacer).Size(FVector2D(200, 5))
			]
		];
	return Window;
}

TArray<FString> FImportAssetsModule::GetAllFileFromDir(FString DestinationPath)
{
	//FString to string
	std::string FileDir_string = TCHAR_TO_UTF8(*DestinationPath);
	//string to char
	char FileDir_char[MAX_PATH];
	int i;
	for (i = 0; i<FileDir_string.length(); i++)
		FileDir_char[i] = FileDir_string[i];
	FileDir_char[i] = '\0';

	TArray<FString> AllFiles;
	WIN32_FIND_DATA FindFileData;
	HANDLE hListFile;
	/** char to TCHAR */
	char CharFilePath[MAX_PATH];
	TCHAR TCharFilePath[MAX_PATH];
	sprintf_s(CharFilePath, "%s\\*", FileDir_char);
	int iLength = MultiByteToWideChar(CP_ACP, 0, CharFilePath, strlen(CharFilePath) + 1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, CharFilePath, strlen(CharFilePath) + 1, TCharFilePath, iLength);

	if ((hListFile = FindFirstFile(TCharFilePath, &FindFileData)) == INVALID_HANDLE_VALUE)
	{
		std::cout << "No File!" << std::endl;
	}
	else
	{
		while (FindNextFile(hListFile, &FindFileData))
		{
			if (lstrcmp(FindFileData.cFileName, TEXT(".")) == 0 || lstrcmp(FindFileData.cFileName, TEXT("..")) == 0)
			{
				continue;
			}
			else
			{
				//WCHAR to char*
				WCHAR *FileName_WCHAR = FindFileData.cFileName;
				char* FileName_char = new char[MAX_PATH];
				int nLen = WideCharToMultiByte(CP_ACP, 0, FileName_WCHAR, -1, NULL, 0, NULL, NULL);
				WideCharToMultiByte(CP_ACP, 0, FileName_WCHAR, -1, FileName_char, nLen, NULL, NULL);
				//char* to string
				std::string FileName(FileName_char);
				int i = FileName.find(".", 0);
				if (i >= 0)
				{
					TCHAR* FileName_TCHAR = this->StringToTCHAR(FileName);
					TCHAR FileName_final[MAX_PATH];
					_tcscpy(FileName_final, FileName_TCHAR);
					TCHAR* FileDir_TCHAR = this->StringToTCHAR(FileDir_string);
					TCHAR FileDir_final[MAX_PATH];
					_tcscpy(FileDir_final, FileDir_TCHAR);
					TCHAR* FilePath_TCHAR;
					TCHAR FilePath_final[MAX_PATH];
					FilePath_TCHAR = _tcscat(FileDir_final, TEXT("/"));
					FilePath_TCHAR = _tcscat(FilePath_TCHAR, FileName_final);
					_tcscpy(FilePath_final, FilePath_TCHAR);
					//strcpy();
					//UE_LOG(LogTemp, Warning, FilePath_final);
					AllFiles.Add(FilePath_final);
				}

			}
		}
	}
	FindClose(hListFile);

	return AllFiles;
}

void FImportAssetsModule::OnImportFBX()
{
	
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString FileDir_FString = SEditableTextBoxPtr1->GetText().ToString();
	TArray<FString> AllFilePathNames = this->GetAllFileFromDir(FileDir_FString);// All files from dirs.
	TArray<FString> DestinationFilePathNames;//The files which can be imported.
	TArray<FString> AllFilePathNames_TEST;
	AllFilePathNames_TEST.Add(TEXT("D:/s_cube1.fbx"));
	AllFilePathNames_TEST.Add(TEXT("D:/image.jpg"));
	FString PackagePath = SEditableTextBoxPtr2->GetText().ToString();
	UFbxFactory* FBXFactory = ConstructObject<UFbxFactory>(UFbxFactory::StaticClass());
	//TextureFactory->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	FBXFactory -> EnableShowOption();
	/** Find the file which can be imported!*/
	for (auto Element_FString : AllFilePathNames)
	{
		std::string Element_string = TCHAR_TO_UTF8(*Element_FString);
		TCHAR* Element_TCHAR = this->StringToTCHAR(Element_string);
		TCHAR Element[MAX_PATH];
		_tcscpy(Element, Element_TCHAR);
		bool flag = FBXFactory->FactoryCanImport(Element);
		if (flag)
		{
			DestinationFilePathNames.Add(Element);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("...\nCan`t Import: %s "),Element);
		}
	}
	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(DestinationFilePathNames, PackagePath, FBXFactory);
	
}

void FImportAssetsModule::OnImportTexture()
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString FileDir_FString = SEditableTextBoxPtr1->GetText().ToString();
	TArray<FString> AllFilePathNames = this->GetAllFileFromDir(FileDir_FString);// All files from dirs.
	TArray<FString> DestinationFilePathNames;//The file list which can be imported.
	TArray<FString> AllFilePathNames_TEST;
	AllFilePathNames_TEST.Add(TEXT("D:/maya/image.jpg"));
	AllFilePathNames_TEST.Add(TEXT("D:/maya/jugg.jpg"));
	FString PackagePath = SEditableTextBoxPtr2->GetText().ToString();
	UTextureFactory * TextureFactory = ConstructObject<UTextureFactory>(UTextureFactory::StaticClass());
	TextureFactory->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	for (auto Element_FString: AllFilePathNames)
	{
		std::string Element_string = TCHAR_TO_UTF8(*Element_FString);
		TCHAR* Element_TCHAR = this->StringToTCHAR(Element_string);
		TCHAR Element[MAX_PATH];
		_tcscpy(Element, Element_TCHAR);
		int i = Element_string.find(".", 0);
		std::string Format_string = Element_string.substr(i);
		TCHAR* Format_TCHAR = this->StringToTCHAR(Format_string);
		TCHAR Format[MAX_PATH];
		_tcscpy(Format, Format_TCHAR);
		if (FString(Format) == TEXT(".jpg") || FString(Format) == TEXT(".tga"))
		{
			DestinationFilePathNames.Add(Element);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("...\nCan`t Import: %s "), Element);
		}
		//UE_LOG(LogTemp, Warning, Format);
	}
	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(DestinationFilePathNames, PackagePath, TextureFactory);

	//FText DialogText = FText::FromString(TEXT("Textures import completed!"));
	//FMessageDialog::Open(EAppMsgType::Ok, DialogText);

}

TArray<FString> FImportAssetsModule::FileDialog()
{
	FString DialogTitle = FString(TEXT("File Dialog"));
	FString DefaultPath = FString(TEXT("D:/"));
	FString DefaultFile = FString(TEXT(""));
	FString FileTypes = FString(TEXT("All Files"));
	uint32 Flags = EFileDialogFlags::Multiple;
	TArray< FString > OutFilenames;
	int32 outFilterIndex;

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = NULL;

		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		/** Test File Dialog*/
		const bool bFileSelected = DesktopPlatform->OpenFileDialog(
			ParentWindowWindowHandle,
			DialogTitle,
			DefaultPath,
			DefaultFile,
			FileTypes,
			Flags,
			OutFilenames,
			outFilterIndex
			);
	}
	return OutFilenames;

}

void FImportAssetsModule::ShowFilePathDialog()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = NULL;

		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		FString FolderName;
		FString LastBrowsePath = FString("D:/");
		const FString Title = LOCTEXT("NewFilePath", "请选择Maya资产FBX所在的文件夹").ToString();
		const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
			ParentWindowWindowHandle,
			Title,
			LastBrowsePath,
			FolderName
			);

		if (bFolderSelected)
		{
			LastBrowsePath = FolderName;
			SEditableTextBoxPtr1->SetText(FText::FromString(LastBrowsePath));
		}
	}


}

void FImportAssetsModule::PluginButtonClicked()
{
	// show the windows
	TSharedPtr<SWindow> ParentWindow;
	// Check if the main frame is loaded.  When using the old main frame it may not be.
	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		ParentWindow = MainFrame.GetParentWindow();
	}
	TSharedRef<SWindow> NewWindow = FImportAssetsModule().ConstructWindowElement();
	//FSlateApplication::Get().AddModalWindow(NewWindow, ParentWindow, false);	Lock the Window!
	FSlateApplication::Get().AddWindowAsNativeChild(NewWindow, ParentWindow.ToSharedRef(), true);
}

void FImportAssetsModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FImportAssetsCommands::Get().PluginAction);
}

void FImportAssetsModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FImportAssetsCommands::Get().PluginAction);
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FImportAssetsModule, ImportAssets)