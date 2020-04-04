#pragma once
#include <napi.h>
#include <zim/writer/creator.h>

#include <map>
#include <memory>
#include <string>

#include "article.h"

class OverriddenZimCreator : public zim::writer::Creator {
 public:
  OverriddenZimCreator(std::string mainPage)
      : zim::writer::Creator(true), mainPage(mainPage) {}

  virtual zim::writer::Url getMainUrl() {
    return zim::writer::Url('A', mainPage);
  }

  std::string mainPage;
};

class ZimCreatorWrapper : public Napi::ObjectWrap<ZimCreatorWrapper> {
 public:
  static void Init(Napi::Env env, Napi::Object exports);
  explicit ZimCreatorWrapper(const Napi::CallbackInfo& info);
  void addArticle(const Napi::CallbackInfo& info);
  void finalise(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
  std::shared_ptr<OverriddenZimCreator> creator_;
  std::string tmpDir_;
  std::string fileName_;
  std::map<std::string, uint64_t> articleCounter_;
};

