// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParnnyCore/Public/ParnnyActor.h"
#include "ParnnyActor_Net.generated.h"

USTRUCT()
struct FMiniJCW
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName Engine = "B48";
	
	FORCEINLINE bool operator==(FMiniJCW const& Other) const
	{
		return Engine == Other.Engine;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FMiniJCW> : TStructOpsTypeTraitsBase2<FMiniJCW>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
		WithIdenticalViaEquality = true,
	};
};


UCLASS()
class PARNNYNETWORK_API AParnnyActor_Net : public AParnnyActor
{
	GENERATED_BODY()
public:
	UPROPERTY(Replicated)
	int32 PropInt;
	
	UPROPERTY(Replicated)
	FRepAttachment  PropStruct;

	UPROPERTY(Replicated)
	TArray<int32> PropArray;

	UPROPERTY(Replicated)
	FVector PropVector;

	UPROPERTY(ReplicatedUsing=OnRep_BoughtJCW)
	bool bHaveMiniJCW;

	// UHT会自动调用这个函数，用于设置同步策略
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	UFUNCTION(Reliable, Client)
	void RPCClient();

	UFUNCTION(Reliable, Server, WithValidation)
	void RPCServerWithValidation();

	UFUNCTION(Reliable, NetMulticast)
	void RPCMulticast();
	
protected:
	UFUNCTION()
	void OnRep_BoughtJCW() const;
};
