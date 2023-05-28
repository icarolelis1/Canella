#ifndef ENTT_ENTITY_VIEW_HPP
#define ENTT_ENTITY_VIEW_HPP

#include <array>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>
#include "../config/config.h"
#include "../core/iterator.hpp"
#include "../core/type_traits.hpp"
#include "entity.hpp"
#include "fwd.hpp"

namespace entt {

/**
 * @cond TURN_OFF_DOXYGEN
 * Internal details not to be documented.
 */

namespace internal {

template<typename... Args, typename Type, std::size_t N>
auto filter_as_tuple(const std::array<const Type *, N> &filter) noexcept {
    return std::apply([](const auto *...curr) { return std::make_tuple(static_cast<Args *>(const_cast<constness_as_t<Type, Args> *>(curr))...); }, filter);
}

template<typename Type, std::size_t N>
[[nodiscard]] auto none_of(const std::array<const Type *, N> &filter, const typename Type::entity_type entt) noexcept {
    return std::apply([entt](const auto *...curr) { return (!curr->contains(entt) && ...); }, filter);
}

template<typename Type, std::size_t Get, std::size_t Exclude>
class view_iterator final {
    using iterator_type = typename Type::const_iterator;

    [[nodiscard]] bool valid() const noexcept {
        return ((Get != 0u) || (*it != tombstone))
               && std::apply([entt = *it](const auto *...curr) { return (curr->contains(entt) && ...); }, pools)
               && none_of(filter, *it);
    }

public:
    using value_type = typename iterator_type::value_type;
    using pointer = typename iterator_type::pointer;
    using reference = typename iterator_type::reference;
    using difference_type = typename iterator_type::difference_type;
    using iterator_category = std::forward_iterator_tag;

    constexpr view_iterator() noexcept
        : it{},
          last{},
          pools{},
          filter{} {}

    view_iterator(iterator_type curr, iterator_type to, std::array<const Type *, Get> value, std::array<const Type *, Exclude> excl) noexcept
        : it{curr},
          last{to},
          pools{value},
          filter{excl} {
        while(it != last && !valid()) {
            ++it;
        }
    }

    view_iterator &operator++() noexcept {
        while(++it != last && !valid()) {}
        return *this;
    }

    view_iterator operator++(int) noexcept {
        view_iterator orig = *this;
        return ++(*this), orig;
    }

    [[nodiscard]] pointer operator->() const noexcept {
        return &*it;
    }

    [[nodiscard]] reference operator*() const noexcept {
        return *operator->();
    }

    template<typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
    friend constexpr bool operator==(const view_iterator<LhsType, LhsArgs...> &, const view_iterator<RhsType, RhsArgs...> &) noexcept;

private:
    iterator_type it;
    iterator_type last;
    std::array<const Type *, Get> pools;
    std::array<const Type *, Exclude> filter;
};

template<typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
[[nodiscard]] constexpr bool operator==(const view_iterator<LhsType, LhsArgs...> &lhs, const view_iterator<RhsType, RhsArgs...> &rhs) noexcept {
    return lhs.it == rhs.it;
}

template<typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
[[nodiscard]] constexpr bool operator!=(const view_iterator<LhsType, LhsArgs...> &lhs, const view_iterator<RhsType, RhsArgs...> &rhs) noexcept {
    return !(lhs == rhs);
}

template<typename It, typename... Type>
struct extended_view_iterator final {
    using iterator_type = It;
    using difference_type = std::ptrdiff_t;
    using value_type = decltype(std::tuple_cat(std::make_tuple(*std::declval<It>()), std::declval<Type>().get_as_tuple({})...));
    using pointer = input_iterator_pointer<value_type>;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;

    constexpr extended_view_iterator()
        : it{},
          pools{} {}

    extended_view_iterator(It from, std::tuple<Type *...> value)
        : it{from},
          pools{value} {}

    extended_view_iterator &operator++() noexcept {
        return ++it, *this;
    }

    extended_view_iterator operator++(int) noexcept {
        extended_view_iterator orig = *this;
        return ++(*this), orig;
    }

