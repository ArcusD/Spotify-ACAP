/*
 * Spotify Connect ACAP for Axis Audio Manager Edge
 *
 * Protocol (reverse-engineered from Soundtrack sbplayer binary):
 *   ACAP calls Register({"Name":"<AudioSourceName>"}) on:
 *     bus name:  com.axis.AudioConf
 *     obj path:  /com/axis/AudioConf
 *     interface: com.axis.AudioConf.Application
 *   Response contains: SrcId (string) and SinkAlsaDevice (string)
 *   AAM Edge shows the app as eligible once this call succeeds.
 *   ACAP then pipes audio to the returned SinkAlsaDevice via librespot.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <axparameter.h>
#include <gio/gio.h>
#include <microhttpd.h>

static volatile int keep_running = 1;
static pid_t librespot_pid = -1;
static char alsa_device[256] = "default";
static char src_id[128] = "";
static int registration_done = 0;
static pthread_mutex_t reg_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  reg_cond  = PTHREAD_COND_INITIALIZER;

/* The AudioSourceName parameter value — read from ACAP params */
static char audio_source_name[128] = "SpotifyConnect";

static struct MHD_Daemon *mhd_daemon = NULL;

static enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection,
                                            const char *url, const char *method,
                                            const char *version, const char *upload_data,
                                            size_t *upload_data_size, void **con_cls)
{
    if (strcmp(method, "GET") != 0)
        return MHD_NO;

    const char *filepath = NULL;
    if (strcmp(url, "/api/state") == 0) {
        filepath = "localdata/state.json";
    } else if (strcmp(url, "/api/log") == 0) {
        filepath = "localdata/librespot.log";
    } else {
        return MHD_NO;
    }

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        if (strcmp(url, "/api/state") == 0) {
            const char *empty_state = "{\"event\":\"stopped\"}";
            struct MHD_Response *response = MHD_create_response_from_buffer(strlen(empty_state), (void *)empty_state, MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Content-Type", "application/json");
            int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
            return ret;
        } else {
            const char *error_msg = "Log not found\n";
            struct MHD_Response *response = MHD_create_response_from_buffer(strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
            MHD_destroy_response(response);
            return ret;
        }
    }

    struct stat stat_buf;
    if (fstat(fd, &stat_buf) != 0) {
        close(fd);
        return MHD_NO;
    }

    struct MHD_Response *response = MHD_create_response_from_fd(stat_buf.st_size, fd);
    if (strcmp(url, "/api/state") == 0) {
        MHD_add_response_header(response, "Content-Type", "application/json");
    } else {
        MHD_add_response_header(response, "Content-Type", "text/plain");
    }

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

void handle_sigterm(int sig) {
    keep_running = 0;
    if (librespot_pid > 0) kill(librespot_pid, SIGTERM);
}

/*
 * Called when com.axis.AudioConf service appears on the system bus.
 * We immediately call Register with our name.
 */
static void on_audioconf_appeared(GDBusConnection *connection,
                                   const gchar *name,
                                   const gchar *name_owner,
                                   gpointer user_data)
{
    syslog(LOG_INFO, "com.axis.AudioConf appeared (%s) — calling Register", name_owner);

    /* Build JSON argument: {"Name": "<AudioSourceName>"} */
    gchar *json_arg = g_strdup_printf("{\"Name\":\"%s\"}", audio_source_name);

    GError *error = NULL;
    GVariant *result = g_dbus_connection_call_sync(
        connection,
        "com.axis.AudioConf",            /* bus name   */
        "/com/axis/AudioConf",           /* obj path   */
        "com.axis.AudioConf.Application",/* interface  */
        "Register",                      /* method     */
        g_variant_new("(s)", json_arg),  /* args: JSON string */
        G_VARIANT_TYPE("(s)"),           /* reply: JSON string */
        G_DBUS_CALL_FLAGS_NONE,
        10000,                           /* timeout 10s */
        NULL,
        &error
    );
    g_free(json_arg);

    if (error) {
        syslog(LOG_WARNING, "Register call failed: %s", error->message);
        g_error_free(error);

        /* Fall back to direct ALSA default */
        pthread_mutex_lock(&reg_mutex);
        strncpy(alsa_device, "default", sizeof(alsa_device)-1);
        registration_done = 1;
        pthread_cond_signal(&reg_cond);
        pthread_mutex_unlock(&reg_mutex);
        return;
    }

    const gchar *reply_json = NULL;
    g_variant_get(result, "(&s)", &reply_json);
    syslog(LOG_INFO, "Register reply: %s", reply_json ? reply_json : "(null)");

    /* Parse SrcId and SinkAlsaDevice from reply JSON */
    /* Reply looks like: {"SrcId":"xxx","SinkAlsaDevice":"hw:1,0"} or similar */
    if (reply_json) {
        const char *sink_key = "SinkAlsaDevice";
        const char *p = strstr(reply_json, sink_key);
        if (p) {
            p += strlen(sink_key);
            p = strchr(p, ':'); /* Find colon */
            if (p) {
                p = strchr(p, '"'); /* Find opening quote of the value */
                if (p) {
                    p++; /* skip opening quote */
                    const char *end = strchr(p, '"');
                    if (end) {
                        size_t len = end - p;
                        if (len > 0 && len < sizeof(alsa_device)) {
                            strncpy(alsa_device, p, len);
                            alsa_device[len] = '\0';
                        }
                    }
                }
            }
        }

        const char *src_key = "SrcId";
        p = strstr(reply_json, src_key);
        if (p) {
            p += strlen(src_key);
            p = strchr(p, ':'); /* Find colon */
            if (p) {
                p = strchr(p, '"'); /* Find opening quote */
                if (p) {
                    p++;
                    const char *end = strchr(p, '"');
                    if (end) {
                        size_t len = end - p;
                        if (len > 0 && len < sizeof(src_id)) {
                            strncpy(src_id, p, len);
                            src_id[len] = '\0';
                        }
                    }
                }
            }
        }
    }

    syslog(LOG_INFO, "Registered: SrcId=%s, SinkAlsaDevice=%s", src_id, alsa_device);
    g_variant_unref(result);

    pthread_mutex_lock(&reg_mutex);
    registration_done = 1;
    pthread_cond_signal(&reg_cond);
    pthread_mutex_unlock(&reg_mutex);
}

static void on_audioconf_vanished(GDBusConnection *connection,
                                   const gchar *name,
                                   gpointer user_data)
{
    syslog(LOG_WARNING, "com.axis.AudioConf vanished — will re-register when it returns");
    pthread_mutex_lock(&reg_mutex);
    registration_done = 0;
    pthread_mutex_unlock(&reg_mutex);
}

void *dbus_thread(void *arg) {
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    g_bus_watch_name(G_BUS_TYPE_SYSTEM,
                     "com.axis.AudioConf",
                     G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                     on_audioconf_appeared,
                     on_audioconf_vanished,
                     NULL,
                     NULL);

    syslog(LOG_INFO, "D-Bus: watching for com.axis.AudioConf...");

    while (keep_running) {
        g_main_context_iteration(g_main_loop_get_context(loop), FALSE);
        usleep(100000);
    }

    g_main_loop_unref(loop);
    return NULL;
}

static void start_librespot(char *device_name, char *bitrate, char *final_device) {
    librespot_pid = fork();
    if (librespot_pid == 0) {
        int log_fd = open("localdata/librespot.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (log_fd >= 0) { dup2(log_fd, STDERR_FILENO); close(log_fd); }

        setenv("RUST_LOG", "librespot=info,libmdns=error", 1);

        char aplay_cmd[512];
        snprintf(aplay_cmd, sizeof(aplay_cmd), "aplay -D %s -f S16_LE -r 44100 -c 2", final_device);

        char *args[] = {
            "./librespot",
            "--name",           device_name,
            "--backend",        "subprocess",
            "--device",         aplay_cmd,
            "--bitrate",        bitrate,
            "--mixer",          "softvol",
            "--initial-volume", "100",
            "--volume-ctrl",    "linear",
            "--cache",          "localdata",
            "--onevent",        "./event_handler.sh",
            NULL
        };
        execv(args[0], args);
        syslog(LOG_ERR, "execv librespot failed: %m");
        exit(1);
    }
    syslog(LOG_INFO, "Librespot started (pid=%d) on ALSA device: %s", librespot_pid, final_device);
}

int main(int argc, char *argv[]) {
    openlog("SpotifyConnect", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Starting Spotify Connect ACAP...");

    signal(SIGTERM, handle_sigterm);
    signal(SIGINT,  handle_sigterm);

    mkdir("localdata", 0755);

    /* Start HTTP daemon */
    mhd_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL,
                                  &answer_to_connection, NULL, MHD_OPTION_END);
    if (!mhd_daemon) {
        syslog(LOG_ERR, "Failed to start libmicrohttpd daemon on port 8000");
    } else {
        syslog(LOG_INFO, "Internal HTTP server listening on port 8000");
    }

    /* Read ACAP parameters */
    AXParameter *ax_parameter = ax_parameter_new("spotifyconnect", NULL);
    char *device_name = NULL;
    char *bitrate = NULL;
    char *source_name_param = NULL;
    if (ax_parameter) {
        ax_parameter_get(ax_parameter, "DeviceName",      &device_name,       NULL);
        ax_parameter_get(ax_parameter, "Bitrate",         &bitrate,            NULL);
        ax_parameter_get(ax_parameter, "AudioSourceName", &source_name_param,  NULL);
    }
    if (!device_name)      device_name      = strdup("Axis Speaker");
    if (!bitrate)          bitrate          = strdup("320");
    if (source_name_param && strlen(source_name_param) > 0)
        strncpy(audio_source_name, source_name_param, sizeof(audio_source_name)-1);

    syslog(LOG_INFO, "AudioSourceName: %s, DeviceName: %s", audio_source_name, device_name);

    /* Start D-Bus registration thread */
    pthread_t dbus_tid;
    pthread_create(&dbus_tid, NULL, dbus_thread, NULL);

    /* Wait up to 10s for successful registration with AudioConf */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 10;

    pthread_mutex_lock(&reg_mutex);
    while (!registration_done && keep_running) {
        int rc = pthread_cond_timedwait(&reg_cond, &reg_mutex, &ts);
        if (rc == ETIMEDOUT) {
            syslog(LOG_WARNING, "Timed out waiting for AudioConf. Falling back to plughw:CARD=adau1761audio");
            strncpy(alsa_device, "plughw:CARD=adau1761audio", sizeof(alsa_device)-1);
            registration_done = 1;
            break;
        }
    }
    pthread_mutex_unlock(&reg_mutex);

    char final_alsa_device[256];
    if (strncmp(alsa_device, "plug:", 5) == 0 || strncmp(alsa_device, "plughw:", 7) == 0) {
        strncpy(final_alsa_device, alsa_device, sizeof(final_alsa_device) - 1);
        final_alsa_device[sizeof(final_alsa_device) - 1] = '\0';
    } else {
        snprintf(final_alsa_device, sizeof(final_alsa_device), "plug:%s", alsa_device);
    }

    syslog(LOG_INFO, "Using ALSA device: '%s'", final_alsa_device);
    start_librespot(device_name, bitrate, final_alsa_device);

    /* Main loop: monitor and auto-restart librespot */
    while (keep_running) {
        int status;
        pid_t r = waitpid(librespot_pid, &status, WNOHANG);
        if (r == librespot_pid) {
            syslog(LOG_ERR, "Librespot exited (code=%d). Restarting in 3s...",
                   WIFEXITED(status) ? WEXITSTATUS(status) : -1);
            sleep(3);
            if (keep_running) start_librespot(device_name, bitrate, final_alsa_device);
        }
        sleep(2);
    }

cleanup:
    if (librespot_pid > 0) {
        kill(librespot_pid, SIGTERM);
        waitpid(librespot_pid, NULL, 0);
    }
    if (mhd_daemon) {
        MHD_stop_daemon(mhd_daemon);
    }
    keep_running = 0;
    pthread_join(dbus_tid, NULL);
    syslog(LOG_INFO, "Spotify Connect ACAP stopped.");
    return 0;
}
