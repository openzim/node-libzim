#pragma once

#include <napi.h>
#include <zim/illustration.h>

#include <exception>
#include <map>
#include <string>

class IllustrationInfo : public Napi::ObjectWrap<IllustrationInfo> {
 public:
  explicit IllustrationInfo(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<IllustrationInfo>(info), ii_{} {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() == 0) {
      // Default constructor
      ii_ = zim::IllustrationInfo{};
      return;
    }

    // IllustrationInfo(ii: External<zim::IllustrationInfo>)
    // Construct from external, used internally
    if (info[0].IsExternal()) {
      auto ext = info[0].As<Napi::External<zim::IllustrationInfo>>();
      ii_ = *(ext.Data());
      return;
    }

    if (!info[0].IsObject()) {
      throw Napi::Error::New(env,
                             "IllustrationInfo constructor expects an "
                             "IllustrationInfo like object");
    }

    auto value = info[0];
    // IllustrationInfo(ii: IllustrationInfo)
    // Copy constructor
    if (InstanceOf(env, value)) {
      auto unwrapped = Unwrap(value.As<Napi::Object>());
      ii_ = zim::IllustrationInfo(unwrapped->ii_);
      return;
    }

    // IllustrationInfo(ii: object)
    // Construct from object
    ii_ = infoFrom(value.As<Napi::Object>());
  }

  static Napi::Object New(Napi::Env env, const zim::IllustrationInfo &ii) {
    Napi::FunctionReference &constructor = GetConstructor(env);
    auto &&data = Napi::External<zim::IllustrationInfo>::New(
        env, new zim::IllustrationInfo(ii),
        [](Napi::BasicEnv /*env*/, zim::IllustrationInfo *ptr) { delete ptr; });
    return constructor.New({data});
  }

  /**
   * Construct zim::IllustrationInfo from a JS object
   */
  static zim::IllustrationInfo infoFrom(Napi::Object obj) {
    zim::IllustrationInfo ii{
        .width =
            obj.Has("width") ? obj.Get("width").ToNumber().Uint32Value() : 0,
        .height =
            obj.Has("height") ? obj.Get("height").ToNumber().Uint32Value() : 0,
        .scale =
            obj.Has("scale") ? obj.Get("scale").ToNumber().FloatValue() : 0.0f,
        .extraAttributes = std::map<std::string, std::string>{},
    };

    if (obj.Has("extraAttributes") && obj.Get("extraAttributes").IsObject()) {
      auto extraAttributes = obj.Get("extraAttributes").ToObject();
      auto keys = extraAttributes.GetPropertyNames();
      for (const auto &e : keys) {
        auto key =
            static_cast<Napi::Value>(e.second).As<Napi::String>().Utf8Value();
        auto value = extraAttributes.Get(key).ToString().Utf8Value();
        ii.extraAttributes[key] = value;
      }
    }

    return ii;
  }

  Napi::Value getWidth(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return Napi::Value::From(env, ii_.width);
  }

  Napi::Value getHeight(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return Napi::Value::From(env, ii_.height);
  }

  Napi::Value getScale(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return Napi::Value::From(env, ii_.scale);
  }

  Napi::Value getExtraAttributes(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::Object obj = Napi::Object::New(env);
    for (const auto &pair : ii_.extraAttributes) {
      obj.Set(pair.first, pair.second);
    }
    obj.Freeze();
    return obj;
  }

  Napi::Value asMetadataItemName(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return Napi::Value::From(env, ii_.asMetadataItemName());
  }

  static Napi::Value fromMetadataItemName(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
      throw Napi::TypeError::New(
          env, "First argument must be a string for metadata item name.");
    }

    auto name = info[0].As<Napi::String>().Utf8Value();
    auto ii = zim::IllustrationInfo::fromMetadataItemName(name);
    return New(env, ii);
  }

  static bool InstanceOf(Napi::Env env, Napi::Value value) {
    if (!value.IsObject()) {
      return false;
    }
    Napi::Object obj = value.As<Napi::Object>();
    Napi::FunctionReference &constructor = GetConstructor(env);
    return obj.InstanceOf(constructor.Value());
  }

  static Napi::FunctionReference &GetConstructor(Napi::Env env) {
    auto constructors = env.GetInstanceData<ModuleConstructors>();
    return constructors->illustrationInfo;
  }

  zim::IllustrationInfo &getInternalIllustrationInfo() { return ii_; }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env, "IllustrationInfo",
        {
            InstanceAccessor<&IllustrationInfo::getWidth>("width"),
            InstanceAccessor<&IllustrationInfo::getHeight>("height"),
            InstanceAccessor<&IllustrationInfo::getScale>("scale"),
            InstanceAccessor<&IllustrationInfo::getExtraAttributes>(
                "extraAttributes"),
            InstanceMethod<&IllustrationInfo::asMetadataItemName>(
                "asMetadataItemName"),
            StaticMethod<&IllustrationInfo::fromMetadataItemName>(
                "fromMetadataItemName"),
        });

    exports.Set("IllustrationInfo", func);
    constructors.illustrationInfo = Napi::Persistent(func);
  }

 private:
  zim::IllustrationInfo ii_;
};

