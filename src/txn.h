#pragma once

#include "../liblmdb/lmdb.h"

/*
`Txn`
Represents a transaction running on a database environment.
(Wrapper for `MDB_txn`)
*/
class TxnWrap : public node::ObjectWrap {
private:
	// The wrapped object
	MDB_txn *txn;
	// Reference to the MDB_env of the wrapped MDB_txn
	MDB_env *env;

	friend class CursorWrap;

public:
	TxnWrap(MDB_env *env, MDB_txn *txn);
	~TxnWrap();

	// Sets up exports for the Env constructor
	static void setupExports(v8::Handle<v8::Object> exports);

	// Constructor (not exposed)
	static NAN_METHOD(ctor);

	// Helper for all the get methods (not exposed)
	//static NAN_METHOD(getCommon(const Arguments &args, Handle<Value>(*successFunc)(MDB_val&));

	// Helper for all the put methods (not exposed)
	//static NAN_METHOD(putCommon(const Arguments &args, void(*fillFunc)(const Arguments&, MDB_val&), void(*freeFunc)(MDB_val&));

	/*
	Commits the transaction.
	(Wrapper for `mdb_txn_commit`)
	*/
	static NAN_METHOD(commit);

	/*
	Aborts the transaction.
	(Wrapper for `mdb_txn_abort`)
	*/
	static NAN_METHOD(abort);

	/*
	Aborts a read-only transaction but makes it renewable with `renew`.
	(Wrapper for `mdb_txn_reset`)
	*/
	static NAN_METHOD(reset);

	/*
	Renews a read-only transaction after it has been reset.
	(Wrapper for `mdb_txn_renew`)
	*/
	static NAN_METHOD(renew);

	/*
	Gets binary data (Node.js Buffer) associated with the given key from a database. You need to open a database in the environment to use this.
	This method is zero-copy and the return value can only be used until the next put operation or until the transaction is committed or aborted.
	(Wrapper for `mdb_get`)

	Parameters:

	* database instance created with calling `openDbi()` on an `Env` instance
	* key for which the value is retrieved
	*/
	static NAN_METHOD(get);

	/*
	Puts binary data (Node.js Buffer) into a database.
	(Wrapper for `mdb_put`)

	Parameters:

	* database instance created with calling `openDbi()` on an `Env` instance
	* key for which the value is stored
	* data to store for the given key
	*/
	static NAN_METHOD(put);

	/*
	Deletes data with the given key from the database.
	(Wrapper for `mdb_del`)

	* database instance created with calling `openDbi()` on an `Env` instance
	* key for which the value is stored
	*/
	static NAN_METHOD(del);
};