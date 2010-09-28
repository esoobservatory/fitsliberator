// The ESA/ESO/NASA FITS Liberator - http://code.google.com/p/fitsliberator
//
// Copyright (c) 2004-2010, ESA/ESO/NASA.
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the names of the European Space Agency (ESA), the European 
//       Southern Observatory (ESO) and the National Aeronautics and Space 
//       Administration (NASA) nor the names of its contributors may be used to
//       endorse or promote products derived from this software without specific
//       prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL ESA/ESO/NASA BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// =============================================================================
//
// The ESA/ESO/NASA FITS Liberator uses NASA's CFITSIO library, libtiff, 
// TinyXML, Boost C++ Libraries, Object Access Library and Intel Threading 
// Building Blocks.
//
// =============================================================================
//
// Project Executive:
//   Lars Lindberg Christensen
//
// Technical Project Manager:
//   Lars Holm Nielsen
//
// Developers:
//   Kaspar Kirstein Nielsen & Teis Johansen
// 
// Technical, scientific support and testing: 
//   Robert Hurt
//   Davide De Martin
//
// =============================================================================


#include "FitsEngine.h"
#include "FitsMath.h"

#ifdef USE_TBB
    #include <limits>

    #include <tbb/parallel_for.h>
    #include <tbb/blocked_range.h>
    #include <tbb/task_scheduler_init.h>

    using namespace std;
    using namespace tbb;
#endif

using FitsLiberator::Engine::FitsEngine;
using FitsLiberator::Engine::Stretch;
using FitsLiberator::Engine::ImageCube;

//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------

inline double signof(double value) {
    return (value < 0.0) ? -1.0 : 1.0;
}

//-----------------------------------------------------------------------------
// Implementation of regular stretch
//-----------------------------------------------------------------------------

