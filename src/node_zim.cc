// Binding to zimlib

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include <zim/writer/zimcreator.h>
#include <zim/blob.h>

#include "./macros.h"

class ZimCreatorWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("ZimCreator").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // SetPrototypeMethod(tpl, "getHandle", GetHandle);
    // SetPrototypeMethod(tpl, "getValue", GetValue);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("ZimCreator").ToLocalChecked(),
      Nan::GetFunction(tpl).ToLocalChecked());
  }

 private:
  explicit ZimCreatorWrap(int argc, char *argv[]) : creator_(argc, argv) {
  }
  ~ZimCreatorWrap() {}
  zim::writer::ZimCreator creator_;

  static NAN_METHOD(New) {
    if (info.IsConstructCall()) {
      // XXX parse info[0] as string array
      ZimCreatorWrap *obj = new ZimCreatorWrap(0, NULL);
      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      const int argc = 1;
      v8::Local<v8::Value> argv[argc] = {info[0]};
      v8::Local<v8::Function> cons = Nan::New(constructor());
      info.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
  }

  /*
  static NAN_METHOD(GetHandle) {
    MyObject* obj = Nan::ObjectWrap::Unwrap<MyObject>(info.Holder());
    info.GetReturnValue().Set(obj->handle());
  }

  static NAN_METHOD(GetValue) {
    MyObject* obj = Nan::ObjectWrap::Unwrap<MyObject>(info.Holder());
    info.GetReturnValue().Set(obj->value_);
  }
  */

  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
};


// Tell node about our module!
NAN_MODULE_INIT(RegisterModule) {
  Nan::HandleScope scope;

  ZimCreatorWrap::Init(target);
}

NODE_MODULE(zim, RegisterModule)
