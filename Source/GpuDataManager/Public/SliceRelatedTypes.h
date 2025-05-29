#pragma once

#include "CoreMinimal.h"
#include "SliceRelatedTypes.generated.h"

USTRUCT(BlueprintType)
struct FPointCloud
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<bool> Points;

	UPROPERTY(BlueprintReadOnly)
	FIntVector PointDensity; // Number of points

	static int32 ToPlainIndex(const FIntVector &Coord, const FIntVector &MatrixSize);
	bool IsValid(const FIntVector &Coord) const;
};

USTRUCT(BlueprintType)
struct FSlice
{
	GENERATED_BODY()
	
	FSlice() = default;
	FSlice(TArray<float> Data, FVector2D TargetPhysicalSize, const FIntPoint TargetResolution);

	UPROPERTY(BlueprintReadOnly)
	FVector2D PhysicalSize;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint Resolution;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> Data;
};

USTRUCT(BlueprintType)
struct FSlicePack
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FName, FSlice> Data;
};

USTRUCT(BlueprintType)
struct FCloud
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FPointCloud PointCloud;

	UPROPERTY(BlueprintReadOnly)
	FSlicePack SlicePack;
};

USTRUCT(BlueprintType)
struct FCloudPack
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FName, FCloud> Data;

	TSharedPtr<FJsonObject> ToJsonObject(TOptional<FString> OutMessage) const;
	static TOptional<FCloudPack> FromJsonObject(TSharedPtr<FJsonObject> Src);

	static FString Serialize(TSharedPtr<FJsonObject> JsonObject);
	static TSharedPtr<FJsonObject> Deserialize(const FString &JsonString);

	static void WriteToFile(const FString &Text, const FString &FileName = "FCloudPackDefault.txt");
	static FString ReadFromFile(const FString &FileName = "FCloudPackDefault.txt");
};