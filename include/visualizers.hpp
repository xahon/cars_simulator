#ifndef MYTONA_VISUALIZERS_HPP
#define MYTONA_VISUALIZERS_HPP

#include <stdexcept>
#include <cstring>
#include <iostream>
#include "structs.hpp"
#include "SDL2/SDL.h"

class sSDL2Display : public sDisplay {
 private:
  int w, h;
  SDL_Window *wnd;
  SDL_Renderer *rnd;

  void destroySDL() {
    SDL_DestroyRenderer(rnd);
    SDL_DestroyWindow(wnd);
    rnd = nullptr;
    wnd = nullptr;
    SDL_Quit();

#ifdef _WIN32
    // Force kill process when SDL window is closed
    exit(0);
#endif
  }

  void drawRect(const sRect &rect, int r, int g, int b) {
    SDL_Rect rc;
    rc.x = rect.x();
    rc.y = h - rect.y();
    rc.w = rect.width();
    rc.h = -rect.height();
    SDL_SetRenderDrawColor(rnd, r, g, b, SDL_ALPHA_OPAQUE);
    if (SDL_RenderFillRect(rnd, &rc) != 0) {
      throw std::runtime_error(SDL_GetError());
    }
  }

 public:
  sSDL2Display(int w, int h) : w(w), h(h) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
      throw std::runtime_error(SDL_GetError());
    }

    wnd = SDL_CreateWindow("Cars", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
    if (wnd == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }

    rnd = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_TARGETTEXTURE);
    if (rnd == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
  }

  ~sSDL2Display() override { destroySDL(); }

  void drawBackground() override {
    if (wnd == nullptr || rnd == nullptr)
      return;

    SDL_SetRenderDrawColor(rnd, 0, 150, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(rnd);
  }

  void drawRoadData(const sRoadData &roadData) override {
    if (wnd == nullptr || rnd == nullptr)
      return;

    for (auto &roadSegment : roadData.roadSegments) {
      auto x0 = std::min(roadSegment.p1.x, roadSegment.p2.x);
      auto x1 = std::max(roadSegment.p1.x, roadSegment.p2.x);
      auto y0 = std::min(roadSegment.p1.y, roadSegment.p2.y);
      auto y1 = std::max(roadSegment.p1.y, roadSegment.p2.y);

      if (y0 == y1) {
        // horizontal
        drawRect(sRect(x0, y0 - roadData.laneSize, x1 - x0, roadData.laneSize * 2), 51, 51, 51);
        drawRect(sRect(x0, y0, x1 - x0, 1), 255, 255, 255);
      }
      if (x0 == x1) {
        // vertical
        drawRect(sRect(x0 - roadData.laneSize, y0, roadData.laneSize * 2, y1 - y0), 51, 51, 51);
        drawRect(sRect(x0, y0, 1, y1 - y0), 255, 255, 255);
      }
    }

    for (const auto &crossing : roadData.crossings) {
      if (crossing.isDeadlocked())
        drawRect(crossing.rect, 40, 40, 120);
      else
        drawRect(crossing.rect, 40, 40, 40);
    }

    for (auto &spawn : roadData.spawns) {
      drawRect(sRect(spawn.first, 1, 1), 255, 0, 0);
    }

    for (const auto *car : roadData.cars) {
#ifdef USE_DEBUGGEE_CAR
      if (car->debuggee)
        drawRect(car->rect, 255, 128, 128);
      else
#endif
      if (car->checkSides) {
        int r, g, b;

        if (dynamic_cast<sHybridCar *>(const_cast<sCar *>(car)) != nullptr) {
          r = 255;
          g = 255;
          b = 0;
        } else if (dynamic_cast<sGasCar *>(const_cast<sCar *>(car)) != nullptr) {
          r = 0;
          g = 0;
          b = 255;
        } else {
          r = 0;
          g = 255;
          b = 0;
        }

        sRect carHoodRect(car->frontPoint() + car->rect.size() / 2 * car->direction.rightPerpendicular() - car->rect.size() / 4 * car->direction,
                          car->frontPoint() + car->rect.size() / 2 * car->direction.leftPerpendicular());
        drawRect(car->rect, 0,0,0);
        drawRect(carHoodRect, r, g, b);
      } else {
        drawRect(car->rect, 255, 0, 0);
      }
      drawRect(sRect(car->frontPoint(), 1, 1), 0, 255, 0);
    }
  }

  void flush() override {
    SDL_RenderPresent(rnd);

    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        destroySDL();
        break;
      }
    }
  }
};

class sTerminalDisplay : public sDisplay {
 private:
  char *buffer;
  int w, h;

  static constexpr char spaceChar = ' ';
  static constexpr char roadChar = '.';
  static constexpr char crossingChar = ':';

  void setChar(int x, int y, char c) {
    if (x >= 0 && x < w && y >= 0 && y < h)
      buffer[x * h + y] = c;
  }

  char getChar(int x, int y) const {
    if (x >= 0 && x < w && y >= 0 && y < h)
      return buffer[x * h + y];
    return '\0';
  }

  void drawRect(const sRect &rect, char c) {
    for (size_t y = rect.p1.y; y < rect.p2.y; ++y) {
      for (size_t x = rect.p1.x; x < rect.p2.x; ++x) {
        setChar(x, y, c);
      }
    }
  }

  void drawRect(sRect &&rect, char c) {
    for (size_t y = rect.p1.y; y < rect.p2.y; ++y) {
      for (size_t x = rect.p1.x; x < rect.p2.x; ++x) {
        setChar(x, y, c);
      }
    }
  }

 public:
  sTerminalDisplay(int w, int h) : w(w), h(h) { buffer = new char[w * h]; }

  ~sTerminalDisplay() override { delete[] buffer; }

  void drawBackground() override { std::memset(buffer, spaceChar, w * h); }

  void drawRoadData(const sRoadData &roadData) override {
    for (auto &roadSegment : roadData.roadSegments) {
      auto x0 = std::min(roadSegment.p1.x, roadSegment.p2.x);
      auto x1 = std::max(roadSegment.p1.x, roadSegment.p2.x);
      auto y0 = std::min(roadSegment.p1.y, roadSegment.p2.y);
      auto y1 = std::max(roadSegment.p1.y, roadSegment.p2.y);

      if (y0 == y1) {
        // horizontal
        drawRect(sRect(x0, y0 - roadData.laneSize, x1 - x0, roadData.laneSize * 2), roadChar);
      }
      if (x0 == x1) {
        // vertical
        drawRect(sRect(x0 - roadData.laneSize, y0, roadData.laneSize * 2, y1 - y0), roadChar);
      }
    }

    for (auto &crossing : roadData.crossings) {
      drawRect(crossing.rect, crossingChar);
    }

    for (auto &spawn : roadData.spawns) {
      drawRect(sRect(spawn.first, 1, 1), '#');
    }

    int i = 0;
    for (const auto *car : roadData.cars) {
      drawRect(car->rect, '0' + (i++));
    }
  }

  void flush() override {
#if defined(_WIN32)
    system("cls");
#else
    system("clear");
#endif
    for (int y = h - 1; y >= 0; --y) {
      for (int x = 0; x < w; ++x) {
        std::cout << getChar(x, y);
      }
      std::cout << std::endl;
    }
  }
};

#endif  // MYTONA_VISUALIZERS_HPP
