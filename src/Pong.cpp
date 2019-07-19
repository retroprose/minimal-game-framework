

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Math.hpp>
#include <Entity.hpp>
#include <Container.hpp>

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


enum CpType : uint32_t {
    Controller = 0,
    Color,
    HatColor,
    Body
};


using MyState = State<
    HashMap<Cp::Controller>,
    VectorMap<Cp::Color>,
    HashMap<Cp::Color>,
    VectorMap<Cp::Body>
>;



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

    //
    Ref<Cp::Body> body;
    Ref<Cp::Color> color;

    MyState state;

    auto playerEntity = state.create();


    {
        state.changeSignature<Body, Color>(playerEntity);
        std::tie(body, color) = state.reference<Body, Color>(playerEntity);

        // notice that references use -> instead of . to access members
        body->position = Vector2(400, 300);    // set to center of screen
        body->velocity = Vector2(0, 0);        // zero velocity
        body->radius = playerRadius;           // radius of player
        color.set( Cp::Color(1.0f, 1.0f, 1.0f) );      // player will be white (notice we dereference with *)

        state.setActive(playerEntity);
    }

    for (int i = 0; i < 50; ++i) {
        auto e = state.create();

        state.changeSignature<Body, Color>(e);

        // random chance of having a hat
        int r = rand()%100;
        if (r < 10) {
            //state.addSignature( e, {State::HatColor} ;

            // state.container<HatColor>().add(e);
        }

        {
            std::tie(body, color) = state.reference<Body, Color>(e);

            //auto hatColor = state.hatColor(e);

            body->position = Vector2(rand()%800, rand()%600);
            body->velocity = Vector2(rand()%5, rand()%5);
            //eref.body->velocity = Vector2(0, 0);
            body->radius = playerRadius;
            color.set( Cp::Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f) );
        }

        // some will not have hats, so need to check that the reference isn't null first
        //if (ref.hatColor.isNull() == false) {
        //    *ref.hatColor = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);
        //}

        //state.setActive(e, true);
        state.setActive(e);
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
*/
/*
        for ( auto& ref : state.iterate<Body>() ) {
            //std::cout << ref.hash.signature() << ", " << ref.hash.index() << std::endl;
            Entity entity = state.entityFromHash(ref.hash);
            if (entity != playerEntity) {
                // every moving entity has a chance of being deleted
                if (rand()%100  == 0) {
                    state.destroy(entity);
                }
            }
        }



        for (int i = 0; i < 1; i++) {
            auto e = state.create();

            state.changeSignature<Body, Color>(e);

            // random chance of having a hat
            int r = rand()%100;
            if (r < 10) {
                //state.addSignature( e, {State::HatColor} ;

                // state.container<HatColor>().add(e);
            }

            {
                std::tie(body, color) = state.reference<Body, Color>(e);

                //auto hatColor = state.hatColor(e);

                body->position = Vector2(rand()%800, rand()%600);
                body->velocity = Vector2(rand()%5, rand()%5);
                //eref.body->velocity = Vector2(0, 0);
                body->radius = playerRadius;
                color.set( Cp::Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f) );
            }

            // some will not have hats, so need to check that the reference isn't null first
            //if (ref.hatColor.isNull() == false) {
            //    *ref.hatColor = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);
            //}

            //state.setActive(e, true);
            //state.activate(e);

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
*/


        for ( auto& ref : state.iterate<Body>() ) {
            std::tie(body) = ref.pack;
            body->position += body->velocity;
        }


        // Rendering code
        // Clear the window
        window.clear(sf::Color(50, 200, 50));

/*
       for (auto& vec : state.proxy<Body, Color>()) {
            for (auto ref : vec) {
                std::tie(body, color) = ref;
                circle.setFillColor(sf::Color(color->r*255, color->g*255, color->b*255));
                circle.setPosition(body->position.x, body->position.y);
                circle.setRadius(body->radius);
                circle.setOrigin(body->radius / 2.0f, body->radius / 2.0f);
                window.draw(circle);
            }
        }
*/

        // This will draw all of the entities.
        for ( auto& ref : state.iterate<Body, Color>() ) {
            std::tie(body, color) = ref.pack;
            circle.setFillColor(sf::Color(color->r*255, color->g*255, color->b*255));
            circle.setPosition(body->position.x, body->position.y);
            circle.setRadius(body->radius);
            circle.setOrigin(body->radius / 2.0f, body->radius / 2.0f);
            window.draw(circle);
        }



/*
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
