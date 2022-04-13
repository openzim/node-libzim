#pragma once

#include <napi.h>
#include <zim/writer/contentProvider.h>
#include <zim/writer/item.h>

#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string>
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
 * Wraps a JS World Object to IndexData and implements IndexData
 */
class IndexDataWrapper : public zim::writer::IndexData {
 public:
  explicit IndexDataWrapper(const Napi::Object &indexData)
      : hasIndexData_{true},
        title_{},
        content_{},
        keywords_{},
        wordcount_{},
        position_{std::nullopt} {
    if (indexData.Has("hasIndexData"))
      hasIndexData_ = indexData.Get("hasIndexData").ToBoolean();

    if (!hasIndexData_) return;

    if (indexData.Has("title")) title_ = indexData.Get("title").ToString();
    if (indexData.Has("content"))
      content_ = indexData.Get("content").ToString();
    if (indexData.Has("keywords"))
      keywords_ = indexData.Get("keywords").ToString();
    if (indexData.Has("wordcount"))
      wordcount_ = indexData.Get("wordcount").ToNumber();

    if (indexData.Has("position") && indexData.Get("position").IsArray()) {
      auto pos = indexData.Get("position").As<Napi::Array>();
      if (pos.Length() < 3) {
        throw Napi::Error::New(indexData.Env(),
                               "position must have a length of 3");
      }
      position_ =
          std::make_tuple(pos.Get((uint32_t)0).ToBoolean().Value(),
                          pos.Get((uint32_t)1).ToNumber().DoubleValue(),
                          pos.Get((uint32_t)2).ToNumber().DoubleValue());
    }

    hasIndexData_ = !(title_.empty() && content_.empty() && keywords_.empty() &&
                      wordcount_ > 0 && !position_.has_value());
  }

  bool hasIndexData() const override { return hasIndexData_; }

  std::string getTitle() const override { return title_; }
  std::string getContent() const override { return content_; }
  std::string getKeywords() const override { return keywords_; }
  uint32_t getWordCount() const override { return wordcount_; }
  GeoPosition getGeoPosition() const override { return *position_; }

 private:
  bool hasIndexData_;
  std::string title_;
  std::string content_;
  std::string keywords_;
  uint32_t wordcount_;
  std::optional<GeoPosition> position_;
};

/**
 * Wraps a JS World Item to a zim::writer::Item
 */
class ItemWrapper : public zim::writer::Item {
 public:
  ItemWrapper(Napi::Env env, const Napi::Object &item)
      : item_{}, hasIndexDataImpl_{false} {
    item_ = Napi::Persistent(item);

    path_ = item_.Get("path").ToString();
    title_ = item_.Get("title").ToString();
    mimeType_ = item_.Get("mimeType").ToString();

    const auto hasHints = item_.Value().ToObject().Has("hints");
    hints_ = hasHints ? Object2Hints(item_.Get("hints").ToObject())
                      : zim::writer::Item::getHints();

    hasIndexDataImpl_ = item_.Value().ToObject().Has("indexData");
  }

  std::string getPath() const override { return path_; }

  std::string getTitle() const override { return title_; }

  std::string getMimeType() const override { return mimeType_; }

  zim::writer::Hints getHints() const override { return hints_; }

  std::shared_ptr<zim::writer::IndexData> getIndexData() const override {
    if (hasIndexDataImpl_) {
      auto data = item_.Get("indexData");
      if (data.IsObject()) {  // allows for setting indexData to null
        return std::make_shared<IndexDataWrapper>(data.ToObject());
      }
      return nullptr;
    }

    // use default implementation
    return zim::writer::Item::getIndexData();
  }

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
  bool hasIndexDataImpl_;
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

  // TODO(kelvinhammond): implement getIndexData for StringItem and FileItem

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func =
        DefineClass(env, "StringItem",
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

