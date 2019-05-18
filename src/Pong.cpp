

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <Math.hpp>

// billy boy
////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // Define some constants
    const int gameWidth = 800;
    const int gameHeight = 600;

    float cursorRadius = 10.f;
    float playerRadius = 20.f;

    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode(gameWidth, gameHeight, 32), "SFML Pong",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Load the sounds used in the game
    sf::SoundBuffer ballSoundBuffer;
    if (!ballSoundBuffer.loadFromFile("resources/ball.wav"))
        return EXIT_FAILURE;
    sf::Sound ballSound(ballSoundBuffer);


    // Create the ball
    sf::CircleShape player;
    player.setRadius(playerRadius - 3);
    player.setOutlineThickness(3);
    player.setOutlineColor(sf::Color::Black);
    player.setFillColor(sf::Color::White);
    player.setOrigin(playerRadius / 2, playerRadius / 2);

       // Create the ball
    sf::CircleShape cursor;
    cursor.setRadius(10.0f);
    cursor.setOutlineThickness(3);
    cursor.setOutlineColor(sf::Color::Black);
    cursor.setFillColor(sf::Color::White);
    cursor.setOrigin(cursorRadius / 2, cursorRadius / 2);


    // Load the text font
    sf::Font font;
    if (!font.loadFromFile("resources/sansation.ttf"))
        return EXIT_FAILURE;

    // Initialize the pause message
    sf::Text pauseMessage;
    pauseMessage.setFont(font);
    pauseMessage.setCharacterSize(40);
    pauseMessage.setPosition(170.f, 150.f);
    pauseMessage.setFillColor(sf::Color::White);
    pauseMessage.setString("Welcome to SFML pong!\nPress space to start the game");

    sf::Clock clock;

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


        sf::Vector2f v;

        v.x = sf::Mouse::getPosition(window).x;
        v.y = sf::Mouse::getPosition(window).y;

        cursor.setPosition(v);

        v.x = v.y = 0.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))       v.y = 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))     v.y = -1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))     v.x = -1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))    v.x = 1.0f;

        v.x += player.getPosition().x;
        v.y += player.getPosition().y;

        player.setPosition(v);


        // Clear the window
        window.clear(sf::Color(50, 200, 50));

        window.draw(player);
        window.draw(cursor);
        window.draw(pauseMessage);

        // Display things on screen
        window.display();
    }

    return EXIT_SUCCESS;
}
