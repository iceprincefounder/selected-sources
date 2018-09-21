// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "DownloadAssetsPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "DownloadAssetsStyle.h"
#include "DownloadAssetsCommands.h"

#include "LevelEditor.h"

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Editor/UnrealEd/Classes/Factories/FbxFactory.h"


#include "../../../../MiliUtilities/TheKaiLibrary.h"
#include "../../../../MiliUtilities/_ThirdParty/rapidjson/document.h"

static const FName DownloadAssetsTabName("DownloadAssets");

#define LOCTEXT_NAMESPACE "FDownloadAssetsModule"

void FDownloadAssetsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FDownloadAssetsStyle::Initialize();
	FDownloadAssetsStyle::ReloadTextures();

	FDownloadAssetsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FDownloadAssetsCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FDownloadAssetsModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FDownloadAssetsModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FDownloadAssetsModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FDownloadAssetsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FDownloadAssetsStyle::Shutdown();

	FDownloadAssetsCommands::Unregister();
}

void FDownloadAssetsModule::PluginButtonClicked()
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

/** Add */
TSharedRef<SWindow> FDownloadAssetsModule::ConstructWindowElement()
{
	float PaddingAmount = 0;
	//TSharedRef<SVerticalBox> VerticalBox = this->TheVerticalBox();

	//ScrollBox_Characters = this->TheScrollBox(FString("Test"), FString("D:/"));
	TSharedRef<SScrollBox> ScrollBox_Characters = this->TheScrollBox(FString("Characters"), FString("//hnas01/data/Projects/SQ/Models/"));
	TSharedRef<SScrollBox> ScrollBox_Props = this->TheScrollBox(FString("Props"), FString("//hnas01/data/Projects/SQ/Models/"));
	TSharedRef<SScrollBox> ScrollBox_Scenes = this->TheScrollBox(FString("Scene"), FString("//hnas01/data/Projects/SQ/Models/"));

	TSharedRef<SVerticalBox> SVerticalBox_Characters = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> SVerticalBox_Props = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> SVerticalBox_Scenes = SNew(SVerticalBox);
	SVerticalBox_Characters.Get().AddSlot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().VAlign(VAlign_Center).Padding(PaddingAmount).AutoHeight()
		[
			SNew(SBox).WidthOverride(200).HeightOverride(25)
			[
				SNew(SBorder).VAlign(VAlign_Center).HAlign(HAlign_Center)
				[
					SNew(STextBlock).Text(LOCTEXT("CharactersTag", "Characters")).ColorAndOpacity(FLinearColor::Yellow)
				]
			]
		]

		+ SVerticalBox::Slot().VAlign(VAlign_Center).Padding(PaddingAmount).AutoHeight()
		[
			SNew(SBox).WidthOverride(200).HeightOverride(275)
			[
				SNew(SBorder)
				[
					ScrollBox_Characters
				]
			]
		]

	];

	SVerticalBox_Props.Get().AddSlot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().VAlign(VAlign_Center).Padding(PaddingAmount).AutoHeight()
		[
			SNew(SBox).WidthOverride(200).HeightOverride(25)
			[
				SNew(SBorder).VAlign(VAlign_Center).HAlign(HAlign_Center)
				[
					SNew(STextBlock).Text(LOCTEXT("PropsTag", "Props")).ColorAndOpacity(FLinearColor::Yellow)
				]
			]
		]

		+ SVerticalBox::Slot().VAlign(VAlign_Center).Padding(PaddingAmount).AutoHeight()
		[
			SNew(SBox).WidthOverride(200).HeightOverride(275)
			[
				SNew(SBorder)
				[
					ScrollBox_Props
				]
			]
		]
	];

	SVerticalBox_Scenes.Get().AddSlot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().VAlign(VAlign_Center).Padding(PaddingAmount).AutoHeight()
		[
			SNew(SBox).WidthOverride(200).HeightOverride(25)
			[
				SNew(SBorder).VAlign(VAlign_Center).HAlign(HAlign_Center)
				[
					SNew(STextBlock).Text(LOCTEXT("ScenesTag", "Scenes")).ColorAndOpacity(FLinearColor::Yellow)
				]
			]
		]

		+ SVerticalBox::Slot().VAlign(VAlign_Center).Padding(PaddingAmount).AutoHeight()
		[
			SNew(SBox).WidthOverride(200).HeightOverride(275)
			[
				SNew(SBorder)
				[
					ScrollBox_Scenes
				]
			]
		]

	];

	TSharedRef<SWindow> Window = SNew(SWindow).Title(LOCTEXT("WindowTitle", "Mili Pipeline Tools")).SizingRule(ESizingRule::FixedSize).Content()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().VAlign(VAlign_Top).Padding(PaddingAmount).AutoHeight()
		[
			SNew(SBox).WidthOverride(600).HeightOverride(50)
			[
				SNew(SBorder).VAlign(VAlign_Center).HAlign(HAlign_Center)
				[
					SNew(STextBlock).Text(LOCTEXT("Title", "Mili Assets Downloader")).ColorAndOpacity(FLinearColor::Yellow)
				]
			]
		]
		+ SVerticalBox::Slot().VAlign(VAlign_Top).Padding(PaddingAmount).AutoHeight()
		[
			SNew(SBox).WidthOverride(600).HeightOverride(40)
			[
				SNew(SBorder).VAlign(VAlign_Center).HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoWidth()
					[
						SNew(SBox).WidthOverride(500).HeightOverride(50)
						[
							SNew(SBorder).VAlign(VAlign_Center).HAlign(HAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoWidth()
								[
									SNew(STextBlock).Text(LOCTEXT("	EnableCheckButton", " Please enable Check Button before you import assets. "))
								]
								/*
								+ SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoWidth()
								[
									SAssignNew(SEditableTextBoxPtr1, SEditableTextBox).Text(LOCTEXT("PackagePathDefault", "/Game/Content/AssetsTemp"))
								]
								+ SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoWidth()
								[
									SNew(SButton).Text(LOCTEXT("My_Button", "Browser"))
									.HAlign(HAlign_Left).VAlign(VAlign_Center)
									.ButtonColorAndOpacity(FLinearColor(0.1, 0.1, 0.1, 0.6))
									.ForegroundColor(FLinearColor(1.0, 1.0, 1.0))
									.OnClicked_Lambda([this]()->FReply {
									this->ButtonClickedTest();
									return FReply::Handled(); })
								]
								*/
							]
						]
					]
					+ SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoWidth()
					[
						SNew(SBox).WidthOverride(100).HeightOverride(40)
						[
							SNew(SBorder).VAlign(VAlign_Center).HAlign(HAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoWidth()
								[
									SAssignNew(SCheckBoxPtr1, SCheckBox).IsChecked(false ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
								]
								+ SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoWidth()
								[
									SNew(STextBlock).Text(LOCTEXT("	Checked", " Checked ")).ColorAndOpacity(FLinearColor(0.7, 0.4, 0.4, 1))
								]
							]
						]
					]
				]
			]
		]

		+ SVerticalBox::Slot().VAlign(VAlign_Bottom).HAlign(HAlign_Center).Padding(PaddingAmount).AutoHeight()
		[
			SNew(SBox).WidthOverride(600).HeightOverride(300)
			[
				SNew(SBorder)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().VAlign(VAlign_Center).AutoWidth()
					[
						SNew(SBox).WidthOverride(200).HeightOverride(300)
						[
							SVerticalBox_Characters
						]
					]
					+ SHorizontalBox::Slot().VAlign(VAlign_Center).AutoWidth()
					[
						SNew(SBox).WidthOverride(200).HeightOverride(300)
						[
							SVerticalBox_Props
						]
					]
					+ SHorizontalBox::Slot().VAlign(VAlign_Center).AutoWidth()
					[
						SNew(SBox).WidthOverride(200).HeightOverride(300)
						[
							SVerticalBox_Scenes
						]
					]
				]
			]
		]

	];
	/** Get screen size: cx cy*/
	int  cx = GetSystemMetrics(SM_CXSCREEN);
	int  cy = GetSystemMetrics(SM_CYSCREEN);
	float x = (cx - 600) / 2;
	float y = (cy - 390) / 2;
	Window.Get().Resize(FVector2D(600, 390));
	Window.Get().MoveWindowTo(FVector2D(x,y));
	return Window;
}

