var dwarfjs = require('bindings')('dwarfjs');
var path = require('path');

function makeTestFun(prop, pattern) {
  if (typeof pattern === 'string')
    return function (x) { return x[prop] === pattern; };
  if (typeof pattern === 'object' && typeof pattern.test === 'function')
    return function (x) { return pattern.test(x[prop]); };
  return null;
}

function filterArray(objArr, defaultProp, searchDesc) {
  var test;
  if (typeof searchDesc === 'string' ||
      (typeof searchDesc === 'object' && typeof searchDesc.test === 'function'))
    test = makeTestFun(defaultProp, searchDesc);
  else if (typeof searchDesc === 'object') {
    var propTests = {};
    Object.keys(searchDesc).forEach(function (prop) {
      propTests[prop] = makeTestFun(prop, searchDesc[prop]);
    });
    test = function (x) {
      return Object.keys(propTests).every(function (prop) {
        return propTests[prop](x);
      });
    };
  }
  if (!test) throw new Error('Bad search pattern');
  return objArr.filter(test);
}

function DwarfDIE(parentDie, dieBinding) {
  var myCU = parentDie ? parentDie.cu : null;
  var myChildren;
  var myAttributes;
  var myName;
  var myTagName;
  var myLine;
  var myPcRange;

  function loadPCRange() {
    if (!myPcRange)
      myPcRange = dieBinding.getPCRange();
  }

  this.__defineGetter__('descendants', function () {
    function findRec(die) {
      return Array.prototype.concat.apply(die.children, die.children.map(findRec));
    }
    var dies = findRec(this);
    dies.find = function (pattern) {
      var found = filterArray(dies, 'name', pattern);
      if (found.length === 1) return found[0];
      return found;
    };
    return dies;
  });

  this.__defineGetter__('children', function () {
    var self = this;
    if (!myChildren)
      myChildren = dieBinding.getChildren().map(function (c) {
        return new DwarfDIE(self, c);
      });
      return myChildren;
  });
  this.__defineGetter__('attributes', function () {
    if (!myAttributes) {
      myAttributes = dieBinding.getAttributes().map(function (attr) {
        if (attr.form && attr.form.match(/^DW_FORM_ref/) && attr.value &&
            attr.value.constructor && attr.value.constructor.name === 'DwarfDIE')
          attr.value = new DwarfDIE(null, attr.value);
        return attr;
      });
      myAttributes.get = function (name) {
        var found = myAttributes.filter(function (a) { return a.name === name; });
        return found.length ? found[0] : null;
      };
    }
    return myAttributes;
  });
  this.__defineGetter__('line', function () {
    if (myLine === undefined) {
      var lines = this.attributes.filter(function (a) {
        return a.name === 'DW_AT_decl_line';
      }).map(function (a) { return a.value; });
      myLine = lines.length ? lines[0] : null;
    }
    return myLine;
  });
  this.__defineGetter__('cu', function () {
    return myCU;
  });
  this.__defineSetter__('cu', function (cu) {
    myCU = cu;
  });
  this.__defineGetter__('name', function () {
    if (!myName)
      myName = dieBinding.getName();
    return myName;
  });
  this.__defineGetter__('tagName', function () {
    if (!myTagName)
      myTagName = dieBinding.getTagName();
    return myTagName;
  });
  this.__defineGetter__('hiPC', function () {
    loadPCRange();
    return myPcRange.hi;
  });
  this.__defineGetter__('loPC', function () {
    loadPCRange();
    return myPcRange.lo;
  });

  // TODO
  this.toString = function () {
    var o = {};

    Object.keys(this).forEach(function (k) {
      if (typeof this[k] === 'function')
        o[k] = this[k]();
      else
        o[k] = this[k];
    });
    return o;
  };
}

function DwarfCompilationUnit(headerBinding) {
  var myDie = null;
  this.__defineGetter__('die', function () {
    if (!myDie) {
      myDie = new DwarfDIE(null, headerBinding.getDIE());
      myDie.cu = this;
    }
    return myDie;
  });
  var myFilename;
  this.__defineGetter__('filename', function () {
    if (!myFilename) {
      var name, dir;
      this.die.attributes.forEach(function (a) {
        if (a.name === 'DW_AT_name') name = a.value;
        else if (a.name === 'DW_AT_comp_dir') dir = a.value;
      });
      myFilename = path.join(dir, name);
    }
    return myFilename;
  });
}

// Dwarf(filename, [options])
// Dwarf(options)
function Dwarf() {
  var opts = {};
  var args = arguments;
  if (args.length < 1)
    throw new Error('libdwarf constructor: no arguments provided');
  if (typeof args[0] === 'string')
    opts.filename = Array.prototype.shift.apply(args);
  if (args.length > 1)
    throw new Error('libdwarf constructor: too many arguments provided');
  if (args.length === 1) {
    if (typeof args[0] !== 'object')
      throw new TypeError('libdwarf constructor: unrecognized argument: expected options hash');
    Object.keys(args[0]).forEach(function (k) { opts[k] = args[0][k]; });
  }
  if (!opts.filename)
    throw new Error('libdwarf constructor: no filename provided');

  this.options = opts;

  var dbgBinding = new dwarfjs.DwarfDbg(this.options.filename);

  var myCunits = null;
  this.__defineGetter__('cunits', function () {
    if (!myCunits) {
      myCunits = dbgBinding.getCUHeaders().map(function (h) {
        return new DwarfCompilationUnit(h);
      });
      myCunits.find = function (pattern) {
        var found = filterArray(myCunits, 'filename', pattern);
        if (found.length === 1) return found[0];
        return found;
      };
    }
    return myCunits;
  });

  var myLoclist = null;
  this.__defineGetter__('loclist', function () {
    if (!myLoclist) // Transform it into a map
      myLoclist = dbgBinding.getLocList().reduce(function (p, c) {
        c.expr = c.expr.locations[0];
        if (!p[c.offset]) p[c.offset] = [];
        p[c.offset].push(c);
        return p;
      }, {});
    return myLoclist;
  });
}

module.exports = Dwarf;
