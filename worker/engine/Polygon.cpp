#include "Polygon.h"

Polygon::Polygon()
{
    polygonVertices = {};       // Inicializando o vetor de vértices
    geometricCenter = Point(0, 0); // Centro geométrico inicial
    maxY = 0;
    minY = 0;
    polygonHeight = 0;
    polygonArea = 0;
    staticMomentX = 0;
    staticMomentY = 0;
    numPoints = 0;
	angle = 0;
    inertiaX = 0;
    inertiaY = 0;
    inertiaXY = 0;
    inertiaX_cg = 0;
    inertiaY_cg = 0;
    inertiaXY_cg = 0;
}

void Polygon::setVertices(std::vector<Point> collectedVertices)
{
    polygonVertices.clear();  // Limpa os vértices atuais
    for (size_t i = 0; i < collectedVertices.size(); i++)
    {
        double coordX = collectedVertices[i].getX();
        double coordY = collectedVertices[i].getY();
        polygonVertices.push_back(Point(coordX, coordY));  // Adiciona os novos vértices
    }
}

void Polygon::addVertice(double coordX, double coordY)
{
    polygonVertices.push_back(Point(coordX, coordY));  // Adiciona um novo vértice
}

void Polygon::setAngle(double collectedAngle)
{
	angle = collectedAngle;
}

void Polygon::removeLastVertice()
{
    if (!polygonVertices.empty())
        polygonVertices.pop_back();  // Remove o último vértice
}

void Polygon::clearPolygonVertices()
{
    polygonVertices.clear();  // Limpa todos os vértices
    geometricCenter = Point(0, 0);  // Redefine o centro geométrico
    maxY = 0;
    minY = 0;
    polygonHeight = 0;
    polygonArea = 0;
    staticMomentX = 0;
    staticMomentY = 0;
    inertiaX = 0;
    inertiaY = 0;
    inertiaXY = 0;
    inertiaX_cg = 0;
    inertiaY_cg = 0;
    inertiaXY_cg = 0;
}

void Polygon::clearGeometricCenter()
{
    geometricCenter = Point(0, 0);  // Limpa o centro geométrico
}

void Polygon::computeMaxCoordY()
{
    if (polygonVertices.empty()) return;

    maxY = polygonVertices[0].getY();
    for (size_t i = 0; i < polygonVertices.size(); i++)
    {
        if (polygonVertices[i].getY() > maxY)
            maxY = polygonVertices[i].getY();
    }
}

void Polygon::computeMinCoordY()
{
    if (polygonVertices.empty()) return;

    minY = polygonVertices[0].getY();
    for (size_t i = 0; i < polygonVertices.size(); i++)
    {
        if (polygonVertices[i].getY() < minY)
            minY = polygonVertices[i].getY();
    }
}

void Polygon::computeHeight()
{
    computeMaxCoordY();
    computeMinCoordY();
    polygonHeight = maxY - minY;
}

void Polygon::computeArea()
{
    polygonArea = 0;
    int polygonSize = polygonVertices.size();
    for (int i = 0; i < polygonSize; i++)
    {
        int j = (i + 1) % polygonSize;
        double factor = polygonVertices[i].getX() * polygonVertices[j].getY() - polygonVertices[j].getX() * polygonVertices[i].getY();
        polygonArea += factor;
    }
    polygonArea = polygonArea / 2;
}

void Polygon::computeStaticMomentX()
{
    staticMomentX = 0;
    int polygonSize = polygonVertices.size();
    for (int i = 0; i < polygonSize; i++)
    {
        int j = (i + 1) % polygonSize;
        double factor = polygonVertices[i].getX() * polygonVertices[j].getY() - polygonVertices[j].getX() * polygonVertices[i].getY();
        staticMomentX += (polygonVertices[i].getY() + polygonVertices[j].getY()) * factor;
    }
    staticMomentX /= 6;
}

void Polygon::computeStaticMomentY()
{
    staticMomentY = 0;
    int polygonSize = polygonVertices.size();
    for (int i = 0; i < polygonSize; i++)
    {
        int j = (i + 1) % polygonSize;
        double factor = polygonVertices[i].getX() * polygonVertices[j].getY() - polygonVertices[j].getX() * polygonVertices[i].getY();
        staticMomentY += (polygonVertices[i].getX() + polygonVertices[j].getX()) * factor;
    }
    staticMomentY /= 6;
}

void Polygon::computeCentroid()
{
    computeArea();
    computeStaticMomentX();
    computeStaticMomentY();

    geometricCenter.setX(staticMomentY / polygonArea);
    geometricCenter.setY(staticMomentX / polygonArea);
}

