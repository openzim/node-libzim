#pragma once

#include <napi.h>
#include <zim/blob.h>
#include <exception>
#include <map>
#include <memory>
#include <string>

#include "common.h"

class Blob : public Napi::ObjectWrap<Blob> {
 public:
  explicit Blob(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<Blob>(info), blob_{} {
    if (info[0].IsExternal()) {
      // handle internal zim::Blob
      // Copy blob (shared_ptr) from external
      blob_ = zim::Blob(*info[0].As<Napi::External<zim::Blob>>().Data());
    } else if (info.Length() > 0) {  // use refcontent_ and copy content
      // TODO(kelvinhammond): avoid copying content somehow in certain scenarios
      // if possible Maybe use a reference object??? What is the lifecycle of
      // that and what if it's changed?

      size_t size = 0;
      std::shared_ptr<char> data = nullptr;
      if (info[0].IsArrayBuffer()) {  // handle ArrayBuffer
        auto buf = info[0].As<Napi::ArrayBuffer>();
        size = buf.ByteLength();
        data = std::shared_ptr<char>(new char[size],
                                     std::default_delete<char[]>());
        memcpy(data.get(), buf.Data(), size);
      } else if (info[0].IsBuffer()) {  // handle Buffer
        auto buf = info[0].As<Napi::Buffer<char>>();
        size = buf.Length();
        data = std::shared_ptr<char>(new char[size],
                                     std::default_delete<char[]>());
        memcpy(data.get(), buf.Data(), size);
      } else if (info[0].IsString()) {  // all others toString()
        auto str = info[0].As<Napi::String>().Utf8Value();  // coerce to string
        size = str.size();
        data = std::shared_ptr<char>(new char[size],
                                     std::default_delete<char[]>());
        memcpy(data.get(), str.c_str(), size);
      } else {
        throw Napi::Error::New(
            info.Env(),
            "Blob constructor expects an ArrayBuffer, Buffer, or String");
      }

      blob_ = zim::Blob(data, size);  // blob takes ownership
    }
  }

  static Napi::Object New(Napi::Env env, zim::Blob &blob) {
    auto external = Napi::External<zim::Blob>::New(env, &blob);
    return GetConstructor(env).New({external});
  }

  Napi::Value getData(const Napi::CallbackInfo &info) {
    try {
      // TODO(kelvinhammond): find a way to have a readonly buffer in NodeJS
      return Napi::Buffer<char>::Copy(info.Env(), blob_.data(), blob_.size());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value toString(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), (std::string)blob_);
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getSize(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), blob_.size());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
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
    return env.GetInstanceData<ModuleConstructors>()->blob;
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func =
        DefineClass(env, "Blob",
                    {
                        InstanceAccessor<&Blob::getData>("data"),
                        InstanceAccessor<&Blob::getSize>("size"),
                        InstanceMethod<&Blob::toString>("toString"),
                    });

    exports.Set("Blob", func);
    constructors.blob = Napi::Persistent(func);
  }

  // internal module methods
  const zim::Blob &blob() const { return blob_; }

 private:
  zim::Blob blob_;
};
