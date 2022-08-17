
#include <napi.h>

#include "archive.h"
#include "blob.h"
#include "common.h"
#include "contentProvider.h"
#include "creator.h"
#include "entry.h"
#include "item.h"
#include "search.h"
#include "suggestion.h"
#include "writerItem.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  // used for storing constructor function references for this instance of the
  // module. This allows the re-use of the module in workers and threads via
  // multiple instances of this module.
  auto constructors = new ModuleConstructors{};
  env.SetInstanceData(constructors);

  IntegrityCheck::Init(env, exports, *constructors);
  Compression::Init(env, exports, *constructors);

  Blob::Init(env, exports, *constructors);
  Item::Init(env, exports, *constructors);
  Entry::Init(env, exports, *constructors);
  Archive::Init(env, exports, *constructors);

  Searcher::Init(env, exports, *constructors);
  Query::Init(env, exports, *constructors);
  Search::Init(env, exports, *constructors);
  SearchResultSet::Init(env, exports, *constructors);
  SearchIterator::Init(env, exports, *constructors);

  SuggestionSearcher::Init(env, exports, *constructors);
  SuggestionSearch::Init(env, exports, *constructors);
  SuggestionResultSet::Init(env, exports, *constructors);
  SuggestionIterator::Init(env, exports, *constructors);

  StringProvider::Init(env, exports, *constructors);
  FileProvider::Init(env, exports, *constructors);
  Creator::Init(env, exports, *constructors);

  StringItem::Init(env, exports, *constructors);
  FileItem::Init(env, exports, *constructors);

  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)

