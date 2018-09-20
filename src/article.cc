// Binding to zim::Article

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include <zim/zim.h>
#include <zim/article.h>

#include <string>

// #define NODE_LIBZIM_TRACE

#include "src/article.h"
#include "src/cluster.h"
#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

NAN_METHOD(ArticleWrap::getRedirectArticle) {
  zim::Article a = getWrappedField(info)->getRedirectArticle();
  RETURN_ARTICLE(a);
}
NAN_METHOD(ArticleWrap::getCluster) {
  zim::Cluster c = getWrappedField(info)->getCluster();
  RETURN_CLUSTER(c);
}
NAN_METHOD(ArticleWrap::getPage) {
  REQUIRE_ARGUMENT_OPT(0, bool, BOOL, layout, true);
  REQUIRE_ARGUMENT_OPT(1, unsigned, UNSIGNED, maxRecurse, 10);
  std::string s = getWrappedField(info)->getPage(layout, maxRecurse);
  RETURN_STD_STRING(s);
}
#if 0
NAN_METHOD(ArticleWrap::getFile) {
  // XXX is this worth implementing?
}
#endif

}  // namespace node_libzim
