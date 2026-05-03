// ======================================================================

#include "NFmiPoint.h"
#include "NFmiTimeZoneFinder.h"
#include <iostream>

using namespace std;

// Prints timezone for multiple coordinates.
// 32700 should not be included in the output, so grep -v 32700 should be empty

int main(void)
{
  NFmiTimeZoneFinder finder;
  if(!finder.ReadFile("timezones.txt"))
	{
	  cerr << "Error: Could not read timezones.txt" << endl;
	  return 1;
	}

  const int nx = 10;
  const int ny = 10;

  const double dx = 360.0/(nx+1);
  const double dy = 180.0/(ny+1);

  for(int i=1; i<nx; i++)
	{
	  for(int j=1; j<ny; j++)
		{
		  NFmiPoint latlon(-180+i*dx,-90+j*dy);
		  cout << finder.Find(latlon) << '\t'
			   << latlon.X() << '\t'
			   << latlon.Y() << endl;
		}
	}

  return 0;
}
