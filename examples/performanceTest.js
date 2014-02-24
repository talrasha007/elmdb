var elmdb = require('../'),
    env = new elmdb.Env();

env.open({
    path: './',
    mapSize: 1024 * 1024 * 256,
    maxDbs: 10,
    flags: elmdb.MDB_NOSYNC | elmdb.MDB_NOMETASYNC | elmdb.MDB_NOLOCK | elmdb.MDB_NOTLS
});

var dbi = env.openDbi({
    name: 'testdb',
    flags: elmdb.MDB_CREATE
});

function performanceTest(fn) {
    var start = new Date();
    fn(function (err, desc) {
        var span = new Date() - start;
        console.log(desc + ": " + (span / 1000) + 's');
    });
}

function put(cb) {
    function pp(k) {
        var txn = env.beginTxn();
        txn.put(dbi, k, '');
        txn.commit();
    }

    for (var i = 0; i < 1000; i++) {
        for (var j = 0; j < 1000; j++) {
            pp(i + '.' + j);
        }
    }

    cb(null, 'insert 1M records');
}

function put1k(cb) {
    function pp(k) {
        txn.put(dbi, k, k);
    }

    for (var i = 0; i < 1000; i++) {
        var txn = env.beginTxn();
        for (var j = 0; j < 1000; j++) {
            pp(i + '.' + j);
        }
        txn.commit();
    }

    cb(null, 'insert 1M records(1k per txn)');
}

function get1k(cb) {
    function gg(k) {
        txn.get(dbi, k);//new Buffer(k));
        //txn.get(dbi, new Buffer(k));
    }

    var txn = env.beginTxn({ flags: elmdb.MDB_RDONLY });
    for (var i = 0; i < 1000; i++) {
        for (var j = 1000; j >= 0; j--) {
            gg(i + '.' + j);
        }
        //txn.reset();
        //txn.renew();
    }

    txn.abort();
    cb(null, 'get 1M records(1k per txn)');
}

function curReadSeq(cb) {
    var txn = env.beginTxn({ flags: elmdb.MDB_RDONLY }),
        cur = new elmdb.Cursor(txn, dbi),
        cnt = 0;

    cur.goToFirst();
    while (cur.goToNext()) {
        cnt++;
        if ((cnt % 10240) === 0) {
            txn.reset();
            txn.renew();
        }
    }

    cur.close();
    txn.abort();

    cb(null, 'cur seq read ' + cnt + ' records');
}

//performanceTest(put);
performanceTest(put1k);
performanceTest(curReadSeq);
performanceTest(get1k);
performanceTest(get1k);
performanceTest(get1k);
