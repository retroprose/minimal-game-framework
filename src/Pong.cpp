

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

/*
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
void test_func(Signature::Id id) {
    std::cout << "RAN ID\n";
}

void test_func(const Signature& signature) {
    std::cout << "RAN SIGNATURE\n";
}

void print_sig(const Signature& s) {
    for (auto& cp : s) {
        std::cout << (int)cp << ", ";
    }
    std::cout << std::endl;
}
*/

/*
template <int ... Ns> struct sequence {};
using sequence_t = sequence<0, 1, 2, 3, 4, 5>;

template<uint8_t... C, class T = sequence<C...>>
void printSeq() {
    std::cout << sizeof...(C) << std::endl;
}


template<std::size_t SIZE>
struct billy {
    std::size_t size = SIZE;
};

template <template<std::size_t> typename T, std::size_t K>
std::size_t extractSize(const T<K>&) {
    return K;
}
*/

/*
    abstract

    default

    derived<T>

    template<T>
    final : public derived<T>

*/

struct PPack {
    std::string text;
};

class Abstract {
protected:
    virtual void print_(const PPack& p) = 0;

public:
    void print(const PPack& p) { this->print_(p); }

};

class Default : public Abstract {
protected:
    void print_(const PPack& p) { std::cout << "I DONT GET RUN!" << std::endl; }

public:
    enum FuncId : uint32_t {
        Print = 0
    };

    void print(const PPack& p) { std::cout << "I AM DEFUALT!" << std::endl; }

};


// specialize this class!
template<typename T>
class Derived : public Default {
public:
    Derived(T& t) { }

};

template<typename T>
class Leaf : public Derived<T> {
private:
    virtual void print_(const PPack& p) { this->print(p); }

public:
    Leaf(T& t) : Derived<T>(t) { }

    using func_tuple_type = std::tuple<
        decltype(&Leaf::print)
    >;
    constexpr static const func_tuple_type func_tuple = std::make_tuple(
        &Leaf::print
    );

};

template<typename T>
constexpr const typename Leaf<T>::func_tuple_type Leaf<T>::func_tuple;


class Any {
private:
    constexpr static const size_t MaxBuffer = 8;
    uint8_t buffer_[MaxBuffer];

    Abstract* base() { reinterpret_cast<Abstract*>(buffer_); }
public:
    Any() {
        ASSERT(false);
    }

    template<typename T>
    Any(T& t) {
        ASSERT(sizeof(Leaf<T>) <= MaxBuffer);
        Abstract* d = new (buffer_) Leaf<T>(t);
        ASSERT(d - reinterpret_cast<Abstract*>(buffer_) == 0);
    }

    //Abstract* operator->() {
    //    return reinterpret_cast<Abstract*>(buffer_);
    //}

    void print(const PPack& p) { base()->print(p); }
};


template<>
class Derived<int> : public Default {
private:
    int& r_;

public:
    Derived(int& r) : r_(r) { }

    void print(const PPack& p) { std::cout << "I AM AN INT! " << r_ << " " << p.text << std::endl; }
};

template<>
class Derived<float> : public Default {
private:
    float& r_;

public:
    Derived(float& r) : r_(r) { }

    void print(const PPack& p) { std::cout << "I AM AN FLOAT! " << r_ << std::endl; }
};

template<>
class Derived<char> : public Default {
public:
    Derived(char& t) { }

};

template<typename T>
Leaf<T> make_leaf(T& t) {
    return Leaf<T>(t);
}

template<uint32_t FUNC, typename T, typename P>
void call_func(T& t, const P& p) {
    ((Leaf<T>(t)).*(std::get<FUNC>(Leaf<T>::func_tuple)))(p);
}


template<uint8_t... Args>
struct int8_sequence { };

using int8_seq = int8_sequence<2,3,1>;

/*
template<typename T>
void t_print(T& t, const PPack& p) {
    std::cout << p.text << std::endl;
    std::cout << "DEFAULT, NOT CALLED" << std::endl;
}

template<typename T, typename X = typename typeof()>
void t_print(T& t, const PPack& p) {
    std::cout << p.text << std::endl;
    t.print(p);
}
*/


//template<typename T>
//auto t_print(T& t, const PPack& p) -> decltype(t.print(p), void()) {
//}