TSharedRef<SScrollBox> FDownloadAssetsModule::TheScrollBox(FString InType, FString InPath)
{
	TSharedRef<SScrollBox> TempScrollBox = SNew(SScrollBox);
	auto TempVerticalBox = SNew(SVerticalBox);
	TempScrollBox.Get().AddSlot()[TempVerticalBox];

	FString Path = InPath + InType;

	TArray<FString> AssetsList = FileWork::GetFoldersFromDir(Path);

	if (AssetsList.Num() != 0)
	{
		for (auto AssstName : AssetsList)
		{
			TempVerticalBox.Get().AddSlot()
				[
					SNew(SButton)
					.HAlign(HAlign_Left).VAlign(VAlign_Center)
					.ButtonColorAndOpacity(FLinearColor(0.1, 0.1, 0.1, 0.6))
					.ForegroundColor(FLinearColor(1.0, 1.0, 1.0))
					.Text(FText::FromString(AssstName))
					.OnClicked_Lambda([this,AssstName, InType, InPath]()->FReply {
					this->ElementButtonClicked(AssstName, InType, InPath);
					return FReply::Handled(); })
					.TextFlowDirection(ETextFlowDirection::RightToLeft)
				];
			//std::string Element_string = TCHAR_TO_UTF8(*element);
			//Convert::LogOutput(1, Element_string);
		}
	}
	return TempScrollBox;
}

