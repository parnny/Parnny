// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTableXlsx.h"

#include "ParnnyLogChannels.h"
#include "cell/cell.hpp"
#include "styles/fill.hpp"
#include "styles/style.hpp"
#include "workbook/workbook.hpp"
#include "worksheet/range.hpp"
#include "worksheet/worksheet.hpp"

FDataTableExporterXlsx::FDataTableExporterXlsx(TArrayView<uint8>& OutExportData)
	: ExportedData(OutExportData)
	, DataSheet(nullptr)
{
	Workbook = new xlnt::workbook();
}

FDataTableExporterXlsx::~FDataTableExporterXlsx()
{
	if (Workbook)
	{
		delete Workbook;
	}
}

bool FDataTableExporterXlsx::WriteTable(const UDataTable& InDataTable)
{
	if (!InDataTable.RowStruct)
	{
		return false;
	}

	TArray<xlnt::worksheet> DefaultSheets;
	for (int32 i=0; i < Workbook->sheet_count(); ++i)
	{
		xlnt::worksheet Worksheet = Workbook->sheet_by_index(i);
		DefaultSheets.Add(Worksheet);
	}
	
	{//Vars
		xlnt::worksheet Worksheet = Workbook->create_sheet_default(1);
		Worksheet.title("Vars");
		{
			Worksheet.cell(xlnt::cell_reference("A1")).value("RowStruct");
			Worksheet.cell(xlnt::cell_reference("B1")).value(TCHAR_TO_UTF8(*InDataTable.RowStruct->GetFullName()));
		}
		{
			Worksheet.cell(xlnt::cell_reference("A2")).value("ExportName");
			Worksheet.cell(xlnt::cell_reference("B2")).value(TCHAR_TO_UTF8(*InDataTable.GetName()));
		}
	}

	{//Meta
		xlnt::worksheet Worksheet = Workbook->create_sheet_default(2);
		Worksheet.title("Meta");
		xlnt::cell_reference CellRef(1,1);
		Worksheet.cell(xlnt::cell_reference("A1")).value("Field");
		for (int32 i=2; i<=5; ++i)
		{
			CellRef.column_index(xlnt::column_t(i));
			FString Prefix = i%2==0 ? "Key" : "Value";
			FString Value = FString::Printf(TEXT("%s%d"), *Prefix, i/2);
			Worksheet.cell(CellRef).value(TCHAR_TO_UTF8(*Value));
		}
	}
	
	{//Data
		xlnt::worksheet Worksheet = Workbook->create_sheet_default(0);
		Worksheet.title("Data");
		DataSheet = &Worksheet;
	}
	
	for (const xlnt::worksheet& Sheet : DefaultSheets)
	{
		Workbook->remove_sheet(Sheet);
	}

	xlnt::cell_reference HeadRef(1, 1);
	if (DataSheet)
	{
		xlnt::cell Cell = DataSheet->cell(HeadRef);
		Cell.value(TCHAR_TO_UTF8(*(TEXT("Table:")+InDataTable.GetName())));
	}
	
	constexpr int32 Col_Name = 2;
	if (DataSheet)
	{
		HeadRef.column_index(xlnt::column_t(Col_Name));
		xlnt::cell Cell = DataSheet->cell(HeadRef);
		Cell.value(TCHAR_TO_UTF8(TEXT("---")));
	}
	
	int32 Col_Header = Col_Name+1;
	HeaderMap.Empty();
	for (TFieldIterator<FProperty> It(InDataTable.RowStruct); It; ++It)
	{
		const FProperty* BaseProp = *It;
		check(BaseProp);
		FString Header = DataTableUtils::GetPropertyExportName(BaseProp);
		if (DataSheet)
		{
			HeadRef.column_index(xlnt::column_t(Col_Header));
			xlnt::cell Cell = DataSheet->cell(HeadRef);
			Cell.data_type(xlnt::cell::type::shared_string);
			Cell.value(TCHAR_TO_UTF8(*Header));
		}
		if (BaseProp->IsA<FNumericProperty>())
		{
			HeaderMap.Add(Header, {Col_Header, xlnt::cell::type::number});
		}
		else if(BaseProp->IsA<FBoolProperty>())
		{
			HeaderMap.Add(Header, {Col_Header, xlnt::cell::type::boolean});
		}
		else
		{
			HeaderMap.Add(Header, {Col_Header, xlnt::cell::type::shared_string});
		}
		++Col_Header;
	}
	
	int32 Cursor_Row = 2;
	xlnt::cell_reference NameRef(Col_Name, 1);
	for (auto RowIt = InDataTable.GetRowMap().CreateConstIterator(); RowIt; ++RowIt)
	{
		NameRef.row(Cursor_Row);
		FName RowName = RowIt.Key();
		uint8* RowData = RowIt.Value();
		if (DataSheet)
		{
			xlnt::cell Cell = DataSheet->cell(NameRef);
			if (FString(RowName.ToString()).IsNumeric())
			{
				Cell.data_type(xlnt::cell::type::number);
				Cell.value(FCString::Atoi(*RowName.ToString()));
			}
			else
			{
				Cell.data_type(xlnt::cell::type::shared_string);
				Cell.value(TCHAR_TO_UTF8(*RowName.ToString()));
			}
		}
		
		WriteRow(InDataTable.RowStruct, RowData, Cursor_Row);
		++Cursor_Row;
	}

	{	// draw some colors
		int32 RowH = DataSheet->highest_row();
		int32 ColL = DataSheet->lowest_column().index;
		int32 ColH = DataSheet->highest_column().index;

		// draw header
		for (int32 Col=ColL; Col <= ColH; ++Col)
		{
			xlnt::cell_reference CellRef(Col, 1);
			xlnt::cell Cell = DataSheet->cell(CellRef);
			xlnt::fill FillHeader = xlnt::fill::solid(xlnt::color(xlnt::rgb_color(200,150,200)));
			Cell.fill(FillHeader);
		}

		// draw comment
		for (int32 Row=2; Row <= RowH; ++Row)
		{
			xlnt::cell_reference CellRef(1, Row);
			xlnt::cell Cell = DataSheet->cell(CellRef);
			xlnt::fill FillComment = xlnt::fill::solid(xlnt::color(xlnt::rgb_color(255,230,153)));
			Cell.fill(FillComment);
		}
	}

	std::vector<uint8> OutExcelData;
	if (Workbook->save(OutExcelData))
	{
		ExportedData = TArrayView<uint8>(OutExcelData.data(), OutExcelData.size());
		return true;
	}
	return false;
}