/*
#define HAS_MEM_FUNC(func, name)                                        \
    template<typename T, typename Sign>                                 \
    struct name {                                                       \
        typedef char yes[1];                                            \
        typedef char no [2];                                            \
        template <typename U, U> struct type_check;                     \
        template <typename _1> static yes &chk(type_check<Sign, &_1::func > *); \
        template <typename   > static no  &chk(...);                    \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes);     \
    }

template<bool C, typename T = void>
struct enable_if {
  typedef T type;
};

template<typename T>
struct enable_if<false, T> { };

HAS_MEM_FUNC(toString, has_to_string);

template<typename T>
typename enable_if< has_to_string<T, std::string(T::*)()>::value, std::string >::type
doSomething(T * t) {
   // something when T has toString ...
   return t->toString();
}

template<typename T>
typename enable_if<!has_to_string<T, std::string(T::*)()>::value, std::string>::type
doSomething(T * t) {
   // something when T doesnt have toString ...
   return "T::toString() does not exist.";
}
*/

/*
template<typename T>                                void printSeq(T& t) { }
template<uint8_t C, uint8_t ... Args, typename T>   void printSeq(T& t) {
    PPack p;
    p.text = "TEST IT OUT!";
    make_leaf(std::get<C>(t)).print(p);
    printSeq<Args...>(t);
}



template<typename T, uint8_t...Args>
void do_something(T t, int8_sequence<Args...>) {
    printSeq<Args...>(t);
}
*/




template<typename T>
void do_something(T& t, int8_sequence<>) { }
template<typename T, uint8_t C, uint8_t...Args>
void do_something(T& t, int8_sequence<C, Args...>) {
    PPack p;
    p.text = "TEST IT OUT!";
    make_leaf(std::get<C>(t)).print(p);
    do_something(t, int8_sequence<Args...>());
}


struct TA { TA(int x) : x_(x) {} int x_; void print() { std::cout << "TA: " << x_ << std::endl; } };
struct TB { TB(int x) : x_(x) {} int x_; void print(int y) { std::cout << "TA: " << x_ << ", " << y << std::endl; } };
struct TC { TC(int x) : x_(x) {} int x_; void print() { std::cout << "TC: " << x_ << std::endl; } };
struct TD { TD(int x) : x_(x) {} int x_;  };
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





// This is the type which holds sequences
template <int ... Ns> struct sequence {};

// First define the template signature
template <int ... Ns> struct seq_gen;

// Recursion case
template <int I, int ... Ns>
struct seq_gen<I, Ns...>
{
    // Take front most number of sequence,
    // decrement it, and prepend it twice.
    // First I - 1 goes into the counter,
    // Second I - 1 goes into the sequence.
    using type = typename seq_gen<I - 1, I - 1, Ns...>::type;
};

// Recursion abort
template <int ... Ns>
struct seq_gen<0, Ns...>
{
    using type = sequence<Ns...>;
};


template <int N>
using sequence_t = typename seq_gen<N>::type;


/*
// THIS WORKS!

// First define the template signature
template <typename T, int ... Ns> struct seq_gen_2;
template <typename T, bool ADD, int I, int ... Ns> struct sequence_gen_helper;

// Recursion case
template <typename T, int I, int ... Ns>
struct seq_gen_2<T, I, Ns...>
{
    // Take front most number of sequence,
    // decrement it, and prepend it twice.
    // First I - 1 goes into the counter,
    // Second I - 1 goes into the sequence.
    using type = typename sequence_gen_helper<T, is_in<typename std::tuple_element<I - 1, T>::type>::value, I, Ns...>::type;
};

// Recursion abort
template <typename T, int ... Ns>
struct seq_gen_2<T, 0, Ns...>
{
    using type = sequence<Ns...>;
};

template <typename T, int I, int ... Ns>
struct sequence_gen_helper<T, true, I, Ns...> {
    using type = typename seq_gen_2<T, I - 1, I - 1, Ns...>::type;
};

template <typename T, int I, int ... Ns>
struct sequence_gen_helper<T, false, I, Ns...> {
    using type = typename seq_gen_2<T, I - 1, Ns...>::type;
};

template <typename T>
using sequence_t_2 = typename seq_gen_2<T, std::tuple_size<T>::value>::type;
*/


