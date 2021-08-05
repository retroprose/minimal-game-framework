#ifndef GAME_HPP
#define GAME_HPP

#include <components.hpp>
#include <map>

#include <fstream>

class Data {
public:
    // other constants
    int32_t enemyCount = 11;

    Scaler textAnimateCounter = 0.00833333333333333f;

    enum Images : uint16_t {
        _null = 0,
        _null_persist,
        enemy_00_a,
        enemy_01_a,
        enemy_02_a,
        enemy_03_a,
        enemy_04_a,
        enemy_05_a,
        enemy_06_a,
        enemy_07_a,
        enemy_08_a,
        enemy_09_a,
        enemy_10_a,
        enemy_00_b,
        enemy_01_b,
        enemy_02_b,
        enemy_03_b,
        enemy_04_b,
        enemy_05_b,
        enemy_06_b,
        enemy_07_b,
        enemy_08_b,
        enemy_09_b,
        enemy_10_b,
        player_ship_0,
        player_ship_1,
        player_shot,
        enemy_shot,
        easy_0,
        easy_1,
        player_boom_0,
        player_boom_1,
        player_boom_2,
        player_boom_3,
        player_boom_4,
        player_boom_5,
        player_boom_6,
        enemy_boom_0,
        enemy_boom_1,
        enemy_boom_2,
        enemy_boom_3,
        enemy_boom_4,
        enemy_boom_5,
        enemy_boom_6,
        local_player_0,
        local_player_1,
        text_ready,
        text_no,
        text_great,
        target,
        image_count,
        _end_list
    };

    static Data& GetGlobal() {
        static Data data;
        Cp::Prefab p;

        // shot cleaner
        p.comp.flags = Cf::Active | Cf::Component | Cf::Body | Cf::ObjectId,
        p.objectId = ObjType::ShotCleaner,
        p.body.position = Vector2(0, 0);
        p.body.velocity = Vector2(0, 0);
        p.body.size = Vector2(960, 540);  // 960, 540
        data.prefabs[p.objectId] = p;

        // player
        p.comp.flags = Cf::Active | Cf::Component | Cf::Body | Cf::ObjectId | Cf::Player | Cf::Animator,
        p.objectId = ObjType::Player,
        p.animator.frame = player_ship_0;
        p.animator.count = 0;
        p.player.slot = -1;
        p.player.delayFire = 0;
        p.player.damage = 0;
        p.body.position = Vector2(0, 0);
        p.body.velocity = Vector2(0, 0);
        p.body.size = Vector2(16, 10);  // 16, 10
        data.prefabs[p.objectId] = p;

        // enemy
        p.comp.flags = Cf::Active | Cf::Component | Cf::Body | Cf::ObjectId | Cf::Enemy | Cf::Animator,
        p.objectId = ObjType::Enemy,
        p.enemy.direction = 1;
        p.enemy.counter = 0;
        p.enemy.delayFire = 0;
        p.animator.frame = player_ship_0;
        p.animator.count = 0;
        p.body.position = Vector2(0, 0);
        p.body.velocity = Vector2(0, 0);
        p.body.size = Vector2(16, 10);  // 16, 10
        data.prefabs[p.objectId] = p;

        // bullet
        p.comp.flags = Cf::Active | Cf::Component | Cf::Body | Cf::ObjectId | Cf::Animator,
        p.objectId = ObjType::Bullet,
        p.animator.frame = player_shot;
        p.animator.count = 0;
        p.body.position = Vector2(0, 0);
        p.body.velocity = Vector2(0, 16);  // 0, 16
        p.body.size = Vector2(12, 20);  // 16, 10
        data.prefabs[p.objectId] = p;

        // bad bullet
        p.comp.flags = Cf::Active | Cf::Component | Cf::Body | Cf::ObjectId | Cf::Animator,
        p.objectId = ObjType::BadBullet,
        p.animator.frame = enemy_shot;
        p.animator.count = 0;
        p.body.position = Vector2(0, 0);
        p.body.velocity = Vector2(0, -8);  // 0, -8
        p.body.size = Vector2(7, 7);  // 7, 7
        data.prefabs[p.objectId] = p;

        // boom
        p.comp.flags = Cf::Active | Cf::Component | Cf::Body | Cf::ObjectId | Cf::Animator,
        p.objectId = ObjType::Boom,
        p.animator.frame = enemy_boom_0;
        p.animator.count = 0;
        p.body.position = Vector2(0, 0);
        p.body.velocity = Vector2(0, 0);
        p.body.size = Vector2(14, 14);  // 14, 14
        data.prefabs[p.objectId] = p;

        // player boom
        p.comp.flags = Cf::Active | Cf::Component | Cf::Body | Cf::ObjectId | Cf::Animator,
        p.objectId = ObjType::PlayerBoom,
        p.animator.frame = player_boom_0;
        p.animator.count = 0;
        p.body.position = Vector2(0, 0);
        p.body.velocity = Vector2(0, 0);
        p.body.size = Vector2(22, 21);  // 22, 21
        data.prefabs[p.objectId] = p;

        return data;
    }

