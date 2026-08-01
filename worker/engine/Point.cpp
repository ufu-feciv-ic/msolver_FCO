#include "Point.h"

Point::Point()
{
    x = 0;
    y = 0;
}

Point::Point(double coordX, double coordY)
{
    x = coordX;
    y = coordY;
}

double Point::getX() const { return x; }

double Point::getY() const { return y; }

Point Point::getPoint() const { return Point(x, y); }

double Point::setX(double coordX) { return x = coordX; }

double Point::setY(double coordY) { return y = coordY; }

Point Point::setPoint(double coordX, double coordY)
{
    return Point(coordX, coordY);
}