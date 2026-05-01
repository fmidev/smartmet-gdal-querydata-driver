#pragma once

#include <string>
#include <string_view>

namespace SmartMet
{
namespace GdalQueryData
{
// QueryData strings (parameter names, descriptions) are stored as Latin-1
// (Windows-1252-ish, but only the ASCII-compatible subset of accented Latin
// letters that newbase cares about). GDAL metadata is UTF-8 by contract.
inline std::string toUtf8(std::string_view latin1)
{
  std::string out;
  out.reserve(latin1.size() + 4);
  for (unsigned char c : latin1)
  {
    if (c < 0x80)
    {
      out.push_back(static_cast<char>(c));
    }
    else
    {
      out.push_back(static_cast<char>(0xC0 | (c >> 6)));
      out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    }
  }
  return out;
}

// UTF-8 → Latin-1 inverse. Single-byte ASCII passes through; two-byte UTF-8
// sequences in the Latin-1 range collapse back to one byte. Anything outside
// Latin-1 (3+ byte sequences) is replaced with '?'.
inline std::string fromUtf8(std::string_view utf8)
{
  std::string out;
  out.reserve(utf8.size());
  for (size_t i = 0; i < utf8.size();)
  {
    unsigned char c = static_cast<unsigned char>(utf8[i]);
    if (c < 0x80)
    {
      out.push_back(static_cast<char>(c));
      ++i;
    }
    else if ((c & 0xE0) == 0xC0 && i + 1 < utf8.size())
    {
      const unsigned char c2 = static_cast<unsigned char>(utf8[i + 1]);
      const unsigned int cp = ((c & 0x1Fu) << 6) | (c2 & 0x3Fu);
      out.push_back(cp < 256 ? static_cast<char>(cp) : '?');
      i += 2;
    }
    else if ((c & 0xF0) == 0xE0 && i + 2 < utf8.size())
    {
      out.push_back('?');
      i += 3;
    }
    else if ((c & 0xF8) == 0xF0 && i + 3 < utf8.size())
    {
      out.push_back('?');
      i += 4;
    }
    else
    {
      out.push_back('?');
      ++i;
    }
  }
  return out;
}

}  // namespace GdalQueryData
}  // namespace SmartMet
