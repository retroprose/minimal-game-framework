#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <cassert>
#include <cstring>
#include <vector>
#include <map>




/*
    This container is simply a wrapper around a vector,
    all objects are stored sequentially in memory.
*/
template<typename T>
class Vector {
protected:
    using table_type = std::vector<T>;
    table_type table;

public:
    using value_type = T;

    struct iterator {
    public:
        iterator() : table(nullptr), index(-1) { }
        iterator(table_type& t, int32_t i) : table(&t), index(t) { }

        bool valid() const {
            return table != nullptr && index >= 0 && index < table.size();
        }

        T& get() {
            assert( valid() );
            return (*table)[index];
        }

        void set(const T& t) {
            assert( valid() );
            (*table)[index] = t;
        }

        T& operator *() {
            assert( valid() );
            return (*table)[index];
        }

        T* operator -> () {
            assert( valid() );
            return &((*table)[index]);
        }

    private:
        table_type* table;
        int32_t index;
    };

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

    iterator find(uint16_t index = 0) {
        return iterator(table, index);
    }

	iterator begin() {
		return iterator(table, 0);
	}

	iterator end() {
		return iterator(table, table.size());
	}

    void sort() {
        std::stable_sort(table.begin(), table.end());
    }

    void insert(uint16_t index) {
        if ((uint32_t)(index + 1) > table.size()) {
            table.resize((uint32_t)(index + 1));
        }
    }

	void append(const T& t) {
		table.push_back(t);
	}

    void erase(uint16_t index) {
        assert(index >= 0 && index < table.size());
        std::swap(table[index], table[table.size() - 1]);
        table.pop_back();
    }

};



template<typename T>
class Sparse {
public:
    using table_type = std::map<uint16_t, T>;

protected:
    using pair_type = std::pair<uint16_t, T>;
    table_type table;

public:
    using value_type = T;

    struct iterator {
    public:
        iterator() : _valid(false) { }
        iterator(typename table_type::iterator i, typename table_type::iterator e) : _valid(true), _it(i), _end(e) { }

        bool next() {
            ++_it;
            return valid();
        }

        uint16_t index() {
            return _it->first;
        }

        bool valid() const {
            return !(valid == false || _it == _end);
        }

        T& get() {
            assert( valid() );
            return _it->second;
        }

        void set(const T& t) {
            assert( valid() );
            _it->second = t;
        }

        T& operator *() {
            assert( valid() );
            return _it->second;
        }

        T* operator -> () {
            assert( valid() );
            return &(_it->second);
        }

    private:
        bool _valid;
        typename table_type::iterator _it;
        typename table_type::iterator _end;
    };

    size_t size() const {
        return table.size();
    }

    bool hasIndex(uint16_t index) const {
        return table.find(index) != table.end();
    }

    T& operator[](uint16_t index) {
        auto it = table.find(index);
        assert(it != table.end());
        return *it;
    }

    iterator find(uint16_t index = 0) {
        iterator ret;
        if (index == 0) {
            ret = iterator(table.begin(), table.end());
        } else {
            ret = iterator(table.find(index), table.end());
        }
        return ret;
    }

    iterator begin() {
        return iterator(table.begin(), table.end());
    }

    iterator end() {
        return iterator(table.end(), table.end());
    }

    iterator insert(uint16_t index) {
        //auto ret = table.insert( pair_type(index, T()) );
        //return Ref<T>( &(*(ret.first)) );
        auto res = table.insert( pair_type(index, T()) );
        return iterator(res.first, table.end());
    }

    void erase(uint16_t index) {
        typename table_type::iterator it = table.find(index);
        if (it != table.end())
            table.erase(it);
    }

};



template<typename T>
class Dense {
public:
//protected:
    using table_type = std::vector<std::vector<T>>;
    table_type table;

    static uint16_t getChunk(uint16_t i) { return (uint16_t)((0xe000 & i) >> 13); }
    static uint16_t getIndex(uint16_t i) { return (uint16_t)(0x1fff & i); }
    static uint16_t fromChunkIndex(uint16_t c, uint16_t i) { return (c << 13) | i; }

public:
    using value_type = T;

