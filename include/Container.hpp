#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <vector>
#include <map>
#include <algorithm>
#include <initializer_list>


template<typename T>
class Ref {
public:
    //template<typename U>
    //Ref(U* ptr) : ptr_( reinterpret_cast<U*>(ptr) ) { }

    Ref(T* ptr = nullptr) : ptr_(ptr) { }

    T& operator *() {
        ASSERT( !isNull() );
        return *ptr_;
    }

    T* operator -> () {
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
class VectorMap {
private:
    using pair_type = std::pair<uint16_t, std::vector<T>>;
    using table_type = std::map<uint16_t, std::vector<T>>;
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

    Ref<T> get(uint32_t hash) {
        Ref<T> ref;
        typename table_type::iterator it = table.find(getVector(hash));
        if (it != table.end() && getIndex(hash) < it->second.size()) {
            ref = Ref<T>( &(it->second[getIndex(hash)]) );
        }
        return ref;
    }

    typename table_type::const_iterator begin() const {
        return table.begin();
    }

    typename table_type::const_iterator end() const {
        return table.end();
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
class EntityMap {
private:
    using pair_type = std::pair<uint32_t, T>;
    using table_type = std::map<uint32_t, T>;
    table_type table;

public:
    using value_type = T;

    size_t size() const {
        return table.size();
    }

    bool hasHash(uint32_t index) const {
        return table.find(index) != table.end();
    }

    Ref<T> get(uint32_t index) {
        Ref<T> ref;
        typename table_type::iterator it = table.find(index);
        if (it != table.end()) {
            ref = Ref<T>( &(it->second) );
        }
        return ref;
    }

    typename table_type::const_iterator begin() const {
        return table.begin();
    }

    typename table_type::const_iterator end() const {
        return table.end();
    }

    void insert(uint32_t index) {
        table[index] = T();
    }

    void erase(uint32_t index) {
        typename table_type::iterator it = table.find(index);
        if (it != table.end())
            table.erase(it);
    }

};



#endif // CONTAINER_HPP
