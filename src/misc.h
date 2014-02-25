#pragma once

#include "../liblmdb/lmdb.h"

void freeNothing(char*, void*);
void exportFlags(v8::Handle<v8::Object> exports);

class MDBVal {
private:
	MDB_val _val;
	uint32_t _intval;
	bool _error;
	bool _shouldFree;

public:
	MDBVal() : _shouldFree(false), _error(false) {
		_val.mv_data = NULL;
		_val.mv_size = 0;
	}

	MDBVal(v8::Local<v8::Value> v, bool stringOnly = false) : _shouldFree(false), _error(false) {
		_val.mv_data = NULL;
		_val.mv_size = 0;
		from(v, stringOnly);
	}

	~MDBVal() {
		if (_shouldFree) delete[](char*)_val.mv_data;
	}

public:
	inline MDB_val val() { return _val; }
	inline bool hasError() { return _error; }

	inline void from(v8::Local<v8::Value> v, bool stringOnly = false) {
		if (v->IsString()) {
			_val.mv_data = NanCString(v, &_val.mv_size);
			_shouldFree = true;
		}
		else if (!stringOnly && v->IsNumber()) {
			_intval = v->Uint32Value();
			_val.mv_data = &_intval;
			_val.mv_size = sizeof(_intval);
		}
		else {
			_error = true;
			//v8::ThrowException(v8::Exception::Error(v8::String::New("Unsupported type.")));
		}
	}

private:
	MDBVal(const MDBVal&);
	MDBVal& operator=(const MDBVal&);
};
