// Fill out your copyright notice in the Description page of Project Settings.


#include "XlsxSubsystem.h"

#include "ContentBrowserDataSubsystem.h"
#include "DirectoryWatcherModule.h"
#include "IDirectoryWatcher.h"
#include "XlsxImportFactory.h"
#include "XlsxSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorFramework/AssetImportData.h"

UE_DISABLE_OPTIMIZATION

void UXlsxSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	if (UContentBrowserDataSubsystem* Subsystem = Collection.InitializeDependency<UContentBrowserDataSubsystem>())
	{
		Subsystem->ActivateDataSource("XlsxData");
	}
	Super::Initialize(Collection);

	const FString FilePath = TEXT("/Script/Engine.Texture2D'/Game/Shared/Textures/XlsxIcon.XlsxIcon'");
	XlsxIcon = LoadObject<UTexture2D>(nullptr, *FilePath, nullptr, LOAD_None, nullptr);

	const UXlsxSettings* Settings = GetMutableDefault<UXlsxSettings>();
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>("DirectoryWatcher");
	if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
	{
		const FString WatchPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir(),Settings->XlsxSourcePath.Path);
		FDelegateHandle WatchHandle;
		DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(
			WatchPath,
			IDirectoryWatcher::FDirectoryChanged::CreateUObject(this, &UXlsxSubsystem::OnDirectoryChanged),
			WatchHandle);
		
		DirectoryWatched.Key = WatchPath;
		DirectoryWatched.Value = WatchHandle;
	}

	const FString ExportPath = FPaths::Combine(FString(TEXT("/Game")), Settings->XlsxExportPath.Path);
	XlsxFilter.bRecursiveClasses = true;
	XlsxFilter.ClassPaths.Add(UDataTable::StaticClass()->GetClassPathName());
	XlsxFilter.PackagePaths.Add(*ExportPath);
	XlsxFilter.bRecursivePaths = true;
}

void UXlsxSubsystem::Deinitialize()
{
	Super::Deinitialize();
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>("DirectoryWatcher");
	if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
	{
		DirectoryWatcher->UnregisterDirectoryChangedCallback_Handle(DirectoryWatched.Key, DirectoryWatched.Value);
	}
}


bool UXlsxSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		return Super::ShouldCreateSubsystem(Outer);
	}
	return false;
}

void UXlsxSubsystem::OnDirectoryChanged(const TArray<FFileChangeData>& InFileChanges)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> XlsxAssets;
	AssetRegistryModule.Get().GetAssets(XlsxFilter, XlsxAssets, true);
	TMap<FString, UDataTable*> DataTableMap;
	for (FAssetData& Data : XlsxAssets)
	{
		if (UDataTable* DataTable = Cast<UDataTable>(Data.GetAsset()))
		{
			FString Filename = DataTable->AssetImportData.Get()->GetFirstFilename();
			DataTableMap.Add(Filename, DataTable);
		}
	}

	TArray<UObject*> ReimportAssets;
	for (const FFileChangeData& ChangeData : InFileChanges)
	{
		if (ChangeData.Action == FFileChangeData::FCA_Modified)
		{
			if (DataTableMap.Contains(ChangeData.Filename))
			{
				if (UDataTable* DataTable = DataTableMap.FindRef(ChangeData.Filename))
				{
					ReimportAssets.AddUnique(DataTable);
				}
			}
		}
	}
	if (ReimportAssets.Num())
	{
		FReimportManager::Instance()->ValidateAllSourceFileAndReimport(ReimportAssets, true, INDEX_NONE, false);
	}
}

UE_ENABLE_OPTIMIZATION