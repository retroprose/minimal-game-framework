

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Math.hpp>
#include <Entity.hpp>


/*
    This file will be the only one that will include the SFML game framework.
    We will be creating a game state of components, updating it,
    then rendering the results to the screen.  In the Entity.hpp file,
    I put a lot of effort into making it clean and organized, here
    I'm just trying to get things to render correctly using whatever I can.
    I consider this throw away code since we will replace this other engines
    anyway.

    The idea here is our entity and game state code stays nice and clean
    and consistent, then we may do some dirty transformations to get it
    to map from the input to the output.  For example I like to use
    positive y axis up, but SFML uses down.  There isn't a trivial way to
    invert it, will take some dirty transformations possibly.

    I don't fully understand the SMFL coordinate system, and I don't plan to.
    We are just using it to test our game state, there are much better options
    out there that could map more directly to our coordinate system.

    The main class that will be used to run the game is the 'BaseState'
    in the Entity.hpp.

    It's interface looks like this:

    These two functions are utility functions, they allow you to adjust memory
    reservation up front to avoid resizes during gameplay.  They never actually need to be
    called if you don't mind resizing when needed.
        void Extend(uint16_t e)                                             // may error if out of memory
        void ReserveSignature(const Signature& signature, uint16_t count)   // may error if out of memory, or if a signature has a bad component id


    This function creates a new entity by reserving a unique index,
    until the destroy function is called.
        Entity Create()                         // may error if out of memory


    These functions give you information about a specific entity.  Valid lets you know if
    an entity is still in use, and GetSignature will return that entities current signature.
        bool Valid(Entity entity) const                     // never errors
        const Signature& GetSignature(Entity entity) const  // may error if entity is invalid, or is an untracked entity


    These functions are used to update an entities signature.  It can be updated with a
    raw component list, or by a signature id.  Storing signature ids is faster since
    component lists don't have to be made. SigId will give you a signature's id,
    if a signature doesn't have one, it will assign it one.  Once a signature gets
    an id, it keeps it for the rest of the program execution.  Change signature
    replaces the signature completely, add adds the components, and remove
    will remove the given components.  Destroy uses change signature to change it
    to the null signature, then destroys the entity freeing up the entity id.
    All of these functions can invalidate references (except SigId), and shouldn't be called
    during a ForEach loop.
        SignatureId SigId(const Signature& signature)                   // never errors

        void ChangeSignature(Entity entity, SignatureId signature_id)   // may error if entity is invalid, or is an untracked entity, or if out of memory or if passed a bad signature id
        void RemoveSignature(Entity entity, SignatureId signature_id)   // may error if entity is invalid, or is an untracked entity, or if out of memory or if passed a bad signature id
        void AddSignature(Entity entity, SignatureId signature_id)      // may error if entity is invalid, or is an untracked entity, or if out of memory or if passed a bad signature id

        void ChangeSignature(Entity entity, const Signature& signature  // may error if entity is invalid, or is an untracked entity, or if out of memory
        void RemoveSignature(Entity entity, const Signature& signature) // may error if entity is invalid, or is an untracked entity, or if out of memory
        void AddSignature(Entity entity, const Signature& signature)    // may error if entity is invalid, or is an untracked entity, or if out of memory

        void DestroyEntity(Entity entity)                               // may error if entity is invalid, or is an untracked entity


    This is a special function that allows you to batch signature updates and entity deletions.
    The use is to create a vector of update commands inside of the ForEach, then at the end
    of the loop you can call this function safely.
        void Batch(std::vector<Cmd>& list)        // simply calls the above functions, may error for same reasons

    These two functions are used to get a entity referencer object.  The template version allows you to
    get a subset of components, while the regular one will use the entities signature to get all of the
    components it has.
        T::EntityRef GetRef(Entity entity)                      // may error if entity is invalid, or is an untracked entity
        T::EntityRef GetRef<uint8_t... Args>(Entity entity)     // may error if entity is invalid, or is an untracked entity

    These are our functions for iterating though all the components in the state.  The only difference
    between then is the 'Fast' version does not access entity values (the entity value on the referencer
    will be the null entity, you can however access it in your update function).
        void ForEach<uint8_t... Args>(F func)           // may error due to bad signature components
        void ForEachFast<uint8_t... Args>(F func)       // may error due to bad signature components

*/


