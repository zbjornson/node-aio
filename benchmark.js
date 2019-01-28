const fs = require("fs");
const aiofs = require(".");
const async = require("async");

function chunked(filename, cb) {
	fs.readFile(filename, cb);
}

function oneshot(filename, cb) {
	// shoddy implementation -- leaks fd in case of errors
	fs.open(filename, "r", 0o666, (err, fd) => {
		if (err) return cb(err);
		fs.fstat(fd, (err, stats) => {
			if (err) return cb(err);
			const data = Buffer.allocUnsafe(stats.size);
			fs.read(fd, data, 0, stats.size, 0, (err, bytesRead) => {
				if (err) return cb(err);
				fs.close(fd, err => {
					cb(err, data);
				});
			});
		});
	});
}

function aio(filename, cb) {
	aiofs.readFile(filename, cb);
}

fs.writeFileSync("./test.dat", Buffer.alloc(16e6, 'x'));

function bm(method, name, cb) {
	const start = Date.now();
	async.timesSeries(50, (n, next) => {
		method("./test.dat", next);
	}, err => {
		if (err) return cb(err);
		const diff = Date.now() - start;
		console.log(name, diff);
		cb();
	});
}

async.series([
	cb => bm(chunked, "fs.readFile()", cb),
	cb => bm(oneshot, "oneshot", cb),
	cb => bm(aio, "aio", cb)
]);
