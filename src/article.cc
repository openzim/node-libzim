
#include "article.h"

#include <napi.h>
#include <zim/article.h>
#include <zim/blob.h>
#include <zim/writer/creator.h>

#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

Napi::FunctionReference Article::constructor;

Napi::Object Article::New(Napi::Env env, zim::Article& article) {
  Napi::HandleScope scope(env);
  if (!article.good()) {
    throw Napi::Error::New(env, "article is not good");
  }

  auto external = Napi::External<zim::Article>::New(env, &article);
  return constructor.New({external});
}

Article::Article(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Article>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (!(info[0].IsObject() || info[0].IsExternal())) {
    throw Napi::Error::New(
        env, "1st argument must be an object with properies for Article");
  }

  if (info[0].IsExternal()) {
    // parse from external
    auto article = reinterpret_cast<zim::Article*>(
        info[0].As<Napi::External<zim::Article>>().Data());
    if (!article->good()) {
      throw Napi::Error::New(info.Env(),
                             "Invalid article, article is not 'good'.");
    }
    auto url = article->getUrl();
    auto ns = std::string(1, article->getNamespace())[0];
    auto aid = std::string(1, ns) + "/" + url;
    auto mimeType = !article->isRedirect() ? article->getMimeType() : "";
    auto redirectUrl =
        article->isRedirect() ? article->getRedirectArticle().getLongUrl() : "";
    article_ = std::make_shared<ZimArticle>(ns, aid, url, article->getTitle(),
                                            mimeType, redirectUrl, "", false);
    setData(article->getData());
  } else {
    // parse from object
    auto props = info[0].ToObject();

    std::string url = props.Get("url").ToString();

    char ns = '.';
    if (props.Has("ns")) {
      std::string str = props.Get("ns").ToString();
      if (str.length() != 1) {
        throw Napi::Error::New(env, "ns must be exactly one character");
      }
      ns = str[0];
    }

    std::string mimeType = props.Has("mimeType")
                               ? std::string(props.Get("mimeType").ToString())
                               : "text/plain";
    std::string title =
        props.Has("title") ? std::string(props.Get("title").ToString()) : "";
    std::string redirectUrl =
        props.Has("redirectUrl")
            ? std::string(props.Get("redirectUrl").ToString())
            : "";
    std::string fileName = props.Has("fileName")
                               ? std::string(props.Get("fileName").ToString())
                               : "";
    bool shouldIndex =
        props.Has("shouldIndex")
            ? static_cast<bool>(props.Get("shouldIndex").ToBoolean())
            : false;

    // `${props.ns}/${props.url}`
    std::string aid = props.Has("aid") ? props.Get("aid").ToString()
                                       : (std::string(1, ns) + "/" + url);

    article_ = std::make_shared<ZimArticle>(ns, aid, url, title, mimeType,
                                            redirectUrl, fileName, shouldIndex);
    setData(info, props.Get("data"));
  }
}

Napi::Value Article::getNs(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), std::string(1, article_->ns));
}
void Article::setNs(const Napi::CallbackInfo& info, const Napi::Value& value) {
  std::string str = value.ToString();
  if (str.length() != 1) {
    throw Napi::Error::New(info.Env(), "ns must be exactly one character");
  }
  article_->ns = str[0];
}

Napi::Value Article::getAid(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), article_->aid);
}

void Article::setAid(const Napi::CallbackInfo& info, const Napi::Value& value) {
  article_->aid = value.ToString();
}

Napi::Value Article::getUrl(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), article_->url);
}
void Article::setUrl(const Napi::CallbackInfo& info, const Napi::Value& value) {
  article_->url = value.ToString();
}

Napi::Value Article::getTitle(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), article_->title);
}

void Article::setTitle(const Napi::CallbackInfo& info,
                       const Napi::Value& value) {
  article_->title = value.ToString();
}

Napi::Value Article::getMimeType(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), article_->mimeType);
}

void Article::setMimeType(const Napi::CallbackInfo& info,
                          const Napi::Value& value) {
  article_->mimeType = value.ToString();
}

Napi::Value Article::getRedirectAid(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), article_->redirectAid);
}

void Article::setRedirectAid(const Napi::CallbackInfo& info,
                             const Napi::Value& value) {
  article_->redirectAid = value.ToString();
}

Napi::Value Article::getFilename(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), article_->getFilename());
}
void Article::setFilename(const Napi::CallbackInfo& info,
                          const Napi::Value& value) {
  article_->fileName = value.ToString();
}

Napi::Value Article::getShouldIndex(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), article_->shouldIndex());
}

void Article::setShouldIndex(const Napi::CallbackInfo& info,
                             const Napi::Value& value) {
  article_->_shouldIndex = value.ToBoolean();
}

Napi::Value Article::getData(const Napi::CallbackInfo& info) {
  return Napi::Buffer<uint8_t>::New(info.Env(), article_->bufferData.data(),
                                    article_->bufferData.size());
}
void Article::setData(const Napi::CallbackInfo& info, const Napi::Value& data) {
  if (data.IsString()) {
    std::string nbuf = data.ToString();
    article_->setData(nbuf.begin(), nbuf.end());
  } else if (data.IsBuffer()) {
    auto nbuf = data.As<Napi::Buffer<uint8_t>>();
    article_->setData(nbuf.Data(), nbuf.Data() + nbuf.Length());
  } else {
    throw Napi::Error::New(info.Env(), "data must be a string or buffer");
  }
}

void Article::setData(const zim::Blob& blob) {
  article_->setData(blob.data(), blob.end());
}

Napi::Value Article::IsRedirect(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), article_->isRedirect());
}

Napi::Value Article::getShouldCompress(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), article_->shouldCompress());
}

void Article::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(
      env, "Article",
      {
          InstanceAccessor("ns", &Article::getNs, &Article::setNs),
          InstanceAccessor("aid", &Article::getAid, &Article::setAid),
          InstanceAccessor("url", &Article::getUrl, &Article::setUrl),
          InstanceAccessor("title", &Article::getTitle, &Article::setTitle),
          InstanceAccessor("mimeType", &Article::getMimeType,
                           &Article::setMimeType),
          InstanceAccessor("redirectAid", &Article::getRedirectAid,
                           &Article::setRedirectAid),
          InstanceAccessor("redirectUrl", &Article::getRedirectAid,
                           &Article::setRedirectAid),
          InstanceAccessor("fileName", &Article::getFilename,
                           &Article::setFilename),
          InstanceAccessor("shouldIndex", &Article::getShouldIndex,
                           &Article::setShouldIndex),
          InstanceAccessor("data", &Article::getData, &Article::setData),
          InstanceAccessor("bufferData", &Article::getData, &Article::setData),
          InstanceMethod("isRedirect", &Article::IsRedirect),
          InstanceAccessor("shouldCompress", &Article::getShouldCompress,
                           nullptr),
      });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Article", func);
}

