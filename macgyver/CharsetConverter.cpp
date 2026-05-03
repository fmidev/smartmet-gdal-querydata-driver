#include "CharsetConverter.h"
#include "Exception.h"
#include <cerrno>
#include <cstring>
#include <iconv.h>
#include <mutex>

struct Fmi::CharsetConverter::Impl
{
  Impl(const std::string& from, const std::string& to)
      : itsIconv(::iconv_open(to.c_str(), from.c_str()))
  {
    if (itsIconv == (iconv_t)-1)
    {
      throw Fmi::Exception(BCP, "Initializing iconv from " + from + " to " + to + " failed");
    }
  }

  ~Impl()
  {
    if (itsIconv != (iconv_t)-1)
    {
      iconv_close(itsIconv);
    }
  }

  iconv_t itsIconv;
  std::mutex m;
};

Fmi::CharsetConverter::CharsetConverter(const std::string& from,
                                        const std::string& to,
                                        std::size_t max_len)
    : from(from), to(to), max_len(max_len), impl(new Impl(from, to))
{
}

Fmi::CharsetConverter::~CharsetConverter() = default;

std::string Fmi::CharsetConverter::convert(const std::string& src) const
{
  char* in = const_cast<char*>(src.c_str());
  char s_out[1024];
  std::size_t i_len = src.length();
  if (max_len > 0U && i_len > max_len)
  {
    throw Fmi::Exception(BCP,
                         "Provided string is too long (" + std::to_string(int(i_len)) + " > " +
                             std::to_string(int(max_len)));
  }
  std::size_t o_len = 1024;
  char* out_ptr = s_out;

  errno = 0;
  std::unique_lock<std::mutex> lock(impl->m);
  std::size_t result = ::iconv(impl->itsIconv, &in, &i_len, &out_ptr, &o_len);
  if (result == (std::size_t)-1)
  {
    if (errno == E2BIG)
    {
      ::iconv(impl->itsIconv, nullptr, nullptr, nullptr, nullptr);
      errno = 0;
      in = const_cast<char*>(src.c_str());
      i_len = src.length();
      o_len = 4 * i_len;
      char* x_out = new char[o_len + 1];
      out_ptr = x_out;
      result = ::iconv(impl->itsIconv, &in, &i_len, &out_ptr, &o_len);
      if (result != (std::size_t)-1)
      {
        std::string retval(x_out, out_ptr);
        delete[] x_out;
        return retval;
      }

      delete[] x_out;
    }

    ::iconv(impl->itsIconv, nullptr, nullptr, nullptr, nullptr);

    switch (errno)
    {
      case E2BIG:
        throw Fmi::Exception(BCP, "Not enough memory for converting from " + from + " to " + to);
      case EILSEQ:
        throw Fmi::Exception(BCP,
                             "An invalid multibyte sequence has been encountered in the input");
      case EINVAL:
        throw Fmi::Exception(BCP,
                             "An incomplete multibyte sequence has been encountered in the input");
      default:
        throw Fmi::Exception(BCP, std::string("Unexpected error: ") +
            strerror_r(errno, s_out, sizeof(s_out))); // We can reuse the same buffer as
                                                      // the exception is thrown anyway
    }
  }
  else
  {
    return std::string(s_out, out_ptr);
  }
}
