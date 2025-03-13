/**
 * This file is for implementation of MIMPI library.
 * */

#include "channel.h"
#include "mimpi.h"
#include "mimpi_common.h"

#include <stdint.h>
#include <pthread.h>

#include <sys/select.h>

#define FINISHED_STATE 0
#define MY_STATE -1
#define RUNS_STATE 1

#define SIGNALING_CODE -1

int n = 0;

typedef struct Msg_linked_list {
    void *data;
    int CNT;
    int SRC;
    int TAG;
    struct Msg_linked_list* next;
} Msg_linked_list;

Msg_linked_list rcv_buf_dummy_head;
Msg_linked_list* rcv_buf_tail;

// -1 - me, 0 - finished, 1 - running:
int* processes_states;

typedef struct Meta_history {
    int CNT;
    int OTHER;
    int TAG;
    struct Meta_history* next;
} Meta_history;

Meta_history** sent_meta_hist_dummy_heads;
Meta_history** sent_meta_hist_tails;

Meta_history** requests_meta_hist_dummy_heads;
Meta_history** requests_meta_hist_tails;

bool global_deadlock_detection_on;

pthread_mutex_t tail_mutex;
pthread_cond_t main_cond;
bool main_turn; // to avoid random main awakings
bool end_thread;

pthread_t buffering_thread;

int MIMPI_World_size() {
    return n;
}

int MIMPI_World_rank() {
    char env_name[20];

    // Construct the environment variable name with the "MIMPI_" prefix
    sprintf(env_name, "MIMPI_%d", getpid());

    // Retrieve the environment variable value
    char* rank_str = getenv(env_name);
    if (rank_str != NULL) {
        return atoi(rank_str);
    }

    return 0;
}

int get_square_channel_read_fd(int writing_process_rank, int sync) {
    const int WORLD_SIZE = MIMPI_World_size();
    const int MY_RANK = MIMPI_World_rank();
    int OFFSET = 0;
    if (writing_process_rank > MY_RANK) {
        OFFSET += -4;
    }
    if (sync == 1) {
        OFFSET += 2;
    }

    return FIRST_FD_NO + 4 * MY_RANK * (WORLD_SIZE - 1) + 4 * writing_process_rank + OFFSET;
}

int get_square_channel_write_fd(int reading_process_rank, int sync) {
    const int WORLD_SIZE = MIMPI_World_size();
    const int MY_RANK = MIMPI_World_rank();
    int OFFSET = 1;
    if (MY_RANK > reading_process_rank) {
        OFFSET += -4;
    }
    if (sync == 1) {
        OFFSET += 2;
    }

    return FIRST_FD_NO + 4 * reading_process_rank * (WORLD_SIZE - 1) + 4 * MY_RANK + OFFSET;
}

void inherit_from_run() {
    const char* n_str = getenv("MIMPI_n");
    n = 1;
    if (n_str != NULL) {
        n = atoi(n_str);
    }
}

void init_processes_states() {
    processes_states = malloc(sizeof(int) * n);

    for (int i = 0; i < n; i++) {
        if (i == MIMPI_World_rank()) {
            processes_states[i] = MY_STATE;
        }
        else {
            processes_states[i] = RUNS_STATE;
        }
    }
}

void init_rcv_buffer() {
    rcv_buf_dummy_head.data = NULL;
    rcv_buf_dummy_head.CNT = -13;
    rcv_buf_dummy_head.SRC = -1;
    rcv_buf_dummy_head.TAG = 0;
    rcv_buf_dummy_head.next = NULL;

    rcv_buf_tail = &rcv_buf_dummy_head;
}

// If the first read int is SIGNALING_CODE then reads count, tag n other.
// Otherwise the first read int is count and then reads tag n sets other to -1.
// Returns -1 if meta src finished, 0 otherise.
int rcv_meta_data(
    const int SRC,
    int* count, 
    int* tag,
    int* other
) {
    const int RECV_FROM_SRC_FD = get_square_channel_read_fd(SRC, 0);
    
    int local_cnt = -1;
    int local_tag = -1;
    int local_other = -1;

    if (chrecv(RECV_FROM_SRC_FD, &local_cnt, sizeof(int)) != sizeof(int)) {
        return -1;
    }

    if (local_cnt != SIGNALING_CODE) {
        //printf("%d got a msg\n", MIMPI_World_rank());

        if (chrecv(RECV_FROM_SRC_FD, &local_tag, sizeof(int)) != sizeof(int)) {
            return -1;
        }

        *count = local_cnt;
        *tag = local_tag;
        *other = -1;
    
        return 0;
    }
    else {
        //printf("%d got a signal about waiting\n", MIMPI_World_rank());

        if (chrecv(RECV_FROM_SRC_FD, &local_cnt, sizeof(int)) != sizeof(int)) {
            return -1;
        }

        if (chrecv(RECV_FROM_SRC_FD, &local_other, sizeof(int)) != sizeof(int)) {
            return -1;
        }

        if (chrecv(RECV_FROM_SRC_FD, &local_tag, sizeof(int)) != sizeof(int)) {
            return -1;
        }

        *count = local_cnt;
        *tag = local_tag;
        *other = local_other;

        //printf("%d and will return 0 from rcv meta, got %d %d %d\n", MIMPI_World_rank(), *local_cnt, *other, *tag);

        return 0;
    }

    return 0;
}

int tags_match(int tag1, int tag2) {
    return tag1 == tag2 || tag1 == MIMPI_ANY_TAG || tag2 == MIMPI_ANY_TAG;
}

int get_match(
    const int CNT1, const int CNT2, 
    const int SRC1, const int SRC2, 
    const int TAG1, const int TAG2
) {
    return CNT1 == CNT2 && SRC1 == SRC2 && tags_match(TAG1, TAG2);
}

