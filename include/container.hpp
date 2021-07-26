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





template<typename T>
class Table {
public:
    Table() : m_capacity(0), m_size(0), m_list(nullptr) { }
    Table(int32_t s) : m_capacity(s), m_size(s), m_list(new T[s]) { }
    Table(const Table& other) : m_capacity(other.m_capacity), m_size(other.m_size), m_list(new T[other.m_capacity]) {
        std::memcpy(m_list, other.m_list, other.m_size * sizeof(T));
    }

    ~Table() {
        if (m_list != nullptr) {
            delete[] m_list;
        }
        m_list = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    Table& operator=(Table other) {
        swap(other);
        return *this;
    }

    void resize(int32_t newSize) {
        if (m_capacity < newSize) {
            Table tmp( (int32_t)nextPowerOf2((uint32_t)newSize) );
            std::memcpy(tmp.m_list, m_list, m_size * sizeof(T));
            swap(tmp);
        }
        m_size = newSize;
    }

    void smartCopy(const Table& other) {
        if (other.m_size > m_capacity) {
            Table tmp( (int32_t)nextPowerOf2((uint32_t)other.m_size) );
            swap(tmp);
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
    void swap(Table& other) {
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_size, other.m_size);
        std::swap(m_list, other.m_list);
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




class PoolTable : Table<uint16_t> {
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
            if (m_size == m_capacity) {
                // don't do anything
                // resize it!
            } else {
                value = (uint16_t)m_size;
                ++m_size;
                m_list[value] = EndOfList;
            }
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






class Components {
public:


protected:


};




#endif // CONTAINER_HPP
