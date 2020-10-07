//
// Based on the start-rk3328-pwm-fan.sh (FriendlyArm)
// Alexander Finger
// Tab = 4

// #define _DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/file.h>

#define msleep(x) usleep(x*1000)
#define BUFSIZE     512
#define KBUFSIZE    512
#define PORTNO      8888


#define PWM0_OFF 0
#define PWM0_ON !PWM0_OFF
/*
 *
 * Kernel 5.x
 *
*/
#define UNEXPORT_PWM0_PATH  "/sys/class/pwm/pwmchip0/unexport"
#define EXPORT_PWM0_PATH    "/sys/class/pwm/pwmchip0/export"
#define PWM0_PATH           "/sys/class/pwm/pwmchip0/pwm0"
#define ENABLE_PWM0_PATH    "/sys/class/pwm/pwmchip0/pwm0/enable"
#define DUTY_PWM0_PATH      "/sys/class/pwm/pwmchip0/pwm0/duty_cycle"
#define PERIOD_PWM0_PATH    "/sys/class/pwm/pwmchip0/pwm0/period"
#define CPU_TEMP_PATH       "/sys/class/thermal/thermal_zone0/temp"
#define PWM0_PID            "/var/run/fan-monitor.pid"

#define CPUTEMPS_MAX 6
int CpuTemps[CPUTEMPS_MAX] = { 75000, 63000, 58000, 55000, 53000, 52000 };
int PwmDutyCycles[CPUTEMPS_MAX] = { 100000, 300000, 500000, 700000, 800000, 900000 };
int Percents[CPUTEMPS_MAX] = { 100, 70, 50, 30, 20, 10 };

#define DefaultDuty         999999
#define DefaultPercents     0


/* BUILD RELEASE:
 * make clean
 * make
 *
 * BUILD DEBUG:
 * make clean
 * make debug
 */

char g_buffer[32];
int g_running;
int global = 1;                 /* In BSS segment, will automatically be assigned '0' */

char *unk = "Unknown";

int Find_pwm0(void)
{
    int fh;

    fh = open(EXPORT_PWM0_PATH, O_RDONLY);
    if (fh < 0) {
#ifdef _DEBUG
        printf("--- No pwm0 available ---\n");
#endif
        return -1;
    }
    close(fh);

    return 1;
}

int Export_pwm0(void)
{
    int fh, ret;
    ssize_t len;

    fh = open(EXPORT_PWM0_PATH, O_WRONLY);
    if (fh < 0) {
#ifdef _DEBUG
        printf("--- No pwm0 available ---\n");
#endif
        return -1;
    }
    len = 1;
    ret = write(fh, "0", (ssize_t) len);
    if (ret < 0) {
#ifdef _DEBUG
        printf("--- Error exporting pwm0 ---\n");
#endif
        return ret;
    }
    // read(fh, buf, sizeof(buf) - 1);
    // sscanf(buf, "%f", &load);
    close(fh);
    return ret;
}

int UnExport_pwm0(void)
{
    int fh, ret;
    // char fbuf[64];
    ssize_t len;

    fh = open(UNEXPORT_PWM0_PATH, O_WRONLY);
    if (fh < 0) {
#ifdef _DEBUG
        printf("--- No unexport available ---\n");
#endif
        return -1;
    }
    len = 1;
    ret = write(fh, "0", (ssize_t) len);
    if (ret < 0) {
#ifdef _DEBUG
        printf("--- Error UnExporting pwm0 ---\n");
#endif
        return ret;
    }
    // read(fh, buf, sizeof(buf) - 1);
    // sscanf(buf, "%f", &load);
    close(fh);
    return ret;
}


int Write_pwm0_period(int period)
{
    int fh, ret;
    char buf[64];
    ssize_t len;

    sprintf(buf, "%d", period);

    fh = open(PERIOD_PWM0_PATH, O_WRONLY);
    if (fh < 0) {
#ifdef _DEBUG
        printf("--- No pwm0 period available ---\n");
#endif
        return -1;
    }
    len = strlen(buf);
    ret = write(fh, buf, (ssize_t) len);
    if (ret < 0) {
#ifdef _DEBUG
        printf("--- Error writing pwm0 period ---\n");
#endif
        return ret;
    }
    // read(fh, buf, sizeof(buf) - 1);
    // sscanf(buf, "%f", &load);
    close(fh);
    return ret;
}

