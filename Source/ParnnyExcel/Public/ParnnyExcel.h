#pragma once

#include "CoreMinimal.h"
#include "XlsxDataSource.h"
#include "XlsxEditorEntry.h"
#include "Modules/ModuleManager.h"

class FParnnyExcelModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void RegisterContentBrowser();
    void RegisterEditorEntry();
    
    TStrongObjectPtr<UContentBrowserXlsxDataSource> XlsxFileDataSource;
    TStrongObjectPtr<UXlsxEditorEntry> XlsxEditorEntry;
};
