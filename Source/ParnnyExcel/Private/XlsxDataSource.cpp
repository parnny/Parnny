// Fill out your copyright notice in the Description page of Project Settings.


#include "XlsxDataSource.h"

#include "AssetViewUtils.h"
#include "XlsxSettings.h"
#include "XlsxSubsystem.h"

#define LOCTEXT_NAMESPACE "UContentBrowserXlsxDataSource"

PRAGMA_DISABLE_OPTIMIZATION

void UContentBrowserXlsxDataSource::Initialize()
{
	ContentBrowserFileData::FFileConfigData XlsxFileConfig;
	{
		auto XlsxItemPassesFilter = [](const FName InFilePath, const FString& InFilename, const FContentBrowserDataFilter& InFilter, const bool bIsFile)
		{
			const FContentBrowserDataPackageFilter* PackageFilter = InFilter.ExtraFilters.FindFilter<FContentBrowserDataPackageFilter>();
			if (PackageFilter && PackageFilter->PathPermissionList && PackageFilter->PathPermissionList->HasFiltering())
			{
				return PackageFilter->PathPermissionList->PassesStartsWithFilter(InFilePath, /*bAllowParentPaths*/!bIsFile);
			}
			return true;
		};

		auto GetXlsxItemAttribute = [](const FName InFilePath, const FString& InFilename, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue)
		{
			// TODO: Need to way to avoid all this ToString() churn (FPackageNameView?)

			if (InAttributeKey == ContentBrowserItemAttributes::ItemIsDeveloperContent)
			{
				const bool bIsDevelopersFolder = AssetViewUtils::IsDevelopersFolder(InFilePath.ToString());
				OutAttributeValue.SetValue(bIsDevelopersFolder);
				return true;
			}

			if (InAttributeKey == ContentBrowserItemAttributes::ItemIsLocalizedContent)
			{
				const bool bIsLocalizedFolder = FPackageName::IsLocalizedPackage(InFilePath.ToString());
				OutAttributeValue.SetValue(bIsLocalizedFolder);
				return true;
			}

			if (InAttributeKey == ContentBrowserItemAttributes::ItemIsEngineContent)
			{
				const bool bIsEngineFolder = AssetViewUtils::IsEngineFolder(InFilePath.ToString(), /*bIncludePlugins*/true);
				OutAttributeValue.SetValue(bIsEngineFolder);
				return true;
			}

			if (InAttributeKey == ContentBrowserItemAttributes::ItemIsProjectContent)
			{
				const bool bIsProjectFolder = AssetViewUtils::IsProjectFolder(InFilePath.ToString(), /*bIncludePlugins*/true);
				OutAttributeValue.SetValue(bIsProjectFolder);
				return true;
			}

			if (InAttributeKey == ContentBrowserItemAttributes::ItemIsPluginContent)
			{
				const bool bIsPluginFolder = AssetViewUtils::IsPluginFolder(InFilePath.ToString());
				OutAttributeValue.SetValue(bIsPluginFolder);
				return true;
			}

			return false;
		};

		auto XlsxItemPreview = [this](const FName InFilePath, const FString& InFilename)
		{
			return true;
		};

		auto XlsxItemCanEditor = [this](const FName InFilePath, const FString& InFilename, FText* OutErrorMsg)
		{
			const FString CleanName = FPaths::GetCleanFilename(InFilename);
			if (CleanName.StartsWith(TEXT("~$")))
			{
				if (OutErrorMsg)
				{
					*OutErrorMsg = LOCTEXT("CanEditTempXlsxFile", "Can not edit temp xlsx file!!!");
				}
				return false;
			}
			return true;
		};

		ContentBrowserFileData::FDirectoryActions XlsxDirectoryActions;
		XlsxDirectoryActions.PassesFilter.BindLambda(XlsxItemPassesFilter, false);
		XlsxDirectoryActions.GetAttribute.BindLambda(GetXlsxItemAttribute);
		XlsxFileConfig.SetDirectoryActions(XlsxDirectoryActions);
		
		ContentBrowserFileData::FFileActions XlsxFileActions;
		XlsxFileActions.TypeExtension = TEXT("xlsx");
		XlsxFileActions.TypeName = FTopLevelAssetPath(TEXT("/Script/Xlsx.Xlsx")); // Fake path to satisfy FFileActions requirements
		XlsxFileActions.TypeDisplayName = LOCTEXT("XlsxTypeName", "Xlsx");
		XlsxFileActions.TypeShortDescription = LOCTEXT("XlsxTypeShortDescription", "Excel Xlsx File");
		XlsxFileActions.TypeFullDescription = LOCTEXT("XlsxTypeFullDescription", "Excel Xlsx File");
		XlsxFileActions.DefaultNewFileName = TEXT("new_xlsx");
		XlsxFileActions.TypeColor = FColor::FromHex("217346FF");
		XlsxFileActions.PassesFilter.BindLambda(XlsxItemPassesFilter, true);
		XlsxFileActions.GetAttribute.BindLambda(GetXlsxItemAttribute);
		XlsxFileActions.Preview.BindLambda(XlsxItemPreview);
		XlsxFileActions.CanEdit.BindLambda(XlsxItemCanEditor);
		XlsxFileConfig.RegisterFileActions(XlsxFileActions);
	}
	Super::Initialize(XlsxFileConfig);

	const UXlsxSettings* XlsxSettings = GetDefault<UXlsxSettings>();
	if (XlsxSettings->XlsxSourcePath.Path.IsEmpty())
	{
		TArray<FString> RootPaths;
		FPackageName::QueryRootContentPaths(RootPaths);
		for (const FString& RootPath : RootPaths)
		{
			const FString RootFilesystemPath = FPackageName::LongPackageNameToFilename(RootPath);
			AddFileMount(*(RootPath / TEXT("Xlsx")), RootFilesystemPath / TEXT("Xlsx"));
		}
	}
	else
	{
		FString RootPath = FPaths::Combine(TEXT("/Game"),XlsxSettings->XlsxSourcePath.Path);
		const FString RootFilesystemPath = FPackageName::LongPackageNameToFilename(RootPath);
		AddFileMount(*RootPath, RootFilesystemPath);
	}
}

