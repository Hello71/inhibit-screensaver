// based on code from VLC
// https://github.com/videolan/vlc/blob/734a505509828c3333067ea6be174f3a6525e486/modules/misc/inhibit/dbus.c

#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>

enum inhibit_api
{
    INVALID = 0,
    FDO_SS, /**< KDE >= 4 and GNOME >= 3.10 */
    FDO_PM, /**< KDE and GNOME <= 2.26 */
    MATE,  /**< >= 1.0 */
    GNOME, /**< GNOME 2.26..3.4 */
};

#define MAX_API (GNOME+1)

/* Currently, all services have identical service and interface names. */
static const char dbus_service[][40] =
{
    [INVALID] = { 0 },
    [FDO_SS] = "org.freedesktop.ScreenSaver",
    [FDO_PM] = "org.freedesktop.PowerManagement.Inhibit",
    [MATE]   = "org.mate.SessionManager",
    [GNOME]  = "org.gnome.SessionManager",
};

static const char dbus_path[][33] =
{
    [INVALID] = { 0 },
    [FDO_SS] = "/ScreenSaver",
    [FDO_PM] = "/org/freedesktop/PowerManagement",
    [MATE]   = "/org/mate/SessionManager",
    [GNOME]  = "/org/gnome/SessionManager",
};

static const char dbus_method_uninhibit[][10] =
{
    [INVALID] = { 0 },
    [FDO_SS] = "UnInhibit",
    [FDO_PM] = "UnInhibit",
    [MATE]   = "Uninhibit",
    [GNOME]  = "Uninhibit",
};

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("usage: inhibit-screensaver APPLICATION REASON");
        exit(1);
    }

    DBusConnection *conn;
    dbus_uint32_t cookie = 0;
    enum inhibit_api api = INVALID;

    DBusError err;
    dbus_error_init(&err);

    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (conn == NULL) {
        fprintf(stderr, "cannot connect to session bus: %s\n", err.message);
        // dbus_error_free(&err);
        exit(1);
    }

    for (unsigned i = 1; i < MAX_API; i++) {
        if (dbus_bus_name_has_owner(conn, dbus_service[i], NULL)) {
            api = i;
            break;
        }
    }

    if (!api) {
        fprintf(stderr, "cannot find screensaver dbus name\n");
        // dbus_error_free(&err);
        exit(1);
    }

    dbus_bool_t ret;

    DBusMessage *msg = dbus_message_new_method_call(dbus_service[api], dbus_path[api], dbus_service[api], "Inhibit");
    if (!msg) {
        fputs("cannot create dbus message", stderr);
        exit(1);
    }

    const char *app = argv[1];
    const char *reason = argv[2];

    switch (api)
    {
        case MATE:
        case GNOME:
        {
            dbus_uint32_t xid = 0; // FIXME ?
            dbus_uint32_t gflags = 0xC;

            ret = dbus_message_append_args(msg, DBUS_TYPE_STRING, &app,
                                                DBUS_TYPE_UINT32, &xid,
                                                DBUS_TYPE_STRING, &reason,
                                                DBUS_TYPE_UINT32, &gflags,
                                                DBUS_TYPE_INVALID);
            break;
        }
        default:
            ret = dbus_message_append_args(msg, DBUS_TYPE_STRING, &app,
                                                DBUS_TYPE_STRING, &reason,
                                                DBUS_TYPE_INVALID);
            break;
    }

    if (!ret) {
        fputs("cannot create dbus Inhibit message (OOM?)", stderr);
        // dbus_error_free(&err);
        exit(1);
    }

    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, DBUS_TIMEOUT_INFINITE, &err);

    dbus_message_unref(msg);

    if (!reply) {
        fprintf(stderr, "cannot send dbus Inhibit message: %s\n", err.message);
        // dbus_error_free(&err);
        exit(1);
    }

    if (!dbus_message_get_args(reply, &err,
                DBUS_TYPE_UINT32, &cookie,
                DBUS_TYPE_INVALID)) {
        fprintf(stderr, "cannot parse dbus Inhibit reply: %s\n", err.message);
        // dbus_message_unref(reply);
        // dbus_error_free(&err);
        exit(1);
    }

    dbus_message_unref(reply);

    char buf;
    while (fread(&buf, 1, 1, stdin) == 1)
        ;

    msg = dbus_message_new_method_call(dbus_service[api], dbus_path[api], dbus_service[api], dbus_method_uninhibit[api]);
    if (!msg) {
        fputs("cannot create dbus message", stderr);
        exit(1);
    }

    if (!dbus_message_append_args(msg, DBUS_TYPE_UINT32, &cookie, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "cannot create dbus UnInhibit message: %s\n", err.message);
        // dbus_error_free(&err);
        exit(1);
    }

    if (!dbus_connection_send(conn, msg, NULL)) {
        fputs("cannot send dbus UnInhibit message", stderr);
        // dbus_error_free(&err);
        exit(1);
    }

    dbus_connection_flush(conn);
    dbus_connection_close(conn);
    dbus_connection_unref(conn);
    return 0;
}
