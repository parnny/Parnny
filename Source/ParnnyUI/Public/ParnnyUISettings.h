// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ParnnyUISettings.generated.h"

/**
 * 
 */
UCLASS(config = Game, defaultconfig)
class PARNNYUI_API UParnnyUISettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(config, EditAnywhere, Category = "Test", Meta=(AllowAbstract=false))
	TSoftClassPtr<UUserWidget> TestWidget;
};