void FDownloadAssetsModule::ElementButtonClicked(const FString InName, const FString InType, const FString InPath)
{
	bool CheckFlag = SCheckBoxPtr1->IsChecked();
	FString HeadType;
	if (InType == FString("Characters"))
	{
		HeadType = FString("c_");
	}
	else if (InType == FString("Props"))
	{
		HeadType = FString("p_");
	}
	else if (InType == FString("Scene"))
	{
		HeadType = FString("s_");
	}
	else
	{
		HeadType = FString("");
	}
	FString InPackageModelsPath = FString("/Game/Materials/") + InType + FString("/") + InName + FString("/default");
	FString InPackageTexturesPath = FString("/Game/Materials/") + InType + FString("/") + InName + FString("/default/Textures");
	FString InTexturesPath = FString("//hnas01/data/Projects/SQ/Textures/") + InType + FString("/") + InName;
	FString InModelsPath = FString("//hnas01/data/Projects/SQ/Models/") + InType + FString("/") + InName;
	FString InJsonDocPath = FString("//hnas01/data/Projects/SQ/Materials/") + InType + FString("/") + InName + FString("/default/") + HeadType + InName + FString("_mat.shadingJson");

	Convert::LogOutput(1, TCHAR_TO_UTF8(*InTexturesPath));
	Convert::LogOutput(1, TCHAR_TO_UTF8(*InModelsPath));
	Convert::LogOutput(1, TCHAR_TO_UTF8(*InJsonDocPath));
	if (CheckFlag)
	{
		FDownloadAssetsModule::OnImportTexture(InTexturesPath, InPackageTexturesPath);
		FDownloadAssetsModule::OnImportFBX(InModelsPath, InPackageModelsPath);
		FDownloadAssetsModule::OnCreateMaterialInstances(InJsonDocPath, InPackageModelsPath, InPackageTexturesPath);
	}
	else
	{
		Convert::LogOutput(1, "Please enable the SCheckBox!");
	}
}

