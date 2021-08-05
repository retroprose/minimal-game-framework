

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <fstream>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


#include <game.hpp>



std::string make_bits(uint32_t b) {
    std::stringstream s;
    uint32_t mask = 0x1;
    for (int32_t i = 0; i < 32; ++i) {
        if (mask & b) {
            s << "1";
        } else {
            s << "0";
        }
        mask <<= 1;
    }
    return s.str();
}



// billy boy
////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main() {

    sf::Color color_table[] = {
        sf::Color(0xff000000),
        sf::Color(0xff000000),

        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),
        sf::Color(0xff0000ff),

        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),
        sf::Color(0xff0000c8),

        sf::Color(0xffff0000),
        sf::Color(0xffc80000),

        sf::Color(0xffff00ff),
        sf::Color(0xff00ffff),

        sf::Color(0xffffff00), // easy_1?
        sf::Color(0xffffff00), // easy_2?

        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),

        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),
        sf::Color(0xffc8c8c8),

        sf::Color(0xff00ff00),
        sf::Color(0xff00c800),

        sf::Color(0xff000000),
        sf::Color(0xff000000),
        sf::Color(0xff000000),
        sf::Color(0xff000000)
    };



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
    /*
    sf::CircleShape player;
    player.setPosition(gameWidth / 2, gameHeight / 2);
    player.setRadius(playerRadius - 3);
    player.setOutlineThickness(3);
    player.setOutlineColor(sf::Color::Black);
    player.setFillColor(sf::Color::White);
    player.setOrigin(playerRadius / 2, playerRadius / 2);
    */
    sf::RectangleShape player;
    player.setPosition(gameWidth / 2, gameHeight / 2);
    player.setSize(sf::Vector2f(playerRadius - 3, playerRadius - 3));
    player.setOutlineThickness(3);
    player.setOutlineColor(sf::Color::Black);
    player.setFillColor(sf::Color::White);
    player.setOrigin(playerRadius / 2, playerRadius / 2);

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
    message.setPosition(0.0f, 500.0f);
    message.setFillColor(sf::Color::White);
    message.setString("WHY IS NOTHING WORKING?!");

    int32_t local_player = 0;
    Slot slot[64];
    for (int32_t i = 0; i < 64; ++i) {
        Slot& s = slot[i];
        s.connected = false;
        s.input.debug = 0;
        s.input.left = false;
        s.input.right = false;
        s.input.nonEmpty = false;
        s.input.primary = false;
        s.input.state = 0;
        s.input.x = 0;
    }
    slot[local_player].connected = true;

    Game game;
    game.setInputEasy(slot);
    game.init(0x03048923);

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
                std::ofstream out("dump.txt");
                for( auto it = game.components.filter(0x0); it.moveNext(); ) {
                    auto r = it.get();
                    out << it.index << " - " << make_bits(r.comp.flags) << std::endl;
                }
                out.close();
            }
        }

        // update coordinates, this is where the game state will be updated
        sf::Vector2f v;

        v.x = sf::Mouse::getPosition(window).x;
        v.y = sf::Mouse::getPosition(window).y;

        cursor.setPosition(v);

        v.x = v.y = 0.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))    v.y = -5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))    v.y =  5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))    v.x = -5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))    v.x =  5.0f;

        //v.x += player.getPosition().x;
        //v.y += player.getPosition().y;

        //player.setPosition(v);

        slot[local_player].input.left = false;
        slot[local_player].input.right = false;
        slot[local_player].input.primary = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))     slot[local_player].input.left = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))    slot[local_player].input.right = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))    slot[local_player].input.primary = true;

        game.setInputEasy(slot);
        game.update();

        // Rendering code
        // Clear the window
        window.clear(sf::Color(50, 200, 50));

        // Drawing function!
        bool draw;
        float tx, ty;
        uint16_t tf;
        for( auto it = game.components.filter(Cf::Active | Cf::Component | Cf::Body | Cf::ObjectId | Cf::Animator); it.moveNext(); ) {
            auto r = it.get();

            draw = true;

            if (r.comp.flags & Cf::Player)
            {
                if (r.player.slot == local_player)
                {
                    draw = false;
                    tx = (float)r.body.position.x;
                    ty = (float)r.body.position.y;
                    tf = r.animator.frame;
                }
            }
            if (draw == true)
            {
                player.setFillColor(color_table[r.animator.frame]);
                v.x = r.body.position.x + (1920 / 2);
                v.y = 1080 - (r.body.position.y + (1080 / 2));
                player.setPosition(v);
                v.x = r.body.size.x * 2;
                v.y = r.body.size.y * 2;
                player.setSize(v);
                window.draw(player);
            }

        }

        if (tf == Data::player_ship_0) tf = Data::local_player_0;
        if (tf == Data::player_ship_1) tf = Data::local_player_1;

        player.setFillColor(color_table[tf]);
        v.x = tx + (1920 / 2);
        v.y = 1080 - (ty + (1080 / 2));
        player.setPosition(v);
        v.x = 16 * 2;
        v.y = 10 * 2;
        player.setSize(v);
        window.draw(player);


        //window.draw(player);
        window.draw(cursor);
        window.draw(message);

        // Display things on screen
        window.display();
    }

    // exit the app
    return EXIT_SUCCESS;
}
