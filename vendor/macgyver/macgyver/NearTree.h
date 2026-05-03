// ======================================================================
/*!
 * \file
 * \brief Interface of class NearTree
 */
// ======================================================================
/*!
 * \class NearTree
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
 * NearTree<NFmiPoint> neartree;
 *
 * neartree.Insert(NFmiPoint(x1,y1));
 * ...
 * neartree.Insert(NFmiPoint(xn,yn));
 *
 * NFmiPoint result;
 *
 * if(neartree.nearest(result,NFmiPoint(x0,y0)))
 * {
 *   // result is the closest point to x0,y0
 * }
 *
 * if(neartree.farthest(result,NFmiPoint(x0,y0)))
 * {
 *   // result is the farthest point from x0,y0
 * }
 *
 * if(neartree.nearest(result,NFmiPoint(x0,y0),100))
 * {
 *   // result is the closest point to x0,y0 withing the radius of 100
 * }
 *
 * std::multimap<double,NFmiPoint> results = neartree.nearestones(results,NFmiPoint(x0,y0),100);
 * \endcode
 * Note that in general the NearTree takes two template parameters,
 * not one. The latter is the functor to be used for calculating
 * the distance. The default value of the functor is NearTree<T>,
 * where T is the value type. The default implementation for type T
 * assumes there are X() and Y() accessors in the value type, and that
 * the distance is the 2D Euclidian distance.
 *
 * The user may specialize the functor externally. For example, assume
 * there is a Point class which has the accessors written in lower case.
 * Then one might define
 * \code
 * template <>
 * class NearTreeDistance<T>
 * {
 * public:
 *    double operator()(const T & lhs, const T & rhs) const
 *    {
 *      const double dx = lhs.x()-rhs.y();
 *      const double dy = lhs.y()-rhs.y();
 *      return std::sqrt(dx*dy+dy*dy);
 *    }
 * };
 * \endcode
 * Alternatively, one may define the functor directly as a non-template
 * and pass it as the second template parameter for NearTree.
 *
 */
// ======================================================================

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <algorithm>
#include <cmath>
#include <map>
#include <optional>
#include <random>
#include <vector>

namespace Fmi
{
// An utility functor to generate default 2D distance calculator
// This can be specialized externally to extend functionality!

template <typename T>
class NearTreeDistance
{
 public:
  double operator()(const T& lhs, const T& rhs) const
  {
    const double dx = lhs.x() - rhs.x();
    const double dy = lhs.y() - rhs.y();
    return sqrt(dx * dx + dy * dy);
  }
};  // class NFmiNearTreeDistance

// The actual class

template <typename T, typename F = NearTreeDistance<T> >
class NearTree
{
 private:
  class Impl
  {
   public:
    using value_type = T;
    using functor_type = F;
    using size_type = std::size_t;

    Impl();
    ~Impl();

    size_type size() const { return itsSize; }
    bool empty() const { return size() == 0; }
    void clear();

    void insert(const value_type& theObject);

    void nearest(std::optional<value_type>& theClosest,
                 const value_type& thePoint,
                 double& theRadius) const;

    void farthest(std::optional<value_type>& theFarthest,
                  const value_type& thePoint,
                  double& theRadius) const;

    void nearestones(std::multimap<double, value_type>& theClosest,
                     const value_type& thePoint,
                     double theRadius) const;

   private:
    size_type itsSize;
    value_type* itsLeftObject;
    value_type* itsRightObject;
    double itsMaxLeft;
    double itsMaxRight;
    Impl* itsLeftBranch;
    Impl* itsRightBranch;

    functor_type Distance;
  };

 public:
  using value_type = T;
  using functor_type = F;
  using size_type = std::size_t;

  ~NearTree();
  NearTree();
  NearTree(const NearTree& other) = delete;
  NearTree(NearTree&& other) = delete;
  NearTree& operator=(const NearTree& other) = delete;
  NearTree& operator=(NearTree&& other) = delete;

  size_type size() const { return impl->size() + buffer.size(); }
  bool empty() const { return impl->empty() && buffer.empty(); }
  void clear();
  void flush() const;

  void insert(const value_type& theObject);

  std::optional<value_type> nearest(const value_type& thePoint, double theRadius = -1.0) const;

  std::optional<value_type> farthest(const value_type& thePoint) const;

  std::multimap<double, value_type> nearestones(const value_type& thePoint, double theRadius) const;

 private:
  std::shared_ptr<Impl> impl;

