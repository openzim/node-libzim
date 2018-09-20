#!/usr/bin/env node
var child_process = require('child_process');
var path = require('path');
var os = require('os');

process.argv.shift(); // take off the 'node'
// Pull our current directory from the 'cdconfigure.js'
var mydir = path.resolve(path.dirname(process.argv.shift()));
var libzim = path.join(mydir, 'openzim', 'zimlib');
var confdir = process.argv.shift();
console.log("Changing to", confdir);

var confArgs = [ path.resolve(path.join(libzim, 'configure')) ].
  concat(process.argv);

//console.log(os.arch());
//console.log(process.version, process.arch, process.config.variables.target_arch);

if (process.config.variables.target_arch !== 'ia32') {
    // needed for x86_64
    confArgs.push('CFLAGS=-fPIC');
    confArgs.push('CXXFLAGS=-fPIC');
} else {
    // needed for multilib builds on x86_64 host
    confArgs.push('CFLAGS=-m32');
    confArgs.push('CXXFLAGS=-m32');
}

console.log(path.join(libzim, 'autogen.sh'));
child_process.execFile(path.join(libzim, 'autogen.sh'), [
    //'autogen.sh'
], {
    cwd: libzim,
    stdio: 'inherit'
}, function(err, stdout, stderr) {
    if (err) { throw err; }

    console.log(confArgs.join(' '));
    child_process.execFile(confArgs[0], confArgs.slice(1), {
        cwd: confdir,
        stdio: 'inherit'
    }, function(err, stdout, stderr) {
        if (err) { throw err; }
        process.exit(0);
    });
});
