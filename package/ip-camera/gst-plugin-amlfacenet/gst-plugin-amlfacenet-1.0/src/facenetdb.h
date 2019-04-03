#ifndef _FACENETDB_H_
#define _FACENETDB_H_

void *db_init (const char *dbfile);
void db_deinit (void *db);
void db_set_threshold (float t);
int db_search_result (void *db, const float faceid[128],
    const char* faceimg, int width, int height,
    const char *format, char *buf, size_t bufsize);

#endif /* _FACENETDB_H_ */
