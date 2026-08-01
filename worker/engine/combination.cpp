#include "combination.h"

Combination::Combination() : Normal(0.0), MsdX(0.0), MsdY(0.0), MsolverXX(0.0), MsolverYY(0.0), isMomentValid(true), isCalculated(false), isNormalForceValid(false), isSafe(false)
{
}

Combination::Combination(double normal, double msdX, double msdY, double msolverxx, double msolveryy, bool isValid, bool isCalculated)
    : Normal(normal), MsdX(msdX), MsdY(msdY), MsolverXX(msolverxx), MsolverYY(msolveryy), isMomentValid(isValid), isCalculated(isCalculated), isNormalForceValid(false), isSafe(false)
{
}

vector<Point> Combination::getMsd() const
{
    vector<Point> result{};
    Point msd = Point(MsdX, MsdY);
    result.push_back(msd);
    
    return result;
}