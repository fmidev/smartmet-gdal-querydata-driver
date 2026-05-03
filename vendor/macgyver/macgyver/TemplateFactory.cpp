// ======================================================================

#include "TemplateFactory.h"
#include "Exception.h"
#include "FileSystem.h"
#include <boost/filesystem/operations.hpp>
#include <boost/make_shared.hpp>

namespace Fmi
{
// CT++ may not be thread safe - but using a thread specific
// storage for cached copies makes using it thread safe

struct TemplateInfo
{
  std::time_t modtime = 0;
  SharedFormatter formatter;

  TemplateInfo() = default;
};

using TemplateMap = std::map<std::filesystem::path, TemplateInfo>;
thread_local TemplateMap itsTemplates{};

TemplateFactory::TemplateFactory() = default;

// ----------------------------------------------------------------------
/*!
 * \brief Get a thread specific template formatter for given template
 */
// ----------------------------------------------------------------------

SharedFormatter TemplateFactory::get(const std::filesystem::path& theFilename) const
{
  try
  {
    if (theFilename.empty())
      throw Fmi::Exception(BCP, "TemplateFactory: Cannot use empty templates");

    const auto& tinfo = itsTemplates.find(theFilename);

    const std::optional<std::time_t> opt_modtime = Fmi::last_write_time(theFilename);
    if (!opt_modtime)
    {
      Fmi::Exception err(BCP, "Failed to get file modification time");
      err.addParameter("Path", theFilename);
      throw err;
    }
    const std::time_t modtime = *opt_modtime;

    // Use cached template if it is up to date
    if (tinfo != itsTemplates.end())
      if (tinfo->second.modtime == modtime)
        return tinfo->second.formatter;

    // Initialize a new formatter

    TemplateInfo newinfo;
    newinfo.modtime = modtime;
    newinfo.formatter = std::make_shared<Fmi::TemplateFormatter>();
    newinfo.formatter->load_template(theFilename.c_str());

    // Cache the new formatter
    itsTemplates.insert(std::make_pair(theFilename, newinfo));

    return newinfo.formatter;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Fmi
