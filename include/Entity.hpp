#ifndef ENTITY_HPP
#define ENTITY_HPP


#include <Container.hpp>


#include <Error.hpp>



struct TSequence {
private:
    template<uint8_t... Ns>
    struct MakeHelper { };

    template<uint8_t H, uint8_t... Ns>
    struct MakeHelper<H, Ns...> {
        static const uint8_t head = H;
        using tail = MakeHelper<Ns...>;
    };

    template <uint8_t APPEND, typename TAIL, uint8_t...Ns>
    struct AppendBack {
        using type = typename AppendBack<APPEND, typename TAIL::tail, Ns..., TAIL::head>::type;
    };

    template <uint8_t APPEND, uint8_t...Ns>
    struct AppendBack<APPEND, MakeHelper<>, Ns...> {
        using type = MakeHelper<Ns..., APPEND>;
    };

    template<bool ADD, uint8_t HEAD, typename S>
    struct IfCond;

    template<uint8_t HEAD, typename S>
    struct IfCond<true, HEAD, S> {
        using type = typename AppendBack<HEAD, S>::type;
    };

    template<uint8_t HEAD, typename S>
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

    template<uint8_t N, uint8_t I = 0, typename OS = MakeHelper<>>
    struct RangeHelper {
        using type = typename RangeHelper<N, I + 1, typename AppendBack<I, OS>::type>::type;
    };

    template <uint8_t N, typename OS>
    struct RangeHelper<N, N, OS> {
        using type = OS;
    };

public:
    template<uint8_t... Ns>
    using Make = MakeHelper<Ns...>;


    template<uint8_t N>
    using Range0 = typename RangeHelper<N>::type;

    template<uint8_t I, uint8_t N>
    using Range = typename RangeHelper<N, I>::type;


    template <template<typename> typename W, typename T, typename IS = typename Range0<std::tuple_size<T>::value>::type, typename OS = MakeHelper<>>
    using Remove = typename RemoveIf<W, T, IS>::type;

    template<typename S, uint8_t V>
    using PushBack = typename AppendBack<V, S>::type;


};



class Signature {
public:

    template<typename S>
    constexpr Signature(S s) : bits( setBits<S>::invoke() ) { }

    constexpr Signature() : bits(0x00000000) { }

    /*
    Signature(const uint8_t* mapper, std::initializer_list<uint8_t> list) {
        bits = 0x00000000;
        const uint8_t* src = list.begin();
        for (;;) {
            if (src == list.end()) break;
            bits |= 0x00000001 << mapper[*src];
            ++src;
        }
    }
    */

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
    template<typename S, bool DUMMY = false>
    struct setBits {
        constexpr static uint32_t invoke() {
            return (0x00000001 << S::head) | setBits<typename S::tail>::invoke();
        }
    };

    template<bool DUMMY>
    struct setBits<TSequence::Make<>, DUMMY> {
        constexpr static uint32_t invoke() { return 0x00000000; }
    };

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
    template<typename... Ts>
    friend class State;

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


class CpInterface {
public:
    void move(uint32_t oldHash, uint32_t newHash) { }

private:


};




template<typename... Ts>
class State {
public:
    constexpr static uint16_t EndOfList = 0xffff;

    State() : head(EndOfList), tail(EndOfList) { }

    static bool getBit(uint8_t bits, uint8_t index)     { return bits & (0x01 << index); }
    static void setBit(uint8_t& bits, uint8_t index)    { bits |=  (0x01 << index); }
    static void unsetBit(uint8_t& bits, uint8_t index)  { bits &= ~(0x01 << index); }

    using pack_type = std::tuple<Ts..., VectorMap<uint8_t>, VectorMap<uint16_t>>;
    using pack_value_type = std::tuple<typename Ts::value_type..., uint8_t, uint16_t>;
    //using pack_ref_type = std::tuple<Ref<typename Ts::value_type>...>;

    constexpr static uint8_t entityIndex = std::tuple_size<pack_type>::value - 1;
    constexpr static uint8_t activeIndex = std::tuple_size<pack_type>::value - 2;

    pack_type pack;

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


    Entity Create() {
        Entity::Tracked entity;
        if (head == EndOfList) {
            entity.index = ginfo.size();
            GenerationInfo info;
            info.generation = 1;
            info.unsafe = 0;
            ginfo.push_back(info);
            hashes.push_back( Hash() );
        } else {
            entity.index = head;
            head = hashes[entity.index].index;
            hashes[entity.index] = Hash();
            if (head == EndOfList)
                tail = EndOfList;
        }
        entity.untracked = 0;
        entity.generation = ginfo[entity.index].generation;
        return Entity(entity);
    }

    /*
    void ChangeSignature(Entity entity, SignatureId signature_id) {
        // Entity is untracked or invalid
        ASSERT(entity.Tracked().untracked == 0 && Valid(entity) == true);
        VectorMapHash old_hash = entityManager.GetHash(entity.Tracked().index); // gets the current hash of the entity
        VectorMapHash new_hash;

        if (old_hash.signature == signature_id) return; // don't switch it with itself!

        // If the signature isn't null, and entityData doesn't contain the signature yet,
        // it needs to be initialized.
        if (signature_id != SignatureManager::Null() && entityData.HasSignature(signature_id) == false) {
            const Signature& reg_sig = signatureManager.Get(signature_id);
            entityData.Register(signature_id, 0);
            for (const auto& cid : reg_sig) {
                pack.Any(cid).Register(signature_id, 0);
            }
        }

        // All components that appear in both the old and new signatures needs to be moved.
        const Signature sig = signatureManager.Get(old_hash.signature) + signatureManager.Get(signature_id);
        entityData.Move(old_hash, signature_id);
        for (const auto& cid : sig) {
            pack.Any(cid).Move(old_hash, signature_id);
        }

        // if the new signature isn't null, the entity data pointing back to the
        // fixed entity value needs to be updated.
        new_hash.signature = signature_id;
        //new_hash.index = 0;
        if (new_hash.signature != SignatureManager::Null() ) {
            new_hash.index = entityData.Size(signature_id) - 1;
            *entityData.Get(new_hash) = entity.Tracked().index;
        }

        // So potentially two different entity components will move,
        // the one being changed, and the one that is displacing the
        // current changed entity by swapping their positions.
        Ref<uint16_t> swapped_entity = entityData.Get(old_hash);
        if (swapped_entity.IsNull() == false) {
            entityManager.GetHash(*swapped_entity) = old_hash;
        }
        entityManager.GetHash(entity.Tracked().index) = new_hash;
    }
    */

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