    Cp::Prefab& prefab(uint8_t i) {
        return prefabs[i];
    }

private:
    typedef std::map<uint8_t, Cp::Prefab> prefab_map;
    prefab_map prefabs;

};



class Game {
public:

    // function pointer!
    typedef void (*collision_function_type)(Game& g, Event& e);
    typedef std::map<uint16_t, collision_function_type> collision_table_type;
    typedef std::map<uint16_t, uint16_t> animation_table_type;

    collision_table_type collision_table;
    animation_table_type animation_table;


    Data& data;

    Table<Slot> slots;

    GlobalState global;
    MersenneTwister rand;
    Cp components;

    Table<Bounds> boundList;
    Table<Event> eventList;

    // Targets (not used!)

    bool gameOver;


    Game() : data( Data::GetGlobal() ) {
        slots.resize(64);
        setupTables();
    }

    void smartCopy(const Game& other) {
        slots.smartCopy(other.slots);
        rand = other.rand;
        global = other.global;
        components.smartCopy(other.components);
    }

    void setInput() {
        // set inputs
    }

    void setInputEasy(Slot* list) {
        // set without converting bits!
        for (int32_t i = 0; i < 64; ++i) {
            slots[i] = list[i];
        }
    }

    void init(uint32_t seed) {
        rand.setSeed(seed);

        gameOver = false; // do I need this now?

        global.playing = false;
        global.enemySpeed = 3; // 3
        global.enemyCount = 0;
        global.textType = Data::text_ready;
        global.textAnimate = 0;

        components.clear();

        eventList.clear();
        for (int32_t j = 0; j < slots.size(); ++j) {
            if (slots[j].connected == true /*&& connectedAtStart[j] == true*/) {
                if (eventList.append() == true) {
                    eventList.last().createPlayer(j, Vector2(j * 60 - 960 + 32, -500)); // recompute!
                }
            }
        }

        // shot cleaners make sure shots don't last forever
        if (eventList.append() == true) {
            eventList.last().createEntity(ObjType::ShotCleaner, Vector2(0, 1090)); // recompute
        }

        if (eventList.append() == true) {
            eventList.last().createEntity(ObjType::ShotCleaner, Vector2(0, -1090)); // recompute
        }

        resolveEvents();
    }


    // register animation, register collision

    static void shotCleanerVsBullet(Game& g, Event& e) {
        // do colision betweem e.A bullet, and e.B enemy!
        auto B = g.components.get(e.B);
        if (B.animator.frame != Data::_null)
        {
            B.animator.frame = Data::_null;
        }
    }

