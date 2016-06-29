// Binding to zim::writer

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include <zim/zim.h>
#include <zim/fileheader.h>
#include <zim/writer/zimcreator.h>
#include <zim/blob.h>

#include <unordered_map>
#include <string>

// #define NODE_LIBZIM_TRACE

#include "src/blob.h"
#include "src/macros.h"
#include "src/uuid.h"
#include "src/wrapper.h"
#include "src/writer.h"

namespace node_libzim {
namespace writer {

#define PROXY_GETFUNC(func, name)                                       \
  v8::Local<v8::Value> func ## val =                                    \
    (proxy.IsEmpty() ? Nan::MaybeLocal<v8::Value>() :                   \
     Nan::Get(Nan::New(proxy), NEW_STR(name)))                          \
    .FromMaybe<v8::Value>(Nan::Undefined());                            \
  Nan::MaybeLocal<v8::Object> func = func ## val->IsUndefined() ?       \
    Nan::MaybeLocal<v8::Object>() : Nan::To<v8::Object>(func ## val);

#define PROXY_DEFAULT_CONSTRUCTOR(Proxy, Wrapper)                       \
  Proxy::Proxy(v8::Local<v8::Object> p,                                 \
               const Nan::FunctionCallbackInfo<v8::Value> *info) : proxy() { \
    /* default no-args superclass constructor */                        \
    proxy.Reset(p);                                                     \
    Wrapper::proxyMap[this] = this;                                     \
  }
#define PROXY_DEFAULT_DESTRUCTOR(Proxy, Wrapper)                        \
  Proxy::~Proxy() {                                                     \
    if (!proxy.IsEmpty()) {                                             \
      proxy.ClearWeak();                                                \
      proxy.Reset();                                                    \
      Wrapper::proxyMap.erase(this);                                    \
    }                                                                   \
  }

#define PROXY_FROMJS(Proxy, Wrapper, WrappedType)                       \
WrappedType *Proxy::FromJS(v8::Local<v8::Value> p,                      \
                           const Nan::FunctionCallbackInfo<v8::Value> *info, \
                           bool owned) {                                \
  Proxy *ap;                                                            \
  /* Does this object already have a proxy? */                          \
  if (p->IsObject()) {                                                  \
    v8::Local<v8::String> hidden_field = NEW_STR("zim::" #Proxy);       \
    v8::Local<v8::Object> obj = Nan::To<v8::Object>(p).ToLocalChecked(); \
    v8::Local<v8::Value> proxy = obj->GetHiddenValue(hidden_field);     \
    if (!proxy.IsEmpty() && proxy->IsExternal()) {                      \
      ap = reinterpret_cast<Proxy*>                                     \
        (v8::Local<v8::External>::Cast(proxy)->Value());                \
      return ap;                                                        \
    }                                                                   \
    /* Have to make one. */                                             \
    ap = new Proxy(obj, info);                                          \
    obj->SetHiddenValue(hidden_field, Nan::New<v8::External>(ap));      \
    /* Clean up the proxy when p goes away. */                          \
    if (!owned) {                                                       \
      ap->MakeWeak();                                                   \
    }                                                                   \
    return ap;                                                          \
  }                                                                     \
  return NULL;                                                          \
}
#define PROXY_GET_STRING(name)                                          \
  PROXY_GETFUNC(func, name);                                            \
  if (!func.IsEmpty()) {                                                \
    Nan::MaybeLocal<v8::Value> result =                                 \
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL); \
    if (!result.IsEmpty()) {                                            \
      /* Convert JavaScript value to C++ string. */                     \
      Nan::Utf8String s(result.ToLocalChecked());                       \
      std::string ss(*s, s.length());                                   \
      return ss;                                                        \
    }                                                                   \
  }
#define PROXY_GET_STRING_BUFFER(name)                                   \
  PROXY_GETFUNC(func, name);                                            \
  if (!func.IsEmpty()) {                                                \
    Nan::MaybeLocal<v8::Value> result =                                 \
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL); \
    if (!result.IsEmpty()) {                                            \
      v8::Local<v8::Value> r = result.ToLocalChecked();                 \
      if (node::Buffer::HasInstance(r)) {                               \
        v8::Local<v8::Object> ro = Nan::To<v8::Object>(r).ToLocalChecked(); \
        std::string ss(node::Buffer::Data(ro), node::Buffer::Length(ro)); \
        return ss;                                                      \
      } else {                                                          \
        /* Convert JavaScript value to C++ string. */                   \
        Nan::Utf8String s(r);                                           \
        std::string ss(*s, s.length());                                 \
        return ss;                                                      \
      }                                                                 \
    }                                                                   \
  }
#define PROXY_GET_BOOL(name)                                            \
  PROXY_GETFUNC(func, name);                                            \
  if (!func.IsEmpty()) {                                                \
    Nan::MaybeLocal<v8::Value> result =                                 \
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL); \
    if (!result.IsEmpty()) {                                            \
      return Nan::To<bool>(result.ToLocalChecked()).FromMaybe(false);   \
    }                                                                   \
  }

// ArticleProxy

PROXY_DEFAULT_CONSTRUCTOR(ArticleProxy, ArticleWrap)
PROXY_DEFAULT_DESTRUCTOR(ArticleProxy, ArticleWrap)
PROXY_FROMJS(ArticleProxy, ArticleWrap, zim::writer::Article)

std::string ArticleProxy::getAid() const {
  PROXY_GET_STRING("getAid");
  Nan::ThrowTypeError("no implementation for Article::getAid");
  return "";
}
char ArticleProxy::getNamespace() const {
  PROXY_GETFUNC(func, "getNamespace");
  if (!func.IsEmpty()) {
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL);
    if (!result.IsEmpty()) {
      Nan::Utf8String s(result.ToLocalChecked());
      return s.length() ? **s : '\0';
    }
  }
  Nan::ThrowTypeError("no implementation for Article::getNamespace");
  return '\0';
}
std::string ArticleProxy::getUrl() const {
  PROXY_GET_STRING("getUrl");
  Nan::ThrowTypeError("no implementation for Article::getUrl");
  return "";
}
std::string ArticleProxy::getTitle() const {
  PROXY_GET_STRING("getTitle");
  Nan::ThrowTypeError("no implementation for Article::getTitle");
  return "";
}
zim::size_type ArticleProxy::getVersion() const {
  PROXY_GETFUNC(func, "getVersion");
  if (!func.IsEmpty()) {
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL);
    if (!result.IsEmpty()) {
      return (zim::size_type)
        Nan::To<int64_t>(result.ToLocalChecked()).FromMaybe(0);
    }
  }
  return this->zim::writer::Article::getVersion();
}
bool ArticleProxy::isRedirect() const {
  PROXY_GET_BOOL("isRedirect");
  return this->zim::writer::Article::isRedirect();
}
bool ArticleProxy::isLinktarget() const {
  PROXY_GET_BOOL("isLinktarget");
  return this->zim::writer::Article::isLinktarget();
}
bool ArticleProxy::isDeleted() const {
  PROXY_GET_BOOL("isDeleted");
  return this->zim::writer::Article::isDeleted();
}
std::string ArticleProxy::getMimeType() const {
  PROXY_GET_STRING("getMimeType");
  Nan::ThrowTypeError("no implementation for Article::getMimeType");
  return "";
}
bool ArticleProxy::shouldCompress() const {
  PROXY_GET_BOOL("shouldCompress");
  return this->zim::writer::Article::shouldCompress();
}
std::string ArticleProxy::getRedirectAid() const {
  PROXY_GET_STRING("getRedirectAid");
  return this->zim::writer::Article::getRedirectAid();
}
std::string ArticleProxy::getParameter() const {
  PROXY_GET_STRING_BUFFER("getParameter");
  return this->zim::writer::Article::getParameter();
}
zim::Blob ArticleProxy::getData() const {
  PROXY_GETFUNC(func, "getData");
  if (!func.IsEmpty()) {
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL);
    if (!result.IsEmpty()) {
      // Convert JavaScript value to C++ object.
      return BlobWrap::FromJS(result.ToLocalChecked());
    }
  }
  Nan::ThrowTypeError("no implementation for Article::getData");
  return zim::Blob(NULL, 0);
}
std::string ArticleProxy::getNextCategory() {
  PROXY_GET_STRING("getNextCategory");
  return this->zim::writer::Article::getNextCategory();
}

