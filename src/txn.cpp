#include <nan.h>

#include "misc.h"

#include "dbi.h"
#include "env.h"
#include "txn.h"

using namespace v8;
using namespace node;

TxnWrap::TxnWrap(MDB_env *env, MDB_txn *txn) {
	this->env = env;
	this->txn = txn;
}

TxnWrap::~TxnWrap() {
	// Close if not closed already
	if (this->txn) {
		mdb_txn_abort(txn);
	}
}

NAN_METHOD(TxnWrap::ctor) {
	NanScope();

	EnvWrap *ew = ObjectWrap::Unwrap<EnvWrap>(args[0]->ToObject());
	int flags = 0;

	if (args[1]->IsObject()) {
		Local<Object> options = args[1]->ToObject();

		// Get flags from options
		//setFlagFromValue(&flags, MDB_RDONLY, "readOnly", false, options);
		Handle<Value> flagArg = options->Get(NanSymbol("flags"));
		if (flagArg->IsUint32()) flags = flagArg->Uint32Value();
	}


	MDB_txn *txn;
	int rc = mdb_txn_begin(ew->env, NULL, flags, &txn);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	TxnWrap* tw = new TxnWrap(ew->env, txn);
	tw->Wrap(args.This());

	NanReturnValue(args.This());
}

NAN_METHOD(TxnWrap::commit) {
	NanScope();

	TxnWrap *tw = ObjectWrap::Unwrap<TxnWrap>(args.This());

	if (!tw->txn) {
		ThrowException(Exception::Error(String::New("The transaction is already closed.")));
		NanReturnUndefined();
	}

	int rc = mdb_txn_commit(tw->txn);
	tw->txn = NULL;
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_METHOD(TxnWrap::abort) {
	NanScope();

	TxnWrap *tw = ObjectWrap::Unwrap<TxnWrap>(args.This());

	if (!tw->txn) {
		ThrowException(Exception::Error(String::New("The transaction is already closed.")));
		NanReturnUndefined();
	}

	mdb_txn_abort(tw->txn);
	tw->txn = NULL;

	NanReturnUndefined();
}

NAN_METHOD(TxnWrap::reset) {
	NanScope();

	TxnWrap *tw = ObjectWrap::Unwrap<TxnWrap>(args.This());

	if (!tw->txn) {
		ThrowException(Exception::Error(String::New("The transaction is already closed.")));
		NanReturnUndefined();
	}

	mdb_txn_reset(tw->txn);

	NanReturnUndefined();
}

NAN_METHOD(TxnWrap::renew) {
	NanScope();

	TxnWrap *tw = ObjectWrap::Unwrap<TxnWrap>(args.This());

	if (!tw->txn) {
		ThrowException(Exception::Error(String::New("The transaction is already closed.")));
		NanReturnUndefined();
	}

	int rc = mdb_txn_renew(tw->txn);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_METHOD(TxnWrap::get) {
	NanScope();

	TxnWrap *tw = ObjectWrap::Unwrap<TxnWrap>(args.This());
	DbiWrap *dw = ObjectWrap::Unwrap<DbiWrap>(args[0]->ToObject());

	if (!tw->txn) {
		ThrowException(Exception::Error(String::New("The transaction is already closed.")));
		NanReturnUndefined();
	}

	MDBVal kk(args[1]);
	MDB_val key = kk.val(), data;

	int rc = mdb_get(tw->txn, dw->dbi, &key, &data);

	if (rc == MDB_NOTFOUND) NanReturnNull();

	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	NanReturnValue(String::New((char*)data.mv_data, int(data.mv_size)));
}

NAN_METHOD(TxnWrap::put) {
	NanScope();

	TxnWrap *tw = ObjectWrap::Unwrap<TxnWrap>(args.This());
	DbiWrap *dw = ObjectWrap::Unwrap<DbiWrap>(args[0]->ToObject());

	if (!tw->txn) {
		ThrowException(Exception::Error(String::New("The transaction is already closed.")));
		NanReturnUndefined();
	}

	int flags = 0;
	MDBVal key(args[1]), data(args[2], true);
	MDB_val tk(key.val()), tv(data.val());
	int rc = mdb_put(tw->txn, dw->dbi, &tk, &tv, flags);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_METHOD(TxnWrap::del) {
	NanScope();

	TxnWrap *tw = ObjectWrap::Unwrap<TxnWrap>(args.This());
	DbiWrap *dw = ObjectWrap::Unwrap<DbiWrap>(args[0]->ToObject());

	if (!tw->txn) {
		ThrowException(Exception::Error(String::New("The transaction is already closed.")));
		NanReturnUndefined();
	}

	//int flags = 0;
	MDBVal kk(args[1]), vv;
	MDB_val key = kk.val(), data;

	if (!args[2]->IsUndefined()) {
		vv.from(args[2]);
		data = vv.val();
	}

	int rc = mdb_del(tw->txn, dw->dbi, &key, args[2]->IsUndefined() ? NULL : &data);
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

void TxnWrap::setupExports(Handle<v8::Object> exports) {
	// TxnWrap: Prepare constructor template
	Local<FunctionTemplate> txnTpl = FunctionTemplate::New(TxnWrap::ctor);
	txnTpl->SetClassName(String::NewSymbol("Txn"));
	txnTpl->InstanceTemplate()->SetInternalFieldCount(1);
	// TxnWrap: Add functions to the prototype
	NODE_SET_METHOD(txnTpl->PrototypeTemplate(), "commit", TxnWrap::commit);
	NODE_SET_METHOD(txnTpl->PrototypeTemplate(), "abort", TxnWrap::abort);
	NODE_SET_METHOD(txnTpl->PrototypeTemplate(), "get", TxnWrap::get);
	NODE_SET_METHOD(txnTpl->PrototypeTemplate(), "put", TxnWrap::put);
	NODE_SET_METHOD(txnTpl->PrototypeTemplate(), "del", TxnWrap::del);
	NODE_SET_METHOD(txnTpl->PrototypeTemplate(), "reset", TxnWrap::reset);
	NODE_SET_METHOD(txnTpl->PrototypeTemplate(), "renew", TxnWrap::renew);
	// TODO: wrap mdb_cmp too
	// TODO: wrap mdb_dcmp too
	// TxnWrap: Get constructor
	NanAssignPersistent(Function, EnvWrap::txnCtor, txnTpl->GetFunction());
}
