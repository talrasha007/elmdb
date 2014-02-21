#include <nan.h>

#include "elmdb.h"
#include "dbi.h"
#include "txn.h"
#include "cursor.h"

using namespace v8;
using namespace node;

CursorWrap::CursorWrap(MDB_cursor *cursor) {
	this->cursor = cursor;
}

CursorWrap::~CursorWrap() {
	if (this->cursor) {
		mdb_cursor_close(this->cursor);
	}
}

NAN_METHOD(CursorWrap::ctor) {
	NanScope();

	// Get arguments
	TxnWrap *tw = ObjectWrap::Unwrap<TxnWrap>(args[0]->ToObject());
	DbiWrap *dw = ObjectWrap::Unwrap<DbiWrap>(args[1]->ToObject());

	// Open the cursor
	MDB_cursor *cursor;
	int rc = mdb_cursor_open(tw->txn, dw->dbi, &cursor);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	// Create wrapper
	CursorWrap* cw = new CursorWrap(cursor);
	cw->keyIsUint32 = dw->keyIsUint32;
	cw->Wrap(args.This());

	NanReturnValue(args.This());
}

NAN_METHOD(CursorWrap::close) {
	NanScope();

	CursorWrap *cw = ObjectWrap::Unwrap<CursorWrap>(args.This());
	mdb_cursor_close(cw->cursor);
	cw->cursor = NULL;
	
	NanReturnUndefined();
}

NAN_METHOD(CursorWrap::del) {
	NanScope();

	CursorWrap *cw = ObjectWrap::Unwrap<CursorWrap>(args.This());
	// TODO: wrap MDB_NODUPDATA flag

	int rc = mdb_cursor_del(cw->cursor, 0);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

template <MDB_cursor_op OP> NAN_METHOD(CursorWrap::getCommon) {
	NanScope();

	CursorWrap *cw = ObjectWrap::Unwrap<CursorWrap>(args.This());
	MDB_val key = { 0 }, data = { 0 };

	switch (OP)
	{
	case MDB_GET_BOTH:
	case MDB_GET_BOTH_RANGE:
		key.mv_data = Buffer::Data(args[0]);
		key.mv_size = Buffer::Length(args[0]);
		data.mv_data = Buffer::Data(args[1]);
		data.mv_size = Buffer::Length(args[1]);
		break;
	case MDB_SET:
	case MDB_SET_KEY:
	case MDB_SET_RANGE:
		key.mv_data = Buffer::Data(args[0]);
		key.mv_size = Buffer::Length(args[0]);
		break;
	default:
		break;
	}

	void* oriKey = key.mv_data;
	int rc = mdb_cursor_get(cw->cursor, &key, &data, OP);

	if (rc == MDB_NOTFOUND) {
		NanReturnNull();
	}
	else if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	Local<Array> ret = Array::New(2);

	// If key is not returned by lmdb, dup it.
	if (oriKey != key.mv_data) ret->Set(0, NanNewBufferHandle((char*)key.mv_data, key.mv_size, freeNothing, NULL));
	else ret->Set(0, NanNewBufferHandle((char*)key.mv_data, key.mv_size));

	ret->Set(1, NanNewBufferHandle((char*)data.mv_data, data.mv_size, freeNothing, NULL));
	
	NanReturnValue(ret);
}

NAN_METHOD(CursorWrap::get) { return getCommon<MDB_GET_CURRENT>(args); }
NAN_METHOD(CursorWrap::goToFirst) { return getCommon<MDB_FIRST>(args); }
NAN_METHOD(CursorWrap::goToLast) { return getCommon<MDB_LAST>(args); }
NAN_METHOD(CursorWrap::goToNext) { return getCommon<MDB_NEXT>(args); }
NAN_METHOD(CursorWrap::goToPrev) { return getCommon<MDB_PREV>(args); }
NAN_METHOD(CursorWrap::goToFirstDup) { return getCommon<MDB_FIRST_DUP>(args); }
NAN_METHOD(CursorWrap::goToLastDup) { return getCommon<MDB_LAST_DUP>(args); }
NAN_METHOD(CursorWrap::goToNextDup) { return getCommon<MDB_NEXT_DUP>(args); }
NAN_METHOD(CursorWrap::goToPrevDup) { return getCommon<MDB_PREV_DUP>(args); }
NAN_METHOD(CursorWrap::goToKey) { return getCommon<MDB_SET>(args); }
NAN_METHOD(CursorWrap::goToRange) { return getCommon<MDB_SET_RANGE>(args); }
NAN_METHOD(CursorWrap::goToDup) { return getCommon<MDB_GET_BOTH>(args); }
NAN_METHOD(CursorWrap::goToDupRange) { return getCommon<MDB_GET_BOTH_RANGE>(args); }

void CursorWrap::setupExports(Handle<Object> exports) {
	// CursorWrap: Prepare constructor template
	Local<FunctionTemplate> cursorTpl = FunctionTemplate::New(CursorWrap::ctor);
	cursorTpl->SetClassName(String::NewSymbol("Cursor"));
	cursorTpl->InstanceTemplate()->SetInternalFieldCount(1);
	// CursorWrap: Add functions to the prototype
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "close", CursorWrap::close);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "getCurrentString", CursorWrap::get);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToFirst", CursorWrap::goToFirst);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToLast", CursorWrap::goToLast);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToNext", CursorWrap::goToNext);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToPrev", CursorWrap::goToPrev);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToKey", CursorWrap::goToKey);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToRange", CursorWrap::goToRange);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToFirstDup", CursorWrap::goToFirstDup);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToLastDup", CursorWrap::goToLastDup);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToNextDup", CursorWrap::goToNextDup);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToPrevDup", CursorWrap::goToPrevDup);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToDup", CursorWrap::goToDup);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "goToDupRange", CursorWrap::goToDupRange);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "del", CursorWrap::del);

	// Set exports
	exports->Set(String::NewSymbol("Cursor"), cursorTpl->GetFunction());
}

