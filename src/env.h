#pragma once

#include "../liblmdb/lmdb.h"

class EnvWrap : public node::ObjectWrap
{
private:
	// The wrapped object
	MDB_env *env;

	friend class TxnWrap;
	friend class DbiWrap;

public:
	EnvWrap();
	~EnvWrap();

	// Sets up exports for the Env constructor
	static void setupExports(v8::Handle<v8::Object> exports);

	/*
	Constructor of the database environment. You need to `open()` it before you can use it.
	(Wrapper for `mdb_env_create`)
	*/
	static NAN_METHOD(ctor);

	/*
	Opens the database environment with the specified options. The options will be used to configure the environment before opening it.
	(Wrapper for `mdb_env_open`)

	Parameters:

	* Options object that contains possible configuration options.

	Possible options are:

	* maxDbs: the maximum number of named databases you can have in the environment (default is 1)
	* maxReaders: the maximum number of concurrent readers of the environment (default is 126)
	* mapSize: maximal size of the memory map (the full environment) in bytes (default is 10485760 bytes)
	* path: path to the database environment
	*/
	static NAN_METHOD(open);

	/*
	Closes the database environment.
	(Wrapper for `mdb_env_close`)
	*/
	static NAN_METHOD(close);

	/*
	Starts a new transaction in the environment.
	(Wrapper for `mdb_txn_begin`)

	Parameters:

	* Options object that contains possible configuration options.

	Possible options are:

	* readOnly: if true, the transaction is read-only
	*/
	static NAN_METHOD(beginTxn);

	/*
	Opens a database in the environment.
	(Wrapper for `mdb_dbi_open`)

	Parameters:

	* Options object that contains possible configuration options.

	Possible options are:

	* create: if true, the database will be created if it doesn't exist
	* keyIsUint32: if true, keys are treated as 32-bit unsigned integers
	* dupSort: if true, the database can hold multiple items with the same key
	* reverseKey: keys are strings to be compared in reverse order
	* dupFixed: if dupSort is true, indicates that the data items are all the same size
	* integerDup: duplicate data items are also integers, and should be sorted as such
	* reverseDup: duplicate data items should be compared as strings in reverse order
	*/
	static NAN_METHOD(openDbi);

	/*
	Flushes all data to the disk asynchronously.
	(Asynchronous wrapper for `mdb_env_sync`)

	Parameters:

	* Callback to be executed after the sync is complete.
	*/
	static NAN_METHOD(sync);
};