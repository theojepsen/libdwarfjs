process.chdir(__dirname);
var DwarfJS = require('../');
var fs = require('fs');
var expect = require('expect.js');
var spawnSync = require('child_process').spawnSync;


describe('Build the test programs', function () {
  it('should compile', function () {
    var make = spawnSync('make', ['all'], {cwd: './progs'});
    if (make.status !== 0) {
      console.log(make.output[2].toString());
    }
    expect(make.status).to.be(0);
  });
});

describe('Open a signle CU', function () {
  var d, cu;
  before(function () {
    d = new DwarfJS('./progs/add.o');
  });
  it('should have one CU', function () {
    expect(d.cunits).to.be.an('array');
    expect(d.cunits).to.have.length(1);
    cu = d.cunits[0];
    expect(cu).to.be.an('object');
  });
  it('CU should have correct filename', function () {
    expect(cu.filename).to.be.a('string');
    expect(cu.filename).to.contain('progs/add.c');
  });
  it('CU should have correct DIE', function () {
    expect(cu.die).to.be.an('object');
    expect(cu.die.name).to.be('add.c');
    expect(cu.die.loPC).to.be(0);
    expect(cu.die.hiPC).to.be.greaterThan(1);
    expect(cu.die.tagName).to.be('DW_TAG_compile_unit');
    expect(cu.die.attributes).to.be.an('array');
    expect(cu.die.attributes.length).to.be.greaterThan(6);
  });
  it('Should have expected children DIEs', function () {
    expect(cu.die.children).to.have.length(3); 
    cu.die.children.forEach(function (child) {
      if (child.tagName === 'DW_TAG_subprogram' && child.name === 'add') {
        expect(child.line).to.be(1);
        expect(child.hiPC).to.be.greaterThan(1);
        expect(child.loPC).to.be.lessThan(child.hiPC);
      }
      else if (child.tagName === 'DW_TAG_subprogram' && child.name === 'sameVars') {
        expect(child.line).to.be(7);
        expect(child.hiPC).to.be.greaterThan(1);
        expect(child.loPC).to.be.lessThan(child.hiPC);
      }
      else if (child.tagName === 'DW_TAG_base_type') {
        expect(child.name).to.be('int');
        expect(child.attributes).to.be.an('array');
        expect(child.attributes.length).to.be.greaterThan(2);
        child.attributes.forEach(function (attr) {
          expect(attr).to.be.an('object');
          expect(attr).to.have.property('name');
          expect(attr).to.have.property('form');
          expect(attr).to.have.property('value');
          if (attr.name === 'DW_AT_byte_size') {
            expect(attr.value).to.be.a('number');
            expect(attr.value).to.be.greaterThan(3);
          }
        });
      }
      else
        expect().fail('Unexpected child tagName: ' + child.tagName);
    });
  });

  it('Should get a list of all dies', function () {
    var descendants = cu.die.descendants;
    expect(descendants).to.be.an('array');
    expect(descendants).to.have.length(8);
    descendants.forEach(function (die) {
      if (['add', 'sameVars', 'int', 'a', 'b', 'c', 'x'].indexOf(die.name) === -1)
        expect().fail('Unexpected die name: ' + die.name);
    });
  });

  it('Should use attributes.get', function () {
    expect(cu.die.attributes.get).to.be.a('function');

    cu.die.descendants.forEach(function (die) {
      expect(die.name).to.be(die.attributes.get('DW_AT_name').value);
      if (die.name === 'a')
        expect(die.attributes.get('DW_AT_decl_line').value).to.be(2);
      if (die.name === 'b')
        expect(die.attributes.get('DW_AT_decl_line').value).to.be(3);
    });
  });

  it('Should find pointer types', function () {
    var d2 = new DwarfJS('./progs/args.o');
    expect(d2.cunits).to.have.length(1);

    var foundPointer = false;
    d2.cunits[0].die.descendants.forEach(function (die) {
      if (die.tagName !== 'DW_TAG_pointer_type') return;

      expect(die.attributes.get('DW_AT_byte_size').value).to.be.greaterThan(3);
      var type = die.attributes.get('DW_AT_type');
      expect(type).to.be.ok();
      expect(type.value).to.be.an('object');
      expect(type.value.name).to.be('char');

      foundPointer = true;
    });

    if (!foundPointer)
      expect().fail("Didn't find pointer type in CU");
  });

  it('Should have correct variables', function () {
    this.timeout(500);
    cu.die.children.forEach(function (child) {
      if (child.tagName !== 'DW_TAG_subprogram') return;
      child.children.forEach(function (variable) {
        expect(variable.children).to.have.length(0);
        if (variable.name === 'a') {
          expect(variable.line).to.be(2);
        }
        else if (variable.name === 'b') {
          expect(variable.line).to.be(3);
        }
        else if (variable.name === 'c') {
          expect(variable.line).to.be(4);
        }
        else if (variable.name === 'x') {
        }
        else
          expect().fail('Unexpected variable: ' + variable.name);

        variable.attributes.forEach(function (attr) {
          expect(attr).to.be.an('object');
          expect(attr).to.have.property('name');
          expect(attr).to.have.property('form');
          expect(attr).to.have.property('value');
          if (attr.name === 'DW_AT_decl_file') {
            expect(attr.value).to.be.a('number');
            expect(attr.value).to.be.greaterThan(0);
          }
          if (attr.name === 'DW_AT_type') {
            expect(attr.value).to.be.an('object');
            expect(attr.value.name).to.be('int');
          }
          if (attr.name === 'DW_AT_location') {
            expect(attr.value).to.be.an('array');
            expect(attr.value).to.have.length(1);
            expect(attr.value[0]).to.have.property('locations');
            expect(attr.value[0].locations).to.be.an('array');
            expect(attr.value[0].locations).to.have.length(1);
            expect(attr.value[0].locations[0].atom).to.be('DW_OP_fbreg');
            if (variable.name === 'a')
              expect(attr.value[0].locations[0].number).to.be(-32);
            if (variable.name === 'b')
              expect(attr.value[0].locations[0].number).to.be(-28);
            if (variable.name === 'c')
              expect(attr.value[0].locations[0].number).to.be(-24);
            if (variable.name === 'x')
              expect(attr.value[0].locations[0].number).to.be(-20);
          }
        });
      });
    });
  });

});

