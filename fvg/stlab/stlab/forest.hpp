/*
    Copyright 2013 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
/**************************************************************************************************/

#ifndef STLAB_FOREST_HPP
#define STLAB_FOREST_HPP

/**************************************************************************************************/

#include <cassert>
#include <cstddef>
#include <iterator>
#include <functional>

#include <stlab/algorithm/reverse.hpp>
#include <stlab/iterator/set_next.hpp>

/**************************************************************************************************/

namespace stlab {

/**************************************************************************************************/

/*
    NOTE (sparent) : These are the edge index value - trailing as 0 and leading as 1 is done to
    reflect that it used to be a leading bool value. Changing the order of this enum requires
    code review as some code relies on the test for leading.
*/

enum {
    forest_trailing_edge,
    forest_leading_edge
};

/**************************************************************************************************/

template <typename I> // I models FullorderIterator
inline void pivot(I& i) {
    i.edge() ^= 1;
}

template <typename I> // I models FullorderIterator
inline I pivot_of(I i) {
    pivot(i);
    return i;
}

/**************************************************************************************************/

template <typename I> // I models a FullorderIterator
inline I leading_of(I i) {
    i.edge() = forest_leading_edge;
    return i;
}

template <typename I> // I models a FullorderIterator
inline I trailing_of(I i) {
    i.edge() = forest_trailing_edge;
    return i;
}

/**************************************************************************************************/

template <typename I> // I models FullorderIterator
I find_parent(I i) {
    do {
        i = trailing_of(i);
        ++i;
    } while (i.edge() != forest_trailing_edge);
    return i;
}

/**************************************************************************************************/

template <typename I> // I models FullorderIterator
bool has_children(const I& i) {
    return !i.equal_node(std::next(leading_of(i)));
}

/**************************************************************************************************/

template <typename I> // I models FullorderIterator
class child_iterator {
public:
    using value_type = typename std::iterator_traits<I>::value_type;
    using difference_type = typename std::iterator_traits<I>::difference_type;
    using reference = typename std::iterator_traits<I>::reference;
    using pointer = typename std::iterator_traits<I>::pointer;
    using iterator_category = typename std::iterator_traits<I>::iterator_category;

    child_iterator() = default;
    explicit child_iterator(I x) : _x(std::move(x)) {}
    template <typename U> child_iterator(const child_iterator<U>& u) : _x(u.base()) {}

    I base() const { return _x; }

    reference operator*() { return dereference(); }
    pointer operator->() { return &dereference(); }
    auto& operator++() { increment(); return *this; }
    auto operator++(int) { auto result{*this}; increment(); return result; }
    auto& operator--() { decrement(); return *this; }
    auto operator--(int) { auto result{*this}; decrement(); return result; }

    friend bool operator==(const child_iterator& a, const child_iterator& b) {
        return a.base() == b.base();
    }
    friend bool operator!=(const child_iterator& a, const child_iterator& b) {
        return !(a == b);
    }

private:
    I _x;

    void increment() {
        pivot(_x);
        ++_x;
    }
    void decrement() {
        --_x;
        pivot(_x);
    }
    reference dereference() { return *_x; }
};

/**************************************************************************************************/

template <typename I> // I models FullorderIterator
I find_edge(I x, std::size_t edge) {
    while (x.edge() != edge)
        ++x;
    return x;
}

template <typename I> // I models FullorderIterator
I find_edge_reverse(I x, std::size_t edge) {
    while (x.edge() != edge)
        --x;
    return x;
}

/**************************************************************************************************/

template <typename I, std::size_t Edge> // I models FullorderIterator
class edge_iterator {
public:
    using value_type = typename std::iterator_traits<I>::value_type;
    using difference_type = typename std::iterator_traits<I>::difference_type;
    using reference = typename std::iterator_traits<I>::reference;
    using pointer = typename std::iterator_traits<I>::pointer;
    using iterator_category = typename std::iterator_traits<I>::iterator_category;

    edge_iterator() = default;
    explicit edge_iterator(I x) : _x(find_edge(x, Edge)) {}
    template <typename U> edge_iterator(const edge_iterator<U, Edge>& u) : _x(u.base()) {}

    I base() const { return _x; }