    static void shotCleanerVsBadBullet(Game& g, Event& e) {
        // do colision betweem e.A bullet, and e.B enemy!
        auto B = g.components.get(e.B);
        if (B.animator.frame != Data::_null)
        {
            B.animator.frame = Data::_null;
        }
    }

    static void bulletVsEnemy(Game& g, Event& e) {
        // do colision betweem e.A bullet, and e.B enemy!
        auto A = g.components.get(e.A);
        auto B = g.components.get(e.B);
        if (A.animator.frame != Data::_null && B.animator.frame != Data::_null)
        {
            A.animator.frame = Data::_null;
            B.animator.frame = Data::_null;
            if (g.eventList.append() == true)
            {
                g.eventList.last().createEntity(ObjType::Boom, B.body.position);
            }
        }
    }

    static void badBulletVsPlayer(Game& g, Event& e) {
        // do colision betweem e.A bullet, and e.B enemy!
        auto A = g.components.get(e.A);
        auto B = g.components.get(e.B);
        if (A.animator.frame != Data::_null && B.animator.frame != Data::_null_persist)
        {
            A.animator.frame = Data::_null;

            B.player.damage = 100;
            B.animator.frame = Data::_null_persist;

            if (g.eventList.append() == true)
            {
                g.eventList.last().createEntity(ObjType::PlayerBoom, B.body.position);
            }
        }
    }

    void setupTables() {
        // Collisions
        registerCollision(ObjType::ShotCleaner, ObjType::Bullet, &shotCleanerVsBullet);
        registerCollision(ObjType::ShotCleaner, ObjType::BadBullet, &shotCleanerVsBadBullet);
        registerCollision(ObjType::Bullet, ObjType::Enemy, &bulletVsEnemy);
        registerCollision(ObjType::BadBullet, ObjType::Player, &badBulletVsPlayer);

        // Animations
        uint16_t player_boom[] = {
            Data::player_boom_0,
            Data::player_boom_1,
            Data::player_boom_2,
            Data::player_boom_3,
            Data::player_boom_4,
            Data::player_boom_5,
            Data::player_boom_6,
            Data::_null,
            Data::_end_list
        };
        registerAnimation(player_boom);

        uint16_t enemy_boom[] = {
            Data::enemy_boom_0,
            Data::enemy_boom_1,
            Data::enemy_boom_2,
            Data::enemy_boom_3,
            Data::enemy_boom_4,
            Data::enemy_boom_5,
            Data::enemy_boom_6,
            Data::_null,
            Data::_end_list
        };
        registerAnimation(enemy_boom);

        uint16_t player[] = {
            Data::player_ship_0,
            Data::player_ship_1,
            Data::_end_list
        };
        registerAnimation(player);

        uint16_t local_player[] = {
            Data::local_player_0,
            Data::local_player_1,

            Data::_end_list
        };
        registerAnimation(local_player);

        uint16_t enemy[] = {
            Data::_null,
            Data::_null,
            Data::_end_list
        };
        for (int32_t i = 0; i < data.enemyCount; i++) {
            enemy[0] = i + 2;
            enemy[1] = i + 2 + data.enemyCount;
            registerAnimation(enemy);
        }
    }

    void registerCollision(ObjType::Type a, ObjType::Type b, collision_function_type d) {
        if (a > b) {
            //collision_table[  ( (uint16_t) ((a << 8) | b)  ] = d;
            collision_table[(uint16_t)((a << 8) | b)] = d;
        } else {
            //k = (ushort)((b << 8) | a);
            // throw exception!
            //OnMessage($"types reversed! {ao}, {bo}");
            assert(a > b);
        }
    }

    void registerAnimation(uint16_t* list) {
        int32_t i = 0;
        while (list[i] != Data::_end_list) {
            animation_table[list[i]] = list[i + 1];
            ++i;
        }
        uint16_t last = list[i - 1];
        if (last != Data::_null) {
            animation_table[last] = list[0];
        }
    }

