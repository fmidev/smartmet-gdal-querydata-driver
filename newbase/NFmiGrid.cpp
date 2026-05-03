// ======================================================================
/*!
 * \file NFmiGrid.cpp
 * \brief Implementation of class NFmiGrid
 */
// ======================================================================
/*!
 * \class NFmiGrid
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiGrid.h"
#include "NFmiLocationBag.h"
#include "NFmiSaveBaseFactory.h"
#include "NFmiWGS84.h"
#include <macgyver/Hash.h>
#include <gis/CoordinateMatrix.h>
#include <macgyver/Exception.h>
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace std;

NFmiLocationCache::NFmiLocationCache()
    : itsGridPoint(), itsLocationIndex(static_cast<unsigned long>(-1)), fNoInterpolation(false)
{
}

NFmiLocationCache::~NFmiLocationCache() = default;

// kuinka paljon interpolointi hila-piste saa poiketa maksimissaan tasaluvuista,
// että erotus tulkitaan interpolaatiossa turhaksi ja indeksi pyöristetään vain lähintä
// pistettä vastaavaksi. HUOM! x- ja y-suunnat pyöristetään erikseen!!!
static const double gGridPointEpsilon = 0.0000001;

// vielä on toinen raja, jota käytetään vain jos epätarkkuus vie
// tietyn hilan indeksi rajojen yli, tällöin pyörittetään aina alas/ylös rajalle.
static const double gOutlineGridPointEpsilon = gGridPointEpsilon * 10;

void NFmiLocationCache::CalcIsInterpolationNeeded(int theGridSizeX, int theGridSizeY)
{
  try
  {
    double xDiff = ::fabs(itsGridPoint.X() - ::round(itsGridPoint.X()));
    double yDiff = ::fabs(itsGridPoint.Y() - ::round(itsGridPoint.Y()));
    fNoInterpolation = (xDiff < gGridPointEpsilon) && (yDiff < gGridPointEpsilon);

    // Lisäksi tarpeeksi lähellä olevat x/y arvot pitää pyöristää lähimpään tasalukuun.
    // Tämä siksi että mm. reunoilla tulee ongelmia, kun cache laskee hilapisteen hivenen yli reunan
    // (tarkkuus virhe)
    if (xDiff < gGridPointEpsilon)
      itsGridPoint.X(::round(itsGridPoint.X()));
    if (yDiff < gGridPointEpsilon)
      itsGridPoint.Y(::round(itsGridPoint.Y()));

    // Jos indeksit menevät laskenta tarkkuuden takia halutun hila koon alle/yli vähäsen,
    // pyöristetään rajalle
    if (itsGridPoint.X() < 0 && ::fabs(itsGridPoint.X()) < gOutlineGridPointEpsilon)
      itsGridPoint.X(0);
    if (itsGridPoint.Y() < 0 && ::fabs(itsGridPoint.Y()) < gOutlineGridPointEpsilon)
      itsGridPoint.Y(0);
    if (itsGridPoint.X() > (theGridSizeX - 1) &&
        ::fabs(itsGridPoint.X() - theGridSizeX + 1) < gOutlineGridPointEpsilon)
      itsGridPoint.X(theGridSizeX - 1);
    if (itsGridPoint.Y() > (theGridSizeY - 1) &&
        ::fabs(itsGridPoint.Y() - theGridSizeY + 1) < gOutlineGridPointEpsilon)
      itsGridPoint.Y(theGridSizeY - 1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiLocationCache NFmiLocationCache::MakePeekedLocation(const NFmiLocationCache &theOrigLocation,
                                                        int theOffsetX,
                                                        int theOffsetY,
                                                        int theGridSizeX,
                                                        int theGridSizeY)
{
  try
  {
    NFmiLocationCache peekedLocation = theOrigLocation;
    if (theOffsetX != 0)
    {  // lasketaan siirto X-suunnassa
      peekedLocation.itsGridPoint.X(peekedLocation.itsGridPoint.X() + theOffsetX);
      if (peekedLocation.itsGridPoint.X() >= 0. &&
          peekedLocation.itsGridPoint.X() <= theGridSizeX - 1.)
        peekedLocation.itsLocationIndex += theOffsetX;
      else
      {
        peekedLocation.itsLocationIndex = gMissingIndex;
        return peekedLocation;  // voidaan lopettaa jo, koska kurkkaus meni jo 'metsään'
      }
    }

    if (theOffsetY != 0)
    {  // lasketaan siirto Y-suunnassa
      peekedLocation.itsGridPoint.Y(peekedLocation.itsGridPoint.Y() + theOffsetY);
      if (peekedLocation.itsGridPoint.Y() >= 0. &&
          peekedLocation.itsGridPoint.Y() <= theGridSizeY - 1.)
        peekedLocation.itsLocationIndex += theOffsetY * theGridSizeX;
      else
      {
        peekedLocation.itsLocationIndex = gMissingIndex;
        return peekedLocation;  // voidaan lopettaa jo, koska kurkkaus meni jo 'metsään'
      }
    }

    return peekedLocation;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Pyöristetään hilapiste lähimpään kokonaislukuun. X- ja Y-säädöt erikseen.
// HUOM! Tämä ei mielestäni vaikuta itsLocationIndex:in arvoon, koska se on osoittamassa jo
// lähimpään hilapisteeseen, mihin tämä pyöristys johtaa.
void NFmiLocationCache::SetToNearestGridPoint(bool fDoX,
                                              bool fDoY,
                                              int theGridSizeX,
                                              int theGridSizeY)
{
  try
  {
    if (fDoX)
      itsGridPoint.X(::round(itsGridPoint.X()));
    if (fDoY)
      itsGridPoint.Y(::round(itsGridPoint.Y()));
    if (fDoX || fDoY)
      CalcIsInterpolationNeeded(theGridSizeX, theGridSizeY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiTimeCache::NFmiTimeCache()
    : itsTimeIndex1(static_cast<unsigned long>(-1)),
      itsTimeIndex2(static_cast<unsigned long>(-1)),
      itsOffset(1)
{
}

NFmiTimeCache::~NFmiTimeCache() = default;

void NFmiTimeCache::CalcIsInterpolationNeeded()
{
  try
  {
    double diff = ::fabs(itsOffset - ::round(itsOffset));

    if (diff < gGridPointEpsilon)  // käytetään tässä samaa epsilonia
    {
      itsOffset = static_cast<float>(::round(itsOffset));
      if (itsOffset == 0)
        itsTimeIndex1 = itsTimeIndex2;
      else
        itsTimeIndex2 = itsTimeIndex1;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theGrid Undocumented
 * \param theStartingCorner Undocumented
 */
