// ======================================================================
/*!
 * \file
 * \brief Interface of class NFmiNearTreeImpl
 */
// ======================================================================
/*!
 * \class NFmiNearTreeImpl
 *
 * \brief Nearest neighbour search tree from C++ Users Journal.
 *
 * Note that in general one should prefer NFmiNearTree over
 * NFmiNearTreeImpl in order to avoid slow searches due to
 * pathological insertion orders.
 *
 * Nearest Neighbor algorithm after Kalantari and McDonald,
 * (IEEE Transactions on Software Engineering, v. SE-9, pp.631-634,1983)
 * modified to use recursion instead of a double-linked tree
 * and simplified so that it does a bit less checking for
 * things like is the distance to the right less than the
 * distance to the left; it was found that these checks little
 * to no difference.
 *
 * copyright by Larry Andrews, 2001
 * may be freely distributed or used as long as this copyright notice
 * is included
 *
 * This template is used to contain a collection of objects. After the
 * collection has been loaded into this structure, it can be quickly
 * queried for which object is "closest" to some probe object of the
 * same type. The major restriction on applicability of the near-tree
 * is that the algorithm only works if the objects obey the triangle
 * inequality. The triangle rule states that the length of any side of
 * a triangle cannot exceed the sum of the lengths of the other two sides.
 *
 * The user of this class needs to provide at least the following
 * functionality for the template to work. For the built-in
 * numerics of C++, they are provided by the system.
 *
 *    - a functor to calculate the distance
 *    - a copy constructor
 *    - a constructor would be nice
 *    - a destructor would be nice
 *
 * NearestPoint retrieves the object nearest to some probe by
 * descending the tree to search out the appropriate object. Speed is
 * gained by pruning the tree if there can be no data below that are
 * nearer than the best so far found.
 *
 * The tree is built in time O(n log n), and retrievals take place in
 * time O(log n).
 *
 * Sample use:
 * \code
 * NFmiNearTreeImpl<NFmiPoint> neartree;
 *
 * neartree.Insert(NFmiPoint(x1,y1));
 * ...
 * neartree.Insert(NFmiPoint(xn,yn));
 *
 * NFmiPoint result;
 *
 * if(NearestPoint(result,NFmiPoint(x0,y0)))
 * {
 *   // result is the closest point to x0,y0
 * }
 *
 * if(FarthestPoint(result,NFmiPoint(x0,y0)))
 * {
 *   // result is the farthest point from x0,y0
 * }
 *
 * if(NearestPoint(result,NFmiPoint(x0,y0),100))
 * {
 *   // result is the closest point to x0,y0 withing the radius of 100
 * }
 *
 * std::vector<NFmiPoint> results;
 * if(NearestPoints(results,NFmiPoint(x0,y0),100) > 0)
 * {
 *   // results now contains atleast one point within radius 100
 * }
 *
 * Note that in general the NFmiNearTreeImpl takes two template parameters,
 * not one. The latter is the functor to be used for calculating
 * the distance. The default value of the functor is NFmiNearTreeImpl<T>,
 * where T is the value type. The default implementation for type T
 * assumes there are X() and Y() accessors in the value type, and that
 * the distance is the 2D Euclidian distance.
 *
 * The user may specialize the functor externally. For example, assume
 * there is a Point class which has the accessors written in lower case.
 * Then one might define
 * \code
 * template <>
 * class NFmiNearTreeImplDistance<T>
 * {
 * public:
 *    double operator()(const T & theLhs, const T & theRhs) const
 *    {
 *      const double dx = theLhs.x()-theRhs.y();
 *      const double dy = theLhs.y()-theRhs.y();
 *      return std::sqrt(dx*dy+dy*dy);
 *    }
 * };
 * \endcode
 * Alternatively, one may define the functor directly as a non-template
 * and pass it as the second template parameter for NFmiNearTreeImpl.
 *
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include <cmath>
#include <vector>

// The actual class

template <typename T, typename F>
class NFmiNearTreeImpl
{
 public:
  typedef T value_type;
  typedef F functor_type;

  ~NFmiNearTreeImpl();
  NFmiNearTreeImpl() = default;

  void Clear();

  void Insert(const value_type& theObject);

  bool NearestPoint(value_type& theClosest,
                    const value_type& thePoint,
                    double theRadius = -1.0) const;

  bool FarthestPoint(value_type& theFarthest, const value_type& thePoint) const;

  unsigned long NearestPoints(std::vector<value_type>& theClosest,
                              const value_type& thePoint,
                              double theRadius) const;

 private:
  unsigned long NearestOnes(std::vector<value_type>& theClosest,
                            const value_type& thePoint,
                            double theRadius) const;

  bool Nearest(value_type& theClosest, const value_type& thePoint, double& theRadius) const;

  bool Farthest(value_type& theFarthest, const value_type& thePoint, double& theRadius) const;

 private:
  NFmiNearTreeImpl(const NFmiNearTreeImpl& theTree);
  NFmiNearTreeImpl& operator=(const NFmiNearTreeImpl& theTree);

  value_type* itsLeftObject = nullptr;         //!< first object stored in this node
  value_type* itsRightObject = nullptr;        //!< second object stored in this node
  double itsMaxLeft{-1.0};                     //!< max dist from itsLeftObject to itsLeftBranch
  double itsMaxRight{-1.0};                    //!< max dist from itsRightObject to itsRightBranch
  NFmiNearTreeImpl* itsLeftBranch = nullptr;   //!< nodes closer to itsLeftObject
  NFmiNearTreeImpl* itsRightBranch = nullptr;  //!< nodes closer to itsRightObject

  functor_type Distance;  // Template functor!!

};  // class NFmiNearTreeImpl

// ----------------------------------------------------------------------
/*!
 * \brief Destructor for class NFmiNearTreeImpl
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
NFmiNearTreeImpl<T, F>::~NFmiNearTreeImpl()
{
  delete itsLeftBranch;
  delete itsRightBranch;
  delete itsLeftObject;
  delete itsRightObject;
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear the tree
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NFmiNearTreeImpl<T, F>::Clear()
{
  // emulating destructor
  delete itsLeftObject;
  delete itsRightObject;
  delete itsLeftBranch;
  delete itsRightBranch;
  // emulating constructor
  itsLeftBranch = nullptr;
  itsRightBranch = nullptr;
  itsLeftObject = nullptr;
  itsRightObject = nullptr;
  itsMaxLeft = -1.0;
  itsMaxRight = -1.0;
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert a new point into the tree
 *
 * Function to insert some "point" as an object into a NFmiNearTreeImpl for
 * later searching.
 *
 *  Three possibilities exist:
 *   -# put the datum into the left postion (first test),
 *   -# into the right position, or
 *   -# into a node descending from the nearer of those positions
 *      when they are both already used.
 *
 * \param theObject is an object of the templated type which is
 *        to be inserted into a Neartree
 *
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NFmiNearTreeImpl<T, F>::Insert(const T& theObject)
{
  double dist_right = 0;
  double dist_left = 0;

  if (itsRightObject != 0)
  {
    dist_right = Distance(theObject, *itsRightObject);
    dist_left = Distance(theObject, *itsLeftObject);
  }

  if (itsLeftObject == 0)
    itsLeftObject = new value_type(theObject);

  else if (itsRightObject == 0)
    itsRightObject = new value_type(theObject);

  else if (dist_left > dist_right)
  {
    if (itsRightBranch == 0)
      itsRightBranch = new NFmiNearTreeImpl();

    // note that the next line assumes that itsMaxRight is
    // negative for a new node

    if (itsMaxRight < dist_right)
      itsMaxRight = dist_right;
    itsRightBranch->Insert(theObject);
  }

  else
  {
    if (itsLeftBranch == 0)
      itsLeftBranch = new NFmiNearTreeImpl();

    // note that the next line assumes that itsMaxLeft is
    // negative for a new node

    if (itsMaxLeft < dist_left)
      itsMaxLeft = dist_left;

    itsLeftBranch->Insert(theObject);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Find closest point within given search radius
 *
 * Function to search a NFmiNearTreeImpl for the object closest to some probe
 * point, thePoint. This function is only here so that the function
 * Nearest can be called without having theRadius const.
 *
 * A negative search radius is interpreted to mean there is no
 * upper limit on the search radius.
 *
 * \param theClosest The object into which to store the closest point
 * \param thePoint The probe point
 * \param theRadius The maximum search radius
 * \return True if a point was found
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
bool NFmiNearTreeImpl<T, F>::NearestPoint(value_type& theClosest,
                                          const value_type& thePoint,
                                          double theRadius) const
{
  double searchradius = theRadius;
  return (Nearest(theClosest, thePoint, searchradius));
}

// ----------------------------------------------------------------------
/*!
 * \brief Find farthest point from probe point
 *
 * Function to search a NFmiNearTreeImpl for the object farthest from some
 * probe point, thePoint. This function is only here so that the
 * function Farthest can be called without the user having to input
 * a search radius and so the search radius can be guaranteed to be
 * negative at the start.
 *
 * \param theFarthest The farthest point found
 * \param thePoint The probe point
 * \return True if a farthest point was found
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
bool NFmiNearTreeImpl<T, F>::FarthestPoint(value_type& theFarthest,
                                           const value_type& thePoint) const
{
  double searchradius = -1.0;
  return (Farthest(theFarthest, thePoint, searchradius));
}

// ----------------------------------------------------------------------
/*!
 * \brief Find all objects within a search radius
 *
 * Function to search a NFmiNearTreeImpl for the set of objects closer
 * to some probe point, thePoint, than theRadius. This is only here so
 * that theClosest can be cleared before starting the work.
 *
 * \param theClosest The vector of closest objects
 * \param thePoint The probe point
 * \param theRadius The maximum search radius
 * \return The number of closest points within the search radius
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
unsigned long NFmiNearTreeImpl<T, F>::NearestPoints(std::vector<value_type>& theClosest,
                                                    const value_type& thePoint,
                                                    double theRadius) const

{
  // clear the contents of the return vector so that things
  // don't accidentally accumulate

  theClosest.clear();
  return (NearestOnes(theClosest, thePoint, theRadius));
}

// ----------------------------------------------------------------------
/*!
 * \brief Find all objects within a search radius
 *
 * Private function to search a NFmiNearTreeImpl for the object closest
 * to some probe point, thePoint. This function is only called by
 * NearestPoints.
 *
 * \param theClosest The vector of closest objects
 * \param thePoint The probe point
 * \param theRadius The maximum search radius
 * \return The number of closest points withing the search radius
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
unsigned long NFmiNearTreeImpl<T, F>::NearestOnes(std::vector<value_type>& theClosest,
                                                  const value_type& thePoint,
                                                  double theRadius) const

{
  long npoints = 0;

  // first test each of the left and right positions to see if
  // one holds a point nearer than the search radius.

  if ((itsLeftObject != nullptr) && (Distance(thePoint, *itsLeftObject) <= theRadius))
  {
    theClosest.push_back(*itsLeftObject);
    npoints++;
  }
  if ((itsRightObject != nullptr) && (Distance(thePoint, *itsRightObject) <= theRadius))
  {
    theClosest.push_back(*itsRightObject);
    npoints++;
  }

  // Now we test to see if the branches below might hold an object
  // nearer than the search radius. The triangle rule is used
  // to test whether it's even necessary to descend.

  if ((itsLeftBranch != nullptr) && (theRadius + itsMaxLeft >= Distance(thePoint, *itsLeftObject)))
    npoints += itsLeftBranch->NearestOnes(theClosest, thePoint, theRadius);

  if ((itsRightBranch != nullptr) &&
      (theRadius + itsMaxRight >= Distance(thePoint, *itsRightObject)))
    npoints += itsRightBranch->NearestOnes(theClosest, thePoint, theRadius);

  return npoints;
}

// ----------------------------------------------------------------------
/*!
 * \brief Find closest point within given search radius
 *
 * Private function to search a NFmiNearTreeImpl for the object closest
 * to some probe point, thePoint. If the search radius is negative,
 * there is no distance limit.
 *
 * This function is only called by NearestPoint.
 *
 * \param theClosest The found closest point
 * \param thePoint The probe point
 * \param theRadius The smallest currently known distance of an object from
 *                  the probe point.
 * \return True only if a point was found within theRadius
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
bool NFmiNearTreeImpl<T, F>::Nearest(value_type& theClosest,
                                     const value_type& thePoint,
                                     double& theRadius) const
{
  bool found = false;

  // first test each of the left and right positions to see if
  // one holds a point nearer than the nearest so far discovered.

  double dist_left = -1.0;

  if (itsLeftObject != nullptr)
  {
    dist_left = Distance(thePoint, *itsLeftObject);
    if (theRadius < 0 || dist_left <= theRadius)
    {
      theRadius = dist_left;
      theClosest = *itsLeftObject;
      found = true;
    }
  }

  double dist_right = -1.0;
  if (itsRightObject != nullptr)
  {
    dist_right = Distance(thePoint, *itsRightObject);
    if (theRadius < 0 || dist_right <= theRadius)
    {
      theRadius = dist_right;
      theClosest = *itsRightObject;
      found = true;
    }
  }

  // If theRadius is negative at this point, the tree is empty

  if (theRadius < 0)
    return false;

  // Now we test to see if the branches below might hold an object
  // nearer than the best so far found. The triangle rule is used
  // to test whether it's even necessary to descend.

  if ((itsLeftBranch != nullptr) && ((theRadius + itsMaxLeft) >= dist_left))
  {
    found |= itsLeftBranch->Nearest(theClosest, thePoint, theRadius);
  }

  if ((itsRightBranch != nullptr) && ((theRadius + itsMaxRight) >= dist_right))
  {
    found |= itsRightBranch->Nearest(theClosest, thePoint, theRadius);
  }

  return found;
}

// ----------------------------------------------------------------------
/*!
 * \brief Find farthest point from the probe point
 *
 * Private function to search a NFmiNearTreeImpl for the object farthest
 * from some probe point, thePoint.
 *
 * This function is only called by FarthestPoint.
 *
 * \param theFarthest The found farthest point
 * \param thePoint The probe point
 * \param theRadius The distance of the farthest point so far
 * \return True only if a point was found
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
bool NFmiNearTreeImpl<T, F>::Farthest(value_type& theFarthest,
                                      const value_type& thePoint,
                                      double& theRadius) const
{
  double tmpradius;
  bool found = false;

  // first test each of the left and right positions to see if
  // one holds a point farther than the farthest so far discovered.
  // the calling function is presumed initially to have set theRadius to a
  // negative value before the recursive calls to FarthestNeighbor

  if ((itsLeftObject != nullptr) && ((tmpradius = Distance(thePoint, *itsLeftObject)) >= theRadius))
  {
    theRadius = tmpradius;
    theFarthest = *itsLeftObject;
    found = true;
  }

  if ((itsRightObject != nullptr) &&
      ((tmpradius = Distance(thePoint, *itsRightObject)) >= theRadius))
  {
    theRadius = tmpradius;
    theFarthest = *itsRightObject;
    found = true;
  }

  // Now we test to see if the branches below might hold an object
  // farther than the best so far found. The triangle rule is used
  // to test whether it's even necessary to descend.

  if ((itsLeftBranch != nullptr) &&
      ((theRadius - itsMaxLeft) <= Distance(thePoint, *itsLeftObject)))
    found |= itsLeftBranch->Farthest(theFarthest, thePoint, theRadius);

  if ((itsRightBranch != nullptr) &&
      ((theRadius - itsMaxRight) <= Distance(thePoint, *itsRightObject)))
    found |= itsRightBranch->Farthest(theFarthest, thePoint, theRadius);

  return found;
}

// ======================================================================
