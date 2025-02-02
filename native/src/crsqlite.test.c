#include "crsqlite.h"
SQLITE_EXTENSION_INIT1

#include "util.h"
#include "consts.h"
#include "tableinfo.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef CHECK_OK
#define CHECK_OK       \
  if (rc != SQLITE_OK) \
  {                    \
    goto fail;         \
  }
#endif

void crsql_close(sqlite3* db);

static void testCreateClockTable()
{
  printf("CreateClockTable\n");

  sqlite3 *db;
  int rc;
  crsql_TableInfo *tc1;
  crsql_TableInfo *tc2;
  crsql_TableInfo *tc3;
  crsql_TableInfo *tc4;
  char *err = 0;

  rc = sqlite3_open(":memory:", &db);
  sqlite3_exec(db, "CREATE TABLE foo (a)", 0, 0, 0);
  sqlite3_exec(db, "CREATE TABLE bar (a primary key)", 0, 0, 0);
  sqlite3_exec(db, "CREATE TABLE baz (a primary key, b)", 0, 0, 0);
  sqlite3_exec(db, "CREATE TABLE boo (a primary key, b, c)", 0, 0, 0);

  rc = crsql_getTableInfo(db, "foo", &tc1, &err);
  CHECK_OK
  rc = crsql_getTableInfo(db, "bar", &tc2, &err);
  CHECK_OK
  rc = crsql_getTableInfo(db, "baz", &tc3, &err);
  CHECK_OK
  rc = crsql_getTableInfo(db, "boo", &tc4, &err);
  CHECK_OK

  rc = crsql_createClockTable(db, tc1, &err);
  CHECK_OK
  rc = crsql_createClockTable(db, tc2, &err);
  CHECK_OK
  rc = crsql_createClockTable(db, tc3, &err);
  CHECK_OK
  rc = crsql_createClockTable(db, tc4, &err);
  CHECK_OK

  crsql_freeTableInfo(tc1);
  crsql_freeTableInfo(tc2);
  crsql_freeTableInfo(tc3);
  crsql_freeTableInfo(tc4);

  // TODO: check that the tables have the expected schema

  printf("\t\e[0;32mSuccess\e[0m\n");
  crsql_close(db);
  return;

fail:
  printf("err: %s %d\n", err, rc);
  sqlite3_free(err);
  crsql_close(db);
  assert(rc == SQLITE_OK);
}

// TODO: add many more cases here.
// 1. Many pk tables
// 2. Only pk tables
// 3. blobs, floats, text, bools, sci notation
// 4. deletes
// 5. pk value changes
static void teste2e()
{
  printf("e2e\n");

  int rc = SQLITE_OK;
  sqlite3 *db;
  sqlite3_stmt *pStmt1;
  sqlite3_stmt *pStmt2;
  char *err = 0;
  rc = sqlite3_open(":memory:", &db);

  rc += sqlite3_exec(db, "create table foo (a primary key, b);", 0, 0, &err);
  rc += sqlite3_exec(db, "select crsql_as_crr('foo');", 0, 0, &err);
  rc += sqlite3_exec(db, "insert into foo values (1, 2.0e2);", 0, 0, &err);

  sqlite3 *db2;
  rc += sqlite3_open(":memory:", &db2);

  rc += sqlite3_prepare_v2(db, "SELECT * FROM crsql_changes", -1, &pStmt1, 0);
  rc += sqlite3_prepare_v2(db, "INSERT INTO crsql_changes VALUES (?, ?, ?, ?, ?, ?)", -1, &pStmt2, 0);
  CHECK_OK

  while (sqlite3_step(pStmt1) == SQLITE_ROW) {
    for (int i = 0; i < 6; ++i) {
      sqlite3_bind_value(pStmt2, i + 1, sqlite3_column_value(pStmt1, i));
    }

    sqlite3_step(pStmt2);
    sqlite3_reset(pStmt2);
  }
  sqlite3_finalize(pStmt1);
  sqlite3_finalize(pStmt2);

  rc += sqlite3_prepare_v2(db, "SELECT * FROM foo", -1, &pStmt1, 0);
  rc += sqlite3_prepare_v2(db, "SELECT * FROM foo", -1, &pStmt2, 0);
  CHECK_OK

  int didCompare = 0;
  while (sqlite3_step(pStmt1) == SQLITE_ROW) {
    rc = sqlite3_step(pStmt2);
    assert(rc == SQLITE_ROW);

    assert(sqlite3_column_int(pStmt1, 0) == sqlite3_column_int(pStmt2, 0));
    assert(sqlite3_column_double(pStmt1, 1) == sqlite3_column_double(pStmt2, 1));

    didCompare = 1;
  }
  sqlite3_finalize(pStmt1);
  sqlite3_finalize(pStmt2);

  assert(didCompare == 1);

  crsql_close(db);
  crsql_close(db2);
  printf("\t\e[0;32mSuccess\e[0m\n");
  return;

fail:
  printf("err: %s %d\n", err, rc);
  sqlite3_free(err);
  sqlite3_close(db);
  assert(rc == SQLITE_OK);
}

void testModifySinglePK() {

}

void testModifyCompoundPK() {

}

void crsqlTestSuite()
{
  printf("\e[47m\e[1;30mSuite: crsql\e[0m\n");

  testCreateClockTable();
  // testSyncBit();
  teste2e();
  // testIdempotence();
  // testColumnAdds();
  // testColumnDrops();
  // testRecreateCrrFromExisting();
  // testRequiredPrimaryKey();
  // testSyncBit();
  // testDbVersion();
  // testSiteId();
  // test all the new logic around perDbData
  // getting, freeing, reusing, releasing, refcounting, etc.
}