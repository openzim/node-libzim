// Binding to zim::Dirent

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include <zim/zim.h>
#include <zim/dirent.h>

// #define NODE_LIBZIM_TRACE

#include "src/dirent.h"
#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

NAN_METHOD(DirentWrap::setCluster) {
  REQUIRE_ARGUMENT_SIZE_T(0, clusterNumber);
  REQUIRE_ARGUMENT_SIZE_T(1, blobNumber);
  getWrappedField(info)->setCluster(clusterNumber, blobNumber);
}
NAN_METHOD(DirentWrap::setUrl) {
  REQUIRE_ARGUMENT_CHAR(0, ns);
  REQUIRE_ARGUMENT_STD_STRING(1, url);
  getWrappedField(info)->setUrl(ns, url);
}
NAN_METHOD(DirentWrap::setArticle) {
  REQUIRE_ARGUMENT_UINT16_T(0, mimeType);
  REQUIRE_ARGUMENT_SIZE_T(1, clusterNumber);
  REQUIRE_ARGUMENT_SIZE_T(2, blobNumber);
  getWrappedField(info)->setArticle(mimeType, clusterNumber, blobNumber);
}

}  // namespace node_libzim
