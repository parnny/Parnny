// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "WidgetWrapper.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class PARNNYUI_API UWidgetWrapper : public UObject
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable)
	bool LoadWidgetBlueprint(UUserWidgetBlueprint* InWidgetBlueprint);

	bool BindEvents();

	UFUNCTION(BlueprintPure)
	UUserWidget* GetWidgetInstance() const;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UUserWidget> WidgetInstance;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UUserWidgetBlueprint> WidgetBlueprint;
};
