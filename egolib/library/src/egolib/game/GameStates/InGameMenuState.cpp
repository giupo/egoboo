//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file egolib/game/GameStates/InGameMenuState.cpp
/// @details Menu while PlayingState is running in background
/// @author Johan Jansen

#include "egolib/game/GameStates/InGameMenuState.hpp"
#include "egolib/game/GameStates/MainMenuState.hpp"
#include "egolib/game/GameStates/PlayingState.hpp"
#include "egolib/game/GameStates/LoadingState.hpp"
#include "egolib/game/GameStates/OptionsScreen.hpp"
#include "egolib/game/Core/GameEngine.hpp"
#include "egolib/game/game.h"
#include "egolib/game/GUI/Button.hpp"
#include "egolib/game/Module/Module.hpp"
#include "egolib/Graphics/GraphicsSystem.hpp"

#ifdef _DEBUG
#include "egolib/game/GameStates/DebugParticlesScreen.hpp"
#endif

InGameMenuState::InGameMenuState(GameState &gameState) :
    _slidyButtons(),
    _backgroundState(gameState)
{
    // Add the buttons
    int yOffset = Ego::GraphicsSystem::get().window->size().y() - 80;
    auto exitButton = std::make_shared<Ego::GUI::Button>(_currentModule->isExportValid() ? "Save and Exit" : "Abort and Exit", SDLK_q);
    exitButton->setPosition({ 20, yOffset });
    exitButton->setSize({ 200, 30 });
    exitButton->setOnClickFunction(
    []{
        _gameEngine->setGameState(std::make_shared<MainMenuState>());
    });
    addComponent(exitButton);
    _slidyButtons.push_front(exitButton);

    yOffset -= exitButton->getHeight() + 10;

    auto optionsButton = std::make_shared<Ego::GUI::Button>("Options", SDLK_o);
    optionsButton->setPosition({ 20, yOffset });
    optionsButton->setSize({ 200, 30 });
    optionsButton->setOnClickFunction(
        []{
            _gameEngine->pushGameState(std::make_shared<OptionsScreen>());
        });
    addComponent(optionsButton);
    _slidyButtons.push_front(optionsButton);

    yOffset -= optionsButton->getHeight() + 10;

    auto restartModuleButton = std::make_shared<Ego::GUI::Button>("Restart Module", SDLK_r);
    restartModuleButton->setPosition({ 20, yOffset });
    restartModuleButton->setSize({ 200, 30 });
    restartModuleButton->setOnClickFunction(
    [this]{
        //Reload current module with current players
        _gameEngine->setGameState(std::make_shared<LoadingState>(_currentModule->getModuleProfile(), _currentModule->getImportPlayers()));
    });
    addComponent(restartModuleButton);
    _slidyButtons.push_front(restartModuleButton);

    yOffset -= restartModuleButton->getHeight() + 10;

    auto newGameButton = std::make_shared<Ego::GUI::Button>("Return to Module", SDLK_ESCAPE);
    newGameButton->setPosition({ 20, yOffset });
    newGameButton->setSize({ 200, 30 });
    newGameButton->setOnClickFunction(
    [this]{
        endState();
    });
    addComponent(newGameButton);
    _slidyButtons.push_front(newGameButton);

    yOffset -= newGameButton->getHeight() + 10;

#ifdef _DEBUG
    auto debugButton = std::make_shared<Ego::GUI::Button>("Debug Particles", SDLK_p);
    debugButton->setPosition({ 20, yOffset });
    debugButton->setSize({ 200, 30 });
    debugButton->setOnClickFunction(
    [this]{
        _gameEngine->pushGameState(std::make_shared<DebugParticlesScreen>());
    });
    addComponent(debugButton);
    _slidyButtons.push_front(debugButton);

    yOffset -= debugButton->getHeight() + 10;    
#endif
}

void InGameMenuState::update()
{
}

void InGameMenuState::drawContainer(Ego::GUI::DrawingContext& drawingContext)
{
    //Render the playing state beackground first
    _backgroundState.drawAll(drawingContext);
}

void InGameMenuState::beginState()
{
    // menu settings
    Ego::GraphicsSystem::get().window->grab_enabled(false);
    _gameEngine->enableMouseCursor();

    //Sliding buttons effect
    float offset = 0;
    for(const auto& button : _slidyButtons)
    {
        button->beginSlidyButtonEffect(button->getWidth() + offset);
        offset += 20;
    }
}
