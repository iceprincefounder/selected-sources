
#include "Cache_ComponentPluginPrivatePCH.h"


#pragma region ///////////////////////////////////////////////Cache方法   可以Game与Plugin对考

#define hgHudLog(Format, ...)  GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,	FString::Printf(TEXT(##Format),  ##__VA_ARGS__))

class util {
public:
	util() {}
	vector<string> splitStr(string source,string delimiter)
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
	string ReplaceString(std::string subject,const std::string& search,const std::string& replace) {
		size_t pos = 0;
		while((pos = subject.find(search,pos)) != std::string::npos) {
			subject.replace(pos,search.length(),replace);
			pos += replace.length();
		}
		return subject;
	}
	void ReplaceStringInPlace(std::string& subject,const std::string& search,const std::string& replace) {
		size_t pos = 0;
		while((pos = subject.find(search,pos)) != std::string::npos) {
			subject.replace(pos,search.length(),replace);
			pos += replace.length();
		}
	}
	string readStringLen(FILE* file1,long lenString) {
		string resultString(lenString,'\0');
		if(lenString > 0) {
			fread(&resultString[0],sizeofChar,(size_t)lenString,file1);
		}
		return resultString;
	}
	TCHAR* toTChar(string str) {
		wstring aa = wstring(str.begin(),str.end());

		static TCHAR aaa260[260];
		for(int i=0; i<std::fmin(260,str.size()+1); i++) {
			memcpy(&aaa260[i],&aa[i],2);
		}
		return aaa260;
	}
	TCHAR* toTChar(char* origChars) {
		string str = string(origChars);
		return toTChar(str);
	}
};
inline bool exists_test2(const char* name) { // checking file existance
	struct stat buffer;
	return (stat(name,&buffer) == 0);
}

