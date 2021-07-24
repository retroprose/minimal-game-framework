#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP


#include <math.hpp>
#include <container.hpp>


struct Animator {
public:
    uint16_t frame;
    uint16_t count;
}

struct Body {
public:
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
}

struct Player {
public:
    int8_t slot;
    uint16_t delayFire;
    uint16_t damage;
}

struct Enemy {
public:
    //Cp.Handle target;
    int8_t direction;
    uint8_t counter;
    uint16_t delayFire;
}

enum ObjType : uint8_t {
    Player = 0,
    Enemy = 1,
    Bullet = 2,
    BadBullet = 3,
    Boom = 4,
    PlayerBoom = 5,
    ShotCleaner = 6
}


class Cp {
public:

    Vector<uint8_t> _objectId;
    Vector<Body> _body;
    Vector<Player> _player;
    Vector<Enemy> _enemy;
    Vector<Animator> _animator;

};



#endif // COMPONENTS_HPP
