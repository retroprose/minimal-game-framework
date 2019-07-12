

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




enum CpType : uint8_t {
    Body = 0,
    Color,
    Controller,
    HatColor
};


using MyState = State<
    VectorMap<Cp::Body>,
    VectorMap<Cp::Color>,
    EntityMap<Cp::Controller>,
    EntityMap<Cp::Color>
>;





/*




*/


struct TA { TA(int x) : x_(x) {} int x_; void print() { std::cout << "TA: " << x_ << std::endl; } };
struct TB { TB(int x) : x_(x) {} int x_; void eat() { std::cout << "TB: EATING" << std::endl; } };
struct TC { TC(int x) : x_(x) {} int x_; void print() { std::cout << "TC: " << x_ << std::endl; } };
struct TD { TD(int x) : x_(x) {} int x_; void eat() { std::cout << "TD: EATING" << std::endl; } };
using TT = std::tuple<TA, TB, TC, TD>;

template<typename T> struct is_in_;

template<typename T> struct is_in_ { constexpr static const bool value = false; };
template<> struct is_in_<TA> { constexpr static const bool value = true; };
template<> struct is_in_<TB> { constexpr static const bool value = false; };
template<> struct is_in_<TC> { constexpr static const bool value = true; };

template<typename T>
using is_in = is_in_<T>;

template<typename T> struct is_out;

template<typename T> struct is_out { constexpr static const bool value = true; };
template<> struct is_out<TA> { constexpr static const bool value = false; };
template<> struct is_out<TB> { constexpr static const bool value = true; };
template<> struct is_out<TC> { constexpr static const bool value = false; };


template<typename T> struct is_TB;
template<typename T> struct is_TB { constexpr static const bool value = false; };
template<> struct is_TB<TB> { constexpr static const bool value = true; };



template<uint8_t... Ns>
struct Sequence { };


struct SequenceGen {
    //
    //
    //  GENERATES SEQUENCE FROM 0 to N
    //
    //
    // First define the template signature
    template <uint8_t ... Ns> struct FromRangeHelper;

    // Recursion case
    template <uint8_t I, uint8_t ... Ns>
    struct FromRangeHelper<I, Ns...>
    {
        using type = typename FromRangeHelper<I - 1, I - 1, Ns...>::type;
    };

    // Recursion abort
    template <uint8_t ... Ns>
    struct FromRangeHelper<0, Ns...>
    {
        using type = Sequence<Ns...>;
    };

    template <uint8_t N>
    using FromRange = typename FromRangeHelper<N>::type;

    template <typename T>
    using TupleRange = typename FromRangeHelper<std::tuple_size<T>::value>::type;


    //
    //
    //  GENERATES SEQUENCE BASED ON CONDITION OF TUPLE ELEMENTS
    //
    //
    // First define the template signature
    template <template<typename> typename W, typename T, uint8_t ... Ns > struct FromCondition;
    template <template<typename> typename W, typename T, bool ADD, uint8_t I, uint8_t ... Ns> struct FromConditionHelper;

    // Recursion case
    template <template<typename> typename W, typename T, uint8_t I, uint8_t ... Ns>
    struct FromCondition<W, T, I, Ns...>
    {
        using type = typename FromConditionHelper<W, T, W<typename std::tuple_element<I - 1, T>::type>::value, I, Ns...>::type;
    };

    // Recursion abort
    template <template<typename> typename W, typename T, uint8_t ... Ns>
    struct FromCondition<W, T, 0, Ns...>
    {
        using type = Sequence<Ns...>;
    };

    // helpers allow to omit values that don't match condition
    template <template<typename> typename W, typename T, uint8_t I, uint8_t ... Ns>
    struct FromConditionHelper<W, T, true, I, Ns...> {
        using type = typename FromCondition<W, T, I - 1, I - 1, Ns...>::type;
    };

    template <template<typename> typename W, typename T, uint8_t I, uint8_t ... Ns>
    struct FromConditionHelper<W, T, false, I, Ns...> {
        using type = typename FromCondition<W, T, I - 1, Ns...>::type;
    };

    template<template<typename> typename W, typename T>
    using TupleCondition = typename FromCondition<W, T, std::tuple_size<T>::value>::type;

};

/*



*/



class CpInterface {
private:
    template<bool C, typename T = void>
    struct enable_if {
      typedef T type;
    };

