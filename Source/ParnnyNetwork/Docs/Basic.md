# Basic 

## ParnnyNote
> UE的属性同步依赖反射系统，
> UE属性同步以Actor为单位，每个Actor有一个对应的ActorChannel，
> 这个Channel主要负责与Actor实例相关的内容，包括属性的同步与变动历史，相对应的ShadowBuffer
> 每个UClass对应一个RepLayout，
> RepLayout中记录了Class中所有需要同步的属性，每个属性对应一个FRepParentCmd，
> 简单属性会对应一个RepLayoutCmd，复杂属性会对应多个RepLayoutCmd。
> 

## Reference
* [服务器同步属性](https://zhuanlan.zhihu.com/p/412517987)
* [客户端接收属性同步数据](https://zhuanlan.zhihu.com/p/587136954)
* [简介](https://www.youtube.com/watch?v=JOJP0CvpB8w)

## Enums
### ENetFields
    
## Functions
### GetLifetimeReplicatedProps 
### RegisterReplicatedLifetimeProperty

## Classes
### FRepLayout
> * 实现实现属性同步,有管理属性历史值，比较属性，发送属性数据等功能
> * ***FRepLayout与UClass一一对应***
> 
#### 创建方法
``` C++
/** Creates a new FRepLayout for the given class. */
static TSharedPtr<FRepLayout> CreateFromClass(UClass* , UNetConnection* , ECreateRepLayoutFlags);

/** Creates a new FRepLayout for the given struct. */
static TSharedPtr<FRepLayout> CreateFromStruct(UStruct * , UNetConnection* , ECreateRepLayoutFlags);

/** Creates a new FRepLayout for the given function. */
static TSharedPtr<FRepLayout> CreateFromFunction(UFunction* , UNetConnection* , ECreateRepLayoutFlags);
```

#### 关键字段
```c++
// 创建一个Shadow buffer实例需要的内存
int32 ShadowDataBufferSize; 

// 顶层的RepLayoutCommand
TArray<FRepParentCmd> Parents; 

// 所有的RepLayoutCommand
TArray<FRepLayoutCmd> Cmds; 

// 将相对handle转换为Cmds数组中的索引,Handle到Command的映射关系
TArray<FHandleToCmdIndex> BaseHandleToCmdIndex;  
```

#### InitFromClass
> * 执行InitFromClass函数时，还没有创建ShadowBuffer
```c++
void InitFromClass(UClass* InObjectClass, const UNetConnection* ServerConnection, const ECreateRepLayoutFlags CreateFlags)
{
    // ...
    // Initializes the ClassReps and NetFields arrays used by replication.
    InObjectClass->SetUpRuntimeReplicationData();
    // ...
    // 循环遍历ClassReps，创建FRepParentCmd
    for (int32 i = 0; i < InObjectClass->ClassReps.Num(); i++)
    {
        // ...
        // 在Parents中添加一条RepParentCmd记录，并记下下标ParentHandle
        const int32 ParentHandle = AddParentProperty(Parents, Property, ArrayIdx);
        // ...
        // 区分对待简单Property和复杂Property
        RelativeHandle = InitFromProperty_r<ERepBuildType::Class>(SharedParams, StackParams);
    }
    // ...
    // 添加ReturnCmd，表述该Class的Property处理结束
    AddReturnCmd(Cmds);
    // ...
    // 通过CDO，得到FLifetimeProperty数组，然后用它来初始化Parents数组，设置Condition，RepNotifyCondition，RepNotifyNumParams等属性。
    UObject* Object = InObjectClass->GetDefaultObject();
    Object->GetLifetimeReplicatedProps(LifetimeProps);
	
	// ...
	if (bIsObjectActor)
	{
	    /*
	    对于Actor.RemoteRole属性，需要特殊处理，RemoteRole虽然会同步，但server和client的值不同。
	    此时会把该属性在Parents数组中对应对应的RepParentCmd.Condition设成COND_None，并设置ERepParentFlags::IsConditional标记。
	    */
    }
    
    // ...
    // 执行BuildHandleToCmdIndexTable_r函数，建立Handle到Cmd数组的映射，主要因为Array需要特殊处理。
    if (!ServerConnection || EnumHasAnyFlags(CreateFlags, ECreateRepLayoutFlags::MaySendProperties))
    {
        BuildHandleToCmdIndexTable_r(0, Cmds.Num() - 1, BaseHandleToCmdIndex);
    }
    // 记录偏移
	// !!!!!!!!!! 此时没有创建ShadowBuffer !!!!!!!!!!
	BuildShadowOffsets<ERepBuildType::Class>(InObjectClass, Parents, Cmds, ShadowDataBufferSize);
}
```


### FRepParentCmd
> + 确定Property如何同步
> + **一个FRepParentCmd对应一个Property**
> + **简单Property会对应一个FRepLayoutCmd**
>   + FIntProperty
>   + FFloatProperty
>   + ...
> + 复杂Property会对应多个FRepLayoutCmd
>   + FArrayProperty
>   + FStructProperty
>   + FMapProperty
>   + ...
>
#### 关键字段
```c++
// 对应的Property
FProperty* Property; 
 // 如果Property是一个C-Style固定大小的数组，则为数组中每个元素创建一个命令。这是命令表示的数组中的元素的索引。对于非数组属性，这将始终为0。
int32 ArrayIndex; 

// Property在Object中的偏移量
int32 Offset;

// Shadow Memory中的偏移量
int32 ShadowOffset; 

 // CmdStart和CmdEnd定义与此父命令关联的FRepLayoutCommands的范围（通过FRepLayouts Cmd数组中的索引）。
uint16 CmdStart;
uint16 CmdEnd;
```
#### 例
> int32 PropInt;
> * FRepParentCmd * 1
> * FRepLayoutCmd * 1
> 
> TArray<int32> PropArray;
> * FRepParentCmd * 1
> * FRepLayoutCmd * N
> 

### FRepLayoutCmd
> + 指导单一元素如何同步，可以是一个普通的顶层Property，也可以是UStruct中的嵌套Property，或者TArray中的元素
#### 关键字段
```c++
// 对应的底层Property，如果是UStruct，对应了UStruct的各个子Property
FProperty* Property;

// 如果Property是一个数组，则此属性将是跳过数组内部元素的命令索引。
uint16 EndCmd;

// 属性的大小
// 如果Property是一个数组，则此属性将是跳过数组内部元素的命令索引。
uint16 ElementSize;

// Property在Object中的偏移量
int32 Offset;

// Shadow Memory中的偏移量
int32 ShadowOffset;

// 相对于数组或顶层列表开头的句柄
uint16 RelativeHandle;

// 父命令的索引
uint16 ParentIndex;

// 用于确定属性是否仍然兼容
uint32 CompatibleChecksum;

// 数据类型
ERepLayoutCmdType Type;

ERepLayoutCmdFlags Flags;
```
#### 特殊
##### **NetDeltaSerialize**
> 对于自己实现了NetDeltaSerialize函数的UStruct，不会生成子Cmd，因为UE不需要处理该属性的内存结构，UStruct自己决定属性何时改变，如何网络同步。

##### **NetSerialize**
> 对于自己实现了NetSerialize函数的UStruct，只会生成一个子Cmd，UE虽然不关心UStruct内部的内存布局，但需要知道内存所在位置，这样才能做diff。
> 

### ShadowBuffer
> * 当初次同步一个Actor实例时，需要为其创建ShadowBuffer
> * 嵌套关系
> ```c++
> UActorChannel 
> - TSharedPtr<FObjectReplicator> ActorReplicator; //负责传输Actor及Component的属性和RPC，创建一个Actor连接并维护连接。
>   - FObjectReplicator // 真正负责处理属性同步和rpc
>       - TSharedPtr<class FReplicationChangelistMgr> ChangelistMgr // 维护了属性的变动历史，用于确定要属性同步要发送的内容
>           - FRepChangelistState RepChangelistState // 实际存储属性变动历史
>               - FRepStateStaticBuffer StaticBuffer // ShadowBuffer
> ```

## **UNetDriver**
> 创建堆栈
```c++
UWorld::Listen(FURL &)
UEngine::CreateNamedNetDriver(UWorld *,FName,FName)
CreateNamedNetDriver_Local(UEngine *,FWorldContext &,FName,FName)
CreateNetDriver_Local(UEngine *,FWorldContext &,FName)
UIpNetDriver::UIpNetDriver(const FObjectInitializer &)
UNetDriver::UNetDriver(const FObjectInitializer &)
```
### **Tick**
> NetDriver的Tick函数是注册在World里
 ```c++
// 注册Tick函数
ENGINE_API void RegisterTickEvents(class UWorld* InWorld);
// 收取处理数据包
ENGINE_API virtual void TickDispatch( float DeltaTime );
// 发送数据包
ENGINE_API virtual void TickFlush(float DeltaSeconds);
```
## 服务器同步属性
### TickFlush && ServerReplicateActors
```c++
void UNetDriver::TickFlush(float DeltaSeconds)
{
    // ...
    if (IsServer() && ClientConnections.Num() > 0 && !bSkipServerReplicateActors)
    {
        // Update all clients.
#if WITH_SERVER_CODE
        int32 Updated = ServerReplicateActors( DeltaSeconds )
        {
            // ...
            // 遍历所有链接
            for ( int32 i=0; i < ClientConnections.Num(); i++ )
            {
                // ...
                // 根据当前的ViewTarget，筛选需要更同步Actor
                if (Connection->ViewTarget)
                {
                    // Get a sorted list of actors for this connection
                    // 根据当前的ViewTarget，对需要更新的Actor进行排序
                    const int32 FinalSortedCount = ServerReplicateActors_PrioritizeActors(...);
                    // 根据排序后的Actor列表，对Actor进行同步
                    const int32 LastProcessedActor = ServerReplicateActors_ProcessPrioritizedActors(...)
                    {
                        // ...
                        UActorChannel* Channel = PriorityActors[j]->Channel;
                        // ...
                        // 同步Actor属性
                        Channel->ReplicateActor()
                        {
                            // ...
                            ActorReplicator->ReplicateProperties(Bunch, RepFlags); // 同步当前Actor的属性
                            Actor->ReplicateSubobjects(this, &Bunch, &RepFlags);    // 同步Subobject的属性
                        }
                    }
                    // ...
                }
            }
        }
        //...
#endif
    }
}
```

## **对比属性**

### FReplicationChangelistMgr
> 管理自对象开始复制以来发生的特定复制对象的更改列表
#### 关键字段
```c++
FRepChangelistState RepChangelistState;
```

### FRepChangelistState
> 存储对象的变更列表历史
> 
#### 描述
> * **ChangeList为环形Buffer，在数量上有限制，超过数量限制时，会将最早的ChangeList合并，所以数据不会丢弃**
> * MAX_CHANGE_HISTORY = 32

#### 关键字段
```c++
/** Circular buffer of changelists. */
FRepChangedHistory ChangeHistory[MAX_CHANGE_HISTORY];
```

### FRepChangedHistory
> 单个更改列表，跟踪更改的属性
#### 关键字段
```c++
// 该Changelist中发生变化的属性列表
TArray<uint16> Changed;
// 是否需要重发
bool Resend;
```

#### CompareProperties
```c++
bool FObjectReplicator::ReplicateProperties( FOutBunch & Bunch, FReplicationFlags RepFlags )
{
    // ...
    // 更新变化列表
    RepLayout.UpdateChangelistMgr(...)
    {
        // 对比属性
        ERepLayoutResult Result = CompareProperties(...);
    }
    // 属性同步
    // FSendingRepState 发送属性的状态
    // FRepChangelistState 属性变化的历史
    RepLayout->ReplicateProperties( FSendingRepState, FRepChangelistState);
    // ...
    // 如果列表满了，合并最早的Changelist
    if ((RepChangelistState->HistoryEnd - RepChangelistState->HistoryStart) == FRepChangelistState::MAX_CHANGE_HISTORY)
	{
	    // ...
        MergeChangeList(...)
	}
}
```

## **发送属性同步数据**
> 生成Changed数据是对于Actor的, 主要使用 **FRepChangelistState** \
    而发送数据是对于 **ClientConnection** 的, 主要使用 **FSendingRepState**

### FSendingRepState
> 需要同步属性时才用到的状态 \
> 每个ClientConnection都有一个
#### 关键字段
```c++
// 需要发送的变化列表
FRepChangedHistory ChangeHistory[MAX_CHANGE_HISTORY];
```

> 
### 关键方法
```c++
bool FRepLayout::ReplicateProperties(...)
{
    // ...
    SendProperties(...)
    {
        // ...
        while (HandleIterator.NextHandle())
	    {
		    const FRepLayoutCmd& Cmd = Cmds[HandleIterator.CmdIndex];
		    // ...
		    WritePropertyHandle(Writer, HandleIterator.Handle, bDoChecksum);
            Cmd.Property->NetSerializeItem(Writer, Writer.PackageMap, const_cast<uint8*>(Data.Data)); // 序列化属性
            // ...
		}
    }
}
```

## **共享序列化数据**
> changelist可以在多个connection间共享 \
> 序列化数据同样可以共享

### UStruct实现序列化数据共享
```c++
USTRUCT()
struct FMiniJCW
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName Engine = "B48";
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
```

## **客户端接收同步属性**
### UNetDriver::PostTickDispatch
```c++
// Tick时处理数据的过程
void UIpNetDriver::TickDispatch(float DeltaTime)
{
    // ...
    // 处理所有收到的数据包
    for (FPacketIterator It(this); It; ++It)
    {
        // ...
        // 处理原始数据包
        Connection->ReceivedRawPacket(...)
        {
            // ...
            // 处理数据包
            ReceivedPacket(Reader)
            {
                // ...
                // Parse the incoming data.
                FInBunch Bunch( this );
                int32 IncomingStartPos      = Reader.GetPosBits();
                uint8 bControl              = Reader.ReadBit();
                Bunch.PacketId              = InPacketId;
                Bunch.bOpen                 = bControl ? Reader.ReadBit() : 0;
                Bunch.bClose                = bControl ? Reader.ReadBit() : 0;
                // ...
                Channel->ReceivedRawBunch(...) //warning: May destroy channel.
                {
                    // ...
                    bDeleted = ReceivedNextBunch( *Release, bLocalSkipAck );
                    // ...
                    return ReceivedSequencedBunch( *HandleBunch )
                    {
                        // ...
                        ReceivedBunch( Bunch )
                        // ...
                        ProcessBunch(Bunch);
                    }
                }
            }
        }
    }
}
```

### Bunch
> UE底层使用定制的UDP协议传输网络数据，可针对业务类型，对一些数据保证可靠性，另一些数据不保证可靠性。 \
> 网络底层包结构为Packet，可理解为TCP/IP传输层的UDP包（实际算应用层，只是含义偏向底层），\
> 上层包结构为Bunch，可理解为TCP/IP应用层的包，是UE自己定义的格式。
> 
#### 关键字段
```c++
class ENGINE_API FInBunch : public FNetBitReader
{
	int32				ChIndex;    // Channel Index, Bunch所属Channel的下标, 该下标在client和server上应该要一致
	uint8				bOpen:1;    // 是否需要打开Channel
	uint8				bClose:1;   // 是否需要关闭Channel
	uint8				bReliable:1;// 是否可靠
	uint8				bPartial:1;	// Not a complete bunch, 完整的Bunch分块传输
	
}
```

### ActorChannel
> ActorChannel是一个Channel，用于传输Actor的属性和RPC \
> ActorChannel的下标在client和server上应该要一致 \
> **客户端上Actor的创建和销毁也由ActorChannel解析服务器发来的消息驱动**
#### ActorChannel的描述
```c++
/**
 * A channel for exchanging actor and its subobject's properties and RPCs.
 *
 * ActorChannel manages the creation and lifetime of a replicated actor. Actual replication of properties and RPCs
 * actually happens in FObjectReplicator now (see DataReplication.h).
 *
 * An ActorChannel bunch looks like this:
 *
 * +----------------------+---------------------------------------------------------------------------+
 * | SpawnInfo            | (Spawn Info) Initial bunch only                                           |
 * |  -Actor Class        |   -Created by ActorChannel                                                |
 * |  -Spawn Loc/Rot      |                                                                           |
 * | NetGUID assigns      |                                                                           |
 * |  -Actor NetGUID      |                                                                           |
 * |  -Component NetGUIDs |                                                                           |
 * +----------------------+---------------------------------------------------------------------------+
 * |                      |                                                                           |
 * +----------------------+---------------------------------------------------------------------------+
 * | NetGUID ObjRef       | (Content chunks) x number of replicating objects (Actor + any components) |
 * |                      |   -Each chunk created by its own FObjectReplicator instance.              |
 * +----------------------+---------------------------------------------------------------------------+
 * |                      |                                                                           |
 * | Properties...        |                                                                           |
 * |                      |                                                                           |
 * | RPCs...              |                                                                           |
 * |                      |                                                                           |
 * +----------------------+---------------------------------------------------------------------------+
 * | </End Tag>           |                                                                           |
 * +----------------------+---------------------------------------------------------------------------+
 */
```
### UActorChannel::ReceivedBunch
```c++
void UActorChannel::ReceivedBunch( FInBunch & Bunch )
{
    // ...
    ProcessBunch(Bunch)
    {
        // ...
        if( Actor == NULL )
        {
            // 序列化Actor
            AActor* NewChannelActor = NULL;
            bSpawnedNewActor = Connection->PackageMap->SerializeNewActor(Bunch, this, NewChannelActor)
            // UPackageMapClient::SerializeNewActor
            {
                FNetworkGUID NetGUID;
                UObject *NewObj = Actor;
                SerializeObject(Ar, AActor::StaticClass(), NewObj, &NetGUID);
            }
        }
        
        // ----------------------------------------------
        //	Read chunks of actor content
        // ----------------------------------------------
        while ( !Bunch.AtEnd() && Connection != NULL && Connection->State != USOCK_Closed )
        {
            // ...
            // 交由ActorReplicator处理
            Replicator->ReceivedBunch()
            {
                // ...
                // 交由LocalRepLayout处理
                LocalRepLayout.ReceiveProperties(...)
            }
        }
    }
}
```

## **远程方法调用(RPC, Remote Procedure Call)**
> * RPC可由client发往server
> * RPC是瞬时的，不存在状态
> * rpc会在游戏tick过程中发送，而通常Actor的同步和Channel创建要在tick末尾，因此可能出现处理rpc时Channel还未创建的情况。在此时立即调用一次ReplicateActor，写入Actor初始同步数据即可。

### RPC函数宏使用
```c++
UFUNCTION(Reliable, Client)
void RPCClient();

UFUNCTION(Reliable, Server, WithValidation)
void RPCServerWithValidation();

UFUNCTION(Reliable, NetMulticast)
void RPCMulticast();
```

### RPC反射代码
```c++
*.h
// 方法定义
UFUNCTION(Reliable, Server, WithValidation)
void RPCServerWithValidation();

*.cpp
// 方法实现
void AParnnyActor_Net::RPCServerWithValidation_Implementation()
{
}
// 验证方法实现
bool AParnnyActor_Net::RPCServerWithValidation_Validate()
{
    return true;
}

*.gen.cpp
// 反射代码
static FName NAME_AParnnyActor_Net_RPCServerWithValidation = FName(TEXT("RPCServerWithValidation"));
void AParnnyActor_Net::RPCServerWithValidation()
{
    ProcessEvent(FindFunctionChecked(NAME_AParnnyActor_Net_RPCServerWithValidation),NULL);
}

DEFINE_FUNCTION(AParnnyActor_Net::execRPCServerWithValidation)
{
    P_FINISH;
    P_NATIVE_BEGIN;
    if (!P_THIS->RPCServerWithValidation_Validate())
    {
        RPC_ValidateFailed(TEXT("RPCServerWithValidation_Validate"));
        return;
    }
    P_THIS->RPCServerWithValidation_Implementation();
    P_NATIVE_END;
}
```

### UNetDriver::ProcessRemoteFunction
```c++
void UObject::ProcessEvent( UFunction* Function, void* Parms )
{
    // ...
    // 判断是不是RPC
    int32 FunctionCallspace = GetFunctionCallspace(Function, NULL);
    if (FunctionCallspace & FunctionCallspace::Remote)
    {
        // AActor::CallRemoteFunction
        CallRemoteFunction(Function, Parms, NULL, NULL)
        {
            // ...
            // 遍历所有的NetDriver
            for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
            {
                if (Driver.NetDriver != nullptr && Driver.NetDriver->ShouldReplicateFunction(...))
                {
                    Driver.NetDriver->ProcessRemoteFunction(this, Function, Parameters, OutParms, Stack, nullptr);
                    bProcessed = true;
                }
            }
        }
    }
}
```
### 单播、多播RPC区别








