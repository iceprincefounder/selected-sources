// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved. 

#include "Cache_ComponentPluginPrivatePCH.h"
#include "DynamicMeshBuilder.h"
#include "EngineGlobals.h"
#include "LocalVertexFactory.h"
#include "Engine/Engine.h"

#define CacheComponent UCache_Component
#define CacheActor ACache_Actor




//////////////////////////////////////////////////////////////////////////

CacheComponent::CacheComponent( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
#pragma region 可与插件对考
{
	EnableJsonCache = false;
	JsonCachePath = FString("d:/tmpCache.json");
	UnitScale = 1.0;
	UseCustomFrame = false;
	CustomFrame = 0.0;
	EnableCameraCache = false;
	UnitScaleEyeSeparation = true;
	PrintDebug = false;
	vr_eyeSeparation = 0.65;


}

void CacheComponent::setParentCacheActor(void* pCacheActor) {
	this->parentCacheActor = pCacheActor;
	this->hasCacheActor = true;
}


void CacheComponent::OnRegister()
{
	Super::OnRegister();
	hgLog("Mili Cache Component Registered!");

	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;

	this->lastCachePath = FString("");

	if(this->hasCacheActor) {

		((CacheActor*)(this->parentCacheActor))->ReadEpcj();
		((CacheActor*)(this->parentCacheActor))->Tick(0.0f);
	}

	auto rootActorP = this->GetAttachmentRootActor();
	UCameraComponent* cameraComp = Cast<UCameraComponent>(rootActorP->GetComponentByClass(UCameraComponent::StaticClass()));
	if(cameraComp) {
		EnableCameraCache=true;
	}
}

FRotator LookAt(FVector forV,FVector upV)
{
	FRotator R;
	R.Yaw = FMath::Atan2(forV.Y,forV.X) * (180.f / PI);
	R.Pitch = FMath::Atan2(forV.Z,FMath::Sqrt(forV.X*forV.X+forV.Y*forV.Y)) * (180.f / PI);
	R.Roll = 0;
	return R;
}
FMatrix LookAtLH(FVector eye,FVector at,FVector up)
{
	FVector z = (at - eye).GetSafeNormal(0.0);  // Forward
	FVector x = FVector::CrossProduct(up,z).GetSafeNormal(0.0); // Right
	FVector y = FVector::CrossProduct(z,x);

	FMatrix m = FMatrix(FPlane(x.X,x.Y,x.Z,-FVector::DotProduct(x,eye)),
		FPlane(y.X,y.Y,y.Z,-FVector::DotProduct(y,eye)),
		FPlane(z.X,z.Y,z.Z,-FVector::DotProduct(z,eye)),
		FPlane(0,0,0,1));
	return m;
}
/*
Matrix4 LookAtRH(const Vector3<T>& eye,const Vector3<T>& at,const Vector3<T>& up)
{
Vector3<T> z = (eye - at).Normalized();  // Forward
Vector3<T> x = up.Cross(z).Normalized(); // Right
Vector3<T> y = z.Cross(x);

Matrix4 m(x.x,x.y,x.z,-(x.Dot(eye)),
y.x,y.y,y.z,-(y.Dot(eye)),
z.x,z.y,z.z,-(z.Dot(eye)),
0,0,0,1);
return m;
}

Matrix4 LookAtLH(const Vector3<T>& eye,const Vector3<T>& at,const Vector3<T>& up)
{
Vector3<T> z = (at - eye).Normalized();  // Forward
Vector3<T> x = up.Cross(z).Normalized(); // Right
Vector3<T> y = z.Cross(x);

Matrix4 m(x.x,x.y,x.z,-(x.Dot(eye)),
y.x,y.y,y.z,-(y.Dot(eye)),
z.x,z.y,z.z,-(z.Dot(eye)),
0,0,0,1);
return m;
}
*/
#pragma endregion 




void CacheComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
#pragma region 可与插件对考2
{
	Super::TickComponent(DeltaTime,TickType,ThisTickFunction);

	//hgLog("CacheComponent Tick!");

	if(this->hasCacheActor) {
		((CacheActor*)(this->parentCacheActor))->Tick(DeltaTime);
	}

#pragma region 刷json缓存
	if(this->EnableJsonCache) {
		this->runningTime += DeltaTime;

		if(this->lastCachePath!=this->JsonCachePath) {

			string strCachePath = TCHAR_TO_UTF8(*(this->JsonCachePath));
			if(!exists_test2(strCachePath.c_str())) {
				hgLog("******************Error, cache Not Exists:    %s ",tchar(strCachePath));
			} else {
				hgLog("Parsing %s",tchar(strCachePath));
				std::ifstream file(strCachePath.c_str());
				std::string jsonStr((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
				this->jsonDoc.Parse(jsonStr.c_str());
				auto& allocator = jsonDoc.GetAllocator();

				hgLog("Json Cache Loaded: %s",tchar(strCachePath));
				if(this->jsonDoc.HasMember("nodes")
					&& this->jsonDoc.HasMember("frameStart")&&this->jsonDoc.HasMember("frameEnd")
					&& this->jsonDoc.HasMember("frameRate")&&this->jsonDoc.HasMember("frameIncreament"))
				{
					this->frameStart = jsonDoc["frameStart"].GetDouble();
					this->frameEnd = jsonDoc["frameEnd"].GetDouble();
					this->frameRate = jsonDoc["frameRate"].GetDouble();
					this->totalSample = jsonDoc["totalSample"].GetInt64();
					this->frameIncreament = jsonDoc["frameIncreament"].GetDouble();
					int frameLen = ceil((frameEnd-frameStart)/frameIncreament)+1;
					hgLog("should have %i frames",frameLen);
					hgLog("frame range, rate, inc: %f, %f, %f, %f",this->frameStart,this->frameEnd,this->frameRate,this->frameIncreament);
					for(auto nodeP = this->jsonDoc["nodes"].MemberBegin(); nodeP !=this->jsonDoc["nodes"].MemberEnd(); nodeP++) {
						for(auto attrP = nodeP->value.MemberBegin(); attrP != nodeP->value.MemberEnd(); attrP++) {
							hgLog("attr size:   %i %s",attrP->value.Size(),tchar(attrP->name.GetString()));
							int tmpSize = attrP->value.Size();
							/*if(tmpSize==1 && tmpSize<frameLen){
							if(attrP->value[0].IsObject() || attrP->value[0].IsArray()){
							while(tmpSize<frameLen){
							attrP->value.PushBack(attrP->value[0], allocator);
							tmpSize+=1;
							}
							}else{
							while(tmpSize<frameLen) {
							attrP->value.PushBack(attrP->value[0].Get,allocator);
							tmpSize+=1;
							}
							}

							hgLog("     -filled to %i", tmpSize);
							}*/
						}
					}
					this->cacheVailed = true;
				} else {
					hgLog("json shoud have 'nodes' 'frameStart' 'frameEnd' 'frameRate' 'frameIncreament' key");
				}
			}
			this->lastCachePath =  this->JsonCachePath;
		}

		if(this->cacheVailed) {
			float curTime = this->runningTime*24.0f;
			if(this->UseCustomFrame)
				curTime =this->CustomFrame;
			float floatFrame = FMath::Fmod(curTime,this->frameEnd - this->frameStart) ;
			float floatItemID = FMath::Min(floatFrame/this->frameIncreament,this->frameEnd/this->frameIncreament-1);
			int lastI = floor(floatItemID)>totalSample-1? totalSample-1:floor(floatItemID);
			int nextI = ceil(floatItemID)>totalSample-1? totalSample-1:ceil(floatItemID);
			float w = FMath::Fmod(floatItemID,1.0);

			auto rootActorP = this->GetAttachmentRootActor();
			float rotCorrection = this->EnableCameraCache? -90:0;
			if(this->PrintDebug)
				hgLog("currentTime: %f,   floatFrame: %f,   ,  floatItemID: %f,  %i, %i, %f",curTime,floatFrame,floatItemID,lastI,nextI,w);
			for(auto nodeP = this->jsonDoc["nodes"].MemberBegin(); nodeP !=this->jsonDoc["nodes"].MemberEnd(); nodeP++) {
				if(nodeP->value.HasMember("transform9")) {
					int maxTrans9 = nodeP->value["transform9"].Size()-1;
					vector<float> T9;
					T9.clear();
					for(int i=0; i<9;i++)
						T9.push_back(nodeP->value["transform9"][FMath::Min(lastI,maxTrans9)][i].GetDouble()*(1.0-w) +nodeP->value["transform9"][FMath::Min(nextI,maxTrans9)][i].GetDouble()*w);
					rootActorP->SetActorLocation(FVector(T9[0],T9[2],T9[1]),false)  ;

					FTransform tmpTrans;
					tmpTrans.SetLocation(FVector(T9[0],T9[2],T9[1])*UnitScale);
					FQuat tmpQuat = FQuat::FastLerp(
						FQuat::MakeFromEuler(FVector(nodeP->value["transform9"][FMath::Min(lastI,maxTrans9)][3].GetDouble(),nodeP->value["transform9"][FMath::Min(lastI,maxTrans9)][4].GetDouble(),nodeP->value["transform9"][FMath::Min(lastI,maxTrans9)][5].GetDouble())),
						FQuat::MakeFromEuler(FVector(nodeP->value["transform9"][FMath::Min(nextI,maxTrans9)][3].GetDouble(),nodeP->value["transform9"][FMath::Min(nextI,maxTrans9)][4].GetDouble(),nodeP->value["transform9"][FMath::Min(nextI,maxTrans9)][5].GetDouble())),
						w
					);
					FVector upV = tmpQuat.GetRightVector();
					FVector forV = tmpQuat.GetUpVector();
					upV = FVector(upV.X,-upV.Z,upV.Y);
					forV = FVector(forV.X,-forV.Z,forV.Y);
					tmpTrans.SetRotation(LookAtLH(FVector(0,0,0),forV,upV).ToQuat());
					/*USphereComponent * sphereComp = Cast<USphereComponent>(rootActorP->GetComponentByClass(USphereComponent::StaticClass()));
					if(sphereComp) {
					FHitResult a;
					sphereComp->SetWorldLocation(forV*100 +this->GetAttachmentRootActor()->GetActorLocation(),false,&a,ETeleportType::None);
					}
					UBoxComponent * boxComp = Cast<UBoxComponent>(rootActorP->GetComponentByClass(UBoxComponent::StaticClass()));
					if(boxComp) {
					FHitResult a;
					boxComp->SetWorldLocation(upV*100 +this->GetAttachmentRootActor()->GetActorLocation(),false,&a,ETeleportType::None);
					}*/

					/*FRotator lookAtRot = LookAt(forV,upV);
					lookAtRot.Roll = tmpQuat.Rotator().Roll;
					tmpTrans.SetRotation(lookAtRot.Quaternion() );*/

					this->GetAttachmentRootActor()->SetActorTransform(tmpTrans);  // SetActorRotation(FVector(T9[3],T9[4],T9[5]),false)  ;
				}
				if(this->EnableCameraCache) {
					if(nodeP->value.HasMember("FOV")) {
						int maxFOV = nodeP->value["FOV"].Size()-1;
						float fov = (nodeP->value["FOV"][FMath::Min(maxFOV,lastI)].GetDouble())*(1.0-w) + (nodeP->value["FOV"][FMath::Min(maxFOV,nextI)].GetDouble())*w;
						UCameraComponent* cameraComp = Cast<UCameraComponent>(rootActorP->GetComponentByClass(UCameraComponent::StaticClass()));
						if(cameraComp) {
							cameraComp->SetFieldOfView(fov);
							cameraComp->SetRelativeRotation(FRotator(90,-90,0).Quaternion());
						}
					}
					if(nodeP->value.HasMember("aiEyeSeparation")) {// && nodeP->value.HasMember("aiSwitchEye")&&nodeP->value.HasMember("dontShiftZAxis")){
						int maxES = nodeP->value["aiEyeSeparation"].Size()-1;
						this->vr_eyeSeparation = ((nodeP->value["aiEyeSeparation"][FMath::Min(maxES,lastI)].GetDouble())*(1.0-w) + (nodeP->value["aiEyeSeparation"][FMath::Min(maxES,nextI)].GetDouble())*w)
							*  (UnitScaleEyeSeparation? this->UnitScale: 1.0)
							;
						//int maxSE = nodeP->value["aiSwitchEye"].Size()-1;
						//int maxDSZ = nodeP->value["dontShiftZAxis"].Size()-1;
						//float switchEye = nodeP->value["aiSwitchEye"][FMath::Min(maxSE,lastI)].GetBool();
						//float dontShiftZ = nodeP->value["dontShiftZAxis"][FMath::Min(maxDSZ,lastI)].GetBool();
						//hgLog(" eyeSep: %f   switchEye: %i    dontShiftZ: %i",eyeSep, switchEye?1:0, dontShiftZ?1:0);
					}


				}
				break;
			}
		}
	}
#pragma endregion

	// Need to send new data to render thread
	MarkRenderDynamicDataDirty();
	// Call this because bounds have changed
	UpdateComponentToWorld();
};


void CacheComponent::SendRenderDynamicData_Concurrent()
{
	if(SceneProxy)
	{
	}
}

#pragma endregion

