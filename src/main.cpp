

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <fstream>
#include <sstream>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


#include <game.hpp>


int32_t random_num(int m) {
    int32_t min = 0;
    int32_t max = m;
    int32_t r = (int32_t)(rand() % (max + 1 - min)) + min;
    return r;
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
        sf::Color(0x000000ff),  // rrggbbaa
        sf::Color(0x000000ff),

        sf::Color(0xff0000ff),  // enemy_00_a
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

        sf::Color(0xc80000ff),  // enemy_00_b
        sf::Color(0xc80000ff),
        sf::Color(0xc80000ff),
        sf::Color(0xc80000ff),
        sf::Color(0xc80000ff),
        sf::Color(0xc80000ff),
        sf::Color(0xc80000ff),
        sf::Color(0xc80000ff),
        sf::Color(0xc80000ff),
        sf::Color(0xc80000ff),
        sf::Color(0xc80000ff),

        sf::Color(0x00ff00ff),  // player_ship_0
        sf::Color(0x00c800ff),

        sf::Color(0x00ffffff),  // player_shot
        sf::Color(0xff00ffff),  // enemy_shot

        sf::Color(0xffffffff), // easy_0?
        sf::Color(0xffffffff), // easy_1?

        sf::Color(0xc8c8c8ff),  // player_boom_0
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),

        sf::Color(0xc8c8c8ff),  // enemy_boom_0
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),
        sf::Color(0xc8c8c8ff),

        sf::Color(0x0000ffff),  // local_player_0
        sf::Color(0x0000c8ff),

        sf::Color(0xffff00ff),  // text_ready
        sf::Color(0xffff00ff),  // text_no
        sf::Color(0xffff00ff),  // text_great
        sf::Color(0xffff00ff)   // target
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
    message.setPosition(0.0f, 0.0f);
    message.setFillColor(sf::Color::White);
    message.setString("WHY IS NOTHING WORKING?!");

    int32_t max_players = 64;
    int32_t max_shoot = 500;
    int32_t max_turn = 500;
    int32_t local_player = 0;
    Slot slot[64];
    uint16_t ai_shoot[64];
    uint16_t ai_turn[64];
    uint8_t ai_direction[64];
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

        ai_direction[i] = random_num(2);
        ai_turn[i] = random_num(max_turn) + 1;
        ai_shoot[i] = random_num(max_shoot) + 1;
        if (i < max_players) {
            s.connected = true;
        }
    }

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
                /*std::ofstream out("dump.txt");
                for( auto it = game.components.filter(0x0); it.moveNext(); ) {
                    auto r = it.get();
                    out << it.index << " - " << make_bits(r.comp.flags) << std::endl;
                }
                out.close();*/
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

        for (int32_t i = 1; i < 64; ++i) {
            slot[i].input.left = false;
            slot[i].input.right = false;
            slot[i].input.primary = false;
            --ai_shoot[i];
            --ai_turn[i];
            if (ai_shoot[i] == 0) {
                slot[i].input.primary = true;
                ai_shoot[i] = random_num(max_shoot) + 1;
            }
            if (ai_turn[i] == 0) {
                ai_direction[i] = !ai_direction[i];
                ai_turn[i] = random_num(max_turn) + 1;
            }
            if (ai_direction[i] == 0) {
                slot[i].input.left = true;
            } else {
                slot[i].input.right = true;
            }
        }

        game.setInputEasy(slot);
        game.update();

        std::stringstream ss;
        //ss << "enemyCount: " << game.global.enemyCount << std::endl;
        //ss << "enemySpeed: " << game.global.enemySpeed << std::endl;
        //ss << "playing: " << game.global.playing << std::endl;
        //ss << "textAnimate: " << game.global.textAnimate << std::endl;
        //ss << "textType: " << game.global.textType << std::endl;
        ss << "capacity: " << game.components.comp.capacity();
        message.setString(ss.str());

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
