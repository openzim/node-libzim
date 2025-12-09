#pragma once

#include <napi.h>
#include <zim/zim.h>

class OpenConfig : public Napi::ObjectWrap<OpenConfig> {
 public:
  explicit OpenConfig(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<OpenConfig>(info), config_{zim::OpenConfig()} {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() > 0) {
      throw Napi::Error::New(
          info.Env(), "OpenConfig constructor does not take any arguments.");
    }
  }

  Napi::Value preloadXapianDb(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsBoolean()) {
      throw Napi::TypeError::New(
          env, "First argument must be a boolean for preloadXapianDb.");
    }
    bool&& value = info[0].As<Napi::Boolean>().Value();
    config_.preloadXapianDb(value);
    return info.This();
  }

  Napi::Value preloadDirentRanges(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsNumber()) {
      throw Napi::TypeError::New(
          env, "First argument must be a number for preloadDirentRanges.");
    }
    int&& value = info[0].As<Napi::Number>().Int32Value();
    config_.preloadDirentRanges(value);
    return info.This();
  }

  Napi::Value getPreloadXapianDb(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Value::From(env, config_.m_preloadXapianDb);
  }

  Napi::Value getPreloadDirentRanges(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Value::From(env, config_.m_preloadDirentRanges);
  }

  static Napi::FunctionReference& GetConstructor(Napi::Env env) {
    auto& constructor = env.GetInstanceData<ModuleConstructors>()->openConfig;
    return constructor;
  }

  const zim::OpenConfig& getInternalConfig() const { return config_; }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors& constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env, "OpenConfig",
        {
            InstanceMethod<&OpenConfig::preloadXapianDb>("preloadXapianDb"),
            InstanceMethod<&OpenConfig::preloadDirentRanges>(
                "preloadDirentRanges"),
            InstanceAccessor<&OpenConfig::getPreloadXapianDb>(
                "m_preloadXapianDb"),
            InstanceAccessor<&OpenConfig::getPreloadDirentRanges>(
                "m_preloadDirentRanges"),
        });

    exports.Set("OpenConfig", func);
    constructors.openConfig = Napi::Persistent(func);
  }

 private:
  zim::OpenConfig config_;
};
