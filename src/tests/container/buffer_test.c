
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <buffer.h>
#include <macro.h>

int main(int argc, char *argv[])
{
    struct buffer buf;
    int err;
    
    err = buffer_init(&buf, 1);
    assert(err == 0);
    
    while(argc--) {
        err = buffer_prepare_write(&buf, strlen(argv[argc]) + sizeof(char));
        assert(err == 0);
        
        buffer_write(&buf, argv[argc], strlen(argv[argc]));
        buffer_write_char(&buf, '\n');
    }
    
    //buffer_remove_unaccessed(&buf);
    
    while(buffer_bytes_accessible(&buf) > 0)
        fprintf(stdout, "%c", buffer_read_char(&buf));
    
    buffer_clear_accessed(&buf);
    
    while(buffer_bytes_accessible(&buf) > 0)
        fprintf(stdout, "%c", buffer_read_char(&buf));
    
    buffer_destroy(&buf);
        
    return EXIT_SUCCESS;
}