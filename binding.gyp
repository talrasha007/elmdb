{
  "targets": [
    {
      "target_name": "elmdb",
	  "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "sources": [
        "liblmdb/mdb.c",
        "liblmdb/lmdb.h",
        "liblmdb/midl.h",
        "liblmdb/midl.c",
        "src/elmdb.cpp",
        "src/misc.cpp",
        "src/misc.h",
        "src/env.cpp",
        "src/env.h",
        "src/txn.cpp",
        "src/txn.h",
        "src/dbi.cpp",
        "src/dbi.h",
        "src/cursor.cpp",
        "src/cursor.h"
      ]
    }
  ]
}
