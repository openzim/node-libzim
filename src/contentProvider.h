#pragma once

#include <napi.h>
#include <zim/writer/contentProvider.h>

#include <exception>
#include <functional>
#include <memory>
#include <string_view>

#include "blob.h"
#include "common.h"

/**
 * Wraps the js world ObjectWrap and Objects to a proper content provider for
 * use with libzim
 */
class ContentProviderWrapper : public zim::writer::ContentProvider {
 public:
  ContentProviderWrapper(const Napi::Object &provider) : provider_{} {
    provider_ = Napi::Reference<Napi::Object>::New(provider, 1);
  }

  zim::size_type getSize() const override {
    auto size = provider_.Get("size");
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
    auto feedFunc = provider_.Get("feed").As<Napi::Function>();
    auto blobObj = feedFunc.Call(provider_.Value(), {});
    if (!blobObj.IsObject()) {
      throw std::runtime_error("ContentProvider.feed must return a blob");
    }
    auto blob = Napi::ObjectWrap<Blob>::Unwrap(blobObj.ToObject());
    // blob is a Blob wrapper which returns a shared_ptr
    return *(blob->blob());
  }

 private:
  // js world reference, could be an ObjectWrap provider or custom js object
  Napi::ObjectReference provider_;
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
      // TODO: need a way to move this to avoid copying
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

    auto testFunc = [](const Napi::CallbackInfo &info) -> Napi::Value {
      auto env = info.Env();
      auto provider = info[0].ToObject();
      std::unique_ptr<zim::writer::ContentProvider> pw =
          std::make_unique<ContentProviderWrapper>(provider);
      std::cout << "size: " << pw->getSize() << std::endl;
      {
        auto feed = pw->feed();
        std::cout << "feed.size: " << feed.size() << std::endl;
        std::cout << "feed.data: " << feed.data() << std::endl;
      }
      std::cout << "feed.size: " << pw->feed().size() << std::endl;
      std::cout << "feed.size: " << pw->feed().size() << std::endl;
      return env.Null();
    };
    exports.Set("testFunc", Napi::Function::New(env, testFunc));
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
      // TODO: need a way to move this to avoid copying
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

/*
class ContentProvider : public Napi::ObjectWrap<ContentProvider> {
 public:
  explicit ContentProvider(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<ContentProvider>(info) {
  }

  static Napi::Object New(Napi::Env env) {
    auto &constructor =
        env.GetInstanceData<ModuleConstructors>()->contentprovider;
    return constructor.New({});
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env, "ContentProvider",
        {
            InstanceMethod<&Creator::configVerbose>("configVerbose"),
        });

    exports.Set("ContentProvider", func);
    constructors.contentprovider = Napi::Persistent(func);
  }

 private:
  Napi::ObjectReference providerRef_;
};
*/