    reference operator*() { return dereference(); }
    pointer operator->() { return &dereference(); }
    auto& operator++() { increment(); return *this; }
    auto operator++(int) { auto result{*this}; increment(); return result; }
    auto& operator--() { decrement(); return *this; }
    auto operator--(int) { auto result{*this}; decrement(); return result; }

    friend bool operator==(const edge_iterator& a, const edge_iterator& b) {
        return a.base() == b.base();
    }
    friend bool operator!=(const edge_iterator& a, const edge_iterator& b) {
        return !(a == b);
    }

private:
    I _x;

    void increment() {
        _x = find_edge(std::next(_x), Edge);
    }

    void decrement() {
        _x = find_edge_reverse(std::prev(_x), Edge);
    }

    reference dereference() {
        return *_x;
    }
};

/**************************************************************************************************/

/*
    I need to establish dereferencable range vs. traversable range.

    Here [f, l) must be a dereferencable range and p() will not be applied outside that range
    although the iterators may travel beyond that range.
*/

template <typename I, // I models a Forest
          typename P> // P models UnaryPredicate of value_type(I)
class filter_fullorder_iterator {
public:
    using value_type = typename std::iterator_traits<I>::value_type;
    using difference_type = typename std::iterator_traits<I>::difference_type;
    using reference = typename std::iterator_traits<I>::reference;
    using pointer = typename std::iterator_traits<I>::pointer;
    using iterator_category = typename std::iterator_traits<I>::iterator_category;

    filter_fullorder_iterator() = default;

    filter_fullorder_iterator(I f, I l, P p)
        : _x(skip_forward(f, l, p)), inside_m(true),
          first_m(f), last_m(l), predicate_m(p) {}

    filter_fullorder_iterator(I f, I l)
        : _x(skip_forward(f, l, P())), inside_m(true),
          first_m(f), last_m(l) {}

    template <typename U>
    filter_fullorder_iterator(const filter_fullorder_iterator<U, P>& x)
        : _x(x.base()), inside_m(x.inside_m),
          first_m(x.first_m), last_m(x.last_m), predicate_m(x.predicate_m) {}

    P predicate() const { return predicate_m; }

    std::size_t edge() const { return this->base().edge(); }
    std::size_t& edge() { return this->base_reference().edge(); }

    bool equal_node(const filter_fullorder_iterator& y) const {
        return this->base().equal_node(y.base());
    }

    I base() const { return _x; }

    reference operator*() { return dereference(); }
    pointer operator->() { return &dereference(); }
    auto& operator++() { increment(); return *this; }
    auto operator++(int) { auto result{*this}; increment(); return result; }
    auto& operator--() { decrement(); return *this; }
    auto operator--(int) { auto result{*this}; decrement(); return result; }

    friend bool operator==(const filter_fullorder_iterator& a, const filter_fullorder_iterator& b) {
        return a.base() == b.base();
    }
    friend bool operator!=(const filter_fullorder_iterator& a, const filter_fullorder_iterator& b) {
        return !(a == b);
    }

private:
    void increment() {
        I i = this->base();

        if (i == last_m)
            inside_m = false;
        ++i;
        if (i == first_m)
            inside_m = true;
        if (inside_m)
            i = skip_forward(i, last_m, predicate_m);
        this->base_reference() = i;
    }

    static I skip_forward(I f, I l, P p)
        // Precondition: l is on a leading edge
    {
        while ((f.edge() == forest_leading_edge) && (f != l) && !p(*f)) {
            f.edge() = forest_trailing_edge;
            ++f;
        }
        return f;
    }

    static I skip_backward(I f, I l, P p)
        // Precondition: f is on a trailing edge
    {
        while ((l.edge() == forest_trailing_edge) && (f != l) && !p(*l)) {
            l.edge() = forest_leading_edge;
            --l;
        }
        return l;
    }

    void decrement() {
        I i = this->base();

        if (i == first_m)
            inside_m = false;
        --i;
        if (i == last_m)
            inside_m = true;
        if (inside_m)
            i = skip_backward(first_m, i, predicate_m);
        this->base_reference() = i;
    }

    reference dereference() {
        return *_x;
    }

