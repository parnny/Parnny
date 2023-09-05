// // Fill out your copyright notice in the Description page of Project Settings.
//
//
// #include "WidgetWrapper.h"
//
// #include "ParnnyLogChannels.h"
// #include "Blueprint/UserWidget.h"
// #include "Blueprint/UserWidgetBlueprint.h"
// #include "Blueprint/WidgetTree.h"
//
// UE_DISABLE_OPTIMIZATION
//
// UWidgetWrapper::UWidgetWrapper(const FObjectInitializer& ObjectInitializer)
// 	: Super(ObjectInitializer)
// {
// 	
// }
//
// bool UWidgetWrapper::LoadWidgetBlueprint(UUserWidgetBlueprint* InWidgetBlueprint)
// {
// 	if (InWidgetBlueprint)
// 	{
// 		WidgetBlueprint = InWidgetBlueprint;
// 		WidgetInstance = NewObject<UUserWidget>(GetOuter(), WidgetBlueprint->GeneratedClass);
// 		WidgetInstance->Initialize();
// 		BindEvents();
// 		return true;
// 	}
// 	return false;
// }
//
// bool UWidgetWrapper::BindEvents()
// {
// 	if (const UWidgetTree* WidgetTree = WidgetInstance->WidgetTree)
// 	{
// 		const UClass* ThisClass = GetClass();
// 		WidgetTree->ForEachWidget([&](UWidget* Child) {
// 			if (const UClass* Class = Child->GetClass())
// 			{
// 				for (FProperty* BaseProp = Class->PropertyLink; BaseProp; BaseProp = BaseProp->PropertyLinkNext)
// 				{
// 					if (const FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(BaseProp))
// 					{
// 						FName Name = *FString::Printf(TEXT("%s%s"), *Child->GetName(), *DelegateProp->GetName());
// 						UE_LOG(LogParnnyUI, Display, TEXT("Binding %s"), *Name.ToString());
// 						if (ThisClass->FindFunctionByName(Name))
// 						{
// 							FScriptDelegate Delegate;
// 							Delegate.BindUFunction(this, Name);
// 							DelegateProp->AddDelegate(Delegate, Child);
// 						}
// 					}
// 				}
// 			}
// 		});
// 	}
// 	return true;
// }
//
// UUserWidget* UWidgetWrapper::GetWidgetInstance() const
// {
// 	return WidgetInstance.Get();
// }
//
// UE_ENABLE_OPTIMIZATION