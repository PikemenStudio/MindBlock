// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorSlicer.h"
#include "JsonObjectConverter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOG_ERROR(ErrorText) UE_LOG(LogTemp, Warning, TEXT("ActorSlicer %s: %s"), *FString(__func__), *FString(ErrorText));

TSharedPtr<FJsonObject> FCloudPack::ToJsonObject(TOptional<FString> OutMessage) const
{
	auto JsonObject = FJsonObjectConverter::UStructToJsonObject(*this);
	if (!JsonObject)
	{
		return nullptr;
	}
	return JsonObject;
}

TOptional<FCloudPack> FCloudPack::FromJsonObject(TSharedPtr<FJsonObject> Src)
{
	FCloudPack CloudPack;
	if (!FJsonObjectConverter::JsonObjectToUStruct<FCloudPack>(Src.ToSharedRef(), &CloudPack))
	{
		return {};
	}
	return CloudPack;
}

FString FCloudPack::Serialize(TSharedPtr<FJsonObject> JsonObject)
{
	FString JsonString;
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString)))
	{
		return "";
	}

	return JsonString;
}

TSharedPtr<FJsonObject> FCloudPack::Deserialize(const FString& JsonString)
{
	TSharedPtr<FJsonObject> JsonObject;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), JsonObject))
	{
		return nullptr;
	}

	return JsonObject;
}

void FCloudPack::WriteToFile(const FString& Text, const FString& FileName)
{
	if (!FFileHelper::SaveStringToFile(*Text, *FileName))
	{
		// Error
		return;
	}
}

FString FCloudPack::ReadFromFile(const FString& FileName)
{
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FileName))
	{
		// Error
		return {};
	}

	FString Result;
	if (!FFileHelper::LoadFileToString(Result, *FileName))
	{
		// Error
		return {};
	}
	return Result;
}

// Sets default values for this component's properties
UActorSlicer::UActorSlicer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UActorSlicer::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UActorSlicer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UActorSlicer::SetCachePointer(const TSoftObjectPtr<UCloudCache> CachePtr, const FName NewCloudCacheTag)
{
	Cache = CachePtr;
	CloudCacheTag = NewCloudCacheTag;
}

int32 FPointCloud::ToPlainIndex(const FIntVector& Coord, const FIntVector& MatrixSize)
{
	return Coord.X + Coord.Y * MatrixSize.X + Coord.Z * MatrixSize.X * MatrixSize.Y;
}

bool FPointCloud::IsValid(const FIntVector& Coord) const
{
	return  Coord.X >= 0 && Coord.X < PointDensity.X &&
			Coord.Y >= 0 && Coord.Y < PointDensity.Y &&
			Coord.Z >= 0 && Coord.Z < PointDensity.Z;
}

FSlice::FSlice(TArray<float> Data, FVector2D TargetPhysicalSize, const FIntPoint TargetResolution) :
	PhysicalSize(std::move(TargetPhysicalSize)),
	Resolution(TargetResolution),
	Data(std::move(Data))
{
}

