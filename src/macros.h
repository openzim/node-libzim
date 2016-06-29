// Helpful macros for this project.

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_MACROS_H_
#define NODE_LIBZIM_MACROS_H_

#include <string>

#define NLZ_ERRORX(msg, ...) \
  fprintf(stderr, msg " %s\n", __VA_ARGS__, __func__)
#define NLZ_ERROR(msg) \
  NLZ_ERRORX(msg "%s", "") /* hack to eat up required argument */

// Poor man's trace mechanism; helpful for tracking down crashes on obscure
// architectures using travis.
#ifdef NODE_LIBZIM_TRACE
# define TRACE(...) NLZ_ERROR(__VA_ARGS__)
# define TRACEX(...) NLZ_ERRORX(__VA_ARGS__)
#else
# define TRACE(...)
# define TRACEX(...)
#endif

/** v8/nan helpers **/

#define NEW_STR(str)                            \
  Nan::New<v8::String>(str).ToLocalChecked()

#define REQUIRE_ARGUMENTS(n)                                            \
  if (info.Length() < (n)) {                                            \
    return Nan::ThrowTypeError("Expected " #n " arguments");            \
  }

#define REQUIRE_ARGUMENT_BOOL(i, var)                                   \
  if (info.Length() <= (i) || !info[i]->IsString()) {                   \
    return Nan::ThrowTypeError("Argument " #i " must be a boolean");    \
  }                                                                     \
  bool var = Nan::To<bool>(info[i]).FromMaybe(false)

#define REQUIRE_ARGUMENT_STRING_NOCONV(i)                               \
  if (info.Length() <= (i) || !info[i]->IsString()) {                   \
    return Nan::ThrowTypeError("Argument " #i " must be a string");     \
  }
#define REQUIRE_ARGUMENT_STRING(i, var)                                 \
  REQUIRE_ARGUMENT_STRING_NOCONV(i)                                     \
  Nan::Utf8String var(info[i])

#define REQUIRE_ARGUMENT_STD_STRING(i, var)                             \
  REQUIRE_ARGUMENT_STRING(i, var ## utf8);                              \
  std::string var(*var ## utf8, var ## utf8.length())
#define REQUIRE_ARGUMENT_STD_STRING_BUFFER(i, var)                      \
  if (info.Length() <= (i) ||                                           \
      !(info[i]->IsString() || node::Buffer::HasInstance(info[i]))) {   \
    return Nan::ThrowTypeError("Argument " #i " must be a string or Buffer"); \
  }                                                                     \
  std::string var;                                                      \
  if (node::Buffer::HasInstance(info[i])) {                             \
    v8::Local<v8::Object> var ## obj =                                  \
      Nan::To<v8::Object>(info[i]).ToLocalChecked();                    \
    var = std::string(node::Buffer::Data(var ## obj),                   \
                      node::Buffer::Length(var ## obj));                \
  } else {                                                              \
    Nan::Utf8String var ## utf8(info[i]);                               \
    var = std::string(*var ## utf8, var ## utf8.length());              \
  }
#define REQUIRE_ARGUMENT_CHAR(i, var)                                   \
  REQUIRE_ARGUMENT_STRING(i, var ## _utf8);                             \
  if (var ## _utf8.length() != 1) {                                     \
    return Nan::ThrowTypeError("Argument " #i " must be a single character");\
  }                                                                     \
  char var = (*var ## _utf8)[0];

#define REQUIRE_ARGUMENT_NUMBER(i)                                      \
    if (info.Length() <= (i) || !info[i]->IsNumber()) {                        \
        return Nan::ThrowTypeError("Argument " #i " must be a number");        \
    }

#define REQUIRE_ARGUMENT_INTEGER(i, var)                                       \
    if (info.Length() <= (i) || !(info[i]->IsInt32() || info[i]->IsUint32())) {\
        return Nan::ThrowTypeError("Argument " #i " must be an integer");      \
    }                                                                          \
    int64_t var = Nan::To<int64_t>(info[i]).FromMaybe(0);
#define REQUIRE_ARGUMENT_INTEGER_TYPE(i, var, type_t)                   \
    REQUIRE_ARGUMENT_NUMBER(i)                                          \
    int64_t var ## _int64 = Nan::To<int64_t>(info[i]).FromMaybe(0);     \
    type_t var = static_cast<type_t>(var ## _int64);
#define REQUIRE_ARGUMENT_UINT16_T(i, var)                               \
    REQUIRE_ARGUMENT_INTEGER_TYPE(i, var, uint16_t)
#define REQUIRE_ARGUMENT_UNSIGNED(i, var)                               \
    REQUIRE_ARGUMENT_INTEGER_TYPE(i, var, unsigned)
#define REQUIRE_ARGUMENT_SIZE_T(i, var)                                 \
    REQUIRE_ARGUMENT_INTEGER_TYPE(i, var, zim::size_type)
#define REQUIRE_ARGUMENT_OFFSET_T(i, var)                               \
    REQUIRE_ARGUMENT_INTEGER_TYPE(i, var, zim::offset_type)

#define REQUIRE_ARGUMENT_OPT(i, type, TYPE, var, defval)        \
    type var = defval;                                          \
    if (info.Length() >= i && !info[i]->IsUndefined()) {        \
        REQUIRE_ARGUMENT_ ## TYPE(i, var ## _present);          \
        var = var ## _present;                                  \
    }

#define RETURN_BOOL(var)                                \
    info.GetReturnValue().Set(Nan::New(var)); return
#define RETURN_STD_STRING(var)                          \
    info.GetReturnValue().Set(NEW_STR(var)); return
#define RETURN_UINT16_T(var)                            \
    info.GetReturnValue().Set(Nan::New(var)); return
#define RETURN_OFFSET_T(var)                                            \
    /* Note that `double` represents integers exactly up to 2^56 */     \
    info.GetReturnValue().Set(Nan::New(static_cast<double>(var))); return
#define RETURN_SIZE_T(var)                              \
    info.GetReturnValue().Set(Nan::New(var)); return
#define RETURN_ARTICLE(var)                                     \
    info.GetReturnValue().Set(ArticleWrap::FromC(var)); return
#define RETURN_BLOB(var)                                                \
    info.GetReturnValue().Set(BlobWrap::FromC(var, false)); return
#define RETURN_CLUSTER(var)                                     \
    info.GetReturnValue().Set(ClusterWrap::FromC(var)); return
#define RETURN_DIRENT(var)                                      \
    info.GetReturnValue().Set(DirentWrap::FromC(var)); return
#define RETURN_UUID(var)                                        \
    info.GetReturnValue().Set(UuidWrap::FromC(var)); return

static NAN_INLINE v8::Local<v8::String>
CAST_STRING(v8::Local<v8::Value> v, v8::Local<v8::String> defaultValue) {
    Nan::EscapableHandleScope scope;
    return scope.Escape(v->IsString() ?
                        Nan::To<v8::String>(v).FromMaybe(defaultValue) :
                        defaultValue);
}

#endif  // NODE_LIBZIM_MACROS_H_
