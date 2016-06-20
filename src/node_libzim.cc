// Binding to zimlib

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include "src/node_libzim_writer.h"
#include "src/macros.h"

namespace node_libzim {

// Tell node about our module!
NAN_MODULE_INIT(RegisterModule) {
  Nan::HandleScope scope;

  v8::Local<v8::Object> writer = Nan::New<v8::Object>();
  Nan::Set(target, NEW_STR("writer"), writer);
  writer::Init(writer);
}

NODE_MODULE(zim, RegisterModule)

}  // namespace node_libzim
