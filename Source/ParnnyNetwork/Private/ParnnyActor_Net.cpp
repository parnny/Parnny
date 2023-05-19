// Fill out your copyright notice in the Description page of Project Settings.


#include "ParnnyActor_Net.h"

#include "Net/UnrealNetwork.h"

DECLARE_LOG_CATEGORY_CLASS(LogParnnyNetwork, Log, All);

void AParnnyActor_Net::OnRep_BoughtJCW() const
{
	if (bHaveMiniJCW)
	{
		UE_LOG(LogParnnyNetwork, Display, TEXT("I have a Mini JCW!"))
	}
}

void AParnnyActor_Net::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 变化时同步
	DOREPLIFETIME(AParnnyActor_Net, PropInt);
	
	// DOREPLIFETIME_CONDITION(AParnnyActor_Net, PropVector, COND_SkipOwner);
	{
		const FProperty* ReplicatedProperty = GetReplicatedProperty(StaticClass(), StaticClass(),GET_MEMBER_NAME_CHECKED(AParnnyActor_Net,PropVector));
		FDoRepLifetimeParams Params;
		Params.Condition = COND_SkipOwner;
		Params.RepNotifyCondition = REPNOTIFY_OnChanged;
		RegisterReplicatedLifetimeProperty(ReplicatedProperty, OutLifetimeProps, Params);
	}

	// 根据条件同步
	{
		// This property will only attempt to send on the initial bunch
		DOREPLIFETIME_CONDITION(AParnnyActor_Net, PropStruct, COND_InitialOnly);
		// This property will only send to the actor's owner
		DOREPLIFETIME_CONDITION(AParnnyActor_Net, PropArray, COND_OwnerOnly);
	}
}