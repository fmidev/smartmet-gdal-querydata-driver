// ======================================================================
/*!
 * \file NFmiCmdLine.cpp
 * \brief Implementation of class NFmiCmdLine
 */
// ======================================================================
/*!
 * \class NFmiCmdLine
 *
 * CmdLine is a class to ease the burden of command line processing.
 *
 * For example,
 *
 * \code
 *	#include <NFmiCmdLine.h>
 *	#include <logfile.h>
 *
 *	int main(int argc, char ** argv)
 *	{
 *    int ListLines=0;
 *	  int Clear=0;
 *	  char InputFile[132]="", OutputFile[132]="";
 *
 *	  // Create logfile for error logging
 *    NFmiLogFile thelogfile;
 *
 *	  // Create command line object; allow -l withvalue -c options
 *	  NFmiCmdLine thecmdline(argc, argv, "l:c");
 *
 *	  // Check for errors
 *	  if( thecmdline.Status().IsError() )
 *	  {
 *	    thelogfile << "ascii2lltbufr: Error on command line \n";
 *	    thelogfile << thecmdline.Status().ErrorLog();
 *	    return 1;
 *	  }
 *
 *	  // Process the command line options
 *	  if(thecmdline.isOption('l'))		// if -l was on the command line
 *	  {
 *      if( thecmdline.OptionValue('l') != NULL )	// If a value was associated
 *			ListLines=atoi(thecmdline.OptionValue('l');
 *		else
 *          ListLines = 10;			// List 10 lines by default
 *	  }
 *	  if(thecmdline.isOption('c')) Clear=1;	// Clear option was specified
 *
 *	  // Process the command line parameters
 *	  switch (thecmdline.NumberofParameters())
 *	  {
 *      case 2:
 *         strncpy(OutputFile,thecmdline.Parameter(2),131);
 *         break;
 *	    case 1:
 *         strncpy(InputFile,thecmdline.Parameter(1),131);
 *         break;
 *	    case 0:
 *         break;
 *	    default:
 *         thelogfile << "Usage: ascii2lltbufr [infile] [outfile]";
 *         return -1;
 *	  }
 *
 * \endcode
 *
 *
 *	The allowed command line format is as in the following example
 *
 * \code
 *	  command -l 34 -c filename1 filename2
 * \endcode
 *
 *  The options are single letters preceded by - sign. The options end
 *	and parameters start on the first item that is not preceded with -
 *	and which is not a value item for the preceding option letter.
 *
 *  The optionsallowed string in the constructor should be like
 *	"l:c", meaning that -l and -c options are allowed, -l can have a value
 *	associated with it but -c cant.
 *
 *  Note that for backward compatibility reasons, any word in the ARGV
 *  list is considered an option if the first character of the word is '-'.
 *  This means using a declaration such as "x:" it is impossible to use
 *  the command line
 *  \code
 *  -x 10
 *  \endcode
 *  To overcome this the class was later extended to understand the '!' directive
 *  as a substitute for the ':' directive. Using "x!" it is possible to
 *  use the -x option naturally.
 *
 *	You can use methods of the LastError base class to check for errors
 *	on the command line.
 *
 * \see NFmiLogFile and NFmiStatus
 *
 */
// ======================================================================

#include "NFmiCmdLine.h"
#include <macgyver/Exception.h>
#include <cstdio>
#include <cstring>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theString Undocumented
 * \param optallow Undocumented
 */
// ----------------------------------------------------------------------