// First define the template signature
template <template<typename> typename W, typename T, int ... Ns > struct seq_gen_2;
template <template<typename> typename W, typename T, bool ADD, int I, int ... Ns> struct sequence_gen_helper;

// Recursion case
template <template<typename> typename W, typename T, int I, int ... Ns>
struct seq_gen_2<W, T, I, Ns...>
{
    // Take front most number of sequence,
    // decrement it, and prepend it twice.
    // First I - 1 goes into the counter,
    // Second I - 1 goes into the sequence.
    using type = typename sequence_gen_helper<W, T, W<typename std::tuple_element<I - 1, T>::type>::value, I, Ns...>::type;
};

// Recursion abort
template <template<typename> typename W, typename T, int ... Ns>
struct seq_gen_2<W, T, 0, Ns...>
{
    using type = sequence<Ns...>;
};

template <template<typename> typename W, typename T, int I, int ... Ns>
struct sequence_gen_helper<W, T, true, I, Ns...> {
    using type = typename seq_gen_2<W, T, I - 1, I - 1, Ns...>::type;
};

template <template<typename> typename W, typename T, int I, int ... Ns>
struct sequence_gen_helper<W, T, false, I, Ns...> {
    using type = typename seq_gen_2<W, T, I - 1, Ns...>::type;
};

template<template<typename> typename W, typename T>
using sequence_t_2 = typename seq_gen_2<W, T, std::tuple_size<T>::value>::type;







template<typename T>
using print_ptr = void (T::*)();

template<typename T>
struct has_print {
    template <typename U, U> struct type_check;
    template <typename _1> static uint16_t &chk(type_check<print_ptr<_1>, &_1::print> *);
    //template <typename _1> static uint16_t &chk(type_check<decltype(&_1::print), &_1::print> *);
    //template <typename _1> static uint16_t &chk(type_check<decltype(&_1::print), &_1::print> *);
    //template <typename _1> static uint16_t &chk(void (_1::*)() = &_1::print);
    template <typename   > static  uint8_t &chk(...);
    static bool const value = sizeof(chk<T>(0)) == sizeof(uint16_t);
};


template<bool C, typename T = void>
struct enable_if {
  typedef T type;
};

template<typename T>
struct enable_if<false, T> { };


template<typename T>
typename enable_if<has_print<T>::value/*, return type*/>::type doPrint(T& t) {
    t.print(); std::cout << std::endl;
}

template<typename T>
typename enable_if<!has_print<T>::value/*, return type*/>::type doPrint(T& t) {
    std::cout << "DEFAULT!" << std::endl;
}

template<typename T>
struct doPrintStruct {
    static void invoke(T& t){
        doPrint<T>(t);
    }
};





template<template<typename> typename F, typename T>
void doLoop(T& t, sequence<>) { }
template<template<typename> typename F, typename T, int C, int...Args>
void doLoop(T& t, sequence<C, Args...>) {
    //F<T>(std::get<C>(t));
    F<typename std::tuple_element<C, T>::type>::invoke(std::get<C>(t));
    //doPrint(std::get<C>(t));
    doLoop<F>(t, sequence<Args...>());
}




template<bool DUMMY = false>
void print_sequence(sequence<>) { }
template<bool DUMMY = false, int C, int...Args>
void print_sequence(sequence<C, Args...>) {
    std::cout << (int)C << ", ";
    print_sequence(sequence<Args...>());
}

static void func2(int i, float f, double d)
{
    std::cout << i << ", " << f << ", " << d << '\n';
}

static void func(double d, float f, int i)
{
    std::cout << d << ", " << f << ", " << i << '\n';
}

// The following code passes all parameters by
// value, for the sake of simplicity
template <typename F, typename TUP, int ... INDICES>
static void tuple_call_(F f, TUP tup, sequence<INDICES...>)
{
    f(std::get<INDICES>(tup) ...);
}

//template <typename F, typename ... Ts>
//static void tuple_call(F f, std::tuple<Ts...> tup)
//{
//    tuple_call_(f, tup, sequence_t<sizeof...(Ts)>{});
//}


