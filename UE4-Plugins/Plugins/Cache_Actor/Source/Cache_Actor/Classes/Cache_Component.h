// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Cache_Actor.h"
#include "rapidjson/document.h"
#include "Cache_Component.generated.h"

//#define CacheComponent UCache_Component


/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(ClassGroup=(Custom),meta=(BlueprintSpawnableComponent), hidecategories=(Object, Physics, Collision, Activation, "Components|Activation"), editinlinenew)
//class CACHE_COMPONENT_API UCache_Component: public UMeshComponent
//class CACHE_ACTOR_API UCache_Component: public UStaticMeshComponent
class CACHE_ACTOR_API UCache_Component: public UMeshComponent
{
	GENERATED_UCLASS_BODY()
	UCache_Component();

#pragma region 可以与插件对考
public:

	// Begin UActorComponent interface.
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime,enum ELevelTick TickType,FActorComponentTickFunction *ThisTickFunction) override;
	virtual void SendRenderDynamicData_Concurrent() override;
	// Begin UActorComponent interface.

	void setParentCacheActor(void* pCacheActor);

	void* parentCacheActor;

	UPROPERTY(Interp) bool EnableJsonCache;
	UPROPERTY(EditAnywhere) FString JsonCachePath;
	UPROPERTY(EditAnywhere) float UnitScale;
	UPROPERTY(Interp) bool UseCustomFrame;
	UPROPERTY(Interp) float CustomFrame;
	UPROPERTY(EditAnywhere) bool EnableCameraCache;
	UPROPERTY(EditAnywhere) bool UnitScaleEyeSeparation;
	UPROPERTY(EditAnywhere) bool PrintDebug;

	UPROPERTY(Interp) float vr_eyeSeparation; //Interp的可以从蓝图中Get


	Document jsonDoc;

private:

	bool hasCacheActor = false;
	double runningTime = 0.0;
	double currentFrame = 0.0;
	FString lastCachePath = FString("");
	bool cacheVailed = false;

	float frameStart=0.0;
	float frameEnd=1.0;
	float frameIncreament=0.5;
	float frameRate = 24.0;
	long totalSample = 0;

#pragma endregion


};


