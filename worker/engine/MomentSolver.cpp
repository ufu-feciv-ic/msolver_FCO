#include "MomentSolver.h"

MomentSolver::MomentSolver()
{
    maxIterations = 0;
    iterations = 0;
    tolerance = 0;
    axialForceSum = 0;
    converged = false;
    Mrd = Point();
    strainResult = StrainDistribution();
}

void MomentSolver::solveEquilibrium(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete,
                                            SteelProperties &steel, StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions,
                                            AnalyticalIntegration &analyticalIntegration, InternalForces &internalForces, double Nsd)
{
    double eps1Region12 = -concrete.getStrainConcreteRupture();
    double eps2Region12 = 0;
    double axialForceRegion12Sum = computeAxialForceResultant(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions,
                                                              analyticalIntegration, internalForces, eps1Region12, eps2Region12, Nsd);

    // std::cout << "eps1: " << strainDistribution.getTopFiberStrain() << std::endl;
    // std::cout << "eps2: " << strainDistribution.getBottomFiberStrain() << std::endl;
    // std::cout << "Soma limite regiao 1-2: " << axialForceRegion12Sum << std::endl;

    const auto &vectorReinforcement = reinforcement.getReinforcement();
    double temp = 0;
    for (size_t i = 0; i < vectorReinforcement.size(); i++)
    {
        if (vectorReinforcement[i].getY() <= temp)
            temp = vectorReinforcement[i].getY();
    }

    double effectiveDepth = polygon.getMaxY() - temp;

    // std::cout << "d: " << effectiveDepth << std::endl;

    double eps1Region23 = -concrete.getStrainConcreteRupture();
    double eps2Region23 = ((steel.getStrainSteelRupture() - eps1Region23) *
                           (polygon.getPolygonHeight() / effectiveDepth)) +
                          eps1Region23;

    double axialForceRegion23Sum = computeAxialForceResultant(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions,
                                                              analyticalIntegration, internalForces, eps1Region23, eps2Region23, Nsd);

    // std::cout << "eps1: " << strainDistribution.getTopFiberStrain() << std::endl;
    // std::cout << "eps2: " << strainDistribution.getBottomFiberStrain() << std::endl;
    // std::cout << "Soma limite regiao 2-3: " << axialForceRegion23Sum << std::endl;

    if (axialForceRegion23Sum <= 0)
    {
        // std::cout << "Regiao 3" << std::endl;
        // Mrd = iterateInRegion3(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions,
           //                    analyticalIntegration, internalForces, Nsd);
        Mrd = testRegion3(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions, analyticalIntegration, internalForces, Nsd);
    }
    else if (axialForceRegion12Sum <= 0)
    {
        // std::cout << "Regiao 2" << std::endl;
        // Mrd = iterateInRegion2(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions,
        //                        analyticalIntegration, internalForces, Nsd);
        Mrd = testRegion2(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions, analyticalIntegration, internalForces, Nsd);
    }
    else
    {
        // std::cout << "Regiao 1" << std::endl;
        // Mrd = iterateInRegion1(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions,
        //                        analyticalIntegration, internalForces, Nsd);
        Mrd = testRegion1(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions, analyticalIntegration, internalForces, Nsd);
    }
}

double MomentSolver::computeAxialForceResultant(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel,
                                                        StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
                                                        InternalForces &internalForces, double strain1, double strain2, double Nsd)
{
    if (strain1 == strain2 && strain1 == -concrete.getStrainConcretePlastic())
    {
        internalForces.setNormalSolicitation(Nsd);
        internalForces.computeMaxCompression(polygon, reinforcement, steel, concrete);
        // std::cout << "sumNormal(comp-Nsd): " << internalForces.getMaxNormalCompression() - Nsd << std::endl; 
        return internalForces.getMaxNormalCompression() - Nsd;
    }
    else if (strain1 == strain2 && strain1 == steel.getStrainSteelRupture())
    {
        internalForces.setNormalSolicitation(Nsd);
        internalForces.computeMaxTraction(polygon, reinforcement, steel);
        // std::cout << "sumNormal(trac-Nsd): " << internalForces.getMaxNormalTraction() - Nsd << std::endl;
        return internalForces.getMaxNormalTraction() - Nsd;
    }
    else
    {
        setStrainDistribution(polygon, concrete, strainDistribution, strain1, strain2);
        setStressRegions(polygon, strainDistribution, stressRegions);
        setInternalForces(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions, analyticalIntegration, internalForces, Nsd);
        // std::cout << "sumNormal(comp+trac-Nsd): " << internalForces.getNormalConcrete() + internalForces.getNormalSteel() - Nsd << std::endl;
        return internalForces.getNormalConcrete() + internalForces.getNormalSteel() - Nsd;
    }
}

