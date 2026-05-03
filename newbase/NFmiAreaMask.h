// ======================================================================
/*!
 * \file NFmiAreaMask.h
 * \brief Interface for NFmiAreaMask class
 */
// ======================================================================

#pragma once

#include "NFmiInfoData.h"
#include "NFmiMetTime.h"
#include "NFmiPoint.h"
#include "NFmiString.h"

#include <memory>

#include <vector>

class NFmiCalculationCondition;
class NFmiDataIdent;
class NFmiLevel;
class NFmiParam;
class NFmiFastQueryInfo;
class NFmiSimpleCondition;
class NFmiCalculationParams;
class NFmiMacroParamValue;

//! Undocumented class
class NFmiAreaMask
{
 public:
  /*!
   * When a mask contains submasks (a masklist), subsequent masks are added up using
   * the operators in the enumeration. Each subsequent mask always contains information
   * on how two consecutive masks will be added up.
   */
  //! Mask operations
  enum BinaryOperator
  {
    kNoValue,
    kAnd,
    kOr,
    kXor,
    kNot
  };

  //! Mask types
  enum Type
  {
    kNoType,
    kBinary,
    kInfo,
    kCalculated,
    kLatlonArea,
    kConstantValue,
    kScriptVariable
  };
  /*
    //! Classification of smartinfo data
    enum DataType
          {
            kEditable = 1,	//!< Editable, watchable and maskable
            kViewable,		//!< Not meant for editing, only for watching and masking
            kStationary,		//!< For example topography (does not change in time)
            kCopyOfEdited,	//!< Copy or original unedited data, used for comparisons
            kObservations,	//!< Observations
            kCompareModels,	//!< A type used in editor comparison modes in infoorganizer
            kCalculatedValue,	//!< For mask classification, some calculated value not in the data
    itself
            kNoDataType		//!< For error conditions
          };
  */

  /*!
   * For calculation interpretation in smarttool. Should use class
   * based methods instead.
   */
  //! Mathematical calculations
  enum CalculationOperationType
  {
    NoType,                    //!< Error condition
    InfoVariable,              //!< A parameter in the queryinfo
    ResultInfoVariable,        //!< A parameter in the queryinfo (unused?)
    CalculatedVariable,        //!< Calculatable variable (sun angle etc)
    Assigment,                 //!< Assignment operator
    Operator,                  //!< +, -, *, /, %, ^ etc
    UnaryOperator,             //!< unary + and -
    UnaryBinaryOperator,       //!< NOT, ^, XOR etc
    BinaryOperatorType,        //!< &&, AND, || OR etc
    Variable,                  //!< A regular variable
    Constant,                  //!< A constant
    FunctionTimeIntergration,  //!< A time dependent function such as min and max
    FunctionAreaIntergration,  //!< An area dependent function such as min and max
    FunctionPeekXY,            //!< Peek infodata with wanted grid offset
    FunctionPeekXY2,  //!< Peek infodata with wanted modified/macroData grid offset (this works with
    //! GridSizeX and Y functions)
    FunctionPeekXY3,             //!< Peek infodata with wanted kilometers x and y direction
    MathFunctionStart,           //!< For example cos(...
    ThreeArgumentFunctionStart,  //!< For example SUMT(-6, 0, RR) eli tuossa on kolme argumenttia,
                                 //   ja ne voivat sisältää mitä tahansa kaavoja/laskuja
    MetFunction,  //!< For example adv(T_Hir) eli tuossa voi olla erilaisia meteorologisia
    //! funktioita (mm. grad, adv, div. lap, rot, jne.), joilla on erilainen määrä
    //! argumentteja.
    VertFunctionStart,  //!< For example vertp_max(WS_hir, p1, p2), which seeks max WS-value between
    //! p1 and p2 pressure levels
    DeltaZFunction,  //!< Can be used with SumZ and MinH type functions, know current delta-height
    //! of z-calculations as height axel is divided to smaller parts.
    SoundingIndexFunction,  //!< Luotaus-index funktioita esim. CAPESUR_HIR, CIN500M_EC jne.
    RampFunction,           //!< A ramp function such as RU, RD and DD
    Comparison,             //!< Mask comparison: >, <, >=, = etc
    InfoMask,               //!< A regular mask such as in T > 3
    CalculationMask,        //!< A calculatable mask such as sunangle>0, lat>65 etc
    StartParenthesis,       //!< Beginning of expression parenthesis
    EndParenthesis,         //!< End of expression parenthesis
    CommaOperator,  //!< ','-merkki joka toimii argumenttien erottimena tietyissä funktioissa
    SimpleConditionCalculation,  //!< Simple condition type calculation inside string-literal, like
                                 //!< "T_ec > xxx"
    EndOfOperations              //!< End of operators marker
  };

