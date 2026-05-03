// ======================================================================

#include "NFmiPoint.h"
#include "NFmiTimeZoneFinder.h"
#include <iostream>

using namespace std;

// Verify that no coordinate is inside two timezones
// If so, this will print info on the duplicate timezones

int main(void)
{
  NFmiTimeZoneFinder finder;
  if(!finder.ReadFile("timezones.txt"))
	{
	  cerr << "Error: Could not read timezones.txt" << endl;
	  return 1;
	}

  cout << "Verifying that Inside() works correctly:" << endl;

  const int nx = 500;
  const int ny = 500;

  const double dx = 360.0/(nx+1);
  const double dy = 180.0/(ny+1);

 
  int percent = 0;
  for(int i=1; i<nx; i++)
	{
	  for(int j=1; j<ny; j++)
		{
		  if(100.0*i*j/(nx*ny) > percent)
			{
			  cout << percent << "%" << endl;
			  percent += 1;
			}

		  NFmiPoint latlon(-180+i*dx,-90+j*dy);
		  finder.Check(latlon);
		}
	}
  
  return 0;
}
