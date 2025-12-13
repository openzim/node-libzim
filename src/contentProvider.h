#pragma once

#include <napi.h>
#include <zim/writer/contentProvider.h>

#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include "blob.h"
#include "common.h"

/**
 * Thread Safe Function wrapper for calling the ContentProvider.feed function
 * asynchronously from libzim
 */
class FeedTSFN {
 public:
  using BlobPtr = zim::Blob;

  FeedTSFN() = delete;

  FeedTSFN(Napi::Env &env, Napi::Function &feedFunc) {
    tsfn_ = TSFN::New(env,
                      feedFunc,    // JavaScript function called asynchronously
                      "FeedTSFN",  // name
                      0,           // max queue size (0 = unlimited).
                      1,           // initial thread count
                      nullptr);    // context
  }

  ~FeedTSFN() { tsfn_.Release(); }

  BlobPtr feed() {
    try {
      DataType promise;
      auto future = promise.get_future();
      tsfn_.NonBlockingCall(&promise);
      return future.get();
    } catch (const std::exception &e) {
      std::cerr << "FeedTSFN feed() exception: " << e.what() << std::endl;
      throw std::runtime_error(std::string("Error in FeedTSFN feed(): ") +
                               e.what());
    }
  }

 private:
  using DataType = std::promise<BlobPtr>;
  using Context = void;

  static void CallJs(Napi::Env env, Napi::Function callback, Context *context,
                     DataType *data) {
    // Is the JavaScript environment still available to call into, eg. the TSFN
    // is not aborted
    if (env != nullptr) {
      try {
        // call feed(): object
        auto result = callback.Call({});
        if (Blob::InstanceOf(env, result)) {
          auto blob = Blob::Unwrap(result.As<Napi::Object>())->blob();
          // Note: Cannot move, blob could be used in nodejs world still
          data->set_value(blob);
        } else {
          data->set_exception(std::make_exception_ptr(std::runtime_error(
              "Expected an object of type Blob from feed()")));
        }
      } catch (const std::exception &e) {
        data->set_exception(std::make_exception_ptr(e));
      }
    } else {
      data->set_exception(std::make_exception_ptr(
          std::runtime_error("Environment is shut down")));
    }
  }

  using TSFN = Napi::TypedThreadSafeFunction<Context, DataType, CallJs>;

 private:
  TSFN tsfn_;
};

/**
 * Wraps the js world ObjectWrap and Objects to a proper content provider for
 * use with libzim
 */
class ContentProviderWrapper : public zim::writer::ContentProvider {
 public:
  explicit ContentProviderWrapper(Napi::Env env, const Napi::Object &provider) {
    size_ = parseSize(provider.Get("size"));

    if (!provider.Get("feed").IsFunction()) {
      throw std::runtime_error("ContentProvider.feed must be a function.");
    }

    auto feedFunc = provider.Get("feed").As<Napi::Function>();
    feedTSFN_ = std::make_unique<FeedTSFN>(env, feedFunc);
  }

  zim::size_type getSize() const override { return size_; }

  /** Parse the size, supports BigInt */
  static zim::size_type parseSize(const Napi::Value size) {
    if (size.IsBigInt()) {
      bool lossless;
      auto &&val = size.As<Napi::BigInt>().Uint64Value(&lossless);
      if (!lossless) {
        throw std::runtime_error(
            "size was not converted to a uint64_t losslessly");
      }
      return val;
    }

    auto val = size.ToNumber().Int64Value();
    if (val < 0) {
      throw std::runtime_error("Size must be greater than 0");
    }
    return static_cast<uint64_t>(val);
  }

  zim::Blob feed() override { return feedTSFN_->feed(); }

 private:
  zim::size_type size_;
  // Unique pointer so that it isn't copied and the destructor will only be
  // called once
  std::unique_ptr<FeedTSFN> feedTSFN_;
};