describe('Open a binary with multiple CUs', function () {
  var d, cu;
  before(function () {
    d = new DwarfJS('./progs/main1');
  });
  it('should have multiple CUs', function () {
    expect(d.cunits).to.have.length(3);
    d.cunits.forEach(function (cu) {
      if (!cu.filename.match(/add.c$/) &&
         !cu.filename.match(/hello.c$/) &&
         !cu.filename.match(/main1.c$/))
        expect().fail('Should not have filename: ' + cu.filename);
    });
  });

  it('Should search for descendants by name', function () {
    cu = d.cunits.filter(function (c) { return c.filename.match(/add.c$/); })[0];
    expect(cu.die.descendants.find).to.be.a('function');
    expect(cu.die.descendants.find('a')).to.be.an('object');
    expect(cu.die.descendants.find('a').name).to.be('a');
    expect(cu.die.descendants.find('x')).to.be.an('array');
    expect(cu.die.descendants.find('x')).to.have.length(2);
    expect(cu.die.descendants.find('x')[0].name).to.be('x');
    expect(cu.die.descendants.find('x')[1].name).to.be('x');
  });

  it('Should search for descendants by properties', function () {
    cu = d.cunits.filter(function (c) { return c.filename.match(/add.c$/); })[0];
    expect(cu.die.descendants.find({name: 'a'})).to.be.an('object');
    expect(cu.die.descendants.find({name: 'a'}).name).to.be('a');
    expect(cu.die.descendants.find({tagName: 'DW_TAG_base_type'}).name).to.be('int');
  });
});