    [[nodiscard]] reference operator*() const noexcept {
        return std::apply([entt = *it](auto *...curr) { return std::tuple_cat(std::make_tuple(entt), curr->get_as_tuple(entt)...); }, pools);
    }

    [[nodiscard]] pointer operator->() const noexcept {
        return operator*();
    }

    [[nodiscard]] constexpr iterator_type base() const noexcept {
        return it;
    }

    template<typename... Lhs, typename... Rhs>
    friend bool constexpr operator==(const extended_view_iterator<Lhs...> &, const extended_view_iterator<Rhs...> &) noexcept;

private:
    It it;
    std::tuple<Type *...> pools;
};

template<typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator==(const extended_view_iterator<Lhs...> &lhs, const extended_view_iterator<Rhs...> &rhs) noexcept {
    return lhs.it == rhs.it;
}

template<typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator!=(const extended_view_iterator<Lhs...> &lhs, const extended_view_iterator<Rhs...> &rhs) noexcept {
    return !(lhs == rhs);
}

} // namespace internal

/**
 * Internal details not to be documented.
 * @endcond
 */

/**
 * @brief View implementation.
 *
 * Primary template isn't defined on purpose. All the specializations give a
 * compile-time error, but for a few reasonable cases.
 */
template<typename, typename, typename>
class basic_view;

/**
 * @brief Multi component view.
 *
 * Multi component views iterate over those entities that are at least in the
 * given storage. During initialization, a multi component view looks at the
 * number of entities available for each component and uses the smallest set in
 * order to get a performance boost when iterating.
 *
 * @b Important
 *
 * Iterators aren't invalidated if:
 *
 * * New elements are added to the storage.
 * * The entity currently pointed is modified (for example, components are added
 *   or removed from it).
 * * The entity currently pointed is destroyed.
 *
 * In all other cases, modifying the pools iterated by the view in any way
 * invalidates all the iterators and using them results in undefined behavior.
 *
 * @tparam Get Types of storage iterated by the view.
 * @tparam Exclude Types of storage used to filter the view.
 */
template<typename... Get, typename... Exclude>
class basic_view<get_t<Get...>, exclude_t<Exclude...>> {
    using base_type = std::common_type_t<typename Get::base_type..., typename Exclude::base_type...>;
    using underlying_type = typename base_type::entity_type;

    template<typename, typename, typename>
    friend class basic_view;

    template<typename Type>
    static constexpr std::size_t index_of = type_list_index_v<std::remove_const_t<Type>, type_list<typename Get::value_type..., typename Exclude::value_type...>>;

    [[nodiscard]] auto opaque_check_set() const noexcept {
        std::array<const common_type *, sizeof...(Get) - 1u> other{};
        std::apply([&other, pos = 0u, view = view](const auto *...curr) mutable { ((curr == view ? void() : void(other[pos++] = curr)), ...); }, pools);
        return other;
    }

    template<std::size_t Curr, std::size_t Other, typename... Args>
    [[nodiscard]] auto dispatch_get(const std::tuple<underlying_type, Args...> &curr) const {
        if constexpr(Curr == Other) {
            return std::forward_as_tuple(std::get<Args>(curr)...);
        } else {
            return storage<Other>().get_as_tuple(std::get<0>(curr));
        }
    }

    template<std::size_t Curr, typename Func, std::size_t... Index>
    void each(Func &func, std::index_sequence<Index...>) const {
        for(const auto curr: storage<Curr>().each()) {
            if(const auto entt = std::get<0>(curr); ((sizeof...(Get) != 1u) || (entt != tombstone)) && ((Curr == Index || storage<Index>().contains(entt)) && ...) && internal::none_of(filter, entt)) {
                if constexpr(is_applicable_v<Func, decltype(std::tuple_cat(std::tuple<entity_type>{}, std::declval<basic_view>().get({})))>) {
                    std::apply(func, std::tuple_cat(std::make_tuple(entt), dispatch_get<Curr, Index>(curr)...));
                } else {
                    std::apply(func, std::tuple_cat(dispatch_get<Curr, Index>(curr)...));
                }
            }
        }
    }

