#include "ParnnyExcel.h"

#include "XlsxDataSource.h"

#define LOCTEXT_NAMESPACE "FParnnyExcelModule"

UE_DISABLE_OPTIMIZATION

void FParnnyExcelModule::StartupModule()
{
	RegisterContentBrowser();
	RegisterEditorEntry();
}

void FParnnyExcelModule::ShutdownModule()
{
    
}

void FParnnyExcelModule::RegisterContentBrowser()
{
#if WITH_EDITOR
	if (GIsEditor && !IsRunningCommandlet())
	{
		XlsxFileDataSource.Reset(NewObject<UContentBrowserXlsxDataSource>(GetTransientPackage(), "XlsxData"));
		XlsxFileDataSource->Initialize();
	}
#endif
}

void FParnnyExcelModule::RegisterEditorEntry()
{
	if (XlsxFileDataSource)
	{
		XlsxEditorEntry.Reset(NewObject<UXlsxEditorEntry>(GetTransientPackage(), "XlsxEntry"));
		XlsxEditorEntry->XlsxFileDataSource = XlsxFileDataSource.Get();
		XlsxEditorEntry->RegisterFileMenus();
		XlsxEditorEntry->RegisterDirectoryMenus();
	}
}

IMPLEMENT_MODULE(FParnnyExcelModule, ParnnyExcel)

UE_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE