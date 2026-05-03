#pragma once

#include <memory>
#include <string>

namespace Fmi
{
class CharsetConverter
{
 public:
  CharsetConverter(const std::string& from, const std::string& to, std::size_t max_len = 0);
  virtual ~CharsetConverter();
  CharsetConverter(const CharsetConverter& other) = delete;
  CharsetConverter(CharsetConverter&& other) = delete;
  CharsetConverter& operator=(const CharsetConverter& other) = delete;
  CharsetConverter& operator=(CharsetConverter&& other) = delete;

  std::string convert(const std::string& src) const;

 private:
  struct Impl;

  const std::string from;
  const std::string to;
  const std::size_t max_len;
  std::unique_ptr<Impl> impl;
};

}  // namespace Fmi
