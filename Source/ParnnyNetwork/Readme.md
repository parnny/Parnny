# 相关内容 

## ParnnyNote
> UE的属性同步依赖反射系统，
> UE属性同步以Actor为单位，每个Actor有一个对应的ActorChannel，
> 这个Channel主要负责与Actor实例相关的内容，包括属性的同步与变动历史，相对应的ShadowBuffer
> 每个UClass对应一个RepLayout，
> RepLayout中记录了Class中所有需要同步的属性，每个属性对应一个FRepParentCmd，
> 简单属性会对应一个RepLayoutCmd，复杂属性会对应多个RepLayoutCmd。
> 

## Reference
[服务器同步属性](https://zhuanlan.zhihu.com/p/412517987)

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