  using buffer_type = std::vector<value_type>;
  mutable buffer_type buffer;

};  // class NearTree

// ----------------------------------------------------------------------
/*!
 * \brief Destructor for class NearTree::Impl
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
NearTree<T, F>::Impl::~Impl()
{
  delete itsLeftBranch;
  delete itsRightBranch;
  delete itsLeftObject;
  delete itsRightObject;
}

// ----------------------------------------------------------------------
/*!
 * \brief Default constructor for class NearTree::Impl
 *
 * Creates an empty tree with no right or left node and with the
 * dMax-below set to negative values so that any match found will be
 * stored since it will greater than the negative value
 *
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
NearTree<T, F>::Impl::Impl()
    : itsSize(0),
      itsLeftObject(0),
      itsRightObject(0),
      itsMaxLeft(-1.0),
      itsMaxRight(-1.0),
      itsLeftBranch(0),
      itsRightBranch(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear the tree
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NearTree<T, F>::Impl::clear()
{
  // emulating destructor
  delete itsLeftBranch;
  delete itsRightBranch;
  delete itsLeftObject;
  delete itsRightObject;
  // emulating constructor
  itsSize = 0;
  itsLeftBranch = 0;
  itsRightBranch = 0;
  itsLeftObject = 0;
  itsRightObject = 0;
  itsMaxLeft = -1.0;
  itsMaxRight = -1.0;
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert a new point into the tree
 *
 * Function to insert some "point" as an object into a NearTree::Impl for
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
void NearTree<T, F>::Impl::insert(const T& theObject)
{
  ++itsSize;

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
      itsRightBranch = new typename NearTree<T, F>::Impl();

    // note that the next line assumes that itsMaxRight is
    // negative for a new node

    if (itsMaxRight < dist_right)
      itsMaxRight = dist_right;

    itsRightBranch->insert(theObject);
  }

  else
  {
    if (itsLeftBranch == 0)
      itsLeftBranch = new typename NearTree<T, F>::Impl();

    // note that the next line assumes that itsMaxLeft is
    // negative for a new node

    if (itsMaxLeft < dist_left)
      itsMaxLeft = dist_left;

    itsLeftBranch->insert(theObject);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Find all objects within a search radius
 *
 * Private function to search a NearTree::Impl for the object closest
 * to some probe point, thePoint. This function is only called by
 * NearestPoints.
 *
 * \param theClosest The list of closest objects
 * \param thePoint The probe point
 * \param theRadius The maximum search radius
 * \return The number of closest points withing the search radius
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NearTree<T, F>::Impl::nearestones(std::multimap<double, value_type>& theClosest,
                                       const value_type& thePoint,
                                       double theRadius) const

{
  // first test each of the left and right positions to see if
  // one holds a point nearer than the search radius.

  if (itsLeftObject != 0)
  {
    double dist = Distance(thePoint, *itsLeftObject);
    if (dist <= theRadius)
      theClosest.insert(std::make_pair(dist, *itsLeftObject));
  }
  if (itsRightObject != 0)
  {
    double dist = Distance(thePoint, *itsRightObject);
    if (dist <= theRadius)
      theClosest.insert(std::make_pair(dist, *itsRightObject));
  }

  // Now we test to see if the branches below might hold an object
  // nearer than the search radius. The triangle rule is used
  // to test whether it's even necessary to descend.

  if ((itsLeftBranch != 0) && (theRadius + itsMaxLeft >= Distance(thePoint, *itsLeftObject)))
  {
    itsLeftBranch->nearestones(theClosest, thePoint, theRadius);
  }

  if ((itsRightBranch != 0) && (theRadius + itsMaxRight >= Distance(thePoint, *itsRightObject)))
  {
    itsRightBranch->nearestones(theClosest, thePoint, theRadius);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Find closest point within given search radius
 *
 * Private function to search a NearTree::Impl for the object closest
 * to some probe point, thePoint. If the search radius is negative,
 * there is no distance limit.
 *
 * This function is only called by NearestPoint.
 *
 * \param theClosest The found closest point
 * \param thePoint The probe point
 *�\param theRadius The smallest currently known distance of an object from
 *                  the probe point.
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NearTree<T, F>::Impl::nearest(std::optional<value_type>& theClosest,
                                   const value_type& thePoint,
                                   double& theRadius) const
{
  double tmpradius;

  // first test each of the left and right positions to see if
  // one holds a point nearer than the nearest so far discovered.

  if (itsLeftObject != 0)
  {
    tmpradius = Distance(thePoint, *itsLeftObject);
    if (theRadius < 0 || tmpradius <= theRadius)
    {
      theRadius = tmpradius;
      theClosest = *itsLeftObject;
    }
  }

  if (itsRightObject != 0)
  {
    tmpradius = Distance(thePoint, *itsRightObject);
    if (theRadius < 0 || tmpradius <= theRadius)
    {
      theRadius = tmpradius;
      theClosest = *itsRightObject;
    }
  }

  // If theRadius is negative at this point, the tree is empty

  if (theRadius < 0)
    return;

  // Now we test to see if the branches below might hold an object
  // nearer than the best so far found. The triangle rule is used
  // to test whether it's even necessary to descend.

  if ((itsLeftBranch != 0) && ((theRadius + itsMaxLeft) >= Distance(thePoint, *itsLeftObject)))
  {
    itsLeftBranch->nearest(theClosest, thePoint, theRadius);
  }

  if ((itsRightBranch != 0) && ((theRadius + itsMaxRight) >= Distance(thePoint, *itsRightObject)))
  {
    itsRightBranch->nearest(theClosest, thePoint, theRadius);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Find farthes point from the probe point
 *
 * Private function to search a NearTree::Impl for the object farthest
 * from some probe point, thePoint.
 *
 * This function is only called by FarthestPoint.
 *
 * \param theFarthest The found farthest point
 * \param thePoint The probe point
 * \param theRadius The distance of the farthest point so far
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NearTree<T, F>::Impl::farthest(std::optional<value_type>& theFarthest,
                                    const value_type& thePoint,
                                    double& theRadius) const
{
  double tmpradius;

  // first test each of the left and right positions to see if
  // one holds a point farther than the farthest so far discovered.
  // the calling function is presumed initially to have set theRadius to a
  // negative value before the recursive calls to FarthestNeighbor

  if ((itsLeftObject != 0) && ((tmpradius = Distance(thePoint, *itsLeftObject)) >= theRadius))
  {
    theRadius = tmpradius;
    theFarthest = *itsLeftObject;
  }

  if ((itsRightObject != 0) && ((tmpradius = Distance(thePoint, *itsRightObject)) >= theRadius))
  {
    theRadius = tmpradius;
    theFarthest = *itsRightObject;
  }

  // Now we test to see if the branches below might hold an object
  // farther than the best so far found. The triangle rule is used
  // to test whether it's even necessary to descend.

  if ((itsLeftBranch != 0) && ((theRadius - itsMaxLeft) <= Distance(thePoint, *itsLeftObject)))
  {
    itsLeftBranch->farthest(theFarthest, thePoint, theRadius);
  }

  if ((itsRightBranch != 0) && ((theRadius - itsMaxRight) <= Distance(thePoint, *itsRightObject)))
  {
    itsRightBranch->farthest(theFarthest, thePoint, theRadius);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor for class NearTree
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
NearTree<T, F>::~NearTree()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Default constructor for class NearTree
 *
 * Creates an empty tree with no right or left node and with the
 * dMax-below set to negative values so that any match found will be
 * stored since it will greater than the negative value
 *
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
NearTree<T, F>::NearTree() : impl(new Impl())
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear the near tree contents
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NearTree<T, F>::clear()
{
  impl->clear();
  buffer.clear();
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert a new point into the tree
 *
 * Function to insert some "point" as an object into a NearTree for
 * later searching.
 *
 * \param theObject is an object of the templated type which is
 *        to be inserted into a Neartree
 *
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
void NearTree<T, F>::insert(const value_type& theObject)
{
  buffer.push_back(theObject);
}

// ----------------------------------------------------------------------
/*!
 * \brief Find closest point within given search radius
 *
 * Function to search a NearTree for the object closest to some probe
 * point, thePoint.
 *
 * A negative search radius effectively means there is no upper
 * limit on the search radius.
 *
 * \param thePoint The probe point
 * \param theRadius The maximum search radius
 * \return The closest point if any
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
std::optional<typename NearTree<T, F>::value_type> NearTree<T, F>::nearest(
    const value_type& thePoint, double theRadius) const
{
  this->flush();
  std::optional<value_type> near;
  double radius = theRadius;
  impl->nearest(near, thePoint, radius);
  return near;
}

// ----------------------------------------------------------------------
/*!
 * \brief Find farthest point from probe point
 *
 * Function to search a NearTree for the object farthest from some
 * probe point, thePoint. This function is only here so that the
 * function Farthest can be called without the user having to input
 * a search radius and so the search radius can be guaranteed to be
 * negative at the start.
 *
 * \param thePoint The probe point
 * \return The farthest point if any
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
std::optional<typename NearTree<T, F>::value_type> NearTree<T, F>::farthest(
    const value_type& thePoint) const
{
  this->flush();
  std::optional<value_type> far;
  double radius = -1;
  impl->farthest(far, thePoint, radius);
  return far;
}

// ----------------------------------------------------------------------
/*!
 * \brief Find all objects within a search radius
 *
 * Function to search a NearTree for the set of objects closer
 * to some probe point, thePoint, than theRadius. This is only here so
 * that theClosest can be cleared before starting the work.
 *
 * \param theClosest The list of closest objects
 * \param thePoint The probe point
 * \param theRadius The maximum search radius
 * \return The number of closest points within the search radius
 */
// ----------------------------------------------------------------------

template <typename T, typename F>
std::multimap<double, typename NearTree<T, F>::value_type> NearTree<T, F>::nearestones(
    const value_type& thePoint, double theRadius) const

{
  this->flush();
  std::multimap<double, value_type> near;
  impl->nearestones(near, thePoint, theRadius);
  return near;
}

// ----------------------------------------------------------------------
/*!
 * \brief Flush the contents of the input buffer
 *
 * The input buffer is used by insert to store all input
 * points. Whenever a find method is invoked, the order
 * of the input buffer is randomized, and the contents
 * are fed into the internal implementation of the NearTree.
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
void NearTree<T, F>::flush() const
{
  if (!buffer.empty())
  {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(buffer.begin(), buffer.end(), g);

    for (const T& it : buffer)
      impl->insert(it);

    buffer.clear();
  }
}

}  // namespace Fmi

// ======================================================================