    I _x;
    bool inside_m;
    I first_m;
    I last_m;
    P predicate_m;
};

/**************************************************************************************************/

/*
    REVISIT (sparent) : This is an interesting case - an edge is a property of an iterator but it
    is determined by examining a vertex in the graph. Here we need to examine the prior vertex
    to determine the edge. If the range is empty (or we are at the "end" of the range) then this
    examination is questionable.

    We let it go, because we know the forest structure is an infinite loop through the root. One
    answer to this would be to construct a reverse iterator which was not "off by one" for forest -
    but that might break people who assume base() is off by one for a reverse iterator, and it still
    assumes a root().
*/

template <typename I> // I models a FullorderIterator
class reverse_fullorder_iterator {
public:
    typedef I iterator_type;

    using value_type = typename std::iterator_traits<I>::value_type;
    using difference_type = typename std::iterator_traits<I>::difference_type;
    using reference = typename std::iterator_traits<I>::reference;
    using pointer = typename std::iterator_traits<I>::pointer;
    using iterator_category = typename std::iterator_traits<I>::iterator_category;

    reverse_fullorder_iterator() : edge_m(forest_trailing_edge) {}
    explicit reverse_fullorder_iterator(I x) : base_m(--x), edge_m(forest_leading_edge - base_m.edge()) {}
    template <typename U>
    reverse_fullorder_iterator(const reverse_fullorder_iterator<U>& x)
        : base_m(--x.base()), edge_m(forest_leading_edge - base_m.edge()) {}

    iterator_type base() const { return std::next(base_m); }

    std::size_t edge() const { return edge_m; }
    std::size_t& edge() { return edge_m; }

    bool equal_node(const reverse_fullorder_iterator& y) const {
        return base_m.equal_node(y.base_m);
    }

    reference operator*() { return dereference(); }
    pointer operator->() { return &dereference(); }
    auto& operator++() { increment(); return *this; }
    auto operator++(int) { auto result{*this}; increment(); return result; }
    auto& operator--() { decrement(); return *this; }
    auto operator--(int) { auto result{*this}; decrement(); return result; }

    friend bool operator==(const reverse_fullorder_iterator& a, const reverse_fullorder_iterator& b) {
        return a.base() == b.base();
    }
    friend bool operator!=(const reverse_fullorder_iterator& a, const reverse_fullorder_iterator& b) {
        return !(a == b);
    }

private:
    void increment() {
        base_m.edge() = forest_leading_edge - edge_m;
        --base_m;
        edge_m = forest_leading_edge - base_m.edge();
    }
    void decrement() {
        base_m.edge() = forest_leading_edge - edge_m;
        ++base_m;
        edge_m = forest_leading_edge - base_m.edge();
    }
    reference dereference() const { return *base_m; }

    bool equal(const reverse_fullorder_iterator& y) const {
        return (base_m == y.base_m) && (edge_m == y.edge_m);
    }

    I base_m;
    std::size_t edge_m;
};

/**************************************************************************************************/

template <typename I> // I models FullorderIterator
class depth_fullorder_iterator {
public:
    using value_type = typename std::iterator_traits<I>::value_type;
    using difference_type = typename std::iterator_traits<I>::difference_type;
    using reference = typename std::iterator_traits<I>::reference;
    using pointer = typename std::iterator_traits<I>::pointer;
    using iterator_category = typename std::iterator_traits<I>::iterator_category;

    depth_fullorder_iterator(difference_type d = 0) : depth_m(d) {}
    explicit depth_fullorder_iterator(I x, difference_type d = 0)
        : _x(x), depth_m(d) {}
    template <typename U>
    depth_fullorder_iterator(const depth_fullorder_iterator<U>& x)
        : _x(x.base()), depth_m(x.depth_m) {}

    difference_type depth() const { return depth_m; }
    std::size_t edge() const { return this->base().edge(); }
    std::size_t& edge() { return _x.edge(); }
    bool equal_node(depth_fullorder_iterator const& y) const {
        return this->base().equal_node(y.base());
    }

    I base() const { return _x; }

    reference operator*() { return dereference(); }
    pointer operator->() { return &dereference(); }
    auto& operator++() { increment(); return *this; }
    auto operator++(int) { auto result{*this}; increment(); return result; }
    auto& operator--() { decrement(); return *this; }
    auto operator--(int) { auto result{*this}; decrement(); return result; }

    friend bool operator==(const depth_fullorder_iterator& a, const depth_fullorder_iterator& b) {
        return a.base() == b.base();
    }
    friend bool operator!=(const depth_fullorder_iterator& a, const depth_fullorder_iterator& b) {
        return !(a == b);
    }

private:
    I _x;
    difference_type depth_m;