    template<typename Func, std::size_t... Index>
    void pick_and_each(Func &func, std::index_sequence<Index...> seq) const {
        ((&storage<Index>() == view ? each<Index>(func, seq) : void()), ...);
    }

public:
    /*! @brief Underlying entity identifier. */
    using entity_type = underlying_type;
    /*! @brief Unsigned integer type. */
    using size_type = std::size_t;
    /*! @brief Common type among all storage types. */
    using common_type = base_type;
    /*! @brief Bidirectional iterator type. */
    using iterator = internal::view_iterator<common_type, sizeof...(Get) - 1u, sizeof...(Exclude)>;
    /*! @brief Iterable view type. */
    using iterable = iterable_adaptor<internal::extended_view_iterator<iterator, Get...>>;

    /*! @brief Default constructor to use to create empty, invalid views. */
    basic_view() noexcept
        : pools{},
          filter{},
          view{} {}

    /**
     * @brief Constructs a multi-type view from a set of storage classes.
     * @param value The storage for the types to iterate.
     * @param excl The storage for the types used to filter the view.
     */
    basic_view(Get &...value, Exclude &...excl) noexcept
        : pools{&value...},
          filter{&excl...},
          view{std::get<0>(pools)} {
        ((view = value.size() < view->size() ? &value : view), ...);
    }

    /**
     * @brief Constructs a multi-type view from a set of storage classes.
     * @param value The storage for the types to iterate.
     * @param excl The storage for the types used to filter the view.
     */
    basic_view(std::tuple<Get &...> value, std::tuple<Exclude &...> excl = {}) noexcept
        : basic_view{std::make_from_tuple<basic_view>(std::tuple_cat(value, excl))} {}

    /**
     * @brief Creates a new view driven by a given component in its iterations.
     * @tparam Type Type of component used to drive the iteration.
     * @return A new view driven by the given component in its iterations.
     */
    template<typename Type>
    [[nodiscard]] basic_view use() const noexcept {
        return use<index_of<Type>>();
    }

    /**
     * @brief Creates a new view driven by a given component in its iterations.
     * @tparam Index Index of the component used to drive the iteration.
     * @return A new view driven by the given component in its iterations.
     */
    template<std::size_t Index>
    [[nodiscard]] basic_view use() const noexcept {
        basic_view other{*this};
        other.view = &storage<Index>();
        return other;
    }

    /**
     * @brief Updates the internal leading view if required.
     * @return A newly created and internally optimized view.
     */
    [[nodiscard]] basic_view refresh() const noexcept {
        return std::apply([](auto *...elem) { return basic_view{*elem...}; }, std::tuple_cat(pools, internal::filter_as_tuple<Exclude...>(filter)));
    }

    /**
     * @brief Returns the leading storage of a view.
     * @return The leading storage of the view.
     */
    [[nodiscard]] const common_type &handle() const noexcept {
        return *view;
    }

    /**
     * @brief Returns the storage for a given component type.
     * @tparam Type Type of component of which to return the storage.
     * @return The storage for the given component type.
     */
    template<typename Type>
    [[nodiscard]] decltype(auto) storage() const noexcept {
        return storage<index_of<Type>>();
    }

    /**
     * @brief Returns the storage for a given index.
     * @tparam Index Index of the storage to return.
     * @return The storage for the given index.
     */
    template<std::size_t Index>
    [[nodiscard]] decltype(auto) storage() const noexcept {
        constexpr auto offset = sizeof...(Get);

        if constexpr(Index < offset) {
            return *std::get<Index>(pools);
        } else {
            return *std::get<Index - offset>(internal::filter_as_tuple<Exclude...>(filter));
        }
    }

    /**
     * @brief Estimates the number of entities iterated by the view.
     * @return Estimated number of entities iterated by the view.
     */
    [[nodiscard]] size_type size_hint() const noexcept {
        return handle().size();
    }

