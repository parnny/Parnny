// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "XlsxEditorEntry.generated.h"

/**
 * 
 */
UCLASS()
class PARNNYEXCEL_API UXlsxEditorEntry : public UObject
{
	GENERATED_BODY()
public:
	void RegisterFileMenus();
	void RegisterDirectoryMenus();

	TObjectPtr<UContentBrowserXlsxDataSource> XlsxFileDataSource;
	
protected:
	void OnExtendMenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths);
	TSharedRef<FExtender> GetExtender(const TArray<FString>& SelectedPaths);

	FDelegateHandle ContentBrowserExtenderHandle;
};
