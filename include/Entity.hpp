#ifndef ENTITY_HPP
#define ENTITY_HPP

#include<typeinfo>
#include<sstream>
#include<fstream>

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


/*
    Little hacky way to emulate fold expressions in C++11
*/
struct cpp17_fold_expand_type {
    template <typename... T>
    cpp17_fold_expand_type(T&&...) { }
};
#define CPP17_FOLD(x)   cpp17_fold_expand_type{ 0, ((x), void(), 0)... }


/*
    used to create static sequences of unsigned integers,
    used for signatures mostly
*/
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
			//uint32_t index = Null;
			//index = (H == key) ? H : index;
			//CPP17_FOLD( index = (Ns == key) ? Ns : index );
			uint32_t index = Null, i = 0;
			for (;;) {
				if (i >= size)			break;
				if (value[i] == key)	index = i;
				++i;
			}
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
    // makes a sequence of Ns...
    template<uint32_t... Ns>
    using Make = MakeHelper<Ns...>;

    // makes a sequence from 0 to N
    template<uint32_t N>
    using Range0 = typename RangeHelper<N>::type;

    // makes a sequence from I to N
    template<uint32_t I, uint32_t N>
    using Range = typename RangeHelper<N, I>::type;

    // removes all values that don't meet condition W
    template <template<typename> typename W, typename T, typename IS = Range0<std::tuple_size<T>::value>, typename OS = Make<>>
    using Remove = typename RemoveIf<W, T, IS>::type;

    // appends a value to a static sequence
    template<typename S, uint32_t V>
    using PushBack = typename AppendBack<V, S>::type;

};

template<uint32_t H, uint32_t... Ns>
constexpr uint32_t TSequence::MakeHelper<H, Ns...>::value[];

// used to trim out values using Remove in TSequence above
template<typename T>    struct needs_move                  { static const bool value = T::needs_move; };
template<typename T>    struct dynamic_comp                { static const bool value = T::dynamic_comp; };


/*
    Class that represents an entity, is simply a 32 bit unsigned integer.
*/
class Entity {
public:
    Entity() { data.raw.value = 0x00000000; }
	Entity(uint32_t in) { data.raw.value = in; }

    bool operator< (const Entity& rhs) const { return data.raw.value <  rhs.data.raw.value; }
    bool operator==(const Entity& rhs) const { return data.raw.value == rhs.data.raw.value; }
    bool operator!=(const Entity& rhs) const { return data.raw.value != rhs.data.raw.value; }

	bool isNull() const {
		return data.raw.value == 0x00000000;
	}

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

	uint8_t* typePtr() {
		uint8_t* ptr = reinterpret_cast<uint8_t*>(&data.raw.value);
		return ptr + 3;
	}

private:
    template<typename... Ts>
    friend class State;

    /*
        Normal entities created with the State manager
    */
    struct Tracked {
        uint32_t index : 16;        // index of entity
        uint32_t generation : 15;   // generation of entity
        uint32_t untracked : 1;     // should always be zero
    };

    /*
        This can be used to make always active entities that don't
        use up any memory.

        For example, if the untracked bit is set, you can use the type
        to store a "TILE_TYPE", and use the user data bits to store
        an x and y value.  This would represent a tile on the map that
        would always be active.  it would be possible to add components to
        tiles if I changed the HashMaps to store entire entity
        values rather than just the index.

        Still experimental.
    */
    struct Untracked {
        uint32_t user : 24;         // user data, can be used any way you want
        uint32_t type : 7;          // type of entity
        uint32_t untracked : 1;     // should always be 1
    };

    struct Raw {
        uint32_t value;
    };

    Entity(Tracked in) { data.tracked = in; }
    Entity(Untracked in) { data.untracked = in; }

    union {
        Tracked tracked;
        Untracked untracked;
        Raw raw;
    } data;

};

/*
    function used to dump out data to a stream
*/
template<typename T>
void __dump_func(std::stringstream& ss, void*) {
	ss << "Data for " << typeid(T).name() << " dumped!" << std::endl;
}

/*
    Interface to VectorMaps
*/
template<typename T>
class IVectorMap {
private:
	Vector<VMHash>& hashData;
	VectorMap<T>& componentData;

public:
    // a function that State uses to make interfaces
	template<uint32_t N, typename S>
	static IVectorMap<T> make(S* s) {
		return IVectorMap<T>(std::get<S::hashComp>(s->data), std::get<N>(s->data));
	}