    /**
     * @brief Returns an iterator to the first entity of the view.
     *
     * The returned iterator points to the first entity of the view. If the view
     * is empty, the returned iterator will be equal to `end()`.
     *
     * @return An iterator to the first entity of the view.
     */
    [[nodiscard]] iterator begin() const noexcept {
        return iterator{handle().begin(), handle().end(), opaque_check_set(), filter};
    }

    /**
     * @brief Returns an iterator that is past the last entity of the view.
     *
     * The returned iterator points to the entity following the last entity of
     * the view. Attempting to dereference the returned iterator results in
     * undefined behavior.
     *
     * @return An iterator to the entity following the last entity of the view.
     */
    [[nodiscard]] iterator end() const noexcept {
        return iterator{handle().end(), handle().end(), opaque_check_set(), filter};
    }

    /**
     * @brief Returns the first entity of the view, if any.
     * @return The first entity of the view if one exists, the null entity
     * otherwise.
     */
    [[nodiscard]] entity_type front() const noexcept {
        const auto it = begin();
        return it != end() ? *it : null;
    }

    /**
     * @brief Returns the last entity of the view, if any.
     * @return The last entity of the view if one exists, the null entity
     * otherwise.
     */
    [[nodiscard]] entity_type back() const noexcept {
        auto it = handle().rbegin();
        for(const auto last = handle().rend(); it != last && !contains(*it); ++it) {}
        return it == handle().rend() ? null : *it;
    }

    /**
     * @brief Finds an entity.
     * @param entt A valid identifier.
     * @return An iterator to the given entity if it's found, past the end
     * iterator otherwise.
     */
    [[nodiscard]] iterator find(const entity_type entt) const noexcept {
        return contains(entt) ? iterator{handle().find(entt), handle().end(), opaque_check_set(), filter} : end();
    }

    /**
     * @brief Returns the components assigned to the given entity.
     * @param entt A valid identifier.
     * @return The components assigned to the given entity.
     */
    [[nodiscard]] decltype(auto) operator[](const entity_type entt) const {
        return get(entt);
    }

    /**
     * @brief Checks if a view is properly initialized.
     * @return True if the view is properly initialized, false otherwise.
     */
    [[nodiscard]] explicit operator bool() const noexcept {
        return view != nullptr;
    }

    /**
     * @brief Checks if a view contains an entity.
     * @param entt A valid identifier.
     * @return True if the view contains the given entity, false otherwise.
     */
    [[nodiscard]] bool contains(const entity_type entt) const noexcept {
        return std::apply([entt](const auto *...curr) { return (curr->contains(entt) && ...); }, pools) && internal::none_of(filter, entt);
    }

    /**
     * @brief Returns the components assigned to the given entity.
     *
     * @warning
     * Attempting to use an entity that doesn't belong to the view results in
     * undefined behavior.
     *
     * @tparam Type Type of the component to get.
     * @tparam Other Other types of components to get.
     * @param entt A valid identifier.
     * @return The components assigned to the entity.
     */
    template<typename Type, typename... Other>
    [[nodiscard]] decltype(auto) get(const entity_type entt) const {
        return get<index_of<Type>, index_of<Other>...>(entt);
    }

    /**
     * @brief Returns the components assigned to the given entity.
     *
     * @sa get
     *
     * @tparam Index Indexes of the components to get.
     * @param entt A valid identifier.
     * @return The components assigned to the entity.
     */
    template<std::size_t... Index>
    [[nodiscard]] decltype(auto) get(const entity_type entt) const {
        if constexpr(sizeof...(Index) == 0) {
            return std::apply([entt](auto *...curr) { return std::tuple_cat(curr->get_as_tuple(entt)...); }, pools);
        } else if constexpr(sizeof...(Index) == 1) {
            return (storage<Index>().get(entt), ...);
        } else {
            return std::tuple_cat(storage<Index>().get_as_tuple(entt)...);
        }
    }

