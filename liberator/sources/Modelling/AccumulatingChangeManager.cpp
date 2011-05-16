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
#include "AccumulatingChangeManager.h"

using namespace FitsLiberator::Modelling;

//-------------------------------------------------------------------------------
//	AccumulatingChangeManager Implementation
//-------------------------------------------------------------------------------

/**
 * Default constructor
 */
AccumulatingChangeManager::AccumulatingChangeManager() {
}

/**
 * Default destructor - warn if mapping not empty.
 */
AccumulatingChangeManager::~AccumulatingChangeManager() {
	if( !mapping.empty() ) {
		#ifdef WINDOWS

		#else
			#ifdef _DEBUG
				DebugStr("\pChangeManager mapping is not empty!");	
			#endif
		#endif
	}
	
	if( !notificationList.empty() ) {
		#ifdef WINDOWS
		
		#else
			#ifdef _DEBUG
				DebugStr("\pChangeManager notification list is not empty!");	
			#endif
		#endif
	}

}

/**
 * Register a mapping between a Subject and an Observer.
 *
 * @param s The Subject
 * @param o The Observer
 */
Void AccumulatingChangeManager::Register( Model * s, Observer * o ) {
    lock.lock();

    Observers* observers = mapping[s];
    
    if( observers == NULL )  { 
        observers = new Observers();
        mapping[s] = observers;
    }
    
    observers->insert( o );

    lock.unlock();
}

/**
 * Unregister a mapping between a Subject and an Observer.
 *
 * @param s The Subject
 * @param o The Observer
 */
Void AccumulatingChangeManager::Unregister( Model * s, Observer * o ) {
    lock.lock();

    Observers* observers = mapping[s];
    
    if( observers != NULL )  {
        observers->erase( o );
    
        if( observers->empty() ) {
            delete observers;
            observers = NULL;
        }
    }
    
    // Since we may have unregistered the last observer attached to the 
    // subject, we test to see if we should remove the subject.
    if( observers == NULL ) {
        mapping.erase(s);
    }

    lock.unlock();
}

/**
 * Makes the ChangeManager aware of that a Subject that has been changed.
 *
 * @param m Pointer to the model that was chaned
*/
Void AccumulatingChangeManager::Notify( Model * m ) {
    lock.lock();

    Observers* observers = mapping[m];
    
    if( observers != NULL ) {
        ObserversIterator it = observers->begin();
    
        while( it != observers->end() ) {
            notificationList[*it].insert( m );
            it++;
        }    
    }

    lock.unlock();
}


/**
 * Send out the accumulated list of notifications.
 *
 */
Void AccumulatingChangeManager::SendNotifications() {
    lock.lock();
    ObserverModelMapping tmpList = notificationList;
    notificationList.clear();
    lock.unlock();
	
    HashIterator it = tmpList.begin();
    
    while( it != tmpList.end() ) {
        ((*it).first)->Update( &(*it).second );
        it++;
    }
}

//-------------------------------------------------------------------------------
//	ACMController Implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for the Controller
 *
 * @param chman Pointer to a ChangeManager
 */
ACMController::ACMController( AccumulatingChangeManager* chman ) {
    _changeManager = chman;
}

/**
 * Sends notifications
 * 
 */
Void ACMController::SendNotifications( ) {
    _changeManager->SendNotifications();
}