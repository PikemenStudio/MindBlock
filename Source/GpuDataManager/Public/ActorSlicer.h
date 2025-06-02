// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SliceRelatedTypes.h"
#include "CloudCache.h"
#include "ActorSlicer.generated.h"

/*
 * @USAGE
 *
 *	Add UCloudCache to some global instance
 * Add as actor component
 * Set pointer to global cache by SetCachePointer
 * Generate new point cloud by GenerateOrLoadPointCloud
 * Calculate slice by CalculateOrLoadSliceOnPlane
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GPUDATAMANAGER_API UActorSlicer : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorSlicer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	// Methods
	UFUNCTION(BlueprintCallable)
	void SetCachePointer(const TSoftObjectPtr<UCloudCache> CachePtr, const FName NewCloudCacheTag);

	UFUNCTION(BlueprintCallable)
	void GenerateOrLoadPointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent, const FIntVector PointDensity);

	UFUNCTION(BlueprintCallable)
	void GeneratePointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent, const FIntVector PointDensity);

	UFUNCTION(BlueprintCallable)
	bool IsCacheSet() const;

	UFUNCTION(BlueprintCallable)
	void DrawPointCloudFromCache(FVector SlicerBoxLocation, FVector SlicerBoxExtent);

    UFUNCTION(BlueprintCallable)
	FSlice CalculateSliceOnPlane(
		const FVector& PlaneOrigin,
		const FRotator& PlaneRotation,
		const FVector& PointCloudExtent,
		const FRotator& PointCloudRotation,
		const FVector& PointCloudOrigin,
		const FVector2D& ImagePhysicalSize,
		const FIntPoint& TargetImageSize
	) const;

	UFUNCTION(BlueprintCallable)
	FSlice CalculateOrLoadSliceOnPlane(
		const FVector& PlaneOrigin,
		const FRotator& PlaneRotation,
		const FVector& PointCloudExtent,
		const FRotator& PointCloudRotation,
		const FVector& PointCloudOrigin,
		const FVector2D& ImagePhysicalSize,
		const FIntPoint& TargetImageSize,
		const FName& SliceTag
	);

	UFUNCTION(BlueprintCallable)
	void CacheSlice(FSlice Slice, FName SliceTag);

	UFUNCTION(BlueprintCallable)
	FString SliceToString(const FSlice &Src);

	FPointCloud GeneratePointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent, FIntVector PointDensity, bool DrawDebugInfo = false) const;
	
private:
	TSoftObjectPtr<UCloudCache> Cache;
	FName CloudCacheTag;
};
