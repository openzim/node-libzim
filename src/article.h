// Binding to zim::Article

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_ARTICLE_H_
#define NODE_LIBZIM_ARTICLE_H_

#include "nan.h"

#include <zim/zim.h>
#include <zim/article.h>

#include "src/blob.h"
#include "src/dirent.h"
#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

class ArticleWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "Article");

    WRAPPER_METHOD_INIT(tpl, getDirent);
    WRAPPER_METHOD_INIT(tpl, getParameter);
    WRAPPER_METHOD_INIT(tpl, getTitle);
    WRAPPER_METHOD_INIT(tpl, getUrl);
    WRAPPER_METHOD_INIT(tpl, getLongUrl);
    WRAPPER_METHOD_INIT(tpl, getLibraryMimeType);
    WRAPPER_METHOD_INIT(tpl, getMimeType);
    WRAPPER_METHOD_INIT(tpl, isRedirect);
    WRAPPER_METHOD_INIT(tpl, isLinktarget);
    WRAPPER_METHOD_INIT(tpl, isDeleted);
    WRAPPER_METHOD_INIT(tpl, getNamespace);
    WRAPPER_METHOD_INIT(tpl, getRedirectIndex);
    WRAPPER_METHOD_INIT(tpl, getRedirectArticle);
    WRAPPER_METHOD_INIT(tpl, getArticleSize);
    WRAPPER_METHOD_INIT(tpl, getCluster);
    WRAPPER_METHOD_INIT(tpl, getData);
    WRAPPER_METHOD_INIT(tpl, getPage);
#if 0
    WRAPPER_METHOD_INIT(tpl, getFile);
#endif
    WRAPPER_METHOD_INIT(tpl, getIndex);
    WRAPPER_METHOD_INIT(tpl, good);

    WRAPPER_INIT_FINISH(tpl);
  }

  WRAPPER_DEFINE_VALUE(ArticleWrap, zim::Article, article)

 private:
  // SmartPtr handles ref counting the original zim::File holding this article.
  WRAPPER_DEFINE_VALUE_DEFAULT_NEW(ArticleWrap, zim::Article, article);

  WRAPPER_METHOD_DECLARE_GET(getDirent, DIRENT);
  WRAPPER_METHOD_DECLARE_GET(getParameter, STRING_BUFFER);
  WRAPPER_METHOD_DECLARE_GET(getTitle, STRING);
  WRAPPER_METHOD_DECLARE_GET(getUrl, STRING);
  WRAPPER_METHOD_DECLARE_GET(getLongUrl, STRING);
  WRAPPER_METHOD_DECLARE_GET(getLibraryMimeType, UINT16_T);
  WRAPPER_METHOD_DECLARE_GET(getMimeType, STRING);
  WRAPPER_METHOD_DECLARE_GET(isRedirect, BOOL);
  WRAPPER_METHOD_DECLARE_GET(isLinktarget, BOOL);
  WRAPPER_METHOD_DECLARE_GET(isDeleted, BOOL);
  WRAPPER_METHOD_DECLARE_GET(getNamespace, CHAR);
  WRAPPER_METHOD_DECLARE_GET(getRedirectIndex, SIZE_T);
  WRAPPER_METHOD_DECLARE(getRedirectArticle);
  WRAPPER_METHOD_DECLARE_GET(getArticleSize, SIZE_T);
  WRAPPER_METHOD_DECLARE(getCluster);
  WRAPPER_METHOD_DECLARE_GET(getData, BLOB);
  WRAPPER_METHOD_DECLARE(getPage);
#if 0
  WRAPPER_METHOD_DECLARE(getFile);
#endif
  WRAPPER_METHOD_DECLARE_GET(getIndex, SIZE_T);
  WRAPPER_METHOD_DECLARE_GET(good, BOOL);
};

}  // namespace node_libzim

#endif  // NODE_LIBZIM_ARTICLE_H_
