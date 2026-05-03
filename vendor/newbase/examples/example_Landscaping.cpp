#include "NFmiFastQueryInfo.h"
#include "NFmiQueryData.h"
#include <iostream>

int main()
{
  NFmiQueryData qd("/smartmet/data/ecmwf/maailma/pinta/querydata");
  NFmiFastQueryInfo qi(&qd);

  NFmiPoint p(-7.932,37.020);

  qi.Param(kFmiTemperature);

  qi.First();
  for(qi.ResetTime(); qi.NextTime(); )
	{
	  float t2m = qi.InterpolatedValue(p);
	  float tfix = qi.LandscapeInterpolatedValue(15,false,p,qi.ValidTime());
	  std::cout << qi.ValidTime().ToStr(kYYYYMMDDHH).CharPtr() << "\t= " << t2m << "\t" << tfix << "\t" << tfix-t2m << std::endl;
	}
  return 0;
}
