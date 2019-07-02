#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <vector>

#include <Error.hpp>

template<typename T>
class Vector {
public:

    T& operator[](uint32_t index) {
        /*
            Index is out of range
        */
        ASSERT(index >= 0 && index < table.size());
        return table[index];
    }

    const T& operator[](uint32_t index) const {
        /*
            Index is out of range
        */
        ASSERT(index >= 0 && index < table.size());
        return table[index];
    }

    size_t Size() const {
        return table.size();
    }

    void Clear() {
        table.clear();
    }

    void Add() {
        table.push_back( T() );
    }

    void Add(const T& t) {
        table.push_back( t );
    }

    void Add(T&& t) {
        // I *think* this is right?
        table.push_back( std::move(t) );
    }

    void Remove(uint32_t index) {
        /*
            Index is out of range
        */
        ASSERT(index >= 0 && index < table.size());
        table[index] = table[table.size() - 1];
        table.pop_back();
    }

private:
    std::vector<T> table;

};

#endif // CONTAINER_HPP