    /**
     * @brief Iterates entities and components and applies the given function
     * object to them.
     *
     * The function object is invoked for each entity. It is provided with the
     * entity itself and a set of references to non-empty components. The
     * _constness_ of the components is as requested.<br/>
     * The signature of the function must be equivalent to one of the following
     * forms:
     *
     * @code{.cpp}
     * void(const entity_type, Type &...);
     * void(Type &...);
     * @endcode
     *
     * @tparam Func Type of the function object to invoke.
     * @param func A valid function object.
     */
    template<typename Func>
    void each(Func func) const {
        pick_and_each(func, std::index_sequence_for<Get...>{});
    }

    /**
     * @brief Returns an iterable object to use to _visit_ a view.
     *
     * The iterable object returns a tuple that contains the current entity and
     * a set of references to its non-empty components. The _constness_ of the
     * components is as requested.
     *
     * @return An iterable object to use to _visit_ the view.
     */
    [[nodiscard]] iterable each() const noexcept {
        return {internal::extended_view_iterator{begin(), pools}, internal::extended_view_iterator{end(), pools}};
    }

    /**
     * @brief Combines two views in a _more specific_ one (friend function).
     * @tparam OGet Component list of the view to combine with.
     * @tparam OExclude Filter list of the view to combine with.
     * @param other The view to combine with.
     * @return A more specific view.
     */
    template<typename... OGet, typename... OExclude>
    [[nodiscard]] auto operator|(const basic_view<get_t<OGet...>, exclude_t<OExclude...>> &other) const noexcept {
        return std::apply(
            [](auto *...curr) { return basic_view<get_t<Get..., OGet...>, exclude_t<Exclude..., OExclude...>>{*curr...}; },
            std::tuple_cat(pools, other.pools, internal::filter_as_tuple<Exclude...>(filter), internal::filter_as_tuple<OExclude...>(other.filter)));
    }

private:
    std::tuple<Get *...> pools;
    std::array<const common_type *, sizeof...(Exclude)> filter;
    const common_type *view;
};

/**
 * @brief Single component view specialization.
 *
 * Single component views are specialized in order to get a boost in terms of
 * performance. This kind of views can access the underlying data structure
 * directly and avoid superfluous checks.
 *
 * @b Important
 *
 * Iterators aren't invalidated if:
 *
 * * New elements are added to the storage.
 * * The entity currently pointed is modified (for example, components are added
 *   or removed from it).
 * * The entity currently pointed is destroyed.
 *
 * In all other cases, modifying the pool iterated by the view in any way
 * invalidates all the iterators and using them results in undefined behavior.
 *
 * @tparam Get Type of storage iterated by the view.
 */
template<typename Get>
class basic_view<get_t<Get>, exclude_t<>, std::void_t<std::enable_if_t<!Get::traits_type::in_place_delete>>> {
    template<typename, typename, typename>
    friend class basic_view;

public:
    /*! @brief Underlying entity identifier. */
    using entity_type = typename Get::entity_type;
    /*! @brief Unsigned integer type. */
    using size_type = std::size_t;
    /*! @brief Common type among all storage types. */
    using common_type = typename Get::base_type;
    /*! @brief Random access iterator type. */
    using iterator = typename common_type::iterator;
    /*! @brief Reversed iterator type. */
    using reverse_iterator = typename common_type::reverse_iterator;
    /*! @brief Iterable view type. */
    using iterable = decltype(std::declval<Get>().each());

    /*! @brief Default constructor to use to create empty, invalid views. */
    basic_view() noexcept
        : pools{},
          filter{},
          view{} {}

    /**
     * @brief Constructs a single-type view from a storage class.
     * @param value The storage for the type to iterate.
     */
    basic_view(Get &value) noexcept
        : pools{&value},
          filter{},
          view{&value} {}

    /**
     * @brief Constructs a single-type view from a storage class.
     * @param value The storage for the type to iterate.
     */
    basic_view(std::tuple<Get &> value, std::tuple<> = {}) noexcept
        : basic_view{std::get<0>(value)} {}

    /**
     * @brief Returns the leading storage of a view.
     * @return The leading storage of the view.
     */
    [[nodiscard]] const common_type &handle() const noexcept {
        return *view;
    }

