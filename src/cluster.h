// Binding to zim::Cluster

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_CLUSTER_H_
#define NODE_LIBZIM_CLUSTER_H_

#include "nan.h"

#include <zim/zim.h>
#include <zim/cluster.h>

#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

class ClusterWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "Cluster");

    WRAPPER_METHOD_INIT(tpl, setCompression);
    WRAPPER_METHOD_INIT(tpl, getCompression);
    WRAPPER_METHOD_INIT(tpl, isCompressed);
    WRAPPER_METHOD_INIT(tpl, getBlob);
    WRAPPER_METHOD_INIT(tpl, getBlobSize);
    WRAPPER_METHOD_INIT(tpl, count);
    WRAPPER_METHOD_INIT(tpl, size);
    WRAPPER_METHOD_INIT(tpl, clear);
    WRAPPER_METHOD_INIT(tpl, addBlob);
    WRAPPER_METHOD_INIT(tpl, good);

    WRAPPER_INIT_FINISH(tpl);
  }

  WRAPPER_DEFINE_VALUE(ClusterWrap, zim::Cluster, cluster)
  WRAPPER_DEFINE_VALUE_DEFAULT_NEW(ClusterWrap, zim::Cluster, cluster)

 private:
  WRAPPER_METHOD_DECLARE(setCompression);
  WRAPPER_METHOD_DECLARE(getCompression);
  WRAPPER_METHOD_DECLARE_GET(isCompressed, BOOL);
  WRAPPER_METHOD_DECLARE(getBlob);
  WRAPPER_METHOD_DECLARE(getBlobSize);
  WRAPPER_METHOD_DECLARE_GET(count, SIZE_T);
  WRAPPER_METHOD_DECLARE_GET(size, SIZE_T);
  WRAPPER_METHOD_DECLARE_SET(clear, VOID);
  WRAPPER_METHOD_DECLARE(addBlob);
  WRAPPER_METHOD_DECLARE(good);
};

}  // namespace node_libzim

#endif  // NODE_LIBZIM_CLUSTER_H_