void Polygon::translateToCentroid()
{
    computeCentroid();
    for (size_t i = 0; i < polygonVertices.size(); i++)
    {
        double coordX = polygonVertices[i].getX() - geometricCenter.getX();
        double coordY = polygonVertices[i].getY() - geometricCenter.getY();
        polygonVertices[i].setX(coordX);
        polygonVertices[i].setY(coordY);
    }
}

void Polygon::rotateAroundCentroid()
{
    double rad = angle * 3.14159265358979323846 / 180;
    double cosAngle = cos(rad);
    double sinAngle = sin(rad);

    for (size_t i = 0; i < polygonVertices.size(); i++)
    {
        double rotateX = (polygonVertices[i].getX() * cosAngle) - (polygonVertices[i].getY() * sinAngle);
        double rotateY = (polygonVertices[i].getX() * sinAngle) + (polygonVertices[i].getY() * cosAngle);
        
        polygonVertices[i].setX(rotateX);
        polygonVertices[i].setY(rotateY);
    }
}

void Polygon::computeInertia()
{
    if (polygonVertices.size() < 3) return;
    
    inertiaX = 0;
    inertiaY = 0;  
    inertiaXY = 0;

    int polygonSize = polygonVertices.size();

    for (int i = 0; i < polygonSize; i++)
    {
        int j = (i + 1) % polygonSize;
        double xi = polygonVertices[i].getX();
        double yi = polygonVertices[i].getY();
        double xj = polygonVertices[j].getX();
        double yj = polygonVertices[j].getY();

        double factor = (xi * yj) - (xj * yi);

        inertiaX += (yi * yi + yi * yj + yj * yj) * factor;
        inertiaY += (xi * xi + xi * xj + xj * xj) * factor;
        inertiaXY += (xi * yj + 2 * xi * yi + 2 * xj * yj + xj * yi) * factor;
    }

    inertiaX /= 12.0;
    inertiaY /= 12.0;
    inertiaXY /= 24.0;

    // Teorema dos Eixos Paralelos para mover a inércia da origem para o centroide
    double cg_x = geometricCenter.getX();
    double cg_y = geometricCenter.getY();

    inertiaX_cg = inertiaX - polygonArea * cg_y * cg_y;
    inertiaY_cg = inertiaY - polygonArea * cg_x * cg_x;
    inertiaXY_cg = inertiaXY - polygonArea * cg_x * cg_y;
}

std::vector<Point> Polygon::getPolygonVertices() const
{
    return polygonVertices;  // Retorna os vértices do polígono
}

Point Polygon::getGeometricCenter() const
{
    return geometricCenter;  // Retorna o centro geométrico do polígono
}

double Polygon::getAngle() const
{
    return angle;
}

double Polygon::getMaxY() const
{
    return maxY;  // Retorna o valor de Y máximo
}

double Polygon::getMinY() const
{
    return minY;  // Retorna o valor de Y mínimo
}

double Polygon::getPolygonHeight() const
{
    return polygonHeight;  // Retorna a altura do polígono
}

double Polygon::getPolygonArea() const
{
    return polygonArea;  // Retorna a área do polígono
}

double Polygon::getVet0X() const
{
    if (!polygonVertices.empty())
        return polygonVertices[0].getX();  // Retorna a coordenada X do primeiro vértice
    else
        return 1;
}

double Polygon::getVet0Y() const
{
    if (!polygonVertices.empty())
        return polygonVertices[0].getY();  // Retorna a coordenada Y do primeiro vértice
    else
        return 1;
}

double Polygon::getInertiaX_cg() const
{
    return inertiaX_cg;
}

double Polygon::getInertiaY_cg() const
{
    return inertiaY_cg;
}

double Polygon::getInertiaXY_cg() const
{
    return inertiaXY_cg;
}

void Polygon::SetNumPoints(int numPointsInput)
{
    numPoints = numPointsInput;

    if (numPoints < 0) numPoints = 0;
    polygonVertices.resize(numPoints);  // Ajusta o tamanho do vetor de vértices
}

int Polygon::GetNumPoints() const
{
    return polygonVertices.size();  // Retorna o número de pontos
}

void Polygon::GetTableData(int index, float* x, float* y) const
{
    if (index < 0 || index >= GetNumPoints()) return;
    *x = polygonVertices[index].getX();  // Obtém a coordenada X do vértice
    *y = polygonVertices[index].getY();  // Obtém a coordenada Y do vértice
}

void Polygon::SetTableData(int index, float x, float y)
{
    if (index < 0 || index >= GetNumPoints()) return;
    polygonVertices[index].setX(x);  // Define a coordenada X do vértice
    polygonVertices[index].setY(y);  // Define a coordenada Y do vértice
}
