// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "ActorSlicer.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GPUDATAMANAGER_API UActorSlicer : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UActorSlicer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	// Methods
	struct FPointCloud
	{
		TArray<bool> Points;
		FIntVector3 PointDensity; // Number of points

		FString ToString() const;

		static int32 ToPlainIndex(const FIntVector &Coord, const FIntVector &MatrixSize);
		bool IsValid(const FIntVector &Coord) const;
	};
	
	FPointCloud GeneratePointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent, FIntVector PointDensity, bool DrawDebugInfo = false) const;

	UFUNCTION(BlueprintCallable)
	void GenerateAndCachePointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent, const FIntVector PointDensity);

	UFUNCTION(BlueprintCallable)
	bool IsCacheSet() const;

	UFUNCTION(BlueprintCallable)
	void DrawPointCloudFromCache(FVector SlicerBoxLocation, FVector SlicerBoxExtent);

	UFUNCTION(BlueprintCallable)
	TArray<float> CalculateSliceOnPlane(
		const FVector& PlaneOrigin,
		const FRotator& PlaneRotation,
		const FVector& PointCloudExtent,
		const FRotator& PointCloudRotation,
		const FVector& PointCloudOrigin,
		const FVector2D& ImagePhysicalSize,
		const FIntPoint& TargetImageSize
	) const;

	UFUNCTION(BlueprintCallable)
	FString SliceToString(const TArray<float> &Src);

private:
	TOptional<FPointCloud> CachedCloud;
};
