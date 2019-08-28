#ifndef FILE_READ_H
# define FILE_READ_H

void	*get_file_contents(const char *path, uint32_t *length);
void	put_file_contents(const char *path, const void *content, uint32_t length);
void	load_saved_external_ram(const char *path);
void	save_external_ram(void);

#endif
