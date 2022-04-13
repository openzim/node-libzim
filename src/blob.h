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
      : Napi::ObjectWrap<Blob>(info),
        blob_{std::make_shared<zim::Blob>()},
        refcontent_{} {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info[0].IsExternal()) {  // handle internal zim::Blob
      blob_ = std::make_shared<zim::Blob>(
          *info[0].As<Napi::External<zim::Blob>>().Data());
    } else if (info.Length() > 0) {  // use refcontent_ and copy content
      // TODO(kelvinhammond): avoid copying content somehow in certain scenarios
      // if possible Maybe use a reference object??? What is the lifecycle of
      // that and what if it's changed?
      if (info[0].IsArrayBuffer()) {  // handle ArrayBuffer
        auto buf = info[0].As<Napi::ArrayBuffer>();
        auto refcontent_ =
            std::string(static_cast<char *>(buf.Data()), buf.ByteLength());
      } else if (info[0].IsBuffer()) {  // handle Buffer
        auto buf = info[0].As<Napi::Buffer<char>>();
        auto refcontent_ = std::string(buf.Data(), buf.Length());
      } else {                             // all others toString()
        refcontent_ = info[0].ToString();  // coerce
      }

      blob_ =
          std::make_shared<zim::Blob>(refcontent_.data(), refcontent_.size());
    }
  }

  static Napi::Object New(Napi::Env env, zim::Blob &blob) {
    auto external = Napi::External<zim::Blob>::New(env, &blob);
    auto &constructor = env.GetInstanceData<ModuleConstructors>()->blob;
    return constructor.New({external});
  }

  Napi::Value getData(const Napi::CallbackInfo &info) {
    try {
      // TODO(kelvinhammond): find a way to have a readonly buffer in NodeJS
      return Napi::Buffer<char>::Copy(info.Env(), blob_->data(), blob_->size());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value toString(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), (std::string)*blob_);
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getSize(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), blob_->size());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
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
  std::shared_ptr<zim::Blob> blob() const { return blob_; }

 private:
  std::shared_ptr<zim::Blob> blob_;
  std::string refcontent_;
};
