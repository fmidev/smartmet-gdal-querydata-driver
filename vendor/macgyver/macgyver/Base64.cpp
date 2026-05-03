/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
   claim that you wrote the original source code. If you use this source code
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/

/*
 * THIS IS AN ALTERED VERSION:
 *
 * 1. Input is std::string instead of a pointer + length
 * 2. File names have been altered
 * 3. Include guards have been added
 * 4. Namespaces Fmi and Base64 have been added
 * 5. Function names have been changed due to above.
 * 6. Comments have been added to the CPP file
 *
 * Mika Heiskanen mika.heiskanen@fmi.fi
 */

#include "Base64.h"
#include "Exception.h"
#include <array>
#include <cctype>

namespace
{
const std::string& base64_chars = *new std::string(
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/");

inline bool is_base64(char c)
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}
}  // namespace

namespace Fmi
{
namespace Base64
{
// ----------------------------------------------------------------------
/*!
 * \brief BASE64 encode the input
 */
// ----------------------------------------------------------------------

std::string encode(const std::string& str)
{
  try
  {
    std::string ret;
    std::size_t i = 0;
    std::size_t j = 0;
    std::size_t in_len = str.size();
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    std::size_t pos = 0;

    while (in_len--)
    {
      char_array_3[i++] = str[pos++];
      if (i == 3)
      {
        char_array_4[0] = (char_array_3[0] & 0xfcU) >> 2;
        char_array_4[1] =
            static_cast<char>((char_array_3[0] & 0x03U) << 4) + ((char_array_3[1] & 0xf0U) >> 4);
        char_array_4[2] =
            static_cast<char>((char_array_3[1] & 0x0fU) << 2) + ((char_array_3[2] & 0xc0U) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3fu;

        for (i = 0; (i < 4); i++)
          ret += base64_chars[static_cast<std::size_t>(char_array_4[i])];
        i = 0;
      }
    }

    if (i)
    {
      for (j = i; j < 3; j++)
        char_array_3[j] = '\0';

      char_array_4[0] = (char_array_3[0] & 0xfcU) >> 2;
      char_array_4[1] =
          static_cast<char>((char_array_3[0] & 0x03U) << 4) + ((char_array_3[1] & 0xf0U) >> 4);
      char_array_4[2] =
          static_cast<char>((char_array_3[1] & 0x0fU) << 2) + ((char_array_3[2] & 0xc0U) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3fU;

      for (j = 0; (j < i + 1); j++)
        ret += base64_chars[static_cast<std::size_t>(char_array_4[j])];

      while ((i++ < 3))
        ret += '=';
    }

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief BASE64 decode the input
 */
// ----------------------------------------------------------------------

std::string decode(const std::string& str)
{
  try
  {
    std::size_t in_len = str.size();
    std::size_t i = 0;
    std::size_t j = 0;
    std::size_t in_ = 0;
    std::array<unsigned char, 4> char_array_4;
    std::array<unsigned char, 3> char_array_3;
    std::string ret;

    while (in_len-- && (str[in_] != '=') && is_base64(str[in_]))
    {
      char_array_4[i++] = str[in_];
      in_++;
      if (i == 4)
      {
        for (i = 0; i < 4; i++)
          char_array_4[i] = static_cast<char>(base64_chars.find(char_array_4[i]));

        char_array_3[0] =
            static_cast<char>((char_array_4[0] << 2) + ((char_array_4[1] & 0x30U) >> 4));
        char_array_3[1] =
            static_cast<char>(((char_array_4[1] & 0xfU) << 4) + ((char_array_4[2] & 0x3cU) >> 2));
        char_array_3[2] = static_cast<char>(((char_array_4[2] & 0x3U) << 6) + char_array_4[3]);

        for (i = 0; (i < 3); i++)
          ret += char_array_3[i];
        i = 0;
      }
    }

    if (i)
    {
      for (j = i; j < 4; j++)
        char_array_4[j] = 0;

      for (j = 0; j < 4; j++)
        char_array_4[j] = static_cast<char>(base64_chars.find(char_array_4[j]));

      char_array_3[0] =
          static_cast<char>((char_array_4[0] << 2) + ((char_array_4[1] & 0x30U) >> 4));
      char_array_3[1] =
          static_cast<char>(((char_array_4[1] & 0xfU) << 4) + ((char_array_4[2] & 0x3cU) >> 2));
      char_array_3[2] = static_cast<char>(((char_array_4[2] & 0x3U) << 6) + char_array_4[3]);

      for (j = 0; (j < i - 1); j++)
        ret += char_array_3[j];
    }

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Base64
}  // namespace Fmi
