#ifndef MYTONA_SIMULATOR_HPP
#define MYTONA_SIMULATOR_HPP

#include <iostream>
#include "structs.hpp"

void resolveCollisions(sRoadData &roadData) {
  std::deque<sCar *> queue(roadData.cars.begin(), roadData.cars.end());
  while (queue.size() > 0) {
    sCar *&car = queue.back();
    queue.pop_back();

    for (auto *&otherCar : roadData.cars) {
      if (car == otherCar)
        continue;
      if (car->rect.overlaps(otherCar->rect)) {
        do {
          car->rect.moveBy(-car->direction * car->rect.size());
        } while (car->rect.overlaps(otherCar->rect));
      }
    }
  }
}

void respawnOutOfFieldCars(sRoadData &roadData, int scrWidth, int scrHeight) {
  sRect screenRect(scrWidth, scrHeight);
  bool respawned = false;

  for (auto *&car : roadData.cars) {
    if (!car->wasInField) {
      if (car->rect.contacts(screenRect)) {
        car->wasInField = true;
      }
    } else {
      if (!car->rect.contacts(screenRect)) {
        sCarFactory::setRandomPositionAndAlign(car, roadData.spawns);
        car->wasInField = false;
        respawned = true;
      }
    }
  }

  if (respawned)
    resolveCollisions(roadData);
}

sCrossingCarInfo updateAndGetCrossingsDatas(sCar *car, sRoadData &roadData) {
  for (auto &crossing : roadData.crossings) {
    auto crossingInfo = crossing.getCrossingInfo(car);
    crossing.updateCarData(car);
    if (crossingInfo.isInCrossing) {
      return crossingInfo;
    }
  }
  return sCrossingCarInfo{car, nullptr, false, false, false};
}

std::vector<sCrossingCarInfo> getVerboseCarsInfo(sRoadData &roadData) {
  std::vector<sCrossingCarInfo> crossingCarsInfos;
  crossingCarsInfos.reserve(roadData.cars.size());
  for (auto *&car : roadData.cars) {
    crossingCarsInfos.emplace_back(updateAndGetCrossingsDatas(car, roadData));
  }
  return crossingCarsInfos;
}

