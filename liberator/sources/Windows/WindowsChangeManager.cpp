#include "WindowsChangeManager.h"

using FitsLiberator::Windows::WindowsChangeManager;

UINT WindowsChangeManager::CM_UPDATE = 0;

WindowsChangeManager::WindowsChangeManager() 
  : super(), notificationBroker(NULL) {
    if(CM_UPDATE == 0) {
        CM_UPDATE = ::RegisterWindowMessage(TEXT("WindowsChangeManager"));
    }
}

WindowsChangeManager::~WindowsChangeManager() {}

void
WindowsChangeManager::Broker(HWND window) {
    this->notificationBroker = window;
}

HWND
WindowsChangeManager::Broker() const {
    return this->notificationBroker;
}

void
WindowsChangeManager::SendNotifications() {
    bool shouldMarshall = (CM_UPDATE != 0) 
        && (notificationBroker != NULL)
        && (::GetCurrentThreadId() != ::GetWindowThreadProcessId(notificationBroker, NULL));

    // TODO: Detect if we have the case where we should marshall but are not able to
    if(shouldMarshall) {
        ::PostMessage(notificationBroker, CM_UPDATE, 0, reinterpret_cast<LPARAM>(this));
    } else {
        Invoke();
    }
}

void 
WindowsChangeManager::Invoke() {
    super::SendNotifications();    
}