#ifdef USE_TBB
    /** The following structs implement the various stretch functions. Stretch
        functions are un-ary functions (except for the linear stretch) and can
        be combined in any order to form effecient inlined function objects. */
    struct Linear {
        double scale, offset, background;
    public:
        Linear(double scale, double offset, double background) {
            this->scale = scale;
            this->offset = offset;
            this->background = background;
        }

        inline double operator()(double value) const {
            return scale * (value - offset) + background;
        }
    };

    template<typename Inner>
    struct Log {
        const Inner& inner;
    public:
        Log(const Inner& i) : inner(i) {}
        inline double operator()(double value) const {
            return log10(inner(value) + 1);
        }
    };

    template<typename Inner>
    struct Sqrt {
        const Inner& inner;
    public:
        Sqrt(const Inner& i) : inner(i) {}
        
        inline double operator()(double value) const {
            value = inner(value);
            return signof(value) * sqrt(::abs(value));
        }
    };

    template<typename Inner>
    struct Power {
        const Inner& inner;
        const double power;
    public:
        Power(double p, const Inner& i) : inner(i), power(p) {}

        inline double operator()(double value) const {
            value = inner(value);
            return signof(value) * pow(::abs(value), power);
        }
    };

    template<typename Inner>
    struct AsinH {
        const Inner& inner;
    public:
        AsinH(const Inner& i) : inner(i) {}

        inline double operator()(double value) const {
            value = inner(value);
            return log(value + sqrt(value * value + 1));
        }
    };

    /** The following two function objects performs the strecthing and is 
        called by the TBB runtime. One version supports a null map and the 
        other does not. */
    template<typename Type, typename Function, typename Size = size_t>
    struct MaskStretcher {
        double*               out;
        const Type*           in;
        const unsigned char*  valid;

        const Function&       stretch;

        static inline double invalid() {
            if(numeric_limits<double>::has_signaling_NaN) {
                return numeric_limits<double>::signaling_NaN();
            } else {
                return numeric_limits<double>::quiet_NaN();
            }
        }
    public:
        MaskStretcher(const Type* data, double* buffer, 
            const unsigned char* nullmap, const Function& f) 
          : in(data), out(buffer), valid(nullmap), stretch(f) {}

        void operator()(const tbb::blocked_range<Size>& range) const {
            double nan = invalid();
            for(Size i = range.begin(); i != range.end(); ++i) {
                out[i] = (valid[i] == 0) ? stretch(in[i]) : nan;
            }
        }
    };

    template<typename Type, typename Function, typename Size = size_t>
    struct Stretcher {
        double*         out;
        const Type*     in;

        const Function& stretch;
    public:
        Stretcher(const Type* data, double* buffer, const Function& f) 
          : in(data), out(buffer), stretch(f) {}

        void operator()(const tbb::blocked_range<Size>& range) const {
            for(Size i = range.begin(); i != range.end(); ++i) {
                out[i] = stretch(in[i]);
            }
        }
    };

    // TODO: Figure out how to merge to the following two functions.

    void FitsEngine::stretchRealValues(
        const Stretch& stretch, Double* rawPixels, Double* buffer, Int count) {

        tbb::task_scheduler_init init;

        const double* in  = rawPixels;
              double* out = buffer;

        switch(stretch.function) {
		    case stretchLinear:
                parallel_for(blocked_range<size_t>(0, count),
                    Stretcher<double, Linear>(in, out, 
                        Linear(stretch.scale, stretch.offset, stretch.scaleBackground)));
                break;
		    case stretchLog:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, Log<Linear> >(in, out,
                        Log<Linear>(
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchSqrt:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, Sqrt<Linear> >(in, out,
                        Sqrt<Linear>(
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchLogSqrt:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, Log<Sqrt<Linear> > >(in, out,
                        Log<Sqrt<Linear> >(
                            Sqrt<Linear>(
                                Linear(stretch.scale, stretch.offset, stretch.scaleBackground)))));
                break;
		    case stretchLogLog:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, Log<Log<Linear> > >(in, out,
                        Log<Log<Linear> >(
                            Log<Linear>(
                                Linear(stretch.scale, stretch.offset, stretch.scaleBackground)))));
                break;
            case stretchCubeR:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, Power<Linear> >(in, out,
                        Power<Linear>(1.0/3.0,
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchAsinh:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, AsinH<Linear> >(in, out,
                        AsinH<Linear>(
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchAsinhAsinh:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, AsinH<AsinH<Linear> > >(in, out,
                        AsinH<AsinH<Linear> >(
                            AsinH<Linear>(
                                Linear(stretch.scale, stretch.offset, stretch.scaleBackground)))));
                break;
		    case stretchAsinhSqrt:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, AsinH<Sqrt<Linear> > >(in, out,
                        AsinH<Sqrt<Linear> >(
                            Sqrt<Linear>(
                                Linear(stretch.scale, stretch.offset, stretch.scaleBackground)))));
                break;
		    case stretchRoot4:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, Power<Linear> >(in, out,
                        Power<Linear>(1.0/4.0,
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchRoot5:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    Stretcher<double, Power<Linear> >(in, out,
                        Power<Linear>(1.0/5.0,
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
        }
    }

    template<typename I>
    Void FitsEngine::_stretch(const Stretch& stretch, I* rawPixels, Byte* nullPixels, Double* buffer, 
                              Int count, Int /*nCpus*/ ) {

        tbb::task_scheduler_init init;//(nCpus);

        const I*             in   = rawPixels;
        double*              out  = buffer;
        const unsigned char* mask = nullPixels;

        switch(stretch.function) {
		    case stretchLinear:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, Linear>(in, out, mask, 
                        Linear(stretch.scale, stretch.offset, stretch.scaleBackground)));
                break;
		    case stretchLog:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, Log<Linear> >(in, out, mask,
                        Log<Linear>(
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchSqrt:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, Sqrt<Linear> >(in, out, mask,
                        Sqrt<Linear>(
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchLogSqrt:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, Log<Sqrt<Linear> > >(in, out, mask,
                        Log<Sqrt<Linear> >(
                            Sqrt<Linear>(
                                Linear(stretch.scale, stretch.offset, stretch.scaleBackground)))));
                break;
		    case stretchLogLog:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, Log<Log<Linear> > >(in, out, mask,
                        Log<Log<Linear> >(
                            Log<Linear>(
                                Linear(stretch.scale, stretch.offset, stretch.scaleBackground)))));
                break;
            case stretchCubeR:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, Power<Linear> >(in, out, mask,
                        Power<Linear>(1.0/3.0,
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchAsinh:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, AsinH<Linear> >(in, out, mask,
                        AsinH<Linear>(
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchAsinhAsinh:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, AsinH<AsinH<Linear> > >(in, out, mask,
                        AsinH<AsinH<Linear> >(
                            AsinH<Linear>(
                                Linear(stretch.scale, stretch.offset, stretch.scaleBackground)))));
                break;
		    case stretchAsinhSqrt:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, AsinH<Sqrt<Linear> > >(in, out, mask,
                        AsinH<Sqrt<Linear> >(
                            Sqrt<Linear>(
                                Linear(stretch.scale, stretch.offset, stretch.scaleBackground)))));
                break;
		    case stretchRoot4:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, Power<Linear> >(in, out, mask,
                        Power<Linear>(1.0/4.0,
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
		    case stretchRoot5:
                tbb::parallel_for(tbb::blocked_range<size_t>(0, count),
                    MaskStretcher<I, Power<Linear> >(in, out, mask,
                        Power<Linear>(1.0/5.0,
                            Linear(stretch.scale, stretch.offset, stretch.scaleBackground))));
			    break;
        }
    }
#else
    Void FitsEngine::stretchRealValues(const Stretch& stretch, Double* rawPixels, Double* out, Int count)
    {
	    //setting up the null map array
	    Byte* nullPixels = new Byte[count];
        std::fill(nullPixels, nullPixels + count, 0);
	    FitsEngine::_stretch(stretch, rawPixels, nullPixels, out, count, 1);
	    delete[] nullPixels;
    }

    template<typename I>
    Void FitsEngine::_stretch(const Stretch& stretch, I* rawPixels, Byte* nullPixels, Double* out, Int count, Int nCpus ) {
        static const double c = 1.0; ///< The loglog stretch constant "to be determined during development"

        #ifdef USE_OPENMP	
            #pragma omp parallel num_threads( nCpus )
	        {
        #endif // USE_OPENMP  
    	
	    register double value;
        switch(stretch.function) {
		    case stretchLinear:
			    #ifdef USE_OPENMP	
	    		    #pragma omp for
			    #endif // USE_OPENMP
                for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
                break;

		    case stretchLog:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for ( Int i = 0; i < count; i++ ) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = log10( value + 1);
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }			
			    break;
		    case stretchSqrt:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
                for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
                        value = FitsMath::signof( value ) * sqrt( abs(value) );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }    
                break;
		    case stretchLogSqrt:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = log10( FitsMath::signof( value ) * sqrt( abs( value ) ) + 1 );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }    
			    break;
		    case stretchLogLog:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
                for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
                        value = log10( c * log10( value + 1 ) + 1 );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }    
                break;
            case stretchCubeR:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
                for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = FitsMath::signof( value ) * pow( abs( value ), 1.0/3.0 );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
                break;

		    case stretchAsinh:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
                for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = log( value + sqrt( value * value + 1) );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
                break;
		    case stretchAsinhAsinh:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    //start by calculating asinh(x)
					    value = log( value + sqrt( value * value + 1) );
					    //then asinh( asinh ( x ) )
					    value = log( value + sqrt( value * value + 1) );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
		    case stretchAsinhSqrt:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    //start by calculating sqrt(x)
					    value = FitsMath::signof( value ) * sqrt( abs( value ) );
					    //then asinh( sqrt ( x ) )
					    value = log( value + sqrt( value * value + 1) );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
		    case stretchRoot4:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = FitsMath::signof( value ) * pow( abs( value ), 0.25 );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
		    case stretchRoot5:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = FitsMath::signof( value ) * pow( abs( value ), 0.20 );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
			case stretchPow15:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = pow( value, 1.5 );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
			case stretchPow2:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = value * value;
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
			case stretchPow3:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = pow( value, 3. );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
			case stretchPow4:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = pow( value, 4.0 );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
			case stretchPow5:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = pow( value, 5.0 );
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
			case stretchExp:
			    #ifdef USE_OPENMP	
			        #pragma omp for
			    #endif // USE_OPENMP
			    for(Int i = 0; i < count; i++) {
                    if( nullPixels[i] == 0 ) {
                        value = applyPreStretch( rawPixels[i], stretch.scale, stretch.offset, stretch.scaleBackground );
					    value = exp(value);
                        out[i] = value;
                    } else {
                        out[i] = FitsMath::NaN;
                    }
                }
			    break;
        }

        #ifdef USE_OPENMP
            }   // omp parallel
        #endif // USE_OPENMP
    }
#endif // USE_TBB

Void FitsEngine::stretch(const Stretch& stretch, ImageCube::PixelFormat bitDepth, Void* rawPixels, 
						 Byte* nullPixels, Double* out, UInt count, Int nCpus ) {
    // Select between the different datatypes, datatypes marked with (1) are not part of the 
    // FITS standard but are used by CFITSIO in case the BSCALE and BZERO keywords are used to
    // change an integer range from signed to unsigned.
    switch(bitDepth) {
        case ImageCube::Unsigned8:        // Unsigned 8-bit integer
            FitsEngine::_stretch(stretch, (Byte*)rawPixels, nullPixels, out, count, nCpus );
            break;
        case ImageCube::Signed16:        // Signed 16-bit integer
            FitsEngine::_stretch(stretch, (Short*)rawPixels, nullPixels, out, count, nCpus );
            break;
        case ImageCube::Signed32:        // Signed 32-bit integer
            FitsEngine::_stretch(stretch, (Int*)rawPixels, nullPixels, out, count, nCpus );
            break;
        case ImageCube::Signed64:    // Signed 64-bit integer
            FitsEngine::_stretch(stretch, (Int*)rawPixels, nullPixels, out, count, nCpus );
            break;
        case ImageCube::Float32:        // 32-bit float
            FitsEngine::_stretch(stretch, (Float*)rawPixels, nullPixels, out, count, nCpus );
            break;
        case ImageCube::Float64:    // 64-bit float
            FitsEngine::_stretch(stretch, (Double*)rawPixels, nullPixels, out, count, nCpus );
            break;
        case ImageCube::Signed8:        // Signed 8-bit (1)
            FitsEngine::_stretch(stretch, (Char*)rawPixels, nullPixels, out, count, nCpus );
            break;
        case ImageCube::Unsigned16:    // Unsigned 16-bit integer (1)
            FitsEngine::_stretch(stretch, (UShort*)rawPixels, nullPixels, out, count, nCpus );
            break;
        case ImageCube::Unsigned32:        // Unsigned 32-bit integer (1)
            FitsEngine::_stretch(stretch, (UInt*)rawPixels, nullPixels, out, count, nCpus );
            break;
        default:
            throw Exception("Invalid bitdepth");
    }
}

//-----------------------------------------------------------------------------
// FitEngine misc. functions
//-----------------------------------------------------------------------------

Void FitsEngine::scale(const Stretch& stretch, Double* pixels, UInt count ) {
    //
    // The Preview and FitsLoader needs values that will fit inside an 8-bit
    // or 16-bit integer so we need to scale the pixels to fit; the following
    // two parameters are used to do a linear mapping.
    // Note however that white level and black level essentially filter out
    // unwanted values, but this function doesn't apply any filtering - that
    // is up to the consumer of the pixels to do. In the case of the Preview
    // markings are added to indicate the filtered values (green and blue)
    // whereas the FitsLoader class just saturates the filtered pixels
    Double scale = stretch.outputMax / (stretch.whiteLevel - stretch.blackLevel);
    Double offset = -stretch.blackLevel * scale;

    Double* end = pixels + count;
    while( pixels != end )
        *pixels++ = scale * (*pixels) + offset;
}

#ifdef USE_TBB
	template<typename Size = size_t>
	struct Scaler {
		const double	scale;
		const double	offset;

		double*			pixels;
	public:
		Scaler(double* data, double s, double o)
			: pixels(data), scale(s), offset(o) {}

		void operator()(const tbb::blocked_range<Size>& range) const {
			for(Size i = range.begin(); i != range.end(); ++i) {
				pixels[i] = scale * pixels[i] + offset;
			}
		}
	};

	Void FitsEngine::scale_par(const Stretch& stretch, Double* pixels, UInt count, UInt) {
        double scale = stretch.outputMax / (stretch.whiteLevel - stretch.blackLevel);
        double offset = -stretch.blackLevel * scale;
        
        tbb::task_scheduler_init init;
		tbb::parallel_for(tbb::blocked_range<size_t>(0, count), 
			Scaler<size_t>(pixels, scale, offset));
	}
#else
	Void FitsEngine::scale_par(const Stretch& stretch, Double* pixels, UInt count, UInt nCpus ) {
		//
		// The Preview and FitsLoader needs values that will fit inside an 8-bit
		// or 16-bit integer so we need to scale the pixels to fit; the following
		// two parameters are used to do a linear mapping.
		// Note however that white level and black level essentially filter out
		// unwanted values, but this function doesn't apply any filtering - that
		// is up to the consumer of the pixels to do. In the case of the Preview
		// markings are added to indicate the filtered values (green and blue)
		// whereas the FitsLoader class just saturates the filtered pixels
		Double scale = stretch.outputMax / (stretch.whiteLevel - stretch.blackLevel);
		Double offset = -stretch.blackLevel * scale;

		#ifdef USE_OPENMP	
		#pragma omp parallel num_threads( nCpus )
		{		
			#pragma omp for		
		#endif // USE_OPENMP  
			for ( Int i = 0; i < count; i++ )
			{
				pixels[i] = scale * (pixels[i]) + offset;
			}
		#ifdef USE_OPENMP
		}//end of omp parallel
		#endif
	}
#endif // USE_TBB

Double FitsEngine::getLinearVal(const Stretch& stretch, Double val) {
	return FitsEngine::linearValStretch( stretch, val, true );
}

Double FitsEngine::getLinearValWithoutStretch(const Stretch& stretch, Double val) {
	return FitsEngine::linearValStretch( stretch, val, false );
}

Double FitsEngine::linearValStretch( const Stretch& stretch, Double val, Bool doStretch )
{
	Double scale = stretch.scale;
	Double offset = stretch.offset;
	Double scaleBackground =  stretch.scaleBackground;
    switch(stretch.function) {
		case stretchLinear:
			if ( doStretch )
			{
				return unapplyPreStretch( val, scale, offset, scaleBackground );
			}
			else
			{
				return val;
			}
		case stretchLog:
			val = FitsMath::power( 10.0, val) - 1;
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset, scaleBackground );
			}
			else
			{
				return val;
			}
		case stretchSqrt:
			val = FitsMath::signof( val ) * FitsMath::power( val, 2.0 );
			//val = FitsMath::power( val, 2.0 );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset, scaleBackground );
			}
			else
			{
				return val;
			}
		case stretchLogSqrt:
			val = FitsMath::power( 10.0, val ) - 1;
			val = FitsMath::signof( val ) * FitsMath::power( ::abs( val ), 2.0 );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset, scaleBackground );
			}
			else
			{
				return val;
			}
		case stretchLogLog:
			val = FitsMath::power( 10.0, val) - 1;
			val = FitsMath::power( 10.0, val) - 1;
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset, scaleBackground );
			}
			else
			{
				return val;
			}
		case stretchCubeR:
			val = FitsMath::signof( val ) * FitsMath::power( ::abs( val ), 3.0 );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
		case stretchAsinh:
			val = sinh( val );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
		case stretchAsinhAsinh:
			val = sinh( val );
			val = sinh( val );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
		case stretchAsinhSqrt:
			val = sinh( val );
			val = FitsMath::signof( val ) * pow( ::abs( val ), 2.0 );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
		case stretchRoot4:
			val = FitsMath::signof( val ) * pow( ::abs( val ), 4.0 );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
		case stretchRoot5:
			val = FitsMath::signof( val ) * pow( ::abs( val ), 5.0 );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
		
		case stretchPow15:
			val = pow( val, 1./1.5 );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
		case stretchPow2:
			val = pow( val, 1./2. );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
		case stretchPow3:
			val = pow( val, 1./3. );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
		case stretchPow4:
			val = pow( val, 1./4. );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
		case stretchPow5:
			val = pow( val, 1./5. );
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
		case stretchExp:
			val = exp(-val);
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
			break;
        default:
			if (doStretch)
			{
				return unapplyPreStretch( val, scale, offset,scaleBackground );
			}
			else
			{
				return val;
			}
	}
}

