// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserFileDataSource.h"
#include "XlsxDataSource.generated.h"

/**
 * 
 */
UCLASS()
class PARNNYEXCEL_API UContentBrowserXlsxDataSource : public UContentBrowserFileDataSource
{
	GENERATED_BODY()

public:
	void Initialize();
	
	virtual bool UpdateThumbnail(const FContentBrowserItemData& InItem, FAssetThumbnail& InThumbnail) override;
};

