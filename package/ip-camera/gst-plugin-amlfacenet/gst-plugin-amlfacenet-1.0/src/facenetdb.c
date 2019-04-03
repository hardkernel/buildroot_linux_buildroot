#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sqlite3.h>
#include "jpeg.h"

static float gs_threshold = 0.6;

static float
calcdiff(const float ldat[128], const float rdat[128]) {
  int i;
  float diff, sum = 0.0;
  for (i = 0; i < 128; i++) {
    diff = ldat[i] - rdat[i];
    sum += diff * diff;
  }
  return sqrtf(sum);
}

static void
calculateDiff(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
  assert (2 == argc);
  const void *ldat = sqlite3_value_blob (argv[0]);
  const void *rdat = sqlite3_value_blob (argv[1]);
  double diff = calcdiff ((const float*)ldat, (const float *)rdat);
  sqlite3_result_double (ctx, diff);
}

static void
_insert_data (sqlite3 *db, float *faceid, char* faceimg, int width, int height) {
  sqlite3_stmt* stmt = NULL;

  sqlite3_prepare (db,
      "CREATE TABLE IF NOT EXISTS `faceinfo` ("
      "`index` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
      "`uid`   INTEGER NOT NULL,"
      "`faceid`    BLOB NOT NULL,"
      "`faceimg`   BLOB NOT NULL"
      ");", -1, &stmt, NULL);
  sqlite3_step (stmt);
  sqlite3_finalize (stmt);
  stmt = NULL;

  sqlite3_prepare (db,
      "CREATE TABLE IF NOT EXISTS `userinfo` ("
      "`uid`   INTEGER NOT NULL PRIMARY KEY UNIQUE,"
      "`name`  TEXT NOT NULL"
      ");", -1, &stmt, NULL);
  sqlite3_step (stmt);
  sqlite3_finalize (stmt);
  stmt = NULL;

  char *jpg = NULL;
  unsigned long szjpg =
    save_jpeg (faceimg, width, height, 80, &jpg);

  if (szjpg == 0) { return; }

  sqlite3_prepare (db,
      "insert into faceinfo (`uid`, `faceid`, `faceimg`) values (0, ?, ?)",
      -1, &stmt, NULL);
  sqlite3_bind_blob (stmt, 1, (void *)faceid, sizeof(float) * 128, NULL);
  sqlite3_bind_blob (stmt, 2, (void *)jpg, szjpg, NULL);
  sqlite3_step (stmt);
  sqlite3_finalize (stmt);
  stmt = NULL;
  free (jpg);
}

void *db_init (const char *dbfile) {
  sqlite3 *db;
  int rc = sqlite3_open (dbfile, &db);
  if (rc != SQLITE_OK) {
    fprintf (stderr, "sqlite3_open %s. %s\n", dbfile, sqlite3_errmsg(db));
    return NULL;
  }
  rc = sqlite3_create_function_v2(db, "CalcDiff", 2, SQLITE_UTF8,
      NULL, calculateDiff, NULL, NULL, NULL);
  if (rc != SQLITE_OK) {
    fprintf (stderr, "sqlite3_create_function_v2 -> calcdiff. %s\n", sqlite3_errmsg(db));
    goto on_error;
  }

  return (void*)db;
on_error:
  if (db) {
    sqlite3_close (db);
    db = NULL;
  }
  return (void *)db;
}

void db_deinit (void *db) {
  if (db) {
    sqlite3_close ((sqlite3 *)db);
  }
}

void db_set_threshold (float t) {
  gs_threshold = t;
}

int db_search_result (void *db, const float faceid[128],
    const char* faceimg, int width, int height,
    const char *format, char *buf, size_t bufsize) {
  if (db == NULL) return -1;

  int ret = -1;
  int rc;
  int uid = -1;
  double face_diff = 0.0;

  sqlite3 *sdb = (sqlite3 *)db;
  sqlite3_stmt* stmt = NULL;

  if (buf) buf[0] = '\0';

  // find the matched user id
  rc = sqlite3_prepare (sdb,
      "SELECT `index`, `uid`, CalcDiff(`faceid`, ?) FROM faceinfo WHERE `uid` > 0",
      -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf (stderr, "sqlite3_prepare. %s\n", sqlite3_errmsg(sdb));
    goto on_search_end;
  }
  rc = sqlite3_bind_blob (stmt, 1, (void *)faceid, sizeof(float) * 128, NULL);
  if (rc != SQLITE_OK) {
    fprintf (stderr, "sqlite3_bind_blob. %s\n", sqlite3_errmsg(sdb));
    goto on_search_end;
  }

  int index;
  while (sqlite3_step (stmt) == SQLITE_ROW) {
    int idx = sqlite3_column_int (stmt, 0);
    int id = sqlite3_column_int (stmt, 1);
    double diff = sqlite3_column_double (stmt, 2);
    if (diff > gs_threshold) continue;
    if (uid == -1) {
      index = idx;
      uid = id;
      face_diff = diff;
    } else {
      if (diff < face_diff) {
        index = idx;
        uid = id;
        face_diff = diff;
      }
    }
  }

  sqlite3_finalize (stmt);
  stmt = NULL;

  if (uid == -1) {
    goto on_search_end;
  }

  // get the detail info
  char sql[256] = {0};
  snprintf (sql, sizeof(sql), "SELECT %s FROM userinfo WHERE uid=?", format);
  rc = sqlite3_prepare (sdb,
      sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf (stderr, "sqlite3_prepare. %s\n", sqlite3_errmsg(sdb));
    goto on_search_end;
  }
  rc = sqlite3_bind_int (stmt, 1, uid);
  if (rc != SQLITE_OK) {
    fprintf (stderr, "sqlite3_bind_int. %s\n", sqlite3_errmsg(sdb));
    goto on_search_end;
  }

  rc = sqlite3_step (stmt);
  if (rc == SQLITE_ROW) {
    int count = sqlite3_column_count (stmt);
    int i;
    for (i = 0; i < count; i++) {
      int type = sqlite3_column_type (stmt, i);
      const char *name = sqlite3_column_name (stmt, i);
      char vbuf[256] = {0};
      switch (type) {
        case SQLITE_TEXT:
          {
            const unsigned char *t =
              sqlite3_column_text (stmt, i);
            snprintf (vbuf, sizeof(vbuf), "%s: %s\n", name, t);
          }
          break;
        case SQLITE_INTEGER:
          {
            int v =
              sqlite3_column_int (stmt, i);
            snprintf (vbuf, sizeof(vbuf), "%s: %d\n", name, v);
          }
          break;
        case SQLITE_FLOAT:
          {
            double v =
              sqlite3_column_double (stmt, i);
            snprintf (vbuf, sizeof(vbuf), "%s: %f\n", name, v);
          }
          break;
        default:
          break;
      }
      if (bufsize > 0) {
        size_t vbuf_len = strlen(vbuf);
        strncat (buf, vbuf, bufsize > vbuf_len ? vbuf_len : bufsize);
        bufsize -= vbuf_len;
      } else {
        break;
      }
    }
    ret = 0;

  } else {
    goto on_search_end;
  }

on_search_end:
  if (stmt) {
    sqlite3_finalize (stmt);
  }
  if (ret < 0 && faceimg != NULL) {
    _insert_data (sdb, faceid, faceimg, width, height);
  }
  return ret;
}

