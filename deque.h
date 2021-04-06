#include <type_traits>
#include <unordered_set>
#include <iostream>
#include <cassert>
#include <deque>

#include <iterator>
#include <vector>

template<typename T>
class Deque {
    size_t cap;
    std::vector<T *> basket;
    static const size_t arr_sz = 10;

public:
    template<bool Const>
    class c_iterator {
    public:
        size_t current;
        std::vector<T *> *basket;
        static const size_t arr_sz = 10;

        c_iterator() = delete;

        c_iterator(std::vector<T *> &deq, size_t n = 0) : basket(&deq) {
            current = n;
        }

        template<bool flag>
        c_iterator(const c_iterator<flag> &it) : current(it.current), basket(it.basket) {}

        c_iterator &operator=(const c_iterator it) {
            basket = it.basket;
            current = it.current;
            return *this;
        }


        std::conditional_t<Const, const T &, T &> operator*() {
            return *((*basket)[current / arr_sz] + current % arr_sz);
        }

        std::conditional_t<Const, const T *, T *> operator->() {
            return ((*basket)[current / arr_sz] + current % arr_sz);
        }

        c_iterator &operator++() {
            current++;
            return *this;
        }

        c_iterator &operator--() {
            current--;
            return *this;
        }

        c_iterator operator+(int a) const {
            auto it = *this;
            it.current += a;
            return it;
        }

        c_iterator operator-(int a) const {
            auto it = *this;
            it.current -= a;
            return it;
        }

        int operator-(c_iterator &it2) {
            return this->current - it2.current;
        }

        bool operator<(const c_iterator &it2) const {
            return this->current < it2->current;
        }

        bool operator>(const c_iterator &it2) const {
            return this->current > it2->current;
        }

        bool operator<=(const c_iterator &it2) const {
            return this->current <= it2->current;
        }

        bool operator>=(const c_iterator &it2) const {
            return this->current >= it2.current;
        }

        bool operator==(const c_iterator &it2) const {
            return this->current == it2.current;
        }

        bool operator!=(const c_iterator &it2) const {
            return this->current != it2.current;
        }
    };


    using iterator = c_iterator<false>;
    using const_iterator = c_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


    iterator begining;
    iterator ending;

    Deque(int a = 0) : begining(basket, a), ending(basket, a) {
        cap = 0;
        recap(a / arr_sz + 1);
        ending.current = begining.current + a;
    }

    Deque(int a, const T &value) : cap(0), begining(basket, a), ending(basket, a) {
        recap(a / arr_sz + 1);
        while (a != 0) {
            this->push_back(value);
            a--;
        }
    }

    Deque(const Deque<T> &deq) : cap(deq.cap), begining(deq.begining), ending(deq.ending) {
        try {
            basket.resize(cap);

            for (size_t i = 0; i < cap; ++i) {
                basket[i] = reinterpret_cast<T *>(new char[arr_sz * sizeof(T)]);
                for (size_t j = 0; j < arr_sz; ++j) {
                    T temp = T(deq.basket[i][j]);
                    *(basket[i] + j) = temp;
                }
            }
            begining.basket = &basket;
            ending.basket = &basket;
        } catch (...) {
            basket.erase(basket.begin(), basket.end());
            throw;
        }
    }

    Deque<T> &operator=(const Deque<T> deq) {
        begining = (deq.begining);
        ending = deq.ending;
        cap = deq.cap;
        basket.resize(cap);
        begining.basket = &basket;
        ending.basket = &basket;

        try {
            for (size_t i = 0; i < cap; ++i) {
                basket[i] = reinterpret_cast<T *>(new char[arr_sz * sizeof(T)]);
                for (size_t j = 0; j < arr_sz; ++j) {
                    T temp = T(deq.basket[i][j]);
                    *(basket[i] + j) = temp;
                }
            }
        } catch (...) {
            basket.erase(basket.begin(), basket.end());
            throw;
        }
        return *this;
    }

    void recap(size_t capacity) {
        if (cap == 0) {
            cap = 3;
            basket.resize(cap);
            begining = iterator(basket, 1);
            ending = iterator(basket, 1);
            for (size_t i = 0; i < cap; ++i) {
                basket[i] = reinterpret_cast<T *>(new char[arr_sz * sizeof(T)]);
            }
        } else {

            begining.current += (arr_sz * capacity);
            ending.current += arr_sz * capacity;
            std::vector<T *> helper(3 * capacity);
            for (size_t i = 0; i < capacity; ++i) {
                helper[i] = reinterpret_cast<T *>(new char[arr_sz * sizeof(T)]);
            }

            for (size_t i = 0; i < capacity; ++i) {
                helper[i + capacity] = basket[i];
            }
            for (size_t i = 0; i < capacity; ++i) {
                helper[i + 2 * capacity] = reinterpret_cast<T *>(new char[arr_sz * sizeof(T)]);
            }
            basket = helper;
            cap = 3 * capacity;
        }
    }

    T &operator[](size_t i) {
        return *(begining + i);

    }

    T &at(size_t i) {
        if (i >= static_cast<size_t>(ending.current - begining.current)) throw std::out_of_range("i>sz");
        return *(begining + i);
    }


    const T &operator[](size_t i) const {
        return *(begining + (int) i);
    }

    const T &at(size_t i) const {
        if (i >= static_cast<size_t>(ending.current - begining.current)) throw std::out_of_range("i>sz");
        return *(begining + i);
    }

    size_t size() const {
        return (int) (ending.current - begining.current);
    }

    void pop_back() {
        --ending;
    }

    void pop_front() { ++begining; }

    void push_front(const T &a) {
        if (begining.current == 0) {
            recap(cap);
        }
        --begining;
        *begining = a;
    }

    void push_back(const T &a) {
        if (cap * arr_sz == ending.current) {
            recap(cap);
        }

        *ending = a;
        ++ending;
    }

    const_iterator begin() const {
        return const_iterator(begining);
    }

    iterator begin() {
        return begining;
    }

    const_iterator end() const {
        return const_iterator(ending);
    }

    iterator end() {
        return ending;
    }

    const_iterator cbegin() const {
        return const_iterator(begining);
    }

    const_iterator cend() const {
        return const_iterator(ending);
    }

    void insert(iterator it, const T &e) {
        T elem = const_cast<T &>(e);
        T last_elem = *(end() - 1);
        auto our_it = ending;
        while (it != our_it) {
            std::swap(*our_it, *(our_it - 1));
            --our_it;
        }
        *it = elem;
        push_back(last_elem);
    }

    void erase(iterator it) {
        while (it + 1 != ending) {
            std::swap(*it, *(it + 1));
            ++it;
        }
        --ending;
    }
};

