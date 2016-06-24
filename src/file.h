// Binding to zim::File

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_FILE_H_
#define NODE_LIBZIM_FILE_H_

#include "nan.h"

#include <zim/zim.h>
#include <zim/file.h>

#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

class FileWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "File");

    WRAPPER_METHOD_INIT(tpl, getFilename);
    WRAPPER_METHOD_INIT(tpl, getFileheader);
    WRAPPER_METHOD_INIT(tpl, getFilesize);

    WRAPPER_METHOD_INIT(tpl, getDirent);
    WRAPPER_METHOD_INIT(tpl, getDirentByTitle);
    WRAPPER_METHOD_INIT(tpl, getCountArticles);

    WRAPPER_METHOD_INIT(tpl, getArticle);
    WRAPPER_METHOD_INIT(tpl, getArticleByUrl);
    WRAPPER_METHOD_INIT(tpl, getArticleByTitle);

    WRAPPER_METHOD_INIT(tpl, getCluster);
    WRAPPER_METHOD_INIT(tpl, getCountClusters);
    WRAPPER_METHOD_INIT(tpl, getClusterOffset);

    WRAPPER_METHOD_INIT(tpl, getBlob);

    WRAPPER_METHOD_INIT(tpl, getNamespaceBeginOffset);
    WRAPPER_METHOD_INIT(tpl, getNamespaceEndOffset);
    WRAPPER_METHOD_INIT(tpl, getNamespaceCount);
    WRAPPER_METHOD_INIT(tpl, getNamespaces);
    WRAPPER_METHOD_INIT(tpl, hasNamespace);

    WRAPPER_METHOD_INIT(tpl, good);
    WRAPPER_METHOD_INIT(tpl, getMTime);
    WRAPPER_METHOD_INIT(tpl, getMimeType);
    WRAPPER_METHOD_INIT(tpl, getChecksum);
    WRAPPER_METHOD_INIT(tpl, verify);
    WRAPPER_INIT_FINISH(tpl);
  }

 private:
  explicit FileWrap(zim::File *file) : file_(file) { }
  virtual ~FileWrap() { delete file_; }

  static NAN_METHOD(New);

  WRAPPER_METHOD_DECLARE_GET(getFilename, STRING);
  WRAPPER_METHOD_DECLARE(getFileheader);
  WRAPPER_METHOD_DECLARE_GET(getFilesize, OFFSET_T);

  WRAPPER_METHOD_DECLARE(getDirent);
  WRAPPER_METHOD_DECLARE(getDirentByTitle);
  WRAPPER_METHOD_DECLARE_GET(getCountArticles, SIZE_T);

  WRAPPER_METHOD_DECLARE(getArticle);
  WRAPPER_METHOD_DECLARE(getArticleByUrl);
  WRAPPER_METHOD_DECLARE(getArticleByTitle);

  WRAPPER_METHOD_DECLARE(getCluster);
  WRAPPER_METHOD_DECLARE_GET(getCountClusters, SIZE_T);
  WRAPPER_METHOD_DECLARE(getClusterOffset);

  WRAPPER_METHOD_DECLARE(getBlob);

  WRAPPER_METHOD_DECLARE(getNamespaceBeginOffset);
  WRAPPER_METHOD_DECLARE(getNamespaceEndOffset);
  WRAPPER_METHOD_DECLARE(getNamespaceCount);
  WRAPPER_METHOD_DECLARE_GET(getNamespaces, STRING);
  WRAPPER_METHOD_DECLARE(hasNamespace);

  WRAPPER_METHOD_DECLARE_GET(good, BOOL);
  WRAPPER_METHOD_DECLARE_GET(getMTime, TIME_T);
  WRAPPER_METHOD_DECLARE(getMimeType);
  WRAPPER_METHOD_DECLARE_GET(getChecksum, STRING);
  WRAPPER_METHOD_DECLARE_GET(verify, BOOL);

  WRAPPER_DEFINE(FileWrap, zim::File, file)
};

}  // namespace node_libzim

#endif  // NODE_LIBZIM_FILE_H_
