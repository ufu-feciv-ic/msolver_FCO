#pragma once

#include "Point.h"

#include <cmath>
#include <vector>

enum class StressStrainConcreteModelType
{
	PARABOLA_RECTANGLE_NBR6118_2014,
	PARABOLA_RECTANGLE_NBR6118_2023
};

class ConcreteProperties
{
private:
	double fck;
	double gammaC;
	double fcd;
	double factorMultiplierFcd;
	double strainConcretePlastic;
	double strainConcreteRupture;
	double strengthReductionFactor;
	double stressStrainExponent;
	vector<Point> curveStressStrain;
	StressStrainConcreteModelType modelType;

	friend class cereal::access;
	template <class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(fck),
			CEREAL_NVP(gammaC),
			CEREAL_NVP(fcd),
			CEREAL_NVP(factorMultiplierFcd),
			CEREAL_NVP(strainConcretePlastic),
			CEREAL_NVP(strainConcreteRupture),
			CEREAL_NVP(strengthReductionFactor),
			CEREAL_NVP(stressStrainExponent),
			CEREAL_NVP(curveStressStrain),
			CEREAL_NVP(modelType)
		);
	}

public:
	ConcreteProperties();

    void calculateParameters();
    void setParameters(StressStrainConcreteModelType model, double collectedFck, double collectedGammaC);
	double computeStress(double strain);
	void setCurveStressStrain();

	double getFck() const;
	double getGammaC() const;
	double getFcd() const;
	double getFactorMultiplierFcd() const;
	double getStrainConcretePlastic() const;
	double getStrainConcreteRupture() const;
	double getStrenghtReductionFactor() const;
	double getStressStrainExponent() const;
	vector<Point> getCurveStressStrain() const;
    StressStrainConcreteModelType getModelType() const;
};
