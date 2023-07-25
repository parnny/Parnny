// Fill out your copyright notice in the Description page of Project Settings.


#include "XlsxImportFactory.h"
#include "DataTableEditorUtils.h"
#include "DataTableXlsx.h"
#include "ParnnyLogChannels.h"
#include "SCSVImportOptions.h"
#include "SPrimaryButton.h"
#include "EditorFramework/AssetImportData.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "XlsxImportFactory"

UXlsxImportFactory::UXlsxImportFactory(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = UDataTable::StaticClass();
	
	bEditorImport = true;
	bText = false;

	--ImportPriority;
	
	Formats.Add(TEXT("xlsx;Excel File"));
}

FText UXlsxImportFactory::GetDisplayName() const
{
	return Super::GetDisplayName();
}

bool UXlsxImportFactory::FactoryCanImport(const FString& Filename)
{
	if (FPaths::GetExtension(Filename).Equals(TEXT("xlsx"), ESearchCase::IgnoreCase))
	{
		return true;
	}
	return false;
}

bool UXlsxImportFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (const UDataTable* DataTable = Cast<UDataTable>(Obj))
	{
		DataTable->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UXlsxImportFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	const UDataTable* DataTable = Cast<UDataTable>(Obj);
	if (DataTable && ensure(NewReimportPaths.Num() == 1))
	{
		DataTable->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UXlsxImportFactory::Reimport(UObject* Obj)
{
	if (UDataTable* DataTable = Cast<UDataTable>(Obj))
	{
		return ImportWithFile(DataTable, DataTable->AssetImportData->GetFirstFilename());
	}
	return EReimportResult::Failed;
}

EReimportResult::Type UXlsxImportFactory::ImportWithFile(UDataTable* DataTable, const FString& Path)
{
	bool bEditorOpened = false;
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem)
	{
		TArray<IAssetEditorInstance*> EditorInstances = AssetEditorSubsystem->FindEditorsForAsset(DataTable);
		for (IAssetEditorInstance* EditorInstance : EditorInstances)
		{
			EditorInstance->CloseWindow();
		}
		bEditorOpened = EditorInstances.Num() > 0;
	}
	xlnt::workbook Workbook;
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *Path, FILEREAD_AllowWrite))
	{
		return EReimportResult::Failed;
	}
	
	std::vector<std::uint8_t> CopyData;
	CopyData.reserve(FileData.Num());
	for (uint8 V : FileData)
	{
		CopyData.push_back(V);
	}

	if (!Workbook.load(CopyData))
	{
		return EReimportResult::Failed;
	}

	TArray<FString> Problems;
	FDataTableImporterXlsx DataTableImporter(&Workbook, Problems);
	DataTableImporter.ReadVars();
	DataTableImporter.ReadMeta();
	DataTableImporter.ReadData(DataTable);
	if (Problems.Num() == 0)
	{
		DataTable->AssetImportData->Update(Path);
		bool bDirty = DataTable->MarkPackageDirty();
		if (bEditorOpened && bDirty && AssetEditorSubsystem)
		{
			AssetEditorSubsystem->OpenEditorForAsset(DataTable);
		}
		return EReimportResult::Succeeded;
	}
	return EReimportResult::Failed;
}

