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
 * 
 * Base classes for implementing the Observer Pattern with a Change Manager.
 *
 * Here is a simple example of how to use the the Observer Pattern with the 
 * ChangeManager:
 *
 * \code
 * class PixelValueModel : public Model {
 *     public:
 *         PixelValueModel( ChangeManager * chman ) : Model( chman ) {};
 *         void setPosition(Int x, Int y);
 *         Int getX();
 *         Int getY();
 *         Int getValue(); 
 *         
 *     private:
 *         Int     x;
 *         Int     y;
 *         Double     value;
 * };
 * 
 * class PixelValueView : public Observer {
 *     public:
 *         PixelValueView( Model * m );
 *         void Update( Model * m );
 *     private:
 *         PixelValueModel * model;
 * };
 * 
 * \endcode
 * 
 * \code 
 *  PixelValueView::PixelValueView( Model * m ) {
 *      model = m;
 *      model->Attach( this );
 *  }
 * 
 * void PixelValueView::Update( Model * m ) {
 *     if( model == m) {
 *         // then fetch something from model
 *     }
 * }
 * \endcode
 */
#ifndef __OBSERVER_H__
#define __OBSERVER_H__

#include "FitsLiberator.h"

#include <set>

namespace FitsLiberator {
    namespace Modelling {
        class Model;
        class Observer;

        typedef std::set<Model *> Models;    ///<


        /**
        * Pure abstract base class for the Change Manager. The Change Manager's responsibility 
        * is to maintain the Model-Observer mappings and to notify the Observers when a 
        * Model is changed.
        */
        class ChangeManager {
            public:
			virtual ~ChangeManager() {}
                virtual Void Register( Model *, Observer * ) = 0;
                virtual Void Unregister( Model *, Observer * ) = 0;
                virtual Void Notify( Model * ) = 0;
        };

        /**
        * Observer of a Model
        */
        class Observer {
            public:
				virtual ~Observer() {}
                virtual Void Update( Models * m ) = 0;
        };

        /**
        * Model which may be observed by any number of Observers that whish to be
        * notified when the Model is changed.
        */
        class Model {
            public:    
				virtual ~Model() {}
                virtual Void Attach( Observer* );
                virtual Void Detach( Observer* );
                virtual Void Notify();
            protected:
                Model( ChangeManager* chman );
            private:
                ChangeManager * _changeManager;        ///< Change Manger

        };
    }
}
#endif