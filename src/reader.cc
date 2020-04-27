
#include "reader.h"

#include <napi.h>
#include <string>

Napi::FunctionReference ZimReaderWrapper::constructor;

ZimReaderWrapper::ZimReaderWrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<ZimReaderWrapper>(info) {
  if (!info[0].IsString()) {
    throw Napi::Error::New(info.Env(),
                           "first argument must be a string for fileName");
  }

  fileName_ = info[0].ToString();
  reader_ = std::make_shared<zim::File>(fileName_);
}

void ZimReaderWrapper::close(const Napi::CallbackInfo& info) {
  reader_ = nullptr;
}

Napi::Value ZimReaderWrapper::isOpen(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), reader_ != nullptr);
}

Napi::Value ZimReaderWrapper::getCountArticles(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), reader_->getCountArticles());
}

Napi::Value ZimReaderWrapper::getArticleById(const Napi::CallbackInfo& info) {
  zim::Article zarticle = reader_->getArticle(info[0].ToNumber().Uint32Value());
  if (!zarticle.good()) {
    throw Napi::Error::New(info.Env(), "Failed to find article");
  }

  return Article::New(info.Env(), &zarticle);
}

Napi::Value ZimReaderWrapper::getArticleByUrl(const Napi::CallbackInfo& info) {
  zim::Article zarticle = reader_->getArticleByUrl(info[0].ToString());
  if (!zarticle.good()) {
    throw Napi::Error::New(info.Env(), "Failed to find article");
  }

  return Article::New(info.Env(), &zarticle);
}

Napi::Value ZimReaderWrapper::suggest(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::HandleScope scope(env);

  std::string query = info[0].ToString();
  auto results = Napi::Array::New(env);

  auto search = reader_->suggestions(query, 0, 10);
  uint64_t idx = 0;
  for (const auto& it : *search) {
    results.Set(idx, Napi::String::New(env, it.getLongUrl()));
    idx++;
  }

  return results;
}

Napi::Value ZimReaderWrapper::search(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::HandleScope scope(env);

  std::string query = info[0].ToString();
  auto results = Napi::Array::New(env);

  auto search = reader_->search(query, 0, 10);
  uint64_t idx = 0;
  for (const auto& it : *search) {
    results.Set(idx, Napi::String::New(env, it.getLongUrl()));
    idx++;
  }

  return results;
}

Napi::Value ZimReaderWrapper::getFilename(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), reader_->getFilename());
}

void ZimReaderWrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(
      env, "ZimReaderWrapper",
      {
          InstanceMethod("close", &ZimReaderWrapper::close),
          InstanceMethod("isOpen", &ZimReaderWrapper::isOpen),
          InstanceMethod("getCountArticles",
                         &ZimReaderWrapper::getCountArticles),
          InstanceMethod("getArticleById", &ZimReaderWrapper::getArticleById),
          InstanceMethod("getArticleByUrl", &ZimReaderWrapper::getArticleByUrl),
          InstanceMethod("getFilename", &ZimReaderWrapper::getFilename),
          InstanceMethod("suggest", &ZimReaderWrapper::suggest),
          InstanceMethod("search", &ZimReaderWrapper::search),
          InstanceAccessor("fileName", &ZimReaderWrapper::getFilename, nullptr),
      });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("ZimReaderWrapper", func);
}

