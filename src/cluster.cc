// Binding to zim::Cluster

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include <zim/zim.h>
#include <zim/cluster.h>

// #define NODE_LIBZIM_TRACE

#include "src/blob.h"
#include "src/cluster.h"
#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

NAN_METHOD(ClusterWrap::setCompression) {
  REQUIRE_ARGUMENT_UINT16_T(0, cc);
  zim::CompressionType c = static_cast<zim::CompressionType>(cc);
  getWrappedField(info)->setCompression(c);
}
NAN_METHOD(ClusterWrap::getCompression) {
  zim::CompressionType c = getWrappedField(info)->getCompression();
  RETURN_UINT16_T(static_cast<uint16_t>(c));
}
NAN_METHOD(ClusterWrap::getBlob) {
  REQUIRE_ARGUMENT_SIZE_T(0, n);
  zim::Blob b = getWrappedField(info)->getBlob(n);
  RETURN_BLOB(b);
}
NAN_METHOD(ClusterWrap::getBlobSize) {
  REQUIRE_ARGUMENT_SIZE_T(0, n);
  zim::size_type s = getWrappedField(info)->getBlobSize(n);
  RETURN_SIZE_T(s);
}
NAN_METHOD(ClusterWrap::clear) {
  getWrappedField(info)->clear();
  v8::Local<v8::String> hidden_field = NEW_STR("zim::ClusterWrap::refs");
  info.Holder()->SetHiddenValue(hidden_field, Nan::Undefined());
}
NAN_METHOD(ClusterWrap::addBlob) {
  REQUIRE_ARGUMENTS(1);
  v8::Local<v8::Value> arg = info[0];
  if (node::Buffer::HasInstance(info[0])) {
    v8::Local<v8::Function> cons = BlobWrap::constructor();
    v8::Local<v8::Value> argv[] = {
      arg,
    };
    arg = cons->NewInstance(1, argv);
  }
  zim::Blob b;
  {
    Nan::TryCatch try_;
    b = BlobWrap::FromJS(arg);
    if (try_.HasCaught()) { try_.ReThrow(); return; }
  }
  getWrappedField(info)->addBlob(b);
  // Record dependency link between this clusterwrap and blobwrap
  // so that blob isn't GC'ed before cluster.
  v8::Local<v8::String> hidden_field = NEW_STR("zim::ClusterWrap::refs");
  v8::Local<v8::Value> refs = Nan::GetPrivate(info.Holder(), hidden_field)
    .ToLocalChecked();
  if (refs.IsEmpty() || !refs->IsArray()) {
    refs = Nan::New<v8::Array>();
    info.Holder()->SetHiddenValue(hidden_field, refs);
  }
  v8::Local<v8::Array> refsArray = v8::Local<v8::Array>::Cast(refs);
  Nan::Set(refsArray, refsArray->Length(), arg);
}
NAN_METHOD(ClusterWrap::good) {
  bool b = static_cast<bool>(getWrappedField(info));
  RETURN_BOOL(b);
}

}  // namespace node_libzim
