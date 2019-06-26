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


class Signature {
public:
    constexpr static const uint8_t MaxArrayLength = 6;
    constexpr static const uint8_t MaxComponents = MaxArrayLength * 4;
    constexpr static const uint8_t NullIndex = 0xff;
    constexpr static const uint8_t NullCp = 0xff;

    template<uint8_t... Args>
    static Signature Gen() {
        Signature s;
        SetElements<0, Args...>(s);
        std::sort(s.begin(), s.end());
        ASSERT(!s.HasDuplicates());
        return s;
    }

    friend Signature operator+(const Signature &lhs, const Signature &rhs);
    friend Signature operator-(const Signature &lhs, const Signature &rhs);

    Signature() {
        Init();
    }

    Signature(std::initializer_list<uint8_t> list) {
        ASSERT(list.size() < MaxComponents);
        Init();
        uint8_t* first = begin();
        uint8_t* dest = first;
        const uint8_t* src = list.begin();
        size_t size = list.size();
        for (uint32_t i = 0; i < size; ++i) {
            *dest = *src;
            ++src;
            ++dest;
        }
        std::sort(first, dest);
        ASSERT(!HasDuplicates());
    }

    bool operator<(const Signature& rhs) const {
        uint8_t r = 0;
        while (bits[r] == rhs.bits[r] && r < MaxArrayLength - 1) ++r;
        return (bits[r] < rhs.bits[r]);
    }

    const uint8_t* begin() const {
        return reinterpret_cast<const uint8_t*>(this);
    }

    const uint8_t* end() const {
        return begin() + MaxComponents;
    }

    const uint8_t& operator[](int i) const {
        ASSERT(i >= 0 && i < MaxComponents);
        return begin()[i];
    }

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

    bool Has(uint8_t k) const {
        return (GetIndexOf(k) != NullIndex);
    }

    bool Contains(const Signature& rhs) const {
        const uint8_t *it_lhs = begin(), *it_rhs = rhs.begin();
         for(;;) {
            if (*it_rhs == NullCp) return true;
            if (*it_lhs == NullCp) return false;
            if ( *it_lhs == *it_rhs ) ++it_rhs;
            ++it_lhs;
        }
    }

    uint16_t Count() const {
        int16_t count = 0;
        const uint8_t* it = begin();
        while ( *it != NullCp && count < MaxComponents - 1 ) {
            count++;
            ++it;
        }
        return count;
    }

private:
    template<uint16_t INDEX, uint8_t C>
    static void SetElements(Signature& s) {
        s.begin()[INDEX] = C;
    }

    template<uint16_t INDEX, uint8_t C1, uint8_t C2, uint8_t ... Args>
    static void SetElements(Signature& s) {
        s.begin()[INDEX] = C1;
        SetElements<INDEX + 1, C2, Args...>(s);
    }

    void Init() {
        bits[0] = 0xffffffff;
        bits[1] = 0xffffffff;
        bits[2] = 0xffffffff;
        bits[3] = 0xffffffff;
        bits[4] = 0xffffffff;
        bits[5] = 0xffffffff;
    }

    uint8_t* begin() {
        return reinterpret_cast<uint8_t*>(this);
    }

    uint8_t* end() {
        return begin() + MaxComponents;
    }

    bool HasDuplicates() const {
        const uint8_t* it = begin();
        for(;;) {
            if (*it == NullCp) return false;
            if (*it == *(it + 1)) return true;
            ++it;
        }
    }

    uint32_t bits[MaxArrayLength];

};

Signature operator+(const Signature &lhs, const Signature &rhs) {
    ASSERT(lhs.Count() + rhs.Count() < Signature::MaxComponents - 1);
    Signature ret;
    uint8_t *it = ret.begin();
    const uint8_t *it_lhs = lhs.begin(), *it_rhs = rhs.begin();
    for(;;) {
        if ( *it_lhs == *it_rhs ) {
            if (*it_lhs == Signature::NullCp) return ret;
            *it = *it_lhs;
            ++it;
            ++it_rhs;
            ++it_lhs;
        } else if ( *it_lhs < *it_rhs ) {
            *it = *it_lhs;
            ++it;
            ++it_lhs;
        } else {
            *it = *it_rhs;
            ++it;
            ++it_rhs;
        }
    }
}

Signature operator-(const Signature &lhs, const Signature &rhs) {
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
            ++it_lhs;
        } else {
            ++it_rhs;
        }
    }
}


