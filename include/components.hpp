#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP


#include <math.hpp>
#include <container.hpp>




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
    /*void destroyEntity(Cp.Handle a)
    {
        Id = EventType.DestroyEntity;
        A = a;
        B = Cp.Handle.Null;
        Key = 0;
        type = 0;
        v = new Vector2(0, 0);
    }

    public void contact(ushort k, Cp.Handle a, Cp.Handle b)
    {
        Id = EventType.Contact;
        A = a;
        B = b;
        Key = k;
    }

    public void createPlayer(ushort d, Vector2 p)
    {
        Id = EventType.CreateEntity;
        A = Cp.Handle.Null;
        B = Cp.Handle.Null;
        type = (byte)ObjType.Player;
        Key = d;
        v = p;
    }

    public void createEntity(ObjType t, Vector2 p)
    {
        Id = EventType.CreateEntity;
        A = Cp.Handle.Null;
        B = Cp.Handle.Null;
        type = (byte)t;
        v = p;
    }

    public EventType::Tp Id;

    public Cp.Handle A;
    public Cp.Handle B;*/

    uint16_t key;
    uint8_t type;
    Vector2 v;
};


struct Bounds {
public:
    uint8_t type;
    //Cp.Handle entity;
    Vector2 lower;
    Vector2 upper;

    Bounds(uint8_t t, Vector2 p, Vector2 d, Vector2 s) {
        type = t;
        //entity = e;
        lower = p - s;
        upper = p + s;
    }

    bool overlap(const Bounds& b) {
        if (b.lower.x > upper.x || b.upper.x < lower.x ||
            b.lower.y > upper.y || b.upper.y < lower.y) {
            return false;
        }
        return true;
    }
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

/*
enum ObjType : uint8_t {
    Player = 0,
    Enemy = 1,
    Bullet = 2,
    BadBullet = 3,
    Boom = 4,
    PlayerBoom = 5,
    ShotCleaner = 6
};
*/

class Cp {
public:

/*
    enum Ids {
        Component = 0,
        ObjectId = 1,
        Body = 2,
        Animator = 3,
        Player = 4,
        Enemy = 5,
        Active = 6
    };
*/

    void smartCopy(const Cp& other) {
        _manager.smartCopy(other._manager);
        _component.smartCopy(other._component);
        _generation.smartCopy(other._generation);
        _objectId.smartCopy(other._objectId);
        _body.smartCopy(other._body);
        _player.smartCopy(other._player);
        _enemy.smartCopy(other._enemy);
        _animator.smartCopy(other._animator);
    }


    PoolTable _manager;

    Table<uint8_t> _component;
    Table<uint16_t> _generation;

    Table<uint8_t> _objectId;
    Table<Body> _body;
    Table<Player> _player;
    Table<Enemy> _enemy;
    Table<Animator> _animator;

};



#endif // COMPONENTS_HPP
