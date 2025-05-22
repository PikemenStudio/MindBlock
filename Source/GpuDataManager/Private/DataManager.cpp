// Fill out your copyright notice in the Description page of Project Settings.


#include "DataManager.h"
#include "Engine/Canvas.h"

#define LOG_ERROR(ErrorText) UE_LOG(LogTemp, Warning, TEXT("GpuDataRenderer %s: %s"), *FString(__func__), *FString(ErrorText));

void ADataManager::OnCanvasRenderTargetUpdate(UCanvas* Canvas, int32 Width, int32 Height)
{
	if (!Canvas)
	{
		LOG_ERROR("Canvas is null");
		return;
	}

	if (CurrentActiveArrayType.IsSet())
	{
		const FVector2d ScreenSize = {static_cast<double>(Width) / 2, static_cast<double>(Height) / 2};
		
		switch (CurrentActiveArrayType.GetValue())
		{
		case EArrayTypes::Float:
			// Transfer float array to Canvas
			{
				if (!FloatDataArray)
				{
					LOG_ERROR("Float array is NULL, but CurrentActiveArrayType is float, skip array uploading");
					UE_LOG(LogTemp, Log, TEXT("GpuDataRenderer %hs: Broadcast update"), __func__);
					OnUpdate.Broadcast(Canvas, Width, Height);
					return;
				}
				auto It = FloatDataArray->Begin();

				while (!It.IsEnd())
				{
					const float CurrentArrayValue = *(*It).GetValue();
					const FLinearColor RenderColor {CurrentArrayValue, 0, 0};
					const FVector2D PixelPos = ToImageCoord(It.GetPlainIndex());

					FCanvasTileItem Tile (PixelPos, FVector2D::UnitVector, RenderColor);
					Tile.BlendMode = SE_BLEND_Opaque;
					Canvas->DrawItem(Tile);

					++It;
				}
				break;
			}
		case EArrayTypes::FVector3f:
			// Transfer FVector2d array to canvas
			{
				if (!Vector3dDataArray)
				{
					LOG_ERROR("Vector3d array is NULL, but CurrentActiveArrayType is FVector3F, skip array uploading");
					UE_LOG(LogTemp, Log, TEXT("GpuDataRenderer %hs: Broadcast update"), __func__);
					OnUpdate.Broadcast(Canvas, Width, Height);
					return;
				}
				
				auto It = Vector3dDataArray->Begin();

				while (!It.IsEnd())
				{
					const FVector3f CurrentArrayValue = *(*It).GetValue();
					const FLinearColor RenderColor {
						CurrentArrayValue.X,
						CurrentArrayValue.Y,
						0};
					const FVector2D PixelPos = ToImageCoord(It.GetPlainIndex());

					FCanvasTileItem Tile (PixelPos, FVector2D::UnitVector, RenderColor);
					Tile.BlendMode = SE_BLEND_Opaque;
					Canvas->DrawItem(Tile);

					++It;
				}
				break;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GpuDataRenderer %hs: Broadcast update"), __func__);
	OnUpdate.Broadcast(Canvas, Width, Height);
}

UMaterialInstanceDynamic* ADataManager::Init()
{
	UE_LOG(LogTemp, Log, TEXT("DataManager initialising begin"));
	if (!this->BaseMaterial || !this->RenderTargetHandler)
	{
		LOG_ERROR("Invalid parameters.");
		if (!BaseMaterial)
		{
			LOG_ERROR("Base material is null");
		}
		else
		{
			LOG_ERROR("Render target handler is null");
		}
		return nullptr;
	}

	const auto WorldContext = GetWorld();
	if (!WorldContext)
	{
		LOG_ERROR("Can't get world context");
		return nullptr;
	}

	MaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (!MaterialInstance)
	{
		LOG_ERROR("Can't create material instance");
		return nullptr;
	}
	
	RenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(WorldContext, RenderTargetHandler, TextureSize, TextureSize);
	if (!RenderTarget)
	{
		LOG_ERROR("Can't create render target");
		return nullptr;
	}

	MaterialInstance->SetTextureParameterValue("Texture", RenderTarget);

	RenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &ADataManager::OnCanvasRenderTargetUpdate);

	RenderTarget->UpdateResourceImmediate(true);

	UE_LOG(LogTemp, Log, TEXT("DataManager initialising done"));
	return MaterialInstance;
}

void ADataManager::Update()
{
	if (!RenderTarget)
	{
		LOG_ERROR("Render target is null");
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Update Render Target"));
	RenderTarget->UpdateResource();
}

void ADataManager::UpdateAsFloatArray()
{
	CurrentActiveArrayType.Emplace(EArrayTypes::Float);
	Update();
}

void ADataManager::UpdateAsFVector3fArray()
{
	CurrentActiveArrayType.Emplace(EArrayTypes::FVector3f);
	Update();
}

void ADataManager::UpdateWithNoSource()
{
	CurrentActiveArrayType = {};
}
