#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP


#include <math.hpp>
#include <container.hpp>







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
        Player = 0,
        Enemy = 1,
        Bullet = 2,
        BadBullet = 3,
        Boom = 4,
        PlayerBoom = 5,
        ShotCleaner = 6
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



class Cp : public Components {
public:
    void resize(int s) {
        Components::resize(s);
        _comp.resize(s);
        _objectId.resize(s);
        _body.resize(s);
        _player.resize(s);
        _enemy.resize(s);
        _animator.resize(s);
    }

    void smartCopy(const Cp& other) {
        Components::smartCopy(other);
        _comp.smartCopy(other._comp);
        _objectId.smartCopy(other._objectId);
        _body.smartCopy(other._body);
        _player.smartCopy(other._player);
        _enemy.smartCopy(other._enemy);
        _animator.smartCopy(other._animator);
    }

    void destroy(Entity entity) {
        Components::destroy(entity);
        _comp[entity.index()].flags = 0x0;
    }

    Entity create() {
        Entity entity = Components::create();
        if (!entity.isNull()) {
            _comp.insert(entity.index());
            _objectId.insert(entity.index());
            _body.insert(entity.index());
            _player.insert(entity.index());
            _enemy.insert(entity.index());
            _animator.insert(entity.index());
        }
        return entity;
    }

    Table<Cf> _comp;
    Table<uint8_t> _objectId;
    Table<Body> _body;
    Table<Player> _player;
    Table<Enemy> _enemy;
    Table<Animator> _animator;

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
                if (comp.flags & Cf::Component) c._comp[e.index()] = comp;
                if (comp.flags & Cf::ObjectId) c._objectId[e.index()] = objectId;
                if (comp.flags & Cf::Body) c._body[e.index()] = body;
                if (comp.flags & Cf::Player) c._player[e.index()] = player;
                if (comp.flags & Cf::Enemy) c._enemy[e.index()] = enemy;
                if (comp.flags & Cf::Animator) c._animator[e.index()] = animator;
            } else {
                std::fstream out("events.txt");
                out << " FAIL! ";
                out.close();
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
            entity(i, c._generation[i]),
            comp(c._comp[i]),
            objectId(c._objectId[i]),
            body(c._body[i]),
            player(c._player[i]),
            enemy(c._enemy[i]),
            animator(c._animator[i])
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
                if (index >= cp._comp.size()) {
                    break;
                }
                if ((cp._comp[index].flags & mask) == mask) {
                    break;
                }
            }
            return index < cp._comp.size();
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
