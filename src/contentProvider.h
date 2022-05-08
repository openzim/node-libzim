#pragma once

#include <napi.h>
#include <zim/writer/contentProvider.h>

#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <string_view>
#include <thread>
#include <utility>

#include "blob.h"
#include "common.h"

/**
 * Wraps the js world ObjectWrap and Objects to a proper content provider for
 * use with libzim
 */
class ContentProviderWrapper : public zim::writer::ContentProvider {
 public:
  explicit ContentProviderWrapper(Napi::Env env, const Napi::Object &provider)
      : MAIN_THREAD_ID{}, provider_{} {
    MAIN_THREAD_ID = std::this_thread::get_id();
    provider_ = Napi::Persistent(provider);
    size_ = parseSize(provider_.Get("size"));

    tsfn_ = Napi::ThreadSafeFunction::New(env, Napi::Function::New<noopCB>(env),
                                          "getContentProvider.feed", 0, 1);
  }

  ~ContentProviderWrapper() { tsfn_.Release(); }

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

  zim::Blob feed() override {
    if (MAIN_THREAD_ID == std::this_thread::get_id()) {
      // on main thread for some reason, do it here
      auto feedFunc = provider_.Get("feed").As<Napi::Function>();
      auto blobObj = feedFunc.Call(provider_.Value(), {});
      if (!blobObj.IsObject()) {
        throw std::runtime_error("ContentProvider.feed must return a blob");
      }
      auto blob = Napi::ObjectWrap<Blob>::Unwrap(blobObj.ToObject());
      return *(blob->blob());
    }

    // called from a thread
    std::promise<zim::Blob> promise;
    auto future = promise.get_future();

    auto callback = [&promise, this](Napi::Env env, Napi::Function) {
      auto feedFunc = provider_.Get("feed").As<Napi::Function>();
      auto blobObj = feedFunc.Call(provider_.Value(), {});
      if (!blobObj.IsObject()) {
        throw std::runtime_error("ContentProvider.feed must return a blob");
      }
      auto blob = Napi::ObjectWrap<Blob>::Unwrap(blobObj.ToObject());
      promise.set_value(*(blob->blob()));
    };

    auto status = tsfn_.BlockingCall(callback);
    if (status != napi_ok) {
      throw std::runtime_error("Error calling ThreadSafeFunction");
    }

    return future.get();
  }

 private:
  // track the main thread
  std::thread::id MAIN_THREAD_ID;
  // js world reference, could be an ObjectWrap provider or custom js object
  Napi::ObjectReference provider_;
  Napi::ThreadSafeFunction tsfn_;
  zim::size_type size_;
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
    try {
      return Napi::Value::From(info.Env(), provider_->getSize());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value feed(const Napi::CallbackInfo &info) {
    try {
      // TODO(kelvinhammond): need a way to move this to avoid copying
      auto blob = provider_->feed();
      return Blob::New(info.Env(), blob);
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
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
    try {
      return Napi::Value::From(info.Env(), provider_->getSize());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value feed(const Napi::CallbackInfo &info) {
    try {
      // TODO(kelvinhammond): need a way to move this to avoid copying
      auto blob = provider_->feed();
      return Blob::New(info.Env(), blob);
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
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

 private:
  std::unique_ptr<zim::writer::FileProvider> provider_;
};

