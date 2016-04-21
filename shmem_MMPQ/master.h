
void handler_master_subscribe(void *temp, size_t nbytes, int req_pe, shmemx_am_token_t token);

void handler_master_putpath(void *msg_new, size_t nbytes, int req_pe, shmemx_am_token_t token);

void handler_master_bestpath(void *msg_new, size_t nbytes, int req_pe, shmemx_am_token_t token);
