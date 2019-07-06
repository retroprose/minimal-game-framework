

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Math.hpp>
#include <Entity.hpp>

#include<fstream>


struct Cp {
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


    log2(n & -n) + 1

    0000

    int -> bit    (0x0001 << int)

    0001    1   0   0000

    0010    3   1   0001

    0100    5   2   0010

    1000    7   3   0011


*/


template<uint8_t ... Values>
class bits {
private:
    template<uint8_t C>
    constexpr static uint32_t set() {
        return (0x000000001 << C);
    }

    template<uint8_t C1, uint8_t C2, uint8_t ... Args>
    constexpr static uint32_t set() {
        return (0x000000001 << C1) | set<C2, Args...>();
    }

public:
    constexpr static const uint32_t value = set<Values...>();

};


void print_bits(uint32_t bits) {
    uint32_t mask = 0x80000000;
    for (int i = 0; i < 32; i++) {
        if (mask & bits)
            std::cout << "1";
        else
            std::cout << "0";
        mask = mask >> 1;
    }
    std::cout << std::endl;
}

enum CpType : uint8_t {
    Body = 0,
    Color,
    Controller,
    HatColor
};

using MyState = BaseState<
    VectorMap<Cp::Body>,
    VectorMap<Cp::Color>,
    EntityMap<Cp::Controller>,
    EntityMap<Cp::Color>
>;

template<typename T>
struct test_single_ref {
    T* t;
};

template<typename T, uint8_t ... Args>
struct test_ref {

    using tuple_type = std::tuple<test_single_ref<typename std::tuple_element<Args, typename T::tuple_type>::type>...>;

    test_ref(T& t) {

    }

    template<uint8_t index>
    typename std::tuple_element<index, tuple_type>::type& get() {
        return std::get<index>(refs);
    }

    tuple_type refs;
};


template<uint8_t ... Args, typename T>
test_ref<T, Args...> make_test_ref(T& t) {
    return test_ref<T, Args...>(t);
}

/*



    struct pretty_ref {
        static buffer_[] // zeroed out  memory




    }


    ref.nullify();

    for ( auto std::tie(body, color) : state.foreach<Body, Color>() ) {

    }

    for ( auto tup : state.foreach<Body, Color>() ) {
        std::tie(ref.body, ref.color) = tup;

        // DO STUFF!

    }





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
*/


template<uint32_t index, typename T>
struct print_ref {
    void operator() () {
    //void operator() (tuple<Ts...>& t) {
        std::cout << index << ": " << typeid(typename std::tuple_element<index, typename T::tuple_type>::type).name() << std::endl;
        print_ref<index - 1, T>()();
    }
};

template<typename T>
struct print_ref<0, T> {
    void operator() () {
        std::cout << 0 << ": " << typeid(typename std::tuple_element<0, typename T::tuple_type>::type).name() << std::endl;
    }
};

template<typename T>
void print_ref_() {
    print_ref<std::tuple_size<typename T::tuple_type>::value - 1, T>()();
}


template<typename T>
struct TestRef {
    T* t;
};


template<typename... Ts>
struct TestState {
    using pack_type = std::tuple<Ts...>;
    using tuple_type = std::tuple<typename Ts::value_type...>;

	using tuple_ref_type = std::tuple<TestRef<typename Ts::value_type>...>;

    struct iterator {
        //using tuple_it_type = std::tuple<typename std::tuple_element<Ts, typename T::pack_type>::iterator...>;
		using tuple_it_type = std::tuple<typename Ts::iterator...>;
        tuple_it_type it_pack;

		bool operator!=(const iterator& rhs) const {
			return std::get<0>(it_pack) != std::get<0>(rhs.it_pack);
		}

		tuple_ref_type operator*() {
			tuple_ref_type ref;
			operate<0, false>::setRef(it_pack, ref);
			return ref;
		}

        iterator& operator++() {
			operate<0, false>::increment(it_pack);
            return *this;
        }

		void setBegin(pack_type& pack) {
			operate<0, false>::setBegin(it_pack, pack);
		}

		void setEnd(pack_type& pack) {
			operate<0, false>::setEnd(it_pack, pack);
		}

        template<uint32_t index, bool dummy>
        struct operate {
			static void setRef(tuple_it_type& it_pack, tuple_ref_type& ref) {
				std::get<index>(ref).t = &(*std::get<index>(it_pack));
                operate<index + 1, dummy>::setRef(it_pack, ref);
			}
			static void setBegin(tuple_it_type& s_it_pack, pack_type& pack) {
				std::get<index>(s_it_pack) = std::get<index>(pack).begin();
                operate<index + 1, dummy>::setBegin(s_it_pack, pack);
			}
			static void setEnd(tuple_it_type& s_it_pack, pack_type& pack) {
				std::get<index>(s_it_pack) = std::get<index>(pack).end();
                operate<index + 1, dummy>::setEnd(s_it_pack, pack);
			}
            static void increment(tuple_it_type& s_it_pack) {
				++std::get<index>(s_it_pack);
                operate<index + 1, dummy>::increment(s_it_pack);
            }
        };

		constexpr static const uint32_t it_pack_size = std::tuple_size<tuple_type>::value;

        template<bool dummy>
        struct operate<it_pack_size - 1, dummy> {
			constexpr static const uint32_t index = it_pack_size - 1;
			static void setRef(tuple_it_type& it_pack, tuple_ref_type& ref) {
				std::get<index>(ref).t = &(*std::get<index>(it_pack));
			}
			static void setBegin(tuple_it_type& it_pack, pack_type& pack) {
				std::get<index>(it_pack) = std::get<index>(pack).begin();
			}
			static void setEnd(tuple_it_type& it_pack, pack_type& pack) {
				std::get<index>(it_pack) = std::get<index>(pack).end();
			}
			static void increment (tuple_it_type& it_pack) {
				++std::get<index>(it_pack);
            }
        };

    };

	iterator begin() {
		iterator it;
		it.setBegin(pack);
		return it;
	}

	iterator end() {
		iterator it;
		it.setEnd(pack);
		return it;
	}

    pack_type pack;
};





////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{

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


	TestState<
		std::vector<int>,
		std::vector<float>
	> state;

	auto& iv = std::get<0>(state.pack);
	auto& fv = std::get<1>(state.pack);

	for (int i = 0; i < 10; i++) {
		iv.push_back(i);
		fv.push_back((float)i * 0.01f);
	}

	TestRef<int> ir;
	TestRef<float> fr;

	//for (auto ref : state) {
	//	std::cout << *(std::get<0>(ref).t) << ", ";
	//	std::cout << *(std::get<1>(ref).t) << std::endl;
	//}

    for (auto ref : state) {
        std::tie(ir, fr) = ref;
		std::cout << *(ir.t) << ", ";
		std::cout << *(fr.t) << std::endl;
	}

	//for (std::tie(ir, fr) : state) {
	//	std::cout << *(ir.t) << ", ";
	//	std::cout << *(fr.t) << std::endl;
	//}

    //MyState state;

    //state.print_things();

    //auto test = make_test_ref<Color, HatColor, Body>(state);

    //print_ref_<test_ref<MyState, Color, HatColor, Body>>();


/*
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
*/


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


/*
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
*/


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


/*
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
*/


        // Rendering code
        // Clear the window
        window.clear(sf::Color(50, 200, 50));



/*
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
*/

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
