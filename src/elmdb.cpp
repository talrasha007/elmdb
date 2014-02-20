#include <nan.h>

#include "env.h"
#include "dbi.h"
#include "txn.h"

void InitAll(v8::Handle<v8::Object> exports) {
	DbiWrap::setupExports(exports);
	TxnWrap::setupExports(exports);
	EnvWrap::setupExports(exports);
}

NODE_MODULE(elmdb, InitAll);
