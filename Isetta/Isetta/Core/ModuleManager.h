/*
 * Copyright (c) 2018 Isetta
 */
#pragma once

namespace Isetta {

class ModuleManager {
 public:
  ModuleManager();
  ~ModuleManager();

  void StartUp();
  void SimulationUpdate(float deltaTime);
  void RenderUpdate(float deltaTime);
  void ShutDown();

 private:
  class MemoryManager* memoryManager;
  class AudioModule* audioModule;
  class WindowModule* windowModule;
  class RenderModule* renderModule;
  class InputModule* inputModule;
  class GUIModule* guiModule;
  class NetworkingModule* networkingModule;
};
}  // namespace Isetta