bool FDataTableExporterXlsx::WriteRow(const UScriptStruct* InRowStruct, const void* InRowData, const int32& RowIndex)
{
	if (!InRowStruct)
	{
		return false;
	}
	xlnt::cell_reference CellRef(1, RowIndex);
	for (TFieldIterator<FProperty> It(InRowStruct); It; ++It)
	{
		FProperty* BaseProp = *It;
		check(BaseProp);
		if (FString Content = WriteStructEntry(InRowData, BaseProp); Content.IsEmpty() == false)
		{
			FString Header = DataTableUtils::GetPropertyExportName(BaseProp);
			const TPair<int32, xlnt::cell::type> HeaderInfo = HeaderMap.FindChecked(Header);
			CellRef.column_index(xlnt::column_t(HeaderInfo.Key));
			xlnt::cell Cell = DataSheet->cell(CellRef);
			Cell.data_type(HeaderInfo.Value);
			if (HeaderInfo.Value == xlnt::cell::type::number)
			{
				if (BaseProp->IsA<FIntProperty>())
				{
					Cell.value(FCString::Atoi(*Content));
				}
				else if (BaseProp->IsA<FInt64Property>())
				{
					Cell.value(FCString::Atoi64(*Content));
				}
				else if (BaseProp->IsA<FFloatProperty>())
				{
					Cell.value(FCString::Atof(*Content));
				}
				else if (BaseProp->IsA<FDoubleProperty>())
				{
					Cell.value(FCString::Atod(*Content));
				}
			}
			else if (HeaderInfo.Value == xlnt::cell::type::boolean)
			{
				Cell.value(Content.Equals(TEXT("true"), ESearchCase::IgnoreCase));
			}
			else
			{
				Cell.value(TCHAR_TO_UTF8(*Content));
			}
		}
	}

	return true;
}

FString FDataTableExporterXlsx::WriteStructEntry(const void* InRowData, FProperty* InProperty)
{
	return DataTableUtils::GetPropertyValueAsString(InProperty, (uint8*)InRowData, EDataTableExportFlags::None);
}

