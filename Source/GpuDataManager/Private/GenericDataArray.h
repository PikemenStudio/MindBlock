// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include <type_traits>
#include "GenericDataArray.generated.h"

template <typename T>
class TGenericDataArray
{
public:
	explicit TGenericDataArray(int32 Capacity, bool ShouldPreAllocateArray = true);
	
	bool SetPlainArrayElement(int32 Index, T Value);
	bool SetArray(TArray<T> Array);
	int32 GetArraySize();
	int32 GetArrayCapacity() const;
	bool ResizeArray(int32 NewSize);

	bool ValidatePlaneIndex(int32 Index);

	template <typename U> requires std::is_same_v<T, U>
	class TGenericDataArrayIterator
	{
	public:
		explicit TGenericDataArrayIterator(TGenericDataArray& Src) : Index(0), Array(Src) {}

		TGenericDataArrayIterator& operator++();
		bool IsEnd() const;
		TOptional<T*> operator*();
		int32 GetPlainIndex() const;

	private:
		int32 Index;
		TGenericDataArray& Array;
	};

	TGenericDataArrayIterator<T> Begin();

protected:
	TArray<T> Data;
	int32 Capacity {};
};

template <typename T>
TGenericDataArray<T>::TGenericDataArray(int32 Capacity, bool ShouldPreAllocateArray)
{
	this->Capacity = Capacity;

	if (ShouldPreAllocateArray)
	{
		Data.Reserve(Capacity);
	}
}

template <typename T>
bool TGenericDataArray<T>::SetPlainArrayElement(int32 Index, T Value)
{
	const bool IsIndexValid = ValidatePlaneIndex(Index);
	if (IsIndexValid)
	{
		Data[Index] = Value;
	}
	return IsIndexValid;
}

template <typename T>
bool TGenericDataArray<T>::SetArray(TArray<T> Array)
{
	// Capacity tells us the size RenderTarget, to use bigger array reinit this
	if (Array.Num() > Capacity)
	{
		return false;
	}

	Data = Array;
	return true;
}

template <typename T>
int32 TGenericDataArray<T>::GetArraySize()
{
	return Data.Num();
}

template <typename T>
int32 TGenericDataArray<T>::GetArrayCapacity() const
{
	return Capacity;
}

template <typename T>
bool TGenericDataArray<T>::ResizeArray(int32 NewSize)
{
	if (NewSize > Capacity)
	{
		return false;
	}

	Data.ResizeTo(NewSize);
	return true;
}

template <typename T>
bool TGenericDataArray<T>::ValidatePlaneIndex(int32 Index)
{
	return Index >= 0 && Index < Data.Num();
}

template <typename T>
template <typename U> requires std::is_same_v<T, U>
typename TGenericDataArray<T>::template TGenericDataArrayIterator<U>& TGenericDataArray<T>::TGenericDataArrayIterator<U>::
operator++()
{
	Index++;
	return *this;
}

template <typename T>
template <typename U> requires std::is_same_v<T, U>
bool TGenericDataArray<T>::TGenericDataArrayIterator<U>::IsEnd() const
{
	return Index >= Array.Data.Num();
}

template <typename T>
template <typename U> requires std::is_same_v<T, U>
TOptional<T*> TGenericDataArray<T>::TGenericDataArrayIterator<U>::operator*()
{
	if (IsEnd())
	{
		return {};
	}
	return &Array.Data[Index];
}

template <typename T>
template <typename U> requires std::is_same_v<T, U>
int32 TGenericDataArray<T>::TGenericDataArrayIterator<U>::GetPlainIndex() const
{
	return Index;
}

template <typename T>
typename TGenericDataArray<T>::template TGenericDataArrayIterator<T> TGenericDataArray<T>::Begin()
{
	return TGenericDataArrayIterator<T>(*this);
}

UENUM()
enum class EArrayTypes
{
	Float,
	FVector3f
};