    void increment() {
        std::size_t old_edge(edge());
        ++_x;
        if (old_edge == edge())
            depth_m += difference_type(old_edge << 1) - 1;
    }

    void decrement() {
        std::size_t old_edge(edge());
        --_x;
        if (old_edge == edge())
            depth_m -= difference_type(old_edge << 1) - 1;
    }

    reference dereference() {
        return *_x;
    }
};

/**************************************************************************************************/

template <typename Forest>
class child_adaptor;
template <typename T>
class forest;

/**************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)
namespace implementation {

template <typename D> // derived class
struct node_base {
    enum next_prior_t {
        prior_s,
        next_s
    };

    typedef D* node_ptr;
    typedef node_ptr& reference;

    node_ptr& link(std::size_t edge, next_prior_t link) { return nodes_m[edge][std::size_t(link)]; }

    node_ptr link(std::size_t edge, next_prior_t link) const {
        return nodes_m[edge][std::size_t(link)];
    }

    #if 0
    node_ptr nodes_m[2][2] = {
        { static_cast<node_ptr>(this), static_cast<node_ptr>(this) },
        { static_cast<node_ptr>(this), static_cast<node_ptr>(this) }
    };
    #else
    node_ptr nodes_m[2][2];

    node_base() : nodes_m {
        { static_cast<node_ptr>(this), static_cast<node_ptr>(this) },
        { static_cast<node_ptr>(this), static_cast<node_ptr>(this) }
    } { }
    #endif
};

template <typename T> // T models Regular
struct node : public node_base<node<T>> {
    typedef T value_type;

    explicit node(const value_type& data) : data_m(data) {}

    value_type data_m;
};

/**************************************************************************************************/

template <typename T>
class forest_const_iterator;

template <typename T> // T is value_type
class forest_iterator {
public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using pointer = T*;
    using iterator_category = std::bidirectional_iterator_tag;

    forest_iterator() : node_m(0), edge_m(forest_leading_edge) {}

    forest_iterator(const forest_iterator& x) : node_m(x.node_m), edge_m(x.edge_m) {}

    std::size_t edge() const { return edge_m; }
    std::size_t& edge() { return edge_m; }
    bool equal_node(forest_iterator const& y) const { return node_m == y.node_m; }

    reference operator*() const { return dereference(); }
    pointer operator->() const { return &dereference(); }
    auto& operator++() { increment(); return *this; }
    auto operator++(int) { auto result{*this}; increment(); return result; }
    auto& operator--() { decrement(); return *this; }
    auto operator--(int) { auto result{*this}; decrement(); return result; }

    friend bool operator==(const forest_iterator& a, const forest_iterator& b) {
        return a.equal(b);
    }
    friend bool operator!=(const forest_iterator& a, const forest_iterator& b) {
        return !(a == b);
    }

private:
    friend class stlab::forest<value_type>;

    template <typename>
    friend class forest_iterator;
    template <typename>
    friend class forest_const_iterator;
    friend struct unsafe::set_next_fn<forest_iterator>;

    typedef node<T> node_t;

    reference dereference() const { return node_m->data_m; }

    void increment() {
        node_t* next(node_m->link(edge_m, node_t::next_s));

        if (edge_m)
            edge_m = std::size_t(next != node_m);
        else
            edge_m = std::size_t(next->link(forest_leading_edge, node_t::prior_s) == node_m);

        node_m = next;
    }

    void decrement() {
        node_t* next(node_m->link(edge_m, node_t::prior_s));

        if (edge_m)
            edge_m = std::size_t(next->link(forest_trailing_edge, node_t::next_s) != node_m);
        else
            edge_m = std::size_t(next == node_m);

        node_m = next;
    }

    bool equal(const forest_iterator& y) const {
        return (node_m == y.node_m) && (edge_m == y.edge_m);
    }

    node_t* node_m;
    std::size_t edge_m;

    forest_iterator(node_t* node, std::size_t edge) : node_m(node), edge_m(edge) {}
};


/**************************************************************************************************/

template <typename T> // T is value_type
class forest_const_iterator {
public:
    using value_type = const T;
    using difference_type = std::ptrdiff_t;
    using reference = const T&;
    using pointer = const T*;
    using iterator_category = std::bidirectional_iterator_tag;