FDataTableImporterXlsx::FDataTableImporterXlsx(xlnt::workbook* InWorkbook, TArray<FString>& OutProblems)
	: ImportProblems(OutProblems)
	, Workbook(InWorkbook)
{
	if (Workbook)
	{
		WorksheetNames.Empty();
		for (std::string& SheetName : Workbook->sheet_titles())
		{
			WorksheetNames.AddUnique(FName(SheetName.c_str()));
		}
	}
}

FDataTableImporterXlsx::~FDataTableImporterXlsx()
{
}

void FDataTableImporterXlsx::ReadVars()
{
	if (WorksheetNames.Contains("Vars") == false)
	{
		return;
	}
	xlnt::worksheet RawSheet = Workbook->sheet_by_title("Vars");
	if (RawSheet.highest_column() == 0)
	{
		return;
	}
	
	for (int32 Row=RawSheet.lowest_row(); Row <= static_cast<int32>(RawSheet.highest_row()) ; ++Row)
	{
		xlnt::cell_reference RefKey(1, Row);
		xlnt::cell_reference RefValue(2, Row);
		
		if (RawSheet.has_cell(RefKey) && RawSheet.has_cell(RefValue))
		{
			xlnt::cell CellKey = RawSheet.cell( RefKey);
			FString VarKey = UTF8_TO_TCHAR(CellKey.to_string().c_str());
			FString VarValue = UTF8_TO_TCHAR(RawSheet.cell(2, Row).to_string().c_str());
			TableVars.Add(*VarKey, VarValue);
		}
	}
}

void FDataTableImporterXlsx::ReadMeta()
{
	if (WorksheetNames.Contains("Meta") == false)
	{
		return;
	}
	xlnt::worksheet RawSheet = Workbook->sheet_by_title("Meta");
	if (RawSheet.highest_column() == 0)
	{
		return;
	}
	for (int32 Row=RawSheet.lowest_row()+1; Row <= static_cast<int32>(RawSheet.highest_row()) ; ++Row)
	{
		xlnt::cell_reference Ref(1, Row);
		if (RawSheet.has_cell(Ref))
		{
			xlnt::cell Cell = RawSheet.cell( Ref);
			if (Cell.data_type() == xlnt::cell_type::shared_string || Cell.data_type() == xlnt::cell_type::inline_string)
			{
				FString FieldName = UTF8_TO_TCHAR(Cell.to_string().c_str());
				FXlsxFieldMeta& FieldMeta = TableFieldMeta.FindOrAdd(*FieldName);
				
				xlnt::cell_reference KeyRef(1, Row);
				xlnt::cell_reference ValueRef(1, Row);
				
				for (int32 Col=2; Col <= static_cast<int32>(RawSheet.highest_column().index); Col+=2)
				{
					KeyRef.column_index(xlnt::column_t(Col));
					ValueRef.column_index(xlnt::column_t(Col+1));
					xlnt::cell KeyCell = RawSheet.cell(KeyRef);
					xlnt::cell ValueCell = RawSheet.cell(ValueRef);
					FieldMeta.Meta.Add(
						UTF8_TO_TCHAR(KeyCell.to_string().c_str()),
						UTF8_TO_TCHAR(ValueCell.to_string().c_str())
						);
				}
			}
		}
	}
}

