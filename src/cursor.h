#pragma once

#include "../liblmdb/lmdb.h"

/*
`Cursor`
Represents a cursor instance that is assigned to a transaction and a database instance
(Wrapper for `MDB_cursor`)
*/
class CursorWrap : public node::ObjectWrap {
private:
	// The wrapped object
	MDB_cursor *cursor;
	// Stores whether keys should be treated as uint32_t
	bool keyIsUint32;
	// Key/data pair where the cursor is at
	MDB_val key, data;

public:
	CursorWrap(MDB_cursor *cursor);
	~CursorWrap();

	// Sets up exports for the Cursor constructor
	static void setupExports(v8::Handle<v8::Object> exports);

	/*
	Opens a new cursor for the specified transaction and database instance.
	(Wrapper for `mdb_cursor_open`)

	Parameters:

	* Transaction object
	* Database instance object
	*/
	static NAN_METHOD(ctor);

	/*
	Closes the cursor.
	(Wrapper for `mdb_cursor_close`)

	Parameters:

	* Transaction object
	* Database instance object
	*/
	static NAN_METHOD(close);

	/*
	Gets the current key-data pair that the cursor is pointing to. Returns the current key.
	(Wrapper for `mdb_cursor_get`)

	Parameters:

	* Callback that accepts the key and value
	*/
	NAN_METHOD(getCurrent);

	
	/*
	Asks the cursor to go to the first key-data pair in the database.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToFirst);

	/*
	Asks the cursor to go to the last key-data pair in the database.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToLast);

	/*
	Asks the cursor to go to the next key-data pair in the database.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToNext);

	/*
	Asks the cursor to go to the previous key-data pair in the database.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToPrev);

	/*
	Asks the cursor to go to the specified key in the database.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToKey);

	/*
	Asks the cursor to go to the first key greater than or equal to the specified parameter in the database.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToRange);

	/*
	For databases with the dupSort option. Asks the cursor to go to the first occurence of the current key.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToFirstDup);

	/*
	For databases with the dupSort option. Asks the cursor to go to the last occurence of the current key.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToLastDup);

	/*
	For databases with the dupSort option. Asks the cursor to go to the next occurence of the current key.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToNextDup);

	/*
	For databases with the dupSort option. Asks the cursor to go to the previous occurence of the current key.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToPrevDup);

	/*
	For databases with the dupSort option. Asks the cursor to go to the specified key/data pair.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToDup);

	/*
	For databases with the dupSort option. Asks the cursor to go to the specified key with the first data that is greater than or equal to the specified.
	(Wrapper for `mdb_cursor_get`)
	*/
	NAN_METHOD(goToDupRange);

	/*
	Deletes the key/data pair to which the cursor refers.
	(Wrapper for `mdb_cursor_del`)
	*/
	NAN_METHOD(del);
};