    forest_const_iterator() : node_m(0), edge_m(forest_leading_edge) {}

    forest_const_iterator(const forest_const_iterator& x) : node_m(x.node_m), edge_m(x.edge_m) {}

    forest_const_iterator(const forest_iterator<T>& x) : node_m(x.node_m), edge_m(x.edge_m) {}

    std::size_t edge() const { return edge_m; }
    std::size_t& edge() { return edge_m; }
    bool equal_node(forest_const_iterator const& y) const { return node_m == y.node_m; }

    reference operator*() const { return dereference(); }
    pointer operator->() const { return &dereference(); }
    auto& operator++() { increment(); return *this; }
    auto operator++(int) { auto result{*this}; increment(); return result; }
    auto& operator--() { decrement(); return *this; }
    auto operator--(int) { auto result{*this}; decrement(); return result; }

    friend bool operator==(const forest_const_iterator& a, const forest_const_iterator& b) {
        return a.equal(b);
    }
    friend bool operator!=(const forest_const_iterator& a, const forest_const_iterator& b) {
        return !(a == b);
    }

private:
    template <typename> friend class stlab::forest;
    template <typename> friend class forest_const_iterator;
    friend struct unsafe::set_next_fn<forest_const_iterator>;

    using node_t = const node<T>;

    reference dereference() const { return node_m->data_m; }

    void increment() {
        node_t* next(node_m->link(edge_m, node_t::next_s));

        if (edge_m)
            edge_m = std::size_t(next != node_m);
        else
            edge_m = std::size_t(next->link(forest_leading_edge, node_t::prior_s) == node_m);

        node_m = next;
    }

    void decrement() {
        node_t* next(node_m->link(edge_m, node_t::prior_s));

        if (edge_m)
            edge_m = std::size_t(next->link(forest_trailing_edge, node_t::next_s) != node_m);
        else
            edge_m = std::size_t(next == node_m);

        node_m = next;
    }

    bool equal(const forest_const_iterator& y) const {
        return (node_m == y.node_m) && (edge_m == y.edge_m);
    }

    node_t* node_m;
    std::size_t edge_m;

    forest_const_iterator(node_t* node, std::size_t edge) : node_m(node), edge_m(edge) {}
};

/**************************************************************************************************/

} // namespace implementation
#endif

/**************************************************************************************************/

namespace unsafe {

template <typename T> // T is node<T>
struct set_next_fn<implementation::forest_iterator<T>> {
    void operator()(implementation::forest_iterator<T> x,
                    implementation::forest_iterator<T> y) const {
        typedef typename implementation::node<T> node_t;

        x.node_m->link(x.edge(), node_t::next_s) = y.node_m;
        y.node_m->link(y.edge(), node_t::prior_s) = x.node_m;
    }
};

} // namespace unsafe

/**************************************************************************************************/

template <typename T>
class forest {
private:
    typedef implementation::node<T> node_t;
    friend class child_adaptor<forest<T>>;

public:
    // types
    typedef T& reference;
    typedef const T& const_reference;
    typedef implementation::forest_iterator<T> iterator;
    typedef implementation::forest_const_iterator<T> const_iterator;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef reverse_fullorder_iterator<iterator> reverse_iterator;
    typedef reverse_fullorder_iterator<const_iterator> const_reverse_iterator;

    typedef child_iterator<iterator> child_iterator;
    /* qualification needed since: A name N used in a class S shall refer to the same declaration
       in its context and when re-evaluated in the completed scope of
       S. */
    typedef stlab::child_iterator<const_iterator> const_child_iterator;
    typedef std::reverse_iterator<child_iterator> reverse_child_iterator;

    typedef edge_iterator<iterator, forest_leading_edge> preorder_iterator;
    typedef edge_iterator<const_iterator, forest_leading_edge> const_preorder_iterator;
    typedef edge_iterator<iterator, forest_trailing_edge> postorder_iterator;
    typedef edge_iterator<const_iterator, forest_trailing_edge> const_postorder_iterator;

#if !defined(ADOBE_NO_DOCUMENTATION)
    forest() = default;
    ~forest() { clear(); }