FPointCloud UActorSlicer::GeneratePointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent, FIntVector PointDensity, bool DrawDebugInfo) const
{
	// Use the box bounds in world space
	FVector Min = SlicerBoxLocation - SlicerBoxExtent;
	FVector Max = SlicerBoxLocation + SlicerBoxExtent;
	
	//DrawDebugSphere(GetWorld(), Min, 50.f, 20, FColor::Yellow, true, 10, 0, 5);
	//DrawDebugSphere(GetWorld(), Max, 50.f, 20, FColor::Red, true, 10, 0, 5);

	const FVector Size = Max - Min;
	FVector Step = Size / FVector(PointDensity);

	FPointCloud Cloud;
	Cloud.PointDensity = PointDensity;
	Cloud.Points.Reserve(PointDensity.X * PointDensity.Y * PointDensity.Z);

	int TruePointsCount = 0;

	for (int32 ZIndex = 0; ZIndex < PointDensity.Z; ++ZIndex)
	{
		for (int32 YIndex = 0; YIndex < PointDensity.Y; ++YIndex)
		{
			for (int32 XIndex = 0; XIndex < PointDensity.X; ++XIndex)
			{
				FVector TestPoint = Min + FVector(XIndex * Step.X, YIndex * Step.Y, ZIndex * Step.Z);
				FVector LowEndPoint { TestPoint.X, TestPoint.Y, Min.Z };
				FVector UpEndPoint { TestPoint.X, TestPoint.Y, Max.Z };

				if (UKismetMathLibrary::NearlyEqual_FloatFloat(TestPoint.Z, LowEndPoint.Z))
				{
					LowEndPoint.Z -= 1;
				}
				if (UKismetMathLibrary::NearlyEqual_FloatFloat(TestPoint.Z, UpEndPoint.Z))
				{
					UpEndPoint.Z += 1;
				}

				TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes {
					UEngineTypes::ConvertToObjectType(ECC_WorldDynamic)
				};

				FHitResult LowHit;
				const bool LowHitFound = UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), LowEndPoint, TestPoint, ObjectTypes, true, {}, EDrawDebugTrace::Type::None, LowHit, false, FLinearColor::Green, FLinearColor::Red, 100);
				FHitResult UpHit;
				const bool UpHitFound = UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), UpEndPoint, TestPoint, ObjectTypes, true, {}, EDrawDebugTrace::Type::None, UpHit, false, FLinearColor::Green, FLinearColor::Red, 100);
				if (LowHitFound && UpHitFound)
				{
					if (DrawDebugInfo)
					{
						DrawDebugSphere(GetWorld(), TestPoint, 1.f, 3, FColor::Green, true, 10, 0, 0.05);
					}

					// Point is in object
					Cloud.Points.Add(true);
					++TruePointsCount;
					continue;
				}

				if (DrawDebugInfo)
				{
					DrawDebugSphere(GetWorld(), TestPoint, 1.f, 3, FColor::White, true, 10, 0, 0.05);
				}

				// Point is under or above object
				Cloud.Points.Add(false);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Found %d true points"), TruePointsCount);
	return Cloud;
}

void UActorSlicer::GeneratePointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent, const FIntVector PointDensity)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetWorld returned null"));
		return;
	}

	const auto Cloud = GeneratePointCloud(SlicerBoxLocation, SlicerBoxExtent, PointDensity, false);
	if (!Cache)
	{
		UE_LOG(LogTemp, Log, TEXT("Cache will not be used, reason: cache pointer is not set"));
		return;
	}
	
	Cache->SetCloudValue(CloudCacheTag, Cloud);
}

void UActorSlicer::GenerateOrLoadPointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent,
	const FIntVector PointDensity)
{
	if (Cache.IsValid())
	{
		bool Success;
		Cache->GetCloud(CloudCacheTag, Success);
		if (Success)
		{
			UE_LOG(LogTemp, Log, TEXT("Found cloud in cache, skip generation"))
			return;
		}
	}

	GeneratePointCloud(SlicerBoxLocation, SlicerBoxExtent, PointDensity);
}

bool UActorSlicer::IsCacheSet() const
{
	return !Cache.IsNull();
}

void UActorSlicer::DrawPointCloudFromCache(FVector SlicerBoxLocation, FVector SlicerBoxExtent)
{
	if (!ensureAlways(IsCacheSet()))
	{
		UE_LOG(LogTemp, Error, TEXT("Point cloud Cache is empty"));
		return;
	}

	bool IsCloudCached;
	auto CachedCloud = Cache->GetCloud(CloudCacheTag, IsCloudCached);
	if (!IsCloudCached)
	{
		UE_LOG(LogTemp, Error, TEXT("Point cloud is not in cache"));
		return;
	}
	
	const FVector Min = SlicerBoxLocation - SlicerBoxExtent;
	const FVector Max = SlicerBoxLocation + SlicerBoxExtent;
	const FVector Size = Max - Min;
	const FVector Step = Size / FVector(CachedCloud.PointDensity);

	for (int32 ZIndex = 0; ZIndex < CachedCloud.PointDensity.Z; ++ZIndex)
	{
		for (int32 YIndex = 0; YIndex < CachedCloud.PointDensity.Y; ++YIndex)
		{
			for (int32 XIndex = 0; XIndex < CachedCloud.PointDensity.X; ++XIndex)
			{
				FVector TestPoint = Min + FVector(XIndex * Step.X, YIndex * Step.Y, ZIndex * Step.Z);

				FColor DebugColor;
				const int32 Index = FPointCloud::ToPlainIndex({XIndex, YIndex, ZIndex}, CachedCloud.PointDensity);
				if (CachedCloud.IsValid({XIndex, YIndex, ZIndex}) && CachedCloud.Points[Index])
				{
					DebugColor = FColor::Green;
				}
				else
				{
					DebugColor = FColor::White;
				}
				
				DrawDebugSphere(GetWorld(), TestPoint, 1.f, 3, DebugColor, true, 10, 0, 0.05);
			}
		}
	}
}

