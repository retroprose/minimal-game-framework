#ifndef ENTITY_HPP
#define ENTITY_HPP


#include <Container.hpp>


/*

    uint16_t head;
    uint16_t tail;
    std::vector<GenerationInfo> ginfo;
    std::vector<VectorMapHash> hashes;

    hash free_map
    vectormap entity;
    vectormap active;


    Extend
    ReserveSignature  // needs comps

    Create
    Destroy entity

    Valid
    GetSignature

    ChangeSignature entity, signature   // needs comps

    EntityFromHash
    HashFromEntity

    ForEach

*/

class BaseState;

class Signature {
public:
    constexpr static Signature Null() { return Signature(); }

    constexpr Signature() : value_(0) { }
    Signature(uint16_t value) : value_(value) { }

    Signature(std::initializer_list<uint8_t> list) : value_(0) {
        ASSERT(list.size() < 16);
        for (auto& cp : list)
            value_ |= 0x0001 << cp;
    }

    bool operator< (const Signature& rhs) const { return value_ <  rhs.value_; }
    bool operator==(const Signature& rhs) const { return value_ == rhs.value_; }
    bool operator!=(const Signature& rhs) const { return value_ != rhs.value_; }

    Signature operator+(const Signature& rhs) const { return value_ | rhs.value_; }
    Signature operator-(const Signature& rhs) const { return value_ & ~rhs.value_; }

    Signature& operator+=(const Signature& rhs) {
        value_ |= rhs.value_;
        return *this;
    }

    Signature& operator-=(const Signature& rhs) {
        value_ &= ~rhs.value_;
        return *this;
    }

    bool has(uint8_t component) const {
        uint16_t mask = 0x0001 << component;
        return ((value_ & mask) == mask);
    }

    bool contains(const Signature& rhs) const {
        return ((value_ & rhs.value_) == rhs.value_);
    }

    const uint16_t& value() const { return value_; }

private:
    uint16_t value_;

};

class ComponentList {
public:
    ComponentList(const Signature& signature) {
        Init(signature, 0);
    }

    ComponentList(const Signature& signature, uint32_t offset) {
        Init(signature, offset);
    }

    void Init(const Signature& signature, uint32_t offset) {
        uint8_t index = 0;
        uint16_t mask = 0x0001;
        for (uint8_t i = 0; i < 16; ++i){
            if (mask & signature.value()) {
                components_[index] = i + offset;
                ++index;
            }
            mask = mask << 1;
        }
        components_[index] = 0xff;
        size_ = index;
    }

    const uint8_t* begin() const { return components_; }
    const uint8_t* end() const { return components_ + size_; }

    const uint8_t& operator[](uint8_t index) const { return components_[index]; }

private:
    uint8_t components_[17];
    uint8_t size_;

};



class Entity {
public:
    Entity() { data_.raw.value = 0x00000000; }

    bool operator< (const Entity& rhs) const { return data_.raw.value <  rhs.data_.raw.value; }
    bool operator==(const Entity& rhs) const { return data_.raw.value == rhs.data_.raw.value; }
    bool operator!=(const Entity& rhs) const { return data_.raw.value != rhs.data_.raw.value; }

    bool untracked() const {
        return data_.tracked.untracked;
    }

    uint16_t index() const {
        ASSERT(data_.tracked.untracked == 0);
        return data_.tracked.index;
    }

    uint16_t generation() const {
        ASSERT(data_.tracked.untracked == 0);
        return data_.tracked.generation;
    }

    uint16_t type() const {
        ASSERT(data_.tracked.untracked == 1);
        return data_.untracked.type;
    }

    uint16_t data() const {
        ASSERT(data_.tracked.untracked == 1);
        return data_.untracked.data;
    }

private:
    friend class BaseState;

    struct Untracked {
        uint32_t data : 24;
        uint32_t type : 7;
        uint32_t untracked : 1;
    };

    struct Tracked {
        uint32_t index : 16;
        uint32_t generation : 15;
        uint32_t untracked : 1;
    };

    struct Raw {
        uint32_t value;
    };

    Entity(Tracked tracked) { data_.tracked = tracked; }

    union {
        Tracked tracked;
        Untracked untracked;
        Raw raw;
    } data_;

};

template<typename T>
class Ref {
public:
    Ref(uint8_t* ptr) : ptr_( reinterpret_cast<T*>(ptr) ) { }

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

struct EntityHash {
    EntityHash() : signature(Signature::Null()), index(0) { }
    EntityHash(Signature s, uint16_t i) : signature(s), index(i) { }
    Signature signature;
    uint16_t index;
};

template<typename T>
class VectorMap {
private:
    using pair_type = std::pair<Signature, std::vector<T>>;
    using table_type = std::map<Signature, std::vector<T>>;
    table_type table;

public:
    size_t size(Signature signature) const {
        size_t s = 0;
        typename table_type::const_iterator it = table.find(signature);
        if (it != table.end()) {
            s = it->second.size();
        }
        return s;
    }

