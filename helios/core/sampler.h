/// Copyright (c) 2021, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///\file sampler.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-11
///
///\brief

#ifndef HELIOS_CORE_SAMPLER_H
#define HELIOS_CORE_SAMPLER_H

#include <hermes/random/rng.h>
#include <hermes/geometry/point.h>
#include <vector>

namespace helios {



// *********************************************************************************************************************
//                                                                                                         Integrator
// *********************************************************************************************************************
/// Sampler base class
class Sampler {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  virtual ~Sampler() = default;
  /// \brief Construct a new Sampler object
  /// \param samplesPerPixel
  Sampler(int64_t samplesPerPixel);
  /// Called when rendering work is ready to start for a given pixel
  /// \param p pixel coordinates in the image
  virtual void startPixel(const hermes::index2 &p);
  /// \return real_t the sample value for the next dimension of the current
  /// sample vector
  virtual real_t get1D() = 0;
  /// \return hermes::point2 the sample values for the next 2 dimensions of the
  /// current sample vector
  virtual hermes::point2 get2D() = 0;
  /// Some samplers do a better job of generating some particular sizes of
  /// arrays \param n  number of samples
  /// \return the preferred size of array for
  /// the sampler
  virtual int roundCount(int n) const;
  /// Notifies the sampler that subsequent requests for sample components should
  /// return values starting at the first dimension of the next sample for the
  /// current pixel
  /// \return true until all samples for the current pixel have been generated
  virtual bool startNextSample();
  /// \param seed a seed to the clone's rng (if any)
  /// \return std::unique_ptr<Sampler> a new instance of an initial sampler
  virtual std::unique_ptr<Sampler> clone(int seed) = 0;
  /// Sets the index of the sample to be generated
  /// \param sampleNum the index of the sample in the current pixel
  /// \return false when the sample number is greater than or equal to the
  /// number of originally requested samples per pixel
  virtual bool setSampleNumber(int64_t sampleNum);
  /// \brief Get the Camera Sample object
  /// Initializes a CameraSample for a given pixel
  /// \param pRaster pixel coordinates
  /// \return CameraSample
  CameraSample getCameraSample(const hermes::index2 &pRaster);
  /// Pre-generates an array of n 1D samples
  /// \param n number of elements
  void request1DArray(int n);
  /// Pre-generates an array of n 2D samples
  /// \param n number of elements
  void request2DArray(int n);
  /// \param n number of samples
  /// \return pointer to the start of a previously requested array
  const real_t *get1DArray(int n);
  /// \param n number of samples
  /// \return pointer to the start of a previously requested array
  const hermes::point2 *get2DArray(int n);

