#ifndef _CONFIG_H_
#define _CONFIG_H_


typedef void config_t;

config_t *config_open(const char *__restrict path);

char *config_read(config_t *__restrict config, const char *__restrict key);

void config_close(config_t *__restrict config);

#endif /* _CONFIG_H_ */