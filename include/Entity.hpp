#ifndef ENTITY_HPP
#define ENTITY_HPP


#include <typeinfo>

#include <Container.hpp>


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



    template<class... Ts>
    void doStuff(std::tuple<Ts...> const& tupOfCtr { ... }

    template<class... Ts>
    void doStuff(std::tuple<Ts...> const& tupOfCtr {
        using value_tuple = std::tuple<typename Ts::value_type...>;
    }





    #include <iostream>
    #include <tuple>

    using namespace std;

    template<int index, typename... Ts>
    struct print_tuple {
        void operator() (tuple<Ts...>& t) {
            cout << get<index>(t) << " ";
            print_tuple<index - 1, Ts...>{}(t);
        }
    };

    template<typename... Ts>
    struct print_tuple<0, Ts...> {
        void operator() (tuple<Ts...>& t) {
            cout << get<0>(t) << " ";
        }
    };

    template<typename... Ts>
    void print(tuple<Ts...>& t) {
        const auto size = tuple_size<tuple<Ts...>>::value;
        print_tuple<size - 1, Ts...>{}(t);
    }

    int main() {
        auto t = make_tuple(1, 2, "abc", "def", 4.0f);
        print(t);

        return 0;
    }



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


/*
class Signature {
public:

    template<uint8_t... Args>
    static Signature make(const uint8_t* mapper) {
        Signature signature;
        setBits<0, Args...>(mapper, signature);
        return signature;
    }

    Signature() {
        bits[0] = 0x00000000;
        bits[1] = 0x00000000;
    }

    Signature(const uint8_t* mapper, std::initializer_list<uint8_t> list) : size_(0) {
        bits[0] = 0x00000000;
        bits[1] = 0x00000000;
        const uint8_t* src = list.begin();
        for (;;) {
            if (src == list.end()) break;
            const uint8_t& value = mapper[*src];
            bits[value >> 1] |= 0x00000001 << (value & 31);
            ++src;
        }
    }

    bool operator[](uint8_t i) const {
        return bits[i >> 1] & ( 0x00000001 << (i & 31) );
    }

    uint32_t staticMask() const {
        return bits[0];
    }

    uint32_t dynamicMask() const {
        return bits[1];
    }

private:
    template<uint16_t INDEX, uint8_t C>
    static void setBits(const uint8_t* mapper, Signature& signature) {
        const uint8_t* value = mapper[INDEX];
        signature.bits[value >> 1] |= 0x00000001 << (value & 31);
    }

    template<uint16_t INDEX, uint8_t C1, uint8_t C2, uint8_t ... Args>
    static void setBits(const uint8_t* mapper, Signature& signature) {
        const uint8_t* value = mapper[INDEX];
        signature.bits[value >> 1] |= 0x00000001 << (value & 31);
        setBits<INDEX + 1, C2, Args...>(signature);
    }

    uint32_t bits[2];

};
*/

class Signature;
class SignatureManager;


class Signature {
public:
    class Id {
    private:
        friend class Signature;
        friend class SignatureManager;

        struct Creator {
            constexpr Creator(uint16_t v) : value_(v) { }
            uint16_t value_;
        };

        constexpr Id(const Creator& c) : value_(c.value_) { }

        uint16_t value_;

    public:
        constexpr Id() : value_(0) { }

        bool operator< (const Id& rhs) const { return value_ <  rhs.value_; }
        bool operator==(const Id& rhs) const { return value_ == rhs.value_; }
        bool operator!=(const Id& rhs) const { return value_ != rhs.value_; }

        uint16_t value() const {
            return value_;
        }

    };

    constexpr static const uint8_t MaxComponents = 4 * 8; // 32
    constexpr static const uint8_t Max32Bit = MaxComponents / 4;
    constexpr static const uint8_t NullIndex = 0xff;
    constexpr static const uint8_t NullCp = 0xff;

    template<uint8_t... Args>
    static Signature make() {
        ASSERT(sizeof...(Args) < MaxComponents - 1);
        Signature s;
        setElements<0, Args...>(s);
        std::sort(s.begin(), s.end());
        ASSERT(!s.hasDuplicates());
        return s;
    }

    Signature() : size_(0) {
        init();
    }

    Signature(std::initializer_list<uint8_t> list) : size_(0) {
        ASSERT(list.size() < MaxComponents - 1);
        init();
        size_ = list.size();
        const uint8_t* src = list.begin();
        uint8_t* dest = begin();
        for (;;) {
            if (src == list.end()) break;
            *dest = *src;
            ++src;
            ++dest;
        }
        std::sort(begin(), end());
        ASSERT(!hasDuplicates());
    }

    Signature operator+(const Signature &rhs) const {
        const Signature& lhs = *this;
        ASSERT(lhs.size() + rhs.size() < MaxComponents - 1);
        Signature ret;
        uint8_t *it = ret.begin();
        const uint8_t *it_lhs = lhs.begin(), *it_rhs = rhs.begin();
        for(;;) {
            if ( *it_lhs == *it_rhs ) {
                if (*it_lhs == NullCp) return ret;
                *it = *it_lhs;
                ++it;
                ++ret.size_;
                ++it_rhs;
                ++it_lhs;
            } else if ( *it_lhs < *it_rhs ) {
                *it = *it_lhs;
                ++it;
                ++ret.size_;
                ++it_lhs;
            } else {
                *it = *it_rhs;
                ++it;
                ++ret.size_;
                ++it_rhs;
            }
        }
    }

    Signature operator-(const Signature &rhs) const {
        const Signature& lhs = *this;
        Signature ret;
        uint8_t *it = ret.begin();
        const uint8_t *it_lhs = lhs.begin(), *it_rhs = rhs.begin();
        for(;;) {
            if ( *it_lhs == *it_rhs ) {
                if (*it_lhs == NullCp) return ret;
                ++it_rhs;
                ++it_lhs;
            } else if ( *it_lhs < *it_rhs ) {
                *it = *it_lhs;
                ++it;
                ++ret.size_;
                ++it_lhs;
            } else {
                ++it_rhs;
            }
        }
    }

    bool operator<(const Signature& rhs) const {
        const uint32_t* la = reinterpret_cast<const uint32_t*>(components);
        const uint32_t* ra = reinterpret_cast<const uint32_t*>(rhs.components);
        uint8_t i = 0;
        while (*la == *ra && i < Max32Bit - 1) {
            ++la;
            ++ra;
            ++i;
        }
        return *la < *ra;
    }

    bool operator==(const Signature& rhs) const {
        const uint32_t* la = reinterpret_cast<const uint32_t*>(components);
        const uint32_t* ra = reinterpret_cast<const uint32_t*>(rhs.components);
        uint8_t i = 0;
        while (*la == *ra && i < Max32Bit) {
            ++la;
            ++ra;
            ++i;
        }
        return i == Max32Bit;
    }

    bool operator!=(const Signature& rhs) const {
        return !(*this == rhs);
    }


    const uint8_t* begin() const {
        return components;
    }

    const uint8_t* end() const {
        return components + size_;
    }

    const uint8_t& operator[](int i) const {
        ASSERT(i >= 0 && i < MaxComponents);
        return components[i];
    }

    uint8_t getIndexOf(uint8_t k) const {
        const uint8_t* it = begin();
        uint8_t i = 0;
        for(;;) {
            if (*it == NullCp) return NullIndex;
            if (*it == k) return i;
            ++it;
            ++i;
        }
    }

    bool gas(uint8_t k) const {
        return (getIndexOf(k) != NullIndex);
    }

    bool contains(const Signature& rhs) const {
        const uint8_t *it_lhs = begin(), *it_rhs = rhs.begin();
         for(;;) {
            if ( *it_rhs == NullCp ) return true;
            if ( *it_lhs == NullCp ) return false;
            if ( *it_lhs == *it_rhs ) ++it_rhs;
            ++it_lhs;
        }
    }

    uint8_t size() const {
        return size_;
    }

private:
    template<uint16_t INDEX, uint8_t C>
    static void setElements(Signature& s) {
        s.begin()[INDEX] = C;
        ++s.size_;
    }

    template<uint16_t INDEX, uint8_t C1, uint8_t C2, uint8_t ... Args>
    static void setElements(Signature& s) {
        s.begin()[INDEX] = C1;
        ++s.size_;
        setElements<INDEX + 1, C2, Args...>(s);
    }

    void init() {
        for (uint8_t* it = begin(); it != begin() + MaxComponents; ++it) {
            *it = 0xff;
        }
    }

    uint8_t* begin() {
        return components;
    }

    uint8_t* end() {
        return components + size_;
    }

    bool hasDuplicates() const {
        const uint8_t* it = begin();
        for(;;) {
            if (*it == NullCp) return false;
            if (*it == *(it + 1)) return true;
            ++it;
        }
    }

    uint8_t components[MaxComponents];
    uint8_t size_;

};


class SignatureManager {
public:
    SignatureManager(const Signature& dynamicSignature) {
        // initialize utility signatures
        signatures.push_back( SignatureInfo(Signature(), Signature::Id::Creator(0)) );
        signatures.push_back( SignatureInfo(dynamicSignature, Signature::Id::Creator(0)) );
        signature_map[signatures[0].signature] = Signature::Id(Signature::Id::Creator(0));
        signature_map[signatures[1].signature] = Signature::Id(Signature::Id::Creator(1));
    }

    Signature::Id getId(const Signature& signature) {
        Signature::Id id;
        std::map<Signature, Signature::Id>::iterator it = signature_map.find(signature);
        if (it == signature_map.end()) {
            Signature::Id sid;
            sid.value_ = signatures.size();
            Signature staticSignature = signature - signatures[1].signature;
            if ( signature != staticSignature ) {
                signatures.push_back( SignatureInfo(staticSignature, sid) );
                signature_map[staticSignature] = sid;
            }
            id.value_ = signatures.size();
            signatures.push_back( SignatureInfo(signature, sid) );
            signature_map[signature] = id;
        } else {
            id = it->second;
        }
        return id;
    }

    Signature::Id getStaticId(const Signature& signature) {
        Signature::Id id = getId(signature);
        return signatures[id.value_].staticId;
    }

    Signature::Id getStaticId(Signature::Id id) const {
        ASSERT(id.value_ < signatures.size());
        return signatures[id.value_].staticId;
    }

    const Signature& getSignature(Signature::Id id) const {
        ASSERT(id.value_ < signatures.size());
        return signatures[id.value_].signature;
    }

    const Signature& getStaticSignature(Signature::Id id) const {
        ASSERT(id.value_ < signatures.size());
        return signatures[signatures[id.value_].staticId.value_].signature;
    }

private:
    struct SignatureInfo {
        SignatureInfo(const Signature& s, Signature::Id i) : signature(s), staticId(i) { }
        Signature signature;
        Signature::Id staticId;
    };

    std::vector<SignatureInfo> signatures;
    std::map<Signature, Signature::Id> signature_map;

};


class Entity {
public:
    struct Hash {
        Hash() : signature(), index(0) { }
        Signature::Id signature;
        uint16_t index;
    };

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


class EntityManager {
public:

    struct GenerationInfo {
        uint16_t generation : 15;
        uint16_t unsafe : 1;
    };

    EntityManager() : head(EndOfList), tail(EndOfList) { }

    void extend(uint16_t e) {
        uint16_t s = ginfo.size();
        Entity::Tracked entity;
        entity.untracked = 0;
        entity.generation = 0;
        GenerationInfo info;
        info.generation = 0;
        info.unsafe = 0;
        for (uint16_t i = s; i < e; ++i) {
            ginfo.push_back(info);
            hashes.push_back( Entity::Hash() );
            entity.index = i;
            destroy( Entity(entity) );
        }
    }

    bool valid(Entity entity) const {
        if ( entity.untracked() == 1 ||
             (entity.index() < ginfo.size() &&
              ginfo[entity.index()].generation == entity.generation() )
           ) {
           return true;
        }
        return false;
    }

    Entity create() {
        Entity::Tracked entity;
        if (head == EndOfList) {
            entity.index = ginfo.size();
            GenerationInfo info;
            info.generation = 1;
            info.unsafe = 0;
            ginfo.push_back(info);
            hashes.push_back( Entity::Hash() );
        } else {
            entity.index = head;
            head = hashes[entity.index].index;
            hashes[entity.index] = Entity::Hash();
            if (head == EndOfList)
                tail = EndOfList;
        }
        entity.untracked = 0;
        entity.generation = ginfo[entity.index].generation;
        return Entity(entity);
    }


    void destroy(Entity entity) {
        ASSERT(entity.untracked() == 0 && ginfo[entity.index()].generation == entity.generation());
        if (ginfo[entity.index()].generation == 0x7fff) {
            ASSERT(false);
            // have this roll over if we feel adventurous
        } else {
            // we won't increment if unsafe is set to 1
            ginfo[entity.index()].unsafe = 0;
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

    Entity::Hash& getHash(uint16_t index) {
        return hashes[index];
    }

    const Entity::Hash& getHash(uint16_t index) const {
        return hashes[index];
    }

    Entity fromIndex(uint16_t index) const {
        ASSERT(index < ginfo.size());
        Entity::Tracked entity;
        entity.index = index;
        entity.generation = ginfo[index].generation;
        entity.untracked = 0;
        return Entity(entity);
    }

private:
    static const uint16_t EndOfList = 0xffff;

    uint16_t head;
    uint16_t tail;
    std::vector<GenerationInfo> ginfo;
    std::vector<Entity::Hash> hashes;
};



/*

class CpInterface {
public:
    class Abstract {
    private:
        virtual void move_(Entity::Hash oldHash, Entity::Hash newHash) = 0;
    public:
        void move(Entity::Hash oldHash, Entity::Hash newHash) { move_(oldHash, newHash); }
    };

    template<typename T>
    class Concrete : public Abstract {
    private:
        virtual void move_(Entity::Hash oldHash, Entity::Hash newHash) { move(oldHash, newHash); }
        T& t_;
    public:
        Concrete(T& t) : t_(t) { }
        void move(Entity::Hash oldHash, Entity::Hash newHash) { std::cout << "RAN DEFAULT!\n";}
    };

    enum FuncId : uint32_t {
        Move = 0
    };

    template<typename T>
    struct FuncTable {
        using func_tuple_type = std::tuple<
            decltype(&T::move)
        >;
        constexpr static const func_tuple_type func_tuple = std::make_tuple(
            &T::move
        );
    };


    class Any {
    private:
        constexpr static const size_t MaxBuffer = 8;
        uint8_t buffer_[MaxBuffer];

    public:
        Any() {
            ASSERT(false);
        }

        template<typename T>
        Any(T& t) {
            ASSERT(sizeof(Concrete<T>) <= MaxBuffer);
            Abstract* d = new (buffer_) Concrete<T>(t);
            ASSERT(d - reinterpret_cast<Abstract*>(buffer_) == 0);
        }

        Abstract* operator->() {
            return reinterpret_cast<Abstract*>(buffer_);
        }

    };

    template<typename T>
    static Concrete<T> make(T& t) {
        return Concrete<T>(t);
    }

    template<typename T>
    static Any make_any(T& t) {
        return Any(t);
    }

    template<uint32_t FUNC, typename T, typename... Args>
    static void loop(T& t, Args... args) {
        Loop<FUNC, T, Args...>::inner::func(t, args...);
    }

    template<typename T>
    static Any make_from_tuple(T& t, uint32_t index) {
        return AnyTable::table[index](t);
    }

private:
    template<uint32_t FUNC, typename T, typename... Args>
    struct Loop {
        using tuple_type = T;
        constexpr static const uint8_t tuple_size = std::tuple_size<tuple_type>::value;

        template<uint32_t INDEX, bool DUMMY = false>
        struct operations {
            static void func(T& t, Args... args) {
                ((make(std::get<INDEX>(t))).*(std::get<FUNC>(CpInterface::FuncTable<Concrete<typename std::tuple_element<INDEX, T>::type>>::func_tuple)))(args...);
                operations<INDEX + 1>::func(t, args...);
            }
            // set table
        };

        template<bool DUMMY>
        struct operations<tuple_size - 1, DUMMY> {
            constexpr static const uint8_t INDEX = tuple_size - 1;
            static void func(T& t, Args... args) {
                ((make(std::get<INDEX>(t))).*(std::get<FUNC>(CpInterface::FuncTable<Concrete<typename std::tuple_element<INDEX, T>::type>>::func_tuple)))(args...);
            }
            // set table
        };

        using inner = operations<0>;

    };






};

template<typename T>
constexpr const typename CpInterface::AnyTable<T>::func_ptr CpInterface::AnyTable<T>::table = gen_table();

template<typename T>
constexpr const typename CpInterface::FuncTable<T>::func_tuple_type CpInterface::FuncTable<T>::func_tuple;

*/

class CpInterface {
protected:
    virtual void move_(Entity::Hash oldHash, Entity::Hash newHash) = 0;

public:
    enum FuncId : uint32_t {
        Move = 0
    };

    template<typename T>
    struct FuncTable {
        using func_tuple_type = std::tuple<
            decltype(&T::move)
        >;
        constexpr static const func_tuple_type func_tuple = std::make_tuple(
            &T::move
        );
    };

    void move(Entity::Hash oldHash, Entity::Hash newHash) { move_(oldHash, newHash); }

};

template<typename T>
constexpr const typename CpInterface::FuncTable<T>::func_tuple_type CpInterface::FuncTable<T>::func_tuple;



template<typename T>
class ConcreteCpInterface : public CpInterface {
private:
    virtual void move_(Entity::Hash oldHash, Entity::Hash newHash) { move(oldHash, newHash); }

    T& t_;

public:
    ConcreteCpInterface(T& t) : t_(t) { }

    void move(Entity::Hash oldHash, Entity::Hash newHash) { /* default is to do nothing... */std::cout << "RAN DEFAULT!\n";}

};


class Any {
private:
    constexpr static const size_t MaxBuffer = 8;
    uint8_t buffer_[MaxBuffer];

public:
    Any() {
        ASSERT(false);
    }

    template<typename T>
    Any(T& t) {
        ASSERT(sizeof(ConcreteCpInterface<T>) <= MaxBuffer);
        CpInterface* d = new (buffer_) ConcreteCpInterface<T>(t);
        ASSERT(d - reinterpret_cast<CpInterface*>(buffer_) == 0);
    }

    CpInterface* operator->() {
        return reinterpret_cast<CpInterface*>(buffer_);
    }

};



class CpStatic {
private:
    template<uint32_t FUNC, typename T, typename... Args>
    struct Loop {
        using tuple_type = T;
        constexpr static const uint8_t tuple_size = std::tuple_size<tuple_type>::value;

        template<uint32_t INDEX, bool DUMMY = false>
        struct operations {
            static void func(T& t, Args... args) {
                ((make_interface(std::get<INDEX>(t))).*(std::get<FUNC>(CpInterface::FuncTable<ConcreteCpInterface<typename std::tuple_element<INDEX, T>::type>>::func_tuple)))(args...);
                operations<INDEX + 1>::func(t, args...);
            }
        };

        template<bool DUMMY>
        struct operations<tuple_size - 1, DUMMY> {
            constexpr static const uint8_t INDEX = tuple_size - 1;
            static void func(T& t, Args... args) {
                ((make_interface(std::get<INDEX>(t))).*(std::get<FUNC>(CpInterface::FuncTable<ConcreteCpInterface<typename std::tuple_element<INDEX, T>::type>>::func_tuple)))(args...);
            }
        };

        using inner = operations<0>;

    };

public:
    template<uint32_t FUNC, typename T, typename... Args>
    static void loop(T& t, Args... args) {
        Loop<FUNC, T, Args...>::inner::func(t, args...);
    }

    template<typename T>
    static ConcreteCpInterface<T> make_interface(T& t) {
        return ConcreteCpInterface<T>(t);
    }

    template<typename T>
    static Any make_any_interface(T& t) {
        return Any(t);
    }

};






template<typename T>
class Ref {
public:
    template<typename U>
    Ref(U* ptr) : ptr_( reinterpret_cast<U*>(ptr) ) { }

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
    using pair_type = std::pair<Signature::Id, std::vector<T>>;
    using table_type = std::map<Signature::Id, std::vector<T>>;
    table_type table;

public:
    using value_type = T;

    size_t size(Signature signature) const {
        size_t s = 0;
        typename table_type::const_iterator it = table.find(signature);
        if (it != table.end()) {
            s = it->second.size();
        }
        return s;
    }

    bool hasHash(Entity::Hash hash) const {
        bool has = true;
        typename table_type::const_iterator it = table.find(hash.signature);
        if (it == table.end() || hash.index >= it->second.size())
            has = false;
        return has;
    }

    Ref<T> get(Entity::Hash hash) {
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

     void move(Entity::Hash oldHash, Entity::Hash newHash) {
        typename table_type::iterator newSig = table.find(newHash.signature);
        if (newSig == table.end()) {
            if (newHash.signature.value() == 0) return;
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


template<typename T>
class ConcreteCpInterface<VectorMap<T>> : public CpInterface {
private:
    virtual void move_(Entity::Hash oldHash, Entity::Hash newHash) { move(oldHash, newHash); }

    VectorMap<T>& t_;

public:
    ConcreteCpInterface(VectorMap<T>& t) : t_(t) { }

    void move(Entity::Hash oldHash, Entity::Hash newHash) { std::cout << "RAN VECTORMAP!\n"; }

};




class HashManager {
private:
    static const uint16_t EndOfList = 0xffff;

    struct FreeInfo {
        uint16_t head;
        uint16_t size;
    };

    std::map<Signature::Id, FreeInfo> freeMap;
    VectorMap<uint16_t> entityData;
    VectorMap<uint8_t> activeData;

public:

    uint16_t& entityIndex(Entity::Hash hash) {
        return *entityData.get(hash);
    }

    Entity::Hash changeSignature(Entity entity, Entity::Hash oldHash, Signature::Id signature) {
        ASSERT(oldHash.signature != signature);

        Entity::Hash newHash;

        newHash.signature = signature;

        typename std::map<Signature::Id, FreeInfo>::iterator newInfo = freeMap.find(newHash.signature);
        if (newInfo == freeMap.end()) {
            newHash.index = 0;
            if (newHash.signature.value() != 0) {
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

        typename std::map<Signature::Id, FreeInfo>::iterator oldInfo = freeMap.find(oldHash.signature);
        if (oldInfo != freeMap.end()) {
            //activeData.set(oldHash, false);
            *activeData.get(oldHash) = false;
            *entityData.get(oldHash) = oldInfo->second.head;
            oldInfo->second.head = oldHash.index;
        }

        entityData.move(oldHash, newHash);
        activeData.move(oldHash, newHash);

        if (newHash.signature.value() != 0) {
            *entityData.get(newHash) = entity.index();
            //activeData.set(newHash, false);
            *activeData.get(oldHash) = false;
        }

        //hashes[entity.index()] = newHash;

        return newHash;
    }



};


class ProxyContainer {
public:
    ProxyContainer(const Signature& s) { }

private:


};


template<typename... Ts>
class State {
public:
    using any_func_ptr = decltype( &CpStatic::make_any_interface<int> );

    using component_tuple_type = std::tuple<Ts...>;
    using pack_type = std::tuple<typename Ts::value_type...>;

    constexpr static size_t component_count = std::tuple_size<component_tuple_type>::value;


    State() : signatureManager(Signature()/* pass dynamic signature here! */) { }

    void extend(uint16_t e) { entityManager.extend(e); }

/*
    void reserveSignature(Signature::Id id, uint32_t count) {
        Entity::Hash oldHash = EntityHash();
        Entity::Hash newHash = EntityHash(id, count);
        activeData.move(oldHash, newHash);
        entityData.move(oldHash, newHash);
        StaticLoop<Ts...>::move(pack, oldHash, newHash);
    }
*/

    // ReserveSignature  // needs comps
/*
    Entity create() { return entityManager.create(); }

    bool valid(Entity entity) const { return entityManager.valid(entity); }

    const Signature& getSignature(Entity entity) const { return signatureManager.getSignature(entityManager.getHash(entity.index()).signature); }

    const Signature& getSignature(Signature::Id id) const { return signatureManager.getSignature(id); }

    Signature::Id getSignature(const Signature& signature) { return signatureManager.getId(signature); }

    template<uint8_t... Args>
    void changeSignature(Entity entity) {
        Signature::Id id = signatureManager.getId( Signature::make<Args...>() );
        Entity::Hash oldHash = entityManager.getHash(entity.index());
        Entity::Hash newHash = hashManager.changeSignature(entity, oldHash, staticId);
        entityManager.getHash(entity.index()) = newHash;  // this needs to be full signature!
        // move all new components!
        StaticLoop<Ts...>::move(pack, oldHash, newHash);
    }

    template<uint8_t... Args>
    void addSignature(Entity entity) {

    }

    void destroy(Entity entity) {
        changeSignature<>(entity);

        entityManager.destroy(entity);
    }

    Entity entityFromHash(Entity::Hash hash) { entityManager.fromIndex(hashManager.entityIndex(hash)); }

    Entity::Hash hashFromEntity(Entity entity) const { entityManager.getHash(entity.index()); }


    ProxyContainer container(Signature::Id id) {
        return ProxyContainer(signatureManager.getSignature(id));
    }

    ProxyContainer container(const Signature& signature) {
        return ProxyContainer(signature);
    }
*/
    void test_loop() {
        //StaticLoopHelper<Ts...>::move(pack, Entity::Hash(), Entity::Hash());
        //StaticLoopHelper<Ts...>::inner::move(pack, Entity::Hash(), Entity::Hash());

        //StaticLoop::move(pack, Entity::Hash(), Entity::Hash());

        //static_loop<&CpInterface::move>(pack, Entity::Hash() );

        //static_loop<CpFuncId::Move>(pack, Entity::Hash(), Entity::Hash());

        CpStatic::loop<CpInterface::Move>(pack, Entity::Hash(), Entity::Hash());

    }

private:

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


    SignatureManager signatureManager;

    EntityManager entityManager;

    HashManager hashManager;

    component_tuple_type pack;

};






#endif // ENTITY_HPP
