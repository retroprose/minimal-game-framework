#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <initializer_list>

#include <Error.hpp>

#include <fstream>
#include <iostream>

/*
    State<P>
    BaseEntityRef

    Ref<T>
    VectorMap<T>
    VectorMapHash
    AnyVectorMap

    Entity
    Signature
    GenerationData



    generation uint16_t
    signature uint16_t

    index uint16_t
    entityIndex uint16_t

    8 bytes

*/


/*
    Forward declare classes used as friend classes
*/
template<typename T>
class BaseState;
class EntityManager;

/*
    The Signature class stores a static list of component id's in
    ascending order to track which components are attached to what entity.
    You could make this a bit string, but this allows you to have a larger
    number of component id's without having to traverse every bit
    any time you want to add or remove all components in a signature.

    These signature will be stored into an array so they can be identified
    by a single 16 bit value.  More than likely every combination of components
    will not be used.
*/
class Signature {
public:
    // the array is defined as an array of 32 bit integers to make comparing faster
    constexpr static const uint8_t MaxArrayLength = 6;
    constexpr static const uint8_t MaxComponents = MaxArrayLength * 4;
    constexpr static const uint8_t NullIndex = 0xff;
    constexpr static const uint8_t NullCp = 0xff;

    /*
        We need this static function becuase you can't have a
        template constructor on a non-template class.
        We can use this to create a list from a compile time
        list of component id's, which helps us generate code
        that doesn't loop though component ids.

        This function asserts if the list is longer than
        MaxComponents minus 1, the last element is a Null
        Component used in splicing lists.  It also asserts
        if the list has any duplicates.
    */
    template<uint8_t... Args>
    static Signature Gen() {
        ASSERT(sizeof...(Args) < MaxComponents - 1);
        Signature s;
        SetElements<0, Args...>(s);
        std::sort(s.begin(), s.end());
        ASSERT(!s.HasDuplicates());
        return s;
    }

    /*
        Defaults to empty signature
    */
    Signature() : _size(0) {
        Init();
    }

    /*
        Signature created from uint8_t list literal.  It also asserts if
        the list is larger than MaxComponents -1 or there are any duplicates.
    */
    Signature(std::initializer_list<uint8_t> list) : _size(0) {
        ASSERT(list.size() < MaxComponents - 1);
        Init();
        uint8_t* first = begin();
        uint8_t* dest = first;
        const uint8_t* src = list.begin();
        _size = list.size();
        for (uint32_t i = 0; i < _size; ++i) {
            *dest = *src;
            ++src;
            ++dest;
        }
        std::sort(first, dest);
        ASSERT(!HasDuplicates());
    }

    /*
        The + operator will splice two signatures together.  It will
        assert if the combined size is larger than MaxComponents -1
    */
    Signature operator+(const Signature &rhs) const {
        const Signature& lhs = *this;
        ASSERT(lhs.Size() + rhs.Size() < Signature::MaxComponents - 1);
        Signature ret;
        uint8_t *it = ret.begin();
        const uint8_t *it_lhs = lhs.begin(), *it_rhs = rhs.begin();
        for(;;) {
            if ( *it_lhs == *it_rhs ) {
                if (*it_lhs == Signature::NullCp) return ret;
                *it = *it_lhs;
                ++it;
                ++ret._size;
                ++it_rhs;
                ++it_lhs;
            } else if ( *it_lhs < *it_rhs ) {
                *it = *it_lhs;
                ++it;
                ++ret._size;
                ++it_lhs;
            } else {
                *it = *it_rhs;
                ++it;
                ++ret._size;
                ++it_rhs;
            }
        }
    }

    /*
        The - operator will return a signature with the components
        in the right hand side removed from the left hand side.
    */
    Signature operator-(const Signature &rhs) const {
        const Signature& lhs = *this;
        Signature ret;
        uint8_t *it = ret.begin();
        const uint8_t *it_lhs = lhs.begin(), *it_rhs = rhs.begin();
        for(;;) {
            if ( *it_lhs == *it_rhs ) {
                if (*it_lhs == Signature::NullCp) return ret;
                ++it_rhs;
                ++it_lhs;
            } else if ( *it_lhs < *it_rhs ) {
                *it = *it_lhs;
                ++it;
                ++ret._size;
                ++it_lhs;
            } else {
                ++it_rhs;
            }
        }
    }