    struct iterator {
    public:
        iterator() : table(nullptr), index(0), chunk(0) { }
        iterator(table_type& t, int32_t c, int32_t i) : table(&t), index(i), chunk(c) { }

        bool valid() const {
            return table != nullptr && chunk >= 0 && chunk < table->size() && index >= 0 && index < (*table)[chunk].size();
        }

        T& get() {
            assert( valid() );
            return (*table)[chunk][index];
        }

        void set(const T& t) {
            assert( valid() );
            (*table)[chunk][index] = t;
        }

        T& operator *() {
            assert( valid() );
            return (*table)[chunk][index];
        }

        T* operator -> () {
            assert( valid() );
            return &((*table)[chunk][index]);
        }

    private:
        table_type* table;
        int32_t index;
        int32_t chunk;
    };

	void clear() {
	    table.clear();
	    //for (uint32_t i = 0; i < table.size(); ++i) {
        //    table[i].clear();
	    //}
	}

	size_t sizeOfChunk(uint16_t c) const {
	    assert(c >= 0 && c < table.size());
        return table[c].size();
	}

	size_t sizeOfPartitions() const {
        return table.size();
    }

    size_t size() const {
        return 0;
    }

    const T& operator[](uint16_t h) const {
        int16_t chunk = getChunk(h);
        int16_t index = getIndex(h);
        assert(chunk >= 0 && chunk < table.size() && index >= 0 && index < table[chunk].size());
        return table[chunk][index];
    }

    T& operator[](uint16_t h) {
        int16_t chunk = getChunk(h);
        int16_t index = getIndex(h);
        assert(chunk >= 0 && chunk < table.size() && index >= 0 && index < table[chunk].size());
        return table[chunk][index];
    }

    iterator find(uint16_t h = 0) {
        int16_t chunk = getChunk(h);
        int16_t index = getIndex(h);
        return iterator(table, chunk, index);
    }

	iterator begin() {
		return iterator(table, 0, 0);
	}

	iterator end() {
	    uint32_t c = table.size() - 1;
		return iterator(table, c, table[c].size() - 1);
	}

    void insert(uint16_t h) {
        int16_t chunk = getChunk(h);
        int16_t index = getIndex(h);
        if ((uint32_t)(chunk + 1) > table.size()) {
            table.resize((uint32_t)(chunk + 1));
        }
        if ((uint32_t)(index + 1) > table[chunk].size()) {
            table[chunk].resize((uint32_t)(index + 1));
        }
    }

    void erase(uint16_t h) {
        // don't need to do anything!
    }

};



class FreeList : public Dense<uint16_t> {
public:
//private:
    std::vector<uint16_t> head_table;

public:
    static const uint16_t EndOfList = 0x0000;

    void resize(int32_t chunk) {
        table.resize(chunk + 1);
        head_table.resize(chunk + 1);
        reset();
    }

    void reset() {
        //reset stuff
        uint16_t eol = EndOfList;
        for (uint32_t i = 0; i < table.size(); ++i) {
            head_table[i] = eol;
            table[i].clear();
            table[i].push_back(eol);
        }
    }

    uint16_t allocate(uint32_t chunk) {
        uint16_t eol = EndOfList;
        uint16_t& head = head_table[chunk];
        std::vector<uint16_t>& list = table[chunk];
        uint16_t index = head;
        if (index == eol) {
            index = list.size();
            list.push_back(eol);
        } else {
            head = list[index];
            list[index] = eol;
        }
        return fromChunkIndex(chunk, index);
    }

    void free(uint16_t value) {
        uint16_t eol = EndOfList;
        uint16_t index = getIndex(value);
        uint16_t chunk = getChunk(value);
        uint16_t& head = head_table[chunk];
        std::vector<uint16_t>& list = table[chunk];
        if (list[index] == eol) {
            list[index] = head;
            head = index;
        }
    }

};




#endif // CONTAINER_HPP
