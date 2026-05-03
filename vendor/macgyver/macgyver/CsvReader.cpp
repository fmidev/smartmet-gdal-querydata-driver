// ======================================================================
/*!
 * \brief CSV file reader
 */
// ======================================================================

#include "CsvReader.h"
#include "Exception.h"
#include "StringConversion.h"
#include <cctype>
#include <fstream>
#include <iostream>  // REMOVE
#include <stdexcept>

using namespace std;

namespace Fmi
{
namespace CsvReader
{
enum CsvState
{
  ExpectingRecord,  // new row should be starting
  ExpectingField,   // new field should be starting
  InsideField,      // inside field
  DoubleQuote       // last character was double quote
};

const char doublequote = '"';
const char comment = '#';

// test end of record characters
bool isnewline(int ch)
{
  return (ch == '\n' || ch == '\r');
}
// utility function for improved error messages
void myerror(const string& prefix, const string& filename, ifstream& file)
{
  try
  {
    string::size_type pos = static_cast<string::size_type>(file.tellg()) - 1;
    throw Fmi::Exception(BCP,
                         prefix + " in file '" + filename + "' at position " + Fmi::to_string(pos));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Read file in CSV format
 */
// ----------------------------------------------------------------------

void read(const string& filename, Callback callback, char delimiter)
{
  try
  {
    // Initial state
    using CsvRow = vector<string>;
    CsvRow row;
    CsvState state = ExpectingRecord;
    bool field_quoted = false;
    string field;

    ifstream input(filename.c_str());
    if (!input)
      throw Fmi::Exception(BCP, "Failed to open '" + filename + "' for reading");

    while (input.good())
    {
      int ch = input.get();
      // cout << "'" << static_cast<char>(ch) << "' " << row.size() << endl;

      switch (state)
      {
        case ExpectingRecord:
        {
          if (ch == comment)
          {
            string line;
            getline(input, line);
            break;
          }
        }
        // fall through
        case ExpectingField:
        {
          if (input.eof() || isnewline(ch))
          {
            // EOF is ok at this stage, while-loop will terminate.
            // If there was just a newline, the while-loop
            // will continue.

            if (state == ExpectingField)
              row.push_back(field);

            if (!row.empty())
            {
              callback(row);
              row.clear();
            }
            state = ExpectingRecord;
          }
          else if (ch == delimiter)
          {
            state = ExpectingField;
            row.push_back("");
          }
          else if (isspace(ch))
            ;  // ignore whitespace before field
          else if (ch == doublequote)
          {
            state = InsideField;
            field_quoted = true;
          }
          else
          {
            field += static_cast<char>(ch);
            state = InsideField;
            field_quoted = false;
          }
          break;
        }
        case InsideField:
        {
          // EOF implies end of unquoted field at this stage
          if (input.eof() || isnewline(ch))
          {
            if (field_quoted)
            {
              if (input.eof())
                myerror("Expecting double quote", filename, input);
              else
                field += static_cast<char>(ch);
            }
            else
            {
              Fmi::trim(field);
              row.push_back(field);
              callback(row);
              field.clear();
              row.clear();
              state = ExpectingRecord;
            }
          }
          else if (ch == delimiter)
          {
            if (field_quoted)
              field += static_cast<char>(ch);
            else
            {
              if (field_quoted)
                row.push_back(field);
              else
              {
                Fmi::trim(field);
                row.push_back(field);
              }
              field.clear();
              state = ExpectingField;
            }
          }
          else if (ch == doublequote)
            state = DoubleQuote;
          else
            field += static_cast<char>(ch);
          break;
        }
        case DoubleQuote:
        {
          if (input.eof() || isnewline(ch))
          {
            if (!field_quoted)
              myerror("Not expecting double quote", filename, input);

            row.push_back(field);
            callback(row);
            row.clear();
            field.clear();
            state = ExpectingRecord;
          }
          else if (ch == delimiter)
          {
            if (field_quoted)
            {
              row.push_back(field);
              field.clear();
              state = ExpectingField;
            }
            else
              myerror("Not expecting delimiter", filename, input);
          }
          else if (ch == doublequote)
          {
            field += doublequote;
            state = InsideField;
          }
          else
            myerror("Illegal character after double quote", filename, input);
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
}  // namespace CsvReader
}  // namespace Fmi

// ======================================================================
