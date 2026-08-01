#include "PolygonStressRegions.h"

PolygonStressRegions::PolygonStressRegions()
{
    originalPolygon = Polygon();
    compressedRegion = Polygon();
    parabolicRegion = Polygon();
    rectangularRegion = Polygon();
    neutralAxisHeight = 0.0;
    plasticHeight = 0.0;
    ruptureHeight = 0.0;
}

void PolygonStressRegions::setOriginalPolygon(const Polygon &polygon)
{
    originalPolygon = polygon;
}

void PolygonStressRegions::setDeformationHeight(double xLN, double xEc2, double xEcpu)
{
    double maxY = originalPolygon.getMaxY();

    neutralAxisHeight = maxY - xLN;
    plasticHeight = maxY - xEc2;
    ruptureHeight = maxY - xEcpu;
}

Point PolygonStressRegions::computeIntersection(Point firstPoint, Point secondPoint, double cutCoordY)
{
    double dx = secondPoint.getX() - firstPoint.getX();
    double dy = secondPoint.getY() - firstPoint.getY();

    double t = (cutCoordY - firstPoint.getY()) / dy;

    return Point((firstPoint.getX() + t * dx), cutCoordY);
}

Polygon PolygonStressRegions::regionAboveHeight(Polygon polygon, bool upperCut, double cutCoordY)
{
    vector<Point> resultVertices;
    const auto& vertices = polygon.getPolygonVertices();

    for (size_t i = 0; i < vertices.size(); ++i) 
    {
        Point current = vertices[i];
        Point next = vertices[(i + 1) % vertices.size()];

        bool currentKeep = upperCut ? current.getY() >= cutCoordY : current.getY() <= cutCoordY;
        bool nextKeep = upperCut ? next.getY() >= cutCoordY : next.getY() <= cutCoordY;

        if (currentKeep)
            resultVertices.push_back(current);

        if (currentKeep != nextKeep) 
        {
            Point intersection = computeIntersection(current, next, cutCoordY);
            resultVertices.push_back(intersection);
        }
    }

    Polygon result;
    result.setVertices(resultVertices);
    return result;
}

Polygon PolygonStressRegions::regionBetweenHeights(Polygon polygon, double upper, double lower)
{
    Polygon lowerCut = regionAboveHeight(polygon, true, lower);   // remove o que está abaixo de lower
    Polygon between = regionAboveHeight(lowerCut, false, upper); // remove o que está acima de upper
    return between;
}

void PolygonStressRegions::generateStressRegions()
{
    compressedRegion = regionAboveHeight(originalPolygon, true, neutralAxisHeight);
    parabolicRegion = regionBetweenHeights(originalPolygon, plasticHeight, neutralAxisHeight);
    rectangularRegion = regionBetweenHeights(originalPolygon, ruptureHeight, plasticHeight);
}

void PolygonStressRegions::clearStressRegions()
{
    compressedRegion.clearPolygonVertices();
    parabolicRegion.clearPolygonVertices();
    rectangularRegion.clearPolygonVertices();
}

Polygon PolygonStressRegions::getOriginalPolygon() const
{
    return originalPolygon;
}

Polygon PolygonStressRegions::getCompressedRegion() const
{
    return compressedRegion;
}

Polygon PolygonStressRegions::getParabolicRegion() const
{
    return parabolicRegion;
}

Polygon PolygonStressRegions::getRectangularRegion() const
{
    return rectangularRegion;
}

double PolygonStressRegions::getNeutralAxisHeight() const
{
    return neutralAxisHeight;
}

double PolygonStressRegions::getPlasticHeight() const
{
    return plasticHeight;
}

double PolygonStressRegions::getRuptureHeight() const
{
    return ruptureHeight;
}