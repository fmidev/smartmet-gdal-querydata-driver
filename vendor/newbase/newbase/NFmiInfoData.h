// ======================================================================
/*!
 * \file NFmiInfoData.h
 * \brief NFmiInfoData namespace declarations \par
 *
 * Tähän on kerätty määrityksiä, mitä tarvitaan mm. erilaisten maskien
 * ja datojen määrityksissä. Olen kerännyt ne tähän, koska Metkun editorin
 * luokkakirjastojen siivous vaatii tietyn koodin siirtämistä newbase:en.
 */
// ======================================================================

#pragma once

namespace NFmiInfoData
{
//! Classification of smartinfo data (Metkun editorissa käytettyjen datojen luokituksia)
enum Type
{
  kNoDataType = 0,  // tämä on mm. virhetilanteita varten
  kEditable = 1,    // tätä voi editoida, katsoa ja maskata
  kViewable = 2,  // tätä dataa ei ole tarkoitus editoida vaan esim. katsoa ja käyttää maskina
                  // (HUOM! tämä on myös ns. pää malli datat tyyppi, ks. kModelHelpData)
  kStationary = 3,  // tarkoittaa mm. topografiaa (eli ei muutu ajan mukana)
  kCopyOfEdited = 4,  // alkuperäisen editoitavan datan kopio, käytetään mm. vertailtaessa näytöllä
                      // alkuperäistä dataa ja uutta muutettua dataa
  kObservations = 5,     // havaintodataa
  kCalculatedValue = 6,  // tämä on mm. maskin luokitusta varten eli esim. latitude/longitude maskia
                         // varten tai tms., joka ei ole queryinfosta poimittu
  kKepaData = 7,  // tämä on viimeisin tietokannassa oleva data (mm. aikasarjoihin vertailun vuoksi)
  kClimatologyData = 8,  // klimatologista dataa (mm. lämpötilan fraktaalit aikasarjaan apudataksi)
  kAnalyzeData = 9,      // Esim. Mesa:in tuottama analyysi data, jota käytetään mm. "analyysin
                         // liu'uttamisessa ennusteeseen" -työkalussa
  kScriptVariableData = 10,  // skiptissä olevien muuttujien hanskaaminen tapahtuu tämän tyyppisen
                             // 'calculaattorin' avulla
  kAnyData = 11,  // tämä on jokeri-tyyppi, kun on annettu epämääräinen parid ja/tai prodid ja pitää
                  // etsiä sitä läpi kaikkien datojen
  kSatelData = 12,         // satelliitti kuva dataa
  kFlashData = 13,         // salama dataa
  kMacroParam = 14,        // makro-parametri eli esim. RESULT = T_EC - T_HIR makrolla laskettu
  kHybridData = 15,        // mallipinta dataa
  kFuzzyData = 16,         // kalman/fuzzy filttereillä muokattua dataa
  kVerificationData = 17,  // verifioinnissa käytettävää dataa (analyysi - ennuste vertailua)
  kModelHelpData = 18,     // tähän laitetaan tietyn mallin ns. apudatat, joissa on esim. apu
                           // parametreja, pika dataa ja dataa eri alueilta
                           // Huom! nyt laitetaan myös esi lasketut sounding-index datat
  kTrajectoryHistoryData = 19,  // tähän laitetaan tietyn mallin ns. trajektorihistoria datat
  kTEMPCodeSoundingData = 20,   // tähän laitetaan TEMP-koodeista puretut luotausdatat
  kCrossSectionMacroParam =
      21,  // poikkileikkaus makro-parametri eli esim. result = T_EC - T_HIR makrolla laskettu
  kEditingHelpData = 22,  // Kun editori on ns. helpdata-moodissa, sillä lähetettyjä datoja luetaan
                          // sisään ja käytetään tämän data tyyppisinä
  kConceptualModelData = 23,  // Kun editorilla halutaan katsoa käsiteanalyysejä, on tämä datatyyppi
  kSingleStationRadarData = 24,  // Tämä data tyyppi on sitä varten että jos tulee yksittäisille
                                 // asemille tutkadataa (aluksi tämän avulla tehtiin saudi param
                                 // popup-valikko)
  kQ3MacroParam = 25,  // makro-parametri joka lasketaan Q3-enginellä eli esim. RESULT = T_EC -
                       // T_HIR makrolla laskettu
  kCapData = 26,       // Kun editorilla halutaan katsoa CAP/Varoke II dataa
  kWmsData = 27,
  kMapLayer = 28,  // Erilaiset karttaaineista datat ovat tämän tyyppisiä (background map,
                   // overlay-map, land borders, wms-map-layer, etc)
  kTimeSerialMacroParam =
      29,  // aikasarja makro-parametri eli esim. result = T_EC - T_HIR makrolla laskettu
  kSoundingParameterData = 1000  // HUOM! tästä on tarkoitus päästä eroon, mukana nyt siksi että on
                                 // olemassa näyttömakroja, joissa on käytetty tätä.
  // Vanha käyttö: luotaus parametrien (CAPE, CIN, jne.) erikois laskelmat tätä kautta, laitoin
  // luvuksi 100, koska joudun laskemaan yhteen aina kaksi datatypeä esim. kHybrid ja tämän, että
  // tiedän minkä tyyppistä dataa käyttetään laskuissa
};

// SmartMetissa on muutama erikoisparamtri, joiden enumin laitoin nyt toistaiseksi tähän
enum SpecialParameters
{
  kFmiSpSynoPlot = 997,  // synop-plot parametri
  kFmiSpDrawDifferenceParam =
      999,  // kun SmartMetissa piirretään editoidun datan ja originaali datan eroa, käytetään tätä
  kFmiSpSynoXProducer = 9990,  // Tämä on SynopX tuottaja id eli synop datat ilman liikkuvia asemia
                               // (laivat, poijut). Minulla ei ollut erikois tuottajille omaa
                               // paikkaa.
  kFmiSpTvsTdQualityCheckParam =
      9991,  // Smartmetin data-quality check erikois parametri eli T >= Td -tarkistus
  kFmiSpStreamline = 9992,  // TotalWind-parametrille lisätään tietyissä tilanteissa uusi meta-param
                            // streamline piirtoa varten. Tämä tulee siis vain SmartMetin
                            // popup-valikkoihin
  kFmiSpEcmwf3Vrk = 9993,  // Pitää tehdä viritys 3 vrk EC datojen kanssa, että ne saadaan mukaan
                           // smarttool-kieleen, eli smarttool-intepreter laittaa ec3vrk-tuottajat
  // tälläisiksi ja smarttool-modifier muuttaa haut oikeisiin tuottajiin ja
  // datatyyppeihin
  kFmiSpSelectedGridPoints = 9994,  // kun visualisoidaan editoinnissa valittuja pisteita
                                    // toolmasterilla, käytetään tätä parametria
  kFmiSpMetarPlot = 9995,           // synop-plot:iin liittyvä metar-plot parametri
  kFmiSpMinMaxPlot = 9996,          // synop-plot:iin liittyvä minmax-plot parametri
  kFmiSpSoundingPlot = 9997,        // synop-plot:iin liittyvä sounding-plot parametri
  kFmiSpMatrixDataDraw =
      9998  // isolineview-luokalla piirrettävän matriisi-dataan liittyvä piirto-parametri
};
}  // end of namespace NFmiInfoData
