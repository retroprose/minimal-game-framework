#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <cassert>
#include <vector>
#include <map>
#include <algorithm>

/*
    This file contains containers implemented with the standard library.
    These could be reimplemented from scratch if you want to avoid the
    standard library all together.
*/

/*
    This class simply encapsulates a pointer.  It will assert if
    the pointer is dereferenced and that pointer is nullptr
*/
template<typename T>
class Ref {
public:
    Ref(T* ptr = nullptr) : ptr_(ptr) { }

    T& get() {
        assert( !isNull() );
        return *ptr_;
    }

    const T& get() const {
        assert( !isNull() );
        return *ptr_;
    }

    void set(const T& t) {
        assert( !isNull() );
        *ptr_ = t;
    }

    T& operator *() {
        assert( !isNull() );
        return *ptr_;
    }

    T* operator -> () {
        assert( !isNull() );
        return ptr_;
    }

    const T& operator *() const {
        assert( !isNull() );
        return *ptr_;
    }

    const T* operator -> () const {
        assert( !isNull() );
        return ptr_;
    }

    bool isNull() const {
        return ptr_ == nullptr;
    }

	void* getVoid() {
		return ptr_;
	}

private:
    T* ptr_; // pointer

};


/*
    A container that contains a single value of type T
*/
template<typename T>
class Single {
private:
    T value;

public:
	constexpr static bool needs_move = false;
	constexpr static bool dynamic_comp = false;

	using value_type = T;

    Ref<T> getRef() {
        return Ref<T>(value);
    }

    T& get() {
        return value;
    }

};

/*
    This container is simply a wrapper around a vector,
    all objects are stored sequentially in memory.
*/
template<typename T>
class Vector {
private:
    using table_type = std::vector<T>;
    table_type table;

public:
	constexpr static bool needs_move = false;
	constexpr static bool dynamic_comp = false;

    using value_type = T;

	void clear() {
		table.clear();
	}

    size_t size() const {
        return table.size();
    }

    const T& operator[](uint16_t index) const {
        assert(index >= 0 && index < table.size());
        return table[index];
    }

    T& operator[](uint16_t index) {
        assert(index >= 0 && index < table.size());
        return table[index];
    }

    Ref<T> find(uint16_t index) {
        Ref<T> ref;
        if (index >= 0 && index < table.size())
            ref = Ref<T>( &(table[index]) );
        return ref;
    }

	typename table_type::iterator begin() {
		return table.begin();
	}

	typename table_type::iterator end() {
		return table.end();
	}

    void sort() {
        std::sort(table.begin(), table.end());
    }

    void extend(uint16_t index) {
        if ((uint32_t)(index + 1) > table.size()) {
            table.resize((uint32_t)(index + 1));
        }
    }

	void add(const T& t) {
		table.push_back(t);
	}


    void erase(uint16_t index) {
        assert(index >= 0 && index < table.size());
        std::swap(table[index], table[table.size() - 1]);
        table.pop_back();
    }

};

/*
    This is a wrapper around some kind of hash table,
    right now its implemented with a map, but
    would probably be best if it was a robin hood
    hashing hash table.
*/
// forward declare outward interface to HashMap
template<typename T>
class IHashMap;

template<typename T>
class HashMap {
public:
	using cinterface = IHashMap<T>;

	constexpr static bool needs_move = false;
	constexpr static bool dynamic_comp = true;

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
        assert(it != table.end());
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

/*
    This may be used to create component id's to components that
    are zero size.  It is used internally with the 'Active'
    component that signifies that the entity is active or not.
*/
// forward declare outward interface to EmptyHashMap
class IEmptyHashMap;

class EmptyHashMap {
public:
	using cinterface = IEmptyHashMap;

	constexpr static bool needs_move = false;
	constexpr static bool dynamic_comp = true;

    void insert(uint16_t index) { }
    void erase(uint16_t index) { }
};



/*
    This is a container that is a combination of the previous containers.
    It is a series of vectors that are grouped by entity signatures
    to make memory access more efficient.
*/

struct VMHash {
    VMHash() : signature(0), index(0) { }
    VMHash(uint16_t s, uint16_t i) : signature(s), index(i) { }
    uint16_t signature;
    uint16_t index;
};

// forward declare outward interface to VectorMap
template<typename T>
class IVectorMap;

template<typename T>
class VectorMap {
public:
	constexpr static bool needs_move = true;
	constexpr static bool dynamic_comp = false;

	using cinterface = IVectorMap<T>;
    using hash_type = VMHash;
    using table_type = std::map<uint16_t, std::vector<T>>;
    using vector_iterator = typename std::vector<T>::iterator;

private:
    using pair_type = std::pair<uint16_t, std::vector<T>>;
    table_type table;

public:
    using value_type = T;

    size_t size(uint16_t signature) const {
        size_t s = 0;
        typename table_type::const_iterator it = table.find(signature);
        if (it != table.end()) {
            s = it->second.size();
        }
        return s;
    }

    bool hasHash(hash_type hash) const {
        bool has = true;
        typename table_type::const_iterator it = table.find(hash.signature);
        if (it == table.end() || hash.index >= it->second.size())
            has = false;
        return has;
    }

    T& operator[](hash_type hash) {
        typename table_type::iterator it = table.find(hash.signature);
        assert(it != table.end() && hash.index < it->second.size());
        return it->second[hash.index];
    }

    Ref<T> find(hash_type hash) {
        Ref<T> ref;
        typename table_type::iterator it = table.find(hash.signature);
        if (it != table.end() && hash.index < it->second.size()) {
            ref = Ref<T>( &(it->second[hash.index]) );
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

    /*
        This function will move a component from one hash to another as
        signatures of entities change.
    */
    void move(hash_type oldHash, hash_type newHash) {
        typename table_type::iterator newSig = table.find(newHash.signature);
        if (newSig == table.end()) {
            if (newHash.signature == 0) return;
            auto ret = table.insert(pair_type(newHash.signature, std::vector<T>()));
            newSig = ret.first; // BUG?!
        }

        if (newSig->second.size() <= newHash.index) {
            newSig->second.resize(newHash.index + 1);
        }

        typename table_type::iterator oldSig = table.find(oldHash.signature);

        if (oldSig != table.end()) {
            std::swap(oldSig->second[oldHash.index], newSig->second[newHash.index]);
        }
    }

};



#endif // CONTAINER_HPP
