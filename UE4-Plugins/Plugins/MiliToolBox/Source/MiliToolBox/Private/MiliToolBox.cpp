
#include "MiliToolBoxPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "SDockTab.h"

#include "MiliToolBoxStyle.h"
#include "MiliToolBoxCommands.h"
#include "LevelEditor.h"

#include "Editor/ContentBrowser/Public/ContentBrowserModule.h"

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Editor/UnrealEd/Classes/Factories/FbxFactory.h"

#include "MiliToolVRUtil.h"
#include "../../../../MiliUtilities/assetUtilities.h"


#pragma region toolBox_setup
static const FName MiliToolBoxTabName("MiliToolBox");

#define LOCTEXT_NAMESPACE "FMiliToolBoxModule"

void MiliToolbar();

void FMiliToolBoxModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FMiliToolBoxStyle::Initialize();
	FMiliToolBoxStyle::ReloadTextures();

	FMiliToolBoxCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FMiliToolBoxCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FMiliToolBoxModule::PluginButtonClicked),
		FCanExecuteAction());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FMiliToolBoxModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FMiliToolBoxModule::AddToolbarExtension));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	//挂入获取内容浏览器的选资产事件
	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//ContentBrowserModule.GetOnAssetSelectionChanged().AddRaw(this,&FMiliToolBoxModule::OnContentBrowserAssetSelectionChanged);
	AssetUtilities tmpAssetUtil;
	ContentBrowserModule.GetOnAssetSelectionChanged().AddRaw(&tmpAssetUtil,&AssetUtilities::OnContentBrowserAssetSelectionChanged);

	util::registerUtilConsolCmd();
	
	IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.UploadShading"), tchar("upload shading assets to the server"),
		FConsoleCommandDelegate().CreateStatic(FMiliToolBoxModule::OnUploadShading)
	);

	/*IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.ProjectConfig"),tchar("config project"),
		FConsoleCommandDelegate().CreateStatic(util::ProjectConfigEditor)
	);*/

	IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.Toolbar"),tchar("mili pipeline toolbar"),
		FConsoleCommandDelegate().CreateStatic(MiliToolbar)
	);

	IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.MiliVR.PyNuke"),tchar("combine 12 images to VR using nuke"), 
		FConsoleCommandWithArgsDelegate().CreateStatic(MiliToolVRUtil::VRPython)
	);
	/*IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.MiliVR.PyNukeMotionBlend"),tchar("combine multiple subframes to frame"),
		FConsoleCommandWithArgsDelegate().CreateStatic(MiliToolVRUtil::MotionBlendPython)
	);*/
	IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.MiliHelp"),tchar("combine 12 images to VR using nuke"),
		FConsoleCommandWithArgsDelegate().CreateStatic(MiliHelp)
	);

	/*IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.TeamConfig"),tchar("mili team config"),
		FConsoleCommandDelegate().CreateStatic(util::TeamConfigEditor)
	);*/

	/*IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.PIT"),tchar("mili team config"),
		FConsoleCommandDelegate().CreateStatic(util::TeamConfigEditor)
	);*/


	
	
}

void FMiliToolBoxModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FMiliToolBoxStyle::Shutdown();

	FMiliToolBoxCommands::Unregister();
}

TCHAR* FMiliToolBoxModule::StringToTCHAR(std::string Argument)
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