template<typename T>
void is_in_loop(T& t) { }
template<int C, int...Args, typename T>
void is_in_loop(T& t) {
    std::cout << (int)(is_in<typename std::tuple_element<C, T>::type>::value) << ", ";
    is_in_loop<Args...>(t);
}


/*
using test_tuple_type = std::tuple<int, float, char>;

auto test_tuple = std::make_tuple(12, 4.5, 'g');

template<typename T>
generic_function(T& t) {
    std::cout << typeid(T).name() << ": " << t << "  GENERIC!" << std::endl;
}

template<typename T>
generic_function2(T& t) {
    std::cout << typeid(T).name() << ": " << t << "  GENERIC NO 2!" << std::endl;
}


template<typename F, uint8_t C, typename T>
void printSeq(T& t) {
    F( std::get<C>(t) );
}

template<typename F, uint8_t C1, uint8_t C2, uint8_t ... Args, typename T>
void printSeq(T& t) {
    F( std::get<C1>(t) );
    printSeq<C2, Args...>(t);
}
*/



//template<sequence<uint8_t C1, uint8_t C2, uint8_t ... Args>>
//template<uint8_t C1, uint8_t C2, uint8_t ... Args>
//void printSeq() {
//    std::cout << (int)C1 << std::endl;
//    printSeq<sequence<C2, Args...>>();
//}

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    //func(1.0, 2.0, 3);

    std::tuple<double, float, int> tup {1.0, 2.0, 3};
    //tuple_call(func, tup); // same effect

    //tuple_call_(func2, tup, sequence<2,1,0>());

    //print_sequence( sequence_t_2<is_in, TT>() );    std::cout << std::endl;
    //print_sequence( sequence_t_2<is_out, TT>() );   std::cout << std::endl;

    //print_sequence( sequence_t_2<has_print, TT>() );   std::cout << std::endl;



    PPack p;
    p.text = "HELLO WORLD";

    int a = 4;
    int b = 5;
    float c = 1.5;
    char d = 'f';

    using test_tuple_type = decltype(std::make_tuple(4, 5, 1.5f, 'f'));
    auto test_tuple = std::make_tuple(4, 5, 1.5f, 'f');

    TT tt = std::make_tuple(1, 4, 6, 4);

    //doPrintLoop(tt, sequence<0, 1, 2, 3>());

    //doLoop<doPrintStruct>(tt, sequence<0, 1, 2, 3>());

    doLoop<doPrintStruct>(tt, sequence<0, 1, 2, 3>());

    //do_something(test_tuple, int8_sequence<2,1,3>());

    //print_sequence( sequence<2,1,3>() );
    //is_in_loop<0, 1, 2, 3>(tt);

    //t_print(std::get<0>(tt), p);
    //t_print(std::get<1>(tt), p);
    //t_print(std::get<2>(tt), p);
    //t_print(std::get<3>(tt), p);

    auto test_a = make_leaf(a);
    auto test_b = make_leaf(b);
    auto test_c = make_leaf(c);
    auto test_d = make_leaf(d);

    //std::cout << "test a: "; test_a.print(p); std::cout << std::endl;
    //std::cout << "test b: "; test_b.print(p); std::cout << std::endl;
    //std::cout << "test c: "; test_c.print(p); std::cout << std::endl;
    //std::cout << "test d: "; test_d.print(p); std::cout << std::endl;

    //std::cout << "test a: "; call_func<Default::Print>(a, p); std::cout << std::endl;
    //std::cout << "test b: "; call_func<Default::Print>(b, p); std::cout << std::endl;
    //std::cout << "test c: "; call_func<Default::Print>(c, p); std::cout << std::endl;
    //std::cout << "test d: "; call_func<Default::Print>(d, p); std::cout << std::endl;

    Any test_any[4] = { Any(a), Any(b), Any(c), Any(d) };

    for (int i = 0; i < 4; ++i) {
        //std::cout << "test any: "; test_any[i].print(p); std::cout << std::endl;
    }

    //printSeq(test_tuple, int8_sequence<2,1,3>());

    //do_something(test_tuple, int8_seq());

    //printSeq<int,0,2,1>(test_tuple, args);

    MyState state;


   //printSeq<sequence<3>>();


    //state.test_loop();


    //std::cout << MyState::activeIndex << " active, entity " << MyState::entityIndex << std::endl;


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
