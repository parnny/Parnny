# Common UI


## 简述
* **Styling System**
  > 不需要单独配置每一个组件的样式，只需要配置组件的样式属性，组件会自动根据属性生成样式
* **Input Mapping System**
  > * 感知玩家输入, 与游戏输入系统分离, 映射到特定的控制器数据资产
  > * 根据不同平台, 映射不同的按钮图标
* **Input Routing System**
* **Activatable Widget**
  > * 界面基类(替换UserWidget成为界面的基类)
  > * 界面焦点控制
* **Menu Stack Widget**
  > * 界面层级管理
  > * 内置过渡动画

### 使用CommonUI时需要替换Viewport类
设置 Engine - General Settings-> GameViewportClientClass 为 **CommonGameViewportClient**



## Styling
> 用于规划各种UI控件的样式风格, 减少UI制作时的样式配置操作
* UCommonBorderStyle
* UCommonButtonStyle
* UCommonTextStyle
* UCommonTextScrollStyle

### 如何指定默认style
> 在项目配置 Plugins - Common UI Editor 下指定

## Input
### ECommonInputType
> 输入类型
```c++
UENUM(BlueprintType)
enum class ECommonInputType : uint8
{
  MouseAndKeyboard,
  Gamepad,
  Touch,
};
```

### UCommonUIInputData
> * 配置默认的点击动作 
> * 配置默认的回退动作

> 配置在 Game - Common Input Settings 的 **InputData** 中

### UCommonInputBaseControllerData
> * 对某一种**游戏控制器**(Xbox,PS,Keyboard) 的描述
> * 每支持一种**游戏控制器**需要配置一个对应的数据资产, eg: ControllerData_Xbox, ControllerData_PS, ControllerData_Keyboard
> * 配置控制器图标(手柄图标, 键盘图标),告知玩家当前控制器的类型
> * 配置相对应控制器的操作图标(X,Y,LT,RB等), 提示玩家执行某个动作所对应的按键

### FCommonInputActionDataBase
> * 继承FRowDataBase
> * 需要使用DataTable配置
> * CommonUIInputData 会引用该数据资产

> * RowName为ActionName
> * Keyboard Input Type Info 为键盘鼠标模式下, 执行该动作所对应的按键
> * Gamepad Input Type Info 为手柄模式下, 执行该动作所对应的按键
> * Touch Input Type Info 为触摸模式下, 执行该动作所对应的按键

### UCommonInputSubsystem
> * 用于管理CommonUI输入
> * ULocalPlayerSubsystem
> * 拦截SlateApplication的输入事件, 并将其转换为CommonUI的输入事件
```c++
void UCommonInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    // ...
    // 注册输入事件拦截器
    CommonInputPreprocessor = MakeShared<FCommonInputPreprocessor>(*this);
    FSlateApplication::Get().RegisterInputPreProcessor(CommonInputPreprocessor, 0);
}
```

### UCommonUIActionRouterBase
> * CommonUI输入路由系统的核心
> * **ULocalPlayerSubsystem**
> * 通过ProcessInput处理输入事件


### 相关引擎定义
### EUINavigation
> * 用于描述UI导航的方向
> * 默认情况下, 导航上下左右四个方向的导航是以鼠标位置为基准的
```c++
FSlateApplication::AttemptNavigation(...)
{
    // ...
    // Find the next widget
    if (NavigationType == EUINavigation::Next || NavigationType == EUINavigation::Previous)
    {
        // Fond the next widget
        FWeakWidgetPath WeakNavigationSource(NavigationSource);
        FWidgetPath NewFocusedWidgetPath = WeakNavigationSource.ToNextFocusedPath(...);
        // ...
    }
    else
    {
        // ...
        NavigationSource.GetDeepestWindow()->GetHittestGrid().FindNextFocusableWidget(...)
    }
}
```

```c++
enum class EUINavigation : uint8
{
    Left,
    Right,
    Up,
    Down,
    Next,
    Previous,
    Invalid,
};
```
### FNavigationConfig
> * 配置UI导航规则
> * 通过FSlateApplication::SetNavigationConfig()设置
```c++
class SLATE_API FNavigationConfig : public TSharedFromThis<FNavigationConfig>
{
    FNavigationConfig()
    // ...
    {
        // 注册默认的导航规则
        AnalogHorizontalKey = EKeys::Gamepad_LeftX;
        AnalogVerticalKey = EKeys::Gamepad_LeftY;
        KeyEventRules.Emplace(EKeys::Left, EUINavigation::Left);
        KeyEventRules.Emplace(EKeys::Gamepad_DPad_Left, EUINavigation::Left);
        KeyEventRules.Emplace(EKeys::Right, EUINavigation::Right);
        KeyEventRules.Emplace(EKeys::Gamepad_DPad_Right, EUINavigation::Right);
        KeyEventRules.Emplace(EKeys::Up, EUINavigation::Up);
        KeyEventRules.Emplace(EKeys::Gamepad_DPad_Up, EUINavigation::Up);
        KeyEventRules.Emplace(EKeys::Down, EUINavigation::Down);
        KeyEventRules.Emplace(EKeys::Gamepad_DPad_Down, EUINavigation::Down);
    }
}
```

