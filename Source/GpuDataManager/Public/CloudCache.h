// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SliceRelatedTypes.h"
#include "CloudCache.generated.h"

#define NOT_IMPLEMENTED UE_LOG(LogTemp, Warning, TEXT("NotImplementedFunction() is not implemented!")); ensure(false)
/**
 * 
 */
UCLASS(BlueprintType)
class GPUDATAMANAGER_API UCloudCache : public UObject
{
	GENERATED_BODY()

public:
	// Work with disk
	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Save CloudPack to the disk, works on any platform"))
	void Save() const;

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Update CloudPack data if necessary"))
	void Load();

	// Work with clouds
	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Save Cloud value on RAM by CloudTag; to update cloud value just provide existed tag"))
	void SetCloudValue(const FName &CloudTag, FPointCloud Cloud);

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Get pair<cloudValue, slicePack> by tag"))
	FCloud GetCloudWithSlices(const FName &CloudTag, bool &Success);

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Get cloud value by tag"))
	FPointCloud GetCloud(const FName &CloudTag, bool &Success );

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Set slice by its tag and tag of the cloud slice was produced from"))
	void SetSlice(const FName &CloudTag, const FName &SliceTag, FSlice Slice);

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Get slice by its tag and tag of the cloud slice was produced from"))
	FSlice GetSlice(const FName &CloudTag, const FName &SliceTag, bool &Success);

	UFUNCTION(BlueprintCallable)
	void FillByTestData();
	
private:
	// RAM storage
	FCloudPack CloudPack {};
};