class StringProvider : public Napi::ObjectWrap<StringProvider> {
 public:
  explicit StringProvider(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<StringProvider>(info), provider_{nullptr} {
    auto env = info.Env();
    if (info.Length() < 1) {
      throw Napi::Error::New(
          env, "StringProvider requires an argument for a string.");
    }

    try {
      if (info[0].IsExternal()) {
        using ProviderPtr = std::unique_ptr<zim::writer::StringProvider>;
        auto &&ptr = *info[0].As<Napi::External<ProviderPtr>>().Data();
        provider_ = std::move(ptr);
      } else {
        auto str = info[0].ToString();  // value is coerced to a js string.
        provider_ = std::make_unique<zim::writer::StringProvider>(str);
      }
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
  }

  static Napi::Object New(
      Napi::Env env,
      std::unique_ptr<zim::writer::ContentProvider> stringProvider) {
    auto &constructor =
        env.GetInstanceData<ModuleConstructors>()->stringProvider;
    auto external =
        Napi::External<decltype(stringProvider)>::New(env, &stringProvider);
    return constructor.New({external});
  }

  static Napi::Object New(Napi::Env env, const std::string_view value) {
    auto &constructor =
        env.GetInstanceData<ModuleConstructors>()->stringProvider;
    auto str = Napi::String::New(env, value.data(), value.size());
    return constructor.New({str});
  }

  Napi::Value getSize(const Napi::CallbackInfo &info) {
    if (!provider_) {
      throw Napi::Error::New(
          info.Env(), "StringProvider has been moved and is no longer valid.");
    }

    try {
      return Napi::Value::From(info.Env(), provider_->getSize());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value feed(const Napi::CallbackInfo &info) {
    if (!provider_) {
      throw Napi::Error::New(
          info.Env(), "StringProvider has been moved and is no longer valid.");
    }

    try {
      auto blob = provider_->feed();
      return Blob::New(info.Env(), blob);
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
    return env.GetInstanceData<ModuleConstructors>()->stringProvider;
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func =
        DefineClass(env, "StringProvider",
                    {
                        InstanceAccessor<&StringProvider::getSize>("size"),
                        InstanceMethod<&StringProvider::getSize>("getSize"),
                        InstanceMethod<&StringProvider::feed>("feed"),
                    });

    exports.Set("StringProvider", func);
    constructors.stringProvider = Napi::Persistent(func);
  }

  // Internal use only
  std::unique_ptr<zim::writer::StringProvider> &&unwrapProvider() {
    return std::move(provider_);
  }

 private:
  std::unique_ptr<zim::writer::StringProvider> provider_;
};

class FileProvider : public Napi::ObjectWrap<FileProvider> {
 public:
  explicit FileProvider(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<FileProvider>(info), provider_{nullptr} {
    auto env = info.Env();
    if (info.Length() < 1) {
      throw Napi::Error::New(
          env, "FileProvider requires an argument for a filepath.");
    }

    try {
      if (info[0].IsExternal()) {
        using ProviderPtr = std::unique_ptr<zim::writer::FileProvider>;
        auto &&ptr = *info[0].As<Napi::External<ProviderPtr>>().Data();
        provider_ = std::move(ptr);
      } else {
        auto filepath = info[0].ToString();  // value is coerced to a js string.
        provider_ = std::make_unique<zim::writer::FileProvider>(filepath);
      }
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
  }

  static Napi::Object New(
      Napi::Env env,
      std::unique_ptr<zim::writer::ContentProvider> fileProvider) {
    auto &constructor = env.GetInstanceData<ModuleConstructors>()->fileProvider;
    auto external =
        Napi::External<decltype(fileProvider)>::New(env, &fileProvider);
    return constructor.New({external});
  }

  static Napi::Object New(Napi::Env env, const std::string_view filepath) {
    auto &constructor = env.GetInstanceData<ModuleConstructors>()->fileProvider;
    auto path = Napi::String::New(env, filepath.data(), filepath.size());
    return constructor.New({path});
  }

  Napi::Value getSize(const Napi::CallbackInfo &info) {
    if (!provider_) {
      throw Napi::Error::New(
          info.Env(), "FileProvider has been moved and is no longer valid.");
    }

    try {
      return Napi::Value::From(info.Env(), provider_->getSize());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value feed(const Napi::CallbackInfo &info) {
    if (!provider_) {
      throw Napi::Error::New(
          info.Env(), "FileProvider has been moved and is no longer valid.");
    }

    try {
      auto blob = provider_->feed();
      return Blob::New(info.Env(), blob);
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
    return env.GetInstanceData<ModuleConstructors>()->fileProvider;
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func =
        DefineClass(env, "FileProvider",
                    {
                        InstanceAccessor<&FileProvider::getSize>("size"),
                        InstanceMethod<&FileProvider::getSize>("getSize"),
                        InstanceMethod<&FileProvider::feed>("feed"),
                    });

    exports.Set("FileProvider", func);
    constructors.fileProvider = Napi::Persistent(func);
  }

  // Internal use only
  std::unique_ptr<zim::writer::FileProvider> &&unwrapProvider() {
    return std::move(provider_);
  }

 private:
  std::unique_ptr<zim::writer::FileProvider> provider_;
};