NFmiCmdLine::NFmiCmdLine(const NFmiString &theString, const char *optallow)
    : itsStatus(),
      itsArgc(),
      itsArgv(),
      itsOptionsAllowed(),
      itsCommand(),
      itsOptionLetters(),
      itsOptionValues(),
      itsOptionCount(),
      itsParameterCount(),
      itsParameters()
{
  try
  {
    int argc = GetSpaceStringSize(theString);
    char **argv;

    NFmiString theStringBuffer(theString);

    argv = new char *[argc];

    NFmiString theValue;
    int k = 0;
    while (GetSpaceStringValue(theStringBuffer, theValue))
    {
      argv[k] = new char[theValue.GetLen() + 1];
      strcpy(argv[k++], theValue);
    }

    Init(argc, const_cast<const char **>(argv), optallow);

    for (int i = 0; i < argc; i++)
      delete argv[i];

    delete[] argv;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theString Undocumented
 * \param theValueString Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCmdLine::GetSpaceStringValue(NFmiString &theString, NFmiString &theValueString)
{
  try
  {
    theString.TrimL();

    if (!theString.GetLen())
      return false;

    int theSpaceIndex = theString.Search(NFmiString(" "));

    if (!theSpaceIndex)
      theSpaceIndex = theString.GetLen() + 1;

    NFmiString theStr1(theString.GetChars(1, theSpaceIndex - 1));
    NFmiString theStr2(theString.GetChars(theSpaceIndex, theString.GetLen() - theSpaceIndex + 1));

    theString = theStr2;
    theValueString = theStr1;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theString Undocumented, unused
 * \return Undocumented
 */
// ----------------------------------------------------------------------

int NFmiCmdLine::GetSpaceStringSize(const NFmiString & /* theString */)
{
  return 3;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param argc Number of arguments on the command line
 * \param argv Arguments on the command line
 * \param optallow Options allowed on the command line, "" if none allowed
 */
// ----------------------------------------------------------------------

NFmiCmdLine::NFmiCmdLine(int argc, const char **argv, const char *optallow)
    : itsStatus(),
      itsArgc(),
      itsArgv(),
      itsOptionsAllowed(),
      itsCommand(),
      itsOptionLetters(),
      itsOptionValues(),
      itsOptionCount(),
      itsParameterCount(),
      itsParameters()
{
  try
  {
    Init(argc, argv, optallow);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param argc Undocumented
 * \param argv Undocumented
 * \param optallow Undocumented
 */
// ----------------------------------------------------------------------

void NFmiCmdLine::Init(int argc, const char **argv, const char *optallow)
{
  try
  {
    const char *p;
    int i;
    itsParameterCount = 0;  // Init to 0, if illegal option, remains uninitialized
    itsOptionCount = 0;     // Init to 0, if illegal option, remains uninitialized
    itsArgc = argc;
    itsArgv = new char *[itsArgc];
    // To make it simple, prepare to have all of argvs
    // either options or parameters. Ususally there will be both
    itsOptionLetters = new char[itsArgc];
    itsOptionValues = new char *[itsArgc];
    itsParameters = new char *[itsArgc];
    // Copy the arguments to the safe object
    for (i = 0; i < itsArgc; i++)
    {
      itsArgv[i] = new char[strlen(argv[i]) + 1];
      strcpy(itsArgv[i], argv[i]);
      itsOptionLetters[i] = '?';
      itsOptionValues[i] = nullptr;
      itsParameters[i] = nullptr;
    }
    // Save optallow
    // If optallow is NULL, force it to "" (No options allowed)
    char nopt[1] = "";
    p = optallow;
    if (p == nullptr)
      p = nopt;
    itsOptionsAllowed = new char[strlen(p) + 1];
    strcpy(itsOptionsAllowed, p);
    // Save Command
    itsCommand = new char[strlen(argv[0]) + 1];
    strcpy(itsCommand, itsArgv[0]);

    // Dig Out & save option letters & values
    i = 1;
    itsOptionCount = 0;
    //  while (true)
    for (;;)  // Marko muutti erilaiseksi iki-luupiksi ja poisti varoituksen
    {
      if (i >= itsArgc)
        break;
      if (itsArgv[i][0] != '-')
        break;
      if (strlen(itsArgv[i]) == 1)
      {
        ++i;
        break;
      }

      if ((p = strchr(itsOptionsAllowed, itsArgv[i][1])) == nullptr)
      {
        char e[256];
#ifdef _MSC_VER
        ::_snprintf(e, sizeof(e) - 1, "NFmiCmdLine::NFmiCmdLine: Illegal option %s\n", itsArgv[i]);
#else
        ::snprintf(e, sizeof(e) - 1, "NFmiCmdLine::NFmiCmdLine: Illegal option %s\n", itsArgv[i]);
#endif
        e[sizeof(e) - 1] = 0;  // pitää varmistaa että päättyy 0-merkkiin!!!!
        itsStatus.ErrorLog(e);
        return;
      }
      itsOptionLetters[itsOptionCount] = *p;
      i++;
      const char mode = *(p + 1);

      if (mode == ':' || mode == '!')  // We allow a value associated to this option letter
      {
        if (i < itsArgc)  // If we still have something on the command line
        {
          if (mode == '!' || itsArgv[i][0] != '-')  // We do have a value for the previous option
          {
            itsOptionValues[itsOptionCount] = new char[strlen(itsArgv[i]) + 1];
            strcpy(itsOptionValues[itsOptionCount], itsArgv[i]);
            i++;
          }
        }
        else if (mode == '!')
        {
          NFmiString msg("NFmiCmdLine::Init: Argument for option ");
          msg += itsArgv[i - 1];
          msg += " missing";
          itsStatus.ErrorLog(msg.CharPtr());
          return;
        }
      }
      itsOptionCount++;
    }

    itsParameterCount = itsArgc - i;
    int j = 0;
    while (i < itsArgc)
    {
      itsParameters[j] = new char[strlen(argv[i]) + 1];
      strcpy(itsParameters[j++], itsArgv[i++]);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiCmdLine::~NFmiCmdLine()
{
  try
  {
    int i;
    for (i = 0; i < itsArgc; i++)
      delete[] itsArgv[i];
    delete[] itsArgv;

    delete[] itsOptionLetters;

    for (i = 0; i < itsOptionCount; i++)
      delete[] itsOptionValues[i];
    delete[] itsOptionValues;

    for (i = 0; i < itsParameterCount; i++)
      delete[] itsParameters[i];
    delete[] itsParameters;

    delete[] itsOptionsAllowed;

    delete[] itsCommand;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the number of options used on the command line
 *
 * \return The number of options
 */
// ----------------------------------------------------------------------

int NFmiCmdLine::NumberofOptions() const
{
  try
  {
    return itsOptionCount;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the number of parameters on the command line
 *
 * \return The number of parameters
 */
// ----------------------------------------------------------------------

int NFmiCmdLine::NumberofParameters() const
{
  try
  {
    return itsParameterCount;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the desired parameter from the command line. if the
 * index is smaller than 1 or greater than the number of parameters,
 * an empty string is returned.
 *
 * \param i The index of the parameter on the command line
 * \return The respective parameter
 */
// ----------------------------------------------------------------------

const char *NFmiCmdLine::Parameter(int i) const
{
  try
  {
    if (i > itsParameterCount || i < 1)
      return "";
    else
      return itsParameters[i - 1];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the program name used on the command line
 *
 * \return The program name
 */
// ----------------------------------------------------------------------

char *NFmiCmdLine::Command() const
{
  try
  {
    return itsCommand;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the desired option letter from the command line. If the
 * given index is out of range, an empty string is returned.
 *
 * \param i The index of the desired option
 * \return The option letter.
 */
// ----------------------------------------------------------------------

char NFmiCmdLine::OptionLetter(int i) const
{
  try
  {
    if (i > itsOptionCount || i < 1)
      return ' ';
    else
      return itsOptionLetters[i - 1];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the value associated with the ith option letter on the
 * command line, or an empty string if the given index is out of
 * bounds.
 *
 * \param i The index of the option letter
 * \return The respective option value
 */
// ----------------------------------------------------------------------

const char *NFmiCmdLine::OptionValue(int i) const
{
  try
  {
    if (i > itsOptionCount || i < 1)
      return "";
    else
      return itsOptionValues[i - 1];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Test if the given option letter was used on the command line
 *
 * \param c The option letter to search for
 * \return 1 if the option was used, 0 if not
 *
 * \todo Should return a boolean instead
 */
// ----------------------------------------------------------------------

int NFmiCmdLine::isOption(char c) const
{
  try
  {
    for (int i = 0; i < itsOptionCount; i++)
      if (itsOptionLetters[i] == c)
        return 1;

    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the value associated with the given option letter, or NULL
 * if the option letter was not used.
 *
 * \param c The option letter
 * \return The option value
 *
 * \bug One should not use NULL in C++, only 0
 */
// ----------------------------------------------------------------------

const char *NFmiCmdLine::OptionValue(char c) const
{
  try
  {
    int i;
    for (i = 0; i < itsOptionCount; i++)
      if (itsOptionLetters[i] == c)
        break;
    if (i <= itsOptionCount)
      return itsOptionValues[i];
    else
      return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Returns the NFmiStatus object internal to the command line object.
 * One can use the methods from the NFmiStatus object to check if
 * the command line was ok, and if not, what was wrong with it. Most
 * useful methods are NFmiStatus::IsError() to check if there was
 * an error, and NFmiStatus::ErrorLog() to get the associated
 * error message. For full details see the documentation for
 * class NFmiStatus.
 *
 * \return The NFmiStatus object
 */
// ----------------------------------------------------------------------

const NFmiStatus &NFmiCmdLine::Status() const
{
  try
  {
    return itsStatus;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ======================================================================