  const int64_t samplesPerPixel; //!< number of samples per pixel

protected:
  hermes::index2
      currentPixel; //!< pixel to which samples are currently being generated
  int64_t currentPixelSampleIndex;      //!< current sample being generated
  std::vector<int> samples1DArraySizes; //!< sizes of requested 1D arrays
  std::vector<int> samples2DArraySizes; //!< sizes of requested 2D arrays
  std::vector<std::vector<real_t>> sampleArray1D; //!< requested 1D arrays
  std::vector<std::vector<hermes::point2>>
      sampleArray2D; //!< requested 2D arrays

private:
  size_t array1DOffset; //!< the index of the next 1D array to return for the
  //!< sample vector
  size_t array2DOffset; //!< the index of the next 2D array to return for the
  //!< sample vector
};

/// Implements some functionality to generate all of the dimension's sample
/// values for all of the sample vectors for a pixel at the same time
class PixelSampler : public Sampler {
public:
  /// \param samplesPerPixel number of samples per pixel
  /// \param nSampledDimensions maximum number of sample dimensions (if more is
  /// requested than uniform random values are generated for additional
  /// dimensions)
  PixelSampler(int64_t samplesPerPixel, int nSampledDimensions);
  ~PixelSampler() = default;
  bool startNextSample() override;
  bool setSampleNumber(int64_t sampleNum) override;
  real_t get1D() override;
  hermes::point2 get2D() override;

protected:
  std::vector<std::vector<real_t>>
      samples1D; //!< precomputed 1D dimensions samples
  std::vector<std::vector<hermes::point2>>
      samples2D;              //!< precomputed 2D dimensions samples
  int current1DDimension = 0; //!< the offset for the current pixel 1D sample
  int current2DDimension = 0; //!< the offset for the current pixel 2D sample
  hermes::PCGRNG rng;
};

/// Base for not pixel-based samplers that generate consecutive samples that are
/// spread across the entire image
class GlobalSampler : public Sampler {
public:
  /// \param samplesPerPixel samples per pixel
  GlobalSampler(int64_t samplesPerPixel);
  /// Performs the inverse mapping from the current pixel and sample index to a
  /// global index into the overall set of sample vectors
  /// \param sampleNum index of sample
  /// \return int64_t global index of samples
  virtual int64_t getIndexForSample(int64_t sampleNum) const = 0;
  /// \param index sample vector index
  /// \param dimension dimension index
  /// \return real_t the sample value for the given dimension of the index-th
  /// sample vector in the sequence
  virtual real_t sampleDimension(int64_t index, int dimension) const = 0;
  void startPixel(const hermes::index2 &p) override;
  bool startNextSample() override;
  bool setSampleNumber(int64_t sampleNum) override;
  real_t get1D() override;
  hermes::point2 get2D() override;

private:
  int dimension; //!< next dimension that the sampler will be asked to generate
  int64_t intervalSampleIndex; //!< the index of the sample that corresponds to
  //!< the current sample
  static const int arrayStartDim =
      5; //!< dimensions up to arrayStartDim are
  //!< devoted to regular 1D and 2D samples (used to CameraSample)
  int arrayEndDim; //!< higher dimensions are devoted to non-array 1D and 2D
  //!< samples
};

/* Sampler interface.
 * Generates a set of multidimensional sample positions per pixel.
 * The samples are generated from PixelStart to PixelEnd - 1, inclusive, and the
 * time values will be in range of shutterOpen and shutterClose.
 */
// class Sampler {
// public:
//   /* Constructor.
//    * @xstart first pixel coordinate on X axis
//    * @xend (last + 1) pixel coordinate on X axis
//    * @ystart first pixel coordinate on Y axis
//    * @yend (last + 1) pixel coordinate on Y axis
//    * @spp number of samples per pixel
//    * @sopen camera's shutter open time
//    * @sclose camera's shutter close time
//    */
//   Sampler(int xstart, int xend, int ystart, int yend, int spp, float sopen,
//           float sclose);
//   /* generate more samples.
//    * @samples **[out]** this function will fill samples with the new samples
//    * @rng **[in]** the pseudo-random number generator
//    *
//    * @return the number of generated samples.
//    */
//   virtual int getMoreSamples(Sample *samples, hermes::RNG &rng) = 0;
//   /* max number of samples it can generates in one call.
//    * help the caller of <getMoreSamples> to allocate memory for the sample.
//    */
//   virtual int maximumSampleCount() = 0;
//   /* report
//    * @samples collection of samples (originally from getModeSamples())
//    * @rays [out] which rays were generated
//    * @Ls [out] which radiance values were computed
//    * @isects [out] which intersections were found
//    * @count number of samples
//    *
//    * This method is usefull for adaptive sampling algorithms.
//    *
//    * @return true if the sample values should be added to the image being
//    * generated
//    */
//   bool reportResults(Sample *samples, const RayDifferential *rays,
//                      const Spectrum *Ls, const Intersection *isects, int
//                      count);
//   /* creates a new Sampler.
//    * @num ranges from 0 to count-1
//    * @count total number of subsamplers being used
//    *
//    * Creates a new Sampler responsible for a subset of the image.
//    */
//   virtual Sampler *getSubSampler(int num, int count) = 0;
//   /* Generate samples for a tile.
//    * @num tile number
//    * @count total number of tiles
//    * @newXStart first pixel coordinate on X axis
//    * @newXEnd (last + 1) pixel coordinate on X axis
//    * @newYStart first pixel coordinate on Y axis
//    * @newYEnd (last + 1) pixel coordinate on Y axis
//    *
//    * A lot of implementations decompose the image into rectangular tiles and
//    * have each subsampler generate samples for each tile. computeSubWindow()
//    * computes a pixel sampling range for a specific tile.
//    */
//   void computeSubWindow(int num, int count, int *newXStart, int *newXEnd,
//                         int *newYStart, int *newYEnd) const;
//   /* Computes the number of samples after rounding to size. // TODO
//    * @size size to be rounded to
//    *
//    * @return the number of samples that should be requested for this size
//    */
//   virtual int roundSize(int size) const = 0;

//   const int xPixelStart, xPixelEnd, yPixelStart, yPixelEnd;
//   const int samplesPerPixel;
//   const float shutterOpen, shutterClose;
// };

// /* Sample implementation.
//  *
//  * The sample get from the integrators the requests for samples. The
//  integrators
//  * can ask for multiple 1D and 2D sampling patterns.
//  */
// struct Sample : public CameraSample {
//   /* Constructor.
//    * @sampler
//    * @surf Surface integrator
//    * @vol Volume integrator
//    * @scene
//    *
//    * Calls <Integrator::requestSamples> methods of the surface and volume
//    * integrators to find out what samples they will need.
//    */
//   Sample(Sampler *sampler, SurfaceIntegrator *surf, VolumeIntegrator *vol,
//          const Scene *scene);
//   /* Request 1D sample pattern.
//    * @num number of samples
//    *
//    * @return array index that can be used to access this sample value later
//    */
//   uint32 add1D(uint32 num) {
//     n1D.emplace_back(num);
//     return n1D.size() - 1;
//   }
//   /* Request 2D sample pattern.
//    * @num number of samples
//    *
//    * @return array index that can be used to access this sample value later
//    */
//   uint32 add2D(uint32 num) {
//     n2D.emplace_back(num);
//     return n2D.size() - 1;
//   }
//   /* allocates sample memory.
//    */
//   void allocateSampleMemory();
//   /* generate clone instances
//    * @count number of copies
//    *
//    * @return array of copies
//    */
//   Sample *duplicate(int count) const;

//   ~Sample() { // TODO
//   }

//   std::vector<uint32> n1D, n2D;

//   // the integrator can retrieve the sample value by oneD[<sampleOffset>][i]
//   float **oneD, **twoD;
// };

// /* generate
//  * @samp **[out]** samples data
//  * @nSamples **[in]** number of samples
//  * @rng **[out]** the pseudo-random number generator
//  * @jitter **[in]** if true, the samples are jittered
//  *
//  * Generates a stratified 1D samples pattern
//  */
// void generateStratifiedSample1D(float *samp, int nSamples, hermes::RNG &rng,
//                                 bool jitter);
// /* generate
//  * @samp **[out]** samples data
//  * @nx **[in]** number of samples in X axis
//  * @ny **[in]** number of samples in Y axis
//  * @rng **[out]** the pseudo-random number generator
//  * @jitter **[in]** if true, the samples are jittered
//  *
//  * Generates a stratified 2D samples pattern
//  */
// void generateStratifiedSample2D(float *samp, int nx, int ny, hermes::RNG &rng,
//                                 bool jitter);
// /* generate
//  * @samples **[out]** samples data
//  * @nSamples **[in]** number of samples
//  * @nDim **[in]** number of dimensions
//  * @rng **[out]** the pseudo-random number generator
//  *
//  * Generates **samples** in an arbitrary dimension **nDim** using LHS (Latin
//  * Hypercube Sampling). LHS divides each dimension's axis into _n_ regions
//  and
//  * generates a jittered sample in each region along the diagonal, then
//  shuffles
//  * the samples in each dimension.
//  */
// void generateLatinHypercube(float *samples, uint32 nSamples, uint32 nDim,
// hermes::RNG &rng);
} // namespace helios

#endif // HELIOS_CORE_SAMPLER_H