    /*
        The less than operator is defined so that we can use
        signatures as keys in maps.  There's no logical reason
        for one to be greater than another.  It is optimized
        to use a uint32_t array to reduce comparisons.
        I'm not sure if the memcmp function would be optimized
        like this, comparing 4 bytes at a time, or if it's even
        possible.
    */
    bool operator<(const Signature& rhs) const {
        uint8_t r = 0;
        while (bits[r] == rhs.bits[r] && r < MaxArrayLength - 1) ++r;
        return (bits[r] < rhs.bits[r]);
    }

    /*
        Begin and end define the begining and end of list of
        component id's, here so we can use for ( const auto& comp_id : signature ) { }

        This uses a bit of hackery to interpret the uint32_t array as an array of
        uint8_t bytes.
    */
    const uint8_t* begin() const {
        return reinterpret_cast<const uint8_t*>(bits);
    }

    const uint8_t* end() const {
        return begin() + _size;
    }

    /*
        Bracket operator lets us access component at specific index.
        The last null component is a valid index, this asserts if
        we try to get an index out of range.
    */
    const uint8_t& operator[](int i) const {
        ASSERT(i >= 0 && i < MaxComponents);
        return begin()[i];
    }

    /*
        Returns index where component 'k' is found
    */
    uint8_t GetIndexOf(uint8_t k) const {
        const uint8_t* it = begin();
        uint8_t i = 0;
        for(;;) {
            if (*it == NullCp) return NullIndex;
            if (*it == k) return i;
            ++it;
            ++i;
        }
    }

    /*
        Returns true if 'k' is in the list of components, false otherwise
    */
    bool Has(uint8_t k) const {
        return (GetIndexOf(k) != NullIndex);
    }

    /*
        If this signature contains ALL component id's in the passed
        signature, this function returns true, false otherwise
    */
    bool Contains(const Signature& rhs) const {
        const uint8_t *it_lhs = begin(), *it_rhs = rhs.begin();
         for(;;) {
            if ( *it_rhs == NullCp ) return true;
            if ( *it_lhs == NullCp ) return false;
            if ( *it_lhs == *it_rhs ) ++it_rhs;
            ++it_lhs;
        }
    }

    /*
        Returns how many component id's are in the signature
        (excluding the null component)
    */
    uint16_t Size() const {
        return _size;
    }

private:
    /*
        Helper functions to create a template list
        of component ids
    */
    template<uint16_t INDEX, uint8_t C>
    static void SetElements(Signature& s) {
        s.begin()[INDEX] = C;
        ++s._size;
    }

    template<uint16_t INDEX, uint8_t C1, uint8_t C2, uint8_t ... Args>
    static void SetElements(Signature& s) {
        s.begin()[INDEX] = C1;
        ++s._size;
        SetElements<INDEX + 1, C2, Args...>(s);
    }

    /*
        Initilizes all components to null component.
    */
    void Init() {
        uint32_t* p = bits;
        *p = 0xffffffff; ++p;
        *p = 0xffffffff; ++p;
        *p = 0xffffffff; ++p;
        *p = 0xffffffff; ++p;
        *p = 0xffffffff; ++p;
        *p = 0xffffffff; ++p;
    }

    /*
        Non-const begin and end are private because
        users can't modify signatures.
    */
    uint8_t* begin() {
        return reinterpret_cast<uint8_t*>(bits);
    }

    uint8_t* end() {
        return begin() + _size;
    }

    /*
        Helper function for duplicate asserts.
        Returns true if there are any duplicates,
        false otherwise.
    */
    bool HasDuplicates() const {
        const uint8_t* it = begin();
        for(;;) {
            if (*it == NullCp) return false;
            if (*it == *(it + 1)) return true;
            ++it;
        }
    }

    uint32_t bits[MaxArrayLength];  // array of uint32_t (4 components per entry)
    uint8_t _size;                  // size of signature component list
};



/*
    A smart reference that is almost identical to a raw pointer,
    with the exception that it is initilized to nullptr, and
    will assert if you try and dereference a null pointer.
*/
template<typename T>
class Ref {
public:
    /*
        This constructor is used to turn a raw byte pointer
        back into a type pointer
    */
    Ref(uint8_t* p) : ptr( reinterpret_cast<T*>(p) ) { }