## Widgets

### UCommonActivatableWidget
### UCommonButtonBase
### UCommonTextBlock
### UCommonActionWidget

### UCommonBoundActionBar && UCommonBoundActionButton
> * 用于收集当前界面的所有动作, 展示在列表中
收集创建按钮相关代码
```c++
void UCommonBoundActionBar::HandleDeferredDisplayUpdate()
{
    // ...
    for (const ULocalPlayer* LocalPlayer : SortedPlayers)
    {
        if (const UCommonUIActionRouterBase* ActionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(...))
        {
            // ...
            TSet<FName> AcceptedBindings;
            // 收集所有动作
            TArray<FUIActionBindingHandle> FilteredBindings = ActionRouter->GatherActiveBindings().FilterByPredicate(...)
            // ... Sort
            // 创建 UCommonBoundActionButton 按钮
            for (FUIActionBindingHandle BindingHandle : FilteredBindings)
            {
                ICommonBoundActionButtonInterface* ActionButton = Cast<ICommonBoundActionButtonInterface>(CreateEntryInternal(ActionButtonClass));
                if (ensure(ActionButton))
                {
                    // 绑定动作
                    ActionButton->SetRepresentedAction(BindingHandle);
                }
            }
        }
    }
}
```
动作注册相关代码
```c++
FUIActionBindingHandle UCommonUserWidget::RegisterUIActionBinding(const FBindUIActionArgs& BindActionArgs)
{
    if (UCommonUIActionRouterBase* ActionRouter = UCommonUIActionRouterBase::Get(*this))
    {
        FBindUIActionArgs FinalBindActionArgs = BindActionArgs;
        if (bDisplayInActionBar && !BindActionArgs.bDisplayInActionBar)
        {
                FinalBindActionArgs.bDisplayInActionBar = true;
        }
        FUIActionBindingHandle BindingHandle = ActionRouter->RegisterUIActionBinding(*this, FinalBindActionArgs);
        ActionBindings.Add(BindingHandle);
        return BindingHandle;
    }
    
    return FUIActionBindingHandle();
}

void UCommonUserWidget::RemoveActionBinding(FUIActionBindingHandle ActionBinding)
{
	ActionBindings.Remove(ActionBinding);
	if (UCommonUIActionRouterBase* ActionRouter = UCommonUIActionRouterBase::Get(*this))
	{
		ActionRouter->RemoveBinding(ActionBinding);
	}
}

void UCommonUserWidget::AddActionBinding(FUIActionBindingHandle ActionBinding)
{
	ActionBindings.Add(ActionBinding);
	if (UCommonUIActionRouterBase* ActionRouter = UCommonUIActionRouterBase::Get(*this))
	{
		ActionRouter->AddBinding(ActionBinding);
	}
}
```
## 其他

### Xbox手柄按键对应

|	按键	|	MouseAndKeyboard	|	Gamepad	|
|	----------------	|	----------------	|	----------------	|
|	左扳机键 LT	|	\	|	Gamepad LeftTrigger	|
|	右扳机键 RT	|	\	|	Gamepad Right Trigger	|
|	左缓冲键 LB	|	Left Ctrl	|	Gamepad Left Shoulder	|
|	右缓冲键 RB	|	Left Alt	|	Gamepad Right Shoulder	|
|	视图键 	|	Tab	|	Gamepad Special Left	|
|	菜单键 	|	Escape	|	Gamepad Special Right	|
|	方向键上	|	Up   	|	Gamepad D-pad Up	|
|	方向键下	|	Down 	|	Gamepad D-pad Down	|
|	方向键左	|	Left 	|	Gamepad D-pad Left	|
|	方向键右	|	Right	|	Gamepad D-pad Right	|
|	左摇杆上	|	Up   	|	Gamepad Left Thumbstick Up	|
|	左摇杆下	|	Down 	|	Gamepad Left Thumbstick Down	|
|	左摇杆左	|	Left 	|	Gamepad Left Thumbstick Left	|
|	左摇杆右	|	Right	|	Gamepad Left Thumbstick Right	|
|	右摇杆上	|	Mouse	|	Gamepad Right Thumbstick Up	|
|	右摇杆下	|	Mouse	|	Gamepad Right Thumbstick Down	|
|	右摇杆左	|	Mouse	|	Gamepad Right Thumbstick Left	|
|	右摇杆右	|	Mouse	|	Gamepad Right Thumbstick Right	|
|	X	|	Page Up	|	Gamepad Face Button Left	|
|	Y	|	Page Down	|	Gamepad Face Button Top	|
|	A	|	Enter	|	Gamepad Face Button Bottom	|
|	B	|	Space Bar	|	Gamepad Face Button Right	|




