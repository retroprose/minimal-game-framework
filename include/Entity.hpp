#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <vector>
#include <map>
#include <algorithm>
#include <initializer_list>


#include <typeinfo>

#include <Error.hpp>


/*
    Signature
    Entity
    Hash
    Ref

    CpInterface
        Concrete
        Any

        CpStatic loop, make any, interface

    State
        ProxyContainer

        EntityManager
        HashManager

    VectorMap
    EntityMap


    uint16_t head;
    uint16_t tail;
    std::vector<GenerationInfo> ginfo;
    std::vector<Hash> hashes;

    hash free_map

    needs activeData
    needs entityData
*/




class Signature {
public:

    template<uint8_t... Args>
    static Signature make(const uint8_t* mapper) {
        Signature signature;
        setBits<0, Args...>(mapper, signature);
        return signature;
    }

    Signature() {
        bits = 0x00000000;
    }

    Signature(const uint8_t* mapper, std::initializer_list<uint8_t> list) {
        bits = 0x00000000;
        const uint8_t* src = list.begin();
        for (;;) {
            if (src == list.end()) break;
            bits |= 0x00000001 << mapper[*src];
            ++src;
        }
    }

    bool operator[](uint8_t i) const {
        return bits & ( 0x00000001 << i );
    }

    uint16_t staticMask() const {
        return bits & 0x0000ffff;
    }

    uint16_t dynamicMask() const {
        return (bits & 0xffff0000) >> 16;
    }

private:
    template<uint16_t INDEX, uint8_t C>
    static void setBits(const uint8_t* mapper, Signature& signature) {
        signature.bits |= 0x00000001 << mapper[INDEX];
    }

    template<uint16_t INDEX, uint8_t C1, uint8_t C2, uint8_t ... Args>
    static void setBits(const uint8_t* mapper, Signature& signature) {
        const uint8_t* value = mapper[INDEX];
        signature.bits |= 0x00000001 << mapper[INDEX];
        setBits<INDEX + 1, C2, Args...>(signature);
    }

    uint32_t bits;

};


struct Hash {
public:
    Hash() { data.raw.value = 0x00000000; }

    bool operator< (const Hash& rhs) const { return data.raw.value <  rhs.data.raw.value; }
    bool operator==(const Hash& rhs) const { return data.raw.value == rhs.data.raw.value; }
    bool operator!=(const Hash& rhs) const { return data.raw.value != rhs.data.raw.value; }

    uint16_t signature() const {
        return data.si.signature;
    }

    uint16_t index() const {
        return data.si.index;
    }

    uint32_t raw() const {
        return data.raw.value;
    }

    Hash(uint32_t in) { data.raw.value = in; }
    Hash(uint16_t s, uint16_t i) { data.si.signature = s; data.si.index = i; }


private:
    struct SignatureIndex {
        uint16_t signature;
        uint16_t index;
    };

    struct Raw {
        uint32_t value;
    };

    Hash(SignatureIndex in) { data.si = in; }

    union {
        SignatureIndex si;
        Raw raw;
    } data;
};


class Entity {
public:
    Entity() { data.raw.value = 0x00000000; }

    bool operator< (const Entity& rhs) const { return data.raw.value <  rhs.data.raw.value; }
    bool operator==(const Entity& rhs) const { return data.raw.value == rhs.data.raw.value; }
    bool operator!=(const Entity& rhs) const { return data.raw.value != rhs.data.raw.value; }

    bool untracked() const {
        return data.tracked.untracked;
    }

    uint16_t index() const {
        ASSERT(data.tracked.untracked == 0);
        return data.tracked.untracked;
    }

    uint16_t generation() const {
        ASSERT(data.tracked.untracked == 0);
        return data.tracked.generation;
    }

    uint16_t type() const {
        ASSERT(data.tracked.untracked == 1);
        return data.untracked.type;
    }

    uint16_t user() const {
        ASSERT(data.tracked.untracked == 1);
        return data.untracked.user;
    }

    uint32_t raw() const {
        return data.raw.value;
    }

private:
    friend class EntityManager;

    struct Tracked {
        uint32_t index : 16;        // index of entity
        uint32_t generation : 15;   // generation of entity
        uint32_t untracked : 1;     // should always be zero
    };

    struct Untracked {
        uint32_t user : 24;         // user data, can be used any way you want
        uint32_t type : 7;          // type of entity
        uint32_t untracked : 1;     // should always be 1
    };

    struct Raw {
        uint32_t value;
    };

    Entity(uint32_t in) { data.raw.value = in; }
    Entity(Tracked in) { data.tracked = in; }
    Entity(Untracked in) { data.untracked = in; }


    union {
        Tracked tracked;
        Untracked untracked;
        Raw raw;
    } data;

};


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




/*

class CpInterface {
public:
    using index_type = uint32_t;

    template <class T, class Tuple>
    struct TupleTypeIndex;

    template <class T, class... Types>
    struct TupleTypeIndex<T, std::tuple<T, Types...>> {
        static const std::size_t value = 0;
    };

    template <class T, class U, class... Types>
    struct TupleTypeIndex<T, std::tuple<U, Types...>> {
        static const std::size_t value = 1 + TupleTypeIndex<T, std::tuple<Types...>>::value;
    };




};
*/



/*
    Signature
    Entity
    Hash
    Ref

    CpInterface
        Concrete
        Any

        CpStatic loop, make any, interface

    State
        ProxyContainer

        EntityManager
        HashManager

    VectorMap
    EntityMap

*/




template<typename... Ts>
class State {
public:
    static bool getBit(uint8_t bits, uint8_t index)     { return bits & (0x01 << index); }
    static bool setBit(uint8_t& bits, uint8_t index)    { bits |=  (0x01 << index); }
    static bool unsetBit(uint8_t& bits, uint8_t index)  { bits &= ~(0x01 << index); }

    using pack_type = std::tuple<Ts...>;
    using pack_value_type = std::tuple<typename Ts::value_type...>;
    using pack_ref_type = std::tuple<Ref<typename Ts::value_type>...>;

    //constexpr tuple_index_list = std::tuple<  >;

    //constexpr static CpInterface::index_type entityIndex = std::tuple_size<pack_type>::value - 1;
    //constexpr static CpInterface::index_type activeIndex = std::tuple_size<pack_type>::value - 2;

    pack_type pack;

    constexpr static uint16_t EndOfList = 0xffff;

    struct GenerationInfo {
        uint16_t generation : 15;
        uint16_t unsafe : 1;
    };

    struct FreeInfo {
        uint16_t head;
        uint16_t size;
    };

    uint16_t head;
    uint16_t tail;
    std::vector<GenerationInfo> ginfo;
    std::vector<Hash> hashes;
    std::map<uint16_t, FreeInfo> freeMap;

    // const map from sig_index to component_index
    // const map from component_index to sig_index
    // const uint32_t only_static_bits;

    /*
    *Extend
    ReserveSignature  // needs comps

    *Create
    Destroy entity

    *Valid
    GetSignature

    ChangeSignature entity, signature   // needs comps




    *EntityFromHash
    *HashFromEntity

    ForEach
    */

    /*
    static void make_mapper() {

        uint32_t dynamicIndex = 0, staticIndex = 0;

        map_to_component
        map_to_signature

        for each INDEX in tuple {
            map_to_signature[INDEX] = dynamicIndex;
            map_to_component[dynamicIndex] = INDEX;
            ++dynamicIndex;
        }


    }
*/


private:



};





#endif // ENTITY_HPP
