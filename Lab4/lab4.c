#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CONFIG_FILE "daemon.conf"
#define LOG_FILE "daemon.log"
#define PID_FILE "daemon.pid"

sigset_t signals_to_log;

typedef struct {
    const char *name;
    int signo;
} SigInfo;

SigInfo signal_table[] = {
    {"SIGHUP", SIGHUP}, {"SIGINT", SIGINT}, {"SIGQUIT", SIGQUIT},
    {"SIGILL", SIGILL}, {"SIGTRAP", SIGTRAP}, {"SIGABRT", SIGABRT},
    {"SIGBUS", SIGBUS}, {"SIGFPE", SIGFPE}, {"SIGKILL", SIGKILL},
    {"SIGUSR1", SIGUSR1}, {"SIGSEGV", SIGSEGV}, {"SIGUSR2", SIGUSR2},
    {"SIGPIPE", SIGPIPE}, {"SIGALRM", SIGALRM}, {"SIGTERM", SIGTERM},
    {NULL, 0}
};

int get_signo(const char *name) {
    for (int i = 0; signal_table[i].name != NULL; i++) {
        if (strcmp(name, signal_table[i].name) == 0) {
            return signal_table[i].signo;
        }
    }
    return -1;
}

void read_config() {
    sigemptyset(&signals_to_log);
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp) return;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        int signo = get_signo(line);
        if (signo != -1) {
            sigaddset(&signals_to_log, signo);
        }
    }
    fclose(fp);
}

void log_signal(int sig) {
    time_t now = time(NULL);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Failed to open log file");
        return;
}

    char buf[128];
    int len = snprintf(buf, sizeof(buf), "[%s] Received signal: %d (%s)\n",
                       time_buf, sig, strsignal(sig));
    write(fd, buf, len);
    close(fd);
}

void handle_sighup(int sig) {
    if (sigismember(&signals_to_log, sig)) {
        log_signal(sig);
    }
    read_config();
}

void handle_sigterm(int sig) {
    if (sigismember(&signals_to_log, sig)) {
        log_signal(sig);
    }
    remove(PID_FILE);
    exit(0);
}

void handle_generic(int sig) {
    log_signal(sig);
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(sig, &sa, NULL);
    kill(getpid(), sig);
}

void setup_signal_handlers() {
    struct sigaction sa;

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = handle_sighup;
    sigaction(SIGHUP, &sa, NULL);

    sa.sa_handler = handle_sigterm;
    sigaction(SIGTERM, &sa, NULL);

    for (int signo = 1; signo < NSIG; signo++) {
        if (signo == SIGHUP || signo == SIGTERM) continue;
        if (sigismember(&signals_to_log, signo)) {
            sa.sa_handler = handle_generic;
            sigaction(signo, &sa, NULL);
        } else {
            sa.sa_handler = SIG_DFL;
            sigaction(signo, &sa, NULL);
        }
    }
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    if (pid > 0) exit(0);

    setsid();
    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void cleanup_pid_file() {
    remove(PID_FILE);
}

int main(int argc, char *argv[]) {
    if (argc == 2 && (strcmp(argv[1], "-q") == 0)) { 
        FILE *pid_fp = fopen(PID_FILE, "r");
        if (!pid_fp) {
            perror("fopen");
            exit(1);
        }
        pid_t pid;
        fscanf(pid_fp, "%d", &pid);
        fclose(pid_fp);
        kill(pid, SIGTERM);
        exit(0);
    }

    daemonize();

    FILE *pid_fp = fopen(PID_FILE, "w");
    if (!pid_fp) {
        perror("Failed to create PID file");
        exit(1);
    }
    if (pid_fp) {
        fprintf(pid_fp, "%d", getpid());
        fclose(pid_fp);
        atexit(cleanup_pid_file);
    }

    read_config();
    setup_signal_handlers();

    while (1) {
        pause();  
    }

    return 0;
}