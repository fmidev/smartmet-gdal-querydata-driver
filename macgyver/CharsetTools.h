// ======================================================================
/*!
 * \brief Tools for character set conversions
 */
// ======================================================================

#pragma once

#include <cstdio>
#include <string>

namespace Fmi
{
unsigned char toupper(unsigned char theChar);
unsigned char tolower(unsigned char theChar);
unsigned char tonordic(unsigned char theChar);
unsigned char tolowernordic(unsigned char theChar);

std::string utf8_to_latin1(const std::string& str);
std::string latin1_to_utf8(const std::string& str);

std::wstring utf8_to_utf16(const std::string& str);
std::string utf16_to_utf8(const std::wstring& str);

bool is_utf8(const std::string& str);
}  // namespace Fmi

// ======================================================================
