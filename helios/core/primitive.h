/*
 * Copyright (c) 2018 FilipeCN
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef HELIOS_CORE_PRIMITIVE_H
#define HELIOS_CORE_PRIMITIVE_H

#include <helios/core/interaction.h>
#include <helios/core/shape.h>
#include <helios/geometry/bounds.h>

namespace helios {

enum class PrimitiveType {
  GEOMETRIC_PRIMITIVE,
  CUSTOM
};

// Base data for primitives
struct Primitive {
  PrimitiveType type{PrimitiveType::GEOMETRIC_PRIMITIVE};
  void *primitive_data{nullptr};
};

/// A primitive is a simple scene element which can interact with light
/// and compute intersections with other elements. It is the bridge between the
/// geometry and shading.
class GeometricPrimitive {
public:
  /// \return world space bounds
  [[nodiscard]] HERMES_DEVICE_CALLABLE bounds3 worldBounds() const;
  /// Computes intersection of primitive with ray
  /// \param r ray
  /// \param si surface interaction object
  /// \return true if intersection exists
  HERMES_DEVICE_CALLABLE bool intersect(const Ray &r, SurfaceInteraction *si) const;
  /// Predicate to ray - primitive intersection
  /// \param r ray
  /// \return true if intersection exits
  [[nodiscard]] HERMES_DEVICE_CALLABLE bool intersectP(const Ray &r) const;
  /// Light emissive primitives contain an area light object
  /// \return area light object if primitive is light emissive, nullptr
  /// otherwise
  // TODO  const AreaLight *getAreaLight() const = 0;
  /// \return primitive's material (nullptr can be returned)
  // TODO  const Material *material() const = 0;
  /// Initializes representations of light scattering properties of the material
  /// at the intersection point on the surface
  /// \param isect intersection information
  /// \param arena memory manager for scattering functions allocation
  /// \param mode indicates if the ray is coming from a camera or a light
  /// source
  /// \param allowMultipleLobes sets how some BRDFs are represented
//  HERMES_DEVICE_CALLABLE void computeScatteringFunctions(SurfaceInteraction *isect,
//                                                         ponos::MemoryArena &arena /*, TransportMode mode*/,
//                                                         bool allowMultipleLobes) const;
private:
  Shape *shape_;
  // TODO std::shared_ptr<Material> material_;
  // TODO std::shared_ptr<AreaLight> areaLight_;
  // TODO MediumInterface mediumInterface;
};

} // namespace helios

#endif
