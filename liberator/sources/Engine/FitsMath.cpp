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

/** @file
 *  Defines the exception base class
 *
 * $Log: FitsMath.cpp,v $
 * Revision 1.5  2010/09/25 15:32:59  kaspar
 * *** empty log message ***
 *
 * Revision 1.4  2010/09/14 13:01:54  kaspar
 * *** empty log message ***
 *
 * Revision 1.3  2008/09/18 08:48:45  kaspar
 * *** empty log message ***
 *
 * Revision 1.2  2008/05/01 13:52:45  teis
 * * Merged in the PDS support
 *
 * Revision 1.1.1.1.2.1  2008/05/01 13:38:58  teis
 * * Started version 2.3
 * * Added initial support for PDS images
 *
 * Revision 1.1.1.1  2008/01/04 13:08:33  lars
 * Initial import
 *
 * Revision 1.10  2005/06/23 12:09:36  kaspar
 * *** empty log message ***
 *
 * Revision 1.9  2005/06/21 17:00:30  teis
 * *** empty log message ***
 *
 * Revision 1.8  2005/06/21 16:24:04  kaspar
 * *** empty log message ***
 *
 * Revision 1.7  2005/02/18 14:03:45  kaspar
 * no message
 *
 * Revision 1.6  2005/02/12 17:09:04  kaspar
 * no message
 *
 * Revision 1.5  2005/02/12 14:10:50  kaspar
 * Added absolute function
 *
 * Revision 1.4  2005/02/01 15:27:19  lars
 * - Updates to Mac GUI.
 * - Fixed various namespace problems between ::Size/::Point and
 *   FitsLiberator::Size and FitsLiberator::Point on Mac.
 *
 * Revision 1.3  2005/01/30 14:38:11  teis
 * Various fixes.
 * FitsLoader can now actually load FITS images, though the stretch is incorrect
 *
 * Revision 1.2  2005/01/23 11:46:04  teis
 * Added Windows stuff
 *
 *
 * @version        $Revision: 1.5 $
 * @date        $Date: 2010/09/25 15:32:59 $
 * 
 * @author        Teis Johansen <teis@siet.dk>
 * @author        Kaspar Kirstein Nielsen <kaspar@barmave.dk>
 * @author        Lars Holm Nielsen <lars@hankat.dk>
 */
#include "FitsMath.h"
#include <math.h>
#include <float.h>

using namespace FitsLiberator::Engine;

const Double FitsMath::NaN = (DoubleMax + HUGE_VAL) / (DoubleMax + HUGE_VAL);

/**
	Returns the maximum value of the given array
	@param vals pointer to array with the values
	@param count number of elements in the array
**/
Double FitsMath::maxval(Double* vals, Int count)
{
	if (vals != NULL)
	{
		Double tmp = vals[0];
		for (Int i = 1; i < count; i++)
		{
			if ( vals[i] > tmp )
			{
				tmp = vals[i];
			}
		}
		return tmp;
	}
	return 0;
}

/**
 * Tests if a double has a finite value.
 * @param d The number to test.
 * @return True if d is finite, false otherwise
 */
/*
Bool FitsMath::isFinite(Double d) {
    #ifdef WINDOWS
        int fp_class = _fpclass(d);
		return (fp_class > _FPCLASS_NINF && fp_class < _FPCLASS_PINF );
    #else
       return isfinite(d);
    #endif
}*/