var fs = require('fs');
var DwarfJS = require('./');
var argv = require('optimist').demand(3).usage('Usage: $0 BINARY_FILENAME INST_PC LOC').argv;

var binary_filename = argv._[0];

var inst_pc = argv._[1];

var loc = argv._[2];
if (typeof loc === 'number')
  if (loc > 0) loc = loc * -1;

if (!fs.existsSync(binary_filename)) {
  console.error('File does not exist:', binary_filename);
  process.exit(1);
}

function printVar(v) {
  console.log('----------------------------');
  console.log("name: '" + v.name + "'");
  console.log('loc : ' + v.loc.number + ' (' + v.loc.atom + ')');
  console.log('func: ' + v.func);
  console.log('hipc: 0x' + v.hipc.toString(16));
  console.log('lopc: 0x' + v.lopc.toString(16));
  console.log('----------------------------');
}

var vf = new DwarfJS.VarFinder(binary_filename);

var matches = vf.find(inst_pc, loc);

if (matches.length > 0) {
  matches.forEach(printVar);
}
else
  console.error("No variables found");
