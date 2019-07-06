

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Math.hpp>
#include <Entity.hpp>

#include<fstream>




struct Body {
	Vector2 position;
	Vector2 velocity;
	Real radius;
};


struct Color {
    Color() { }
    Color(Real r_, Real g_, Real b_, Real a_) : r(r_), g(g_), b(b_), a(a_) { }
    Color(Real r_, Real g_, Real b_) : r(r_), g(g_), b(b_), a(1.0f) { }
    Real r;
    Real g;
    Real b;
    Real a;
};


struct Controller {
    Vector2 cursor;
	Vector2 move;
};

/*

    struct Cp {
        enum {
            Body = 0,
            Color,
            Controller,
            HatColor
        };
    };

    State<
        2,
        VectorMap<Body>,
        VectorMap<Color>,
        EntityMap<Controller>,
        EntityMap<Color>
    > state;

*/


/*
struct Cp {

    enum etype : uint8_t {
        Controller = 0,
        Body,
        Color,
        HatColor,
        _ComponentCount
    };


    using type_table = std::tuple<
        ::Controller,
        ::Body,
        ::Color,
        ::Color
    >;


    struct EntityRef : public BaseEntityRef {
        Ref<::Controller> controller;
        Ref<::Body> body;
        Ref<::Color> color;
        Ref<::Color> hatColor;
    };


    VectorMap<::Controller> controllerData;
    VectorMap<::Body> bodyData;
    VectorMap<::Color> colorData;
    VectorMap<::Color> hatColorData;


    AnyVectorRef Any(uint32_t component_id) {
        switch(component_id) {
        case Controller:        return AnyVectorRef(controllerData);
        case Body:              return AnyVectorRef(bodyData);
        case Color:             return AnyVectorRef(colorData);
        case HatColor:          return AnyVectorRef(hatColorData);
        default:                return AnyVectorRef();
        }
    }
};
*/



class State : public BaseState {
public:
    enum StaticComp: uint8_t {
        Body = 0,
        Color
        //Controller = 16,
        //HatColor
    };

     enum DynamicComp: uint8_t {
        Controller = 0,
        HatColor
    };

    struct EntityRef {
        Entity entity;
        EntityHash hash;
        Ref<::Body> body;
        Ref<::Color> color;
        Ref<::Controller> controller;
        Ref<::Color> hatColor;
    };

    AnyComponentMap any(uint8_t cp) {
        switch(cp) {
        //case Body:          return AnyComponentMap(bodyData);
        //case Color:         return AnyComponentMap(colorData);
        //case Controller:    return AnyComponentMap(controllerData);
        //case HatColor:      return AnyComponentMap(hatColorData);
        default:            return AnyComponentMap();
        }
    }

    void moveComponents(EntityHash oldHash, EntityHash newHash) {
        for (auto& cp : ComponentList(newHash.signature)) {
            switch(cp) {
            case Body:       bodyData.move(oldHash, newHash);   break;
            case Color:     colorData.move(oldHash, newHash);   break;
            default:                           ASSERT(false);   break;
            }
        }
    }

    //void reserveSignature(Signature signature, uint32_t count = 0) {
    //    BaseState::reserveSignature(signature, count);
    //    moveComponents( EntityHash(), EntityHash(signature, count) );
    //}


    void changeSignature(Entity entity, Signature signature) {
        EntityHash oldHash = getHash(entity);
        if (oldHash.signature == signature) return;
        EntityHash newHash = BaseState::changeSignature(entity, oldHash, signature);
        moveComponents(oldHash, newHash);
    }

    void destroy(Entity entity) {
        changeSignature(entity, Signature::Null());
        removeSignature(entity, 0xffff);
        BaseState::destroy(entity);
    }

    void addSignature(Entity entity, Signature signature) {
        dynamicSignature[entity.index()] -= signature;
        for (auto& cp : ComponentList(signature)) {
            switch(cp) {
            case Controller:    controllerData.add(entity); break;
            case HatColor:       hatColorData.add(entity);  break;
            default:                        ASSERT(false);  break;
            }
        }
    }

