#pragma once

#include <napi.h>
#include <zim/illustration.h>
#include <zim/writer/creator.h>

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "common.h"
#include "illustration.h"
#include "writerItem.h"

// Handles creator_->finishZimCreation() operations in the background off the
// main thread.
class CreatorAsyncWorker : public Napi::AsyncWorker {
 public:
  CreatorAsyncWorker(std::shared_ptr<zim::writer::Creator> creator,
                     Napi::Promise::Deferred promise)
      : Napi::AsyncWorker(promise.Env()),
        creator_{creator},
        promise_{promise} {}

  ~CreatorAsyncWorker() {}

  void Execute() override {
    try {
      creator_->finishZimCreation();
    } catch (const std::exception &e) {
      std::cerr << "Error: finishZimCreation failed: " << e.what() << std::endl;
      SetError(e.what());
    }
  }

  void OnOK() override {
    auto env = Env();
    promise_.Resolve(env.Undefined());
  }

  void OnError(const Napi::Error &error) override {
    promise_.Reject(error.Value());
  }

 private:
  std::shared_ptr<zim::writer::Creator> creator_;
  Napi::Promise::Deferred promise_;
};

class AddItemAsyncWorker : public Napi::AsyncWorker {
 public:
  AddItemAsyncWorker(Napi::Env &env,
                     std::shared_ptr<zim::writer::Creator> creator,
                     std::shared_ptr<zim::writer::Item> item)
      : Napi::AsyncWorker(env),
        creator_{creator},
        item_{item},
        promise_(Napi::Promise::Deferred::New(env)) {}

  ~AddItemAsyncWorker() {}

  Napi::Promise Promise() const { return promise_.Promise(); };

  void Execute() override {
    try {
      creator_->addItem(item_);
    } catch (const std::exception &e) {
      std::cerr << "Error: AddItemAsyncWorker failed: " << e.what()
                << std::endl;
      SetError(e.what());
    }
  }

  void OnOK() override {
    auto env = Env();
    promise_.Resolve(env.Undefined());
  }

  void OnError(const Napi::Error &error) override {
    promise_.Reject(error.Value());
  }

