

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


// billy boy
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
    sf::Sound ballSound(ballSoundBuffer);


    // Create the player circle to be controlled by WASD
    sf::CircleShape player;
    player.setPosition(gameWidth / 2, gameHeight / 2);
    player.setRadius(playerRadius - 3);
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
    message.setString("SFML keyboard and mouse demo\n We will turn this into a top down shooter!");


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

        v.x += player.getPosition().x;
        v.y += player.getPosition().y;

        player.setPosition(v);


        // Rendering code
        // Clear the window
        window.clear(sf::Color(50, 200, 50));

        window.draw(player);
        window.draw(cursor);
        window.draw(message);

        // Display things on screen
        window.display();
    }

    // exit the app
    return EXIT_SUCCESS;
}
