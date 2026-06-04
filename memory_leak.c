#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#define DEFAULT_DURATION_SEC    36000    
#define INITIAL_DELAY_SEC       300     
#define BURST_MB_PER_SECOND     1       
#define ADD_MIN_MB              0       
#define ADD_MAX_MB              3  
#define FREE_MIN_MB             0       
#define FREE_MAX_MB             2      
#define CHUNK_SIZE_KB           1024    

typedef struct MemChunk {
    void            *ptr;
    size_t           size;
    struct MemChunk *next;
    struct MemChunk *prev;
} MemChunk;

static MemChunk    *chunk_head   = NULL;   
static MemChunk    *chunk_tail   = NULL;   
static size_t       total_held   = 0;      
static size_t       total_alloc  = 0;      
static size_t       total_freed  = 0;      
static long         cycle_count  = 0;
static volatile int running      = 1;
static void   handle_signal(int sig);
static void   register_chunk(void *ptr, size_t size);
static size_t free_oldest_mb(int mb_to_free);
static void   do_fake_work(size_t size);
static void   print_stats(const char *phase, int delta_mb);
static double bytes_to_mb(size_t b);

int main(int argc, char *argv[]){
    int duration = DEFAULT_DURATION_SEC;
    if (argc >= 2) duration = atoi(argv[1]);
    srand((unsigned int)time(NULL));
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);
    time_t start = time(NULL);
    printf("=== aging_process started ===\n");
    printf("PID              : %d\n",   getpid());
    printf("Initial Standby  : %d s (%d min)\n", INITIAL_DELAY_SEC, INITIAL_DELAY_SEC / 60);
    printf("Base Rate        : %d MB/s\n", BURST_MB_PER_SECOND);
    printf("ADD  cycle range : base + rand[%d, %d]  =>  [%d, %d] MB allocated\n",
           ADD_MIN_MB,  ADD_MAX_MB,
           BURST_MB_PER_SECOND + ADD_MIN_MB,
           BURST_MB_PER_SECOND + ADD_MAX_MB);
    printf("FREE cycle range : rand[%d, %d] MB freed from oldest chunks\n\n",
           FREE_MIN_MB, FREE_MAX_MB);
    printf("[INFO] Process locked. No memory will leak for the first %d minutes.\n",
           INITIAL_DELAY_SEC / 60);
    fflush(stdout);

    while (running) {
        time_t now = time(NULL);

        if ((now - start) >= duration) {
            printf("\n[INFO] Max duration reached during standby. Exiting.\n");
            goto shutdown;
        }

        if ((now - start) >= INITIAL_DELAY_SEC)
            break;

        long remaining = INITIAL_DELAY_SEC - (now - start);
        printf("\r[STANDBY] Waiting... %02ld:%02ld remaining   ",
               remaining / 60, remaining % 60);
        fflush(stdout);
        sleep(1);
    }

    if (!running) goto shutdown;

    printf("\n[INFO] Standby over. Entering paired ADD/FREE loop.\n\n");
    fflush(stdout);

    while (running) {

        if ((time(NULL) - start) >= duration) {
            printf("\n[INFO] Max duration reached. Stopping.\n");
            break;
        }

        int add_offset = ADD_MIN_MB + rand() % (ADD_MAX_MB - ADD_MIN_MB + 1);
        int add_mb     = BURST_MB_PER_SECOND + add_offset;

        size_t add_target = (size_t)add_mb  * 1024ULL * 1024ULL;
        size_t chunk_size = (size_t)CHUNK_SIZE_KB * 1024ULL;
        size_t added      = 0;

        while (added < add_target && running) {
            void *block = malloc(chunk_size);
            if (!block) {
                printf("\n[CRITICAL] malloc() failed after %.2f MB held! OOM.\n",
                       bytes_to_mb(total_held));
                fflush(stdout);
                running = 0;
                break;
            }
            memset(block, 0xAA, chunk_size);
            do_fake_work(chunk_size);
            register_chunk(block, chunk_size);
            total_held  += chunk_size;
            total_alloc += chunk_size;
            added       += chunk_size;
        }

        if (!running) break;

        cycle_count++;
        print_stats(" ADD", +add_mb);
        sleep(1);

        if ((time(NULL) - start) >= duration) {
            printf("\n[INFO] Max duration reached. Stopping.\n");
            break;
        }

        int    free_mb = FREE_MIN_MB + rand() % (FREE_MAX_MB - FREE_MIN_MB + 1);
        size_t freed   = free_oldest_mb(free_mb);

        total_held  -= freed;
        total_freed += freed;

        cycle_count++;
        print_stats("FREE", -(int)(freed / (1024 * 1024)));
        sleep(1);
    }

shutdown:
    printf("\n=== Shutting down ===\n");
    printf("Total allocated  : %.2f MB\n", bytes_to_mb(total_alloc));
    printf("Total freed      : %.2f MB\n", bytes_to_mb(total_freed));
    printf("Net held         : %.2f MB\n", bytes_to_mb(total_held));
    return 0;
}

static void register_chunk(void *ptr, size_t size){
    MemChunk *c = malloc(sizeof(MemChunk));
    if (!c) return;
    c->ptr  = ptr;
    c->size = size;
    c->next = NULL;
    c->prev = chunk_tail;

    if (chunk_tail) chunk_tail->next = c;
    else            chunk_head       = c;  
    chunk_tail = c;
}

static size_t free_oldest_mb(int mb_to_free){
    size_t target = (size_t)mb_to_free * 1024ULL * 1024ULL;
    size_t freed  = 0;

    while (chunk_head && freed < target) {
        MemChunk *c = chunk_head;

        freed      += c->size;
        chunk_head  = c->next;
        if (chunk_head) chunk_head->prev = NULL;
        else            chunk_tail       = NULL;

        free(c->ptr);
        free(c);
    }

    return freed;
}

static void do_fake_work(size_t size){
    volatile double acc = 0.0;
    size_t iters = size / 256;
    if (iters > 10000) iters = 10000;
    for (size_t i = 0; i < iters; i++)
        acc += sin((double)i) * cos((double)(i ^ 0xA5));
    (void)acc;
}

static void print_stats(const char *phase, int delta_mb){
    printf("[%s][cycle %5ld] %+3d MB  |  held=%.2f MB"
           "  alloc=%.2f  freed=%.2f\n",
           phase, cycle_count, delta_mb,
           bytes_to_mb(total_held),
           bytes_to_mb(total_alloc),
           bytes_to_mb(total_freed));
    fflush(stdout);
}

static double bytes_to_mb(size_t b) { return (double)b / (1024.0 * 1024.0); }
static void   handle_signal(int sig) { (void)sig; running = 0; }