template<typename T>
class Ref {
public:
    Ref(void* p) : ptr( reinterpret_cast<T*>(p) ) { }
    Ref(T* p = nullptr) : ptr(p) { }

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

    bool IsNull() const {
        return ptr == nullptr;
    }

    void* Ptr() { return ptr; }

private:
    T* ptr;

};


struct VectorMapHash {
    uint16_t signature;
    uint16_t index;
};

template<typename T>
class VectorMap {
public:
    size_t Size(uint16_t signature) const {
        size_t s = 0;
        typename table_type::const_iterator it = table.find(signature);
        if (it != table.end())
            s = it->second.size();
        return s;
    }

    Ref<T> Get(VectorMapHash hash) {
        Ref<T> ref;
        typename table_type::iterator it = table.find(hash.signature);
        if (it != table.end() && hash.index >= 0 && hash.index < it->second.size())
            ref = Ref<T>( &(it->second[hash.index]) );
        return ref;
    }

    void Register(uint16_t signature, uint16_t count) {
        typename table_type::iterator it = table.find(signature);
        if (it == table.end()) {
            table[signature] = std::vector<T>();
            table[signature].reserve(count);
        }
    }

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
                std::swap(old_vec[old_hash.index], old_vec[old_vec.size() - 1]);
                old_vec.pop_back();
            }
        } else {
             if (old_sig != table.end()) {
                std::vector<T>& old_vec = old_sig->second;
                std::swap(old_vec[old_hash.index], old_vec[old_vec.size() - 1]);
                old_vec.pop_back();
            }
        }
    }

    template<typename F>
    void ForEachSig(F func) {
        typename table_type::iterator it = table.begin();
        while (it != table.end()) {
            func(it->first);
            ++it;
        }
    }

private:
    using table_type = std::unordered_map<uint16_t, std::vector<T>>;
    table_type table;

};

template<typename T>
class BaseState;
class GenerationData;

class Entity {
public:
    struct tracked_t {
        uint32_t index : 16;
        uint32_t generation : 15;
        uint32_t untracked : 1;
    };

    struct untracked_t {
        uint32_t user : 24;
        uint32_t type : 7;
        uint32_t untracked : 1;
    };

    // the only tracked entity we are allowed to make is the null entity
    Entity() { data.raw.value = 0x00000000; }
    // to create untracked entities
    Entity(uint8_t t, uint16_t u) {
        data.untracked.untracked = 1;
        data.untracked.type = t;
        data.untracked.user = u;
    }

    bool operator< (const Entity& rhs) const { return data.raw.value <  rhs.data.raw.value; }
    bool operator==(const Entity& rhs) const { return data.raw.value == rhs.data.raw.value; }
    bool operator!=(const Entity& rhs) const { return data.raw.value != rhs.data.raw.value; }

    uint32_t Raw() const { return data.raw.value; }
    tracked_t Tracked() const { return data.tracked; }
    untracked_t Untracked() const { return data.untracked; }

private:
    Entity(uint32_t in) { data.raw.value = in; }
    Entity(tracked_t in) { data.tracked = in; }
    Entity(untracked_t in) { data.untracked = in; }

    friend class GenerationData;

    struct raw_t {
        uint32_t value;
    };

    union {
        tracked_t tracked;
        untracked_t untracked;
        raw_t raw;
    } data;

};



class GenerationData {
public:

    struct GenerationInfo {
        uint16_t generation : 15;
        uint16_t unsafe : 1;
    };

