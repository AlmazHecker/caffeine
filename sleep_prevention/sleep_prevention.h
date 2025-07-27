#ifdef _WIN32
#include <windows.h>
#endif

#ifndef SLEEP_PREVENTION_H
#define SLEEP_PREVENTION_H

class SleepPrevention {
public:
    SleepPrevention();
    ~SleepPrevention();

    bool enableSleepPrevention();
    bool disableSleepPrevention();
    bool isActive() const;
    const char* getPlatformName() const;

private:
#ifdef _WIN32
    EXECUTION_STATE previousState;
#elif __APPLE__
    unsigned int assertionID;
    bool assertionActive;
#elif __linux__
    struct DBusConnection* connection;
    unsigned int cookie;
    bool inhibitActive;
    void initDBus();
    bool sendDBusInhibit(bool inhibit);
#endif
};

#endif // SLEEP_PREVENTION_H
#pragma once
