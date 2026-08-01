#include "ConcreteProperties.h"

ConcreteProperties::ConcreteProperties() :
	fck(30.0),
	gammaC(1.4),
	fcd(0.0),
	factorMultiplierFcd(0.0),
	strainConcretePlastic(0.0),
	strainConcreteRupture(0.0),
	strengthReductionFactor(0.0),
	stressStrainExponent(0.0)
{
	modelType = StressStrainConcreteModelType::PARABOLA_RECTANGLE_NBR6118_2023;
	curveStressStrain = {};
	calculateParameters();
}

void ConcreteProperties::calculateParameters()
{
	if (fck <= 50)
	{
		fcd = fck / gammaC;
		factorMultiplierFcd = 0.85;
		strainConcretePlastic = 2;
		strainConcreteRupture = 3.5;
		stressStrainExponent = 2;
	}
	else
	{
		fcd = fck / gammaC;
		factorMultiplierFcd = 0.85 * (1 - (fck - 50) / 200);
		strainConcretePlastic = 2 + 0.085 * pow((fck - 50), 0.53);
		strainConcreteRupture = 2.6 + 35 * pow(((90 - fck) / 100), 4);
		stressStrainExponent = 1.4 + 23.4 * pow(((90 - fck) / 100), 4);

		if (strainConcretePlastic > strainConcreteRupture)
			strainConcretePlastic = strainConcreteRupture;
	}

	strengthReductionFactor = (fck <= 40) ? 1 : pow((40.0 / fck), (1.0 / 3.0));
}

void ConcreteProperties::setParameters(StressStrainConcreteModelType model, double collectedFck, double collectedGammaC)
{
	modelType = model;
    fck = collectedFck;
    gammaC = collectedGammaC;

    calculateParameters();
}

double ConcreteProperties::computeStress(double strain)
{
	double absStrain = -strain;

	if (absStrain <= 0)
		return 0;

	switch (modelType)
	{
	case StressStrainConcreteModelType::PARABOLA_RECTANGLE_NBR6118_2014:

		if (absStrain <= strainConcretePlastic)

			return 0.85 * fcd * (1 - (pow((1 - (absStrain / strainConcretePlastic)), stressStrainExponent)));
		else if (absStrain <= strainConcreteRupture)
			return 0.85 * fcd;

		return 0;
		break;

	case StressStrainConcreteModelType::PARABOLA_RECTANGLE_NBR6118_2023:

		strengthReductionFactor = (fck <= 40) ? 1 : pow((40.0 / fck), (1.0 / 3.0));

		if (absStrain <= strainConcretePlastic)
			return 0.85 * strengthReductionFactor * fcd * (1 - (pow((1 - (absStrain / strainConcretePlastic)), stressStrainExponent)));
		else if (absStrain <= strainConcreteRupture)
			return 0.85 * strengthReductionFactor * fcd;

		return 0;
		break;

	default:
		return 0;
	}
}

void ConcreteProperties::setCurveStressStrain()
{
	curveStressStrain.clear();

    double step = 0.05; // passo de deformação desejado (ex: 0.05 ‰ = 0.00005)
    int numPoints = static_cast<int>(strainConcreteRupture / step) + 1;

    for (int i = 0; i < numPoints; ++i)
    {
        double strain = -i * step;
        double stress = computeStress(strain);
        curveStressStrain.push_back(Point(-strain, stress));
    }

    // Garante o último ponto, se necessário (por causa de arredondamentos)
    if (curveStressStrain.back().getX() < strainConcreteRupture)
    {
        double stress = computeStress(-strainConcreteRupture);
        curveStressStrain.push_back(Point(strainConcreteRupture, stress));
    }

	// curveStressStrain.clear();

	// double step = (strainConcreteRupture - 0) / 60;

	// for (double strain = 0; strain > -strainConcreteRupture; strain -= step)
	// {
	// 	double stress = computeStress(strain);

	// 	curveStressStrain.push_back(Point(-strain, stress));
	// }
}

double ConcreteProperties::getFck() const
{
	return fck;
}

double ConcreteProperties::getGammaC() const
{
	return gammaC;
}

double ConcreteProperties::getFcd() const
{
	return fcd;
}

double ConcreteProperties::getFactorMultiplierFcd() const
{
    return factorMultiplierFcd;
}

double ConcreteProperties::getStrainConcretePlastic() const
{
	return strainConcretePlastic;
}

double ConcreteProperties::getStrainConcreteRupture() const
{
	return strainConcreteRupture;
}

double ConcreteProperties::getStrenghtReductionFactor() const
{
	return strengthReductionFactor;
}

double ConcreteProperties::getStressStrainExponent() const
{
	return stressStrainExponent;
}

vector<Point> ConcreteProperties::getCurveStressStrain() const
{
	return curveStressStrain;
}

StressStrainConcreteModelType ConcreteProperties::getModelType() const
{
	return modelType;
}