  //! Similar to CalculationOperationType, should redesign
  enum CalculationOperator
  {
    NotOperation,
    Add,  //!< Addition (+)
    Sub,  //!< Substraction (-)
    Div,  //!< Division (/)
    Mul,  //!< Multiplication (*)
    Mod,  //!< Modulo (%)
    Pow   //!< Power (^)
  };

  //! Integratable functions
  enum FunctionType
  {
    NotFunction,
    Avg,             //!< Average
    Min,             //!< Minimum
    MinH,            //!< Height of minimum
    Max,             //!< Maximum
    MaxH,            //!< Height of maximum
    WAvg,            //!< Distance weighted average
    Sum,             //!< Sum
    Get,             //!< Get -function just gets value
    FindH,           //!< FindH -function finds the height where wanted value exists
    FindC,           //!< FindC -function finds how many wanted values there is
    FindHeightCond,  //!< FindHeightCond -function finds the height where wanted simple-condition
                     //!< turns true
    FindCountCond,   //!< FindCountCond -function calculates how many time simple-condition status
                     //!< changes

    // Seuraaavt ovat ns. 'meteorologisia' funktioita
    Grad,        //!< Gradientti
    Adv,         //!< Advektio
    Divergence,  //!< Divergence en uskaltanut laittaa Div:ia, koska se on jo määritelty
    //! CalculationOperator-kohdassa
    Lap,  //!< Laplace
    Rot,  //!< Rotor
    // Seuraaavt ovat edellä olevien funktioiden kakkos versiot, missä peek-xy kurkkaukset tehdään
    // aina leveys- ja pituuspiirien suuntaisesti
    Grad2,        //!< Gradientti
    Adv2,         //!< Advektio
    Divergence2,  //!< Divergence en uskaltanut laittaa Div:ia, koska se on jo määritelty
    //! CalculationOperator-kohdassa
    Lap2,  //!< Laplace
    Rot2,  //!< Rotor
    // Seuraavat ovat vertikaaliset met-funktion haku tavat
    VertP,    //!< vertikaali-hakua paine-rajoissa hakien
    VertZ,    //!< vertikaali-hakua korkeus[m]-rajoissa hakien
    VertFL,   //!< vertikaali-hakua lentopinta rajoissa hakien
    VertHyb,  //!< vertikaali-hakua hybrid-pintojen mukaan hakien
    // Seuraavat ovat Probability laskujen alue määritys
    AreaRect,    //!< Todennäköisyys/kokooma laskut laatikon sisältä
    AreaCircle,  //!< Todennäköisyys/kokooma laskut ympyrän sisältä
    // Seuraavat ovat Probability laskujen ehtoja
    ProbOver,       //!< Todennäköisyys ehto: arvo on alle rajan (value < limit)
    ProbOverEq,     //!< Todennäköisyys ehto: arvo on alle rajan (value <= limit)
    ProbUnder,      //!< Todennäköisyys ehto: arvo on yli rajan (value > limit)
    ProbUnderEq,    //!< Todennäköisyys ehto: arvo on yli rajan (value >= limit)
    ProbEqual,      //!< Todennäköisyys ehto: arvo on sama kuin raja (value == limit)
    ProbNotEqual,   //!< Todennäköisyys ehto: arvo on erisuuri kuin raja (value != limit)
    ProbBetween,    //!< Todennäköisyys ehto: arvo on rajojen välissä (limit1 < value < limit2)
    ProbBetweenEq,  //!< Todennäköisyys ehto: arvo on rajojen välissä (limit1 <= value <= limit2)
    // Seuraavat ovat lähin asema data arvo laskujen funktioita
    ClosestObsValue,  //!< Todennäköisyys ehto: arvo on alle rajan (value < limit)
    // Seuraavat ovat lähin asema data arvo laskujen ehtoja (ei ole vielä oikeasti kuin yksi)
    ClosestObsTimeOffset,  //!< Todennäköisyys ehto: arvo on alle rajan (value < limit)
    // TimeRange funktiot käyvät dataa läpi aina datan originaali aika-stepeillä
    TimeRange,  //!< Esim. Laske maksimi arvo aikavälillä -3 - +3 h, datan omassa aikaresoluutiossa
    // Seuraavat ovat aika + vertikaalitasojen väliset hakufunktiot (hae esim. maksimi tietyltä
    // aika- ja level-väleiltä)
    TimeVertP,    //!< aika+vertikaali-hakua paine-rajoissa hakien
    TimeVertZ,    //!< aika+vertikaali-hakua korkeus[m]-rajoissa hakien
    TimeVertFL,   //!< aika+vertikaali-hakua lentopinta rajoissa hakien
    TimeVertHyb,  //!< aika+vertikaali-hakua hybrid-pintojen mukaan hakien
    Occurrence,  //!< aika-väli + aluehaku datasta, laskee kuinka monta kertaa tarkastelupisteessä
    Occurrence2,  //!< Sama kuin edellä, mutta tarkoitettu toistaiseksi vain hiladatalle (käytetään
                  //!< occurance with simple-condition funktion kanssa)
    PeekT,       //!< 'Kurkistetaan' arvo halutun aikahypyn [h] päästä
    Resolution,  //!< Tällä asetetaan macroParamin lasketun hilan toive resoluutio, joko jostain
                 //! datasta tai suoraan kilometreinä.
    CalculationPoint,  //!< Tällä asetetaan macroParamin laskentapiste (lat,lon). Laskut saavat
                       //! muissa hilapisteissä puuttuvaa.
    ObservationRadius,  //!< Tällä määrätään että laskuissa otetaan huomioon havainnoista vain x
                        //![km] säteellä olevat arvot.
    //! joku ehto toteutuu
    Med,                  //!< Mediaani laskut
    ProbSimpleCondition,  //!< Todennäköisyys laskut käyttäen ehtolauseketta esim. "T_ec > 0"
    LatestValue,          //!< Hae datasta sen viimeisimmän ajan arvo
    PreviousFullDays,     //!< Laske datasta aikasarja integraatio mennen halutun määrän päiviä
                          //!< taaksepäin aina halutun päivän 0 lokaali tuntiin asti.
    TimeDuration,  //!< Katso laskentahetkestä eteen/taaksepäin kuinka kauan simple-condition ehto
                   //!< pitää paikkaansa
    LocalExtremes,  //!< Etsi lokaalit (ja globaalit) minimit ja maksimit kentästä, mutta vain jos
                    //!< ne ovat tarpeeksi 'merkittäviä'
    SymbolTooltipFile,  //!< Tällä määritetään mahdollinen tiedosto, josta haetaan tooltippiä varten
                        //!< aputekstejä eri symboleille
    MacroParamDescription,  //!< Jos tooltippiin halutaan tälle macroParmille yleisselite, se
                            //!< annetaan tällä
    CalculationType,  //!< Tällä voi määritellä että onko joku laskenta esim. indeksi tyyppinen vai
                      //!< normi reaaliluku
    PeekZ,  //!< 'Kurkistetaan' arvo vertikaali suunnassa halutussa yksikössä
            //!< (hPa/m/FL/hybrid-level)
    SimpleConditionUsedAsStationData,  //!< Jos pääfunktion (esim. area_sum funktion) datan tuottaja
                                       //!< on sama kuin simple-condition tuottaja ja kyse on
                                       //!< asemadatasta, pitää kyseistä simple-condition dataa
                                       //!< käsitellä myös asemadatana.
    ModAvg,  //!< Käytetään suunta parametrien kanssa, ottaa huomioon jatkuvuuden 0/360 kohdassa
    ModMin,  //!< Käytetään suunta parametrien kanssa, ottaa huomioon jatkuvuuden 0/360 kohdassa
    ModMax,  //!< Käytetään suunta parametrien kanssa, ottaa huomioon jatkuvuuden 0/360 kohdassa
    WorkingThreadCount,  //!< Jos käyttäjä haluaa optimoida laskentoja ja käyttää tietyn määrän
                         //!< loogisia coreja laskennoissa
    FixedBaseData,  //!< Jos käyttäjä haluaa fiksata käytetyn laskenta hilan johonkin olemassa
                    //!< olevan datan hilaan
    MultiParamTooltipFile,  //!< Mahd. tiedosto, josta haetaan tooltippiä varten aputekstejä monen
                            //!< parametrin avulla
    MultiParam2,  //!< 2. käytetty multi-param, pakollinen, jos käytetty MultiParamTooltipFile:a
    MultiParam3,  //!< 3. käytetty multi-param, mahdollinen, jos käytetty MultiParamTooltipFile:a
    SecondParamFromExtremeTime  // Etsitään par1:n min/max ja siitä paikasta ja extreme ajasta
                                // palautetaan toisen parametrin arvo
  };

