//      __________        ___               ______            _
//     / ____/ __ \____  / (_)___  ___     / ____/___  ____ _(_)___  ___
//    / /_  / / / / __ \/ / / __ \/ _ \   / __/ / __ \/ __ `/ / __ \/ _ \
//   / __/ / /_/ / / / / / / / / /  __/  / /___/ / / / /_/ / / / / /  __/
//  /_/    \____/_/ /_/_/_/_/ /_/\___/  /_____/_/ /_/\__, /_/_/ /_/\___/
//                                                  /____/
// FOnline Engine
// https://fonline.ru
// https://github.com/cvet/fonline
//
// MIT License
//
// Copyright (c) 2006 - present, Anton Tsvetinskiy aka cvet <cvet@tut.by>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#include "Common.h"

#include "Entity.h"
#include "FileSystem.h"
#include "Settings.h"

DECLARE_EXCEPTION(ScriptSystemException);
DECLARE_EXCEPTION(ScriptException);

class NameResolver
{
public:
    virtual int GetEnumValue(const string& enum_value_name, bool& fail) = 0;
    virtual int GetEnumValue(const string& enum_name, const string& value_name, bool& fail) = 0;
    virtual string GetEnumValueName(const string& enum_name, int value) = 0;
};

template<typename... Args>
class ScriptEvent : public NonCopyable
{
public:
    using Callback = std::function<bool(Args...)>;

    ScriptEvent() = default;

    ScriptEvent& operator+=(Callback cb)
    {
        callbacks.push_back(cb);
        return *this;
    }

    bool operator()(Args... args)
    {
        for (auto& cb : callbacks)
            if (cb(std::forward<Args>(args)...))
                return true;
        return false;
    }

private:
    vector<Callback> callbacks {};
};

template<typename TRet, typename... Args>
class ScriptFunc
{
public:
    using Func = std::function<bool(Args..., TRet&)>;

    ScriptFunc() = default;
    ScriptFunc(Func f) : func {f} {}
    operator bool() { return !!func; }
    bool operator()(Args... args) { return func(std::forward<Args>(args)..., ret); }
    TRet GetResult() { return ret; }

private:
    Func func {};
    TRet ret {};
};

template<typename... Args>
class ScriptFunc<void, Args...>
{
public:
    using Func = std::function<bool(Args...)>;

    ScriptFunc() = default;
    ScriptFunc(Func f) : func {f} {}
    operator bool() { return !!func; }
    bool operator()(Args... args) { return func(std::forward<Args>(args)...); }

private:
    Func func {};
};

class ScriptSystem : public NameResolver, public NonMovable
{
public:
    ScriptSystem(void* obj, GlobalSettings& sett, FileManager& file_mngr);
    virtual ~ScriptSystem() = default;
    int GetEnumValue(const string& enum_value_name, bool& fail) { return 0; }
    int GetEnumValue(const string& enum_name, const string& value_name, bool& fail) { return 0; }
    string GetEnumValueName(const string& enum_name, int value) { return ""; }
    void RemoveEntity(Entity* entity) {}

    template<typename TRet, typename... Args, std::enable_if_t<!std::is_void_v<TRet>, int> = 0>
    ScriptFunc<TRet, Args...> FindFunc(const string& func_name)
    {
        return ScriptFunc<TRet, Args...>();
    }

    template<typename TRet = void, typename... Args, std::enable_if_t<std::is_void_v<TRet>, int> = 0>
    ScriptFunc<void, Args...> FindFunc(const string& func_name)
    {
        return ScriptFunc<void, Args...>();
    }

    template<typename TRet, typename... Args, std::enable_if_t<!std::is_void_v<TRet>, int> = 0>
    bool CallFunc(const string& func_name, Args... args, TRet& ret)
    {
        auto func = FindFunc<TRet, Args...>(func_name);
        if (func && func(args...))
        {
            ret = func.GetResult();
            return true;
        }
        return false;
    }

    template<typename TRet = void, typename... Args, std::enable_if_t<std::is_void_v<TRet>, int> = 0>
    bool CallFunc(const string& func_name, Args... args)
    {
        auto func = FindFunc<void, Args...>(func_name);
        return func && func(args...);
    }

protected:
    void* mainObj;
    GlobalSettings& settings;
    FileManager& fileMngr;

    struct NativeImpl;
    shared_ptr<NativeImpl> pNativeImpl {};
    struct AngelScriptImpl;
    shared_ptr<AngelScriptImpl> pAngelScriptImpl {};
    struct MonoImpl;
    shared_ptr<MonoImpl> pMonoImpl {};

    /*
public:
    void RunModuleInitFunctions();

    string GetDeferredCallsStatistics();
    void ProcessDeferredCalls();
    // Todo: rework FONLINE_
    / *#if defined(FONLINE_SERVER) || defined(FONLINE_EDITOR)
        bool LoadDeferredCalls();
    #endif* /

    StrVec GetCustomEntityTypes();
    / *#if defined(FONLINE_SERVER) || defined(FONLINE_EDITOR)
        bool RestoreCustomEntity(const string& type_name, uint id, const DataBase::Document& doc);
    #endif* /

    void RemoveEventsEntity(Entity* entity);

    void HandleRpc(void* context);

    string GetActiveFuncName();

    uint BindByFuncName(const string& func_name, const string& decl, bool is_temp, bool disable_log = false);
    // uint BindByFunc(asIScriptFunction* func, bool is_temp, bool disable_log = false);
    uint BindByFuncNum(hash func_num, bool is_temp, bool disable_log = false);
    // asIScriptFunction* GetBindFunc(uint bind_id);
    string GetBindFuncName(uint bind_id);

    // hash GetFuncNum(asIScriptFunction* func);
    // asIScriptFunction* FindFunc(hash func_num);
    hash BindScriptFuncNumByFuncName(const string& func_name, const string& decl);
    // hash BindScriptFuncNumByFunc(asIScriptFunction* func);
    uint GetScriptFuncBindId(hash func_num);
    void PrepareScriptFuncContext(hash func_num, const string& ctx_info);

    void CacheEnumValues();


    void PrepareContext(uint bind_id, const string& ctx_info);
    void SetArgUChar(uchar value);
    void SetArgUShort(ushort value);
    void SetArgUInt(uint value);
    void SetArgUInt64(uint64 value);
    void SetArgBool(bool value);
    void SetArgFloat(float value);
    void SetArgDouble(double value);
    void SetArgObject(void* value);
    void SetArgEntity(Entity* value);
    void SetArgAddress(void* value);
    bool RunPrepared();
    void RunPreparedSuspend();
    // asIScriptContext* SuspendCurrentContext(uint time);
    // void ResumeContext(asIScriptContext* ctx);
    void RunSuspended();
    void RunMandatorySuspended();
    // bool CheckContextEntities(asIScriptContext* ctx);
    uint GetReturnedUInt();
    bool GetReturnedBool();
    void* GetReturnedObject();
    float GetReturnedFloat();
    double GetReturnedDouble();
    void* GetReturnedRawAddress();

private:
    HashIntMap scriptFuncBinds {}; // Func Num -> Bind Id
    StrIntMap cachedEnums {};
    map<string, IntStrMap> cachedEnumNames {};*/
};
