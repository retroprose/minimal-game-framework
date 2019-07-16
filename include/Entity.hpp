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



template<typename... Ts>
class State {
public:
    template<typename T> struct needs_move                  { static const bool value = false; };
    template<typename T> struct needs_move<VectorMap<T>>    { static const bool value = true;  };

    template<typename T> struct dynamic_comp                { static const bool value = false; };
    template<typename T> struct dynamic_comp<HashMap<T>>    { static const bool value = true;  };

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
    constexpr static size_t pack_size = std::tuple_size<data_type>::value;

    template<uint32_t N>
    using container_type = typename std::tuple_element<N, data_type>::type;

    template<uint32_t N>
    using component_type = typename std::tuple_element<N, data_type>::type::value_type;

    template<uint32_t... Ns>
    using ref_type = std::tuple<Ref<component_type<Ns>>...>;

    template<uint32_t... Ns>
    using iter_type = std::tuple<typename std::vector<component_type<Ns>>::iterator...>;

    using vector_map_sequence = TSequence::Remove<needs_move, data_type, TSequence::Range0<pack_size - 6>>;
    using dynamic_comp_sequence = TSequence::Remove<dynamic_comp, data_type, TSequence::Range0<pack_size - 6>>;

    constexpr static uint32_t hashComp = pack_size - 6;
    constexpr static uint32_t generationComp = pack_size - 5;
    constexpr static uint32_t activeComp = pack_size - 4;
    constexpr static uint32_t entityComp = pack_size - 3;
    constexpr static uint32_t freeHashComp = pack_size - 2;
    constexpr static uint32_t freeEntityComp = pack_size - 1;

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

    uint32_t signature(Entity entity) {
        //auto& activeData = std::get<State::activeComp>(data);
        //auto& hashData = std::get<State::hashComp>(data);
        return 0;
    }
/*
    Entity entityFromHash(Hash hash) {
        auto& generationData = std::get<State::generationComp>(data);
        auto& entityData = std::get<State::entityComp>(data);
        Entity::Tracked entity;
        entity.untracked = 0;
        entity.index = entityData[hash.raw()];
        entity.generation = generationData[hash.raw()].generation;
        return Entity(entity);
    }

    Hash hashFromEntity(Entity entity) {
        auto& hashData = std::get<State::hashComp>(data);
        return hashData[entity.index()];
    }
*/
    void activate(Entity entity) {
        ASSERT(valid(entity));
        auto& hashData = std::get<State::hashComp>(data);
        ASSERT(hashData[entity.index()].signature() != 0);
        auto& activeData = std::get<State::activeComp>(data);
        activeData[hashData[entity.index()].raw()].setBit(7);
    }

    void deactivate(Entity entity) {
        ASSERT(valid(entity));
        auto& hashData = std::get<State::hashComp>(data);
        ASSERT(hashData[entity.index()].signature() != 0);
        auto& activeData = std::get<State::activeComp>(data);
        activeData[hashData[entity.index()].raw()].unsetBit(7);
    }

    bool getActive(Entity entity) {
        ASSERT(valid(entity));
        auto& hashData = std::get<State::hashComp>(data);
        ASSERT(hashData[entity].signature() != 0);
        auto& activeData = std::get<State::activeComp>(data);
        return activeData[hashData[entity.index()]].getBit(7);
    }