/*
    The body component will represent a position in space, a velocity, and store the radius of the
    object.
*/
struct Body {
	Vector2 position;
	Vector2 velocity;
	Real radius;
};

/*
    A component that simply stores a color.
*/
struct Color {
    Color() { }
    Color(Real r_, Real g_, Real b_, Real a_) : r(r_), g(g_), b(b_), a(a_) { }
    Color(Real r_, Real g_, Real b_) : r(r_), g(g_), b(b_), a(1.0f) { }
    Real r;
    Real g;
    Real b;
    Real a;
};

/*
    A component that links to our controller, and it used to move a
    circle around the screen.
*/
struct Controller {
    Vector2 cursor;
	Vector2 move;
};



/*
    This structure represents the pack of components that will
    be used in the game state class.
*/
struct Cp {
    /*
        Every component gets an id.
    */
    enum etype : uint8_t {
        Controller = 0,
        Body,
        Color,
        HatColor,
        _ComponentCount
    };

    /*
        A tuple that needs to be synced with the enum above.
        This allows us to get a type by an integer.
    */
    using type_table = std::tuple<
        ::Controller,
        ::Body,
        ::Color,
        ::Color
    >;

    /*
        This object is used to reference into an entities
        components.
    */
    struct EntityRef : public BaseEntityRef {
        Ref<::Controller> controller;
        Ref<::Body> body;
        Ref<::Color> color;
        Ref<::Color> hatColor;
    };

    /*
        These are the containers that store each component.
        Right now they all use the same type of container,
        but I would like to have other components that
        may use a different storage scheme.
    */
    VectorMap<::Controller> controllerData;
    VectorMap<::Body> bodyData;
    VectorMap<::Color> colorData;
    VectorMap<::Color> hatColorData;

    /*
        This function allows us to access components dynamically though an id.
        the default here asserts if an invalid id is passed.
    */
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
    message.setPosition(0.0f, 450.0f);
    message.setFillColor(sf::Color::White);
    message.setString("Use WASD to move, mouse to move cursor.\nLeft and right mouse button adds and\nRemoves player's hat.");

    /*
        Here we are defining the state, 'Cp' is our pack of component containers
    */
    BaseState<Cp> state;

    /*
        Here we are creating the controllable entity.  The process goes like this:
        Create entity (which is a unique 32 bit value), add components (starts with
        'null signature' zero components), then get a referencer to update the values
        of each component.
    */
    auto playerEntity = state.Create();

    state.ChangeSignature(playerEntity, {Cp::Controller, Cp::Color, Cp::Body});

    auto pref = state.GetRef(playerEntity);

    // notice that references use -> instead of . to access members
    pref.body->position = Vector2(400, 300);    // set to center of screen
    pref.body->velocity = Vector2(0, 0);        // zero velocity
    pref.body->radius = playerRadius;           // radius of player
    *pref.color = Color(1.0f, 1.0f, 1.0f);      // player will be white (notice we dereference with *)


    /*
        Here we will create an initial 50 moving entities,
        some with 'hats' some without.  The 'hat' is just a
        smaller circle of a color on top.  (used to be 500 for
        testing, but hard to see player)
    */
    for (int i = 0; i < 50; i++) {
        auto e = state.Create();

        state.ChangeSignature(e, {Cp::Body, Cp::Color});

        // random chance of having a hat
        int r = rand()%100;
        if (r < 10) {
            state.AddSignature( e, {Cp::HatColor} );
        }

        auto eref = state.GetRef(e);

        eref.body->position = Vector2(rand()%800, rand()%600);
        eref.body->velocity = Vector2(rand()%5, rand()%5);
        //eref.body->velocity = Vector2(0, 0);
        eref.body->radius = playerRadius;
        *eref.color = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);