    template<typename T>
    struct enable_if<false, T> { };

public:
    template<template<typename> typename F, typename T>
    static void loop(T& t, Sequence<>) { }
    template<template<typename> typename F, typename T, uint8_t C, uint8_t...Args>
    static void loop(T& t, Sequence<C, Args...>) {
        F<typename std::tuple_element<C, T>::type>::invoke(std::get<C>(t));
        loop<F>(t, Sequence<Args...>());
    }

    template<template<typename> typename F, typename T>
    static void loop(T& t) {
        loop<F>(t, SequenceGen::TupleCondition<F, T>());
    }


    //
    //
    //  Print function!
    // can specialize this if you want!
    //
    //
    //
    template<typename T>
    class Print {
    private:
        template <typename U, U> struct type_check;
        template <typename _1> static uint16_t &chk(type_check<void (_1::*)(), &_1::print> *);
        template <typename   > static  uint8_t &chk(...);

    public:
        static bool const value = sizeof(chk<T>(0)) == sizeof(uint16_t);

        //template<typename _1> static typename enable_if<Print<_1>::value, RETURN_TYPE (void)>::type invoke(_1& t) {
        template<typename _1> static typename enable_if<Print<_1>::value   >::type invoke(_1& t) {
            t.print();
        }

        template<typename _1> static typename enable_if<!Print<_1>::value   >::type invoke(_1& t) {
            std::cout << "DEFAULT!" << std::endl;
        }
    };

    template<typename T>
    static void print(T& t) {
        Print<T>::invoke(t);
    }

};




class Any {
public:
    // this guy is going into a table of function pointers
    template<typename T>
    static Any make(T& t) { return Any(t); }

    struct Abstract {
        virtual void print() = 0;
    };

    Any() {
        ASSERT(false);
    }

    template<typename T>
    Any(T& t) {
        ASSERT(sizeof(Default<T>) <= MaxBuffer);
        Abstract* d = new (buffer_) Default<T>(t);
        ASSERT(d - reinterpret_cast<Abstract*>(buffer_) == 0);
    }

    Abstract* operator->() {
        return reinterpret_cast<Abstract*>(buffer_);
    }

private:
    template<typename T>
    struct Default : public Abstract {
        Default<T>(T& t) : t_(t) { }
        T& t_;
        virtual void print() { CpInterface::Print<T>::invoke(t_); }
    };

    constexpr static const size_t MaxBuffer = 8;
    uint8_t buffer_[MaxBuffer];

};






template<bool DUMMY = false>
void print_sequence(Sequence<>) { }
template<bool DUMMY = false, uint8_t C, uint8_t...Args>
void print_sequence(Sequence<C, Args...>) {
    std::cout << (int)C << ", ";
    print_sequence(Sequence<Args...>());
}



//
//
//  GENERATES SEQUENCE FROM 0 to N
//
//
// First define the template signature

/*
template <typename S, bool ADD>
struct AddIfCondition;

template <typename S>
struct AddIfCondition<S, true> {
    using type = S;
};

template <typename S>
struct AddIfCondition<S, false> {
    using type = S::tail;
};
*/

// 3, 5, 6, 8     4        5, 6, 8    3     4


template<uint8_t... Ns>
struct TestSeq {
    using tail = TestSeq<>;
};

template<uint8_t H, uint8_t... Ns>
struct TestSeq<H, Ns...> {
    static const uint8_t head = H;
    using tail = TestSeq<Ns...>;
};


template <uint8_t APPEND, typename TAIL, uint8_t...Ns>
struct AppendBack {
    using type = typename AppendBack<APPEND, typename TAIL::tail, Ns..., TAIL::head>::type;
};

template <uint8_t APPEND, uint8_t...Ns>
struct AppendBack<APPEND, TestSeq<>, Ns...> {
    using type = TestSeq<Ns..., APPEND>;
};


template<bool ADD, uint8_t HEAD, typename S>
struct IfCond;

template<uint8_t HEAD, typename S>
struct IfCond<true, HEAD, S> {
    using type = typename AppendBack<HEAD, S>::type;
};

template<uint8_t HEAD, typename S>
struct IfCond<false, HEAD, S> {
    using type = S;
};



template <template<typename> typename W, typename T, typename IS, typename OS = TestSeq<>>
struct TestSeqCpy {
    using type = typename TestSeqCpy<W, T, typename IS::tail, typename IfCond<W<typename std::tuple_element<IS::head, T>::type>::value, IS::head, OS>::type>::type;
};