    /*
        This constructor simply sets the pointer to passed pointer
    */
    Ref(T* p = nullptr) : ptr(p) { }

    /*
        The next 4 operators are your basic
        dereference operations
    */
    T& operator *() {
        ASSERT( !IsNull() );
        return *ptr;
    }

    T* operator -> () {
        ASSERT( !IsNull() );
        return ptr;
    }

    const T& operator *() const {
        ASSERT( !IsNull() );
        return *ptr;
    }

    const T* operator -> () const {
        ASSERT( !IsNull() );
        return ptr;
    }

    /*
        Returns true if the reference is null, false otherwise.
    */
    bool IsNull() const {
        return ptr == nullptr;
    }

    /*
        This is used for a bit of dynamic hackery when operating
        on components with variable component id's.
    */
    uint8_t* BytePtr() { return reinterpret_cast<uint8_t*>(ptr); }

private:
    T* ptr; // pointer

};

/*
    The VectorMap is the container that partitions components in memory so that
    components with the same signature are close in memory.  This allows
    the component system 'ForEach' loop have better cache efficentcy.

    I went for this method because I didn't like the idea of just ignoring
    all the components on an entity that aren't used, and having to iterate
    every entity and check the signature to see if if the entity signature matches.
    With this model there is zero memory overhead for unused components.

    Components end up being chunked together by signature.  This way you only iterate
    signatures and traverse all entities with that signature without having to make any
    checks at all (you know they are all active and match that signature!)
*/

/*
    This 32 bit structure is the hash value that individual components are
    accessed by.  The structure of the container is simply a hash map of vectors.
    So each signature has its own vector of components, and they are moved from
    one vector to another when signatures are changed.  Notice signatures are simply
    16 bit values, these values index into an array of our 24 byte signature lists.
*/
struct VectorMapHash {
    uint16_t signature;
    uint16_t index;
};

template<typename T>
class VectorMap {
private:
    /*
        I'm using the standard library to implement this container,
        but it could be done without to remove dependency on the
        standary library.  The only two standard containers I'm using
        are maps and vectors, and algorithms to sort vectors.
    */
    using table_type = std::unordered_map<uint16_t, std::vector<T>>;
    table_type table;

public:
    /*
        Returns the size of vector for a given signature (0 if vector is empty or doesn't exist)
    */
    size_t Size(uint16_t signature) const {
        size_t s = 0;
        typename table_type::const_iterator it = table.find(signature);
        if (it != table.end())
            s = it->second.size();
        return s;
    }

    /*
        These two functions allow us to use for( const auto& signature : vectormap ) { }
        It's probably possible make iterators that iterate individual components
        (I would like to do this) but the for each for components requires a bit
        more logic and was simpler to write as a function that takes a lambda.
    */
    typename table_type::const_iterator begin() const {
        return table.begin();
    }

    typename table_type::const_iterator end() const {
        return table.end();
    }

    /*
        Returns a reference to component at given signature and index.
        Returns a null reference if it is not found.
    */
    Ref<T> Get(VectorMapHash hash) {
        Ref<T> ref;
        typename table_type::iterator it = table.find(hash.signature);
        if (it != table.end() && hash.index >= 0 && hash.index < it->second.size())
            ref = Ref<T>( &(it->second[hash.index]) );
        return ref;
    }

    /*
        Registers a signature with this vector map, and reserves
        the amount of memory given.  These vectors can be implemented to have
        static memory sizes if you don't want to use the standard library and
        don't want write your own dynamic memory management.

        In a previous version of this entity framework, Components were stored
        in a single giant array, and a map of signatures pointed to the first component
        with that signature, and stored a number of components in that signature.
        (each signature type had to be preallocated with a static number)
    */
    void Register(uint16_t signature, uint16_t count) {
        typename table_type::iterator it = table.find(signature);
        if (it == table.end()) {
            table[signature] = std::vector<T>();
            table[signature].reserve(count);
        }
    }

