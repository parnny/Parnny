# Gameplay

## ParnnyNote
> 
> 
> 
> 


## Net Mode
```c++
enum ENetMode
{
	// 独立游戏:没有网络的游戏，只有一个或多个本地玩家。仍然被认为是服务器，因为它具有所有服务器功能
	NM_Standalone,
	// 专用服务器模式:没有本地玩家的服务器。
	NM_DedicatedServer,
	// 监听服务器模式:一个服务器也有一个本地玩家谁是主持游戏,可用于其他玩家在网络上。
	// 房间制游戏
	NM_ListenServer,
	// 网络客户端模式:客户端连接到远程服务器。
	NM_Client,
};
```

* Playable : GameInstance是否有LocalPlayer,是否可以响应玩家输入, 是否有渲染
* Authority: 是否有权力控制游戏状态, 是否具有GameMode
* Service: 是否可以作为服务器开放给其他客户端


| ENetMode | Playable | Authority  | Service |
|----------|----------|------------ |---------|
|     Standalone     | √        |      √      |     ×    |
| DedicatedServer      | ×        |    √        |    √     |
| ListenServer      | √        |     √       |    √     |
| Client      | √        |      ×      |    ×     |

## Struct
### FURL
```c++

```

## Replication System Basics
> * GameEngine创建NetDriver
> * NetDriver创建NetConnection, 服务器对应每个客户端创建一个NetConnection, 客户端只有一个NetConnection
> * NetConnection创建Channel, Channel有多个, 用于不同的功能, 比如ControlChannel, VoiceChannel, ActorChannel
### GameServer
```c++
// From EngineInit()-> GEngineLoop.Init()
// UGameEngine::Start()
GEngine->Start()
{
    GameInstance->StartGameInstance()
    {
        // ...
        FURL URL(&DefaultURL, *PackageName, TRAVEL_Partial);
        if (URL.Valid)
        {
            // UEngine::Browse
            BrowseRet = Engine->Browse(*WorldContext, URL, Error)
            {
                if( URL.IsLocalInternal() )
                {
                    // Local map file.
                    return LoadMap( WorldContext, URL, NULL, Error ) //...
                    {
                        // ...
                        WorldContext.World()->SetGameMode(URL);
                        // ...
                        // Listen for clients.
                        if (Pending == NULL && (!GIsClient || URL.HasOption(TEXT("Listen"))))
                        {
                            WorldContext.World()->Listen(URL))
                        }
                    }
                }
            }
        }
    }
}
```
### GameClient
```c++
// From EngineInit()-> GEngineLoop.Init()
// UGameEngine::Start()
GEngine->Start()
{
    GameInstance->StartGameInstance()
    {
        // ...
        FURL URL(&DefaultURL, *PackageName, TRAVEL_Partial);
        if (URL.Valid)
        {
            // UEngine::Browse
            BrowseRet = Engine->Browse(*WorldContext, URL, Error)
            {
                if( URL.IsLocalInternal() )
                {
                   // ... As Server 
                }
                else if( URL.IsInternal() && GIsClient )
                {
                    // ...
                    WorldContext.PendingNetGame = NewObject<UPendingNetGame>();
                    WorldContext.PendingNetGame->Initialize(URL); //-V595
                    // UPendingNetGame::InitNetDriver()
                    WorldContext.PendingNetGame->InitNetDriver()
                    {
                        // ...
                        NetDriver->InitConnect(...)
                    }
                }
            }
        }
    }
}
```

### Actor Replication
* **Lifetime**
* **Property Replication**
* **RPC**

### Actor Code
```c++
class ENGINE_API AActor : public UObject
{
    // ...
    // 根据距离判断Actor 是否需要Replicate(视野)
    bool IsWithinNetRelevancyDistance(const FVector& SrcLocation) const;
    // ...
	// 同步频率
	UPROPERTY(Category=Replication, EditDefaultsOnly, BlueprintReadWrite)
	float NetUpdateFrequency;
	// 同步优先级
	UPROPERTY(Category=Replication, EditDefaultsOnly, BlueprintReadWrite)
	float NetPriority;
	// 获取Actor的网络优先级(规则)
	virtual float GetNetPriority(...);
}
```

## RPC
### Server Call Client
> * JCW Start Engine
> * Server: Pawn->Client_JCWStartEngine() @ Server
> * ClientA: Client_JCWStartEngine_Implementation() @ ClientA

### Client Call Server
> * JCW Oiling
> * ClientA: Pawn->Server_JCWOiling() @ ClientA
> * Server: Server_JCWOiling_Implementation() @ Server

### Server Call Multicast
> * JCW Horn In City
> * ClientA: Pawn->Server_JCWHorn() @ ClientA
> * Server: JCWHorn_Implementation() @ Server
> * ClientA: JCWHorn_Implementation() @ ClientA
> * ClientB: JCWHorn_Implementation() @ ClientB

> 多播在调用时,可能ClientB与Server不相关,所有收不到
> * Pawn->IsNetRelevantFor(ClientA) == true
> * Pawn->IsNetRelevantFor(ClientB) == false

### Reliable
> * Unreliable的RPC, 会在网络不稳定时丢失(带宽不足,丢包等)
> * Unreliable的RPC不保证顺序