    GenerationData() : head(END_OF_LIST), tail(END_OF_LIST) { }

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
        for (uint16_t i = s; i < e; i++) {
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
            if (head == END_OF_LIST)
                tail = END_OF_LIST;
        }
        entity.untracked = 0;
        entity.generation = ginfo[entity.index].generation;
        return Entity(entity);
    }

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


    void Destroy(Entity entity) {
        ASSERT(entity.Tracked().untracked == 0 && ginfo[entity.Tracked().index].generation == entity.Tracked().generation);
        if (ginfo[entity.Tracked().index].generation == 0x7fff) {
            ginfo[entity.Tracked().index].unsafe = 0;
            ginfo[entity.Tracked().index].generation = 1;
            hashes[entity.Tracked().index].index = END_OF_LIST;
            if (head == END_OF_LIST) {
                head = entity.Tracked().index;
            } else {
                hashes[tail].index = entity.Tracked().index;
            }
            tail = entity.Tracked().index;
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

    VectorMapHash& GetHash(Entity entity) {
        //std::cout << entity.Tracked().untracked << " == " << 0 << " && " << ginfo[entity.Tracked().index].generation << " == " << entity.Tracked().generation << std::endl;
        ASSERT(entity.Tracked().untracked == 0 && ginfo[entity.Tracked().index].generation == entity.Tracked().generation);
        return hashes[entity.Tracked().index];
    }

    const VectorMapHash& GetHash(Entity entity) const {
        ASSERT(entity.Tracked().untracked == 0 && ginfo[entity.Tracked().index].generation == entity.Tracked().generation);
        return hashes[entity.Tracked().index];std::vector<Entity> removebatch;
    }

    Entity FromIndex(uint16_t index) const {
        ASSERT(index < ginfo.size());
        Entity::tracked_t entity;
        entity.index = index;
        entity.generation = ginfo[index].generation;
        entity.untracked = 0;
        return Entity(entity);
    }

private:
    static const uint16_t END_OF_LIST = 0xffff;

    uint16_t head;
    uint16_t tail;
    std::vector<GenerationInfo> ginfo;
    std::vector<VectorMapHash> hashes;
};




class AnyVectorRef {
private:
    constexpr static const size_t MaxBuffer = 8;

    struct base {
        virtual void Move(VectorMapHash old_hash, uint16_t new_hash_signature) = 0;
        virtual void Register(uint16_t signature, uint16_t count) = 0;
        virtual void* Get(VectorMapHash hash) = 0;
    };

    template<typename T>
    struct derived : public base {
        derived(T& t) : r(t) { }
        void Move(VectorMapHash old_hash, uint16_t new_hash_signature) { r.Move(old_hash, new_hash_signature); }
        void Register(uint16_t signature, uint16_t count) { r.Register(signature, count); }
        void* Get(VectorMapHash hash) { return r.Get(hash).Ptr(); }
        T& r;
    };

    base* Base() { return reinterpret_cast<base*>(_buffer); }

    uint8_t _buffer[MaxBuffer];

public:
    AnyVectorRef() {
        ASSERT(false);
    }

    template<typename T>
    AnyVectorRef(T& t) {
        ASSERT(sizeof(derived<T>) <= MaxBuffer);
        derived<T>* d = new (_buffer) derived<T>(t);
        ASSERT(reinterpret_cast<derived<T>*>(d) - reinterpret_cast<derived<T>*>(_buffer) == 0);
    }

    void Move(VectorMapHash old_hash, uint16_t new_hash_signature) { Base()->Move(old_hash, new_hash_signature); }
    void Register(uint16_t signature, uint16_t count) { Base()->Register(signature, count); }
    void* Get(VectorMapHash hash) { return Base()->Get(hash); }

};


template<typename T>
class BaseState;


class BaseEntityRef {
public:
    Entity GetEntity() const { return entity; }
    VectorMapHash GetHash() const { return hash; }

protected:
    template<typename T>
    friend class BaseState;

    void Set(uint8_t ptr_id, void* ptr) {
        begin()[ptr_id] = ptr;
    }

    void Increment(uint8_t ptr_id, size_t sz) {
        reinterpret_cast<uint8_t**>(begin())[ptr_id] += sz;
    }

    void** begin() {
        return reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(this) + sizeof(BaseEntityRef));
    }

    Entity entity;
    VectorMapHash hash;

};



template<typename T>
class BaseState {
public:
    static const uint16_t NullSignature = 0;

    BaseState() {
        // put in the null signature
        uint16_t index = signatures.size();
        Signature n;
        signatures.push_back(n);
        signature_map[n] = index;
    }

    const Signature& GetSignature(uint16_t index) {
        ASSERT(index >= 0 && index < signatures.size());
        return signatures[index];
    }

    uint16_t GetSignature(const Signature& signature) {
        return RegisterSigniture(signature);
    }

    bool Valid(Entity entity) const {
        return generationData.Valid(entity);
    }

    void Extend(uint16_t e) {
        generationData.Extend(e);
    }

    VectorMapHash FromEntity(Entity entity) const {
        return generationData.GetHash(entity);
    }

    Entity FromHash(VectorMapHash hash) {
        return generationData.FromIndex(*entityData.Get(hash));
    }

    Entity Create() {
        return generationData.Create();
    }

    // AddSignature
    // RemoveSignature

