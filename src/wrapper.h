// Helpers for writing wrapper classes.

// Copyright (c) 2016 C. Scott Ananian <cscott@cscott.net>
#ifndef NODE_LIBZIM_WRAPPER_H_
#define NODE_LIBZIM_WRAPPER_H_

#include "nan.h"

#include <string>

#include "src/macros.h"

#define WRAPPER_INIT(tpl, name)                                         \
  v8::Local<v8::String> class_name = NEW_STR(name);                     \
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New); \
  tpl->SetClassName(class_name);                                        \
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
#define WRAPPER_INIT_FINISH(tpl)                                        \
  cons_template().Reset(tpl);                                           \
  Nan::Set(target, class_name, constructor());
#define WRAPPER_METHOD_INIT(tpl, name)                                  \
  Nan::SetPrototypeMethod(tpl, #name, name)
#define WRAPPER_METHOD_DECLARE(name)                                    \
  static NAN_METHOD(name)
#define WRAPPER_METHOD_DECLARE_GET(name, TYPE)                          \
  static NAN_METHOD(name) { WRAPPER_GET_ ## TYPE(name); }
#define WRAPPER_METHOD_DECLARE_SET(name, TYPE)                          \
  static NAN_METHOD(name) { WRAPPER_SET_ ## TYPE(name); }
#define WRAPPER_GETTER_INIT(tpl, name)                                  \
  Nan::SetAccessor(tpl, NEW_STR(#name), name)
#define WRAPPER_GETTER_DECLARE(name)                                    \
  static void name(v8::Local<v8::String> field,                         \
                   const Nan::PropertyCallbackInfo<v8::Value>& info)
#define WRAPPER_DEFINE(Wrapper, WrappedType, field)                     \
 private:                                                               \
  WrappedType *field ## _;                                              \
                                                                        \
  static inline WrappedType *                                           \
  getWrappedField(const Nan::FunctionCallbackInfo<v8::Value> &info) {   \
    Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(info.Holder());     \
    return obj->field ## _;                                             \
  }                                                                     \
  WRAPPER_DEFINE_SHORT()
#define WRAPPER_DEFINE_VALUE(Wrapper, WrappedType, field)               \
 public:                                                                \
  static v8::Local<v8::Object> FromC(const WrappedType &val) {          \
    Nan::EscapableHandleScope scope;                                    \
    v8::Local<v8::Value> argv[] = {                                     \
      Nan::New<v8::External>(const_cast<WrappedType*>(&val))            \
    };                                                                  \
    return scope.Escape(constructor()->NewInstance(1, argv));           \
  }                                                                     \
  static WrappedType FromJS(v8::Local<v8::Value> v) {                     \
    if (v->IsObject()) {                                                \
      Nan::HandleScope scope;                                           \
      v8::Local<v8::Object> o = Nan::To<v8::Object>(v).ToLocalChecked();\
      v8::Local<v8::String> hidden_field = NEW_STR(#WrappedType);       \
      v8::Local<v8::Value> vv = o->GetHiddenValue(hidden_field);        \
      if (!vv.IsEmpty() && o->SameValue(vv)) {                          \
        Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(o);             \
        return obj->field ## _;                                         \
      }                                                                 \
    }                                                                   \
    Nan::ThrowTypeError("Not a " #WrappedType ".");                     \
    return WrappedType();                                               \
  }                                                                     \
 private:                                                               \
  explicit Wrapper(const WrappedType *v) : field ## _(*v) { }                 \
  virtual ~Wrapper() { }                                                \
  WrappedType field ## _;                                               \
  static inline WrappedType *                                           \
  getWrappedField(const Nan::FunctionCallbackInfo<v8::Value> &info) {   \
    Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(info.Holder());     \
    return &(obj->field ## _);                                          \
  }                                                                     \
  WRAPPER_DEFINE_SHORT()
#define WRAPPER_DEFINE_VALUE_DEFAULT_NEW(Wrapper, WrappedType, field)   \
  static NAN_METHOD(New) {                                              \
    if (!info.IsConstructCall()) {                                      \
      return Nan::ThrowTypeError("You must use `new` with this constructor.");\
    }                                                                   \
    Wrapper *w;                                                         \
    if (info.Length() > 0 && info[0]->IsExternal()) {                   \
      /* Create JS wrapper for existing value */                        \
      WrappedType* wt = reinterpret_cast<WrappedType*>                  \
        (v8::Local<v8::External>::Cast(info[0])->Value());              \
      w = new Wrapper(wt);                                              \
    } else {                                                            \
      /* Create new JS object using default no-arg constructor. */      \
      WrappedType wt{};                                                 \
      w = new Wrapper(&wt);                                             \
    }                                                                   \
    v8::Local<v8::String> hidden_field = NEW_STR(#WrappedType);         \
    info.This()->SetHiddenValue(hidden_field, info.This());             \
    w->Wrap(info.This());                                               \
  }
#define WRAPPER_DEFINE_SHORT()                                          \
  static inline Nan::Persistent<v8::FunctionTemplate> & cons_template() { \
    static Nan::Persistent<v8::FunctionTemplate> my_template;           \
    return my_template;                                                 \
  }                                                                     \
  static inline v8::Local<v8::Function> constructor() {                 \
    Nan::EscapableHandleScope scope;                                    \
    v8::Local<v8::FunctionTemplate> t = Nan::New(cons_template());      \
    return scope.Escape(Nan::GetFunction(t).ToLocalChecked());          \
  }

#define WRAPPER_GET_TYPE(name, type)                                    \
  type r = getWrappedField(info)->name();                               \
  info.GetReturnValue().Set(Nan::New(r))
#define WRAPPER_GET_STRING(name)                                        \
  std::string r = getWrappedField(info)->name();                        \
  info.GetReturnValue().Set(NEW_STR(r))
#define WRAPPER_GET_CHAR(name)                                          \
  const char r = getWrappedField(info)->name();                         \
  const char rr[2] = { r, 0 };                                          \
  info.GetReturnValue().Set(Nan::New<v8::String>(rr, 1)                 \
                            .FromMaybe(Nan::EmptyString()))
#define WRAPPER_GET_BOOL(name) WRAPPER_GET_TYPE(name, bool)
#define WRAPPER_GET_SIZE_T(name) WRAPPER_GET_TYPE(name, zim::size_type)
#define WRAPPER_GET_OFFSET_T(name)                                      \
  zim::offset_type r = getWrappedField(info)->name();                   \
  RETURN_OFFSET_T(r)
#define WRAPPER_GET_UINT16_T(name) WRAPPER_GET_TYPE(name, uint16_t)
#define WRAPPER_GET_UNSIGNED(name) WRAPPER_GET_TYPE(name, unsigned)
#define WRAPPER_GET_TIME_T(name)                                        \
  time_t r = getWrappedField(info)->name();                             \
  /* Note that `double` represents integers exactly up to 2^56 */       \
  info.GetReturnValue().Set(Nan::New(static_cast<double>(r)));
#define WRAPPER_GET_VALUE_TYPE(name, WrapperType) \
  info.GetReturnValue().Set(WrapperType::FromC(getWrappedField(info)->name()))
#define WRAPPER_GET_BLOB(name)                                          \
  const zim::Blob r = getWrappedField(info)->name();                    \
  RETURN_BLOB(r)
#define WRAPPER_GET_UUID(name) WRAPPER_GET_VALUE_TYPE(name, UuidWrap)
#define WRAPPER_GET_DIRENT(name) WRAPPER_GET_VALUE_TYPE(name, DirentWrap)

#define WRAPPER_SET_VOID(name)                                          \
  getWrappedField(info)->name();                                        \
  info.GetReturnValue().Set(Nan::Undefined());
#define WRAPPER_SET_STRING(name)                                        \
  REQUIRE_ARGUMENT_STD_STRING(0, val);                                  \
  getWrappedField(info)->name(val);                                     \
  info.GetReturnValue().Set(Nan::Undefined());
#define WRAPPER_SET_INTEGER(name, type)                                 \
  REQUIRE_ARGUMENT_INTEGER(0, val);                                     \
  getWrappedField(info)->name(static_cast<type>(val));                  \
  info.GetReturnValue().Set(Nan::Undefined());
#define WRAPPER_SET_UINT16_T(name) WRAPPER_SET_INTEGER(name, uint16_t)
#define WRAPPER_SET_SIZE_T(name) WRAPPER_SET_INTEGER(name, zim::size_type)
#define WRAPPER_SET_OFFSET_T(name) WRAPPER_SET_INTEGER(name, zim::offset_type)
#define WRAPPER_SET_UUID(name)                                          \
  REQUIRE_ARGUMENTS(1);                                                 \
  Nan::TryCatch try_;                                                   \
  const zim::Uuid r = UuidWrap::FromJS(info[0]);                        \
  if (try_.HasCaught()) { try_.ReThrow(); return; }                     \
  getWrappedField(info)->name(r);                                       \
  info.GetReturnValue().Set(Nan::Undefined());

#endif  // NODE_LIBZIM_WRAPPER_H_
