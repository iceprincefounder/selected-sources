#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <sys/types.h>

#include "Array.h"
#include "SlateBasics.h"
#include "ModuleManager.h"
#include "AssetRegistryModule.h"
#include "Internationalization.h"
#include "Runtime/AssetRegistry/Public/AssetData.h"
#include "Editor/ContentBrowser/Public/ContentBrowserModule.h"

#include "_ThirdParty/dirent.h"
#include "_ThirdParty/base64.cpp"
#include "_ThirdParty/rapidjson/document.h"
#include "_ThirdParty/rapidjson/stringbuffer.h"
#include "_ThirdParty/rapidjson/writer.h"

using namespace std;
using namespace rapidjson;

#define sizeofChar 1
#define sizeofLong 4
#define sizeofFloat 4
#define sizeofDouble 8

#define tchar(str) util().toTChar(str)
#define toString(fstr) util::ToString(fstr)
#define f2tchar(fstr) tchar(TCHAR_TO_UTF8(*(fstr)))
#define hgLog(Format, ...)\
{\
	UE_LOG(LogTemp,Warning,TEXT(##Format), ##__VA_ARGS__); \
}
class util {
public:
	util() {}
	#pragma region string
	static vector<string> splitStr(string source,string delimiter)
	{
		vector<string> result;
		int i = 0,j = 0;
		do {
			string newStr = source.substr(i,source.size());
			j = newStr.find(delimiter);
			if(j == -1) {
				result.push_back(newStr);
				break;
			}
			string tmp = newStr.substr(0,j);
			i += j + delimiter.size();
			result.push_back(tmp);
		} while(i<source.size());
		return result;
	}
	static string ReplaceString(std::string subject,const std::string& search,const std::string& replace) {
		size_t pos = 0;
		while((pos = subject.find(search,pos)) != std::string::npos) {
			subject.replace(pos,search.length(),replace);
			pos += replace.length();
		}
		return subject;
	}
	static void ReplaceStringInPlace(std::string& subject,const std::string& search,const std::string& replace) {
		size_t pos = 0;
		while((pos = subject.find(search,pos)) != std::string::npos) {
			subject.replace(pos,search.length(),replace);
			pos += replace.length();
		}
	}
	static string JoinString(vector<string> stringVec, string connection, int start=0, int end=-1)
	{
		long start2 = std::min(long(stringVec.size()-1),  std::max( long(0),   long(start<0? stringVec.size()+start : start) ) );
		long end2 =  std::min(long(stringVec.size()-1),  std::max( start2,      long(end<0? stringVec.size()+end : end)  )  );
		if(start2==end2 || stringVec.size()==0)
			return string("");
		
		string result=stringVec[start2];
		for(long i=start2+1; i<=end2; i++)
		{
			result += connection+stringVec[i];
		}
		return result;
	}
	static string readStringLen(FILE* file1,long lenString) {
		string resultString(lenString,'\0');
		if(lenString > 0) {
			fread(&resultString[0],sizeofChar,(size_t)lenString,file1);
		}
		return resultString;
	}
	static string readTxt(const char* filepath) {
		std::ifstream infile(filepath,std::ios::in | std::ios::binary);
		if(infile.is_open()) {
			std::string contents;
			infile.seekg(0,std::ios::end);
			contents.resize(infile.tellg()); // same as: std::string contents(infile.tellg(), '\0');
			infile.seekg(0,std::ios::beg);
			infile.read(&contents[0],contents.size());
			infile.close();
			return(contents);
		}
		return string("");
		throw(errno);
	}
	static bool writeTxt(const char* filePath, string textToWrite)
	{
		std::ofstream outfile(filePath, std::ios::out | std::ios::binary);
		if(outfile.is_open()){
			outfile.write(&textToWrite[0],textToWrite.size());
			outfile.close();
			return true;
		}
		return false;
	}
	#pragma endregion
	
	#pragma region TChar
	static TCHAR* toTChar(char* origChars) {
		return ANSI_TO_TCHAR(origChars);
		//string str = string(origChars);
		//return toTChar(str);
	}
	static TCHAR* toTChar(string str) {
		return ANSI_TO_TCHAR(str.c_str());
		/*wstring aa = wstring(str.begin(),str.end());

		static TCHAR aaa260[260];
		for(int i=0; i<std::fmin(260,str.size()+1); i++) {
		memcpy(&aaa260[i],&aa[i],2);
		}
		return aaa260;*/
	}
	static string ToString(FString fstring)
	{
		return string(TCHAR_TO_UTF8(*fstring));
	}
	static string ToString(FName fname)
	{
		return ToString(fname.ToString());
	}
	#pragma endregion

	#pragma region exe_python
	struct exeResult
	{
		size_t exitCode;
		STARTUPINFOW siStartupInfo;
		PROCESS_INFORMATION piProcessInfo;
	};	
	static size_t ExecuteProcess(std::wstring FullPathToExe,std::wstring Parameters, size_t SecondsToWait=INFINITE)
	{
		size_t iMyCounter = 0,iReturnVal = 0,iPos = 0;
		std::wstring sTempStr = L"";

		/* - NOTE - You should check here to see if the exe even exists */

		/* Add a space to the beginning of the Parameters */
		if(Parameters.size() != 0)
		{
			if(Parameters[0] != L' ')
			{
				Parameters.insert(0,L" ");
			}
		}

		/* The first parameter needs to be the exe itself */
		sTempStr = FullPathToExe;
		iPos = sTempStr.find_last_of(L"\\");
		sTempStr.erase(0,iPos +1);
		Parameters = sTempStr.append(Parameters);

		/* CreateProcessW can modify Parameters thus we allocate needed memory */
		wchar_t * pwszParam = new wchar_t[Parameters.size() + 1];
		if(pwszParam == 0)
		{
			return 1;
		}
		const wchar_t* pchrTemp = Parameters.c_str();
		wcscpy_s(pwszParam,Parameters.size() + 1,pchrTemp);

		/* CreateProcess API initialization */
		STARTUPINFOW siStartupInfo;
		PROCESS_INFORMATION piProcessInfo;
		memset(&siStartupInfo,0,sizeof(siStartupInfo));
		memset(&piProcessInfo,0,sizeof(piProcessInfo));
		siStartupInfo.cb = sizeof(siStartupInfo);

		if(CreateProcessW(const_cast<LPCWSTR>(FullPathToExe.c_str()),
			pwszParam,0,0,false,
			CREATE_DEFAULT_ERROR_MODE,0,0,
			&siStartupInfo,&piProcessInfo) != false)
		{
			/* Watch the process. */
			auto dwExitCode = WaitForSingleObject(piProcessInfo.hProcess,(SecondsToWait * 1000));
		} else
		{
			/* CreateProcess failed */
			iReturnVal = GetLastError();
		}

		/* Free memory */
		delete[]pwszParam;
		pwszParam = 0;

		/* Release handles */
		CloseHandle(piProcessInfo.hProcess);
		CloseHandle(piProcessInfo.hThread);

		return iReturnVal;
	}
	static size_t ExecuteProcess(std::string FullPathToExe,std::string Parameters,size_t SecondsToWait=INFINITE)
	{
		std::wstring FullPathToExe2(FullPathToExe.begin(), FullPathToExe.end());
		std::wstring Parameters2(Parameters.begin(), Parameters.end());
		return ExecuteProcess(FullPathToExe2,Parameters2, SecondsToWait);
	}
	static size_t Python(std::string pyCode, size_t SecondsToWait=INFINITE)
	{
		string pyPath = "D:\\tmpUePy.py";
		string pipelinePath = util::ReplaceString(util::GetInstallData("Pipeline_Path"), "\\","/");
		string pyCode2 = "#coding:utf-8  \nimport os,sys; os.environ['Pipeline_Path']='"+pipelinePath+"'; \n";
		pyCode2 += "sys.path.append(os.environ['Pipeline_Path']+'/Maya/Python') \n";
		pyCode2 += "sys.path.append(os.environ['Pipeline_Path']+'/UnrealEngine') \n\n";
		pyCode2 += "os.environ['g_UnrealProjectPath'] = '"+toString(FPaths::GetProjectFilePath())+"'\n";
		pyCode2 += "os.environ['g_UnrealActiveProject'] = '"+util::GetActiveProject()+"'\n\n";

		pyCode2 += pyCode;
		FILE* file = fopen(pyPath.c_str(),"wb");
		fwrite(pyCode2.c_str(), 1,pyCode2.size(), file);
		fclose(file);
		string exePath = util::JoinString(util::splitStr(pipelinePath,"/"),"\\",0,-2)+"\\MiliProducer\\bin\\uePyPrompt\\uePyPrompt.exe";
		//string exePath = util::JoinString(util::splitStr(pipelinePath,"/"),"\\",0,-2)+"\\MiliProducer\\bin\\pyEnv\\scripts\\python.exe";
		return ExecuteProcess( exePath, pyPath,SecondsToWait);
	}
	#pragma endregion

	#pragma region configs
	static map<string, vector<string>> ListDir(string dirPath)
	{
		map<string,vector<string>> result;
		vector<string> files;
		vector<string> dirs;

		DIR *dir;
		struct dirent *ent;
		if((dir = opendir(dirPath.c_str())) != NULL) {
			// print all the files and directories within directory 
			while((ent = readdir(dir)) != NULL) {
				//printf("%s\n",ent->d_name);
				files.push_back(string(ent->d_name));
			}
			closedir(dir);
		} else {
			// could not open directory 
			perror("");
			//return EXIT_FAILURE;
		}



		result["files"] = files;
		result["dirs"] = dirs;
		return result;

	}
	static string UserDir()
	{
		string result = "C:";
		
		result.append(getenv("homepath") + string("\\Documents\\UnrealEngine"));
		hgLog("userPath: %s",tchar(result));
		return result;
	}
	static string GetInstallData(string varName)
	{
		string result = "";
		string envJson = util::UserDir()+"/env.json";

		rapidjson::Document jsonDoc;
		jsonDoc.Parse(util::readTxt(envJson.c_str()).c_str());
		if(jsonDoc.HasMember(varName.c_str()))
		{
			result = string(jsonDoc[varName.c_str()].GetString());
		}

		hgLog("install data %s",tchar(varName+"="+result));

		return result;
	}
	static string OVar(string varName, string defaultValue, bool setDefault=false)
	{
		bool setDefault2 = setDefault;
		string result = "";
		string optionJsonPath = util::UserDir()+"\\userOptions.json";
		string optionJson = util::readTxt(optionJsonPath.c_str());
		hgLog("jsonPath: %s", tchar(optionJson));
		
		rapidjson::Document jsonDoc;
		jsonDoc.Parse(optionJson.c_str());
		auto varNameChar = varName.c_str();
		if(jsonDoc.HasMember(varNameChar))
		{
			result = string(jsonDoc[varNameChar].GetString());
			hgLog("json has key: %s", tchar(varName+" : "+result));
		} else {
			setDefault2 = true;
			result = defaultValue;
			Value key(varNameChar, jsonDoc.GetAllocator());
			Value val(result.c_str(), jsonDoc.GetAllocator());
			jsonDoc.AddMember(key, val, jsonDoc.GetAllocator() );
		}

		if (setDefault2)
		{
			result = defaultValue;
			hgLog("test result: %s, %s",tchar(result),tchar("asdasd"));
			jsonDoc[varNameChar].SetString(rapidjson::GenericStringRef<char>(result.c_str()));
			hgLog(" \n\njsonTest: %s: %i ", tchar(varName+" -  "+string(jsonDoc[varNameChar].GetString())  +" , "+ result), result.size()  );
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			jsonDoc.Accept(writer);
			string reststring = string(buffer.GetString());
			hgLog("jsonString (%i):  %s", reststring.size(), tchar(buffer.GetString()));
			
			util::writeTxt(optionJsonPath.c_str(),reststring);
		}

		hgLog("ovar %s",tchar(varName+" - "+result));


		return result;
	}
	static double OVar(string varName, double defaultValue, bool setDefault=false){
		double result =defaultValue;
		string resultS = util::OVar(varName,std::to_string(defaultValue), setDefault);
		result = std::atof(resultS.c_str());
		return result;
	}
	static TArray<double> OVar(string varName, TArray<double> defaultValue, bool setDefault=false){
		auto result = defaultValue;
		//TODO: 实现

		return result;
	}
	#pragma endregion

	#pragma region projects
	static string GetActiveProject(){
		string result ;
		
		FString uprj;
		//uprj = FUnrealEdMisc::Get().GetPendingProjectName();
		//hgLog("project name: %s", *uprj);

		uprj = FPaths::GetProjectFilePath();
		hgLog("project file path: %s",*uprj);

		uprj = uprj.LeftChop(9); //去除 .uproject 尾缀
		vector<string> strVec = util::splitStr(toString(uprj),"/");
		result = strVec[strVec.size()-1];
		hgLog("project name: %s",tchar(result));

		uprj = FPaths::VideoCaptureDir();
		hgLog("video path: %s",*uprj);
		
		uprj = FPaths::GameContentDir();
		hgLog("contentPath: %s", tchar( toString(uprj).substr(5)  ));

		if(result=="RnD")
		{
			result=string("SQ");
		}

		return result;
	}

	static string GetServerPath() {
		string result = "\\\\hnas01\\data\\Projects";
		return result;
	}
	static string GetLocalPath() {
		string result = "D:\\Projects";
		return result;
	}
	
	static string ProjectConfig(string varName, string defaultValue, bool setDefault=false)
	{
		string result = defaultValue;

		string projectConfigJsonPath = util::GetServerPath()+"\\"+util::GetActiveProject()+"\\ProjectConfig\\ProjectConfig";
		hgLog("project config path: %s", tchar(projectConfigJsonPath));
		string projectConfigText = util::readTxt(projectConfigJsonPath.c_str());

		auto pairs = util::splitStr(projectConfigText,"\r\n\r\n");
		bool foundVar = false;
		for(auto& pair : pairs)
		{
			auto lines = util::splitStr(pair,"\r\n");
			if(lines.size()==2)
			{
				auto tmpValue =  base64_decode(lines[1]);
				//TODO: 解码utf-8

				if (lines[0] == varName)
				{
					foundVar = true;
					result = tmpValue;
				}
			}
		}

		if(!foundVar || setDefault)
		{
			//TODO: 实现本地写出修改和上传
		}

		return result;
	}
	static void ProjectConfigEditor(){
		string pyCmd=string("")
			+"from PySide import QtGui, QtCore; import sys  \n"
			+"app = QtGui.QApplication(sys.argv)  \n"
			+"import Mili.Module.utilties.ProjectConfig as ProjectConfig;  reload(ProjectConfig) ;  ProjectConfigInst=ProjectConfig.ProjectConfig(); ProjectConfigInst.editWindow()  \n"
			+"app.exec_()  \n"
			;
		util::Python(pyCmd);
	}
	static void TeamConfigEditor() {
		string pyCmd=string("")
			+"from PySide import QtGui, QtCore; import sys  \n"
			+"app = QtGui.QApplication(sys.argv)  \n"
			+"import Mili.Module.utilties.MiliUser as MiliUser;reload(MiliUser);MiliUser.MiliUser(update=True).editWindow() \n"
			+"app.exec_()  \n"
			;
		util::Python(pyCmd);
	}
	static void ProjectInfoTabel() {
		string pyCmd=string("")
			+"from PySide import QtGui, QtCore; import sys  \n"
			+"app = QtGui.QApplication(sys.argv)  \n"
			+"import Mili.Module.logInfo.infoTableView as infoTableView;reload(infoTableView);infoTableView.infoTableView() \n"
			+"app.exec_()  \n"
			;
		util::Python(pyCmd);
	}

	static void registerUtilConsolCmd(){
		IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.ProjectConfig"),tchar("config project"),
			FConsoleCommandDelegate().CreateStatic(util::ProjectConfigEditor)
		);

		IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.TeamConfig"),tchar("mili team config"),
			FConsoleCommandDelegate().CreateStatic(util::TeamConfigEditor)
		);

		IConsoleManager::Get().RegisterConsoleCommand(tchar("Mili.PIT"),tchar("mili team config"),
			FConsoleCommandDelegate().CreateStatic(util::ProjectInfoTabel)
		);


	}
	#pragma endregion
};

