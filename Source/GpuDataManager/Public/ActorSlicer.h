// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "ActorSlicer.generated.h"

USTRUCT(BlueprintType)
struct FPointCloud
{
	GENERATED_BODY()
	
	TArray<bool> Points;
	FIntVector3 PointDensity; // Number of points

	FString ToString() const;

	static int32 ToPlainIndex(const FIntVector &Coord, const FIntVector &MatrixSize);
	bool IsValid(const FIntVector &Coord) const;
};

UCLASS(BlueprintType)
class USlice : public UObject, public TArray<float>
{
	GENERATED_BODY()

public:
	USlice() = default;
	USlice(TArray Data, FVector2D TargetPhysicalSize, const FIntPoint TargetResolution);

	UFUNCTION(BlueprintCallable)
	void ReInit(TArray<float> Data, FVector2D TargetPhysicalSize, const FIntPoint TargetResolution);
	
	UFUNCTION(BlueprintCallable)
	TArray<float>& GetSliceData();

	UFUNCTION(BlueprintCallable)
	FVector2D& GetTargetPhysicalSize();
	
	UFUNCTION(BlueprintCallable)
	FIntPoint GetTargetResolution();
	
protected:
	FVector2D PhysicalSize;
	FIntVector2 Resolution;
};

UCLASS(BlueprintType)
class USlicePack : public UObject, public TMap<FName, USlice>
{
	GENERATED_BODY()
public:
	using TMap<FName, USlice>::TMap;
};

UCLASS(BlueprintType)
class UCloud : public UObject, public TPair<FPointCloud, USlicePack>
{
	GENERATED_BODY()
public:
	using TPair<FPointCloud, USlicePack>::TPair;
};

UCLASS(BlueprintType)
class UCloudPack : public UObject, public TMap<FName, UCloud>
{
	GENERATED_BODY()
public:
	using TMap<FName, UCloud>::TMap;
};

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
