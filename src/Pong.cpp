

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Math.hpp>
#include <Entity.hpp>

#include <sstream>
#include <fstream>
#include <iostream>
#include <typeinfo>

/*
    State

*/




// physics component, our first component
struct Body {
	Vector2 position;
	Vector2 displacement;
	Vector2 velocity;

	Real radius;
	Real damping;
	Real invMass;
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

// struct animator
struct Animator {

	void SetAni(uint16_t a) {
		count = 0;
		node = a;
		//color.a = 1.0f;
	}

    Vector2 direction;
	Real z;
	Real zv;
    struct Hash {
        uint16_t signiture;
        uint16_t index;
    };
	Color color;

	uint16_t node;
	uint16_t count;
};

// player component
struct Buttons {
	uint16_t reload : 1;
	uint16_t run : 1;
	uint16_t fire : 1;
	uint16_t grenade : 1;
	uint16_t up : 1;
	uint16_t down : 1;
	uint16_t left : 1;
	uint16_t right : 1;
};

struct Control {
	Vector2 cursor;		// relative to player
	Vector2 move;		// normalized
	Vector2 lastMove;
	Buttons buttons;
};

struct Player {
	Control control;

	int8_t number;

	//Map::Location location;
	int16_t lastSector;

	Real health;
	Real energy;
	int32_t shotgun;
	int32_t grenade;
	int32_t bullet;
	int32_t power;

	Entity healTarget;
	uint8_t healCounter;

	uint8_t delayReload;
	uint8_t delayGrenade;
	uint8_t delayFire;
};

struct Lich {
	Entity circle;
	Entity raise;

	int32_t largestDeadCount;

	int16_t lastSector;

	Real health;

	Vector2 patrol;

	uint8_t raiseCooldown;
};

struct RadarBlip {

	struct Flags {
		uint8_t lastBlip : 1;
		uint8_t blip : 1;
		uint8_t f2 : 1;
		uint8_t f3 : 1;
		uint8_t f4 : 1;
		uint8_t f5 : 1;
		uint8_t f6 : 1;
		uint8_t f7 : 1;
	};

	Flags flags[2];
};




struct Cp {
    enum etype : uint8_t {
        Player = 0,
        RadarBlip,
        Body,
        Animator,
        Body2,
        _ComponentCount
    };

    using type_table = std::tuple<
        ::Player,
        ::RadarBlip,
        ::Body,
        ::Animator,
        ::Body
    >;

    struct EntityRef : public BaseEntityRef {
        Ref<::Player> player;
        Ref<::RadarBlip> radarBlip;
        Ref<::Body> body;
        Ref<::Animator> animator;
        Ref<::Body> body2;
    };

    VectorMap<::Player> playerData;
    VectorMap<::RadarBlip> radarBlipData;
    VectorMap<::Body> bodyData;
    VectorMap<::Animator> animatorData;
    VectorMap<::Body> bodyData2;

    AnyVectorRef Any(uint32_t component_id) {
        switch(component_id) {
        case Player:            return AnyVectorRef(playerData);
        case RadarBlip:         return AnyVectorRef(radarBlipData);
        case Body:              return AnyVectorRef(bodyData);
        case Body2:             return AnyVectorRef(bodyData2);
        case Animator:          return AnyVectorRef(animatorData);
        default:                return AnyVectorRef();
        }
    }
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
    //Vector2 cursorPos, playerPos = Vector2(gameWidth / 2, gameHeight / 2);
    Vector2 cursorPos;

    // Load the text font
    sf::Font font;
    if (!font.loadFromFile("resources/sansation.ttf"))
        return EXIT_FAILURE;

    // Initialize the info message
    sf::Text message;
    message.setFont(font);
    message.setCharacterSize(40);
    message.setPosition(0.0f, 500.0f);
    message.setFillColor(sf::Color::White);
    message.setString("SFML keyboard and mouse demo\n We will turn this into a top down shooter!");


    BaseState<Cp> state;


    //get<Cp::Player>(ref)->position;

    //std::cout << typeid(std::tuple_element<0, Cp::type_table>::type).name() << std::endl;



    state.Extend(5000);

    auto playerEntity = state.Create();

    state.ChangeSignature(playerEntity, {Cp::Player, Cp::Animator, Cp::Body});

    auto pref = state.GetRef(playerEntity);