    /*
        This function moves a component at a position to a new position with the
        new given signature.  The new index can be determined by the 'Size'
        function after calling this function (components moved are always
        moved to the end of the new signature vector.  the signature
        they are moved from takes the last component in the array and
        swaps it with component being moved)
    */
    void Move(VectorMapHash old_hash, uint16_t new_hash_signature) {
        ASSERT(old_hash.signature != new_hash_signature);
        typename table_type::iterator old_sig = table.find(old_hash.signature);
        typename table_type::iterator new_sig = table.find(new_hash_signature);
        if (new_sig != table.end()) {
            std::vector<T>& new_vec = new_sig->second;
            uint16_t new_hash_index = new_vec.size();
            new_vec.push_back( T() );
            if (old_sig != table.end()) {
                std::vector<T>& old_vec = old_sig->second;
                new_vec[new_hash_index] = old_vec[old_hash.index];
                old_vec[old_hash.index] = old_vec[old_vec.size() - 1];
                //std::swap(old_vec[old_hash.index], old_vec[old_vec.size() - 1]);
                old_vec.pop_back();
            }
        } else {
             if (old_sig != table.end()) {
                std::vector<T>& old_vec = old_sig->second;
                old_vec[old_hash.index] = old_vec[old_vec.size() - 1];
                //std::swap(old_vec[old_hash.index], old_vec[old_vec.size() - 1]);
                old_vec.pop_back();
            }
        }
    }

};



/*
    This class is used as your main entity reference mechanism.  It uses a bit of
    union hackery because there are two types of entities, tracked and untracked.

    Tracked entities use generation values and index into the list of active
    entity values.  They can be checked for validity and be deleted.

    Untracked entities are always considered valid and active, and use the
    bits differently as there is no generation data needed to be tracked.
    These are useful for refering to things such as tiles in tile maps
    without having to use any extra memory for tracking.  For example
    you can have a component that doesn't use a VectorMap, but is a
    2 dimensional grid of tiles.  You would have a tile type value
    to determine entities that are of tile type, then the user data can be
    two values that index into that 2 dimensional array.
*/
class Entity {
public:
    struct tracked_t {
        uint32_t index : 16;        // index of entity
        uint32_t generation : 15;   // generation of entity
        uint32_t untracked : 1;     // should always be zero
    };

    struct untracked_t {
        uint32_t user : 24;         // user data, can be used any way you want
        uint32_t type : 7;          // type of entity
        uint32_t untracked : 1;     // should always be 1
    };

    /*
        the only tracked entity we are allowed to make is the null entity
    */
    Entity() { data.raw.value = 0x00000000; }

    /*
        To create untracked entities
    */
    Entity(uint8_t t, uint16_t u) {
        data.untracked.untracked = 1;
        data.untracked.type = t;
        data.untracked.user = u;
    }

    /*
        Entities can be used as keys to maps, and compared to one another
    */
    bool operator< (const Entity& rhs) const { return data.raw.value <  rhs.data.raw.value; }
    bool operator==(const Entity& rhs) const { return data.raw.value == rhs.data.raw.value; }
    bool operator!=(const Entity& rhs) const { return data.raw.value != rhs.data.raw.value; }

    /*
        Return data in different ways.  In order for this hackery to work,
        the untracked bit of both tracked_t and untracked_t have to line up.
    */
    uint32_t Raw() const { return data.raw.value; }
    tracked_t Tracked() const { return data.tracked; }
    untracked_t Untracked() const { return data.untracked; }

private:
    /*
        Only the entity manager is allowed to create tracked entity values
    */
    friend class EntityManager;

    Entity(uint32_t in) { data.raw.value = in; }
    Entity(tracked_t in) { data.tracked = in; }
    Entity(untracked_t in) { data.untracked = in; }

    struct raw_t {
        uint32_t value;
    };

    union {
        tracked_t tracked;
        untracked_t untracked;
        raw_t raw;
    } data;

};

/*
    This class is based on another bit of hackery, it uses a form
    of type erasure to get generic references to the different types
    of vector maps.  It's basically like a delegate.  There are probably
    better ways to write this, but I wanted to get polymophism without
    class hiearchies and without having to allocate memory, AND making
    the class simple to understand and debug.

    There are plenty of articles stemming from the first in 2005 of
    "Member Function Pointers and the Fastest Possible C++ Delegates"
    just google "fastest possible C++ delegates".

    Link to original article: https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible
*/
class AnyVectorRef {
private:
    /*
        These 8 bytes account for the 4 for the vtable pointer, and 4 for the pointer to the object itself.
        If this needs to be compiled for 64 bit, it needs to be 16 bytes...
    */
    constexpr static const size_t MaxBuffer = 8;