 private:
  std::shared_ptr<zim::writer::Creator> creator_;
  std::shared_ptr<zim::writer::Item> item_;
  Napi::Promise::Deferred promise_;
};

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
      // normal api does not return creator but I'm returning it instead of
      // void because it allows a user to chain config and then start all in
      // one
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value addItem(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      if (!info[0].IsObject()) {
        throw Napi::Error::New(env, "addItem requires an item object");
      }

      std::shared_ptr<zim::writer::Item> item{};
      auto obj = info[0].As<Napi::Object>();
      if (StringItem::InstanceOf(env, obj)) {
        item = Napi::ObjectWrap<StringItem>::Unwrap(obj)->getItem();
      } else if (FileItem::InstanceOf(env, obj)) {
        item = Napi::ObjectWrap<FileItem>::Unwrap(obj)->getItem();
      } else {
        item = std::make_shared<ItemWrapper>(env, info[0].ToObject());
      }

      if (item == nullptr) {
        // should not get here
        throw Napi::Error::New(env, "addItem failed to created an item");
      }

      auto wk = new AddItemAsyncWorker(env, creator_, item);
      wk->Queue();
      return wk->Promise();

    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value finishZimCreation(const Napi::CallbackInfo &info) {
    try {
      // Start this in a thread using Napi::AsyncWorker
      // pass a promise to the async worker and return it here
      Napi::Promise::Deferred deferred =
          Napi::Promise::Deferred::New(info.Env());

      auto wk = new CreatorAsyncWorker(creator_, deferred);
      wk->Queue();
      return deferred.Promise();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  void addMetadata(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      if (info.Length() < 2) {
        throw Napi::Error::New(env,
                               "addMetadata requests arguments (name, "
                               "content|provider, [mimetype]).");
      }

      auto name = info[0].ToString().Utf8Value();
      auto content = info[1];
      if (content.IsObject()) {
        // addMetadata(name: string, content: ContentProvider, [mimetype])
        auto obj = content.As<Napi::Object>();
        std::unique_ptr<zim::writer::ContentProvider> provider{nullptr};

        // Determine the type of ContentProvider
        if (StringProvider::InstanceOf(env, content)) {
          auto wrapped = Napi::ObjectWrap<StringProvider>::Unwrap(obj);
          provider = wrapped->unwrapProvider();
        } else if (FileProvider::InstanceOf(env, content)) {
          auto wrapped = Napi::ObjectWrap<FileProvider>::Unwrap(obj);
          provider = wrapped->unwrapProvider();
        } else {
          // Fallback to generic ContentProviderWrapper
          provider = std::make_unique<ContentProviderWrapper>(env, obj);
        }

        if (provider == nullptr) {
          throw Napi::Error::New(
              env, "addMetadata failed to create ContentProvider from object");
        }

        if (info.Length() > 2) {  // preserves default argument
          auto mimetype = info[2].ToString().Utf8Value();
          creator_->addMetadata(name, std::move(provider), mimetype);
        } else {
          const std::string mimetype = "text/plain;charset=utf-8";
          creator_->addMetadata(name, std::move(provider), mimetype);
        }
      } else {  // addMetadata(name: string, content: string, [mimetype])
        auto str = content.ToString().Utf8Value();
        if (info.Length() > 2) {
          auto mimetype = info[2].ToString().Utf8Value();
          creator_->addMetadata(name, str, mimetype);
        } else {
          creator_->addMetadata(name, str);
        }
      }
    } catch (const std::exception &err) {
      std::cerr << "Error: addMetadata failed: " << err.what() << std::endl;
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  void addIllustration(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();

      // Inline template function to handle both size and IllustrationInfo
      const auto addIllusWithContent = [&](auto &opt1) {
        auto content = info[1];
        if (content.IsObject()) {
          std::unique_ptr<zim::writer::ContentProvider> provider =
              std::make_unique<ContentProviderWrapper>(env, content.ToObject());
          creator_->addIllustration(opt1, std::move(provider));
        } else {
          auto str = content.ToString().Utf8Value();
          creator_->addIllustration(opt1, str);
        }
      };

      auto arg0 = info[0];
      if (arg0.IsNumber()) {
        auto size = arg0.ToNumber().Uint32Value();
        addIllusWithContent(size);
      } else if (arg0.IsObject()) {
        // Parse as IllustrationInfo
        auto obj = arg0.ToObject();

        // getIllustrationItem(illusInfo: IllustrationInfo)
        // getIllustrationItem(illusInfo: object)
        auto illusInfo =
            IllustrationInfo::InstanceOf(env, obj)
                ? IllustrationInfo::Unwrap(obj)->getInternalIllustrationInfo()
                : IllustrationInfo::infoFrom(obj);
        addIllusWithContent(illusInfo);
      } else {
        throw Napi::Error::New(
            env,
            "addIllustration first argument must be size[number] or "
            "IllustrationInfo[object]");
      }
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  void addRedirection(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      if (info.Length() < 3) {
        throw Napi::Error::New(env,
                               "addRedirection requires arguments: path, "
                               "title, targetPath, [hints]");
      }

      auto path = info[0].As<Napi::String>().Utf8Value();
      auto title = info[1].As<Napi::String>().Utf8Value();
      auto targetPath = info[2].As<Napi::String>().Utf8Value();
      if (info[3].IsObject()) {
        auto hints = Object2Hints(info[3].ToObject());
        creator_->addRedirection(path, title, targetPath, hints);
      } else {
        creator_->addRedirection(path, title, targetPath);
      }
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  void setMainPath(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      if (!info[0].IsString()) {
        throw Napi::Error::New(
            env, "setMainPath requires a string argument: mainPath");
      }
      auto mainPath = info[0].ToString().Utf8Value();
      creator_->setMainPath(mainPath);
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  void setUuid(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      if (!info[0].IsString()) {
        throw Napi::Error::New(env, "setUuid requires a string argument: uuid");
      }
      auto uuid = info[0].ToString().Utf8Value();
      if (uuid.size() != 16) {
        throw Napi::Error::New(env, "uuid must be 16 characters long");
      }
      creator_->setUuid(uuid.c_str());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
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
            InstanceMethod<&Creator::addMetadata>("addMetadata"),
            InstanceMethod<&Creator::addIllustration>("addIllustration"),
            InstanceMethod<&Creator::addRedirection>("addRedirection"),
            InstanceMethod<&Creator::setMainPath>("setMainPath"),
            InstanceMethod<&Creator::setUuid>("setUuid"),

            InstanceMethod<&Creator::finishZimCreation>("finishZimCreation"),
        });

    exports.Set("Creator", func);
    constructors.creator = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::writer::Creator> creator_;
};

