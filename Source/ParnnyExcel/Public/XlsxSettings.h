// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "XlsxSettings.generated.h"

/**
 * 
 */
UCLASS(config = Parnny, defaultconfig, DisplayName="Xlsx")
class PARNNYEXCEL_API UXlsxSettings : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()
	
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;

	UPROPERTY(Config, EditAnywhere, meta=(RelativeToGameContentDir, ConfigRestartRequired=true))
	FDirectoryPath XlsxSourcePath;
	
	UPROPERTY(Config, EditAnywhere, meta=(RelativeToGameContentDir, ConfigRestartRequired=true))
	FDirectoryPath XlsxExportPath;
};