struct FRectangleGrid
{
	FVector Min;
	FVector Max;
	FVector XPoint, YPoint;
	FIntVector2 TargetPointsNumber;

	FVector GetPoint(const FIntVector2 &Coords) const
	{
		check(Coords.X >= 0 && Coords.Y >= 0);
		check(Coords.X < TargetPointsNumber.X && Coords.Y < TargetPointsNumber.Y);
		
		const FVector PlaneXAxis = XPoint - Min;
		const FVector PlaneYAxis = YPoint - Min;
		const float XAlpha = static_cast<float>(Coords.X) / (TargetPointsNumber.X - 1);
		const float YAlpha = static_cast<float>(Coords.Y) / (TargetPointsNumber.Y - 1);
		return Min + PlaneXAxis * XAlpha + PlaneYAxis * YAlpha;
	}
};
FSlice UActorSlicer::CalculateSliceOnPlane(const FVector& PlaneOrigin,
		const FRotator& PlaneRotation,
		const FVector& PointCloudExtent,
		const FRotator& PointCloudRotation,
		const FVector& PointCloudOrigin,
		const FVector2D& ImagePhysicalSize,
		const FIntPoint& TargetImageSize) const
{
	if (Cache.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("Point cloud is not cached!"));
		return {};
	}
	bool IsCloudCached;
	auto CachedCloud = Cache->GetCloud(CloudCacheTag, IsCloudCached);
	if (!IsCloudCached) {
		UE_LOG(LogTemp, Error, TEXT("Point cloud is not cached!"));
		return {};
	}

	TArray<float> Output;
	Output.SetNumZeroed(TargetImageSize.X * TargetImageSize.Y);

	// Calculate slicer data
	FRotationMatrix PlaneRotator(PlaneRotation);
	FVector PlaneNormal = PlaneRotator.GetUnitAxis(EAxis::Z);
	const FVector XAxis = PlaneRotator.GetUnitAxis(EAxis::X); // Right
	const FVector YAxis = PlaneRotator.GetUnitAxis(EAxis::Y); // Up
	FPlane Plane(PlaneOrigin, PlaneNormal);

	FVector ProjectionCenter = PointCloudOrigin - Plane.PlaneDot(PointCloudOrigin) * PlaneNormal;
	FVector TopLeft = ProjectionCenter - XAxis * ImagePhysicalSize.X / 2 + YAxis * ImagePhysicalSize.Y / 2;
	FVector TopRight = ProjectionCenter + XAxis * ImagePhysicalSize.X / 2 + YAxis * ImagePhysicalSize.Y / 2;
	FVector BottomLeft = ProjectionCenter - XAxis * ImagePhysicalSize.X / 2 - YAxis * ImagePhysicalSize.Y / 2;
	FVector BottomRight = ProjectionCenter + XAxis * ImagePhysicalSize.X / 2 - YAxis * ImagePhysicalSize.Y / 2;
	
	FRectangleGrid ProjectionCanvas {
	.Min = BottomLeft,
	.Max = TopRight,
	.XPoint = BottomRight,
	.YPoint = TopLeft,
	.TargetPointsNumber = { TargetImageSize.X, TargetImageSize.Y } };

	// DrawDebugSphere(GetWorld(), TopLeft, 1.f, 3, FColor::Blue, true, 10, 0, 0.05);
	// DrawDebugSphere(GetWorld(), TopRight, 1.f, 3, FColor::Blue, true, 10, 0, 0.05);
	// DrawDebugSphere(GetWorld(), BottomLeft, 1.f, 3, FColor::Emerald, true, 10, 0, 0.05);
	// DrawDebugSphere(GetWorld(), BottomRight, 1.f, 3, FColor::Black, true, 10, 0, 0.05);

	// Calculate cloud data
	FVector Min = PointCloudOrigin - PointCloudExtent;
	FVector Max = PointCloudOrigin + PointCloudExtent;
	
	const FIntVector3& Density = CachedCloud.PointDensity;
	const FVector3d Step = (Max - Min) / FVector3d(Density);
	
	for (int32 YIndex = 0; YIndex < TargetImageSize.Y; ++YIndex)
	{
		for (int32 XIndex = 0; XIndex < TargetImageSize.X; ++XIndex)
		{
			// Calculate closest cloud point
			FVector RealTargetCoords = ProjectionCanvas.GetPoint({XIndex, YIndex});
			//DrawDebugSphere(GetWorld(), RealTargetCoords, 2.f, 3, FColor::Blue, true, 10, 0, 0.05);

			FIntVector LocalCloudCoords = FIntVector((RealTargetCoords - Min) / Step);
			LocalCloudCoords.X = UKismetMathLibrary::Clamp(LocalCloudCoords.X, 0, Density.X - 1);
			LocalCloudCoords.Y = UKismetMathLibrary::Clamp(LocalCloudCoords.Y, 0, Density.Y - 1);
			LocalCloudCoords.Z = UKismetMathLibrary::Clamp(LocalCloudCoords.Z, 0, Density.Z - 1);

			// Calculate average cloud value
			auto GetValue = [&](const FIntVector &CloudCoords) -> float
			{
				if (!CachedCloud.IsValid(CloudCoords))
				{
					return 0.f;
				}
				return CachedCloud.Points[CachedCloud.ToPlainIndex(CloudCoords, Density)];
			};
			unsigned int ValueAccumulator = GetValue(LocalCloudCoords);
			ValueAccumulator += GetValue({LocalCloudCoords.X + 1, LocalCloudCoords.Y, LocalCloudCoords.Z});
			ValueAccumulator += GetValue({LocalCloudCoords.X, LocalCloudCoords.Y + 1, LocalCloudCoords.Z});
			ValueAccumulator += GetValue({LocalCloudCoords.X, LocalCloudCoords.Y, LocalCloudCoords.Z + 1});
			ValueAccumulator += GetValue({LocalCloudCoords.X + 1, LocalCloudCoords.Y + 1, LocalCloudCoords.Z});
			ValueAccumulator += GetValue({LocalCloudCoords.X, LocalCloudCoords.Y + 1, LocalCloudCoords.Z + 1});
			ValueAccumulator += GetValue({LocalCloudCoords.X + 1, LocalCloudCoords.Y, LocalCloudCoords.Z + 1});
			ValueAccumulator += GetValue({LocalCloudCoords.X + 1, LocalCloudCoords.Y + 1, LocalCloudCoords.Z + 1});

			const float AverageValue = 256.f / 8.f * static_cast<float>(ValueAccumulator);
			
			// Write Pixel
			Output[YIndex * TargetImageSize.Y + XIndex] = AverageValue;
		}
	}

	FSlice Slice(Output, ImagePhysicalSize, TargetImageSize);
	return Slice;
}

