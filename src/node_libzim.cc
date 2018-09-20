// Binding to zimlib

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include "src/article.h"
#include "src/blob.h"
#include "src/cluster.h"
#include "src/dirent.h"
#include "src/file.h"
#include "src/fileheader.h"
#include "src/macros.h"
#include "src/uuid.h"
#include "src/writer.h"

namespace node_libzim {

// Tell node about our module!
NAN_MODULE_INIT(RegisterModule) {
  Nan::HandleScope scope;

  ArticleWrap::Init(target);
  BlobWrap::Init(target);
  ClusterWrap::Init(target);
  DirentWrap::Init(target);
  FileWrap::Init(target);
  FileheaderWrap::Init(target);
  UuidWrap::Init(target);

  v8::Local<v8::Object> writer = Nan::New<v8::Object>();
  Nan::Set(target, NEW_STR("writer"), writer);
  writer::Init(writer);
}

NODE_MODULE(zim, RegisterModule)

}  // namespace node_libzim