    forest(const forest&);
    forest(forest&&) noexcept;
    forest& operator=(const forest& x) {
        auto tmp = x;
        *this = std::move(tmp);
        return *this;
    }
    forest& operator=(forest&& x) noexcept {
        clear();
        splice(end(), x);
        return *this;
    }

    void swap(forest&);
#endif

    size_type size() const;
    size_type max_size() const { return size_type(-1); }
    bool size_valid() const { return size_m != 0 || empty(); }
    bool empty() const { return begin() == end(); } // Don't test size which may be expensive

    // iterators
    iterator root() { return iterator(tail(), forest_leading_edge); }
    const_iterator root() const { return const_iterator(tail(), forest_leading_edge); }

    iterator begin() { return ++root(); }
    iterator end() { return iterator(tail(), forest_trailing_edge); }
    const_iterator begin() const { return ++root(); }
    const_iterator end() const { return const_iterator(tail(), forest_trailing_edge); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    reference front() {
        assert(!empty());
        return *begin();
    }
    const_reference front() const {
        assert(!empty());
        return *begin();
    }
    reference back() {
        assert(!empty());
        return *(--end());
    }
    const_reference back() const {
        assert(!empty());
        return *(--end());
    }

    // modifiers
    void clear() {
        erase(begin(), end());
        assert(empty()); // Make sure our erase is correct
    }

    iterator erase(const iterator& position);
    iterator erase(const iterator& first, const iterator& last);

    iterator insert(const iterator& position, T x) {
        iterator result(new node_t(std::move(x)), true);

        if (size_valid())
            ++size_m;

        unsafe::set_next(std::prev(position), result);
        unsafe::set_next(std::next(result), position);

        return result;
    }

    void push_front(const T& x) { insert(begin(), x); }
    void push_back(const T& x) { insert(end(), x); }
    void pop_front() {
        assert(!empty());
        erase(begin());
    }
    void pop_back() {
        assert(!empty());
        erase(--end());
    }

    iterator insert(iterator position, const_child_iterator first, const_child_iterator last);

    iterator splice(iterator position, forest<T>& x);
    iterator splice(iterator position, forest<T>& x, iterator i);
    iterator splice(iterator position, forest<T>& x, child_iterator first, child_iterator last);
    iterator splice(iterator position, forest<T>& x, child_iterator first, child_iterator last,
                    size_type count);

    iterator insert_parent(child_iterator front, child_iterator back, const T& x);
    void reverse(child_iterator first, child_iterator last);

private:
#if !defined(ADOBE_NO_DOCUMENTATION)

    friend class implementation::forest_iterator<value_type>;
    friend class implementation::forest_const_iterator<value_type>;
    friend struct unsafe::set_next_fn<iterator>;

    mutable size_type size_m = 0;
    implementation::node_base<node_t> tail_m;

    node_t* tail() { return static_cast<node_t*>(&tail_m); }
    const node_t* tail() const { return static_cast<const node_t*>(&tail_m); }
#endif
};

/**************************************************************************************************/

template <typename T>
bool operator==(const forest<T>& x, const forest<T>& y) {
    if (x.size() != y.size())
        return false;

    for (typename forest<T>::const_iterator first(x.begin()), last(x.end()), pos(y.begin());
         first != last; ++first, ++pos) {
        if (first.edge() != pos.edge())
            return false;
        if (first.edge() && (*first != *pos))
            return false;
    }

    return true;
}

/**************************************************************************************************/

template <typename T>
bool operator!=(const forest<T>& x, const forest<T>& y) {
    return !(x == y);
}

/**************************************************************************************************/

namespace unsafe {

template <typename I> // I models a FullorderIterator
struct set_next_fn<child_iterator<I>> {
    void operator()(child_iterator<I> x, child_iterator<I> y) {
        unsafe::set_next(pivot_of(x.base()), y.base());
    }
};

} // namespace unsafe

/**************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)

/**************************************************************************************************/

template <typename T>
forest<T>::forest(const forest& x)
    : forest() {
    insert(end(), const_child_iterator(x.begin()), const_child_iterator(x.end()));
}

/**************************************************************************************************/

template <typename T>
forest<T>::forest(forest&& x) noexcept
    : forest() {
    splice(end(), x);
}

/**************************************************************************************************/

template <typename T>
void forest<T>::swap(forest& x) {
    std::swap(*this, x);
}

#endif

/**************************************************************************************************/

template <typename T>
typename forest<T>::size_type forest<T>::size() const {
    if (!size_valid()) {
        const_preorder_iterator first(begin());
        const_preorder_iterator last(end());

        size_m = size_type(std::distance(first, last));
    }

    return size_m;
}

/**************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::erase(const iterator& first, const iterator& last) {
    difference_type stack_depth(0);
    iterator position(first);

    while (position != last) {
        if (position.edge() == forest_leading_edge) {
            ++stack_depth;
            ++position;
        } else {
            if (stack_depth > 0)
                position = erase(position);
            else
                ++position;
            stack_depth = std::max<difference_type>(0, stack_depth - 1);
        }
    }
    return last;
}

/**************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::erase(const iterator& position) {
    /*
        NOTE (sparent) : After the first call to set_next() the invariants of the forest are
        violated and we can't determing leading/trailing if we navigate from the effected node.
        So we gather all the iterators up front then do the set_next calls.
    */