std::pair<sCar *, sVec> getNextCarPositionPair(const sCrossingCarInfo &carCrossingInfo, const std::vector<sCrossingCarInfo> &crossingCarsInfos, sRoadData &roadData) {
#define DEBUG_CAR if (carCrossingInfo.car->debuggee)
  sRect forwardRect = carCrossingInfo.car->forwardRect();
  bool shouldMove = true;
  bool frontCollisionPrevented = false;
  bool dangerousCollision = false;

  bool debuggeePrinted = false;

  // check for car in front of that one
  for (auto &otherCarInfo : crossingCarsInfos) {
    const auto *otherCar = otherCarInfo.car;
    if (carCrossingInfo.car == otherCar)
      continue;

    if (!dangerousCollision) {
      dangerousCollision = carCrossingInfo.car->futureRect().overlaps(otherCar->rect);
    }
    if (forwardRect.overlaps(otherCar->rect) && otherCarInfo.car->direction == carCrossingInfo.car->direction) {
      // don't collide into the back
      shouldMove = false;
      frontCollisionPrevented = true;
      DEBUG_CAR {
        std::cout << "Yield: Front collision" << std::endl;
        debuggeePrinted = true;
      }
    }
  }

  DEBUG_CAR {
    if (!carCrossingInfo.car->checkSides) {
      std::cout << "Move: Force deadlocked crossing" << std::endl;
      debuggeePrinted = true;
    }
  }
  if (carCrossingInfo.car->checkSides && !frontCollisionPrevented) {
    if (carCrossingInfo.isInCrossing && carCrossingInfo.isTouched) {
      // if inside the crossing check for a car coming from the right
      sVec leftPerpendicular = carCrossingInfo.car->direction.leftPerpendicular();
      auto carFromRightIt = std::find_if(                                                              //
      /**/ crossingCarsInfos.begin(),                                                                  //
      /**/ crossingCarsInfos.end(),                                                                    //
      [&](const sCrossingCarInfo &info) -> bool {                                                      //
        return info.crossing == carCrossingInfo.crossing && info.car->direction == leftPerpendicular;  //
      });

      if (carFromRightIt != crossingCarsInfos.end()) {
        shouldMove = false;
        DEBUG_CAR {
          std::cout << "Yield: Right car. other dir: x=" << carFromRightIt->car->direction.x << ", y=" << carFromRightIt->car->direction.y << std::endl;
          debuggeePrinted = true;
        }
      } else {
        // pass left car that is already started to cross intersection
        sVec rightPerpendicular = carCrossingInfo.car->direction.rightPerpendicular();
        auto carFromLeftIt = std::find_if(                                                                                                        //
        /**/ crossingCarsInfos.begin(),                                                                                                           //
        /**/ crossingCarsInfos.end(),                                                                                                             //
        [&](const sCrossingCarInfo &info) -> bool {                                                                                               //
          return !info.isTouched && !info.justWentOut && info.crossing == carCrossingInfo.crossing && info.car->direction == rightPerpendicular;  //
        });
        if (carFromLeftIt != crossingCarsInfos.end()) {
          // car in the middle of intersection from left found
          shouldMove = false;
          DEBUG_CAR {
            std::cout << "Yield: Pass left car" << std::endl;
            debuggeePrinted = true;
          }
        } else {
          DEBUG_CAR {
            std::cout << "Move: No left and right obstacles" << std::endl;
            debuggeePrinted = true;
          }
        }
      }
    }
  }

  if (shouldMove && dangerousCollision) {
    shouldMove = false;
    DEBUG_CAR {
      std::cout << "Yield: Dangerous collision" << std::endl;
      debuggeePrinted = true;
    }
  }

  if (!debuggeePrinted) {
    DEBUG_CAR {
      std::cout << "Move: No special condition" << std::endl;
      debuggeePrinted = true;
    }
  }

  return shouldMove                                                                    //
         ? std::make_pair(carCrossingInfo.car, carCrossingInfo.car->futurePosition())  //
         : std::make_pair(carCrossingInfo.car, carCrossingInfo.car->rect.position());  //
}

std::vector<std::pair<sCar *, sVec>> getNextCarsPositionPairs(sRoadData &roadData, std::vector<sCrossingCarInfo> &verboseCarsInfo) {
  std::vector<std::pair<sCar *, sVec>> movingsData;
  movingsData.reserve(roadData.cars.size());
  for (auto &carInfo : verboseCarsInfo) {
    movingsData.emplace_back(getNextCarPositionPair(carInfo, verboseCarsInfo, roadData));
  }
  return movingsData;
}

void resolveDeadlocks(sRoadData &roadData) {
  for (auto &crossing : roadData.crossings) {
    if (!crossing.isDeadlocked()) {
      continue;
    }

    std::sort(crossing.cars.begin(), crossing.cars.end(), [](const sCar *c1, const sCar *c2) -> bool {  //
      return c1->rect.x() < c2->rect.x();                                                               //
    });

    sCar *carToMoveFirst = nullptr;
    for (auto *&car : crossing.cars) {
      sRect bigForwardCast = car->forwardRect();
      if (car->direction.y == 0) {
        bigForwardCast.setWidth(crossing.rect.width());
      }
      if (car->direction.x == 0) {
        bigForwardCast.setHeight(crossing.rect.width());
      }

      bool carFound = true;
      for (auto *&otherCar : crossing.cars) {
        if (car == otherCar)
          continue;
        if (bigForwardCast.overlaps(otherCar->rect)) {
          carFound = false;
          break;
        }
      }
      if (carFound) {
        carToMoveFirst = car;
        break;
      }
    }
    if (carToMoveFirst != nullptr)
      carToMoveFirst->checkSides = false;
  }
}

void handleMovings(std::vector<std::pair<sCar *, sVec>> &moveData) {
  for (auto &pair : moveData) {
    auto *&car = pair.first;
    car->rect.moveTo(pair.second);
  }
}

#endif  // MYTONA_SIMULATOR_HPP
