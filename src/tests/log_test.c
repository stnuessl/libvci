#include <assert.h>
#include <log.h>


int main(int argc, char *argv[])
{
    struct log *l1, *l2;
    uint8_t flags;
    
    flags = LOG_PRINT_DATE | 
            LOG_PRINT_TIMESTAMP | 
            LOG_PRINT_PID | 
            LOG_PRINT_NAME | 
            LOG_PRINT_HOSTNAME | 
            LOG_PRINT_LEVEL;
                
    /* open dummy */
    l1 = log_new("/dev/null", "MyComponent", flags);
    l2 = log_new("/dev/null", "MyOtherComponent", flags);

    assert(l1);
    assert(l2);
    
    log_set_severity_cap(l1, LOG_SEVERITY_INFO);
    log_set_severity_cap(l2, LOG_SEVERITY_INFO);
    
    /* set file now */
    log_set_file(l1, stdout);
    log_set_file(l2, stdout);
    
    log_error(l1, "error message\n");
    log_critical(l1, "critical message\n");
    log_warning(l1, "warning message\n");
    log_message(l2, "general message\n");
    log_debug(l2, "debug message\n");
    log_info(l2, "info message\n");
    
    log_delete(l1);
    log_delete(l2);
    
    return 0;
}