#ifndef ENTITY_HPP
#define ENTITY_HPP


#include <Container.hpp>

/*
    TSequence
    Entity
    Hash
    Any
    State

    Ref
    Vector
    HashMap
    VectorMap
    EmptyHashMap
*/


struct cpp17_fold_expand_type {
    template <typename... T>
    cpp17_fold_expand_type(T&&...) { }
};
#define CPP17_FOLD(x)   cpp17_fold_expand_type{ 0, ((x), void(), 0)... }



struct TSequence {
private:
    constexpr static uint32_t Null = 0xffffffff;

    template<uint32_t... Ns>
    struct MakeHelper { };

    template<uint32_t H, uint32_t... Ns>
    struct MakeHelper<H, Ns...> {
        static const uint32_t size = sizeof...(Ns) + 1;
        static const uint32_t head = H;
        using tail = MakeHelper<Ns...>;
        constexpr static uint32_t value[sizeof...(Ns) + 2] = { H, Ns..., Null };
        static uint32_t find(uint32_t key) {
            uint32_t index = Null;
            index = (H == key) ? H : index;
            CPP17_FOLD( index = (Ns == key) ? Ns : index );
            ASSERT(index != Null);
            return index;
        }
    };

    template <uint32_t APPEND, typename TAIL, uint32_t...Ns>
    struct AppendBack {
        using type = typename AppendBack<APPEND, typename TAIL::tail, Ns..., TAIL::head>::type;
    };

    template <uint32_t APPEND, uint32_t...Ns>
    struct AppendBack<APPEND, MakeHelper<>, Ns...> {
        using type = MakeHelper<Ns..., APPEND>;
    };

    template<bool ADD, uint32_t HEAD, typename S>
    struct IfCond;

    template<uint32_t HEAD, typename S>
    struct IfCond<true, HEAD, S> {
        using type = typename AppendBack<HEAD, S>::type;
    };

    template<uint32_t HEAD, typename S>
    struct IfCond<false, HEAD, S> {
        using type = S;
    };

    template <template<typename> typename W, typename T, typename IS, typename OS = MakeHelper<>>
    struct RemoveIf {
        using type = typename RemoveIf<W, T, typename IS::tail, typename IfCond<W<typename std::tuple_element<IS::head, T>::type>::value, IS::head, OS>::type>::type;
    };

    template <template<typename> typename W, typename T, typename OS>
    struct RemoveIf<W, T, MakeHelper<>, OS> {
        using type = OS;
    };

    template<uint32_t N, uint32_t I = 0, typename OS = MakeHelper<>>
    struct RangeHelper {
        using type = typename RangeHelper<N, I + 1, typename AppendBack<I, OS>::type>::type;
    };

    template <uint32_t N, typename OS>
    struct RangeHelper<N, N, OS> {
        using type = OS;
    };

public:
    template<uint32_t... Ns>
    using Make = MakeHelper<Ns...>;

    template<uint32_t N>
    using Range0 = typename RangeHelper<N>::type;

    template<uint32_t I, uint32_t N>
    using Range = typename RangeHelper<N, I>::type;

    template <template<typename> typename W, typename T, typename IS = Range0<std::tuple_size<T>::value>, typename OS = Make<>>
    using Remove = typename RemoveIf<W, T, IS>::type;

    template<typename S, uint32_t V>
    using PushBack = typename AppendBack<V, S>::type;

};