int my_min(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

// Also allowed to set helper_thread_code.
int rcv_NOT_meta_data(const int SRC_FD, void *data, const int CNT) {
    if (CNT <= 512) {
        if (chrecv(SRC_FD, data, CNT) != CNT) {
            return -1;
        }
    }
    else {
        int total_read_bytes_cnt = 0;

        while (total_read_bytes_cnt < CNT) {
            int read = chrecv(
                SRC_FD, data + total_read_bytes_cnt, 
                my_min(512, CNT - total_read_bytes_cnt)
            );
            
            if (read <= 0) {
                return -1;
            }

            total_read_bytes_cnt += read;
        }
    }

    return 0;
}

void free_node(Msg_linked_list* ptr) {
    free(ptr->data);
    free(ptr);
}

void print_bufor() {
    Msg_linked_list* ptr = rcv_buf_dummy_head.next;

    printf("%d printing bufor:\n", MIMPI_World_rank());
    while (ptr) {
        printf("At there is an iter\n");
        printf("%d %d %d\n", ptr->CNT, ptr->SRC, ptr->TAG);
        ptr = ptr->next;
    }
    printf("%d printing bufor done\n\n", MIMPI_World_rank());
}

void print_bufor_detailed() {
    print_bufor();
}

int proper_state_for_selecting(const int SRC_RANK) {
    return processes_states[SRC_RANK] == RUNS_STATE;
}

// Always returns NULL.
// Ends at the very beginning if n == 1.
// Calls functions which can change helper_thread_code.
void* parallel_buffering(void* pass) {
    //printf("%d helper about to throw THE party!\n", MIMPI_World_rank());

    if (n == 1) {
        return NULL;
    }

    while (1) {
        //printf("%d helper: time to throw a party (waiting for the permission)!\n", MIMPI_World_rank());

        // before receving any msges to the buffer we'll select descs ready to
        // be read from; to use select let's start with initing fd_set read_set:
        fd_set read_fds;
        FD_ZERO(&read_fds);

        // calculate the set size 
        // (the set doesn't contain already closed channels):
        int set_size = 0;
        for (int src_rank = 0; src_rank < n; src_rank++) {
            //printf("%d helper: ->%d\n", MIMPI_World_rank(), processes_states[src_rank]);
            if (proper_state_for_selecting(src_rank)) {
                set_size++;
            }
        }

        //printf("%d helper: set size=%d\n", MIMPI_World_rank(), set_size);

        // prepare fds table for select function:
        int* fds = malloc(sizeof(int) * set_size);

        int fds_table_idx = 0;
        for (int src_rank = 0; src_rank < n; src_rank++) {
            //printf("%d helper: -->%d\n", MIMPI_World_rank(), processes_states[src_rank]);
            if (proper_state_for_selecting(src_rank)) { // if it requires checking receive1/2 states then
                // also update that in calcing set size, one function probably the best option
                //printf("%d helper: adding to fds src=%d\n", MIMPI_World_rank(), src_rank);
                fds[fds_table_idx++] = get_square_channel_read_fd(
                    src_rank, 
                    0
                );
            }
        }

        // Add file descriptors to the set:
        int max_fd = -1;
        for (int i = 0; i < set_size; i++) {
            FD_SET(fds[i], &read_fds);

            if (max_fd < fds[i]) {
                max_fd = fds[i];
            }
        }

        // Call select:
        if (max_fd + 1 >= 20) {
            //printf("%d helper: about to select, nfds=%d\n", MIMPI_World_rank(), max_fd + 1);
            //ASSERT_SYS_OK(select(max_fd + 1, &read_fds, NULL, NULL, NULL)); // No timeout
            select(max_fd + 1, &read_fds, NULL, NULL, NULL); // No timeout
            //printf("%d helper: select ended, will the thread end???\n", MIMPI_World_rank());
        }
        // there is no need to handle else because the thread will end in a normal way
        // since main will notice the last writer leaving since it w ogolnosci
        // zauwaza procesy ktr state ustawiamy na 0

        if (end_thread == 1) { // why not?
            //printf("%d helper: about to about to return after select\n", MIMPI_World_rank());
            
            free(fds);
            // IMO THERE IS NO NEED TO ALLOW MAIN TO WORK BECAUSE THIS PLACE
            // IN CODE BY DEFINITION (IF) MEANS THAT MAIN IS IN MIMPIFINALIZE
            // SO IT DOESN'T REQUIRE BEING WAKED UP IN RCV_FROM_BUF BECAUSE
            // IT'S IN FINALIZE AND THAT FUNCTION IS ALREADY IMPLEMENTED IN SUCH
            // A WAY THAT WAKING UP MAIN ISN'T NECESSARY
            //main_turn = 1;
            //pthread_cond_signal(&main_cond);
            //ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));
            
            //printf("%d helper: about return\n", MIMPI_World_rank());
            
            return NULL;
        }
        /*else {
            printfc("%d helper: the helper won't end after select ended\n", MIMPI_World_rank());
        }*/

        // Check which file descriptors are ready after selecting:
        for (int src_rank = 0; src_rank < n; src_rank++) {
            if (proper_state_for_selecting(src_rank)) {
                const int SRC_FD = get_square_channel_read_fd(
                    src_rank, 
                    0
                );
                
                if (FD_ISSET(SRC_FD, &read_fds)) {
                    //printf("%d helper: %d SENT STH **OR** LEFT THE CHAT :)\n", MIMPI_World_rank(), src_rank);
                    // time to receive:
                    // Handle input from SRC_FD
                    // For example, read data, process it, and store it in a buffer
                    // it's important to remember about changing the state of a src
                    
                    // receive meta data:
                    int received_count = -1;
                    int received_tag = -1;
                    int other = -1;

                    int rcv_meta_res = rcv_meta_data(
                        src_rank,
                        &received_count,
                        &received_tag,
                        &other
                    );

                    //printfc("%d helper: res=%d (if 0 then read NOT meta DATA)\n", MIMPI_World_rank(), res);

                    if (rcv_meta_res == -1) {
                        // While rcving meta it turned out that remote finished.

                        /*printf(
                            "%d helper: B set %d state to 0, AND MAIN TURN TO 1\n", 
                            MIMPI_World_rank(), 
                            src_rank
                        );*/
                        processes_states[src_rank] = FINISHED_STATE;
                        main_turn = 1;
                        pthread_cond_signal(&main_cond);
                    }
                    else if (other != -1) {
                        //printf("%d helper: %d signaled waiting for me\n", MIMPI_World_rank(), other);
                        // We've got a signal about starting to rcv from us.
                        // Time to update requests meta history:
                        Meta_history* new_node = malloc(sizeof(Meta_history));

                        new_node->CNT = received_count;
                        new_node->OTHER = other;
                        new_node->TAG = received_tag;
                        new_node->next = NULL;

                        //printf("%d helper: %d \n", MIMPI_World_rank(), src_rank);
                        //printf("%d helper: %p \n", MIMPI_World_rank(), requests_meta_hist_tails[src_rank]);

                        (requests_meta_hist_tails[src_rank])->next = new_node;
                        //printf("%d helper: I CO1.5?\n", MIMPI_World_rank());
                        requests_meta_hist_tails[src_rank] = new_node;

                        //printf("%d helper: I CO2?\n", MIMPI_World_rank());

                        main_turn = 1;
                        pthread_cond_signal(&main_cond);
                    }
                    else {
                        // meta res != 0 and other == 1 <==> a msg, to for a new
                        // node
                        Msg_linked_list* new_tail = malloc(sizeof(Msg_linked_list));

                        new_tail->CNT = received_count;
                        new_tail->data = malloc(received_count);
                        
                        //printf("%d MALLOCED%d|\n", MIMPI_World_rank(), received_count);    
                        
                        new_tail->next = NULL;
                        new_tail->SRC = src_rank;
                        new_tail->TAG = received_tag;

                        //printf("%d helper: time to rcv NOT meta data :)\n", MIMPI_World_rank());
                        rcv_meta_res = rcv_NOT_meta_data(SRC_FD, new_tail->data, received_count);

                        if (rcv_meta_res == -1) {
                            //printf("%d helper: A error in reading data, set %d state to 0, AND MAIN TURN TO 1\n", MIMPI_World_rank(), src_rank);
                            free_node(new_tail);
                            processes_states[src_rank] = FINISHED_STATE;
                            main_turn = 1;
                            pthread_cond_signal(&main_cond);
                        }
                        else {
                            //printfc("%d helper: waits to lock\n", MIMPI_World_rank());
                            
                            ASSERT_ZERO(pthread_mutex_lock(&tail_mutex));
                            
                            //printfc("%d helper: locked\n", MIMPI_World_rank());

                            //printfc("%d helper: let's see the bufor BEFORE the adding:\n", MIMPI_World_rank());
                            /*printfc(
                                "%d helper: but first if the new tail is ok: %d %d %d\n",
                                MIMPI_World_rank(),
                                new_tail->CNT,
                                //new_tail->data,
                                //new_tail->next,
                                new_tail->SRC,
                                new_tail->TAG
                            );*/
                            //print_bufor_detailed();

                            rcv_buf_tail->next = new_tail;
                            rcv_buf_tail = new_tail;
                            
                            //printf("%d helper: let's see the bufor AFTER the adding:\n", MIMPI_World_rank());
                            //print_bufor_detailed();

                            //printf("%d helper: sets MAIN TURN TO 1, SIGNAL AND UNLOCK:\n", MIMPI_World_rank());
                            main_turn = 1;
                            pthread_cond_signal(&main_cond);
                            ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));
                        }
                    }
                }
            }
        }

        //printf("%d helper: unlocking the mutex :)\n", MIMPI_World_rank());

        // PROBABLY WE DON'T NEED TO WAKE UP THE MAIN THREAD **HERE** BECAUSE WE
        // EVEN MUST NOT DO THIS IN SOME SENSE (PERHAPS IT WOULDN'T BE A PROBLEM
        // BUT FOR SURE IMO WE DON'T NEED TO DO THIS, EVIDENCE: TESTS). IF MAIN
        // WAITS IT WAITS FOR NEW DATA IN THE BUFFER, AND IF HELPER ADDS STH 
        // THEN IT WAKES UP MAIN (SOMEWHERE ABOVE)
        free(fds);
    }
}

