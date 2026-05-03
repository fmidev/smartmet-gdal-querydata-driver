// ======================================================================
// NEWBASE ver104
// REV 24112001 Mika Heiskanen
// ======================================================================

//**********************************************************
// C++ Class Name : NFmiPtrList
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/ver104/Src/nmetedit/include/NFmiPtrList.h
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_TemplateClass
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : ptrlist
//  - GD View Type      : Class Diagram
//  - GD View Name      : Class Diagram
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Wed - Jan 27, 1999
//
//
//  Description:
//   Template Lista-luokka, johon talletetaan
//   olioiden osoittimia. Oliot
//   luodaan dynaamisesti ulkopuolella ja annetaan
//   listalle. Listan voi
//   laittaa tarvittaessa tuhoamaan oliot Clear-metodilla
//   jos parametriksi
//   annetaan true.
//   Lista on kahteensuuntaan linkitetty ja siitä
//   voi periä järjestetyn listan.
//   Lista määrittelee luokan sisäisesti luokat
//   Node ja Iterator, joista luokkaa
//   Iterator käytetään iteraattorina eli sen
//   avulla voidaan listaa käydä läpi.
//   Listassa on aina erikseen alku ja loppu node:t,
//   joilla ei ole dataa, eli
//   listan start nodesta pitää edetä kerran ennenkuin
//   dataa on käytössä.
//
//  Change Log:
// Changed 1999.08.31/Marko	Lisäsin listaan uuden metodin Find().
//
//**********************************************************
#pragma once
#include "NFmiGlobals.h"

template <class Type>
class NFmiPtrList
{
 private:
  class Node
  {
   public:
    Node(Type *theData = 0, Node *thePrevious = 0, Node *theNext = 0)
        : itsData(theData), itsPrevious(thePrevious), itsNext(theNext){};
    void SwapData(Node &theOther)
    {
      Type *tmp = itsData;
      itsData = theOther.itsData;
      theOther.itsData = tmp;
    }
    Type *itsData;
    Node *itsPrevious;
    Node *itsNext;

   private:
    Node(const Node &theNode);
    Node &operator=(const Node &theNode);
  };  // Node
  Node *itsStart;
  Node *itsEnd;
  unsigned long itsCounter;  // Lisäsin nämä 8.9.99 /Mikael

 public:
  class Iterator
  {
   private:
    NFmiPtrList<Type> *itsList;  // Lisäsin tämän 8.9.99 /Mikael
    Node *itsCurrent;

   public:
    friend class NFmiPtrList<Type>;

    Iterator() : itsList(0), itsCurrent(0) {}
    // Lisäsin seuraavaan iteraattoriin toisen parametrin 8.9.99 /Mikael
    Iterator(Node *theNode, NFmiPtrList<Type> *theList) : itsList(theList), itsCurrent(theNode){};
    Iterator(const Iterator &theIter) : itsList(theIter.itsList), itsCurrent(theIter.itsCurrent){};

    Iterator &operator=(const Iterator &theIter)
    {
      itsList = theIter.itsList;
      itsCurrent = theIter.itsCurrent;
      return *this;
    }
    Node *GetNode() { return itsCurrent; }
    bool Next()  // miten koodin saisi headerin hännille?
    {
      if (itsCurrent && itsCurrent->itsNext)
      {
        itsCurrent = itsCurrent->itsNext;
        if (itsCurrent->itsNext)  // tämä tarkistus on 'dummy end' -noden takia
          return true;
      }
      return false;
    };

    bool Previous()  // miten koodin saisi headerin hännille?
    {
      if (itsCurrent && itsCurrent->itsPrevious)
      {
        itsCurrent = itsCurrent->itsPrevious;
        if (itsCurrent->itsPrevious)  // tämä tarkistus on 'dummy end' -noden takia
          return true;
      }
      return false;
    };

