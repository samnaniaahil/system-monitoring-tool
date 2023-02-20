#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <utmp.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/resource.h>

struct cpuStats;

void displaySystemUsage(int num_samples, int delay, bool graphicsUsage);
void displayUserUsage(int num_samples, int delay);
void displaySystemInfo();
void printMemoryInfo();
void printCPUUsage(double cpu_usage, int line_ind, bool graphicsUsage);
void getCPUStats();
double calculateTotalCPUUsage(struct cpuStats *cs_prev, struct cpuStats *cs_curr);

typedef struct cpuStats {
    char cpu_id[10];
    long user_time;
    long nice_time;
    long system_time;
    long idle_time;
    long iowait_time;
    long irq_time;
    long softirq_time;
    long steal_time;
} cpuStats;


int main(int argc, char **argv) {

    int num_samples = 10;
    int delay = 1;
    int num_pos_args = 0;

    bool displaySysInfo = false;
    bool displayUserInfo = false;
    bool displaySeq = false;
    bool displayGraphics = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--system") == 0) {
            displaySysInfo = true;
        }
        else if (strcmp(argv[i], "--user") == 0) {
            displayUserInfo = true;
        }
        else if (strcmp(argv[i], "--sequential") == 0) {
            displaySeq = true;
        }
        else if (strcmp(argv[i], "--graphics") == 0) {
            displayGraphics = true;
        }
        else if (strchr(argv[i], '=') != NULL) {
            char arg_cpy[20];
            strcpy(arg_cpy, argv[i]);
            char *type = strtok(arg_cpy, "=");

            if (strcmp(type, "--samples") == 0) {
                num_samples = atoi(strtok(NULL, "="));
            }
            else if (strcmp(type, "--tdelay") == 0) {
                delay = atoi(strtok(NULL, "="));
            }
        }
        else if (num_pos_args <= 1 && strchr(argv[i], '-') == NULL) {
            if (num_pos_args == 0) {
                num_samples = atoi(argv[i]);
            }
            else if (num_pos_args == 1) {
                delay = atoi(argv[i]);
            }
            num_pos_args++;
        }
    }

    if (num_samples < 0) {
        num_samples = 10;
    }
    if (delay <= 0) {
        delay = 1;
    }
    
    printf("Nbr of samples: %d -- every %d secs\n", num_samples, delay);
    
    // Print memory used by current program
    FILE* proccess_p = fopen("/proc/self/status", "r");
    char line[64];
    while (fgets(line, sizeof(line), proccess_p) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            printf("%s", line);
            break;
        }
    }
    fclose(proccess_p);

    if (displaySeq) {
        for (int x = 0; x < num_samples; x++) {
            printf("------- Iteration %d ---------\n", x);
            displaySystemUsage(1, delay, false);
            printf("--------------------------------\n");
            displayUserUsage(1, 0);
            printf("--------------------------------\n\n");
        }  
    }
    else {
        if (displaySysInfo || (!displaySysInfo && !displayUserInfo)) {
            printf("--------------------------------\n");
            displaySystemUsage(num_samples, delay, displayGraphics);
        }

        if (displayUserInfo && !displaySysInfo) {
            displayUserUsage(num_samples, delay);
            
        }
        else if (displayUserInfo || (!displaySysInfo && !displayUserInfo)) {
            printf("--------------------------------\n");
            displayUserUsage(1, 0);
        }
    }

    printf("--------------------------------\n");
    displaySystemInfo();
    printf("--------- Finished! ------------\n");

    return 0;
}


void displaySystemInfo() {
    struct utsname uts;
    uname(&uts);

    printf("##### System Information #####\n");
    printf("System name = %s\n", uts.sysname);
    printf("Machine name = %s\n", uts.nodename);
    printf("Version = %s\n", uts.version);
    printf("Release = %s\n", uts.release);
    printf("Architecture = %s\n", uts.machine);
}

void getCPUStats(cpuStats *cs) {
    FILE *fp = fopen("/proc/stat", "r");

    fscanf(fp, "%s %ld %ld %ld %ld %ld %ld %ld %ld", cs->cpu_id, &(cs->user_time), &(cs->nice_time),
        &(cs->system_time), &(cs->idle_time), &(cs->iowait_time), &(cs->irq_time), &(cs->softirq_time), &(cs->steal_time));

    fclose(fp);
}