class AssetUtilities
{
public:
	AssetUtilities();
	~AssetUtilities();

	bool GatherAssetsInfo(TArray<FAssetData>& DependentAssets, bool bAvoidRDTDAssets=true) const;
	bool GatherAssetsInfo(TArray<FName> PackageNamesToMigrate,TArray<FAssetData>& DependentAssets,bool bAvoidRDTDAssets=true) const;
	bool GatherAssetsInfo(FString ClassName,TArray<FAssetData>& DependentAssets,bool bAvoidRDTDAssets=true, bool firstSelectedAsset = false) const;
	void RecursiveGetDependencies(const FName& PackageName,TSet<FName>& AllDependencies) const;
	void OnContentBrowserAssetSelectionChanged(const TArray<FAssetData>& NewSelectedAssets,bool bIsPrimaryBrowser);

};




#define LOCTEXT_NAMESPACE "FMiliToolBoxModule"

TArray<FAssetData> MiliToolBoxContentSelAssets;


AssetUtilities::AssetUtilities()
{
}
AssetUtilities::~AssetUtilities()
{
}

void AssetUtilities::RecursiveGetDependencies(const FName& PackageName,TSet<FName>& AllDependencies) const
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	TArray<FName> Dependencies;
	bool bFoundAsset = AssetRegistryModule.Get().GetDependencies(PackageName,Dependencies,EAssetRegistryDependencyType::All,true);

	/*hgLog("FoundAsset: %s  %i,   %i dependency", *(PackageName.ToString()), bFoundAsset?1:0, Dependencies.Num());
	for(int i=0; i<Dependencies.Num(); i++){
	hgLog("  %s",*(Dependencies[i].ToString()));
	}*/

	for(auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
	{
		if(!AllDependencies.Contains(*DependsIt))
		{
			const bool bIsEnginePackage = (*DependsIt).ToString().StartsWith(TEXT("/Engine"));
			const bool bIsScriptPackage = (*DependsIt).ToString().StartsWith(TEXT("/Script"));
			if(!bIsEnginePackage && !bIsScriptPackage)
			{
				AllDependencies.Add(*DependsIt);
				RecursiveGetDependencies(*DependsIt,AllDependencies);
			}
		}
	}
}

