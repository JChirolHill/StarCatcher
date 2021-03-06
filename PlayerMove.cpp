//
//  PlayerMove.cpp
//  Game-mac
//
//  Created by J Hill on 9/19/19.
//  Copyright © 2019 Sanjay Madhav. All rights reserved.
//

#include "PlayerMove.h"
#include "Actor.h"
#include "SDL/SDL.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "AnimatedSprite.h"
#include "SpriteComponent.h"
#include "Player.h"
#include "Bag.h"

PlayerMove::PlayerMove(Actor* owner)
: MoveComponent(owner) {
    mYSpeed = 0.0f;
}

void PlayerMove::ProcessInput(const Uint8* keyState) {
    // check move left and right
    if(keyState[SDL_SCANCODE_LEFT] && !keyState[SDL_SCANCODE_RIGHT]) {
        SetForwardSpeed(-1.0f * mForwardSpeed);
        mMovingRight = false;
    }
    else if(!keyState[SDL_SCANCODE_LEFT] && keyState[SDL_SCANCODE_RIGHT]) {
        SetForwardSpeed(mForwardSpeed);
        mMovingRight = true;
    }
    else {
        SetForwardSpeed(0.0f);
    }
}

void PlayerMove::Update(float deltaTime) {
    // update player position
    Vector2 updatedPosition = mOwner->GetPosition() + GetForwardSpeed() * mOwner->GetForward() * deltaTime;
    updatedPosition.y = mOwner->GetPosition().y + mYSpeed * deltaTime;
    if(updatedPosition.x < EDGE_OFFSET) {
        updatedPosition.x = EDGE_OFFSET;
    }
    else if(updatedPosition.x > mOwner->GetGame()->GetScreenDimen().x - EDGE_OFFSET) {
        updatedPosition.x = mOwner->GetGame()->GetScreenDimen().x - EDGE_OFFSET;
    }
    mOwner->SetPosition(updatedPosition);
    
    // update animation
    std::string animName = mOwner->GetComponent<AnimatedSprite>()->GetAnimName();
    Game* game = mOwner->GetGame();
//    SDL_Log("hug: %d", ((Player*)mOwner)->mHug);
    if(game->mStarsDone && !game->mWin) { // lost
        SwitchAnim("sad");
        mOwner->SetState(ActorState::Paused);
        mOwner->GetGame()->mGameOver = true;
    }
    else if(((Player*)mOwner)->mHug) { // won and gave star
        SwitchAnim("hug");
        if(!mPlayedEnding) {
//            Mix_PlayChannel(-1, mOwner->GetGame()->GetSound("Assets/Sounds/ending_cut.wav"), 0);
            mPlayedEnding = true;
        }
    }
    else if(game->mStarsDone && game->mWin) { // won and still need to prepare star
        if(!mPreparedStar) { // prepare star
            SwitchAnim("prepare");
            mMovingRight = false;
            // set callback for when done preparing star
            mOwner->GetComponent<AnimatedSprite>()->SetOnRunOnce([this] {
                std::string nextAnim = "idleLeft";
                AnimatedSprite* as = this->mOwner->GetComponent<AnimatedSprite>();
                as->SetAnimation(nextAnim);
                this->mOwner->GetComponent<PlayerMove>()->mPreparedStar = true;
            });
        }
        else {
            // show final star
            ((Player*)mOwner)->GetBag()->GetComponent<SpriteComponent>()->SetIsVisible(true);
        }
    }
    else if(GetForwardSpeed() < 0) { // moving left
        SwitchAnim("runLeft");
    }
    else if(GetForwardSpeed() > 0) { // moving right
        SwitchAnim("runRight");
    }
    else { // idle
        if(mMovingRight) {
            SwitchAnim("idleRight");
        }
        else {
            SwitchAnim("idleLeft");
        }
    }
}

void PlayerMove::SwitchAnim(std::string animName) {
    mOwner->GetComponent<AnimatedSprite>()->SetAnimation(animName);
}
