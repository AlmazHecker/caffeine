#include "sleep_prevention.h"

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <CoreFoundation/CoreFoundation.h>
#elif __linux__
#include <dbus/dbus.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#endif

SleepPrevention::SleepPrevention() {
#ifdef _WIN32
    previousState = 0;
#elif __APPLE__
    assertionID = 0;
    assertionActive = false;
#elif __linux__
    connection = nullptr;
    cookie = 0;
    inhibitActive = false;
    initDBus();
#endif
}

SleepPrevention::~SleepPrevention() {
    disableSleepPrevention();
#ifdef __linux__
    if (connection) {
        dbus_connection_unref(connection);
    }
#endif
}

#ifdef __linux__
void SleepPrevention::initDBus() {
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error)) {
        printf("DBus connection error: %s\n", error.message);
        dbus_error_free(&error);
        connection = nullptr;
    }
}

bool SleepPrevention::sendDBusInhibit(bool inhibit) {
    if (!connection) return false;

    const char* service = "org.gnome.SessionManager";
    const char* path = "/org/gnome/SessionManager";
    const char* interface = "org.gnome.SessionManager";
    const char* method = inhibit ? "Inhibit" : "Uninhibit";

    DBusMessage* msg = dbus_message_new_method_call(service, path, interface, method);
    if (!msg) return false;

    if (inhibit) {
        const char* app_id = "Caffeine";
        const char* reason = "Preventing system sleep";
        uint32_t toplevel_xid = 0;
        uint32_t flags = 8;

        if (!dbus_message_append_args(msg,
            DBUS_TYPE_STRING, &app_id,
            DBUS_TYPE_UINT32, &toplevel_xid,
            DBUS_TYPE_STRING, &reason,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID)) {
            dbus_message_unref(msg);
            return false;
        }
    }
    else {
        if (!dbus_message_append_args(msg,
            DBUS_TYPE_UINT32, &cookie,
            DBUS_TYPE_INVALID)) {
            dbus_message_unref(msg);
            return false;
        }
    }

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(connection, msg, 1000, nullptr);
    dbus_message_unref(msg);

    if (!reply) return false;

    if (inhibit) {
        if (!dbus_message_get_args(reply, nullptr,
            DBUS_TYPE_UINT32, &cookie,
            DBUS_TYPE_INVALID)) {
            dbus_message_unref(reply);
            return false;
        }
    }

    dbus_message_unref(reply);
    return true;
}
#endif

bool SleepPrevention::enableSleepPrevention() {
#ifdef _WIN32
    previousState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    return previousState != 0;
#elif __APPLE__
    if (assertionActive) return true;

    CFStringRef reasonForActivity = CFSTR("Caffeine: Preventing system sleep");
    IOReturn success = IOPMAssertionCreateWithName(
        kIOPMAssertionTypeNoDisplaySleep,
        kIOPMAssertionLevelOn,
        reasonForActivity,
        &assertionID
    );

    if (success == kIOReturnSuccess) {
        assertionActive = true;
        return true;
    }
    return false;
#elif __linux__
    if (inhibitActive) return true;

    if (sendDBusInhibit(true)) {
        inhibitActive = true;
        return true;
    }

    if (system("systemd-inhibit --what=idle --who=Caffeine --why='Preventing sleep' --mode=block sleep infinity &") == 0) {
        inhibitActive = true;
        return true;
    }

    return false;
#endif
}

bool SleepPrevention::disableSleepPrevention() {
#ifdef _WIN32
    if (previousState != 0) {
        SetThreadExecutionState(ES_CONTINUOUS);
        previousState = 0;
        return true;
    }
    return false;
#elif __APPLE__
    if (assertionActive && assertionID != 0) {
        IOReturn success = IOPMAssertionRelease(assertionID);
        if (success == kIOReturnSuccess) {
            assertionActive = false;
            assertionID = 0;
            return true;
        }
    }
    return false;
#elif __linux__
    if (inhibitActive) {
        if (connection && sendDBusInhibit(false)) {
            inhibitActive = false;
            return true;
        }

        system("pkill -f 'systemd-inhibit.*Caffeine'");
        inhibitActive = false;
        return true;
    }
    return false;
#endif
}

bool SleepPrevention::isActive() const {
#ifdef _WIN32
    return previousState != 0;
#elif __APPLE__
    return assertionActive;
#elif __linux__
    return inhibitActive;
#endif
}

const char* SleepPrevention::getPlatformName() const {
#ifdef _WIN32
    return "Windows";
#elif __APPLE__
    return "macOS";
#elif __linux__
    return "Linux";
#else
    return "Unknown";
#endif
}
