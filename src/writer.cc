#include "writer.h"

#include <napi.h>

Napi::FunctionReference ZimCreatorWrapper::constructor;

ZimCreatorWrapper::ZimCreatorWrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<ZimCreatorWrapper>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (!info[0].IsObject()) {
    throw Napi::Error::New(
        env, "first argument must be an object with at least fileName");
  }

  auto opts = info[0].ToObject();
  std::string fileName = opts.Get("fileName").ToString();
  std::string mainPage =
      opts.Has("mainPage") ? std::string(opts.Get("mainPage").ToString()) : "";
  std::string fullTextIndexLanguage =
      opts.Has("fullTextIndexLanguage")
          ? std::string(opts.Get("fullTextIndexLanguage").ToString())
          : "";
  int minChunkSize = opts.Has("minChunkSize")
                         ? opts.Get("minChunkSize").ToNumber().Int32Value()
                         : 2048;

  zim::CompressionType comp = zim::zimcompLzma;
  if (opts.Has("compression")) {
      std::string comp_str = std::string(opts.Get("compression").ToString());
      if (comp_str == "lzma") { comp = zim::zimcompLzma; }
      if (comp_str == "zstd") { comp = zim::zimcompZstd; }
  }

  creator_ = std::make_shared<OverriddenZimCreator>(mainPage, comp);
  creator_->setIndexing(!fullTextIndexLanguage.empty(), fullTextIndexLanguage);
  creator_->setMinChunkSize(minChunkSize);
  creator_->startZimCreation(fileName);
}

void ZimCreatorWrapper::addArticle(const Napi::CallbackInfo& info) {
  Article* article =
      Napi::ObjectWrap<Article>::Unwrap(info[0].As<Napi::Object>());
  creator_->addArticle(article->getArticle());
}

void ZimCreatorWrapper::finalise(const Napi::CallbackInfo& info) {
  creator_->finishZimCreation();
  creator_ = nullptr;
}

void ZimCreatorWrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(
      env, "ZimCreatorWrapper",
      {
          InstanceMethod("addArticle", &ZimCreatorWrapper::addArticle),
          InstanceMethod("finalise", &ZimCreatorWrapper::finalise),
      });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("ZimCreatorWrapper", func);
}

