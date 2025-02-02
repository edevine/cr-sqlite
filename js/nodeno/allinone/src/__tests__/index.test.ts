import * as nanoid from "nanoid";
import sqlite from "../index.js";
import { test, expect } from "vitest";

// test that we wrapped correctly
test("failing example", () => {
  const db1 = sqlite.open(":memory:");

  db1.execMany([
    `CREATE TABLE IF NOT EXISTS todo_list ("name" primary key, "creation_time");`,
    `CREATE TABLE IF NOT EXISTS todo ("id" primary key, "list", "text", "complete");`,
  ]);
  db1.execMany([
    `SELECT crsql_as_crr('todo_list');`,
    `SELECT crsql_as_crr('todo');`,
  ]);

  let list = [
    "milk",
    "potatos",
    "avocado",
    "butter",
    "cheese",
    "broccoli",
    "spinach",
  ];
  // `insert or ignore` given this is a notebook and ppl will re-run cells.
  db1.exec(`INSERT OR IGNORE INTO todo_list VALUES ('groceries', ?)`, [
    Date.now(),
  ]);
  list.forEach((item) =>
    db1.exec(`INSERT INTO todo VALUES (?, 'groceries', ?, 0)`, [
      nanoid.nanoid(),
      item,
    ])
  );

  list = ["test", "document", "explain", "onboard", "hire"];
  db1.exec(`INSERT OR IGNORE INTO todo_list VALUES ('work', ?)`, [Date.now()]);
  list.forEach((item) =>
    db1.exec(`INSERT INTO todo VALUES (?, 'work', ?, 0)`, [
      nanoid.nanoid(),
      item,
    ])
  );

  let groceries = db1.execO(
    `SELECT "list", "text" FROM "todo" WHERE "list" = 'groceries'`
  );
  let work = db1.execO(
    `SELECT "list", "text" FROM "todo" WHERE "list" = 'work'`
  );

  let changesets = db1.execA("SELECT * FROM crsql_changes where version > -1");

  const db2 = sqlite.open(":memory:");
  db2.execMany([
    `CREATE TABLE IF NOT EXISTS todo_list ("name" primary key, "creation_time");`,
    `CREATE TABLE IF NOT EXISTS todo ("id" primary key, "list", "text", "complete");`,
    `SELECT crsql_as_crr('todo_list');`,
    `SELECT crsql_as_crr('todo');`,
  ]);

  const siteid = db1.execA(`SELECT crsql_siteid()`)[0][0];
  db2.transaction(() => {
    for (const cs of changesets) {
      db2.exec(`INSERT INTO crsql_changes VALUES (?, ?, ?, ?, ?, ?)`, cs);
    }
  });

  groceries = db2.execO(
    `SELECT "list", "text" FROM "todo" WHERE "list" = 'groceries'`
  );
  work = db2.execO(`SELECT "list", "text" FROM "todo" WHERE "list" = 'work'`);

  let db1version = db1.execA(`SELECT crsql_dbversion()`)[0][0];
  let db2version = db2.execA(`SELECT crsql_dbversion()`)[0][0];

  db1.exec(`INSERT OR IGNORE INTO todo_list VALUES (?, ?)`, [
    "home",
    Date.now(),
  ]);
  db2.exec(`INSERT OR IGNORE INTO todo_list VALUES (?, ?)`, [
    "home",
    Date.now(),
  ]);
  db1.exec(`INSERT INTO todo VALUES (?, ?, ?, ?)`, [
    nanoid.nanoid(),
    "home",
    "paint",
    0,
  ]);
  db2.exec(`INSERT INTO todo VALUES (?, ?, ?, ?)`, [
    nanoid.nanoid(),
    "home",
    "mow",
    0,
  ]);
  db1.exec(`INSERT INTO todo VALUES (?, ?, ?, ?)`, [
    nanoid.nanoid(),
    "home",
    "water",
    0,
  ]);
  // given each item is a nanoid for primary key, `weed` will show up twice
  db2.exec(`INSERT INTO todo VALUES (?, ?, ?, ?)`, [
    nanoid.nanoid(),
    "home",
    "weed",
    0,
  ]);
  db1.exec(`INSERT INTO todo VALUES (?, ?, ?, ?)`, [
    nanoid.nanoid(),
    "home",
    "weed",
    0,
  ]);
  // and complete things on other lists
  db1.exec(`UPDATE todo SET complete = 1 WHERE list = 'groceries'`);

  let changesets1 = db1.execA("SELECT * FROM crsql_changes where version > ?", [
    db1version,
  ]);
  let changesets2 = db2.execA("SELECT * FROM crsql_changes where version > ?", [
    db2version,
  ]);
});
