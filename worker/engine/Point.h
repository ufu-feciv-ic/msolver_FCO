#pragma once

#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

using std::vector;

class Point
{
private:
    double x;
    double y;

    friend class cereal::access;
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(x), CEREAL_NVP(y));
    }

public:
    Point();
    Point(double coordX, double coordY);

    double getX() const;
    double getY() const;
    Point getPoint() const;

    double setX(double coordX);
    double setY(double coordY);
    Point setPoint(double coordX, double coordY);
};