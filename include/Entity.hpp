#ifndef ENTITY_HPP
#define ENTITY_HPP


#include <Container.hpp>



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
        static const uint32_t head = H;
        using tail = MakeHelper<Ns...>;
        constexpr static uint32_t value[sizeof...(Ns) + 2] = { H, Ns..., Null };
        static uint32_t find(uint32_t key) {
            uint32_t index = Null;
            index = (H == key) ? H : index;
            CPP17_FOLD( index = (Ns == key) ? Ns : index );
            ASSERT(index != Null);
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



template<typename... Ts>
class State {
public:
    template<typename T> struct needs_move                  { static const bool value = false; };
    template<typename T> struct needs_move<VectorMap<T>>    { static const bool value = true;  };

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

    struct ActiveComp {
        ActiveComp() : bits(0x00) { }
        bool getBit(uint8_t index) const    { return bits & (0x01 << index); }
        void setBit(uint8_t index)          { bits |=  (0x01 << index); }
        void unsetBit(uint8_t index)        { bits &= ~(0x01 << index); }
        void clear()                        { bits = 0x00; }
        uint8_t bits;
    };

    struct FreeComp {
        FreeComp() : head(EndOfList), tail(EndOfList) { }
        uint16_t head;
        uint16_t tail;
    };

    using data_type = std::tuple<
        Ts...,                  // user defined components
        Vector<Hash>,           // hashes -6
        Vector<GenerationInfo>, // generation id -5
        VectorMap<ActiveComp>,  // active bit/dynamic comps -4
        VectorMap<uint16_t>,    // entity -3
        HashMap<FreeInfo>,      // free hashes -2
        Single<FreeComp>        // free entities -1
    >;

    //using value_type = std::tuple<typename Ts::value_type...>;
    //using ref_type = std::tuple<Ref<typename Ts::value_type>...>;

    constexpr static size_t pack_size = std::tuple_size<data_type>::value;

    template<uint32_t N>
    using container_type = typename std::tuple_element<N, data_type>::type;

    template<uint32_t N>
    using component_type = typename std::tuple_element<N, data_type>::type::value_type;

    using vector_map_sequence = TSequence::Remove<needs_move, data_type, TSequence::Range0<pack_size - 6>>;


    constexpr static uint32_t hashComp = pack_size - 6;
    constexpr static uint32_t generationComp = pack_size - 5;
    constexpr static uint32_t activeComp = pack_size - 4;
    constexpr static uint32_t entityComp = pack_size - 3;
    constexpr static uint32_t freeHashComp = pack_size - 2;
    constexpr static uint32_t freeEntityComp = pack_size - 1;


    data_type data;


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


    template<uint32_t... Ns>
    void changeSignature(Entity entity) {
        auto& freeData = std::get<State::freeHashComp>(data);
        auto& hashData = std::get<State::hashComp>(data);
        auto& activeData = std::get<State::activeComp>(data);
        auto& entityData = std::get<State::entityComp>(data);

        Hash::SignatureIndex nh;

        nh.signature = 0x0000;

        // generate signature hash
        CPP17_FOLD( nh.signature |= (0x0001 << vector_map_sequence::find(Ns)) );

        Hash oldHash = hashData[entity.index()];
        auto newInfo = freeData.find(nh.signature);
        if ( newInfo.isNull() ) {
            nh.index = 0;
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

        auto oldInfo = freeData.find(oldHash.signature());
        if ( !oldInfo.isNull() ) {
            activeData[oldHash.raw()].unsetBit(7);
            entityData[oldHash.raw()] = oldInfo->head;
            oldInfo->head = oldHash.index();
        }

        entityData.move( oldHash.raw(), newHash.raw() );
        activeData.move( oldHash.raw(), newHash.raw() );

        // move all components needed to move
        CPP17_FOLD( std::get<Ns>(data).move(oldHash.raw(), newHash.raw()) );

        if (newHash.signature() != 0) {
            activeData[newHash.raw()].unsetBit(7);
            entityData[newHash.raw()] = entity.index();
        }

        hashData[entity.index()] = newHash;
    }

    template<uint32_t... Ns, typename F>
    void ForEach(F func) {
        auto& generationData = std::get<State::generationComp>(data);
        auto& entityData = std::get<State::entityComp>(data);
        Entity::Tracked entity;
        entity.untracked = 0;
        uint16_t sigHash = 0x0000;
        // generate signature hash
        CPP17_FOLD( sigHash |= (0x0001 << vector_map_sequence::find(Ns)) );
        for (auto& kv : entityData) {
            for (auto& index : kv.second) {
                entity.generation = generationData[index].generation;
                entity.index = index;
                func( Entity(entity) );
            }
        }
    }


private:
    template<typename T, uint32_t COMP>
    class IComponent;

    template<typename T, uint32_t COMP>
    class IComponent<VectorMap<T>, COMP> {
    private:
        State& state;

    public:
        IComponent(State* s) : state(*s) { }

        Ref<T> get(Entity entity) {
            auto& componentData = std::get<COMP>(state.data);
            auto& hashData = std::get<State::hashComp>(state.data);
            return componentData.find(hashData[entity.index()].raw());
        }

        //Ref<T> get(Hash hash) {
        //    return std::get<COMP>(data).get(hash);
        //}

    };
    template<typename T, uint32_t COMP>
    friend class IComponent;

public:


    template<uint32_t N>
    auto get(Entity entity) -> decltype(IComponent<container_type<N>, N>(this).get(entity)) {
        return IComponent<container_type<N>, N>(this).get(entity);
    }
/*
    template<uint32_t N>
    auto get(Hash hash) -> decltype(IComponent<component_type<N>, N>(this).get(hash)) {
        return IComponent<component_type<N>, N>(this).get(hash);
    }
*/

//    template<uint32_t N>
//    auto get(Entity entity) -> uint32_t {
//        return N;
//    }


};






#endif // ENTITY_HPP
