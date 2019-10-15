#include <cstring>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif
#include "include/structs.hpp"
#include "include/simulator.hpp"
#include "include/visualizers.hpp"

static constexpr int SCREEN_WIDTH = 640;
static constexpr int SCREEN_HEIGHT = 480;
static constexpr int CARS_COUNT = 20;
static constexpr int ROAD_WIDTH = 40;
static constexpr int CAR_SIZE_SMALL = 20;
static constexpr int CAR_SIZE_BIG = 40;
static constexpr int DELAY_BETWEEN_FRAMES_MS = 10;

#ifdef __WIN32__
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
  // auto *display = new sTerminalDisplay(SCREEN_WIDTH, SCREEN_HEIGHT);
  auto *display = new sSDL2Display(SCREEN_WIDTH, SCREEN_HEIGHT);

  std::srand(std::time(0));
  sVec roadSegment1_p1 = sVec(0, SCREEN_HEIGHT / 2);
  sVec roadSegment1_p2 = sVec(SCREEN_WIDTH, SCREEN_HEIGHT / 2);
  sVec roadSegment2_p1 = sVec(SCREEN_WIDTH / 3, SCREEN_HEIGHT);
  sVec roadSegment2_p2 = sVec(SCREEN_WIDTH / 3, 0);
  // sVec roadSegment3_p1 = sVec(2 * SCREEN_WIDTH / 3, SCREEN_HEIGHT);
  // sVec roadSegment3_p2 = sVec(2 * SCREEN_WIDTH / 3, 0);

  sRoadData roadData(ROAD_WIDTH, {
                                 sLineSegment(roadSegment1_p1, roadSegment1_p2),  //
                                 sLineSegment(roadSegment2_p1, roadSegment2_p2),  //
                                 //  sLineSegment(roadSegment3_p1, roadSegment3_p2)   // FIXME: Deadlock checking doesn't work as expected
                                 });

  roadData.createSpawn(roadSegment1_p1, eCarAlignment::CAR_MOVE_EAST, CAR_SIZE_SMALL, CAR_SIZE_BIG);
  roadData.createSpawn(roadSegment1_p2, eCarAlignment::CAR_MOVE_WEST, CAR_SIZE_SMALL, CAR_SIZE_BIG);
  roadData.createSpawn(roadSegment2_p1, eCarAlignment::CAR_MOVE_SOUTH, CAR_SIZE_SMALL, CAR_SIZE_BIG);
  roadData.createSpawn(roadSegment2_p2, eCarAlignment::CAR_MOVE_NORTH, CAR_SIZE_SMALL, CAR_SIZE_BIG);
  // roadData.createSpawn(roadSegment3_p1, eCarAlignment::CAR_MOVE_SOUTH, CAR_SIZE_SMALL, CAR_SIZE_BIG);
  // roadData.createSpawn(roadSegment3_p2, eCarAlignment::CAR_MOVE_NORTH, CAR_SIZE_SMALL, CAR_SIZE_BIG);

  for (int i = 0; i < CARS_COUNT; ++i) {
    auto *car = sCarFactory::createRandomCar(roadData.spawns, CAR_SIZE_BIG, CAR_SIZE_SMALL);
    car->speed = 1;
#ifdef USE_DEBUGGEE_CAR
    if (i == 5)
      car->debuggee = true;
#endif
    roadData.cars.push_back(car);
  }

  bool isRunning = true;

  while (isRunning) {
    display->drawBackground();
    resolveCollisions(roadData);
    respawnOutOfFieldCars(roadData, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto verboseCarsInfo = getVerboseCarsInfo(roadData);
    auto moveData = getNextCarsPositionPairs(roadData, verboseCarsInfo);
    resolveDeadlocks(roadData);
    handleMovings(moveData);
    display->drawRoadData(roadData);
    display->flush();
#ifndef _WIN32
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_BETWEEN_FRAMES_MS));
#else
    Sleep(DELAY_BETWEEN_FRAMES_MS);
#endif
  }

  delete display;
  display = nullptr;
  return 0;
}