    void removeSignature(Entity entity, Signature signature) {
        dynamicSignature[entity.index()] -= signature;
        for (auto& cp : ComponentList(signature)) {
            switch(cp) {
            case Controller:    controllerData.remove(entity); break;
            case HatColor:      hatColorData.remove(entity);   break;
            default:                          break;
            }
        }
    }

    Ref<::Body> body(Entity entity)             { return bodyData.get( getHash(entity) ); }
    Ref<::Color> color(Entity entity)           { return colorData.get( getHash(entity) ); }

    Ref<::Body> body(EntityHash hash)           { return bodyData.get(hash); }
    Ref<::Color> color(EntityHash hash)         { return colorData.get(hash); }

    Ref<::Controller> controller(Entity entity) { return controllerData.get(entity); }
    Ref<::Color> hatColor(Entity entity)        { return hatColorData.get(entity); }

    EntityMap<::Controller>& controller()       { return controllerData; }
    EntityMap<::Color>& hatColor()              { return hatColorData; }

private:
    VectorMap<::Body> bodyData;
    VectorMap<::Color> colorData;

    EntityMap<::Controller> controllerData;
    EntityMap<::Color> hatColorData;

};


/*
class Signature {
public:
    constexpr static Signature Null() { return Signature(); }

    constexpr Signature() : value_(0) { }
    Signature(uint16_t value) : value_(value) { }

    Signature(std::initializer_list<uint8_t> list) : value_(0) {
        for (auto& cp : list)
            value_ |= 0x0001 << cp;
    }

    bool operator< (const Signature& rhs) const { return value_ <  rhs.value_; }
    bool operator==(const Signature& rhs) const { return value_ == rhs.value_; }
    bool operator!=(const Signature& rhs) const { return value_ != rhs.value_; }

    Signature operator+(const Signature& rhs) const { return value_ | rhs.value_; }
    Signature operator-(const Signature& rhs) const { return value_ & ~rhs.value_; }

    Signature& operator+=(const Signature& rhs) {
        value_ |= rhs.value_;
        return *this;
    }

    Signature& operator-=(const Signature& rhs) {
        value_ &= ~rhs.value_;
        return *this;
    }

    bool has(uint8_t component) const {
        uint16_t mask = 0x0001 << component;
        return ((value_ & mask) == mask);
    }

    bool contains(const Signature& rhs) const {
        return ((value_ & rhs.value_) == rhs.value_);
    }

    const uint16_t& value() const { return value_; }

private:
    uint16_t value_;

};


struct EntityHash {
    EntityHash() : signature(), index(0) { }
    EntityHash(Signature s, uint16_t i) : signature(s), index(i) { }
    Signature signature;
    uint16_t index;
};


template<typename T>
class VectorMap {
public:
    using pair_type = std::pair<Signature, std::vector<T>>;

    using table_type = std::map<Signature, std::vector<T>>;

    size_t size(Signature signature) const {
        size_t s = 0;
        typename table_type::const_iterator it = table.find(signature);
        if (it != table.end()) {
            s = it->second.size();
        }
        return s;
    }

    bool hasHash(EntityHash hash) const {
        bool has = true;
        typename table_type::const_iterator it = table.find(hash.signature);
        if (it == table.end() || hash.index >= it->second.size())
            has = false;
        return has;
    }



    typename table_type::iterator begin() {
        return table.begin();
    }

    typename table_type::iterator end() {
        return table.end();
    }

    void move(EntityHash oldHash, EntityHash newHash) {
        typename table_type::iterator newSig = table.find(newHash.signature);
        if (newSig == table.end()) {
            if (newHash.signature == Signature::Null()) return;
            auto ret = table.insert(pair_type(newHash.signature, std::vector<T>()));
            newSig = ret.first;
        }

        if (newSig->second.size() <= newHash.index) {
            newSig->second.resize(newHash.index + 1);
        }

        typename table_type::iterator oldSig = table.find(oldHash.signature);

        if (oldSig != table.end()) {
            std::swap(oldSig->second[oldHash.index], newSig->second[newHash.index]);
        }
    }

    void add_value(Signature k, T t) {
        table[k].push_back(t);
    }

    void add_map(Signature k) {
        table[k] = std::vector<T>();
    }

    void print_out() {
        std::ofstream os("dump.txt");
        for (auto& kv : table) {
            os << "key: " << kv.first.value() << std::endl;
            for (int i = 0; i < kv.second.size(); ++i) {
                os << "\tarray: " << kv.second[i] << std::endl;
            }
        }
        os.close();
    }


private:
    table_type table;

};

template<typename T>
class ProxyVectorMap {
private:
    VectorMap<T>& table;
    Signature signature_;

public:
    ProxyVectorMap(VectorMap<T>& t, Signature signature) : table(t), signature_(signature) { }

    struct return_stuff {
        return_stuff(const Signature s, T& t) : signature(s), thing(t) { }
        Signature signature;
        T& thing;
    };

    struct iterator {
        typename VectorMap<T>::table_type::iterator tb;
        typename VectorMap<T>::table_type::iterator te;
        typename std::vector<T>::iterator vb;
        typename std::vector<T>::iterator ve;
        Signature signature;

        const Signature& get_sig() {
            return tb->first;
        }

        return_stuff operator*() {
            return return_stuff(tb->first, *vb);
        }

        //T& operator *() {
        //    return *vb;
        //}

        bool operator!=(const iterator& rhs) const {
            return tb != rhs.tb;
        }

         // overloaded prefix ++ operator
        // Define prefix decrement operator.
        iterator& operator++() {
           ++vb;
           if (vb == ve) {
                ++tb;
                // need logic here to skip over signatures
                // while tb doent contain and tb != te
                while ( tb != te && !signature.contains(tb->first) ) {
                    ++tb;
                }
                if (tb == te) {
                    vb = typename std::vector<T>::iterator();
                    ve = typename std::vector<T>::iterator();
                } else {
                    vb = tb->second.begin();
                    ve = tb->second.end();
                }
           }
           return *this;
        }

    };

    iterator begin() {
        iterator it;
        it.signature = signature_;
        it.tb = table.begin();
        it.te = table.end();
        while ( it.tb != it.te && !it.signature.contains(it.tb->first) ) {
            ++(it.tb);
        }
        if (it.tb != it.te) {
            it.vb = it.tb->second.begin();
            it.ve = it.tb->second.end();
        } else {
            it.vb = typename std::vector<T>::iterator();
            it.ve = typename std::vector<T>::iterator();
        }
        return it;
    }

    iterator end() {
        iterator it;
        it.signature = signature_;
        it.tb = table.end();
        it.te = table.end();
        it.vb = typename std::vector<T>::iterator();
        it.ve = typename std::vector<T>::iterator();
        return it;
    }



};

template<typename U>
static ProxyVectorMap<U> Gen(VectorMap<U>& t, const Signature& s) {
    return ProxyVectorMap<U>(t, s);
}


#include<iostream>
*/


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
/*
    uint16_t mask_table[] = {
        0x0ff0,
        0xa0f0,
        0x0f00,
        0xe0f0,
        0x00f0
    };


    uint16_t mask_table[] = {
        0,
        1,
        2,
        3,
        4
    };


    VectorMap<uint32_t> vm;
    for (int j = 0; j < 5; ++j) {
        uint16_t mask = mask_table[j];
        vm.add_map(mask);
        for (int i = 0; i < 10; ++i) {
            vm.add_value(j, i);
        }
    }
    vm.print_out();

    VectorMap<uint32_t>::iterator it = vm.begin();
    while (it != vm.end()) {
        std::cout << it.get_sig().value << ": " << *it << std::endl;
        ++it;
    }


    //for ( auto& value : Gen(vm, 2) ) {
    //    std::cout << value << std::endl;
    //}

    for ( auto value : Gen(vm, 0x00f0) ) {
    //for ( auto value : Gen(vm, 2) ) {
        std::cout << value.signature.value() << ": " << value.thing << std::endl;
    }
*/


    // Define some constants
    const int gameWidth = 800;
    const int gameHeight = 600;

    float cursorRadius = 10.0f;
    float playerRadius = 20.0f;

    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode(gameWidth, gameHeight, 32), "SFML Pong",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    // Load the sounds used in the game
    sf::SoundBuffer ballSoundBuffer;
    if (!ballSoundBuffer.loadFromFile("resources/ball.wav"))
        return EXIT_FAILURE;

    // Create the circle shape that will be used to render everything
    sf::CircleShape circle;
    circle.setOutlineThickness(3);
    circle.setOutlineColor(sf::Color::Black);
    //circle.setFillColor(sf::Color::White);

    // our new vectors
    Vector2 cursorPos, playerPos = Vector2(gameWidth / 2, gameHeight / 2);
    //Vector2 cursorPos;

    // Load the text font
    sf::Font font;
    if (!font.loadFromFile("resources/sansation.ttf"))
        return EXIT_FAILURE;

    // Initialize the info message
    sf::Text message;
    message.setFont(font);
    message.setCharacterSize(40);
    message.setPosition(0.0f, 450.0f);
    message.setFillColor(sf::Color::White);
    message.setString("Use WASD to move, mouse to move cursor.\nLeft and right mouse button adds and\nRemoves player's hat.");



    State::EntityRef ref;

    State state;


    auto playerEntity = state.create();

    state.changeSignature(playerEntity, {State::Color, State::Body});
    state.addSignature(playerEntity, {State::Controller});

    ref.body = state.body(playerEntity);
    ref.color = state.color(playerEntity);

    // notice that references use -> instead of . to access members
    ref.body->position = Vector2(400, 300);    // set to center of screen
    ref.body->velocity = Vector2(0, 0);        // zero velocity
    ref.body->radius = playerRadius;           // radius of player
    *ref.color = Color(1.0f, 1.0f, 1.0f);      // player will be white (notice we dereference with *)

    state.setActive(playerEntity, true);

    for (int i = 0; i < 50; i++) {
        auto e = state.create();

        state.changeSignature(e, {State::Body, State::Color});

        // random chance of having a hat
        int r = rand()%100;
        if (r < 10) {
            state.addSignature( e, {State::HatColor} );
        }

        ref.body = state.body(e);
        ref.color = state.color(e);
        ref.hatColor = state.hatColor(e);

        ref.body->position = Vector2(rand()%800, rand()%600);
        ref.body->velocity = Vector2(rand()%5, rand()%5);
        //eref.body->velocity = Vector2(0, 0);
        ref.body->radius = playerRadius;
        *ref.color = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);

        // some will not have hats, so need to check that the reference isn't null first
        if (ref.hatColor.isNull() == false) {
            *ref.hatColor = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);
        }

        state.setActive(e, true);
    }


    // This will be the main game loop
    while (window.isOpen())
    {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
            {
                window.close();
            }
        }



        state.ForEach({State::Body}, [&](EntityHash hash) {
            ref.entity = state.getEntity(hash);
            if (ref.entity != playerEntity) {
                // every moving entity has a chance of being deleted
                if (rand()%100  == 0) {
                    state.destroy(ref.entity);
                }
            }
        });



        for (int i = 0; i < 1; i++) {
            auto e = state.create();

            state.changeSignature(e, {State::Body, State::Color});

            // random chance of having a hat
            int r = rand()%100;
            if (r < 10) {
                state.addSignature( e, {State::HatColor} );
            }

            ref.body = state.body(e);
            ref.color = state.color(e);
            ref.hatColor = state.hatColor(e);

            ref.body->position = Vector2(rand()%800, rand()%600);
            ref.body->velocity = Vector2(rand()%5, rand()%5);
            //eref.body->velocity = Vector2(0, 0);
            ref.body->radius = playerRadius;
            *ref.color = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);

            // some will not have hats, so need to check that the reference isn't null first
            if (ref.hatColor.isNull() == false) {
                *ref.hatColor = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);
            }

            state.setActive(e, true);
        }



        // convert the sfml vector to our math vector
        cursorPos = Vector2(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);

        // get a movement vector
        Vector2 move(0.0f, 0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))    move.y = -1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))    move.y =  1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))    move.x = -1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))    move.x =  1.0f;

        // this will make the player move a constant speed in all directions, even diagonals
        move.Normalize();



        // check to make sure playerEntity is still valid
        if (state.valid(playerEntity) == true) {

            ref.controller = state.controller(playerEntity);

            // check to make sure the entity does in fact have that component
            if (!ref.controller.isNull()) {

                // set controller data
                ref.controller->cursor = cursorPos;
                ref.controller->move = move;


                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    state.addSignature( playerEntity, {State::HatColor} );
                    // notice how we need to re-get the reference after updating the entity
                    ref.hatColor = state.hatColor(playerEntity);
                    *ref.hatColor = Color(0, 0, 0);
                } else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
                    state.removeSignature( playerEntity, {State::HatColor} );
                }
            }
        }


        for (auto& kv : state.controller()) {
            ref.hash = state.getHashFromIndex(kv.first);
            ref.controller = &kv.second;
            ref.body = state.body(ref.hash);
            ref.body->velocity = ref.controller->move;
        }


        state.ForEach({ State::Body }, [&](EntityHash hash) {
            ref.body = state.body(hash);
            ref.body->position += ref.body->velocity;
        });



        // Rendering code
        // Clear the window
        window.clear(sf::Color(50, 200, 50));




        // This will draw all of the entities.
        state.ForEach( {State::Body, State::Color}, [&](EntityHash hash) {
            ref.body = state.body(hash);
            ref.color = state.color(hash);
            circle.setFillColor(sf::Color(ref.color->r*255, ref.color->g*255, ref.color->b*255));
            circle.setPosition(ref.body->position.x, ref.body->position.y);
            circle.setRadius(ref.body->radius);
            circle.setOrigin(ref.body->radius / 2.0f, ref.body->radius / 2.0f);
            window.draw(circle);
        });


        for (auto& kv : state.hatColor()) {
            ref.hash = state.getHashFromIndex(kv.first);
            ref.hatColor = &kv.second;
            ref.body = state.body(ref.hash);

            circle.setFillColor(sf::Color(ref.hatColor->r*255, ref.hatColor->g*255, ref.hatColor->b*255));
            circle.setPosition(ref.body->position.x, ref.body->position.y);
            circle.setRadius(ref.body->radius / 2);
            // This doesn't make a lot of sense, I would think I would have to set the origin
            // to half of the hat's radius like above, but they are both centered for some
            // reason...  But this is why we are doing this, this is all throw away code
            // since we will use completely different rendering engines later on.
            // The idea here is our entity and game state code stays nice and clean
            // and consistent, then we may do some dirty transformations to get it
            // to map from the input to the output.  For example I like to use
            // positive y axis up, but SFML uses down.  There isn't a trivial way to
            // invert it, will take some dirty transformations possibly.
            circle.setOrigin(0, 0);
            window.draw(circle);
        }


        // draw cursor
        circle.setFillColor(sf::Color::White);
        circle.setPosition(cursorPos.x, cursorPos.y);
        circle.setRadius(cursorRadius);
        circle.setOrigin(cursorRadius / 2, cursorRadius / 2);
        window.draw(circle);

        window.draw(message);

        // Display things on screen
        window.display();
    }

    // exit the app
    return EXIT_SUCCESS;
}
