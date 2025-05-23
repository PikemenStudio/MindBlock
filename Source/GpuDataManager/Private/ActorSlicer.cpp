// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorSlicer.h"

#include <string>

#include "Kismet/BlueprintTypeConversions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UActorSlicer::UActorSlicer() : CachedCloud{}
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

FString UActorSlicer::FPointCloud::ToString() const
{
	FString Out;
	for (int32 z = 0; z < PointDensity.Z; ++z)
	{
		Out += FString::Printf(TEXT("Z=%d\n"), z);
		for (int32 y = 0; y < PointDensity.Y; ++y)
		{
			for (int32 x = 0; x < PointDensity.X; ++x)
			{
				const int32 Index = ToPlainIndex({x, y, z}, this->PointDensity);
				Out += Points[Index] ? TEXT("X") : TEXT(".");
			}
			Out += TEXT("\n");
		}
	}
	return Out;
}

int32 UActorSlicer::FPointCloud::ToPlainIndex(const FIntVector& Coord, const FIntVector& MatrixSize)
{
	return Coord.X + Coord.Y * MatrixSize.X + Coord.Z * MatrixSize.X * MatrixSize.Y;
}

bool UActorSlicer::FPointCloud::IsValid(const FIntVector& Coord) const
{
	return  Coord.X >= 0 && Coord.X < PointDensity.X &&
			Coord.Y >= 0 && Coord.Y < PointDensity.Y &&
			Coord.Z >= 0 && Coord.Z < PointDensity.Z;
}

UActorSlicer::FPointCloud UActorSlicer::GeneratePointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent, FIntVector PointDensity, bool DrawDebugInfo) const
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

void UActorSlicer::GenerateAndCachePointCloud(FVector SlicerBoxLocation, FVector SlicerBoxExtent, const FIntVector PointDensity)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetWorld returned null"));
		return;
	}

	CachedCloud = GeneratePointCloud(SlicerBoxLocation, SlicerBoxExtent, PointDensity, false);
	UE_LOG(LogTemp, Log, TEXT("Point cloud: %s"), *CachedCloud->ToString());
}

bool UActorSlicer::IsCacheSet() const
{
	return CachedCloud.IsSet();
}

void UActorSlicer::DrawPointCloudFromCache(FVector SlicerBoxLocation, FVector SlicerBoxExtent)
{
	if (!ensureAlways(IsCacheSet()))
	{
		UE_LOG(LogTemp, Error, TEXT("Point cloud Cache is empty"));
		return;
	}
	const FVector Min = SlicerBoxLocation - SlicerBoxExtent;
	const FVector Max = SlicerBoxLocation + SlicerBoxExtent;
	const FVector Size = Max - Min;
	const FVector Step = Size / FVector(CachedCloud->PointDensity);

	for (int32 ZIndex = 0; ZIndex < CachedCloud->PointDensity.Z; ++ZIndex)
	{
		for (int32 YIndex = 0; YIndex < CachedCloud->PointDensity.Y; ++YIndex)
		{
			for (int32 XIndex = 0; XIndex < CachedCloud->PointDensity.X; ++XIndex)
			{
				FVector TestPoint = Min + FVector(XIndex * Step.X, YIndex * Step.Y, ZIndex * Step.Z);

				FColor DebugColor;
				const int32 Index = FPointCloud::ToPlainIndex({XIndex, YIndex, ZIndex}, CachedCloud->PointDensity);
				if (CachedCloud->IsValid({XIndex, YIndex, ZIndex}) && CachedCloud->Points[Index])
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
TArray<float> UActorSlicer::CalculateSliceOnPlane(const FVector& PlaneOrigin,
		const FRotator& PlaneRotation,
		const FVector& PointCloudExtent,
		const FRotator& PointCloudRotation,
		const FVector& PointCloudOrigin,
		const FVector2D& ImagePhysicalSize,
		const FIntPoint& TargetImageSize) const
{
	if (!CachedCloud.IsSet()) {
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

	const FPointCloud& Cloud = CachedCloud.GetValue();
	const FIntVector3& Density = Cloud.PointDensity;
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
				if (!Cloud.IsValid(CloudCoords))
				{
					return 0.f;
				}
				return Cloud.Points[Cloud.ToPlainIndex(CloudCoords, Density)];
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

	return Output;
}

FString UActorSlicer::SliceToString(const TArray<float>& Src)
{
	FString Result;
	Result.Reserve(Src.Num() + UKismetMathLibrary::Sqrt(Src.Num()));
	
	const int ImageSize = UKismetMathLibrary::Sqrt(Src.Num());
	for (int Y = 0; Y < ImageSize; ++Y)
	{
		for (int X = 0; X < ImageSize; ++X)
		{
			const int Index = X + Y * ImageSize;
			const float Value = Src.IsValidIndex(Index) ? Src[Index] : 0.0f;

			if (Value >= 1)
				Result += TEXT("X"); // full block
			else
				Result += TEXT("0");
		}
		Result += TEXT('\n');
	}
	return Result;
}

