#include "lux/kit.hpp"
#include <string.h>
#include <vector>
#include <set>
#include <stdio.h>

using namespace std;
using namespace lux;
int main()
{
  kit::Agent gameState = kit::Agent();
  // initialize
  gameState.initialize();

  while (true)
  {
    /** Do not edit! **/
    // wait for updates
    gameState.update();

    vector<string> actions = vector<string>();

    /** AI Code Goes Below! **/

    Player player = gameState.players[gameState.id];
    Player opponent = gameState.players[(gameState.id + 1) % 2];

    GameMap gameMap = gameState.map;

    vector<Cell *> resourceTiles = vector<Cell *>();
    for (int y = 0; y < gameState.mapHeight; y++)
    {
      for (int x = 0; x < gameState.mapWidth; x++)
      {
        Cell *cell = gameMap.getCell(x, y);
        if (cell->hasResource())
        {
          resourceTiles.push_back(cell);
        }
      }
    }

    int citiesToBuild = 0;
    for (auto it : player.cities)
    {
      City *city = it.second;
      if (city->fuel > city->getLightUpkeep() * (int) GAME_CONSTANTS["PARAMETERS"]["NIGHT_LENGTH"] + 1000)
      {
        citiesToBuild += 1;
      }
      for (auto citytile : city->citytiles)
      {
        if (citytile->canAct()) {
          if (player.units.size() < player.cityTileCount)
          {
            actions.push_back(citytile->buildWorker());
          } else {
            actions.push_back(citytile->research());
          }
        }
      }
    }

    set<Cell *> movedOnTiles = set<Cell *>();
    set<Cell *> targetedResources = set<Cell *>();

    for (int i = 0; i < player.units.size(); i++)
    {
      Unit unit = player.units[i];
      if (unit.isWorker())
      {
        if (unit.getCargoSpaceLeft() > 0)
        {
          Cell *closestResourceTile;
          float closestDist = 9999999;
          for (auto it = resourceTiles.begin(); it != resourceTiles.end(); it++)
          {
            auto cell = *it;
            if (cell->resource.type == ResourceType::coal && !player.researchedCoal()) continue;
            if (cell->resource.type == ResourceType::uranium && !player.researchedUranium()) continue;
            if (targetedResources.find(cell) == targetedResources.end())
            {
              float dist = cell->pos.distanceTo(unit.pos);
              if (dist < closestDist)
              {
                closestDist = dist;
                closestResourceTile = cell;
              }
            }
          }
          if (closestResourceTile != nullptr)
          {
            targetedResources.insert(closestResourceTile);
            auto dir = unit.pos.directionTo(closestResourceTile->pos);
            auto newCell = gameMap.getCellByPos(unit.pos.translate(dir, 1));
            if (movedOnTiles.find(newCell) == movedOnTiles.end())
            {
              actions.push_back(unit.move(dir));
              movedOnTiles.insert(newCell);
            }
          }
        }
        else
        {
          // if we have cities, return to them
          if (player.cities.size() > 0)
          {
            auto city_iter = player.cities.begin();
            auto city = city_iter->second;

            float closestDist = 999999;
            CityTile *closestCityTile;
            for (auto citytile : city->citytiles)
            {
              float dist = citytile->pos.distanceTo(unit.pos);
              if (dist < closestDist)
              {
                closestCityTile = citytile;
                closestDist = dist;
              }
            }
            if (closestCityTile != nullptr)
            {
              auto dir = unit.pos.directionTo(closestCityTile->pos);
              if (citiesToBuild > 0 && unit.pos.isAdjacent(closestCityTile->pos) && unit.canBuild(gameMap))
              {
                actions.push_back(unit.buildCity());
              }
              else
              {
                auto newcell = gameMap.getCellByPos(unit.pos.translate(dir, 1));
                if (movedOnTiles.find(newcell) == movedOnTiles.end()) {
                  actions.push_back(unit.move(dir));
                  movedOnTiles.insert(newcell);
                }
              }
            }
          }
        }
      }
    }
    
    /** AI Code Goes Above! **/

    /** Do not edit! **/
    for (int i = 0; i < actions.size(); i++)
    {
      if (i != 0)
        cout << ",";
      cout << actions[i];
    }
    cout << endl;
    // end turn
    gameState.end_turn();
  }

  return 0;
}