    Type *CurrentPtr() { return itsCurrent->itsData; };
    Type &Current() { return *itsCurrent->itsData; };
    bool AddBefore(Type *theItem)
    {
      if (!theItem)  // nolla-pointteria ei voi viedä listaan
        return false;
      Node *previousNode = itsCurrent->itsPrevious;
      if (previousNode)
      {
        Node *temp = new Node(theItem, previousNode, itsCurrent);
        itsCurrent->itsPrevious = temp;
        previousNode->itsNext = temp;
      }
      else
      {  // somebody is using this method at start-node! so we create new first node here
        Node *nextNode = itsCurrent->itsNext;
        Node *temp = new Node(theItem, itsCurrent, nextNode);
        itsCurrent->itsNext = temp;
        nextNode->itsPrevious = temp;
      }
      itsList->itsCounter++;  // Lisäsin 8.9.99 Mikael
      return true;
    };

    bool AddAfter(Type *theItem)
    {
      if (!theItem)  // nolla-pointteria ei voi viedä listaan
        return false;
      Node *nextNode = itsCurrent->itsNext;
      if (nextNode)
      {
        Node *temp = new Node(theItem, itsCurrent, nextNode);
        itsCurrent->itsNext = temp;
        nextNode->itsPrevious = temp;
      }
      else
      {  // somebody is using this method at end-node! So we create new last node here.
        Node *previousNode = itsCurrent->itsPrevious;
        Node *temp = new Node(theItem, previousNode, itsCurrent);
        itsCurrent->itsPrevious = temp;
        previousNode->itsNext = temp;
      }
      itsList->itsCounter++;  // Lisäsin 8.9.99 Mikael
      return true;
    };

    bool Remove(bool fDeleteData)  // poiston jälkeen current tulee olemaan previous-node
    {
      if (itsCurrent->itsPrevious && itsCurrent->itsNext)
      {
        Node *previousNode = itsCurrent->itsPrevious;
        Node *nextNode = itsCurrent->itsNext;
        previousNode->itsNext = nextNode;
        nextNode->itsPrevious = previousNode;
        if (fDeleteData)
          delete itsCurrent->itsData;
        delete itsCurrent;
        itsCurrent = previousNode;
        itsList->itsCounter--;  // Lisäsin 8.9.99 Mikael
        return true;
      }
      return false;  // currentti oli start tai end!
    }

  };                      // Iterator
  friend class Iterator;  // Lisäsin tämän ja kopiokonstruktorin 9.9.99 /Mikael
  //	friend class NFmiPtrList<Type>::Iterator;
  NFmiPtrList();
  NFmiPtrList(NFmiPtrList<Type> &theList);

  ~NFmiPtrList();
  void Clear(bool fDeleteItem = false);
  Iterator Start();
  Iterator End();
  bool AddStart(Type *theItem);
  bool AddEnd(Type *theItem);
  bool RemoveStart(bool fDeleteItem = false);
  bool RemoveEnd(bool fDeleteItem = false);
  bool Remove(unsigned long index, bool fDeleteItem = false);
  unsigned long NumberOfItems();
  Iterator Index(unsigned long index);
  bool Add(Type *theItem, unsigned long index);
  bool Swap(unsigned long ind1, unsigned long ind2);
  bool Sort(bool fAscendingOrder = true);
  bool Find(const Type &theValue, Iterator &theFoundItemIterator);

 protected:
  bool Swap(Iterator &iter1, Iterator &iter2);

 private:
  NFmiPtrList &operator=(const NFmiPtrList &theList);
};

/* // En tiedä oikeaa syntaksia, että Iterator-koodi saataisiin tänne!
template <class Type>
inline
bool NFmiPtrList<Type>::Iterator::Next()
{
        if(current->next)
        {
                current = current->next;
                if(current->next) // jos ollaan 'dummy end' -nodessa, palautetaan false
                        return true;
        }
        return false;
}*/

template <class Type>
NFmiPtrList<Type>::NFmiPtrList() : itsStart(new Node()), itsEnd(new Node()), itsCounter(0)
{
  itsStart->itsNext = itsEnd;
  itsEnd->itsPrevious = itsStart;
}

template <class Type>
NFmiPtrList<Type>::NFmiPtrList(NFmiPtrList<Type> &theList)
{
  itsStart = new Node;
  itsEnd = new Node;
  itsStart->itsNext = itsEnd;
  itsEnd->itsPrevious = itsStart;
  itsCounter = 0;
  Iterator iter = theList.Start();

  for (; iter.Next();)
    AddEnd(iter.CurrentPtr());
}