bool AssetUtilities::GatherAssetsInfo(TArray<FName> PackageNamesToMigrate,TArray<FAssetData>& DependentAssets,bool bAvoidRDTDAssets) const
{
	TSet<FName> AllDependencies;
	bool result = false;
	{
		for(auto& iPackageIt : PackageNamesToMigrate)
		{
			if(!AllDependencies.Contains(iPackageIt))
			{
				AllDependencies.Add(iPackageIt);
				RecursiveGetDependencies(iPackageIt,AllDependencies);
				result = true;
			}
		}
	}
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//TSet<FAssetData> DependentAssets;
	//DependentAssets.Empty();
	hgLog("%i Package includes",AllDependencies.Num());
	for(auto& iAsset : AllDependencies)
	{

		TArray<FAssetData> tmpDependentAssets;
		AssetRegistryModule.Get().GetAssetsByPackageName(iAsset,tmpDependentAssets);

		if(bAvoidRDTDAssets && tmpDependentAssets.Num()>0)
		{
			FString iPath = tmpDependentAssets[0].PackageName.ToString();
			if(iPath.StartsWith(FString("/Game/_RD_Content")) || iPath.StartsWith(FString("/Game/_td_Content")))
			{
				hgLog("  (avoided rd/td) %s",*(iAsset.ToString()));
				continue;
			}
		}

		hgLog("  %s",*(iAsset.ToString()));
		DependentAssets.Append(tmpDependentAssets);
	}
	hgLog("%i Rest Asset",DependentAssets.Num());
	return result;
}