    /**
     * @brief Returns the storage for a given component type.
     * @tparam Type Type of component of which to return the storage.
     * @return The storage for the given component type.
     */
    template<typename Type = typename Get::value_type>
    [[nodiscard]] decltype(auto) storage() const noexcept {
        static_assert(std::is_same_v<std::remove_const_t<Type>, typename Get::value_type>, "Invalid component type");
        return storage<0>();
    }

    /**
     * @brief Returns the storage for a given index.
     * @tparam Index Index of the storage to return.
     * @return The storage for the given index.
     */
    template<std::size_t Index>
    [[nodiscard]] decltype(auto) storage() const noexcept {
        return *std::get<Index>(pools);
    }

    /**
     * @brief Returns the number of entities that have the given component.
     * @return Number of entities that have the given component.
     */
    [[nodiscard]] size_type size() const noexcept {
        return handle().size();
    }

    /**
     * @brief Checks whether a view is empty.
     * @return True if the view is empty, false otherwise.
     */
    [[nodiscard]] bool empty() const noexcept {
        return handle().empty();
    }

    /**
     * @brief Returns an iterator to the first entity of the view.
     *
     * The returned iterator points to the first entity of the view. If the view
     * is empty, the returned iterator will be equal to `end()`.
     *
     * @return An iterator to the first entity of the view.
     */
    [[nodiscard]] iterator begin() const noexcept {
        return handle().begin();
    }

    /**
     * @brief Returns an iterator that is past the last entity of the view.
     *
     * The returned iterator points to the entity following the last entity of
     * the view. Attempting to dereference the returned iterator results in
     * undefined behavior.
     *
     * @return An iterator to the entity following the last entity of the view.
     */
    [[nodiscard]] iterator end() const noexcept {
        return handle().end();
    }

    /**
     * @brief Returns an iterator to the first entity of the reversed view.
     *
     * The returned iterator points to the first entity of the reversed view. If
     * the view is empty, the returned iterator will be equal to `rend()`.
     *
     * @return An iterator to the first entity of the reversed view.
     */
    [[nodiscard]] reverse_iterator rbegin() const noexcept {
        return handle().rbegin();
    }

    /**
     * @brief Returns an iterator that is past the last entity of the reversed
     * view.
     *
     * The returned iterator points to the entity following the last entity of
     * the reversed view. Attempting to dereference the returned iterator
     * results in undefined behavior.
     *
     * @return An iterator to the entity following the last entity of the
     * reversed view.
     */
    [[nodiscard]] reverse_iterator rend() const noexcept {
        return handle().rend();
    }

    /**
     * @brief Returns the first entity of the view, if any.
     * @return The first entity of the view if one exists, the null entity
     * otherwise.
     */
    [[nodiscard]] entity_type front() const noexcept {
        return empty() ? null : *begin();
    }

    /**
     * @brief Returns the last entity of the view, if any.
     * @return The last entity of the view if one exists, the null entity
     * otherwise.
     */
    [[nodiscard]] entity_type back() const noexcept {
        return empty() ? null : *rbegin();
    }

    /**
     * @brief Finds an entity.
     * @param entt A valid identifier.
     * @return An iterator to the given entity if it's found, past the end
     * iterator otherwise.
     */
    [[nodiscard]] iterator find(const entity_type entt) const noexcept {
        return contains(entt) ? handle().find(entt) : end();
    }

    /**
     * @brief Returns the identifier that occupies the given position.
     * @param pos Position of the element to return.
     * @return The identifier that occupies the given position.
     */
    [[nodiscard]] entity_type operator[](const size_type pos) const {
        return begin()[pos];
    }

    /**
     * @brief Returns the component assigned to the given entity.
     * @param entt A valid identifier.
     * @return The component assigned to the given entity.
     */
    [[nodiscard]] decltype(auto) operator[](const entity_type entt) const {
        return storage().get(entt);
    }

    /**
     * @brief Checks if a view is properly initialized.
     * @return True if the view is properly initialized, false otherwise.
     */
    [[nodiscard]] explicit operator bool() const noexcept {
        return view != nullptr;
    }