void FDownloadAssetsModule::ButtonClickedTest()
{
	FDownloadAssetsModule::OnCreateMaterialInstances(FString("//hnas01/data/Projects/SQ/Materials/Characters/baoan/default/c_baoan_mat.shadingJson"), FString("/Game/Materials/Characters/baoan/default"),FString("/Game/Materials/Characters/baoan/default/Textures"));
}

void FDownloadAssetsModule::OnImportTexture(const FString InFullPath,const FString InPackagePath)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString FileDir_FString = InFullPath;
	TArray<FString> AllFilePathNames = FileWork::GetFilesFromDir(FileDir_FString);// All files from dirs.
	TArray<FString> DestinationFilePathNames;//The file list which can be imported.
	TArray<FString> AllFilePathNames_TEST;
	AllFilePathNames_TEST.Add(TEXT("D:/maya/image.jpg"));
	AllFilePathNames_TEST.Add(TEXT("D:/maya/jugg.jpg"));
	//FString PackagePath = FString("/Game/Content/AssetsTemp");
	//FString PackagePath = SEditableTextBoxPtr -> GetText().ToString();
	FString PackagePath = InPackagePath;
	UTextureFactory * TextureFactory = ConstructObject<UTextureFactory>(UTextureFactory::StaticClass());
	TextureFactory->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	for (auto Element_FString : AllFilePathNames)
	{
		std::string Element_string = TCHAR_TO_UTF8(*Element_FString);
		TCHAR* Element_TCHAR = Convert::StringToTCHAR(Element_string);
		TCHAR Element[MAX_PATH];
		_tcscpy(Element, Element_TCHAR);
		int i = Element_string.find(".", 0);
		std::string Format_string = Element_string.substr(i);
		TCHAR* Format_TCHAR = Convert::StringToTCHAR(Format_string);
		TCHAR Format[MAX_PATH];
		_tcscpy(Format, Format_TCHAR);
		if (FString(Format) == TEXT(".jpg") || FString(Format) == TEXT(".tga"))
		{
			DestinationFilePathNames.Add(Element);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("...\nYour file`s format is not suitable!!! : %s "), Element);
		}
	}
	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(DestinationFilePathNames, PackagePath, TextureFactory);

	//FText DialogText = FText::FromString(TEXT("Textures import completed!"));
	//FMessageDialog::Open(EAppMsgType::Ok, DialogText);

}

void FDownloadAssetsModule::OnImportFBX(const FString InFullPath, const FString InPackagePath)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString FileDir_FString = InFullPath;
	TArray<FString> AllFilePathNames = FileWork::GetFilesFromDir(FileDir_FString);// All files from dirs.
	TArray<FString> DestinationFilePathNames;//The files which can be imported.
	TArray<FString> AllFilePathNames_TEST;
	AllFilePathNames_TEST.Add(TEXT("D:/s_cube1.fbx"));
	AllFilePathNames_TEST.Add(TEXT("D:/image.jpg"));
	//FString PackagePath = FString("/Game/Content/AssetsTemp");
	FString PackagePath = InPackagePath;
	//FString PackagePath = SEditableTextBoxPtr->GetText().ToString();
	UFbxFactory* FBXFactory = ConstructObject<UFbxFactory>(UFbxFactory::StaticClass());
	//TextureFactory->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	FBXFactory->EnableShowOption();
	/** Find the file which can be imported!*/
	for (auto Element_FString : AllFilePathNames)
	{
		std::string Element_string = TCHAR_TO_UTF8(*Element_FString);
		TCHAR* Element_TCHAR = Convert::StringToTCHAR(Element_string);
		TCHAR Element[MAX_PATH];
		_tcscpy(Element, Element_TCHAR);
		bool flag = FBXFactory->FactoryCanImport(Element);
		if (flag)
		{
			DestinationFilePathNames.Add(Element);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("...\nYour file had damaged!!! : %s "), Element);
		}
	}
	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(DestinationFilePathNames, PackagePath, FBXFactory);

}

