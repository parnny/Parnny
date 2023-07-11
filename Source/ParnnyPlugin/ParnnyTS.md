# About TS


## 动态加载脚本
```c++
void FJsEnvImpl::LoadFileTest(const FString& Path)
{
    const auto Isolate = MainIsolate;
    v8::Isolate::Scope ScopeObject(MainIsolate);
    v8::HandleScope HandleScopeObject(MainIsolate);
    const v8::Local<v8::Context> Context = DefaultContext.Get(MainIsolate);
    v8::Context::Scope ContextScopeObject(Context);
    v8::TryCatch TryCatch(Isolate);
    v8::Local<v8::Value> Args[] = {FV8Utils::ToV8String(Isolate, Path)};
    __USE(Require.Get(Isolate)->Call(Context, v8::Undefined(Isolate), 1, Args));
    if (TryCatch.HasCaught())
    {
        Logger->Error(FV8Utils::TryCatchToString(Isolate, &TryCatch));
    }
}
```