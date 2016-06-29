// Binding to zim::File

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include <zim/zim.h>
#include <zim/file.h>

#include <string>

// #define NODE_LIBZIM_TRACE

#include "src/article.h"
#include "src/cluster.h"
#include "src/dirent.h"
#include "src/file.h"
#include "src/fileheader.h"
#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

NAN_METHOD(FileWrap::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("You must use `new` with this constructor.");
  }
  REQUIRE_ARGUMENT_STD_STRING(0, fname);
  FileWrap *f = new FileWrap(new zim::File(fname));
  f->Wrap(info.Holder());
  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(FileWrap::getFileheader) {
  const zim::Fileheader &fh = getWrappedField(info)->getFileheader();
  // The fileheader is owned by this file object; pass along the file
  // object so the fileheader can hold a reference to it.
  // XXX store weak ref here in file as well to cache result?
  info.GetReturnValue().Set(FileheaderWrap::FromC(info.Holder(), &fh));
}
NAN_METHOD(FileWrap::getDirent) {
  REQUIRE_ARGUMENT_SIZE_T(0, idx);
  const zim::Dirent d = getWrappedField(info)->getDirent(idx);
  RETURN_DIRENT(d);
}
NAN_METHOD(FileWrap::getDirentByTitle) {
  REQUIRE_ARGUMENT_SIZE_T(0, idx);
  const zim::Dirent d = getWrappedField(info)->getDirent(idx);
  RETURN_DIRENT(d);
}
NAN_METHOD(FileWrap::getArticle) {
  zim::Article a;
  if (info.Length() == 1) {
    REQUIRE_ARGUMENT_SIZE_T(0, idx);
    a = getWrappedField(info)->getArticle(idx);
  } else if (info.Length() == 2) {
    REQUIRE_ARGUMENT_CHAR(0, ns);
    REQUIRE_ARGUMENT_STD_STRING(1, url);
    a = getWrappedField(info)->getArticle(ns, url);
  } else {
    return Nan::ThrowTypeError
      ("Must have single integer argument or two string arguments.");
  }
  RETURN_ARTICLE(a);
}
NAN_METHOD(FileWrap::getArticleByUrl) {
  REQUIRE_ARGUMENT_STD_STRING(0, url);
  zim::Article a = getWrappedField(info)->getArticleByUrl(url);
  RETURN_ARTICLE(a);
}
NAN_METHOD(FileWrap::getArticleByTitle) {
  zim::Article a;
  if (info.Length() == 1) {
    REQUIRE_ARGUMENT_SIZE_T(0, idx);
    a = getWrappedField(info)->getArticleByTitle(idx);
  } else if (info.Length() == 2) {
    REQUIRE_ARGUMENT_CHAR(0, ns);
    REQUIRE_ARGUMENT_STD_STRING(1, title);
    a = getWrappedField(info)->getArticleByTitle(ns, title);
  } else {
    return Nan::ThrowTypeError
      ("Must have single integer argument or two string arguments.");
  }
  RETURN_ARTICLE(a);
}
NAN_METHOD(FileWrap::getCluster) {
  REQUIRE_ARGUMENT_SIZE_T(0, idx);
  const zim::Cluster c = getWrappedField(info)->getCluster(idx);
  RETURN_CLUSTER(c);
}
NAN_METHOD(FileWrap::getClusterOffset) {
  REQUIRE_ARGUMENT_SIZE_T(0, idx);
  const zim::offset_type r = getWrappedField(info)->getClusterOffset(idx);
  RETURN_OFFSET_T(r);
}
NAN_METHOD(FileWrap::getBlob) {
  REQUIRE_ARGUMENT_SIZE_T(0, clusterIdx);
  REQUIRE_ARGUMENT_SIZE_T(1, blobIdx);
  const zim::Blob b = getWrappedField(info)->getBlob(clusterIdx, blobIdx);
  RETURN_BLOB(b);
}
NAN_METHOD(FileWrap::getNamespaceBeginOffset) {
  REQUIRE_ARGUMENT_CHAR(0, ns);
  zim::size_type sz = getWrappedField(info)->getNamespaceBeginOffset(ns);
  RETURN_SIZE_T(sz);
}
NAN_METHOD(FileWrap::getNamespaceEndOffset) {
  REQUIRE_ARGUMENT_CHAR(0, ns);
  zim::size_type sz = getWrappedField(info)->getNamespaceEndOffset(ns);
  RETURN_SIZE_T(sz);
}
NAN_METHOD(FileWrap::getNamespaceCount) {
  REQUIRE_ARGUMENT_CHAR(0, ns);
  zim::size_type sz = getWrappedField(info)->getNamespaceCount(ns);
  RETURN_SIZE_T(sz);
}
NAN_METHOD(FileWrap::hasNamespace) {
  REQUIRE_ARGUMENT_CHAR(0, ns);
  bool r = getWrappedField(info)->hasNamespace(ns);
  RETURN_BOOL(r);
}
NAN_METHOD(FileWrap::getMimeType) {
  REQUIRE_ARGUMENT_UINT16_T(0, idx);
  const std::string& r = getWrappedField(info)->getMimeType(idx);
  RETURN_STD_STRING(r);
}

}  // namespace node_libzim