    /**
     * @brief Checks if a view contains an entity.
     * @param entt A valid identifier.
     * @return True if the view contains the given entity, false otherwise.
     */
    [[nodiscard]] bool contains(const entity_type entt) const noexcept {
        return handle().contains(entt);
    }

    /**
     * @brief Returns the component assigned to the given entity.
     *
     * @warning
     * Attempting to use an entity that doesn't belong to the view results in
     * undefined behavior.
     *
     * @tparam Elem Type or index of the component to get.
     * @param entt A valid identifier.
     * @return The component assigned to the entity.
     */
    template<typename Elem>
    [[nodiscard]] decltype(auto) get(const entity_type entt) const {
        static_assert(std::is_same_v<std::remove_const_t<Elem>, typename Get::value_type>, "Invalid component type");
        return get<0>(entt);
    }

    /*! @copydoc get */
    template<std::size_t... Elem>
    [[nodiscard]] decltype(auto) get(const entity_type entt) const {
        if constexpr(sizeof...(Elem) == 0) {
            return storage().get_as_tuple(entt);
        } else {
            return storage<Elem...>().get(entt);
        }
    }

    /**
     * @brief Iterates entities and components and applies the given function
     * object to them.
     *
     * The function object is invoked for each entity. It is provided with the
     * entity itself and a reference to the component if it's a non-empty one.
     * The _constness_ of the component is as requested.<br/>
     * The signature of the function must be equivalent to one of the following
     * forms:
     *
     * @code{.cpp}
     * void(const entity_type, Type &);
     * void(typename Type &);
     * @endcode
     *
     * @note
     * Empty types aren't explicitly instantiated and therefore they are never
     * returned during iterations.
     *
     * @tparam Func Type of the function object to invoke.
     * @param func A valid function object.
     */
    template<typename Func>
    void each(Func func) const {
        if constexpr(is_applicable_v<Func, decltype(*each().begin())>) {
            for(const auto pack: each()) {
                std::apply(func, pack);
            }
        } else if constexpr(Get::traits_type::page_size == 0u) {
            for(size_type pos{}, last = size(); pos < last; ++pos) {
                func();
            }
        } else {
            for(auto &&component: storage()) {
                func(component);
            }
        }
    }

    /**
     * @brief Returns an iterable object to use to _visit_ a view.
     *
     * The iterable object returns a tuple that contains the current entity and
     * a reference to its component if it's a non-empty one. The _constness_ of
     * the component is as requested.
     *
     * @return An iterable object to use to _visit_ the view.
     */
    [[nodiscard]] iterable each() const noexcept {
        return storage().each();
    }

    /**
     * @brief Combines two views in a _more specific_ one (friend function).
     * @tparam OGet Component list of the view to combine with.
     * @tparam OExclude Filter list of the view to combine with.
     * @param other The view to combine with.
     * @return A more specific view.
     */
    template<typename... OGet, typename... OExclude>
    [[nodiscard]] auto operator|(const basic_view<get_t<OGet...>, exclude_t<OExclude...>> &other) const noexcept {
        return std::apply(
            [](auto *...curr) { return basic_view<get_t<Get, OGet...>, exclude_t<OExclude...>>{*curr...}; },
            std::tuple_cat(pools, other.pools, internal::filter_as_tuple<OExclude...>(other.filter)));
    }

private:
    std::tuple<Get *> pools;
    std::array<const common_type *, 0u> filter;
    const common_type *view;
};

/**
 * @brief Deduction guide.
 * @tparam Type Type of storage classes used to create the view.
 * @param storage The storage for the types to iterate.
 */
template<typename... Type>
basic_view(Type &...storage) -> basic_view<get_t<Type...>, exclude_t<>>;

/**
 * @brief Deduction guide.
 * @tparam Get Types of components iterated by the view.
 * @tparam Exclude Types of components used to filter the view.
 */
template<typename... Get, typename... Exclude>
basic_view(std::tuple<Get &...>, std::tuple<Exclude &...> = {}) -> basic_view<get_t<Get...>, exclude_t<Exclude...>>;

} // namespace entt

#endif