void init_parallel_buffering() {
    //main_waits_for = -1;
    
    //printf("%d states\n", MIMPI_World_rank());
    init_processes_states();

    /*for (int i = 0; i < n; i++) {
        printf("%d main: at the beginning:%d\n", MIMPI_World_rank(), processes_states[i]);
    }*/

    init_rcv_buffer();

    ASSERT_ZERO(pthread_mutex_init(&tail_mutex, NULL));
    ASSERT_ZERO(pthread_cond_init(&main_cond, NULL));
    //ASSERT_ZERO(pthread_cond_init(&helper_cond, NULL));
    main_turn = 0;
    end_thread = 0;

    ASSERT_ZERO(pthread_create(&buffering_thread, NULL, parallel_buffering, NULL));
}

// copies enable to the global var, allocs only if enable is true
void init_deadlocks_detection(bool enable_deadlock_detection) {
    global_deadlock_detection_on = enable_deadlock_detection;

    if (enable_deadlock_detection) {
        sent_meta_hist_dummy_heads = malloc(MIMPI_World_size() * sizeof(Meta_history*));
        requests_meta_hist_dummy_heads = malloc(MIMPI_World_size() * sizeof(Meta_history*));

        sent_meta_hist_tails = malloc(MIMPI_World_size() * sizeof(Meta_history*));
        requests_meta_hist_tails = malloc(MIMPI_World_size() * sizeof(Meta_history*));
        
        //printf("%d main: CZEKAJ...\n", MIMPI_World_rank());

        for (int i = 0; i < MIMPI_World_size(); i++) {
            //printf("%d main: i=%d\n", MIMPI_World_rank(), i);
            sent_meta_hist_dummy_heads[i] = malloc(sizeof(Meta_history));
            //printf("%d main: im curious EVEN HERE? %p\n", MIMPI_World_rank(), sent_meta_hist_dummy_heads[0]);
            sent_meta_hist_dummy_heads[i]->next = NULL;
            sent_meta_hist_dummy_heads[i]->CNT = -2137;
            sent_meta_hist_dummy_heads[i]->OTHER = -2137;
            sent_meta_hist_dummy_heads[i]->TAG = -2137;

            sent_meta_hist_tails[i] = sent_meta_hist_dummy_heads[i];
            
            requests_meta_hist_dummy_heads[i] = malloc(sizeof(Meta_history));
            requests_meta_hist_dummy_heads[i]->next = NULL;
            requests_meta_hist_dummy_heads[i]->CNT = -2137;
            requests_meta_hist_dummy_heads[i]->OTHER = -2137;
            requests_meta_hist_dummy_heads[i]->TAG = -2137;

            requests_meta_hist_tails[i] = requests_meta_hist_dummy_heads[i];
        }

        //printf("%d main: im curious? %p\n", MIMPI_World_rank(), sent_meta_hist_dummy_heads[0]);
        //printf("%d main: im curious? %p\n", MIMPI_World_rank(), sent_meta_hist_dummy_heads[1]);
        //printf("%d main: im curious? %p\n", MIMPI_World_rank(), sent_meta_hist_dummy_heads[2]);
        //printf("%d main: im curious? %p\n", MIMPI_World_rank(), sent_meta_hist_dummy_heads[3]);
    }
}

void MIMPI_Init(bool enable_deadlock_detection) {
    inherit_from_run();
    init_deadlocks_detection(enable_deadlock_detection);
    channels_init();
    init_parallel_buffering();
}

void close_descs() {
    const int MY_RANK = MIMPI_World_rank();

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                continue;
            }

            int base = FIRST_FD_NO + 4 * i * (n - 1) + 4 * j;

            if (i < j) {
                base -= 4;
            }

            if (i == MY_RANK) { // close reading fds in the cell
                //printf("%d closes %d %d\n", MY_RANK, base, base + 2);
                ASSERT_SYS_OK(close(base));
                ASSERT_SYS_OK(close(base + 2));
            }
            if (j == MY_RANK) { // close writing fds in the cell
                //printf("%d closes %d %d\n", MY_RANK, base + 1, base + 3);
                ASSERT_SYS_OK(close(base + 1));
                ASSERT_SYS_OK(close(base + 3));
            }
        }
    }
}

void free_buffer() {
    Msg_linked_list* ptr = rcv_buf_dummy_head.next;

    while (ptr != NULL) {
        Msg_linked_list* prev = ptr;
        ptr = ptr->next;
        free_node(prev);
    }
}

void free_n_histories_n_their_heads_table(Meta_history** histories_dummy_heads_table) {
    for (int i = 0; i < MIMPI_World_size(); i++) {
        Meta_history* ptr = histories_dummy_heads_table[i];
        
        while (ptr != NULL) {
            Meta_history* prev = ptr;
            ptr = ptr->next;
            free(prev);
        }
    }

    free(histories_dummy_heads_table);
}

