// Binding to zim::Blob

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_BLOB_H_
#define NODE_LIBZIM_BLOB_H_

#include "nan.h"

#include <zim/zim.h>
#include <zim/blob.h>

#include "src/macros.h"

namespace node_libzim {

class BlobWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::String> class_name = NEW_STR("Blob");
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(class_name);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "data", data);
    Nan::SetPrototypeMethod(tpl, "size", size);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, class_name, Nan::GetFunction(tpl).ToLocalChecked());
  }
  static v8::Local<v8::Object> FromC(zim::Blob b, bool owned) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Value> argv[] = {
      Nan::New<v8::External>(&b),
      Nan::New(owned)
    };
    return scope.Escape(Nan::New(constructor())->NewInstance(2, argv));
  }
  static zim::Blob FromJS(v8::Local<v8::Value> v) {
    if (v->IsObject()) {
      Nan::HandleScope scope;
      v8::Local<v8::Object> o = Nan::To<v8::Object>(v).ToLocalChecked();
      v8::Local<v8::String> hidden_field = NEW_STR("zim::BlobWrap");
      v8::Local<v8::Value> b = o->GetHiddenValue(hidden_field);
      if (!b.IsEmpty() && node::Buffer::HasInstance(b)) {
        v8::Local<v8::Object> bo = Nan::To<v8::Object>(b).ToLocalChecked();
        return zim::Blob(node::Buffer::Data(bo), node::Buffer::Length(bo));
      }
    }
    Nan::ThrowTypeError("Not a Blob.");
    return zim::Blob(NULL, 0);
  }

 private:
  explicit BlobWrap(zim::Blob *blob, bool owned) {
    blob_ = *blob;
    char *data = const_cast<char*>(blob_.data());  // Sketchy!
    unsigned size = blob_.size();
    Nan::MaybeLocal<v8::Object> buf = owned ?
      Nan::NewBuffer(data, size) :
      Nan::NewBuffer(data, size, DontFree, NULL);
    buffer_.Reset(buf.ToLocalChecked());
  }
  explicit BlobWrap(v8::Local<v8::Object> buffer) {
    buffer_.Reset(buffer);
    blob_ = zim::Blob(node::Buffer::Data(buffer), node::Buffer::Length(buffer));
  }
  ~BlobWrap() {
    buffer_.Reset();
  }

  static void DontFree(char *data, void *hint) { /* no op */ }

  static NAN_METHOD(New) {
    if (!info.IsConstructCall()) {
      return Nan::ThrowTypeError("You must use `new` with this constructor.");
    }
    BlobWrap *b;
    if (info[0]->IsExternal()) {
      // Create JS wrapper for existing Blob; data is not owned.
      zim::Blob* zb = reinterpret_cast<zim::Blob*>
        (v8::Local<v8::External>::Cast(info[0])->Value());
      b = new BlobWrap(zb, info.Length() > 1 ? info[1]->IsTrue() : false);
    } else if (node::Buffer::HasInstance(info[0])) {
      // Create new JS object representing blob; we own data.
      b = new BlobWrap(Nan::To<v8::Object>(info[0]).ToLocalChecked());
    } else {
      return Nan::ThrowTypeError("Constructor argument must be a Buffer.");
    }
    v8::Local<v8::String> hidden_field = NEW_STR("zim::BlobWrap");
    info.This()->SetHiddenValue(hidden_field, Nan::New(b->buffer_));
    b->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }

  static NAN_METHOD(data) {
    BlobWrap* b = Nan::ObjectWrap::Unwrap<BlobWrap>(info.Holder());
    info.GetReturnValue().Set(Nan::New(b->buffer_));
  }

  static NAN_METHOD(size) {
    BlobWrap* b = Nan::ObjectWrap::Unwrap<BlobWrap>(info.Holder());
    info.GetReturnValue().Set(Nan::New(b->blob_.size()));
  }

  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }

  zim::Blob blob_;
  Nan::Persistent<v8::Object> buffer_;
};

}  // namespace node_libzim
#endif  // NODE_LIBZIM_BLOB_H_
