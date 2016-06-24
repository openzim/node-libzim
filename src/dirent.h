// Binding to zim::Dirent

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_DIRENT_H_
#define NODE_LIBZIM_DIRENT_H_

#include "nan.h"

#include <zim/zim.h>
#include <zim/dirent.h>

#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

class DirentWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "Dirent");

    WRAPPER_METHOD_INIT(tpl, isRedirect);
    WRAPPER_METHOD_INIT(tpl, isLinktarget);
    WRAPPER_METHOD_INIT(tpl, isDeleted);
    WRAPPER_METHOD_INIT(tpl, isArticle);
    WRAPPER_METHOD_INIT(tpl, getMimeType);
    WRAPPER_METHOD_INIT(tpl, getVersion);
    WRAPPER_METHOD_INIT(tpl, setVersion);
    WRAPPER_METHOD_INIT(tpl, getClusterNumber);
    WRAPPER_METHOD_INIT(tpl, getBlobNumber);
    WRAPPER_METHOD_INIT(tpl, setCluster);
    WRAPPER_METHOD_INIT(tpl, getRedirectIndex);
    WRAPPER_METHOD_INIT(tpl, getNamespace);

    WRAPPER_METHOD_INIT(tpl, getTitle);
    WRAPPER_METHOD_INIT(tpl, getUrl);
    WRAPPER_METHOD_INIT(tpl, getLongUrl);
    WRAPPER_METHOD_INIT(tpl, getParameter);
    WRAPPER_METHOD_INIT(tpl, getDirentSize);

    WRAPPER_METHOD_INIT(tpl, setTitle);
    WRAPPER_METHOD_INIT(tpl, setUrl);
    WRAPPER_METHOD_INIT(tpl, setParameter);
    WRAPPER_METHOD_INIT(tpl, setRedirect);
    WRAPPER_METHOD_INIT(tpl, setMimeType);
    WRAPPER_METHOD_INIT(tpl, setLinktarget);
    WRAPPER_METHOD_INIT(tpl, setDeleted);
    WRAPPER_METHOD_INIT(tpl, setArticle);

    WRAPPER_INIT_FINISH(tpl);
    WRAPPER_GETTER_INIT(Nan::To<v8::Object>(constructor()).ToLocalChecked(),
                        redirectMimeType);
    WRAPPER_GETTER_INIT(Nan::To<v8::Object>(constructor()).ToLocalChecked(),
                        linktargetMimeType);
    WRAPPER_GETTER_INIT(Nan::To<v8::Object>(constructor()).ToLocalChecked(),
                        deletedMimeType);
  }

  WRAPPER_DEFINE_VALUE(DirentWrap, zim::Dirent, dirent)
  WRAPPER_DEFINE_VALUE_DEFAULT_NEW(DirentWrap, zim::Dirent, dirent)

 private:
  WRAPPER_GETTER_DECLARE(redirectMimeType) {
      RETURN_UINT16_T(zim::Dirent::redirectMimeType);
  }
  WRAPPER_GETTER_DECLARE(linktargetMimeType) {
      RETURN_UINT16_T(zim::Dirent::linktargetMimeType);
  }
  WRAPPER_GETTER_DECLARE(deletedMimeType) {
      RETURN_UINT16_T(zim::Dirent::deletedMimeType);
  }

  WRAPPER_METHOD_DECLARE_GET(isRedirect, BOOL);
  WRAPPER_METHOD_DECLARE_GET(isLinktarget, BOOL);
  WRAPPER_METHOD_DECLARE_GET(isDeleted, BOOL);
  WRAPPER_METHOD_DECLARE_GET(isArticle, BOOL);
  WRAPPER_METHOD_DECLARE_GET(getMimeType, UINT16_T);
  WRAPPER_METHOD_DECLARE_GET(getVersion, SIZE_T);
  WRAPPER_METHOD_DECLARE_SET(setVersion, SIZE_T);
  WRAPPER_METHOD_DECLARE_GET(getClusterNumber, SIZE_T);
  WRAPPER_METHOD_DECLARE_GET(getBlobNumber, SIZE_T);
  WRAPPER_METHOD_DECLARE(setCluster);
  WRAPPER_METHOD_DECLARE_GET(getRedirectIndex, SIZE_T);
  WRAPPER_METHOD_DECLARE_GET(getNamespace, CHAR);

  WRAPPER_METHOD_DECLARE_GET(getTitle, STRING);
  WRAPPER_METHOD_DECLARE_GET(getUrl, STRING);
  WRAPPER_METHOD_DECLARE_GET(getLongUrl, STRING);
  WRAPPER_METHOD_DECLARE_GET(getParameter, STRING);
  WRAPPER_METHOD_DECLARE_GET(getDirentSize, UNSIGNED);

  WRAPPER_METHOD_DECLARE_SET(setTitle, STRING);
  WRAPPER_METHOD_DECLARE(setUrl);
  WRAPPER_METHOD_DECLARE_SET(setParameter, STRING);
  WRAPPER_METHOD_DECLARE_SET(setRedirect, SIZE_T);
  WRAPPER_METHOD_DECLARE_SET(setMimeType, UINT16_T);
  WRAPPER_METHOD_DECLARE_SET(setLinktarget, VOID);
  WRAPPER_METHOD_DECLARE_SET(setDeleted, VOID);
  WRAPPER_METHOD_DECLARE(setArticle);
};

}  // namespace node_libzim

#endif  // NODE_LIBZIM_DIRENT_H_
