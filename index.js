const bindings = require("./build/Release/aio.node");
const fs = require("fs");

exports.readFile = function (path, cb) {
	if (typeof cb !== "function") {
		throw new TypeError("Callback must be a function");
	}
	fs.open(path, "r", 0o666, (err, fd) => {
		if (err) return cb(err);
		fs.fstat(fd, (err, stats) => {
			if (err) return cb(err);
			bindings.readFile(fd, stats.size, cb);
		});
	});
};
