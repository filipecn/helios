#include <helios/samplers/stratified_sampler.h>

using namespace hermes;

namespace helios {

//void StratifiedSampler::latinHypercube(real_t *samples, int nSamples, int nDim, PCGRNG &rng) {
//   generate LHS samples along diagonal
//  real_t invNSamples = static_cast<real_t>(1) / nSamples;
//  for (int i = 0; i < nSamples; ++i)
//    for (int j = 0; j < nDim; ++j) {
//      real_t sj = (i + (rng.uniformFloat())) * invNSamples;
//    }
//   permute LHS samples in each dimension
//  for (int i = 0; i < nDim; ++i)
//    for (int j = 0; j < nSamples; ++j) {
//      int other = j + rng.uniformUInt32(nSamples - j);
//      std::swap(samples[nDim * j + i], samples[nDim * other + i]);
//    }
//}

StratifiedSampler::StratifiedSampler(const size2 &grid_resolution,
                                     bool jitter_samples,
                                     u32 sampled_dimensions)
    : resolution_(grid_resolution), jitter_samples_(jitter_samples) {
  SamplePool::PoolDescriptor descriptor = {
      .samples_per_pixel = resolution_.total(),
      .dimensions = sampled_dimensions,
      .array1_sizes = {},
      .array2_sizes = {}
  };
  pool_.set(descriptor);
}

HERMES_DEVICE_CALLABLE StratifiedSampler::StratifiedSampler(const StratifiedSampler &other) {
  resolution_ = other.resolution_;
  jitter_samples_ = other.jitter_samples_;
  pool_ = other.pool_;
  rng = other.rng;
}

HERMES_DEVICE_CALLABLE StratifiedSampler::~StratifiedSampler() {}

HERMES_DEVICE_CALLABLE void StratifiedSampler::startPixel(const index2 &p) {
  const u32 sampled_dimensions = pool_.dimensionCount();
  const real_t inv_n_samples = static_cast<real_t>(1) / resolution_.total();
  const real_t dx = static_cast<real_t>(1) / resolution_.width;
  const real_t dy = static_cast<real_t>(1) / resolution_.height;

  for (auto ij : range2(resolution_)) {
    u32 pixel_sample = ij.j * resolution_.width + ij.i;
    // generate samples for dimensions
    for (u32 d = 0; d < sampled_dimensions; ++d) {
      // generate 1d sample
      real_t delta = jitter_samples_ ? rng.uniformFloat() : 0.5f;
      pool_.get1DSample(pixel_sample, d) = fminf((pixel_sample + delta) * inv_n_samples, Constants::one_minus_epsilon);
      // generate 2d sample
      real_t jx = jitter_samples_ ? rng.uniformFloat() : 0.5f;
      real_t jy = jitter_samples_ ? rng.uniformFloat() : 0.5f;
      point2 &sample_p = pool_.get2DSample(pixel_sample, d);
      sample_p.x = fminf((ij.i + jx) * dx, Constants::one_minus_epsilon);
      sample_p.y = fminf((ij.j + jy) * dy, Constants::one_minus_epsilon);
    }
    // generate samples for 1d arrays
    for (u32 a = 0; a < pool_.array1Count(); ++a) {
      real_t *arr = pool_.get1DArray(pixel_sample, a);
      for (u32 j = 0; j < pool_.get1DArraySize(a); ++j) {
        real_t delta = jitter_samples_ ? rng.uniformFloat() : 0.5f;
        arr[j] = fminf((pixel_sample + delta) * inv_n_samples, Constants::one_minus_epsilon);
      }
    }
    for (u32 a = 0; a < pool_.array2Count(); ++a) {
      point2 *arr = pool_.get2DArray(pixel_sample, a);
      for (u32 j = 0; j < pool_.get2DArraySize(a); ++j) {
        real_t jx = jitter_samples_ ? rng.uniformFloat() : 0.5f;
        real_t jy = jitter_samples_ ? rng.uniformFloat() : 0.5f;
        arr[j].x = fminf((ij.i + jx) * dx, Constants::one_minus_epsilon);
        arr[j].y = fminf((ij.j + jy) * dy, Constants::one_minus_epsilon);
      }
    }
  }

  // shuffle samples
  pool_.reset();
}

HERMES_DEVICE_CALLABLE void StratifiedSampler::setDataPtr(byte *data) {
  pool_.setData(data);
}

HERMES_DEVICE_CALLABLE SamplePool::size_type StratifiedSampler::memorySize() const {
  return pool_.itemSize();
}

HERMES_DEVICE_CALLABLE SamplePool StratifiedSampler::samplePool() const {
  return pool_;
}

HERMES_DEVICE_CALLABLE CameraSample StratifiedSampler::cameraSample(const index2 &p) {
  CameraSample cs;
  cs.film = pool_.get2DSample() + vec2(p.i, p.j);
  cs.time = pool_.get1DSample();
  cs.lens = pool_.get2DSample();
  return cs;
}

HERMES_DEVICE_CALLABLE bool StratifiedSampler::startNextSample() {
  return pool_.startNextSample();
}

HERMES_DEVICE_CALLABLE void StratifiedSampler::setIndex(u32 i) {
  pool_.setPoolIndex(i);
}

HERMES_DEVICE_CALLABLE u32 StratifiedSampler::samplesPerPixel() const {
  return resolution_.total();
}

// StratifiedSampler::StratifiedSampler(int xstart, int xend, int ystart, int
// yend,
//                                      int xs, int ys, bool jitter, float
//                                      sopen, float sclose)
//     : Sampler(xstart, xend, ystart, yend, xs * ys, sopen, sclose) {
//   jitterSamples = jitter;
//   xPos = xPixelStart;
//   yPos = yPixelStart;
//   xPixelSamples = xs;
//   yPixelSamples = ys;
//   buffer = new float[5 * xPixelSamples * yPixelSamples];
// }

// int StratifiedSampler::getMoreSamples(Sample *samples, ponos::RNG &rng) {
//   if (yPos == yPixelEnd)
//     return 0;
//   int nSamples = xPixelSamples * yPixelSamples;
//   // Generate stratified camera samples for (xPos, yPos)
//   // // generate initial stratified samples into buffer memory
//   float *bufp = buffer;
//   float *imageSamples = bufp;
//   bufp += 2 * nSamples;
//   float *lensSamples = bufp;
//   bufp += 2 * nSamples;
//   float *timeSamples = bufp;
//   generateStratifiedSample2D(imageSamples, xPixelSamples, yPixelSamples, rng,
//                              jitterSamples);
//   generateStratifiedSample2D(imageSamples, xPixelSamples, yPixelSamples, rng,
//                              jitterSamples);
//   generateStratifiedSample1D(imageSamples, xPixelSamples * yPixelSamples,
//   rng,
//                              jitterSamples);
//   // // shift stratified image samples to pixel coordinates
//   for (int o = 0; o < 2 * xPixelSamples * yPixelSamples; o += 2) {
//     imageSamples[o] += xPos;
//     imageSamples[o + 1] += yPos;
//   }
//   // // decorrelate sample dimensions
//   // TODO pg 354 shuffle(lensSamples, xPixelSamples * yPixelSamples, 2, rng);
//   // TODO shuffle(timeSamples, xPixelSamples * yPixelSamples, 1, rng);
//   // // initialize stratified samples with sample values
//   for (int i = 0; i < nSamples; i++) {
//     samples[i].imageX = imageSamples[2 * i];
//     samples[i].imageY = imageSamples[2 * i + 1];
//     samples[i].lensU = lensSamples[2 * i];
//     samples[i].lensV = lensSamples[2 * i + 1];
//     samples[i].time = ponos::lerp(timeSamples[i], shutterOpen, shutterClose);
//     // generate stratified samples for integrators
//     for (uint32 j = 0; j < samples[i].n1D.size(); i++)
//       generateLatinHypercube(samples[i].oneD[j], samples[i].n1D[j], 1, rng);
//     for (uint32 j = 0; j < samples[i].n2D.size(); i++)
//       generateLatinHypercube(samples[i].twoD[j], samples[i].n2D[j], 2, rng);
//   }
//   // Advance to next pixel for stratified sampling
//   if (++xPos == xPixelSamples) {
//     xPos = xPixelStart;
//     ++yPos;
//   }
//   return nSamples;
// }

// Sampler *StratifiedSampler::getSubSampler(int num, int count) {
//   int x0, x1, y0, y1;
//   computeSubWindow(num, count, &x0, &x1, &y0, &y1);
//   if (x0 == x1 || y0 == y1)
//     return nullptr;
//   return new StratifiedSampler(x0, x1, y0, y1, xPixelSamples, yPixelSamples,
//                                jitterSamples, shutterOpen, shutterClose);
// }

} // namespace helios
