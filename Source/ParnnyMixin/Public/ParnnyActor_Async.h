// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ControlFlowNode.h"
#include "ParnnyActor.h"
#include "ParnnyActor_Async.generated.h"

UCLASS()
class PARNNYMIXIN_API AParnnyActor_Async : public AParnnyActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AParnnyActor_Async();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Parnny|ControlFlow")
	void BasicFlowEntry();

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Parnny|ControlFlow")
	void BranchFlowEntry();

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Parnny|ControlFlow")
	void ForkFlowEntry();

protected:
	void QueueStep1(FControlFlowNodeRef SubFlow);
	void QueueStep2(FControlFlowNodeRef SubFlow);
	void QueueStep3(FControlFlowNodeRef SubFlow);
	void QueueStep4();
	
	void BranchStep1(FControlFlowNodeRef SubFlow);
	void BranchStep2(FControlFlowNodeRef SubFlow);
	
	void ForkStep1(FControlFlowNodeRef SubFlow);
	void ForkStep2(FControlFlowNodeRef SubFlow);
	void ForkStep3(FControlFlowNodeRef SubFlow);
};
