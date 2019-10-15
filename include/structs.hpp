#ifndef MYTONA_STRUCTS_HPP
#define MYTONA_STRUCTS_HPP

#include <vector>
#include <deque>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <limits>

struct sVec;
struct sRect;
struct sLineSegment;
struct sCar;
struct sCrossingCarInfo;
struct sCrossing;
struct sRoadData;
struct sCarFactory;
struct sDisplay;
struct sGasCar;
struct sElectroCar;
struct sHybridCar;

struct sVec {
  int x, y;

  explicit sVec(int x = 0, int y = 0) : x(x), y(y) {}

  sVec operator-() const { return sVec(-x, -y); }
  sVec rightPerpendicular() const { return sVec(y, -x); }
  sVec leftPerpendicular() const { return sVec(-y, x); }

  bool operator==(const sVec &other) const { return x == other.x && y == other.y; }
  bool operator!=(const sVec &other) const { return !(*this == other); }

  sVec &operator+=(const sVec &other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  sVec operator+(const sVec &other) const {
    sVec t = *this;
    t += other;
    return t;
  }

  sVec &operator-=(const sVec &other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  sVec operator-(const sVec &other) const {
    sVec t = *this;
    t -= other;
    return t;
  }

  sVec &operator*=(const sVec &other) {
    x *= other.x;
    y *= other.y;
    return *this;
  }

  sVec operator*(const sVec &other) const {
    sVec t = *this;
    t *= other;
    return t;
  }

  sVec &operator*=(int scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  sVec operator*(int scalar) const {
    sVec t = *this;
    t *= scalar;
    return t;
  }

  sVec &operator/=(int scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  sVec operator/(int scalar) const {
    sVec t = *this;
    t /= scalar;
    return t;
  }
};

struct sRect {
  sVec p1, p2;

  sRect(sVec p1 = sVec(), sVec p2 = sVec())                    //
      : p1(sVec(std::min(p1.x, p2.x), std::min(p1.y, p2.y))),  //
        p2(sVec(std::max(p1.x, p2.x), std::max(p1.y, p2.y))) {}

  sRect(sVec p1, int w, int h) : p1(p1), p2(p1.x + w, p1.y + h) {}
  sRect(int x, int y, int w, int h) : p1(x, y), p2(x + w, y + h) {}

  sRect(int w, int h) : p1(sVec()), p2(w, h) {}

  int x() const { return p1.x; }
  int y() const { return p1.y; }
  int width() const { return p2.x - p1.x; }
  int height() const { return p2.y - p1.y; }
  sVec position() const { return p1; }
  sVec size() const { return p2 - p1; }

  sRect &setWidth(int width) {
    p2.x = p1.x + width;
    return *this;
  }

  sRect &setHeight(int height) {
    p2.y = p1.y + height;
    return *this;
  }

  sRect &moveBy(const sVec &vec) {
    moveBy(vec.x, vec.y);
    return *this;
  }

  sRect &moveTo(const sVec &vec) {
    moveTo(vec.x, vec.y);
    return *this;
  }

  sRect &moveBy(int x, int y) {
    p1.x += x;
    p2.x += x;
    p1.y += y;
    p2.y += y;
    return *this;
  }

  sRect &moveTo(int x, int y) {
    auto w = width();
    auto h = height();

    p1.x = x;
    p1.y = y;
    p2.x = x + w;
    p2.y = y + h;
    return *this;
  }

  sRect &swapWidthHeight() {
    auto oldWidth = width();
    setWidth(height());
    setHeight(oldWidth);
    return *this;
  }

  bool isInside(const sVec &vec) const {
    return vec.x > p1.x && vec.x < p2.x &&  //
           vec.y > p1.y && vec.y < p1.y;
  }

  bool isOnEdge(const sVec &vec) const {
    return vec.x == p1.x || vec.x == p2.x ||  //
           vec.y == p1.y || vec.y == p2.y;
  }

  bool isInsideOrOnEdge(const sVec &vec) const { return isInside(vec) || isOnEdge(vec); }

  bool touches(const sRect &other) const {
    return p1.x == other.p2.x ||  //
           p2.x == other.p1.x ||  //
           p1.y == other.p2.y ||  //
           p2.y == other.p1.y;    //
  }

  bool overlaps(const sRect &other) const {
    return p1.x < other.p2.x &&  //
           p2.x > other.p1.x &&  //
           p1.y < other.p2.y &&  //
           p2.y > other.p1.y;    //
  }

  bool contacts(const sRect &other) const { return touches(other) || overlaps(other); }
};

struct sLineSegment {
  sVec p1, p2;

  sLineSegment(sVec p1, sVec p2) : p1(p1), p2(p2) {}

  float slope() const {
    if (p1.x == p2.x)
      return std::numeric_limits<float>::infinity();
    return (p2.y - p1.y) / (p2.x - p1.x);
  }

  bool intersects(const sLineSegment &other, sVec *intersectionPoint) const {
    float a1 = p2.y - p1.y;
    float b1 = p1.x - p2.x;
    float c1 = a1 * (p1.x) + b1 * (p1.y);
    float a2 = other.p2.y - other.p1.y;
    float b2 = other.p1.x - other.p2.x;
    float c2 = a2 * (other.p1.x) + b2 * (other.p1.y);

    float det = a1 * b2 - a2 * b1;

    if (det == 0)  // The lines are parallel
      return false;

    intersectionPoint->x = (b2 * c1 - b1 * c2) / det;
    intersectionPoint->y = (a1 * c2 - a2 * c1) / det;
    return true;
  }
};

enum eCarAlignment {
  CAR_MOVE_WEST,
  CAR_MOVE_EAST,
  CAR_MOVE_NORTH,
  CAR_MOVE_SOUTH,
};

struct sCar {
  sRect rect;
  sVec direction;
  int speed = 1;
  bool wasInField = false;
  bool checkSides = true;
  bool debuggee = false;

  virtual ~sCar() = default;

  sCar &setAlignment(eCarAlignment alignment) {
    switch (alignment) {
      case eCarAlignment::CAR_MOVE_WEST:
        if (rect.height() > rect.width()) {
          rect.swapWidthHeight();
        }
        direction.x = -1;
        direction.y = 0;
        break;

      case eCarAlignment::CAR_MOVE_SOUTH:
        if (rect.height() < rect.width()) {
          rect.swapWidthHeight();
        }
        direction.x = 0;
        direction.y = -1;
        break;

      case eCarAlignment::CAR_MOVE_NORTH:
        if (rect.height() < rect.width()) {
          rect.swapWidthHeight();
        }
        direction.x = 0;
        direction.y = 1;
        break;

      case eCarAlignment::CAR_MOVE_EAST:
      default:
        if (rect.height() > rect.width()) {
          rect.swapWidthHeight();
        }
        direction.x = 1;
        direction.y = 0;
        break;
    }
    return *this;
  }

  sRect forwardRect() const {
    sRect f = rect;
    f.moveBy(direction * rect.size());
    return f;
  }

  sRect futureRect() const {
    sRect f = rect;
    f.moveBy(direction * speed);
    return f;
  }

  sVec futurePosition() const {  //
    return sVec(rect.p1.x + direction.x * speed, rect.p1.y + direction.y * speed);
  }

  sVec frontPoint() const {
    return sVec(rect.x() + rect.size().x / 2 + rect.size().x * direction.x / 2,  //
                rect.y() + rect.size().y / 2 + rect.size().y * direction.y / 2);
  }

  virtual void move() {}
  virtual int getFuel() = 0;
  virtual void refill(int count) = 0;
};

struct sGasCar : public virtual sCar {
  int fuel;

  int getFuel() { return fuel; }

  void refill(int count) { fuel += count; }

  void move() {
    --fuel;
    sCar::move();
  }
};

struct sElectroCar : public virtual sCar {
  int charge;

  int getFuel() { return charge; }

  void refill(int count) { charge += count; }

  void move() {
    --charge;
    sCar::move();
  }
};

struct sHybridCar : public sGasCar, public sElectroCar {
  void refill(int count) {
    charge += count / 2;
    fuel += count / 2;
  }

  int getFuel() { return charge + fuel; }

  void move() {
    if (rand() % 2 == 0)
      --charge;
    else
      --fuel;
    sCar::move();
  }
};

struct sCrossingCarInfo {
  sCar *car;
  sCrossing *crossing;
  bool isInCrossing;
  bool isTouched;
  bool justWentOut;
};

struct sCrossing {
  sRect rect;
  std::vector<sCar *> cars;

  explicit sCrossing(sRect rect) : rect(rect) {}

  sCrossingCarInfo getCrossingInfo(const sCar *car) const {
    sCrossingCarInfo info{const_cast<sCar *>(car), const_cast<sCrossing *>(this), false, false, false};
    info.isInCrossing = car->rect.contacts(rect);
    if (info.isInCrossing) {
      sVec frontPoint = car->frontPoint();
      info.isTouched = car->rect.touches(rect) && rect.isInsideOrOnEdge(frontPoint);
      info.justWentOut = car->rect.touches(rect) && !rect.isInsideOrOnEdge(frontPoint);
    }
    return info;
  }

  void updateCarData(sCar *car) {
    auto crossingInfo = getCrossingInfo(car);
    auto foundCarIt = std::find(cars.begin(), cars.end(), car);
    bool infoStored = foundCarIt != cars.end();
    if (crossingInfo.isInCrossing && !infoStored) {
      cars.push_back(car);
    } else if (!crossingInfo.isInCrossing && infoStored) {
      cars.erase(foundCarIt);
      car->checkSides = true;
    }
  }

  bool isDeadlocked() const {
    bool                //
    hasLeft = false,    //
    hasRight = false,   //
    hasTop = false,     //
    hasBottom = false,  //
    hasNoChecker = false;

    for (auto *car : cars) {
      sCrossingCarInfo crossingInfo = getCrossingInfo(car);
      if (hasLeft && hasRight && hasTop && hasBottom)
        return true;
      if (car->direction.x < 0) {
        hasRight = crossingInfo.isTouched;
      } else if (car->direction.x > 0) {
        hasLeft = crossingInfo.isTouched;
      } else if (car->direction.y < 0) {
        hasTop = crossingInfo.isTouched;
      } else if (car->direction.y > 0) {
        hasBottom = crossingInfo.isTouched;
      }
      if (!car->checkSides) {
        hasNoChecker = true;
      }
    }
    return hasLeft && hasRight && hasTop && hasBottom && !hasNoChecker;
  }
};

typedef std::pair<sVec, eCarAlignment> sSpawn;

struct sRoadData {
  int laneSize;
  std::vector<sLineSegment> roadSegments;
  std::vector<sSpawn> spawns;
  std::vector<sCrossing> crossings;
  std::vector<sCar *> cars;

  sRoadData(int laneSize, std::vector<sLineSegment> roadSegments)  //
      : laneSize(laneSize), roadSegments(roadSegments) {
    sVec intersectionPoint;
    for (auto it1 = roadSegments.begin(); it1 != (roadSegments.end() - 1); ++it1) {
      for (auto it2 = it1 + 1; it2 != roadSegments.end(); ++it2) {
        if (it1->intersects(*it2, &intersectionPoint)) {
          crossings.emplace_back(sRect(                                               //
          /**/ sVec(intersectionPoint.x - laneSize, intersectionPoint.y - laneSize),  //
          /**/ sVec(intersectionPoint.x + laneSize, intersectionPoint.y + laneSize)   //
          ));
        }
      }
    }
  }

  void createSpawn(const sVec &position, eCarAlignment alignment, int carSizeSmall, int carSizeBig) {
    sVec spawnPosition = position;
    switch (alignment) {
      case eCarAlignment::CAR_MOVE_EAST:
        spawnPosition.y += -laneSize / 2 - carSizeSmall / 2;
        spawnPosition.x += -carSizeBig;
        break;
      case eCarAlignment::CAR_MOVE_WEST:
        spawnPosition.y += laneSize / 2 - carSizeSmall / 2;
        spawnPosition.x += carSizeBig;
        break;
      case eCarAlignment::CAR_MOVE_SOUTH:
        spawnPosition.x += -laneSize / 2 - carSizeSmall / 2;
        spawnPosition.y += carSizeBig;
        break;
      case eCarAlignment::CAR_MOVE_NORTH:
        spawnPosition.x += laneSize / 2 - carSizeSmall / 2;
        spawnPosition.y += -carSizeBig;
        break;

      default:
        exit(100);
        break;
    }

    spawns.emplace_back(std::make_pair(spawnPosition, alignment));
  }
};

struct sCarFactory {
 private:
  sCarFactory() = delete;

 public:
  static sCar *createRandomCar(std::vector<sSpawn> &spawns, int w, int h) {
    sCar *car = nullptr;
    int type = rand() % 3;
    if (type == 0) {
      car = new sGasCar();
    } else if (type == 1) {
      car = new sElectroCar();
    } else {
      car = new sHybridCar();
    }

    car->rect.setWidth(w);
    car->rect.setHeight(h);
    setRandomPositionAndAlign(car, spawns);
    return car;
  }

  static sCar *setRandomPositionAndAlign(sCar *car, std::vector<sSpawn> &spawns) {
    std::pair<sVec, eCarAlignment> randomSpawn = spawns[std::rand() % spawns.size()];
    placeAtSpawn(car, randomSpawn);
    return car;
  }

  static sCar *placeAtSpawn(sCar *car, const sSpawn &spawn) {
    car->setAlignment(spawn.second);
    car->rect.moveTo(spawn.first - car->direction * car->rect.size());
    return car;
  }
};

struct sDisplay {
  virtual ~sDisplay() = default;

  virtual void drawBackground() = 0;
  virtual void drawRoadData(const sRoadData &roadData) = 0;
  virtual void flush() = 0;
};

#endif  // MYTONA_STRUCTS_HPP
