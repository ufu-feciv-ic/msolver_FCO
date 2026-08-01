#pragma once

#include "Point.h"
#include "Polygon.h"
#include "Reinforcement.h"
#include "ConcreteProperties.h"
#include "SteelProperties.h"
#include "StrainDistribution.h"
#include "PolygonStressRegions.h"
#include "AnalyticalIntegration.h"
#include "InternalForces.h"
#include "combination.h"
#include "MomentSolver.h"

using std::vector;
using std::cout;
using std::endl;

class Section
{
public:
    Polygon originalPolygon;
    Polygon workingPolygon;
    Reinforcement originalReinforcement;
    Reinforcement workingReinforcement;
    ConcreteProperties concrete;
    SteelProperties steel;
    StrainDistribution strainDistribution;
    PolygonStressRegions stressRegions;
    AnalyticalIntegration analyticalIntegration;
    InternalForces internalForces;
    MomentSolver momentSolver;

    vector<Combination> combinations;
    vector<Point> envelopeMoments;
    double version = 0.1;

    Section();

    void updateGeometricProperties();
    void defineGeometry(const Polygon& polygon, const Reinforcement& reinforcement);
    void definePolygon(const Polygon &polygon);
    void defineReinforcement(const Reinforcement &reinforcement);
    void defineMaterials(const ConcreteProperties &c, const SteelProperties &s);
    void defineIntegrationModel(NormativeIntegrationVersion modelVersion);

    void resetWorkingState();
    void applyAngleandCenterline(double angleDegrees);

    void verifyEquilibrium(double Nsd, double eps1, double eps2);
    void computeEquilibrium(double Nsd, double eps1, double eps2);
    void computeEnvelope(double Nsd);
    bool isMomentSafe(const Point& momentPoint) const;

    const Polygon& getWorkingPolygon() const { return workingPolygon; }
    const Reinforcement& getWorkingReinforcement() const { return workingReinforcement; }
    const vector<Point>& getEnvelopeMoments() const { return envelopeMoments; }
    const MomentSolver& getMomentSolver() const { return momentSolver; }
    const InternalForces& getInternalForces() const { return internalForces; }
    const PolygonStressRegions& getStressRegions() const { return stressRegions; }
    std::vector<Combination> getCombinations() const { return combinations; }

    //void computeSectionEquilibriumSolver(double Nsd);
    //void computeSectionMomentEnvelope(double Nsd);
    void printSectionData();

    template <class Archive>
    void save(Archive& archive) const
    {
        archive(
            CEREAL_NVP(version),
            CEREAL_NVP(originalPolygon),
            CEREAL_NVP(originalReinforcement),
            CEREAL_NVP(concrete),
            CEREAL_NVP(steel),
            CEREAL_NVP(combinations)
        );
    }

    template <class Archive>
    void load(Archive& archive)
    {
        archive(
            CEREAL_NVP(version),
            CEREAL_NVP(originalPolygon),
            CEREAL_NVP(originalReinforcement),
            CEREAL_NVP(concrete),
            CEREAL_NVP(steel),
            CEREAL_NVP(combinations)
        );

        defineGeometry(originalPolygon, originalReinforcement);
        defineMaterials(concrete, steel);
    }
};