int Write_pwm0_duty_cycle(int duty_cycle)
{
    int fh, ret;
    char buf[64];
    ssize_t len;

    sprintf(buf, "%d", duty_cycle);

    fh = open(DUTY_PWM0_PATH, O_WRONLY);
    if (fh < 0) {
#ifdef _DEBUG
        printf("--- No pwm0 duty_cycle available ---\n");
#endif
        return -1;
    }
    len = strlen(buf);
    ret = write(fh, buf, (ssize_t) len);
    if (ret < 0) {
#ifdef _DEBUG
        printf("--- Error writing pwm0 duty_cycle ---\n");
#endif
        return ret;
    }
    // read(fh, buf, sizeof(buf) - 1);
    // sscanf(buf, "%f", &load);
    close(fh);
    return ret;
}

int Write_pwm0_enable(int enable)
{
    int fh, ret;
    char buf[64];
    ssize_t len;

    sprintf(buf, "%d", enable);

    fh = open(ENABLE_PWM0_PATH, O_WRONLY);
    if (fh < 0) {
#ifdef _DEBUG
        printf("--- No pwm0 period available ---\n");
#endif
        return -1;
    }
    len = strlen(buf);
    ret = write(fh, buf, (ssize_t) len);
    if (ret < 0) {
#ifdef _DEBUG
        printf("--- Error writing pwm0 enable ---\n");
#endif
        return ret;
    }
    // read(fh, buf, sizeof(buf) - 1);
    // sscanf(buf, "%f", &load);
    close(fh);
    return ret;
}


int Read_CpuTemp(void)
{
    int fh, ret;
    char buf[32];
    int temp;

    fh = open(CPU_TEMP_PATH, O_RDONLY);
    if (fh < 0) {
#ifdef _DEBUG
        printf("--- No CpuTemp available ---\n");
#endif
        return -1;
    }
    ret = read(fh, buf, 31);
    if (ret < 0) {
#ifdef _DEBUG
        printf("--- No CpuTemp available (Reading) ---\n");
#endif
        return ret;
    }
    close(fh);
    temp = atoi(buf);
    return temp;
}

void usage(char *appname)
{
    fprintf(stderr, "\nUsage: %s [-debug]\n\n", appname);
    fprintf(stderr, "  -debug  Defines the file to debug.\n");
    fprintf(stderr, "\nFor more information please visit https://github.com/avafinger/fan-monitor\n\n");

    exit(1);
}

void sig_catchint(int sig)
{
#ifdef _DEBUG
    printf("--- SIG: %d ---\n", sig);
#endif
    switch (sig) {
    case 1:
        g_running = 0;
        return;
    case 2:
        g_running = 0;
        exit(0);
    case 4:
        // SIGILL Illegal Instruction
        g_running = 0;
        break;
    case 8:
        // SIGFPE Floating Point Exception
        g_running = 0;
        break;
    case 11:
        // SIGSEGV Segmentation Violation
        g_running = 0;
        exit(-1);
    case 13:
        // SIGPIPE
        return;
    case 15:
        // SIGTERM
        g_running = 0;
        break;
    case 22:
        // SIGABRT Abnormal Termination
        g_running = 0;
        break;
    default:
        g_running = 0;
        break;
    }
}

void setsigs(void)
{
    int i;
    int numsigs = NSIG;
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_catchint;
    sigemptyset(&sa.sa_mask);
    for (i = 1; i < numsigs; i++) {
        switch (i) {
        case SIGKILL:          // 9
        case SIGUSR1:          // 10
        case SIGUSR2:          // 12
        case SIGCHLD:          // 17
        case SIGCONT:          // 18
        case SIGSTOP:          // 19
            continue;
        }
        sigaction(i, &sa, NULL);
    }
}

