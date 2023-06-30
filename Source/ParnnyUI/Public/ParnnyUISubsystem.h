// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonInputTypeEnum.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ParnnyUISubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PARNNYUI_API UParnnyUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	void OnInputMethodChanged(ECommonInputType CommonInput);
	void OnFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldPath, const TSharedPtr<SWidget>& OldWidget, const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewWidget);

	ECommonInputType CurrentInputType;
};
