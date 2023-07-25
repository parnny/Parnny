// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XlsxDefines.generated.h"

USTRUCT(BlueprintType)
struct FXlsxImportSettings
{
	GENERATED_BODY()

	FString Filename;
	TObjectPtr<UScriptStruct> ImportRowStruct;
};

UENUM()
enum EXlsxImportSettingsWindowCallback
{
	PropertyChanged,
	SettingsConfirm,
	WindowClose,
};

UENUM(BlueprintType)
enum class EXlsxEnumExample : uint8
{
	EnumCase1,
	EnumCase2,
	EnumCase3,
};

USTRUCT(BlueprintType)
struct FXlsxStructSubCase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FieldString;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FieldInt32;
};

USTRUCT(BlueprintType)
struct FXlsxStructExample : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> FieldObject;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EXlsxEnumExample FieldEnum;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector FieldVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FXlsxStructSubCase> FieldArray;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FXlsxStructSubCase FieldComplex;
};


USTRUCT(BlueprintType)
struct FXlsxFieldMeta
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, meta=(ForceInlineRow))
	TMap<FName, FName> Meta;

	bool HasMeta(const FName& Name) const
	{
		return Meta.Contains(Name);
	}

	FName GetMeta(const FName& Name) const
	{
		return Meta.FindRef(Name);
	}
};