// ======================================================================
/*!
 * \file NFmiSettingsImpl.cpp
 * \brief Definition of NFmiSettingsImpl methods and data
 */
// ======================================================================
/*!
 * \class NFmiSettingsImpl
 *
 * NFmiSettingsImpl is a singleton whose purpose is to read configuration files
 * and establish parameter - value correspondances which the user can query.
 * The main idea is to provide default paths, files and application specific
 * defaults via a configuration file instead of hard coding directly into
 * the source code. This is particularly important due to platform specific
 * changes as the routine environment keeps changing.
 *
 * By default NFmiSettingsImpl assumes one wishes to use the global platform
 * specific configuration file named \c smartmet.conf. This can be overridden
 * using the init method.
 *
 * Sample usage for the default configuration file:
 * \code
 * string querydata_path = NFmiSettingsImpl::Instance().Value("default_querydata_path");
 * string font_path = NFmiSettingsImpl::Instance().Value("default_font_path");
 * \endcode
 *
 * Sample usage for an alternative configuration file follows
 * \code
 * if(NFmiSettingsImpl::Instance().Init("application.conf"))
 * {
 *    string path = NFmiSettingsImpl::Instance().Value("default_path","my_alternative_path");
 *    if(NFmiSettingsImpl::Instance().IsSet("default_timeout"))
 *        settimeout(atof(NFmiSettingsImpl::Instance().Value("default_timeout")));
 *    ...
 * }
 * else
 *   throw std::runtime_error("Could not read configuration file blah blah");
 * \endcode
 * Automatic error checking can be accomplished with
 * \code
 * string value = NFmiSettingsImpl::Instance().Require("variablename");
 * \endcode
 * where an exception is thrown if the variable is not set.
 *
 * The user should understand the design choices made in this class.
 * For one, any global configuration setting can be made available via
 * global variables. However, as soon as this involves complicated behaviour,
 * such as reading a configuration file, one is in danger of losing the
 * encapsulation of low level details. The standard way to solve such
 * problems in object oriented environments is to use a singleton, whose
 * definition is: make sure a class has an unique instance, and provide
 * one single way to access it. See for example the book Design Patterns
 * for more details.
 *
 * What the definition of a singleton implies is that there must be
 * a single method to access the class, and in particular that using
 * any method of the class must occur via the first access method.
 * That is, most calls are of the form \c Singleton::Instance().Method()
 *
 * The main point is that one never actually creates on instance of the
 * singleton itself, in this case NFmiSettingsImpl, but instead calls the
 * instance method, which is static and is hence always accessible without
 * an actual instance.
 *
 * One is of course allowed to use shortcuts like
 * \code
 * NFmiSettingsImpl & settings = NFmiSettingsImpl::Instance();
 * settings.Method(blah,blah);
 * \endcode
 * to shorten code, but in general it is rarely necessary, as in the
 * case of global configuration variables one typically needs to access
 * the variables fairly rarely.
 *
 * Understanding the design choices made in using the singleton design
 * pattern for implementing NFmiSettingsImpl should greatly enhance the ease
 * of use of the class, otherwise the user will just be guessing why calling
 * the instance method is necessary everywhere.
 *
 * The read configuration file(s) are processed by NFmiPreProcessor.
 * This means the text is allowed to have all the common types of
 * comments. Also, if necessary, it is possible to add conditional
 * processing, for example to have different settings based on the
 * current date.
 *
 * It is also possible to insert new settings directory using
 * \code
 * NFmiSettingsImpl::Instance().Set(keyword,value);
 * \endcode
 * in case configuration files cannot be used as is.
 *
 * One may use the values of previously defined variables
 * in the definition of a new variable as follows
 * \code
 * maindir = /data
 * share = ${maindir}/share
 * prod = ${maindir}/prod
 * \endcode
 * The values will be expanded when accessed, not when defined.
 * Hence the order of the definitions may be significant.
 * When parenthesis are used, the definition is expanded as
 * soon as it is encountered:
 * \code
 * maindir = /data
 * maindir = $(maindir)/share
 * \endcode
 * Using braces the above would result in an infinite recursion,
 * but using parenthesis we get the expected result.
 *
 * It is customary to use namespace-like variable names
 * to avoid collisions between the global settings for different
 * programs. For example, one might define
 * \code
 * qdpoint::default_location = Helsinki
 * qdpoint::default_data = /data/pal/querydata/pal/skandinavia/pinta
 * qdpoint::default_coordinate_file = /smartmet/share/coordinates/all.txt
 * \endcode
 * In such cases it may be more convenient to rewrite the declaration
 * as
 * \code
 * qdpoint
 * {
 *    default_location = Helsinki
 *    default_data = /data/pal/querydata/pal/skandinavia/pinta
 *    default_coordinate_file = /smartmet/share/coordinates/all.txt
 * }
 * \endcode
 * NFmiSettingsImpl will understand the block structure to mean each
 * variable name is a namespace type specifier to be prepended
 * to all variable names withing the block that follows.
 * The namespaces may be nested arbitrarily.
 *
 * Finally, often different namespaces have many similar settings,
 * for example
 * \code
 * textgen::day1::settings
 * {
 *    start = 6
 *    end = 21
 *    precision = 10
 *    limit = 10
 * }
 * textgen::day2::settings
 * {
 *    start = 6
 *    end = 21
 *    precision = 10
 *    limit = 20
 * }
 * \endcode
 * This can be accomplished more cleanly with
 * \code
 * textgen::dayx::settings
 * {
 *    start = 6
 *    end = 21
 *    precision = 10
 * }
 * textgen::day1::settings
 * {
 *    use textgen::dayx::settings
 *    limit = 10
 * }
 * textgen::day2::settings
 * {
 *    use textgen::dayx::settings
 *    limit = 10
 * }
 * \endcode
 * The benefits become more clear once there are more common settings.
 *
 */