	IVectorMap(Vector<VMHash>& v, VectorMap<T>& vm) : hashData(v), componentData(vm) { }

	void dumpFunc(std::stringstream& ss, Entity entity) {
		__dump_func<T>(ss, componentData.find(hashData[entity.index()]).getVoid());
	}

	Ref<T> find(Entity entity) {
		return componentData.find(hashData[entity.index()]);
	}

	Ref<T> find(VMHash hash) {
		return componentData.find(hash);
	}

	Ref<T> find(Entity entity, VMHash hash) {
		return componentData.find(hash);
	}

};

/*
    Interface to HashMap
*/
template<typename T>
class IHashMap {
private:
	HashMap<T>& componentData;

public:
	struct DReference {
		uint16_t index;
		Ref<T> ref;
	};

	template<uint32_t N, typename S>
	static IHashMap<T> make(S* s) {
		return IHashMap<T>(std::get<N>(s->data));
	}

	IHashMap(HashMap<T>& vm) : componentData(vm) { }

	void dumpFunc(std::stringstream& ss, Entity entity) {
		__dump_func<T>(ss, componentData.find(entity.index()).getVoid());
	}

	template<typename F>
	void forEach(F func) {
		DReference ref;
		for (auto&& kv : componentData) {
			ref.index = kv.first;
			ref.ref = Ref<T>(&(kv.second));
			func(ref);
		}
	}

	Ref<T> find(Entity entity) {
		return componentData.find(entity.index());
	}

	Ref<T> find(Entity entity, VMHash hash) {
		return componentData.find(entity.index());
	}

};

/*
    interface for EmptyHashMap
*/
class IEmptyHashMap {
public:
	template<uint32_t N, typename S>
	static IEmptyHashMap make(S* s) {
		return IEmptyHashMap();
	}

	IEmptyHashMap()  { }
};


/*
    Bit of a hacky way to create a stack object that can be any type of interface
*/
class Any {
private:
    constexpr static const size_t MaxBuffer = 16;

    uint8_t buffer_[MaxBuffer];

    template<bool C, typename T = void>
    struct enable_if {
        typedef T type;
    };

    template<typename T>
    struct enable_if<false, T> { };

    struct Base {
		virtual void dumpFunc(std::stringstream& ss, Entity entity) = 0;

    };

	//
	//  Print function!
	// can specialize this if you want!
	//
	//  template<typename _1> static typename enable_if<Print<_1>::value, RETURN_TYPE (void)>::type invoke(_1& t) {
	//
	template<typename T>
	struct DumpFunc {
		template <typename U, U> struct type_check;
		template <typename _1> static uint16_t &chk(type_check<void (_1::*)(std::stringstream&, Entity), &_1::dumpFunc> *);
		template <typename   > static  uint8_t &chk(...);
		static bool const value = sizeof(chk<T>(0)) == sizeof(uint16_t);
		template<typename _1> static typename enable_if<!DumpFunc<_1>::value>::type invoke(_1& t, std::stringstream& ss, Entity entity) { ss << "DEFAULT\n"; }
		template<typename _1> static typename enable_if<DumpFunc<_1>::value>::type invoke(_1& t, std::stringstream& ss, Entity entity) { t.dumpFunc(ss, entity); }
	};

    template<typename T>
    struct Derived : public Base {
        T t_;
        Derived(const T& t) : t_(t) { }
		void dumpFunc(std::stringstream& ss, Entity entity) { DumpFunc<T>::invoke(t_, ss, entity); }
    };

    Base* base() { return reinterpret_cast<Base*>(buffer_); }

public:
    Any() {
        assert(false);
    }

    template<typename T>
    Any(const T& t) {
        assert(sizeof(Derived<T>) <= MaxBuffer);
        Base* d = new (buffer_) Derived<T>(t);
        assert(d - base() == 0);
    }

    Base* operator->() { return reinterpret_cast<Base*>(buffer_); }

};


/*
    bit flags that represent present components
*/
class Signature {
public:
	Signature(uint32_t v) : value_(v) { }

	bool contains(const Signature& rhs) const {
		return (value_ & rhs.value_) == rhs.value_;
	}

	uint32_t value() const {
		return value_;
	}

private:
	uint32_t value_;

};




template<typename... Ts>
class State {
public:
    // used to signify ends of linked lists.
    constexpr static uint16_t EndOfList = 0xffff;

