// Binding to zim::Uuid

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_UUID_H_
#define NODE_LIBZIM_UUID_H_

#include "nan.h"

#include <zim/zim.h>
#include <zim/uuid.h>

#include <sstream>

#include "src/macros.h"
#include "src/wrapper.h"

namespace node_libzim {

class UuidWrap : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    WRAPPER_INIT(tpl, "Uuid");

    Nan::SetMethod(tpl, "generate", generate);
    WRAPPER_METHOD_INIT(tpl, data);
    WRAPPER_METHOD_INIT(tpl, size);
    WRAPPER_METHOD_INIT(tpl, toString);

    WRAPPER_INIT_FINISH(tpl);
  }

  WRAPPER_DEFINE_VALUE(UuidWrap, zim::Uuid, uuid)
  WRAPPER_DEFINE_VALUE_DEFAULT_NEW(UuidWrap, zim::Uuid, uuid)

  WRAPPER_METHOD_DECLARE(data) {
    zim::Uuid *u = getWrappedField(info);
    Nan::MaybeLocal<v8::Object> buf = Nan::CopyBuffer(u->data, u->size());
    if (!buf.IsEmpty()) {
      info.GetReturnValue().Set(buf.ToLocalChecked());
    }
  }

  WRAPPER_METHOD_DECLARE_GET(size, UNSIGNED);

  WRAPPER_METHOD_DECLARE(toString) {
    zim::Uuid *u = getWrappedField(info);
    std::ostringstream data;
    data << *u;
    RETURN_STD_STRING(data.str());
  }

  static NAN_METHOD(generate) {
    zim::Uuid u = zim::Uuid::generate();
    RETURN_UUID(u);
  }
};

}  // namespace node_libzim
#endif  // NODE_LIBZIM_UUID_H_
