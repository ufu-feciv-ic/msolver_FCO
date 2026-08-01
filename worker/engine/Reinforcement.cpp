#include "Reinforcement.h"

Reinforcement::Reinforcement()
{
	reinforcement = {};
	diameters = {};
	areas = {};
	effectiveDepth = 0;
    totalArea = 0;
}

void Reinforcement::setReinforcement(std::vector<Point> collectedReinforcement, std::vector<double> collectedDiameters)
{
	for (size_t i = 0; i < collectedReinforcement.size(); i++)
	{
		double coordX = collectedReinforcement[i].getX();
		double coordY = collectedReinforcement[i].getY();

		reinforcement.push_back(Point(coordX, coordY));

		double diameter = collectedDiameters[i]; // mm

		diameters.push_back(diameter);
	}
}

// Reinforcement.cpp
void Reinforcement::addReinforcement(double coordX, double coordY, double diameter)
{
    reinforcement.push_back(Point(coordX, coordY));
    diameters.push_back(diameter);
    
    // Diâmetro em mm, área em cm^2
    double tempArea = M_PI * std::pow(diameter / 20.0, 2); 
    if (diameter <= 0) tempArea = 0.0; // Garante area zero para diametro zero
    areas.push_back(tempArea);
}

void Reinforcement::removeLastReinforcement()
{
	reinforcement.pop_back();
	diameters.pop_back();
	areas.pop_back();
}

void Reinforcement::clearReinforcement()
{
	reinforcement.clear();
	diameters.clear();
	areas.clear();
    totalArea = 0.0; 
}

// Reinforcement.cpp
void Reinforcement::computeArea()
{
    areas.clear();
    totalArea = 0.0;

    for (size_t i = 0; i < diameters.size(); ++i) // Itera sobre os diâmetros
    {
        double currentDiameter_mm = diameters[i];
         
        double tempDiameter_cm = currentDiameter_mm / 10.0; // mm para cm
        double tempArea_cm2 = std::pow(tempDiameter_cm, 2) * M_PI / 4.0;

        // Garante que diâmetros inválidos (<=0) resultem em área zero
        if (currentDiameter_mm <= 0) {
            tempArea_cm2 = 0.0;
        }
        
        areas.push_back(tempArea_cm2); // Adiciona a área calculada

        totalArea += tempArea_cm2;
    }
}

void Reinforcement::translateToCentroidPolygon(Point centroid)
{
	for (size_t i = 0; i < reinforcement.size(); i++)
	{
		double coordX = reinforcement[i].getX() - centroid.getX();
		double coordY = reinforcement[i].getY() - centroid.getY();

		reinforcement[i].setX(coordX);
		reinforcement[i].setY(coordY);
	}
}

void Reinforcement::rotateAroundCentroidPolygon(double angle)
{
	double rad = angle * 3.14159265358979323846 / 180;
	double cosAngle = cos(rad);
	double sinAngle = sin(rad);

	for (size_t i = 0; i < reinforcement.size(); i++)
	{
		double rotateX = (reinforcement[i].getX() * cosAngle) - (reinforcement[i].getY() * sinAngle);
		double rotateY = (reinforcement[i].getX() * sinAngle) + (reinforcement[i].getY() * cosAngle);
		
		reinforcement[i].setX(rotateX);
		reinforcement[i].setY(rotateY);
	}
}

void Reinforcement::setEffectiveDepth(double maxY)
{
	double temp = 0;

	for (size_t i = 0; i < reinforcement.size(); i++)
	{
		if (reinforcement[i].getY() <= temp)
			temp = reinforcement[i].getY();
	}

	if (maxY > temp)
	{
		effectiveDepth = maxY - temp; // cm
	}
	else
	{
		effectiveDepth = 0;
	}
}

vector<Point> Reinforcement::getReinforcement() const
{
	return reinforcement;
}

vector<double> Reinforcement::getDiameters() const
{
	return diameters;
}

vector<double> Reinforcement::getAreas() const
{
	return areas;
}

double Reinforcement::getEffectiveDepth() const
{
    return effectiveDepth;
}

double Reinforcement::getTotalArea() const
{
    return totalArea; // Retorna a área total calculada
}

void Reinforcement::SetNumPoints(int numPointsInput)
{
    if (numPointsInput < 0) numPointsInput = 0; // Garante que não é negativo

    size_t newSize = static_cast<size_t>(numPointsInput);
    size_t oldSize = reinforcement.size();

    // Redimensiona todos os três vetores
    reinforcement.resize(newSize);
    diameters.resize(newSize);
    areas.resize(newSize); // Redimensiona o vetor de áreas também

    if (newSize > oldSize)
    {
        for (size_t i = oldSize; i < newSize; ++i)
        {
            reinforcement[i] = Point(0.0, 0.0); // Ponto padrão
            diameters[i] = 10.0; // DIÂMETRO PADRÃO NÃO-ZERO (10 mm)
            areas[i] = 0.0; // A área será recalculada pelo computeArea() posteriormente          
        }
    }
    
}

int Reinforcement::GetNumPoints() const
{
    return reinforcement.size();  
}

void Reinforcement::GetTableData(int index, double* x, double* y, double* d) const
{
    if (index < 0 || index >= GetNumPoints()) return;
    *x = reinforcement[index].getX();  
    *y = reinforcement[index].getY();  
	*d = diameters[index];  
}

// Reinforcement.cpp
void Reinforcement::SetTableData(int index, double x, double y, double d)
{
    if (index < 0 || index >= GetNumPoints()) return;

    reinforcement[index].setX(x);
    reinforcement[index].setY(y);
    diameters[index] = d;

    // IMPORTANTE: Recalcular a área individual da barra.
    // Isso é redundante se computeArea() for chamado logo em seguida na Interface.
    // MAS, para robustez da classe Reinforcement em si, é bom que a área de 'areas[index]'
    // reflita o 'diameters[index]' assim que ele é setado.
    // Duplicar o calculo da area aqui:
    double tempDiameter_cm = d / 10.0; // mm para cm
    double tempArea_cm2 = std::pow(tempDiameter_cm, 2) * M_PI / 4.0;
    if (d <= 0) tempArea_cm2 = 0.0;
    areas[index] = tempArea_cm2; // Atualiza a area para este indice
}


void Reinforcement::addNewDefaultReinforcement(double defaultDiameter) {
    addReinforcement(0.0, 0.0, defaultDiameter);
}