#include <algorithm>
#include <vector>
#include <iostream>

#include "raylib.h"
#include "raymath.h"

#include "Game.h"
#include "Sprite.h"
#include "Menu.h"

void Game::DrawSprites() {
    for (size_t i = 0; i < sprites.size(); i++) {
        sprites[i]->Draw();
    }
}

void Game::MoveSprites() {
    for (size_t i = 0; i < sprites.size(); i++) {
        sprites[i]->Move();
    }
    auto removed = remove_if(sprites.begin(), sprites.end(), [](Sprite* sprite) { return sprite->state == Sprite::State::FINISHED; });
    std::for_each(removed, sprites.end(), [&](Sprite* s) { 
        printf("REMOVE sprite:%d\n", s->type);
        auto& v = spriteMap[s->type];
        auto vRemoved = remove_if(v.begin(), v.end(), [&](Sprite* sprite) { return sprite == s; });
        v.erase(vRemoved, vRemoved);
        delete s;
    });
    sprites.erase(removed, removed);
}

void Game::CheckCollision() {
    // Collision Player<->Enemy
    for (auto enemy : spriteMap[Sprite::Type::ENEMY]) {
        if (IsCollision(player, enemy)) {
            // std::cout << "PLAYER collided with ENEMY:" << std::endl;
            player->Collision(enemy);
        }
    }

    // Collision Player<->Block
    for (auto block : spriteMap[Sprite::Type::BLOCK]) {
        if (IsCollision(player, block)) {
            // std::cout << "PLAYER collided with BLOCK:" << std::endl;
            player->Collision(block);
        }
    }

    // Collision  Player<->Ladder
    for (auto ladder : spriteMap[Sprite::Type::LADDER]) {
        if (IsCollision(player, ladder)) {
            // std::cout << "PLAYER collided with LADDER:" << std::endl;
            player->Collision(ladder);
        }
    }

    // Collision Weapon<->Enemy
    for (auto weapon : spriteMap[Sprite::Type::WEAPON]) {
        for (auto enemy : spriteMap[Sprite::Type::ENEMY]) {
            if (IsCollision(weapon, enemy)) {
                // std::cout << "WEAPON collided with ENEMY:" << std::endl;
                weapon->Collision(enemy);
                enemy->Collision(weapon);
            }
        }
    }

    // Collision Weapon<->Block
    for (auto weapon : spriteMap[Sprite::Type::WEAPON]) {
        for (auto block : spriteMap[Sprite::Type::BLOCK]) {
            if (IsCollision(weapon, block)) {
                // std::cout << "WEAPON collided with BLOCK:" << std::endl;
                weapon->Collision(block);
            }
        }
    }

    // Collision Enemy<->Block
    for (auto enemy : spriteMap[Sprite::Type::ENEMY]) {
        for (auto block : spriteMap[Sprite::Type::BLOCK]) {
            if (IsCollision(enemy, block)) {
                // std::cout << "ENEMY collided with BLOCK:" << std::endl;
                // FIXME: Now implemented in Enemy::checkCollision
                enemy->Collision(block);
            }
        }
    }
}

Rectangle Game::getPlayerPosition() {
    for (auto sprite : sprites) {
        if (sprite->type == Sprite::Type::PLAYER) {
            return sprite->position.rectangle;
        }
    }
    return { 0, 0, 0, 0 };
}

void Game::AddEnemy(float x, float y, Enemy::Kind kind, int heading) {
    Sprite* s = Enemy::create(this, x, y, kind, heading);
    spriteMap[Sprite::Type::ENEMY].push_back(s);
    sprites.push_back(s);
}

void Game::AddWeapon(float x, float y) {
    auto w = spriteMap.find(Sprite::Type::WEAPON);
    if (w != spriteMap.end() && !w->second.empty()) return;
    weapon = new Weapon(this, x, y, 20, 0, PURPLE);
    spriteMap[Sprite::Type::WEAPON].push_back(weapon);
    sprites.push_back(weapon);
}

