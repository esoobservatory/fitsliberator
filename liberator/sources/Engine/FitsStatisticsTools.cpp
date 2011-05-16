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

//-----------------------------------------------------------------------------
// TODO:
//      Since the FitsStatisticsTools class only contains static functions, it
//      makes more sense to ditch the class all-together and just have the 
//      functions. These could then be split into several files to make it 
//      easier to browse the code.
//-----------------------------------------------------------------------------

#include "FitsStatisticsTools.h"
#include "Environment.h"
#include "Stretch.h"
#include "FitsMath.h"

#ifdef USE_TBB
    #include <limits>

    #include <tbb/parallel_reduce.h>
    #include <tbb/blocked_range.h>
    #include <tbb/task_scheduler_init.h>

    using namespace std;
    using namespace tbb;

    #undef min
    #undef max
#elif defined(USE_OPENMP)
    #include <omp.h>
#endif  // USE_TBB

using namespace FitsLiberator::Engine;

//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------

inline bool valid(double value) {
    return value != FitsMath::NaN && FitsMath::isFinite(value);
}

//-----------------------------------------------------------------------------
// Implementations of FitsStatisticsTools::getRange
//-----------------------------------------------------------------------------

Void FitsStatisticsTools::getRange( Double* pixels, Int nPixels, Int* pixelCnt,
								    Double* min, Double* max, Double* mean_acc )
{
	
    
	for ( Int i = 0; i < nPixels; i++ )
	{
		if ( pixels[i] != FitsMath::NaN && FitsMath::isFinite( pixels[i] ) )		
		{
			if ( pixels[i] > *max )
				*max = pixels[i];
			
			if ( pixels[i] < *min )
				*min = pixels[i];

			*mean_acc += pixels[i];
			(*pixelCnt)++;
		}
	}

}

#ifdef USE_TBB
    class ImageRange {
        double * pixels;
    public:
        double minimum, maximum, sum;
        size_t count;
        
        ImageRange(double * data, double min, double max)
          : pixels(data) {
            this->minimum = min;
            this->maximum = max;
            this->sum = 0.0;
            this->count = 0;
        }
        
        ImageRange(const ImageRange& x, split) : pixels(x.pixels){
            minimum = numeric_limits<double>::max();
            maximum = numeric_limits<double>::min();
            sum     = 0;
            count   = 0;
        }

        void operator()(const tbb::blocked_range<size_t>& r) {
            double _min = minimum, _max = maximum, _sum = sum;
            size_t _count = count;

            for(size_t i = r.begin(); i != r.end(); ++i) {
                double value = pixels[i];
                if(valid(value)) {
                    if(value > _max) _max = value;
                    if(value < _min) _min = value;
                    _sum += value;
                    ++_count;
                }
            }

            minimum = _min;
            maximum = _max;
            sum     = _sum;
            count   = _count;
        }
        
        void join(const ImageRange& y) {
            if(y.maximum > maximum) maximum = y.maximum;
            if(y.minimum < minimum) minimum = y.minimum;
            sum   += y.sum;
            count += y.count;
        }
    };

    Void FitsStatisticsTools::getRange_par(
        Double* pixels, Int nPixels, Int* pixelCnt,
        Double* min, Double* max, Double* mean_acc, Int /*nCpus*/ )
    {

        ImageRange range(pixels, *min, *max);

        tbb::task_scheduler_init init;//(nCpus);
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, nPixels), range);

        *min      = range.minimum;
        *max      = range.maximum;
        *mean_acc = range.sum;
        *pixelCnt = range.count;
    }

