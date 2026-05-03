//
// Adds std::optional stream I/O missing in libstdc++
//
// Taken with small modifications from boost (boost/optional/optional_io.hpp).
//
#pragma once

#include <istream>
#include <optional>
#include <ostream>

// std namespace does not provide stream input and output operators for std
namespace std
{

template<class CharType, class CharTrait>
inline
basic_ostream<CharType, CharTrait>&
operator<<(basic_ostream<CharType, CharTrait>& out, nullopt_t)
{
  if (out.good())
  {
    out << "--";
  }

  return out;
}

template<class CharType, class CharTrait, class T>
inline
basic_ostream<CharType, CharTrait>&
operator<<(basic_ostream<CharType, CharTrait>& out, const optional<T>& v)
{
  if (out.good())
  {
    if (!v)
         out << "--" ;
    else out << ' ' << *v ;
  }

  return out;
}

template<class CharType, class CharTrait, class T>
inline
basic_istream<CharType, CharTrait>&
operator>>(basic_istream<CharType, CharTrait>& in, optional<T>& v)
{
  if (in.good())
  {
    int d = in.get();
    if (d == ' ')
    {
      T x;
      in >> x;
      v = x;
    }
    else
    {
      if (d == '-')
      {
        d = in.get();

        if (d == '-')
        {
          v = std::nullopt;
          return in;
        }
      }

      in.setstate( ios::failbit );
    }
  }

  return in;
}

}
