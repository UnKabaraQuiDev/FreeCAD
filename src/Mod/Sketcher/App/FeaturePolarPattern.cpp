#include "FeaturePolarPattern.h"

#include <gp_Ax1.hxx>

using namespace Sketcher;

PROPERTY_SOURCE(Sketcher::PolarPattern, Sketcher::Transformed)

PolarPattern::PolarPattern()
{
    ADD_PROPERTY_TYPE(
        Center,
        (Base::Vector3d(0.0, 0.0, 0.0)),
        "Polar Pattern",
        App::Prop_None,
        "Center of the pattern"
    );
    ADD_PROPERTY_TYPE(Angle, (360.0), "Polar Pattern", App::Prop_None, "Total angle of the pattern");
    ADD_PROPERTY_TYPE(Occurrences, (2), "Polar Pattern", App::Prop_None, "Number of occurrences");
}

short PolarPattern::mustExecute() const
{
    return inherited::mustExecute() || Center.isTouched() || Angle.isTouched()
        || Occurrences.isTouched();
}

const std::list<gp_Trsf> PolarPattern::getTransformations() const
{
    std::list<gp_Trsf> result;

    const int count = Occurrences.getValue();

    if (count <= 0) {
        return result;
    }

    if (count == 1) {
        result.emplace_back();
        return result;
    }

    const double step = Angle.getValue() / static_cast<double>(count - 1);

    const Base::Vector3d center = Center.getValue();

    const gp_Ax1 axis(gp_Pnt(center.x, center.y, 0.0), gp_Dir(0.0, 0.0, 1.0));

    for (int i = 0; i < count; ++i) {
        gp_Trsf transformation;

        transformation.SetRotation(axis, step * static_cast<double>(i));

        result.push_back(transformation);
    }

    return result;
}