    struct base {
        virtual void Move(VectorMapHash old_hash, uint16_t new_hash_signature) = 0;
        virtual void Register(uint16_t signature, uint16_t count) = 0;
        virtual uint8_t* Get(VectorMapHash hash) = 0;
    };

    template<typename T>
    struct derived : public base {
        derived(T& t) : r(t) { }
        void Move(VectorMapHash old_hash, uint16_t new_hash_signature) { r.Move(old_hash, new_hash_signature); }
        void Register(uint16_t signature, uint16_t count) { r.Register(signature, count); }
        uint8_t* Get(VectorMapHash hash) { return r.Get(hash).BytePtr(); }
        T& r;
    };

    base* Base() { return reinterpret_cast<base*>(_buffer); }

    /*
        This is a raw memory buffer than derived classes are newed onto.
    */
    uint8_t _buffer[MaxBuffer];

public:
    /*
        This should never happen...
    */
    AnyVectorRef() {
        ASSERT(false);
    }

    /*
        This constructor asserts if the buffer isn't large enough to contain the
        deived class, or if the resulting memory address of the new doesn't
        start at the start of the buffer.  This class makes the assumption that
        the pointer to base type will be the same as pointer to derived type
        since it's only single inheritance...  I'm not sure if this is completely
        garrenteed across all compilers though.  The assert may not even catch
        a condition that it isn't...    but it works for now.
    */
    template<typename T>
    AnyVectorRef(T& t) {
        ASSERT(sizeof(derived<T>) <= MaxBuffer);
        derived<T>* d = new (_buffer) derived<T>(t);
        ASSERT(reinterpret_cast<derived<T>*>(d) - reinterpret_cast<derived<T>*>(_buffer) == 0);
    }

    void Move(VectorMapHash old_hash, uint16_t new_hash_signature) { Base()->Move(old_hash, new_hash_signature); }
    void Register(uint16_t signature, uint16_t count) { Base()->Register(signature, count); }
    uint8_t* Get(VectorMapHash hash) { return Base()->Get(hash); }

};




/*
    This class is responsible for giving out entity values.  When an entry is active, the
    has vector is used to store the hash value where the entities components can be found
    in the vector maps.  When the entry is inactive, that hash is used as a linked list
    of unused slots that will be used to create new entities.
*/
class EntityManager {
public:

    struct GenerationInfo {
        uint16_t generation : 15;
        uint16_t unsafe : 1;
    };

    EntityManager() : head(END_OF_LIST), tail(END_OF_LIST) { }

    bool Valid(Entity entity) const {
        if ( entity.Tracked().untracked == 1 ||
             (entity.Tracked().index < ginfo.size() &&
              ginfo[entity.Tracked().index].generation == entity.Tracked().generation )
           ) {
           return true;
        }
        return false;
    }

    void Extend(uint16_t e) {
        uint16_t s = ginfo.size();
        Entity::tracked_t entity;
        entity.untracked = 0;
        entity.generation = 0;
        GenerationInfo info;
        info.generation = 0;
        info.unsafe = 0;
        for (uint16_t i = s; i < e; ++i) {
            ginfo.push_back(info);
            hashes.push_back( VectorMapHash() );
            entity.index = i;
            Destroy( Entity(entity) );
        }
    }

    Entity Create() {
        Entity::tracked_t entity;
        if (head == END_OF_LIST) {
            entity.index = ginfo.size();
            GenerationInfo info;
            info.generation = 1;
            info.unsafe = 0;
            ginfo.push_back(info);
            hashes.push_back( VectorMapHash() );
        } else {
            entity.index = head;
            head = hashes[entity.index].index;
            hashes[entity.index] = VectorMapHash();
            if (head == END_OF_LIST)
                tail = END_OF_LIST;
        }
        entity.untracked = 0;
        entity.generation = ginfo[entity.index].generation;
        return Entity(entity);
    }

