#include <cstddef>

/// A non-owning memory view in a C++ std::vector<char> like object.
class ExternalBuffer
{
  public:
    ExternalBuffer(unsigned char *data, size_t length) : data_(data), length_(length) {}

    void *data()
    {
        return reinterpret_cast<void *>(data_);
    }

    size_t byte_length()
    {
        return length_ * sizeof(unsigned char);
    }

    size_t length()
    {
        return length_;
    }

  private:
    unsigned char *data_;
    /// The actual
    size_t length_;
};

#include "nbind/api.h"
#include <nan.h>
#include <node_version.h>

namespace nbind
{

template <>
struct BindingType<ExternalBuffer>
{
    typedef ExternalBuffer Type;

    static bool checkType(WireType /*unused*/)
    {
        return false;
    }

    static Type fromWireType(WireType /*unused*/)
    {
        NBIND_ERR("can not create a BorrowedBuffer from a JS object");
        return Type(nullptr, 0);
    }

    static WireType toWireType(Type &&arg)
    {
        auto buffer = v8::ArrayBuffer::New(
            v8::Isolate::GetCurrent(), arg.data(), arg.byte_length());

        return v8::Float64Array::New(buffer, 0, arg.length());
    }
};

} // namespace nbind