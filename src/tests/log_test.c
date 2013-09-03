#include <assert.h>
#include <log.h>


int main(int argc, char *argv[])
{
    struct log *l;
    uint8_t flags;
    
    flags = LOG_PRINT_DATE | 
            LOG_PRINT_TIMESTAMP | 
            LOG_PRINT_PID | 
            LOG_PRINT_NAME | 
            LOG_PRINT_HOSTNAME | 
            LOG_PRINT_LEVEL;
                
    
    l = log_new("/tmp/test.log", "MyLogFile", flags, LOG_INFO);
    assert(l);
    
    log_error(l, "error message\n");
    log_critical(l, "critical message\n");
    log_warning(l, "warning message\n");
    log_message(l, "general message\n");
    log_info(l, "info message\n");
    log_debug(l, "debug message\n");
    
    log_delete(l);
    
    return 0;
}