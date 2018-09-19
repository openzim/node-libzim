// Binding to zim::Fileheader

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include <zim/zim.h>
#include <zim/fileheader.h>

// #define NODE_LIBZIM_TRACE

#include "src/fileheader.h"
#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

NAN_METHOD(FileheaderWrap::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("You must use `new` with this constructor.");
  }
  if (info.Length() == 2 && info[0]->IsObject() && info[1]->IsExternal()) {
    v8::Local<v8::Object> obj = Nan::To<v8::Object>(info[0]).ToLocalChecked();
    zim::Fileheader *fh = reinterpret_cast<zim::Fileheader *>
      (v8::Local<v8::External>::Cast(info[1])->Value());
    FileheaderWrap *w = new FileheaderWrap(obj, fh);
    w->Wrap(info.This());
  } else {
    return Nan::ThrowTypeError("You can not construct a Fileheader directly.");
  }
}
v8::Local<v8::Object> FileheaderWrap::FromC(v8::Local<v8::Object> file,
                                            const zim::Fileheader *fileheader) {
  Nan::EscapableHandleScope scope;
  v8::Local<v8::Value> argv[] = {
    file,
    Nan::New<v8::External>(const_cast<zim::Fileheader*>(fileheader))
  };
  return scope.Escape(Nan::NewInstance(constructor(), 2, argv).ToLocalChecked());
}

}  // namespace node_libzim