// ArticleSourceProxy

PROXY_DEFAULT_CONSTRUCTOR(ArticleSourceProxy, ArticleSourceWrap)
PROXY_DEFAULT_DESTRUCTOR(ArticleSourceProxy, ArticleSourceWrap)
PROXY_FROMJS(ArticleSourceProxy, ArticleSourceWrap, zim::writer::ArticleSource)

void ArticleSourceProxy::setFilename(const std::string& fname) {
  PROXY_GETFUNC(func, "setFilename");
  if (!func.IsEmpty()) {
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = { NEW_STR(fname) };
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy),
                          argc, argv);
    if (!result.IsEmpty()) {
      return;  // Handled by JavaScript.
    }
  }
  // Use default superclass implementation.
  return this->zim::writer::ArticleSource::setFilename(fname);
}

const zim::writer::Article* ArticleSourceProxy::getNextArticle() {
  PROXY_GETFUNC(func, "getNextArticle");
  if (!func.IsEmpty()) {
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL);
    if (!result.IsEmpty()) {
      // Convert JavaScript value to C++ object.
      return ArticleProxy::FromJS(result.ToLocalChecked(), NULL, false);
    }
  }
  // Throw: this method doesn't have a default superclass implementation.
  Nan::ThrowTypeError("no implementation for ArticleSource::getNextArticle");
  return NULL;
}
zim::Uuid ArticleSourceProxy::getUuid() {
  PROXY_GETFUNC(func, "getUuid");
  if (!func.IsEmpty()) {
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL);
    if (!result.IsEmpty()) {
      // Convert JavaScript value to C++ object.
      return UuidWrap::FromJS(result.ToLocalChecked());
    }
  }
  return this->zim::writer::ArticleSource::getUuid();
}
std::string ArticleSourceProxy::getMainPage() {
  PROXY_GET_STRING("getMainPage");
  return this->zim::writer::ArticleSource::getMainPage();
}
std::string ArticleSourceProxy::getLayoutPage() {
  PROXY_GET_STRING("getLayoutPage");
  return this->zim::writer::ArticleSource::getLayoutPage();
}
zim::writer::Category* ArticleSourceProxy::getCategory(const std::string& cid) {
  PROXY_GETFUNC(func, "getCategory");
  if (!func.IsEmpty()) {
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = { NEW_STR(cid) };
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy),
                          argc, argv);
    if (!result.IsEmpty()) {
      // Convert JavaScript value to C++ object.
      return CategoryProxy::FromJS(result.ToLocalChecked(), NULL, false);
    }
  }
  return this->zim::writer::ArticleSource::getCategory(cid);
}