int service_handler(void)
{
    int spin, duty, perc;
    int cputemp, i;

    g_running = 1;

    /*
     * main loop: wait
     */
    while (g_running) {
        duty = DefaultDuty;
        cputemp = Read_CpuTemp();
        i = spin = perc = 0;
        while (i < CPUTEMPS_MAX) {
            if (cputemp > CpuTemps[i]) {
                duty = PwmDutyCycles[i];
                perc = Percents[i];
                break;
            }
            i++;
        }
        Write_pwm0_duty_cycle(duty);
#ifdef _DEBUG
        printf("** Temp: %d, duty: %d, %d%%\n", cputemp, duty, perc);
#endif
        msleep(2500);
    }
    return g_running;
}

int main(int argc, char **argv)
{
    pid_t child_pid;
#ifdef _FLOCK_2_
    struct flock fl;
#endif
    int i;
    /* int status; */
    int ret, fd;
    int local;

    /* ----- Arguments ----- */
    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            if (strcasecmp(argv[i], "-debug") == 0) {
                if ((i + i) == argc) {
                    usage(argv[0]);
                    return 1;
                }
                i++;
            } else {
                usage(argv[0]);
                return 1;
            }
        }
    }

    local = 0;
#ifdef _DEBUG
    printf("Debug mode!\n");
#endif

    fd = open(PWM0_PID, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd != -1) {
        close(fd);
        perror("Already running\n");
        exit(1);
    }

    ret = UnExport_pwm0();
    ret = Export_pwm0();
    if (ret == -1) {
        perror("Unable to run FAN monitor on pwm0!\n");
        exit(1);
    }
    msleep(20);
    Write_pwm0_period(1000000);
    msleep(20);
    Write_pwm0_duty_cycle(100000);
    msleep(20);
    Write_pwm0_enable(PWM0_ON);
    msleep(20);
    Write_pwm0_duty_cycle(DefaultDuty);

    /* now create new process */
#ifdef _DEBUG
    child_pid = 0;
#else
    child_pid = fork();
#endif

    if (child_pid >= 0) {       /* fork succeeded */
        if (child_pid == 0) {   /* fork() returns 0 for the child process */
#ifdef _DEBUG
            printf("--- Child process! ---\n");
#endif

            setsigs();
            // Increment the local and global variables
            local++;
            global++;
#ifdef _DEBUG
            printf("Child PID =  %d, parent pid = %d\n", getpid(), getppid());
            printf("Child's local = %d, Child's global = %d\n", local, global);
#endif

            fd = open(PWM0_PID, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
            if (fd == -1) {
                perror("Unable to save PID. Fatal.\n");
                exit(1);
            }
#ifdef _FLOCK_
            if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
                /* other process already open, abort */
                perror("already running...\n");
                exit(1);
            }
#endif

#ifdef _FLOCK_2_
            fl.l_start = 0;
            fl.l_len = 0;
            fl.l_type = F_WRLCK;
            fl.l_whence = SEEK_SET;
            if (fcntl(fd, F_SETLK, &fl) < 0) {
                perror("already running...\n");
                exit(1);
            }
#endif


            ret = sprintf(g_buffer, "%d", getpid());
            ret = write(fd, &g_buffer, (ssize_t) ret);
            close(fd);

            ret = service_handler();

	    Write_pwm0_duty_cycle(DefaultDuty);
            unlink(PWM0_PID);
#ifdef _DEBUG
            printf("--- Child says bye! ret=%d---\n\n", ret);
#endif
            return ret;
        } else {                /* parent process */
#ifdef _DEBUG
            printf("--- Parent process! ---\n");
            printf("Parent PID =  %d, child pid = %d\n", getpid(), child_pid);
#endif

#ifdef _TEST_SPAWN
            /* wait for child to exit, and store child's exit status */
            wait(&status);
            printf("child exit code (from Parent): %d\n", WEXITSTATUS(status));
#endif
            /* The changes in local and global variable in child process should not reflect here in parent process. */
#ifdef _DEBUG
            printf("\nParent's local = %d, parent's  global = %d\n", local, global);
            printf("--- Parent says bye! ---\n");
#endif

            exit(0);            /* parent exits */
        }
    } else {                    /* failure */
        perror("Unable to fork!");
        exit(1);
    }
}