    /*
    Entity CreateUnsafe() {
        Entity::tracked_t entity;
        if (head == END_OF_LIST) {
            entity.index = ginfo.size();
            GenerationInfo info;
            info.generation = 1;
            info.unsafe = 1;
            ginfo.push_back(info);
            hashes.push_back( VectorMapHash() );
        } else {
            ginfo[head].unsafe = 1;
            entity.index = head;
            head = hashes[entity.index].index;
            if (head == END_OF_LIST)
                tail = END_OF_LIST;
        }
        entity.untracked = 0;
        entity.generation = ginfo[entity.index].generation;
        return Entity(entity);
    }
    */

    void Destroy(Entity entity) {
        ASSERT(entity.Tracked().untracked == 0 && ginfo[entity.Tracked().index].generation == entity.Tracked().generation);
        if (ginfo[entity.Tracked().index].generation == 0x7fff) {
            ASSERT(false);
            /*ginfo[entity.Tracked().index].unsafe = 0;
            ginfo[entity.Tracked().index].generation = 1;
            hashes[entity.Tracked().index].index = END_OF_LIST;
            if (head == END_OF_LIST) {
                head = entity.Tracked().index;
            } else {
                hashes[tail].index = entity.Tracked().index;
            }
            tail = entity.Tracked().index;*/
        } else {
            // we won't incerment if unsafe is set to 1
            ginfo[entity.Tracked().index].unsafe = 0;
            ++ginfo[entity.Tracked().index].generation;
            hashes[entity.Tracked().index].index = END_OF_LIST;
            if (head == END_OF_LIST) {
                head = entity.Tracked().index;
            } else {
                hashes[tail].index = entity.Tracked().index;
            }
            tail = entity.Tracked().index;
        }
    }

    VectorMapHash& GetHash(uint16_t entity_index) {
        return hashes[entity_index];
    }

    const VectorMapHash& GetHash(uint16_t entity_index) const {
        return hashes[entity_index];
    }
/*
    VectorMapHash& GetHash(Entity entity) {
        ASSERT(entity.Tracked().untracked == 0 && ginfo[entity.Tracked().index].generation == entity.Tracked().generation);
        return hashes[entity.Tracked().index];
    }

    const VectorMapHash& GetHash(Entity entity) const {
        ASSERT(entity.Tracked().untracked == 0 && ginfo[entity.Tracked().index].generation == entity.Tracked().generation);
        return hashes[entity.Tracked().index];
    }
*/
    Entity FromIndex(uint16_t index) const {
        ASSERT(index < ginfo.size());
        Entity::tracked_t entity;
        entity.index = index;
        entity.generation = ginfo[index].generation;
        entity.untracked = 0;
        return Entity(entity);
    }

//private:
    static const uint16_t END_OF_LIST = 0xffff;

    uint16_t head;
    uint16_t tail;
    std::vector<GenerationInfo> ginfo;
    std::vector<VectorMapHash> hashes;
};



/*
    This class is the base for entity references.  See examples for how to make
    derived versions of this class.
*/
class BaseEntityRef {
public:
    Entity GetEntity() const { return entity; }
    VectorMapHash GetHash() const { return hash; }

protected:
    /*
        Only the BaseState class can manipulate this object.
    */
    template<typename T>
    friend class BaseState;

    /*
        Used to modify references as raw byte pointers.  This function
        assumes the memory under the first two member variables are
        all Ref<T> objects which take 4 bytes.  They will be interpreted
        as raw uint8_t values.
    */
    uint8_t** begin() {
        return reinterpret_cast<uint8_t**>(reinterpret_cast<uint8_t*>(this) + sizeof(BaseEntityRef));
    }

    Entity entity;
    VectorMapHash hash;

};





/*
    This is the class that stores the state of your game.

    void Extend(uint16_t e)
    uint16_t RegisterSigniture(const Signature& signature, uint16_t count = 0)
    Entity Create()


    const Signature& GetSignature(uint16_t index) const
    const Signature& GetSignature(Entity entity) const
    bool Valid(Entity entity) const
    VectorMapHash FromEntity(Entity entity) const
    Entity FromHash(VectorMapHash hash)

    typename T::EntityRef GetRef(Entity entity)
    void ForEach(F func)

    // The following functions can invalidate references and
    // should NOT be called within any ForEach lambda function
    void Destroy(Entity entity)
    void UpdateSignature(Entity entity, uint16_t signature_id)
    void UpdateSignature(Entity entity, const Signature& signature)
    void RemoveSignature(Entity entity, const Signature& signature)
    void AddSignature(Entity entity, const Signature& signature)
*/
template<typename T>
class BaseState {
public:
    static const uint16_t NullSignature = 0;

