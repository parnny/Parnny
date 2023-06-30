// Fill out your copyright notice in the Description page of Project Settings.

#include "ParnnyActor_Async.h"
#include "ControlFlow.h"
#include "ControlFlowBranch.h"
#include "ControlFlowConcurrency.h"
#include "ControlFlowManager.h"
#include "ParnnyLogChannels.h"

// Sets default values
AParnnyActor_Async::AParnnyActor_Async()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AParnnyActor_Async::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AParnnyActor_Async::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AParnnyActor_Async::BasicFlowEntry()
{
	FControlFlow& BasicFlow = FControlFlowStatics::Create(this, TEXT("BasicFlow"))
		.QueueStep(this, &ThisClass::QueueStep1)
		.QueueStep(this, &ThisClass::QueueStep2)
		.QueueStep(this, &ThisClass::QueueStep3)
		.QueueStep(this, &ThisClass::QueueStep4);
	BasicFlow.ExecuteFlow();
}

void AParnnyActor_Async::BranchFlowEntry()
{
	FControlFlow& BranchFlow = FControlFlowStatics::Create(this, TEXT("BranchFlow"));
	BranchFlow.BranchFlow([this](TSharedRef<FControlFlowBranch> Branch)
	{
		Branch->AddOrGetBranch(1).QueueStep(this, &ThisClass::BranchStep1);
		Branch->AddOrGetBranch(2).QueueStep(this, &ThisClass::BranchStep2);
		return 1;
	});
	BranchFlow.ExecuteFlow();
}

void AParnnyActor_Async::ForkFlowEntry()
{
	FControlFlow& ForkFlow = FControlFlowStatics::Create(this, TEXT("ForkFlow"));
	ForkFlow.ForkFlow([this](TSharedRef<FConcurrentControlFlows> Concurrent)
	{
		Concurrent->AddOrGetFlow(1).QueueStep(this, &ThisClass::ForkStep1);
		Concurrent->AddOrGetFlow(2).QueueStep(this, &ThisClass::ForkStep2);
		Concurrent->AddOrGetFlow(3).QueueStep(this, &ThisClass::ForkStep3);
	});
	ForkFlow.ExecuteFlow();
}

void AParnnyActor_Async::QueueStep1(FControlFlowNodeRef SubFlow)
{
	UE_LOG(LogParnnyMixin, Log, TEXT("%s: ContinueFlow"), ANSI_TO_TCHAR(__FUNCTION__));
	SubFlow->ContinueFlow();
}

void AParnnyActor_Async::QueueStep2(FControlFlowNodeRef SubFlow)
{
	UE_LOG(LogParnnyMixin, Log, TEXT("%s: ContinueFlow"), ANSI_TO_TCHAR(__FUNCTION__));
	SubFlow->ContinueFlow();
}

void AParnnyActor_Async::QueueStep3(FControlFlowNodeRef SubFlow)
{
	UE_LOG(LogParnnyMixin, Warning, TEXT("%s: CancelFlow"), ANSI_TO_TCHAR(__FUNCTION__));
	SubFlow->CancelFlow();
}

void AParnnyActor_Async::QueueStep4()
{
	check(0);
}

void AParnnyActor_Async::BranchStep1(FControlFlowNodeRef SubFlow)
{
	UE_LOG(LogParnnyMixin, Warning, TEXT("%s"), ANSI_TO_TCHAR(__FUNCTION__));
}

void AParnnyActor_Async::BranchStep2(FControlFlowNodeRef SubFlow)
{
	UE_LOG(LogParnnyMixin, Warning, TEXT("%s"), ANSI_TO_TCHAR(__FUNCTION__));
}

void AParnnyActor_Async::ForkStep1(FControlFlowNodeRef SubFlow)
{
	UE_LOG(LogParnnyMixin, Log, TEXT("%s: ContinueFlow"), ANSI_TO_TCHAR(__FUNCTION__));
	SubFlow->ContinueFlow();
}

void AParnnyActor_Async::ForkStep2(FControlFlowNodeRef SubFlow)
{
	UE_LOG(LogParnnyMixin, Warning, TEXT("%s: CancelFlow"), ANSI_TO_TCHAR(__FUNCTION__));
	SubFlow->CancelFlow();
}

void AParnnyActor_Async::ForkStep3(FControlFlowNodeRef SubFlow)
{
	UE_LOG(LogParnnyMixin, Warning, TEXT("%s: ContinueFlow"), ANSI_TO_TCHAR(__FUNCTION__));
	SubFlow->ContinueFlow();
}

