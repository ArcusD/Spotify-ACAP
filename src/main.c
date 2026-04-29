#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <axparameter.h>

static volatile int keep_running = 1;
static pid_t child_pid = -1;

void handle_sigterm(int sig) {
    keep_running = 0;
    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
    }
}

int main(int argc, char *argv[]) {
    openlog("SpotifyConnect", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Starting Spotify Connect ACAP...");

    signal(SIGTERM, handle_sigterm);
    signal(SIGINT, handle_sigterm);

    AXParameter *ax_parameter = ax_parameter_new("spotifyconnect", NULL);
    char *device_name = NULL;
    char *bitrate = NULL;

    if (ax_parameter) {
        ax_parameter_get(ax_parameter, "DeviceName", &device_name, NULL);
        ax_parameter_get(ax_parameter, "Bitrate", &bitrate, NULL);
    }

    if (!device_name) device_name = strdup("Axis Speaker");
    if (!bitrate) bitrate = strdup("160");

    syslog(LOG_INFO, "Device Name: %s, Bitrate: %s", device_name, bitrate);

    child_pid = fork();
    if (child_pid == 0) {
        // Child process: execute librespot
        // Note: librespot must be in the same directory as this binary
        // Enable debug logging for librespot
        setenv("RUST_LOG", "debug", 1);
        
        // Redirect stdout and stderr to a log file
        int fd = open("localdata/librespot.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        char *args[] = {
            "./librespot",
            "--name", device_name,
            "--bitrate", bitrate,
            "--backend", "alsa",
            "--mixer", "softvol",
            "--volume-ctrl", "log",
            "--disable-audio-cache",
            "--initial-volume", "100",
            "--onevent", "./event_handler.sh",
            NULL
        };
        syslog(LOG_INFO, "Executing librespot...");
        execv(args[0], args);
        
        // If execv returns, it failed
        syslog(LOG_ERR, "Failed to execute librespot: %m");
        exit(1);
    } else if (child_pid < 0) {
        syslog(LOG_ERR, "Failed to fork: %m");
    } else {
        // Parent process: monitor child
        while (keep_running) {
            int status;
            pid_t result = waitpid(child_pid, &status, WNOHANG);
            if (result == child_pid) {
                if (WIFEXITED(status)) {
                    syslog(LOG_ERR, "librespot process exited with status %d", WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    syslog(LOG_ERR, "librespot process killed by signal %d", WTERMSIG(status));
                }
                break;
            }
            sleep(5);
        }
    }

    if (device_name) free(device_name);
    if (bitrate) free(bitrate);
    if (ax_parameter) ax_parameter_free(ax_parameter);

    syslog(LOG_INFO, "Spotify Connect ACAP stopped.");
    closelog();
    
    // Sleep for 5 seconds before exiting to prevent high-CPU crash loops
    // in case librespot fails instantly.
    sleep(5);
    
    return 0;
}
