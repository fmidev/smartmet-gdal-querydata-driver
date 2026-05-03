// ======================================================================
// This works a dual purpose - demonstrate the use of NFmiSettingsImpl,
// and validate that it works properly.
// ======================================================================

#include "NFmiSettingsImpl.h"
#include "NFmiFileSystem.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(void)
{
  // Luodaan mallitiedosto, jossa on joitain asetuksia

  const string conffile = "test.conf";

  ofstream out(conffile.c_str());
  out << "# This is permitted" << endl
	  << "// This is permitted too" << endl
	  << "/* This is permitted too */" << endl;
  out << "a = ali" << endl
	  << "b = baba" << endl
	  << "ab = ali baba" << endl
	  << "tab \t = \t tabulator" << endl
	  << "alphabet = a b c d e f g" << endl;
  out.close();

  // Initialize
  NFmiSettingsImpl::Instance().Read(conffile);

  if(NFmiSettingsImpl::Instance().IsSet("a"))
	cout << "The variable a is set, which is correct" << endl
		 << "This also implies using searchpaths works properly" << endl;
  else
	cout << "The variable a is not set, which is INCORRECT!" << endl;

  cout << "a = "
	   << NFmiSettingsImpl::Instance().Value("a","BUG!")
	   << endl
	   << "b = "
	   << NFmiSettingsImpl::Instance().Value("b","BUG!")
	   << endl
	   << "ab= "
	   << NFmiSettingsImpl::Instance().Value("ab","(Not set, because = must be a separate word!)")
	   << endl
	   << "tab = "
	   << NFmiSettingsImpl::Instance().Value("tab","BUG!")
	   << endl
	   << "alphabet = "
	   << NFmiSettingsImpl::Instance().Value("alphabet","BUG!") << endl;

  NFmiFileSystem::RemoveFile(conffile);
  return 0;

}