// CategoryProxy

PROXY_DEFAULT_CONSTRUCTOR(CategoryProxy, CategoryWrap)
PROXY_DEFAULT_DESTRUCTOR(CategoryProxy, CategoryWrap)
PROXY_FROMJS(CategoryProxy, CategoryWrap, zim::writer::Category)

zim::Blob CategoryProxy::getData() {
  PROXY_GETFUNC(func, "getData");
  if (!func.IsEmpty()) {
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL);
    if (!result.IsEmpty()) {
      // Convert JavaScript value to C++ object.
      return BlobWrap::FromJS(result.ToLocalChecked());
    }
  }
  Nan::ThrowTypeError("no implementation for Category::getData");
  return zim::Blob(NULL, 0);
}
std::string CategoryProxy::getUrl() const {
  PROXY_GET_STRING("getUrl");
  Nan::ThrowTypeError("no implementation for Category::getUrl");
  return "";
}
std::string CategoryProxy::getTitle() const {
  PROXY_GET_STRING("getTitle");
  Nan::ThrowTypeError("no implementation for Category::getTitle");
  return "";
}

// ZimCreatorProxy

ZimCreatorProxy::ZimCreatorProxy(
  v8::Local<v8::Object> p,
  const Nan::FunctionCallbackInfo<v8::Value> *info
) : proxy() {
  /* XXX Invoke alternate superclass constructor */
  proxy.Reset(p);
  ZimCreatorWrap::proxyMap[this] = this;                                     \
}
PROXY_DEFAULT_DESTRUCTOR(ZimCreatorProxy, ZimCreatorWrap)
PROXY_FROMJS(ZimCreatorProxy, ZimCreatorWrap, zim::writer::ZimCreator)

