#pragma once

#include <list>
#include <vector>

#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>

#include "Feature.h"
#include "App/GeoFeatureGroupExtension.h"
#include "App/GroupExtension.h"
#include "App/PropertyContainer.h"
#include "App/PropertyUnits.h"

namespace Sketcher
{

class SketcherExport Transformed: public Feature, public App::GeoFeatureGroupExtension
{
    typedef Feature inherited;
    PROPERTY_HEADER_WITH_OVERRIDE(Sketcher::Transformed);

public:
    Transformed();

    //    App::PropertyLinkList Originals;
    App::PropertyIntegerList SuppressedIndices;

    short mustExecute() const override;

    /**
     * Return the transformations that should be applied to Base.
     *
     * The returned transformations are applied to every geometry
     * element of Base.
     */
    virtual std::list<gp_Trsf> getTransformations() const
    {
        return std::list<gp_Trsf>();
        ;
    }

    /// Whether the first transformation is already represented by the original support.
    virtual bool hasOriginalTransformation() const
    {
        return true;
    }
    virtual bool isTransformationSuppressed(int index) const;
    virtual void setTransformationSuppressed(int index, bool suppressed);
    const std::list<gp_Trsf> getFilteredTransformations(
        const std::vector<App::DocumentObject*> originals
    );

protected:
    /**
     * Generate the transformed sketch geometry.
     */
    App::DocumentObjectExecReturn* executeModifier() override;

    /**
     * Transform one geometry element.
     *
     * Kept here so subclasses do not need to deal with the
     * Geometry property directly.
     */
    static Part::Geometry* transformGeometry(
        const Part::Geometry* geometry,
        const gp_Trsf& transformation
    );
};

}  // namespace Sketcher
