// Fill out your copyright notice in the Description page of Project Settings.


#include "CloudCache.h"

#include "JsonObjectConverter.h"
#include "Kismet/KismetSystemLibrary.h"

void UCloudCache::Save()
{
	const auto JsonObject = CloudPack.ToJsonObject({});
	if (!JsonObject)
	{
		// Error
		return;
	}

	const auto JsonString = FCloudPack::Serialize(JsonObject);
	if (JsonString.Len() == 0)
	{
		// Error
		return;
	}

	FCloudPack::WriteToFile(JsonString);
}

void UCloudCache::Load()
{
	const auto JsonString = FCloudPack::ReadFromFile();
	if (JsonString.Len() == 0)
	{
		// Error
		return;
	}

	const auto JsonObject = FCloudPack::Deserialize(JsonString);
	if (!JsonObject)
	{
		// Error
		return;
	}

	const auto ResultStructInst = FCloudPack::FromJsonObject(JsonObject);
	if (!ResultStructInst)
	{
		// Error
		return;
	}

	CloudPack.Data.Empty();
	CloudPack = ResultStructInst.GetValue();
}

void UCloudCache::SetCloudValue(const FName& CloudTag, FPointCloud Cloud)
{
	CloudPack.Data.FindOrAdd(CloudTag).PointCloud = std::move(Cloud);
}

FCloud UCloudCache::GetCloudWithSlices(const FName& CloudTag, bool &Success)
{
	const auto Value = CloudPack.Data.Find(CloudTag);
	Success = Value;
	if (Success)
	{
		return *Value;
	}
	return {};
}

FPointCloud UCloudCache::GetCloud(const FName& CloudTag, bool &Success)
{
	const auto Value = CloudPack.Data.Find(CloudTag);
	Success = Value;
	if ( Value)
	{
		return Value->PointCloud;
	}
	return {};
}

void UCloudCache::SetSlice(const FName& CloudTag, const FName& SliceTag, FSlice Slice)
{
	CloudPack.Data.FindOrAdd(CloudTag).SlicePack.Data.FindOrAdd(SliceTag) = std::move(Slice);
}

FSlice UCloudCache::GetSlice(const FName& CloudTag, const FName& SliceTag, bool &Success)
{
	const auto Value = CloudPack.Data.Find(CloudTag);
	if (!Value)
	{
		Success = false;
		return {};
	}
	const auto ResultValue = Value->SlicePack.Data.Find(SliceTag);
	Success = ResultValue;
	if (Success)
	{
		return *ResultValue;
	}
	return {};
}

void UCloudCache::FillByTestData()
{
	FSlice NewSlice({ 1, 1, 1 }, { 1, 1 }, { 1, 1 });
	CloudPack.Data = {
		{
			"TestCloudTag", FCloud {
				.PointCloud = FPointCloud({1, 1, 1 }, { 1, 1, 1}),
				.SlicePack = FSlicePack {
					.Data = {{ "NewSliceTag", std::move(NewSlice) }}
				}
			}
		}
	};
}
