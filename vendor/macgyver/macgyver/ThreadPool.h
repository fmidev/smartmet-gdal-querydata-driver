// ======================================================================
/*!
 * \brief Declaration of ThreadPool class
 *
 * ThreadPool is a dynamic size thread pool implementation
 */
// ======================================================================

#pragma once
#include <boost/thread.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <stdexcept>

namespace Fmi
{
namespace ThreadPool
{
using Task = std::function<void()>;
using MutexType = boost::mutex;
using Lock = boost::unique_lock<MutexType>;
using ConditionVariableType = boost::condition_variable;

// ======================================================================
/*!
 * \brief Declaration of Worker class
 *
 * Worker represents a single working thread for the ThreadPool
 */
// ======================================================================

template <class Executor>
class Worker : public std::enable_shared_from_this<Worker<Executor> >
{
  using ParentPtr = Executor*;
  using IteratorType = typename std::list<std::shared_ptr<Worker<Executor> > >::iterator;
  using Ptr = std::shared_ptr<Worker<Executor> >;

  struct Private { public: explicit Private() = default; };

 public:
  // ======================================================================
  /*!
   * \brief Worker constructor (private due to use of private struct - use factory method instead)
   *
   * Construct Worker with the associated pool
   */
  // ======================================================================

  Worker(Private, ParentPtr theParentPool) : itsParent(theParentPool), itsThread() {}

  // ======================================================================
  /*!
   * \brief Worker factory method
   *
   * Factory method to create a new Worker
   */
  // ======================================================================
  static Ptr create(ParentPtr theParentPool)
  {
    return std::make_shared<Worker<Executor> >(Private(), theParentPool);
  }

  ~Worker() {}
  // ======================================================================
  /*!
   * \brief The thread execution function
   *
   * The main function to be executed by the associated thread
   */
  // ======================================================================

  void run()
  {
    bool success = true;
    while (success)
    {
      success = itsParent->executeOne();
    }
    // Thread will terminate once control reaches here
    itsParent->workerDied(this->shared_from_this());
  }

  // ======================================================================
  /*!
   * \brief Attach the associated internal thread
   *
   */
  // ======================================================================

  void attachThread()
  {
    itsThread.reset(new boost::thread(boost::bind(&Worker<Executor>::run, this)));
  }

  // ======================================================================
  /*!
   * \brief Set the location iterator
   *
   * Sets the location iterator member
   */
  // ======================================================================

  void setLocation(IteratorType location) { itsLocation = location; }
  // ======================================================================
  /*!
   * \brief Get the location iterator
   *
   * Gets the location iterator member
   */
  // ======================================================================

  IteratorType getLocation() { return itsLocation; }
  void interrupt() { itsThread->interrupt(); }
  void join() { itsThread->join(); }

 private:
  ParentPtr itsParent;

  std::shared_ptr<boost::thread> itsThread;

  IteratorType itsLocation;
};

// ======================================================================
/*!
 * \brief Scheduler concept
 *
 * Schedulers are used by the ThreadPool as the incoming
 * message queue. Scheduler decides which tasks are fed to the Pool.
 *
 * A model of the Scheduler concept must fulfill the following API:
 *
 * Constructor(std::size_t maxSize);
 * Construct the Scheduler with maximum size.
 *
 * bool push(const TaskType task);
 * Returns true if task was inserted into the task queue.
 *
 * TaskType pop();
 * Get the next task for processing
 *
 * std::size_t size();
 * Get the current task queue size
 *
 * std::size_t maxSize();
 * Get the max task queue size
 *
 * bool empty();
 * Check if task queue is empty
 */
// ======================================================================

// ======================================================================
/*!
 * \brief Declaration of FifoScheduler class
 *
 * FifoScheduler is a model of Scheduler concept.
 * It uses First in - First out strategy.
 */
// ======================================================================

class FifoScheduler
{
 public:
  FifoScheduler(std::size_t maxSize) : itsMaxSize(maxSize) {}
  bool push(const Task& theTask)
  {
    if (itsQueue.size() < itsMaxSize)
    {
      itsQueue.push(theTask);
      return true;
    }
    else
    {
      return false;
    }
  }

  Task pop()
  {
    Task thisTask(itsQueue.front());
    itsQueue.pop();
    return thisTask;
  }

  std::size_t size() { return itsQueue.size(); }
  std::size_t maxSize() { return itsMaxSize; }
  bool empty() { return itsQueue.empty(); }

 private:
  std::size_t itsMaxSize;

