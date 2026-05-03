#pragma once

#include "Exception.h"
#include <atomic>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <type_traits>
#include <boost/chrono.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include "TypeName.h"

namespace Fmi
{
  template <typename ObjectType>
  class WorkerPool
  {
  private:
      struct PoolItem
      {
          typename std::list<PoolItem>::iterator next_free;
          bool reserved;
          std::shared_ptr<ObjectType> ptr;
      };

      class Releaser {
      public:
          Releaser(WorkerPool& pool, typename std::list<PoolItem>::iterator it)
              : pool(pool)
              , it(it)
          {
          }

          void operator ()(ObjectType*)
          {
              pool.release(it);
          }

      private:
          WorkerPool& pool;
          typename std::list<PoolItem>::iterator it;
      };

  public:
      WorkerPool(std::size_t initial_size, std::size_t max_size = 0, std::size_t step = 5)
          : in_shutdown(false)
          , initial_pool_size(initial_size < 1 ? 1 : initial_size)
          , max_pool_size(max_size <= initial_pool_size ? initial_pool_size : max_size)
          , max_reached_pool_size(0)
          , step(step > 0 ? step : 5)
          , curr_pool_size(0)
          , num_reserved(0)
      {
          static_assert(std::is_default_constructible<ObjectType>::value,
             "Type is not default constructable: provide factory method as the first argument" );
          first_free_item = items.end();
          create_pool_object = []() { return std::make_shared<ObjectType>(); };
      }

      WorkerPool(
          std::function<std::shared_ptr<ObjectType>()> create_pool_object,
          std::size_t initial_size, std::size_t max_size = 0, std::size_t step = 5)

          : in_shutdown(false)
          , create_pool_object(create_pool_object)
          , initial_pool_size(initial_size < 1 ? 1 : initial_size)
          , max_pool_size(max_size <= initial_pool_size ? initial_pool_size : max_size)
          , max_reached_pool_size(0)
          , step(step > 0 ? step : 5)
          , curr_pool_size(0)
          , num_reserved(0)
      {
          first_free_item = items.end();
          if (!create_pool_object) {
              throw Fmi::Exception(BCP, "Factory method not provided");
          }
      }

      virtual ~WorkerPool()
      {
          shutdown();
          // Wait for all pool objects to be released
          boost::unique_lock<boost::mutex> lock(pm_cond);
          p_cond.wait_for(lock, boost::chrono::seconds(30), [this]() { return num_reserved == 0; });
          if (num_reserved != 0U) {
              std::cerr << METHOD_NAME << ": timed out while waiting for all objects to be released" << std::endl;
              abort();
          }
      }

      void shutdown()
      {
          boost::unique_lock<boost::mutex> lock(pm_cond);
          in_shutdown = true;
          p_cond.notify_all();
      }

      std::shared_ptr<ObjectType> reserve()
      {
          boost::unique_lock<boost::mutex> lock(pm_cond);
          p_cond.wait(
              lock,
              [this] ()
              {
                  return in_shutdown || (first_free_item != items.end()) || (curr_pool_size < max_pool_size);
              });

          if (in_shutdown) {
              throw Fmi::Exception(BCP, "Cannot get object from pool after shutdown is requested").disableLogging();
          }

          if (first_free_item == items.end()) {
              curr_pool_size++;
              if (curr_pool_size > max_reached_pool_size) {
                  max_reached_pool_size = curr_pool_size;
              }
              lock.unlock();  // No nead to keep lock while creating a new object
              PoolItem new_item;
              new_item.next_free = items.end();
              new_item.reserved = false;
              new_item.ptr = create_pool_object();
              if (!new_item.ptr) {
                  throw Fmi::Exception(BCP, "Factory method returned empty shared_ptr");
              }
              lock.lock();
              auto it = items.insert(items.end(), new_item);
              first_free_item = it;
          }

          auto curr_item = first_free_item;
          auto next_free = curr_item->next_free;
          curr_item->next_free = items.end();
          curr_item->reserved = true;
          num_reserved++;
          first_free_item = next_free;
          lock.unlock();

          return std::shared_ptr<ObjectType>(curr_item->ptr.get(), Releaser(*this, curr_item));
      }

      std::size_t get_max_reached_pool_size() const
      {
          boost::unique_lock<boost::mutex> lock(pm_cond);
           return max_reached_pool_size;
      }

      std::size_t get_curr_pool_size() const
      {
          boost::unique_lock<boost::mutex> lock(pm_cond);
          return curr_pool_size;
      }

      std::size_t get_num_reserved() const
      {
          boost::unique_lock<boost::mutex> lock(pm_cond);
          return num_reserved;
      }

      void cancel()
      {
          boost::unique_lock<boost::mutex> lock(pm_cond);
          for (auto& item : items) {
              item.ptr->cancel();
          }
      }

  private:
      void release(typename std::list<PoolItem>::iterator it)
      {
          std::shared_ptr<ObjectType> curr;
          boost::unique_lock<boost::mutex> lock(pm_cond);
          num_reserved--;
          if ((curr_pool_size > initial_pool_size) && (num_reserved + step > curr_pool_size)) {
              curr_pool_size--;
              curr = it->ptr;
              items.erase(it);
          } else {
              it->reserved = false;
              it->next_free = first_free_item;
              first_free_item = it;
              p_cond.notify_one();
          }
          lock.unlock();
          // No need to keep lock to ensure that current object destructio
      }

  private:
      bool in_shutdown;
      std::function<std::shared_ptr<ObjectType>()> create_pool_object;
      std::size_t initial_pool_size;
      std::size_t max_pool_size;
      std::size_t max_reached_pool_size;
      std::size_t step;
      std::size_t curr_pool_size;
      std::size_t num_reserved;
      boost::condition_variable p_cond;
      mutable boost::mutex pm_cond;
      std::list<PoolItem> items;
      typename std::list<PoolItem>::iterator first_free_item;
  };
}