// ======================================================================
/*!
 * \example example_NFmiSettingsImpl.cpp
 * An example of using NFmiSettingsImpl. The program creates a dummy
 * configuration file, and verifies that it is parsed correctly.
 */
// ======================================================================

#include "NFmiSettingsImpl.h"
#include "NFmiFileSystem.h"
#include "NFmiPreProcessor.h"
#include "NFmiStringTools.h"
#include <macgyver/Exception.h>

#include <boost/algorithm/string/trim.hpp>

#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>

#define DEFAULT_FILE "smartmet.conf"

using namespace std;

// Static variable initializations
NFmiSettingsImpl* NFmiSettingsImpl::itsInstance = nullptr;
bool NFmiSettingsImpl::itIsDestroyed = false;

// Hidden internal methods and variables
namespace
{
//! Read the given filename

string readfile(const std::string& filename)
{
  try
  {
    ifstream in(filename.c_str(), std::ios::binary);
    if (!in)
      throw Fmi::Exception(BCP, "Failed to open " + filename + " for reading");

    return NFmiStringTools::ReadFile(in);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//! Save new contents to given filename
void savefile(const std::string& filename, const std::string& contents)
{
  try
  {
    NFmiFileSystem::SafeFileSave(filename, contents);
    /*
            ofstream out(filename.c_str(), ios::out);
            if(!out)
              throw Fmi::Exception(BCP,"Failed to open "+filename+" for writing");
            out << contents;
    */
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//! Perform string replacements in settings

void replace_assignment(string& contents, const string& theVariableName, const string& theNewValue)
{
  try
  {
    istringstream input(contents);
    string::size_type pos1 = 0;
    for (;;)
    {
      pos1 = contents.find(theVariableName, pos1);
      if (pos1 == string::npos)
        throw Fmi::Exception(BCP, "Unable to save new value for variable " + theVariableName);

      pos1 += theVariableName.size();
      input.seekg(pos1);

      string token;
      input >> token;
      if (token == "=")
      {
        getline(input, token);
        const string::size_type pos2 = input.tellg();

        {
          contents.replace(pos1, pos2 - pos1, " = " + theNewValue + '\n');
        }
        return;
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

//! Return the completed filename from the search path, or empty string
string findfile(const std::string& filename, const std::list<std::string>& search_path)
{
  try
  {
    if (NFmiFileSystem::FileExists(filename))
      return (filename);

    const list<string>::const_iterator begin = search_path.begin();
    const list<string>::const_iterator end = search_path.end();
    for (list<string>::const_iterator iter = begin; iter != end; ++iter)
    {
      string trialname = *iter + '/' + filename;
      if (NFmiFileSystem::FileExists(trialname))
        return trialname;
    }
    return string("");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Expand variable value
 *
 * All occurrances of ${variablename} will be expanded
 * using the given map of name-value pairs. If the variable
 * is not defined in the map, an exception is thrown.
 *
 * To protect against infinite recursion, a maximum length
 * of 10000 characters is set, also a maximum of 100
 * expansions will be done. Once the limits have been reached,
 * an exception is thrown.
 *
 * \param value The value to expand
 * \param data The data to expand with
 * \param delim1 Start delimiter, for example ${ or $(
 * \param delim2 End delimiter, for example } or )
 * \param namespaces List of namespaces in use
 * \return The expanded value
 */
// ----------------------------------------------------------------------

std::string expand(const std::string& value,
                   const std::map<std::string, std::string>& data,
                   const std::string& delim1,
                   const std::string& delim2,
                   const std::list<std::string>& namespaces)
{
  try
  {
    string ret = value;

    const unsigned long maxlength = 10000;
    const unsigned long maxexpansions = 100;

    unsigned long expansions = 0;
    while (value.size() < maxlength && expansions < maxexpansions)
    {
      string::size_type pos1 = ret.find(delim1);
      if (pos1 == string::npos)
        break;
      string::size_type pos2 = ret.find(delim2, pos1);
      if (pos2 == string::npos)
        throw Fmi::Exception(BCP, "Missing " + delim2 + " while expanding value " + value);
      const string name = ret.substr(pos1 + delim1.size(), pos2 - pos1 - delim2.size() - 1);

      auto it = data.find(name);
      if (it == data.end())
      {
        for (const auto& iter : namespaces)
        {
          const string newname = iter + "::" + name;
          it = data.find(newname);
          if (it != data.end())
            break;
        }
      }
      if (it == data.end())
        throw Fmi::Exception(
            BCP, "Cannot expand " + value + " since variable " + name + " has no value");

      ret.replace(pos1, pos2 + 1 - pos1, it->second);
    }

    if (value.size() >= maxlength)
      throw Fmi::Exception(BCP, "Result of expanding value " + value + " is too long");

    if (expansions >= maxexpansions)
      throw Fmi::Exception(BCP, "Too many expansions in value " + value + " (possible recursion)");

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

// ----------------------------------------------------------------------
/*!
 * The constructor of the NFmiSettingsImpl class is used privately
 * by the static create method. It initializes all internal
 * variables to the state of first initialization.
 */
// ----------------------------------------------------------------------

NFmiSettingsImpl::NFmiSettingsImpl()
    : itsFilename(DEFAULT_FILE), itsData(), itIsInitialized(false), itsNamespaces()
{
  try
  {
#ifdef UNIX
    itsSearchPath.emplace_back("/smartmet/cnf");
    itsSearchPath.emplace_back("/fmi/conf");
    itsSearchPath.emplace_back("/var/www/share/conf");
    itsSearchPath.emplace_back(".");

#else
    itsSearchPath.push_back("conf");
#endif
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * The destructor of the NFmiSettingsImpl class will make sure that
 * any subsequent destructor call will be detected. \see instance
 * and \see create for more details.
 */
// ----------------------------------------------------------------------

NFmiSettingsImpl::~NFmiSettingsImpl()
{
  try
  {
    itsInstance = nullptr;  // the pointer is address of static, not by new()
    itIsDestroyed = true;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Return an instance of the NFmiSettingsImpl. This is the only means
 * of access to the singleton, calling any other method without
 * using instance()->method() is bound to fail.
 *
 * The instance method handles creating the sole instance of the
 * object and detects the dead reference problem by throwing
 * a runtime_error.
 */
// ----------------------------------------------------------------------

NFmiSettingsImpl& NFmiSettingsImpl::Instance()
{
  try
  {
    if (!itsInstance)
    {
      if (itIsDestroyed)
        Die();
      else
        Create();
    }
    return *itsInstance;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * The Meyers technique of creating a singleton involves using
 * a static variable in a method, which is thus guaranteed to
 * be initialized when the method is called. The instance thus
 * created is stored by its address for global use later on.
 * The instance is automatically destroyed by C++ at exit, since
 * the variable is static.
 */
// ----------------------------------------------------------------------

void NFmiSettingsImpl::Create()
{
  try
  {
    static NFmiSettingsImpl theSettings;
    NFmiSettingsImpl::itsInstance = &theSettings;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * If two singletons use each other, we may encounter a dead reference
 * problem if one singleton is destroyed before the other (at exit).
 * This is the function called when the instance method detects such a case.
 * At the moment the function merely throws an error with the message
 * "NFmiSettingsImpl detected a dead reference problem".
 */
// ----------------------------------------------------------------------

void NFmiSettingsImpl::Die()
{
  try
  {
    throw Fmi::Exception(BCP, "NFmiSettingsImpl detected a dead reference problem");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Clear the settings
 */
// ----------------------------------------------------------------------

void NFmiSettingsImpl::Clear()
{
  try
  {
    itsFilename = DEFAULT_FILE;
    itsData.clear();
    itsNamespaces.clear();
    itsExpandedVariables.clear();
    itsChangedVariables.clear();
    itsFilenames.clear();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Test whether the given variable is set. Also makes sure the
 * settings have been initialized.
 */
// ----------------------------------------------------------------------

bool NFmiSettingsImpl::IsSet(const std::string& theName) const
{
  try
  {
    if (!itIsInitialized)
      Init();

    return (itsData.find(theName) != itsData.end());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Return the value, or if it is not set the given alternate value.
 */
// ----------------------------------------------------------------------

const std::string NFmiSettingsImpl::Value(const std::string& theName,
                                          const std::string& theAlternate) const
{
  try
  {
    if (!itIsInitialized)
      Init();

    DataType::const_iterator pos = itsData.find(theName);
    if (pos == itsData.end())
      return theAlternate;
    else
      return expand(pos->second, itsData, "${", "}", itsNamespaces);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Return the value, or if it is not set throw an exception
 */
// ----------------------------------------------------------------------

const std::string NFmiSettingsImpl::Require(const std::string& theName) const
{
  try
  {
    if (!itIsInitialized)
      Init();

    DataType::const_iterator pos = itsData.find(theName);
    if (pos == itsData.end())
      throw Fmi::Exception(BCP, "The variable " + theName + " is required to have a value");

    return expand(pos->second, itsData, "${", "}", itsNamespaces);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new setting
 */
// ----------------------------------------------------------------------

void NFmiSettingsImpl::Set(const std::string& theName,
                           const std::string& theValue,
                           bool modifyOnlyExisting) const
{
  try
  {
    if (itsExpandedVariables.find(theName) != itsExpandedVariables.end())
      throw Fmi::Exception(
          BCP, "Cannot reset variable " + theName + " value, it must be edited by hand");

    itIsInitialized = true;

    const list<string> nonamespaces;
    const string expanded = expand(theValue, itsData, "$(", ")", nonamespaces);

    auto foundIter = itsData.find(theName);
    if ((modifyOnlyExisting && foundIter == itsData.end()) ||
        (foundIter != itsData.end() && theValue == foundIter->second))
      return;  // Jos asetettavaa arvoa ei löydy asetuksista TAI sen arvo ei muutu, ei tehdä mitään

    pair<DataType::iterator, bool> result = itsData.insert(make_pair(theName, expanded));

    if (!result.second)
    {
      if (expanded != theValue)
        throw Fmi::Exception(BCP, "Cannot modify variable " + theName + " with expanding value");
      else
        result.first->second = expanded;
    }

    itsChangedVariables.insert(theName);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a new setting
 */
// ----------------------------------------------------------------------

void NFmiSettingsImpl::InternalSet(const std::string& theName, const std::string& theValue) const
{
  try
  {
    itIsInitialized = true;

    const string expanded = expand(theValue, itsData, "$(", ")", itsNamespaces);

    pair<DataType::iterator, bool> result = itsData.insert(make_pair(theName, expanded));

    if (!result.second)
      result.first->second = expanded;

    if (expanded != theValue)
      itsExpandedVariables.insert(theName);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Append a new path to the list of search paths.
 */
// ----------------------------------------------------------------------

#undef SearchPath  // win32 macro pitää undefinata
void NFmiSettingsImpl::SearchPath(const std::string& thePath) const
{
  try
  {
    itsSearchPath.push_back(thePath);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Initialize the settings with the given filename. If the name is
 * empty, itsFilename is not changed and is used instead. Returns
 * false on failure, and sets itIsInitialized accordingly. The
 * initialization should only fail if the configuration file
 * does not exist.
 */
// ----------------------------------------------------------------------

bool NFmiSettingsImpl::Init(const std::string& theFilename) const
{
  try
  {
    if (!theFilename.empty())
      itsFilename = theFilename;

    itsData.clear();
    itIsInitialized = Read(itsFilename);
    return itIsInitialized;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read the given filename. Returns false on failure. The algorithm
 * is to first strip all comments. Then all lines are split into
 * distinct words, of which the second one is expected to be '='.
 * The words from the third onwards are joined with spaces in between,
 * and the result is assigned to the name specified by the first word
 * on the line.
 *
 * \todo Improve the parser so that = does not have to be a separate word.
 */
// ----------------------------------------------------------------------

bool NFmiSettingsImpl::Read(const std::string& theFilename) const
{
  try
  {
    // Safety against an empty name
    if (theFilename.empty())
      return false;

    // Find the file from the search path
    string filename = findfile(theFilename, itsSearchPath);
    if (filename.empty())
      return false;

    // Process the file

    const bool strip_pound = true;
#ifdef UNIX
    // AKa 21-Dec-2007: Disallow '/* ... */' comments on Linux:
    //      subpath filters can contain '/*'.
    //
    //      Note: Also '//' comments could be disallowed, since '//' can occur
    //            in Linux path meaning single '/' (i.e. if automatically crafted)
    //
    NFmiPreProcessor processor(strip_pound,
                               true,  // "//" (double slash)
                               false,
                               false);  // "/*" and nested
    processor.SetIncluding("include", "", "");
    processor.SetDefine("#define");
#else
    NFmiPreProcessor processor(strip_pound);
#endif
    if (!processor.ReadAndStripFile(filename))
      return false;

    // Extract the assignments
    string text = processor.GetString();
    istringstream input(text);

    unsigned long words;
    string var;
    while (true)
    {
      // Extract variable name by seeking the next non variable name token
      size_t startpos = input.tellg();

      string token;
      for (words = 0; true; ++words)
      {
        input >> token;

        if (!input.good())
          break;

        if (token == "}" || token == "{" || token == "=" || token == "+=")
          break;
        if (words == 0 && token == "use")
          break;
        if (words == 0)
          var = token;
      }

      if (!input.good())
      {
        if (words == 0)
          break;
        else
          throw Fmi::Exception(BCP, "Error parsing the end of the configuration file");
      }

      // Extract name containing multiple words
      if (words > 1)
      {
        size_t endpos = input.tellg();
        size_t sz = static_cast<long>(endpos) - startpos - token.size();

        input.seekg(startpos, ios::beg);
        std::vector<char> buffer(sz + 1);
        input.read(&buffer[0], sz);
        buffer[sz] = '\0';
        input.seekg(endpos, ios::beg);

        var = &buffer[0];
        boost::algorithm::trim(var);

        if (var.find('\n') != string::npos)
          throw Fmi::Exception(BCP, "Variable name '" + var + "' contains a newline");
      }

      if (token == "}")
      {
        if (words > 0)
          throw Fmi::Exception(BCP, "Name space ended after variable name '" + var + "'");

        if (itsNamespaces.empty())
          throw Fmi::Exception(BCP, "Too many }'s in the settings");
        itsNamespaces.pop_front();
      }
      else if (token == "use")
      {
        if (itsNamespaces.empty())
          throw Fmi::Exception(BCP, "Cannot use 'use' outside namespaces");

        // Copy all variables from the given namespace to this one
        string nspace;
        input >> nspace;

        for (DataType::const_iterator it = itsData.begin(); it != itsData.end(); ++it)
        {
          if (it->first.substr(0, nspace.size()) == nspace)
          {
            string suffix = it->first.substr(nspace.size());
            if (!suffix.empty())
            {
              string newvar = itsNamespaces.front() + suffix;
              InternalSet(newvar, it->second);

              pair<FileMap::iterator, bool> result =
                  itsFilenames.insert(make_pair(newvar, filename));
              if (!result.second)
                result.first->second = filename;
              itsChangedVariables.erase(newvar);
            }
          }
        }
      }
      else if (token == "=" || token == "+=")
      {
        if (words == 0)
          throw Fmi::Exception(BCP, "Assignment without variable name encountered");

        string line;
        getline(input, line);

        istringstream lineinput(line);
        string val, word;
        while (lineinput >> word)
        {
          if (!val.empty())
            val += ' ';
          val += word;
        }

        string fullvarname = (itsNamespaces.empty() ? var : itsNamespaces.front() + "::" + var);

        if (token == "=")
          InternalSet(fullvarname, val);
        else
        {
          string tmpval = "$(" + var + ")," + val;
          InternalSet(fullvarname, tmpval);
        }

        // save the origin of the variable, possibly overriding any old value
        pair<FileMap::iterator, bool> result =
            itsFilenames.insert(make_pair(fullvarname, filename));
        if (!result.second)
          result.first->second = filename;

        // The value is now from file, and hence does not have a changed value
        itsChangedVariables.erase(fullvarname);
      }
      else if (token == "{")
      {
        if (words == 0)
          throw Fmi::Exception(BCP, "Must start namespace after a variable name");

        if (itsNamespaces.empty())
          itsNamespaces.push_front(var);
        else
          itsNamespaces.push_front(itsNamespaces.front() + "::" + var);
      }
      else
        throw Fmi::Exception(BCP,
                             "Expecting = or { after variable name '" + var +
                                 "' in the settings file, not token '" + token + "'");
    }

    itIsInitialized = true;

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Save the settings
 */
// ----------------------------------------------------------------------

void NFmiSettingsImpl::Save() const
{
  try
  {
    // nothing to do if no variables have been saved
    if (itsChangedVariables.empty())
      return;

    // collect all files to be modified

    std::set<string> modified_files;
    for (const auto& itsChangedVariable : itsChangedVariables)
    {
      auto foundIter = itsFilenames.find(itsChangedVariable);
      if (foundIter != itsFilenames.end())
        modified_files.insert(foundIter->second);  // laitetaan vain niiden tiedostojen nimet
                                                   // mukaan, missä oli muuttuneita muuttujia
    }
    /*
      std::set<string> modified_files;
      for(FileMap::const_iterator kt=itsFilenames.begin(); kt!=itsFilenames.end(); ++kt)
            modified_files.insert(kt->second);
    */

    // then process the files one by one
    for (const auto& modified_file : modified_files)
    {
      // collect all modified variables in this file
      std::set<string> modified_variables;
      for (FileMap::const_iterator jt = itsFilenames.begin(); jt != itsFilenames.end(); ++jt)
        if (modified_file == jt->second)
          if (itsChangedVariables.find(jt->first) != itsChangedVariables.end())
            modified_variables.insert(jt->first);

      // read old contents
      string contents = readfile(modified_file);

      // perform replacements
      for (const auto& modified_variable : modified_variables)
        replace_assignment(contents, modified_variable, Value(modified_variable));

      // save new contents
      savefile(modified_file, contents);
    }

    // indicate that all variables now have unchanged state compared to file
    itsChangedVariables.clear();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return all settings as a string suitable for saving and printing
 */
// ----------------------------------------------------------------------

string NFmiSettingsImpl::ToString(const string& thePrefix) const
{
  try
  {
    ostringstream out;
    for (DataType::const_iterator it = itsData.begin(); it != itsData.end(); ++it)
    {
      if (thePrefix.empty() || it->first.compare(0, thePrefix.size(), thePrefix) == 0)
      {
        out << it->first << " = " << it->second << endl;
      }
    }
    return out.str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return all child keys that fit to given prefix e.g. if there is
 * Q2Module::HelpData::Dynamic::goes_ch1::FilenameFilter and
 * Q2Module::HelpData::Dynamic::sounding::FilenameFilter -keys
 * and you ask
 * list = settings.ListChildren("Q2Module::HelpData::Dynamic");
 * returned list would contain "goes_ch1" and "sounding" values
 * So it will list all the next level (from prefix) of namespace values (individual list).
 */
// ----------------------------------------------------------------------

std::vector<std::string> NFmiSettingsImpl::ListChildren(const std::string& thePrefix) const
{
  try
  {
    set<string> strs;  // this eliminates duplicate values
    for (DataType::const_iterator it = itsData.begin(); it != itsData.end(); ++it)
    {
      if (thePrefix.empty() || it->first.compare(0, thePrefix.size(), thePrefix) == 0)
      {
        string tmp(it->first.begin() + thePrefix.size(), it->first.end());
        NFmiStringTools::TrimL(tmp, ':');
        std::string::size_type idx = tmp.find(':');
        string tmp2 = tmp;
        if (idx != std::string::npos)
          tmp2 = std::string(tmp.begin(), tmp.begin() + idx);
        strs.insert(tmp2);
      }
    }

    return std::vector<std::string>(strs.begin(), strs.end());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
