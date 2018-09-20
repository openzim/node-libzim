// Binding to zim::Fileheader

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_FILEHEADER_H_
#define NODE_LIBZIM_FILEHEADER_H_

#include "nan.h"

#include <zim/zim.h>
#include <zim/fileheader.h>

#include "src/macros.h"
#include "src/uuid.h"
#include "src/wrapper.h"

namespace node_libzim {

class FileheaderWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "Fileheader");

    WRAPPER_METHOD_INIT(tpl, getUuid);
    WRAPPER_METHOD_INIT(tpl, setUuid);

    WRAPPER_METHOD_INIT(tpl, getArticleCount);
    WRAPPER_METHOD_INIT(tpl, setArticleCount);

    WRAPPER_METHOD_INIT(tpl, getTitleIdxPos);
    WRAPPER_METHOD_INIT(tpl, setTitleIdxPos);

    WRAPPER_METHOD_INIT(tpl, getUrlPtrPos);
    WRAPPER_METHOD_INIT(tpl, setUrlPtrPos);

    WRAPPER_METHOD_INIT(tpl, getMimeListPos);
    WRAPPER_METHOD_INIT(tpl, setMimeListPos);

    WRAPPER_METHOD_INIT(tpl, getClusterCount);
    WRAPPER_METHOD_INIT(tpl, setClusterCount);

    WRAPPER_METHOD_INIT(tpl, getClusterPtrPos);
    WRAPPER_METHOD_INIT(tpl, setClusterPtrPos);

    WRAPPER_METHOD_INIT(tpl, hasMainPage);
    WRAPPER_METHOD_INIT(tpl, getMainPage);
    WRAPPER_METHOD_INIT(tpl, setMainPage);

    WRAPPER_METHOD_INIT(tpl, hasLayoutPage);
    WRAPPER_METHOD_INIT(tpl, getLayoutPage);
    WRAPPER_METHOD_INIT(tpl, setLayoutPage);

    WRAPPER_METHOD_INIT(tpl, hasChecksum);
    WRAPPER_METHOD_INIT(tpl, getChecksumPos);
    WRAPPER_METHOD_INIT(tpl, setChecksumPos);

    WRAPPER_INIT_FINISH(tpl);
  }
  static v8::Local<v8::Object> FromC(v8::Local<v8::Object> file,
                                     const zim::Fileheader *fileheader);

 private:
  explicit FileheaderWrap(v8::Local<v8::Object> file,
                          zim::Fileheader *fileheader) {
    // Fileheader is stored in a File object.  Keep the file object
    // alive as long as the fileheader is alive!
    file_.Reset(file);
    fileheader_ = fileheader;
  }
  virtual ~FileheaderWrap() {
    // We don't own the fileheader*, so don't delete that.
    // Instead, just release the reference to the File object
    // (which owns the fileheader)
    if (!file_.IsEmpty()) { file_.Reset(); }
  }
  Nan::Persistent<v8::Object> file_;

  static NAN_METHOD(New);

  WRAPPER_METHOD_DECLARE_GET(getUuid, UUID);
  WRAPPER_METHOD_DECLARE_SET(setUuid, UUID);

  WRAPPER_METHOD_DECLARE_GET(getArticleCount, SIZE_T);
  WRAPPER_METHOD_DECLARE_SET(setArticleCount, SIZE_T);

  WRAPPER_METHOD_DECLARE_GET(getTitleIdxPos, OFFSET_T);
  WRAPPER_METHOD_DECLARE_SET(setTitleIdxPos, OFFSET_T);

  WRAPPER_METHOD_DECLARE_GET(getUrlPtrPos, OFFSET_T);
  WRAPPER_METHOD_DECLARE_SET(setUrlPtrPos, OFFSET_T);

  WRAPPER_METHOD_DECLARE_GET(getMimeListPos, OFFSET_T);
  WRAPPER_METHOD_DECLARE_SET(setMimeListPos, OFFSET_T);

  WRAPPER_METHOD_DECLARE_GET(getClusterCount, SIZE_T);
  WRAPPER_METHOD_DECLARE_SET(setClusterCount, SIZE_T);

  WRAPPER_METHOD_DECLARE_GET(getClusterPtrPos, OFFSET_T);
  WRAPPER_METHOD_DECLARE_SET(setClusterPtrPos, OFFSET_T);

  WRAPPER_METHOD_DECLARE_GET(hasMainPage, BOOL);
  WRAPPER_METHOD_DECLARE_GET(getMainPage, SIZE_T);
  WRAPPER_METHOD_DECLARE_SET(setMainPage, SIZE_T);

  WRAPPER_METHOD_DECLARE_GET(hasLayoutPage, BOOL);
  WRAPPER_METHOD_DECLARE_GET(getLayoutPage, SIZE_T);
  WRAPPER_METHOD_DECLARE_SET(setLayoutPage, SIZE_T);

  WRAPPER_METHOD_DECLARE_GET(hasChecksum, BOOL);
  WRAPPER_METHOD_DECLARE_GET(getChecksumPos, OFFSET_T);
  WRAPPER_METHOD_DECLARE_SET(setChecksumPos, OFFSET_T);

  WRAPPER_DEFINE(FileheaderWrap, zim::Fileheader, fileheader)
};

}  // namespace node_libzim

#endif  // NODE_LIBZIM_FILEHEADER_H_
