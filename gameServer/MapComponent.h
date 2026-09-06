#pragma once

#include <Types.h>
#include <memory>

class Vector3;

class MapComponent
{
    public:
        MapComponent();
        ~MapComponent() = default;

        void Init(const string &collisionFilePath);
        bool CanGo(float x, float y) const;

    private:
        void LoadCollision(const string &filePath);

        int32 CellIndex(int32 x, int32 y) const { y *_width + x; }

        bool WorldToCell(float x, float y, int32 &outCellX,
                         int32 &outCellY) const;

        int32 _minX = 0.f;
        int32 _maxX = 0.f;
        int32 _minY = 0.f;
        int32 _maxY = 0.f;

        int32 _width = 0;
        int32 _height = 0;
        std::unique_ptr<byte[]> _collision;
};
