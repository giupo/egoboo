#include "game/GUI/ComponentContainer.hpp"
#include "game/GameStates/GameState.hpp"
#include "game/Core/GameEngine.hpp"
#include "game/GUI/Component.hpp"

namespace Ego {
namespace GUI {

ComponentContainer::ComponentContainer() :
    _componentList(),
    _componentDestroyed(false),
    _semaphoreLock(0),
    _containerMutex() {
    //ctor
}

ComponentContainer::~ComponentContainer() {
    if (_semaphoreLock != 0) {
        throw new std::logic_error("Destructing ComponentContainer while iterating");
    }
}


void ComponentContainer::addComponent(std::shared_ptr<Component> component) {
    std::lock_guard<std::mutex> lock(_containerMutex);
    _componentList.push_back(component);
    component->_parent = this;
}

void ComponentContainer::removeComponent(std::shared_ptr<Component> component) {
    std::lock_guard<std::mutex> lock(_containerMutex);
    _componentList.erase(std::remove(_componentList.begin(), _componentList.end(), component), _componentList.end());
}

void ComponentContainer::clearComponents() {
    std::lock_guard<std::mutex> lock(_containerMutex);
    _componentList.clear();
}

size_t ComponentContainer::getComponentCount() const {
    return _componentList.size();
}

void ComponentContainer::drawAll() {
    //Render the container itself
    drawContainer();

    //Draw reach GUI component
    _gameEngine->getUIManager()->beginRenderUI();
    for (const std::shared_ptr<Component> component : iterator()) {
        if (!component->isVisible()) continue;  //Ignore hidden/destroyed components
        component->draw();
    }
    _gameEngine->getUIManager()->endRenderUI();
}

bool ComponentContainer::notifyMouseMoved(const Events::MouseMovedEventArgs& e) {
    //Iterate over GUI components in reverse order so GUI components added last (i.e on top) consume events first
    ComponentIterator it = iterator();
    for (auto i = it.rbegin(); i != it.rend(); ++i) {
        std::shared_ptr<Component> component = *i;
        if (!component->isEnabled()) continue;
        if (component->notifyMouseMoved(e)) return true;
    }
    return false;
}

bool ComponentContainer::notifyKeyboardKeyPressed(const Events::KeyboardKeyPressedEventArgs& e) {
    //Iterate over GUI components in reverse order so GUI components added last (i.e on top) consume events first
    ComponentIterator it = iterator();
    for (auto i = it.rbegin(); i != it.rend(); ++i) {
        std::shared_ptr<Component> component = *i;
        if (!component->isEnabled()) continue;
        if (component->notifyKeyboardKeyPressed(e)) return true;
    }
    return false;
}

bool ComponentContainer::notifyMouseButtonPressed(const Events::MouseButtonPressedEventArgs& e) {
    //Iterate over GUI components in reverse order so GUI components added last (i.e on top) consume events first
    ComponentIterator it = iterator();
    for (auto i = it.rbegin(); i != it.rend(); ++i) {
        std::shared_ptr<Component> component = *i;
        if (!component->isEnabled()) continue;
        if (component->notifyMouseButtonPressed(e)) return true;
    }
    return false;
}

bool ComponentContainer::notifyMouseButtonReleased(const Events::MouseButtonReleasedEventArgs& e) {
    //Iterate over GUI components in reverse order so GUI components added last (i.e on top) consume events first
    ComponentIterator it = iterator();
    for (auto i = it.rbegin(); i != it.rend(); ++i) {
        std::shared_ptr<Component> component = *i;
        if (!component->isEnabled()) continue;
        if (component->notifyMouseButtonReleased(e)) return true;
    }
    return false;
}

bool ComponentContainer::notifyMouseWheelTurned(const Events::MouseWheelTurnedEventArgs& e) {
    //Iterate over GUI components in reverse order so GUI components added last (i.e on top) consume events first
    ComponentIterator it = iterator();
    for (auto i = it.rbegin(); i != it.rend(); ++i) {
        std::shared_ptr<Component> component = *i;
        if (!component->isEnabled()) continue;
        if (component->notifyMouseWheelTurned(e)) return true;
    }
    return false;
}

void ComponentContainer::notifyDestruction() {
    //Deferred destruction
    _componentDestroyed = true;
}


void ComponentContainer::bringComponentToFront(std::shared_ptr<Component> component) {
    removeComponent(component);
    addComponent(component);
}

void ComponentContainer::lock() {
    std::lock_guard<std::mutex> lock(_containerMutex);
    _semaphoreLock++;
}

void ComponentContainer::unlock() {
    std::lock_guard<std::mutex> lock(_containerMutex);
    if (_semaphoreLock == 0) {
        throw new std::logic_error("ComponentContainer calling unlock() without lock()");
    }

    //Release one lock
    _semaphoreLock--;

    //If all locks are released, remove all destroyed components
    if (_semaphoreLock == 0 && _componentDestroyed) {
        _componentList.erase(std::remove_if(_componentList.begin(), _componentList.end(),
                                            [](std::shared_ptr<Component> component) {return component->isDestroyed(); }),
                             _componentList.end());
        _componentDestroyed = false;
    }
}

void ComponentContainer::setComponentList(const std::vector<std::shared_ptr<Component>> &list) {
    clearComponents();
    for (const auto& component : list) addComponent(component);
}

} // namespace GUI
} // namespace Ego
