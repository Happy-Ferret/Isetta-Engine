/*
 * Copyright (c) 2018 Isetta
 */
#include "Core/ModuleManager.h"
#include "Audio/AudioModule.h"
#include "Core/Memory/MemoryManager.h"
#include "Graphics/GUIModule.h"
#include "Graphics/RenderModule.h"
#include "Graphics/Window.h"
#include "Input/InputModule.h"
#include "Networking/NetworkingModule.h"

namespace Isetta {
ModuleManager::ModuleManager() {
  memoryManager = new MemoryManager{};
  audioModule = new AudioModule{};
  windowModule = new WindowModule{};
  renderModule = new RenderModule{};
  inputModule = new InputModule{};
  guiModule = new GUIModule{};
  networkingModule = new NetworkingModule{};
}

ModuleManager::~ModuleManager() {
  delete networkingModule;
  delete windowModule;
  delete audioModule;
  delete renderModule;
  delete inputModule;
  delete guiModule;
  delete memoryManager;
}

void ModuleManager::StartUp() {
  memoryManager->StartUp();
  audioModule->StartUp();
  windowModule->StartUp();
  renderModule->StartUp(windowModule->winHandle);
  inputModule->StartUp(windowModule->winHandle);
  guiModule->StartUp(windowModule->winHandle);
  networkingModule->StartUp();
  memoryManager->RegisterTests();
}

void ModuleManager::SimulationUpdate(float deltaTime) {
  audioModule->Update(deltaTime);
  inputModule->Update(deltaTime);
  networkingModule->Update(deltaTime);
  memoryManager->Update();
}

void ModuleManager::RenderUpdate(float deltaTime) {
  renderModule->Update(deltaTime);
  guiModule->Update(deltaTime);
  windowModule->Update(deltaTime);
}

void ModuleManager::ShutDown() {
  networkingModule->ShutDown();
  audioModule->ShutDown();
  guiModule->ShutDown();
  inputModule->ShutDown();
  renderModule->ShutDown();
  windowModule->ShutDown();
  memoryManager->ShutDown();
}
}  // namespace Isetta