#pragma region MiliCache
DEF_MiliEpcCache::DEF_MiliEpcCache(const char* epcj_Path) {
	this->epcjPath= string(epcj_Path)+".epcj";
	this->epcfPath= string(epcj_Path)+".epcf";
	this->parseEpcj();
	this->getLerpedFrameCache(float(this->timeRangeMin));
}
void DEF_MiliEpcCache::parseEpcj() {
	UE_LOG(LogTemp,Warning,TEXT("cache epcj path: %s"),tchar(this->epcjPath));

	std::ifstream file(this->epcjPath.c_str());
	std::string epcjString((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());

	epcjDoc.Parse(epcjString.c_str());

	this->assetName = epcjDoc["assetName"].GetString();
	this->floatPerVertexId = epcjDoc["floatPerVertexId"].GetInt();
	this->totalUVs = epcjDoc["totalUVs"].GetInt();
	this-> timeRangeMin = epcjDoc["timeRange"][0].GetInt();
	this->timeRangeMax = epcjDoc["timeRange"][1].GetInt();

	this->totalExtraInts = 0;
	if(epcjDoc.HasMember("perFrameIntDataList")) {
		hgLog("extra ints groups : %i ",epcjDoc["perFrameIntDataList"].Size());
		for(unsigned int i=0; i<epcjDoc["perFrameIntDataList"].Size(); i++) {
			string tmpName = string(epcjDoc["perFrameIntDataList"][i][0].GetString());
			vector<int> posSize;
			posSize.push_back(this->totalExtraInts);
			posSize.push_back(epcjDoc["perFrameIntDataList"][i][1].GetInt());
			hgLog("    extra ints: '%s'  %i  %i",tchar(tmpName.c_str()),posSize[0],posSize[1]);
			this->intListNamesPosSize[tmpName] = posSize;
			this->totalExtraInts += posSize[1];
		}
	}
	this->totalExtraFloats=0;
	if(epcjDoc.HasMember("perFrameFloatDataList")) {
		hgLog("extra float groups : %i ",epcjDoc["perFrameFloatDataList"].Size());
		for(unsigned int i=0; i<epcjDoc["perFrameFloatDataList"].Size(); i++) {
			string tmpName = string(epcjDoc["perFrameFloatDataList"][i][0].GetString());
			vector<int> posSize;
			posSize.push_back(this->totalExtraFloats);
			posSize.push_back(epcjDoc["perFrameFloatDataList"][i][1].GetInt());
			hgLog("    extra floats: '%s'  %i  %i",tchar(tmpName.c_str()),posSize[0],posSize[1]);
			this->floatListNamesPosSize[tmpName] = posSize;
			this->totalExtraFloats += posSize[1];
		}
	}
	this->extraTransformIndex.clear();
	if(epcjDoc.HasMember("extraTransforms")) {
		hgLog("extra transforms : %i ",epcjDoc["extraTransforms"].Size());
		for(unsigned int i=0; i<epcjDoc["extraTransforms"].Size(); i++) {
			string tmpName = string(epcjDoc["extraTransforms"][i].GetString());
			this->extraTransformIndex.push_back(tmpName);
			hgLog("    transform: '%s'",tchar(tmpName.c_str()));
		}
	}

	hgLog("time range: %i - %i",this->timeRangeMin,this->timeRangeMax);

	GEngine->AddOnScreenDebugMessage(5,10.0f,FColor::Green,
		FString::Printf(TEXT("jsonSize: %i,   assetName: %s,   floatPerVertexId: %i,   totalUVs:  %i,  frameRange: %i - %i"),
			epcjString.size(),
			tchar(epcjDoc["assetName"].GetString()),
			epcjDoc["floatPerVertexId"].GetInt(),
			epcjDoc["totalUVs"].GetInt(),
			epcjDoc["timeRange"][0].GetInt(),
			epcjDoc["timeRange"][1].GetInt()
		)
	);

	return;
}
void DEF_MiliEpcCache::readEpcfFrame(int frame) {

	string frameStr= std::to_string(frame);

	if(this->memCacheFrames.count(frameStr) == 0) {
		hgLog("read frame %i",frame)
			int64 startIndex = this->epcjDoc["seekIndex"][frameStr.c_str()][0].GetInt();
		int64 endIndex = this->epcjDoc["seekIndex"][frameStr.c_str()][1].GetInt();
		int64 length = endIndex-startIndex+1;
		FILE* file = fopen(this->epcfPath.c_str(),"rb");
		fseek(file,startIndex,SEEK_SET);

		int64 texDataLen = length/4  - this->totalExtraFloats  - this->totalExtraInts  - this->extraTransformIndex.size()* this->floatPerVertexId;
		//hgLog("------------------- ints: %i    floats %i   transform floats %i,  texDataLen %i",this->totalExtraInts,  this->totalExtraFloats,(this->extraTransformIndex).size()*9,  texDataLen);
		vector<int> extraInts;
		extraInts.resize(this->totalExtraInts);
		if(this->totalExtraInts>0)
			fread(&extraInts[0],4,(size_t)(this->totalExtraInts),file);
		memIntsCacheFrames[frameStr] = extraInts;

		vector<float> extraFloats;
		extraFloats.resize(this->totalExtraFloats);
		if(this->totalExtraFloats>0)
			fread(&extraFloats[0],4,(size_t)(this->totalExtraFloats),file);
		memFloatsCacheFrames[frameStr] = extraFloats;
		//hgLog("   -bb  %f %f %f",extraFloats[0],extraFloats[1],extraFloats[2],extraFloats[3],extraFloats[4],extraFloats[5]);

		vector<float> Transforms;
		Transforms.resize((this->extraTransformIndex).size()* 9);
		if(Transforms.size()>0)
			fread(&Transforms[0],4,(size_t)(Transforms.size()),file);
		memExtraTransformsCacheFrames[frameStr] = Transforms;
		//hgLog("   -t  %f %f %f",Transforms[6],Transforms[7],Transforms[8]);

		//fseek(file,startIndex+ (this->totalExtraFloats + this->totalExtraInts + this->extraTransformIndex.size()*9)*4,SEEK_SET);
		vector<float> tmpCache;
		tmpCache.resize(texDataLen);
		this->currentFrameCache.resize(texDataLen);
		fread(&tmpCache[0],4,(size_t)texDataLen,file);

		fclose(file);

		memCacheFrames[frameStr] = tmpCache;
	} else {
		//this->currentFrameCache = memCacheFrames[frameStr];
	}
}
void DEF_MiliEpcCache::getLerpedFrameCache(float frame) {
	int beginFrame = int(frame);
	int endFrame = beginFrame == this->timeRangeMax? beginFrame: beginFrame+1;
	float weight = beginFrame == this->timeRangeMax? beginFrame : frame-beginFrame;
	float startWeight = 1.0-weight;
	this->readEpcfFrame(beginFrame);
	this->readEpcfFrame(endFrame);
	vector <float> &frameCache = this->memCacheFrames[to_string(beginFrame)];
	vector <float> &nextFrameCache = this->memCacheFrames[to_string(endFrame)];
	vector<float> lerpedCache;
	//hgLog("float frame: %f,   int frame: %i, %i,  weight: %f, %f,   size: %i", frame, beginFrame, endFrame,  startWeight, weight,frameCache.size());
	lerpedCache.resize(frameCache.size());
	for(int i=0; i<lerpedCache.size(); i++) {
		lerpedCache[i] =  frameCache[i]*startWeight + nextFrameCache[i]*weight;
		//lerpedCache[i]  = frameCache[i];
	}
	this->currentFrameCache = lerpedCache;
	//this->currentFrameCacheP = &lerpedCache;

}

void DEF_MiliEpcCache::clearMemCache() {
	hgLog("===================================== Clear Mem Cache");
	memCacheFrames.clear();
	memIntsCacheFrames.clear();
	memFloatsCacheFrames.clear();
	memExtraTransformsCacheFrames.clear();

	intListNamesPosSize.clear();
	floatListNamesPosSize.clear();
	totalExtraInts=0;
	totalExtraFloats=0;
	extraTransformIndex.clear();
}

vector<float> DEF_MiliEpcCache::getExtraFloatGrp(int frame,string floatGrpName) {
	vector<float> result;
	result.clear();
	if(this->floatListNamesPosSize.count(floatGrpName)<=0)
		return result;
	string frameStr= std::to_string(frame);
	if(this->memCacheFrames.count(frameStr) == 0)
		this->getLerpedFrameCache(float(frame));
	for(int i=this->floatListNamesPosSize[floatGrpName][0]; i<(this->floatListNamesPosSize[floatGrpName][0]+this->floatListNamesPosSize[floatGrpName][1]); i++) {
		result.push_back(this->memFloatsCacheFrames[frameStr][i]);
	}
	return result;
}

#pragma endregion

#pragma endregion 

////////////////////////////////////////////////////////////////不能对考
A_DEF_CacheActor::A_DEF_CacheActor(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)

#pragma region ///////////////////////////////////////////////Actor方法   可以Game与Plugin对考
{
	PrimaryActorTick.bCanEverTick = false;//已经通过component来调用tick了，避免双重tick造成播放时速度加快一倍。

	CacheStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CacheStaticMesh"));
	CacheComponent = CreateDefaultSubobject<DEF_CacheComponent>(TEXT("Cache_Component"));
	RootComponent = CacheComponent;
	CacheComponent->setParentCacheActor((void*)this);

	ModelUnitScale = 1.0f;
	CacheUVChannel = 2;
	EnableCache = true;
	CachePath = FString("d:/tmpCache");
	useCustomFrame=false;
	customCacheFrame = 0;
	printFrameData = -1000000;

	hgLog("%s (MiliCacheActor) Initialized !!  ",f2tchar(this->GetName()));
}

void A_DEF_CacheActor::BeginPlay()
{
	Super::BeginPlay();
	hgLog("%s (MiliCacheActor) BeginPlay!  ",f2tchar(this->GetName()));
}
// Called every frame
void A_DEF_CacheActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewLocation = GetActorLocation();

	float DeltaHeight = FMath::Sin(runningTime+DeltaTime) -FMath::Sin(runningTime);
	NewLocation.Z += DeltaHeight*2.0f;
	runningTime += DeltaTime;
	//SetActorLocation(NewLocation);
	if(EnableCache) { //如果激活缓存
		if(this->epcjInitialized==0  ||  this->lastCachePath != this->CachePath)
			this->ReadEpcj();

		if(this->epcjInitialized==1) {
			float curTime = (runningTime+DeltaTime)*24.0f;
			if(this->useCustomFrame)
				curTime =this->customCacheFrame;
			float floatFrame = FMath::Fmod(curTime,this->cacheObject->timeRangeMax - this->cacheObject->timeRangeMin) + this->cacheObject->timeRangeMin;
			int cacheFrame = int(floatFrame);
			int nextFrame = cacheFrame< this->cacheObject->timeRangeMax ?  cacheFrame : cacheFrame+1;
			this->currentFrame = cacheFrame;
			float interpWeight = nextFrame==cacheFrame? float(cacheFrame) : (floatFrame-cacheFrame)/(nextFrame-cacheFrame);


			//this->cacheObject->readEpcfFrame(cacheFrame);
			this->cacheObject->getLerpedFrameCache(floatFrame);
			this->updateCacheTex();

			if(cacheFrame==this->printFrameData) { //打印debug帧数据
				for(int ii=0; ii<this->cacheObject->currentFrameCache.size(); ii+=1) {
					uint8 tmp0[4];
					memcpy(&tmp0[0],(uint8*)&this->cacheObject->currentFrameCache[ii],4);
					uint8 tmp[4] ={tmp0[3], tmp0[2], tmp0[1], tmp0[0]};
					uint8 tmp2[4] ={tmp[0], tmp[1], tmp[2], tmp[3]};
					float tmpF = -10.0f;
					memcpy(&tmpF,(float*)&tmp2[0],4);

					if(this->TickLog)
						hgLog("%i       %f   %f   %f   %f            %f, %f",ii,tmp[0]/255.0,tmp[1]/255.0,tmp[2]/255.0,tmp[3]/255.0,tmpF,this->cacheObject->currentFrameCache[ii]);
				}
			}

			FVector bMin,bMax,bCenter,bSize,bToScale;
			this->CacheStaticMesh->GetLocalBounds(bMin,bMax);
			bCenter = (bMin+bMax)*0.5;
			bSize=(bMax-bMin);
			bToScale = FVector(1e6)/bSize;


			vector<float> cacheBB = this->cacheObject->getExtraFloatGrp(cacheFrame,"_BoundingBox");
			if(cacheBB.size()==6) {
				if(this->TickLog) {
					hgLog("boundingBox:  %f %f %f   %f %f %f",cacheBB[0],cacheBB[1],cacheBB[2],cacheBB[3],cacheBB[4],cacheBB[5]);
					hgLog("BB  Size ------------------   %i",cacheBB.size());
				}
				FVector newBBMin = FVector(cacheBB[0],cacheBB[2],cacheBB[1])*(this->ModelUnitScale);
				FVector newBBMax = FVector(cacheBB[3],cacheBB[5],cacheBB[4])*(this->ModelUnitScale);
				FVector newBBCenter = (newBBMax+newBBMin)*0.5;
				bToScale = (newBBMax-newBBMin)/bSize;
				this->CacheStaticMesh->SetWorldLocation(newBBCenter-bCenter*bToScale);
				this->CacheStaticMesh->SetWorldRotation(FQuat(0,0,0,0));
				this->CacheStaticMesh->SetWorldScale3D(bToScale);
			}
		}

	}

}

