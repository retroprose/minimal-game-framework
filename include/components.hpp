#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP


#include <math.hpp>
#include <container.hpp>





/*
    Class that represents an entity, is simply a 32 bit unsigned integer.
*/
class Entity {
public:
    Entity() { }
    Entity(uint16_t i) : _index(0), _generation(0) { }
    Entity(uint16_t i, uint16_t g) : _index(i), _generation(g) { }

    bool isNull() { return _index == 0 && _generation == 0; }

    uint16_t index() { return _index; }
    uint16_t generation() { return _generation; }

private:
    uint16_t _index;
    uint16_t _generation;
};






struct Animator {
public:
    uint16_t frame;
    uint16_t count;
};

struct Body {
public:
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
};

struct Player {
public:
    int8_t slot;
    uint16_t delayFire;
    uint16_t damage;
};

struct Enemy {
public:
    //Cp.Handle target;
    int8_t direction;
    uint8_t counter;
    uint16_t delayFire;
};

struct ObjType {
public:
    enum Type : uint8_t {
        Null = 0,
        Player = 1,
        Enemy = 2,
        Bullet = 3,
        BadBullet = 4,
        Boom = 5,
        PlayerBoom = 6,
        ShotCleaner = 7
    };
};

struct Cf {
public:
    //Cf() : flags(0x0) { }

    enum Type : uint8_t {
        Component = 1 << 0,
        ObjectId = 1 << 1,
        Body = 1 << 2,
        Player = 1 << 3,
        Enemy = 1 << 4,
        Animator = 1 << 5,
        Active = 1 << 6
    };

    uint8_t flags;
};





struct Control {
public:
    // 3 bits state - 0 is unready, 1+ difficulty, 7 synced
    // non empty 1 bit
    // left, right, primary 3 bits
    uint8_t state;

    int16_t x;

    // 10 bits, 6 left over
    bool nonEmpty;

    bool left;
    bool right;

    bool primary;

    uint32_t debug;
};

struct Slot {
public:
    bool connected;
    //bool Broken;
    Control input;
};

struct GlobalState {
public:
    bool playing;

    Scaler enemySpeed;
    int32_t enemyCount;

    uint16_t textType;
    Scaler textAnimate;
};

struct EventType {
    enum Tp {
        Null = 0,
        DestroyEntity,
        CreateEntity,
        Contact,
        Shoot,
        Count
    };
};

struct Event {
public:
    void destroyEntity(Entity a) {
        id = EventType::DestroyEntity;
        A = a;
        B = Entity();
        key = 0;
        type = 0;
        v = Vector2(0, 0);
    }

    void contact(uint16_t k, Entity a, Entity b) {
        id = EventType::Contact;
        A = a;
        B = b;
        key = k;
    }

    void createPlayer(uint16_t d, Vector2 p) {
        id = EventType::CreateEntity;
        A = Entity();
        B = Entity();
        type = ObjType::Player;
        key = d;
        v = p;
    }

    void createEntity(ObjType::Type t, Vector2 p) {
        id = EventType::CreateEntity;
        A = Entity();
        B = Entity();
        type = t;
        v = p;
    }

    EventType::Tp id;

    Entity A;
    Entity B;

    uint16_t key;
    uint8_t type;
    Vector2 v;
};


struct Bounds {
public:
    uint8_t type;
    Entity entity;
    Vector2 lower;
    Vector2 upper;

    Bounds() { }
    Bounds(Entity e, uint8_t t, Vector2 p, Vector2 d, Vector2 s) {
        type = t;
        entity = e;
        lower = p - s;
        upper = p + s;
    }

    bool operator <(const Bounds& other) {
        return lower.x < other.lower.x;
    }

    bool overlap(const Bounds& b) {
        if (b.lower.x > upper.x || b.upper.x < lower.x ||
            b.lower.y > upper.y || b.upper.y < lower.y) {
            return false;
        }
        return true;
    }
};



