
#include <napi.h>

#include "article.h"
#include "reader.h"
#include "writer.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Article::Init(env, exports);
  ZimCreatorWrapper::Init(env, exports);
  ZimReaderWrapper::Init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)

