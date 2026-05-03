#include "Hash.h"
#include "StringConversion.h"
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

namespace
{

// wyhash-style 128-bit multiply mixer (a.k.a. mum/wymix) customized with ChatGPT 5

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
inline uint64_t mulmix64(uint64_t a, uint64_t b) noexcept
{
  unsigned __int64 hi, lo = _umul128(a, b, &hi);
  return lo ^ hi;
}
#elif defined(__SIZEOF_INT128__)
inline uint64_t mulmix64(uint64_t a, uint64_t b) noexcept
{
  __uint128_t r = ((__uint128_t)a) * b;
  return (uint64_t)r ^ (uint64_t)(r >> 64);
}
#else
// Portable fallback if 128-bit is unavailable (still good in practice)
inline uint64_t mulmix64(uint64_t a, uint64_t b) noexcept
{
  a ^= (a >> 23);
  a *= 0x2127599bf4325c37ULL;
  b ^= (b >> 23);
  b *= 0x165667919e3779f9ULL;
  uint64_t x = a ^ b;
  x ^= x >> 27;
  x *= 0x9fb21c651e98df25ULL;
  x ^= x >> 25;
  return x;
}
#endif

// fixed constants for determinism
constexpr uint64_t K0 = 0xa0761d6478bd642FULL;
constexpr uint64_t K1 = 0xe7037ed1a0b428DBULL;
constexpr uint64_t KF = 0x94d049bb133111ebULL;

inline uint64_t avalanche64(uint64_t x) noexcept
{
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ULL;
  x ^= x >> 27;
  x *= KF;
  x ^= x >> 31;
  return x;
}

inline uint64_t mix2(uint64_t a, uint64_t b) noexcept
{
  return avalanche64(mulmix64(a ^ K0, b ^ K1));
}

// cast signed to its unsigned counterpart, then widen to u64
template <class T>
inline uint64_t to_u64_integral(T v) noexcept
{
  using U = typename std::make_unsigned<T>::type;
  return static_cast<uint64_t>(static_cast<U>(v));
}

inline uint32_t bits_float(float v) noexcept
{
  uint32_t u;
  std::memcpy(&u, &v, sizeof(u));
  return u;
}
inline uint64_t bits_double(double v) noexcept
{
  uint64_t u;
  std::memcpy(&u, &v, sizeof(u));
  return u;
}

// load 8 bytes from a byte pointer as little-endian, independent of host
inline uint64_t load_le64(const unsigned char* p) noexcept
{
  uint64_t x = 0;
  for (unsigned i = 0; i < 8; ++i)
    x |= (uint64_t)p[i] << (8u * i);
  return x;
}

std::size_t hash_mix(std::size_t n) noexcept
{
  uint64_t x = static_cast<uint64_t>(n);
  x = mulmix64(x ^ K0, x ^ K1);
  x = avalanche64(x);
  return static_cast<std::size_t>(x);
}

}  // namespace

namespace Fmi
{

// integers & chars
std::size_t hash_value(bool v)
{
  return hash_mix(v ? 1u : 0u);
}
std::size_t hash_value(char v)
{
  return hash_mix(to_u64_integral(v));
}
std::size_t hash_value(signed char v)
{
  return hash_mix(to_u64_integral(v));
}
std::size_t hash_value(unsigned char v)
{
  return hash_mix(to_u64_integral(v));
}
std::size_t hash_value(char16_t v)
{
  return hash_mix(static_cast<uint64_t>(v));
}
std::size_t hash_value(char32_t v)
{
  return hash_mix(static_cast<uint64_t>(v));
}
std::size_t hash_value(wchar_t v)
{
  return hash_mix(to_u64_integral(v));
}
std::size_t hash_value(short v)
{
  return hash_mix(to_u64_integral(v));
}
std::size_t hash_value(unsigned short v)
{
  return hash_mix(static_cast<uint64_t>(v));
}
std::size_t hash_value(int v)
{
  return hash_mix(to_u64_integral(v));
}
std::size_t hash_value(unsigned int v)
{
  return hash_mix(static_cast<uint64_t>(v));
}
std::size_t hash_value(long v)
{
  return hash_mix(to_u64_integral(v));
}
std::size_t hash_value(unsigned long v)
{
  return hash_mix(static_cast<uint64_t>(v));
}
std::size_t hash_value(long long v)
{
  return hash_mix(to_u64_integral(v));
}
std::size_t hash_value(unsigned long long v)
{
  return hash_mix(static_cast<uint64_t>(v));
}

// This should probably be replaced by siphash for better collision protection
std::size_t hash_value(const std::string& str)
{
  return std::hash<std::string>{}(str);
}

std::size_t hash_value(const Fmi::Date& date)
{
  const auto type = date.type();
  auto hash = hash_value(1024 + static_cast<int>(type));
  if (type == Fmi::date_time::Base::NORMAL)
    hash_combine(hash, hash_value(date.get_impl().time_since_epoch().count()));
  return hash;
}

std::size_t hash_value(const TimeDuration& duration)
{
  const auto type = duration.type();
  auto hash = hash_value(2048 + static_cast<int>(type));
  if (type == Fmi::date_time::Base::NORMAL)
    hash_combine(hash, hash_value(duration.get_impl().count()));
  return hash;
}

std::size_t hash_value(const DateTime& time)
{
  const auto type = time.type();
  auto hash = hash_value(3072 + static_cast<int>(type));
  if (type == Fmi::date_time::Base::NORMAL)
    hash_combine(hash, hash_value(time.get_impl().time_since_epoch().count()));
  return hash;
}

std::size_t hash_value(const Fmi::LocalDateTime& time)
{
  // Getting UTC time gets local time at first and then converts it to UTC
  // As result local time is used here for better efficiency
  auto hash = Fmi::hash_value(time.local_time());
  Fmi::hash_combine(hash, Fmi::hash_value(time.zone()));
  return hash;
}

std::size_t hash_value(const Fmi::TimeZonePtr& zone)
{
  return Fmi::hash_value(zone->name());
}

// floats
std::size_t hash_value(float v)
{
  if (v == 0.0f)
    return hash_mix(0);
  return hash_mix(static_cast<std::size_t>(bits_float(v)));
}

std::size_t hash_value(double v)
{
  if (v == 0.0)
    return hash_mix(0);
  return hash_mix(static_cast<std::size_t>(bits_double(v)));
}

std::size_t hash_value(long double v)
{
  if constexpr (sizeof(long double) == 8)
  {
    return hash_value(static_cast<double>(v));
  }
  else
  {
    // Endian-stable hashing of the raw bytes (works for 80- and 128-bit)
    const unsigned char* p = reinterpret_cast<const unsigned char*>(&v);
    size_t len = sizeof(long double);
    uint64_t acc = 0x27d4eb2f165667c5ULL;
    while (len >= 8)
    {
      uint64_t w = load_le64(p);
      acc = mix2(acc, w);
      p += 8;
      len -= 8;
    }
    uint64_t tail = 0;
    for (unsigned i = 0; i < len; ++i)
      tail |= (uint64_t)p[i] << (8u * i);
    acc = mix2(acc, tail ^ (uint64_t(sizeof(long double)) << 56));
    return static_cast<std::size_t>(avalanche64(acc));
  }
}

// combiner
void hash_combine(std::size_t& seed, std::size_t value)
{
  if (seed == bad_hash || value == bad_hash)
    seed = bad_hash;
  else
  {
    uint64_t s = static_cast<uint64_t>(seed);
    uint64_t v = static_cast<uint64_t>(value);
    s = mix2(s, v);
    seed = static_cast<std::size_t>(s);
  }
}

}  // namespace Fmi
