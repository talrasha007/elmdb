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
	Flushes all data to the disk asynchronously.
	(Asynchronous wrapper for `mdb_env_sync`)

	Parameters:

	* Callback to be executed after the sync is complete.
	*/
	static NAN_METHOD(sync);
};