// ======================================================================
/*!
 * \file NFmiDataModifierLogical.cpp
 * \brief Implementation of class NFmiDataModifierLogical
 */
// =====================================================================
/*!
 * \class NFmiDataModifierLogical
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierLogical.h"
#include "NFmiDataModifierBoolean.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierLogical::~NFmiDataModifierLogical() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theCondition Undocumented
 * \param thePrimaryModifier Undocumented
 * \param theSecondaryModifier Undocumented
 *
 * \todo Use cloning instead for security reasons
 */
// ----------------------------------------------------------------------

NFmiDataModifierLogical::NFmiDataModifierLogical(NFmiDataModifierBoolean *theCondition,
                                                 NFmiDataModifier *thePrimaryModifier,
                                                 NFmiDataModifier *theSecondaryModifier)

    : itsCondition(theCondition),
      itsSecondaryModifier(theSecondaryModifier),
      itsPrimaryModifier(thePrimaryModifier)
{
  itsExpressionType = "logical_control";
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiDataModifierLogical::FloatValue()
{
  if (itsCondition && itsPrimaryModifier && itsSecondaryModifier)
    return itsCondition->BooleanValue() ? itsPrimaryModifier->FloatValue()
                                        : itsSecondaryModifier->FloatValue();
  else
    return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiDataModifierLogical::WriteOperand(std::ostream &file) const
{
  itsCondition->Write(file);
  itsPrimaryModifier->Write(file);
  itsSecondaryModifier->Write(file);
  return file;
}

// ======================================================================