    pref.body->position = Vector2(400, 300);
    pref.body->velocity = Vector2(0, 0);
    pref.animator->color = Color(1.0f, 1.0f, 1.0f);

    for (int i = 0; i < 500; i++) {
        auto e = state.Create();

        state.ChangeSignature(e, {Cp::Body, Cp::Animator});

        int r = rand()%100;
        if (r < 10) {
            state.AddSignature( e, {Cp::RadarBlip} );
        }

        auto eref = state.GetRef(e);

        eref.body->position = Vector2(rand()%800, rand()%600);
        eref.body->velocity = Vector2(rand()%5, rand()%5);
        //eref.body->velocity = Vector2(0, 0);
        eref.animator->color = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);

    }

    std::vector<Cmd> cmdbatch;

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

        cmdbatch.clear();
        state.ForEach<Cp::Body>([&](Cp::EntityRef& r) {
            if (rand()%100  == 0) {
                if (r.GetEntity() != playerEntity) {
                    cmdbatch.push_back( {r.GetEntity(), Cmd::Destroy} );
                }
            }
        });
        state.Batch(cmdbatch);



        for (int i = 0; i < 10; i++) {
             auto e = state.Create();

             state.ChangeSignature( e, {Cp::Body, Cp::Animator});

            int r = rand()%100;
            if (r < 10) {
                state.AddSignature( e, {Cp::RadarBlip});
            }

            auto eref = state.GetRef(e);

            eref.body->position = Vector2(rand()%800, rand()%600);
            eref.body->velocity = Vector2(rand()%5, rand()%5);
            //eref.body->velocity = Vector2(0, 0);
            eref.animator->color = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);
        }


        //std::stringstream ss;
        //ss << "count: " << state.radarBlip.data.size();
        //ss << "largest: " << state.entityManager.largest_generation << "    " << state.entityManager.table.size();
        //message.setString(ss.  //       template<typename T>
   //         static void Incerment(T& t, const int32_t*& i) { t += *i++; }str());

        // update coordinates, this is where the game state will be updated
        cursorPos = Vector2(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);

        // get a movement vector
        Vector2 move(0.0f, 0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))    move.y = -1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))    move.y =  1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))    move.x = -1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))    move.x =  1.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            /*const Signature& sig = state.GetSignature(playerEntity);
            int32_t i = 0;
            while (sig[i] != Signature::NullCp) {
                std::cout << ", " << Cp::CONST_COMP_STRING[sig[i]];
                ++i;
            }
            std::cout << std::endl;*/
        }

        // this will make the player move a constant speed in all directions, even diagonals
        move.Normalize();


        // move player
        if (state.Valid(playerEntity) == true) {
            auto playerRef = state.GetRef<Cp::Player>(playerEntity);
            if (!playerRef.player.IsNull()) {

                playerRef.player->control.cursor = cursorPos;
                playerRef.player->control.move = move;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) {
                    state.AddSignature( playerEntity, {Cp::RadarBlip} );
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
                    state.RemoveSignature( playerEntity, {Cp::RadarBlip} );
                }
            }
        }

        state.ForEachFast<Cp::Body, Cp::Player>([&](Cp::EntityRef& r) {
            r.body->velocity = r.player->control.move;
        });

        state.ForEachFast<Cp::Body>([&](Cp::EntityRef& r) {
            r.body->position += r.body->velocity;
        });


  //       template<typename T>
   //         static void Incerment(T& t, const int32_t*& i) { t += *i++; }
        // Rendering code
        // Clear the window
        window.clear(sf::Color(50, 200, 50));

        // draw everything
        state.ForEachFast<Cp::Body, Cp::Animator>([&](Cp::EntityRef& r) {
            circle.setFillColor(sf::Color(r.animator->color.r*255, r.animator->color.g*255, r.animator->color.b*255));
            circle.setPosition(r.body->position.x, r.body->position.y);
            circle.setRadius(playerRadius);
            circle.setOrigin(playerRadius / 2, playerRadius / 2);
            window.draw(circle);
        });


        state.ForEachFast<Cp::RadarBlip, Cp::Body>([&](Cp::EntityRef& r) {
            circle.setFillColor(sf::Color::Black);
            circle.setPosition(r.body->position.x, r.body->position.y);
            circle.setRadius(playerRadius / 2);
            circle.setOrigin((playerRadius / 2)/ 2, (playerRadius / 2)/2);
            window.draw(circle);
        });


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