void FDataTableImporterXlsx::ReadData(UDataTable* DataTable)
{
	if (WorksheetNames.Contains("Data") == false)
	{
		return;
	}
	xlnt::worksheet RawSheet = Workbook->sheet_by_title("Data");

	const UScriptStruct* RowStruct = DataTable->GetRowStruct();
	if (!RowStruct) return;
	
	int32 RowL = RawSheet.lowest_row();
	int32 RowH = RawSheet.highest_row();
	int32 ColL = RawSheet.lowest_column().index;
	int32 ColH = RawSheet.highest_column().index;
	
	if (RowH == 0 || ColH == 0) return;
	
	TMap<FName, int32> FieldIndexMap;
	for (TFieldIterator<const FProperty> It(RowStruct, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		const FProperty* Prop = *It;
		FName PropName = Prop->GetFName();
		FieldIndexMap.Add(PropName, INDEX_NONE);
		if (const FXlsxFieldMeta* FieldMeta = TableFieldMeta.Find(PropName))
		{
			if (FieldMeta->HasMeta("Ref"))
			{
				FName RefSheetName = FieldMeta->GetMeta("Ref");
				TMap<FName, FString>& RefMap = RefSheetsMap.FindOrAdd(RefSheetName);
				ReadReferenceData(RefSheetName, RefMap);
			}
		}
	}
	
	int32 RowNameCol = -1;
	{
		xlnt::cell_reference FieldRef(1, 1);
		for (int32 Col=ColL; Col <= ColH; ++Col)
		{
			FieldRef.column_index(xlnt::column_t(Col));
			xlnt::cell Cell = RawSheet.cell(FieldRef);
			FName FieldName = UTF8_TO_TCHAR(Cell.to_string().c_str());
			if (FieldName.ToString().StartsWith(TEXT("#")))
			{
				continue;
			}
			FieldIndexMap.Add(FieldName, Col);
			if (FieldName == "---" || FieldName == "RowName")
			{
				RowNameCol = Col;
			}
		}
	}

	if (RowNameCol == -1)
	{
		return;
	}

	DataTable->EmptyTable();
	const TMap<FName, uint8*>& RowMap = DataTable->GetRowMap();
	uint8* RowData = static_cast<uint8*>(FMemory::Malloc(RowStruct->GetStructureSize()));
	
	bool bValidData = true;
	for (int32 Row=RowL+1; Row <= RowH ; ++Row)
	{
		{
			xlnt::cell_reference HeadRef(1, Row);
			xlnt::cell HeadCell = RawSheet.cell(HeadRef);
			FString Value = UTF8_TO_TCHAR(HeadCell.to_string().c_str());
			if (Value.StartsWith("#"))
			{
				continue;
			}
			if (Value.Equals(TEXT(">>>"), ESearchCase::IgnoreCase))
			{
				bValidData = true;
				continue;
			}

			if (Value.Equals(TEXT("<<<"), ESearchCase::IgnoreCase))
			{
				bValidData = false;
				continue;
			}
		}
		if (bValidData == false)
		{
			continue;
		}

		xlnt::cell_reference RowNameRef(RowNameCol, Row);
		xlnt::cell RowNameCell = RawSheet.cell(RowNameRef);
		FName RowName = (RowNameCell.data_type() == xlnt::cell_type::number) ? *FString::FromInt(RowNameCell.value<int>()) : UTF8_TO_TCHAR(RowNameCell.to_string().c_str());

		if (RowMap.Contains(RowName))
		{
			UE_LOG(LogParnnyExcel, Error, TEXT("Duplicate row name: %s found while enable DataTable mode. '%s'!"), *RowName.ToString(), *DataTable->GetPathName());
			continue;
		}
		
		RowStruct->InitializeStruct(RowData);
		
		for (TFieldIterator<const FProperty> It(RowStruct, EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			const FProperty* Prop = *It;
			FName PropName = Prop->GetFName();
			int32 Col = FieldIndexMap.FindRef(PropName);
			
			if (Col == INDEX_NONE) continue;
			
			FString Value;
			if (const FXlsxFieldMeta* FieldMeta = TableFieldMeta.Find(PropName))
			{
				if (FieldMeta->HasMeta("Ref"))
				{
					FName RefSheetName = FieldMeta->GetMeta("Ref");
					if (RefSheetsMap.Contains(RefSheetName))
					{
						if (const TMap<FName, FString>* RefMap = RefSheetsMap.Find(RefSheetName))
						{
							Value = RefMap->FindRef(RowName);
						}
					}
				}
			}
			
			FString CellValue;
			{
				xlnt::cell_reference CellRef(Col, Row);
				xlnt::cell Cell = RawSheet.cell(CellRef);
				CellValue = UTF8_TO_TCHAR(Cell.to_string().c_str());
				if (Value.StartsWith("#", ESearchCase::IgnoreCase))
				{
					continue;
				}
			}
			
			if (Value.IsEmpty() && Col != INDEX_NONE)
			{
				Value = CellValue;
			}
			
			FString ErrorStr = DataTableUtils::AssignStringToProperty(Value, Prop, RowData);
			if (ErrorStr.Len() > 0)
			{
				UE_LOG(LogParnnyExcel, Error, TEXT("Problem assigning string '%s' to property '%s' on row '%d' : %s"), *Value, *PropName.ToString(), Row, *ErrorStr);
			}
		}
		FTableRowBase* TableRowBase = reinterpret_cast<FTableRowBase*>(RowData);
		DataTable->AddRow(RowName, *TableRowBase);
	}
	FMemory::Free(RowData);
}

FString FDataTableImporterXlsx::GetVar(const FName VarName) const
{
	if (TableVars.Contains(VarName))
	{
		return TableVars.FindRef(VarName);
	}
	return FString();
}

bool FDataTableImporterXlsx::ReadReferenceData(const FName& RefSheetName, TMap<FName, FString>& OutRowMap) const
{
	if (WorksheetNames.Contains(TCHAR_TO_UTF8(*RefSheetName.ToString())) == false)
	{
		return false;
	}
	xlnt::worksheet RefSheet = Workbook->sheet_by_title(TCHAR_TO_UTF8(*RefSheetName.ToString()));
	int32 RowL = RefSheet.lowest_row();
	int32 RowH = RefSheet.highest_row();
	int32 ColL = RefSheet.lowest_column().index;
	int32 ColH = RefSheet.highest_column().index;
	
	if (RowH == 0 || ColH == 0)
	{
		return false;
	}

	TMap<FName, int32> FieldIndexMap;
	int32 RowNameCol = -1;
	{
		xlnt::cell_reference FieldRef(1, 1);
		for (int32 Col=ColL; Col <= ColH; ++Col)
		{
			FieldRef.column_index(xlnt::column_t(Col));
			xlnt::cell Cell = RefSheet.cell(FieldRef);
			FName FieldName = UTF8_TO_TCHAR(Cell.to_string().c_str());
			if (FieldName.ToString().StartsWith(TEXT("#")))
			{
				continue;
			}
			FieldIndexMap.Add(FieldName, Col);
			if (FieldName == "---" || FieldName == "RowName")
			{
				RowNameCol = Col;
			}
		}
	}

	if (RowNameCol == -1)
	{
		return false;
	}
	
	bool bValidData = true;
	for (int32 Row=RowL+1; Row <= RowH ; ++Row)
	{
		{
			xlnt::cell_reference HeadRef(1, Row);
			xlnt::cell HeadCell = RefSheet.cell(HeadRef);
			FString Value = UTF8_TO_TCHAR(HeadCell.to_string().c_str());
			if (Value.StartsWith("#"))
			{
				continue;
			}
			if (Value.Equals(TEXT(">>>"), ESearchCase::IgnoreCase))
			{
				bValidData = true;
				continue;
			}

			if (Value.Equals(TEXT("<<<"), ESearchCase::IgnoreCase))
			{
				bValidData = false;
				continue;
			}
			if (bValidData == false)
			{
				continue;
			}
		}

		xlnt::cell_reference RowNameRef(RowNameCol, Row);
		xlnt::cell RowNameCell = RefSheet.cell(RowNameRef);
		FName RowName = UTF8_TO_TCHAR(RowNameCell.to_string().c_str());

		if (OutRowMap.Contains(RowName))
		{
			continue;
		}

		FString TupleString;
		for(TTuple<FName, int>& Field : FieldIndexMap)
		{
			FName PropName = Field.Key;
			if (Field.Value == RowNameCol || Field.Key.IsNone() || Field.Value == INDEX_NONE) continue;
			
			xlnt::cell_reference CellRef(Field.Value, Row);
			xlnt::cell Cell = RefSheet.cell(CellRef);
			
			FString Value = UTF8_TO_TCHAR(Cell.to_string().c_str());
			
			if (TupleString.Len())
			{
				TupleString.AppendChar(',');
			}

			xlnt::cell_type CellType = Cell.data_type();
			if (CellType == xlnt::cell_type::number)
			{
				TupleString = TupleString.Append(FString::Printf(TEXT("%s=%s"), *PropName.ToString(), *Value));
			}
			else
			{
				TupleString = TupleString.Append(FString::Printf(TEXT("%s=\"%s\""), *PropName.ToString(), *Value));
			}
		}
		if (TupleString.Len())
		{
			OutRowMap.Add(RowName, FString::Printf(TEXT("(%s)"), *TupleString));
		}
	}
	return OutRowMap.Num() > 0;
}
