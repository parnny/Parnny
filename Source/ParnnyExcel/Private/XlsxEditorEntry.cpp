// Fill out your copyright notice in the Description page of Project Settings.


#include "XlsxEditorEntry.h"

#include "ContentBrowserDataMenuContexts.h"
#include "ContentBrowserFileDataCore.h"
#include "ContentBrowserFileDataPayload.h"
#include "ContentBrowserItem.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "DataTableXlsx.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "XlsxDataSource.h"
#include "XlsxSettings.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "UXlsxEditorEntry"

void UXlsxEditorEntry::RegisterFileMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.ItemContextMenu.XlsxData"))
	{
		Menu->AddDynamicSection(TEXT("DynamicSection_Xlsx"), FNewToolMenuDelegate::CreateLambda([&](UToolMenu* InMenu)
		{
			const UContentBrowserDataMenuContext_FileMenu* ContextObject = InMenu->FindContext<UContentBrowserDataMenuContext_FileMenu>();
			if (!XlsxFileDataSource) return;

			TArray<TSharedRef<const FContentBrowserFileItemDataPayload>> SelectedFiles;
			for (const FContentBrowserItem& SelectedItem : ContextObject->SelectedItems)
			{
				if (const FContentBrowserItemData* ItemDataPtr = SelectedItem.GetPrimaryInternalItem())
				{
					if (TSharedPtr<const FContentBrowserFileItemDataPayload> FilePayload = ContentBrowserFileData::GetFileItemPayload(XlsxFileDataSource.Get(), *ItemDataPtr))
					{
						SelectedFiles.Add(FilePayload.ToSharedRef());
					}
				}
			}
			
			if (SelectedFiles.Num() > 0)
			{
				// Run
				{
					FToolMenuSection& Section = InMenu->AddSection("Xlsx File", LOCTEXT("XlsxMenuHeading", "Xlsx File"));
					Section.InsertPosition.Position = EToolMenuInsertType::First;

					const FExecuteAction ExecuteAction_Open = FExecuteAction::CreateLambda([this, SelectedFiles]()
					{
						ContentBrowserFileData::EditFileItems(SelectedFiles);
					});

					const FExecuteAction ExecuteAction_Export = FExecuteAction::CreateLambda([this, SelectedFiles]()
					{
						TArray<FString> Files;
						FString DestinationPath;
						for (auto& SelectedFile : SelectedFiles)
						{
							Files.Add(SelectedFile->GetFilename());
						}
						if (const UXlsxSettings* XlsxSettings = GetDefault<UXlsxSettings>())
						{
							DestinationPath = FPaths::Combine(TEXT("/Game"), XlsxSettings->XlsxExportPath.Path);
						}
						GEditor->GetEditorSubsystem<UImportSubsystem>()->ImportNextTick(Files, DestinationPath);
					});
					
					Section.AddMenuEntry(
						"OpenXlsxFile",
						LOCTEXT("OpenXlsxFile", "Open..."),
						LOCTEXT("OpenXlsxFileToolTip", "Open Xlsx File."),
						FSlateIcon(),
						FUIAction(ExecuteAction_Open)
					);
					
					Section.AddMenuEntry(
						"ExportXlsxFile",
						LOCTEXT("ExportXlsxFile", "Export As DataTable ..."),
						LOCTEXT("ExportXlsxFileToolTip", "Export As DataTable"),
						FSlateIcon(),
						FUIAction(ExecuteAction_Export)
					);
				}
			}
		}));
	}
}