    /*
        The constructor of your state will set the first
        signature to the null signature (zero components)
        It has a unique trait that vectormaps do not have
        null signature vectors, so components are automatically
        erased when signature is set to null for an entity.
    */
    BaseState() {
        uint16_t index = signatures.size();
        Signature n;
        signatures.push_back(n);
        signature_map[n] = index;
    }

    /*
        This function is called internally when entities are created with new
        signatures.  It can also be called up front to remove start up memory
        allocations to get the vectors up to size for your game/level.

        Signatures are stored in a vector and also a map that maps from
        the signature value to where it is in the vector.
    */
    uint16_t RegisterSigniture(const Signature& signature, uint16_t count = 0) {
        uint16_t index = 0;
        std::map<Signature, uint16_t>::const_iterator it = signature_map.find(signature);
        if (it == signature_map.end()) {
            // if its not found, register a new one by putting it in the vector,
            // and the signature map.  Then register the new signature with all
            // of the component vector maps associated with that signature.
            index = signatures.size();
            signatures.push_back(signature);
            signature_map[signature] = index;
            entityData.Register(index, count);
            for (const auto& cid : signature) {
                pack.Any(cid).Register(index, count);
            }
        } else {
            // if we found it then just return the index it already
            // exists int the vector
            index = it->second;
        }
        return index;
    }

    const Signature& GetSignature(uint16_t index) const {
        ASSERT(index >= 0 && index < signatures.size());
        return signatures[index];
    }

   const Signature& GetSignature(Entity entity) const {
        return signatures[entityManager.GetHash(entity.Tracked().index).signature];
    }

    bool Valid(Entity entity) const {
        return entityManager.Valid(entity);
    }

    void Extend(uint16_t e) {
        entityManager.Extend(e);
    }

    VectorMapHash FromEntity(Entity entity) const {
        return entityManager.GetHash(entity.Tracked().index);
    }

    Entity FromHash(VectorMapHash hash) {
        return entityManager.FromIndex(*entityData.Get(hash));
    }

    Entity Create() {
        return entityManager.Create();
    }

    void UpdateSignature(Entity entity, uint16_t signature_id) {
        ASSERT(entity.Tracked().untracked == 0 && Valid(entity) == true);
        VectorMapHash old_hash = entityManager.GetHash(entity.Tracked().index);
        VectorMapHash new_hash;

        if (old_hash.signature == signature_id) return; // don't switch it with itself!

        // cache this for faster updates
        const Signature sig = signatures[old_hash.signature] + signatures[signature_id];
        entityData.Move(old_hash, signature_id);
        for (const auto& cid : sig) {
            pack.Any(cid).Move(old_hash, signature_id);
        }

        new_hash.signature = signature_id;
        //new_hash.index = 0;
        if (new_hash.signature != 0) {
            new_hash.index = entityData.Size(signature_id) - 1;
            *entityData.Get(new_hash) = entity.Tracked().index;
        }

        Ref<uint16_t> swapped_entity = entityData.Get(old_hash);
        if (swapped_entity.IsNull() == false) {
            entityManager.GetHash(*swapped_entity) = old_hash;
        }
        entityManager.GetHash(entity.Tracked().index) = new_hash;
    }

    bool CheckForDups() {
        bool dups = false;
        std::map<uint16_t, bool> indexes;
        for (const auto& kv : entityData) {
            for (int i = 0; i < entityData.Size(kv.first); i++) {
                VectorMapHash vhash;
                vhash.signature = kv.first;
                vhash.index = i;
                if (entityData.Get(vhash).IsNull() == false) {
                    uint16_t value = *entityData.Get(vhash);
                    std::map<uint16_t, bool>::const_iterator it = indexes.find(value);
                    if (it != indexes.end()) {
                        dups = true;
                        std::ofstream os("silly.txt", std::ios::app);
                        os << "BROKEN: " << value << std::endl;
                        os.close();
                    } else {
                        indexes[value] = true;
                    }
                } else {
                    std::ofstream os("silly.txt", std::ios::app);
                    os << "IS EMPTY! " << vhash.signature << " " << vhash.index << std::endl;
                    os.close();
                }
            }
        }
        return dups;
    }