// frees if and only if global enable deadlock detection is set to 1
void free_histories() {
    if (global_deadlock_detection_on) {
        free_n_histories_n_their_heads_table(sent_meta_hist_dummy_heads);
        free_n_histories_n_their_heads_table(requests_meta_hist_dummy_heads);
    
        free(sent_meta_hist_tails);
        free(requests_meta_hist_tails);
    }
}

void free_mem() {
    free_buffer();
    free_histories();
    free(processes_states);
}

void MIMPI_Finalize() {
    // We will need to wait for the helper thread to end by its own
    // (but of course w/ some help from us which is setting end_thread to 1
    // below) after it receives a msg for example that a remote has finished
    // (remote closed it's descs which is done below **without any waiting**).
    // YEAH THIS WORKS, IT'S ABOUT THAT SYNCING ALL IN THE FINALIZE, WHICH
    // IS DONE BY USING BELOW JOIN, EVERYONE WAITS FOR THEIR THREADS TO FINISH
    // AND THEIR HELPERS END WHEN THEY RCV STH (WAKING UP FROM SELECT) FOR 
    // EXAMPLE INFO THAT SOMEONE LEFT (CLOSED THEIR DESC) WHICH 
    // IS DONE BELOW **WITHOUT** ANY WAITING AND ALSO END_THREAD NEEDS TO BE
    // SET TO 1 FOR A HELPER TO END BUT IT'S ALSO DONE "IMIDIATELLY AFTER THE
    // COMMENT".
    
    end_thread = 1; // this ensures that after select the helper will end
    // this will inform other process that we ended, and imo thanks to that in the future
    // our helper will be informed in that someone ends and this will end our select and this the helper
    // since we have set end thread to 1.
    close_descs(); // since we are also closing our reading it requires a specific handle in select

    //printf("%d closed, descs, about to jooin :)\n", MIMPI_World_rank());

    // Teraz nie mozna chyba nic wiecej robic wiec czekamy az
    // KTOS INNY ZAMKNIE SWOJE FDS, CO KAZDY ROBI NA POCZATKU (PO END_THREAD := 1 ) TEJ FUNKCJI
    ASSERT_ZERO(pthread_join(buffering_thread, NULL));

    free_mem(); // Dopiero teraz zeby nie harmowac processes_states table, and any strong arg why not?
    
    // THE MUTEX IS FREE (THE HELPER UNLOCKED IT RIGHT BEFORE RETURNING AND THERE IS NO ONE
    // (IN THIS PROCESS) TO CHANGE IT).
    ASSERT_ZERO(pthread_cond_destroy(&main_cond));
    //ASSERT_ZERO(pthread_cond_destroy(&helper_cond));
    ASSERT_ZERO(pthread_mutex_destroy(&tail_mutex));

    channels_finalize();
}

void remember_this_send(const int CNT, const int DST, const int TAG) {
    Meta_history* new_node = malloc(sizeof(Meta_history));

    new_node->CNT = CNT;
    new_node->OTHER = DST;
    new_node->TAG = TAG;
    new_node->next = NULL;

    (sent_meta_hist_tails[DST])->next = new_node;
    sent_meta_hist_tails[DST] = new_node;
}