template <class Type>
NFmiPtrList<Type>::~NFmiPtrList()
{
  Clear(false);
  delete itsStart;
  delete itsEnd;
}

template <class Type>
void NFmiPtrList<Type>::Clear(bool fDeleteItem)
{
  Node *temp = itsStart->itsNext;  // temp is here the first node in the list, may be the end node
  while (temp->itsNext)            // if temp was the end node, the next loop will be skipped
  {
    temp = temp->itsNext;
    if (fDeleteItem)
      delete temp->itsPrevious->itsData;
    delete temp->itsPrevious;
  }
  itsStart->itsNext = itsEnd;
  itsEnd->itsPrevious = itsStart;
  itsCounter = 0;  // Marko 10.9.99
}

template <class Type>
typename NFmiPtrList<Type>::Iterator NFmiPtrList<Type>::Start()
{
  return Iterator(itsStart, this);
}

template <class Type>
typename NFmiPtrList<Type>::Iterator NFmiPtrList<Type>::End()
{
  return Iterator(itsEnd, this);
}

template <class Type>
bool NFmiPtrList<Type>::AddStart(Type *theItem)
{
  if (!theItem)  // nolla-pointteria ei voi viedä listaan
    return false;
  Node *firstRealNode = itsStart->itsNext;
  Node *newNode = new Node(theItem, itsStart, firstRealNode);
  if (!newNode)
    return false;
  itsStart->itsNext = newNode;
  firstRealNode->itsPrevious = newNode;
  itsCounter++;
  return true;
}

template <class Type>
bool NFmiPtrList<Type>::AddEnd(Type *theItem)
{
  if (!theItem)  // nolla-pointteria ei voi viedä listaan
    return false;
  Node *lastRealNode = itsEnd->itsPrevious;
  Node *newNode = new Node(theItem, lastRealNode, itsEnd);
  if (!newNode)
    return false;
  itsEnd->itsPrevious = newNode;
  lastRealNode->itsNext = newNode;
  itsCounter++;
  return true;
}

template <class Type>
bool NFmiPtrList<Type>::Swap(Iterator &iter1, Iterator &iter2)
{
  // the begin and end nodes can't be swapped
  // third check perhaps needless - a list including same nodes perhaps never exist??
  if (!iter1.itsCurrent || !iter2.itsCurrent)
    return false;
  // first creating and setting iterators pointing
  // to the both sides of the items to be swapped
  Iterator iter1previous(iter1);
  Iterator iter1next(iter1);
  iter1previous.Previous();
  iter1next.Next();

  Iterator iter2previous(iter2);
  Iterator iter2next(iter2);
  iter2previous.Previous();
  iter2next.Next();

  // then saving the pointers towars adjacent nodes of the both nodes to be swapped
  Node *temp1next = iter1.itsCurrent->itsNext;
  Node *temp1prev = iter1.itsCurrent->itsPrevious;
  Node *temp2next = iter2.itsCurrent->itsNext;
  Node *temp2prev = iter2.itsCurrent->itsPrevious;

  // There must be separated two case:
  // 1) the nodes to be swapped are not adjacent
  if (temp1next != iter2.itsCurrent)
  {
    iter1.itsCurrent->itsNext = temp2next;
    iter1.itsCurrent->itsPrevious = temp2prev;
    iter1previous.itsCurrent->itsNext = iter2.itsCurrent;
    iter1next.itsCurrent->itsPrevious = iter2.itsCurrent;

    iter2.itsCurrent->itsNext = temp1next;
    iter2.itsCurrent->itsPrevious = temp1prev;
    iter2previous.itsCurrent->itsNext = iter1.itsCurrent;
    iter2next.itsCurrent->itsPrevious = iter1.itsCurrent;
  }
  else
  // 2) swapping adjacent nodes
  {
    iter1.itsCurrent->itsNext = temp2next;
    iter1.itsCurrent->itsPrevious = iter2.itsCurrent;
    iter1previous.itsCurrent->itsNext = iter2.itsCurrent;

    iter2.itsCurrent->itsNext = iter1.itsCurrent;
    iter2.itsCurrent->itsPrevious = temp1prev;
    iter2next.itsCurrent->itsPrevious = iter1.itsCurrent;
  }
  // at last swapping of iterators to retain the iterators place in the list
  Iterator tempIter = iter1;
  iter1 = iter2;
  iter2 = tempIter;

  return true;
}

