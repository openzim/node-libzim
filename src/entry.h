#pragma once

#include <napi.h>
#include <zim/entry.h>
#include <exception>
#include <memory>

#include "item.h"

class Entry : public Napi::ObjectWrap<Entry> {
 public:
  explicit Entry(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<Entry>(info), entry_{nullptr} {
    auto env = info.Env();

    if (!info[0].IsExternal()) {
      throw Napi::Error::New(
          env, "Entry must be constructed internally by another class.");
    }

    try {
      entry_ = std::make_shared<zim::Entry>(
          *info[0].As<Napi::External<zim::Entry>>().Data());
    } catch (const std::exception &err) {
      throw Napi::Error::New(env, err.what());
    }
  }

  static Napi::Object New(Napi::Env env, zim::Entry &&entry) {
    auto external = Napi::External<zim::Entry>::New(env, &entry);
    auto &constructor = env.GetInstanceData<ModuleConstructors>()->entry;
    return constructor.New({external});
  }

  Napi::Value isRedirect(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), entry_->isRedirect());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getTitle(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), entry_->getTitle());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getPath(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), entry_->getPath());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getItem(const Napi::CallbackInfo &info) {
    try {
      if (info[0].IsBoolean()) {  // follow redirect
        return Item::New(info.Env(), entry_->getItem(info[0].ToBoolean()));
      }
      return Item::New(info.Env(), entry_->getItem());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getRedirect(const Napi::CallbackInfo &info) {
    try {
      return Item::New(info.Env(), entry_->getRedirect());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getRedirectEntry(const Napi::CallbackInfo &info) {
    try {
      return Entry::New(info.Env(), entry_->getRedirectEntry());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getIndex(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), entry_->getIndex());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env, "Entry",
        {
            InstanceAccessor<&Entry::isRedirect>("isRedirect"),
            InstanceAccessor<&Entry::getTitle>("title"),
            InstanceAccessor<&Entry::getPath>("path"),
            InstanceAccessor<&Entry::getItem>("item"),
            InstanceMethod<&Entry::getItem>("getItem"),
            InstanceAccessor<&Entry::getRedirect>("redirect"),
            InstanceAccessor<&Entry::getRedirectEntry>("redirectEntry"),
            InstanceAccessor<&Entry::getIndex>("index"),
        });

    exports.Set("Entry", func);
    constructors.entry = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::Entry> entry_;
};
