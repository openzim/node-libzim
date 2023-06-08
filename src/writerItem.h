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
#include <thread>

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
 *
 * NOTE: should be initialized on the main thread
 */
class ItemWrapper : public zim::writer::Item {
 public:
  ItemWrapper(Napi::Env env, Napi::Object item)
      : MAIN_THREAD_ID{}, item_{}, hasIndexDataImpl_{false} {
    MAIN_THREAD_ID = std::this_thread::get_id();
    item_ = Napi::Persistent(item);

    path_ = item.Get("path").ToString();
    title_ = item.Get("title").ToString();
    mimeType_ = item.Get("mimeType").ToString();

    const auto hasHints = item.Has("hints");
    hints_ = hasHints ? Object2Hints(item.Get("hints").ToObject())
                      : zim::writer::Item::getHints();

    hasIndexDataImpl_ = item.Has("hasIndexData");

    if (hasIndexDataImpl_) {
      auto indexDataFuncValue = item.Get("getIndexData");
      if (!indexDataFuncValue.IsFunction()) {
        throw std::runtime_error("getIndexData must be a function");
      }

      auto indexDataFunc = indexDataFuncValue.As<Napi::Function>();
      indexDataFunc_ = Napi::Persistent(indexDataFunc);
      indexDataTSNF_ = Napi::ThreadSafeFunction::New(
          env, indexDataFunc, "ItemWrapper.indexData", 0, 1);
    }

    auto providerFuncValue = item.Get("getContentProvider");
    if (!providerFuncValue.IsFunction()) {
      throw std::runtime_error("getContentProvider must be a function");
    }
    auto providerFunc = providerFuncValue.As<Napi::Function>();
    contentProviderFunc_ = Napi::Persistent(providerFunc);
    contentProviderTSNF_ = Napi::ThreadSafeFunction::New(
        env, providerFunc, "ItemWrapper.contentProvider", 5, 1);
  }

  ~ItemWrapper() {
    if (hasIndexDataImpl_) {
      indexDataTSNF_.Release();
    }
    contentProviderTSNF_.Release();
  }

  std::string getPath() const override { return path_; }

  std::string getTitle() const override { return title_; }

  std::string getMimeType() const override { return mimeType_; }

  zim::writer::Hints getHints() const override { return hints_; }

  std::shared_ptr<zim::writer::IndexData> getIndexData() const override {
    if (!hasIndexDataImpl_) {
      // use default implementation
      return zim::writer::Item::getIndexData();
    }

    if (MAIN_THREAD_ID == std::this_thread::get_id()) {
      auto data = indexDataFunc_.Call(item_.Value(), {});
      return data.IsObject()
                 ? std::make_shared<IndexDataWrapper>(data.ToObject())
                 : nullptr;
    }

    // called from a thread
    using IndexDataWrapperPtr = std::shared_ptr<IndexDataWrapper>;
    std::promise<IndexDataWrapperPtr> promise;
    auto future = promise.get_future();

    auto callback = [&promise, this](Napi::Env env, Napi::Function idxFunc) {
      auto data = idxFunc.Call(item_.Value(), {});
      promise.set_value(
          data.IsObject() ? std::make_shared<IndexDataWrapper>(data.ToObject())
                          : nullptr);
    };

    auto status = indexDataTSNF_.BlockingCall(callback);
    if (status != napi_ok) {
      throw std::runtime_error("Error calling indexData ThreadSafeFunction");
    }

    return future.get();
  }

  /**
   * ContentProvider is dynamic and must be created on the main javascript
   * thread.
   */
  std::unique_ptr<zim::writer::ContentProvider> getContentProvider()
      const override {
    if (MAIN_THREAD_ID == std::this_thread::get_id()) {
      auto env = contentProviderFunc_.Env();
      auto provider = contentProviderFunc_.Call(item_.Value(), {});
      if (provider.IsObject()) {
        return std::make_unique<ContentProviderWrapper>(env,
                                                        provider.ToObject());
      } else if (provider.IsNull() || provider.IsUndefined()) {
        return nullptr;
      }

      throw std::runtime_error(
          "getContentProvider must return an object or null");
    }

    using ContentProviderWrapperPtr = std::unique_ptr<ContentProviderWrapper>;
    std::promise<ContentProviderWrapperPtr> promise;
    auto future = promise.get_future();

    auto callback = [&promise, this](Napi::Env env,
                                     Napi::Function providerFunc) {
      auto provider = providerFunc.Call(item_.Value(), {});
      if (provider.IsObject()) {
        auto ptr =
            std::make_unique<ContentProviderWrapper>(env, provider.ToObject());
        promise.set_value(std::move(ptr));
      } else if (provider.IsNull() || provider.IsUndefined()) {
        promise.set_value(nullptr);
      } else {
        throw std::runtime_error(
            "getContentProvider must return an object or null");
      }
    };

    auto status = contentProviderTSNF_.BlockingCall(callback);
    if (status != napi_ok) {
      throw std::runtime_error(
          "Error calling contentProvider ThreadSafeFunction");
    }

    return future.get();
  }

 private:
  std::thread::id MAIN_THREAD_ID;
  // js world reference, could be an ObjectWrap provider or custom js object
  Napi::ObjectReference item_;

  std::string path_;
  std::string title_;
  std::string mimeType_;
  zim::writer::Hints hints_;
  bool hasIndexDataImpl_;

  Napi::FunctionReference indexDataFunc_;
  Napi::ThreadSafeFunction indexDataTSNF_;
  Napi::FunctionReference contentProviderFunc_;
  Napi::ThreadSafeFunction contentProviderTSNF_;
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
      auto path = info[0].ToString().Utf8Value();
      auto mimetype = info[1].ToString().Utf8Value();
      auto title = info[2].ToString().Utf8Value();
      auto hints = Object2Hints(info[3].ToObject());
      std::string content;

      if (info[0].IsArrayBuffer()) {  // handle ArrayBuffer
        auto buf = info[0].As<Napi::ArrayBuffer>();
        auto size = buf.ByteLength();
        content = std::string(reinterpret_cast<const char *>(buf.Data()), size);
      } else if (info[0].IsBuffer()) {  // handle Buffer
        auto buf = info[0].As<Napi::Buffer<char>>();
        auto size = buf.Length();
        content = std::string(reinterpret_cast<const char *>(buf.Data()), size);
      } else {
        content = info[4].ToString().Utf8Value();
      }

      item_ = zim::writer::StringItem::create(path, mimetype, title, hints,
                                              content);
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
  }

  std::shared_ptr<zim::writer::StringItem> getItem() { return item_; }

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
                        InstanceMethod<&StringItem::getContentProvider>(
                            "getContentProvider"),
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

  std::shared_ptr<zim::writer::FileItem> getItem() { return item_; }

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
      return FileProvider::New(info.Env(), item_->getContentProvider());
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
            InstanceMethod<&FileItem::getContentProvider>("getContentProvider"),
            InstanceAccessor<&FileItem::getHints>("hints"),
        });

    exports.Set("FileItem", func);
    constructors.fileItem = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::writer::FileItem> item_;
};