    void Print(const std::string& file) {
        std::ofstream os(file);
        os << "head: " << entityManager.head << std::endl;
        os << "tail: " << entityManager.tail << std::endl;
        os << "index, generation, hash sig, hash index, entity" << std::endl;
        for (int i = 0; i < entityManager.ginfo.size(); ++i) {
            os << "e" << i << ": g" << entityManager.ginfo[i].generation << ", s" << entityManager.hashes[i].signature << ", i" << entityManager.hashes[i].index << ", d";
            Ref<uint16_t> thing = entityData.Get(entityManager.hashes[i]);
            if (thing.IsNull() == true)
                os << "NULL";
            else
                os << *thing;
            os << std::endl;
        }
        os.close();
    }

    void UpdateSignature(Entity entity, const Signature& signature) {
        UpdateSignature( entity, RegisterSigniture(signature) );
    }

    void RemoveSignature(Entity entity, const Signature& signature) {
        VectorMapHash old_hash = entityManager.GetHash(entity.Tracked().index);
        Signature new_sig = signatures[old_hash.signature] - signature;
        UpdateSignature( entity, RegisterSigniture(new_sig) );
    }

    void AddSignature(Entity entity, const Signature& signature) {
        VectorMapHash old_hash = entityManager.GetHash(entity.Tracked().index);
        Signature new_sig = signatures[old_hash.signature] + signature;
        UpdateSignature( entity, RegisterSigniture(new_sig) );
    }

    void Destroy(Entity entity) {
        //Print("first.txt");

        UpdateSignature(entity, NullSignature);
        entityManager.Destroy(entity);

        //if (CheckForDups()==true){
        //    Print("second.txt");
        //    ASSERT(false);
        //}
    }



    typename T::EntityRef GetRef(Entity entity) {
        ASSERT(Valid(entity) == true);
        typename T::EntityRef r;
        VectorMapHash& hash = entityManager.GetHash(entity.Tracked().index);
        r.entity = entity;
        r.hash = hash;
        const Signature& signature = GetSignature(hash.signature);
        for (const auto& cid : signature) {
            r.begin()[cid] = pack.Any(cid).Get(r.hash);
        }
        return r;
    }

    template<uint8_t... Args, typename F>
    void ForEach(F func) {
        uint32_t i;
        size_t size;
        Signature for_sig = Signature::Gen<Args...>();
        typename T::EntityRef r;
        r.entity = Entity();        // have to get entity if needed
        for ( const auto& kv : entityData ) {
            if ( signatures[kv.first].Contains(for_sig) == true ) {
                size = entityData.Size(kv.first);
                r.hash.signature = kv.first;
                r.hash.index = 0;
                SetRef<Args...>(r);
                for (i = 0; i < size; ++i) {
                    func(r);
                    IncrementRef<Args...>(r);
                }
            }
        }
    }

//protected:
    template<uint8_t C>
    static void IncrementRef(typename T::EntityRef& r) {
        r.begin()[C] += sizeof(typename std::tuple_element<C, typename T::type_table>::type);
        ++r.hash.index;
    }

    template<uint8_t C1, uint8_t C2, uint8_t ... Args>
    static void IncrementRef(typename T::EntityRef& r) {
        r.begin()[C1] += sizeof(typename std::tuple_element<C1, typename T::type_table>::type);
        IncrementRef<C2, Args...>(r);
    }

    template<uint8_t C>
    void SetRef(typename T::EntityRef& r) {
        r.begin()[C] = pack.Any(C).Get(r.hash);
    }

    template<uint8_t C1, uint8_t C2, uint8_t ... Args>
    void SetRef(typename T::EntityRef& r) {
        r.begin()[C1] = pack.Any(C1).Get(r.hash);
        SetRef<C2, Args...>(r);
    }

    std::map<Signature, uint16_t> signature_map;
    std::vector<Signature> signatures;

    EntityManager entityManager;

    VectorMap<uint16_t> entityData;

    T pack;

};





#endif // ENTITY_HPP
