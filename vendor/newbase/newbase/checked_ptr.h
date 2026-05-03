/*!
 *
 *

> Olen etsimässä juuri editori kirjastoista mahdollisia pointteri virhe
> kaato mahdollisuuksia. Monissa luokissa mitä käytän on pointtereitä
> toisiin olioihin ja nämä pointterit alustetaan usein konstruktorissa.
> Niitä ei tarkasteta myöhemmin kun rakennettua oliota käytetään
> myöhemmin. Tuli vain mieleen, pitäisikö tehdä joku 0-pointteri
> poikkeusluokka (vaikka newbase:een), jota voisi heitellä eri luokkien
> konstruktoreista tälläisissä tapauksissa. Silloin tulisi tarkastettua
> pointterit ja ongelmat voisi voisi käsitellä ylempänä ja luokkia voisi
> käyttää turvallisemmin kun niiden dataosien pointterit olisi
> tarkastettu.

Miten olisi checked_ptr templaatti, joka olisi kuten auto_ptr, mutta
ei omistaisi dataa, ja nollapointterin dereferointi heittäisi aina
poikkeuksen? Parempi saattaisi olla smart_ptr, jolla olisi vastaava
policy, mutta se taitaa vaatia enemmän muutoksia kuin checked_ptr.

Ehkä voisi yrittää ensin siirtyä checked_ptr luokkaan, ja sitten siirtyä
asteittain smart_ptr luokkaan?

Käytin mallina g++:n auto_ptr luokkaa, ja yksinkertaistin hieman asioita,
alla on (kääntämätön) lopputulos.

  *ptr ja
  ptr->metodi

heittäisivät veivin, jos pointteri on nolla. get() kuitenkin toimisi ihan
normaalisti, sitä kautta saa testattua sisällön. Myöskin delete lakkaa
toimimasta, koska objekti ei olekaan enää pointteri, eli mahdolliset
deallokointi virheet löytyvät tätä kautta.
 */

#pragma once

#include <stdexcept>

struct checked_ptr_err : public std::runtime_error
{
  checked_ptr_err(const std::string& s) : std::runtime_error(s) {}
};

struct checked_ptr_deref_err : public checked_ptr_err
{
  checked_ptr_deref_err(const std::string& s) : checked_ptr_err(s) {}
};

struct checked_ptr_get_err : public checked_ptr_err
{
  checked_ptr_get_err(const std::string& s) : checked_ptr_err(s) {}
};

template <typename T>
class checked_ptr
{
 private:
  T* p;

 public:
  typedef T element_type;
  ~checked_ptr() {}
  explicit checked_ptr(element_type* ptr = 0) : p(ptr) {}
  checked_ptr(const checked_ptr& P) : p(P.p) {}
  checked_ptr& operator=(element_type* ptr)
  {
    p = ptr;
    return *this;
  }
  checked_ptr& operator=(const checked_ptr& P)
  {
    p = P.p;
    return *this;
  }
  element_type* get() { return p; }
  element_type& operator*() const
  {
    if (p)
      return *p;
    throw checked_ptr_deref_err("Trying to dereference a null checked_ptr");
  }

  element_type* operator->() const
  {
    if (p)
      return p;
    throw checked_ptr_get_err("Trying to get a null checked_ptr");
  }

  void reset(element_type* ptr = 0) { p = ptr; }
};
