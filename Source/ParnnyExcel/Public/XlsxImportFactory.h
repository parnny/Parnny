// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorReimportHandler.h"
#include "XlsxDefines.h"
#include "Factories/Factory.h"
#include "XlsxImportFactory.generated.h"

/**
 * 
 */
UCLASS()
class PARNNYEXCEL_API UXlsxImportFactory : public UFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

	//~ Begin UFactory Interface
	virtual FText GetDisplayName() const override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual bool CanReimport( UObject* Obj, TArray<FString>& OutFilenames ) override;
	virtual void SetReimportPaths( UObject* Obj, const TArray<FString>& NewReimportPaths ) override;
	virtual EReimportResult::Type Reimport( UObject* Obj ) override;
	static EReimportResult::Type ImportWithFile(UDataTable* DataTable, const FString& Path);
	
	virtual UObject* FactoryCreateFile(UClass *InClass, UObject *InParent, FName InName, EObjectFlags Flags, const FString &Filename, const TCHAR *Parms, FFeedbackContext *Warn, bool &bOutOperationCanceled) override;

protected:
	void OpenImportSettingsWindow(bool& bImport);
	
	FXlsxImportSettings ImportSettings;
	
	UPROPERTY()
	TObjectPtr<UDataTable> DataTableImportOptions;
};
