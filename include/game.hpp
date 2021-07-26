#ifndef GAME_HPP
#define GAME_HPP

#include <components.hpp>

class Game {
public:

    // collision table
    // animation table

    // constant data


    Table<Slot> slots;

    GlobalState global;
    MersenneTwister rand;
    Cp components;

    // Table<Bounds> boundList;
    // Table<Event> eventList;

    // Targets (not used!)

    bool gameOver;


    Game() {
        slots.resize(64);
        // constant = static const
        // setupCollisionTable();
    }

    // register animation, register collision

    void setupCollisionTable() {

    }

    void smartCopy(const Game& other) {
        slots.smartCopy(other.slots);
        rand = other.rand;
        global = other.global;
        // components.smartCopy(components);
    }

    void setInput() {
        // set inputs
    }

    void init(uint32_t seed) {

    }

    void update() {
        updateAnimators();
        updatePlayers();
        updateEnemies();
        integrate();
        resolveState();
        fillContactList();
        resolveEvents();
    }

    void fastForward() {
        updateAnimators();
        updatePlayers();
        updateEnemies();
        integrate();
        resolveState();
    }

    void resolveEvents() {

    }

    void resolveState() {

    }

    void fillContactList() {

    }

    void integrate() {

    }

    void updateAnimators() {

    }

    void updateEnemies() {

    }

    void updatePlayers() {

    }



private:



};





#endif // GAME_HPP