  //! Function direction, e.g. with 'met'-functions x- and/or y-direction
  enum MetFunctionDirection
  {
    NoDirection,    // default value, not defined
    DirectionX,     // calcualtions are made only in x-direction or along the latitude
    DirectionY,     // calcualtions are made only in y-direction or along the longitude
    DirectionXandY  // calcualtions are made both x- and y-directions
  };

  //! Regular unary functions applicable to variables and expressions
  enum MathFunctionType
  {
    NotMathFunction,
    Exp,
    Sqrt,
    Log,
    Log10,
    Sin,
    Cos,
    Tan,
    Sinh,
    Cosh,
    Tanh,
    Asin,
    Acos,
    Atan,
    Ceil,
    Floor,
    Round,
    Abs,
    Rand
  };

  enum class SimpleConditionRule
  {
    NotAllowed,
    Allowed,
    MustHave
  };

  virtual ~NFmiAreaMask();
  NFmiAreaMask() {}
  virtual void Initialize() = 0;  // tämä on konstruktorin jälkeen kutsuttava virtuaalinen
                                  // initialisointi (koska konstruktorissa ei voi kutsua
                                  // virtuaali funktioita)

  // Default constructor intentionally left for the compiler
  // NFmiAreaMask ();

  // Copy constructor intentionally left for the compiler
  // NFmiAreaMask (const NFmiMaskOperation & theMaskOperation);

