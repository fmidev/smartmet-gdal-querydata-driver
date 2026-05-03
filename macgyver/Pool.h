#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <list>
#include <tuple>
#include <exception>
#include <boost/chrono.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include "AsyncTaskGroup.h"
#include "DateTime.h"
#include "StringConversion.h"
#include "TypeName.h"
#include "TypeTraits.h"

namespace Fmi
{
    enum class PoolInitType
    {
        Sequential,
        Parallel
    };

    /**
     * @brief A thread-safe object pool implementation
     *
     * This class implements a thread-safe object pool for managing reusable objects.
     * It allows pre-allocation of a specified number of objects and can grow
     * dynamically up to a maximum limit.
     *
     * The pool can be initialized either sequentially or in parallel, depending on the
     * parameters from Args are passed to the ItemType constructor when creating new items.
     *
     * All constructor arguments must be copy constructable as they are stored in the pool instance.
     * Note that if reference of type is provided, argument itself must be copyable as well
     * (as references are removed when storing in tuple).
     *
     * Template parameters:
     * - InitType: Specifies the initialization type (sequential or parallel).
     * - ItemType: The type of objects managed by the pool.
     * - Args: Additional arguments passed to the ItemType constructor or factory method when creating new items.
     */
    template <PoolInitType InitType, typename ItemType, typename... Args>
    class Pool
    {
        static_assert(are_all_parameters_copyable<typename std::decay<Args>::type...>(),
            "All Args parameters must be copyable");

        struct ItemRec
        {
            /**
             * @brief Actual pool item
             */
            std::unique_ptr<ItemType> data;

            /**
             * @brief Pointer to the next free item record in the pool.
             *
             * Value is nullptr when one of following is true:
             * - Item is currently in use
             * - Item is the last item in the free item chain
             */
            ItemRec* next = nullptr;

            /**
            *  @brief Indicates whether the item is currently in use
            *
            *  nextptr is not sufficient as it is nullptr also for the last free item
            */
            bool in_use = false;

            ItemRec(std::unique_ptr<ItemType>&& item)
                : data(std::move(item))
            {
            }

            ItemRec(const ItemRec&) = delete;
            ItemRec& operator=(const ItemRec&) = delete;
            ItemRec(ItemRec&&) = default;
            ItemRec& operator=(ItemRec&&) = default;
        };

    public:
        class Ptr : private std::unique_ptr<ItemType, std::function<void(ItemType*)>>
        {
            friend class Pool;
            using base = std::unique_ptr<ItemType, std::function<void(ItemType*)>>;

            std::weak_ptr<bool> alive_flag;
            Ptr& self;

            Ptr(ItemType* ptr, std::function<void(ItemType*)> release, std::shared_ptr<bool>& alive_flag)
                : std::unique_ptr<ItemType, std::function<void(ItemType*)>>(ptr, release)
                , alive_flag(alive_flag)
                , self(*this)
            {
            }

        public:
            Ptr(Ptr&& other)
                : base(std::move(other))
                , alive_flag(std::move(other.alive_flag))
                , self(*this)
            {
            }

            virtual ~Ptr()
            {
                if (!this->base::get())
                    return;

                // Release is called automatically by unique_ptr destructor
                // We need however handle case when pool is already destroyed
                if (alive_flag.expired())
                {
                    // Pool is already destroyed, do not put item back to pool
                    // but delete the item to avoid memory leak

                    std::cerr << "Warning: Pool is already destroyed, deleting item of type "
                               << Fmi::demangle_cpp_type_name(typeid(ItemType).name())
                               << " at " << (void*)this->base::get()
                               << std::endl;
                    auto* ptr = this->release();
                    delete ptr;
                }
            }

            // We do not want to expose all std::unique_ptr<> methods for example release()
            inline operator ItemType*() const { return this->base::get(); }
            inline operator bool() const { return this->base::get() != nullptr; }
            inline ItemType* operator->() const { return this->base::get(); }
            inline ItemType* get() const { return this->base::get(); }
            inline void reset() { if (this->base::get() && !alive_flag.expired()) this->base::reset(); }
        };

        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;
        Pool(Pool&&) = default;
        Pool& operator=(Pool&&) = default;

        /**
         * @brief Constructs a Pool with specified start and maximum sizes
         *
         * @param start_size The initial number of items to pre-allocate in the pool (minimum 2)
         * @param max_size The maximum number of items the pool can grow to (must be >= start_size)
         * @param args Additional arguments passed to the ItemType constructor when creating new items
         *
         * Note that if max_size is larger than start_size, related objects must exist during the whole pool lifetime
         * in case when they are passed to constructor as references or pointers.
         */
        Pool(std::size_t start_size, std::size_t max_size, Args... args)
        try
            : start_size(std::max(std::size_t(2), start_size))
            , max_size(std::max(start_size, max_size))
            , constructor_args(typename std::decay<Args>::type(args)...)
            , alive_flag(std::make_shared<bool>(true))
            , current_size(0)
            , in_use_count(0)
            , createItemCb([this]()
              { return std::make_unique<ItemType>(std::get<typename std::decay<Args>::type>(constructor_args)...); })
        {
            init(args...);
        }
        catch (...)
        {
            throw Fmi::Exception::Trace(BCP, "Error initializing Pool of type " +
                Fmi::demangle_cpp_type_name(typeid(ItemType).name()));
        }

