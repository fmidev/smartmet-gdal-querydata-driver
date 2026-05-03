
#include "NFmiMetMath.h"
#include <iomanip>
#include <iostream>

using namespace std;

int main(void)
{

  for(int mode=1; mode<=3; mode++)
	{
	  if(mode==1)
		cout << "UCAR taulukko kastepistelämpötilalle:" << endl << endl;
	  else if(mode==2)
		cout << "FMI taulukko kastepistelämpötilalle:" << endl << endl;
	  else
		cout << "UCAR-FMI taulukko:" << endl << endl;

	  for(int rh=10; rh<=100; rh+=10)
		cout << '\t' << rh;
	  cout << endl;

	  for(int t=-40; t<=40; t+=5)
		{
		  cout << t;
		  for(int rh=10; rh<=100; rh+=10)
			{
			  cout << '\t';
			  float ucar = FmiDewPointUCAR(t,rh);
			  float fmi = FmiDewPointFMI(t,rh);

			  float value = (mode==1 ? ucar : mode==2 ? fmi : ucar-fmi);
			  cout << setprecision(1) << fixed << value;
			}
		  cout << endl;
		}
	  cout << endl;
	}
}
