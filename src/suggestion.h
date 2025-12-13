#pragma once

#include <napi.h>
#include <zim/suggestion.h>

#include <exception>
#include <functional>
#include <memory>
#include <sstream>
#include <utility>

#include "archive.h"
#include "common.h"
#include "entry.h"

class SuggestionIterator : public Napi::ObjectWrap<SuggestionIterator> {
 public:
  explicit SuggestionIterator(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<SuggestionIterator>(info), iterator_{} {
    if (info[0].IsExternal()) {
      iterator_ = *info[0].As<Napi::External<decltype(iterator_)>>().Data();
    }
  }

  static Napi::Object New(Napi::Env env, zim::SuggestionIterator iterator) {
    auto ptr = std::make_shared<zim::SuggestionIterator>(iterator);
    auto external = Napi::External<decltype(ptr)>::New(env, &ptr);
    auto &constructor =
        env.GetInstanceData<ModuleConstructors>()->suggestionIterator;
    return constructor.New({external});
  }

  Napi::Value getEntry(const Napi::CallbackInfo &info) {
    try {
      return Entry::New(info.Env(), (*iterator_).getEntry());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getTitle(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), (*iterator_)->getTitle());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getPath(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), (*iterator_)->getPath());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getSnippet(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), (*iterator_)->getSnippet());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value hasSnippet(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), (*iterator_)->hasSnippet());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func = DefineClass(
        env, "SuggestionIterator",
        {
            InstanceAccessor<&SuggestionIterator::getEntry>("entry"),
            InstanceAccessor<&SuggestionIterator::getTitle>("title"),
            InstanceAccessor<&SuggestionIterator::getPath>("path"),
            InstanceAccessor<&SuggestionIterator::getSnippet>("snippet"),
            InstanceAccessor<&SuggestionIterator::hasSnippet>("haSnippet"),
        });

    exports.Set("SuggestionIterator", func);
    constructors.suggestionIterator = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::SuggestionIterator> iterator_;
};