FSlice UActorSlicer::CalculateOrLoadSliceOnPlane(const FVector& PlaneOrigin, const FRotator& PlaneRotation,
	const FVector& PointCloudExtent, const FRotator& PointCloudRotation, const FVector& PointCloudOrigin,
	const FVector2D& ImagePhysicalSize, const FIntPoint& TargetImageSize, const FName& SliceTag)
{
	if (Cache.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("Cache is null, can't calculate slice"));
		return {};
	}
	
	// Check for cache
	bool Success;
	FSlice SliceFromCache = Cache->GetSlice(CloudCacheTag, SliceTag, Success);

	if (Success)
	{
		return SliceFromCache;
	}

	FSlice NewSlice = CalculateSliceOnPlane(PlaneOrigin, PlaneRotation, PointCloudExtent, PointCloudRotation, PointCloudOrigin, ImagePhysicalSize, TargetImageSize);
	Cache->SetSlice(CloudCacheTag, SliceTag, NewSlice);
	return NewSlice;
}

void UActorSlicer::CacheSlice(FSlice Slice, FName SliceTag)
{
	Cache->SetSlice(CloudCacheTag, SliceTag, std::move(Slice));
}

FString UActorSlicer::SliceToString(const FSlice& Src)
{
	FString Result;
	Result.Reserve(Src.Data.Num() + UKismetMathLibrary::Sqrt(Src.Data.Num()));
	
	const int ImageSize = UKismetMathLibrary::Sqrt(Src.Data.Num());
	for (int Y = 0; Y < ImageSize; ++Y)
	{
		for (int X = 0; X < ImageSize; ++X)
		{
			const int Index = X + Y * ImageSize;
			const float Value = Src.Data.IsValidIndex(Index) ? Src.Data[Index] : 0.0f;

			if (Value >= 1)
				Result += TEXT("X"); // full block
			else
				Result += TEXT("0");
		}
		Result += TEXT('\n');
	}
	return Result;
}
