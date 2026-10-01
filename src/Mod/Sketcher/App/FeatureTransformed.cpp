#include "FeatureTransformed.h"

#include <Base/Exception.h>
#include <Base/Tools.h>
#include <Mod/Part/App/TopoShape.h>

using namespace Sketcher;

PROPERTY_SOURCE_ABSTRACT(Sketcher::Transformed, Sketcher::Feature)

Transformed::Transformed()
{}

short Transformed::mustExecute() const
{
    return inherited::mustExecute() || Originals.isTouched() || SuppressedIndices.isTouched();
}

App::DocumentObjectExecReturn* Transformed::executeModifier()
{
    auto* baseObject = Base.getValue();

    if (!baseObject) {
        return new App::DocumentObjectExecReturn("No base sketch specified.");
    }

    auto* baseSketch = freecad_cast<SketchObject*>(baseObject);

    if (!baseSketch) {
        return new App::DocumentObjectExecReturn("Base must be a SketchObject.");
    }

    const auto transformations = getTransformations();

    std::vector<Part::Geometry*> result;

    for (const auto* geometry : baseSketch->Geometry.getValues()) {
        if (!geometry) {
            continue;
        }

        for (const auto& transformation : transformations) {
            auto* transformed = transformGeometry(geometry, transformation);

            if (transformed) {
                result.push_back(transformed);
            }
        }
    }

    Geometry.setValues(std::move(result));

    return DocumentObject::StdReturn;
}

bool Transformed::isTransformationSuppressed(int index) const
{
    if (index < 0) {
        return false;
    }

    const auto& suppressed = SuppressedIndices.getValues();

    if (static_cast<std::size_t>(index) >= suppressed.size()) {
        return false;
    }

    return suppressed[index];
}

void Transformed::setTransformationSuppressed(int index, bool suppressed)
{
    if (index < 0) {
        return;
    }

    auto values = SuppressedIndices.getValues();

    const auto requiredSize = static_cast<std::size_t>(index + 1);

    if (values.size() < requiredSize) {
        values.resize(requiredSize, false);
    }

    values[index] = suppressed;

    SuppressedIndices.setValues(values);
}

const std::list<gp_Trsf> Transformed::getFilteredTransformations(
    const std::vector<App::DocumentObject*> originals
)
{
    const std::list<gp_Trsf> transformations = getTransformations();

    std::list<gp_Trsf> result;

    int index = 0;

    for (const auto& transformation : transformations) {
        if (!isTransformationSuppressed(index)) {
            result.push_back(transformation);
        }

        ++index;
    }

    return result;
}

Part::Geometry* Transformed::transformGeometry(
    const Part::Geometry* geometry,
    const gp_Trsf& transformation
)
{
    if (!geometry) {
        return nullptr;
    }

    auto* result = geometry->copy();

    Base::Matrix4D matrix;
    Part::TopoShape::convertToMatrix(transformation, matrix);

    result->transform(matrix);

    return result;
}
