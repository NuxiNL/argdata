// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <program.h>

#include <argdata.hpp>
#include <cstdlib>
#include <memory>
#include <optional>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <leveldb/db.h>
#include <leveldb/env.h>

// Inserts key-value pairs into a LevelDB by reading them from a
// terminal. After detecting an end-of-file, it prints all of the
// key-value pairs in the LevelDB to the terminal.
//
// Usage:
//
//     $ x86_64-unknown-cloudabi-c++ -std=c++1z -O2 
//           -o edit-leveldb edit-leveldb.cc \
//           -lleveldb -lsnappy -lboost_iostreams
//     $ cat edit-leveldb.yaml
//     %TAG ! tag:nuxi.nl,2015:cloudabi/
//     ---
//     database: !file
//       path: db
//       terminal: !fd stdout
//     $ mkdir db
//     $ cloudabi-run edit-leveldb < edit-leveldb.yaml

static bool edit_leveldb(int fd_database, int fd_terminal) {
  // Use Boost iostreams to access the terminal. Standard C++ iostreams
  // cannot be constructed from file POSIX file descriptors.
  boost::iostreams::stream<boost::iostreams::file_descriptor> terminal(
      fd_terminal, boost::iostreams::never_close_handle);

  // Open the LevelDB, using an environment with a custom base directory
  // file descriptor.
  std::unique_ptr<leveldb::Env> env(
      leveldb::Env::DefaultWithDirectory(fd_database));
  std::unique_ptr<leveldb::DB> database;
  {
    leveldb::Options options;
    options.create_if_missing = true;
    options.env = env.get();
    leveldb::DB *database_ptr;
    leveldb::Status status = leveldb::DB::Open(options, ".", &database_ptr);
    if (!status.ok()) {
      terminal << "Got error: " << status.ToString() << std::endl;
      return false;
    }
    database.reset(database_ptr);
  }

  // Read key-value pairs from the terminal and insert them into the LevelDB.
  for (;;) {
    terminal << "  Key: ";
    std::string key;
    std::getline(terminal, key);
    if (!terminal.good())
      break;

    terminal << "Value: ";
    std::string value;
    std::getline(terminal, value);
    if (!terminal.good())
      break;

    database->Put(leveldb::WriteOptions(), key, value);
  }
  terminal.clear();
  terminal << std::endl;

  // Print all of the key-value pairs to the terminal.
  std::unique_ptr<leveldb::Iterator> it(
      database->NewIterator(leveldb::ReadOptions()));
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    terminal << '|' << it->key().ToString() << "| -> |"
             << it->value().ToString() << '|' << std::endl;
  }
  return true;
}

void program_main(const argdata_t *ad) {
  // Extract file descriptors from fields provided by cloudabi-run.
  std::optional<int> fd_database, fd_terminal;
  for (auto i : ad->as_map()) {
    auto key = i.first->as_str();
    if (key == "database")
      fd_database = i.second->get_fd();
    if (key == "terminal")
      fd_terminal = i.second->get_fd();
  }

  std::exit(fd_database && fd_terminal &&
                    edit_leveldb(*fd_database, *fd_terminal)
                ? 0
                : 1);
}
