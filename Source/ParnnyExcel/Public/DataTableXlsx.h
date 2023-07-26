// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XlsxDefines.h"
#include "cell/cell.hpp"

class PARNNYEXCEL_API FDataTableExporterXlsx
{
public:
	FDataTableExporterXlsx(TArrayView<uint8>& OutExportData);
	~FDataTableExporterXlsx();
	
	bool WriteTable(const UDataTable& InDataTable);

	bool WriteRow(const UScriptStruct* InRowStruct, const void* InRowData, const int32& RowIndex);

private:
	FString WriteStructEntry(const void* InRowData, FProperty* InProperty);
	
	TMap<FString,TPair<int32, xlnt::cell::type>> HeaderMap;
	TArrayView<uint8>& ExportedData;
	xlnt::workbook* Workbook;
	xlnt::worksheet* DataSheet;
};


class PARNNYEXCEL_API FDataTableImporterXlsx
{
public:
	FDataTableImporterXlsx(xlnt::workbook* InWorkbook, TArray<FString>& OutProblems);
	~FDataTableImporterXlsx();
	
	void ReadVars();
	void ReadMeta();
	void ReadData(UDataTable* DataTable);

	FString GetVar(FName VarName) const;
	
private:
	bool ReadReferenceData(const FName& RefSheetName, TMap<FName, FString>& OutRowMap) const;
	
	TArray<FString>& ImportProblems;
	xlnt::workbook* Workbook;
	TMap<FName, FXlsxFieldMeta> TableFieldMeta;
	TMap<FName, FString> TableVars;
	TMap<FName, TMap<FName, FString>> RefSheetsMap;
	TArray<FName> WorksheetNames;
};