void A_DEF_CacheActor::ReadEpcj() {
	this->epcjInitialized = 0;
	cache.clearMemCache();

	string strCachePath = TCHAR_TO_UTF8(*(this->CachePath));
	string epcjPath = strCachePath+".epcj";
	if(!exists_test2(epcjPath.c_str())) {
		hgLog("******************Error, cache Not Exists:    %s ",tchar(epcjPath));
		this->epcjInitialized=-1;
		return;
	}

	const char* chaCachePath  = strCachePath.c_str();
	hgLog("******************Check Cache Path:    %s  %i",tchar(epcjPath),exists_test2(epcjPath.c_str())? 1:0)
		this->cache = DEF_MiliEpcCache(chaCachePath);
	this->cacheObject = &(cache);

	int floatPerVertexId =  cacheObject->floatPerVertexId;
	int ww = FMath::Pow(2,FMath::CeilToFloat(FMath::Log2(FMath::Pow(this->cacheObject->totalUVs* floatPerVertexId,0.5))));
	hgLog("totalUVs %i,   totalPixels %i   test %f %f %f    %i  ",this->cacheObject->totalUVs,ww*ww,
		FMath::Pow(this->cacheObject->totalUVs*floatPerVertexId,0.5),
		FMath::Log2(FMath::Pow(this->cacheObject->totalUVs*floatPerVertexId,0.5)),
		FMath::CeilToFloat(FMath::Log2(FMath::Pow(this->cacheObject->totalUVs*floatPerVertexId,0.5))),
		ww
	);


	int w = ww;
	int h = ww;

	//Create a dynamic texture with the default compression (B8G8R8A8)
	mDynamicTexture = UTexture2D::CreateTransient(w,h);
	mDynamicTexture->AddressX = TextureAddress::TA_Clamp;  //::TA_Wrap;
	mDynamicTexture->AddressY = TextureAddress::TA_Clamp;
	//Make sure it won't be compressed
	mDynamicTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	mDynamicTexture->Filter = TextureFilter::TF_Nearest;
	//Turn off Gamma-correction
	mDynamicTexture->SRGB = 0;
	//Guarantee no garbage collection by adding it as a root reference
	mDynamicTexture->AddToRoot();
	//Update the texture with new variable values.
	mDynamicTexture->UpdateResource();


	mDynamicMaterials.Empty();
	for(int i=0; i<CacheStaticMesh->GetNumMaterials(); i++)
	{
		mDynamicMaterials.Add(CacheStaticMesh->CreateAndSetMaterialInstanceDynamic(i));
	}
	GEngine->AddOnScreenDebugMessage(1,3.5f,FColor::Green,FString::Printf(TEXT("Cache image wh: %i  %i"),w,h));
	GEngine->AddOnScreenDebugMessage(2,3.5f,FColor::Green,FString::Printf(TEXT("Num mat: %i"),CacheStaticMesh->GetNumMaterials()));
	hgLog("Num mat: %i",CacheStaticMesh->GetNumMaterials());

	FTexture2DMipMap& readMip = mDynamicTexture->PlatformData->Mips[0]; //从同一张贴图中读取新像素
	mDataSize = w * h * 4; // * 4 because we're working with uint8's - which are 4 bytes large
	mDataSqrtSize = w * 4; // * 4 because we're working with uint8's - which are 4 bytes large
						   // Initalize our dynamic pixel array with data size
	mDynamicColors = new uint8[mDataSize];
	mTextureColors = new uint8[mDataSize];
	GEngine->AddOnScreenDebugMessage(3,3.5f,FColor::Green,FString::Printf(TEXT("Texture DataSize: %i"),mDataSize));

	// Copy our current texture's colors into our dynamic colors
	FMemory::Memcpy(mDynamicColors,mTextureColors,mDataSize);
	readMip.BulkData.GetCopy((void**)&mTextureColors);
	// Create a new texture region with the width and height of our dynamic texture
	mUpdateTextureRegion = new FUpdateTextureRegion2D(0,0,0,0,w,h);
	// Set the Paramater in our material to our texture
	for(int i=0; i<CacheStaticMesh->GetNumMaterials(); i++)
	{
		mDynamicMaterials[i]->SetTextureParameterValue("Cache Image",mDynamicTexture);
		mDynamicMaterials[i]->SetScalarParameterValue("Cache Img Width",w);
		mDynamicMaterials[i]->SetScalarParameterValue("Enable Cache Image",this->EnableCache);

		mDynamicMaterials[i]->SetScalarParameterValue("Unit Scale",this->ModelUnitScale);


	}
	this->epcjInitialized = 1;
	this->lastCachePath = this->CachePath;
}

