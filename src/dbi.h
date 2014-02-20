#pragma once

#include "../liblmdb/lmdb.h"

/*
`Dbi`
Represents a database instance in an environment.
(Wrapper for `MDB_dbi`)
*/
class DbiWrap : public node::ObjectWrap {
private:
	// Stores whether or not the MDB_dbi needs closing
	bool needsClose;
	// Stores whether keys should be treated as uint32_t
	bool keyIsUint32;
	// The wrapped object
	MDB_dbi dbi;
	// Reference to the MDB_env of the wrapped MDB_dbi
	MDB_env *env;

	friend class TxnWrap;
	friend class CursorWrap;

public:
	DbiWrap(MDB_env *env, MDB_dbi dbi);
	~DbiWrap();

	// Sets up exports for the Env constructor
	static void setupExports(v8::Handle<v8::Object> exports);

	// Constructor (not exposed)
	static NAN_METHOD(ctor);

	/*
	Closes the database instance.
	Wrapper for `mdb_dbi_close`)
	*/
	static NAN_METHOD(close);

	/*
	Drops the database instance, either deleting it completely (default) or just freeing its pages.

	Parameters:

	* Options object that contains possible configuration options.

	Possible options are:

	* justFreePages - indicates that the database pages need to be freed but the database shouldn't be deleted

	*/
	static NAN_METHOD(drop);
};