    void resolveEvents() {
        for (int32_t event_index = 0; event_index < eventList.size(); ++event_index) {

            Cp& cp = components;
            Event& evt = eventList[event_index];

            switch (evt.id) {
                case EventType::Contact:
                    {
                        auto find_it = collision_table.find(evt.key);
                        if (find_it != collision_table.end()) {
                            if (cp.valid(evt.A) == true && cp.valid(evt.B) == true) {
                                (find_it->second)(*this, evt);
                            }
                        }
                    }
                    break;

                case EventType::DestroyEntity:
                    cp.destroy(evt.A);
                    //OnDestroy();
                    break;

                case EventType::CreateEntity:
                    {
                        // create from prefab!
                        Cp::Prefab prefab = data.prefab(evt.type);
                        Entity entity = cp.create();


                        if (!entity.isNull()) {

                            prefab.setEntity(cp, entity);


                            auto r = cp.get(entity.index());



                            if (r.comp.flags & Cf::Body) {
                                r.body.position = evt.v;

                            }
                            if (r.comp.flags & Cf::Player) {
                                r.player.slot = evt.key;

                            }
                            if (r.comp.flags & Cf::Enemy) {
                                r.enemy.delayFire = (uint16_t)(rand.next() % 2000);
                                //r.enemy.target = Entity();
                                r.animator.frame = (uint16_t)(rand.next(data.enemyCount) + 2);

                            }



                            //OnCreate(entity);
                        }


                    }
                    break;

                default:
                    //OnMessage($"Attempted to process bad event {evt.Id}");
                    assert(false);
                    break;
            }
        }

        eventList.clear();
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

    void resolveState() {
        if (global.playing == false)
        {
            global.textAnimate += data.textAnimateCounter;

            if (global.textAnimate > 1)
            {
                if (global.textType != Data::text_ready)
                {
                    gameOver = true;
                }
                else
                {

                    global.playing = true;

                    // fix all ships
                    //Targets.Clear();


                    for (auto it = components.filter(Cf::Active | Cf::Player); it.moveNext(); )
                    {
                        auto r = it.get();
                        if (slots[r.player.slot].connected == true)
                        {
                            r.player.damage = 0;
                            r.animator.frame = Data::player_ship_0;
                        }
                    }

                    // j 24, i 20
                    // create new set of entities
                    for (int j = 0; j < 24; ++j)
                    {
                        for (int i = 0; i < 20; ++i)
                        {
                            if (eventList.append() == true)
                            {
                                eventList.last().createEntity(ObjType::Enemy, Vector2(j * 60 - 960 + 32, i * 32 - 100));
                            }
                        }
                    }
                }
            }
        }
    }

    void fillContactList() {
        boundList.clear();

        for (auto it = components.filter(Cf::Active | Cf::Body); it.moveNext();)
        {
            auto r = it.get();
            if (boundList.append())
            {
                boundList.last() = Bounds(r.entity, r.objectId, r.body.position, r.body.velocity, r.body.size);
            }
        }

        // sort it here!
        boundList.sort();

        for (int i = 0; i < boundList.size(); ++i)
        {
            Bounds& iter = boundList[i];
            for (int j = i + 1; j < boundList.size(); ++j)
            {
                Bounds& nextIter = boundList[j];
                if (iter.overlap(nextIter) == true)
                {
                    if (eventList.append() == true)
                    {
                        if (iter.type > nextIter.type)
                        {
                            eventList.last().contact((uint16_t)((iter.type << 8) | nextIter.type), iter.entity, nextIter.entity);
                        }
                        else
                        {
                            eventList.last().contact((uint16_t)((nextIter.type << 8) | iter.type), nextIter.entity, iter.entity);
                        }
                    }
                }
            }
        }
    }

    void integrate() {
        for( auto it = components.filter(Cf::Active | Cf::Body); it.moveNext(); ) {
            auto r = it.get();
            r.body.position += r.body.velocity;
        }
    }

    void updateAnimators() {
        for (auto it = components.filter(Cf::Active | Cf::Animator); it.moveNext();)
        {
            auto r = it.get();

            ++r.animator.count;
            if (r.animator.count > 3)
            {
                r.animator.count = 0;
                auto find_it = animation_table.find(r.animator.frame);
                if (find_it != animation_table.end())
                {
                    r.animator.frame = find_it->second;
                }
            }
            if (r.animator.frame == Data::_null)
            {
                if (eventList.append() == true)
                {
                    eventList.last().destroyEntity(r.entity);
                }
            }
        }
    }

    void updateEnemies() {
        global.enemyCount = 0;
        for (auto it = components.filter(Cf::Active | Cf::Body | Cf::Enemy); it.moveNext();)
        {
            auto r = it.get();

            ++r.enemy.counter;
            if (r.enemy.counter > 150)
            {
                r.enemy.counter = 0;
                r.enemy.direction = -r.enemy.direction;
            }

            r.body.velocity.x = r.enemy.direction * global.enemySpeed;

            if (r.enemy.delayFire > 0)
                --r.enemy.delayFire;

            if (r.enemy.delayFire == 0)
            {
                r.enemy.delayFire = 2000;
                if (eventList.append() == true)
                {
                    eventList.last().createEntity(ObjType::BadBullet, r.body.position);
                }
            }

            ++global.enemyCount;

        }
        // calculate enemy speed based on count
        global.enemySpeed = 3;

        // if enemy count is zero, set playing to false, text to great job!
        if (global.playing == true && global.enemyCount == 0)
        {
            global.playing = false;
            global.textAnimate = 0;
            global.textType = Data::text_great;
            // also repair all ships!
            // actually this is done by the reset function!
        }
    }

    void updatePlayers() {
        //Targets.Clear();
        bool livePlayer = false;

        for (auto it = components.filter(Cf::Active | Cf::Body | Cf::Player); it.moveNext();)
        {
            auto r = it.get();

            auto& slot = slots[r.player.slot];

            r.body.velocity.x = 0;
            if (r.player.delayFire > 0)
                --r.player.delayFire;

            if (r.player.damage > 0)
            {
                // DO NOTHING!
            }
            else
            {
                if (slot.connected == true)
                {
                    livePlayer = true;
                }
                else
                {
                    // kill if disconnect!
                    r.player.damage = 100;
                    r.animator.frame = Data::_null_persist;

                    if (eventList.append() == true)
                    {
                        eventList.last().createEntity(ObjType::PlayerBoom, r.body.position);
                    }
                }

                if (slot.input.left == true) r.body.velocity.x = -5;
                if (slot.input.right == true) r.body.velocity.x = 5;

                /*if ( Scaler.Abs(r.body.position.x - slot.Input.X) < Const.v0_00005 )
                {

                }
                else if (slot.Input.X < r.body.position.x)
                {
                    r.body.velocity.x = -5;
                }
                else if (slot.Input.X > r.body.position.x)
                {
                    r.body.velocity.x = 5;
                }*/

                if (slot.input.primary && r.player.delayFire == 0)
                {
                    r.player.delayFire = 24;

                    if (eventList.append() == true)
                    {
                        eventList.last().createEntity(ObjType::Bullet, r.body.position);
                    }
                }
            }



        }

        // if all players are damaged, set playing to false, text to no way! and destroy all enemies
        if (global.playing == true && livePlayer == false)
        {
            global.playing = false;
            global.textAnimate = 0;
            global.textType = Data::text_no;

            for (auto it = components.filter(Cf::Active | Cf::Body | Cf::Enemy); it.moveNext();)
            {
                auto r = it.get();
                if (eventList.append() == true)
                {
                    eventList.last().destroyEntity(r.entity);
                }
            }
        }
    }



private:



};







#endif // GAME_HPP