template <class Type>
bool NFmiPtrList<Type>::Sort(bool fAscendingOrder)

// bool Sort(bool fAscendingOrder = true)
{
  Iterator theIter1 = Start();
  Iterator theIter2;
  while (theIter1.Next())
  {
    theIter2 = theIter1;
    while (theIter2.Next())
    {
      if ((theIter2.Current() < theIter1.Current() && fAscendingOrder) ||
          (theIter1.Current() < theIter2.Current() && !fAscendingOrder))
      {
        Swap(theIter1, theIter2);
      }
    }
  }
  return true;
}

template <class Type>
bool NFmiPtrList<Type>::RemoveStart(bool fDeleteItem)
{
  // Iterator::Remove() deals the checking that end and star nodes
  // will never be removed
  Iterator iter = Start();
  if (iter.Next())
    return iter.Remove(fDeleteItem);
  return false;
}

template <class Type>
bool NFmiPtrList<Type>::RemoveEnd(bool fDeleteItem)
{
  // Iterator::Remove() deals the checking that end and star nodes
  // will never be removed
  Iterator iter = End();
  if (iter.Previous())
    return iter.Remove(fDeleteItem);
  return false;
}

template <class Type>
bool NFmiPtrList<Type>::Remove(unsigned long theIndex, bool fDeleteItem)
{
  // As the method NumberOfItems() this method had to be implemented
  // through counting the items in the list beginning from the start
  // untill the counter has reached the value of index given (if not,
  // the returning is false) and then removing the corresponding item
  Iterator iter = Start();
  unsigned long counter;
  for (counter = 0; counter < theIndex && iter.Next(); counter++)
    ;
  if (counter == theIndex)
    return iter.Remove(fDeleteItem);
  return false;
}

template <class Type>
unsigned long NFmiPtrList<Type>::NumberOfItems()
{
  // Muutin seuraavaa koodia 8.9.99 /Mikael
  return itsCounter;
}

template <class Type>
typename NFmiPtrList<Type>::Iterator NFmiPtrList<Type>::Index(unsigned long theIndex)
{
  // Returns iter pointing to node in order to index in the list.
  // If not succeeded returning iterator points to the end of the list,
  // or if given index <=0 returns iterator pointing to the start of list.
  // Index count starts from 1 i.e first item in list is with index number 1
  // and second is 2 and so on.
  Iterator iter(itsStart, this);
  if (theIndex > 0)
  {
    for (unsigned long ind = 0; iter.Next(); ind++)
    {
      if (ind + 1 == theIndex)
        return iter;
    }
  }
  return iter;
}

template <class Type>
bool NFmiPtrList<Type>::Add(Type *theItem, unsigned long theIndex)
{
  // Add theItem to the list in oder to index so that
  // theItem will have the given index in the list.
  // If not succeeded the returs false.
  Iterator iter(itsStart, this);

  unsigned long ind = 0;
  do
  {
    if (++ind == theIndex)
      return iter.AddAfter(theItem);
  } while (iter.Next());
  return false;
}

template <class Type>
bool NFmiPtrList<Type>::Swap(unsigned long ind1, unsigned long ind2)
{
  Iterator iter1 = Index(ind1);
  Iterator iter2 = Index(ind2);
  //	return Swap(iter1,iter2); // tämä ei toiminut, tein uuden virityksen
  iter1.GetNode()->SwapData(*iter2.GetNode());
  return true;
}

template <class Type>
bool NFmiPtrList<Type>::Find(const Type &theValue, Iterator &theFoundItemIterator)
{
  Iterator iter(itsStart, this);
  for (; iter.Next();)
  {
    if (iter.Current() == theValue)
    {
      theFoundItemIterator = iter;
      return true;
    }
  }
  return false;
}
