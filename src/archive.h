#pragma once

#include <napi.h>
#include <zim/archive.h>
#include <exception>
#include <memory>
#include <sstream>

#include "entry.h"
#include "item.h"

class Archive : public Napi::ObjectWrap<Archive> {
 public:
  explicit Archive(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<Archive>(info), archive_{nullptr} {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    auto opts = info[0].ToObject();
    if (!opts.Has("fileName")) {
      throw Napi::Error::New(env, "fileName required");
    }

    std::string filename = opts.Get("fileName").ToString();
    try {
      archive_ = std::make_shared<zim::Archive>(filename);
    } catch (const std::exception &e) {
      throw Napi::Error::New(env, e.what());
    }
  }

  Napi::Value getFilename(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->getFilename());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getFilesize(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->getFilesize());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getAllEntryCount(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->getAllEntryCount());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getEntryCount(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->getEntryCount());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getArticleCount(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->getArticleCount());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getUuid(const Napi::CallbackInfo &info) {
    try {
      // TODO: convert this to static_cast<std::string>(archive_->getUuid())
      // This didn't work when building because of the below error
      // undefined symbol:
      // _ZNK3zim4UuidcvNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEEv
      std::ostringstream out;
      out << archive_->getUuid();
      return Napi::Value::From(info.Env(), out.str());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getMetadata(const Napi::CallbackInfo &info) {
    try {
      auto name = info[0].ToString();
      return Napi::Value::From(info.Env(), archive_->getMetadata(name));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  /* TODO: enable on next release
  Napi::Value getMetadataItem(const Napi::CallbackInfo &info) {
    try {
      auto name = info[0].ToString();
      return Item::New(info.Env(), archive_->getMetadataItem(name));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }
  */

  Napi::Value getMetadataKeys(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      Napi::HandleScope scope(env);
      auto res = Napi::Array::New(env);
      size_t idx = 0;
      for (const auto &key : archive_->getMetadataKeys()) {
        res.Set(idx++, Napi::String::New(env, key));
      }
      return res;
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getIllustrationItem(const Napi::CallbackInfo &info) {
    try {
      if (info.Length() > 0) {
        auto size = static_cast<unsigned int>(info[0].ToNumber().Uint32Value());
        return Item::New(info.Env(), archive_->getIllustrationItem(size));
      }
      return Item::New(info.Env(), archive_->getIllustrationItem());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getIllustrationSizes(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      Napi::HandleScope scope(env);

      // returns a native Set object
      auto SetConstructor = env.Global().Get("Set").As<Napi::Function>();
      auto result = SetConstructor.New({});
      auto add = result.Get("add").As<Napi::Function>();
      for (const auto &size : archive_->getIllustrationSizes()) {
        add.Call(result, {Napi::Value::From(env, size)});
      }
      return result;
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getEntryByPath(const Napi::CallbackInfo &info) {
    try {
      if (info[0].IsNumber()) {
        auto &&idx = info[0].ToNumber();
        return Entry::New(info.Env(), archive_->getEntryByPath(idx));
      } else if (info[0].IsString()) {
        auto &&path = info[0].ToString();
        return Entry::New(info.Env(), archive_->getEntryByPath(path));
      }

      throw Napi::Error::New(
          info.Env(), "Entry index must be a string (path) or number (index).");
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getEntryByTitle(const Napi::CallbackInfo &info) {
    try {
      if (info[0].IsNumber()) {
        auto &&idx = info[0].ToNumber();
        return Entry::New(info.Env(), archive_->getEntryByTitle(idx));
      } else if (info[0].IsString()) {
        auto &&path = info[0].ToString();
        return Entry::New(info.Env(), archive_->getEntryByTitle(path));
      }

      throw Napi::Error::New(
          info.Env(), "Entry index must be a string (path) or number (index).");
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getEntryByClusterOrder(const Napi::CallbackInfo &info) {
    try {
      if (info[0].IsNumber()) {
        auto &&idx = info[0].ToNumber();
        return Entry::New(info.Env(), archive_->getEntryByClusterOrder(idx));
      }
      throw Napi::Error::New(info.Env(), "Entry index must be a number.");
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getMainEntry(const Napi::CallbackInfo &info) {
    try {
      return Entry::New(info.Env(), archive_->getMainEntry());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getRandomEntry(const Napi::CallbackInfo &info) {
    try {
      return Entry::New(info.Env(), archive_->getRandomEntry());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value hasEntryByPath(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(),
                               archive_->hasEntryByPath(info[0].ToString()));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value hasEntryByTitle(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(),
                               archive_->hasEntryByTitle(info[0].ToString()));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value hasMainEntry(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->hasMainEntry());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value hasIllustration(const Napi::CallbackInfo &info) {
    try {
      if (info.Length() > 0) {
        auto size = static_cast<unsigned int>(info[0].ToNumber().Uint32Value());
        return Napi::Value::From(info.Env(), archive_->hasIllustration(size));
      }
      return Napi::Value::From(info.Env(), archive_->hasIllustration());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value hasFulltextIndex(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->hasFulltextIndex());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value hasTitleIndex(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->hasTitleIndex());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  template <typename RangeT>
  static Napi::Value NewEntryRange(Napi::Env env, RangeT range) {
    // should be called from C++ only, exceptions propogated up.
    Napi::Object iterable = Napi::Object::New(env);
    Napi::Function iterator = Napi::Function::New(
        env, [range](const Napi::CallbackInfo &info) mutable -> Napi::Value {
          Napi::Env env = info.Env();
          Napi::Object iter = Napi::Object::New(env);

          auto it = range.begin();
          iter["next"] = Napi::Function::New(
              env,
              [range,
               it](const Napi::CallbackInfo &info) mutable -> Napi::Value {
                Napi::Env env = info.Env();
                Napi::Object res = Napi::Object::New(env);
                if (it != range.end()) {
                  res["done"] = false;
                  res["value"] = Entry::New(env, zim::Entry(*it));
                  it++;
                } else {
                  res["done"] = true;
                }
                return res;
              });
          return iter;
        });

    iterable.Set(Napi::Symbol::WellKnown(env, "iterator"), iterator);
    iterable["size"] = Napi::Value::From(env, range.size());
    iterable["offset"] = Napi::Function::New(
        env, [range](const Napi::CallbackInfo &info) -> Napi::Value {
          if (info.Length() < 2) {
            throw Napi::Error::New(
                info.Env(), "start and maxResults are required for offset.");
          }
          if (!(info[0].IsNumber() && info[1].IsNumber())) {
            throw Napi::Error::New(
                info.Env(), "start and maxResults must be of type Number.");
          }
          auto start = info[0].ToNumber();
          auto maxResults = info[1].ToNumber();
          return NewEntryRange(info.Env(), range.offset(start, maxResults));
        });
    iterable.Freeze();

    return iterable;
  }

  Napi::Value iterByPath(const Napi::CallbackInfo &info) {
    try {
      return NewEntryRange(info.Env(), archive_->iterByPath());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value iterByTitle(const Napi::CallbackInfo &info) {
    try {
      return NewEntryRange(info.Env(), archive_->iterByTitle());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value iterEfficient(const Napi::CallbackInfo &info) {
    try {
      return NewEntryRange(info.Env(), archive_->iterEfficient());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value findByPath(const Napi::CallbackInfo &info) {
    try {
      auto path = info[0].ToString();
      return NewEntryRange(info.Env(), archive_->findByPath(path));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value findByTitle(const Napi::CallbackInfo &info) {
    try {
      auto title = info[0].ToString();
      return NewEntryRange(info.Env(), archive_->findByTitle(title));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value hasChecksum(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->hasChecksum());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value getChecksum(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->getChecksum());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value check(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->check());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value checkIntegrity(const Napi::CallbackInfo &info) {
    try {
      auto env = info.Env();
      const auto &&checkType = IntegrityCheck::symbolToEnum(env, info[0]);
      return Napi::Value::From(env, archive_->checkIntegrity(checkType));
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value isMultiPart(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->isMultiPart());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  Napi::Value hasNewNamespaceScheme(const Napi::CallbackInfo &info) {
    try {
      return Napi::Value::From(info.Env(), archive_->hasNewNamespaceScheme());
    } catch (const std::exception &err) {
      throw Napi::Error::New(info.Env(), err.what());
    }
  }

  static Napi::Value validate(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    try {
      if (info.Length() < 2) {
        throw Napi::Error::New(
            env, "validate requires zimPath and [IntegrityCheck, ...]");
      } else if (!info[0].IsString()) {
        throw Napi::Error::New(env, "zimPath must be a string");
      } else if (!info[1].IsArray()) {
        throw Napi::Error::New(env, "IntegrityCheckList must be an array");
      }

      auto &&zimPath = info[0].ToString();
      auto symbolList = info[1].As<Napi::Array>();
      zim::IntegrityCheckList flags{};
      for (size_t i = 0; i < symbolList.Length(); i++) {
        const auto bit = IntegrityCheck::symbolToEnum(env, symbolList.Get(i));
        auto &&isAll = (bit == zim::IntegrityCheck::COUNT ||
                        static_cast<size_t>(bit) >= flags.size());
        if (isAll) {  // This handle IntegrityCheck::COUNT
          flags.set();
          break;
        }
        flags.set(static_cast<size_t>(bit));
      }
      return Napi::Value::From(env, zim::validate(zimPath, flags));
    } catch (const std::exception &err) {
      throw Napi::Error::New(env, err.what());
    }
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors &constructors) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(
        env, "Archive",
        {
            InstanceAccessor<&Archive::getFilename>("filename"),
            InstanceAccessor<&Archive::getFilesize>("filesize"),
            InstanceAccessor<&Archive::getAllEntryCount>("allEntryCount"),
            InstanceAccessor<&Archive::getEntryCount>("entryCount"),
            InstanceAccessor<&Archive::getArticleCount>("articleCount"),
            InstanceAccessor<&Archive::getUuid>("uuid"),
            InstanceMethod<&Archive::getMetadata>("getMetadata"),
            // TODO: is included in next version
            // InstanceMethod<&Archive::getMetadataItem>("getMetadataItem"),
            InstanceAccessor<&Archive::getMetadataKeys>("metadataKeys"),
            InstanceMethod<&Archive::getIllustrationItem>(
                "getIllustrationItem"),
            InstanceAccessor<&Archive::getIllustrationSizes>(
                "illustrationSizes"),
            InstanceMethod<&Archive::getEntryByPath>("getEntryByPath"),
            InstanceMethod<&Archive::getEntryByTitle>("getEntryByTitle"),
            InstanceMethod<&Archive::getEntryByClusterOrder>(
                "getEntryByClusterOrder"),
            InstanceAccessor<&Archive::getMainEntry>("mainEntry"),
            InstanceAccessor<&Archive::getRandomEntry>("randomEntry"),
            InstanceMethod<&Archive::getRandomEntry>("getRandomEntry"),
            InstanceMethod<&Archive::hasEntryByPath>("hasEntryByPath"),
            InstanceMethod<&Archive::hasEntryByTitle>("hasEntryByTitle"),
            InstanceMethod<&Archive::hasMainEntry>("hasMainEntry"),
            InstanceMethod<&Archive::hasIllustration>("hasIllustration"),
            InstanceMethod<&Archive::hasFulltextIndex>("hasFulltextIndex"),
            InstanceMethod<&Archive::hasTitleIndex>("hasTitleIndex"),
            InstanceMethod<&Archive::iterByPath>("iterByPath"),
            InstanceMethod<&Archive::iterByTitle>("iterByTitle"),
            InstanceMethod<&Archive::iterEfficient>("iterEfficient"),
            InstanceMethod<&Archive::findByPath>("findByPath"),
            InstanceMethod<&Archive::findByTitle>("findByTitle"),
            InstanceAccessor<&Archive::hasChecksum>("hasChecksum"),
            InstanceAccessor<&Archive::getChecksum>("checksum"),
            InstanceMethod<&Archive::check>("check"),
            InstanceMethod<&Archive::checkIntegrity>("checkIntegrity"),
            InstanceAccessor<&Archive::isMultiPart>("isMultiPart"),
            InstanceAccessor<&Archive::hasNewNamespaceScheme>(
                "hasNewNamespaceScheme"),
            StaticMethod<&Archive::validate>("validate"),
        });

    exports.Set("Archive", func);
    constructors.archive = Napi::Persistent(func);
  }

  // internal module methods
  std::shared_ptr<zim::Archive> archive() { return archive_; }

 private:
  std::shared_ptr<zim::Archive> archive_;
};
