// Binding to zim::Uuid

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_UUID_H_
#define NODE_LIBZIM_UUID_H_

#include "nan.h"

#include <zim/zim.h>
#include <zim/uuid.h>

#include <sstream>

#include "src/macros.h"

namespace node_libzim {

class UuidWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::String> class_name = NEW_STR("Uuid");
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(class_name);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetMethod(tpl, "generate", generate);
    Nan::SetPrototypeMethod(tpl, "data", data);
    Nan::SetPrototypeMethod(tpl, "size", size);
    Nan::SetPrototypeMethod(tpl, "toString", toString);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, class_name, Nan::GetFunction(tpl).ToLocalChecked());
  }
  static v8::Local<v8::Object> FromC(zim::Uuid u) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Value> argv[] = {
      Nan::New<v8::External>(&u)
    };
    return scope.Escape(Nan::New(constructor())->NewInstance(1, argv));
  }
  static zim::Uuid FromJS(v8::Local<v8::Value> v) {
    if (v->IsObject()) {
      Nan::HandleScope scope;
      v8::Local<v8::Object> o = Nan::To<v8::Object>(v).ToLocalChecked();
      v8::Local<v8::String> hidden_field = NEW_STR("zim::UuidWrap");
      v8::Local<v8::Value> b = o->GetHiddenValue(hidden_field);
      if (!b.IsEmpty() && node::Buffer::HasInstance(b)) {
        v8::Local<v8::Object> bo = Nan::To<v8::Object>(b).ToLocalChecked();
        return zim::Uuid(node::Buffer::Data(bo));
      }
    }
    Nan::ThrowTypeError("Not a Uuid.");
    return zim::Uuid();
  }

 private:
  explicit UuidWrap(zim::Uuid *uuid) {
    uuid_ = *uuid;
    char *data = uuid_.data;
    unsigned size = uuid_.size();
    Nan::MaybeLocal<v8::Object> buf =
        Nan::NewBuffer(data, size, DontFree, NULL);
    buffer_.Reset(buf.ToLocalChecked());
  }
  explicit UuidWrap(v8::Local<v8::Object> buffer) {
    buffer_.Reset(buffer);
    uuid_ = zim::Uuid(node::Buffer::Data(buffer));
  }
  ~UuidWrap() {
    buffer_.Reset();
  }

  static void DontFree(char *data, void *hint) { /* no op */ }

  static NAN_METHOD(New) {
    if (!info.IsConstructCall()) {
      return Nan::ThrowTypeError("You must use `new` with this constructor.");
    }
    UuidWrap *u;
    if (info[0]->IsExternal()) {
      // Create JS wrapper for existing Uuid; data is not owned.
      zim::Uuid* zu = reinterpret_cast<zim::Uuid*>
        (v8::Local<v8::External>::Cast(info[0])->Value());
      u = new UuidWrap(zu);
    } else if (node::Buffer::HasInstance(info[0]) &&
               node::Buffer::Length(Nan::To<v8::Object>
                                    (info[0]).ToLocalChecked()) == 16) {
      // Create new JS object representing uuid; we own data.
      u = new UuidWrap(Nan::To<v8::Object>(info[0]).ToLocalChecked());
    } else {
      return Nan::ThrowTypeError
        ("Constructor argument must be a Buffer of length 16.");
    }
    v8::Local<v8::String> hidden_field = NEW_STR("zim::UuidWrap");
    info.This()->SetHiddenValue(hidden_field, Nan::New(u->buffer_));
    u->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }

  static NAN_METHOD(data) {
    UuidWrap* u = Nan::ObjectWrap::Unwrap<UuidWrap>(info.Holder());
    info.GetReturnValue().Set(Nan::New(u->buffer_));
  }

  static NAN_METHOD(size) {
    UuidWrap* u = Nan::ObjectWrap::Unwrap<UuidWrap>(info.Holder());
    info.GetReturnValue().Set(Nan::New(u->uuid_.size()));
  }

  static NAN_METHOD(toString) {
    UuidWrap* u = Nan::ObjectWrap::Unwrap<UuidWrap>(info.Holder());
    std::ostringstream data;
    data << u->uuid_;
    info.GetReturnValue().Set(NEW_STR(data.str()));
  }

  static NAN_METHOD(generate) {
    info.GetReturnValue().Set(FromC(zim::Uuid::generate()));
  }

  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }

  zim::Uuid uuid_;
  Nan::Persistent<v8::Object> buffer_;
};

}  // namespace node_libzim
#endif  // NODE_LIBZIM_UUID_H_
