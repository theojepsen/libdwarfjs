var DwarfJS = require('./dwarf.js');

function VarFinder(binary_filename) {
  var dwarf = new DwarfJS(binary_filename);
  this.dwarf = dwarf;

  var vars = [];

  function recDie(args) {
    var hipc = args.hipc;
    var lopc = args.lopc;
    var loclistOff = args.loclistOff;
    var func = args.func;
    if (typeof args.die.hiPC === 'number') hipc = args.die.hiPC;
    if (typeof args.die.loPC === 'number') lopc = args.die.loPC;

    // If it's a variable, add it to the variables array
    if (args.die.tagName === 'DW_TAG_variable' || args.die.tagName === 'DW_TAG_formal_parameter') {
      var name, loc, varhipc, varlopc;
      name = args.die.name;
      var locAttr = args.die.attributes.get('DW_AT_location');
      var locs = locAttr && locAttr.value;
      if (locs && locs[0] && locs[0].locations[0]) {
        varhipc = locs[0].hipc;
        varlopc = locs[0].lopc;
        loc = locs[0].locations[0];
      }

      if (name && loc)
        vars.push({
          die: args.die,
          name: name,
          func: func,
          loc: loc,
          loclistOff: loclistOff,
          hipc: varhipc < args.hipc ? varhipc : args.hipc,
          lopc: varlopc > args.lopc ? varlopc : args.lopc
        });
    } // If it's a function, update function name and frame base
    else if (args.die.tagName === 'DW_TAG_subprogram') {
      var base = args.die.attributes.get('DW_AT_frame_base');
      loclistOff =  base && base.value !== undefined ? base.value : loclistOff;
      func = args.die.name || func;
    }

    // Recurse down on children
    args.die.children.forEach(function (child) {
      recDie({die: child,
             hipc: hipc,
             func: func,
             lopc: lopc,
             loclistOff: loclistOff
      });
    });
  }

  dwarf.cunits.forEach(function (h) {
    recDie({die: h.die, hipc: 0xffffffffffffffff, lopc: 0});
  });

  function getStackOffset(pc, v) {
    if (typeof v.loclistOff !== 'number') return v.loc.number; // no DW_AT_frame_base for this function
    var ll = dwarf.loclist[v.loclistOff];
    if (!ll) return null;
    var offset = null;
    ll.some(function (entry) {
      var absRange = [v.lopc + entry.begin, v.lopc + entry.end];
      if (absRange[0] <= pc && pc <= absRange[1]) {
        offset = entry.expr.number + v.loc.number;
        return true;
      }
    });
    return offset;
  }


  this.find = function (locDesc) {
    var pc = locDesc.pc;
    if (locDesc.rbp !== undefined && locDesc.memAddr !== undefined)
      locDesc.rbpOffset = locDesc.memAddr - locDesc.rbp;

    // No specific location provided, so find anything in scope
    var find_any_var_in_scope = false;
    if (!locDesc.rbpOffset && !locDesc.reg && !locDesc.memAddr)
      find_any_var_in_scope = true;

    var matches = vars.filter(function (v) {
      return v.lopc <= pc && pc <= v.hipc;
    }).filter(function (v) {
      if (locDesc.rbpOffset !== undefined) {
        var vOff = getStackOffset(pc, v);
        return vOff === locDesc.rbpOffset;
      }
      else if (locDesc.memAddr !== undefined) {
        return v.loc.number == locDesc.memAddr;
      }
      else if (v.loc.atom === locDesc.reg)
        return true;
      return find_any_var_in_scope;
    });
    return matches;
  };
}

module.exports = VarFinder;