double calculateTotalCPUUsage(cpuStats *cs_prev, cpuStats *cs_curr) {
    double usage_prev = cs_prev->user_time + cs_prev->nice_time + cs_prev->system_time 
        + cs_prev->irq_time + cs_prev->softirq_time + cs_prev->steal_time;

    double usage_curr = cs_curr->user_time + cs_curr->nice_time + cs_curr->system_time 
        + cs_curr->irq_time + cs_curr->softirq_time + cs_curr->steal_time;
    
    double cpu_usage = ((usage_curr - usage_prev) / (double)usage_prev) * 100.0;
    
    return cpu_usage;
}

void printMemoryInfo() {
    FILE *fp_mem = fopen("/proc/meminfo", "r");
    long total_ram;
    long free_ram;
    long total_swap;
    long free_swap;

    fscanf(fp_mem, "MemTotal: %ld kB MemFree: %ld kB \
    MemAvailable: %*d kB Buffers: %*d kB Cached: %*d kB SwapCached: %*d kB Active: %*d kB Inactive: %*d kB \
    Active(anon): %*d kB Inactive(anon): %*d kB Active(file): %*d kB Inactive(file): %*d kB Unevictable: %*d kB Mlocked: %*d kB \
    SwapTotal: %ld kB SwapFree: %ld kB", 
    &total_ram, &free_ram, &total_swap, &free_swap);

    // Physical memory
    printf("%.2f GB / %.2f GB", (total_ram-free_ram)/1024000.0, total_ram/1024000.0);
    // Virtual memory
    printf(" -- %.2f GB / %.2f GB\n", ((total_ram-free_ram) + (total_swap-free_swap))/1024000.0, (total_ram+total_swap)/1024000.0);

    fclose(fp_mem);
}

void displaySystemUsage(int num_samples, int delay, bool graphicsUsage) {
    printf("Number of cores: %d\n", get_nprocs_conf());
    printf("Total CPU Usage: \n");
    printf("--------------------------------\n");

    printf("#### Memory Usage ####\n");
    printf("(Physical Mem. Used / Tot) -- (Virtual Mem. used / Tot)\n");

    for (int i = 0; i < num_samples; i++) {
        printMemoryInfo();

        cpuStats cs_prev;
        cpuStats cs_curr;
        getCPUStats(&cs_prev);
        sleep(delay);
        getCPUStats(&cs_curr);
        double cpu_usage = calculateTotalCPUUsage(&cs_prev, &cs_curr);
        printCPUUsage(cpu_usage, i, graphicsUsage);
    }
}

void displayUserUsage(int num_samples, int delay) { 
    int num_prev_users = 0;
    printf("#### Sessions/Users ####\n");
    for (int i = 0; i < num_samples; i++) {
        struct utmp *ut_p;
        setutent();
        ut_p = getutent();

        // Delete previous user list from console
        for (int j = 0; j < num_prev_users; j++) {
            printf("\x1b[2K"); // Delete current line
            printf("\033[%dA", 1); // Move up 1 line
        }

        num_prev_users = 0;
        while (ut_p != NULL) {
            if (ut_p->ut_type == USER_PROCESS) {
                printf("%s %7s %7s\n", ut_p->ut_user, ut_p->ut_line, ut_p->ut_host);
                num_prev_users++;
            }
            ut_p = getutent();
        }
        endutent();

        ("\033F");
        fflush(stdout);
        sleep(delay);
    }
}

void printCPUUsage(double cpu_usage, int line_ind, bool graphicsUsage) {
    printf("\033[%dA", line_ind+5); // Move up n lines
    printf("\x1b[2K"); // Delete current line
    printf("Total CPU Usage: %.5f%%", cpu_usage);
    
    if (graphicsUsage) {
        if (cpu_usage < 0) {
            printf(" | (-) ");
        }
        else {
            printf(" | (+) ");
        }
        for (int j = 0; j < (int)(cpu_usage); j++) {
            printf("*");
        }
    }
    printf("\n");
    printf("\033[%dB", line_ind+5); // Move down n lines
    printf("\033F");
    fflush(stdout);
}