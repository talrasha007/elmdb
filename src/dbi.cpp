#include <nan.h>

#include "env.h"
#include "dbi.h"

using namespace v8;
using namespace node;

DbiWrap::DbiWrap(MDB_env *env, MDB_dbi dbi) {
	this->needsClose = false;
	this->env = env;
	this->dbi = dbi;
}

DbiWrap::~DbiWrap() {
	// Close if not closed already
	if (needsClose) {
		mdb_dbi_close(env, dbi);
	}
}

NAN_METHOD(DbiWrap::ctor) {
	NanScope();

	MDB_dbi dbi;
	MDB_txn *txn;
	int rc;
	int flags = 0;
	int keyIsUint32 = 0;
	char *cname = NULL;

	EnvWrap *ew = ObjectWrap::Unwrap<EnvWrap>(args[0]->ToObject());
	if (args[1]->IsObject()) {
		Local<Object> options = args[1]->ToObject();
		Local<String> name = options->Get(String::NewSymbol("name"))->ToString();

		size_t l;
		cname = NanCString(name, &l);

		// Get flags from options

		// NOTE: mdb_set_relfunc is not exposed because MDB_FIXEDMAP is "highly experimental"
		// NOTE: mdb_set_relctx is not exposed because MDB_FIXEDMAP is "highly experimental"
		//setFlagFromValue(&flags, MDB_REVERSEKEY, "reverseKey", false, options);
		//setFlagFromValue(&flags, MDB_DUPSORT, "dupSort", false, options);
		//setFlagFromValue(&flags, MDB_DUPFIXED, "dupFixed", false, options);
		//setFlagFromValue(&flags, MDB_INTEGERDUP, "integerDup", false, options);
		//setFlagFromValue(&flags, MDB_REVERSEDUP, "reverseDup", false, options);
		//setFlagFromValue(&flags, MDB_CREATE, "create", false, options);
		Handle<Value> flagArg = options->Get(NanSymbol("flags"));
		if (flagArg->IsUint32()) {
			flags = flagArg->Uint32Value();
			keyIsUint32 = flags & MDB_INTEGERKEY;
		}

		// TODO: wrap mdb_set_compare
		// TODO: wrap mdb_set_dupsort

		// See if key is uint32_t
		//setFlagFromValue(&keyIsUint32, 1, "keyIsUint32", false, options);
		//if (keyIsUint32) {
		//	flags |= MDB_INTEGERKEY;
		//}
	}
	else {
		ThrowException(Exception::Error(String::New("Invalid parameters.")));
		NanReturnUndefined();
	}

	// Open transaction
	rc = mdb_txn_begin(ew->env, NULL, 0, &txn);
	if (rc != 0) {
		delete cname;
		mdb_txn_abort(txn);
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	// Open database
	rc = mdb_dbi_open(txn, cname, flags, &dbi);
	delete cname;
	if (rc != 0) {
		mdb_txn_abort(txn);
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	// Commit transaction
	rc = mdb_txn_commit(txn);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	// Create wrapper
	DbiWrap* dw = new DbiWrap(ew->env, dbi);
	dw->needsClose = true;
	dw->Wrap(args.This());
	dw->keyIsUint32 = keyIsUint32;

	NanReturnValue(args.This());
}

NAN_METHOD(DbiWrap::close) {
	NanScope();

	DbiWrap *dw = ObjectWrap::Unwrap<DbiWrap>(args.This());
	mdb_dbi_close(dw->env, dw->dbi);
	dw->needsClose = false;

	NanReturnUndefined();
}

NAN_METHOD(DbiWrap::drop) {
	NanScope();

	DbiWrap *dw = ObjectWrap::Unwrap<DbiWrap>(args.This());
	int del = 1;
	int rc;
	MDB_txn *txn;

	// Check if the database should be deleted
	if (args.Length() == 2 && args[1]->IsObject()) {
		Handle<Object> options = args[1]->ToObject();
		Handle<Value> opt = options->Get(String::NewSymbol("justFreePages"));
		del = opt->IsBoolean() ? !(opt->BooleanValue()) : 1;
	}

	// Begin transaction
	rc = mdb_txn_begin(dw->env, NULL, 0, &txn);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	// Drop database
	rc = mdb_drop(txn, dw->dbi, del);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	// Commit transaction
	rc = mdb_txn_commit(txn);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

void DbiWrap::setupExports(Handle<Object> exports) {
	// DbiWrap: Prepare constructor template
	Local<FunctionTemplate> dbiTpl = FunctionTemplate::New(DbiWrap::ctor);
	dbiTpl->SetClassName(String::NewSymbol("Dbi"));
	dbiTpl->InstanceTemplate()->SetInternalFieldCount(1);
	// DbiWrap: Add functions to the prototype
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "close", DbiWrap::close);
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "drop", DbiWrap::drop);

	// TODO: wrap mdb_stat too
	// DbiWrap: Get constructor
	//EnvWrap::dbiCtor = Persistent<Function>::New(dbiTpl->GetFunction());
	NanAssignPersistent(Function, EnvWrap::dbiCtor, dbiTpl->GetFunction());
}