bool UContentBrowserXlsxDataSource::UpdateThumbnail(const FContentBrowserItemData& InItem, FAssetThumbnail& InThumbnail)
{
	if (InItem.GetItemName().ToString().StartsWith(TEXT("~$")))
	{
		return Super::UpdateThumbnail(InItem, InThumbnail);
	}
	if (const UXlsxSubsystem* XlsxSubsystem = GEditor->GetEditorSubsystem<UXlsxSubsystem>())
	{
		if (XlsxSubsystem->XlsxIcon)
		{
			InThumbnail.SetAsset(XlsxSubsystem->XlsxIcon);
			return true;
		}
	}
	return Super::UpdateThumbnail(InItem, InThumbnail);
}

bool UContentBrowserXlsxDataSource::CanEditItem(const FContentBrowserItemData& InItem, FText* OutErrorMsg)
{
	if (InItem.GetItemName().ToString().StartsWith(TEXT("~$")))
	{
		if (OutErrorMsg)
		{
			*OutErrorMsg = LOCTEXT("CanEditTempXlsxFile", "Can not edit temp xlsx file!!!");
		}
		return false;
	}
	return Super::CanEditItem(InItem, OutErrorMsg);
}

bool UContentBrowserXlsxDataSource::EditItem(const FContentBrowserItemData& InItem)
{
	if (InItem.GetItemName().ToString().StartsWith(TEXT("~$")))
	{
		return false;
	}
	return Super::EditItem(InItem);
}

#undef LOCTEXT_NAMESPACE

PRAGMA_ENABLE_OPTIMIZATION