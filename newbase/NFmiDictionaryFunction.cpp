
/*!
 * Tämä on Editorin käyttämä sana kirja funktio.
 * Kieli versiot stringeihin tulevat täältä.
 */

#include "NFmiDictionaryFunction.h"
#include <macgyver/Exception.h>

// HUOM! Tämä on kopio NFmiEditMapGeneralDataDoc-luokan metodista, kun en voinut antaa tänne
// dokumenttia
std::string GetDictionaryString(const char *theMagicWord)
{
  try
  {
    const std::string baseWords = "MetEditor::Dictionary::";

    std::string usedMagicWord(theMagicWord);
    NFmiStringTools::ReplaceAll(usedMagicWord, "\t", R"(\t)");  // pitää muuttaa mahdollinen
    // tabulaattori merkkisarjaksi, missä on
    // kenoviiva ja t peräkkäin, koska
    // tabulaattori on siten sanakirjassa
    std::string finalMagicWord(baseWords);
    finalMagicWord += usedMagicWord;
    std::string dictionarySentense =
        NFmiSettings::Optional<std::string>(finalMagicWord.c_str(), std::string(theMagicWord));
    NFmiStringTools::ReplaceAll(
        dictionarySentense,
        R"(\t)",
        "\t");  // pitää muuttaa mahdolliset "\\t"-merkkijonot takaisin tabulaattoreiksi
    return dictionarySentense;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