void FDownloadAssetsModule::OnCreateMaterialInstances(const FString InFullPath, const FString InPackagePath, const FString InTexturePath)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	FAssetData Data = AssetRegistryModule.Get().GetAssetByObjectPath(FName("/Game/_RD_Content/Materials/RD_Base_M_Inst.RD_Base_M_Inst"));
	UObject* MaterialInstence = Data.GetAsset();
	// Basic Textures
	FAssetData DiffuseData = AssetRegistryModule.Get().GetAssetByObjectPath(FName("/Game/_RD_Content/Textures/RD_DefaultDiffuse.RD_DefaultDiffuse"));
	UObject* DiffuseTexture = DiffuseData.GetAsset();
	UTexture* DiffuseTexture_CASTED = Cast<UTexture>(DiffuseTexture);
	FAssetData SpecularData = AssetRegistryModule.Get().GetAssetByObjectPath(FName("/Game/_RD_Content/Textures/RD_DefaultWhite.RD_DefaultWhite"));
	UObject* SpecularTexture = SpecularData.GetAsset();
	UTexture* SpecularTexture_CASTED = Cast<UTexture>(SpecularTexture);
	FAssetData NormalData = AssetRegistryModule.Get().GetAssetByObjectPath(FName("/Game/_RD_Content/Textures/RD_DefaultNormal.RD_DefaultNormal"));
	UObject* NormalTexture = NormalData.GetAsset();
	UTexture* NormalTexture_CASTED = Cast<UTexture>(NormalTexture);
	FAssetData DefaultTextureData = AssetRegistryModule.Get().GetAssetByObjectPath(FName("/Game/_RD_Content/Textures/RD_DefaultBlack.RD_DefaultBlack"));
	UObject* DefaultTexture = SpecularData.GetAsset();
	UTexture* DefaultTexture_CASTED = Cast<UTexture>(DefaultTexture);

	//Read RapodJson information
	rapidjson::Document jsonDoc;
	std::string strCachePath = TCHAR_TO_UTF8(*InFullPath);
	std::ifstream file(strCachePath.c_str());
	std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	jsonDoc.Parse(jsonStr.c_str());
	const rapidjson::Value& AllMarerials = jsonDoc["MatTextures"];

	for (rapidjson::Value::ConstMemberIterator ItemA = AllMarerials.MemberBegin(); ItemA != AllMarerials.MemberEnd(); ++ItemA)
	{
		const rapidjson::Value& Material = jsonDoc["MatTextures"][ItemA->name.GetString()];
		std::string MaterialName = ItemA->name.GetString();
		FString MaterialName_FString = FString(Convert::StringToTCHAR(MaterialName));
		// Duplicate New Material Instance
		UObject* MaterialInstence_COPY = AssetToolsModule.Get().DuplicateAsset(MaterialName_FString, InPackagePath, MaterialInstence);
		UMaterialInstanceConstant * MaterialInstanceConstant = Cast<UMaterialInstanceConstant>(MaterialInstence_COPY);

		for (rapidjson::Value::ConstMemberIterator ItemB = Material.MemberBegin(); ItemB != Material.MemberEnd(); ++ItemB)
		{
			std::string KeyName = ItemB->name.GetString();
			const std::string KeyList[6] = { "DiffuseColor","Kd","Ks","Normal","Roughness","SpecularColor"};
			for (std::string TempKey : KeyList)
			{
				/** Ensure that TempKey is the element of ItemB */
				if (TempKey == KeyName)
				{
					/** Set ParameterName*/
					FName ParameterName;
					UTexture* ParameterValue;
					if (TempKey == "DiffuseColor")
					{
						ParameterName = FName(TEXT("BasicColorTex"));
						ParameterValue = DiffuseTexture_CASTED;
					}
					else if (TempKey == "Kd")
					{
						ParameterName = FName(TEXT("BaseColor强度"));
						ParameterValue = DefaultTexture_CASTED;
					}
					else if (TempKey == "Ks")
					{
						ParameterName = FName(TEXT("Specular强度"));
						ParameterValue = DefaultTexture_CASTED;
					}
					else if (TempKey == "Normal")
					{
						ParameterName = FName(TEXT("NormalTex"));
						ParameterValue = NormalTexture_CASTED;
					}
					else if (TempKey == "Roughness")
					{
						ParameterName = FName(TEXT("Roughness强度"));
						ParameterValue = DefaultTexture_CASTED;
					}
					else if (TempKey == "SpecularColor")
					{
						ParameterName = FName(TEXT("Specular&RoughnessTex"));
						ParameterValue = SpecularTexture_CASTED;
					}
					else
					{
						ParameterName = FName(TEXT("NULL"));
						ParameterValue = DefaultTexture_CASTED;
					}

					/** Set MaterialInstanceConstant*/
					if (jsonDoc["MatTextures"][ItemA->name.GetString()][ItemB->name.GetString()][0].IsDouble())
					{
						double value = jsonDoc["MatTextures"][ItemA->name.GetString()][ItemB->name.GetString()][0].GetDouble();
						MaterialInstanceConstant->SetScalarParameterValueEditorOnly(ParameterName, value);
					}
					else if (jsonDoc["MatTextures"][ItemA->name.GetString()][ItemB->name.GetString()][0].IsString())
					{
						/** Find the texture in content browser and set it to material instance*/
						std::string value = jsonDoc["MatTextures"][ItemA->name.GetString()][ItemB->name.GetString()][0].GetString();
						std::vector<std::string> PathSplitKeys = Convert::Split(value,"/");
						std::string TextureName = PathSplitKeys[PathSplitKeys.size()-1];
						std::vector<std::string> NameSplitKeys = Convert::Split(TextureName, ".");
						if (NameSplitKeys.size() == 2)
						{
							/** Get UTexture from content browser*/
							std::string UTextureName = NameSplitKeys[0];
							std::string InPackagePath_string = Convert::FStringToString(InTexturePath);
							std::string TexturePath = InPackagePath_string + "/" + UTextureName + "." + UTextureName;
							FAssetData TextureData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(Convert::StringToTCHAR(TexturePath)));
							if (TextureData.IsValid())
							{
								UObject* Texture = TextureData.GetAsset();
								UTexture* Texture_CASTED = Cast<UTexture>(Texture);
								MaterialInstanceConstant->SetTextureParameterValueEditorOnly(ParameterName, Texture_CASTED);
							}
							else
							{
								MaterialInstanceConstant->SetTextureParameterValueEditorOnly(ParameterName, ParameterValue);
								std::string Logs = "Missing file:" + TexturePath;
								Convert::LogOutput(2, Logs);
							}
						}
						else
						{
							MaterialInstanceConstant->SetTextureParameterValueEditorOnly(ParameterName, ParameterValue);
						}
					}
					else if (jsonDoc["MatTextures"][ItemA->name.GetString()][ItemB->name.GetString()][0].IsArray())
					{
						MaterialInstanceConstant->SetTextureParameterValueEditorOnly(ParameterName, ParameterValue);
					}
					else
					{
						NULL;
					}

				}
				
			}
		}

	}



	//UObject* MaterialInstence_COPY = AssetToolsModule.Get().DuplicateAsset(FString("MI_Copy"), InPackagePath, MaterialInstence);
	//UMaterialInstanceConstant * MaterialInstanceConstant = Cast<UMaterialInstanceConstant>(MaterialInstence_COPY);
	//MaterialInstanceConstant->SetScalarParameterValueEditorOnly(FName(TEXT("Metalic")), 0.314159);
	//MaterialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("Specular")), SpecularTexture_CASTED);
	
}

void FDownloadAssetsModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FDownloadAssetsCommands::Get().PluginAction);
}

void FDownloadAssetsModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FDownloadAssetsCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDownloadAssetsModule, DownloadAssets)