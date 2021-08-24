#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP


#include <entity.hpp>

#include <math2d/math2d.h>

/*
struct Body { };

struct Render { };

struct Animator { };

struct Monkey { };

struct Path { };


struct Player { };

struct Radar { };


struct Tile { };
*/


struct Body {
    Vector2 position;
    Vector2 velocity;
    Scaler radius;
};


struct Color {
    Color() { }
    Color(Scaler r_, Scaler g_, Scaler b_, Scaler a_) : r(r_), g(g_), b(b_), a(a_) { }
    Color(Scaler r_, Scaler g_, Scaler b_) : r(r_), g(g_), b(b_), a(1.0f) { }
    Scaler r;
    Scaler g;
    Scaler b;
    Scaler a;
};


struct Controller {
    Vector2 cursor;
    Vector2 move;
};


class Cp : public BaseComponents {
public:
//private:
    Dense<Body> _body;
    Dense<Color> _color;

    Sparse<Controller> _controller;
    Sparse<Color> _hatColor;

public:
    enum EnumId : uint8_t {
        // Dynamic components first!
        iActive = 0,
        iHatColor,
        iController,

        // Static components third!
        iBody,
        iColor
    };

    enum EnumFlag : uint32_t {
        // this order doesn't matter due to above enum
        fActive = 1 << iActive,
        fHatColor = 1 << iHatColor,
        fController = 1 << iController,
        fBody = 1 << iBody,
        fColor = 1 << iColor
    };

    int32_t _Body_Color;

    // enter starting dense component!
    Cp() : BaseComponents() {
        // register the dense types here!
        _Body_Color = signature(fBody | fColor);
    }

    Entity create(int32_t c) {
        Entity e = BaseComponents::create(c);
        if (!e.isNull()) {
            Flags32 s = getSignature(c);
            if (s.contains(fBody))  _body.insert(e.index());
            if (s.contains(fColor)) _color.insert(e.index());
        }
        return e;
    }

    void destroy(Entity e) {
        Flags32 f;
        f.addAll();
        remove(e, f);
        BaseComponents::destroy(e);
    }

    // find functions!
    auto body(Entity e) -> decltype( _body.begin() ) { return _body.find(toIndex(e)); }
    auto color(Entity e) -> decltype( _color.begin() ) { return _color.find(toIndex(e)); }

    auto hatColor(Entity e) -> decltype( _hatColor.begin() ) { return _hatColor.find(toIndex(e)); }
    auto controller(Entity e) -> decltype( _controller.begin() ) { return _controller.find(toIndex(e)); }

    // list all of the things!
    void add(Entity e, Flags32 s) {
        if ( BaseComponents::add(e, s) ) {
            if (s.contains(fController))    _controller.insert(e.index());
            if (s.contains(fHatColor))      _hatColor.insert(e.index());
        }
    }

    void remove(Entity e, Flags32 s) {
        if ( BaseComponents::remove(e, s) ) {
            if (s.contains(fController))    _controller.erase(e.index());
            if (s.contains(fHatColor))      _hatColor.erase(e.index());
        }
    }

    template<typename T>
    T get(Entity e) { return T(*this, e); }

};


struct _Body {
    _Body(Cp& c, Entity e) :
        body(*c.body(e))
    { }
    Body& body;
};

struct _Body_Color {
    _Body_Color(Cp& c, Entity e) :
        body(*c.body(e)),
        color(*c.color(e))
    { }
    Body& body;
    Color& color;
};


struct _Body_Controller {
    _Body_Controller(Cp& c, Entity e) :
        body(*c.body(e)),
        controller(*c.controller(e))
    { }
    Body& body;
    Controller& controller;
};



#endif // COMPONENTS_HPP