MIMPI_Retcode MIMPI_Send(
    void const *data,
    int count,
    int destination,
    int tag
) {
    if (destination == MIMPI_World_rank()) {
        return MIMPI_ERROR_ATTEMPTED_SELF_OP;
    }

    if (destination >= MIMPI_World_size()) {
        return MIMPI_ERROR_NO_SUCH_RANK;
    }

    if (global_deadlock_detection_on) {
        remember_this_send(count, destination, tag);
    }

    // Btw the following code works because each destination has n-1 channels
    // each for a distinct sender, so there will be no interwining of msges and
    // meta info of different sources.

    // In the future calls of recv in the process we don't want to
    // consider the destination as receiving since we will send sth to it.

    // first send meta data and then data:
    const int DST_FD = get_square_channel_write_fd(destination, 0);

    // send count as a meta data:
    int count_send_res = 0; 
    while (count_send_res == 0) {
        count_send_res = chsend(DST_FD, &count, sizeof(count));

        if (count_send_res == -1) {
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }
    
    // send tag as a meta data:
    int tag_send_res = 0;
    while (tag_send_res == 0) {
        tag_send_res = chsend(DST_FD, &tag, sizeof(tag));

        if (tag_send_res == -1) {
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }

    // send data:
    int sent = 0;
    while (sent < count) {
        int send_res = chsend(DST_FD, data + sent, count - sent);

        if (send_res == -1) {
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
        else {
            sent += send_res;
        }
    }

    return MIMPI_SUCCESS;
}

// Assumes tail mutex aquired.
// Changes tail val if the tail was removed:
void rmv_from_buf(Msg_linked_list* prev_ptr, Msg_linked_list* ptr) {
    int tail_rmv = 0;
    
    if (rcv_buf_tail == ptr) { // the tail and the ptr point to the same node:
        tail_rmv = 1;
    }
    
    Msg_linked_list* next = ptr->next;
    prev_ptr->next = next;
    
    free_node(ptr);

    if (tail_rmv) {
        rcv_buf_tail = prev_ptr;
    }
}

void handle_match(void* data, Msg_linked_list* ptr, Msg_linked_list* prev_ptr) {
    memcpy(data, ptr->data, ptr->CNT);
    rmv_from_buf(prev_ptr, ptr);
}

int count_send_matches(const int CNT, const int SRC, const int TAG) {
    int matching_sends = 0;
    //printf("%d main: will we leave that? %d\n", MIMPI_World_rank(), SRC);
    //printf("%d main: will we leave that? %p\n", MIMPI_World_rank(), sent_meta_hist_dummy_heads[SRC]);

    Meta_history* ptr_sent = sent_meta_hist_dummy_heads[SRC]->next;
    //printf("%d main: will we leave that 2?\n", MIMPI_World_rank());
    
    while (ptr_sent != NULL) {
        //printf("%d main: will we leave that 3?\n", MIMPI_World_rank());
        matching_sends += get_match(
            CNT, ptr_sent->CNT, 
            SRC, ptr_sent->OTHER, 
            TAG, ptr_sent->TAG
        );

        ptr_sent = ptr_sent->next;
    }

    //printf("%d main: will we leave that? YES\n", MIMPI_World_rank());

    return matching_sends;
}

int check_deadlock_implying_condiction(
    const int CNT, 
    const int SRC, 
    const int TAG,
    int matching_sends,
    Meta_history** last_checked_request,
    int* matching_requests
) {
    Meta_history* ptr = (*last_checked_request)->next;

    while (ptr != NULL) {
        *matching_requests += get_match(
            CNT, ptr->CNT,
            SRC, ptr->OTHER,
            TAG, ptr->TAG
        );

        *last_checked_request = ptr;
        ptr = ptr->next;
    }

    if (matching_sends < *matching_requests) {
        return 1;
    }

    return 0;
}

MIMPI_Retcode get_from_buffer(
    void* data, 
    const int CNT, 
    const int SRC, 
    const int TAG
) {
    //printf("%d d00\n", MIMPI_World_rank());
    //printf("%d main: lets first try to get from buffer from src=%d\n", MIMPI_World_rank(), SRC);

    // New idea: to solve the first type of remote_finished_error in MIMPI_Receive,
    // in the main thread (so here, since get_from_buffer is only called in MIMPI_Receive)
    // we will aquire the mutex and then search the entire buffer; if there is
    // a match then we receive it into data; if there is no match everything 
    // depends if the src has been finished 
    // (notice that the helper thread doesn't have the mutex then);
    // if the src has been finished then we return REMOTE_FINISHED_ERROR,
    // otherwise we need to na przemian dzialac z pomocniczym (ktr ogarnia CALY
    // select majac mutexa (why not, to nas ani go nie zaglodzi i jest bezpieczne))
    // until pomocniczny albo zupdateuje state co sprawdzamy po zgarnieciu mutexa
    // i wtedy remote_error albo znajdziemy odpowiedz w buforze albo przeplatamy dalej.

    ASSERT_ZERO(pthread_mutex_lock(&tail_mutex));
    //while (!main_turn) {
        //ASSERT_ZERO(pthread_cond_wait(&main_cond, &tail_mutex));
    //}

    //printf("%d d1\n", MIMPI_World_rank());

    Msg_linked_list* prev_ptr = &rcv_buf_dummy_head;
    Msg_linked_list* ptr = rcv_buf_dummy_head.next;

    //printf("%d main: mutex got, let's see the buffer:\n", MIMPI_World_rank());
    //print_bufor();

    while (ptr != NULL) {
        //printf("%d main: try to match\n", MIMPI_World_rank());
        int match = get_match(
            CNT, ptr->CNT,
            SRC, ptr->SRC,
            TAG, ptr->TAG
        );
        
        if (match) {
            //printf("%d main: easy found already in buf bro FOUND\n", MIMPI_World_rank());
            handle_match(data, ptr, prev_ptr);
            main_turn = 0;
            //pthread_cond_signal(&helper_cond);
            //main_waits_for = -1;
            //clean_deadlock_vars_after_rcv(SRC);
            ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));
            return MIMPI_SUCCESS;           
        }
        /*else {
            printf(
                "%d main: didn't match: %d %d %d %d %d %d\n", 
                MIMPI_World_rank(),
                CNT, ptr->CNT,
                SRC, ptr->SRC,
                TAG, ptr->TAG
            );
        }*/

        prev_ptr = ptr;
        ptr = ptr->next;
    }

    //printf("%d main: NOT found in buf, so let the helper work!!!!!\n", MIMPI_World_rank());

    int MATCHING_SENDS = 0;
    int matching_requests = 0;
    Meta_history* last_checked_request = NULL;

    if (global_deadlock_detection_on) {
        //printf("%d main: GLOBAL ON SO THIS\n", MIMPI_World_rank());
        MATCHING_SENDS = count_send_matches(CNT, SRC, TAG);
        //printf("%d main: d1\n", MIMPI_World_rank());
        last_checked_request = requests_meta_hist_dummy_heads[SRC];

        //printf("%d main: d2\n", MIMPI_World_rank());

        int deadlock = check_deadlock_implying_condiction(
            CNT, 
            SRC, 
            TAG, 
            MATCHING_SENDS, 
            &last_checked_request, 
            &matching_requests
        );
        
        if (deadlock) {
            main_turn = 0;
            ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));
            return MIMPI_ERROR_DEADLOCK_DETECTED;
        }
    }

    //printf("%d main: d3\n", MIMPI_World_rank());

    // Now everything depends of if src has been finished:
    if (processes_states[SRC] == FINISHED_STATE) {
        //printf("%d main: d4\n", MIMPI_World_rank());
        //printf("%d I'll tell u even more, not only NOT found, also src finished!!!\n", MIMPI_World_rank());
        main_turn = 0;
        //pthread_cond_signal(&helper_cond);
        //main_waits_for = -1;
        //clean_deadlock_vars_after_rcv(SRC);
        ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));
        return MIMPI_ERROR_REMOTE_FINISHED;
    }

    // We will work now alternately (the main and the helper thread)
    // until remote finshes or we match:
    
    main_turn = 0; // SINCE AN ABOVE PRINT SAYS LET'S HELPER WORK WE WILL NEED TO WAIT
    // I TAK CZY SIAK, WIEC NIC NIE ZASZKODZI USTAWIC SOBIE MAIN TURN NA FALSE (0)

    //pthread_cond_signal(&helper_cond);
    ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));

    while (1) {
        //printf("%d main: waiting for the mutex in while(1), MAIN TURN=%d\n", MIMPI_World_rank(), main_turn);
        ASSERT_ZERO(pthread_mutex_lock(&tail_mutex));
        while (!main_turn) {
            ASSERT_ZERO(pthread_cond_wait(&main_cond, &tail_mutex));
        }
        //printf("%d main: guys, we've got the mutex and cond, let's see buf\n", MIMPI_World_rank());
 
        // After we got the mutex we need to check if the helper thread
        // detected that the src has finished BUT FIRST WE NEED TO CHECK THE BUFFOR FOR A MSG
        // (it really implice that the error should be returned because
        // the fact that we arrived to this place in code means that
        // there was no matching msg waiting in the buffer at the beginning
        // of the function and (*) it was check if the src has been finished
        // (it wasn't) so we were waiting and now the source is finished,
        // so (*) it has changed recently.
        ptr = prev_ptr->next; // We don't need to again 
        // check the buffer from the beginning since the only thing could change
        // while we were waiting was that a new node was added
        // (the helper isn't allowed to remove from the buffer)

        //if (ptr == NULL) {
            //printf("%d main: there is imo no new nodes\n", MIMPI_World_rank());
        //}
        //else {
            //printf("%d main: a new node! maybe matches!\n", MIMPI_World_rank());
        //}

        // Again, to the end of buffer and then pass the mutex to the helper
        // so that sth a writer ends or sth new is added to the buffer.
        while (ptr != NULL) {
            // Try to match:
            int match = get_match(
                CNT, ptr->CNT, 
                SRC, ptr->SRC, 
                TAG, ptr->TAG
            );
        
            if (match) {
                //printf("%d main: match!\n", MIMPI_World_rank());
                handle_match(data, ptr, prev_ptr);
                main_turn = 0;
                //pthread_cond_signal(&helper_cond);
                //main_waits_for = -1;
                //clean_deadlock_vars_after_rcv(SRC);
                ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));
                return MIMPI_SUCCESS;
            }
            else { // Try another recently appeared msg from buffer:
                //printf("%d main: not match, let's try the next msg from buf is there is any\n", MIMPI_World_rank());
                prev_ptr = ptr;
                ptr = ptr->next;
            }
        }

        //printf("%d main: checking buf done!!!\n", MIMPI_World_rank());

        if (global_deadlock_detection_on) {
            //printf("%d main: d5?xd\n", MIMPI_World_rank());
            int deadlock = check_deadlock_implying_condiction(
                CNT, 
                SRC, 
                TAG, 
                MATCHING_SENDS, 
                &last_checked_request, 
                &matching_requests
            );
            //printf("%d main: d6\n", MIMPI_World_rank());
        
            if (deadlock) {
                main_turn = 0;
                ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));
                return MIMPI_ERROR_DEADLOCK_DETECTED;
            }
        }

        if (processes_states[SRC] == FINISHED_STATE) {
            //printf("%d after helper signaled src seems to be finished!!!\n", MIMPI_World_rank());
            
            main_turn = 0; // CHYBA TO MA ZNACZENIE, BO REMOTE FINISH NIE
            // KONIECZNIE KONCZY, WIEC TRZEBA ODPOWIEDNIO TURN ZROBIC
            
            //pthread_cond_signal(&helper_cond);
            //clean_deadlock_vars_after_rcv(SRC);
            ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));
            return MIMPI_ERROR_REMOTE_FINISHED;
        }

        //printf("%d main: src NOT finished, maybe there's a new node?\n", MIMPI_World_rank());

        //printf("%d main: we need to wait again for the helper, MAIN_TURN:=0\n", MIMPI_World_rank());
        // We are again at the end of buffer so we again need to wait until a msg appears or :
        main_turn = 0;
        //pthread_cond_signal(&helper_cond);
        ASSERT_ZERO(pthread_mutex_unlock(&tail_mutex));
        //printf("%d main: MAIN_TURN SHOULD BE 0:%d\n", MIMPI_World_rank(), main_turn);
    }
}