template<uint32_t H, uint32_t... Ns>
constexpr uint32_t TSequence::MakeHelper<H, Ns...>::value[];


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
    template<typename... Ts>
    friend class State;

    struct SignatureIndex {
        uint16_t index;
        uint16_t signature;
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
        return data.tracked.index;
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
    template<typename... Ts>
    friend class State;

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


/*
class Any {
private:
    constexpr static const size_t MaxBuffer = 8;

    uint8_t buffer_[MaxBuffer];

    template<bool C, typename T = void>
    struct enable_if {
        typedef T type;
    };

    template<typename T>
    struct enable_if<false, T> { };

    struct Base {
        virtual void print() = 0;
        virtual void move(Hash oldHash, Hash newHash) = 0;
    };

    template<typename T>
    struct Derived : public Base {
        //
        //  Print function!
        // can specialize this if you want!
        //
        //  template<typename _1> static typename enable_if<Print<_1>::value, RETURN_TYPE (void)>::type invoke(_1& t) {
        //
        struct Print {
            template <typename U, U> struct type_check;
            template <typename _1> static uint16_t &chk(type_check<void (_1::*)(), &_1::print> *);
            template <typename   > static  uint8_t &chk(...);
            static bool const value = sizeof(chk<T>(0)) == sizeof(uint16_t);
            template<typename _1> static typename enable_if<!Print::value>::type invoke(T& t) { std::cout << "DEFAULT!" << std::endl; }
            template<typename _1> static typename enable_if<Print::value>::type invoke(T& t) { t.print(); }
        };
        T t_;
        Derived(const T& t) : t_(t) { }
        void print() { Print::invoke(t_); }
        void move(Hash oldHash, Hash newHash) { //Move::<T>::invoke(t_, oldHash, newHash);
         }
    };

    Base* base() { return reinterpret_cast<Base*>(buffer_); }

public:
    Any() {
        ASSERT(false);
    }

    template<typename T>
    Any(T& t) {
        ASSERT(sizeof(Derived<T>) <= MaxBuffer);
        Base* d = new (buffer_) Derived<T>(t);
        ASSERT(d - base() == 0);
    }

    Base* operator->() { return reinterpret_cast<Base*>(buffer_); }

};
*/

template<typename T>    struct needs_move                  { static const bool value = false; };
template<typename T>    struct needs_move<VectorMap<T>>    { static const bool value = true;  };

template<typename T>    struct dynamic_comp                { static const bool value = false; };
template<typename T>    struct dynamic_comp<HashMap<T>>    { static const bool value = true;  };
template<>              struct dynamic_comp<EmptyHashMap>  { static const bool value = true;  };

template<typename... Ts>
class State {
public:
    constexpr static uint16_t EndOfList = 0xffff;

    struct GenerationInfo {
        GenerationInfo() : generation(0), unsafe(0) { }
        GenerationInfo(uint16_t g) : generation(g), unsafe(0) { }
        uint16_t generation : 15;
        uint16_t unsafe : 1;
    };

    struct FreeInfo {
        FreeInfo() : head(EndOfList), size(1) { }
        uint16_t head;
        uint16_t size;
    };

    struct FreeComp {
        FreeComp() : head(EndOfList), tail(EndOfList) { }
        uint16_t head;
        uint16_t tail;
    };

    using data_type = std::tuple<
        Ts...,                  // user defined components
        EmptyHashMap,           // active component -7
        Vector<Hash>,           // hashes -6
        Vector<GenerationInfo>, // generation id -5
        VectorMap<uint8_t>,     // dynamic comps -4
        VectorMap<uint16_t>,    // entity -3
        HashMap<FreeInfo>,      // free hashes -2
        Single<FreeComp>        // free entities -1
    >;
    constexpr static size_t pack_size = std::tuple_size<data_type>::value;

    template<uint32_t N>
    using container_type = typename std::tuple_element<N, data_type>::type;

    template<uint32_t N>
    using component_type = typename container_type<N>::value_type;

    /*template<uint32_t... Ns>
    using ref_type = std::tuple<Ref<component_type<Ns>>...>;

    template<uint32_t... Ns>
    using iter_type = std::tuple<typename std::vector<component_type<Ns>>::iterator...>;*/

    using all_sequence = TSequence::Range0<pack_size - 6>;
    constexpr static uint32_t activeComp = pack_size - 7;
    constexpr static uint32_t hashComp = pack_size - 6;
    constexpr static uint32_t generationComp = pack_size - 5;
    constexpr static uint32_t dynamicComp = pack_size - 4;
    constexpr static uint32_t entityComp = pack_size - 3;
    constexpr static uint32_t freeHashComp = pack_size - 2;
    constexpr static uint32_t freeEntityComp = pack_size - 1;

    using vector_map_sequence = TSequence::Remove<needs_move, data_type, all_sequence>;
    using dynamic_comp_sequence = TSequence::Remove<dynamic_comp, data_type, all_sequence>;

    static const uint32_t activeCompFlagIndex;
    static bool is_active(uint8_t f) {
        return f & (0x01 << activeCompFlagIndex);
    }

/*
    template<uint32_t... Ns>
    struct Reference {
        Entity entity;
        Hash hash;
        ref_type<Ns...> pack;
    };
*/

    data_type data;

/*
    void extend(uint16_t e) {
        auto& generationData = std::get<State::generationComp>(data);
        auto& hashData = std::get<State::hashComp>(data);
        uint16_t s = generationData.size();
        Entity::Tracked entity;
        entity.untracked = 0;
        entity.generation = 0;
        generationData.extend(e);
        hashData.extend(e);
        for (uint16_t i = s; i < e; ++i) {
            entity.index = i;
            destroy( Entity(entity) );
        }
    }

    template<uint32_t... Ns>
    void reserveSignature(uint16_t count) {
        auto& hashData = std::get<State::hashComp>(data);
        auto& activeData = std::get<State::activeComp>(data);
        auto& entityData = std::get<State::entityComp>(data);

        Hash::SignatureIndex nh;
        Hash::SignatureIndex oh;

        nh.index = count;
        nh.signature = 0x0000;

        oh.index = 0;
        oh.signature = 0x0000;

        Hash newHash = Hash(nh);
        Hash oldHash = Hash(oh);

        // generate signature hash
        CPP17_FOLD( nh.signature |= (0x0001 << vector_map_sequence::find(Ns)) );

        entityData.move( oldHash.raw(), newHash.raw() );
        activeData.move( oldHash.raw(), newHash.raw() );

        // move all components needed to move
        CPP17_FOLD( std::get<Ns>(data).move(oldHash.raw(), newHash.raw()) );
    }
*/

    Entity create() {
        auto& freeData = std::get<State::freeEntityComp>(data).get();
        auto& generationData = std::get<State::generationComp>(data);
        auto& hashData = std::get<State::hashComp>(data);
        Entity::Tracked entity;
        if (freeData.head == EndOfList) {
            entity.index = generationData.size();
            generationData.extend(entity.index);
            hashData.extend(entity.index);
            generationData[entity.index] = GenerationInfo(1);
            hashData[entity.index] = Hash();
        } else {
            entity.index = freeData.head;
            freeData.head = hashData[entity.index].index();
            hashData[entity.index] = Hash();
            if (freeData.head == EndOfList)
                freeData.tail = EndOfList;
        }
        entity.untracked = 0;
        entity.generation = generationData[entity.index].generation;
        return Entity(entity);
    }

    bool valid(Entity entity) const {
        if ( entity.untracked() == 1 || std::get<State::generationComp>(data)[entity.index()].generation == entity.generation() ) {
            return true;
        }
        return false;
    }
/*
    uint32_t signature(Entity entity) {
        uint32_t r = 0x00000000;
        auto& hashData = std::get<State::hashComp>(data);
        Hash hash = hashData[entity.index()];
        uint16_t s = hash.signature();
        uint16_t d = hash.index();

        if (hash.signature() != 0) {
            auto& dynamicData = std::get<State::dynamicComp>(data);
            d = dynamicData[hash.raw()];
        }

        if (d & (0x0001 << 0)) r |= (0x00000001 << dynamic_comp_sequence::value[0]);
        if (d & (0x0001 << 1)) r |= (0x00000001 << dynamic_comp_sequence::value[1]);
        if (d & (0x0001 << 2)) r |= (0x00000001 << dynamic_comp_sequence::value[2]);
        if (d & (0x0001 << 3)) r |= (0x00000001 << dynamic_comp_sequence::value[3]);
        if (d & (0x0001 << 4)) r |= (0x00000001 << dynamic_comp_sequence::value[4]);
        if (d & (0x0001 << 5)) r |= (0x00000001 << dynamic_comp_sequence::value[5]);
        if (d & (0x0001 << 6)) r |= (0x00000001 << dynamic_comp_sequence::value[6]);
        if (d & (0x0001 << 7)) r |= (0x00000001 << dynamic_comp_sequence::value[7]);

        if (s & (0x0001 <<  0)) r |= (0x00000001 << vector_map_sequence::value[ 0]);
        if (s & (0x0001 <<  1)) r |= (0x00000001 << vector_map_sequence::value[ 1]);
        if (s & (0x0001 <<  2)) r |= (0x00000001 << vector_map_sequence::value[ 2]);
        if (s & (0x0001 <<  3)) r |= (0x00000001 << vector_map_sequence::value[ 3]);
        if (s & (0x0001 <<  4)) r |= (0x00000001 << vector_map_sequence::value[ 4]);
        if (s & (0x0001 <<  5)) r |= (0x00000001 << vector_map_sequence::value[ 5]);
        if (s & (0x0001 <<  6)) r |= (0x00000001 << vector_map_sequence::value[ 6]);
        if (s & (0x0001 <<  7)) r |= (0x00000001 << vector_map_sequence::value[ 7]);
        if (s & (0x0001 <<  8)) r |= (0x00000001 << vector_map_sequence::value[ 8]);
        if (s & (0x0001 <<  9)) r |= (0x00000001 << vector_map_sequence::value[ 9]);
        if (s & (0x0001 << 10)) r |= (0x00000001 << vector_map_sequence::value[10]);
        if (s & (0x0001 << 11)) r |= (0x00000001 << vector_map_sequence::value[11]);
        if (s & (0x0001 << 12)) r |= (0x00000001 << vector_map_sequence::value[12]);
        if (s & (0x0001 << 13)) r |= (0x00000001 << vector_map_sequence::value[13]);
        if (s & (0x0001 << 14)) r |= (0x00000001 << vector_map_sequence::value[14]);
        if (s & (0x0001 << 15)) r |= (0x00000001 << vector_map_sequence::value[15]);

        return r;
    }
*/
    Entity entityFromIndex(uint16_t index) {
        auto& generationData = std::get<State::generationComp>(data);
        Entity::Tracked entity;
        entity.untracked = 0;
        entity.index = index;
        entity.generation = generationData[entity.index].generation;
        return Entity(entity);
    }

    Hash hashFromIndex(uint16_t index) {
        auto& hashData = std::get<State::hashComp>(data);
        return hashData[index];
    }

    Entity entityFromHash(Hash hash) {
        auto& entityData = std::get<State::entityComp>(data);
        return entityFromIndex( entityData[hash.raw()] );
    }

    Hash hashFromEntity(Entity entity) {
        return hashFromIndex(entity.index());
    }

    template<uint32_t... Ns>
    void changeSignature(Entity entity) {
        auto& freeData = std::get<State::freeHashComp>(data);
        auto& hashData = std::get<State::hashComp>(data);
        auto& dynamicData = std::get<State::dynamicComp>(data);
        auto& entityData = std::get<State::entityComp>(data);

        Hash::SignatureIndex nh;

        nh.index = 0;
        nh.signature = 0x0000;

        // generate signature hash
        CPP17_FOLD( nh.signature |= (0x0001 << vector_map_sequence::find(Ns)) );

        Hash oldHash = hashData[entity.index()];

        if (oldHash.signature() == nh.signature) return;

        auto newInfo = freeData.find(nh.signature);
        if ( newInfo.isNull() ) {
            //nh.index = 0;
            if (nh.signature != 0) {
                newInfo = freeData.insert(nh.signature);
                *newInfo = FreeInfo();
            }
        } else {
            if (newInfo->head == EndOfList) {
                nh.index = newInfo->size;
                ++(newInfo->size);
            } else {
                nh.index = newInfo->head;
                newInfo->head = entityData[Hash(nh).raw()];
            }
        }
        Hash newHash = Hash(nh);


        uint8_t dynamicSig = 0x00;
        auto oldInfo = freeData.find(oldHash.signature());
        if ( !oldInfo.isNull() ) {
            entityData[oldHash.raw()] = oldInfo->head;
            oldInfo->head = oldHash.index();
            dynamicSig = dynamicData[oldHash.raw()];
            dynamicSig &= ~(0x01 << activeCompFlagIndex);
            dynamicData[oldHash.raw()] = dynamicSig;
        } else {
            dynamicSig = (uint8_t)oldHash.index();
            dynamicSig &= ~(0x01 << activeCompFlagIndex);
        }

        entityData.move( oldHash.raw(), newHash.raw() );
        dynamicData.move( oldHash.raw(), newHash.raw() );

        // move all components needed to move
        CPP17_FOLD( std::get<Ns>(data).move(oldHash.raw(), newHash.raw()) );

        if (newHash.signature() != 0) {
            entityData[newHash.raw()] = entity.index();
            dynamicData[newHash.raw()] = dynamicSig;
        } else {
            // new hash is null, move dynamic signature to hash index
            nh.index = (uint16_t)dynamicSig;
            newHash = Hash(nh);
        }

        hashData[entity.index()] = newHash;
    }

    template<uint32_t... Ns>
    void add(Entity entity) {
        auto& hashData = std::get<State::hashComp>(data);
        auto& hash = hashData[entity.index()];

        uint16_t signature = 0x0000;
        CPP17_FOLD( signature |= (0x0001 << dynamic_comp_sequence::find(Ns)) );

        if (hash.signature() == 0) {
            hash = Hash(hash.signature(), hash.index() | signature);
        } else {
            auto& dynamicData = std::get<State::dynamicComp>(data);
            auto& oldSig = dynamicData[hash.raw()];
            oldSig |= signature;
        }

        CPP17_FOLD( std::get<Ns>(data).insert(entity.index()) );
    }

    template<uint32_t... Ns>
    void remove(Entity entity) {
        auto& hashData = std::get<State::hashComp>(data);
        auto& hash = hashData[entity.index()];

        uint16_t signature = 0x0000;
        CPP17_FOLD( signature |= (0x0001 << dynamic_comp_sequence::find(Ns)) );

        if (hash.signature() == 0) {
             hash = Hash(hash.signature(), hash.index() & ~signature);
        } else {
            auto& dynamicData = std::get<State::dynamicComp>(data);
            auto& oldSig = dynamicData[hash.raw()];
            oldSig &= ~signature;
        }

        CPP17_FOLD( std::get<Ns>(data).erase(entity.index()) );
    }



    void setActive(Entity entity) {
        add<activeComp>(entity);
    }

    void setInactive(Entity entity) {
        remove<activeComp>(entity);
    }

    bool isActive(Entity entity) {
        auto& hashData = std::get<State::hashComp>(data);
        Hash hash = hashData[entity.index()];
        uint16_t d = hash.index();

        if (hash.signature() != 0) {
            auto& dynamicData = std::get<State::dynamicComp>(data);
            d = dynamicData[hash.raw()];
        }

        return is_active(d);
    }

    template<typename T>
    struct removeAll;
    template<uint32_t... Ns>
    struct removeAll<TSequence::Make<Ns...>> {
        static void invoke(State* s, Entity entity) {
            s->remove<Ns...>(entity);
        }
    };

    void destroy(Entity entity) {
        // need to remove all dynamic comps here!
        removeAll<dynamic_comp_sequence>::invoke(this, entity);
        changeSignature(entity);
        auto& freeData = std::get<State::freeEntityComp>(data).get();
        auto& generationData = std::get<State::generationComp>(data);
        auto& hashData = std::get<State::hashComp>(data);
        if (generationData[entity.index()].generation == 0x7fff) {
            //    For debugging it's good to know if we hit the
            //    end of a generation increment.  Otherwise we
            //    can just let it turn over at our own risk...

            //    If this happens, try using the Extend function
            //    to get more active entities from the start.
            ASSERT(false);
            //generationData[entity.index()].unsafe = 0;
            //generationData[entity.index()].generation = 1;
            //hashData[entity.index()] = Hash(0, EndOfList);
            //if (freeData.head == EndOfList) {
            //    freeData.head = entity.index();
            //} else {
            //    hashData[freeData.tail].index() = entity.index();
            //}
            //freeData.tail = entity.index();
        } else {
            // we won't increment if unsafe is set to 1
            generationData[entity.index()].unsafe = 0;
            ++generationData[entity.index()].generation;
            hashData[entity.index()] = Hash(0, EndOfList);
            if (freeData.head == EndOfList) {
                freeData.head = entity.index();
            } else {
                hashData[freeData.tail] = Hash(0, entity.index());
            }
            freeData.tail = entity.index();
        }
    }

    template<uint32_t... Ns, typename F>
    void forEach(F func) {
        auto& dynamicData = std::get<State::dynamicComp>(data);
        Hash::SignatureIndex hash;
        uint16_t signature = 0x0000;
        CPP17_FOLD( signature |= (0x0001 << vector_map_sequence::find(Ns)) );
        for (auto& kv : dynamicData) {
            hash.signature = kv.first;
            if ( (hash.signature & signature) == signature ) {
                for (hash.index = 0; hash.index < kv.second.size(); ++hash.index) {
                    if ( is_active(dynamicData[Hash(hash).raw()]) == true ) {
                        func( Hash(hash) );
                    }
                }
            }
        }
    }

    /*
        IComponent classes are for interfacing with the component data from the outside
    */
    template<typename T, uint32_t COMP>
    class IComponent;

    template<typename T, uint32_t COMP>
    class IComponent<VectorMap<T>, COMP> {
    private:
        State& state;

    public:
        IComponent(State* s) : state(*s) { }

        Ref<T> find(Entity entity) {
            auto& componentData = std::get<COMP>(state.data);
            auto& hashData = std::get<State::hashComp>(state.data);
            return componentData.find(hashData[entity.index()].raw());
        }

        Ref<T> find(Hash hash) {
            return std::get<COMP>(state.data).find(hash.raw());
        }

        Ref<T> find(Entity entity, Hash hash) {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.find( hash.raw() );
        }

    };

    template<typename T, uint32_t COMP>
    class IComponent<HashMap<T>, COMP> {
    private:
        State& state;

    public:
        IComponent(State* s) : state(*s) { }

        typename container_type<COMP>::table_type::iterator begin() {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.begin();
        }

        typename container_type<COMP>::table_type::iterator end() {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.end();
        }

        Ref<T> find(Entity entity) {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.find( entity.index() );
        }

        Ref<T> find(Entity entity, Hash hash) {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.find( entity.index() );
        }

    };

    template<typename T, uint32_t COMP>
    friend class IComponent;


    template<uint32_t N>
    auto find(Entity entity) -> decltype(IComponent<container_type<N>, N>(this).find(entity)) {
        return IComponent<container_type<N>, N>(this).find(entity);
    }

    template<uint32_t N>
    auto find(Hash hash) -> decltype(IComponent<container_type<N>, N>(this).find(hash)) {
        return IComponent<container_type<N>, N>(this).find(hash);
    }

    template<uint32_t N>
    auto container() -> decltype(IComponent<container_type<N>, N>(this)) {
        return IComponent<container_type<N>, N>(this);
    }


    /*
    template<typename S>
    struct any_helper;

    template<uint32_t... Ns>
    struct any_helper<TSequence::Make<Ns...>> {
        using func_ptr = decltype(&State::container<0>);
        static func_ptr table[] = { &State::container<Ns>... };
    };

    //Any any(uint32_t comp) {
    //    return Any( this->(*(any_helper<all_sequence>::table[comp]))() );
    //}
*/

    /*
    template<typename T, typename S, typename R>
    struct reference_helper;

    template<typename T, typename S, uint32_t... Ns>
    struct reference_helper<T, S, TSequence::Make<Ns...>> {
        static void invoke(State& state, T& t, Entity entity, Hash hash) {
            CPP17_FOLD( std::get<Ns>(t) = state.container<S::value[Ns]>().find(entity, hash) );
        }
    };

    template<uint32_t... Ns>
    ref_type<Ns...> reference(Entity entity) {
        auto& hashData = std::get<State::hashComp>(data);
        Hash hash = hashData[entity.index()];
        ref_type<Ns...> ref;
        reference_helper<ref_type<Ns...>, TSequence::Make<Ns...>, TSequence::Range0<sizeof...(Ns)>>::invoke(*this, ref, entity, hash);
        return ref;
    }
    */


    /*
        This 'container proxy' is used to iterate though signatures of components
    */
    /*
    template<typename T, typename P, typename S, typename R>
    class ContainerProxy;

    template<typename T, typename P, typename S, uint32_t... Ns>
    class ContainerProxy<T, P, S, TSequence::Make<Ns...>> {
    public:
        ContainerProxy(State* s, uint16_t g) : state(s), signature(g) { }

        class Iterator {
        public:
            Iterator(State* s, uint16_t g) : state(s), signature(g) { }

            bool operator==(const Iterator& rhs) const {
                return table == rhs.table;
            }

            bool operator!=(const Iterator& rhs) const {
                return table != rhs.table;
            }

            T& operator*() {
                CPP17_FOLD( std::get<Ns>(references.pack) = Ref<component_type<S::value[Ns]>>( &(*(std::get<Ns>(pack))) ) );
                references.hash = Hash(hash.signature, hash.index);
                //references.entity = ?;
                return references;
            }

            Iterator& operator++() {
                for (;;) {
                    ++hash.index;
                    ++dynamic;
                    CPP17_FOLD( ++(std::get<Ns>(pack)) );
                    if (dynamic == dynamic_end) {
                        for (;;) {
                            ++table;
                            if (table == table_end) {
                                return *this;
                            }
                            if ( (table->first & signature) == signature ) {
                                hash.signature = signature;
                                hash.index = 0;
                                dynamic = table->second.begin();
                                dynamic_end = table->second.end();
                                CPP17_FOLD( std::get<Ns>(pack) = std::get<S::value[Ns]>(state->data).begin(table->first) );
                                break;
                            }
                        }
                    }
                    if (is_active(*dynamic) == true) {
                        return *this;
                    }
                }
            }

        private:
            friend class ContainerProxy;

            using table_iterator = typename container_type<dynamicComp>::table_type::iterator;
            using vector_iterator = typename std::vector<component_type<dynamicComp>>::iterator;

            State* state;
            uint16_t signature;

            table_iterator table;
            table_iterator table_end;
            vector_iterator dynamic;
            vector_iterator dynamic_end;

            P pack;
            T references;

            Entity::Tracked entity;
            Hash::SignatureIndex hash;
        };

        Iterator begin() {
            auto& dynamicData = std::get<dynamicComp>(state->data);
            Iterator it(state, signature);
            it.table_end = dynamicData.end();
            it.table = dynamicData.begin();
            while ( it.table != it.table_end && (it.table->first & signature) != signature ) {
                ++it.table;
            }
            if ( it.table != it.table_end ) {
                it.hash.index = 0;
                it.hash.signature = it.table->first;
                it.dynamic = it.table->second.begin();
                it.dynamic_end = it.table->second.end();
                CPP17_FOLD( std::get<Ns>(it.pack) = std::get<S::value[Ns]>(state->data).begin(it.table->first) );
                if ( it.dynamic != it.dynamic_end && is_active( *(it.dynamic) ) == false ) {
                    ++it;
                }
            }
            return it;
        }

        Iterator end() {
            auto& dynamicData = std::get<dynamicComp>(state->data);
            Iterator it(state, signature);
            it.table_end = dynamicData.end();
            it.table = it.table_end;
            return it;
        }

    private:
        State* state;
        uint16_t signature;
    };

    template<typename T, typename P, typename S, typename R>
    friend class ContainerProxy;

    template<uint32_t... Ns>
    using ContainerProxyHelp = ContainerProxy<Reference<Ns...>, iter_type<Ns...>, TSequence::Make<Ns...>, TSequence::Range0<sizeof...(Ns)>>;

    template<uint32_t... Ns>
    ContainerProxyHelp<Ns...> iterate() {
        // make signature here
        uint16_t signature = 0x0000;
        CPP17_FOLD( signature |= (0x0001 << vector_map_sequence::find(Ns)) );
        return ContainerProxyHelp<Ns...>(this, signature);
    }
    */
};

template<typename... Ts>
const uint32_t State<Ts...>::activeCompFlagIndex = State<Ts...>::dynamic_comp_sequence::find(activeComp);


#endif // ENTITY_HPP
