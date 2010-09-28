
#pragma once

#include <Carbon.h> 
#include <time.h>
#include "AccumulatingChangeManager.h"

namespace FitsLiberator {
    namespace Mac {
        class MacChangeManager : public FitsLiberator::Modelling::AccumulatingChangeManager {
            typedef FitsLiberator::Modelling::AccumulatingChangeManager super;
			/** Gets the currently executing thread. */
			ThreadID currentThread();
            
			ThreadID        mainThread;
        public:
	    static const UInt32 kEventClassCustom      = FOUR_CHAR_CODE('cust');
	    static const UInt32 kEventKindCustomUpdate = 1;
 
            MacChangeManager();
            virtual ~MacChangeManager();
			
            void Invoke();
            virtual void SendNotifications();
        };
    }
}