  std::queue<Task> itsQueue;
};

// ======================================================================
/*!
 * \brief Declaration of ThreadPool class
 *
 * ThreadPool is a dynamic size thread pool implementation.
 */
// ======================================================================

template <class SchedulingPolicy = FifoScheduler>
class ThreadPool
{
 private:
  using PoolType = ThreadPool<SchedulingPolicy>;

  friend class Worker<PoolType>;

 public:
  // ======================================================================
  /*!
   * \brief Constructor
   *
   * Constructs ThreadPool with start pool size and maximum task
   * queue size.
   */
  // ======================================================================

  ThreadPool(std::size_t poolSize,
             std::size_t schedulerSize = std::numeric_limits<std::size_t>::max())
      : itsIsRunning(false),
        itsWorkerCount(0),
        itsTargetWorkerCount(poolSize),
        itsActiveCount(0),
        itsShutdownGracefully(true),
        itsDataEvent(),
        itsWorkerDeathEvent(),
        itsAllIdleEvent(),
        itsMutex(),
        itsScheduler(schedulerSize)
  {
  }

  ~ThreadPool() = default;
  ThreadPool(const ThreadPool& other) = default;
  ThreadPool(ThreadPool&& other) = default;
  ThreadPool& operator=(const ThreadPool& other) = default;
  ThreadPool& operator=(ThreadPool&& other) = default;

  // ======================================================================
  /*!
   * \brief Stops ThreadPool activity
   *
   * Stops the ThreadPool activity. Use this to shutdown the pool.
   * If itsShutdownGracefully is true, waits for
   * active threads to finish executing the current task. If not, terminates
   * all threads "immediately" (as quickly as they can be interrupted).
   */
  // ======================================================================

  void shutdown()
  {
    Lock lock(itsMutex);
    itsTargetWorkerCount = 0;
    itsDataEvent.notify_all();
    if (itsShutdownGracefully)
    {
      while (itsWorkerCount > 0)
      {
        itsWorkerDeathEvent.wait(lock);
      }
    }
    else
    {
      for (auto it = itsWorkers.begin(); it != itsWorkers.end(); ++it)
      {
        (*it)->interrupt();
      }
      lock.unlock();
      for (auto it = itsWorkers.begin(); it != itsWorkers.end(); ++it)
      {
        (*it)->join();
      }
    }

    itsAllIdleEvent.notify_one();
  }

  // ======================================================================
  /*!
   * \brief Starts ThreadPool activity
   *
   * Starts the ThreadPool activity. Processing starts after this method
   * is called.
   */
  // ======================================================================

  void start()
  {
    // Create initial workers
    Lock lock(itsMutex);
    if (!itsIsRunning)
    {
      for (unsigned int i = 0; i < itsTargetWorkerCount; ++i)
      {
        addWorker();
      }
      itsIsRunning = true;
    }
  }

  // ======================================================================
  /*!
   * \brief Sets graceful shutdown status
   *
   * If graceful shutdown is true, destructor waits for threads to finish
   * ongoing tasks. If false, threads are killed immediately
   */
  // ======================================================================

  void setGracefulShutdown(bool state)
  {
    Lock lock(itsMutex);
    itsShutdownGracefully = state;
  }
  // ======================================================================
  /*!
   * \brief Schedule task for processing
   *
   * Attempts to add task to the task queue. Returnts true if succesfull.
   * May fail if task queue is full.
   */
  // ======================================================================

  bool schedule(const Task& newTask)
  {
    bool inserted;
    Lock lock(itsMutex);
    {
      inserted = itsScheduler.push(newTask);
      if (inserted)
      {
        // Tell threads that new data is available
        itsDataEvent.notify_one();
      }
    }
    return inserted;
  }

  // ======================================================================
  /*!
   * \brief Resize the thread pool
   *
   * Resizes the thread pool to new target size. If target is greater
   * than the current size, new threads are added immediately. If lower,
   * threads are killed as they finish executing.
   */
  // ======================================================================

  void resize(std::size_t newSize)
  {
    Lock lock(itsMutex);

    if (newSize == itsWorkerCount)
      return;

    if (newSize < itsWorkerCount)
    {
      // Drain the pool
      itsTargetWorkerCount = newSize;
      itsDataEvent.notify_all();  // Notify threads which queue for data
    }
    else
    {
      // Fill the pool
      while (itsWorkerCount < newSize)
      {
        addWorker();
      }

      itsTargetWorkerCount = newSize;
    }
  }

