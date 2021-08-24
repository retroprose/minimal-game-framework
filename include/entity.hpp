#ifndef ENTITY_HPP
#define ENTITY_HPP

#include<typeinfo>
#include<sstream>
#include<fstream>

#include <container.hpp>


template<typename T>
class Flags {
public:
    using value_type = T;

    Flags() { }
    Flags(T v) : _value(v) { }

    bool contains(Flags other) const {
        return (_value & other._value) == other._value;
    }

    bool operator==(Flags other) const {
        return _value == other._value;
    }

    bool operator!=(Flags other) const {
        return _value == other._value;
    }

    Flags inverted() const {
        return ~_value;
    }

    void removeAll() {
        _value = 0x00000000;
    }

    void addAll() {
        _value = 0xffffffff;
    }

    void add(Flags other) {
        _value |= other._value;
    }

    void remove(Flags other) {
        _value &= ~other._value;
    }

    template<typename U>
    U truncate() const {
        return U( (typename U::value_type)_value );
    }

    //Flags operator|(Flags other) const {
    //    return _value | other._value;
    //}

//private:
    T _value;

};
using Flags8 = Flags<uint8_t>;
using Flags16 = Flags<uint16_t>;
using Flags32 = Flags<uint32_t>;



class Entity {
public:
    Entity() { }
    Entity(uint16_t i) : _index(i), _generation(0) { }
    Entity(uint16_t i, uint16_t g) : _index(i), _generation(g) { }

    bool isNull() { return _index == 0 && _generation == 0; }

    uint16_t index() { return _index; }
    uint16_t generation() { return _generation; }

private:
    uint16_t _index;
    uint16_t _generation;
};



class BaseComponents {
public:
//protected:
    BaseComponents() {
        signature(0);
    }

    Flags32 denseFlags;
    Vector<Flags32> signatures;

    FreeList freeList;
    Dense<uint16_t> generations;
    Dense<Flags8> _dynamicComp;

    int32_t signature(Flags32 f) {
        int32_t index = -1;
        bool already_exists = false;
        for (uint32_t i = 0; i < signatures.size(); ++i) {
            if (signatures[i] == f) {
                already_exists = true;
            }
        }
        assert(already_exists == false);

        // can do this from signature!

        index = signatures.size();
        signatures.append(f);
        denseFlags.add(f);
        freeList.resize(index);

        return index;
    }

    Flags32 getSignature(int32_t chunk) {
        return signatures[chunk];
    }

    bool valid(Entity entity) {
        return !entity.isNull() && entity.generation() == generations[entity.index()];
    }

    bool add(Entity entity, Flags32 s) {
        bool b = valid(entity);
        if (b == true) {
            s.remove(denseFlags);
            _dynamicComp[entity.index()].add( s.truncate<Flags8>() );
        }
        return b;
    }

    bool remove(Entity entity, Flags32 s) {
        bool b = valid(entity);
        if (b == true) {
            s.remove(denseFlags);
            _dynamicComp[entity.index()].remove( s.truncate<Flags8>() );
        }
        return b;
    }

    Entity create(int32_t chunk) {
        Entity entity(0);
        uint16_t value = freeList.allocate(chunk);
        if (value != FreeList::EndOfList) {
            generations.insert(value);  // this will be tricky
            _dynamicComp.insert(value);

            _dynamicComp[value].removeAll();
            entity = Entity(value, generations[value]);
        }
        return entity;
    }

    void destroy(Entity entity) {
        if (valid(entity) == true)
        {
            ++generations[entity.index()];
            freeList.free(entity.index());
            _dynamicComp[entity.index()].removeAll();
        }
    }

    bool has(Entity entity, Flags32 s) {
        uint32_t chunk = FreeList::getChunk(entity.index());
        Flags32 es = signatures[chunk];
        es.add( _dynamicComp[entity.index()].truncate<Flags32>() );
        return es.contains(s);
    }

    // do I need this?
    //auto dynamicComp(Entity e) -> decltype( _dynamicComp.begin() ) { return _dynamicComp.find(toIndex(e)); }

    Entity fromIndex(uint16_t i) {
        return Entity(i, generations[i]);
    }

    Entity entityTrick(Entity entity) {
        return Entity(entity.index(), generations[entity.index()]);
    }

    uint16_t toIndex(Entity e) {
        return e.index();
    }

    friend class iterator;

    class iterator {
    public:


std::string getBinary(uint32_t value) {
    std::stringstream ss;
    uint32_t mask = 0x1;
    for (int i = 0; i < 32; i++) {
        if (value & mask) {
            ss << "1";
        } else {
            ss << "0";
        }
        mask = mask << 1;
    }
    return ss.str();
}



    //private:
        // other stuff
        Flags32 _denseFlags;
        Flags8 _sparseFlags;
        uint16_t _index;
        uint16_t _chunk;
        BaseComponents* _cp;

    public:
        iterator(BaseComponents* p, Flags32 f) : _cp(p), _chunk(0), _index(0) {
            _denseFlags = f;
            _denseFlags.remove(_cp->denseFlags.inverted());
            f.remove(_cp->denseFlags);
            _sparseFlags = f.truncate<Flags8>();
            next();
        }

        Entity entity() {
            uint16_t i = FreeList::fromChunkIndex(_chunk, _index);
            return Entity(i, _cp->generations[i]);
        }

        Entity fastEntity() {
            uint16_t i = FreeList::fromChunkIndex(_chunk, _index);
            return Entity(i, 0);
        }

        bool next() {
            // funky
            uint16_t i;
            bool foundMatch = true;
            for (;;) {
                ++_index;
                if (_index >= _cp->_dynamicComp.sizeOfChunk(_chunk)) {
                    for (;;) {
                        if (_chunk < _cp->_dynamicComp.sizeOfPartitions() - 1) {
                            ++_chunk;
                            _index = 0;
                        } else {
                            foundMatch = false;
                            break;
                        }
                        if (_cp->getSignature(_chunk).contains(_denseFlags)) {
                            break;
                        }
                    }
                    if (foundMatch == false) {
                        break;
                    }
                }
                i = FreeList::fromChunkIndex(_chunk, _index);
                if (_cp->_dynamicComp[i].contains(_sparseFlags)) {
                    break;
                }
            }
            return foundMatch;
        }

    };

    iterator filter(Flags32 flags) {
        return iterator(this, flags);
    }

};








#endif // ENTITY_HPP