        /**
         * @brief Constructs a Pool with specified start and maximum sizes and a custom item creation callback
         *
         * @param createItemCb A callback function that creates and returns a unique_ptr to a new ItemType instance
         * @param start_size The initial number of items to pre-allocate in the pool (minimum 2)
         * @param max_size The maximum number of items the pool can grow to (must be >= start_size)
         * @param args Additional arguments passed to the createItemCb when creating new items
         *
         * Note that if max_size is larger than start_size, related objects must exist during the whole pool lifetime
         * in case when they are passed to factory method as references or pointers.
         */
        Pool(
            std::function<std::unique_ptr<ItemType>(Args... args)> createItemCb_,
            std::size_t start_size,
            std::size_t max_size,
            Args... args)
        try
            : start_size(std::max(std::size_t(2), start_size))
            , max_size(std::max(start_size, max_size))
            , constructor_args(typename std::decay<Args>::type(args)...)
            , alive_flag(std::make_shared<bool>(true))
            , current_size(0)
            , in_use_count(0)
            , createItemCb([this, createItemCb_]()
              { return createItemCb_(std::get<typename std::decay<Args>::type>(constructor_args)...); })
        {
            init(args...);
        }
        catch (...)
        {
            throw Fmi::Exception::Trace(BCP, "Error initializing Pool of type " +
                Fmi::demangle_cpp_type_name(typeid(ItemType).name()));
        }

        virtual ~Pool()
        {
            // Hold mutex while resetting alive_flag to prevent race condition
            // with get() and Ptr constructor accessing alive_flag
            std::size_t count = 0;
            std::size_t items_in_use = 0;
            std::size_t total_size = 0;

            {
                boost::unique_lock<boost::mutex> lock(mutex);
                alive_flag.reset();

                for (auto& item_rec : pool_data)
                {
                    if (item_rec.in_use)
                    {
                        // Drop the item data that is still in use. Ownership is transferred to the Pool<>::Ptr instance
                        // As result, the item will not be returned to the pool but deleted when Ptr is destroyed
                        // See Ptr::~Ptr() for details
                        item_rec.data.release();
                        count++;
                    }
                }

                items_in_use = in_use_count;
                total_size = current_size;
            }

            if (items_in_use)
            {
                // There are some items in use. Output message about it to stderr
                std::cerr << "Pool of " << Fmi::demangle_cpp_type_name(typeid(ItemType).name()) << " is being destroyed while items are still in use" << std::endl;
                std::cerr << "Items in use: " << items_in_use << " (counted: " << count << ")"  << std::endl;
                std::cerr << "Total pool size: " << total_size << std::endl;
            }
        }

        Ptr get()
        {
            ItemRec* rec = acquire(std::nullopt);
            return Ptr(
                rec->data.get(),
                [this, rec](ItemType* ptr) { this->release(rec->data.get(), rec); },
                alive_flag);
        }

        Ptr get(const Fmi::TimeDuration& timeout)
        {
            ItemRec* rec = acquire(timeout);
            return Ptr(
                rec->data.get(),
                [this, rec](ItemType* ptr) { this->release(rec->data.get(), rec); },
                alive_flag);
        }

        std::size_t size() const
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            return current_size;
        }

