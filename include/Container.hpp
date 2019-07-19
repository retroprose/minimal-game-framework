#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <vector>
#include <map>
#include <algorithm>

#include <Error.hpp>

template<typename T>
class Ref {
public:
    Ref(T* ptr = nullptr) : ptr_(ptr) { }

    T& get() {
        ASSERT( !isNull() );
        return *ptr_;
    }

    const T& get() const {
        ASSERT( !isNull() );
        return *ptr_;
    }

    void set(const T& t) {
        ASSERT( !isNull() );
        *ptr_ = t;
    }

    T& operator *() {
        ASSERT( !isNull() );
        return *ptr_;
    }

    T* operator -> () {
        bool b = (!isNull());
        ASSERT( !isNull() );
        return ptr_;
    }

    const T& operator *() const {
        ASSERT( !isNull() );
        return *ptr_;
    }

    const T* operator -> () const {
        ASSERT( !isNull() );
        return ptr_;
    }

    bool isNull() const {
        return ptr_ == nullptr;
    }

    // overloaded prefix ++ operator
    // Define prefix decrement operator.
    Ref& operator++() {
       ++ptr_;
       return *this;
    }

    uint8_t* bytePtr() { return reinterpret_cast<uint8_t*>(ptr_); }

private:
    T* ptr_; // pointer

};

template<typename T>
Ref<T> makeRef(T& t) {
    return Ref<T>(&t);
}


template<typename T>
class Single {
private:
    T value;

public:
    using value_type = T;

    Ref<T> getRef() {
        return Ref<T>(value);
    }

    T& get() {
        return value;
    }

};


template<typename T>
class Vector {
private:
    using table_type = std::vector<T>;
    table_type table;

public:
    using value_type = T;

    size_t size() const {
        return table.size();
    }

    const T& operator[](uint16_t index) const {
        return table[index];
    }

    T& operator[](uint16_t index) {
        return table[index];
    }

    Ref<T> find(uint16_t index) {
        return Ref<T>( &(table[index]) );
    }

    typename table_type::const_iterator begin() const {
        return table.begin();
    }

    typename table_type::const_iterator end() const {
        return table.end();
    }

    void sort() {
        std::sort(table.begin(), table,end());
    }

    void extend(uint16_t index) {
        if ((uint32_t)(index + 1) > table.size()) {
            table.resize((uint32_t)(index + 1));
        }
    }

    void erase(uint16_t index) {
        std::swap(table[index], table[table.size() - 1]);
        table.pop_back();
    }

};



template<typename T>
class VectorMap {
public:
     using table_type = std::map<uint32_t, std::vector<T>>;
     using vector_iterator = typename std::vector<T>::iterator;

private:
    using pair_type = std::pair<uint32_t, std::vector<T>>;
    table_type table;

public:
    using value_type = T;

    static uint16_t getVector(uint32_t h) {
        return (h & 0xffff0000) >> 16;
    }

    static uint16_t getIndex(uint32_t h) {
        return (h & 0x0000ffff);
    }

    size_t size(uint32_t hash) const {
        size_t s = 0;
        typename table_type::const_iterator it = table.find(getVector(hash));
        if (it != table.end()) {
            s = it->second.size();
        }
        return s;
    }

    bool hasHash(uint32_t hash) const {
        bool has = true;
        typename table_type::const_iterator it = table.find(getVector(hash));
        if (it == table.end() || getIndex(hash) >= it->second.size())
            has = false;
        return has;
    }

    T& operator[](uint32_t hash) {
        typename table_type::iterator it = table.find(getVector(hash));
        ASSERT(it != table.end() && getIndex(hash) < it->second.size());
        return it->second[getIndex(hash)];
    }

    Ref<T> find(uint32_t hash) {
        Ref<T> ref;
        typename table_type::iterator it = table.find(getVector(hash));
        if (it != table.end() && getIndex(hash) < it->second.size()) {
            ref = Ref<T>( &(it->second[getIndex(hash)]) );
        }
        return ref;
    }

    typename table_type::iterator begin() {
        return table.begin();
    }

    typename table_type::iterator end() {
        return table.end();
    }

    vector_iterator begin(uint16_t signature) {
        return table[signature].begin();
    }

    vector_iterator end(uint16_t signature) {
        return table[signature].begin();
    }

    void move(uint32_t oldHash, uint32_t newHash) {
        typename table_type::iterator newSig = table.find(getVector(newHash));
        if (newSig == table.end()) {
            if (getVector(newHash) == 0) return;
            auto ret = table.insert(pair_type(getVector(newHash), std::vector<T>()));
            newSig = ret.first; // BUG?!
        }

        if (newSig->second.size() <= getIndex(newHash)) {
            newSig->second.resize(getIndex(newHash) + 1);
        }

        typename table_type::iterator oldSig = table.find(getVector(oldHash));

        if (oldSig != table.end()) {
            std::swap(oldSig->second[getIndex(oldHash)], newSig->second[getIndex(newHash)]);
        }
    }

};


template<typename T>
class HashMap {
public:
    using table_type = std::map<uint16_t, T>;

private:
    using pair_type = std::pair<uint16_t, T>;
    table_type table;

public:
    using value_type = T;

    size_t size() const {
        return table.size();
    }

    bool hasHash(uint16_t index) const {
        return table.find(index) != table.end();
    }

    T& operator[](uint16_t index) {
        auto it = table.find(index);
        ASSERT(it != table.end());
        return *it;
    }

    Ref<T> find(uint16_t index) {
        Ref<T> ref;
        auto it = table.find(index);
        if (it != table.end()) {
            ref = Ref<T>( &(it->second) );
        }
        return ref;
    }

    typename table_type::iterator begin() {
        return table.begin();
    }

    typename table_type::iterator end() {
        return table.end();
    }

    Ref<T> insert(uint16_t index) {
        //auto ret = table.insert( pair_type(index, T()) );
        //return Ref<T>( &(*(ret.first)) );
        table.insert( pair_type(index, T()) );
        return Ref<T>( &(table[index]) );
    }

    void erase(uint16_t index) {
        typename table_type::iterator it = table.find(index);
        if (it != table.end())
            table.erase(it);
    }

};


class EmptyHashMap {
public:
    void insert(uint16_t index) { }
    void erase(uint16_t index) { }
};




#endif // CONTAINER_HPP