#else
    Void FitsStatisticsTools::getRange_par(
        Double* pixels, Int nPixels, Int* pixelCnt,
        Double* min, Double* max, Double* mean_acc, Int nCpus )
    {
	    Double min_int = *min;
	    Double max_int = *max;
	    Double mean_acc_int = 0;
	    Int pixelCnt_int = 0;

        #ifdef USE_OPENMP	
            #pragma omp parallel num_threads( nCpus ) firstprivate(min_int,max_int,mean_acc_int,pixelCnt_int)
	            {
            #pragma omp for
        #endif // USE_OPENMP
	    for ( Int i = 0; i < nPixels; i++ )
	    {
		    if ( pixels[i] != FitsMath::NaN && FitsMath::isFinite( pixels[i] ) )		
		    {
			    if ( pixels[i] > max_int )
				    max_int = pixels[i];
    			
			    if ( pixels[i] < min_int )
				    min_int = pixels[i];

			    mean_acc_int += pixels[i];
			    pixelCnt_int++;
		    }
	    }
        #ifdef USE_OPENMP	
            #pragma omp critical
	        {
            // We use critical to make sure that the global comparison is done once at 
            // a time
        #endif // USE_OPENMP

        // Merge the local values with the global ones
	    if ( min_int < *min ) *min = min_int;
	    if ( max_int > *max ) *max = max_int;
	    *mean_acc += mean_acc_int;
	    *pixelCnt += pixelCnt_int;		
        
        #ifdef USE_OPENMP
	        }//end of omp critical
	        }//end of parallel region
        #endif // USE_OPENMP
    }
#endif // USE_TBB

//-----------------------------------------------------------------------------
// Implementations of FitsStatisticsTools::getHistogram
//-----------------------------------------------------------------------------

#ifdef USE_TBB
    class Histogram {
        const double * pixels;
        const double   min;
        const double   mean;
        const double   binSize;
    public:
        vector<double> histogram;
        double         stddev;
    
        Histogram(const double * data, 
                  const double   min, 
                  const double   mean, 
                  const double   binSize, 
                  size_t         length) 
          : pixels(data), min(min), mean(mean), stddev(0.0),
            binSize(binSize), histogram(length, 0.0) {
        }
        
        Histogram(const Histogram& x, split) 
          : pixels(x.pixels), min(x.min), mean(x.mean), stddev(0.0),
            binSize(x.binSize), histogram(x.histogram.size(), 0.0) {
        }
        
        void operator()(const tbb::blocked_range<size_t>& r) {
            for(size_t i = r.begin(); i != r.end(); ++i) {
                double value = pixels[i];
                if(valid(value)) {
                    vector<double>::size_type bin;
                    
                    bin = (vector<double>::size_type)FitsMath::round(binSize * (value - min));
                    ++histogram[bin];
                    
                    stddev += (mean - value) * (mean - value);
                }
            }
        }
        
        void join(const Histogram& y) {
            
            for(vector<double>::size_type i = 0; i < histogram.size(); ++i) {
                histogram[i] += y.histogram[i];
            }
            
            stddev += y.stddev;
        }
    };

    void FitsStatisticsTools::getHistogram_par(
        Double* pixels, Int length, Double* stdev, Double mean, 
        Double min, Double invBinSize, Vector<Double>& histogram, Int /*nCpus*/)
    {	
        Histogram f(pixels, min, mean, invBinSize, histogram.size());
        
        tbb::task_scheduler_init init;//(nCpus);
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, length), f);
        
        for(vector<double>::size_type i = 0; i < histogram.size(); ++i) {
            histogram[i] = f.histogram[i];
        }
        
        *stdev = f.stddev;
    }
#else
    Void FitsStatisticsTools::getHistogram_par(
        Double* pixels, Int length, Double* stdev, Double mean, 
		Double min, Double invBinSize, Vector<Double>& histogram, Int nCpus)
    {	
	    const Vector<Double>::size_type size = histogram.size();
	    Double stdev_int = 0;
		

        #ifdef USE_OPENMP
            #pragma omp parallel num_threads( nCpus ) firstprivate( stdev_int )
	        {
				Double* hist_tmp = new Double[size];
				for ( Int i = 0; i < size; i++ )
					hist_tmp[i] = 0.;
			#pragma omp for
        #endif  // USE_OPENMP
				for ( Int i = 0; i < length; i++)
				{		 			
					if ( pixels[i] != FitsMath::NaN && FitsMath::isFinite( pixels[i] ) )		
					{
						stdev_int += (mean - pixels[i]) * (mean - pixels[i]);

						Int binIndex = (Int)FitsMath::round( invBinSize * (pixels[i] - min) );
						
						/*#ifdef USE_OPENMP
							#pragma omp atomic
						#endif // USE_OPENMP
						histogram[binIndex]++;
						*/
						#ifdef USE_OPENMP
							hist_tmp[binIndex]++;
						#else
							histogram[binIndex]++;
						#endif
					}
				}
    
        #ifdef USE_OPENMP
				#pragma omp critical
				{
					for ( Int i = 0; i < size; i++ )
						histogram[i] += hist_tmp[i];
        #endif // USE_OPENMP
					*stdev += stdev_int;

        #ifdef USE_OPENMP
				} // omp critical
				delete hist_tmp;
			} // omp parallel
        #endif // USE_OPENMP
    }