Point MomentSolver::computeMomentResultant(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel,
                                                    StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
                                                    InternalForces &internalForces, double strain1, double strain2, double Nsd)
{
    setStrainDistribution(polygon, concrete, strainDistribution, strain1, strain2);
    setStressRegions(polygon, strainDistribution, stressRegions);
    setInternalForces(polygon, reinforcement, concrete, steel, strainDistribution, stressRegions, analyticalIntegration, internalForces, Nsd);
    double Mrdxx = internalForces.getMomentXXSection();
    double Mrdyy = internalForces.getMomentYYSection();
    Point result = {0, 0};
    result.setX(Mrdyy);
    result.setY(Mrdxx);
    return result;
}

void MomentSolver::setStrainDistribution(Polygon &polygon, ConcreteProperties &concrete, StrainDistribution &strainDistribution,
                                                 double strain1, double strain2)
{
    strainDistribution.setStrain(strain1, strain2);
    strainDistribution.computeStrainDistribution(concrete.getStrainConcretePlastic(),
                                                 concrete.getStrainConcreteRupture(), polygon.getPolygonHeight());
}

void MomentSolver::setStressRegions(Polygon &polygon, StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions)
{
    stressRegions.setOriginalPolygon(polygon);
    stressRegions.setDeformationHeight(strainDistribution.getNeutralAxisCoord(),
                                       strainDistribution.getPlasticStrainCoord(), strainDistribution.getRuptureStrainCoord());
    stressRegions.generateStressRegions();
}

void MomentSolver::setInternalForces(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel,
                                             StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration,
                                             InternalForces &internalForces, double Nsd)
{
    internalForces.setNormalSolicitation(Nsd);
    internalForces.computeNormalConcrete(analyticalIntegration, concrete, stressRegions, strainDistribution);
    internalForces.computeMomentUUConcrete(analyticalIntegration, concrete, stressRegions, strainDistribution);
    internalForces.computeMomentVVConcrete(analyticalIntegration, concrete, stressRegions, strainDistribution);
    internalForces.computeNormalSteel(polygon, reinforcement, steel, strainDistribution);
    internalForces.computeMomentUUSteel(polygon, reinforcement, steel, strainDistribution);
    internalForces.computeMomentVVSteel(polygon, reinforcement, steel, strainDistribution);
    internalForces.computeMomentUUSection();
    internalForces.computeMomentVVSection();
    internalForces.computeMomentXXSection(polygon.getAngle());
    internalForces.computeMomentYYSection(polygon.getAngle());
}

double MomentSolver::findRootBrent(std::function<double(double)> func, double a, double b, double tol, int maxIter)
{
    double fa = func(a);
    double fb = func(b);

    if (fa * fb >= 0.0)
    {
        std::cerr << "Erro: Brent requer fa * fb < 0 (raiz não isolada no intervalo)" << std::endl;
        return a;
    }

    double c = a;
    double fc = fa;
    double s = b;
    double fs = fb;
    double d = b - a;
    bool mflag = true;

    for (int iter = 0; iter < maxIter; ++iter)
    {
        if (fa != fc && fb != fc)
        {
            // Interpolação inversa quadrática
            s = a * fb * fc / ((fa - fb) * (fa - fc))
              + b * fa * fc / ((fb - fa) * (fb - fc))
              + c * fa * fb / ((fc - fa) * (fc - fb));
        }
        else
        {
            // Método da secante
            s = b - fb * (b - a) / (fb - fa);
        }

        double cond1 = (s < (3 * a + b) / 4 || s > b);
        double cond2 = (mflag && std::abs(s - b) >= std::abs(b - c) / 2);
        double cond3 = (!mflag && std::abs(s - b) >= std::abs(c - d) / 2);
        double cond4 = (mflag && std::abs(b - c) < tol);
        double cond5 = (!mflag && std::abs(c - d) < tol);

        if (cond1 || cond2 || cond3 || cond4 || cond5) 
        {
            // Bisseção
            s = (a + b) / 2;
            mflag = true;
        } 
        else 
        {
            mflag = false;
        }

        fs = func(s);
        d = c;
        c = b;
        fc = fb;

        if (fa * fs < 0) 
        {
            b = s;
            fb = fs;
        } 
        else 
        {
            a = s;
            fa = fs;
        }

        if (std::abs(fa) < std::abs(fb)) 
        {
            std::swap(a, b);
            std::swap(fa, fb);
        }

        if (std::abs(b - a) < tol || std::abs(fs) < tol) 
        {
            iterations = iter;
            return s;
        }

        iterations = iter;
    }

    std::cerr << "Brent não convergiu em " << maxIter << " iterações." << std::endl;
    return s;
}

