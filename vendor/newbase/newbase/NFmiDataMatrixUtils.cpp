#include "NFmiDataMatrixUtils.h"
#include "NFmiInterpolation.h"
#include "NFmiQueryDataUtil.h"
#include <macgyver/Exception.h>

namespace
{
double FixIndexOnEdges(double index, size_t matrixSize)
{
  try
  {
    const double epsilon = 0.000001;
    if (index < 0 && NFmiQueryDataUtil::IsEqualEnough(index, 0., epsilon))
      index = 0;
    else
    {
      double xMaxLimit = matrixSize - 1.;
      if (index >= xMaxLimit && NFmiQueryDataUtil::IsEqualEnough(index, xMaxLimit, epsilon))
        index = xMaxLimit - epsilon;
    }
    return index;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

namespace DataMatrixUtils
{
// Tämä funktio laskee interpoloidun arvon matriisin datasta.
// Annettu piste on halutussa suhteellisessa koordinaatistossa (esim. 0,0  -  1,1 maailmassa)
// ja kyseinen koordinaatisto (rect-olio) pitää antaa parametrina.
// HUOM! rect-olio ja matriisi ovat y-akselin suhteen käänteisiä!
float InterpolatedValue(const NFmiDataMatrix<float>& m,
                        const NFmiPoint& thePoint,
                        const NFmiRect& theRelativeCoords,
                        FmiParameterName theParamId,
                        bool fDontInvertY,
                        FmiInterpolationMethod interp)
{
  try
  {
    float value = kFloatMissing;
    double xInd =
        ((m.NX() - 1) * (thePoint.X() - theRelativeCoords.Left())) / theRelativeCoords.Width();
    // yInd-laskussa pitää y-akseli kääntää
    double yInd =
        fDontInvertY
            ? ((m.NY() - 1) * (thePoint.Y() - theRelativeCoords.Top())) / theRelativeCoords.Height()
            : ((m.NY() - 1) *
               (theRelativeCoords.Height() - (thePoint.Y() - theRelativeCoords.Top()))) /
                  theRelativeCoords.Height();
    // xInd ja yInd voivat mennä juuri pikkuisen yli rajojen ja laskut menevät muuten pieleen, mutta
    // tässä korjataan indeksejä juuri pikkuisen, että laskut menevät näissä tapauksissa läpi ja
    // riittävän oikein arvoin.
    xInd = ::FixIndexOnEdges(xInd, m.NX());
    yInd = ::FixIndexOnEdges(yInd, m.NY());

    int x1 = static_cast<int>(std::floor(xInd));
    int y1 = static_cast<int>(std::floor(yInd));
    int x2 = x1 + 1;
    int y2 = y1 + 1;
    if (x1 >= 0 && x2 < static_cast<int>(m.NX()) && y1 >= 0 && y2 < static_cast<int>(m.NY()))
    {  // lasketaan tulos vain jos ollaan matriisin sisällä, tähän voisi reunoille laskea erikois
       // arvoja jos haluaa
      double xFraction = xInd - x1;
      double yFraction = yInd - y1;
      if (interp == kNearestPoint)
        return m.At(FmiRound(xInd), FmiRound(yInd));
      else
      {
        if (theParamId == kFmiWindDirection || theParamId == kFmiWaveDirection)
          value = static_cast<float>(NFmiInterpolation::ModBiLinear(
              xFraction, yFraction, m.At(x1, y2), m.At(x2, y2), m.At(x1, y1), m.At(x2, y1), 360));
        else if (theParamId == kFmiWindVectorMS)
          value = static_cast<float>(NFmiInterpolation::WindVector(
              xFraction, yFraction, m.At(x1, y2), m.At(x2, y2), m.At(x1, y1), m.At(x2, y1)));
        else
          value = static_cast<float>(NFmiInterpolation::BiLinear(
              xFraction, yFraction, m.At(x1, y2), m.At(x2, y2), m.At(x1, y1), m.At(x2, y1)));
      }
    }
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PrettyPrint(std::ostream& s,
                 const NFmiDataMatrix<float>& m,
                 bool printYInverted,
                 bool printIndexAxies)
{
  try
  {
    typedef typename NFmiDataMatrix<float>::size_type sz_type;
    sz_type rows = m.NY();
    sz_type columns = m.NX();

    s << static_cast<unsigned int>(columns) << " " << static_cast<unsigned int>(rows) << std::endl;

    if (printYInverted == false)
    {
      for (sz_type j = 0; j < rows; j++)
      {
        for (sz_type i = 0; i < columns; i++)
          s << m[i][j] << " ";
        s << std::endl;
      }
    }
    else
    {  // tulostus käänteisessä rivi-järjestyksessä
      for (long j = rows - 1; j >= 0; j--)
      {
        if (printIndexAxies)
          s << j << "\t";
        for (sz_type i = 0; i < columns; i++)
          s << m[i][j] << " ";
        s << std::endl;

        if (j == 0)  // luulen että koska j on unsigned tyyppinen, pitää tässä tarkastella 0-riviä,
                     // koska for-loopissa j-- lauseke ei ikinä vie j:n arvoa negatiiviseksi
                     // (kierähtää 0:sta tosi isoksi luvuksi)
        {
          if (printIndexAxies)
          {
            for (sz_type i = 0; i < columns; i++)
              s << i << " ";
          }
          break;
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace DataMatrixUtils
