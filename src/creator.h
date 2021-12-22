#pragma once

#include <napi.h>
#include <zim/writer/creator.h>

#include <exception>
#include <functional>
#include <memory>

#include "common.h"
#include "writerItem.h"

class Creator : public Napi::ObjectWrap<Creator> {
 public:
  explicit Creator(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<Creator>(info), creator_{nullptr} {
    creator_ = std::make_shared<zim::writer::Creator>();
  }

  static Napi::Object New(Napi::Env env) {
    auto &constructor = env.GetInstanceData<ModuleConstructors>()->creator;
    return constructor.New({});
  }

  Napi::Value configVerbose(const Napi::CallbackInfo &info) {
    try {
      creator_->configVerbose(info[0].ToBoolean());
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value configCompression(const Napi::CallbackInfo &info) {
    try {
      auto &&val = Compression::symbolToEnum(info.Env(), info[0]);
      creator_->configCompression(val);
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value configClusterSize(const Napi::CallbackInfo &info) {
    try {
      auto val = info[0].ToNumber().Int64Value();
      if (val < 0) {
        throw Napi::Error::New(info.Env(),
                               "cluster size must be a positive integer.");
      }
      creator_->configClusterSize(val);
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value configIndexing(const Napi::CallbackInfo &info) {
    try {
      if (info.Length() < 2) {
        throw Napi::Error::New(info.Env(),
                               "configIndexing must be called with "
                               "indexing[bool] and language[string]");
      }

      auto indexing = info[0].ToBoolean();
      auto language = info[1].ToString();
      creator_->configIndexing(indexing, language);
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value configNbWorkers(const Napi::CallbackInfo &info) {
    try {
      auto val = info[0].ToNumber();
      creator_->configNbWorkers(val);
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value startZimCreation(const Napi::CallbackInfo &info) {
    try {
      auto val = info[0].ToString();
      creator_->startZimCreation(val);
      // normal api does not return creator but I'm returning it instead of void
      // because it allows a user to chain config and then start all in one
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  void addItem(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      if (!info[0].IsObject()) {
        throw Napi::Error::New(env, "addItem requires an item object");
      }
      auto item = info[0].ToObject();
      creator_->addItem(std::make_shared<ItemWrapper>(env, item));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  void finishZimCreation(const Napi::CallbackInfo &info) {
    try {
      creator_->finishZimCreation();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env, "Creator",
        {
            InstanceMethod<&Creator::configVerbose>("configVerbose"),
            InstanceMethod<&Creator::configCompression>("configCompression"),
            InstanceMethod<&Creator::configClusterSize>("configClusterSize"),
            InstanceMethod<&Creator::configIndexing>("configIndexing"),
            InstanceMethod<&Creator::configNbWorkers>("configNbWorkers"),
            InstanceMethod<&Creator::startZimCreation>("startZimCreation"),
            InstanceMethod<&Creator::addItem>("addItem"),

            InstanceMethod<&Creator::finishZimCreation>("finishZimCreation"),
        });

    exports.Set("Creator", func);
    constructors.creator = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::writer::Creator> creator_;
};