  // ======================================================================
  /*!
   * \brief Get current thread pool size (number of workers)
   */
  // ======================================================================

  std::size_t getPoolSize()
  {
    Lock lock(itsMutex);
    return itsWorkerCount;
  }

  // ======================================================================
  /*!
   * \brief Get current queue size (number of pending tasks)
   */
  // ======================================================================

  std::size_t getQueueSize()
  {
    Lock lock(itsMutex);
    return itsScheduler.size();
  }

  // ======================================================================
  /*!
   * \brief Get maximum queue size
   */
  // ======================================================================

  std::size_t getQueueMaxSize()
  {
    Lock lock(itsMutex);
    return itsScheduler.maxSize();
  }

  // ======================================================================
  /*!
   * \brief Get active count (number of busy workers)
   */
  // ======================================================================

  std::size_t getActiveCount()
  {
    Lock lock(itsMutex);
    return itsActiveCount;
  }

  // ======================================================================
  /*!
   * \brief Wait until all queued tasks are finished.
   */
  // ======================================================================

  void join()
  {
    Lock lock(itsMutex);
    if (itsActiveCount == 0)
    {
      return;
    }
    else
    {
      itsAllIdleEvent.wait(lock);
    }
  }

 private:
  bool executeOne()
  {
    Task thisTask;

    {
      Lock lock(itsMutex);

      // Check if reduction to thread pool size has been requested
      if (itsTargetWorkerCount < itsWorkerCount)
      {
        // Kill this worker
        return false;
      }

      // If scheduler is empty, wait for data
      while (itsScheduler.empty())
      {
        --itsActiveCount;

        // Signal if all workers are idle
        if (itsActiveCount == 0)
        {
          itsAllIdleEvent.notify_one();
        }

        itsDataEvent.wait(lock);
        if (itsTargetWorkerCount < itsWorkerCount)
        {
          // Size reduction requested, terminate this worker
          return false;
        }

        ++itsActiveCount;
      }

      thisTask = itsScheduler.pop();
    }

    // Tasks are run concurrently outside lock
    try
    {
      thisTask();
    }
    catch (std::exception& ex)
    {
// Exceptions cause thread to terminate
#ifndef NDEBUG
      std::cout << "Caught exception: " << ex.what() << std::endl;
#endif
      return false;
    }
    catch (...)
    {
      std::cout << "Caugh unknown exception, continuing" << std::endl;
      return false;
    }

    return true;
  }

  // ======================================================================
  /*!
   * \brief Adds a new worker to the pool
   */
  // ======================================================================

  void addWorker()
  {
    // The calling function must lock the mutex!
    std::shared_ptr<Worker<PoolType> > newWorker = Worker<PoolType>::create(this);
    itsWorkers.push_back(newWorker);
    auto thisIterator = --itsWorkers.end();
    newWorker->setLocation(thisIterator);
    newWorker->attachThread();
    ++itsActiveCount;
    ++itsWorkerCount;
  }

  // ======================================================================
  /*!
   * \brief Signals that a worker has died.
   *
   * This function is the final act of a dying worker
   */
  // ======================================================================

  void workerDied(std::shared_ptr<Worker<PoolType> > theWorkerThatDied)
  {
    // Is called from the worker thread, must lock the pool mutex
    Lock lock(itsMutex);

    itsWorkers.erase(theWorkerThatDied->getLocation());
    --itsWorkerCount;
    --itsActiveCount;
#ifndef NDEBUG
    std::cout << "Thread " << boost::this_thread::get_id() << " dies" << std::endl;
    std::cout << "Workers left: " << itsWorkerCount << std::endl;
#endif

    // Replace the killed worker if necessary
    if (itsTargetWorkerCount > itsWorkerCount)
    {
#ifndef NDEBUG
      std::cout << "Recreating worker" << std::endl;
#endif
      addWorker();
    }

    // Notify that thread has been destructed
    itsWorkerDeathEvent.notify_one();
  }

  bool itsIsRunning;

  std::size_t itsWorkerCount;

  std::size_t itsTargetWorkerCount;

  std::size_t itsActiveCount;

  bool itsShutdownGracefully;

  ConditionVariableType itsDataEvent;

  ConditionVariableType itsWorkerDeathEvent;

  ConditionVariableType itsAllIdleEvent;

  MutexType itsMutex;

  SchedulingPolicy itsScheduler;

  std::list<std::shared_ptr<Worker<PoolType> > > itsWorkers;
};
}  // namespace ThreadPool
}  // namespace Fmi
