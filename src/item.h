#pragma once

#include <napi.h>
#include <zim/item.h>
#include <exception>
#include <memory>

#include "blob.h"

class Item : public Napi::ObjectWrap<Item> {
 public:
  explicit Item(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<Item>(info), item_{nullptr} {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!info[0].IsExternal()) {
      throw Napi::Error::New(
          env, "Item must be constructed internally by another class.");
    }

    item_ = std::make_shared<zim::Item>(
        *info[0].As<Napi::External<zim::Item>>().Data());
  }

  static Napi::Object New(Napi::Env env, zim::Item &&item) {
    auto external = Napi::External<zim::Item>::New(env, &item);
    auto &constructor = env.GetInstanceData<ModuleConstructors>()->item;
    return constructor.New({external});
  }

  Napi::Value getTitle(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getTitle());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getPath(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getPath());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getMimetype(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getMimetype());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getData(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      size_t offset = 0;

      // load offset if defined and is Number or BigInt
      if (info[0].IsBigInt()) {
        offset = info[0].As<Napi::BigInt>().Uint64Value(nullptr);
      } else if (info[0].IsNumber()) {
        int64_t val = info[0].ToNumber().Int64Value();
        if (val < 0) {
          throw Napi::Error::New(env,
                                 "Offset must be greater than or equal to 0");
        }
        offset = static_cast<size_t>(val);
      }

      // load size if defined and is Number or BigInt
      if (info.Length() > 1) {
        size_t size;
        if (info[1].IsBigInt()) {
          size = info[1].As<Napi::BigInt>().Uint64Value(nullptr);
        } else if (info[1].IsNumber()) {
          int64_t val = info[1].ToNumber().Int64Value();
          if (val < 0) {
            throw Napi::Error::New(env,
                                   "Size must be greater than or equal to 0");
          }
          size = static_cast<size_t>(val);
        } else {  // fail here because the wrong type defaults to 0
          throw Napi::Error::New(env, "Size must be an Number or BigInt");
        }

        auto blob = item_->getData(offset, size);
        return Blob::New(env, blob);
      }

      auto blob = item_->getData(offset);
      return Blob::New(env, blob);
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getSize(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getSize());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getDirectAccessInformation(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      const auto dai = item_->getDirectAccessInformation();
      auto res = Napi::Object::New(env);
      res["filename"] = Napi::Value::From(env, dai.filename);
      res["offset"] = Napi::Value::From(env, dai.offset);
      res["isValid"] = Napi::Value::From(env, dai.isValid());
      res.Freeze();
      return res;
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getIndex(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getIndex());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func =
        DefineClass(env, "Item",
                    {
                        InstanceAccessor<&Item::getTitle>("title"),
                        InstanceAccessor<&Item::getPath>("path"),
                        InstanceAccessor<&Item::getMimetype>("mimetype"),
                        InstanceAccessor<&Item::getData>("data"),
                        InstanceMethod<&Item::getData>("getData"),
                        InstanceAccessor<&Item::getSize>("size"),
                        InstanceAccessor<&Item::getDirectAccessInformation>(
                            "directAccessInformation"),
                        InstanceAccessor<&Item::getIndex>("index"),
                    });

    exports.Set("Item", func);
    constructors.item = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::Item> item_;
};
