#pragma once

#include <atomic>
#include <memory>
#include "Exception.h"

namespace Fmi
{
    template <typename Type>
    class AtomicSharedPtr : private std::shared_ptr<Type>
    {
        std::shared_ptr<Type> content;
    public:
        AtomicSharedPtr() = default;

        AtomicSharedPtr(const std::shared_ptr<Type>& ptr)
            : content(ptr)
        {
        }

        AtomicSharedPtr(const AtomicSharedPtr<Type>& ptr)
            : content(std::shared_ptr<Type>(std::atomic_load(ptr)))
        {
        }

        AtomicSharedPtr<Type>& operator=(const AtomicSharedPtr<Type>& ptr)
        {
            std::atomic_store(&content, ptr.load());
            return *this;
        }

        std::shared_ptr<Type> load() const
        {
            return std::atomic_load(&content);
        }

        void store(const std::shared_ptr<Type>& ptr)
        {
            std::atomic_store(&content, ptr);
        }

        void reset()
        {
            std::atomic_store(&content, std::shared_ptr<Type>());
        }

        std::shared_ptr<Type> exchange(std::shared_ptr<Type>& ptr)
        {
            return std::atomic_exchange(&content, ptr);
        }
    };
}