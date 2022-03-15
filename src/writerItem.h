#pragma once

#include <napi.h>
#include <zim/writer/contentProvider.h>
#include <zim/writer/item.h>

#include <exception>
#include <functional>
#include <future>
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

/**
 * Wraps a JS World Item to a zim::writer::Item
 */
class ItemWrapper : public zim::writer::Item {
 public:
  ItemWrapper(Napi::Env env, const Napi::Object &item) : item_{} {
    item_ = Napi::Persistent(item);

    path_ = item_.Get("path").ToString();
    title_ = item_.Get("title").ToString();
    mimeType_ = item_.Get("mimeType").ToString();
    auto hasHints = item_.Value().ToObject().Has("hints");
    hints_ = hasHints ? Object2Hints(item_.Get("hints").ToObject())
                      : zim::writer::Item::getHints();
  }

  std::string getPath() const override { return path_; }

  std::string getTitle() const override { return title_; }

  std::string getMimeType() const override { return mimeType_; }

  zim::writer::Hints getHints() const override { return hints_; }

  /**
   * ContentProvider is dynamic and must be created on the main javascript
   * thread.
   */
  std::unique_ptr<zim::writer::ContentProvider> getContentProvider()
      const override {
    return std::make_unique<ContentProviderWrapper>(
        item_.Get("contentProvider").ToObject());
  }

 private:
  // js world reference, could be an ObjectWrap provider or custom js object
  Napi::ObjectReference item_;

  std::string path_;
  std::string title_;
  std::string mimeType_;
  zim::writer::Hints hints_;
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

