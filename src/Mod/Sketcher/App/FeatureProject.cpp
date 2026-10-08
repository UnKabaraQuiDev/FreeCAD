#include "FeatureProject.h"

using namespace Sketcher;

PROPERTY_SOURCE(Sketcher::Project, Sketcher::Feature)

Project::Project()
{
    ADD_PROPERTY_TYPE(
        ProjectPlane,
        (nullptr),
        "Mirrored",
        (App::PropertyType)(App::Prop_None),
        "Mirror plane"
    );
}

short Project::mustExecute() const
{
    return inherited::mustExecute() || ProjectPlane.isTouched();
}

App::DocumentObjectExecReturn* Project::executeModifier()
{}
