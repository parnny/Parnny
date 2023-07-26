// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "XlsxSubsystem.generated.h"

struct FFileChangeData;
/**
 * 
 */
UCLASS()
class PARNNYEXCEL_API UXlsxSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	void OnDirectoryChanged(const TArray<FFileChangeData>& InFileChanges);
	
	UPROPERTY()
	TObjectPtr<UTexture2D> XlsxIcon;

private:
	TPair<FString, FDelegateHandle> DirectoryWatched;

	FARFilter XlsxFilter;
};