class SuggestionResultSet : public Napi::ObjectWrap<SuggestionResultSet> {
 public:
  explicit SuggestionResultSet(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<SuggestionResultSet>(info), resultSet_{nullptr} {
    Napi::Env env = info.Env();

    if (!info[0].IsExternal()) {
      throw Napi::Error::New(env,
                             "SuggestionResultSet must be created internally.");
    }

    resultSet_ = *info[0].As<Napi::External<decltype(resultSet_)>>().Data();
  }

  static Napi::Object New(Napi::Env env,
                          const zim::SuggestionResultSet &resultSet) {
    // done this way to avoid copying the zim::SuggestionResultSet during
    // creation also need to copy it to lose the const qualifier so just do it
    // here.
    auto ptr = std::make_shared<zim::SuggestionResultSet>(resultSet);
    auto external = Napi::External<decltype(ptr)>::New(env, &ptr);
    auto &constructor =
        env.GetInstanceData<ModuleConstructors>()->suggestionResultSet;
    return constructor.New({external});
  }

  Napi::Value getSize(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), resultSet_->size());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getIterator(const Napi::CallbackInfo &info) {
    try {
      auto range = resultSet_;
      return Napi::Function::New(
          info.Env(), [range](const Napi::CallbackInfo &info) -> Napi::Value {
            Napi::Env env = info.Env();
            Napi::Object iter = Napi::Object::New(env);

            auto it = range->begin();
            iter["next"] = Napi::Function::New(
                info.Env(),
                [it,
                 range](const Napi::CallbackInfo &info) mutable -> Napi::Value {
                  Napi::Env env = info.Env();
                  Napi::Object res = Napi::Object::New(env);

                  if (it != range->end()) {
                    res["done"] = false;
                    res["value"] = SuggestionIterator::New(env, it);
                    it++;
                  } else {
                    res["done"] = true;
                  }
                  return res;
                });
            return iter;
          });
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func =
        DefineClass(env, "SuggestionResultSet",
                    {
                        InstanceAccessor<&SuggestionResultSet::getSize>("size"),
                        InstanceAccessor<&SuggestionResultSet::getIterator>(
                            Napi::Symbol::WellKnown(env, "iterator")),
                    });

    exports.Set("SuggestionResultSet", func);
    constructors.suggestionResultSet = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::SuggestionResultSet> resultSet_;
};

class SuggestionSearch : public Napi::ObjectWrap<SuggestionSearch> {
 public:
  explicit SuggestionSearch(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<SuggestionSearch>(info), search_{nullptr} {
    Napi::Env env = info.Env();

    if (!info[0].IsExternal()) {
      throw Napi::Error::New(env, "Search must be created internally.");
    }

    search_ = std::make_shared<zim::SuggestionSearch>(
        std::move(*info[0].As<Napi::External<zim::SuggestionSearch>>().Data()));
  }

  static Napi::Object New(Napi::Env env, zim::SuggestionSearch search) {
    // NOTE: search will be std::move into a shared_ptr and invalid after this.
    auto external = Napi::External<zim::SuggestionSearch>::New(env, &search);
    auto &constructor =
        env.GetInstanceData<ModuleConstructors>()->suggestionSearch;
    return constructor.New({external});
  }

  Napi::Value getResults(const Napi::CallbackInfo &info) {
    try {
      // TODO(kelvinhammond): construct SearchResultSet and return
      auto env = info.Env();
      if (!(info[0].IsNumber() && info[1].IsNumber())) {
        throw Napi::Error::New(env,
                               "getResults must be called with start and "
                               "maxResults values of type Number");
      }

      auto start = info[0].ToNumber();
      auto maxResults = info[1].ToNumber();
      return SuggestionResultSet::New(env,
                                      search_->getResults(start, maxResults));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getEstimatedMatches(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), search_->getEstimatedMatches());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func = DefineClass(
        env, "SuggestionSearch",
        {
            InstanceMethod<&SuggestionSearch::getResults>("getResults"),
            InstanceAccessor<&SuggestionSearch::getEstimatedMatches>(
                "estimatedMatches"),
        });

    exports.Set("SuggestionSearch", func);
    constructors.suggestionSearch = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::SuggestionSearch> search_;
};

class SuggestionSearcher : public Napi::ObjectWrap<SuggestionSearcher> {
 public:
  explicit SuggestionSearcher(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<SuggestionSearcher>(info),
        suggestionSearcher_{nullptr} {
    Napi::Env env = info.Env();

    // TODO(kelvinhammond): Ask about support for suggestions from multiple
    // archives
    /*
    if (info[0].IsArray()) {
      auto array = info[0].As<Napi::Array>();
      if (array.Length() < 1) {
        throw Napi::Error::New(env,
                               "SuggestionSearcher: argument array must "
                               "contain at least 1 archive.");
      }

      std::vector<zim::Archive> archives;
      for (size_t i = 0; i < array.Length(); i++) {
        auto obj = array.Get(i);
        if (!obj.IsObject()) {
          throw Napi::Error::New(
              env,
              "SuggestionSearcher: array arguments must be Archive objects");
        }

        auto archive = Napi::ObjectWrap<Archive>::Unwrap(obj.As<Napi::Object>())
                           ->archive();
        archives.emplace_back(*archive);
      }

      suggestionSearcher_ = std::make_shared<zim::SuggestionSearcher>(archives);
    } else */
    if (info[0].IsObject()) {  // one archive
      auto archive =
          Napi::ObjectWrap<Archive>::Unwrap(info[0].As<Napi::Object>())
              ->archive();
      suggestionSearcher_ = std::make_shared<zim::SuggestionSearcher>(*archive);
    } else {
      throw Napi::Error::New(
          env,
          "SuggestionSearcher: argument 1 must be an Archive object "
          "or an array of Archive objects.");
    }
  }

  /*
  Napi::Value addArchive(const Napi::CallbackInfo &info) {
    try {
      if (!info[0].IsObject()) {
        throw Napi::Error::New(info.Env(),
                               "argument 1 must be an Archive object.");
      }

      auto archive =
          Napi::ObjectWrap<Archive>::Unwrap(info[0].As<Napi::Object>())
              ->archive();
      suggestionSearcher_->addArchive(*archive);
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }
  */

  Napi::Value suggest(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      if (info[0].IsString()) {
        auto &&search = suggestionSearcher_->suggest(info[0].ToString());
        return SuggestionSearch::New(env, std::move(search));
      }
      throw Napi::Error::New(env, "suggest argument must be a string");
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value setVerbose(const Napi::CallbackInfo &info) {
    try {
      suggestionSearcher_->setVerbose(info[0].ToBoolean());
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func = DefineClass(
        env, "SuggestionSearcher",
        {
            // InstanceMethod<&SuggestionSearcher::addArchive>("addArchive"),
            InstanceMethod<&SuggestionSearcher::suggest>("suggest"),
            InstanceMethod<&SuggestionSearcher::setVerbose>("setVerbose"),
        });

    exports.Set("SuggestionSearcher", func);
    constructors.suggestionSearcher = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::SuggestionSearcher> suggestionSearcher_;
};

