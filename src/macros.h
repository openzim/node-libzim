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

#define REQUIRE_ARGUMENT_NUMBER(i)                                      \
    if (info.Length() <= (i) || !info[i]->IsNumber()) {                        \
        return Nan::ThrowTypeError("Argument " #i " must be a number");        \
    }

#define REQUIRE_ARGUMENT_INTEGER(i, var)                                       \
    if (info.Length() <= (i) || !(info[i]->IsInt32() || info[i]->IsUint32())) {\
        return Nan::ThrowTypeError("Argument " #i " must be an integer");      \
    }                                                                          \
    int64_t var = Nan::To<int64_t>(info[i]).FromMaybe(0);

static NAN_INLINE v8::Local<v8::String>
CAST_STRING(v8::Local<v8::Value> v, v8::Local<v8::String> defaultValue) {
    Nan::EscapableHandleScope scope;
    return scope.Escape(v->IsString() ?
                        Nan::To<v8::String>(v).FromMaybe(defaultValue) :
                        defaultValue);
}

#endif  // NODE_LIBZIM_MACROS_H_