template <template<typename> typename W, typename T, typename OS>
struct TestSeqCpy<W, T, TestSeq<>, OS> {
    using type = OS;
};


template <uint8_t N, uint8_t I = 0, typename OS = TestSeq<>>
struct MakeSeq {
    using type = typename MakeSeq<N, I + 1, typename AppendBack<I, OS>::type>::type;
};

template <uint8_t N, typename OS>
struct MakeSeq<N, N, OS> {
    using type = OS;
};

/*
template <typename IS, typename OS>
struct TestSeqCpy {
    using type = typename TestSeqCpy<typename IS::tail, typename AppendBack<IS::head, OS>::type>::type;
};

template <typename OS>
struct TestSeqCpy<TestSeq<>, OS> {
    using type = OS;
};
*/

/*
template <typename S, uint8_t...Ns>
struct TestSeqCpy {
    using type = typename TestSeqCpy<typename S::tail, Ns..., S::head>::type;
};

template <uint8_t...Ns>
struct TestSeqCpy<TestSeq<>, Ns...> {
    using type = TestSeq<Ns...>;
};


template <typename IS, typename OS>
struct TestSeqCpy2 {
    using type = typename TestSeqCpy<typename S::tail, Ns..., S::head>::type;
};

template <typename IS, typename OS>
struct TestSeqCpy2<TestSeq<>, OS> {
    using type = TestSeqCpy<;
};
*/

/*
template <typename S, uint8_t...Ns>
struct StripSeq {
    using type = typename TestSeqHelper<typename S::tail, Ns..., S::head>::type;
};

template <uint8_t...Ns>
struct StripSeq<TestSeq<>, Ns...> {
    using type = TestSeq<Ns...>;
};
*/

/*
template <template<typename> typename W, typename T, bool ADD, typename S, uint8_t...Ns>
struct TestSeqHelper { };

template <template<typename> typename W, typename T, typename S, uint8_t...Ns>
struct TestSeqHelper<W, T, true, S, Ns...> {
    using type = typename TestSeqHelper<W, T, W<typename std::tuple_element<S::head, T>::type>::value, typename S::tail, Ns..., S::head>::type;
};

template <template<typename> typename W, typename T, typename S, uint8_t...Ns>
struct TestSeqHelper<W, T, false, S, Ns...> {
    using type = typename TestSeqHelper<W, T, W<typename std::tuple_element<S::head, T>::type>::value, typename S::tail, Ns...>::type;
};

template <template<typename> typename W, typename T, typename S, uint8_t...Ns>
struct TestSeqHelper<W, T, false, TestSeq<>, Ns...> {
    using type = TestSeq<Ns...>;
};
*/


/*
template<uint8_t HEAD, typename TAIL>
struct SmartSequence {
    static const uint8_t head = HEAD;
    using tail = TAIL;
};

template <template<typename> typename W, typename T, uint8_t... Ns>
struct SmartFromRange {
    using type = void;
};

template <template<typename> typename W, typename T, bool ADD, uint8_t... Ns>
struct SmartFromRangeHelper;

template <template<typename> typename W, typename T, uint8_t I, uint8_t... Ns>
struct SmartFromRangeHelper<W, T, true, I, Ns...> {
    using type = SmartSequence<I, typename SmartFromRange<Ns...>::type>;
};

template <template<typename> typename W, typename T, uint8_t... Ns>
struct SmartFromRangeHelper<W, T, false, Ns...> {
    using type = typename SmartFromRange<Ns...>::type;
};

//template<uint8_t I, uint8_t...Ns>
//struct SmartFromRange<I, Ns...> {
//    using type = SmartSequence<I, typename SmartFromRange<Ns...>::type>;
//};

template <template<typename> typename W, typename T, uint8_t I, uint8_t... Ns>
struct SmartFromRange<W, T, I, Ns...> {
    using type = typename SmartFromRangeHelper<W, T, W<typename std::tuple_element<I, T>::type>::value, I, Ns...>::type;
};
*/

/*
template<uint8_t HEAD, uint8_t... Ns>
struct SmartSequence {
    constexpr static uint8_t head = HEAD;
    constexpr static uint8_t tail[] = { Ns... };
};

template<>
struct SmartSequence<> { };
*/

