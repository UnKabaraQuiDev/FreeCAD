#include "SketchObjectLinearPattern.h"

#include "opencascade/Precision.hxx"
#include "opencascade/gp_Trsf.hxx"

using namespace Sketcher;

PROPERTY_SOURCE(Sketcher::SketchObjectLinearPattern, Sketcher::SketchObjectTransformed)

SketchObjectLinearPattern::SketchObjectLinearPattern()
{
    ADD_PROPERTY_TYPE(
        Direction,
        (Base::Vector3d(1.0, 0.0, 0.0)),
        "Linear Pattern",
        App::Prop_None,
        "Direction of the pattern"
    );
    ADD_PROPERTY_TYPE(Length, (10.0), "Linear Pattern", App::Prop_None, "Total length of the pattern");
    ADD_PROPERTY_TYPE(Occurrences, (2), "Linear Pattern", App::Prop_None, "Number of occurrences");

    SketchObjectLinearPattern::getClassTypeId();
}

short SketchObjectLinearPattern::mustExecute() const
{
    return inherited::mustExecute() || Direction.isTouched() || Length.isTouched()
        || Occurrences.isTouched();
}

std::list<gp_Trsf> SketchObjectLinearPattern::getTransformations() const
{
    std::list<gp_Trsf> result;

    const int count = Occurrences.getValue();

    if (count <= 0) {
        return result;
    }

    Base::Vector3d direction = Direction.getValue();

    const double length = Length.getValue();

    if (direction.Length() <= Precision::Confusion()) {
        return result;
    }

    direction.Normalize();

    if (count == 1) {
        result.emplace_back();
        return result;
    }

    for (int i = 0; i < count; ++i) {
        gp_Trsf transformation;

        const double distance = length * static_cast<double>(i) / static_cast<double>(count - 1);

        transformation.SetTranslation(
            gp_Vec(direction.x * distance, direction.y * distance, direction.z * distance)
        );

        result.push_back(transformation);
    }

    return result;
}
