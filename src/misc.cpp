#include <nan.h>

#include "misc.h"

using namespace v8;
using namespace node;

void freeNothing(char*, void*) { }

#define EXPORT_FLAG(flag) exports->Set(NanSymbol(#flag), Number::New(flag))

void exportFlags(v8::Handle<v8::Object> exports) {
	EXPORT_FLAG(MDB_NOSUBDIR);
	EXPORT_FLAG(MDB_RDONLY);
	EXPORT_FLAG(MDB_WRITEMAP);
	EXPORT_FLAG(MDB_NOMETASYNC);
	EXPORT_FLAG(MDB_NOSYNC);
	EXPORT_FLAG(MDB_MAPASYNC);

	EXPORT_FLAG(MDB_NOTLS);
	EXPORT_FLAG(MDB_NOLOCK);

	EXPORT_FLAG(MDB_INTEGERKEY);
	EXPORT_FLAG(MDB_REVERSEKEY);
	EXPORT_FLAG(MDB_DUPSORT);
	EXPORT_FLAG(MDB_DUPFIXED);
	EXPORT_FLAG(MDB_INTEGERDUP);
	EXPORT_FLAG(MDB_REVERSEDUP);
	EXPORT_FLAG(MDB_CREATE);

	EXPORT_FLAG(MDB_NOOVERWRITE);
	EXPORT_FLAG(MDB_NODUPDATA);
	//EXPORT_FLAG(MDB_RESERVE);
	//EXPORT_FLAG(MDB_APPEND);
	//EXPORT_FLAG(MDB_APPENDDUP);
}