#endif // USE_TBB

Void FitsStatisticsTools::getHistogram( Double* pixels, Int length, Double* stdev, Double mean, 
									   Double min, Double invBinSize, Vector<Double>& histogram )
{	
	for ( Int i = 0; i < length; i++)
	{		 			
		if ( pixels[i] != FitsMath::NaN && FitsMath::isFinite( pixels[i] ) )		
		{
			*stdev += (mean - pixels[i]) * (mean - pixels[i]);

			Int binIndex = (Int)FitsMath::round( invBinSize * (pixels[i] - min) );
			histogram[binIndex] = histogram[binIndex] + 1;
		}
	}
}

Void FitsStatisticsTools::scaleHistogram( Vector<Double>& histogram, Double* median, Double min,
				Double max, Double* maxBinCount, UInt pixelCount )
{
// Calculate median while scaling the histogram
    *maxBinCount = 0.;
    Double medianCount = 0.;

	UInt binCount = histogram.size();

	UInt cnt = 0;
	for( UInt i = 0; i < binCount; i++ )
	{
		Double bin = histogram[i];
		// Count pixels for the median
		if( medianCount < (Double)pixelCount / 2.0 )
		{
			medianCount += histogram[cnt];	
			cnt++;
		}
		if (histogram[i] > 0.0)
		{
			histogram[i] = FitsMath::log10( bin );
		}
		if( histogram[i] > *maxBinCount )
		{
			*maxBinCount = histogram[i];
		}
	}
	*median = ((Double)cnt / (Double)binCount) * (max - min) + min;

}

//-----------------------------------------------------------------------------
// Implementations of FitsStatisticsTools::initialGuess
//-----------------------------------------------------------------------------

Void FitsStatisticsTools::initialGuess( InitialGuess algorithm, Double minPercent, Double maxPercent, 
									   Double* blackLevel, Double* whiteLevel, Double min, 
									   Double max, Double mean, Double stdev, Double median, 
									   Vector<Double>& histoBins )
{
    switch(algorithm) {
    case guessMeanPMStddev:
        {
		    //the initial guess is set to mean +/- stdev
		    Double b = mean - stdev;
		    Double w = mean + stdev;

		    if ( b < min || !FitsMath::isFinite( b ) )
		    {
			    b = min;
		    }
		    if ( w > max || !FitsMath::isFinite( w ))
		    {
			    w = max;
		    }
		    *blackLevel = b;
		    *whiteLevel = w;
        }
        break;
    case guessPercentage:
        {
		    Double rangeMin = 0;
		    Double rangeMax = 0;
		    Int minBin		= 0;
		    Int maxBin		= 0;
		    Double sum = 0;
		    UInt i = 0;
		    //the initial guess is set to min=3% and max=97%
		    for ( i = 0; i < histoBins.size(); i++)
		    {
			    //sum += pow(10.0,histoBins[i]);
			    sum += histoBins[i];
		    }
		    for ( i = 0; i < histoBins.size(); i++)
		    {
			    if ( rangeMin / sum <= minPercent )
			    {
				    //rangeMin += pow( 10.0, histoBins[i] );
				    rangeMin += histoBins[i];
				    minBin = i;
			    }
			    if ( rangeMax / sum <= maxPercent )
			    {
				    //rangeMax += pow( 10.0, histoBins[i] );
				    rangeMax += histoBins[i];
				    maxBin = i;
			    }
		    }
		    *blackLevel = ((Double)minBin / (Double)histoBins.size()) * (max - min) + min;	
    		
		    *whiteLevel = ((Double)maxBin / (Double)histoBins.size()) * (max - min) + min;	
        }
        break;
    case guessMedianPMStddev:
        {
		    //the initial guess is set to median +/- stdev
		    Double b = median - stdev;
		    Double w = median + stdev;

		    if ( b < min || !FitsMath::isFinite( b ) )
		    {
			    b = min;
		    }
		    if ( w > max || !FitsMath::isFinite( w ))
		    {
			    w = max;
		    }
		    *blackLevel = b;
		    *whiteLevel = w;	
        }
        break;
	}
}