    bool hasHash(EntityHash hash) const {
        bool has = true;
        typename table_type::const_iterator it = table.find(hash.signature);
        if (it == table.end() || hash.index >= it->second.size())
            has = false;
        return has;
    }

    Ref<T> get(EntityHash hash) {
        Ref<T> ref;
        typename table_type::iterator it = table.find(hash.signature);
        if (it != table.end() && hash.index < it->second.size()) {
            ref = Ref<T>( &(it->second[hash.index]) );
        }
        return ref;
    }

    typename table_type::const_iterator begin() const {
        return table.begin();
    }

    typename table_type::const_iterator end() const {
        return table.end();
    }

     void move(EntityHash oldHash, EntityHash newHash) {
        typename table_type::iterator newSig = table.find(newHash.signature);
        if (newSig == table.end()) {
            if (newHash.signature == Signature::Null()) return;
            auto ret = table.insert(pair_type(newHash.signature, std::vector<T>()));
            newSig = ret.first;
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

template<>
class VectorMap<bool> {
private:
    using pair_type = std::pair<Signature, std::vector<bool>>;
    using table_type = std::map<Signature, std::vector<bool>>;
    table_type table;

public:
    size_t size(Signature signature) const {
        size_t s = 0;
        typename table_type::const_iterator it = table.find(signature);
        if (it != table.end()) {
            s = it->second.size();
        }
        return s;
    }

    bool hasHash(EntityHash hash) const {
        bool has = true;
        typename table_type::const_iterator it = table.find(hash.signature);
        if (it == table.end() || hash.index >= it->second.size())
            has = false;
        return has;
    }

    bool get(EntityHash hash) const {
        ASSERT( hasHash(hash) );
        return table.find(hash.signature)->second[hash.index];
    }

    void set(EntityHash hash, bool value) {
        ASSERT( hasHash(hash) );
        table.find(hash.signature)->second[hash.index] = value;
    }

    typename table_type::const_iterator begin() const {
        return table.begin();
    }

    typename table_type::const_iterator end() const {
        return table.end();
    }

     void move(EntityHash oldHash, EntityHash newHash) {
        typename table_type::iterator newSig = table.find(newHash.signature);
        if (newSig == table.end()) {
            if (newHash.signature == Signature::Null()) return;
            auto ret = table.insert(pair_type(newHash.signature, std::vector<bool>()));
            newSig = ret.first;
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


template<typename T>
class EntityMap {
private:
    using table_type = std::map<uint16_t, T>;
    table_type table;

public:
    size_t size() const {
        return table.size();
    }

    typename table_type::iterator begin() {
        return table.begin();
    }

    typename table_type::iterator end() {
        return table.end();
    }

    Ref<T> get(Entity entity) {
        Ref<T> ref;
        typename table_type::iterator it = table.find(entity.index());
        if (it != table.end())
            ref = Ref<T>( &(it->second) );
        return ref;
    }

    void add(Entity entity) {
        typename table_type::iterator it = table.find(entity.index());
        if (it == table.end())
            table[entity.index()] = T();
    }

    void remove(Entity entity) {
        table.erase(entity.index());
    }

};



class AnyComponentMap {
private:
    /*
        These 8 bytes account for the 4 for the vtable pointer, and 4 for the pointer to the object itself.
        If this needs to be compiled for 64 bit, it needs to be 16 bytes...
    */
    constexpr static const size_t MaxBuffer = 8;

    struct Base {
        virtual uint8_t* get(EntityHash hash) = 0;
    };

    template<typename T>
    struct Derived : public Base {
        Derived(T& t) : r(t) { }
        uint8_t* get(EntityHash hash) { return 0; }
        T& r;
    };

    template<typename U>
    struct Derived<VectorMap<U>> : public Base {
        Derived(VectorMap<U>& t) : r(t) { }
        uint8_t* get(EntityHash hash) { return 0; }
        VectorMap<U>& r;
    };

    template<typename U>
    struct Derived<EntityMap<U>> : public Base {
        Derived(EntityMap<U>& t) : r(t) { }
        uint8_t* get(EntityHash hash) { return 0; }
        EntityMap<U>& r;
    };

    Base* base() { return reinterpret_cast<Base*>(buffer_); }

    /*
        This is a raw memory buffer than derived classes are newed onto.
    */
    uint8_t buffer_[MaxBuffer];

public:

    AnyComponentMap() {
        ASSERT(false);
    }

    template<typename T>
    AnyComponentMap(T& t) {
        ASSERT(sizeof(Derived<T>) <= MaxBuffer);
        Base* d = new (buffer_) Derived<T>(t);
        ASSERT(d - base() == 0);
    }


    uint8_t* get(EntityHash hash) { return base()->get(hash); }

};




class BaseState {
public:
    BaseState() : head(EndOfList), tail(EndOfList) { }

/*
    void reserveSignature(Signature signature, uint32_t count) {
        activeData.move(EntityHash(), EntityHash(signature, count));
        entityData.move(EntityHash(), EntityHash(signature, count));
    }
*/

   bool valid(Entity entity) const {
        if ( entity.untracked() == 1 ||
             (entity.index() < ginfo.size() &&
              ginfo[entity.index()].generation == entity.generation() )
           ) {
           return true;
        }
        return false;
    }

    Entity getEntityFromIndex(uint16_t index) const {
        Entity::Tracked tracked;
        tracked.untracked = 0;
        tracked.index = index;
        tracked.generation = ginfo[tracked.index].generation;
        return Entity(tracked);
    }

    EntityHash getHashFromIndex(uint16_t index) const {
        return hashes[index];
    }

    Entity getEntity(EntityHash hash) {
        Entity::Tracked tracked;
        tracked.untracked = 0;
        tracked.index = *entityData.get(hash);
        tracked.generation = ginfo[tracked.index].generation;
        return Entity(tracked);
    }

    EntityHash getHash(Entity entity) const {
        return hashes[entity.index()];
    }

    bool getActive(Entity entity) {
        return activeData.get( getHash(entity) );
    }

    void setActive(Entity entity, bool value) {
        activeData.set(getHash(entity), value);
    }

    Entity create() {
        EntityHash hash;
        Entity::Tracked entity;
        if (head == EndOfList) {
            entity.index = ginfo.size();
            GenerationInfo info;
            info.generation = 1;
            info.unsafe = 0;
            ginfo.push_back(info);
            hashes.push_back( EntityHash() );
            dynamicSignature.push_back(Signature::Null());
        } else {
            entity.index = head;
            head = hashes[entity.index].index;
            hashes[entity.index] = EntityHash();
            dynamicSignature[entity.index] = Signature::Null();
            if (head == EndOfList)
                tail = EndOfList;
        }
        entity.untracked = 0;
        entity.generation = ginfo[entity.index].generation;
        return Entity(entity);
    }

    EntityHash changeSignature(Entity entity, EntityHash oldHash, Signature signature) {
        ASSERT(oldHash.signature != signature);

        EntityHash newHash;

        newHash.signature = signature;

        std::map<Signature, FreeInfo>::iterator newInfo = freeMap.find(newHash.signature);
        if (newInfo == freeMap.end()) {
            newHash.index = 0;
            if (newHash.signature != Signature::Null()) {
                FreeInfo info;
                info.size = 1;
                info.head = EndOfList;
                freeMap[newHash.signature] = info;
            }
        } else {
            if (newInfo->second.head == EndOfList) {
                newHash.index = newInfo->second.size;
                ++(newInfo->second.size);
            } else {
                newHash.index = newInfo->second.head;
                newInfo->second.head = *entityData.get(newHash);
            }
        }

        std::map<Signature, FreeInfo>::iterator oldInfo = freeMap.find(oldHash.signature);
        if (oldInfo != freeMap.end()) {
            activeData.set(oldHash, false);
            *entityData.get(oldHash) = oldInfo->second.head;
            oldInfo->second.head = oldHash.index;
        }

        entityData.move(oldHash, newHash);
        activeData.move(oldHash, newHash);

        if (newHash.signature != Signature::Null()) {
            *entityData.get(newHash) = entity.index();
            activeData.set(newHash, false);
        }

        hashes[entity.index()] = newHash;

        return newHash;
    }

    void destroy(Entity entity) {
        ASSERT(entity.untracked() == 0 && ginfo[entity.index()].generation == entity.generation());
        if (ginfo[entity.index()].generation == 0x7fff) {
            // this is a roll over
            ASSERT(false);
        } else {
            // we won't increment if unsafe is set to 1
            //ginfo[entity.Tracked().index].unsafe = 0;
            ++ginfo[entity.index()].generation;
            hashes[entity.index()].index = EndOfList;
            if (head == EndOfList) {
                head = entity.index();
            } else {
                hashes[tail].index = entity.index();
            }
            tail = entity.index();
        }
    }

    template<typename F>
    void ForEach(Signature signature, F func) {
        EntityHash hash;
        for ( const auto& kv : activeData ) {
            hash.signature = kv.first;
            if (kv.first.contains(signature)) {
                for (hash.index = 0; hash.index < kv.second.size(); ++hash.index) {
                    if (kv.second[hash.index] == true) {
                        func(hash);
                    }
                }
            }
        }
    }



protected:
    constexpr static uint16_t EndOfList = 0xffff;

    struct FreeInfo {
        uint16_t head;
        uint16_t size;
    };

    struct GenerationInfo {
        uint16_t generation : 15;
        uint16_t unsafe : 1;
    };

    uint16_t head;
    uint16_t tail;
    std::vector<GenerationInfo> ginfo;
    std::vector<EntityHash> hashes;
    std::vector<Signature> dynamicSignature;

    std::map<Signature, FreeInfo> freeMap;
    VectorMap<uint16_t> entityData;
    VectorMap<bool>     activeData;


};




#endif // ENTITY_HPP