TSharedRef<SWindow> FMiliToolBoxModule::ConstructWindowElement()
{
	
	auto ButtonGrid = SNew(SVerticalBox);
	//TSharedRef<SVerticalBox> ButtonGrid = SNew(SVerticalBox);

	TSharedRef<SWindow> Window2 = SNew(SWindow)
		.Title(LOCTEXT("WindowTitle","Mili Pipeline"))
		.SizingRule(ESizingRule::UserSized)
		.Content()[ButtonGrid];

	Window2.Get().Resize(FVector2D(400,300));

	string ueMenuPath = util::GetInstallData("Pipeline_Path")+"/UnrealEngine/ueMenu.json";
	hgLog("\n-------------------------------------\nMenuJsonPath:  %s",tchar(ueMenuPath));
	string jsonString = util::readTxt(ueMenuPath.c_str());
	//string jsonString = util::readTxt("d:/ueMenu.json");
	//hgLog("---- %i \n%s",jsonString.size(), tchar(jsonString));
	Document jsonDoc;
	jsonDoc.Parse(jsonString.c_str());
	
	//hgLog("%s", tchar(jsonDoc[0]["TagName"].GetString()));
	for(auto iTag = jsonDoc.Begin(); iTag != jsonDoc.End(); ++iTag){
		//hgLog("%s",tchar((*iTag)["TagName"].GetString()));

		ButtonGrid.Get().AddSlot().VAlign(VAlign_Bottom).MaxHeight(30)[
			SNew(STextBlock).Text(FText::FromString(tchar((*iTag)["TagName"].GetString()))).MinDesiredWidth(400)
				.ColorAndOpacity(FLinearColor(0.6,0.6,0.2))
		];

		auto tmpHBox = SNew(SHorizontalBox);
		//auto tmpHBox = SNew(SUniformGridPanel);
		//auto tmpHBox = SNew(SWrapBox);
		ButtonGrid.Get().AddSlot()[tmpHBox];

		for(auto iBtn = (*iTag)["Content"].Begin(); iBtn != (*iTag)["Content"].End(); ++iBtn) {
			hgLog("  %s",tchar((*iBtn)["Label"].GetString()), tchar((*iBtn)["Description"].GetString()), tchar((*iBtn)["Cmd"].GetString()));

			const char* btnCmd = (*iBtn)["Cmd"].GetString();
			string btnCmdStr = string(btnCmd);

			auto fun = [](float a,float b) {
				return (std::abs(a) < std::abs(b));
			};

			tmpHBox.Get().AddSlot()[
				SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
					.ButtonColorAndOpacity(FLinearColor(0.1,0.1,0.1,0.6))
					.ForegroundColor(FLinearColor(1.0,1.0,1.0))
					.Text(FText::FromString(tchar((*iBtn)["Label"].GetString())))
					.OnClicked_Lambda([this,btnCmdStr]()->FReply {
						this->OnToolBoxButtonClicked(btnCmdStr.c_str()) ;
						return FReply::Handled(); })
					.TextFlowDirection(ETextFlowDirection::RightToLeft)
			];

		}
	}
	


	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("WindowTitle", "Mili Pipeline"))
		.SizingRule(ESizingRule::Autosized)
		.Content()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().VAlign(VAlign_Center)
				[
					SNew(SBorder).BorderBackgroundColor(FLinearColor::Yellow).Padding(FMargin(80.0f, 10.0f))
					[
						SNew(STextBlock).Text(LOCTEXT("Title", "Tool Box")).ColorAndOpacity(FLinearColor::Yellow)
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(SSpacer).Size(FVector2D(200, 10))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(STextBlock).Text(LOCTEXT("FileName", "Assets File Path:"))
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
							.Text(LOCTEXT("Mili_UploadShading", "Upload Shading"))
								.OnClicked_Lambda([this]()->FReply {
									this->OnToolBoxButtonClicked("Mili.UploadShading");
									return FReply::Handled(); })
					]
					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton).HAlign(HAlign_Center)
						.Text(LOCTEXT("Mili_UploadShot", "Upload Shot"))
							.OnClicked_Lambda([this]()->FReply {
							this->OnToolBoxButtonClicked("???");
							return FReply::Handled(); })
					]
					+ SUniformGridPanel::Slot(2,0)
						[
							SNew(SButton).HAlign(HAlign_Center)
							.Text(LOCTEXT("Mili_Help","Help"))
						.OnClicked_Lambda([this]()->FReply {
						this->OnToolBoxButtonClicked("---Help Me !");
						return FReply::Handled(); })
						]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5).VAlign(VAlign_Bottom)
			[
				SNew(SSpacer).Size(FVector2D(200, 5))
			]
		];
	return Window2;
}

TArray<FString> FMiliToolBoxModule::GetAllFileFromDir(FString DestinationPath)
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

void FMiliToolBoxModule::OnToolBoxButtonClicked(const char* command)
{
	hgLog("--- %s", tchar(command));
	GEngine->Exec(nullptr, tchar(command));
}

TArray<FString> FMiliToolBoxModule::FileDialog()
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

void FMiliToolBoxModule::ShowFilePathDialog()
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

void FMiliToolBoxModule::PluginButtonClicked()
{
	// show the windows
	TSharedPtr<SWindow> ParentWindow;
	// Check if the main frame is loaded.  When using the old main frame it may not be.
	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		ParentWindow = MainFrame.GetParentWindow();
	}
	TSharedRef<SWindow> NewWindow = this->ConstructWindowElement();
	//FSlateApplication::Get().AddModalWindow(NewWindow, ParentWindow, false);	Lock the Window!
	FSlateApplication::Get().AddWindowAsNativeChild(NewWindow, ParentWindow.ToSharedRef(), true);
}

void FMiliToolBoxModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FMiliToolBoxCommands::Get().PluginAction);
}

void FMiliToolBoxModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FMiliToolBoxCommands::Get().PluginAction);
}

#pragma endregion

void FMiliToolBoxModule::OnUploadShading()
{
	AssetUtilities tmpAssetUtil;
	TArray<FAssetData> DependentAssets;
	tmpAssetUtil.GatherAssetsInfo(TEXT("StaticMesh"),DependentAssets);
	string pyCmd = "import json\n";
	if(DependentAssets.Num()>0)
	{
		pyCmd += "assetDict={ \n";
		pyCmd +="  'ToUploadAssets':[ ";
		for(auto& iAsset : DependentAssets)
		{
			pyCmd +="\n    { 'filePath': '"+toString(FPaths::GameContentDir()) +toString( iAsset.PackageName).substr(6)+".uasset', 'assetType':'"+toString(iAsset.GetClass()->GetName())+"' },";
		}
		pyCmd += "\n  ] \n}  \n\nprint json.dumps(assetDict, indent=2)  ";

		pyCmd += "\nimport UEPyMili.Shading.UploadShading as UploadShading; reload(UploadShading)";
		pyCmd += "\nUploadShading.UploadShading('"+util::GetActiveProject()+"').ShowUI('"+toString(DependentAssets[0].AssetName.ToString())+"','default', uploadDict=assetDict)";

		pyCmd +="\nimport time; time.sleep(1)";
		hgLog("python finished: %i",util::Python(pyCmd)? 1:0);
	}

	/*for(auto& iAsset : DependentAssets)
	{
	FString iPath = iAsset.PackageName.ToString();
	hgLog("filtered: RD:%i   TD:%i  %s",iPath.StartsWith(FString("/Game/_RD_Content"))? 1:0,iPath.StartsWith(FString("/Game/_td_Content"))? 1:0,  *iPath);
	}*/

	util::OVar("ooo","qqqqq",true);
	util::OVar("oo3",33,true);
	util::GetActiveProject();
	util::ProjectConfig("pyGates_default", "asd");
}

void FMiliToolBoxModule::OnUploadShot()
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
	for(auto Element_FString: AllFilePathNames)
	{
		std::string Element_string = TCHAR_TO_UTF8(*Element_FString);
		TCHAR* Element_TCHAR = this->StringToTCHAR(Element_string);
		TCHAR Element[MAX_PATH];
		_tcscpy(Element,Element_TCHAR);
		int i = Element_string.find(".",0);
		std::string Format_string = Element_string.substr(i);
		TCHAR* Format_TCHAR = this->StringToTCHAR(Format_string);
		TCHAR Format[MAX_PATH];
		_tcscpy(Format,Format_TCHAR);
		if(FString(Format) == TEXT(".jpg") || FString(Format) == TEXT(".tga"))
		{
			DestinationFilePathNames.Add(Element);
		} else
		{
			UE_LOG(LogTemp,Warning,TEXT("...\nCan`t Import: %s "),Element);
		}
		//UE_LOG(LogTemp, Warning, Format);
	}
	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(DestinationFilePathNames,PackagePath,TextureFactory);

	//FText DialogText = FText::FromString(TEXT("Textures import completed!"));
	//FMessageDialog::Open(EAppMsgType::Ok, DialogText);

}

void FMiliToolBoxModule::MiliHelp(const TArray<FString>& Args)
{
	if(Args.Num()>0)
	{
		util::Python("__import__('webbrowser').open('http://wiki.milipictures.com:8020/Tool_Reference:_Unreal:_"+toString(Args[0])+"')", 0 );
	}
}

//TSharedRef<SDockTab> MiliToolbar()
void MiliToolbar()
{
	hgLog("Mili Toolbar Called!");
	auto tmp = FMiliToolBoxModule();
	tmp.PluginButtonClicked();
		/*.ShouldAutosize(true)
	.Icon(FEditorStyle::GetBrush("ToolBar.Icon"));
	[
		SNew(SHorizontalBox)
		.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("LevelEditorToolbar")))
		+SHorizontalBox::Slot()
		.FillWidth(1)
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Left)
		//[
			//FLevelEditorToolBar::MakeLevelEditorToolBar(LevelEditorCommands.ToSharedRef(),SharedThis(this))
		//]
	];*/
}




#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMiliToolBoxModule, MiliToolBox)