    // stores generation count of entities
    struct GenerationInfo {
        GenerationInfo() : generation(0), unsafe(0) { }
        GenerationInfo(uint16_t g) : generation(g), unsafe(0) { }
        uint16_t generation : 15;
        uint16_t unsafe : 1;
    };

    // stores head of each signature list of components
    struct FreeInfo {
        FreeInfo() : head(EndOfList), size(1) { }
        uint16_t head;
        uint16_t size;
    };

    // stores linked list of free entity indicies
    struct FreeComp {
        FreeComp() : head(EndOfList), tail(EndOfList) { }
        uint16_t head;
        uint16_t tail;
    };

    /*
        This is the main state tuple.  It is the only non-static
        member of this class.  The internal use components
        are appended to the user components.
    */
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
    constexpr static size_t pack_size = std::tuple_size<data_type>::value; // size of tuple

    // types of containers
    template<uint32_t N>
    using container_type = typename std::tuple_element<N, data_type>::type;

    // types of components
    template<uint32_t N>
    using component_type = typename container_type<N>::value_type;

    // types of interfaces
    template<uint32_t N>
    using interface_type = typename container_type<N>::cinterface;

    // all sequence is a sequence of all components
    // internal use component ids are computed here so they can be used in algorithms
    using all_sequence = TSequence::Range0<pack_size - 6>;
    constexpr static uint32_t activeComp = pack_size - 7;
    constexpr static uint32_t hashComp = pack_size - 6;
    constexpr static uint32_t generationComp = pack_size - 5;
    constexpr static uint32_t dynamicComp = pack_size - 4;
    constexpr static uint32_t entityComp = pack_size - 3;
    constexpr static uint32_t freeHashComp = pack_size - 2;
    constexpr static uint32_t freeEntityComp = pack_size - 1;

    // sequence of all vector maps that need to move static components
    using vector_map_sequence = TSequence::Remove<needs_move, data_type, all_sequence>;
    // sequence of all dynamically added components
    using dynamic_comp_sequence = TSequence::Remove<dynamic_comp, data_type, all_sequence>;

    // caches the active flag for quick checking.
    static const uint32_t activeCompFlagIndex;
    static bool is_active(uint8_t f) {
        return f & (0x01 << activeCompFlagIndex);
    }

    // tuple of references
    template<uint32_t... Ns>
    using ref_type = std::tuple<Ref<component_type<Ns>>...>;

    // tuple of iterators for for each loop
	template<typename S>
	struct iter_type;

	template<uint32_t... Ns>
	struct iter_type<TSequence::Make<Ns...>> {
		std::tuple<typename std::vector<component_type<Ns>>::iterator...> pack;
	};

	// used for getting references with the 'reference' function
	template<uint32_t... Ns>
	struct Reference {
		Entity entity;
		VMHash hash;
		ref_type<Ns...> pack;
	};

	// used in static for each loops
	template<typename S>
	struct SRef;

    template<uint32_t... Ns>
    struct SRef<TSequence::Make<Ns...>> {
        VMHash hash;
        ref_type<Ns...> pack;
    };

    // used in dynamic for each loops
	template<uint32_t N>
	using DRef = typename IHashMap<component_type<N>>::DReference;

    // the only non-static member that is the component data
    data_type data;


    /*
        These two functions once implemented can be used to
        help startup performance, for now it just allocates memory
        using the standard library vector and map rules
    */
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

    /*
        Create entity function reserves an entity value.
        Entities start with no static or dynamic components
        present.
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

    /*
        Returns valid if the entity is untracked or generations line up.
    */
    bool valid(Entity entity) const {
        if (
            entity.untracked() == 1 ||
            std::get<State::generationComp>(data)[entity.index()].generation == entity.generation()
        ) {
            return true;
        }
        return false;
    }

    /*
        genSignature generates a signature from
        a sequence.  Flags don't map directly
        to component ids, so that static and dynamic
        component flags can be separate for easy
        organization.
    */
	template<typename S>
	struct SetFlagsImpl;
	template<uint32_t... Ns>
	struct SetFlagsImpl<TSequence::Make<Ns...>> {
		static void invoke(uint16_t& fs) {
			CPP17_FOLD(fs |= (0x0001 << TSequence::Make<Ns...>::find(Ns)));
		}
	};