        // some will not have hats, so need to check that the reference isn't null first
        if (eref.hatColor.IsNull() == false) {
            *eref.hatColor = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);
        }
    }

    // the command batch array
    Vector<Cmd> cmdbatch;

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
            this is an example of using the command batch function.
            Here we aren't using 'ForEachFast' because we access the
            entity value.
        */
        cmdbatch.Clear();
        state.ForEach<Cp::Body>([&](Cp::EntityRef& r) {
            // don't delete the player!
            if (r.GetEntity() != playerEntity) {
                // every moving entity has a chance of being deleted
                if (rand()%100  == 0) {
                    cmdbatch.Add( {r.GetEntity(), Cmd::Destroy} );
                }
            }
        });
        state.Batch(cmdbatch); // after the for each its now safe to run commands.

        /*
            Create 1 new moving objects. (it used to be 10, but hard to see player)
        */
        for (int i = 0; i < 1; i++) {
            auto e = state.Create();

            state.ChangeSignature(e, {Cp::Body, Cp::Color});

            int r = rand()%100;
            if (r < 10) {
                state.AddSignature( e, {Cp::HatColor} );
            }

            auto eref = state.GetRef(e);

            eref.body->position = Vector2(rand()%800, rand()%600);
            eref.body->velocity = Vector2(rand()%5, rand()%5);
            //eref.body->velocity = Vector2(0, 0);
            eref.body->radius = playerRadius;
            *eref.color = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);

            if (eref.hatColor.IsNull() == false) {
                *eref.hatColor = Color(rand()%10000/10000.0f, rand()%10000/10000.0f, rand()%10000/10000.0f);
            }
        }

        /*
            Here is where we will update the controller data
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
            Here we are going to set the player's controller data.
            for multi-player, would set multiple control data.
        */

        // check to make sure playerEntity is still valid
        if (state.Valid(playerEntity) == true) {
            // get a reference to the controller component
            auto playerRef = state.GetRef<Cp::Controller>(playerEntity);
            // check to make sure the entity does in fact have that component
            if (!playerRef.controller.IsNull()) {

                // set controller data
                playerRef.controller->cursor = cursorPos;
                playerRef.controller->move = move;

                /*
                    Here we are testing the add and remove component feature
                    of our entity framework.  We are not in a for each right now
                    so we can add and remove components freely.
                */
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    state.AddSignature( playerEntity, {Cp::HatColor} );
                    // notice how we need to re-get the reference after updating the entity
                    pref = state.GetRef(playerEntity);
                    *pref.hatColor = Color(0, 0, 0);
                } else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
                    state.RemoveSignature( playerEntity, {Cp::HatColor} );
                }
            }
        }

        /*
            This is the update player for each, it does not access
            the entity value, so we can use ForEachFast
        */
        state.ForEachFast<Cp::Body, Cp::Controller>([&](Cp::EntityRef& ref) {
            ref.body->velocity = ref.controller->move;
        });

        /*
            This updates ALL entities with bodies, even the player.
            Also notice how we can use ForEachFast
        */
        state.ForEachFast<Cp::Body>([&](Cp::EntityRef& ref) {
            ref.body->position += ref.body->velocity;
        });


        // Rendering code
        // Clear the window
        window.clear(sf::Color(50, 200, 50));


        // This will draw all of the entities.
        state.ForEachFast<Cp::Body, Cp::Color>([&](Cp::EntityRef& ref) {
            circle.setFillColor(sf::Color(ref.color->r*255, ref.color->g*255, ref.color->b*255));
            circle.setPosition(ref.body->position.x, ref.body->position.y);
            circle.setRadius(ref.body->radius);
            circle.setOrigin(ref.body->radius / 2.0f, ref.body->radius / 2.0f);
            window.draw(circle);
        });

        // This will draw all of the entity hats.
        state.ForEachFast<Cp::HatColor, Cp::Body>([&](Cp::EntityRef& ref) {
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
