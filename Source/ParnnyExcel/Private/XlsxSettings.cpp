// Fill out your copyright notice in the Description page of Project Settings.


#include "XlsxSettings.h"

UXlsxSettings::UXlsxSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	XlsxSourcePath.Path = TEXT("/Xlsx");
	XlsxExportPath.Path = TEXT("/Xlsx");
}

FName UXlsxSettings::GetCategoryName() const
{
	return FName("Parnny");
}

FName UXlsxSettings::GetSectionName() const
{
	return FName("Xlsx");
}