class Cp {
public:
     void resize(int s) {
        manager.resize(s);
        generation.resize(s);
        comp.resize(s);
        objectId.resize(s);
        body.resize(s);
        player.resize(s);
        enemy.resize(s);
        animator.resize(s);
    }

    void smartCopy(const Cp& other) {
        manager.smartCopy(other.manager);
        generation.smartCopy(other.generation);
        comp.smartCopy(other.comp);
        objectId.smartCopy(other.objectId);
        body.smartCopy(other.body);
        player.smartCopy(other.player);
        enemy.smartCopy(other.enemy);
        animator.smartCopy(other.animator);
    }

    bool valid(Entity entity) {
        return !entity.isNull() && entity.generation() == generation[entity.index()];
    }

    void destroy(Entity entity) {
        if (valid(entity) == true) {
            ++generation[entity.index()];
            comp[entity.index()].flags = 0x0;
            manager.free( entity.index() );
        }
    }

    Entity create() {
        Entity entity(0);
        uint16_t value = manager.allocate();
        if (value != PoolTable::EndOfList) {
            generation.insert(value);  // this will be tricky
            comp.insert(value);
            objectId.insert(value);
            body.insert(value);
            player.insert(value);
            enemy.insert(value);
            animator.insert(value);

            comp[entity.index()].flags = 0x0;
            entity = Entity(value, generation[value]);
        }
        return entity;
    }

    void clear() {
        for (int32_t i = 0; i < generation.capacity(); ++i) {
            generation[i] = 0;
        }
        resize(0);
        manager.reset();
    }





    PoolTable manager;

    Table<uint16_t> generation;
    Table<Cf> comp;
    Table<uint8_t> objectId;
    Table<Body> body;
    Table<Player> player;
    Table<Enemy> enemy;
    Table<Animator> animator;

    struct Prefab {
    public:
        Cf comp;
        uint8_t objectId;
        Body body;
        Player player;
        Enemy enemy;
        Animator animator;

        void setEntity(Cp& c, Entity e) {
            if (c.valid(e) == true) {
                if (comp.flags & Cf::Component) c.comp[e.index()] = comp;
                if (comp.flags & Cf::ObjectId) c.objectId[e.index()] = objectId;
                if (comp.flags & Cf::Body) c.body[e.index()] = body;
                if (comp.flags & Cf::Player) c.player[e.index()] = player;
                if (comp.flags & Cf::Enemy) c.enemy[e.index()] = enemy;
                if (comp.flags & Cf::Animator) c.animator[e.index()] = animator;
            }
        }
    };


    // this has to iterate, can change, and outputs the other type!
    struct HelperRef {
    public:
        Entity entity;

        Cf& comp;
        uint8_t& objectId;
        Body& body;
        Player& player;
        Enemy& enemy;
        Animator& animator;

        HelperRef(Cp& c, uint16_t i) :
            entity(i, c.generation[i]),
            comp(c.comp[i]),
            objectId(c.objectId[i]),
            body(c.body[i]),
            player(c.player[i]),
            enemy(c.enemy[i]),
            animator(c.animator[i])
        { }
    };

    struct Iterator {
    public:
        HelperRef get() {
            return HelperRef(cp, index);
        }

        bool moveNext() {
            for (;;) {
                index++;
                if (index >= cp.comp.size()) {
                    break;
                }
                if ((cp.comp[index].flags & mask) == mask) {
                    break;
                }
            }
            return index < cp.comp.size();
        }

        Iterator(Cp& c, uint8_t m, int32_t i) : cp(c), mask(m), index(i) { }
        Cp& cp;
        uint8_t mask;
        int32_t index;
    };

    HelperRef get(uint16_t i) { return HelperRef(*this, i); }
    HelperRef get(Entity e) { return HelperRef(*this, e.index()); }

    Iterator filter(uint8_t mask) { return Iterator(*this, mask, -1); }

};



#endif // COMPONENTS_HPP