void Game::AddScore(int score) {
    this->score += score;
}

Game::Game() : menu(this, {"Continue", "Break", "Restart", "Quit"}) {}

Game::~Game() {
    std::for_each(begin(sprites), end(sprites), [](Sprite* s) { delete s; });
}

void Game::Spawn() {
    Sprite* s = new Block(this, 0, 0, screenWidth, wallThickness, BLACK);
    spriteMap[Sprite::Type::BLOCK].push_back(s);
    sprites.push_back(s);
    s = new Block(this, 0, screenHeight - wallThickness, screenWidth, wallThickness, GRAY);
    spriteMap[Sprite::Type::BLOCK].push_back(s);
    sprites.push_back(s);
    s = new Block(this, 0, 0, wallThickness, screenHeight, BLACK);
    spriteMap[Sprite::Type::BLOCK].push_back(s);
    sprites.push_back(s);
    s = new Block(this, screenWidth - wallThickness, 0, wallThickness, screenHeight, BLACK);
    spriteMap[Sprite::Type::BLOCK].push_back(s);
    sprites.push_back(s);
    s = new Block(this, 500, 500, 150, wallThickness, RED);
    spriteMap[Sprite::Type::BLOCK].push_back(s);
    sprites.push_back(s);
    s = new Block(this, 200, 400, 150, wallThickness, RED);
    spriteMap[Sprite::Type::BLOCK].push_back(s);
    sprites.push_back(s);
    s = new Block(this, 550, 400, 150, wallThickness, RED);
    spriteMap[Sprite::Type::BLOCK].push_back(s);
    sprites.push_back(s);

    s = new Ladder(this, 700, 390, 30, 200, ORANGE);
    spriteMap[Sprite::Type::LADDER].push_back(s);
    sprites.push_back(s);

    s = Enemy::create(this, 400, 200, Enemy::Kind::BALL1, 1);
    spriteMap[Sprite::Type::ENEMY].push_back(s);
    sprites.push_back(s);
    // s = Enemy::create(this, 100, 200, Enemy::Kind::BALL1, 1);
    // spriteMap[Sprite::Type::ENEMY].push_back(s);
    // sprites.push_back(s);

    // weapon = new Weapon(this, 0, 0, 20, 0, PURPLE);
    // sprites.push_back(weapon);

    player = new Player(this, 400, screenHeight - wallThickness - 64, 35, 64, BLACK, {0, 0});
    sprites.push_back(player);
}

void Game::Draw() {
    // Draw
    BeginDrawing();

    ClearBackground(RAYWHITE);

    // DrawText("PRESS SPACE to PAUSE BALL MOVEMENT", 10, GetScreenHeight() - 25,
    // 20, LIGHTGRAY);

    DrawSprites();
    DrawText(TextFormat("Score: %03i", score), 100, wallThickness / 2, 20, GREEN);

    // On pause, we draw a blinking message
    // if (pause && ((framesCounter / 30) % 2)) DrawText("PAUSED", 350, 200, 30,
    // GRAY);

    if (pause) {
        menu.Draw();
    }

    DrawFPS(10, 10);

    EndDrawing();
}

void Game::Update() {
    frameCounter++;

    if (GetKeyPressed() == KEY_ESCAPE)
        pause = !pause;

    if (pause) {
        menu.Update();
    } else {
        MoveSprites();
    }
}

void Game::OnMenu(std::string name) {
    std::cout << "Menu selected:" << name << std::endl;
    if (name == "Continue") {
        pause = false;
    } else if (name == "Break") {
    } else if (name == "Restart") {
    } else if (name == "Quit") {
        quitSelected = true;
    }
}

int Game::MainLoop() {
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Pang");
    SetExitKey(KEY_F10);
    SetTargetFPS(60);

    Spawn();

    while (!WindowShouldClose() && !quitSelected) {
        Update();
        CheckCollision();
        Draw();
    }

    // De-Initialization
    //---------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------

    return 0;
}
