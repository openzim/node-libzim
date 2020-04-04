#pragma once
#include <napi.h>
#include <zim/file.h>
#include <zim/search.h>

#include <memory>

#include "article.h"

class ZimReaderWrapper : public Napi::ObjectWrap<ZimReaderWrapper> {
 public:
  static void Init(Napi::Env env, Napi::Object exports);
  explicit ZimReaderWrapper(const Napi::CallbackInfo& info);

  void close(const Napi::CallbackInfo& info);
  Napi::Value isOpen(const Napi::CallbackInfo& info);

  Napi::Value getCountArticles(const Napi::CallbackInfo& info);
  Napi::Value getArticleById(const Napi::CallbackInfo& info);
  Napi::Value getArticleByUrl(const Napi::CallbackInfo& info);
  Napi::Value suggest(const Napi::CallbackInfo& info);
  Napi::Value search(const Napi::CallbackInfo& info);

  Napi::Value getFilename(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
  std::shared_ptr<zim::File> reader_;
  std::string fileName_;
};
