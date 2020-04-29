#pragma once

#include <napi.h>
#include <zim/article.h>
#include <zim/blob.h>
#include <zim/writer/creator.h>

#include <memory>
#include <string>
#include <vector>

/**
 * ZimArticle implementation
 */
class ZimArticle : public zim::writer::Article {
 public:
  ZimArticle(char ns, std::string aid, std::string url, std::string title,
             std::string mimeType, std::string redirectAid,
             std::string fileName, bool _shouldIndex)
      : ns(ns),
        aid(aid),
        url(url),
        title(title),
        mimeType(mimeType),
        redirectAid(redirectAid),
        fileName(fileName),
        _shouldIndex(_shouldIndex),
        bufferData{} {}

  char ns;
  std::string aid;
  std::string url;
  std::string title;
  std::string mimeType;
  std::string redirectAid;
  std::string fileName;
  bool _shouldIndex;
  std::vector<uint8_t> bufferData;
  zim::Blob _data;

  void setData(const std::vector<uint8_t> &data) {
    bufferData = data;
    _data = zim::Blob(reinterpret_cast<char *>(bufferData.data()),
                      bufferData.size());
  }

  template <class InputIt>
  void setData(InputIt first, InputIt last) {
    bufferData = std::vector<uint8_t>(first, last);
    _data = zim::Blob(reinterpret_cast<char *>(bufferData.data()),
                      bufferData.size());
  }

  zim::Blob getData() const { return _data; }
  std::string getFilename() const { return fileName; }
  virtual zim::size_type getSize() const { return _data.size(); }
  bool shouldIndex() const { return _shouldIndex; }
  zim::writer::Url getUrl() const { return zim::writer::Url(ns, url); }
  std::string getTitle() const { return title; }
  bool isRedirect() const { return !redirectAid.empty(); }
  std::string getMimeType() const { return mimeType; }

  zim::writer::Url getRedirectUrl() const {
    return zim::writer::Url(ns, redirectAid);
  }

  bool shouldCompress() const {
    return getMimeType().find("text") == 0 ||
           getMimeType() == "application/javascript" ||
           getMimeType() == "application/json" ||
           getMimeType() == "image/svg+xml";
  }
};

/**
 * Javascript Article binding object
 */
class Article : public Napi::ObjectWrap<Article> {
 public:
  static void Init(Napi::Env env, Napi::Object exports);
  static Napi::Object New(Napi::Env env, zim::Article *article);
  explicit Article(const Napi::CallbackInfo &info);

  Napi::Value getNs(const Napi::CallbackInfo &info);
  void setNs(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value getAid(const Napi::CallbackInfo &info);
  void setAid(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value getUrl(const Napi::CallbackInfo &info);
  void setUrl(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value getTitle(const Napi::CallbackInfo &info);
  void setTitle(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value getMimeType(const Napi::CallbackInfo &info);
  void setMimeType(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value getRedirectAid(const Napi::CallbackInfo &info);
  void setRedirectAid(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value getFilename(const Napi::CallbackInfo &info);
  void setFilename(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value getShouldIndex(const Napi::CallbackInfo &info);
  void setShouldIndex(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value getData(const Napi::CallbackInfo &info);
  void setData(const Napi::CallbackInfo &info, const Napi::Value &value);

  void setData(const zim::Blob &blob);

  Napi::Value IsRedirect(const Napi::CallbackInfo &info);
  Napi::Value getShouldCompress(const Napi::CallbackInfo &info);

  auto getArticle() { return article_; }

 private:
  static Napi::FunctionReference constructor;
  std::shared_ptr<ZimArticle> article_;
};