  virtual bool IsMasked(const NFmiPoint &theLatLon) const = 0;
  virtual bool IsMasked(int theIndex) const = 0;
  virtual double MaskValue(const NFmiPoint &theLatLon) const = 0;

  // fUseTimeInterpolationAlways-parametri on SmartTool-kielen MaxT, SumT jne. funktioita varten ja
  // tarkemmin niissä laskettavia argumenttien toimivuuden takia.
  // Tällä virityksellä varmistetaan että uudet funktiot toimivat ja olemassa olevat toimivat
  // nykyisillä optimoinneilla mahdollisimman nopeasti.
  virtual double Value(const NFmiCalculationParams &theCalculationParams,
                       bool fUseTimeInterpolationAlways) = 0;
  virtual double HeightValue(double theHeight,
                             const NFmiCalculationParams &theCalculationParams) = 0;
  virtual double PressureValue(double thePressure,
                               const NFmiCalculationParams &theCalculationParams) = 0;
  virtual bool IsEnabled() const = 0;
  virtual void Enable(bool theNewState) = 0;
  virtual bool Time(const NFmiMetTime &theTime) = 0;
  virtual void Condition(const NFmiCalculationCondition &theOperation) = 0;
  virtual const NFmiCalculationCondition &Condition() const = 0;
  virtual bool IsRampMask() const = 0;
  virtual bool IsWantedParam(const NFmiDataIdent &theParam,
                             const NFmiLevel *theLevel = 0) const = 0;
  virtual const NFmiString MaskString() const = 0;
  virtual std::shared_ptr<NFmiFastQueryInfo> Info() = 0;
  virtual void UpdateInfo(std::shared_ptr<NFmiFastQueryInfo> &theInfo) = 0;
  virtual const NFmiDataIdent *DataIdent() const = 0;
  virtual const NFmiParam *Param() const = 0;
  virtual const NFmiLevel *Level() const = 0;
  virtual void Level(const NFmiLevel &) = 0;
  virtual NFmiInfoData::Type GetDataType() const = 0;
  virtual void SetDataType(NFmiInfoData::Type theType) = 0;
  virtual bool UseLevelInfo() const = 0;
  virtual bool UsePressureLevelInterpolation() const = 0;
  virtual void UsePressureLevelInterpolation(bool newValue) = 0;
  virtual double UsedPressureLevelValue() const = 0;
  virtual void UsedPressureLevelValue(double newValue) = 0;

