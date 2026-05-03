// ======================================================================
/*!
 * \file
 * \brief Interface of class NFmiNearTree
 */
// ======================================================================
/*!
 * \class NFmiNearTree
 *
 * \brief Nearest neighbour search tree from C++ Users Journal.
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
 * The insertion of points is randomized so that pathological insertion
 * orders should not occur.
 *
 * Sample use:
 * \code
 * NFmiNearTree<NFmiPoint> neartree;
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
 * \endcode
 * Note that in general the NFmiNearTree takes two template parameters,
 * not one. The latter is the functor to be used for calculating
 * the distance. The default value of the functor is NFmiNearTree<T>,
 * where T is the value type. The default implementation for type T
 * assumes there are X() and Y() accessors in the value type, and that
 * the distance is the 2D Euclidian distance.
 *
 * The user may specialize the functor externally. For example, assume
 * there is a Point class which has the accessors written in lower case.
 * Then one might define
 * \code
 * template <>
 * class NFmiNearTreeDistance<T>
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
 * and pass it as the second template parameter for NFmiNearTree.
 *
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include "NFmiNearTreeImpl.h"
#include <algorithm>
#include <cmath>
#include <vector>

// An utility functor to generate default 2D distance calculator
// This can be specialized externally to extend functionality!

template <typename T>
class NFmiNearTreeDistance
{
 public:
  double operator()(const T& theLhs, const T& theRhs) const
  {
    const double dx = theLhs.X() - theRhs.X();
    const double dy = theLhs.Y() - theRhs.Y();
    return sqrt(dx * dx + dy * dy);
  }
};  // class NFmiNearTreeDistance

// The actual class

template <typename T, typename F = NFmiNearTreeDistance<T> >
class NFmiNearTree
{
 public:
  typedef T value_type;
  typedef F functor_type;

  ~NFmiNearTree();
  NFmiNearTree();

  void Clear();

  void Insert(const value_type& theObject);

  bool NearestPoint(value_type& theClosest,
                    const value_type& thePoint,
                    double theRadius = -1.0) const;

  bool FarthestPoint(value_type& theFarthest, const value_type& thePoint) const;

  unsigned long NearestPoints(std::vector<value_type>& theClosest,
                              const value_type& thePoint,
                              double theRadius) const;

  void Flush() const;

 private:
  NFmiNearTree(const NFmiNearTree& theTree);
  NFmiNearTree& operator=(const NFmiNearTree& theTree);

  mutable NFmiNearTreeImpl<value_type, functor_type> itsImpl;
  typedef std::vector<value_type> buffer_type;
  mutable buffer_type itsInputBuffer;

};  // class NFmiNearTree

// ----------------------------------------------------------------------
/*!
 * \brief Destructor for class NFmiNearTree
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
NFmiNearTree<T, F>::~NFmiNearTree()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Default constructor for class NFmiNearTree
 *
 * Creates an empty tree with no right or left node and with the
 * dMax-below set to negative values so that any match found will be
 * stored since it will greater than the negative value
 *
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
NFmiNearTree<T, F>::NFmiNearTree() : itsImpl(), itsInputBuffer()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear the near tree contents
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NFmiNearTree<T, F>::Clear()
{
  itsImpl.Clear();
  itsInputBuffer.clear();
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert a new point into the tree
 *
 * Function to insert some "point" as an object into a NFmiNearTree for
 * later searching.
 *
 * \param theObject is an object of the templated type which is
 *        to be inserted into a Neartree
 *
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NFmiNearTree<T, F>::Insert(const value_type& theObject)
{
  itsInputBuffer.push_back(theObject);
}

// ----------------------------------------------------------------------
/*!
 * \brief Find closest point within given search radius
 *
 * Function to search a NFmiNearTree for the object closest to some probe
 * point, thePoint. This function is only here so that the function
 * Nearest can be called without having theRadius const.
 *
 * A negative search radius effectively means there is no upper
 * limit on the search radius.
 *
 * \param theClosest The object into which to store the closest point
 * \param thePoint The probe point
 * \param theRadius The maximum search radius
 * \return True if a point was found
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
bool NFmiNearTree<T, F>::NearestPoint(value_type& theClosest,
                                      const value_type& thePoint,
                                      double theRadius) const
{
  Flush();
  return itsImpl.NearestPoint(theClosest, thePoint, theRadius);
}

// ----------------------------------------------------------------------
/*!
 * \brief Find farthest point from probe point
 *
 * Function to search a NFmiNearTree for the object farthest from some
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
bool NFmiNearTree<T, F>::FarthestPoint(value_type& theFarthest, const value_type& thePoint) const
{
  Flush();
  return itsImpl.FarthestPoint(theFarthest, thePoint);
}

// ----------------------------------------------------------------------
/*!
 * \brief Find all objects within a search radius
 *
 * Function to search a NFmiNearTree for the set of objects closer
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
unsigned long NFmiNearTree<T, F>::NearestPoints(std::vector<value_type>& theClosest,
                                                const value_type& thePoint,
                                                double theRadius) const

{
  Flush();
  return itsImpl.NearestPoints(theClosest, thePoint, theRadius);
}

// ----------------------------------------------------------------------
/*!
 * \brief Flush the contents of the input buffer
 *
 * The input buffer is used by insert to store all input
 * points. Whenever a find method is invoked, the order
 * of the input buffer is randomized, and the contents
 * are fed into the internal implementation of the NFmiNearTree.
 * This should provide maximum safety against pathological
 * insertion orders, such as lexicographically increasing
 * sequences.
 *
 * It is the presence of the input buffer which effectively
 * forces the internal variables to be mutable so that
 * the find methods can be made const.
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NFmiNearTree<T, F>::Flush() const
{
  if (!itsInputBuffer.empty())
  {
    std::random_shuffle(itsInputBuffer.begin(), itsInputBuffer.end());

    for (typename buffer_type::const_iterator it = itsInputBuffer.begin();
         it != itsInputBuffer.end();
         ++it)
      itsImpl.Insert(*it);

    itsInputBuffer.clear();
  }
}

// ======================================================================
