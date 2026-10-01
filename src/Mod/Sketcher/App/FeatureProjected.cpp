#include "FeatureProjected.h"

using namespace Sketcher;

PROPERTY_SOURCE(Sketcher::Projected, Sketcher::Feature)

Projected::Projected()
{
    ADD_PROPERTY_TYPE(
        ProjectPlane,
        (nullptr),
        "Mirrored",
        (App::PropertyType)(App::Prop_None),
        "Mirror plane"
    );
}

short Projected::mustExecute() const
{
    return inherited::mustExecute() || ProjectPlane.isTouched();
}

App::DocumentObjectExecReturn* Projected::executeModifier()
{
    return nullptr;
}