  virtual void LowerLimit(double theLowerLimit) = 0;
  virtual void UpperLimit(double theUpperLimit) = 0;
  virtual double LowerLimit() const = 0;
  virtual double UpperLimit() const = 0;
  virtual void MaskOperation(FmiMaskOperation theMaskOperation) = 0;
  virtual FmiMaskOperation MaskOperation() const = 0;

  virtual bool AddMask(NFmiAreaMask *theMask) = 0;
  virtual NFmiAreaMask *AreaMask(int theIndex) const = 0;
  virtual bool RemoveSubMask(int theIndex) = 0;
  virtual void MaskType(Type theType) = 0;
  virtual Type MaskType() const = 0;
  virtual NFmiAreaMask *Clone() const = 0;
  virtual void PostBinaryOperator(BinaryOperator newOperator) = 0;
  virtual BinaryOperator PostBinaryOperator() const = 0;
  virtual CalculationOperationType GetCalculationOperationType() const = 0;
  virtual void SetCalculationOperationType(CalculationOperationType newValue) = 0;
  virtual CalculationOperator GetCalculationOperator() const = 0;
  virtual void SetCalculationOperator(CalculationOperator newValue) = 0;

  virtual MathFunctionType GetMathFunctionType() const = 0;
  virtual void SetMathFunctionType(MathFunctionType newValue) = 0;
  virtual FunctionType GetFunctionType() const = 0;
  virtual void SetFunctionType(FunctionType newType) = 0;
  virtual FunctionType GetSecondaryFunctionType() const = 0;
  virtual void SetSecondaryFunctionType(FunctionType newType) = 0;
  virtual MetFunctionDirection GetMetFunctionDirection() const = 0;
  virtual void GetMetFunctionDirection(MetFunctionDirection newValue) = 0;
  virtual int IntegrationFunctionType() const = 0;
  virtual void IntegrationFunctionType(int newValue) = 0;
  // tämän avulla annetaan laskuissa tarvittavia eri pituisia argumenttilistaja
  // (käytössä ainakin uusille vertikaali funktioille)
  virtual void SetArguments(std::vector<float> &theArgumentVector) = 0;
  virtual int FunctionArgumentCount() const = 0;  // käytössä ainakin uusille vertikaali funktioille
  virtual void FunctionArgumentCount(
      int newValue) = 0;  // käytössä ainakin uusille vertikaali funktioille

  // HUOM! seuraavat toimivat oikeasti vain NFmiBinaryMask:in kanssa.
  virtual void SetAll(bool theNewState) = 0;  // Asettaa koko maskin kaikki arvot halutuksi.
  virtual void Mask(int theIndex, bool newStatus) = 0;
  // Joillain smarttool funktoilla voi olla simple-condition ehto (esim. "T_ec > T_hir"), jota
  // käytetään mm. erilaisissa integraatiolaskuissa
  virtual const std::shared_ptr<NFmiSimpleCondition> &SimpleCondition() const = 0;
  virtual void SimpleCondition(std::shared_ptr<NFmiSimpleCondition> &theSimpleCondition) = 0;
  virtual float FunctionDataTimeOffsetInHours() const = 0;
  virtual void FunctionDataTimeOffsetInHours(float newValue) = 0;
  // Jos kyse infoAreaMask:ista ja kyse on asemadatasta, ja on käytetty havaintoasemien etäisyys
  // rajoitinta (observationradius = x), palautetaan false, jos lähin havaintoasema on liian kaukana
  // laskentapisteestä. Kaikissa muissa tilanteissa palautetaan true.
  // Jos kyse multi-data tapauksesta, asetetaan theCalculationParams.itsCurrentMultiInfoData
  // osoittamaan oikeaan dataan.
  virtual bool CheckPossibleObservationDistance(
      const NFmiCalculationParams &theCalculationParamsInOut) = 0;

  static std::shared_ptr<NFmiFastQueryInfo> DoShallowCopy(
      const std::shared_ptr<NFmiFastQueryInfo> &theInfo);
  static std::vector<std::shared_ptr<NFmiFastQueryInfo>> DoShallowCopy(
      const std::vector<std::shared_ptr<NFmiFastQueryInfo>> &infoVector);
  static std::shared_ptr<NFmiAreaMask> DoShallowCopy(
      const std::shared_ptr<NFmiAreaMask> &theMask);
  static std::vector<std::shared_ptr<NFmiAreaMask>> DoShallowCopy(
      const std::vector<std::shared_ptr<NFmiAreaMask>> &theMaskVector);

};  // class NFmiAreaMask

// ======================================================================
