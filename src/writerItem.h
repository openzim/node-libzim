#pragma once

#include <napi.h>
#include <zim/writer/contentProvider.h>
#include <zim/writer/item.h>

#include <exception>
#include <functional>
#include <memory>
#include <string_view>

#include "blob.h"
#include "common.h"
#include "contentProvider.h"

zim::writer::Hints Object2Hints(const Napi::Object &obj) {
  zim::writer::Hints hints{};
  if (obj.Has("COMPRESS")) {
    hints[zim::writer::COMPRESS] = obj["COMPRESS"].ToNumber().Int64Value();
  }

  if (obj.Has("FRONT_ARTICLE")) {
    hints[zim::writer::FRONT_ARTICLE] =
        obj["FRONT_ARTICLE"].ToNumber().Int64Value();
  }

  return hints;
}

// This was way more complicated than expected. Cannot call back into JS context
// easily.
using Context = Napi::ObjectReference;
using DataType = std::unique_ptr<ContentProviderWrapper>;
void Callback(Napi::Env env, Napi::Function jsCallback, Context *ctx,
              DataType *data) {
  std::cout << "Hello world" << std::endl;
  if (env != nullptr) {
    if (ctx != nullptr) {
      auto &item_ = *ctx;
      *data = std::make_unique<ContentProviderWrapper>(
          item_.Get("contentProvider").ToObject());
      std::cout << "data == nullptr: " << (*data == nullptr) << std::endl;
    }
  }
}
using TSFN = Napi::TypedThreadSafeFunction<Context, DataType, Callback>;

/**
 * Wraps a JS World Item to a zim::writer::Item
 */
class ItemWrapper : public zim::writer::Item {
 public:
  ItemWrapper(Napi::Env env, const Napi::Object &item) : item_{}, tsfn_{} {
    item_ = Napi::Reference<Napi::Object>::New(item, 1);
    tsfn_ = TSFN::New(env, Napi::Function(), "ItemWrapper.getContentProvider",
                      0, 1, &item_);
  }

  std::string getPath() const override { return item_.Get("path").ToString(); }

  std::string getTitle() const override {
    return item_.Get("title").ToString();
  }

  std::string getMimeType() const override {
    return "plain/text";
    // return item_.Get("mimeType").ToString();
  }

  std::unique_ptr<zim::writer::ContentProvider> getContentProvider()
      const override {
    // return std::make_unique<zim::writer::StringProvider>("hello world");

    std::unique_ptr<ContentProviderWrapper> ptr;

    auto status = tsfn_.BlockingCall(&ptr);
    if (status != napi_ok) {
      throw std::runtime_error("Failed to call to nodejs");
    }
    std::cout << "ptr == nullptr: " << (ptr == nullptr) << std::endl;
    // return ptr;
    /*
    return std::make_unique<ContentProviderWrapper>(
        item_.Get("getContentProvider")
            .As<Napi::Function>()
            .MakeCallback(item_.Value(), {})
            .ToObject());
            */

    return std::make_unique<ContentProviderWrapper>(
        item_.Get("contentProvider").ToObject());
  }

  zim::writer::Hints getHints() const override {
    /*
  if (item_.Value().ToObject().Has("hints")) {
    return Object2Hints(item_.Get("hints").ToObject());
  }
  */
    return zim::writer::Item::getHints();
  }

 private:
  // js world reference, could be an ObjectWrap provider or custom js object
  Napi::ObjectReference item_;
  TSFN tsfn_;
};

