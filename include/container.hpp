#ifndef CONTAINER_HPP
#define CONTAINER_HPP


/*
    interface!
    public interface IContainer
    {
        void Copy(IContainer other);
        bool Insert(int index);
        void Resize(int size);
        //void CopyPaste(int from, int to);
    }
*/

#include <cstring>


template<typename T>
class Table {
public:
    Table() : m_capacity(0), m_size(0), m_list(nullptr) { }
    Table(int32_t s) : m_capacity(s), m_size(s), m_list(new T[s]) { }

    ~Table() {
        destroy();
    }

    void destroy() {
        if (m_list != nullptr) {
            delete[] m_list;
        }
        m_list = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    void swap(Table& other) {
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_size, other.m_size);
        std::swap(m_list, other.m_list);
    }

    Table& operator=(const Table& other) {
        if (other.m_capacity > m_capacity) {
            Table tmp(other.m_capacity);
            swap(tmp);
        }
        m_size = other.m_size;
        std::memcpy(m_list, other.m_list, other.m_size * sizeof(T));
        return *this;
    }

    void resize(int32_t newSize) {
        int32_t s = m_capacity;
        while (newSize > s) {
            s += s;
        }
        if (s > m_capacity) {
            Table tmp(s);
            std::memcpy(tmp.m_list, m_list, m_size * sizeof(T));
            swap(tmp);
        }
        m_size = s;
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

    // get by index

    // get last

protected:
    int32_t m_capacity;
    int32_t m_size;
    T* m_list;

};



#endif // CONTAINER_HPP
