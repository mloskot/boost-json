//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STATIC_STACK_HPP
#define BOOST_JSON_DETAIL_STATIC_STACK_HPP

#include <boost/json/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <cstring>
#include <new>
#include <utility>

namespace boost {
namespace json {
namespace detail {

// NOTE T must be trivial
template<class T, std::size_t N>
class static_stack
{
    T* top_ = t_;
    T* begin_ = t_;
    T* end_ = t_ + N;
    storage_ptr sp_;
    union
    {
        T t_[N];
        int unused;
    };

public:
    using value_type = T;

    ~static_stack()
    {
        if(begin_ != t_)
            sp_->deallocate(begin_,
                capacity() * sizeof(T),
                alignof(T));
    }

    explicit
    static_stack(
        storage_ptr sp = {}) noexcept
        : sp_(std::move(sp))
    {
    }

    std::size_t
    size() const noexcept
    {
        return top_ - begin_;
    }

    T&
    operator*() noexcept
    {
        return *(top_ - 1);
    }

    T const&
    operator*() const noexcept
    {
        return *(top_ - 1);
    }

    T*
    operator->() noexcept
    {
        return top_ - 1;
    }

    T const*
    operator->() const noexcept
    {
        return top_ - 1;
    }

    bool
    empty() const noexcept
    {
        return top_ == begin_;
    }

    void
    clear() noexcept
    {
        top_ = begin_;
    }

    void
    push(T const& t)
    {
        if(top_ == end_)
            grow();
        *top_ = t;
        ++top_;
    }

    template<class... Args>
    void
    emplace(Args&&... args)
    {
        if(top_ == end_)
            grow();
        ::new(top_) T(
            std::forward<Args>(args)...);
        ++top_;
    }

    void
    pop() noexcept
    {
        --top_;
    }

private:
    std::size_t
    capacity() const noexcept
    {
        return end_ - begin_;
    }

    void
    grow()
    {
        auto const n = 2 * capacity();
        auto const begin =
            reinterpret_cast<T*>(
                sp_->allocate(
                    n * sizeof(T),
                    alignof(T)));
        if(! empty())
        {
            std::memcpy(begin, begin_,
                size() * sizeof(T));
            top_ = begin + (top_ - begin_);
            if(begin_ != t_)
                sp_->deallocate(begin_,
                    capacity() * sizeof(T),
                    alignof(T));
        }
        else
        {
            top_ = begin;
        }
        begin_ = begin;
        end_ = begin_ + n;
    }
};

} // detail
} // json
} // boost

#endif
