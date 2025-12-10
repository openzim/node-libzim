#pragma once

#include <napi.h>
#include <zim/search.h>

#include <exception>
#include <functional>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "archive.h"
#include "common.h"
#include "entry.h"

class Query : public Napi::ObjectWrap<Query> {
 public:
  explicit Query(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<Query>(info), query_{std::make_shared<zim::Query>()} {
    if (info.Length() > 0) {
      query_->setQuery(info[0].ToString().Utf8Value());
    }
  }

  Napi::Value setQuery(const Napi::CallbackInfo &info) {
    setQuery(info, info[0]);
    return info.This();
  }

  void setQuery(const Napi::CallbackInfo &info, const Napi::Value &value) {
    try {
      if (!value.IsString()) {
        throw Napi::Error::New(info.Env(), "Query must be a string.");
      }
      query_->setQuery(value.ToString().Utf8Value());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  void setGeorangeObject(const Napi::CallbackInfo &info,
                         const Napi::Value &value) {
    // allows this method to be used as a set accessor too
    try {
      auto env = info.Env();
      constexpr auto errMsg =
          "georange must be called with an object containing latitude, "
          "longitude, and distance";
      if (!value.IsObject()) {
        throw Napi::Error::New(env, errMsg);
      }

      auto obj = value.ToObject();
      auto valid =
          obj.Has("longitude") && obj.Has("longitude") && obj.Has("distance");
      if (!valid) {
        throw Napi::Error::New(env, errMsg);
      }

      auto latitude = obj.Get("latitude").ToNumber();
      auto longitude = obj.Get("longitude").ToNumber();
      auto distance = obj.Get("distance").ToNumber();
      query_->setGeorange(latitude, longitude, distance);
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value setGeorange(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();

      // support for object { latitude, longitude, distance }
      if (info[0].IsObject()) {
        setGeorangeObject(info, info[0].ToObject());
        return info.This();
      } else if (info.Length() < 3) {  // support for args like C++
        throw Napi::Error::New(env,
                               "georange must be called with 3 arguments: "
                               "latitude, longitude, and distance");
      }
      auto latitude = info[0].ToNumber();
      auto longitude = info[1].ToNumber();
      auto distance = info[2].ToNumber();
      query_->setGeorange(latitude, longitude, distance);
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getQuery(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), query_->m_query);
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getGeorange(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      if (!query_->m_geoquery) {
        return env.Null();
      }

      auto obj = Napi::Object::New(env);
      obj["latitude"] = query_->m_latitude;
      obj["longitude"] = query_->m_longitude;
      obj["distance"] = query_->m_distance;

      return obj;
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func = DefineClass(
        env, "Query",
        {
            InstanceMethod<&Query::setQuery>("setQuery"),
            InstanceAccessor<&Query::getQuery, &Query::setQuery>("query"),
            InstanceMethod<&Query::setGeorange>("setGeorange"),
            InstanceAccessor<&Query::getGeorange, &Query::setGeorangeObject>(
                "georange"),
            InstanceMethod<&Query::getQuery>("toString"),
        });

    exports.Set("Query", func);
    constructors.query = Napi::Persistent(func);
  }

  std::shared_ptr<zim::Query> query() { return query_; }

 private:
  std::shared_ptr<zim::Query> query_;
};

class SearchIterator : public Napi::ObjectWrap<SearchIterator> {
 public:
  explicit SearchIterator(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<SearchIterator>(info), searchIterator_{} {
    if (info[0].IsExternal()) {
      searchIterator_ =
          *info[0].As<Napi::External<zim::SearchIterator>>().Data();
    }
  }

  static Napi::Object New(Napi::Env env, zim::SearchIterator iterator) {
    auto external = Napi::External<zim::SearchIterator>::New(env, &iterator);
    auto &constructor =
        env.GetInstanceData<ModuleConstructors>()->searchIterator;
    return constructor.New({external});
  }

  Napi::Value getPath(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), searchIterator_.getPath());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getTitle(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), searchIterator_.getTitle());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getScore(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), searchIterator_.getScore());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getSnippet(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), searchIterator_.getSnippet());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getWordCount(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), searchIterator_.getWordCount());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getFileIndex(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), searchIterator_.getFileIndex());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getZimId(const Napi::CallbackInfo &info) {
    try {
      // TODO(kelvinhammond): convert this to
      // static_cast<std::string>(archive_->getUuid()) This didn't work when
      // building because of the below error undefined symbol:
      // _ZNK3zim4UuidcvNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEEv
      std::ostringstream out;
      out << searchIterator_.getZimId();
      return Napi::Value::From(info.Env(), out.str());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getEntry(const Napi::CallbackInfo &info) {
    try {
      auto iterator = zim::Entry(*searchIterator_);
      return Entry::New(info.Env(), std::move(iterator));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func = DefineClass(
        env, "SearchIterator",
        {
            InstanceAccessor<&SearchIterator::getPath>("path"),
            InstanceAccessor<&SearchIterator::getTitle>("title"),
            InstanceAccessor<&SearchIterator::getScore>("score"),
            InstanceAccessor<&SearchIterator::getSnippet>("snippet"),
            InstanceAccessor<&SearchIterator::getWordCount>("wordCount"),
            InstanceAccessor<&SearchIterator::getFileIndex>("fileIndex"),
            InstanceAccessor<&SearchIterator::getZimId>("zimId"),
            InstanceAccessor<&SearchIterator::getEntry>("entry"),
        });

    exports.Set("SearchIterator", func);
    constructors.searchIterator = Napi::Persistent(func);
  }

 private:
  zim::SearchIterator searchIterator_;
};

class SearchResultSet : public Napi::ObjectWrap<SearchResultSet> {
 public:
  explicit SearchResultSet(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<SearchResultSet>(info), searchResultSet_{nullptr} {
    Napi::Env env = info.Env();

    if (!info[0].IsExternal()) {
      throw Napi::Error::New(env,
                             "SearchResultSet must be created internally.");
    }

    searchResultSet_ =
        *info[0].As<Napi::External<decltype(searchResultSet_)>>().Data();
  }

  static Napi::Object New(Napi::Env env,
                          const zim::SearchResultSet &resultSet) {
    // done this way to avoid copying the zim::SearchResultSet during creation
    // also need to copy it to lose the const qualifier so just do it here.
    auto ptr = std::make_shared<zim::SearchResultSet>(resultSet);
    auto external = Napi::External<decltype(ptr)>::New(env, &ptr);
    auto &constructor =
        env.GetInstanceData<ModuleConstructors>()->searchResultSet;
    return constructor.New({external});
  }

  Napi::Value getSize(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), searchResultSet_->size());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getIterator(const Napi::CallbackInfo &info) {
    try {
      auto range = searchResultSet_;
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
                    // This only returns the entry and loses info
                    // res["value"] = Entry::New(env, zim::Entry(*it));
                    res["value"] = SearchIterator::New(env, it);
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
        DefineClass(env, "SearchResultSet",
                    {
                        InstanceAccessor<&SearchResultSet::getSize>("size"),
                        InstanceAccessor<&SearchResultSet::getIterator>(
                            Napi::Symbol::WellKnown(env, "iterator")),
                    });

    exports.Set("SearchResultSet", func);
    constructors.searchResultSet = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::SearchResultSet> searchResultSet_;
};

class Search : public Napi::ObjectWrap<Search> {
 public:
  explicit Search(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<Search>(info), search_{nullptr} {
    if (!info[0].IsExternal()) {
      throw Napi::Error::New(info.Env(), "Search must be created internally.");
    }

    search_ = std::make_shared<zim::Search>(
        std::move(*info[0].As<Napi::External<zim::Search>>().Data()));
  }

  static Napi::Object New(Napi::Env env, zim::Search search) {
    // NOTE: search will be std::move into a shared_ptr and invalid after this.
    auto external = Napi::External<zim::Search>::New(env, &search);
    auto &constructor = env.GetInstanceData<ModuleConstructors>()->search;
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
      return SearchResultSet::New(env, search_->getResults(start, maxResults));
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
        env, "Search",
        {
            InstanceMethod<&Search::getResults>("getResults"),
            InstanceAccessor<&Search::getEstimatedMatches>("estimatedMatches"),
        });

    exports.Set("Search", func);
    constructors.search = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::Search> search_;
};

class Searcher : public Napi::ObjectWrap<Searcher> {
 public:
  explicit Searcher(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<Searcher>(info), searcher_{nullptr} {
    Napi::Env env = info.Env();

    if (info[0].IsArray()) {
      auto array = info[0].As<Napi::Array>();
      if (array.Length() < 1) {
        throw Napi::Error::New(
            env, "Searcher: argument array must contain at least 1 archive.");
      }

      std::vector<zim::Archive> archives;
      for (size_t i = 0; i < array.Length(); i++) {
        auto obj = array.Get(i);
        if (!obj.IsObject()) {
          throw Napi::Error::New(
              env, "Searcher: array arguments must be Archive objects");
        }

        auto archive = Napi::ObjectWrap<Archive>::Unwrap(obj.As<Napi::Object>())
                           ->archive();
        archives.emplace_back(*archive);
      }

      searcher_ = std::make_shared<zim::Searcher>(archives);
    } else if (info[0].IsObject()) {  // one archive
      auto archive =
          Napi::ObjectWrap<Archive>::Unwrap(info[0].As<Napi::Object>())
              ->archive();
      searcher_ = std::make_shared<zim::Searcher>(*archive);
    } else {
      throw Napi::Error::New(env,
                             "Searcher: argument 1 must be an Archive object "
                             "or an array of Archive objects.");
    }
  }

  Napi::Value addArchive(const Napi::CallbackInfo &info) {
    try {
      if (!info[0].IsObject()) {
        throw Napi::Error::New(info.Env(),
                               "argument 1 must be an Archive object.");
      }

      auto archive =
          Napi::ObjectWrap<Archive>::Unwrap(info[0].As<Napi::Object>())
              ->archive();
      searcher_->addArchive(*archive);
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value search(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      if (info[0].IsString()) {
        // coerce to query
        auto &&query = zim::Query(info[0].ToString().Utf8Value());
        auto &&search = searcher_->search(query);
        return Search::New(env, std::move(search));
      } else if (info[0].IsObject()) {
        auto &&query =
            Napi::ObjectWrap<Query>::Unwrap(info[0].As<Napi::Object>())
                ->query();
        auto &&search = searcher_->search(*query);
        return Search::New(env, std::move(search));
      }

      throw Napi::Error::New(env, "search argument must be a query or string");
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value setVerbose(const Napi::CallbackInfo &info) {
    try {
      searcher_->setVerbose(info[0].ToBoolean());
      return info.This();
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::Function func =
        DefineClass(env, "Searcher",
                    {
                        InstanceMethod<&Searcher::addArchive>("addArchive"),
                        InstanceMethod<&Searcher::search>("search"),
                        InstanceMethod<&Searcher::setVerbose>("setVerbose"),
                    });

    exports.Set("Searcher", func);
    constructors.searcher = Napi::Persistent(func);
  }

 private:
  std::shared_ptr<zim::Searcher> searcher_;
};