    template<uint32_t... Ns>
    void changeSignature(Entity entity) {
        auto& freeData = std::get<State::freeHashComp>(data);
        auto& hashData = std::get<State::hashComp>(data);
        auto& activeData = std::get<State::activeComp>(data);
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

    /*
    void destroy(Entity entity) {
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
*/


private:
    template<typename T, uint32_t COMP>
    class IComponent;

    template<typename T, uint32_t COMP>
    friend class IComponent;

    template<typename T, uint32_t COMP>
    class IComponent<VectorMap<T>, COMP> {
    private:
        State& state;

    public:
        IComponent(State* s) : state(*s) { }

        /*
        void add(Entity entity) {
            auto& componentData = std::get<COMP>(state.data);
            auto& hashData = std::get<State::hashComp>(state.data);
            // update active data
        }

        void remove(Entity entity) {
            auto& componentData = std::get<COMP>(state.data);
            auto& hashData = std::get<State::hashComp>(state.data);
            // update active data
        }
*/

        Ref<T> find(Entity entity) {
            auto& componentData = std::get<COMP>(state.data);
            auto& hashData = std::get<State::hashComp>(state.data);
            return componentData.find(hashData[entity.index()].raw());
        }

        Ref<T> find(Hash hash) {
            return std::get<COMP>(state.data).get(hash);
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
/*
        typename HashMap<T>::table_type::iterator begin() {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.begin();
        }

        typename HashMap<T>::table_type::iterator end() {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.end();
        }
*/
        Ref<T> find(Entity entity) {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.find( entity.index() );
        }

        Ref<T> find(Entity entity, Hash hash) {
            auto& componentData = std::get<COMP>(state.data);
            return componentData.find( entity.index() );
        }

    };

public:

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

    template<uint32_t N>
    auto container() -> decltype(IComponent<container_type<N>, N>(this)) {
        return IComponent<container_type<N>, N>(this);
    }

    template<uint32_t N>
    auto find(Entity entity) -> decltype(IComponent<container_type<N>, N>(this).find(entity)) {
        return IComponent<container_type<N>, N>(this).find(entity);
    }

    /*
        This 'container proxy' is used to iterate though signatures of components
    */
    template<typename T, typename P, typename S, typename R>
    class ContainerProxy;

    template<typename T, typename P, typename S, uint32_t... Ns>
    class ContainerProxy<T, P, S, TSequence::Make<Ns...>> {
    public:
        ContainerProxy(State* s, uint16_t g) : state(s), signature(g) { }

        class Iterator {
        public:
            Iterator(State* s, uint16_t g) : state(s) {
                hash.signature = g;
            }

            bool operator==(const Iterator& rhs) const {
                return table == rhs.table;
            }

            bool operator!=(const Iterator& rhs) const {
                return table != rhs.table;
            }

            T& operator*() {
                CPP17_FOLD( std::get<Ns>(references) = Ref<component_type<S::value[Ns]>>( &(*(std::get<Ns>(pack))) ) );
                //references.hash = Hash(hash.signature, hash.index);
                //references.entity = ?;
                return references;
            }

            Iterator& operator++() {
                for (;;) {
                    ++hash.index;
                    ++active;
                    CPP17_FOLD( ++(std::get<Ns>(pack)) );
                    if (active == active_end) {
                        for (;;) {
                             ++table;
                             if (table == table_end) {
                                return *this;
                             }
                             if ( (table->first & hash.signature) == hash.signature ) {
                                hash.index = 0;
                                active = table->second.begin();
                                active_end = table->second.end();
                                CPP17_FOLD( std::get<Ns>(pack) = std::get<S::value[Ns]>(state->data).begin(table->first) );
                                break;
                             }
                        }
                    }
                    if (active->getBit(7) == true) {
                        return *this;
                    }
                }
            }

        private:
            friend class ContainerProxy;

            using table_iterator = typename container_type<activeComp>::table_type::iterator;
            using vector_iterator = typename std::vector<component_type<activeComp>>::iterator;

            State* state;
            table_iterator table;
            table_iterator table_end;
            vector_iterator active;
            vector_iterator active_end;

            P pack;
            T references;

            Entity::Tracked entity;
            Hash::SignatureIndex hash;
        };

        Iterator begin() {
            auto& activeData = std::get<activeComp>(state->data);
            Iterator it(state, signature);
            it.table_end = activeData.end();
            it.table = activeData.begin();
            while ( (it.table->first & signature) != signature && it.table != it.table_end ) {
                ++it.table;
            }
            if ( it.table != it.table_end ) {
                //it.hash.index = 0;
                it.active = it.table->second.begin();
                it.active_end = it.table->second.end();
                CPP17_FOLD( std::get<Ns>(it.pack) = std::get<S::value[Ns]>(state->data).begin(it.table->first) );
                if ( it.active != it.active_end && it.active->getBit(7) == false ) {
                    ++it;
                }
            }
            return it;
        }

        Iterator end() {
            auto& activeData = std::get<activeComp>(state->data);
            Iterator it(state, signature);
            it.table_end = activeData.end();
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
    using ContainerProxyHelp = ContainerProxy<ref_type<Ns...>, iter_type<Ns...>, TSequence::Make<Ns...>, TSequence::Range0<sizeof...(Ns)>>;

    template<uint32_t... Ns>
    ContainerProxyHelp<Ns...> iterate() {
        // make signature here
        uint16_t signature = 0x0000;
        CPP17_FOLD( signature |= (0x0001 << vector_map_sequence::find(Ns)) );
        return ContainerProxyHelp<Ns...>(this, signature);
    }

};






#endif // ENTITY_HPP
