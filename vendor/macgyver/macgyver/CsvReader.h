// ======================================================================
/*!
 * \brief CSV file reader
 */
// ======================================================================

#pragma once

#include <functional>
#include <string>
#include <vector>

namespace Fmi
{
namespace CsvReader
{
using row_type = std::vector<std::string>;
using Callback = std::function<void(const row_type& row)>;

void read(const std::string& filename, Callback callback, char delimiter = ',');
}  // namespace CsvReader
}  // namespace Fmi

// ======================================================================