UObject* UXlsxImportFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
                                               const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Parms);

	bOutOperationCanceled = false;
	TArray<FString> Problems;

	xlnt::workbook Workbook;

	ImportSettings.Filename = InName.ToString();

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *Filename, FILEREAD_AllowWrite))
	{
		return nullptr;
	}
	
	std::vector<std::uint8_t> CopyData;
	CopyData.reserve(FileData.Num());
	for (uint8 V : FileData)
	{
		CopyData.push_back(V);
	}

	if (!Workbook.load(CopyData))
	{
		return nullptr;
	}
	
	FDataTableImporterXlsx DataTableImporter(&Workbook, Problems);
	DataTableImporter.ReadVars();
	DataTableImporter.ReadMeta();

	UDataTable* ExistingTable = FindObject<UDataTable>(InParent, *InName.ToString());

	bool bDoImport = true;

	if (ExistingTable)
	{
		ImportSettings.ImportRowStruct = ExistingTable->RowStruct;
	}
	FString VarRowStruct = DataTableImporter.GetVar(TEXT("RowStruct"));
	if (!VarRowStruct.IsEmpty())
	{
		if (UScriptStruct* FoundStruct = FindObject<UScriptStruct>(nullptr, *VarRowStruct, true))
		{
			if (FoundStruct->IsChildOf(FTableRowBase::StaticStruct()))
			{
				ImportSettings.ImportRowStruct = FoundStruct;
			}
		}
	}

	if (IsAutomatedImport())
	{
		if (!ImportSettings.ImportRowStruct)
		{
			UE_LOG(LogParnnyExcel, Error, TEXT("A Data table row type must be specified in the import settings json file for automated import"));
			bDoImport = false;
		}
	}
	
	if (!ImportSettings.ImportRowStruct && !IsAutomatedImport())
	{
		bDoImport = false;
		OpenImportSettingsWindow(bDoImport);
	}

	UObject* NewAsset = nullptr;
	if (bDoImport)
	{
		// If there is an existing table, need to call this to free data memory before recreating object
		UDataTable::FOnDataTableChanged OldOnDataTableChanged;
		UClass* DataTableClass = UDataTable::StaticClass();
		if (ExistingTable)
		{
			OldOnDataTableChanged = MoveTemp(ExistingTable->OnDataTableChanged());
			ExistingTable->OnDataTableChanged().Clear();
			DataTableClass = ExistingTable->GetClass();
			ExistingTable->EmptyTable();
		}

		// Create/reset table
		UDataTable* NewTable = NewObject<UDataTable>(InParent, DataTableClass, InName, Flags);
		NewTable->RowStruct = ImportSettings.ImportRowStruct;
		if (!CurrentFilename.IsEmpty())
		{
			NewTable->AssetImportData->Update(CurrentFilename);
		}

		// Go ahead and create table from string
		DataTableImporter.ReadData(NewTable);

		// hook delegates back up and inform listeners of changes
		NewTable->OnDataTableChanged() = MoveTemp(OldOnDataTableChanged);
		NewTable->OnDataTableChanged().Broadcast();

		// Print out
		UE_LOG(LogParnnyExcel, Log, TEXT("Imported DataTable '%s' - %d Problems"), *InName.ToString(), Problems.Num());
		NewAsset = NewTable;
	}
	
	return NewAsset;
}

void UXlsxImportFactory::OpenImportSettingsWindow(bool& bImport)
{
	TSharedPtr<SWindow> ParentWindow;
	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		const IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		ParentWindow = MainFrame.GetParentWindow();
	}

	const TSharedPtr<SWindow> SettingsWindow = SNew(SWindow)
		.SizingRule(ESizingRule::Autosized)
		.Title(FText::FromString(TEXT("Export Settings")));
	
	const TSharedPtr<SWidget> RowStructCombo = FDataTableEditorUtils::MakeRowStructureComboBox(FDataTableEditorUtils::FOnDataTableStructSelected::CreateLambda([&](UScriptStruct* InStruct)
	{
		ImportSettings.ImportRowStruct = InStruct;
	}));
		
	SettingsWindow->SetContent
	(
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush(TEXT("Brushes.Panel")))
		.Padding(10)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.Padding(FMargin(3))
				.Visibility(EVisibility::HitTestInvisible)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Import_CurrentFileTitle", "Current File "))
					]
					+SHorizontalBox::Slot()
					.Padding(5, 0, 0, 0)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(ImportSettings.Filename))
					]
				]
			]
			
			// Data row struct
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(STextBlock)
				.Text( LOCTEXT("ChooseRowType", "Choose DataTable Row Type") )
				.Visibility( EVisibility::HitTestInvisible )
			]
			
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				RowStructCombo.ToSharedRef()
			]
			
			// Apply/Apply to All/Cancel
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.Padding(2)
				[
					SNew(SPrimaryButton)
					.Text(LOCTEXT("Import", "Apply"))
					.OnClicked_Lambda([&]
					{
						bImport = true;
						if (SettingsWindow)
							SettingsWindow->RequestDestroyWindow();
						return FReply::Handled();
					})
				]
				
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2)
				[
					SNew(SButton)
					.Text(LOCTEXT("Cancel", "Cancel"))
					.OnClicked_Lambda([&]
					{
						bImport = false;
						if (SettingsWindow)
							SettingsWindow->RequestDestroyWindow();
						return FReply::Handled();
					})
				]
			]
		]
	);
	FSlateApplication::Get().AddModalWindow(SettingsWindow.ToSharedRef(), ParentWindow, false);
}

#undef LOCTEXT_NAMESPACE