    if (size_valid())
        --size_m;

    iterator leading_prior(std::prev(leading_of(position)));
    iterator leading_next(std::next(leading_of(position)));
    iterator trailing_prior(std::prev(trailing_of(position)));
    iterator trailing_next(std::next(trailing_of(position)));

    if (has_children(position)) {
        unsafe::set_next(leading_prior, leading_next);
        unsafe::set_next(trailing_prior, trailing_next);
    } else {
        unsafe::set_next(leading_prior, trailing_next);
    }

    delete position.node_m;

    return position.edge() ? std::next(leading_prior) : trailing_next;
}

/**************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::splice(iterator position, forest<T>& x) {
    return splice(position, x, child_iterator(x.begin()), child_iterator(x.end()),
                  x.size_valid() ? x.size() : 0);
}

/**************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::splice(iterator position, forest<T>& x, iterator i) {
    i.edge() = forest_leading_edge;
    return splice(position, x, child_iterator(i), ++child_iterator(i), has_children(i) ? 0 : 1);
}

/**************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::insert(iterator pos, const_child_iterator f,
                                               const_child_iterator l) {
    for (const_iterator first(f.base()), last(l.base()); first != last; ++first, ++pos) {
        if (first.edge())
            pos = insert(pos, *first);
    }

    return pos;
}

/**************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::splice(iterator pos, forest<T>& x, child_iterator first,
                                               child_iterator last, size_type count) {
    if (first == last || first.base() == pos)
        return pos;

    if (&x != this) {
        if (count) {
            if (size_valid())
                size_m += count;
            if (x.size_valid())
                x.size_m -= count;
        } else {
            size_m = 0;
            x.size_m = 0;
        }
    }

    iterator back(std::prev(last.base()));

    unsafe::set_next(std::prev(first), last);

    unsafe::set_next(std::prev(pos), first.base());
    unsafe::set_next(back, pos);

    return first.base();
}

/**************************************************************************************************/

template <typename T>
inline typename forest<T>::iterator forest<T>::splice(iterator pos, forest<T>& x,
                                                      child_iterator first, child_iterator last) {
    return splice(pos, x, first, last, 0);
}

/**************************************************************************************************/

template <typename T>
typename forest<T>::iterator forest<T>::insert_parent(child_iterator first, child_iterator last,
                                                      const T& x) {
    iterator result(insert(last.base(), x));
    if (first == last)
        return result;
    splice(trailing_of(result), *this, first, child_iterator(result));
    return result;
}

/**************************************************************************************************/

template <typename T>
void forest<T>::reverse(child_iterator first, child_iterator last) {
    iterator prior(first.base());
    --prior;
    first = unsafe::reverse_nodes(first, last);
    unsafe::set_next(prior, first.base());
}

/**************************************************************************************************/

template <typename I> // I models FullorderIterator
child_iterator<I> child_begin(const I& x) {
    return child_iterator<I>(std::next(leading_of(x)));
}

/**************************************************************************************************/

template <typename I> // I models FullorderIterator
child_iterator<I> child_end(const I& x) {
    return child_iterator<I>(trailing_of(x));
}

/**************************************************************************************************/

template <typename Forest>
class child_adaptor {
public:
    typedef Forest forest_type;
    typedef typename Forest::value_type value_type;
    typedef typename Forest::iterator iterator_type;
    typedef typename Forest::reference reference;
    typedef typename Forest::const_reference const_reference;
    typedef typename Forest::difference_type difference_type;
    typedef typename Forest::child_iterator iterator;