        std::size_t in_use() const
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            return in_use_count;
        }

        void dumpInfo(std::ostream& os)
        {
            int count = 0;
            boost::unique_lock<boost::mutex> lock(mutex);
            os << "Pool info for items of type " << Fmi::demangle_cpp_type_name(typeid(ItemType).name()) << std::endl;
            os << "Total items: " << pool_data.size() << std::endl;
            os << "In use items: " << in_use_count << std::endl;
            os << "Top free item: " << (void*)top << std::endl;
            for (const auto& item : pool_data)
            {
                os << "Item[" << ++count << "]: " << (void*)&item
                   << ", next: " << (void*)item.next << std::endl;
            }
        }

    private:

        void init(Args... args)
        {
            const auto grow = [this]() {
                std::unique_ptr<ItemType> new_item(createItemCb());
                boost::unique_lock<boost::mutex> lock(mutex);
                // Add new item to the pool. List iterators are not invalidated by growing list
                ItemRec& item_rec = pool_data.emplace_back(ItemRec(std::move(new_item)));
                item_rec.next = top;
                top = &item_rec;
                current_size++;
                next_current_size++;
            };

            if constexpr (InitType == PoolInitType::Sequential)
            {
                for (std::size_t i = 0; i < start_size; ++i)
                {
                    grow();
                }
            }
            else if constexpr (InitType == PoolInitType::Parallel)
            {
                std::atomic<bool> have_errors = false;
                std::atomic<int> num_errors = 0;

                const auto init_task_error = [this, &have_errors, &num_errors]
                    (const std::string& name)
                {
                    bool trueVal = true;
                    num_errors++;

                    // Report only the first error in details
                    if (have_errors.exchange(trueVal))
                        return;

                    std::exception_ptr eptr = std::current_exception();
                    if (!eptr)
                        return;

                    try
                    {
                        rethrow_exception(eptr);
                    }
                    catch(...)
                    {
                        auto error = Fmi::Exception::Trace(BCP, "Error initializing pool item of type " +
                                Fmi::demangle_cpp_type_name(typeid(ItemType).name()))
                            .addParameter("Task name", name);
                        //std::cout << error << std::endl;
                        throw error;
                    }
                };

                AsyncTaskGroup task_group;
                task_group.stop_on_error(true);

                for (std::size_t i = 0; i < start_size; ++i)
                {
                    task_group.add("pool_item_init[" + Fmi::to_string(i+1) + "]", grow);
                }

                task_group.on_task_error(init_task_error);
                task_group.wait();
            }
            else
            {
                throw Fmi::Exception(BCP, "Unsupported PoolInitType value");
            }
        }

        ItemRec* acquire(std::optional<Fmi::TimeDuration> timeout)
        {
            if (timeout && timeout->is_special())
                throw Fmi::Exception(BCP, "Special time values not supported as timeout value");

            const auto fetch_top = [this]() -> ItemRec*
            {
                assert(top != nullptr);
                ItemRec* item_rec = top;
                top = top->next;
                item_rec->next = nullptr;
                item_rec->in_use = true;
                in_use_count++;
                return item_rec;
            };

            boost::unique_lock<boost::mutex> lock(mutex);

            // Check if pool is being destroyed after acquiring mutex
            // to prevent use after destruction
            if (!alive_flag)
            {
                throw Fmi::Exception(BCP, "Pool is being destroyed");
            }

            if (top)
            {
                //---------------------------------------------------------------------
                // Item is available, use it
                //---------------------------------------------------------------------
                return fetch_top();
            }
            else if (next_current_size < max_size)
            {
                //---------------------------------------------------------------------
                // Item is not available, max limit not exceeded, create a new item
                //---------------------------------------------------------------------
                // Update count while mutex is still locked to avoid growing over
                // max_size also in case of concurrent calls
                next_current_size++;
                // Unlock mutex while creating new item (it may take some time for example
                // of database connection)
                lock.unlock();
                std::unique_ptr<ItemType> new_item;
                try
                {
                    new_item = createItemCb();
                }
                catch (...)
                {
                    // Creating new item failed. Decrement count and rethrow
                    // the exception. This really only matters when pool expansion
                    // is attempted.
                    lock.lock();
                    next_current_size--;
                    throw;
                }
                // Update top and in_use_count while mutex is locked
                lock.lock();
                // Add new item to the pool. List iterators are not invalidated by growing list
                ItemRec& item_rec = pool_data.emplace_back(ItemRec(std::move(new_item)));
                // One could optimize this part by avoiding putting new item in free
                // item chain, but it would complicate the logic
                current_size++;
                item_rec.next = top;
                top = &item_rec;
                // No need to notify waiting threads as we are going to use the new item directly
                return fetch_top();
            }
            else
            {
                if (timeout)
                {
                    int ms = static_cast<int>(timeout->total_milliseconds());
                    if (!cond_var.wait_for(lock, boost::chrono::milliseconds(ms), [this] { return top != nullptr; }))
                    {
                        throw Fmi::Exception(BCP, "Timeout while waiting for pool item");
                    }
                }
                else
                {
                    cond_var.wait(lock, [this] { return top != nullptr; });
                }

                if (top)
                {
                    //-----------------------------------------------------------------
                    // Item is available, use it
                    //-----------------------------------------------------------------
                    return fetch_top();
                }

                // Should not be here
                throw Fmi::Exception(BCP, "Internal error");
            }
        }

        void release(ItemType* item, ItemRec* item_rec)
        {
            if (!item)
                return;

            // Check if pool is still alive before accessing members
            // If pool is destroyed, the item will be deleted by Ptr destructor
            if (!alive_flag)
                return;

            // Cannot use Pool members directly as this is a static method
            // Can potentially cause race condition when pool is being destroyed
            boost::unique_lock<boost::mutex> lock(mutex);

            // Double-check after acquiring lock
            if (!alive_flag)
                return;

            item_rec->next = top;
            item_rec->in_use = false;
            top = item_rec;
            in_use_count--;
            cond_var.notify_one();
        }

        const std::size_t start_size;
        const std::size_t max_size;
        const std::tuple<typename std::decay<Args>::type...> constructor_args;

        std::shared_ptr<bool> alive_flag;

        /**
         * @brief Current number of items in the pool
         */
        std::size_t current_size = 0;

        /**
         * @brief Next size of the pool when growing (used to control max_size limit)
         *
         * Intended to avoid growing over max_size in case of concurrent calls to acquire()
         */
        std::size_t next_current_size = 0;
        std::size_t in_use_count = 0;

        mutable boost::mutex mutex;
        boost::condition_variable cond_var;

        std::function<std::unique_ptr<ItemType>()> createItemCb;

        ItemRec* top = nullptr;
        std::list<ItemRec> pool_data;
    };
}
