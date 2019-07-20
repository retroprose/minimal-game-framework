#ifndef ENTITY_HPP
#define ENTITY_HPP


#include <Container.hpp>

/*
    TSequence
    Entity
    Any
    State

    Ref
    Vector
    HashMap
    VMHash
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
            assert(index != Null);
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
        assert(data.tracked.untracked == 0);
        return data.tracked.index;
    }

    uint16_t generation() const {
        assert(data.tracked.untracked == 0);
        return data.tracked.generation;
    }

    uint16_t type() const {
        assert(data.tracked.untracked == 1);
        return data.untracked.type;
    }

    uint16_t user() const {
        assert(data.tracked.untracked == 1);
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
        assert(false);
    }

    template<typename T>
    Any(T& t) {
        assert(sizeof(Derived<T>) <= MaxBuffer);
        Base* d = new (buffer_) Derived<T>(t);
        assert(d - base() == 0);
    }

    Base* operator->() { return reinterpret_cast<Base*>(buffer_); }

};
*/



/*
    extend
    reserve

    create
    valid
    signature

    entityFromIndex
    hashFromIndex
    entityFromHash
    hashFromEntity

    change
    add
    remove

    isActive
    setActive
    setInactive

    removeAll
    destroy

    forEach (static & dynamic)
    reference

    container
    any         (dynamic version of container)
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
        Vector<VMHash>,         // hashes -6
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

    template<uint32_t... Ns>
    using refs_type = std::tuple<Ref<component_type<Ns>>...>;

    template<typename T>
    struct Reference;

    template<uint32_t... Ns>
    struct Reference<TSequence::Make<Ns...>> {
        uint16_t index;
        Entity entity;
        VMHash hash;
        refs_type<Ns...> pack;
    };

    template<uint32_t N>
    struct SReference {
        uint16_t index;
        Ref<component_type<N>> value;
    };


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
            hashData[entity.index] = VMHash();
        } else {
            entity.index = freeData.head;
            freeData.head = hashData[entity.index].index;
            hashData[entity.index] = VMHash();
            if (freeData.head == EndOfList)
                freeData.tail = EndOfList;
        }
        entity.untracked = 0;
        entity.generation = generationData[entity.index].generation;
        return Entity(entity);
    }

    bool valid(Entity entity) const {
        if (
            entity.untracked() == 1 ||
            entity.generation() == 0 ||
            std::get<State::generationComp>(data)[entity.index()].generation == entity.generation()
        ) {
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

    VMHash hashFromIndex(uint16_t index) {
        auto& hashData = std::get<State::hashComp>(data);
        return hashData[index];
    }

    Entity entityFromHash(VMHash hash) {
        auto& entityData = std::get<State::entityComp>(data);
        return entityFromIndex( entityData[hash] );
    }

    VMHash hashFromEntity(Entity entity) {
        return hashFromIndex(entity.index());
    }

    template<uint32_t... Ns>
    void change(Entity entity) {
        auto& freeData = std::get<State::freeHashComp>(data);
        auto& hashData = std::get<State::hashComp>(data);
        auto& dynamicData = std::get<State::dynamicComp>(data);
        auto& entityData = std::get<State::entityComp>(data);

        VMHash newHash(0, 0);

        // generate signature hash
        CPP17_FOLD( newHash.signature |= (0x0001 << vector_map_sequence::find(Ns)) );

        VMHash oldHash = hashData[entity.index()];

        if (oldHash.signature == newHash.signature) return;

        auto newInfo = freeData.find(newHash.signature);
        if ( newInfo.isNull() ) {
            if (newHash.signature != 0) {
                newInfo = freeData.insert(newHash.signature);
                *newInfo = FreeInfo();
            }
        } else {
            if (newInfo->head == EndOfList) {
                newHash.index = newInfo->size;
                ++(newInfo->size);
            } else {
                newHash.index = newInfo->head;
                newInfo->head = entityData[newHash];
            }
        }

        uint8_t dynamicSig = 0x00;
        auto oldInfo = freeData.find(oldHash.signature);
        if ( !oldInfo.isNull() ) {
            entityData[oldHash] = oldInfo->head;
            oldInfo->head = oldHash.index;
            dynamicSig = dynamicData[oldHash];
            dynamicSig &= ~(0x01 << activeCompFlagIndex);
            dynamicData[oldHash] = dynamicSig;
        } else {
            dynamicSig = (uint8_t)oldHash.index;
            dynamicSig &= ~(0x01 << activeCompFlagIndex);
        }

        entityData.move( oldHash, newHash );
        dynamicData.move( oldHash, newHash );

        // move all components needed to move
        CPP17_FOLD( std::get<Ns>(data).move(oldHash, newHash) );

        if (newHash.signature != 0) {
            entityData[newHash] = entity.index();
            dynamicData[newHash] = dynamicSig;
        } else {
            // new hash is null, move dynamic signature to hash index
            newHash.index = (uint16_t)dynamicSig;
        }

        hashData[entity.index()] = newHash;
    }

    template<uint32_t... Ns>
    void add(Entity entity) {
        auto& hashData = std::get<State::hashComp>(data);
        auto& hash = hashData[entity.index()];

        uint16_t signature = 0x0000;
        CPP17_FOLD( signature |= (0x0001 << dynamic_comp_sequence::find(Ns)) );

        if (hash.signature == 0) {
            hash.index |= signature;
        } else {
            auto& dynamicData = std::get<State::dynamicComp>(data);
            auto& oldSig = dynamicData[hash];
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

        if (hash.signature == 0) {
            hash.index &= ~signature;
        } else {
            auto& dynamicData = std::get<State::dynamicComp>(data);
            auto& oldSig = dynamicData[hash];
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
        VMHash hash = hashData[entity.index()];
        uint16_t d = hash.index;

        if (hash.signature() != 0) {
            auto& dynamicData = std::get<State::dynamicComp>(data);
            d = dynamicData[hash];
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
        change(entity);
        auto& freeData = std::get<State::freeEntityComp>(data).get();
        auto& generationData = std::get<State::generationComp>(data);
        auto& hashData = std::get<State::hashComp>(data);
        if (generationData[entity.index()].generation == 0x7fff) {
            //    For debugging it's good to know if we hit the
            //    end of a generation increment.  Otherwise we
            //    can just let it turn over at our own risk...

            //    If this happens, try using the Extend function
            //    to get more active entities from the start.
            assert(false);
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
            hashData[entity.index()] = VMHash(0, EndOfList);
            if (freeData.head == EndOfList) {
                freeData.head = entity.index();
            } else {
                hashData[freeData.tail] = VMHash(0, entity.index());
            }
            freeData.tail = entity.index();
        }
    }

    template<typename S, typename R>
    struct ReferenceImpl;

    template<typename S, uint32_t... Ns>
    struct ReferenceImpl<S, TSequence::Make<Ns...>> {
        static void invoke(State& state, Reference<S>& t) {
            CPP17_FOLD( std::get<Ns>(t.pack) = state.container<S::value[Ns]>().find(t.entity, t.hash) );
        }
    };

    template<typename S>
    Reference<S> reference(Entity entity);

    template<uint32_t... Ns>
    Reference<TSequence::Make<Ns...>> reference(Entity entity) {
        Reference<TSequence::Make<Ns...>> ref;
        ref.index = entity.index();
        ref.entity = entity;
        auto& hashData = std::get<State::hashComp>(data);
        ref.hash = hashData[entity.index()];
        ReferenceImpl<TSequence::Make<Ns...>, TSequence::Range0<TSequence::Make<Ns...>::size>>::invoke(*this, ref);
        return ref;
    }


    template<typename S>
    struct Iterators;

    template<uint32_t... Ns>
    struct Iterators<TSequence::Make<Ns...>> {
        std::tuple<typename std::vector<component_type<Ns>>::iterator...> pack;
    };

    template<typename S, typename F, typename R>
    struct forEachImpl;

    template<typename S, typename F, uint32_t... Ns>
    struct forEachImpl<S, F, TSequence::Make<Ns...>> {
        static void invoke(State& state, F func) {
            auto& dynamicData = std::get<State::dynamicComp>(state.data);
            Entity nullEntity;
            Reference<S> ref;
            Iterators<S> its;
            VMHash hash;
            uint16_t signature = 0x0000;
            CPP17_FOLD( signature |= (0x0001 << vector_map_sequence::find(S::value[Ns])) );
            for (auto& kv : dynamicData) {
                hash.signature = kv.first;
                if ( (hash.signature & signature) == signature ) {
                    CPP17_FOLD( std::get<Ns>(its.pack) = std::get<S::value[Ns]>(state.data).begin(hash.signature) );
                    for (hash.index = 0; hash.index < kv.second.size(); ++hash.index) {
                        if ( is_active(dynamicData[hash]) == true ) {
                            ref.index = 0;
                            ref.hash = hash;
                            ref.entity = nullEntity;
                            CPP17_FOLD( std::get<Ns>(ref.pack) = Ref<component_type<S::value[Ns]>>( &(*(std::get<Ns>(its.pack))) ) );
                            func(ref);
                            CPP17_FOLD( ++(std::get<Ns>(its.pack)) );
                        }
                    }
                }
            }
        }
    };

    template<typename S, typename F>
    void forEach(F func) {
        forEachImpl<S, F, TSequence::Range0<S::size>>::invoke(*this, func);
    }

    template<uint32_t N, typename F>
    void forEach(F func) {
        SReference<N> ref;
        auto& componentData = std::get<N>(data);
        auto it = componentData.begin();
        while ( it != componentData.end() ) {
            ref.index = it->first;
            ref.value = Ref<component_type<N>>( &(it->second) );
            func(ref);
            ++it;
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
            return componentData.find(hashData[entity.index()]);
        }

        Ref<T> find(VMHash hash) {
            return std::get<COMP>(state.data).find(hash);
        }

        Ref<T> find(Entity entity, VMHash hash) {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.find(hash);
        }

    };

    template<typename T, uint32_t COMP>
    class IComponent<HashMap<T>, COMP> {
    private:
        State& state;

    public:
        IComponent(State* s) : state(*s) { }

        Ref<T> find(Entity entity) {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.find( entity.index() );
        }

        Ref<T> find(Entity entity, VMHash hash) {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.find( entity.index() );
        }

    };

    template<typename T, uint32_t COMP>
    friend class IComponent;


    template<uint32_t N>
    auto container() -> decltype(IComponent<container_type<N>, N>(this)) {
        return IComponent<container_type<N>, N>(this);
    }

    template<uint32_t N>
    auto find(Entity entity) -> decltype(container<N>().find(entity)) {
        return container<N>().find(entity);
    }

    template<uint32_t N>
    auto find(VMHash hash) -> decltype(container<N>().find(hash)) {
        return container<N>().find(hash);
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
        this function gets a reference to an entity components
    */



};

template<typename... Ts>
const uint32_t State<Ts...>::activeCompFlagIndex = State<Ts...>::dynamic_comp_sequence::find(activeComp);


#endif // ENTITY_HPP