    child_adaptor(forest_type& f, iterator_type& i) : forest_m(f), iterator_m(i) {}

    value_type& back() { return *(--child_end(iterator_m)); }
    value_type& front() { return *(child_begin(iterator_m)); }

    void push_back(const value_type& x) { forest_m.insert(child_end(iterator_m).base(), x); }
    void push_front(const value_type& x) { forest_m.insert(child_begin(iterator_m).base(), x); }

    void pop_back() { forest_m.erase(--child_end(iterator_m).base()); }
    void pop_front() { forest_m.erase(child_begin(iterator_m).base()); }

private:
    child_adaptor(); // not defined

    forest_type& forest_m;
    iterator_type& iterator_m;
};

/**************************************************************************************************/

template <typename I>
struct forest_range {
    I _f;
    I _l;

    auto begin() const { return _f; }
    auto end() { return _l; }
};

/**************************************************************************************************/

template <typename I> // I models FullorderIterator
inline auto child_range(const I& x) {
    return forest_range<child_iterator<I>>{ child_begin(x), child_end(x) };
}

/**************************************************************************************************/

template <typename R, typename P> // R models FullorderRange
inline auto filter_fullorder_range(R& x, P p) {
    typedef filter_fullorder_iterator<typename R::iterator, P> iterator;

    return forest_range<iterator>{ iterator(std::begin(x), std::end(x), p), iterator(std::end(x), std::end(x), p) };
}

template <typename R, typename P> // R models FullorderRange
inline auto filter_fullorder_range(const R& x, P p) {
    typedef filter_fullorder_iterator<typename R::const_iterator, P> iterator;

    return forest_range<iterator>{ iterator(std::begin(x), std::end(x), p), iterator(std::end(x), std::end(x), p) };
}

/**************************************************************************************************/

template <typename R> // R models FullorderRange
inline auto reverse_fullorder_range(R& x) {
    typedef reverse_fullorder_iterator<typename R::iterator> iterator;

    return forest_range<iterator>{ iterator(std::end(x)), iterator(std::begin(x)) };
}

template <typename R> // R models FullorderRange
inline auto reverse_fullorder_range(const R& x) {
    typedef reverse_fullorder_iterator<typename R::const_iterator> iterator;

    return forest_range<iterator>{ iterator(std::end(x)), iterator(std::begin(x)) };
}


/**************************************************************************************************/

template <typename R> // R models FullorderRange
inline auto depth_range(R& x) {
    typedef depth_fullorder_iterator<typename R::iterator> iterator;

    return forest_range<iterator>{ iterator(std::begin(x)), iterator(std::end(x)) };
}

template <typename R> // R models FullorderRange
inline auto depth_range(const R& x) {
    typedef depth_fullorder_iterator<typename R::const_iterator> iterator;

    return forest_range<iterator>{ iterator(std::begin(x)), iterator(std::end(x)) };
}

/**************************************************************************************************/

template <typename R> // R models FullorderRange
inline auto postorder_range(R& x) {
    typedef edge_iterator<typename R::iterator, forest_trailing_edge> iterator;

    return forest_range<iterator>{ iterator(std::begin(x)), iterator(std::end(x)) };
}

template <typename R> // R models FullorderRange
inline auto postorder_range(const R& x) {
    typedef edge_iterator<typename R::const_iterator, forest_trailing_edge> iterator;

    return forest_range<iterator>{ iterator(std::begin(x)), iterator(std::end(x)) };
}

/**************************************************************************************************/

template <typename R> // R models FullorderRange
inline auto preorder_range(R& x) {
    using iterator = edge_iterator<typename R::iterator, forest_leading_edge>;

    return forest_range<iterator>{ iterator(std::begin(x)), iterator(std::end(x)) };
}

template <typename R> // R models FullorderRange
inline auto preorder_range(const R& x) {
    using iterator = edge_iterator<typename R::const_iterator, forest_leading_edge>;

    return forest_range<iterator>{ iterator(std::begin(x)), iterator(std::end(x)) };
}

/**************************************************************************************************/

} // namespace stlab

/**************************************************************************************************/

#endif // STLAB_FOREST_HPP

/**************************************************************************************************/