/*
template<typename TAIL, uint8_t HEAD>
struct SmartSequence<TAIL, HEAD> {
    static const uint8_t head = HEAD;
    using tail = TAIL;
};
*/

//template <template<typename> typename W, typename T, typename S, uint8_t...Ns>
//struct GenSmartSequence;

//template <template<typename> typename W, bool B, typename T, typename S>
//struct GenSmartSequenceHelper;

//template <template<typename> typename W, typename T, typename S, uint8_t...Ns>
//struct GenSmartSequence {
//    using type = typename GenSmartSequence<W, T, I - 1, I - 1, Ns...>::type;
//};
/*
// Recursion case
template <template<typename> typename W, typename T, typename IS, typename OS>
struct GenSmartSequence
{
    using type = typename GenSmartSequence<W, T, typename IS::tail, SmartSequence<IS::head, OS>>::type;
};

// Recursion abort
template <template<typename> typename W, typename T, typename OS>
struct GenSmartSequence<W, T, SmartSequence<>, OS>
{
    using type = OS;
};
*/

//template <template<typename> typename W, bool B, typename T, typename S>
//struct GenSmartSequenceHelper {

//}

/*
template<typename...>

template<typename T>
void print_smart_sequence(){
    std::cout << (int)(T::head) << ", ";
    print_smart_sequence<typename T::tail>();
}
template<>
void print_smart_sequence<TestSeq<>>() {}

template<uint8_t...Ns>
void print_smart_sequence2() {
    print_smart_sequence<TestSeq<Ns...>>();
}
*/

template<typename T>
void print_smart_sequence(T) {
    std::cout << (int)T::head << ", ";
    print_smart_sequence(typename T::tail());
}
template<>
void print_smart_sequence(TestSeq<>) { }






template<>
class CpInterface::Print<TD> {
public:
    static bool const value = true;
    static void invoke(TD& t) {
        std::cout << "TD: DO SOMETHING MAGICAL! " << t.x_ << std::endl;
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

    //std::cout << typeid(SmartSequence<>).name() << std::endl;

    auto long_tuple = std::make_tuple(TA(0),TB(1),TC(2),TA(3),TB(4),TB(5),TD(6),TB(7),TA(8));

    //print_sequence(SequenceGen::TupleCondition<is_TB, decltype(long_tuple)>());

    //print_sequence(SequenceGen::Strip<is_TB, decltype(long_tuple), >>());

    //print_smart_sequence<SmartSequence<0, 4, 2, 5, 7, 2>>();

    //print_smart_sequence(TestSeqHelper<is_TB, decltype(long_tuple), TestSeq<0, 2, 4, 5, 6>>::type());

    //print_smart_sequence<3, 4, 5, 6, 2, 5>();

    //print_smart_sequence(TestSeqCpy<TestSeq<0, 4, 2, 5, 7, 2>>::type());

    //std::cout << typeid(SequenceGen::FromRangeHelper<5>::type).name() << std::endl;


    //std::cout << name_tbl[test_struct<4, 5, 2, 5>::value] << std::endl;
    //std::cout << name_tbl[test_struct<Sequence<3, 4, 2, 5>>::value] << std::endl;

    std::cout << std::endl;

    using type1 = TestSeq<0, 2, 4, 5, 6>;
    using type2 = TestSeqCpy<is_TB, decltype(long_tuple), type1>::type;
    using type3 = MakeSeq<12>::type;

    print_smart_sequence(type1()); std::cout << std::endl;
    print_smart_sequence(type2()); std::cout << std::endl;
    print_smart_sequence(type3()); std::cout << std::endl;


    TT tt = std::make_tuple(1, 4, 6, 4);

    //print_sequence( SequenceGen::FromTupleCondition<DoThing::Print, TT>() );   std::cout << std::endl;

    //print_sequence( Sequence<0,1,2,3>() );   std::cout << std::endl;

    // Huzzah!!! this is ok!

    //DoThing::loop<DoThing::Print>( tt, SequenceGen::FromTupleRange<decltype(tt)>() );

    CpInterface::loop<CpInterface::Print>(tt);

    Any test_any[4] = {
        std::get<0>(tt),
        std::get<1>(tt),
        std::get<2>(tt),
        std::get<3>(tt)
    };

    for (int i = 0; i < 4; ++i) {
        test_any[i]->print();
    }


    MyState state;



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


    //MyState state;

    //state.do_thing();

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