void signal_src_about_waiting(const int CNT, const int SRC, const int TAG) {
    const int FD = get_square_channel_write_fd(SRC, 0);
    
    int wrote1 = 0;
    int tmp = SIGNALING_CODE;

    while (wrote1 == 0) {
        wrote1 = chsend(FD, &tmp, sizeof(tmp));
    }

    int wrote2 = 0;
    while (wrote2 == 0) {
        wrote2 = chsend(FD, &CNT, sizeof(CNT));
    }

    int wrote3 = 0;
    const int MY_RANK = MIMPI_World_rank();
    while (wrote3 == 0) {
        wrote3 = chsend(FD, &MY_RANK, sizeof(MY_RANK));
    }

    int wrote4 = 0;
    while (wrote4 == 0) {
        wrote4 = chsend(FD, &TAG, sizeof(TAG));
    }
}

MIMPI_Retcode MIMPI_Recv(
    void *data,
    int count,
    int source,
    int tag
) {
    if (source == MIMPI_World_rank()) {
        return MIMPI_ERROR_ATTEMPTED_SELF_OP;
    }

    if (source >= MIMPI_World_size()) {
        return MIMPI_ERROR_NO_SUCH_RANK;
    }

    if (global_deadlock_detection_on) {
        signal_src_about_waiting(count, source, tag);
    }

    return get_from_buffer(data, count, source, tag);
}

