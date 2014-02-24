#include <nan.h>

#include "misc.h"

#include "env.h"
#include "dbi.h"
#include "txn.h"
#include "cursor.h"

void InitAll(v8::Handle<v8::Object> exports) {
	exportFlags(exports);

	DbiWrap::setupExports(exports);
	TxnWrap::setupExports(exports);
	EnvWrap::setupExports(exports);
	CursorWrap::setupExports(exports);
}

NODE_MODULE(elmdb, InitAll);
