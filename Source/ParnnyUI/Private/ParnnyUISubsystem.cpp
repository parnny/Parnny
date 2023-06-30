// Fill out your copyright notice in the Description page of Project Settings.


#include "ParnnyUISubsystem.h"
#include "CommonInputSubsystem.h"
#include "ParnnyLogChannels.h"
#include "Blueprint/SlateBlueprintLibrary.h"

void UParnnyUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UCommonInputSubsystem* InputSubsystem = Collection.InitializeDependency<UCommonInputSubsystem>();
	InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &UParnnyUISubsystem::OnInputMethodChanged);
	Super::Initialize(Collection);
	FSlateApplication::Get().OnFocusChanging().AddUObject(this,&UParnnyUISubsystem::OnFocusChanging);
}

void UParnnyUISubsystem::OnInputMethodChanged(ECommonInputType CommonInput)
{
	CurrentInputType = CommonInput;
}

void UParnnyUISubsystem::OnFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldPath,
                                         const TSharedPtr<SWidget>& OldWidget, const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewWidget)
{
	if (NewWidget)
	{
		if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
			{
				const FGeometry& Geometry = NewWidget->GetTickSpaceGeometry();
				const FVector2d Local = Geometry.LocalToAbsolute(FVector2D(0.5));

				FVector2D PixelPosition, ViewportPosition;
				USlateBlueprintLibrary::AbsoluteToViewport(PC, Local, PixelPosition, ViewportPosition);
				UE_LOG(LogParnnyUI, Log, TEXT("OnFocusChanging: PixelPosition:%s, ViewportPosition:%s"), *PixelPosition.ToString(), *ViewportPosition.ToString());
				int32 ViewX,ViewY;
				PC->GetViewportSize(ViewX,ViewY);
				PC->SetMouseLocation(PixelPosition.X, PixelPosition.Y);
				PC->SetShowMouseCursor(true);
			}
		}
	}
}
