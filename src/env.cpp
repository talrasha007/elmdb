#include <nan.h>

#include "env.h"

using namespace v8;
using namespace node;

typedef struct EnvSyncData {
	uv_work_t request;
	NanCallback* callback;
	EnvWrap *ew;
	MDB_env *env;
	int rc;
} EnvSyncData;

EnvWrap::EnvWrap() {
	this->env = NULL;
}

EnvWrap::~EnvWrap() {
	// Close if not closed already
	if (this->env) {
		mdb_env_close(env);
	}
}

NAN_METHOD(EnvWrap::ctor) {
	NanScope();
	int rc;

	EnvWrap* wrapper = new EnvWrap();
	rc = mdb_env_create(&(wrapper->env));

	if (rc != 0) {
		mdb_env_close(wrapper->env);
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	wrapper->Wrap(args.This());
	NanReturnValue(args.This());
}

template<class T>
int applyUint32Setting(int(*f)(MDB_env *, T), MDB_env* e, Local<Object> options, T dflt, const char* keyName) {
	int rc;
	const Handle<Value> value = options->Get(NanSymbol(keyName));
	if (value->IsUint32()) {
		rc = f(e, value->Uint32Value());
	}
	else {
		rc = f(e, dflt);
	}

	return rc;

}

NAN_METHOD(EnvWrap::open) {
	NanScope();
	int rc;
	int flags = 0;

	// Get the wrapper
	EnvWrap *ew = ObjectWrap::Unwrap<EnvWrap>(args.This());

	if (!ew->env) {
		ThrowException(Exception::Error(String::New("The environment is already closed.")));
		NanReturnUndefined();
	}

	Local<Object> options = args[0]->ToObject();
	Local<String> path = options->Get(NanSymbol("path"))->ToString();

	// Parse the maxDbs option
	rc = applyUint32Setting<unsigned>(&mdb_env_set_maxdbs, ew->env, options, 1, "maxDbs");
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	// Parse the mapSize option
	Handle<Value> mapSizeOption = options->Get(NanSymbol("mapSize"));
	if (mapSizeOption->IsNumber()) {
		double mapSizeDouble = mapSizeOption->NumberValue();
		size_t mapSizeSizeT = (size_t)mapSizeDouble;
		rc = mdb_env_set_mapsize(ew->env, mapSizeSizeT);
		if (rc != 0) {
			ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
			NanReturnUndefined();
		}
	}

	// Parse the maxDbs option
	rc = applyUint32Setting<unsigned>(&mdb_env_set_maxreaders, ew->env, options, 1, "maxReaders");
	if (rc != 0) {
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	// NOTE: MDB_FIXEDMAP is not exposed here since it is "highly experimental" + it is irrelevant for this use case
	// NOTE: MDB_NOTLS is not exposed here because it is irrelevant for this use case, as node will run all this on a single thread anyway
	//setFlagFromValue(&flags, MDB_NOSUBDIR, "noSubdir", false, options);
	//setFlagFromValue(&flags, MDB_RDONLY, "readOnly", false, options);
	//setFlagFromValue(&flags, MDB_WRITEMAP, "useWritemap", false, options);
	//setFlagFromValue(&flags, MDB_NOMETASYNC, "noMetaSync", false, options);
	//setFlagFromValue(&flags, MDB_NOSYNC, "noSync", false, options);
	//setFlagFromValue(&flags, MDB_MAPASYNC, "mapAsync", false, options);
	Handle<Value> flagArg = options->Get(NanSymbol("flags"));
	if (flagArg->IsUint32()) flags = flagArg->Uint32Value();

	size_t l;
	char *cpath = NanCString(path, &l);
	// TODO: make file attributes configurable
	rc = mdb_env_open(ew->env, cpath, flags, 0664);
	delete[] cpath;

	if (rc != 0) {
		mdb_env_close(ew->env);
		ew->env = NULL;
		ThrowException(Exception::Error(String::New(mdb_strerror(rc))));
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_METHOD(EnvWrap::close) {
	NanScope();
	EnvWrap *ew = ObjectWrap::Unwrap<EnvWrap>(args.This());

	if (!ew->env) {
		ThrowException(Exception::Error(String::New("The environment is already closed.")));
		NanReturnUndefined();
	}

	mdb_env_close(ew->env);
	ew->env = NULL;

	NanReturnUndefined();
}

void sync_cb(uv_work_t *request) {
	// Performing the sync (this will be called on a separate thread)
	EnvSyncData *d = static_cast<EnvSyncData*>(request->data);
	d->rc = mdb_env_sync(d->env, 1);
}

void after_sync_cb(uv_work_t *request, int) {
	// Executed after the sync is finished
	EnvSyncData *d = static_cast<EnvSyncData*>(request->data);
	const unsigned argc = 1;
	Handle<Value> argv[argc];

	if (d->rc == 0) {
		argv[0] = Null();
	}
	else {
		argv[0] = Exception::Error(String::New(mdb_strerror(d->rc)));
	}

	d->callback->Call(argc, argv);
	delete d->callback;
	delete d;
}

NAN_METHOD(EnvWrap::sync) {
	NanScope();

	EnvWrap *ew = ObjectWrap::Unwrap<EnvWrap>(args.This());

	if (!ew->env) {
		ThrowException(Exception::Error(String::New("The environment is already closed.")));
		NanReturnUndefined();
	}

	Handle<Function> callback = Handle<Function>::Cast(args[0]);

	EnvSyncData *d = new EnvSyncData;
	d->request.data = d;
	d->ew = ew;
	d->env = ew->env;
	d->callback = new NanCallback(callback);

	uv_queue_work(uv_default_loop(), &(d->request), sync_cb, after_sync_cb);

	NanReturnUndefined();
}

void EnvWrap::setupExports(Handle<v8::Object> exports) {
	// EnvWrap: Prepare constructor template
	Local<FunctionTemplate> envTpl = FunctionTemplate::New(EnvWrap::ctor);
	envTpl->SetClassName(NanSymbol("Env"));
	envTpl->InstanceTemplate()->SetInternalFieldCount(1);
	// EnvWrap: Add functions to the prototype
	NODE_SET_METHOD(envTpl->PrototypeTemplate(), "open", EnvWrap::open);
	NODE_SET_METHOD(envTpl->PrototypeTemplate(), "close", EnvWrap::close);
	//NODE_SET_METHOD(envTpl->PrototypeTemplate(), "beginTxn", EnvWrap::beginTxn);
	//NODE_SET_METHOD(envTpl->PrototypeTemplate(), "openDbi", EnvWrap::openDbi);
	NODE_SET_METHOD(envTpl->PrototypeTemplate(), "sync", EnvWrap::sync);
	// TODO: wrap mdb_env_copy too
	// TODO: wrap mdb_env_stat too
	// TODO: wrap mdb_env_info too

	// Set exports
	exports->Set(NanSymbol("Env"), envTpl->GetFunction());
}
