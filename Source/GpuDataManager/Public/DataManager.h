// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "GenericDataArray.h"
#include "DataManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUpdate, UCanvas*, Canvas, int32, Width, int32, Height);

UCLASS(Blueprintable)
class GPUDATAMANAGER_API ADataManager : public AActor
{
	GENERATED_BODY()

public:
	ADataManager() = default;
	
	// Settings
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UMaterialInstance* BaseMaterial {};
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UCanvasRenderTarget2D> RenderTargetHandler;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName TextureParameterName;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 TextureSize = 64;

	// Handler
	UFUNCTION()
	void OnCanvasRenderTargetUpdate(UCanvas* Canvas, int32 Width, int32 Height);

	// Methods
	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Create material (available from MaterialInstance), create render target and set material parameter"))
	UMaterialInstanceDynamic *Init();

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Broadcast update event"))
	void Update();

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Set float array as active source and call update function"))
	void UpdateAsFloatArray();

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Set FVector2d array as active source and call update function"))
	void UpdateAsFVector3fArray();

	UFUNCTION(BlueprintCallable,
		meta=(ToolTip="Call update function without any array transfer"))
	void UpdateWithNoSource();

	UPROPERTY(BlueprintAssignable,
		meta=(ToolTip="Subscribe to this to handle render target update"))
	FOnUpdate OnUpdate;

	// Public members
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMaterialInstanceDynamic *MaterialInstance {};

	// Export modifiers methods
	UFUNCTION(BlueprintCallable)
	void SetFloatArray(const TArray<float> Array)
	{
		GetOrCreateFloatArray()->SetArray(std::move(Array));
	}

	// Utilities
	UFUNCTION(BlueprintCallable)
	FVector2D ToImageCoord(const int32 PlainCoord) const
	{
		return {
			static_cast<double>(PlainCoord % TextureSize),
		static_cast<double>(PlainCoord / TextureSize)};
	}

	TGenericDataArray<float>* GetOrCreateFloatArray()
	{
		if (FloatDataArray == nullptr)
		{
			FloatDataArray = new TGenericDataArray<float>(TextureSize * TextureSize);
		}
		return FloatDataArray;
	}
	TGenericDataArray<FVector3f>* GetOrCreateFVector3fArray()
	{
		if (Vector3dDataArray == nullptr)
		{
			Vector3dDataArray = new TGenericDataArray<FVector3f>(TextureSize * TextureSize);
		}
		return Vector3dDataArray;
	}

private:
	UCanvasRenderTarget2D *RenderTarget {};

	// Data modifiers
	TGenericDataArray<float>* FloatDataArray {};
	TGenericDataArray<FVector3f>* Vector3dDataArray {};

	TOptional<EArrayTypes> CurrentActiveArrayType;
};