void ZimCreatorProxy::create(const std::string &fname,
                             zim::writer::ArticleSource* src) {
  PROXY_GETFUNC(func, "create");
  if (!func.IsEmpty()) {
    const int argc = 2;
    v8::Local<v8::Value> argv[argc] = {
      NEW_STR(fname),
      ArticleSourceWrap::FromC(src, false)
    };
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy),
                          argc, argv);
    if (!result.IsEmpty()) {
      return;
    }
  }
  return this->zim::writer::ZimCreator::create(fname, *src);
}
unsigned ZimCreatorProxy::getMinChunkSize() {
  PROXY_GETFUNC(func, "getMinChunkSize");
  if (!func.IsEmpty()) {
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL);
    if (!result.IsEmpty()) {
      return Nan::To<uint32_t>(result.ToLocalChecked()).FromMaybe(0);
    }
  }
  return this->zim::writer::ZimCreator::getMinChunkSize();
}
void ZimCreatorProxy::setMinChunkSize(int s) {
  PROXY_GETFUNC(func, "setMinChunkSize");
  if (!func.IsEmpty()) {
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = { Nan::New(s) };
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy),
                          argc, argv);
    if (!result.IsEmpty()) {
      return;
    }
  }
  return this->zim::writer::ZimCreator::setMinChunkSize(s);
}
zim::offset_type ZimCreatorProxy::getCurrentSize() {
  PROXY_GETFUNC(func, "getCurrentSize");
  if (!func.IsEmpty()) {
    Nan::MaybeLocal<v8::Value> result =
      Nan::CallAsFunction(func.ToLocalChecked(), Nan::New(proxy), 0, NULL);
    if (!result.IsEmpty()) {
      return (zim::offset_type)
        Nan::To<int64_t>(result.ToLocalChecked()).FromMaybe(0);
    }
  }
  return this->zim::writer::ZimCreator::getCurrentSize();
}

// WRAPPERS

// ArticleWrap

std::unordered_map<const zim::writer::Article*, ArticleProxy*>
  ArticleWrap::proxyMap;

// ArticleSourceWrap

std::unordered_map<const zim::writer::ArticleSource*, ArticleSourceProxy*>
  ArticleSourceWrap::proxyMap;

NAN_METHOD(ArticleSourceWrap::getNextArticle) {
  const zim::writer::Article *a = getWrappedField(info)->getNextArticle();
  // Convert to wrapped article
  info.GetReturnValue().Set(ArticleWrap::FromC(a, false));
}
NAN_METHOD(ArticleSourceWrap::getCategory) {
  REQUIRE_ARGUMENT_STD_STRING(0, cid);
  zim::writer::Category *c = getWrappedField(info)->getCategory(cid);
  // Convert to wrapped category
  info.GetReturnValue().Set(CategoryWrap::FromC(c, false));
}

// CategoryWrap

std::unordered_map<const zim::writer::Category*, CategoryProxy*>
  CategoryWrap::proxyMap;

// ZimCreatorWrap

std::unordered_map<const zim::writer::ZimCreator*, ZimCreatorProxy*>
  ZimCreatorWrap::proxyMap;

NAN_METHOD(ZimCreatorWrap::create) {
  REQUIRE_ARGUMENT_STD_STRING(0, fname);
  REQUIRE_ARGUMENTS(2);
  zim::writer::ArticleSource *as =
    ArticleSourceProxy::FromJS(info[1], NULL, false);
  getWrappedField(info)->create(fname, *as);
  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(ZimCreatorWrap::setMinChunkSize) {
  WRAPPER_SET_INTEGER(setMinChunkSize, int);
}

NAN_MODULE_INIT(Init) {
  ArticleWrap::Init(target);
  ArticleSourceWrap::Init(target);
  CategoryWrap::Init(target);
  ZimCreatorWrap::Init(target);
}

}  // namespace writer
}  // namespace node_libzim
