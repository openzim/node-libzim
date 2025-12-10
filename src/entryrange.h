#pragma once

#include <napi.h>
#include <zim/entry.h>
#include <exception>
#include <memory>

#include "entry.h"

class EntryRange : public Napi::ObjectWrap<EntryRange> {
 public:
  static constexpr const char *ENTRY_RANGE_CONSTRUCTOR_NAME = "EntryRange";

  explicit EntryRange(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<EntryRange>(info) {
    auto env = info.Env();

    if (!info[0].IsExternal()) {
      throw Napi::Error::New(
          env, "EntryRange must be constructed internally by another class.");
    }

    if (info[0].IsExternal()) {
      try {
        entry_ = std::make_shared<zim::Entry>(
            *info[0].As<Napi::External<zim::Entry>>().Data());
      } catch (const std::exception &err) {
        throw Napi::Error::New(env, err.what());
      }
    }
  }

  template <typename RangeT>
  static Napi::Object New(Napi::Env env, RangeT range) {
    Napi::Function iterator = Napi::Function::New(
        env, [range](const Napi::CallbackInfo &info) mutable -> Napi::Value {
          Napi::Env env = info.Env();
          Napi::Object iter = Napi::Object::New(env);

          auto it = range.begin();
          iter["next"] = Napi::Function::New(
              env,
              [range,
               it](const Napi::CallbackInfo &info) mutable -> Napi::Value {
                Napi::Env env = info.Env();
                Napi::Object res = Napi::Object::New(env);
                if (it != range.end()) {
                  res["done"] = false;
                  res["value"] = Entry::New(env, zim::Entry(*it));
                  it++;
                } else {
                  res["done"] = true;
                }
                return res;
              });
          return iter;
        });

    auto offset = Napi::Function::New(
        env, [range](const Napi::CallbackInfo &info) -> Napi::Value {
          if (info.Length() < 2) {
            throw Napi::Error::New(
                info.Env(), "start and maxResults are required for offset.");
          }
          if (!(info[0].IsNumber() && info[1].IsNumber())) {
            throw Napi::Error::New(
                info.Env(), "start and maxResults must be of type Number.");
          }
          auto start = info[0].ToNumber();
          auto maxResults = info[1].ToNumber();
          return NewEntryRange(info.Env(), range.offset(start, maxResults));
        });

    auto size = Napi::Value::From(env, range.size());

    auto &constructor = env.GetInstanceData<ConstructorsMap>()->at(
        ENTRY_RANGE_CONSTRUCTOR_NAME);
    return constructor.New({iterator, size, offset});
  }

  Napi::Value getSize(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), entry_->getIndex());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ConstructorsMap &constructors) {
    Napi::Function func =
        DefineClass(env, "EntryRange",
                    {
                        InstanceAccessor<&EntryRange::getSize>("size"),
                        InstanceMethod<&EntryRange::getItem>("getItem"),
                    });

    exports.Set("EntryRange", func);
    constructors.insert_or_assign(ENTRY_RANGE_CONSTRUCTOR_NAME,
                                  Napi::Persistent(func));
  }

 private:
  std::shared_ptr<zim::Entry> entry_;
};
