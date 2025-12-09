#pragma once

#include <napi.h>
#include <zim/zim.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using IntegrityCheckMap =
    std::vector<std::pair<zim::IntegrityCheck, Napi::Reference<Napi::Symbol>>>;

using CompressionMap =
    std::vector<std::pair<zim::Compression, Napi::Reference<Napi::Symbol>>>;

struct ModuleConstructors {
  Napi::FunctionReference archive;
  Napi::FunctionReference openConfig;
  Napi::FunctionReference illustrationInfo;
  Napi::FunctionReference entry;
  Napi::FunctionReference item;
  Napi::FunctionReference blob;

  Napi::FunctionReference searcher;
  Napi::FunctionReference query;
  Napi::FunctionReference search;
  Napi::FunctionReference searchResultSet;
  Napi::FunctionReference searchIterator;

  Napi::FunctionReference suggestionSearcher;
  Napi::FunctionReference suggestionSearch;
  Napi::FunctionReference suggestionResultSet;
  Napi::FunctionReference suggestionIterator;

  Napi::FunctionReference stringProvider;
  Napi::FunctionReference fileProvider;
  Napi::FunctionReference creator;

  Napi::FunctionReference stringItem;
  Napi::FunctionReference fileItem;

  Napi::FunctionReference compression;
  CompressionMap compressionMap;

  Napi::FunctionReference integrityCheck;
  IntegrityCheckMap integrityCheckMap;
};

class Compression : public Napi::ObjectWrap<Compression> {
 public:
  explicit Compression(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<Compression>(info) {}

  static zim::Compression symbolToEnum(Napi::Env env,
                                       const Napi::Value& value) {
    if (!value.IsSymbol()) {
      throw Napi::Error::New(env,
                             "Value must be a symbol for Compression value.");
    }

    auto& compressionMap =
        env.GetInstanceData<ModuleConstructors>()->compressionMap;
    Napi::HandleScope scope(env);
    for (const auto& [bit, symbolRef] : compressionMap) {
      if (!symbolRef.IsEmpty() && symbolRef.Value() == value) {
        return bit;
      }
    }
    throw Napi::Error::New(env, "Invalid Symbol for Compression value.");
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors& constructors) {
    Napi::HandleScope scope(env);

    constexpr auto attrs =
        static_cast<napi_property_attributes>(napi_default | napi_enumerable);
    std::vector<PropertyDescriptor> props;
    props.reserve(7);

    const auto&& values = std::vector<std::pair<zim::Compression, const char*>>{
        {zim::Compression::None, "None"},
        {zim::Compression::Zstd, "Zstd"},
    };
    for (const auto& [value, name] : values) {
      auto symbol = Napi::Symbol::New(env, name);
      constructors.compressionMap.push_back({value, Napi::Persistent(symbol)});
      props.push_back(StaticValue(name, symbol, attrs));
    }

    Napi::Function func = DefineClass(env, "Compression", props);
    exports.Set("Compression", func);
    constructors.compression = Napi::Persistent(func);
  }
};

class IntegrityCheck : public Napi::ObjectWrap<IntegrityCheck> {
 public:
  explicit IntegrityCheck(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<IntegrityCheck>(info) {}

  static zim::IntegrityCheck symbolToEnum(Napi::Env env,
                                          const Napi::Value& value) {
    if (!value.IsSymbol()) {
      throw Napi::Error::New(
          env, "Value must be a symbol for IntegrityCheck value.");
    }
    auto& integrityCheckMap =
        env.GetInstanceData<ModuleConstructors>()->integrityCheckMap;
    Napi::HandleScope scope(env);
    for (const auto& [bit, symbolRef] : integrityCheckMap) {
      if (!symbolRef.IsEmpty() && symbolRef.Value() == value) {
        return bit;
      }
    }
    throw Napi::Error::New(env, "Invalid Symbol for IntegrityCheck value.");
  }

  static void Init(Napi::Env env, Napi::Object exports,
                   ModuleConstructors& constructors) {
    Napi::HandleScope scope(env);

    constexpr auto attrs =
        static_cast<napi_property_attributes>(napi_default | napi_enumerable);
    std::vector<PropertyDescriptor> props;
    props.reserve(7);

    const auto&& values =
        std::vector<std::pair<zim::IntegrityCheck, const char*>>{
            {zim::IntegrityCheck::CHECKSUM, "CHECKSUM"},
            {zim::IntegrityCheck::DIRENT_PTRS, "DIRENT_PTRS"},
            {zim::IntegrityCheck::DIRENT_ORDER, "DIRENT_ORDER"},
            {zim::IntegrityCheck::TITLE_INDEX, "TITLE_INDEX"},
            {zim::IntegrityCheck::CLUSTER_PTRS, "CLUSTER_PTRS"},
            {zim::IntegrityCheck::DIRENT_MIMETYPES, "DIRENT_MIMETYPES"},
            {zim::IntegrityCheck::COUNT, "COUNT"},
        };
    for (const auto& [value, name] : values) {
      auto symbol = Napi::Symbol::New(env, name);
      constructors.integrityCheckMap.push_back(
          {value, Napi::Persistent(symbol)});
      props.push_back(StaticValue(name, symbol, attrs));
    }

    Napi::Function func = DefineClass(env, "IntegrityCheck", props);
    exports.Set("IntegrityCheck", func);
    constructors.integrityCheck = Napi::Persistent(func);
  }
};

