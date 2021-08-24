

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <fstream>
#include <sstream>
#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


#include <components.hpp>


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

void freeListDump(std::string filename, FreeList& fl) {
    std::ofstream ss(filename, std::ios::app);
    ss << "List" << std::endl;
    for (int j = 0; j < fl.table.size(); j++) {
        ss << "chunk: " << j << " - " << fl.head_table[j] << std::endl;
        for (int i = 0; i < fl.table[j].size(); i++) {
            ss << i << ": " << fl.table[j][i] << std::endl;
        }
    }
    ss.close();
}

void mapDump(std::string f, std::map<uint16_t, bool>& c) {
    std::ofstream ss(f, std::ios::app);
    ss << "List" << std::endl;
    auto it = c.begin();
    while (it != c.end()) {
        uint16_t value = it->first;
        ss << FreeList::getChunk(value) << ": " << FreeList::getIndex(value) << " - " << value << std::endl;
        ++it;
    }
    ss.close();
}


// billy boy
////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main() {


    // Define some constants
    const int gameWidth = 1920; //800;
    const int gameHeight = 1080; //600;

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
    sf::Sound ballSound(ballSoundBuffer);


    // Create the player circle to be controlled by WASD
    sf::RectangleShape rectangle;
    rectangle.setPosition(gameWidth / 2, gameHeight / 2);
    rectangle.setSize(sf::Vector2f(playerRadius - 3, playerRadius - 3));
    rectangle.setOutlineThickness(3);
    rectangle.setOutlineColor(sf::Color::Black);
    rectangle.setFillColor(sf::Color::White);
    rectangle.setOrigin(playerRadius / 2, playerRadius / 2);

    // Create the cursor that will follow the mouse
    sf::CircleShape cursor;
    cursor.setPosition(0.0f, 0.0f);
    cursor.setRadius(cursorRadius);
    cursor.setOutlineThickness(3);
    cursor.setOutlineColor(sf::Color::Black);
    cursor.setFillColor(sf::Color::White);
    cursor.setOrigin(cursorRadius / 2, cursorRadius / 2);


    // Load the text font
    sf::Font font;
    if (!font.loadFromFile("resources/sansation.ttf"))
        return EXIT_FAILURE;

    // Initialize the info message
    sf::Text message;
    message.setFont(font);
    message.setCharacterSize(40);
    message.setPosition(0.0f, 0.0f);
    message.setFillColor(sf::Color::White);
    message.setString("WHY IS NOTHING WORKING?!");

    /*
    std::string dfile = "test_free_list.txt";
    { std::ofstream outie(dfile); outie.close(); }

    std::map<uint16_t, bool> map_values;
    FreeList flist;
    flist.resize(5);
    int32_t test_values[] = {
        0,3,2,1,0,2,1,0,0,4,4,3,0,-1
    };

    int32_t al;
    int32_t value;
    int32_t g = 0;
    while ( (value = test_values[g++]) != -1 ) {
        al = flist.allocate(value);
        map_values[al] = true;
    }

    //freeListDump(dfile, flist);
    mapDump(dfile, map_values);

    flist.free(4);
    flist.free(1);
    flist.free(2);
    flist.free(8194);
    flist.free(24578);

    freeListDump(dfile, flist);
    */


    bool start = true;

    Cp cp;
    auto playerEntity = cp.create(cp._Body_Color);

    {
        cp.add(playerEntity, Cp::fActive | Cp::fController);
        auto r = cp.get<_Body_Color>(playerEntity);

        // notice that references use -> instead of . to access members
        r.body.position = Vector2(400, 300);    // set to center of screen
        r.body.velocity = Vector2(0, 0);        // zero velocity
        r.body.radius = playerRadius;           // radius of player
        r.color = Color(1.0f, 1.0f, 1.0f);      // player will be white (notice we dereference with *)
    }



    for (int i = 0; i < 50; ++i) {
        auto e = cp.create(cp._Body_Color);

        // random chance of having a hat
        int r = rand()%100;
        if (r < 10) {
            //state.add<HatColor>(e);
        }

        {
            cp.add(e, Cp::fActive);

            auto r = cp.get<_Body_Color>(e);

            r.body.position = Vector2(rand()%800, rand()%600);
            r.body.velocity = Vector2(rand()%5, rand()%5);
            //r.body.velocity = Vector2(0, 0);
            r.body.radius = playerRadius;
            r.color = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);

            // some will not have hats, so need to check that the reference isn't null first
            //if (hatColor.isNull() == false) {
            //    *hatColor = Cp::Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);
            //}

        }

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
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return)
            {
                // pressed return!
            }
        }



        {

            for (auto it = cp.filter(Cp::fActive | Cp::fBody); it.next();) {
                Entity e = it.entity();
                auto& body = *cp.body(e);
                if (e != playerEntity) {
                    // every moving entity has a chance of being deleted
                    if (rand()%100 == 0) {
                        cp.destroy(e);
                    }
                }
            }
        }

        for (int i = 0; i < 1; ++i) {
            auto e = state.create();

            state.change<Body, Color>(e);

            // random chance of having a hat
            int r = rand()%100;
            if (r < 10) {
                state.add<HatColor>(e);
            }

            {
                std::tie(body, color, hatColor) = state.reference<Body, Color, HatColor>(e).pack;

                body->position = Vector2(rand()%800, rand()%600);
                body->velocity = Vector2(rand()%5, rand()%5);
                //ref.body->velocity = Vector2(0, 0);
                body->radius = playerRadius;
                *color = Cp::Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);

                // some will not have hats, so need to check that the reference isn't null first
                if (hatColor.isNull() == false) {
                    *hatColor = Cp::Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);
                }

            }
            state.setActive(e);
        }


        cursor.setPosition(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);


        Vector2 move(0.0f, 0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))    move.y = -5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))    move.y =  5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))    move.x = -5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))    move.x =  5.0f;
        v.Normalize();


        // check to make sure playerEntity is still valid
        /*if (cp.valid(playerEntity) == true) {

            auto itController = cp.controller(playerEntity);

            // check to make sure the entity does in fact have that component
            if ( controller.valid() ) {
                Controller& controller = *itController;

                // set controller data
                controller.cursor = cursorPos;
                controller.move = move;

                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    state.add<HatColor>(playerEntity);
                    // notice how we need to re-get the reference after updating the entity
                    hatColor = state.find<HatColor>(playerEntity);
                    *hatColor = Cp::Color(0, 0, 0);
                } else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
                    state.remove<HatColor>(playerEntity);
                }
            }
        }

        {
            for (auto it = cp.controller(entity); it.next();) {
                auto r = cp.get<_Body_Controller>(Entity(it.index()));
                r.body.velocity = r.controller.move;
            }
        }*/

        {
            for (auto it = cp.filter(Cp::fActive | Cp::fBody); it.next();) {
                auto r = cp.get<_Body_Color>(it.fastEntity());
                r.body.position += r.body.velocity;
            }
        }


        // Rendering code
        // Clear the window
        window.clear(sf::Color(50, 200, 50));

        // draw stuff!
        for (auto it = cp.filter(Cp::fActive | Cp::fBody | Cp::fColor); it.next();) {
            auto r = cp.get<_Body_Color>(it.fastEntity());
            rectangle.setFillColor(sf::Color(r.color.r*255, r.color.g*255, r.color.b*255));
            rectangle.setPosition(r.body.position.x, r.body.position.y);
            rectangle.setSize(sf::Vector2f(r.body.radius, r.body.radius));
            rectangle.setOrigin(r.body.radius / 2.0f, r.body.radius / 2.0f);
            window.draw(rectangle);
        }


        //window.draw(player);
        window.draw(cursor);
        window.draw(message);

        // Display things on screen
        window.display();
    }

    // exit the app
    return EXIT_SUCCESS;
}