// ----------------------------------------------------------------------

NFmiGrid::NFmiGrid(const NFmiGrid &theGrid, FmiDirection theStartingCorner)
    : NFmiGridBase(theGrid), itsArea(theGrid.itsArea ? theGrid.itsArea->Clone() : nullptr)
{
  try
  {
    if (theStartingCorner != kBase)
    {
      Origo(theGrid.Origo());
      Swap();
      itsData->InitMissingValues(*theGrid.DataPool());
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiGrid &NFmiGrid::operator=(const NFmiGrid &theGrid)
{
  try
  {
    if (this != &theGrid)
    {
      NFmiGridBase::operator=(theGrid);
      itsArea = theGrid.itsArea ? theGrid.itsArea->Clone() : nullptr;
    }
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theGrid Undocumented
 * \param howToInterpolate Undocumented
 * \return Undocumented
 * \todo Kill the dog
 */
// ----------------------------------------------------------------------

bool NFmiGrid::Init(NFmiGrid &theGrid, FmiInterpolationMethod howToInterpolate)
{
  try
  {
    unsigned long counter = 0;
    double theValue;
    auto *values = new float[Size()];

    Reset();
    while (Next())
    {
      theValue = kFloatMissing;
      theGrid.InterpolateToLatLonPoint(LatLon(), theValue, howToInterpolate);
      values[counter] = static_cast<float>(theValue);
      counter++;
    }

    if (itsData)
      delete itsData;
    itsData = new NFmiDataPool();
    itsData->Init(Size(), values);

    itsData->InitMissingValues(*theGrid.DataPool());  // 24.7.98/EL
    delete[] values;

    Reset();
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theGrid Undocumented
 * \param theSubGrid Undocumented
 * \param howToInterpolate Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGrid::Init(NFmiGrid &theGrid,
                    NFmiGrid &theSubGrid,
                    FmiInterpolationMethod howToInterpolate)
{
  try
  {
    // Interpolate from 'theSubGrid', when inside 'theSubGrid' area.
    // Otherwise, interpolate from 'theGrid'

    unsigned long counter = 0;
    double theValue;
    auto *values = new float[Size()];

    Reset();
    while (Next())
    {
      theValue = kFloatMissing;
      NFmiPoint latLon = LatLon();

      // Is latlon point inside the sub grid area?
      if (theSubGrid.Area()->IsInside(latLon))
        theSubGrid.InterpolateToLatLonPoint(latLon, theValue, howToInterpolate);
      else
        theGrid.InterpolateToLatLonPoint(latLon, theValue, howToInterpolate);

      values[counter] = static_cast<float>(theValue);
      counter++;
    }

    if (itsData)
      delete itsData;
    itsData = new NFmiDataPool();
    itsData->Init(Size(), values);

    // HUOM! Puuttuvien arvojen lista otetaan "päägridin" datapoolista
    itsData->InitMissingValues(*theGrid.DataPool());
    delete[] values;

    Reset();
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGrid::XYToGrid(double x, double y) const
{
  try
  {
    // x: scale, convert and add the base
    // y: swap, scale, convert and add the base

    //  double x_ = x / itsArea->Width() * (itsXNumber - 1) + Base();
    double x_ = x / itsArea->Width() * (itsXNumber - 1);
    //  double y_ = (itsArea->Height() - y) / itsArea->Height() * (itsYNumber - 1) + Base();
    double y_ = (itsArea->Height() - y) / itsArea->Height() * (itsYNumber - 1);

    return NFmiPoint(x_, y_);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param x_ Undocumented
 * \param y_ Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGrid::GridToXY(double x_, double y_) const
{
  try
  {
    //  double x = itsArea->Left() + (x_ - Base()) / (itsXNumber - 1) * itsArea->Width();
    double x = itsArea->Left() + x_ / (itsXNumber - 1) * itsArea->Width();
    //  double y = itsArea->Bottom() - (y_ - Base()) / (itsYNumber - 1) * itsArea->Height();
    double y = itsArea->Bottom() - y_ / (itsYNumber - 1) * itsArea->Height();
    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theGrid Undocumented
 * \param theStep Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGrid::Coverage(const NFmiGrid &theGrid, unsigned long theStep)
{
  try
  {
    double counter = 0;
    double sum = 0;

    for (unsigned long y = 0; y < itsYNumber; y += theStep)
    {
      for (unsigned long x = 0; x < itsXNumber; x += theStep)
      {
        if (theGrid.itsArea->IsInside(GridToLatLon(x, y)))
        {
          counter += 1;
        }
        sum += 1;
      }
    }

    return sum ? counter / sum : 0.;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newArea Undocumented
 * \param theXNumber Undocumented
 * \param theYNumber Undocumented
 * \param theStartingCorner Undocumented
 * \param theInterpolationMethod Undocumented
 * \return Undocumented
 * \todo Should return an std::shared_ptr
 */
// ----------------------------------------------------------------------

NFmiGrid *NFmiGrid::CreateNewGrid(NFmiArea *newArea,
                                  unsigned long theXNumber,
                                  unsigned long theYNumber,
                                  FmiDirection theStartingCorner,
                                  FmiInterpolationMethod theInterpolationMethod)
{
  try
  {
    auto *newGrid =
        new NFmiGrid(newArea, theXNumber, theYNumber, theStartingCorner, theInterpolationMethod);
    ResetCrop();
    newGrid->Init(*this, theInterpolationMethod);
    return newGrid;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiGrid::Write(std::ostream &file) const
{
  try
  {
    if (itsArea)
    {
      file << itsArea->ClassId() << " " << itsArea->ClassName() << std::endl;
      file << *itsArea;
    }
    else
    {
      file << 0 << " NFmiArea" << std::endl;
    }

    NFmiGridBase::Write(file);

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiGrid::Read(std::istream &file)
{
  try
  {
    unsigned long classId;
    std::string className;

    file >> classId;
    file >> className;

    if (classId)
    {
      itsArea = static_cast<NFmiArea *>(CreateSaveBase(classId));
      file >> *itsArea;
      NFmiArea *fixedArea = itsArea->DoPossiblePacificFix();
      if (fixedArea)
      {
        delete itsArea;
        itsArea = fixedArea;
      }
    }

    NFmiGridBase::Read(file);

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Metodin nimi ei tarkoita gridien sisältöä vaan rakennetta.
 * Metodi tarkistaa, onko area:t vertailtavissa grideissä samat ja onko
 * hilojen määrät x- ja y-suunnassa samat. Tietoa voidaan käyttää mm. optimoinnissa.
 *
 * \param theOtherGrid Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGrid::AreGridsIdentical(const NFmiGrid &theOtherGrid) const
{
  try
  {
    if ((itsXNumber == theOtherGrid.itsXNumber) && (itsYNumber == theOtherGrid.itsYNumber))
    {
      // areoiden tarkastelu on hankalaa ja tämä jää mahdollisesti vaillinaiseksi
      // miksi ClassId()-metodi ei voi olla const:i???
      if (const_cast<NFmiGrid *>(this)->ClassId() == const_cast<NFmiGrid &>(theOtherGrid).ClassId())
      {
        // ei oteta huomioon polster:in ja stereogriphicin mahdollista samanlaisuutta
        if (*itsArea == *theOtherGrid.Area())
          return true;
      }
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa currentin paikan suhteellisen sijainnin
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGrid::RelativePoint() const
{
  try
  {
    double relativeX = itsCurrentX / (itsXNumber - 1);
    double relativeY = itsCurrentY / (itsYNumber - 1);
    return NFmiPoint(relativeX, relativeY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa currentin paikan suhteellisen sijainnin
 * Huom!!! tämä ei toimi ainakaan vielä, eikä ehkä tarvitakaan ollenkaan (Marko).
 *
 * \param theIndex Undocumented, unused
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGrid::RelativePoint(unsigned long /* theIndex */) const
{
  try
  {
    double relativeX = itsCurrentX / (itsXNumber - 1);
    double relativeY = itsCurrentY / (itsYNumber - 1);
    return NFmiPoint(relativeX, relativeY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newLon Undocumented
 * \param newLat Undocumented
 * \param theMaxDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGrid::NearestLatLon(double newLon,
                             double newLat,
                             double theMaxDistance,
                             NFmiPoint *theGridPoint)
{
  try
  {
    NFmiPoint gridPoint = LatLonToGrid(newLon, newLat);
    if (theGridPoint)
      *theGridPoint = gridPoint;
    if (!GridPoint(gridPoint.X(), gridPoint.Y()))
      return false;
    if (theMaxDistance == kFloatMissing * 1000.)  // optimization..
      return true;

    NFmiPoint currentPoint = LatLon();
    NFmiLocation location1(currentPoint.X(), currentPoint.Y());
    NFmiLocation location2(newLon, newLat);
    if (location1.Distance(location2) <= theMaxDistance)
      return true;

    return false;  // doesn't set iterator into the original GridPoint
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//***********************************************************************************
//************** Tästä alkaa raakainteger dataan erikoistunut koodiosa **************
//************** Pitää testata, uudelleen muokata hieman helppotajuisemmaksi ********
//************** Kunhan saadaan testatuksi ja mietityksi loppuun asti ***************
//***********************************************************************************

// ----------------------------------------------------------------------
/*!
 * \param in Undocumented
 * \param theStartOffsetInBytes Undocumented
 * \param theDataStartsAfterString Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

static bool SeekStartingPoint(ifstream &in,
                              unsigned int theStartOffsetInBytes,
                              const string &theDataStartsAfterString)
{
  try
  {
    if (theStartOffsetInBytes)
    {
      in.ignore(theStartOffsetInBytes);
      return !in.fail();
    }
    else if (!theDataStartsAfterString.empty())
    {
      // int maxIgnoredCount = std::numeric_limits<double>::max(); // ei piru käänny!!!???!!!!
      int maxIgnoredCount = 2000000000;  // tähän iso luku
      int firstChar = theDataStartsAfterString[0];
      do
      {
        in.ignore(maxIgnoredCount, firstChar);
        if (!in.fail())
        {
          if (theDataStartsAfterString.size() == 1)
            return true;
          else if (theDataStartsAfterString.size() == 2)
          {
            int secondChar = theDataStartsAfterString[1];
            if (in.peek() == secondChar)
            {
              // int ch = in.get();
              in.get();
              return !(in.fail());
            }
          }
        }
      } while (!in.fail());
    }
    else
      return !(in.fail());

    return false;  // eof saavutettu, haku meni pieleen
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theStartingCorner Undocumented
 * \param walkXDimFirst Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGrid::SwapData(FmiDirection theStartingCorner, bool walkXDimFirst)
{
  try
  {
    if (walkXDimFirst)
      return NFmiGridBase::Swap(theStartingCorner);
    else
    {
      // ei ole vielä toteutettu!!!!!!!
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Gridin alustus integertyyppisen raakadata-tiedostosta (kirjoitettu
 * tiedostoon raakataulukkona, ei formatoitua dataa). Lukee datan
 * 'raaka'-integereinä, mutta muuttaa ne lennossa float muotoon.
 *
 * \param theFileName Name of the file containing the raw data
 * \param nx Number of raw data elements in X-direction
 * \param ny Number of raw data elements in Y-direction
 * \param theElementSizeInBytes The size of a single raw data element
 * \param isLittleEndian True, if the raw data is in little endian mode
 * \param theStartOffsetInBytes How many initial bytes to skip.
 *        If zero, one can use theDataStartsAfterString
 * \param theDataStartsAfterString If theStartOffsetInBytes is zero, this
 *        string is the start marker for the actual data.
 * \param theStartingCorner The grid corner where the raw data starts.
 * \param walkXDimFirst Whether to advance first in X- or Y-direction.
 * \param theBeforeConversionMissingValue Undocumented
 * \param theAfterConversionMissingValue Once the raw data has been converted
 *        to floats, this value will be converted into kFloatMissing
 * \param isSigned True if the raw data is signed
 * \param theConversionScale Multiplier for the data value, default is 1.
 * \param theConversionBase Shift for the data value, default is 0.
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiGrid::Init(const std::string &theFileName,
                    unsigned long nx,
                    unsigned long ny,
                    unsigned long theElementSizeInBytes,
                    bool isLittleEndian,
                    unsigned long theStartOffsetInBytes,
                    const string &theDataStartsAfterString,
                    FmiDirection theStartingCorner,
                    bool walkXDimFirst,
                    double theBeforeConversionMissingValue,
                    float theAfterConversionMissingValue,
                    bool isSigned,
                    float theConversionScale,
                    float theConversionBase)
{
  try
  {
    assert(walkXDimFirst);  // Juoksutusta ensin y-dimensio suuntaan ei ole vielä toteutettu!
    assert(theElementSizeInBytes > 0);  // DataElementin tavukoon pitää olla suurempi kuin 0!
    assert(theElementSizeInBytes <
           5);  // Ei ole vielä toteutettu yli 4 tavun integerien käsittelyä!
    assert(theDataStartsAfterString.size() <=
           2);  // Ei ole vielä toteutettu yli 2 merkin mittaisia datan alku merkkijonoja!

    bool status = NFmiGridBase::Init(nx, ny);
    ifstream in(theFileName.c_str(), ios::in | ios::binary);
    if (status && in)
    {
      unsigned long rowByteSize = nx * theElementSizeInBytes;
      std::vector<char> buffer(rowByteSize);
      std::vector<float> floatData(nx);  // tehdään float taulu, johon mahtuu rivin arvot
      if ((status = SeekStartingPoint(in, theStartOffsetInBytes, theDataStartsAfterString)) == true)
      {
        for (unsigned long i = 0; i < ny; i++)
        {
          in.read(&buffer[0],
                  rowByteSize);  // HUOM! read ei suostu ottamaan unsigned char-pointteria
          if (in.fail())
          {
            status = false;
            break;
          }

          int offset = (isLittleEndian ? 0 : theElementSizeInBytes - 1);
          int step = (isLittleEndian ? 1 : -1);

          for (unsigned int elem = 0; elem < nx; elem++)
          {
            double dvalue = 0.0;
            unsigned int pos = elem * theElementSizeInBytes + offset;
            double scale = 1.0;
            for (unsigned int b = 0; b < theElementSizeInBytes; b++)
            {
              unsigned char ch = buffer[pos];
              pos += step;
              if (isSigned && (b + 1 == theElementSizeInBytes))
              {
                dvalue += (ch & 0x7F) * scale;
                if ((ch & 0x80) != 0)
                  dvalue = -dvalue;  // safe, since loop ends now
              }
              else
                dvalue += ch * scale;
              scale *= 256;
            }

            if (dvalue == theBeforeConversionMissingValue)
              floatData[elem] = kFloatMissing;
            else
            {
              auto value = static_cast<float>(dvalue * theConversionScale + theConversionBase);
              if (value == theAfterConversionMissingValue)
                floatData[elem] = kFloatMissing;
              else
                floatData[elem] = value;
            }
          }

          for (unsigned long j = 0; j < floatData.size(); j++)
            itsData->FloatValue(i * nx + j, floatData[j]);
        }
      }
      in.close();
      if (status)
        status = SwapData(theStartingCorner, walkXDimFirst);
    }
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//***********************************************************************************
//************** Tähän loppuu raakainteger dataan erikoistunut koodiosa *************
//************** Pitää testata, uudelleen muokata hieman helppotajuisemmaksi ********
//************** Kunhan saadaan testatuksi ja mietityksi loppuun asti ***************
//***********************************************************************************

typedef pair<int, double> IndDistPari;

// ----------------------------------------------------------------------
/*! HUOM!!! EI OLE VIELÄ TOTEUTETTU, tähän on vain kopioitu komentteihin
 *  locationbagin koodi
 *
 *	Hakee listan paikkaindeksi/etäisyys metreinä pareja.
 *
 *	Listaan haetaan annettua paikkaa lähimmat datapisteet järjestyksessä
 *  lähimmästä kauimpaan. Listaan haetaan joko haluttu määrä lähimpiä
 *  pisteitä tai hakua voi myös rajoittaa maksimi etäisyydellä.
 *	Jos maksimi määräksi laitetaan -1, haetaan paikkoja niin paljon kuin
 *  löytyy (maxEtäisyys rajoitus huomiooon ottaen).
 *
 *	Huom! maxWantedLocations ohittaa maxDistance määrityksen (jos molemmat annettu)
 *
 *	Erilaisia kombinaatioita haun rajoituksessa:
 *
 *	\li theMaxWantedLocations = -1 ja theMaxDistance = kFloatMissing =>
 *      palauttaa kaikki sortattuna.
 *	\li theMaxWantedLocations = 5 ja theMaxDistance = kFloatMissing =>
 *      palauttaa 5 sortattuna.
 *	\li theMaxWantedLocations = -1 ja theMaxDistance = 45678.9m =>
 *      palauttaa kaikki annetun säteen sisältä sortattuna.
 *	\li theMaxWantedLocations = 5 ja theMaxDistance = 45678.9m =>
 *      palauttaa max 5 annetun säteen sisältä sortattuna.
 */
// ----------------------------------------------------------------------

std::vector<pair<int, double> > NFmiGrid::NearestLocations(const NFmiLocation & /* theLocation */,
                                                           int /* theMaxWantedLocations */,
                                                           double /* theMaxDistance */) const
{
  try
  {
    int size = 1;  //(int)this->GetSize();
    std::vector<IndDistPari> tempValues(size, make_pair(-1, kFloatMissing));

#ifdef COMMENTED_OUT

    for (int i = 0; i < size; i++)
      tempValues[i] = make_pair(i, theLocation.Distance(*this->itsLocations[i]));
    if (theMaxWantedLocations == -1 &&
        theMaxDistance ==
            kFloatMissing)  // molemmat rajoittimet puuttuvat, palautetaan kaikki sortattuna
    {
      sort(tempValues.begin(), tempValues.end(), LocationIndexDistanceLess<IndDistPari>());
      return tempValues;
    }
    else if (theMaxWantedLocations != -1 &&
             theMaxDistance == kFloatMissing)  // halutaan n kpl lahimpiä paikkoja
    {
      nth_element(tempValues.begin(),
                  tempValues.begin() + theMaxWantedLocations,
                  tempValues.end(),
                  LocationIndexDistanceLess<IndDistPari>());
      return std::vector<IndDistPari>(
          tempValues.begin(),
          tempValues.begin() + theMaxWantedLocations);  // palautetaan haluttu määrä locatioita
    }
    else  // theMaxDistance != kFloatMissing) // haetaan kaikki annetun säteen sisällä olevat paikat
    {
      sort(tempValues.begin(), tempValues.end(), LocationIndexDistanceLess<IndDistPari>());
      std::vector<IndDistPari>::iterator pos =
          find_if(tempValues.begin(),
                  tempValues.end(),
                  LocationIndexDistanceGreater<IndDistPari>(theMaxDistance));

      if (theMaxWantedLocations != -1)
      {
        std::vector<IndDistPari>::iterator maxWantedPos =
            tempValues.begin() + theMaxWantedLocations;
        if (pos > maxWantedPos)
          pos = maxWantedPos;
      }

      if (pos == tempValues.end())
        return tempValues;
      else
        return std::vector<IndDistPari>(tempValues.begin(), pos);
    }

#endif  // COMMENTED_OUT

    return tempValues;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiGrid::operator==(const NFmiGrid &theGrid) const
{
  try
  {
    if (this == &theGrid)
      return true;

    if (NFmiGridBase::operator==(theGrid))
    {
      if (itsArea && theGrid.itsArea)
      {
        if (*itsArea == *(theGrid.itsArea))
          return true;
      }
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Halutaan laskea this-gridin avulla interpoloituja arvoja theTargetGrid:lle. Sitä varten lasketaan
// kerran
// halutut hilapisteet (targetin hilan pisteet this-hilassa). Sisältää myös tiedon hilapisteen
// indeksistä.
// Tätä voidaan käyttää apuna kun interpolidaan monta kertaan hilasta toiseen arvoja (esim. eri
// ajoille,
// leveleille ja parametreille), cache lasketaan vain kerran. Cache-pisteet voidaan sitten antaa eri
// InterpolatedValue-metodeille. Ks. mallia NFmiQueryDataUtil::FillGridData-metodista.
void NFmiGrid::CalcLatlonCachePoints(NFmiGrid &theTargetGrid,
                                     NFmiDataMatrix<NFmiLocationCache> &theLocationCache)
{
  try
  {
    unsigned long sourceSizeX = XNumber();
    unsigned long sourceSizeY = YNumber();
    unsigned long targetSizeX = theTargetGrid.XNumber();
    unsigned long targetSizeY = theTargetGrid.YNumber();
    theLocationCache.Resize(targetSizeX, targetSizeY);
    for (theTargetGrid.Reset(); theTargetGrid.Next();)
    {
      NFmiLocationCache locCache;
      NFmiPoint latlon(theTargetGrid.LatLon());
      if (NearestLatLon(latlon.X(), latlon.Y(), 100 * 1000, &locCache.itsGridPoint))
      {
        locCache.itsLocationIndex = Index();
        locCache.CalcIsInterpolationNeeded(sourceSizeX, sourceSizeY);
      }
      theLocationCache[theTargetGrid.Index() % targetSizeX][theTargetGrid.Index() / targetSizeX] =
          locCache;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the given point is in the data
 */
// ----------------------------------------------------------------------

bool NFmiGrid::IsInside(const NFmiPoint &theLatLon) const
{
  try
  {
    return IsInsideGrid(LatLonToGrid(theLatLon));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Maailma datoja annetaan latlon- projektioissa, niin että
// alue menee esim. (0, -90) -> (359.5, 90) hilana, eli hila ei mene 360:een
// vaan jaa yhtä hila saraketta vajaaksi, koska siinä olisi sama data
// kuin mitä olisi 1. sarakkeessa 0-pituuspiirillä.
// Tästä seuraa ongelmia kun dataa interpoloideen erilaisiin alueisiin,
// jotka menevät Atlanti keskeisillä alueilla ja 0-pituuspiirin ylitse.
// Tällöin tarvitaan tietoa onko data levitettävissä, eli voidaanko 1.
// sarake ottaa käyttöön interpolaatioissa myös uudeksi viimeiseksi sarakkeeksi.
// TODO ei ole tehty tutkimaan Atlantikeskeistä hilaa joka menee esim.
// (-180, -90) -> (179.5, 90)
bool NFmiGrid::IsStrechableGlobalGrid(const NFmiGrid &theGrid)
{
  try
  {
    if (theGrid.Area() && theGrid.Area()->ClassId() == kNFmiLatLonArea)
    {
      const NFmiArea &area = *(theGrid.Area());
      if (area.PacificView())
      {
        const NFmiPoint globalBL(0, -90);
        if (area.BottomLeftLatLon() == globalBL && area.TopRightLatLon().X() < 360)
        {
          float totalLongitudeDiff =
              static_cast<float>(area.TopRightLatLon().X() - area.BottomLeftLatLon().X());
          float gridPointLongitudeDiff = totalLongitudeDiff / (theGrid.XNumber() - 1);
          if (360 - area.TopRightLatLon().X() <= gridPointLongitudeDiff * 1.01f)
            return true;
        }
      }
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the grid + projection combination
 */
// ----------------------------------------------------------------------

std::size_t NFmiGrid::HashValue() const
{
  try
  {
    std::size_t hash = NFmiGridBase::HashValue();
    if (itsArea != nullptr)
      Fmi::hash_combine(hash, itsArea->HashValue());

    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const Fmi::SpatialReference &NFmiGrid::SpatialReference() const
{
  try
  {
    if (itsArea)
      return itsArea->SpatialReference();

    return NFmiWGS84::SpatialReference();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the native coordinates in a 2D matrix
 */
// ----------------------------------------------------------------------

Fmi::CoordinateMatrix NFmiGrid::CoordinateMatrix(bool wrap) const
{
  try
  {
    return itsArea->CoordinateMatrix(XNumber(), YNumber(), wrap);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
