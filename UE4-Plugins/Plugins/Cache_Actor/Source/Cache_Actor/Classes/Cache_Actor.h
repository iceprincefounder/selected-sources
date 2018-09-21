// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <map>
#include <vector>
#include "rapidjson/document.h"
#include "Cache_Actor.generated.h"

#define DEF_CacheComponent  UCache_Component
#define DEF_MiliEpcCache MiliEpcCache
#define A_DEF_CacheActor  ACache_Actor



#pragma region ///////////////////////////////////////////////Cache类   可以Game与Plugin对考
using namespace rapidjson;
using namespace std;
#define sizeofChar 1
#define sizeofLong 4
#define sizeofFloat 4
#define sizeofDouble 8
#define tchar(str) util().toTChar(str)
#define f2tchar(fstr) tchar(TCHAR_TO_UTF8(*(fstr)))
#define hgLog(Format, ...)\
{\
	UE_LOG(LogTemp,Warning,TEXT(##Format), ##__VA_ARGS__); \
}

class DEF_MiliEpcCache {
private:
	std::string epcjPath;
	std::string epcfPath;
	map<string,vector<float>> memCacheFrames;
	map<string,vector<int>> memIntsCacheFrames;
	map<string,vector<float>> memFloatsCacheFrames;
	map<string,vector<float>> memExtraTransformsCacheFrames;

public:
	DEF_MiliEpcCache() {}
	DEF_MiliEpcCache(const char* epcj_Path);
	vector <float> currentFrameCache;
	vector <float> *currentFrameCacheP;

	map<string,vector<int>> intListNamesPosSize;
	int totalExtraInts;
	map<string,vector<int>> floatListNamesPosSize;
	int totalExtraFloats;
	vector<string> extraTransformIndex;


	float currentFrame = 1.0;
	Document epcjDoc;
	void parseEpcj();
	void readEpcfFrame(int frame);
	void getLerpedFrameCache(float frame);
	void clearMemCache();
	vector<int> getExtraIntGrp(int frame,string intGrpName);
	vector<float> getExtraFloatGrp(int frame,string floatGrpName);
	vector<float> getTransform(int frame,string transformName);

	const char* assetName;
	int floatPerVertexId;
	int totalUVs;
	int timeRangeMin;
	int timeRangeMax;

};

#pragma endregion

////////////////////////////////////////////////////////////////////// 不能互考

UCLASS(hidecategories=(Input,Collision,Replication),showcategories=("Input|MouseInput","Input|TouchInput"))
class ACache_Actor: public AActor
{
	GENERATED_UCLASS_BODY()


#pragma region //////////////////////////////////////////////////Actor类 可以Game与Plugin对考
private:
	TArray<class UMaterialInstanceDynamic*> mDynamicMaterials;
	UTexture2D *mDynamicTexture;
	int32 mDataSize;
	int32 mDataSqrtSize;
	FUpdateTextureRegion2D *mUpdateTextureRegion;
	uint8 *mDynamicColors;
	uint8 *mTextureColors;
	DEF_MiliEpcCache cache;
	DEF_MiliEpcCache* cacheObject;
	bool textureInitalized = false;
	int epcjInitialized = 0;
	int currentFrame = 0;
	FString lastCachePath;



public:
	// Sets default values for this actor's properties
	//AMiliCacheActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	virtual void ReadEpcj();

	virtual void UpdateTextureRegions(UTexture2D* Texture,int32 MipIndex,uint32 NumRegions,FUpdateTextureRegion2D* Regions,uint32 SrcPitch,uint32 SrcBpp,uint8* SrcData,bool bFreeData) ;

	virtual void updateCacheTex();


	UPROPERTY(EditAnywhere) UStaticMeshComponent* CacheStaticMesh;
	class DEF_CacheComponent* CacheComponent;

	UPROPERTY(Interp) bool EnableCache;

	UPROPERTY(EditAnywhere) FString CachePath;

	UPROPERTY(EditAnywhere) float ModelUnitScale;

	UPROPERTY(EditAnywhere) int CacheUVChannel;

	UPROPERTY(Interp) bool useCustomFrame;

	UPROPERTY(Interp) float customCacheFrame;


	UPROPERTY(EditAnywhere) float printFrameData;

	UPROPERTY(EditAnywhere) bool TickLog;


	float runningTime;
};

#pragma endregion