void UXlsxEditorEntry::OnExtendMenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths)
{
	TMap<FString, TArray<FString>> ImportFiles;
	FString GameFullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	const UXlsxSettings* Settings = GetDefault<UXlsxSettings>();
	const FString SourceFullPath = FPaths::Combine(GameFullPath, Settings->XlsxSourcePath.Path);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	for (FString SelectedPath : SelectedPaths)
	{
		if (SelectedPath.Contains(SelectedPath, ESearchCase::IgnoreCase))
		{
			FString FullPath = FPaths::Combine(GameFullPath, SelectedPath.Replace(TEXT("/Game/"), TEXT("")));
			PlatformFile.IterateDirectoryRecursively(*FullPath, [&](const TCHAR* FilenameOrDirectory, bool bIsDirectory) -> bool
			{
				if (!bIsDirectory)
				{
					const FString FilePath = FString(FilenameOrDirectory);
					const FString FileExtension = FPaths::GetExtension(FilePath);
					const FString FileName = FPaths::GetCleanFilename(FilePath);
					if (FileExtension.Equals(TEXT("xlsx"), ESearchCase::IgnoreCase) && !FileName.Contains("~"))
					{
						const FString FileDirectory = FPaths::GetPath(FilePath);
						const FString SufPath = FileDirectory.Replace(*SourceFullPath, TEXT(""));
						const FString DestinationPath = FPaths::Combine(TEXT("/Game"), Settings->XlsxExportPath.Path, SufPath);
						TArray<FString>& Files = ImportFiles.FindOrAdd(DestinationPath);
						Files.Add(FilePath);
					}
				}
				return true;
			});
		}
	}
	
	if (ImportFiles.Num())
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ExportAllXlsxFiles", "Export All Xlsx Files"),
			LOCTEXT("ExportAllXlsxFilesToopTip", "Export All Xlsx Files To Export Path"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([ImportFiles]()
				{
					for (TTuple<FString, TArray<FString>> Tuple : ImportFiles)
					{
						GEditor->GetEditorSubsystem<UImportSubsystem>()->ImportNextTick(Tuple.Value, Tuple.Key);
					}
				})
		));
	}
}

TSharedRef<FExtender> UXlsxEditorEntry::GetExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> Extender(new FExtender());
	Extender->AddMenuExtension(
		"FolderContext",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateUObject(this, &ThisClass::OnExtendMenu, SelectedPaths));
	return Extender;
}

void UXlsxEditorEntry::RegisterDirectoryMenus()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ExistExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	ExistExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateUObject(this, &ThisClass::GetExtender));
	ContentBrowserExtenderHandle = ExistExtenders.Last().GetHandle();
}

namespace XlsxEditorEntry
{
	void ExecuteExportAsXlsx(const FToolMenuContext& ToolMenuContext)
	{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

		const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

		const UContentBrowserAssetContextMenuContext* Context = UContentBrowserAssetContextMenuContext::FindContextWithAssets(ToolMenuContext);
		for (const UDataTable* DataTable : Context->LoadSelectedObjects<UDataTable>())
		{
			const FText Title = FText::Format(LOCTEXT("DataTable_ExportXlsxDialogTitle", "Export '{0}' as Xlsx..."), FText::FromString(*DataTable->GetName()));
			const FString CurrentFilename = DataTable->AssetImportData->GetFirstFilename();
			const FString FileTypes = TEXT("Excel(*.xlsx)|*.xlsx");

			TArray<FString> OutFilenames;
			DesktopPlatform->SaveFileDialog(
				ParentWindowWindowHandle,
				Title.ToString(),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetPath(CurrentFilename),
				(CurrentFilename.IsEmpty()) ? TEXT("") : FPaths::GetBaseFilename(CurrentFilename) + TEXT(".xlsx"),
				FileTypes,
				EFileDialogFlags::None,
				OutFilenames
			);

			if (OutFilenames.Num() > 0)
			{
				TArrayView<uint8> DataView;
				FDataTableExporterXlsx DataTableExporterXlsx(DataView);
				DataTableExporterXlsx.WriteTable(*DataTable);
				if (FFileHelper::SaveArrayToFile(DataView, *OutFilenames[0]))
				{
					FString Abs = FPaths::ConvertRelativePathToFull(OutFilenames[0]);
					FPlatformProcess::LaunchFileInDefaultExternalApplication(*Abs);
				}
			}
		}
	}
	static FDelayedAutoRegisterHelper DelayedAutoRegister_DataTable(EDelayedRegisterRunPhase::EndOfEngineInit, []{ 
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
		{
			FToolMenuOwnerScoped OwnerScoped(UE_MODULE_NAME);
			UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UDataTable::StaticClass());
		
			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				{
					const TAttribute<FText> Label = LOCTEXT("DataTable_ExportAsXlsx", "Export as Xlsx");
					const TAttribute<FText> ToolTip = LOCTEXT("DataTable_ExportAsXlsxTooltip", "Export the data table as a file containing Xlsx data.");
					const FSlateIcon Icon = FSlateIcon();
	
					FToolUIAction UIAction;
					UIAction.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&ExecuteExportAsXlsx);
					InSection.AddMenuEntry("DataTable_ExportAsCSV", Label, ToolTip, Icon, UIAction);
				}
			}));
		}));
	});
}

#undef LOCTEXT_NAMESPACE