    void UpdateSignature(Entity entity, uint16_t signature_id) {
        ASSERT(entity.Tracked().untracked == 0 && Valid(entity) == true);
        VectorMapHash old_hash = generationData.GetHash(entity);
        VectorMapHash new_hash;

        if (old_hash.signature == signature_id) return; // don't switch it with itself!

        // cache this for faster updates
        Signature sig = signatures[old_hash.signature] + signatures[signature_id];
        entityData.Move(old_hash, signature_id);
        uint32_t i = 0;
        while (sig[i] != Signature::NullCp) {
            pack.Any(sig[i]).Move(old_hash, signature_id);
            ++i;
        }

        new_hash.signature = signature_id;
        if (new_hash.signature != 0) {
            new_hash.index = entityData.Size(signature_id) - 1;
            *entityData.Get(new_hash) = entity.Tracked().index;
        }
        generationData.GetHash(entity) = new_hash;
    }

    void UpdateSignature(Entity entity, const Signature& signature) {
        UpdateSignature( entity, GetSignature(signature) );
    }

    void RemoveSignature(Entity entity, const Signature& signature) {
        VectorMapHash old_hash = generationData.GetHash(entity);
        Signature new_sig = signatures[old_hash.signature] - signature;
        UpdateSignature( entity, GetSignature(new_sig) );
    }

    void AddSignature(Entity entity, const Signature& signature) {
        VectorMapHash old_hash = generationData.GetHash(entity);
        Signature new_sig = signatures[old_hash.signature] + signature;
        UpdateSignature( entity, GetSignature(new_sig) );
    }

    void Destroy(Entity entity) {
        UpdateSignature(entity, NullSignature);
        generationData.Destroy(entity);
    }

    const Signature& GetSignature(Entity entity) const {
        return signatures[generationData.GetHash(entity).signature];
    }

    typename T::EntityRef GetRef(Entity entity) {
        ASSERT(Valid(entity) == true);
        uint8_t cp;
        typename T::EntityRef r;
        VectorMapHash& hash = generationData.GetHash(entity);
        r.entity = entity;
        r.hash = hash;
        const Signature& signature = GetSignature(hash.signature);
        uint32_t i = 0;
        while ((cp = signature[i]) != Signature::NullCp) {
            r.Set(cp, pack.Any(cp).Get(hash));
            ++i;
        }
        return r;
    }

    template<uint8_t... Args, typename F>
    void ForEach(F func) {
        uint32_t i;
        size_t size;
        VectorMapHash hash;
        Signature for_sig = Signature::Gen<Args...>();
        typename T::EntityRef r;
        r.entity = Entity();        // have to get entity if needed
        hash.index = 0;
        entityData.ForEachSig([&](uint16_t signature) {
            if ( signatures[signature].Contains(for_sig) == true ) {
                hash.signature = signature;
                size = entityData.Size(signature);
                r.hash = hash;
                SetRef<Args...>(r, hash);
                for (i = 0; i < size; ++i) {
                    func(r);
                    ++r.hash.index;      // increment the hash index
                    IncrementRef<Args...>(r);
                }
            }
        });
    }

protected:
    template<uint8_t C>
    static void IncrementRef(typename T::EntityRef& r) {
        r.Increment(C, sizeof(typename std::tuple_element<C, typename T::type_table>::type));
    }

    template<uint8_t C1, uint8_t C2, uint8_t ... Args>
    static void IncrementRef(typename T::EntityRef& r) {
        r.Increment(C1, sizeof(typename std::tuple_element<C1, typename T::type_table>::type));
        IncrementRef<C2, Args...>(r);
    }

    template<uint8_t C>
    void SetRef(typename T::EntityRef& r, VectorMapHash hash) {
        r.Set(C, pack.Any(C).Get(hash));
    }

    template<uint8_t C1, uint8_t C2, uint8_t ... Args>
    void SetRef(typename T::EntityRef& r, VectorMapHash hash) {
        r.Set(C1, pack.Any(C1).Get(hash));
        SetRef<C2, Args...>(r, hash);
    }

    uint16_t RegisterSigniture(const Signature& signature, uint16_t count = 0) {
        uint16_t index = 0;
        std::map<Signature, uint16_t>::const_iterator it = signature_map.find(signature);
        if (it == signature_map.end()) {
            index = signatures.size();
            signatures.push_back(signature);
            signature_map[signature] = index;
            entityData.Register(index, count);
            uint32_t i = 0;
            while (signature[i] != Signature::NullCp) {
                pack.Any(signature[i]).Register(index, count);
                ++i;
            }
        } else {
            index = it->second;
        }
        return index;
    }

    std::map<Signature, uint16_t> signature_map;
    std::vector<Signature> signatures;

    GenerationData generationData;

    VectorMap<uint16_t> entityData;

    T pack;

};



#endif // ENTITY_HPP
