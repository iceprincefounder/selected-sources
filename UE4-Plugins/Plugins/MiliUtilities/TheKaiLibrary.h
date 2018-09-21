#pragma once

#include "ModuleManager.h"
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>



#include "SlateBasics.h"
#include "SlateExtras.h"

#include "LevelEditor.h"

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Editor/UnrealEd/Classes/Factories/FbxFactory.h"

#include "../../../../MiliUtilities/_ThirdParty/dirent.h"

class Convert
{
public:
	static TCHAR* StringToTCHAR(std::string argument);
	static void LogOutput(int mode,std::string argument);
	static std::string FStringToString(FString argument);
	static char* StringToChar(std::string argument);
	static std::vector<std::string> Split(std::string str, std::string pattern);
};

class FileWork
{
public:
	static TArray<FString> GetFilesFromDir(FString DestinationPath);
	static TArray<FString> GetFoldersFromDir(FString DestinationPath);
};

char* Convert::StringToChar(std::string argument)
{
	std::string str = argument;
	char *cstr = new char[str.length() + 1];
	strcpy(cstr, str.c_str());
	// do stuff
	//delete[] cstr;
	return cstr;
}

void Convert::LogOutput(int mode, std::string argument)
{
	TCHAR* Text_TCHAR_P = StringToTCHAR(argument);
	TCHAR Text[MAX_PATH];
	wcscpy_s(Text, Text_TCHAR_P);
	
	if (mode == 0)
	{
		UE_LOG(LogTemp, Log, Text);
	}
	else if (mode == 1)
	{
		UE_LOG(LogTemp, Warning, Text);
	}
	else if (mode == 2)
	{
		UE_LOG(LogTemp, Error, Text);
	}
	else
	{
		NULL;
	}
}

TCHAR* Convert::StringToTCHAR(std::string argument)
{
	std::string argument_string = argument;
	/** string to char */
	char argument_char[MAX_PATH];
	int i;
	for (i = 0; i<argument_string.length(); i++)
		argument_char[i] = argument_string[i];
	argument_char[i] = '\0';
	/** char to TCHAR */
	TCHAR* argument_TCHAR = new TCHAR[MAX_PATH];
	char argument_char2[MAX_PATH];
	sprintf_s(argument_char2, "%s", argument_char);
	int iLength = MultiByteToWideChar(CP_ACP, 0, argument_char2, strlen(argument_char2) + 1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, argument_char2, strlen(argument_char2) + 1, argument_TCHAR, iLength);
	TCHAR* result = argument_TCHAR;
	return result;
}

std::string Convert::FStringToString(FString argument)
{
	std::string result = TCHAR_TO_UTF8(*argument);
	return result;
}

std::vector<std::string> Convert::Split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;//扩展字符串以方便操作  
	int size = str.size();

	for (int i = 0; i<size; i++)
	{
		pos = str.find(pattern, i);
		if (pos<size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

TArray<FString> FileWork::GetFilesFromDir(FString path)
{
	//FString to string
	std::string FileDir_string = TCHAR_TO_UTF8(*path);
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
		NULL;
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
					TCHAR* FileName_TCHAR = Convert::StringToTCHAR(FileName);
					TCHAR FileName_final[MAX_PATH];
					_tcscpy(FileName_final, FileName_TCHAR);
					TCHAR* FileDir_TCHAR = Convert::StringToTCHAR(FileDir_string);
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

TArray<FString> FileWork::GetFoldersFromDir(FString path)
{
	TArray<FString> result;
	std::string path_string = TCHAR_TO_UTF8(*path);
	//const char* PATH = "D:/Test";
	char* PATH = Convert::StringToChar(path_string);
	
	DIR *dir = opendir(PATH);

	struct dirent *entry = readdir(dir);

	while (entry != NULL)
	{
		if (entry->d_type == DT_DIR)
		{
			//printf("%s\n", entry->d_name);
			std::string folder_string(entry->d_name);
			if (folder_string != "."&&folder_string != "..")
			{
				FString folder = FString(Convert::StringToTCHAR(folder_string));
				result.Add(folder);
			}
		}
		entry = readdir(dir);
	}

	closedir(dir);
	return result;
}