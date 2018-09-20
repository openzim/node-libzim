// Wrappers for zim::writer classes.

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_WRITER_H_
#define NODE_LIBZIM_WRITER_H_

#include "nan.h"

#include <zim/zim.h>
#include <zim/writer/articlesource.h>
#include <zim/writer/zimcreator.h>

#include <unordered_map>
#include <string>

#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {
namespace writer {

#define DEFINE_PROXY(Proxy, WrappedType)                                \
  static WrappedType *FromJS(v8::Local<v8::Value> p,                    \
                             const Nan::FunctionCallbackInfo<v8::Value> *info, \
                             bool owned);                               \
  virtual ~Proxy();                                                     \
  Nan::Persistent<v8::Object> proxy;                                    \
 private:                                                               \
  explicit Proxy(v8::Local<v8::Object> p,                               \
                 const Nan::FunctionCallbackInfo<v8::Value> *info);     \
  void MakeWeak() {                                                     \
    proxy.SetWeak(this, WeakCallback, Nan::WeakCallbackType::kParameter); \
  }                                                                     \
  static void WeakCallback(const Nan::WeakCallbackInfo<Proxy> &info) {  \
    Proxy *pp = info.GetParameter();                                    \
    delete pp;                                                          \
  }

// Proxies

class ArticleProxy : public zim::writer::Article {
 public:
  virtual std::string getAid() const;
  virtual char getNamespace() const;
  virtual std::string getUrl() const;
  virtual std::string getTitle() const;
  virtual zim::size_type getVersion() const;
  virtual bool isRedirect() const;
  virtual bool isLinktarget() const;
  virtual bool isDeleted() const;
  virtual std::string getMimeType() const;
  virtual bool shouldCompress() const;
  virtual std::string getRedirectAid() const;
  virtual std::string getParameter() const;
  virtual zim::Blob getData() const;
  virtual std::string getNextCategory();

  DEFINE_PROXY(ArticleProxy, zim::writer::Article)
};

class ArticleSourceProxy : public zim::writer::ArticleSource {
 public:
  virtual void setFilename(const std::string& fname);
  virtual const zim::writer::Article* getNextArticle();
  virtual zim::Uuid getUuid();
  virtual std::string getMainPage();
  virtual std::string getLayoutPage();
  virtual zim::writer::Category* getCategory(const std::string& cid);

  DEFINE_PROXY(ArticleSourceProxy, zim::writer::ArticleSource)
};

class CategoryProxy : public zim::writer::Category {
 public:
  virtual zim::Blob getData();
  virtual std::string getUrl() const;
  virtual std::string getTitle() const;

  DEFINE_PROXY(CategoryProxy, zim::writer::Category)
};

class ZimCreatorProxy : public zim::writer::ZimCreator {
 public:
  virtual void create(const std::string &fname,
                      zim::writer::ArticleSource* src);
  virtual unsigned getMinChunkSize();
  virtual void setMinChunkSize(int s);
  virtual zim::offset_type getCurrentSize();

