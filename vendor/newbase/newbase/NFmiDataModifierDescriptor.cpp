// ======================================================================
/*!
 * \file NFmiDataModifierDescriptor.cpp
 * \brief Implementation of class NFmiDataModifierDescriptor
 */
// ======================================================================
/*!
 * \class NFmiDataModifierDescriptor
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierDescriptor.h"
#include "NFmiParamDataModifierList.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierDescriptor::~NFmiDataModifierDescriptor()
{
  delete itsSystematicModifiers;  // Omistaa listan
  delete itsVarianceModifiers;    // Omistaa listan
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierDescriptor::NFmiDataModifierDescriptor()
    : NFmiDataDescriptor(), itsSystematicModifiers(nullptr), itsVarianceModifiers(nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiParamDataModifier* NFmiDataModifierDescriptor::VarianceModifier(const NFmiDataIdent& theParam,
                                                                    const NFmiLevel* theLevel)
{
  NFmiParamDataModifier* modifier = nullptr;
  if (itsVarianceModifiers)
    if (itsVarianceModifiers->Find(theParam, theLevel))
      modifier = itsVarianceModifiers->Current();
  return modifier;
}

// ----------------------------------------------------------------------
/*!
 * \param theList Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierDescriptor::SetVarianceModifiers(NFmiParamDataModifierList* theList)
{
  // HUOM!!! Ottaa annetun listan omistukseensa!
  if (itsVarianceModifiers)
    delete itsVarianceModifiers;
  itsVarianceModifiers = theList;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiParamDataModifierList* NFmiDataModifierDescriptor::GetVarianceModifiers()
{
  return itsVarianceModifiers;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiDataModifierDescriptor::Index() const
{
  return 0;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiDataModifierDescriptor::Size() const
{
  return 0;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierDescriptor::IsActive() const
{
  return false;
}
// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierDescriptor::SetActivity(bool /* theActivityState*/)
{
  return false;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierDescriptor::NextActive()
{
  return false;
}
// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::ostream& NFmiDataModifierDescriptor::Write(std::ostream& file) const
{
  return file;
}
// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::istream& NFmiDataModifierDescriptor::Read(std::istream& file)
{
  return file;
}
// ======================================================================
