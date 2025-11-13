#pragma once

#include <napi.h>
#include <zim/illustration.h>
#include <exception>
#include <memory>

class Illustration : public Napi::ObjectWrap<Illustration> {
 public:
  explicit Illustration(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<Item>(info), illustration_{nullptr} {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!info[0].IsExternal()) {
      throw Napi::Error::New(
          env, "Illustration must be constructed internally by another class.");
    }

    illustration_ = std::make_shared<zim::Illustration>(
        *info[0].As<Napi::External<zim::Illustration>>().Data());
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env, "Illustration",
        {
            InstanceAccessor<&Illustration::getTitle>("title"),
            InstanceAccessor<&Illustration::getPath>("path"),
            InstanceAccessor<&Illustration::getMimetype>("mimetype"),
            InstanceAccessor<&Illustration::getData>("data"),
            InstanceMethod<&Illustration::getData>("getData"),
            InstanceAccessor<&Illustration::getSize>("size"),
            InstanceAccessor<&Illustration::getDirectAccessInformation>(
                "directAccessInformation"),
            InstanceAccessor<&Illustration::getIndex>("index"),
        });

    exports.Set("Illustration", func);
    constructors.illustration = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::Illustration> illustration_;
}