	template<uint32_t... Ns>
	static Signature genSignature() {
		using local_all = TSequence::Make<Ns...>;
		using local_vector_map = TSequence::Remove<needs_move, data_type, local_all>;
		using local_dynamic_comp = TSequence::Remove<dynamic_comp, data_type, local_all>;

		uint16_t s = 0x0000;
		uint16_t d = 0x0000;

		SetFlagsImpl<local_vector_map>::invoke(s);
		SetFlagsImpl<local_dynamic_comp>::invoke(d);

		return Signature( (d << 16) | s );
	}

	/*
        If the entity has any static components,
        dynamic signatures are stored in a internal
        static component, if the entity has no
        static components, it is simply sored in
        the entitie's hash index (which is unused
        if there are no static components)
    */
	Signature signature(Entity entity) {
		assert(valid(entity));
        uint32_t r = 0x00000000;
        auto& hashData = std::get<State::hashComp>(data);
        VMHash hash = hashData[entity.index()];
        uint16_t s = hash.signature;
        uint16_t d = hash.index;

        if (hash.signature != 0) {
            auto& dynamicData = std::get<State::dynamicComp>(data);
            d = dynamicData[hash];
        }

		return Signature( (d << 16) | s );
    }

    /*
        Next 4 functions map from entities to hashes and hashes to entities.
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
		assert(valid(entity));
        return hashFromIndex(entity.index());
    }

    /*
        This function sets the static signature for an entity.
        You can't add or remove, you just have to change
        the signature to the new one.

        changing the static signature of an entity
        automatically deactivates it to avoid double updates
        during a for each loop.

        This means the entity needs to be reactivated
        after changing static signatures.
    */
    template<uint32_t... Ns>
    void change(Entity entity) {
		assert(valid(entity));

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

    /*
        add and remove will add the given components to
        whatever components are already there
        (as opposed to the static component interface)
    */
    template<uint32_t... Ns>
    void add(Entity entity) {
		assert(valid(entity));

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
		assert(valid(entity));

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

    /*
        functions to check for active and
        inactive entities.  Simply adds
        an empty component 'Active'

        created entities and entities getting
        their static signatures changed should
        be stored in an activate list
        and activated at the end of a game loop.
    */
    void setActive(Entity entity) {
		assert(valid(entity));
        add<activeComp>(entity);
    }

    void setInactive(Entity entity) {
		assert(valid(entity));
        remove<activeComp>(entity);
    }

    bool isActive(Entity entity) {
		assert(valid(entity));
        auto& hashData = std::get<State::hashComp>(data);
        VMHash hash = hashData[entity.index()];
        uint16_t d = hash.index;

        if (hash.signature != 0) {
            auto& dynamicData = std::get<State::dynamicComp>(data);
            d = dynamicData[hash];
        }

        return is_active(d);
    }

    /*
        removes all dynamic components
    */
    template<typename T>
    struct removeAll;
    template<uint32_t... Ns>
    struct removeAll<TSequence::Make<Ns...>> {
        static void invoke(State* s, Entity entity) {
            s->remove<Ns...>(entity);
        }
    };

    /*
        destroys and entity, by changing the static signature to null,
        and removing all dynamic components.
    */
    void destroy(Entity entity) {
		assert(valid(entity));
		assert(entity.untracked() == 0);
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
            //assert(false);
			generationData[entity.index()].unsafe = 0;
			generationData[entity.index()].generation = 1;
			hashData[entity.index()] = VMHash(0, EndOfList);
			if (freeData.head == EndOfList) {
				freeData.head = entity.index();
			} else {
				hashData[freeData.tail] = VMHash(0, entity.index());
			}
			freeData.tail = entity.index();
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

    /*
        For each loop
    */
	template<typename S, typename F, typename R>
	struct forEachImpl;

	template<typename S, typename F, uint32_t... Ns>
    struct forEachImpl<S, F, TSequence::Make<Ns...>> {
		static void invoke(State* s, F func) {
			auto& dynamicData = std::get<State::dynamicComp>(s->data);
			iter_type<S> it;
			SRef<S> ref;
			uint16_t signature = 0x0000;
			CPP17_FOLD(signature |= (0x0001 << vector_map_sequence::find(S::value[Ns])));
			for (auto&& kv : dynamicData) {
				ref.hash.signature = kv.first;
				if ((ref.hash.signature & signature) == signature) {
					auto dynamicIt = dynamicData.begin(kv.first);
					CPP17_FOLD(std::get<Ns>(it.pack) = std::get<S::value[Ns]>(s->data).begin(kv.first));
					for (ref.hash.index = 0; ref.hash.index < kv.second.size(); ++ref.hash.index) {
						if (is_active(*dynamicIt) == true) {
							CPP17_FOLD(std::get<Ns>(ref.pack) = Ref<component_type<S::value[Ns]>>(&(*(std::get<Ns>(it.pack)))));
							func(ref);
						}
						++dynamicIt;
						CPP17_FOLD(++(std::get<Ns>(it.pack)));
					}
				}
			}
		}
	};

	template<typename S, typename F>
	void forEach(F func) {
		forEachImpl<S, F, TSequence::Range0<S::size>>::invoke(this, func);
	}

    /*
        This is how components are operated on from the outside
    */
	template<uint32_t N>
	interface_type<N> container() {
		return interface_type<N>::template make<N>(this);
	}

	/*
        helper find functions to save some typing
	*/
    template<uint32_t N>
    auto find(Entity entity) -> decltype(container<N>().find(entity)) {
		assert(valid(entity));
		return container<N>().find(entity);
    }

    template<uint32_t N>
	auto find(VMHash hash) -> decltype(container<N>().find(hash)) {
		return container<N>().find(hash);
	}

	// just a fast way to use 'auto' instead of writing out Ref<ComponentType>
	template<uint32_t N>
	auto reftype() -> decltype(Ref<component_type<N>>()) {
		return Ref<component_type<N>>();
	}


	/*
        These functions can access components dynamically with the 'Any' class
	*/
	template<uint32_t N>
	static Any any_helper(State* s) {
		return Any(s->container<N>());
	}
	typedef Any (*any_func_ptr)(State*);

	template<typename S>
	struct any_helper_struct;

	template<uint32_t... Ns>
	struct any_helper_struct<TSequence::Make<Ns...>> {
	    // couldn't get right syntax for this...
		//static const any_func_ptr table[] = { (&any_helper<Ns>)... };
        static any_func_ptr* get_table() {
            static any_func_ptr table[] = { (&any_helper<Ns>)... };
            return table;
        }
	};

	Any any(uint32_t comp) {
		return any_helper_struct<all_sequence>::get_table()[comp](this);
	}

    /*
        Dumps out all components of an entity
    */
	void dumpFunc(std::stringstream& ss, Entity entity) {
		Signature all = signature(entity);

		auto& hashData = std::get<State::hashComp>(data);
		auto& dynamicData = std::get<State::dynamicComp>(data);

		uint16_t s = all.value() & 0x0000ffff;
		uint16_t d = (all.value() & 0xffff0000) >> 16;

		uint32_t i, mask;

		mask = 0x0001;
		for (i = 0; i < vector_map_sequence::size; ++i) {
			if (s & mask) {
				any(vector_map_sequence::value[i])->dumpFunc(ss, entity);
			}
			mask = mask << 1;
		}

		mask = 0x0001;
		for (i = 0; i < dynamic_comp_sequence::size; ++i) {
			if (d & mask) {
				any(dynamic_comp_sequence::value[i])->dumpFunc(ss, entity);
			}
			mask = mask << 1;
		}
	}

    /*
        this function gets a reference to an entity components
    */
    template<typename T, typename S, typename R>
    struct ReferenceImpl;

    template<typename T, typename S, uint32_t... Ns>
    struct ReferenceImpl<T, S, TSequence::Make<Ns...>> {
        static void invoke(State& state, T& t) {
            CPP17_FOLD( std::get<Ns>(t.pack) = state.container<S::value[Ns]>().find(t.entity, t.hash) );
        }
    };

    template<uint32_t... Ns>
    Reference<Ns...> reference(Entity entity) {
		assert(valid(entity));
        Reference<Ns...> ref;
        ref.entity = entity;
        auto& hashData = std::get<State::hashComp>(data);
        ref.hash = hashData[entity.index()];
        ReferenceImpl<Reference<Ns...>, TSequence::Make<Ns...>, TSequence::Range0<sizeof...(Ns)>>::invoke(*this, ref);
        return ref;
    }




};

// don't know what's wrong with this definition
//template<typename... Ts>
//template<typename S>
//const typename State<Ts...>::any_func_ptr State<Ts...>::any_helper_struct<S>::table[];

template<typename... Ts>
const uint32_t State<Ts...>::activeCompFlagIndex = State<Ts...>::dynamic_comp_sequence::find(activeComp);


#endif // ENTITY_HPP
