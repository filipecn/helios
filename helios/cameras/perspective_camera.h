#ifndef HELIOS_CAMERAS_PERSPECTIVE_H
#define HELIOS_CAMERAS_PERSPECTIVE_H

#include <helios/core/camera.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                  PerspectiveCamera
// *********************************************************************************************************************
/// Camera based on the perspective projection transformation.
class PerspectiveCamera {
public:
  /// \param cam2world camera to world transform
  /// \param screenWindow screen space extent of the image
  /// \param sopen shutter open time
  /// \param sclose shutter close time
  /// \param lensr lens radius_
  /// \param focald focal distance
  /// \param fov field of view
  /// \param film
  PerspectiveCamera(const AnimatedTransform &cam2world,
                    const hermes::bbox2 &screen_window,
                    const hermes::size2 &film_resolution,
                    real_t sopen, real_t sclose,
                    real_t lensr, real_t focald, real_t fov);

  HERMES_DEVICE_CALLABLE real_t generateRay(const CameraSample &sample, Ray *ray) const;
  HERMES_DEVICE_CALLABLE real_t generateRayDifferential(const CameraSample &sample, RayDifferential *rd) const;

  static hermes::MemoryDumper::RegionLayout memoryDumpLayout();

private:
  Camera camera_;
  ProjectiveCamera projective_camera_;
  hermes::vec3 dx_camera_, dy_camera_;
  real_t A;
};

} // namespace helios

#endif // HELIOS_CAMERAS_PERSPECTIVE_H
