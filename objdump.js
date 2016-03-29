var fs = require('fs');
var DwarfJS = require('./');
var argv = require('optimist').demand(1).usage('Usage: $0 BINARY_FILENAME').argv;
var util = require('util');

var binary_filename = argv._[0];
binary_filename = binary_filename || '/u/t/j/tjepsen/dwarf/c/read_nums';

if (!fs.existsSync(binary_filename)) {
  console.error('File does not exist:', binary_filename);
  process.exit(1);
}

var d = new DwarfJS(binary_filename);

function logDepth() {
  var d = arguments[0];
  var padding = new Array(d + 1).join('  ');
  arguments[0] = padding;
  console.log.apply(console, arguments);
}

function recDie(die, depth) {
  var ranges = '';
  if (die.hiPC || die.loPC)
    ranges = ' (0x' + die.hiPC.toString(16) + ' -> 0x' + die.loPC.toString(16) + ')';
  logDepth(depth, die.tagName + ": '" + die.name + "'" + ranges);
  die.attributes.forEach(function (a) {
    var val = a.value;
    if (val && val.constructor && val.constructor.name === 'DwarfDIE')
      val = val.name;
    if (a.name === 'DW_AT_location') {
      if (a.value[0] && a.value[0].locations) {
        if (a.value[0].locations[0])
          val = '(' + a.value[0].locations[0].atom + ': ' + a.value[0].locations[0].number + ')';
        if (a.value[0].locations.length !== 1)
          val += ' (loc count: ' + a.value[0].locations.length + ')';
      }
      if (a.value.length !== 1)
        val += ' (locdesc count: ' + a.value.length + ')';
    }
    var form = val !== undefined ? '' : '(' + a.form + ')';
    logDepth(depth+2, a.name + ': ' + val + form);
  });


  die.children.forEach(function (child) {
    recDie(child, depth+1);
  });
}

d.cunits.forEach(function (h) {
  recDie(h.die, 0);
});