  DEFINE_PROXY(ZimCreatorProxy, zim::writer::ZimCreator)
};

// Wrappers

#define PROXY_WRAPPER_DEFINE(Wrapper, WrappedType, ProxyType, field)    \
  static NAN_METHOD(New) {                                              \
    if (!info.IsConstructCall()) {                                      \
      return Nan::ThrowTypeError("You must use `new` with this constructor."); \
    }                                                                   \
    Wrapper *obj;                                                       \
    WrappedType *c;                                                     \
    if (info.Length() > 0 && info[0]->IsExternal()) {                   \
      c = reinterpret_cast<WrappedType*>                                \
        (v8::Local<v8::External>::Cast(info[0])->Value());              \
      obj = new Wrapper(c, info.Length() > 1 ? info[1]->IsTrue() : false);\
    } else {                                                            \
      c =                                                               \
        ProxyType::FromJS(Nan::New<v8::Object>(), &info, true);         \
      obj = new Wrapper(c, true);                                       \
    }                                                                   \
    obj->Wrap(info.Holder());                                           \
    info.GetReturnValue().Set(Nan::Undefined());                        \
  }                                                                     \
  static v8::Local<v8::Value> FromC(const WrappedType *o, bool owned) { \
    Nan::EscapableHandleScope scope;                                    \
    if (!o) {                                                           \
      return scope.Escape(Nan::Null());                                 \
    }                                                                   \
    /* Instance of ProxyType don't need to be re-wrapped. */            \
    if (proxyMap.count(o)) {                                            \
      return scope.Escape(Nan::New(proxyMap.at(o)->proxy));             \
    }                                                                   \
    v8::Local<v8::Value> argv[] = {                                     \
      Nan::New<v8::External>(const_cast<WrappedType*>(o)),              \
      Nan::New(owned)                                                   \
    };                                                                  \
    return scope.Escape(Nan::NewInstance(constructor(), 2, argv).ToLocalChecked());           \
  }                                                                     \
  static std::unordered_map<const WrappedType*, ProxyType*> proxyMap;   \
 private:                                                               \
  explicit Wrapper(WrappedType *field, bool owned) :                    \
    owned_(owned), field ## _(field) { }                                \
  virtual ~Wrapper() {                                                  \
    if (owned_) {                                                       \
      ProxyType *proxy = static_cast<ProxyType *>(field ## _);          \
      delete proxy;                                                     \
    }                                                                   \
  }                                                                     \
  bool owned_;                                                          \
  WRAPPER_DEFINE(Wrapper, WrappedType, field)

class ArticleWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "Article");
    WRAPPER_METHOD_INIT(tpl, getAid);
    WRAPPER_METHOD_INIT(tpl, getNamespace);
    WRAPPER_METHOD_INIT(tpl, getUrl);
    WRAPPER_METHOD_INIT(tpl, getTitle);
    WRAPPER_METHOD_INIT(tpl, getVersion);
    WRAPPER_METHOD_INIT(tpl, isRedirect);
    WRAPPER_METHOD_INIT(tpl, isLinktarget);
    WRAPPER_METHOD_INIT(tpl, isDeleted);
    WRAPPER_METHOD_INIT(tpl, getMimeType);
    WRAPPER_METHOD_INIT(tpl, shouldCompress);
    WRAPPER_METHOD_INIT(tpl, getRedirectAid);
    WRAPPER_METHOD_INIT(tpl, getParameter);
    WRAPPER_METHOD_INIT(tpl, getNextCategory);
    WRAPPER_METHOD_INIT(tpl, getData);
    WRAPPER_INIT_FINISH(tpl);
  }
  WRAPPER_METHOD_DECLARE_GET(getAid, STRING);
  WRAPPER_METHOD_DECLARE_GET(getNamespace, CHAR);
  WRAPPER_METHOD_DECLARE_GET(getUrl, STRING);
  WRAPPER_METHOD_DECLARE_GET(getTitle, STRING);
  WRAPPER_METHOD_DECLARE_GET(getVersion, SIZE_T);
  WRAPPER_METHOD_DECLARE_GET(isRedirect, BOOL);
  WRAPPER_METHOD_DECLARE_GET(isLinktarget, BOOL);
  WRAPPER_METHOD_DECLARE_GET(isDeleted, BOOL);
  WRAPPER_METHOD_DECLARE_GET(getMimeType, STRING);
  WRAPPER_METHOD_DECLARE_GET(shouldCompress, BOOL);
  WRAPPER_METHOD_DECLARE_GET(getRedirectAid, STRING);
  WRAPPER_METHOD_DECLARE_GET(getParameter, STRING_BUFFER);
  WRAPPER_METHOD_DECLARE_GET(getNextCategory, STRING);
  WRAPPER_METHOD_DECLARE_GET(getData, BLOB);
  PROXY_WRAPPER_DEFINE(ArticleWrap, zim::writer::Article,
                       ArticleProxy, article)
};

class ArticleSourceWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "ArticleSource");
    WRAPPER_METHOD_INIT(tpl, setFilename);
    WRAPPER_METHOD_INIT(tpl, getNextArticle);
    WRAPPER_METHOD_INIT(tpl, getUuid);
    WRAPPER_METHOD_INIT(tpl, getMainPage);
    WRAPPER_METHOD_INIT(tpl, getLayoutPage);
    WRAPPER_METHOD_INIT(tpl, getCategory);
    WRAPPER_INIT_FINISH(tpl);
  }
  WRAPPER_METHOD_DECLARE_SET(setFilename, STRING);
  WRAPPER_METHOD_DECLARE(getNextArticle);
  WRAPPER_METHOD_DECLARE_GET(getUuid, UUID);
  WRAPPER_METHOD_DECLARE_GET(getMainPage, STRING);
  WRAPPER_METHOD_DECLARE_GET(getLayoutPage, STRING);
  WRAPPER_METHOD_DECLARE(getCategory);
  PROXY_WRAPPER_DEFINE(ArticleSourceWrap, zim::writer::ArticleSource,
                       ArticleSourceProxy, articleSource)
};

class CategoryWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "Category");
    WRAPPER_METHOD_INIT(tpl, getData);
    WRAPPER_METHOD_INIT(tpl, getUrl);
    WRAPPER_METHOD_INIT(tpl, getTitle);
    WRAPPER_INIT_FINISH(tpl);
  }
  WRAPPER_METHOD_DECLARE_GET(getData, BLOB);
  WRAPPER_METHOD_DECLARE_GET(getUrl, STRING);
  WRAPPER_METHOD_DECLARE_GET(getTitle, STRING);
  PROXY_WRAPPER_DEFINE(CategoryWrap, zim::writer::Category,
                       CategoryProxy, category)
};

class ZimCreatorWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "ZimCreator");
    WRAPPER_METHOD_INIT(tpl, create);
    WRAPPER_METHOD_INIT(tpl, getMinChunkSize);
    WRAPPER_METHOD_INIT(tpl, setMinChunkSize);
    WRAPPER_METHOD_INIT(tpl, getCurrentSize);
    WRAPPER_INIT_FINISH(tpl);
  }
  WRAPPER_METHOD_DECLARE(create);
  WRAPPER_METHOD_DECLARE_GET(getMinChunkSize, UNSIGNED);
  WRAPPER_METHOD_DECLARE(setMinChunkSize);
  WRAPPER_METHOD_DECLARE_GET(getCurrentSize, OFFSET_T);
  PROXY_WRAPPER_DEFINE(ZimCreatorWrap, zim::writer::ZimCreator,
                       ZimCreatorProxy, zimCreator)
};

NAN_MODULE_INIT(Init);

}  // namespace writer
}  // namespace node_libzim

#endif  // NODE_LIBZIM_WRITER_H_
