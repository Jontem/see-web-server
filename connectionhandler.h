#ifndef CONNECTIONHANDLER_HEADER_FILE
#define CONNECTIONHANDLER_HEADER_FILE

void connection_worker(int worker_id);
void create_workers(int num_workers);
void signal_connection();

#endif