bool AssetUtilities::GatherAssetsInfo(TArray<FAssetData>& DependentAssets,bool bAvoidRDTDAssets) const
{
	TArray<FName> PackageNamesToMigrate;
	bool result = false;

	for(auto& iAsset: MiliToolBoxContentSelAssets)
	{
		PackageNamesToMigrate.Add(iAsset.PackageName);
		hgLog("Gather Dependencies: %s (%s)",*(iAsset.PackageName.ToString()),*(iAsset.GetClass()->GetName()));
	}
	result = AssetUtilities().GatherAssetsInfo(PackageNamesToMigrate,DependentAssets,bAvoidRDTDAssets);
	return result;
}

bool AssetUtilities::GatherAssetsInfo(FString ClassName,TArray<FAssetData>& DependentAssets,bool bAvoidRDTDAssets,bool firstSelectedAsset) const
{
	TArray<FName> PackageNamesToMigrate;
	bool result = false;
	for(auto& iAsset: MiliToolBoxContentSelAssets)
	{
		if(iAsset.GetClass()->GetName() == ClassName)
		{
			PackageNamesToMigrate.Add(iAsset.PackageName);
			hgLog("Gather Dependencies: %s (%s)",*(iAsset.PackageName.ToString()),*(iAsset.GetClass()->GetName()));
			result = true;

			if(firstSelectedAsset)
			{
				break;
			}
		}
	}
	AssetUtilities().GatherAssetsInfo(PackageNamesToMigrate,DependentAssets,bAvoidRDTDAssets);
	return result;
}

void AssetUtilities::OnContentBrowserAssetSelectionChanged(const TArray<FAssetData>& NewSelectedAssets,bool bIsPrimaryBrowser)
{
	if(!bIsPrimaryBrowser) return;
	hgLog("Mili: %i Assets Selected",NewSelectedAssets.Num());


	MiliToolBoxContentSelAssets.Empty();
	for(auto& iAsset : NewSelectedAssets)
	{
		hgLog("Mili:   %s (%s)    %s",*(iAsset.AssetName.ToString()),*(iAsset.GetClass()->GetName()),*(iAsset.PackagePath.ToString()));
		MiliToolBoxContentSelAssets.Add(iAsset);
	}

}