Point MomentSolver::testRegion1(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration, InternalForces &internalForces, double Nsd)
{
    double epcu = -concrete.getStrainConcreteRupture();
    double epc2 = -concrete.getStrainConcretePlastic();
    double h = polygon.getPolygonHeight();

    double x_plastic = h * ((epcu - epc2) / epcu);
    double y_plastic = epc2;

    auto func = [&](double k) {
        double epsTop = y_plastic + k * (-x_plastic);
        double epsBottom = y_plastic + k * (h - x_plastic);
        return computeAxialForceResultant(
            polygon, reinforcement, concrete, steel,
            strainDistribution, stressRegions,
            analyticalIntegration, internalForces,
            epsTop, epsBottom, Nsd
        );
    };

    double k_min = (-epcu) / h;
    double k_max = 0;

    if (func(k_min) * func(k_max) >= 0.0) {
        converged = false;
        std::cerr << "Erro: Brent - raiz não isolada no intervalo (Região 1)" << std::endl;
        return Point(0,0);
    }

    iterations = 0;
    maxIterations = 100;
    tolerance = 1e-5;

    double k_root = findRootBrent(func, k_min, k_max, tolerance, maxIterations);
    double epsTop = y_plastic + k_root * (-x_plastic);
    double epsBottom = y_plastic + k_root * (h - x_plastic);

    strainResult.setStrain(epsTop, epsBottom);
    converged = true;

    return computeMomentResultant(
        polygon, reinforcement, concrete, steel,
        strainDistribution, stressRegions,
        analyticalIntegration, internalForces,
        epsTop, epsBottom, Nsd
    );
}

Point MomentSolver::testRegion2(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration, InternalForces &internalForces, double Nsd)
{
    double epsA = -concrete.getStrainConcreteRupture(); // fixo
    
    double epsBmin = 0;
    //double epsBmax = steel.getStrainSteelRupture();

    const auto &vectorReinf = reinforcement.getReinforcement();
    double yLowestRebar = 0;
    for (const auto &bar : vectorReinf) 
    {
        if (bar.getY() <= yLowestRebar)
            yLowestRebar = bar.getY();
    }

    double d = polygon.getMaxY() - yLowestRebar;
    double h = polygon.getPolygonHeight();
    double epsBmax = ((steel.getStrainSteelRupture() - epsA) * (h / d)) + epsA;
    
    // Lambda com captura de contexto
    auto func = [&](double epsB) 
    {
        return computeAxialForceResultant(
            polygon, reinforcement, concrete, steel,
            strainDistribution, stressRegions,
            analyticalIntegration, internalForces,
            epsA, epsB, Nsd
        );
    };

    // Verifica se há mudança de sinal (raiz isolada)
    if (func(epsBmin) * func(epsBmax) >= 0.0) 
    {
        converged = false;
        std::cerr << "Erro: função não muda de sinal no intervalo inicial (Region 2)" << std::endl;
        return Point(0,0);
    }

    iterations = 0;
    maxIterations = 100;
    tolerance = 1e-4;

    double rootEpsB = findRootBrent(func, epsBmin, epsBmax, tolerance, maxIterations);
    converged = true;

    strainResult.setStrain(epsA, rootEpsB);
    return computeMomentResultant(
        polygon, reinforcement, concrete, steel,
        strainDistribution, stressRegions,
        analyticalIntegration, internalForces,
        epsA, rootEpsB, Nsd
    );
}

Point MomentSolver::testRegion3(Polygon &polygon, Reinforcement &reinforcement, ConcreteProperties &concrete, SteelProperties &steel, StrainDistribution &strainDistribution, PolygonStressRegions &stressRegions, AnalyticalIntegration &analyticalIntegration, InternalForces &internalForces, double Nsd)
{
    const auto &vectorReinf = reinforcement.getReinforcement();
    double yLowestRebar = 0;
    for (const auto &bar : vectorReinf) 
    {
        if (bar.getY() <= yLowestRebar)
            yLowestRebar = bar.getY();
    }

    double d = polygon.getMaxY() - yLowestRebar;
    double h = polygon.getPolygonHeight();
    double epcu = -concrete.getStrainConcreteRupture();
    double epsu = steel.getStrainSteelRupture();

    auto func = [&](double epsTop) 
    {
        double epsBottom = ((epsu - epsTop) * (h / d)) + epsTop;
        return computeAxialForceResultant(
            polygon, reinforcement, concrete, steel,
            strainDistribution, stressRegions,
            analyticalIntegration, internalForces,
            epsTop, epsBottom, Nsd
        );
    };

    double epsTopMin = epcu;
    double epsTopMax = epsu;

    if (func(epsTopMin) * func(epsTopMax) >= 0.0) 
    {
        converged = false;
        std::cerr << "Erro: Brent - raiz não isolada no intervalo (Região 3)" << std::endl;
        return Point(0,0);
    }

    iterations = 0;
    maxIterations = 100;
    tolerance = 1e-4;

    double rootEpsTop = findRootBrent(func, epsTopMin, epsTopMax, tolerance, maxIterations);
    double epsBottom = ((epsu - rootEpsTop) * (h / d)) + rootEpsTop;

    strainResult.setStrain(rootEpsTop, epsBottom);
    converged = true;

    return computeMomentResultant(
        polygon, reinforcement, concrete, steel,
        strainDistribution, stressRegions,
        analyticalIntegration, internalForces,
        rootEpsTop, epsBottom, Nsd
    );
}

int MomentSolver::getIterations() const
{
    return iterations;
}

bool MomentSolver::isConverged() const
{
    return converged;
}

Point MomentSolver::getMoment() const
{
    return Mrd;
}

double MomentSolver::getTopFiberStrain() const
{
    return strainResult.getTopFiberStrain();
}

double MomentSolver::getBottomFiberStrain() const
{
    return strainResult.getBottomFiberStrain();
}
