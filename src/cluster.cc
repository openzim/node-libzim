// Binding to zim::Cluster

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include <zim/zim.h>
#include <zim/cluster.h>

// #define NODE_LIBZIM_TRACE

#include "src/cluster.h"
#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

NAN_METHOD(ClusterWrap::setCompression) {
}
NAN_METHOD(ClusterWrap::getCompression) {
}
NAN_METHOD(ClusterWrap::getBlob) {
}
NAN_METHOD(ClusterWrap::getBlobSize) {
}
NAN_METHOD(ClusterWrap::addBlob) {
}
NAN_METHOD(ClusterWrap::good) {
}

}  // namespace node_libzim
