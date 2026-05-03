#include "NFmiVector.h"
#include <iostream>

using namespace std;
int main()
{

  NFmiVector<3> tmp(1,2,3);
  -tmp;
  cout << tmp[0] << ' ' << tmp[1] << ' ' << tmp[2] << endl;

  NFmiVector<3> blah(4,5,6);
  blah.Swap(tmp);
  cout << tmp[0] << ' ' << tmp[1] << ' ' << tmp[2] << endl;

}
