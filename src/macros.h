// Helpful macros for this project.

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_MACROS_H_
#define NODE_LIBZIM_MACROS_H_

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


#endif  // NODE_LIBZIM_MACROS_H_
