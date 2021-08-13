#ifndef HELIOS_SAMPLERS_STRATIFIED_SAMPLER_H
#define HELIOS_SAMPLERS_STRATIFIED_SAMPLER_H

#include <helios/samplers/sample_pool.h>
#include <hermes/random/rng.h>
#include <helios/core/camera.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                  StratifiedSampler
// *********************************************************************************************************************
/// Divides the sampled region into rectangular regions and generates a single sample inside each region.
class StratifiedSampler {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  /// Loop over the given number of strata in the domain and place a sample
  /// point in each one
  /// \param samp **[out]** samples
  /// \param nSamples number of samples
  /// \param rng random number generator
  /// \param jitter true to jitter points from strata centers
  static void sample1D(real_t *samp, int nSamples, hermes::PCGRNG &rng, bool jitter);
  /// Loop over the given number of strata in the domain and place a sample
  /// point in each one
  /// \param samp **[out]** samples
  /// \param nx number of samples along x axis
  /// \param ny number of samples along y axis
  /// \param rng random number generator \param jitter true to jitter points
  /// from strata centers
  static void sample2D(hermes::point2 *samp, int nx, int ny, hermes::PCGRNG &rng, bool jitter);
  /// Generates an arbitrary number of Latin Hypercube samples in an arbitrary
  /// dimension
  /// \param samples **[out]** samples array (size must be at least nSamples *
  /// nDim)
  /// \param nSamples number of samples
  /// \param nDim samples dimension
  /// \param rng random number generator
  static void latinHypercube(real_t *samples, int nSamples, int nDim, hermes::PCGRNG &rng);
  /// Shuffles an array of n-dimensional samples
  /// \tparam T data type
  /// \param samp **[out]** samples array
  /// \param count number of samples
  /// \param nDimensions number of dimensions per sample
  /// \param rng random number generator
//  template<typename T>
//  static void shuffle(T *samp, int count, int nDimensions, hermes::PCGRNG &rng) {
//    for (int i = 0; i < count; ++i) {
//      int other = i + rng.uniformUint32(count - i);
//      for (int j = 0; j < nDimensions; ++j)
//        std::swap(samp[nDimensions] * i + j], samp[nDimensions * other + j]);
//    }
//  }
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  StratifiedSampler() = default;
  ///
  /// \param grid_resolution
  /// \param jitter_samples
  /// \param sampled_dimensions
  StratifiedSampler(const hermes::size2 &grid_resolution, bool jitter_samples,
                    u32 sampled_dimensions);

  HERMES_DEVICE_CALLABLE StratifiedSampler(const StratifiedSampler& other);
  HERMES_DEVICE_CALLABLE ~StratifiedSampler();
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE void setIndex(u32 i);
  HERMES_DEVICE_CALLABLE void setDataPtr(byte *data);
  HERMES_DEVICE_CALLABLE void startPixel(const hermes::index2 &p);
  [[nodiscard]] HERMES_DEVICE_CALLABLE CameraSample cameraSample(const hermes::index2 &p);
  [[nodiscard]] HERMES_DEVICE_CALLABLE SamplePool samplePool() const;
  [[nodiscard]] HERMES_DEVICE_CALLABLE SamplePool::size_type memorySize() const;
  HERMES_DEVICE_CALLABLE bool startNextSample();
  HERMES_DEVICE_CALLABLE u32 samplesPerPixel() const;

private:
  hermes::size2 resolution_;
  bool jitter_samples_{false};
  SamplePool pool_;
  hermes::PCGRNG rng;
};

} // namespace helios

#endif // HELIOS_SAMPLERS_STRATIFIED_SAMPLER_H
