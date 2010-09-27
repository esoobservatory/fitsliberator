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
 * Accumulating Change Manager for the Observer Pattern. This Change Manager
 * maintains a list over changed subjects, and when told, it send out notifications
 * to their observers.  
 */
#ifndef __AccumulatingChangeManager_H__
#define __AccumulatingChangeManager_H__

#include "FitsLiberator.h"
#include "Observer.h"
#ifdef WINDOWS
#include <hash_map>
#else
#include <ext/hash_map>
#include "HashFuncs.h"
#endif
#include <set>
#include <boost/thread.hpp>

namespace FitsLiberator {
    namespace Modelling {

        typedef std::set<Observer *>			Observers;			///<
        typedef std::set<Observer *>::iterator	ObserversIterator;	///<
        typedef std::set<Model *>::iterator		ModelsIterator;		///<

        #ifdef __GNUC__
//            struct eqModel { bool operator()(Model * s1, Model * s2) const { return s1 == s2; } };
//            struct eqObserver { bool operator()(Observer * s1, Observer * s2) const { return s1 == s2; } };



//
//            typedef __gnu_cxx::hash_map< Model *, Observers *, __gnu_cxx::hash<Model *>, eqModel > ModelObserverMapping;
//            typedef __gnu_cxx::hash_map< Observer *, Models, __gnu_cxx::hash<Observer *>, eqObserver > ObserverModelMapping;
//            typedef __gnu_cxx::hash_map< Observer *, Models, __gnu_cxx::hash<Observer *>, eqObserver>::iterator HashIterator;
            typedef __gnu_cxx::hash_map< Model *, Observers *> ModelObserverMapping;
            typedef __gnu_cxx::hash_map< Observer *, Models> ObserverModelMapping;
            typedef __gnu_cxx::hash_map< Observer *, Models>::iterator HashIterator;
        #else
            typedef stdext::hash_map< Model *, Observers * > ModelObserverMapping;
            typedef stdext::hash_map< Observer *, Models > ObserverModelMapping;
            typedef stdext::hash_map< Observer *, Models >::iterator HashIterator;
        #endif

        /**
        * Accumulating Change Manager for the Observer Pattern. This Change Manager
        * maintains a list over changed models, and when told, it send out notifications
        * to their observers.  
        */
        class AccumulatingChangeManager : public ChangeManager {
            public:
            	AccumulatingChangeManager();
				virtual ~AccumulatingChangeManager();

                void Register( Model * s, Observer* o );
                void Unregister( Model * s, Observer* o );
                void Notify( Model * s );
                virtual void SendNotifications();
                
            private:    
                boost::mutex            lock;
                ModelObserverMapping    mapping;            ///< Model-Observer mapping.
                ObserverModelMapping    notificationList;    ///< List of changed models.
        };
        
        /**
         * Base class for Controllers using the ChangeManager.
         */
        class ACMController {
            public:
				virtual ~ACMController() {}
                virtual Void SendNotifications();
			protected:
				ACMController( AccumulatingChangeManager* chman );
            private:
                AccumulatingChangeManager * _changeManager;
        };

    }
}
#endif