describe('Using VarFinder to find variables', function () {

  it('should find correct variables on the stack', function () {
    var vf = new DwarfJS.VarFinder('./progs/main1');
    var matchesA = vf.find({pc: 0x4004d6, rbpOffset: (-32 + 16)});
    expect(matchesA).to.be.an('array');
    expect(matchesA).to.have.length(1);
    expect(matchesA[0]).to.be.an('object');
    expect(matchesA[0].die).to.be.an('object');
    expect(matchesA[0].die.name).to.be('a');
    expect(matchesA[0].func).to.be('add');
    expect(matchesA[0].hipc).to.be(0x4004eb);
    expect(matchesA[0].lopc).to.be(0x4004c4);

    var matchesB = vf.find({pc: 0x4004d6, rbpOffset: (-28 + 16)});
    expect(matchesB).to.have.length(1);
    expect(matchesB[0].die).to.be.an('object');
    expect(matchesB[0].die.name).to.be('b');

    var matchesC = vf.find({pc: 0x4004d6, rbpOffset: (-24 + 16)});
    expect(matchesC).to.have.length(1);
    expect(matchesC[0].die).to.be.an('object');
    expect(matchesC[0].die.name).to.be('c');
  });

  it('should find stack variables using absolute addreses', function () {
    var vf = new DwarfJS.VarFinder('./progs/main1');
    var matchesA = vf.find({pc: 0x4004d6, memAddr: 1000, rbp: 1016});
    expect(matchesA).to.be.an('array');
    expect(matchesA).to.have.length(1);
    expect(matchesA[0].die.name).to.be('a');

    var matchesB = vf.find({pc: 0x4004d6, memAddr: 1000, rbp: 1012});
    expect(matchesB).to.have.length(1);
    expect(matchesB[0].die.name).to.be('b');

    var matchesC = vf.find({pc: 0x4004d6, memAddr: 1000, rbp: 1008});
    expect(matchesC).to.have.length(1);
    expect(matchesC[0].die.name).to.be('c');
  });

  it('Should find correct variables in registers', function () {
    var vf = new DwarfJS.VarFinder('./progs/registers.o');
    var matchesA = vf.find({pc: 0x12, reg: 'DW_OP_reg12'});
    expect(matchesA).to.be.an('array');
    expect(matchesA).to.have.length(1);
    expect(matchesA[0]).to.be.an('object');
    expect(matchesA[0].die).to.be.an('object');
    expect(matchesA[0].die.name).to.be('regvar1');
    expect(matchesA[0].func).to.be('regadd');
    expect(matchesA[0].hipc).to.be(0x1e);
    expect(matchesA[0].lopc).to.be(0x0);

    var matchesB = vf.find({pc: 0x12, reg: 'DW_OP_reg3'});
    expect(matchesB).to.have.length(1);
    expect(matchesB[0].die).to.be.an('object');
    expect(matchesB[0].die.name).to.be('regvar2');
  });

  it('Should find argument variables', function () {
    var vf = new DwarfJS.VarFinder('./progs/args.o');

    var matchesA = vf.find({pc: 0x12, rbpOffset: (-36 + 16)});
    expect(matchesA).to.have.length(1);
    expect(matchesA[0].die.name).to.be('arg1');
    expect(matchesA[0].func).to.be('takeargs');
    expect(matchesA[0].hipc).to.be(0x2c);
    expect(matchesA[0].lopc).to.be(0x0);

    var matchesB = vf.find({pc: 0x12, rbpOffset: (-48 + 16)});
    expect(matchesB).to.have.length(1);
    expect(matchesB[0].die.name).to.be('arg2');

    var matchesC = vf.find({pc: 0x12, rbpOffset: (-20 + 16)});
    expect(matchesC).to.have.length(1);
    expect(matchesC[0].die.name).to.be('b');
    expect(matchesC[0].func).to.be('takeargs');

  });
});
