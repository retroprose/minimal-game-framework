#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <cassert>
#include <cstring>
#include <fstream>


/*
    Generic table
*/
template<typename T>
class Table {
public:
    Table() : m_capacity(0), m_size(0), m_list(nullptr) { }
    Table(int32_t s) : m_capacity(s), m_size(s), m_list(new T[s]) { }
    ~Table() { destroy(); }

    Table& operator=(const Table&) = delete;
    Table(const Table&) = delete;

    void destroy() {
        if (m_list != nullptr) {
            delete[] m_list;
        }
        m_list = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    void resize(int32_t newSize) {
        if (m_capacity < newSize) {
            // temp variables
            T* tmpList = m_list;

            // create new list
            m_capacity = nextSize(newSize);
            m_list = new T[m_capacity];

            // copy to new list
            std::memcpy(m_list, tmpList, m_size * sizeof(T));

            // delete old list
            if (tmpList != nullptr) {
                delete[] tmpList;
            }
        }
        m_size = newSize;
    }

    void smartCopy(const Table& other) {
        if (m_capacity < other.m_size) {
            if (m_list != nullptr) {
                delete[] m_list;
            }
            m_capacity = nextSize(other.m_size);
            m_list = new T[m_capacity];
        }
        std::memcpy(m_list, other.m_list, other.m_size * sizeof(T));
        m_size = other.m_size;
    }

    void clear() {
        m_size = 0;
    }

    bool append() {
        resize(m_size + 1);
        return true;
    }

    bool insert(int32_t index) {
        if (m_size < index + 1) {
            resize(index + 1);
        }
        return true;
    }

    void sort() {
        std::sort(m_list, m_list + m_size);
    }

    int32_t capacity() {
        return m_capacity;
    }

    int32_t size() {
        return m_size;
    }

    T& operator[](int32_t i) {
        return m_list[i];
    }

    T& last() {
        return m_list[m_size - 1];
    }

protected:
    int32_t nextSize(int32_t s) {
        return (int32_t)nextPowerOf2( (uint32_t)s );
    }

    uint32_t nextPowerOf2(uint32_t n) {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
    }

    int32_t m_capacity;
    int32_t m_size;
    T* m_list;

};




class PoolTable : public Table<uint16_t> {
public:
    static const uint16_t EndOfList = 0;

    PoolTable() : Table(), m_head(EndOfList) {
        reset();
    }

    void smartCopy(const PoolTable& other) {
        m_head = other.m_head;
        Table::smartCopy(other);
    }

    void reset() {
        resize(1);
        m_head = EndOfList;
        m_list[0] = EndOfList;
    }

    uint16_t allocate() {
        uint16_t value = m_head;
        if (value == EndOfList) {
            value = (uint16_t)m_size;
            resize(m_size + 1);
            m_list[value] = EndOfList;
        } else {
            m_head = m_list[value];
            m_list[value] = EndOfList;
        }
        return value;
    }

    void free(uint16_t value) {
        if (m_list[value] == EndOfList) {
            m_list[value] = m_head;
            m_head = value;
        }
    }

protected:
    uint16_t m_head;

};








/*
    Class that represents an entity, is simply a 32 bit unsigned integer.
*/

class Entity {
public:
    Entity() { }
    Entity(uint16_t i) : _index(0), _generation(0) { }
    Entity(uint16_t i, uint16_t g) : _index(i), _generation(g) { }

    bool isNull() { return _index == 0 && _generation == 0; }

    uint16_t index() { return _index; }
    uint16_t generation() { return _generation; }

private:
    uint16_t _index;
    uint16_t _generation;
};







class Components {
public:
    void resize(int s) {
        _manager.resize(s);
        _generation.resize(s);
    }

    void smartCopy(const Components& other) {
        _manager.smartCopy(other._manager);
        _generation.smartCopy(other._generation);
    }

    bool valid(Entity entity) {
        return !entity.isNull() && entity.generation() == _generation[entity.index()];
    }

    Entity fromIndex(uint16_t index) {
        return Entity(index, _generation[index]);
    }

    void destroy(Entity entity) {
        if (valid(entity) == true) {
            ++_generation[entity.index()];
            _manager.free( entity.index() );
        }
    }

    Entity create() {
        Entity entity(0);
        uint16_t value = _manager.allocate();
        if (value != PoolTable::EndOfList) {
            _generation.insert(value);  // this will be tricky
            entity = Entity(value, _generation[value]);
        }
        return entity;
    }

    void clear() {
        for (int32_t i = 0; i < _generation.capacity(); ++i) {
            _generation[i] = 0;
        }
        resize(0);
        _manager.reset();
    }

protected:
    PoolTable _manager;
    Table<uint16_t> _generation;

};




#endif // CONTAINER_HPP