void A_DEF_CacheActor::UpdateTextureRegions(UTexture2D* Texture,int32 MipIndex,uint32 NumRegions,FUpdateTextureRegion2D* Regions,uint32 SrcPitch,uint32 SrcBpp,uint8* SrcData,bool bFreeData)
{
	if(Texture && Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;


		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*,RegionData,RegionData,
			bool,bFreeData,bFreeData,
			{
				for(uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if(RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
		if(bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});

	}
}

void A_DEF_CacheActor::updateCacheTex() {
	int pixelAmount = this->mDataSize / 4;
	if(this->textureInitalized==true)
		pixelAmount = this->cacheObject->currentFrameCache.size();
	else {
		while(this->cacheObject->currentFrameCache.size()<pixelAmount) {
			this->cacheObject->currentFrameCache.push_back(0.0f);
		}
	}

	if(this->TickLog)
		hgLog("%i------------------ pix amount: %i   ",this->currentFrame,pixelAmount);


	for(int i = 0; i < pixelAmount; i++)
	{
		int blue = i * 4 + 0;
		int green = i * 4 + 1;
		int red = i * 4 + 2;
		int alpha = i * 4 + 3;

		int ii = fmod(i,this->cacheObject->currentFrameCache.size());

		mDynamicColors[red] =0;
		mDynamicColors[green] = 0;
		mDynamicColors[blue] =0;
		mDynamicColors[alpha] = 0;

		memcpy(&(mDynamicColors[blue]),(uint8*)&(this->cacheObject->currentFrameCache[i]),4);

		//hgLog("%i  %i  %i  %i ,",mDynamicColors[red],mDynamicColors[green],mDynamicColors[blue],mDynamicColors[alpha]);
	}
	//GEngine->AddOnScreenDebugMessage(4,3.5f,FColor::Green,FString::Printf(TEXT("sinf..: %f"),sinf));

	UpdateTextureRegions(mDynamicTexture,0,1,mUpdateTextureRegion,mDataSqrtSize,(uint32)4,mDynamicColors,false);
	this->textureInitalized = true;
}
#pragma endregion 
