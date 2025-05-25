// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActorSlicer.h"
#include "CloudCache.generated.h"

#define NOT_IMPLEMENTED UE_LOG(LogTemp, Warning, TEXT("NotImplementedFunction() is not implemented!")); ensure(false)
/**
 * 
 */
UCLASS()
class GPUDATAMANAGER_API UCloudCache : public UObject
{
	GENERATED_BODY()

public:
	// Work with disk
	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Save CloudPack to the disk, works on any platform"))
	void Save() { NOT_IMPLEMENTED; }

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Update CloudPack data if necessary"))
	void Load() { NOT_IMPLEMENTED; }

	// Work with clouds
	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Save Cloud value on RAM by CloudTag; to update cloud value just provide existed tag"))
	void SetCloudValue(const FName &CloudTag, FPointCloud Cloud) { NOT_IMPLEMENTED; }

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Get pair<cloudValue, slicePack> by tag"))
	UCloud* GetCloudWithSlices(const FName &CloudTag) { NOT_IMPLEMENTED; return nullptr; }

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Get cloud value by tag"))
	FPointCloud& GetCloud(const FName &CloudTag) { NOT_IMPLEMENTED; FPointCloud *a = new FPointCloud; return *a; }

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Set slice by its tag and tag of the cloud slice was produced from"))
	void SetSlice(const FName &CloudTag, const FName &SliceTag, USlice *Slice) { NOT_IMPLEMENTED; }

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Get slice by its tag and tag of the cloud slice was produced from"))
	USlice* GetSlice(const FName &CloudTag, const FName &SliceTag) { NOT_IMPLEMENTED; return nullptr; }
	
private:
	// RAM storage
	UCloudPack CloudPack {};
};
