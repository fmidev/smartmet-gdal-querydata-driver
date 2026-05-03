// ======================================================================
/*!
 * \brief A factory for thread safe template formatting
 */
// ======================================================================

#pragma once

#include "TemplateFormatter.h"
#include <filesystem>
#include <boost/shared_ptr.hpp>

namespace Fmi
{
using SharedFormatter = std::shared_ptr<Fmi::TemplateFormatter>;

class TemplateFactory
{
 public:
  SharedFormatter get(const std::filesystem::path& theFilename) const;

  TemplateFactory();

  TemplateFactory(const TemplateFactory& other) = delete;
  TemplateFactory(TemplateFactory&& other) = delete;
  TemplateFactory& operator=(const TemplateFactory& other) = delete;
  TemplateFactory& operator=(TemplateFactory&& other) = delete;

};  // class TemplateFactory

}  // namespace Fmi
