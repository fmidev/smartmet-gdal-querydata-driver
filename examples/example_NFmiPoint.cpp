/*
 * © Ilmatieteenlaitos/Marko.
 * Original 10.12.2001
 * path: newbase/examples/example_NFmiPoint.cpp
 * 
 * Luo erilaisia NFmiPoint ilmentymiä ja manipuloi niitä ja tulostaa
 * olioiden arvoja cout:iin.
 * Aja tämä ohjelma komennolla: example_NFmiPoint.exe > example_NFmiPoint.txt
 * Jolloin voit verrata tuloksia newbase\examples\results\example_NFmiPoint.txt
 * tiedostossa oleviin.
 */
// ======================================================================

#include <iostream> // cout
#include "NFmiPoint.h"

using namespace std; // kerrotaan std:n käytöstä, ettei tarvitse kirjoittaa std::cout jne.
int main(void)
{
	NFmiPoint p;
	cout << p; // TULOS: 0 0

	p.Set(20, 30);
	cout << p; // TULOS: 20 30

	p.X(35);
	p.Y(45);
	cout << p; // TULOS: 35 45

	cout << p.X() << " " << p.Y() << endl;; // TULOS: 35 45

	NFmiPoint p2(2, 4); // luodaan toinen piste

	if(p == p2) // operator== -testi
		cout << "Jos menee tähän, homma ei toiminut oikein!!!!" << endl;
	else
		cout << "Pisteet p ja p2 olivat eri suuria, kuten pitikin." << endl;

	NFmiPoint p3;
	p3 = p + p2; // operator+= laskee pisteiden x ja y arvot yhteen
	cout << p3; // TULOS: 37 49

	double distance = p3.Distance(p);
	cout << "p3:n ja p:n välinen etäisyys oli " << distance << endl;

	FmiDirection dir = p3.DirectionOfDifference(p);
	cout << "p sijaitsee suhteessa p3:een kulmassa ";
	switch(dir)
	{
	case kUpLeft: cout << "UpLeft, Väärin!" << endl; break;
	case kUpRight: cout << "UpRight, Väärin!" << endl; break;
	case kLeftDown: cout << "LeftDown, Väärin!" << endl; break;
	case kRightDown: cout << "RightDown, Oikein." << endl; break;
	default: cout << "Ei sen tänne pitänyt mennä ollenkaan!!!!" << endl; break;
	}



	return 0;
}

// annakaisan testikommentti