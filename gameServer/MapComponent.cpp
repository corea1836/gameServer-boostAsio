#include "MapComponent.h"
#include <chrono>
#include <cstddef>
#include <fstream>
#include <memory>
#include <string>

void MapComponent::Init(const string &collisionFilePath)
{
    LoadCollision(collisionFilePath);
}

bool MapComponent::CanGo(float x, float y) const
{
    int32 cx, cy;
    if (WorldToCell(x, y, cx, cy) == false)
        return false;

    return true;
}

void MapComponent::LoadCollision(const string &filePath)
{
    std::ifstream in(filePath);
    string line;

    std::getline(in, line);
    _minX = std::stoi(line);
    std::getline(in, line);
    _maxX = std::stoi(line);
    std::getline(in, line);
    _minY = std::stoi(line);
    std::getline(in, line);
    _maxY = std::stoi(line);

    _width = _maxX - _minX + 1;
    _height = _maxY - _minY + 1;

    _collision = std::make_unique<byte[]>(_width * _height);

    for (int32 y = 0; y < _height; y++)
    {
        std::getline(in, line);
        for (int32 x = 0; x < _width; x++)
            _collision[CellIndex(x, y)] = (line[x] == '1') ? 1 : 0;
    }
}

bool MapComponent::WorldToCell(float x, float y, int32 &outCellX,
                               int32 &outCellY) const
{
    int32 fx = static_cast<int32>(std::floor(x));
    int32 fy = static_cast<int32>(std::floor(y));

    if (fx < _minX || fx > _maxX)
        return false;
    if (fy < _minY || fy > _maxY)
        return false;

    outCellX = fx - _minX;
    outCellY = _maxY - fy;

    return (outCellX >= 0 && outCellX < _width && outCellY >= 0 &&
            outCellY < _height);
}
