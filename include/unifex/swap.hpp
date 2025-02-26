/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <unifex/config.hpp>
#include <unifex/detail/concept_macros.hpp>
#include <unifex/type_traits.hpp>

#include <unifex/detail/prologue.hpp>

namespace unifex {

template <typename T, typename U, typename = void>
extern bool const is_swappable_with_v;

template <typename T, typename U, typename = void>
extern bool const is_nothrow_swappable_with_v;

namespace _swap {
    void swap();

    template <typename T, typename U>
    decltype(static_cast<void>(swap(UNIFEX_DECLVAL(T&&), UNIFEX_DECLVAL(U&&))), std::true_type{}) _try_adl_swap(int);

    template <typename T, typename U>
    std::false_type _try_adl_swap(long);

    template <typename T, typename U = T>
    inline constexpr bool is_adl_swappable_v = decltype(_swap::_try_adl_swap<T, U>(0))::value;

    struct _fn {
        // Dispatch to user-defined swap found via ADL:
        UNIFEX_TEMPLATE(typename T, typename U)
            (requires is_adl_swappable_v<T, U>)
        constexpr void operator()(T &&t, U &&u) const
            noexcept(noexcept(swap((T &&) t, (U &&) u))) {
            swap((T &&) t, (U &&) u);
        }

        // For intrinsically swappable (i.e., movable) types for which
        // a swap overload cannot be found via ADL, swap by moving.
        UNIFEX_TEMPLATE(typename T)
            (requires (!is_adl_swappable_v<T &>))
        constexpr auto operator()(T &a, T &b) const
            noexcept(noexcept(a = T(static_cast<T&&>(b)))) ->
            decltype(static_cast<void>(a = T(static_cast<T&&>(b)))) {
            T tmp = static_cast<T &&>(a);
            a = static_cast<T &&>(b);
            b = static_cast<T &&>(tmp);
        }

        // For arrays of intrinsically swappable (i.e., movable) types
        // for which a swap overload cannot be found via ADL, swap array
        // elements by moving.
        UNIFEX_TEMPLATE(typename T, typename U, std::size_t N)
            (requires (!is_adl_swappable_v<T (&)[N], U (&)[N]>) &&
                is_swappable_with_v<T &, U &>)
        constexpr void operator()(T (&t)[N], U (&u)[N]) const
            noexcept(is_nothrow_swappable_with_v<T &, U &>) {
            for(std::size_t i = 0; i < N; ++i)
                (*this)(t[i], u[i]);
        }
    };
} // namespace _swap

namespace _swap_cpo {
inline constexpr _swap::_fn swap {};
} // namespace _swap_cpo
using namespace _swap_cpo;

template <typename, typename, typename>
inline constexpr bool is_swappable_with_v = false;

template <typename T, typename U>
inline constexpr bool is_swappable_with_v<
    T, U, decltype(unifex::swap(UNIFEX_DECLVAL(T&&), UNIFEX_DECLVAL(U&&)))> = true;

template <typename, typename, typename>
inline constexpr bool is_nothrow_swappable_with_v = false;

template <typename T, typename U>
inline constexpr bool is_nothrow_swappable_with_v<
    T, U, decltype(unifex::swap(UNIFEX_DECLVAL(T&&), UNIFEX_DECLVAL(U&&)))> = 
    noexcept(swap(UNIFEX_DECLVAL(T&&), UNIFEX_DECLVAL(U&&)));
} // namespace unifex

#include <unifex/detail/epilogue.hpp>