MIMPI_Retcode MIMPI_Barrier() {
    if (n < 2) {
        return MIMPI_SUCCESS;
    }

    const int MY_RANK = MIMPI_World_rank();
    //printf("%d enters barrier\n", MY_RANK);
    int LEFT_TREEWISE_CHILD_RANK = 2 * MY_RANK + 1;
    int RIGHT_TREEWISE_CHILD_RANK = 2 * MY_RANK + 2;
    int TREEWISE_PARENT_RANK = MY_RANK == 0 ? -1 : (MY_RANK - 1) / 2;

    const int SIZEOF_RANK = sizeof(MY_RANK);

    int tmp = 0;

    // 4 parts: 
    // - receiving "signals" from the EXISTING treewise children 
    //   (n == 16 || n < 16)
    // - "signaling" readyness to the treewise parent
    // - receiving feedback from the treewise parent
    // - giving feedback to the EXISTING treewise children (n == 16 || n < 16)

    // part 1:
    if (LEFT_TREEWISE_CHILD_RANK < MIMPI_World_size()) {
        // receive the "signal" from the left treewise child:
        const int FD = get_square_channel_read_fd(
            LEFT_TREEWISE_CHILD_RANK,
            1
        );

        //printf("%d waitss to rcv from left at %d\n", MY_RANK, FD);
        int kek = chrecv(FD, &tmp, SIZEOF_RANK);

        if (kek != SIZEOF_RANK) {
            //printfc("%d rcved from left %d at %d which is NOT ok, so ERROR\n", MY_RANK, kek, FD);
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
        //printf("%d rcved from left %d which IS ok\n", MY_RANK, kek);
    }
    if (RIGHT_TREEWISE_CHILD_RANK < MIMPI_World_size()) {
        // receive the "signal" from the right treewise child:
        const int FD = get_square_channel_read_fd(
            RIGHT_TREEWISE_CHILD_RANK,
            1
        );

        if (chrecv(FD, &tmp, SIZEOF_RANK) != SIZEOF_RANK) {
            //printfc("b1\n");
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }

    // part 2:
    if (TREEWISE_PARENT_RANK >= 0) {
        // "signal" readyness the treewise parent:
        const int FD = get_square_channel_write_fd(
            TREEWISE_PARENT_RANK,
            1
        );

        //printfc("%d is about to signal readyness uptree at %d\n", MY_RANK, FD);

        int kek = chsend(FD, &MY_RANK, SIZEOF_RANK);
        
        if (kek != SIZEOF_RANK) {
            //printfc("b2 fd=%d who=%d parent_rank=%d, kek=%d\n", FD, MY_RANK, TREEWISE_PARENT_RANK, kek);
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }

    // part 3:
    if (TREEWISE_PARENT_RANK >= 0) {
        // receive feedback from the treewise parent:
        const int FD = get_square_channel_read_fd(
            TREEWISE_PARENT_RANK,
            1
        );

        //printf("%d will wait to rcv feedback from the parent at %d", MY_RANK, FD);

        //printf("%d is about to rcv feedback frmo the parent at %d\n", MY_RANK, FD);
        
        int kek = chrecv(FD, &tmp, SIZEOF_RANK);

        if (kek != SIZEOF_RANK) {
            //printf("%d rcved feedback %d at %d which is NOT ok, so ERROR\n", MY_RANK, kek, FD);
            return MIMPI_ERROR_REMOTE_FINISHED;
        }

        //printf("%d rcved feedback %d which is ok", FD, kek);

        //printf("%d is rcved feedback frmo the parent\n", MY_RANK);
    }

    // part 4:
    // send feedback to the treewise children:
    if (LEFT_TREEWISE_CHILD_RANK < MIMPI_World_size()) {
        const int FD = get_square_channel_write_fd(
            LEFT_TREEWISE_CHILD_RANK,
            1
        );

        //printf("->%d sends feedback to %d at %d!!!\n", MY_RANK, LEFT_TREEWISE_CHILD_RANK, FD);
        if (chsend(FD, &MY_RANK, SIZEOF_RANK) != SIZEOF_RANK) {
            //printfc("b4\n");
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }
    if (RIGHT_TREEWISE_CHILD_RANK < MIMPI_World_size()) {
        const int FD = get_square_channel_write_fd(
            RIGHT_TREEWISE_CHILD_RANK,
            1
        );
        
        if (chsend(FD, &MY_RANK, SIZEOF_RANK) != SIZEOF_RANK) {
            //printfc("b5\n");
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }

    //printf("%d leaves barrier\n", MY_RANK);

    return MIMPI_SUCCESS;
}

MIMPI_Retcode MIMPI_Bcast(
    void *data,
    int count,
    int root
) {
    // 0 part - prepare const variables necessary for bcasting
    // 1 part - sending uptree info that progrs and their subtrees are ready
    // 2 part - since now everything is synced we can end by bcasting downtree

    // 0 part:
    const int MY_RANK = MIMPI_World_rank();
    const int MAPPED_RANK = (MY_RANK - root + n) % n;

    int TREEWISE_PARENT_RANK = -1;
    int MAPPED_TREEWISE_PARENT_RANK = -1;
    if (MAPPED_RANK > 0) {
        MAPPED_TREEWISE_PARENT_RANK = (MAPPED_RANK - 1) / 2;
        TREEWISE_PARENT_RANK = (MAPPED_TREEWISE_PARENT_RANK + root) % n;
    }

    int LEFT_TREEWISE_CHILD_RANK = -1;
    int MAPPED_LEFT_TREEWISE_CHILD_RANK = -1;
    if (MAPPED_RANK * 2 + 1 < n) {
        MAPPED_LEFT_TREEWISE_CHILD_RANK = MAPPED_RANK * 2 + 1;
        
        LEFT_TREEWISE_CHILD_RANK = MAPPED_LEFT_TREEWISE_CHILD_RANK + root;
        LEFT_TREEWISE_CHILD_RANK %= n;
    }

    int RIGHT_TREEWISE_CHILD_RANK = -1;
    int MAPPED_RIGHT_TREEWISE_CHILD_RANK = -1;
    if (MAPPED_RANK * 2 + 2 < n) {
        MAPPED_RIGHT_TREEWISE_CHILD_RANK = MAPPED_RANK * 2 + 2;
        
        RIGHT_TREEWISE_CHILD_RANK = MAPPED_RIGHT_TREEWISE_CHILD_RANK + root;
        RIGHT_TREEWISE_CHILD_RANK %= n;
    }

    const int SIZEOF_RANK = sizeof(MY_RANK);

    int tmp = 0;
    
    // part 1 - uptree communication towards the root to "signal" readyness:
    if (LEFT_TREEWISE_CHILD_RANK != -1) { // recv from the left treewise child
        const int FD = get_square_channel_read_fd(
            LEFT_TREEWISE_CHILD_RANK,
            1
        );
        
        if (chrecv(FD, &tmp, SIZEOF_RANK) != SIZEOF_RANK) {
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }
    if (RIGHT_TREEWISE_CHILD_RANK != -1) { // recv from the ri treewise child
        const int FD = get_square_channel_read_fd(
            RIGHT_TREEWISE_CHILD_RANK,
            1
        );

        if (chrecv(FD, &tmp, SIZEOF_RANK) != SIZEOF_RANK) {
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }
    if (TREEWISE_PARENT_RANK != -1) { // send to the treewise parent
        const int FD = get_square_channel_write_fd(
            TREEWISE_PARENT_RANK,
            1
        );

        if (chsend(FD, &MY_RANK, SIZEOF_RANK) != SIZEOF_RANK) {
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }

    // part 2 - bcasting downtree and this way being free to leave:
    if (TREEWISE_PARENT_RANK != -1) { // recv bcast from the treewise parent
        const int N_BYTES = count;
        const int FD = get_square_channel_read_fd(
            TREEWISE_PARENT_RANK,
            1
        );
        
        int total_received = 0;

        while (total_received < N_BYTES) {
            int read = chrecv(FD, data + total_received, N_BYTES - total_received);

            if (read <= 0) {
                return MIMPI_ERROR_REMOTE_FINISHED;
            }

            total_received += read;
        }
    }
    if (LEFT_TREEWISE_CHILD_RANK != -1) { // bcast to the left treewise child
        const int N_BYTES = count;
        const int FD = get_square_channel_write_fd(
            LEFT_TREEWISE_CHILD_RANK,
            1
        );
        
        int total_sent = 0;

        while (total_sent < N_BYTES) {
            int wrote = chsend(FD, data + total_sent, N_BYTES - total_sent);

            if (wrote < 0) {
                return MIMPI_ERROR_REMOTE_FINISHED;
            }

            total_sent += wrote;
        }
    }
    if (RIGHT_TREEWISE_CHILD_RANK != -1) { // bcast to the ri treewise child
        const int N_BYTES = count;
        const int FD = get_square_channel_write_fd(
            RIGHT_TREEWISE_CHILD_RANK,
            1
        );
        
        int total_sent = 0;

        while (total_sent < N_BYTES) {
            int wrote = chsend(FD, data + total_sent, N_BYTES - total_sent);

            if (wrote < 0) {
                return MIMPI_ERROR_REMOTE_FINISHED;
            }

            total_sent += wrote;
        }
    }
    // since part 1 synced we are free to leave after passing data to children

    return MIMPI_SUCCESS;
}

uint8_t min(uint8_t left, uint8_t right) {
    if (left < right) {
        return left;
    }
    return right;
}

uint8_t max(uint8_t left, uint8_t right) {
    if (left < right) {
        return right;
    }
    return left;
}

void reduce(int count, uint8_t* tmp_calc, uint8_t* tmp_rcv, MIMPI_Op op) {
    if (op == MIMPI_SUM) {
        for (int i = 0; i < count; i++) {
            tmp_calc[i] = tmp_calc[i] + tmp_rcv[i];
        }
    }
    else if (op == MIMPI_PROD) {
        for (int i = 0; i < count; i++) {
            tmp_calc[i] = tmp_calc[i] * tmp_rcv[i];
        }
    }
    else if (op == MIMPI_MIN) {
        for (int i = 0; i < count; i++) {
            tmp_calc[i] = min(tmp_calc[i], tmp_rcv[i]);
        }
    }
    else {
        for (int i = 0; i < count; i++) {
            tmp_calc[i] = max(tmp_calc[i], tmp_rcv[i]);
        }
    }
}

MIMPI_Retcode MIMPI_Reduce(
    void const *send_data,
    void *recv_data,
    int count,
    MIMPI_Op op,
    int root
) {    
    // part 0 - prepare const variables necessary for bcasting
    // part 1 - sending info about readyness, a node after recving that info
    //          from children before sending info to the treewise parent
    //          reduces data from children
    // part 2 - when the root recves reduced info from all children then
    //          everything is synced, so we can let children go and leave

    // 0 part:
    const int MY_RANK = MIMPI_World_rank();
    const int MAPPED_RANK = (MY_RANK - root + n) % n;

    int TREEWISE_PARENT_RANK = -1;
    int MAPPED_TREEWISE_PARENT_RANK = -1;
    if (MAPPED_RANK > 0) {
        MAPPED_TREEWISE_PARENT_RANK = (MAPPED_RANK - 1) / 2;
        TREEWISE_PARENT_RANK = (MAPPED_TREEWISE_PARENT_RANK + root) % n;
    }

    int LEFT_TREEWISE_CHILD_RANK = -1;
    int MAPPED_LEFT_TREEWISE_CHILD_RANK = -1;
    if (MAPPED_RANK * 2 + 1 < n) {
        MAPPED_LEFT_TREEWISE_CHILD_RANK = MAPPED_RANK * 2 + 1;
        
        LEFT_TREEWISE_CHILD_RANK = MAPPED_LEFT_TREEWISE_CHILD_RANK + root;
        LEFT_TREEWISE_CHILD_RANK %= n;
    }

    int RIGHT_TREEWISE_CHILD_RANK = -1;
    int MAPPED_RIGHT_TREEWISE_CHILD_RANK = -1;
    if (MAPPED_RANK * 2 + 2 < n) {
        MAPPED_RIGHT_TREEWISE_CHILD_RANK = MAPPED_RANK * 2 + 2;
        
        RIGHT_TREEWISE_CHILD_RANK = MAPPED_RIGHT_TREEWISE_CHILD_RANK + root;
        RIGHT_TREEWISE_CHILD_RANK %= n;
    }

    int tmp = 0;

    // if the left treewise child doesn't exist then there is no the right one
    // as well
    if (LEFT_TREEWISE_CHILD_RANK == -1) { // no children
        // send data:
        int fd = get_square_channel_write_fd(
            TREEWISE_PARENT_RANK,
            1
        );

        int total_sent = 0;

        while (total_sent < count) {
            int wrote = chsend(fd, send_data + total_sent, count - total_sent);
            
            if (wrote < 0) {
                return MIMPI_ERROR_REMOTE_FINISHED;
            }

            total_sent += wrote;
        }

        fd = get_square_channel_read_fd(
            TREEWISE_PARENT_RANK,
            1
        );

        //printf("%d waitf for the feedback from the parent at%d\n", MY_RANK, fd);
        // recv feedback and be free to leave:
        if (chrecv(fd, &tmp, sizeof(tmp)) != sizeof(tmp)) {
            //printf("%d ferror\n", MY_RANK);
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
        //printf("%d fgot feedback from the parent\n", MY_RANK);

        return MIMPI_SUCCESS;
    }
    
    // there is the left treewise child:
    uint8_t* tmp_rcv = malloc(count);
    uint8_t* tmp_calc = recv_data;
        
    if (MY_RANK != root) {
        tmp_calc = malloc(count);
    }

    memcpy(tmp_calc, send_data, count);

    int left_read_fd = get_square_channel_read_fd(
        LEFT_TREEWISE_CHILD_RANK,
        1
    );

    //printf("%d will wait for the data from the left at %d with count\n", MY_RANK, left_read_fd);
    
    int total_read = 0;
    while (total_read < count) {
        int read = chrecv(
            left_read_fd, 
            tmp_rcv + total_read, 
            count - total_read
        );
        
        if (read <= 0) {
            //printf("%d got the data from the left, BUT ERROR\n", MY_RANK);
            free(tmp_rcv);

            if (MY_RANK != root) {
                free(tmp_calc);
            }

            return MIMPI_ERROR_REMOTE_FINISHED;
        }

        total_read += read;
    }

    //printf("%d got the data from the left\n", MY_RANK);

    // reduce tmp_calc (send_data) and tmp_rcv into tmp_calc:
    reduce(count, tmp_calc, tmp_rcv, op);

    // if there is the right treewise child then recv and reduce its data 
    // as well:
    if (RIGHT_TREEWISE_CHILD_RANK != -1) {
        const int FD = get_square_channel_read_fd(
            RIGHT_TREEWISE_CHILD_RANK,
            1
        );

        //printf("%d wait for the data from the right at %d w/ count\n", MY_RANK, FD);
        
        int total_read = 0;

        while (total_read < count) {
            int read = chrecv(FD, tmp_rcv + total_read, count - total_read);

            if (read <= 0) {
                //printf("%d wait for the data from the right, BUT ERROR\n", MY_RANK);
                free(tmp_rcv);
            
                if (MY_RANK != root) {
                    free(tmp_calc);
                }

                return MIMPI_ERROR_REMOTE_FINISHED;
            }
                
            total_read += read;
        }

        //printf("%d got the data from the right\n", MY_RANK);
        // reduce tmp_calc (send_data) and tmp_rcv into tmp_calc:
        reduce(count, tmp_calc, tmp_rcv, op);
    }

    // send tmp_calc uptree and recv feedback from the treewise parent:
    if (TREEWISE_PARENT_RANK != -1) {
        // send tmp_calc uptree:
        int FD = get_square_channel_write_fd(
            TREEWISE_PARENT_RANK,
            1
        );

        int total_sent = 0;

        while (total_sent < count) {
            int wrote = chsend(FD, tmp_calc + total_sent, count - total_sent);

            if (wrote < 0) {
                free(tmp_rcv);
            
                if (MY_RANK != root) {
                    free(tmp_calc);
                }

                return MIMPI_ERROR_REMOTE_FINISHED;
            }

            total_sent += wrote;
        }

        FD = get_square_channel_read_fd(
            TREEWISE_PARENT_RANK,
            1
        );

        //printf("%d wait for the feedback from the parent at %d\n", MY_RANK, FD);
        // recv feedback from the treewise parent:
        if (chrecv(FD, &tmp, sizeof(tmp)) != sizeof(tmp)) {
            //printf("%d wait got the data from the parent, BUT ERROR\n", MY_RANK);
            free(tmp_rcv);

            if (MY_RANK != root) {
                free(tmp_calc);
            }

            return MIMPI_ERROR_REMOTE_FINISHED;
        }

        //printf("%d wait got the data from the parent\n", MY_RANK);
    }

    // free mem:
    free(tmp_rcv);
    if (MY_RANK != root) {
        free(tmp_calc);
    }

    int fd = get_square_channel_write_fd(
        LEFT_TREEWISE_CHILD_RANK,
        1
    );

    //printf("%d wille send feedback to the left at %d\n", MY_RANK, fd + 1);

    // send feedback to the treewise children:
    if (chsend(fd, &MY_RANK, sizeof(MY_RANK)) != sizeof(MY_RANK)) {
        //printf("%d sende AND ERROR\n", MY_RANK);
        return MIMPI_ERROR_REMOTE_FINISHED;
    }

    //printf("%d sende correctly\n", MY_RANK);

    if (RIGHT_TREEWISE_CHILD_RANK != -1) {        
        int FD = get_square_channel_write_fd(
            RIGHT_TREEWISE_CHILD_RANK,
            1
        );

        //printf("%d will send feedback to the right at %d\n", MY_RANK, READ_FD + 1);
        
        if (chsend(FD, &MY_RANK, sizeof(MY_RANK)) != sizeof(MY_RANK)) {
            return MIMPI_ERROR_REMOTE_FINISHED;
        }
    }

    return MIMPI_SUCCESS;
}