class StringItem : public Napi::ObjectWrap<StringItem> {
 public:
  explicit StringItem(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<StringItem>(info), item_{nullptr} {
    auto env = info.Env();
    if (info.Length() < 5) {
      throw Napi::Error::New(env,
                             "StringItem requires arguments (path, mimetype, "
                             "title, hints, content)");
    }

    try {
      auto path = info[0].ToString();
      auto mimetype = info[1].ToString();
      auto title = info[2].ToString();
      auto hints = Object2Hints(info[3].ToObject());
      auto content = info[4].ToString();
      item_ = zim::writer::StringItem::create(path, mimetype, title, hints,
                                              content);
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
  }

  Napi::Value getPath(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getPath());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getTitle(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getTitle());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getMimeType(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getMimeType());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getHints(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      auto hints = item_->getHints();
      auto obj = Napi::Object::New(env);
      if (hints.find(zim::writer::COMPRESS) != hints.end()) {
        obj.Set("COMPRESS",
                Napi::Value::From(env, hints[zim::writer::COMPRESS]));
      }
      if (hints.find(zim::writer::FRONT_ARTICLE) != hints.end()) {
        obj.Set("FRONT_ARTICLE",
                Napi::Value::From(env, hints[zim::writer::FRONT_ARTICLE]));
      }
      return obj;
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getContentProvider(const Napi::CallbackInfo &info) {
    try {
      return StringProvider::New(info.Env(), item_->getContentProvider());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env, "StringItem",
        {
            InstanceAccessor<&StringItem::getPath>("path"),
            InstanceAccessor<&StringItem::getTitle>("title"),
            InstanceAccessor<&StringItem::getMimeType>("mimeType"),
            InstanceAccessor<&StringItem::getContentProvider>(
                "contentProvider"),
            InstanceAccessor<&StringItem::getHints>("hints"),

            // used by ItemWrapper for MakeCallback function references
            InstanceMethod<&StringItem::getPath>("getPath"),
            InstanceMethod<&StringItem::getTitle>("getTitle"),
            InstanceMethod<&StringItem::getMimeType>("getMimeType"),
            InstanceMethod<&StringItem::getContentProvider>(
                "getContentProvider"),
        });

    exports.Set("StringItem", func);
    constructors.stringItem = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::writer::StringItem> item_;
};

class FileItem : public Napi::ObjectWrap<FileItem> {
 public:
  explicit FileItem(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<FileItem>(info), item_{nullptr} {
    auto env = info.Env();
    if (info.Length() < 5) {
      throw Napi::Error::New(env,
                             "FileItem requires arguments (path, mimetype, "
                             "title, hints, content)");
    }

    try {
      auto path = info[0].ToString();
      auto mimetype = info[1].ToString();
      auto title = info[2].ToString();
      auto hints = Object2Hints(info[3].ToObject());
      auto filepath = info[4].ToString();
      item_ = std::make_shared<zim::writer::FileItem>(path, mimetype, title,
                                                      hints, filepath);
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
  }

  Napi::Value getPath(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getPath());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getTitle(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getTitle());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getMimeType(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), item_->getMimeType());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getHints(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      auto hints = item_->getHints();
      auto obj = Napi::Object::New(env);
      if (hints.find(zim::writer::COMPRESS) != hints.end()) {
        obj.Set("COMPRESS",
                Napi::Value::From(env, hints[zim::writer::COMPRESS]));
      }
      if (hints.find(zim::writer::FRONT_ARTICLE) != hints.end()) {
        obj.Set("FRONT_ARTICLE",
                Napi::Value::From(env, hints[zim::writer::FRONT_ARTICLE]));
      }
      return obj;
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getContentProvider(const Napi::CallbackInfo &info) {
    try {
      return StringProvider::New(info.Env(), item_->getContentProvider());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env, "FileItem",
        {
            InstanceAccessor<&FileItem::getPath>("path"),
            InstanceAccessor<&FileItem::getTitle>("title"),
            InstanceAccessor<&FileItem::getMimeType>("mimeType"),
            InstanceAccessor<&FileItem::getContentProvider>("contentProvider"),
            InstanceAccessor<&FileItem::getHints>("hints"),
        });

    exports.Set("FileItem", func);
    constructors.fileItem = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::writer::FileItem> item_;
};

/*
class WriterItem : public Napi::ObjectWrap<WriterItem> {
 public:
  explicit WriterItem(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<WriterItem>(info), provider_{nullptr} {
    auto env = info.Env();
    if (info.Length() < 1) {
      throw Napi::Error::New(
          env, "StringProvider requires an argument for a string.");
    }

    try {
      auto str = info[0].ToString();  // value is coerced to a js string.
      provider_ = std::make_shared<zim::writer::StringProvider>(str);
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
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
  }

 private:
  std::shared_ptr<zim::writer::StringProvider> provider_;
};
*/

