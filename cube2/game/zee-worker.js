
// zee.js: zlib compiled to js

var Zee = (function() {


var Module = {
  'noExitRuntime': true
};
// Note: For maximum-speed code, see "Optimizing Code" on the Emscripten wiki, https://github.com/kripken/emscripten/wiki/Optimizing-Code
// Note: Some Emscripten settings may limit the speed of the generated code.
try {
  this['Module'] = Module;
  Module.test;
} catch(e) {
  this['Module'] = Module = {};
}
// The environment setup code below is customized to use Module.
// *** Environment setup code ***
var ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof require === 'function';
var ENVIRONMENT_IS_WEB = typeof window === 'object';
var ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;
if (ENVIRONMENT_IS_NODE) {
  // Expose functionality in the same simple way that the shells work
  // Note that we pollute the global namespace here, otherwise we break in node
  Module['print'] = function(x) {
    process['stdout'].write(x + '\n');
  };
  Module['printErr'] = function(x) {
    process['stderr'].write(x + '\n');
  };
  var nodeFS = require('fs');
  var nodePath = require('path');
  Module['read'] = function(filename, binary) {
    filename = nodePath['normalize'](filename);
    var ret = nodeFS['readFileSync'](filename);
    // The path is absolute if the normalized version is the same as the resolved.
    if (!ret && filename != nodePath['resolve'](filename)) {
      filename = path.join(__dirname, '..', 'src', filename);
      ret = nodeFS['readFileSync'](filename);
    }
    if (ret && !binary) ret = ret.toString();
    return ret;
  };
  Module['readBinary'] = function(filename) { return Module['read'](filename, true) };
  Module['load'] = function(f) {
    globalEval(read(f));
  };
  if (!Module['arguments']) {
    Module['arguments'] = process['argv'].slice(2);
  }
  module.exports = Module;
}
if (ENVIRONMENT_IS_SHELL) {
  Module['print'] = print;
  if (typeof printErr != 'undefined') Module['printErr'] = printErr; // not present in v8 or older sm
  Module['read'] = read;
  Module['readBinary'] = function(f) {
    return read(f, 'binary');
  };
  if (!Module['arguments']) {
    if (typeof scriptArgs != 'undefined') {
      Module['arguments'] = scriptArgs;
    } else if (typeof arguments != 'undefined') {
      Module['arguments'] = arguments;
    }
  }
  this['Module'] = Module;
}
if (ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_WORKER) {
  if (!Module['print']) {
    Module['print'] = function(x) {
      console.log(x);
    };
  }
  if (!Module['printErr']) {
    Module['printErr'] = function(x) {
      console.log(x);
    };
  }
  this['Module'] = Module;
}
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  Module['read'] = function(url) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, false);
    xhr.send(null);
    return xhr.responseText;
  };
  if (!Module['arguments']) {
    if (typeof arguments != 'undefined') {
      Module['arguments'] = arguments;
    }
  }
}
if (ENVIRONMENT_IS_WORKER) {
  // We can do very little here...
  var TRY_USE_DUMP = false;
  if (!Module['print']) {
    Module['print'] = (TRY_USE_DUMP && (typeof(dump) !== "undefined") ? (function(x) {
      dump(x);
    }) : (function(x) {
      // self.postMessage(x); // enable this if you want stdout to be sent as messages
    }));
  }
  Module['load'] = importScripts;
}
if (!ENVIRONMENT_IS_WORKER && !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_SHELL) {
  // Unreachable because SHELL is dependant on the others
  throw 'Unknown runtime environment. Where are we?';
}
function globalEval(x) {
  eval.call(null, x);
}
if (!Module['load'] == 'undefined' && Module['read']) {
  Module['load'] = function(f) {
    globalEval(Module['read'](f));
  };
}
if (!Module['print']) {
  Module['print'] = function(){};
}
if (!Module['printErr']) {
  Module['printErr'] = Module['print'];
}
if (!Module['arguments']) {
  Module['arguments'] = [];
}
// *** Environment setup code ***
// Closure helpers
Module.print = Module['print'];
Module.printErr = Module['printErr'];
// Callbacks
if (!Module['preRun']) Module['preRun'] = [];
if (!Module['postRun']) Module['postRun'] = [];
// === Auto-generated preamble library stuff ===
//========================================
// Runtime code shared with compiler
//========================================
var Runtime = {
  stackSave: function () {
    return STACKTOP;
  },
  stackRestore: function (stackTop) {
    STACKTOP = stackTop;
  },
  forceAlign: function (target, quantum) {
    quantum = quantum || 4;
    if (quantum == 1) return target;
    if (isNumber(target) && isNumber(quantum)) {
      return Math.ceil(target/quantum)*quantum;
    } else if (isNumber(quantum) && isPowerOfTwo(quantum)) {
      var logg = log2(quantum);
      return '((((' +target + ')+' + (quantum-1) + ')>>' + logg + ')<<' + logg + ')';
    }
    return 'Math.ceil((' + target + ')/' + quantum + ')*' + quantum;
  },
  isNumberType: function (type) {
    return type in Runtime.INT_TYPES || type in Runtime.FLOAT_TYPES;
  },
  isPointerType: function isPointerType(type) {
  return type[type.length-1] == '*';
},
  isStructType: function isStructType(type) {
  if (isPointerType(type)) return false;
  if (isArrayType(type)) return true;
  if (/<?{ ?[^}]* ?}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types
  // See comment in isStructPointerType()
  return type[0] == '%';
},
  INT_TYPES: {"i1":0,"i8":0,"i16":0,"i32":0,"i64":0},
  FLOAT_TYPES: {"float":0,"double":0},
  or64: function (x, y) {
    var l = (x | 0) | (y | 0);
    var h = (Math.round(x / 4294967296) | Math.round(y / 4294967296)) * 4294967296;
    return l + h;
  },
  and64: function (x, y) {
    var l = (x | 0) & (y | 0);
    var h = (Math.round(x / 4294967296) & Math.round(y / 4294967296)) * 4294967296;
    return l + h;
  },
  xor64: function (x, y) {
    var l = (x | 0) ^ (y | 0);
    var h = (Math.round(x / 4294967296) ^ Math.round(y / 4294967296)) * 4294967296;
    return l + h;
  },
  getNativeTypeSize: function (type, quantumSize) {
    if (Runtime.QUANTUM_SIZE == 1) return 1;
    var size = {
      '%i1': 1,
      '%i8': 1,
      '%i16': 2,
      '%i32': 4,
      '%i64': 8,
      "%float": 4,
      "%double": 8
    }['%'+type]; // add '%' since float and double confuse Closure compiler as keys, and also spidermonkey as a compiler will remove 's from '_i8' etc
    if (!size) {
      if (type.charAt(type.length-1) == '*') {
        size = Runtime.QUANTUM_SIZE; // A pointer
      } else if (type[0] == 'i') {
        var bits = parseInt(type.substr(1));
        assert(bits % 8 == 0);
        size = bits/8;
      }
    }
    return size;
  },
  getNativeFieldSize: function (type) {
    return Math.max(Runtime.getNativeTypeSize(type), Runtime.QUANTUM_SIZE);
  },
  dedup: function dedup(items, ident) {
  var seen = {};
  if (ident) {
    return items.filter(function(item) {
      if (seen[item[ident]]) return false;
      seen[item[ident]] = true;
      return true;
    });
  } else {
    return items.filter(function(item) {
      if (seen[item]) return false;
      seen[item] = true;
      return true;
    });
  }
},
  set: function set() {
  var args = typeof arguments[0] === 'object' ? arguments[0] : arguments;
  var ret = {};
  for (var i = 0; i < args.length; i++) {
    ret[args[i]] = 0;
  }
  return ret;
},
  STACK_ALIGN: 8,
  getAlignSize: function (type, size, vararg) {
    // we align i64s and doubles on 64-bit boundaries, unlike x86
    if (type == 'i64' || type == 'double' || vararg) return 8;
    if (!type) return Math.min(size, 8); // align structures internally to 64 bits
    return Math.min(size || (type ? Runtime.getNativeFieldSize(type) : 0), Runtime.QUANTUM_SIZE);
  },
  calculateStructAlignment: function calculateStructAlignment(type) {
    type.flatSize = 0;
    type.alignSize = 0;
    var diffs = [];
    var prev = -1;
    type.flatIndexes = type.fields.map(function(field) {
      var size, alignSize;
      if (Runtime.isNumberType(field) || Runtime.isPointerType(field)) {
        size = Runtime.getNativeTypeSize(field); // pack char; char; in structs, also char[X]s.
        alignSize = Runtime.getAlignSize(field, size);
      } else if (Runtime.isStructType(field)) {
        size = Types.types[field].flatSize;
        alignSize = Runtime.getAlignSize(null, Types.types[field].alignSize);
      } else if (field[0] == 'b') {
        // bN, large number field, like a [N x i8]
        size = field.substr(1)|0;
        alignSize = 1;
      } else {
        throw 'Unclear type in struct: ' + field + ', in ' + type.name_ + ' :: ' + dump(Types.types[type.name_]);
      }
      if (type.packed) alignSize = 1;
      type.alignSize = Math.max(type.alignSize, alignSize);
      var curr = Runtime.alignMemory(type.flatSize, alignSize); // if necessary, place this on aligned memory
      type.flatSize = curr + size;
      if (prev >= 0) {
        diffs.push(curr-prev);
      }
      prev = curr;
      return curr;
    });
    type.flatSize = Runtime.alignMemory(type.flatSize, type.alignSize);
    if (diffs.length == 0) {
      type.flatFactor = type.flatSize;
    } else if (Runtime.dedup(diffs).length == 1) {
      type.flatFactor = diffs[0];
    }
    type.needsFlattening = (type.flatFactor != 1);
    return type.flatIndexes;
  },
  generateStructInfo: function (struct, typeName, offset) {
    var type, alignment;
    if (typeName) {
      offset = offset || 0;
      type = (typeof Types === 'undefined' ? Runtime.typeInfo : Types.types)[typeName];
      if (!type) return null;
      if (type.fields.length != struct.length) {
        printErr('Number of named fields must match the type for ' + typeName + ': possibly duplicate struct names. Cannot return structInfo');
        return null;
      }
      alignment = type.flatIndexes;
    } else {
      var type = { fields: struct.map(function(item) { return item[0] }) };
      alignment = Runtime.calculateStructAlignment(type);
    }
    var ret = {
      __size__: type.flatSize
    };
    if (typeName) {
      struct.forEach(function(item, i) {
        if (typeof item === 'string') {
          ret[item] = alignment[i] + offset;
        } else {
          // embedded struct
          var key;
          for (var k in item) key = k;
          ret[key] = Runtime.generateStructInfo(item[key], type.fields[i], alignment[i]);
        }
      });
    } else {
      struct.forEach(function(item, i) {
        ret[item[1]] = alignment[i];
      });
    }
    return ret;
  },
  dynCall: function (sig, ptr, args) {
    if (args && args.length) {
      if (!args.splice) args = Array.prototype.slice.call(args);
      args.splice(0, 0, ptr);
      return Module['dynCall_' + sig].apply(null, args);
    } else {
      return Module['dynCall_' + sig].call(null, ptr);
    }
  },
  functionPointers: [],
  addFunction: function (func) {
    for (var i = 0; i < Runtime.functionPointers.length; i++) {
      if (!Runtime.functionPointers[i]) {
        Runtime.functionPointers[i] = func;
        return 2 + 2*i;
      }
    }
    throw 'Finished up all reserved function pointers. Use a higher value for RESERVED_FUNCTION_POINTERS.';
  },
  removeFunction: function (index) {
    Runtime.functionPointers[(index-2)/2] = null;
  },
  warnOnce: function (text) {
    if (!Runtime.warnOnce.shown) Runtime.warnOnce.shown = {};
    if (!Runtime.warnOnce.shown[text]) {
      Runtime.warnOnce.shown[text] = 1;
      Module.printErr(text);
    }
  },
  funcWrappers: {},
  getFuncWrapper: function (func, sig) {
    assert(sig);
    if (!Runtime.funcWrappers[func]) {
      Runtime.funcWrappers[func] = function() {
        return Runtime.dynCall(sig, func, arguments);
      };
    }
    return Runtime.funcWrappers[func];
  },
  UTF8Processor: function () {
    var buffer = [];
    var needed = 0;
    this.processCChar = function (code) {
      code = code & 0xff;
      if (needed) {
        buffer.push(code);
        needed--;
      }
      if (buffer.length == 0) {
        if (code < 128) return String.fromCharCode(code);
        buffer.push(code);
        if (code > 191 && code < 224) {
          needed = 1;
        } else {
          needed = 2;
        }
        return '';
      }
      if (needed > 0) return '';
      var c1 = buffer[0];
      var c2 = buffer[1];
      var c3 = buffer[2];
      var ret;
      if (c1 > 191 && c1 < 224) {
        ret = String.fromCharCode(((c1 & 31) << 6) | (c2 & 63));
      } else {
        ret = String.fromCharCode(((c1 & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
      }
      buffer.length = 0;
      return ret;
    }
    this.processJSString = function(string) {
      string = unescape(encodeURIComponent(string));
      var ret = [];
      for (var i = 0; i < string.length; i++) {
        ret.push(string.charCodeAt(i));
      }
      return ret;
    }
  },
  stackAlloc: function (size) { var ret = STACKTOP;STACKTOP = (STACKTOP + size)|0;STACKTOP = ((((STACKTOP)+7)>>3)<<3); return ret; },
  staticAlloc: function (size) { var ret = STATICTOP;STATICTOP = (STATICTOP + size)|0;STATICTOP = ((((STATICTOP)+7)>>3)<<3); return ret; },
  dynamicAlloc: function (size) { var ret = DYNAMICTOP;DYNAMICTOP = (DYNAMICTOP + size)|0;DYNAMICTOP = ((((DYNAMICTOP)+7)>>3)<<3); if (DYNAMICTOP >= TOTAL_MEMORY) enlargeMemory();; return ret; },
  alignMemory: function (size,quantum) { var ret = size = Math.ceil((size)/(quantum ? quantum : 8))*(quantum ? quantum : 8); return ret; },
  makeBigInt: function (low,high,unsigned) { var ret = (unsigned ? ((+(((low)>>>(0))))+((+(((high)>>>(0))))*(+(4294967296)))) : ((+(((low)>>>(0))))+((+(((high)|(0))))*(+(4294967296))))); return ret; },
  GLOBAL_BASE: 8,
  QUANTUM_SIZE: 4,
  __dummy__: 0
}
//========================================
// Runtime essentials
//========================================
var __THREW__ = 0; // Used in checking for thrown exceptions.
var ABORT = false; // whether we are quitting the application. no code should run after this. set in exit() and abort()
var undef = 0;
// tempInt is used for 32-bit signed values or smaller. tempBigInt is used
// for 32-bit unsigned values or more than 32 bits. TODO: audit all uses of tempInt
var tempValue, tempInt, tempBigInt, tempInt2, tempBigInt2, tempPair, tempBigIntI, tempBigIntR, tempBigIntS, tempBigIntP, tempBigIntD;
var tempI64, tempI64b;
var tempRet0, tempRet1, tempRet2, tempRet3, tempRet4, tempRet5, tempRet6, tempRet7, tempRet8, tempRet9;
function abort(text) {
  Module.print(text + ':\n' + (new Error).stack);
  ABORT = true;
  throw "Assertion: " + text;
}
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}
var globalScope = this;
// C calling interface. A convenient way to call C functions (in C files, or
// defined with extern "C").
//
// Note: LLVM optimizations can inline and remove functions, after which you will not be
//       able to call them. Closure can also do so. To avoid that, add your function to
//       the exports using something like
//
//         -s EXPORTED_FUNCTIONS='["_main", "_myfunc"]'
//
// @param ident      The name of the C function (note that C++ functions will be name-mangled - use extern "C")
// @param returnType The return type of the function, one of the JS types 'number', 'string' or 'array' (use 'number' for any C pointer, and
//                   'array' for JavaScript arrays and typed arrays; note that arrays are 8-bit).
// @param argTypes   An array of the types of arguments for the function (if there are no arguments, this can be ommitted). Types are as in returnType,
//                   except that 'array' is not possible (there is no way for us to know the length of the array)
// @param args       An array of the arguments to the function, as native JS values (as in returnType)
//                   Note that string arguments will be stored on the stack (the JS string will become a C string on the stack).
// @return           The return value, as a native JS value (as in returnType)
function ccall(ident, returnType, argTypes, args) {
  return ccallFunc(getCFunc(ident), returnType, argTypes, args);
}
Module["ccall"] = ccall;
// Returns the C function with a specified identifier (for C++, you need to do manual name mangling)
function getCFunc(ident) {
  try {
    var func = globalScope['Module']['_' + ident]; // closure exported function
    if (!func) func = eval('_' + ident); // explicit lookup
  } catch(e) {
  }
  assert(func, 'Cannot call unknown function ' + ident + ' (perhaps LLVM optimizations or closure removed it?)');
  return func;
}
// Internal function that does a C call using a function, not an identifier
function ccallFunc(func, returnType, argTypes, args) {
  var stack = 0;
  function toC(value, type) {
    if (type == 'string') {
      if (value === null || value === undefined || value === 0) return 0; // null string
      if (!stack) stack = Runtime.stackSave();
      var ret = Runtime.stackAlloc(value.length+1);
      writeStringToMemory(value, ret);
      return ret;
    } else if (type == 'array') {
      if (!stack) stack = Runtime.stackSave();
      var ret = Runtime.stackAlloc(value.length);
      writeArrayToMemory(value, ret);
      return ret;
    }
    return value;
  }
  function fromC(value, type) {
    if (type == 'string') {
      return Pointer_stringify(value);
    }
    assert(type != 'array');
    return value;
  }
  var i = 0;
  var cArgs = args ? args.map(function(arg) {
    return toC(arg, argTypes[i++]);
  }) : [];
  var ret = fromC(func.apply(null, cArgs), returnType);
  if (stack) Runtime.stackRestore(stack);
  return ret;
}
// Returns a native JS wrapper for a C function. This is similar to ccall, but
// returns a function you can call repeatedly in a normal way. For example:
//
//   var my_function = cwrap('my_c_function', 'number', ['number', 'number']);
//   alert(my_function(5, 22));
//   alert(my_function(99, 12));
//
function cwrap(ident, returnType, argTypes) {
  var func = getCFunc(ident);
  return function() {
    return ccallFunc(func, returnType, argTypes, Array.prototype.slice.call(arguments));
  }
}
Module["cwrap"] = cwrap;
// Sets a value in memory in a dynamic way at run-time. Uses the
// type data. This is the same as makeSetValue, except that
// makeSetValue is done at compile-time and generates the needed
// code then, whereas this function picks the right code at
// run-time.
// Note that setValue and getValue only do *aligned* writes and reads!
// Note that ccall uses JS types as for defining types, while setValue and
// getValue need LLVM types ('i8', 'i32') - this is a lower-level operation
function setValue(ptr, value, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
    switch(type) {
      case 'i1': HEAP8[(ptr)]=value; break;
      case 'i8': HEAP8[(ptr)]=value; break;
      case 'i16': HEAP16[((ptr)>>1)]=value; break;
      case 'i32': HEAP32[((ptr)>>2)]=value; break;
      case 'i64': (tempI64 = [value>>>0,((Math.min((+(Math.floor((value)/(+(4294967296))))), (+(4294967295))))|0)>>>0],HEAP32[((ptr)>>2)]=tempI64[0],HEAP32[(((ptr)+(4))>>2)]=tempI64[1]); break;
      case 'float': HEAPF32[((ptr)>>2)]=value; break;
      case 'double': HEAPF64[((ptr)>>3)]=value; break;
      default: abort('invalid type for setValue: ' + type);
    }
}
Module['setValue'] = setValue;
// Parallel to setValue.
function getValue(ptr, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
    switch(type) {
      case 'i1': return HEAP8[(ptr)];
      case 'i8': return HEAP8[(ptr)];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP32[((ptr)>>2)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return HEAPF64[((ptr)>>3)];
      default: abort('invalid type for setValue: ' + type);
    }
  return null;
}
Module['getValue'] = getValue;
var ALLOC_NORMAL = 0; // Tries to use _malloc()
var ALLOC_STACK = 1; // Lives for the duration of the current function call
var ALLOC_STATIC = 2; // Cannot be freed
var ALLOC_DYNAMIC = 3; // Cannot be freed except through sbrk
var ALLOC_NONE = 4; // Do not allocate
Module['ALLOC_NORMAL'] = ALLOC_NORMAL;
Module['ALLOC_STACK'] = ALLOC_STACK;
Module['ALLOC_STATIC'] = ALLOC_STATIC;
Module['ALLOC_DYNAMIC'] = ALLOC_DYNAMIC;
Module['ALLOC_NONE'] = ALLOC_NONE;
// allocate(): This is for internal use. You can use it yourself as well, but the interface
//             is a little tricky (see docs right below). The reason is that it is optimized
//             for multiple syntaxes to save space in generated code. So you should
//             normally not use allocate(), and instead allocate memory using _malloc(),
//             initialize it with setValue(), and so forth.
// @slab: An array of data, or a number. If a number, then the size of the block to allocate,
//        in *bytes* (note that this is sometimes confusing: the next parameter does not
//        affect this!)
// @types: Either an array of types, one for each byte (or 0 if no type at that position),
//         or a single type which is used for the entire block. This only matters if there
//         is initial data - if @slab is a number, then this does not matter at all and is
//         ignored.
// @allocator: How to allocate memory, see ALLOC_*
function allocate(slab, types, allocator, ptr) {
  var zeroinit, size;
  if (typeof slab === 'number') {
    zeroinit = true;
    size = slab;
  } else {
    zeroinit = false;
    size = slab.length;
  }
  var singleType = typeof types === 'string' ? types : null;
  var ret;
  if (allocator == ALLOC_NONE) {
    ret = ptr;
  } else {
    ret = [_malloc, Runtime.stackAlloc, Runtime.staticAlloc, Runtime.dynamicAlloc][allocator === undefined ? ALLOC_STATIC : allocator](Math.max(size, singleType ? 1 : types.length));
  }
  if (zeroinit) {
    var ptr = ret, stop;
    assert((ret & 3) == 0);
    stop = ret + (size & ~3);
    for (; ptr < stop; ptr += 4) {
      HEAP32[((ptr)>>2)]=0;
    }
    stop = ret + size;
    while (ptr < stop) {
      HEAP8[((ptr++)|0)]=0;
    }
    return ret;
  }
  if (singleType === 'i8') {
    if (slab.subarray || slab.slice) {
      HEAPU8.set(slab, ret);
    } else {
      HEAPU8.set(new Uint8Array(slab), ret);
    }
    return ret;
  }
  var i = 0, type, typeSize, previousType;
  while (i < size) {
    var curr = slab[i];
    if (typeof curr === 'function') {
      curr = Runtime.getFunctionIndex(curr);
    }
    type = singleType || types[i];
    if (type === 0) {
      i++;
      continue;
    }
    if (type == 'i64') type = 'i32'; // special case: we have one i32 here, and one i32 later
    setValue(ret+i, curr, type);
    // no need to look up size unless type changes, so cache it
    if (previousType !== type) {
      typeSize = Runtime.getNativeTypeSize(type);
      previousType = type;
    }
    i += typeSize;
  }
  return ret;
}
Module['allocate'] = allocate;
function Pointer_stringify(ptr, /* optional */ length) {
  // Find the length, and check for UTF while doing so
  var hasUtf = false;
  var t;
  var i = 0;
  while (1) {
    t = HEAPU8[(((ptr)+(i))|0)];
    if (t >= 128) hasUtf = true;
    else if (t == 0 && !length) break;
    i++;
    if (length && i == length) break;
  }
  if (!length) length = i;
  var ret = '';
  if (!hasUtf) {
    var MAX_CHUNK = 1024; // split up into chunks, because .apply on a huge string can overflow the stack
    var curr;
    while (length > 0) {
      curr = String.fromCharCode.apply(String, HEAPU8.subarray(ptr, ptr + Math.min(length, MAX_CHUNK)));
      ret = ret ? ret + curr : curr;
      ptr += MAX_CHUNK;
      length -= MAX_CHUNK;
    }
    return ret;
  }
  var utf8 = new Runtime.UTF8Processor();
  for (i = 0; i < length; i++) {
    t = HEAPU8[(((ptr)+(i))|0)];
    ret += utf8.processCChar(t);
  }
  return ret;
}
Module['Pointer_stringify'] = Pointer_stringify;
// Memory management
var PAGE_SIZE = 4096;
function alignMemoryPage(x) {
  return ((x+4095)>>12)<<12;
}
var HEAP;
var HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;
var STATIC_BASE = 0, STATICTOP = 0, staticSealed = false; // static area
var STACK_BASE = 0, STACKTOP = 0, STACK_MAX = 0; // stack area
var DYNAMIC_BASE = 0, DYNAMICTOP = 0; // dynamic area handled by sbrk
function enlargeMemory() {
  abort('Cannot enlarge memory arrays in asm.js. Either (1) compile with -s TOTAL_MEMORY=X with X higher than the current value, or (2) set Module.TOTAL_MEMORY before the program runs.');
}
var TOTAL_STACK = Module['TOTAL_STACK'] || 5242880;
var TOTAL_MEMORY = Module['TOTAL_MEMORY'] || 16777216;
var FAST_MEMORY = Module['FAST_MEMORY'] || 2097152;
// Initialize the runtime's memory
// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
assert(!!Int32Array && !!Float64Array && !!(new Int32Array(1)['subarray']) && !!(new Int32Array(1)['set']),
       'Cannot fallback to non-typed array case: Code is too specialized');
var buffer = new ArrayBuffer(TOTAL_MEMORY);
HEAP8 = new Int8Array(buffer);
HEAP16 = new Int16Array(buffer);
HEAP32 = new Int32Array(buffer);
HEAPU8 = new Uint8Array(buffer);
HEAPU16 = new Uint16Array(buffer);
HEAPU32 = new Uint32Array(buffer);
HEAPF32 = new Float32Array(buffer);
HEAPF64 = new Float64Array(buffer);
// Endianness check (note: assumes compiler arch was little-endian)
HEAP32[0] = 255;
assert(HEAPU8[0] === 255 && HEAPU8[3] === 0, 'Typed arrays 2 must be run on a little-endian system');
Module['HEAP'] = HEAP;
Module['HEAP8'] = HEAP8;
Module['HEAP16'] = HEAP16;
Module['HEAP32'] = HEAP32;
Module['HEAPU8'] = HEAPU8;
Module['HEAPU16'] = HEAPU16;
Module['HEAPU32'] = HEAPU32;
Module['HEAPF32'] = HEAPF32;
Module['HEAPF64'] = HEAPF64;
function callRuntimeCallbacks(callbacks) {
  while(callbacks.length > 0) {
    var callback = callbacks.shift();
    if (typeof callback == 'function') {
      callback();
      continue;
    }
    var func = callback.func;
    if (typeof func === 'number') {
      if (callback.arg === undefined) {
        Runtime.dynCall('v', func);
      } else {
        Runtime.dynCall('vi', func, [callback.arg]);
      }
    } else {
      func(callback.arg === undefined ? null : callback.arg);
    }
  }
}
var __ATINIT__ = []; // functions called during startup
var __ATMAIN__ = []; // functions called when main() is to be run
var __ATEXIT__ = []; // functions called during shutdown
var runtimeInitialized = false;
function ensureInitRuntime() {
  if (runtimeInitialized) return;
  runtimeInitialized = true;
  callRuntimeCallbacks(__ATINIT__);
}
function preMain() {
  callRuntimeCallbacks(__ATMAIN__);
}
function exitRuntime() {
  callRuntimeCallbacks(__ATEXIT__);
}
// Tools
// This processes a JS string into a C-line array of numbers, 0-terminated.
// For LLVM-originating strings, see parser.js:parseLLVMString function
function intArrayFromString(stringy, dontAddNull, length /* optional */) {
  var ret = (new Runtime.UTF8Processor()).processJSString(stringy);
  if (length) {
    ret.length = length;
  }
  if (!dontAddNull) {
    ret.push(0);
  }
  return ret;
}
Module['intArrayFromString'] = intArrayFromString;
function intArrayToString(array) {
  var ret = [];
  for (var i = 0; i < array.length; i++) {
    var chr = array[i];
    if (chr > 0xFF) {
      chr &= 0xFF;
    }
    ret.push(String.fromCharCode(chr));
  }
  return ret.join('');
}
Module['intArrayToString'] = intArrayToString;
// Write a Javascript array to somewhere in the heap
function writeStringToMemory(string, buffer, dontAddNull) {
  var array = intArrayFromString(string, dontAddNull);
  var i = 0;
  while (i < array.length) {
    var chr = array[i];
    HEAP8[(((buffer)+(i))|0)]=chr
    i = i + 1;
  }
}
Module['writeStringToMemory'] = writeStringToMemory;
function writeArrayToMemory(array, buffer) {
  for (var i = 0; i < array.length; i++) {
    HEAP8[(((buffer)+(i))|0)]=array[i];
  }
}
Module['writeArrayToMemory'] = writeArrayToMemory;
function unSign(value, bits, ignore, sig) {
  if (value >= 0) {
    return value;
  }
  return bits <= 32 ? 2*Math.abs(1 << (bits-1)) + value // Need some trickery, since if bits == 32, we are right at the limit of the bits JS uses in bitshifts
                    : Math.pow(2, bits)         + value;
}
function reSign(value, bits, ignore, sig) {
  if (value <= 0) {
    return value;
  }
  var half = bits <= 32 ? Math.abs(1 << (bits-1)) // abs is needed if bits == 32
                        : Math.pow(2, bits-1);
  if (value >= half && (bits <= 32 || value > half)) { // for huge values, we can hit the precision limit and always get true here. so don't do that
                                                       // but, in general there is no perfect solution here. With 64-bit ints, we get rounding and errors
                                                       // TODO: In i64 mode 1, resign the two parts separately and safely
    value = -2*half + value; // Cannot bitshift half, as it may be at the limit of the bits JS uses in bitshifts
  }
  return value;
}
if (!Math['imul']) Math['imul'] = function(a, b) {
  var ah  = a >>> 16;
  var al = a & 0xffff;
  var bh  = b >>> 16;
  var bl = b & 0xffff;
  return (al*bl + ((ah*bl + al*bh) << 16))|0;
};
// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// PRE_RUN_ADDITIONS (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var runDependencyTracking = {};
var calledInit = false, calledRun = false;
var runDependencyWatcher = null;
function addRunDependency(id) {
  runDependencies++;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
  } else {
    Module.printErr('warning: run dependency added without ID');
  }
}
Module['addRunDependency'] = addRunDependency;
function removeRunDependency(id) {
  runDependencies--;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    Module.printErr('warning: run dependency removed without ID');
  }
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    } 
    // If run has never been called, and we should call run (INVOKE_RUN is true, and Module.noInitialRun is not false)
    if (!calledRun && shouldRunNow) run();
  }
}
Module['removeRunDependency'] = removeRunDependency;
Module["preloadedImages"] = {}; // maps url to image data
Module["preloadedAudios"] = {}; // maps url to audio data
function addPreRun(func) {
  if (!Module['preRun']) Module['preRun'] = [];
  else if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
  Module['preRun'].push(func);
}
function loadMemoryInitializer(filename) {
  function applyData(data) {
    HEAPU8.set(data, STATIC_BASE);
  }
  // always do this asynchronously, to keep shell and web as similar as possible
  addPreRun(function() {
    if (ENVIRONMENT_IS_NODE || ENVIRONMENT_IS_SHELL) {
      applyData(Module['readBinary'](filename));
    } else {
      Browser.asyncLoad(filename, function(data) {
        applyData(data);
      }, function(data) {
        throw 'could not load memory initializer ' + filename;
      });
    }
  });
}
// === Body ===
STATIC_BASE = 8;
STATICTOP = STATIC_BASE + 15192;
/* global initializers */ __ATINIT__.push({ func: function() { runPostSets() } });
/* memory initializer */ allocate([176,50,0,0,248,52,0,0,128,57,0,0,232,51,0,0,152,51,0,0,80,51,0,0,8,51,0,0,160,50,0,0,72,50,0,0,128,57,0,0,12,0,8,0,140,0,8,0,76,0,8,0,204,0,8,0,44,0,8,0,172,0,8,0,108,0,8,0,236,0,8,0,28,0,8,0,156,0,8,0,92,0,8,0,220,0,8,0,60,0,8,0,188,0,8,0,124,0,8,0,252,0,8,0,2,0,8,0,130,0,8,0,66,0,8,0,194,0,8,0,34,0,8,0,162,0,8,0,98,0,8,0,226,0,8,0,18,0,8,0,146,0,8,0,82,0,8,0,210,0,8,0,50,0,8,0,178,0,8,0,114,0,8,0,242,0,8,0,10,0,8,0,138,0,8,0,74,0,8,0,202,0,8,0,42,0,8,0,170,0,8,0,106,0,8,0,234,0,8,0,26,0,8,0,154,0,8,0,90,0,8,0,218,0,8,0,58,0,8,0,186,0,8,0,122,0,8,0,250,0,8,0,6,0,8,0,134,0,8,0,70,0,8,0,198,0,8,0,38,0,8,0,166,0,8,0,102,0,8,0,230,0,8,0,22,0,8,0,150,0,8,0,86,0,8,0,214,0,8,0,54,0,8,0,182,0,8,0,118,0,8,0,246,0,8,0,14,0,8,0,142,0,8,0,78,0,8,0,206,0,8,0,46,0,8,0,174,0,8,0,110,0,8,0,238,0,8,0,30,0,8,0,158,0,8,0,94,0,8,0,222,0,8,0,62,0,8,0,190,0,8,0,126,0,8,0,254,0,8,0,1,0,8,0,129,0,8,0,65,0,8,0,193,0,8,0,33,0,8,0,161,0,8,0,97,0,8,0,225,0,8,0,17,0,8,0,145,0,8,0,81,0,8,0,209,0,8,0,49,0,8,0,177,0,8,0,113,0,8,0,241,0,8,0,9,0,8,0,137,0,8,0,73,0,8,0,201,0,8,0,41,0,8,0,169,0,8,0,105,0,8,0,233,0,8,0,25,0,8,0,153,0,8,0,89,0,8,0,217,0,8,0,57,0,8,0,185,0,8,0,121,0,8,0,249,0,8,0,5,0,8,0,133,0,8,0,69,0,8,0,197,0,8,0,37,0,8,0,165,0,8,0,101,0,8,0,229,0,8,0,21,0,8,0,149,0,8,0,85,0,8,0,213,0,8,0,53,0,8,0,181,0,8,0,117,0,8,0,245,0,8,0,13,0,8,0,141,0,8,0,77,0,8,0,205,0,8,0,45,0,8,0,173,0,8,0,109,0,8,0,237,0,8,0,29,0,8,0,157,0,8,0,93,0,8,0,221,0,8,0,61,0,8,0,189,0,8,0,125,0,8,0,253,0,8,0,19,0,9,0,19,1,9,0,147,0,9,0,147,1,9,0,83,0,9,0,83,1,9,0,211,0,9,0,211,1,9,0,51,0,9,0,51,1,9,0,179,0,9,0,179,1,9,0,115,0,9,0,115,1,9,0,243,0,9,0,243,1,9,0,11,0,9,0,11,1,9,0,139,0,9,0,139,1,9,0,75,0,9,0,75,1,9,0,203,0,9,0,203,1,9,0,43,0,9,0,43,1,9,0,171,0,9,0,171,1,9,0,107,0,9,0,107,1,9,0,235,0,9,0,235,1,9,0,27,0,9,0,27,1,9,0,155,0,9,0,155,1,9,0,91,0,9,0,91,1,9,0,219,0,9,0,219,1,9,0,59,0,9,0,59,1,9,0,187,0,9,0,187,1,9,0,123,0,9,0,123,1,9,0,251,0,9,0,251,1,9,0,7,0,9,0,7,1,9,0,135,0,9,0,135,1,9,0,71,0,9,0,71,1,9,0,199,0,9,0,199,1,9,0,39,0,9,0,39,1,9,0,167,0,9,0,167,1,9,0,103,0,9,0,103,1,9,0,231,0,9,0,231,1,9,0,23,0,9,0,23,1,9,0,151,0,9,0,151,1,9,0,87,0,9,0,87,1,9,0,215,0,9,0,215,1,9,0,55,0,9,0,55,1,9,0,183,0,9,0,183,1,9,0,119,0,9,0,119,1,9,0,247,0,9,0,247,1,9,0,15,0,9,0,15,1,9,0,143,0,9,0,143,1,9,0,79,0,9,0,79,1,9,0,207,0,9,0,207,1,9,0,47,0,9,0,47,1,9,0,175,0,9,0,175,1,9,0,111,0,9,0,111,1,9,0,239,0,9,0,239,1,9,0,31,0,9,0,31,1,9,0,159,0,9,0,159,1,9,0,95,0,9,0,95,1,9,0,223,0,9,0,223,1,9,0,63,0,9,0,63,1,9,0,191,0,9,0,191,1,9,0,127,0,9,0,127,1,9,0,255,0,9,0,255,1,9,0,0,0,7,0,64,0,7,0,32,0,7,0,96,0,7,0,16,0,7,0,80,0,7,0,48,0,7,0,112,0,7,0,8,0,7,0,72,0,7,0,40,0,7,0,104,0,7,0,24,0,7,0,88,0,7,0,56,0,7,0,120,0,7,0,4,0,7,0,68,0,7,0,36,0,7,0,100,0,7,0,20,0,7,0,84,0,7,0,52,0,7,0,116,0,7,0,3,0,8,0,131,0,8,0,67,0,8,0,195,0,8,0,35,0,8,0,163,0,8,0,99,0,8,0,227,0,8,0,48,0,0,0,40,15,0,0,1,1,0,0,30,1,0,0,15,0,0,0,0,0,0,0,0,0,5,0,16,0,5,0,8,0,5,0,24,0,5,0,4,0,5,0,20,0,5,0,12,0,5,0,28,0,5,0,2,0,5,0,18,0,5,0,10,0,5,0,26,0,5,0,6,0,5,0,22,0,5,0,14,0,5,0,30,0,5,0,1,0,5,0,17,0,5,0,9,0,5,0,25,0,5,0,5,0,5,0,21,0,5,0,13,0,5,0,29,0,5,0,3,0,5,0,19,0,5,0,11,0,5,0,27,0,5,0,7,0,5,0,23,0,5,0,200,4,0,0,160,15,0,0,0,0,0,0,30,0,0,0,15,0,0,0,0,0,0,0,0,0,0,0,24,16,0,0,0,0,0,0,19,0,0,0,7,0,0,0,0,0,0,0,2,0,0,0,4,0,0,0,4,0,0,0,2,0,0,0,16,0,16,0,16,0,16,0,16,0,16,0,16,0,16,0,17,0,17,0,17,0,17,0,18,0,18,0,18,0,18,0,19,0,19,0,19,0,19,0,20,0,20,0,20,0,20,0,21,0,21,0,21,0,21,0,16,0,73,0,195,0,0,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10,0,11,0,13,0,15,0,17,0,19,0,23,0,27,0,31,0,35,0,43,0,51,0,59,0,67,0,83,0,99,0,115,0,131,0,163,0,195,0,227,0,2,1,0,0,0,0,0,0,16,0,16,0,16,0,16,0,17,0,17,0,18,0,18,0,19,0,19,0,20,0,20,0,21,0,21,0,22,0,22,0,23,0,23,0,24,0,24,0,25,0,25,0,26,0,26,0,27,0,27,0,28,0,28,0,29,0,29,0,64,0,64,0,1,0,2,0,3,0,4,0,5,0,7,0,9,0,13,0,17,0,25,0,33,0,49,0,65,0,97,0,129,0,193,0,1,1,129,1,1,2,1,3,1,4,1,6,1,8,1,12,1,16,1,24,1,32,1,48,1,64,1,96,0,0,0,0,16,0,17,0,18,0,0,0,8,0,7,0,9,0,6,0,10,0,5,0,11,0,4,0,12,0,3,0,13,0,2,0,14,0,1,0,15,0,0,0,96,7,0,0,0,8,80,0,0,8,16,0,20,8,115,0,18,7,31,0,0,8,112,0,0,8,48,0,0,9,192,0,16,7,10,0,0,8,96,0,0,8,32,0,0,9,160,0,0,8,0,0,0,8,128,0,0,8,64,0,0,9,224,0,16,7,6,0,0,8,88,0,0,8,24,0,0,9,144,0,19,7,59,0,0,8,120,0,0,8,56,0,0,9,208,0,17,7,17,0,0,8,104,0,0,8,40,0,0,9,176,0,0,8,8,0,0,8,136,0,0,8,72,0,0,9,240,0,16,7,4,0,0,8,84,0,0,8,20,0,21,8,227,0,19,7,43,0,0,8,116,0,0,8,52,0,0,9,200,0,17,7,13,0,0,8,100,0,0,8,36,0,0,9,168,0,0,8,4,0,0,8,132,0,0,8,68,0,0,9,232,0,16,7,8,0,0,8,92,0,0,8,28,0,0,9,152,0,20,7,83,0,0,8,124,0,0,8,60,0,0,9,216,0,18,7,23,0,0,8,108,0,0,8,44,0,0,9,184,0,0,8,12,0,0,8,140,0,0,8,76,0,0,9,248,0,16,7,3,0,0,8,82,0,0,8,18,0,21,8,163,0,19,7,35,0,0,8,114,0,0,8,50,0,0,9,196,0,17,7,11,0,0,8,98,0,0,8,34,0,0,9,164,0,0,8,2,0,0,8,130,0,0,8,66,0,0,9,228,0,16,7,7,0,0,8,90,0,0,8,26,0,0,9,148,0,20,7,67,0,0,8,122,0,0,8,58,0,0,9,212,0,18,7,19,0,0,8,106,0,0,8,42,0,0,9,180,0,0,8,10,0,0,8,138,0,0,8,74,0,0,9,244,0,16,7,5,0,0,8,86,0,0,8,22,0,64,8,0,0,19,7,51,0,0,8,118,0,0,8,54,0,0,9,204,0,17,7,15,0,0,8,102,0,0,8,38,0,0,9,172,0,0,8,6,0,0,8,134,0,0,8,70,0,0,9,236,0,16,7,9,0,0,8,94,0,0,8,30,0,0,9,156,0,20,7,99,0,0,8,126,0,0,8,62,0,0,9,220,0,18,7,27,0,0,8,110,0,0,8,46,0,0,9,188,0,0,8,14,0,0,8,142,0,0,8,78,0,0,9,252,0,96,7,0,0,0,8,81,0,0,8,17,0,21,8,131,0,18,7,31,0,0,8,113,0,0,8,49,0,0,9,194,0,16,7,10,0,0,8,97,0,0,8,33,0,0,9,162,0,0,8,1,0,0,8,129,0,0,8,65,0,0,9,226,0,16,7,6,0,0,8,89,0,0,8,25,0,0,9,146,0,19,7,59,0,0,8,121,0,0,8,57,0,0,9,210,0,17,7,17,0,0,8,105,0,0,8,41,0,0,9,178,0,0,8,9,0,0,8,137,0,0,8,73,0,0,9,242,0,16,7,4,0,0,8,85,0,0,8,21,0,16,8,2,1,19,7,43,0,0,8,117,0,0,8,53,0,0,9,202,0,17,7,13,0,0,8,101,0,0,8,37,0,0,9,170,0,0,8,5,0,0,8,133,0,0,8,69,0,0,9,234,0,16,7,8,0,0,8,93,0,0,8,29,0,0,9,154,0,20,7,83,0,0,8,125,0,0,8,61,0,0,9,218,0,18,7,23,0,0,8,109,0,0,8,45,0,0,9,186,0,0,8,13,0,0,8,141,0,0,8,77,0,0,9,250,0,16,7,3,0,0,8,83,0,0,8,19,0,21,8,195,0,19,7,35,0,0,8,115,0,0,8,51,0,0,9,198,0,17,7,11,0,0,8,99,0,0,8,35,0,0,9,166,0,0,8,3,0,0,8,131,0,0,8,67,0,0,9,230,0,16,7,7,0,0,8,91,0,0,8,27,0,0,9,150,0,20,7,67,0,0,8,123,0,0,8,59,0,0,9,214,0,18,7,19,0,0,8,107,0,0,8,43,0,0,9,182,0,0,8,11,0,0,8,139,0,0,8,75,0,0,9,246,0,16,7,5,0,0,8,87,0,0,8,23,0,64,8,0,0,19,7,51,0,0,8,119,0,0,8,55,0,0,9,206,0,17,7,15,0,0,8,103,0,0,8,39,0,0,9,174,0,0,8,7,0,0,8,135,0,0,8,71,0,0,9,238,0,16,7,9,0,0,8,95,0,0,8,31,0,0,9,158,0,20,7,99,0,0,8,127,0,0,8,63,0,0,9,222,0,18,7,27,0,0,8,111,0,0,8,47,0,0,9,190,0,0,8,15,0,0,8,143,0,0,8,79,0,0,9,254,0,96,7,0,0,0,8,80,0,0,8,16,0,20,8,115,0,18,7,31,0,0,8,112,0,0,8,48,0,0,9,193,0,16,7,10,0,0,8,96,0,0,8,32,0,0,9,161,0,0,8,0,0,0,8,128,0,0,8,64,0,0,9,225,0,16,7,6,0,0,8,88,0,0,8,24,0,0,9,145,0,19,7,59,0,0,8,120,0,0,8,56,0,0,9,209,0,17,7,17,0,0,8,104,0,0,8,40,0,0,9,177,0,0,8,8,0,0,8,136,0,0,8,72,0,0,9,241,0,16,7,4,0,0,8,84,0,0,8,20,0,21,8,227,0,19,7,43,0,0,8,116,0,0,8,52,0,0,9,201,0,17,7,13,0,0,8,100,0,0,8,36,0,0,9,169,0,0,8,4,0,0,8,132,0,0,8,68,0,0,9,233,0,16,7,8,0,0,8,92,0,0,8,28,0,0,9,153,0,20,7,83,0,0,8,124,0,0,8,60,0,0,9,217,0,18,7,23,0,0,8,108,0,0,8,44,0,0,9,185,0,0,8,12,0,0,8,140,0,0,8,76,0,0,9,249,0,16,7,3,0,0,8,82,0,0,8,18,0,21,8,163,0,19,7,35,0,0,8,114,0,0,8,50,0,0,9,197,0,17,7,11,0,0,8,98,0,0,8,34,0,0,9,165,0,0,8,2,0,0,8,130,0,0,8,66,0,0,9,229,0,16,7,7,0,0,8,90,0,0,8,26,0,0,9,149,0,20,7,67,0,0,8,122,0,0,8,58,0,0,9,213,0,18,7,19,0,0,8,106,0,0,8,42,0,0,9,181,0,0,8,10,0,0,8,138,0,0,8,74,0,0,9,245,0,16,7,5,0,0,8,86,0,0,8,22,0,64,8,0,0,19,7,51,0,0,8,118,0,0,8,54,0,0,9,205,0,17,7,15,0,0,8,102,0,0,8,38,0,0,9,173,0,0,8,6,0,0,8,134,0,0,8,70,0,0,9,237,0,16,7,9,0,0,8,94,0,0,8,30,0,0,9,157,0,20,7,99,0,0,8,126,0,0,8,62,0,0,9,221,0,18,7,27,0,0,8,110,0,0,8,46,0,0,9,189,0,0,8,14,0,0,8,142,0,0,8,78,0,0,9,253,0,96,7,0,0,0,8,81,0,0,8,17,0,21,8,131,0,18,7,31,0,0,8,113,0,0,8,49,0,0,9,195,0,16,7,10,0,0,8,97,0,0,8,33,0,0,9,163,0,0,8,1,0,0,8,129,0,0,8,65,0,0,9,227,0,16,7,6,0,0,8,89,0,0,8,25,0,0,9,147,0,19,7,59,0,0,8,121,0,0,8,57,0,0,9,211,0,17,7,17,0,0,8,105,0,0,8,41,0,0,9,179,0,0,8,9,0,0,8,137,0,0,8,73,0,0,9,243,0,16,7,4,0,0,8,85,0,0,8,21,0,16,8,2,1,19,7,43,0,0,8,117,0,0,8,53,0,0,9,203,0,17,7,13,0,0,8,101,0,0,8,37,0,0,9,171,0,0,8,5,0,0,8,133,0,0,8,69,0,0,9,235,0,16,7,8,0,0,8,93,0,0,8,29,0,0,9,155,0,20,7,83,0,0,8,125,0,0,8,61,0,0,9,219,0,18,7,23,0,0,8,109,0,0,8,45,0,0,9,187,0,0,8,13,0,0,8,141,0,0,8,77,0,0,9,251,0,16,7,3,0,0,8,83,0,0,8,19,0,21,8,195,0,19,7,35,0,0,8,115,0,0,8,51,0,0,9,199,0,17,7,11,0,0,8,99,0,0,8,35,0,0,9,167,0,0,8,3,0,0,8,131,0,0,8,67,0,0,9,231,0,16,7,7,0,0,8,91,0,0,8,27,0,0,9,151,0,20,7,67,0,0,8,123,0,0,8,59,0,0,9,215,0,18,7,19,0,0,8,107,0,0,8,43,0,0,9,183,0,0,8,11,0,0,8,139,0,0,8,75,0,0,9,247,0,16,7,5,0,0,8,87,0,0,8,23,0,64,8,0,0,19,7,51,0,0,8,119,0,0,8,55,0,0,9,207,0,17,7,15,0,0,8,103,0,0,8,39,0,0,9,175,0,0,8,7,0,0,8,135,0,0,8,71,0,0,9,239,0,16,7,9,0,0,8,95,0,0,8,31,0,0,9,159,0,20,7,99,0,0,8,127,0,0,8,63,0,0,9,223,0,18,7,27,0,0,8,111,0,0,8,47,0,0,9,191,0,0,8,15,0,0,8,143,0,0,8,79,0,0,9,255,0,16,5,1,0,23,5,1,1,19,5,17,0,27,5,1,16,17,5,5,0,25,5,1,4,21,5,65,0,29,5,1,64,16,5,3,0,24,5,1,2,20,5,33,0,28,5,1,32,18,5,9,0,26,5,1,8,22,5,129,0,64,5,0,0,16,5,2,0,23,5,129,1,19,5,25,0,27,5,1,24,17,5,7,0,25,5,1,6,21,5,97,0,29,5,1,96,16,5,4,0,24,5,1,3,20,5,49,0,28,5,1,48,18,5,13,0,26,5,1,12,22,5,193,0,64,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,2,0,0,0,2,0,0,0,2,0,0,0,2,0,0,0,3,0,0,0,3,0,0,0,3,0,0,0,3,0,0,0,4,0,0,0,4,0,0,0,4,0,0,0,4,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,2,0,0,0,2,0,0,0,3,0,0,0,3,0,0,0,4,0,0,0,4,0,0,0,5,0,0,0,5,0,0,0,6,0,0,0,6,0,0,0,7,0,0,0,7,0,0,0,8,0,0,0,8,0,0,0,9,0,0,0,9,0,0,0,10,0,0,0,10,0,0,0,11,0,0,0,11,0,0,0,12,0,0,0,12,0,0,0,13,0,0,0,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,3,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,150,48,7,119,44,97,14,238,186,81,9,153,25,196,109,7,143,244,106,112,53,165,99,233,163,149,100,158,50,136,219,14,164,184,220,121,30,233,213,224,136,217,210,151,43,76,182,9,189,124,177,126,7,45,184,231,145,29,191,144,100,16,183,29,242,32,176,106,72,113,185,243,222,65,190,132,125,212,218,26,235,228,221,109,81,181,212,244,199,133,211,131,86,152,108,19,192,168,107,100,122,249,98,253,236,201,101,138,79,92,1,20,217,108,6,99,99,61,15,250,245,13,8,141,200,32,110,59,94,16,105,76,228,65,96,213,114,113,103,162,209,228,3,60,71,212,4,75,253,133,13,210,107,181,10,165,250,168,181,53,108,152,178,66,214,201,187,219,64,249,188,172,227,108,216,50,117,92,223,69,207,13,214,220,89,61,209,171,172,48,217,38,58,0,222,81,128,81,215,200,22,97,208,191,181,244,180,33,35,196,179,86,153,149,186,207,15,165,189,184,158,184,2,40,8,136,5,95,178,217,12,198,36,233,11,177,135,124,111,47,17,76,104,88,171,29,97,193,61,45,102,182,144,65,220,118,6,113,219,1,188,32,210,152,42,16,213,239,137,133,177,113,31,181,182,6,165,228,191,159,51,212,184,232,162,201,7,120,52,249,0,15,142,168,9,150,24,152,14,225,187,13,106,127,45,61,109,8,151,108,100,145,1,92,99,230,244,81,107,107,98,97,108,28,216,48,101,133,78,0,98,242,237,149,6,108,123,165,1,27,193,244,8,130,87,196,15,245,198,217,176,101,80,233,183,18,234,184,190,139,124,136,185,252,223,29,221,98,73,45,218,21,243,124,211,140,101,76,212,251,88,97,178,77,206,81,181,58,116,0,188,163,226,48,187,212,65,165,223,74,215,149,216,61,109,196,209,164,251,244,214,211,106,233,105,67,252,217,110,52,70,136,103,173,208,184,96,218,115,45,4,68,229,29,3,51,95,76,10,170,201,124,13,221,60,113,5,80,170,65,2,39,16,16,11,190,134,32,12,201,37,181,104,87,179,133,111,32,9,212,102,185,159,228,97,206,14,249,222,94,152,201,217,41,34,152,208,176,180,168,215,199,23,61,179,89,129,13,180,46,59,92,189,183,173,108,186,192,32,131,184,237,182,179,191,154,12,226,182,3,154,210,177,116,57,71,213,234,175,119,210,157,21,38,219,4,131,22,220,115,18,11,99,227,132,59,100,148,62,106,109,13,168,90,106,122,11,207,14,228,157,255,9,147,39,174,0,10,177,158,7,125,68,147,15,240,210,163,8,135,104,242,1,30,254,194,6,105,93,87,98,247,203,103,101,128,113,54,108,25,231,6,107,110,118,27,212,254,224,43,211,137,90,122,218,16,204,74,221,103,111,223,185,249,249,239,190,142,67,190,183,23,213,142,176,96,232,163,214,214,126,147,209,161,196,194,216,56,82,242,223,79,241,103,187,209,103,87,188,166,221,6,181,63,75,54,178,72,218,43,13,216,76,27,10,175,246,74,3,54,96,122,4,65,195,239,96,223,85,223,103,168,239,142,110,49,121,190,105,70,140,179,97,203,26,131,102,188,160,210,111,37,54,226,104,82,149,119,12,204,3,71,11,187,185,22,2,34,47,38,5,85,190,59,186,197,40,11,189,178,146,90,180,43,4,106,179,92,167,255,215,194,49,207,208,181,139,158,217,44,29,174,222,91,176,194,100,155,38,242,99,236,156,163,106,117,10,147,109,2,169,6,9,156,63,54,14,235,133,103,7,114,19,87,0,5,130,74,191,149,20,122,184,226,174,43,177,123,56,27,182,12,155,142,210,146,13,190,213,229,183,239,220,124,33,223,219,11,212,210,211,134,66,226,212,241,248,179,221,104,110,131,218,31,205,22,190,129,91,38,185,246,225,119,176,111,119,71,183,24,230,90,8,136,112,106,15,255,202,59,6,102,92,11,1,17,255,158,101,143,105,174,98,248,211,255,107,97,69,207,108,22,120,226,10,160,238,210,13,215,84,131,4,78,194,179,3,57,97,38,103,167,247,22,96,208,77,71,105,73,219,119,110,62,74,106,209,174,220,90,214,217,102,11,223,64,240,59,216,55,83,174,188,169,197,158,187,222,127,207,178,71,233,255,181,48,28,242,189,189,138,194,186,202,48,147,179,83,166,163,180,36,5,54,208,186,147,6,215,205,41,87,222,84,191,103,217,35,46,122,102,179,184,74,97,196,2,27,104,93,148,43,111,42,55,190,11,180,161,142,12,195,27,223,5,90,141,239,2,45,0,0,0,0,65,49,27,25,130,98,54,50,195,83,45,43,4,197,108,100,69,244,119,125,134,167,90,86,199,150,65,79,8,138,217,200,73,187,194,209,138,232,239,250,203,217,244,227,12,79,181,172,77,126,174,181,142,45,131,158,207,28,152,135,81,18,194,74,16,35,217,83,211,112,244,120,146,65,239,97,85,215,174,46,20,230,181,55,215,181,152,28,150,132,131,5,89,152,27,130,24,169,0,155,219,250,45,176,154,203,54,169,93,93,119,230,28,108,108,255,223,63,65,212,158,14,90,205,162,36,132,149,227,21,159,140,32,70,178,167,97,119,169,190,166,225,232,241,231,208,243,232,36,131,222,195,101,178,197,218,170,174,93,93,235,159,70,68,40,204,107,111,105,253,112,118,174,107,49,57,239,90,42,32,44,9,7,11,109,56,28,18,243,54,70,223,178,7,93,198,113,84,112,237,48,101,107,244,247,243,42,187,182,194,49,162,117,145,28,137,52,160,7,144,251,188,159,23,186,141,132,14,121,222,169,37,56,239,178,60,255,121,243,115,190,72,232,106,125,27,197,65,60,42,222,88,5,79,121,240,68,126,98,233,135,45,79,194,198,28,84,219,1,138,21,148,64,187,14,141,131,232,35,166,194,217,56,191,13,197,160,56,76,244,187,33,143,167,150,10,206,150,141,19,9,0,204,92,72,49,215,69,139,98,250,110,202,83,225,119,84,93,187,186,21,108,160,163,214,63,141,136,151,14,150,145,80,152,215,222,17,169,204,199,210,250,225,236,147,203,250,245,92,215,98,114,29,230,121,107,222,181,84,64,159,132,79,89,88,18,14,22,25,35,21,15,218,112,56,36,155,65,35,61,167,107,253,101,230,90,230,124,37,9,203,87,100,56,208,78,163,174,145,1,226,159,138,24,33,204,167,51,96,253,188,42,175,225,36,173,238,208,63,180,45,131,18,159,108,178,9,134,171,36,72,201,234,21,83,208,41,70,126,251,104,119,101,226,246,121,63,47,183,72,36,54,116,27,9,29,53,42,18,4,242,188,83,75,179,141,72,82,112,222,101,121,49,239,126,96,254,243,230,231,191,194,253,254,124,145,208,213,61,160,203,204,250,54,138,131,187,7,145,154,120,84,188,177,57,101,167,168,75,152,131,59,10,169,152,34,201,250,181,9,136,203,174,16,79,93,239,95,14,108,244,70,205,63,217,109,140,14,194,116,67,18,90,243,2,35,65,234,193,112,108,193,128,65,119,216,71,215,54,151,6,230,45,142,197,181,0,165,132,132,27,188,26,138,65,113,91,187,90,104,152,232,119,67,217,217,108,90,30,79,45,21,95,126,54,12,156,45,27,39,221,28,0,62,18,0,152,185,83,49,131,160,144,98,174,139,209,83,181,146,22,197,244,221,87,244,239,196,148,167,194,239,213,150,217,246,233,188,7,174,168,141,28,183,107,222,49,156,42,239,42,133,237,121,107,202,172,72,112,211,111,27,93,248,46,42,70,225,225,54,222,102,160,7,197,127,99,84,232,84,34,101,243,77,229,243,178,2,164,194,169,27,103,145,132,48,38,160,159,41,184,174,197,228,249,159,222,253,58,204,243,214,123,253,232,207,188,107,169,128,253,90,178,153,62,9,159,178,127,56,132,171,176,36,28,44,241,21,7,53,50,70,42,30,115,119,49,7,180,225,112,72,245,208,107,81,54,131,70,122,119,178,93,99,78,215,250,203,15,230,225,210,204,181,204,249,141,132,215,224,74,18,150,175,11,35,141,182,200,112,160,157,137,65,187,132,70,93,35,3,7,108,56,26,196,63,21,49,133,14,14,40,66,152,79,103,3,169,84,126,192,250,121,85,129,203,98,76,31,197,56,129,94,244,35,152,157,167,14,179,220,150,21,170,27,0,84,229,90,49,79,252,153,98,98,215,216,83,121,206,23,79,225,73,86,126,250,80,149,45,215,123,212,28,204,98,19,138,141,45,82,187,150,52,145,232,187,31,208,217,160,6,236,243,126,94,173,194,101,71,110,145,72,108,47,160,83,117,232,54,18,58,169,7,9,35,106,84,36,8,43,101,63,17,228,121,167,150,165,72,188,143,102,27,145,164,39,42,138,189,224,188,203,242,161,141,208,235,98,222,253,192,35,239,230,217,189,225,188,20,252,208,167,13,63,131,138,38,126,178,145,63,185,36,208,112,248,21,203,105,59,70,230,66,122,119,253,91,181,107,101,220,244,90,126,197,55,9,83,238,118,56,72,247,177,174,9,184,240,159,18,161,51,204,63,138,114,253,36,147,0,0,0,0,55,106,194,1,110,212,132,3,89,190,70,2,220,168,9,7,235,194,203,6,178,124,141,4,133,22,79,5,184,81,19,14,143,59,209,15,214,133,151,13,225,239,85,12,100,249,26,9,83,147,216,8,10,45,158,10,61,71,92,11,112,163,38,28,71,201,228,29,30,119,162,31,41,29,96,30,172,11,47,27,155,97,237,26,194,223,171,24,245,181,105,25,200,242,53,18,255,152,247,19,166,38,177,17,145,76,115,16,20,90,60,21,35,48,254,20,122,142,184,22,77,228,122,23,224,70,77,56,215,44,143,57,142,146,201,59,185,248,11,58,60,238,68,63,11,132,134,62,82,58,192,60,101,80,2,61,88,23,94,54,111,125,156,55,54,195,218,53,1,169,24,52,132,191,87,49,179,213,149,48,234,107,211,50,221,1,17,51,144,229,107,36,167,143,169,37,254,49,239,39,201,91,45,38,76,77,98,35,123,39,160,34,34,153,230,32,21,243,36,33,40,180,120,42,31,222,186,43,70,96,252,41,113,10,62,40,244,28,113,45,195,118,179,44,154,200,245,46,173,162,55,47,192,141,154,112,247,231,88,113,174,89,30,115,153,51,220,114,28,37,147,119,43,79,81,118,114,241,23,116,69,155,213,117,120,220,137,126,79,182,75,127,22,8,13,125,33,98,207,124,164,116,128,121,147,30,66,120,202,160,4,122,253,202,198,123,176,46,188,108,135,68,126,109,222,250,56,111,233,144,250,110,108,134,181,107,91,236,119,106,2,82,49,104,53,56,243,105,8,127,175,98,63,21,109,99,102,171,43,97,81,193,233,96,212,215,166,101,227,189,100,100,186,3,34,102,141,105,224,103,32,203,215,72,23,161,21,73,78,31,83,75,121,117,145,74,252,99,222,79,203,9,28,78,146,183,90,76,165,221,152,77,152,154,196,70,175,240,6,71,246,78,64,69,193,36,130,68,68,50,205,65,115,88,15,64,42,230,73,66,29,140,139,67,80,104,241,84,103,2,51,85,62,188,117,87,9,214,183,86,140,192,248,83,187,170,58,82,226,20,124,80,213,126,190,81,232,57,226,90,223,83,32,91,134,237,102,89,177,135,164,88,52,145,235,93,3,251,41,92,90,69,111,94,109,47,173,95,128,27,53,225,183,113,247,224,238,207,177,226,217,165,115,227,92,179,60,230,107,217,254,231,50,103,184,229,5,13,122,228,56,74,38,239,15,32,228,238,86,158,162,236,97,244,96,237,228,226,47,232,211,136,237,233,138,54,171,235,189,92,105,234,240,184,19,253,199,210,209,252,158,108,151,254,169,6,85,255,44,16,26,250,27,122,216,251,66,196,158,249,117,174,92,248,72,233,0,243,127,131,194,242,38,61,132,240,17,87,70,241,148,65,9,244,163,43,203,245,250,149,141,247,205,255,79,246,96,93,120,217,87,55,186,216,14,137,252,218,57,227,62,219,188,245,113,222,139,159,179,223,210,33,245,221,229,75,55,220,216,12,107,215,239,102,169,214,182,216,239,212,129,178,45,213,4,164,98,208,51,206,160,209,106,112,230,211,93,26,36,210,16,254,94,197,39,148,156,196,126,42,218,198,73,64,24,199,204,86,87,194,251,60,149,195,162,130,211,193,149,232,17,192,168,175,77,203,159,197,143,202,198,123,201,200,241,17,11,201,116,7,68,204,67,109,134,205,26,211,192,207,45,185,2,206,64,150,175,145,119,252,109,144,46,66,43,146,25,40,233,147,156,62,166,150,171,84,100,151,242,234,34,149,197,128,224,148,248,199,188,159,207,173,126,158,150,19,56,156,161,121,250,157,36,111,181,152,19,5,119,153,74,187,49,155,125,209,243,154,48,53,137,141,7,95,75,140,94,225,13,142,105,139,207,143,236,157,128,138,219,247,66,139,130,73,4,137,181,35,198,136,136,100,154,131,191,14,88,130,230,176,30,128,209,218,220,129,84,204,147,132,99,166,81,133,58,24,23,135,13,114,213,134,160,208,226,169,151,186,32,168,206,4,102,170,249,110,164,171,124,120,235,174,75,18,41,175,18,172,111,173,37,198,173,172,24,129,241,167,47,235,51,166,118,85,117,164,65,63,183,165,196,41,248,160,243,67,58,161,170,253,124,163,157,151,190,162,208,115,196,181,231,25,6,180,190,167,64,182,137,205,130,183,12,219,205,178,59,177,15,179,98,15,73,177,85,101,139,176,104,34,215,187,95,72,21,186,6,246,83,184,49,156,145,185,180,138,222,188,131,224,28,189,218,94,90,191,237,52,152,190,0,0,0,0,101,103,188,184,139,200,9,170,238,175,181,18,87,151,98,143,50,240,222,55,220,95,107,37,185,56,215,157,239,40,180,197,138,79,8,125,100,224,189,111,1,135,1,215,184,191,214,74,221,216,106,242,51,119,223,224,86,16,99,88,159,87,25,80,250,48,165,232,20,159,16,250,113,248,172,66,200,192,123,223,173,167,199,103,67,8,114,117,38,111,206,205,112,127,173,149,21,24,17,45,251,183,164,63,158,208,24,135,39,232,207,26,66,143,115,162,172,32,198,176,201,71,122,8,62,175,50,160,91,200,142,24,181,103,59,10,208,0,135,178,105,56,80,47,12,95,236,151,226,240,89,133,135,151,229,61,209,135,134,101,180,224,58,221,90,79,143,207,63,40,51,119,134,16,228,234,227,119,88,82,13,216,237,64,104,191,81,248,161,248,43,240,196,159,151,72,42,48,34,90,79,87,158,226,246,111,73,127,147,8,245,199,125,167,64,213,24,192,252,109,78,208,159,53,43,183,35,141,197,24,150,159,160,127,42,39,25,71,253,186,124,32,65,2,146,143,244,16,247,232,72,168,61,88,20,155,88,63,168,35,182,144,29,49,211,247,161,137,106,207,118,20,15,168,202,172,225,7,127,190,132,96,195,6,210,112,160,94,183,23,28,230,89,184,169,244,60,223,21,76,133,231,194,209,224,128,126,105,14,47,203,123,107,72,119,195,162,15,13,203,199,104,177,115,41,199,4,97,76,160,184,217,245,152,111,68,144,255,211,252,126,80,102,238,27,55,218,86,77,39,185,14,40,64,5,182,198,239,176,164,163,136,12,28,26,176,219,129,127,215,103,57,145,120,210,43,244,31,110,147,3,247,38,59,102,144,154,131,136,63,47,145,237,88,147,41,84,96,68,180,49,7,248,12,223,168,77,30,186,207,241,166,236,223,146,254,137,184,46,70,103,23,155,84,2,112,39,236,187,72,240,113,222,47,76,201,48,128,249,219,85,231,69,99,156,160,63,107,249,199,131,211,23,104,54,193,114,15,138,121,203,55,93,228,174,80,225,92,64,255,84,78,37,152,232,246,115,136,139,174,22,239,55,22,248,64,130,4,157,39,62,188,36,31,233,33,65,120,85,153,175,215,224,139,202,176,92,51,59,182,89,237,94,209,229,85,176,126,80,71,213,25,236,255,108,33,59,98,9,70,135,218,231,233,50,200,130,142,142,112,212,158,237,40,177,249,81,144,95,86,228,130,58,49,88,58,131,9,143,167,230,110,51,31,8,193,134,13,109,166,58,181,164,225,64,189,193,134,252,5,47,41,73,23,74,78,245,175,243,118,34,50,150,17,158,138,120,190,43,152,29,217,151,32,75,201,244,120,46,174,72,192,192,1,253,210,165,102,65,106,28,94,150,247,121,57,42,79,151,150,159,93,242,241,35,229,5,25,107,77,96,126,215,245,142,209,98,231,235,182,222,95,82,142,9,194,55,233,181,122,217,70,0,104,188,33,188,208,234,49,223,136,143,86,99,48,97,249,214,34,4,158,106,154,189,166,189,7,216,193,1,191,54,110,180,173,83,9,8,21,154,78,114,29,255,41,206,165,17,134,123,183,116,225,199,15,205,217,16,146,168,190,172,42,70,17,25,56,35,118,165,128,117,102,198,216,16,1,122,96,254,174,207,114,155,201,115,202,34,241,164,87,71,150,24,239,169,57,173,253,204,94,17,69,6,238,77,118,99,137,241,206,141,38,68,220,232,65,248,100,81,121,47,249,52,30,147,65,218,177,38,83,191,214,154,235,233,198,249,179,140,161,69,11,98,14,240,25,7,105,76,161,190,81,155,60,219,54,39,132,53,153,146,150,80,254,46,46,153,185,84,38,252,222,232,158,18,113,93,140,119,22,225,52,206,46,54,169,171,73,138,17,69,230,63,3,32,129,131,187,118,145,224,227,19,246,92,91,253,89,233,73,152,62,85,241,33,6,130,108,68,97,62,212,170,206,139,198,207,169,55,126,56,65,127,214,93,38,195,110,179,137,118,124,214,238,202,196,111,214,29,89,10,177,161,225,228,30,20,243,129,121,168,75,215,105,203,19,178,14,119,171,92,161,194,185,57,198,126,1,128,254,169,156,229,153,21,36,11,54,160,54,110,81,28,142,167,22,102,134,194,113,218,62,44,222,111,44,73,185,211,148,240,129,4,9,149,230,184,177,123,73,13,163,30,46,177,27,72,62,210,67,45,89,110,251,195,246,219,233,166,145,103,81,31,169,176,204,122,206,12,116,148,97,185,102,241,6,5,222,0,0,0,0,119,7,48,150,238,14,97,44,153,9,81,186,7,109,196,25,112,106,244,143,233,99,165,53,158,100,149,163,14,219,136,50,121,220,184,164,224,213,233,30,151,210,217,136,9,182,76,43,126,177,124,189,231,184,45,7,144,191,29,145,29,183,16,100,106,176,32,242,243,185,113,72,132,190,65,222,26,218,212,125,109,221,228,235,244,212,181,81,131,211,133,199,19,108,152,86,100,107,168,192,253,98,249,122,138,101,201,236,20,1,92,79,99,6,108,217,250,15,61,99,141,8,13,245,59,110,32,200,76,105,16,94,213,96,65,228,162,103,113,114,60,3,228,209,75,4,212,71,210,13,133,253,165,10,181,107,53,181,168,250,66,178,152,108,219,187,201,214,172,188,249,64,50,216,108,227,69,223,92,117,220,214,13,207,171,209,61,89,38,217,48,172,81,222,0,58,200,215,81,128,191,208,97,22,33,180,244,181,86,179,196,35,207,186,149,153,184,189,165,15,40,2,184,158,95,5,136,8,198,12,217,178,177,11,233,36,47,111,124,135,88,104,76,17,193,97,29,171,182,102,45,61,118,220,65,144,1,219,113,6,152,210,32,188,239,213,16,42,113,177,133,137,6,182,181,31,159,191,228,165,232,184,212,51,120,7,201,162,15,0,249,52,150,9,168,142,225,14,152,24,127,106,13,187,8,109,61,45,145,100,108,151,230,99,92,1,107,107,81,244,28,108,97,98,133,101,48,216,242,98,0,78,108,6,149,237,27,1,165,123,130,8,244,193,245,15,196,87,101,176,217,198,18,183,233,80,139,190,184,234,252,185,136,124,98,221,29,223,21,218,45,73,140,211,124,243,251,212,76,101,77,178,97,88,58,181,81,206,163,188,0,116,212,187,48,226,74,223,165,65,61,216,149,215,164,209,196,109,211,214,244,251,67,105,233,106,52,110,217,252,173,103,136,70,218,96,184,208,68,4,45,115,51,3,29,229,170,10,76,95,221,13,124,201,80,5,113,60,39,2,65,170,190,11,16,16,201,12,32,134,87,104,181,37,32,111,133,179,185,102,212,9,206,97,228,159,94,222,249,14,41,217,201,152,176,208,152,34,199,215,168,180,89,179,61,23,46,180,13,129,183,189,92,59,192,186,108,173,237,184,131,32,154,191,179,182,3,182,226,12,116,177,210,154,234,213,71,57,157,210,119,175,4,219,38,21,115,220,22,131,227,99,11,18,148,100,59,132,13,109,106,62,122,106,90,168,228,14,207,11,147,9,255,157,10,0,174,39,125,7,158,177,240,15,147,68,135,8,163,210,30,1,242,104,105,6,194,254,247,98,87,93,128,101,103,203,25,108,54,113,110,107,6,231,254,212,27,118,137,211,43,224,16,218,122,90,103,221,74,204,249,185,223,111,142,190,239,249,23,183,190,67,96,176,142,213,214,214,163,232,161,209,147,126,56,216,194,196,79,223,242,82,209,187,103,241,166,188,87,103,63,181,6,221,72,178,54,75,216,13,43,218,175,10,27,76,54,3,74,246,65,4,122,96,223,96,239,195,168,103,223,85,49,110,142,239,70,105,190,121,203,97,179,140,188,102,131,26,37,111,210,160,82,104,226,54,204,12,119,149,187,11,71,3,34,2,22,185,85,5,38,47,197,186,59,190,178,189,11,40,43,180,90,146,92,179,106,4,194,215,255,167,181,208,207,49,44,217,158,139,91,222,174,29,155,100,194,176,236,99,242,38,117,106,163,156,2,109,147,10,156,9,6,169,235,14,54,63,114,7,103,133,5,0,87,19,149,191,74,130,226,184,122,20,123,177,43,174,12,182,27,56,146,210,142,155,229,213,190,13,124,220,239,183,11,219,223,33,134,211,210,212,241,212,226,66,104,221,179,248,31,218,131,110,129,190,22,205,246,185,38,91,111,176,119,225,24,183,71,119,136,8,90,230,255,15,106,112,102,6,59,202,17,1,11,92,143,101,158,255,248,98,174,105,97,107,255,211,22,108,207,69,160,10,226,120,215,13,210,238,78,4,131,84,57,3,179,194,167,103,38,97,208,96,22,247,73,105,71,77,62,110,119,219,174,209,106,74,217,214,90,220,64,223,11,102,55,216,59,240,169,188,174,83,222,187,158,197,71,178,207,127,48,181,255,233,189,189,242,28,202,186,194,138,83,179,147,48,36,180,163,166,186,208,54,5,205,215,6,147,84,222,87,41,35,217,103,191,179,102,122,46,196,97,74,184,93,104,27,2,42,111,43,148,180,11,190,55,195,12,142,161,90,5,223,27,45,2,239,141,0,0,0,0,25,27,49,65,50,54,98,130,43,45,83,195,100,108,197,4,125,119,244,69,86,90,167,134,79,65,150,199,200,217,138,8,209,194,187,73,250,239,232,138,227,244,217,203,172,181,79,12,181,174,126,77,158,131,45,142,135,152,28,207,74,194,18,81,83,217,35,16,120,244,112,211,97,239,65,146,46,174,215,85,55,181,230,20,28,152,181,215,5,131,132,150,130,27,152,89,155,0,169,24,176,45,250,219,169,54,203,154,230,119,93,93,255,108,108,28,212,65,63,223,205,90,14,158,149,132,36,162,140,159,21,227,167,178,70,32,190,169,119,97,241,232,225,166,232,243,208,231,195,222,131,36,218,197,178,101,93,93,174,170,68,70,159,235,111,107,204,40,118,112,253,105,57,49,107,174,32,42,90,239,11,7,9,44,18,28,56,109,223,70,54,243,198,93,7,178,237,112,84,113,244,107,101,48,187,42,243,247,162,49,194,182,137,28,145,117,144,7,160,52,23,159,188,251,14,132,141,186,37,169,222,121,60,178,239,56,115,243,121,255,106,232,72,190,65,197,27,125,88,222,42,60,240,121,79,5,233,98,126,68,194,79,45,135,219,84,28,198,148,21,138,1,141,14,187,64,166,35,232,131,191,56,217,194,56,160,197,13,33,187,244,76,10,150,167,143,19,141,150,206,92,204,0,9,69,215,49,72,110,250,98,139,119,225,83,202,186,187,93,84,163,160,108,21,136,141,63,214,145,150,14,151,222,215,152,80,199,204,169,17,236,225,250,210,245,250,203,147,114,98,215,92,107,121,230,29,64,84,181,222,89,79,132,159,22,14,18,88,15,21,35,25,36,56,112,218,61,35,65,155,101,253,107,167,124,230,90,230,87,203,9,37,78,208,56,100,1,145,174,163,24,138,159,226,51,167,204,33,42,188,253,96,173,36,225,175,180,63,208,238,159,18,131,45,134,9,178,108,201,72,36,171,208,83,21,234,251,126,70,41,226,101,119,104,47,63,121,246,54,36,72,183,29,9,27,116,4,18,42,53,75,83,188,242,82,72,141,179,121,101,222,112,96,126,239,49,231,230,243,254,254,253,194,191,213,208,145,124,204,203,160,61,131,138,54,250,154,145,7,187,177,188,84,120,168,167,101,57,59,131,152,75,34,152,169,10,9,181,250,201,16,174,203,136,95,239,93,79,70,244,108,14,109,217,63,205,116,194,14,140,243,90,18,67,234,65,35,2,193,108,112,193,216,119,65,128,151,54,215,71,142,45,230,6,165,0,181,197,188,27,132,132,113,65,138,26,104,90,187,91,67,119,232,152,90,108,217,217,21,45,79,30,12,54,126,95,39,27,45,156,62,0,28,221,185,152,0,18,160,131,49,83,139,174,98,144,146,181,83,209,221,244,197,22,196,239,244,87,239,194,167,148,246,217,150,213,174,7,188,233,183,28,141,168,156,49,222,107,133,42,239,42,202,107,121,237,211,112,72,172,248,93,27,111,225,70,42,46,102,222,54,225,127,197,7,160,84,232,84,99,77,243,101,34,2,178,243,229,27,169,194,164,48,132,145,103,41,159,160,38,228,197,174,184,253,222,159,249,214,243,204,58,207,232,253,123,128,169,107,188,153,178,90,253,178,159,9,62,171,132,56,127,44,28,36,176,53,7,21,241,30,42,70,50,7,49,119,115,72,112,225,180,81,107,208,245,122,70,131,54,99,93,178,119,203,250,215,78,210,225,230,15,249,204,181,204,224,215,132,141,175,150,18,74,182,141,35,11,157,160,112,200,132,187,65,137,3,35,93,70,26,56,108,7,49,21,63,196,40,14,14,133,103,79,152,66,126,84,169,3,85,121,250,192,76,98,203,129,129,56,197,31,152,35,244,94,179,14,167,157,170,21,150,220,229,84,0,27,252,79,49,90,215,98,98,153,206,121,83,216,73,225,79,23,80,250,126,86,123,215,45,149,98,204,28,212,45,141,138,19,52,150,187,82,31,187,232,145,6,160,217,208,94,126,243,236,71,101,194,173,108,72,145,110,117,83,160,47,58,18,54,232,35,9,7,169,8,36,84,106,17,63,101,43].concat([150,167,121,228,143,188,72,165,164,145,27,102,189,138,42,39,242,203,188,224,235,208,141,161,192,253,222,98,217,230,239,35,20,188,225,189,13,167,208,252,38,138,131,63,63,145,178,126,112,208,36,185,105,203,21,248,66,230,70,59,91,253,119,122,220,101,107,181,197,126,90,244,238,83,9,55,247,72,56,118,184,9,174,177,161,18,159,240,138,63,204,51,147,36,253,114,0,0,0,0,1,194,106,55,3,132,212,110,2,70,190,89,7,9,168,220,6,203,194,235,4,141,124,178,5,79,22,133,14,19,81,184,15,209,59,143,13,151,133,214,12,85,239,225,9,26,249,100,8,216,147,83,10,158,45,10,11,92,71,61,28,38,163,112,29,228,201,71,31,162,119,30,30,96,29,41,27,47,11,172,26,237,97,155,24,171,223,194,25,105,181,245,18,53,242,200,19,247,152,255,17,177,38,166,16,115,76,145,21,60,90,20,20,254,48,35,22,184,142,122,23,122,228,77,56,77,70,224,57,143,44,215,59,201,146,142,58,11,248,185,63,68,238,60,62,134,132,11,60,192,58,82,61,2,80,101,54,94,23,88,55,156,125,111,53,218,195,54,52,24,169,1,49,87,191,132,48,149,213,179,50,211,107,234,51,17,1,221,36,107,229,144,37,169,143,167,39,239,49,254,38,45,91,201,35,98,77,76,34,160,39,123,32,230,153,34,33,36,243,21,42,120,180,40,43,186,222,31,41,252,96,70,40,62,10,113,45,113,28,244,44,179,118,195,46,245,200,154,47,55,162,173,112,154,141,192,113,88,231,247,115,30,89,174,114,220,51,153,119,147,37,28,118,81,79,43,116,23,241,114,117,213,155,69,126,137,220,120,127,75,182,79,125,13,8,22,124,207,98,33,121,128,116,164,120,66,30,147,122,4,160,202,123,198,202,253,108,188,46,176,109,126,68,135,111,56,250,222,110,250,144,233,107,181,134,108,106,119,236,91,104,49,82,2,105,243,56,53,98,175,127,8,99,109,21,63,97,43,171,102,96,233,193,81,101,166,215,212,100,100,189,227,102,34,3,186,103,224,105,141,72,215,203,32,73,21,161,23,75,83,31,78,74,145,117,121,79,222,99,252,78,28,9,203,76,90,183,146,77,152,221,165,70,196,154,152,71,6,240,175,69,64,78,246,68,130,36,193,65,205,50,68,64,15,88,115,66,73,230,42,67,139,140,29,84,241,104,80,85,51,2,103,87,117,188,62,86,183,214,9,83,248,192,140,82,58,170,187,80,124,20,226,81,190,126,213,90,226,57,232,91,32,83,223,89,102,237,134,88,164,135,177,93,235,145,52,92,41,251,3,94,111,69,90,95,173,47,109,225,53,27,128,224,247,113,183,226,177,207,238,227,115,165,217,230,60,179,92,231,254,217,107,229,184,103,50,228,122,13,5,239,38,74,56,238,228,32,15,236,162,158,86,237,96,244,97,232,47,226,228,233,237,136,211,235,171,54,138,234,105,92,189,253,19,184,240,252,209,210,199,254,151,108,158,255,85,6,169,250,26,16,44,251,216,122,27,249,158,196,66,248,92,174,117,243,0,233,72,242,194,131,127,240,132,61,38,241,70,87,17,244,9,65,148,245,203,43,163,247,141,149,250,246,79,255,205,217,120,93,96,216,186,55,87,218,252,137,14,219,62,227,57,222,113,245,188,223,179,159,139,221,245,33,210,220,55,75,229,215,107,12,216,214,169,102,239,212,239,216,182,213,45,178,129,208,98,164,4,209,160,206,51,211,230,112,106,210,36,26,93,197,94,254,16,196,156,148,39,198,218,42,126,199,24,64,73,194,87,86,204,195,149,60,251,193,211,130,162,192,17,232,149,203,77,175,168,202,143,197,159,200,201,123,198,201,11,17,241,204,68,7,116,205,134,109,67,207,192,211,26,206,2,185,45,145,175,150,64,144,109,252,119,146,43,66,46,147,233,40,25,150,166,62,156,151,100,84,171,149,34,234,242,148,224,128,197,159,188,199,248,158,126,173,207,156,56,19,150,157,250,121,161,152,181,111,36,153,119,5,19,155,49,187,74,154,243,209,125,141,137,53,48,140,75,95,7,142,13,225,94,143,207,139,105,138,128,157,236,139,66,247,219,137,4,73,130,136,198,35,181,131,154,100,136,130,88,14,191,128,30,176,230,129,220,218,209,132,147,204,84,133,81,166,99,135,23,24,58,134,213,114,13,169,226,208,160,168,32,186,151,170,102,4,206,171,164,110,249,174,235,120,124,175,41,18,75,173,111,172,18,172,173,198,37,167,241,129,24,166,51,235,47,164,117,85,118,165,183,63,65,160,248,41,196,161,58,67,243,163,124,253,170,162,190,151,157,181,196,115,208,180,6,25,231,182,64,167,190,183,130,205,137,178,205,219,12,179,15,177,59,177,73,15,98,176,139,101,85,187,215,34,104,186,21,72,95,184,83,246,6,185,145,156,49,188,222,138,180,189,28,224,131,191,90,94,218,190,152,52,237,0,0,0,0,184,188,103,101,170,9,200,139,18,181,175,238,143,98,151,87,55,222,240,50,37,107,95,220,157,215,56,185,197,180,40,239,125,8,79,138,111,189,224,100,215,1,135,1,74,214,191,184,242,106,216,221,224,223,119,51,88,99,16,86,80,25,87,159,232,165,48,250,250,16,159,20,66,172,248,113,223,123,192,200,103,199,167,173,117,114,8,67,205,206,111,38,149,173,127,112,45,17,24,21,63,164,183,251,135,24,208,158,26,207,232,39,162,115,143,66,176,198,32,172,8,122,71,201,160,50,175,62,24,142,200,91,10,59,103,181,178,135,0,208,47,80,56,105,151,236,95,12,133,89,240,226,61,229,151,135,101,134,135,209,221,58,224,180,207,143,79,90,119,51,40,63,234,228,16,134,82,88,119,227,64,237,216,13,248,81,191,104,240,43,248,161,72,151,159,196,90,34,48,42,226,158,87,79,127,73,111,246,199,245,8,147,213,64,167,125,109,252,192,24,53,159,208,78,141,35,183,43,159,150,24,197,39,42,127,160,186,253,71,25,2,65,32,124,16,244,143,146,168,72,232,247,155,20,88,61,35,168,63,88,49,29,144,182,137,161,247,211,20,118,207,106,172,202,168,15,190,127,7,225,6,195,96,132,94,160,112,210,230,28,23,183,244,169,184,89,76,21,223,60,209,194,231,133,105,126,128,224,123,203,47,14,195,119,72,107,203,13,15,162,115,177,104,199,97,4,199,41,217,184,160,76,68,111,152,245,252,211,255,144,238,102,80,126,86,218,55,27,14,185,39,77,182,5,64,40,164,176,239,198,28,12,136,163,129,219,176,26,57,103,215,127,43,210,120,145,147,110,31,244,59,38,247,3,131,154,144,102,145,47,63,136,41,147,88,237,180,68,96,84,12,248,7,49,30,77,168,223,166,241,207,186,254,146,223,236,70,46,184,137,84,155,23,103,236,39,112,2,113,240,72,187,201,76,47,222,219,249,128,48,99,69,231,85,107,63,160,156,211,131,199,249,193,54,104,23,121,138,15,114,228,93,55,203,92,225,80,174,78,84,255,64,246,232,152,37,174,139,136,115,22,55,239,22,4,130,64,248,188,62,39,157,33,233,31,36,153,85,120,65,139,224,215,175,51,92,176,202,237,89,182,59,85,229,209,94,71,80,126,176,255,236,25,213,98,59,33,108,218,135,70,9,200,50,233,231,112,142,142,130,40,237,158,212,144,81,249,177,130,228,86,95,58,88,49,58,167,143,9,131,31,51,110,230,13,134,193,8,181,58,166,109,189,64,225,164,5,252,134,193,23,73,41,47,175,245,78,74,50,34,118,243,138,158,17,150,152,43,190,120,32,151,217,29,120,244,201,75,192,72,174,46,210,253,1,192,106,65,102,165,247,150,94,28,79,42,57,121,93,159,150,151,229,35,241,242,77,107,25,5,245,215,126,96,231,98,209,142,95,222,182,235,194,9,142,82,122,181,233,55,104,0,70,217,208,188,33,188,136,223,49,234,48,99,86,143,34,214,249,97,154,106,158,4,7,189,166,189,191,1,193,216,173,180,110,54,21,8,9,83,29,114,78,154,165,206,41,255,183,123,134,17,15,199,225,116,146,16,217,205,42,172,190,168,56,25,17,70,128,165,118,35,216,198,102,117,96,122,1,16,114,207,174,254,202,115,201,155,87,164,241,34,239,24,150,71,253,173,57,169,69,17,94,204,118,77,238,6,206,241,137,99,220,68,38,141,100,248,65,232,249,47,121,81,65,147,30,52,83,38,177,218,235,154,214,191,179,249,198,233,11,69,161,140,25,240,14,98,161,76,105,7,60,155,81,190,132,39,54,219,150,146,153,53,46,46,254,80,38,84,185,153,158,232,222,252,140,93,113,18,52,225,22,119,169,54,46,206,17,138,73,171,3,63,230,69,187,131,129,32,227,224,145,118,91,92,246,19,73,233,89,253,241,85,62,152,108,130,6,33,212,62,97,68,198,139,206,170,126,55,169,207,214,127,65,56,110,195,38,93,124,118,137,179,196,202,238,214,89,29,214,111,225,161,177,10,243,20,30,228,75,168,121,129,19,203,105,215,171,119,14,178,185,194,161,92,1,126,198,57,156,169,254,128,36,21,153,229,54,160,54,11,142,28,81,110,134,102,22,167,62,218,113,194,44,111,222,44,148,211,185,73,9,4,129,240,177,184,230,149,163,13,73,123,27,177,46,30,67,210,62,72,251,110,89,45,233,219,246,195,81,103,145,166,204,176,169,31,116,12,206,122,102,185,97,148,222,5,6,241,0,0,0,0,0,0,0,0,2,0,0,0,4,0,4,0,8,0,4,0,8,0,0,0,4,0,5,0,16,0,8,0,8,0,0,0,4,0,6,0,32,0,32,0,8,0,0,0,4,0,4,0,16,0,16,0,6,0,0,0,8,0,16,0,32,0,32,0,6,0,0,0,8,0,16,0,128,0,128,0,6,0,0,0,8,0,32,0,128,0,0,1,6,0,0,0,32,0,128,0,2,1,0,4,6,0,0,0,32,0,2,1,2,1,0,16,6,0,0,0,16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,3,0,0,0,4,0,0,0,5,0,0,0,6,0,0,0,7,0,0,0,8,0,0,0,10,0,0,0,12,0,0,0,14,0,0,0,16,0,0,0,20,0,0,0,24,0,0,0,28,0,0,0,32,0,0,0,40,0,0,0,48,0,0,0,56,0,0,0,64,0,0,0,80,0,0,0,96,0,0,0,112,0,0,0,128,0,0,0,160,0,0,0,192,0,0,0,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,3,0,0,0,4,0,0,0,6,0,0,0,8,0,0,0,12,0,0,0,16,0,0,0,24,0,0,0,32,0,0,0,48,0,0,0,64,0,0,0,96,0,0,0,128,0,0,0,192,0,0,0,0,1,0,0,128,1,0,0,0,2,0,0,0,3,0,0,0,4,0,0,0,6,0,0,0,8,0,0,0,12,0,0,0,16,0,0,0,24,0,0,0,32,0,0,0,48,0,0,0,64,0,0,0,96,0,0,105,110,118,97,108,105,100,32,99,111,100,101,32,108,101,110,103,116,104,115,32,115,101,116,0,0,0,0,0,0,0,0,105,110,99,111,114,114,101,99,116,32,100,97,116,97,32,99,104,101,99,107,0,0,0,0,114,101,113,117,101,115,116,101,100,32,108,101,110,103,116,104,32,100,111,101,115,32,110,111,116,32,102,105,116,32,105,110,32,105,110,116,0,0,0,0,105,110,99,111,109,112,97,116,105,98,108,101,32,118,101,114,115,105,111,110,0,0,0,0,116,111,111,32,109,97,110,121,32,108,101,110,103,116,104,32,111,114,32,100,105,115,116,97,110,99,101,32,115,121,109,98,111,108,115,0,0,0,0,0,99,111,109,112,114,101,115,115,101,100,32,100,97,116,97,32,101,114,114,111,114,0,0,0,98,117,102,102,101,114,32,101,114,114,111,114,0,0,0,0,110,101,101,100,32,100,105,99,116,105,111,110,97,114,121,0,105,110,118,97,108,105,100,32,115,116,111,114,101,100,32,98,108,111,99,107,32,108,101,110,103,116,104,115,0,0,0,0,105,110,116,101,114,110,97,108,32,101,114,114,111,114,58,32,105,110,102,108,97,116,101,32,115,116,114,101,97,109,32,99,111,114,114,117,112,116,0,0,105,110,115,117,102,102,105,99,105,101,110,116,32,109,101,109,111,114,121,0,0,0,0,0,105,110,118,97,108,105,100,32,98,108,111,99,107,32,116,121,112,101,0,0,0,0,0,0,117,110,101,120,112,101,99,116,101,100,32,101,110,100,32,111,102,32,102,105,108,101,0,0,100,97,116,97,32,101,114,114,111,114,0,0,0,0,0,0,104,101,97,100,101,114,32,99,114,99,32,109,105,115,109,97,116,99,104,0,0,0,0,0,117,110,107,110,111,119,110,32,104,101,97,100,101,114,32,102,108,97,103,115,32,115,101,116,0,0,0,0,0,0,0,0,115,116,114,101,97,109,32,101,114,114,111,114,0,0,0,0,117,110,107,110,111,119,110,32,104,101,97,100,101,114,32,102,108,97,103,115,32,115,101,116,0,0,0,0,0,0,0,0,117,110,107,110,111,119,110,32,99,111,109,112,114,101,115,115,105,111,110,32,109,101,116,104,111,100,0,0,0,0,0,0,102,105,108,101,32,101,114,114,111,114,0,0,0,0,0,0,105,110,118,97,108,105,100,32,100,105,115,116,97,110,99,101,32,116,111,111,32,102,97,114,32,98,97,99,107,0,0,0,105,110,118,97,108,105,100,32,119,105,110,100,111,119,32,115,105,122,101,0,0,0,0,0,49,46,50,46,53,0,0,0,49,46,50,46,53,0,0,0,58,32,0,0,0,0,0,0,117,110,107,110,111,119,110,32,99,111,109,112,114,101,115,115,105,111,110,32,109,101,116,104,111,100,0,0,0,0,0,0,105,110,118,97,108,105,100,32,108,105,116,101,114,97,108,47,108,101,110,103,116,104,32,99,111,100,101,0,0,0,0,0,111,117,116,32,111,102,32,109,101,109,111,114,121,0,0,0,111,117,116,32,111,102,32,109,101,109,111,114,121,0,0,0,111,117,116,32,111,102,32,109,101,109,111,114,121,0,0,0,105,110,99,111,114,114,101,99,116,32,108,101,110,103,116,104,32,99,104,101,99,107,0,0,114,101,113,117,101,115,116,101,100,32,108,101,110,103,116,104,32,100,111,101,115,32,110,111,116,32,102,105,116,32,105,110,32,105,110,116,0,0,0,0,115,116,114,101,97,109,32,101,110,100,0,0,0,0,0,0,105,110,99,111,114,114,101,99,116,32,100,97,116,97,32,99,104,101,99,107,0,0,0,0,105,110,118,97,108,105,100,32,100,105,115,116,97,110,99,101,32,116,111,111,32,102,97,114,32,98,97,99,107,0,0,0,105,110,118,97,108,105,100,32,100,105,115,116,97,110,99,101,32,99,111,100,101,0,0,0,105,110,99,111,114,114,101,99,116,32,104,101,97,100,101,114,32,99,104,101,99,107,0,0,105,110,118,97,108,105,100,32,108,105,116,101,114,97,108,47,108,101,110,103,116,104,32,99,111,100,101,0,0,0,0,0,105,110,118,97,108,105,100,32,100,105,115,116,97,110,99,101,32,99,111,100,101,0,0,0,105,110,118,97,108,105,100,32,100,105,115,116,97,110,99,101,115,32,115,101,116,0,0,0,105,110,118,97,108,105,100,32,108,105,116,101,114,97,108,47,108,101,110,103,116,104,115,32,115,101,116,0,0,0,0,0,105,110,116,101,114,110,97,108,32,101,114,114,111,114,58,32,100,101,102,108,97,116,101,32,115,116,114,101,97,109,32,99,111,114,114,117,112,116,0,0,105,110,118,97,108,105,100,32,99,111,100,101,32,45,45,32,109,105,115,115,105,110,103,32,101,110,100,45,111,102,45,98,108,111,99,107,0,0,0,0,105,110,118,97,108,105,100,32,98,105,116,32,108,101,110,103,116,104,32,114,101,112,101,97,116,0,0,0,0,0,0,0,105,110,99,111,114,114,101,99,116,32,108,101,110,103,116,104,32,99,104,101,99,107,0,0,0,1,2,3,4,5,6,7,8,8,9,9,10,10,11,11,12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,0,1,2,3,4,4,5,5,6,6,6,6,7,7,7,7,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,0,0,16,17,18,18,19,19,20,20,20,20,21,21,21,21,22,22,22,22,22,22,22,22,23,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29])
, "i8", ALLOC_NONE, Runtime.GLOBAL_BASE)
var tempDoublePtr = Runtime.alignMemory(allocate(12, "i8", ALLOC_STATIC), 8);
assert(tempDoublePtr % 8 == 0);
function copyTempFloat(ptr) { // functions, because inlining this code increases code size too much
  HEAP8[tempDoublePtr] = HEAP8[ptr];
  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];
  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];
  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];
}
function copyTempDouble(ptr) {
  HEAP8[tempDoublePtr] = HEAP8[ptr];
  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];
  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];
  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];
  HEAP8[tempDoublePtr+4] = HEAP8[ptr+4];
  HEAP8[tempDoublePtr+5] = HEAP8[ptr+5];
  HEAP8[tempDoublePtr+6] = HEAP8[ptr+6];
  HEAP8[tempDoublePtr+7] = HEAP8[ptr+7];
}
  function _llvm_lifetime_start() {}
  function _llvm_lifetime_end() {}
  Module["_memset"] = _memset;var _llvm_memset_p0i8_i32=_memset;
  Module["_memcpy"] = _memcpy;var _llvm_memcpy_p0i8_p0i8_i32=_memcpy;
  var ERRNO_CODES={EPERM:1,ENOENT:2,ESRCH:3,EINTR:4,EIO:5,ENXIO:6,E2BIG:7,ENOEXEC:8,EBADF:9,ECHILD:10,EAGAIN:11,EWOULDBLOCK:11,ENOMEM:12,EACCES:13,EFAULT:14,ENOTBLK:15,EBUSY:16,EEXIST:17,EXDEV:18,ENODEV:19,ENOTDIR:20,EISDIR:21,EINVAL:22,ENFILE:23,EMFILE:24,ENOTTY:25,ETXTBSY:26,EFBIG:27,ENOSPC:28,ESPIPE:29,EROFS:30,EMLINK:31,EPIPE:32,EDOM:33,ERANGE:34,ENOMSG:35,EIDRM:36,ECHRNG:37,EL2NSYNC:38,EL3HLT:39,EL3RST:40,ELNRNG:41,EUNATCH:42,ENOCSI:43,EL2HLT:44,EDEADLK:45,ENOLCK:46,EBADE:50,EBADR:51,EXFULL:52,ENOANO:53,EBADRQC:54,EBADSLT:55,EDEADLOCK:56,EBFONT:57,ENOSTR:60,ENODATA:61,ETIME:62,ENOSR:63,ENONET:64,ENOPKG:65,EREMOTE:66,ENOLINK:67,EADV:68,ESRMNT:69,ECOMM:70,EPROTO:71,EMULTIHOP:74,ELBIN:75,EDOTDOT:76,EBADMSG:77,EFTYPE:79,ENOTUNIQ:80,EBADFD:81,EREMCHG:82,ELIBACC:83,ELIBBAD:84,ELIBSCN:85,ELIBMAX:86,ELIBEXEC:87,ENOSYS:88,ENMFILE:89,ENOTEMPTY:90,ENAMETOOLONG:91,ELOOP:92,EOPNOTSUPP:95,EPFNOSUPPORT:96,ECONNRESET:104,ENOBUFS:105,EAFNOSUPPORT:106,EPROTOTYPE:107,ENOTSOCK:108,ENOPROTOOPT:109,ESHUTDOWN:110,ECONNREFUSED:111,EADDRINUSE:112,ECONNABORTED:113,ENETUNREACH:114,ENETDOWN:115,ETIMEDOUT:116,EHOSTDOWN:117,EHOSTUNREACH:118,EINPROGRESS:119,EALREADY:120,EDESTADDRREQ:121,EMSGSIZE:122,EPROTONOSUPPORT:123,ESOCKTNOSUPPORT:124,EADDRNOTAVAIL:125,ENETRESET:126,EISCONN:127,ENOTCONN:128,ETOOMANYREFS:129,EPROCLIM:130,EUSERS:131,EDQUOT:132,ESTALE:133,ENOTSUP:134,ENOMEDIUM:135,ENOSHARE:136,ECASECLASH:137,EILSEQ:138,EOVERFLOW:139,ECANCELED:140,ENOTRECOVERABLE:141,EOWNERDEAD:142,ESTRPIPE:143};
  var ___errno_state=0;function ___setErrNo(value) {
      // For convenient setting and returning of errno.
      HEAP32[((___errno_state)>>2)]=value
      return value;
    }
  var _stdin=allocate(1, "i32*", ALLOC_STATIC);
  var _stdout=allocate(1, "i32*", ALLOC_STATIC);
  var _stderr=allocate(1, "i32*", ALLOC_STATIC);
  var __impure_ptr=allocate(1, "i32*", ALLOC_STATIC);var FS={currentPath:"/",nextInode:2,streams:[null],ignorePermissions:true,createFileHandle:function (stream, fd) {
        if (typeof stream === 'undefined') {
          stream = null;
        }
        if (!fd) {
          if (stream && stream.socket) {
            for (var i = 1; i < 64; i++) {
              if (!FS.streams[i]) {
                fd = i;
                break;
              }
            }
            assert(fd, 'ran out of low fds for sockets');
          } else {
            fd = Math.max(FS.streams.length, 64);
            for (var i = FS.streams.length; i < fd; i++) {
              FS.streams[i] = null; // Keep dense
            }
          }
        }
        // Close WebSocket first if we are about to replace the fd (i.e. dup2)
        if (FS.streams[fd] && FS.streams[fd].socket && FS.streams[fd].socket.close) {
          FS.streams[fd].socket.close();
        }
        FS.streams[fd] = stream;
        return fd;
      },removeFileHandle:function (fd) {
        FS.streams[fd] = null;
      },joinPath:function (parts, forceRelative) {
        var ret = parts[0];
        for (var i = 1; i < parts.length; i++) {
          if (ret[ret.length-1] != '/') ret += '/';
          ret += parts[i];
        }
        if (forceRelative && ret[0] == '/') ret = ret.substr(1);
        return ret;
      },absolutePath:function (relative, base) {
        if (typeof relative !== 'string') return null;
        if (base === undefined) base = FS.currentPath;
        if (relative && relative[0] == '/') base = '';
        var full = base + '/' + relative;
        var parts = full.split('/').reverse();
        var absolute = [''];
        while (parts.length) {
          var part = parts.pop();
          if (part == '' || part == '.') {
            // Nothing.
          } else if (part == '..') {
            if (absolute.length > 1) absolute.pop();
          } else {
            absolute.push(part);
          }
        }
        return absolute.length == 1 ? '/' : absolute.join('/');
      },analyzePath:function (path, dontResolveLastLink, linksVisited) {
        var ret = {
          isRoot: false,
          exists: false,
          error: 0,
          name: null,
          path: null,
          object: null,
          parentExists: false,
          parentPath: null,
          parentObject: null
        };
        path = FS.absolutePath(path);
        if (path == '/') {
          ret.isRoot = true;
          ret.exists = ret.parentExists = true;
          ret.name = '/';
          ret.path = ret.parentPath = '/';
          ret.object = ret.parentObject = FS.root;
        } else if (path !== null) {
          linksVisited = linksVisited || 0;
          path = path.slice(1).split('/');
          var current = FS.root;
          var traversed = [''];
          while (path.length) {
            if (path.length == 1 && current.isFolder) {
              ret.parentExists = true;
              ret.parentPath = traversed.length == 1 ? '/' : traversed.join('/');
              ret.parentObject = current;
              ret.name = path[0];
            }
            var target = path.shift();
            if (!current.isFolder) {
              ret.error = ERRNO_CODES.ENOTDIR;
              break;
            } else if (!current.read) {
              ret.error = ERRNO_CODES.EACCES;
              break;
            } else if (!current.contents.hasOwnProperty(target)) {
              ret.error = ERRNO_CODES.ENOENT;
              break;
            }
            current = current.contents[target];
            if (current.link && !(dontResolveLastLink && path.length == 0)) {
              if (linksVisited > 40) { // Usual Linux SYMLOOP_MAX.
                ret.error = ERRNO_CODES.ELOOP;
                break;
              }
              var link = FS.absolutePath(current.link, traversed.join('/'));
              ret = FS.analyzePath([link].concat(path).join('/'),
                                   dontResolveLastLink, linksVisited + 1);
              return ret;
            }
            traversed.push(target);
            if (path.length == 0) {
              ret.exists = true;
              ret.path = traversed.join('/');
              ret.object = current;
            }
          }
        }
        return ret;
      },findObject:function (path, dontResolveLastLink) {
        FS.ensureRoot();
        var ret = FS.analyzePath(path, dontResolveLastLink);
        if (ret.exists) {
          return ret.object;
        } else {
          ___setErrNo(ret.error);
          return null;
        }
      },createObject:function (parent, name, properties, canRead, canWrite) {
        if (!parent) parent = '/';
        if (typeof parent === 'string') parent = FS.findObject(parent);
        if (!parent) {
          ___setErrNo(ERRNO_CODES.EACCES);
          throw new Error('Parent path must exist.');
        }
        if (!parent.isFolder) {
          ___setErrNo(ERRNO_CODES.ENOTDIR);
          throw new Error('Parent must be a folder.');
        }
        if (!parent.write && !FS.ignorePermissions) {
          ___setErrNo(ERRNO_CODES.EACCES);
          throw new Error('Parent folder must be writeable.');
        }
        if (!name || name == '.' || name == '..') {
          ___setErrNo(ERRNO_CODES.ENOENT);
          throw new Error('Name must not be empty.');
        }
        if (parent.contents.hasOwnProperty(name)) {
          ___setErrNo(ERRNO_CODES.EEXIST);
          throw new Error("Can't overwrite object.");
        }
        parent.contents[name] = {
          read: canRead === undefined ? true : canRead,
          write: canWrite === undefined ? false : canWrite,
          timestamp: Date.now(),
          inodeNumber: FS.nextInode++
        };
        for (var key in properties) {
          if (properties.hasOwnProperty(key)) {
            parent.contents[name][key] = properties[key];
          }
        }
        return parent.contents[name];
      },createFolder:function (parent, name, canRead, canWrite) {
        var properties = {isFolder: true, isDevice: false, contents: {}};
        return FS.createObject(parent, name, properties, canRead, canWrite);
      },createPath:function (parent, path, canRead, canWrite) {
        var current = FS.findObject(parent);
        if (current === null) throw new Error('Invalid parent.');
        path = path.split('/').reverse();
        while (path.length) {
          var part = path.pop();
          if (!part) continue;
          if (!current.contents.hasOwnProperty(part)) {
            FS.createFolder(current, part, canRead, canWrite);
          }
          current = current.contents[part];
        }
        return current;
      },createFile:function (parent, name, properties, canRead, canWrite) {
        properties.isFolder = false;
        return FS.createObject(parent, name, properties, canRead, canWrite);
      },createDataFile:function (parent, name, data, canRead, canWrite) {
        if (typeof data === 'string') {
          var dataArray = new Array(data.length);
          for (var i = 0, len = data.length; i < len; ++i) dataArray[i] = data.charCodeAt(i);
          data = dataArray;
        }
        var properties = {
          isDevice: false,
          contents: data.subarray ? data.subarray(0) : data // as an optimization, create a new array wrapper (not buffer) here, to help JS engines understand this object
        };
        return FS.createFile(parent, name, properties, canRead, canWrite);
      },createLazyFile:function (parent, name, url, canRead, canWrite) {
        if (typeof XMLHttpRequest !== 'undefined') {
          if (!ENVIRONMENT_IS_WORKER) throw 'Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc';
          // Lazy chunked Uint8Array (implements get and length from Uint8Array). Actual getting is abstracted away for eventual reuse.
          var LazyUint8Array = function() {
            this.lengthKnown = false;
            this.chunks = []; // Loaded chunks. Index is the chunk number
          }
          LazyUint8Array.prototype.get = function(idx) {
            if (idx > this.length-1 || idx < 0) {
              return undefined;
            }
            var chunkOffset = idx % this.chunkSize;
            var chunkNum = Math.floor(idx / this.chunkSize);
            return this.getter(chunkNum)[chunkOffset];
          }
          LazyUint8Array.prototype.setDataGetter = function(getter) {
            this.getter = getter;
          }
          LazyUint8Array.prototype.cacheLength = function() {
              // Find length
              var xhr = new XMLHttpRequest();
              xhr.open('HEAD', url, false);
              xhr.send(null);
              if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
              var datalength = Number(xhr.getResponseHeader("Content-length"));
              var header;
              var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
              var chunkSize = 1024*1024; // Chunk size in bytes
              if (!hasByteServing) chunkSize = datalength;
              // Function to get a range from the remote URL.
              var doXHR = (function(from, to) {
                if (from > to) throw new Error("invalid range (" + from + ", " + to + ") or no bytes requested!");
                if (to > datalength-1) throw new Error("only " + datalength + " bytes available! programmer error!");
                // TODO: Use mozResponseArrayBuffer, responseStream, etc. if available.
                var xhr = new XMLHttpRequest();
                xhr.open('GET', url, false);
                if (datalength !== chunkSize) xhr.setRequestHeader("Range", "bytes=" + from + "-" + to);
                // Some hints to the browser that we want binary data.
                if (typeof Uint8Array != 'undefined') xhr.responseType = 'arraybuffer';
                if (xhr.overrideMimeType) {
                  xhr.overrideMimeType('text/plain; charset=x-user-defined');
                }
                xhr.send(null);
                if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
                if (xhr.response !== undefined) {
                  return new Uint8Array(xhr.response || []);
                } else {
                  return intArrayFromString(xhr.responseText || '', true);
                }
              });
              var lazyArray = this;
              lazyArray.setDataGetter(function(chunkNum) {
                var start = chunkNum * chunkSize;
                var end = (chunkNum+1) * chunkSize - 1; // including this byte
                end = Math.min(end, datalength-1); // if datalength-1 is selected, this is the last block
                if (typeof(lazyArray.chunks[chunkNum]) === "undefined") {
                  lazyArray.chunks[chunkNum] = doXHR(start, end);
                }
                if (typeof(lazyArray.chunks[chunkNum]) === "undefined") throw new Error("doXHR failed!");
                return lazyArray.chunks[chunkNum];
              });
              this._length = datalength;
              this._chunkSize = chunkSize;
              this.lengthKnown = true;
          }
          var lazyArray = new LazyUint8Array();
          Object.defineProperty(lazyArray, "length", {
              get: function() {
                  if(!this.lengthKnown) {
                      this.cacheLength();
                  }
                  return this._length;
              }
          });
          Object.defineProperty(lazyArray, "chunkSize", {
              get: function() {
                  if(!this.lengthKnown) {
                      this.cacheLength();
                  }
                  return this._chunkSize;
              }
          });
          var properties = { isDevice: false, contents: lazyArray };
        } else {
          var properties = { isDevice: false, url: url };
        }
        return FS.createFile(parent, name, properties, canRead, canWrite);
      },createPreloadedFile:function (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile) {
        Browser.init();
        var fullname = FS.joinPath([parent, name], true);
        function processData(byteArray) {
          function finish(byteArray) {
            if (!dontCreateFile) {
              FS.createDataFile(parent, name, byteArray, canRead, canWrite);
            }
            if (onload) onload();
            removeRunDependency('cp ' + fullname);
          }
          var handled = false;
          Module['preloadPlugins'].forEach(function(plugin) {
            if (handled) return;
            if (plugin['canHandle'](fullname)) {
              plugin['handle'](byteArray, fullname, finish, function() {
                if (onerror) onerror();
                removeRunDependency('cp ' + fullname);
              });
              handled = true;
            }
          });
          if (!handled) finish(byteArray);
        }
        addRunDependency('cp ' + fullname);
        if (typeof url == 'string') {
          Browser.asyncLoad(url, function(byteArray) {
            processData(byteArray);
          }, onerror);
        } else {
          processData(url);
        }
      },createLink:function (parent, name, target, canRead, canWrite) {
        var properties = {isDevice: false, link: target};
        return FS.createFile(parent, name, properties, canRead, canWrite);
      },createDevice:function (parent, name, input, output) {
        if (!(input || output)) {
          throw new Error('A device must have at least one callback defined.');
        }
        var ops = {isDevice: true, input: input, output: output};
        return FS.createFile(parent, name, ops, Boolean(input), Boolean(output));
      },forceLoadFile:function (obj) {
        if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;
        var success = true;
        if (typeof XMLHttpRequest !== 'undefined') {
          throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
        } else if (Module['read']) {
          // Command-line.
          try {
            // WARNING: Can't read binary files in V8's d8 or tracemonkey's js, as
            //          read() will try to parse UTF8.
            obj.contents = intArrayFromString(Module['read'](obj.url), true);
          } catch (e) {
            success = false;
          }
        } else {
          throw new Error('Cannot load without read() or XMLHttpRequest.');
        }
        if (!success) ___setErrNo(ERRNO_CODES.EIO);
        return success;
      },ensureRoot:function () {
        if (FS.root) return;
        // The main file system tree. All the contents are inside this.
        FS.root = {
          read: true,
          write: true,
          isFolder: true,
          isDevice: false,
          timestamp: Date.now(),
          inodeNumber: 1,
          contents: {}
        };
      },init:function (input, output, error) {
        // Make sure we initialize only once.
        assert(!FS.init.initialized, 'FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)');
        FS.init.initialized = true;
        FS.ensureRoot();
        // Allow Module.stdin etc. to provide defaults, if none explicitly passed to us here
        input = input || Module['stdin'];
        output = output || Module['stdout'];
        error = error || Module['stderr'];
        // Default handlers.
        var stdinOverridden = true, stdoutOverridden = true, stderrOverridden = true;
        if (!input) {
          stdinOverridden = false;
          input = function() {
            if (!input.cache || !input.cache.length) {
              var result;
              if (typeof window != 'undefined' &&
                  typeof window.prompt == 'function') {
                // Browser.
                result = window.prompt('Input: ');
                if (result === null) result = String.fromCharCode(0); // cancel ==> EOF
              } else if (typeof readline == 'function') {
                // Command line.
                result = readline();
              }
              if (!result) result = '';
              input.cache = intArrayFromString(result + '\n', true);
            }
            return input.cache.shift();
          };
        }
        var utf8 = new Runtime.UTF8Processor();
        function simpleOutput(val) {
          if (val === null || val === 10) {
            output.printer(output.buffer.join(''));
            output.buffer = [];
          } else {
            output.buffer.push(utf8.processCChar(val));
          }
        }
        if (!output) {
          stdoutOverridden = false;
          output = simpleOutput;
        }
        if (!output.printer) output.printer = Module['print'];
        if (!output.buffer) output.buffer = [];
        if (!error) {
          stderrOverridden = false;
          error = simpleOutput;
        }
        if (!error.printer) error.printer = Module['print'];
        if (!error.buffer) error.buffer = [];
        // Create the temporary folder, if not already created
        try {
          FS.createFolder('/', 'tmp', true, true);
        } catch(e) {}
        // Create the I/O devices.
        var devFolder = FS.createFolder('/', 'dev', true, true);
        var stdin = FS.createDevice(devFolder, 'stdin', input);
        var stdout = FS.createDevice(devFolder, 'stdout', null, output);
        var stderr = FS.createDevice(devFolder, 'stderr', null, error);
        FS.createDevice(devFolder, 'tty', input, output);
        FS.createDevice(devFolder, 'null', function(){}, function(){});
        // Create default streams.
        FS.streams[1] = {
          path: '/dev/stdin',
          object: stdin,
          position: 0,
          isRead: true,
          isWrite: false,
          isAppend: false,
          isTerminal: !stdinOverridden,
          error: false,
          eof: false,
          ungotten: []
        };
        FS.streams[2] = {
          path: '/dev/stdout',
          object: stdout,
          position: 0,
          isRead: false,
          isWrite: true,
          isAppend: false,
          isTerminal: !stdoutOverridden,
          error: false,
          eof: false,
          ungotten: []
        };
        FS.streams[3] = {
          path: '/dev/stderr',
          object: stderr,
          position: 0,
          isRead: false,
          isWrite: true,
          isAppend: false,
          isTerminal: !stderrOverridden,
          error: false,
          eof: false,
          ungotten: []
        };
        // TODO: put these low in memory like we used to assert on: assert(Math.max(_stdin, _stdout, _stderr) < 15000); // make sure these are low, we flatten arrays with these
        HEAP32[((_stdin)>>2)]=1;
        HEAP32[((_stdout)>>2)]=2;
        HEAP32[((_stderr)>>2)]=3;
        // Other system paths
        FS.createPath('/', 'dev/shm/tmp', true, true); // temp files
        // Newlib initialization
        for (var i = FS.streams.length; i < Math.max(_stdin, _stdout, _stderr) + 4; i++) {
          FS.streams[i] = null; // Make sure to keep FS.streams dense
        }
        FS.streams[_stdin] = FS.streams[1];
        FS.streams[_stdout] = FS.streams[2];
        FS.streams[_stderr] = FS.streams[3];
        allocate([ allocate(
          [0, 0, 0, 0, _stdin, 0, 0, 0, _stdout, 0, 0, 0, _stderr, 0, 0, 0],
          'void*', ALLOC_NORMAL) ], 'void*', ALLOC_NONE, __impure_ptr);
      },quit:function () {
        if (!FS.init.initialized) return;
        // Flush any partially-printed lines in stdout and stderr. Careful, they may have been closed
        if (FS.streams[2] && FS.streams[2].object.output.buffer.length > 0) FS.streams[2].object.output(10);
        if (FS.streams[3] && FS.streams[3].object.output.buffer.length > 0) FS.streams[3].object.output(10);
      },standardizePath:function (path) {
        if (path.substr(0, 2) == './') path = path.substr(2);
        return path;
      },deleteFile:function (path) {
        path = FS.analyzePath(path);
        if (!path.parentExists || !path.exists) {
          throw 'Invalid path ' + path;
        }
        delete path.parentObject.contents[path.name];
      }};function _lseek(fildes, offset, whence) {
      // off_t lseek(int fildes, off_t offset, int whence);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/lseek.html
      if (FS.streams[fildes] && !FS.streams[fildes].object.isDevice) {
        var stream = FS.streams[fildes];
        var position = offset;
        if (whence === 1) {  // SEEK_CUR.
          position += stream.position;
        } else if (whence === 2) {  // SEEK_END.
          position += stream.object.contents.length;
        }
        if (position < 0) {
          ___setErrNo(ERRNO_CODES.EINVAL);
          return -1;
        } else {
          stream.ungotten = [];
          stream.position = position;
          return position;
        }
      } else {
        ___setErrNo(ERRNO_CODES.EBADF);
        return -1;
      }
    }
  Module["_strlen"] = _strlen;
  Module["_strcpy"] = _strcpy;
  Module["_strcat"] = _strcat;
  var ___dirent_struct_layout={__size__:1040,d_ino:0,d_name:4,d_off:1028,d_reclen:1032,d_type:1036};function _open(path, oflag, varargs) {
      // int open(const char *path, int oflag, ...);
      // http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html
      // NOTE: This implementation tries to mimic glibc rather than strictly
      // following the POSIX standard.
      var mode = HEAP32[((varargs)>>2)];
      // Simplify flags.
      var accessMode = oflag & 3;
      var isWrite = accessMode != 0;
      var isRead = accessMode != 1;
      var isCreate = Boolean(oflag & 512);
      var isExistCheck = Boolean(oflag & 2048);
      var isTruncate = Boolean(oflag & 1024);
      var isAppend = Boolean(oflag & 8);
      // Verify path.
      var origPath = path;
      path = FS.analyzePath(Pointer_stringify(path));
      if (!path.parentExists) {
        ___setErrNo(path.error);
        return -1;
      }
      var target = path.object || null;
      var finalPath;
      // Verify the file exists, create if needed and allowed.
      if (target) {
        if (isCreate && isExistCheck) {
          ___setErrNo(ERRNO_CODES.EEXIST);
          return -1;
        }
        if ((isWrite || isCreate || isTruncate) && target.isFolder) {
          ___setErrNo(ERRNO_CODES.EISDIR);
          return -1;
        }
        if (isRead && !target.read || isWrite && !target.write) {
          ___setErrNo(ERRNO_CODES.EACCES);
          return -1;
        }
        if (isTruncate && !target.isDevice) {
          target.contents = [];
        } else {
          if (!FS.forceLoadFile(target)) {
            ___setErrNo(ERRNO_CODES.EIO);
            return -1;
          }
        }
        finalPath = path.path;
      } else {
        if (!isCreate) {
          ___setErrNo(ERRNO_CODES.ENOENT);
          return -1;
        }
        if (!path.parentObject.write) {
          ___setErrNo(ERRNO_CODES.EACCES);
          return -1;
        }
        target = FS.createDataFile(path.parentObject, path.name, [],
                                   mode & 0x100, mode & 0x80);  // S_IRUSR, S_IWUSR.
        finalPath = path.parentPath + '/' + path.name;
      }
      // Actually create an open stream.
      var id;
      if (target.isFolder) {
        var entryBuffer = 0;
        if (___dirent_struct_layout) {
          entryBuffer = _malloc(___dirent_struct_layout.__size__);
        }
        var contents = [];
        for (var key in target.contents) contents.push(key);
        id = FS.createFileHandle({
          path: finalPath,
          object: target,
          // An index into contents. Special values: -2 is ".", -1 is "..".
          position: -2,
          isRead: true,
          isWrite: false,
          isAppend: false,
          error: false,
          eof: false,
          ungotten: [],
          // Folder-specific properties:
          // Remember the contents at the time of opening in an array, so we can
          // seek between them relying on a single order.
          contents: contents,
          // Each stream has its own area for readdir() returns.
          currentEntry: entryBuffer
        });
      } else {
        id = FS.createFileHandle({
          path: finalPath,
          object: target,
          position: 0,
          isRead: isRead,
          isWrite: isWrite,
          isAppend: isAppend,
          error: false,
          eof: false,
          ungotten: []
        });
      }
      return id;
    }
  function _close(fildes) {
      // int close(int fildes);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/close.html
      if (FS.streams[fildes]) {
        if (FS.streams[fildes].currentEntry) {
          _free(FS.streams[fildes].currentEntry);
        }
        FS.streams[fildes] = null;
        return 0;
      } else {
        ___setErrNo(ERRNO_CODES.EBADF);
        return -1;
      }
    }
  function _recv(fd, buf, len, flags) {
      var info = FS.streams[fd];
      if (!info) return -1;
      if (!info.hasData()) {
        ___setErrNo(ERRNO_CODES.EAGAIN); // no data, and all sockets are nonblocking, so this is the right behavior
        return -1;
      }
      var buffer = info.inQueue.shift();
      if (len < buffer.length) {
        if (info.stream) {
          // This is tcp (reliable), so if not all was read, keep it
          info.inQueue.unshift(buffer.subarray(len));
        }
        buffer = buffer.subarray(0, len);
      }
      HEAPU8.set(buffer, buf);
      return buffer.length;
    }
  function _pread(fildes, buf, nbyte, offset) {
      // ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/read.html
      var stream = FS.streams[fildes];
      if (!stream || stream.object.isDevice) {
        ___setErrNo(ERRNO_CODES.EBADF);
        return -1;
      } else if (!stream.isRead) {
        ___setErrNo(ERRNO_CODES.EACCES);
        return -1;
      } else if (stream.object.isFolder) {
        ___setErrNo(ERRNO_CODES.EISDIR);
        return -1;
      } else if (nbyte < 0 || offset < 0) {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      } else {
        var bytesRead = 0;
        while (stream.ungotten.length && nbyte > 0) {
          HEAP8[((buf++)|0)]=stream.ungotten.pop()
          nbyte--;
          bytesRead++;
        }
        var contents = stream.object.contents;
        var size = Math.min(contents.length - offset, nbyte);
        if (contents.subarray) { // typed array
          HEAPU8.set(contents.subarray(offset, offset+size), buf);
        } else
        if (contents.slice) { // normal array
          for (var i = 0; i < size; i++) {
            HEAP8[(((buf)+(i))|0)]=contents[offset + i]
          }
        } else {
          for (var i = 0; i < size; i++) { // LazyUint8Array from sync binary XHR
            HEAP8[(((buf)+(i))|0)]=contents.get(offset + i)
          }
        }
        bytesRead += size;
        return bytesRead;
      }
    }function _read(fildes, buf, nbyte) {
      // ssize_t read(int fildes, void *buf, size_t nbyte);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/read.html
      var stream = FS.streams[fildes];
      if (stream && ('socket' in stream)) {
        return _recv(fildes, buf, nbyte, 0);
      } else if (!stream) {
        ___setErrNo(ERRNO_CODES.EBADF);
        return -1;
      } else if (!stream.isRead) {
        ___setErrNo(ERRNO_CODES.EACCES);
        return -1;
      } else if (nbyte < 0) {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      } else {
        var bytesRead;
        if (stream.object.isDevice) {
          if (stream.object.input) {
            bytesRead = 0;
            while (stream.ungotten.length && nbyte > 0) {
              HEAP8[((buf++)|0)]=stream.ungotten.pop()
              nbyte--;
              bytesRead++;
            }
            for (var i = 0; i < nbyte; i++) {
              try {
                var result = stream.object.input();
              } catch (e) {
                ___setErrNo(ERRNO_CODES.EIO);
                return -1;
              }
              if (result === undefined && bytesRead === 0) {
                ___setErrNo(ERRNO_CODES.EAGAIN);
                return -1;
              }
              if (result === null || result === undefined) break;
              bytesRead++;
              HEAP8[(((buf)+(i))|0)]=result
            }
            return bytesRead;
          } else {
            ___setErrNo(ERRNO_CODES.ENXIO);
            return -1;
          }
        } else {
          var ungotSize = stream.ungotten.length;
          bytesRead = _pread(fildes, buf, nbyte, stream.position);
          if (bytesRead != -1) {
            stream.position += (stream.ungotten.length - ungotSize) + bytesRead;
          }
          return bytesRead;
        }
      }
    }
  var ERRNO_MESSAGES={0:"Success",1:"Not super-user",2:"No such file or directory",3:"No such process",4:"Interrupted system call",5:"I/O error",6:"No such device or address",7:"Arg list too long",8:"Exec format error",9:"Bad file number",10:"No children",11:"No more processes",12:"Not enough core",13:"Permission denied",14:"Bad address",15:"Block device required",16:"Mount device busy",17:"File exists",18:"Cross-device link",19:"No such device",20:"Not a directory",21:"Is a directory",22:"Invalid argument",23:"Too many open files in system",24:"Too many open files",25:"Not a typewriter",26:"Text file busy",27:"File too large",28:"No space left on device",29:"Illegal seek",30:"Read only file system",31:"Too many links",32:"Broken pipe",33:"Math arg out of domain of func",34:"Math result not representable",35:"No message of desired type",36:"Identifier removed",37:"Channel number out of range",38:"Level 2 not synchronized",39:"Level 3 halted",40:"Level 3 reset",41:"Link number out of range",42:"Protocol driver not attached",43:"No CSI structure available",44:"Level 2 halted",45:"Deadlock condition",46:"No record locks available",50:"Invalid exchange",51:"Invalid request descriptor",52:"Exchange full",53:"No anode",54:"Invalid request code",55:"Invalid slot",56:"File locking deadlock error",57:"Bad font file fmt",60:"Device not a stream",61:"No data (for no delay io)",62:"Timer expired",63:"Out of streams resources",64:"Machine is not on the network",65:"Package not installed",66:"The object is remote",67:"The link has been severed",68:"Advertise error",69:"Srmount error",70:"Communication error on send",71:"Protocol error",74:"Multihop attempted",75:"Inode is remote (not really error)",76:"Cross mount point (not really error)",77:"Trying to read unreadable message",79:"Inappropriate file type or format",80:"Given log. name not unique",81:"f.d. invalid for this operation",82:"Remote address changed",83:"Can\t access a needed shared lib",84:"Accessing a corrupted shared lib",85:".lib section in a.out corrupted",86:"Attempting to link in too many libs",87:"Attempting to exec a shared library",88:"Function not implemented",89:"No more files",90:"Directory not empty",91:"File or path name too long",92:"Too many symbolic links",95:"Operation not supported on transport endpoint",96:"Protocol family not supported",104:"Connection reset by peer",105:"No buffer space available",106:"Address family not supported by protocol family",107:"Protocol wrong type for socket",108:"Socket operation on non-socket",109:"Protocol not available",110:"Can't send after socket shutdown",111:"Connection refused",112:"Address already in use",113:"Connection aborted",114:"Network is unreachable",115:"Network interface is not configured",116:"Connection timed out",117:"Host is down",118:"Host is unreachable",119:"Connection already in progress",120:"Socket already connected",121:"Destination address required",122:"Message too long",123:"Unknown protocol",124:"Socket type not supported",125:"Address not available",126:"ENETRESET",127:"Socket is already connected",128:"Socket is not connected",129:"TOOMANYREFS",130:"EPROCLIM",131:"EUSERS",132:"EDQUOT",133:"ESTALE",134:"Not supported",135:"No medium (in tape drive)",136:"No such host or network path",137:"Filename exists with different case",138:"EILSEQ",139:"Value too large for defined data type",140:"Operation canceled",141:"State not recoverable",142:"Previous owner died",143:"Streams pipe error"};function _strerror_r(errnum, strerrbuf, buflen) {
      if (errnum in ERRNO_MESSAGES) {
        if (ERRNO_MESSAGES[errnum].length > buflen - 1) {
          return ___setErrNo(ERRNO_CODES.ERANGE);
        } else {
          var msg = ERRNO_MESSAGES[errnum];
          for (var i = 0; i < msg.length; i++) {
            HEAP8[(((strerrbuf)+(i))|0)]=msg.charCodeAt(i)
          }
          HEAP8[(((strerrbuf)+(i))|0)]=0
          return 0;
        }
      } else {
        return ___setErrNo(ERRNO_CODES.EINVAL);
      }
    }function _strerror(errnum) {
      if (!_strerror.buffer) _strerror.buffer = _malloc(256);
      _strerror_r(errnum, _strerror.buffer, 256);
      return _strerror.buffer;
    }
  function ___errno_location() {
      return ___errno_state;
    }var ___errno=___errno_location;
  function _send(fd, buf, len, flags) {
      var info = FS.streams[fd];
      if (!info) return -1;
      info.sender(HEAPU8.subarray(buf, buf+len));
      return len;
    }
  function _pwrite(fildes, buf, nbyte, offset) {
      // ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/write.html
      var stream = FS.streams[fildes];
      if (!stream || stream.object.isDevice) {
        ___setErrNo(ERRNO_CODES.EBADF);
        return -1;
      } else if (!stream.isWrite) {
        ___setErrNo(ERRNO_CODES.EACCES);
        return -1;
      } else if (stream.object.isFolder) {
        ___setErrNo(ERRNO_CODES.EISDIR);
        return -1;
      } else if (nbyte < 0 || offset < 0) {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      } else {
        var contents = stream.object.contents;
        while (contents.length < offset) contents.push(0);
        for (var i = 0; i < nbyte; i++) {
          contents[offset + i] = HEAPU8[(((buf)+(i))|0)];
        }
        stream.object.timestamp = Date.now();
        return i;
      }
    }function _write(fildes, buf, nbyte) {
      // ssize_t write(int fildes, const void *buf, size_t nbyte);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/write.html
      var stream = FS.streams[fildes];
      if (stream && ('socket' in stream)) {
          return _send(fildes, buf, nbyte, 0);
      } else if (!stream) {
        ___setErrNo(ERRNO_CODES.EBADF);
        return -1;
      } else if (!stream.isWrite) {
        ___setErrNo(ERRNO_CODES.EACCES);
        return -1;
      } else if (nbyte < 0) {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      } else {
        if (stream.object.isDevice) {
          if (stream.object.output) {
            for (var i = 0; i < nbyte; i++) {
              try {
                stream.object.output(HEAP8[(((buf)+(i))|0)]);
              } catch (e) {
                ___setErrNo(ERRNO_CODES.EIO);
                return -1;
              }
            }
            stream.object.timestamp = Date.now();
            return i;
          } else {
            ___setErrNo(ERRNO_CODES.ENXIO);
            return -1;
          }
        } else {
          var bytesWritten = _pwrite(fildes, buf, nbyte, stream.position);
          if (bytesWritten != -1) stream.position += bytesWritten;
          return bytesWritten;
        }
      }
    }
  function _llvm_bswap_i32(x) {
      return ((x&0xff)<<24) | (((x>>8)&0xff)<<16) | (((x>>16)&0xff)<<8) | (x>>>24);
    }
  function _abort() {
      ABORT = true;
      throw 'abort() at ' + (new Error().stack);
    }
  function _sysconf(name) {
      // long sysconf(int name);
      // http://pubs.opengroup.org/onlinepubs/009695399/functions/sysconf.html
      switch(name) {
        case 8: return PAGE_SIZE;
        case 54:
        case 56:
        case 21:
        case 61:
        case 63:
        case 22:
        case 67:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 69:
        case 28:
        case 101:
        case 70:
        case 71:
        case 29:
        case 30:
        case 199:
        case 75:
        case 76:
        case 32:
        case 43:
        case 44:
        case 80:
        case 46:
        case 47:
        case 45:
        case 48:
        case 49:
        case 42:
        case 82:
        case 33:
        case 7:
        case 108:
        case 109:
        case 107:
        case 112:
        case 119:
        case 121:
          return 200809;
        case 13:
        case 104:
        case 94:
        case 95:
        case 34:
        case 35:
        case 77:
        case 81:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
        case 88:
        case 89:
        case 90:
        case 91:
        case 94:
        case 95:
        case 110:
        case 111:
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118:
        case 120:
        case 40:
        case 16:
        case 79:
        case 19:
          return -1;
        case 92:
        case 93:
        case 5:
        case 72:
        case 6:
        case 74:
        case 92:
        case 93:
        case 96:
        case 97:
        case 98:
        case 99:
        case 102:
        case 103:
        case 105:
          return 1;
        case 38:
        case 66:
        case 50:
        case 51:
        case 4:
          return 1024;
        case 15:
        case 64:
        case 41:
          return 32;
        case 55:
        case 37:
        case 17:
          return 2147483647;
        case 18:
        case 1:
          return 47839;
        case 59:
        case 57:
          return 99;
        case 68:
        case 58:
          return 2048;
        case 0: return 2097152;
        case 3: return 65536;
        case 14: return 32768;
        case 73: return 32767;
        case 39: return 16384;
        case 60: return 1000;
        case 106: return 700;
        case 52: return 256;
        case 62: return 255;
        case 2: return 100;
        case 65: return 64;
        case 36: return 20;
        case 100: return 16;
        case 20: return 6;
        case 53: return 4;
        case 10: return 1;
      }
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
  function _time(ptr) {
      var ret = Math.floor(Date.now()/1000);
      if (ptr) {
        HEAP32[((ptr)>>2)]=ret
      }
      return ret;
    }
  function _sbrk(bytes) {
      // Implement a Linux-like 'memory area' for our 'process'.
      // Changes the size of the memory area by |bytes|; returns the
      // address of the previous top ('break') of the memory area
      // We control the "dynamic" memory - DYNAMIC_BASE to DYNAMICTOP
      var self = _sbrk;
      if (!self.called) {
        DYNAMICTOP = alignMemoryPage(DYNAMICTOP); // make sure we start out aligned
        self.called = true;
        assert(Runtime.dynamicAlloc);
        self.alloc = Runtime.dynamicAlloc;
        Runtime.dynamicAlloc = function() { abort('cannot dynamically allocate, sbrk now has control') };
      }
      var ret = DYNAMICTOP;
      if (bytes != 0) self.alloc(bytes);
      return ret;  // Previous break location.
    }
  var Browser={mainLoop:{scheduler:null,shouldPause:false,paused:false,queue:[],pause:function () {
          Browser.mainLoop.shouldPause = true;
        },resume:function () {
          if (Browser.mainLoop.paused) {
            Browser.mainLoop.paused = false;
            Browser.mainLoop.scheduler();
          }
          Browser.mainLoop.shouldPause = false;
        },updateStatus:function () {
          if (Module['setStatus']) {
            var message = Module['statusMessage'] || 'Please wait...';
            var remaining = Browser.mainLoop.remainingBlockers;
            var expected = Browser.mainLoop.expectedBlockers;
            if (remaining) {
              if (remaining < expected) {
                Module['setStatus'](message + ' (' + (expected - remaining) + '/' + expected + ')');
              } else {
                Module['setStatus'](message);
              }
            } else {
              Module['setStatus']('');
            }
          }
        }},isFullScreen:false,pointerLock:false,moduleContextCreatedCallbacks:[],workers:[],init:function () {
        if (!Module["preloadPlugins"]) Module["preloadPlugins"] = []; // needs to exist even in workers
        if (Browser.initted || ENVIRONMENT_IS_WORKER) return;
        Browser.initted = true;
        try {
          new Blob();
          Browser.hasBlobConstructor = true;
        } catch(e) {
          Browser.hasBlobConstructor = false;
          console.log("warning: no blob constructor, cannot create blobs with mimetypes");
        }
        Browser.BlobBuilder = typeof MozBlobBuilder != "undefined" ? MozBlobBuilder : (typeof WebKitBlobBuilder != "undefined" ? WebKitBlobBuilder : (!Browser.hasBlobConstructor ? console.log("warning: no BlobBuilder") : null));
        Browser.URLObject = typeof window != "undefined" ? (window.URL ? window.URL : window.webkitURL) : console.log("warning: cannot create object URLs");
        // Support for plugins that can process preloaded files. You can add more of these to
        // your app by creating and appending to Module.preloadPlugins.
        //
        // Each plugin is asked if it can handle a file based on the file's name. If it can,
        // it is given the file's raw data. When it is done, it calls a callback with the file's
        // (possibly modified) data. For example, a plugin might decompress a file, or it
        // might create some side data structure for use later (like an Image element, etc.).
        function getMimetype(name) {
          return {
            'jpg': 'image/jpeg',
            'jpeg': 'image/jpeg',
            'png': 'image/png',
            'bmp': 'image/bmp',
            'ogg': 'audio/ogg',
            'wav': 'audio/wav',
            'mp3': 'audio/mpeg'
          }[name.substr(name.lastIndexOf('.')+1)];
        }
        var imagePlugin = {};
        imagePlugin['canHandle'] = function(name) {
          return !Module.noImageDecoding && /\.(jpg|jpeg|png|bmp)$/i.test(name);
        };
        imagePlugin['handle'] = function(byteArray, name, onload, onerror) {
          var b = null;
          if (Browser.hasBlobConstructor) {
            try {
              b = new Blob([byteArray], { type: getMimetype(name) });
            } catch(e) {
              Runtime.warnOnce('Blob constructor present but fails: ' + e + '; falling back to blob builder');
            }
          }
          if (!b) {
            var bb = new Browser.BlobBuilder();
            bb.append((new Uint8Array(byteArray)).buffer); // we need to pass a buffer, and must copy the array to get the right data range
            b = bb.getBlob();
          }
          var url = Browser.URLObject.createObjectURL(b);
          var img = new Image();
          img.onload = function() {
            assert(img.complete, 'Image ' + name + ' could not be decoded');
            var canvas = document.createElement('canvas');
            canvas.width = img.width;
            canvas.height = img.height;
            var ctx = canvas.getContext('2d');
            ctx.drawImage(img, 0, 0);
            Module["preloadedImages"][name] = canvas;
            Browser.URLObject.revokeObjectURL(url);
            if (onload) onload(byteArray);
          };
          img.onerror = function(event) {
            console.log('Image ' + url + ' could not be decoded');
            if (onerror) onerror();
          };
          img.src = url;
        };
        Module['preloadPlugins'].push(imagePlugin);
        var audioPlugin = {};
        audioPlugin['canHandle'] = function(name) {
          return !Module.noAudioDecoding && name.substr(-4) in { '.ogg': 1, '.wav': 1, '.mp3': 1 };
        };
        audioPlugin['handle'] = function(byteArray, name, onload, onerror) {
          var done = false;
          function finish(audio) {
            if (done) return;
            done = true;
            Module["preloadedAudios"][name] = audio;
            if (onload) onload(byteArray);
          }
          function fail() {
            if (done) return;
            done = true;
            Module["preloadedAudios"][name] = new Audio(); // empty shim
            if (onerror) onerror();
          }
          if (Browser.hasBlobConstructor) {
            try {
              var b = new Blob([byteArray], { type: getMimetype(name) });
            } catch(e) {
              return fail();
            }
            var url = Browser.URLObject.createObjectURL(b); // XXX we never revoke this!
            var audio = new Audio();
            audio.addEventListener('canplaythrough', function() { finish(audio) }, false); // use addEventListener due to chromium bug 124926
            audio.onerror = function(event) {
              if (done) return;
              console.log('warning: browser could not fully decode audio ' + name + ', trying slower base64 approach');
              function encode64(data) {
                var BASE = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
                var PAD = '=';
                var ret = '';
                var leftchar = 0;
                var leftbits = 0;
                for (var i = 0; i < data.length; i++) {
                  leftchar = (leftchar << 8) | data[i];
                  leftbits += 8;
                  while (leftbits >= 6) {
                    var curr = (leftchar >> (leftbits-6)) & 0x3f;
                    leftbits -= 6;
                    ret += BASE[curr];
                  }
                }
                if (leftbits == 2) {
                  ret += BASE[(leftchar&3) << 4];
                  ret += PAD + PAD;
                } else if (leftbits == 4) {
                  ret += BASE[(leftchar&0xf) << 2];
                  ret += PAD;
                }
                return ret;
              }
              audio.src = 'data:audio/x-' + name.substr(-3) + ';base64,' + encode64(byteArray);
              finish(audio); // we don't wait for confirmation this worked - but it's worth trying
            };
            audio.src = url;
            // workaround for chrome bug 124926 - we do not always get oncanplaythrough or onerror
            Browser.safeSetTimeout(function() {
              finish(audio); // try to use it even though it is not necessarily ready to play
            }, 10000);
          } else {
            return fail();
          }
        };
        Module['preloadPlugins'].push(audioPlugin);
        // Canvas event setup
        var canvas = Module['canvas'];
        canvas.requestPointerLock = canvas['requestPointerLock'] ||
                                    canvas['mozRequestPointerLock'] ||
                                    canvas['webkitRequestPointerLock'];
        canvas.exitPointerLock = document['exitPointerLock'] ||
                                 document['mozExitPointerLock'] ||
                                 document['webkitExitPointerLock'] ||
                                 function(){}; // no-op if function does not exist
        canvas.exitPointerLock = canvas.exitPointerLock.bind(document);
        function pointerLockChange() {
          Browser.pointerLock = document['pointerLockElement'] === canvas ||
                                document['mozPointerLockElement'] === canvas ||
                                document['webkitPointerLockElement'] === canvas;
        }
        document.addEventListener('pointerlockchange', pointerLockChange, false);
        document.addEventListener('mozpointerlockchange', pointerLockChange, false);
        document.addEventListener('webkitpointerlockchange', pointerLockChange, false);
        if (Module['elementPointerLock']) {
          canvas.addEventListener("click", function(ev) {
            if (!Browser.pointerLock && canvas.requestPointerLock) {
              canvas.requestPointerLock();
              ev.preventDefault();
            }
          }, false);
        }
      },createContext:function (canvas, useWebGL, setInModule) {
        var ctx;
        try {
          if (useWebGL) {
            ctx = canvas.getContext('experimental-webgl', {
              alpha: false
            });
          } else {
            ctx = canvas.getContext('2d');
          }
          if (!ctx) throw ':(';
        } catch (e) {
          Module.print('Could not create canvas - ' + e);
          return null;
        }
        if (useWebGL) {
          // Set the background of the WebGL canvas to black
          canvas.style.backgroundColor = "black";
          // Warn on context loss
          canvas.addEventListener('webglcontextlost', function(event) {
            alert('WebGL context lost. You will need to reload the page.');
          }, false);
        }
        if (setInModule) {
          Module.ctx = ctx;
          Module.useWebGL = useWebGL;
          Browser.moduleContextCreatedCallbacks.forEach(function(callback) { callback() });
          Browser.init();
        }
        return ctx;
      },destroyContext:function (canvas, useWebGL, setInModule) {},fullScreenHandlersInstalled:false,lockPointer:undefined,resizeCanvas:undefined,requestFullScreen:function (lockPointer, resizeCanvas) {
        Browser.lockPointer = lockPointer;
        Browser.resizeCanvas = resizeCanvas;
        if (typeof Browser.lockPointer === 'undefined') Browser.lockPointer = true;
        if (typeof Browser.resizeCanvas === 'undefined') Browser.resizeCanvas = false;
        var canvas = Module['canvas'];
        function fullScreenChange() {
          Browser.isFullScreen = false;
          if ((document['webkitFullScreenElement'] || document['webkitFullscreenElement'] ||
               document['mozFullScreenElement'] || document['mozFullscreenElement'] ||
               document['fullScreenElement'] || document['fullscreenElement']) === canvas) {
            canvas.cancelFullScreen = document['cancelFullScreen'] ||
                                      document['mozCancelFullScreen'] ||
                                      document['webkitCancelFullScreen'];
            canvas.cancelFullScreen = canvas.cancelFullScreen.bind(document);
            if (Browser.lockPointer) canvas.requestPointerLock();
            Browser.isFullScreen = true;
            if (Browser.resizeCanvas) Browser.setFullScreenCanvasSize();
          } else if (Browser.resizeCanvas){
            Browser.setWindowedCanvasSize();
          }
          if (Module['onFullScreen']) Module['onFullScreen'](Browser.isFullScreen);
        }
        if (!Browser.fullScreenHandlersInstalled) {
          Browser.fullScreenHandlersInstalled = true;
          document.addEventListener('fullscreenchange', fullScreenChange, false);
          document.addEventListener('mozfullscreenchange', fullScreenChange, false);
          document.addEventListener('webkitfullscreenchange', fullScreenChange, false);
        }
        canvas.requestFullScreen = canvas['requestFullScreen'] ||
                                   canvas['mozRequestFullScreen'] ||
                                   (canvas['webkitRequestFullScreen'] ? function() { canvas['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) } : null);
        canvas.requestFullScreen();
      },requestAnimationFrame:function (func) {
        if (!window.requestAnimationFrame) {
          window.requestAnimationFrame = window['requestAnimationFrame'] ||
                                         window['mozRequestAnimationFrame'] ||
                                         window['webkitRequestAnimationFrame'] ||
                                         window['msRequestAnimationFrame'] ||
                                         window['oRequestAnimationFrame'] ||
                                         window['setTimeout'];
        }
        window.requestAnimationFrame(func);
      },safeCallback:function (func) {
        return function() {
          if (!ABORT) return func.apply(null, arguments);
        };
      },safeRequestAnimationFrame:function (func) {
        return Browser.requestAnimationFrame(function() {
          if (!ABORT) func();
        });
      },safeSetTimeout:function (func, timeout) {
        return setTimeout(function() {
          if (!ABORT) func();
        }, timeout);
      },safeSetInterval:function (func, timeout) {
        return setInterval(function() {
          if (!ABORT) func();
        }, timeout);
      },getUserMedia:function (func) {
        if(!window.getUserMedia) {
          window.getUserMedia = navigator['getUserMedia'] ||
                                navigator['mozGetUserMedia'];
        }
        window.getUserMedia(func);
      },getMovementX:function (event) {
        return event['movementX'] ||
               event['mozMovementX'] ||
               event['webkitMovementX'] ||
               0;
      },getMovementY:function (event) {
        return event['movementY'] ||
               event['mozMovementY'] ||
               event['webkitMovementY'] ||
               0;
      },mouseX:0,mouseY:0,mouseMovementX:0,mouseMovementY:0,calculateMouseEvent:function (event) { // event should be mousemove, mousedown or mouseup
        if (Browser.pointerLock) {
          // When the pointer is locked, calculate the coordinates
          // based on the movement of the mouse.
          // Workaround for Firefox bug 764498
          if (event.type != 'mousemove' &&
              ('mozMovementX' in event)) {
            Browser.mouseMovementX = Browser.mouseMovementY = 0;
          } else {
            Browser.mouseMovementX = Browser.getMovementX(event);
            Browser.mouseMovementY = Browser.getMovementY(event);
          }
          // check if SDL is available
          if (typeof SDL != "undefined") {
          	Browser.mouseX = SDL.mouseX + Browser.mouseMovementX;
          	Browser.mouseY = SDL.mouseY + Browser.mouseMovementY;
          } else {
          	// just add the mouse delta to the current absolut mouse position
          	// FIXME: ideally this should be clamped against the canvas size and zero
          	Browser.mouseX += Browser.mouseMovementX;
          	Browser.mouseY += Browser.mouseMovementY;
          }        
        } else {
          // Otherwise, calculate the movement based on the changes
          // in the coordinates.
          var rect = Module["canvas"].getBoundingClientRect();
          var x = event.pageX - (window.scrollX + rect.left);
          var y = event.pageY - (window.scrollY + rect.top);
          // the canvas might be CSS-scaled compared to its backbuffer;
          // SDL-using content will want mouse coordinates in terms
          // of backbuffer units.
          var cw = Module["canvas"].width;
          var ch = Module["canvas"].height;
          x = x * (cw / rect.width);
          y = y * (ch / rect.height);
          Browser.mouseMovementX = x - Browser.mouseX;
          Browser.mouseMovementY = y - Browser.mouseY;
          Browser.mouseX = x;
          Browser.mouseY = y;
        }
      },xhrLoad:function (url, onload, onerror) {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, true);
        xhr.responseType = 'arraybuffer';
        xhr.onload = function() {
          if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            onload(xhr.response);
          } else {
            onerror();
          }
        };
        xhr.onerror = onerror;
        xhr.send(null);
      },asyncLoad:function (url, onload, onerror, noRunDep) {
        Browser.xhrLoad(url, function(arrayBuffer) {
          assert(arrayBuffer, 'Loading data file "' + url + '" failed (no arrayBuffer).');
          onload(new Uint8Array(arrayBuffer));
          if (!noRunDep) removeRunDependency('al ' + url);
        }, function(event) {
          if (onerror) {
            onerror();
          } else {
            throw 'Loading data file "' + url + '" failed.';
          }
        });
        if (!noRunDep) addRunDependency('al ' + url);
      },resizeListeners:[],updateResizeListeners:function () {
        var canvas = Module['canvas'];
        Browser.resizeListeners.forEach(function(listener) {
          listener(canvas.width, canvas.height);
        });
      },setCanvasSize:function (width, height, noUpdates) {
        var canvas = Module['canvas'];
        canvas.width = width;
        canvas.height = height;
        if (!noUpdates) Browser.updateResizeListeners();
      },windowedWidth:0,windowedHeight:0,setFullScreenCanvasSize:function () {
        var canvas = Module['canvas'];
        this.windowedWidth = canvas.width;
        this.windowedHeight = canvas.height;
        canvas.width = screen.width;
        canvas.height = screen.height;
        // check if SDL is available   
        if (typeof SDL != "undefined") {
        	var flags = HEAPU32[((SDL.screen+Runtime.QUANTUM_SIZE*0)>>2)];
        	flags = flags | 0x00800000; // set SDL_FULLSCREEN flag
        	HEAP32[((SDL.screen+Runtime.QUANTUM_SIZE*0)>>2)]=flags
        }
        Browser.updateResizeListeners();
      },setWindowedCanvasSize:function () {
        var canvas = Module['canvas'];
        canvas.width = this.windowedWidth;
        canvas.height = this.windowedHeight;
        // check if SDL is available       
        if (typeof SDL != "undefined") {
        	var flags = HEAPU32[((SDL.screen+Runtime.QUANTUM_SIZE*0)>>2)];
        	flags = flags & ~0x00800000; // clear SDL_FULLSCREEN flag
        	HEAP32[((SDL.screen+Runtime.QUANTUM_SIZE*0)>>2)]=flags
        }
        Browser.updateResizeListeners();
      }};
__ATINIT__.unshift({ func: function() { if (!Module["noFSInit"] && !FS.init.initialized) FS.init() } });__ATMAIN__.push({ func: function() { FS.ignorePermissions = false } });__ATEXIT__.push({ func: function() { FS.quit() } });Module["FS_createFolder"] = FS.createFolder;Module["FS_createPath"] = FS.createPath;Module["FS_createDataFile"] = FS.createDataFile;Module["FS_createPreloadedFile"] = FS.createPreloadedFile;Module["FS_createLazyFile"] = FS.createLazyFile;Module["FS_createLink"] = FS.createLink;Module["FS_createDevice"] = FS.createDevice;
___errno_state = Runtime.staticAlloc(4); HEAP32[((___errno_state)>>2)]=0;
Module["requestFullScreen"] = function(lockPointer, resizeCanvas) { Browser.requestFullScreen(lockPointer, resizeCanvas) };
  Module["requestAnimationFrame"] = function(func) { Browser.requestAnimationFrame(func) };
  Module["pauseMainLoop"] = function() { Browser.mainLoop.pause() };
  Module["resumeMainLoop"] = function() { Browser.mainLoop.resume() };
  Module["getUserMedia"] = function() { Browser.getUserMedia() }
STACK_BASE = STACKTOP = Runtime.alignMemory(STATICTOP);
staticSealed = true; // seal the static portion of memory
STACK_MAX = STACK_BASE + 5242880;
DYNAMIC_BASE = DYNAMICTOP = Runtime.alignMemory(STACK_MAX);
assert(DYNAMIC_BASE < TOTAL_MEMORY); // Stack must fit in TOTAL_MEMORY; allocations from here on may enlarge TOTAL_MEMORY
var Math_min = Math.min;
function invoke_ii(index,a1) {
  try {
    return Module["dynCall_ii"](index,a1);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    asm["setThrew"](1, 0);
  }
}
function invoke_vi(index,a1) {
  try {
    Module["dynCall_vi"](index,a1);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    asm["setThrew"](1, 0);
  }
}
function invoke_vii(index,a1,a2) {
  try {
    Module["dynCall_vii"](index,a1,a2);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    asm["setThrew"](1, 0);
  }
}
function invoke_iiii(index,a1,a2,a3) {
  try {
    return Module["dynCall_iiii"](index,a1,a2,a3);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    asm["setThrew"](1, 0);
  }
}
function invoke_v(index) {
  try {
    Module["dynCall_v"](index);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    asm["setThrew"](1, 0);
  }
}
function invoke_iii(index,a1,a2) {
  try {
    return Module["dynCall_iii"](index,a1,a2);
  } catch(e) {
    if (typeof e !== 'number' && e !== 'longjmp') throw e;
    asm["setThrew"](1, 0);
  }
}
function asmPrintInt(x, y) {
  Module.print('int ' + x + ',' + y);// + ' ' + new Error().stack);
}
function asmPrintFloat(x, y) {
  Module.print('float ' + x + ',' + y);// + ' ' + new Error().stack);
}
// EMSCRIPTEN_START_ASM
var asm=(function(global,env,buffer){"use asm";var a=new global.Int8Array(buffer);var b=new global.Int16Array(buffer);var c=new global.Int32Array(buffer);var d=new global.Uint8Array(buffer);var e=new global.Uint16Array(buffer);var f=new global.Uint32Array(buffer);var g=new global.Float32Array(buffer);var h=new global.Float64Array(buffer);var i=env.STACKTOP|0;var j=env.STACK_MAX|0;var k=env.tempDoublePtr|0;var l=env.ABORT|0;var m=+env.NaN;var n=+env.Infinity;var o=0;var p=0;var q=0;var r=0;var s=0,t=0,u=0,v=0,w=0.0,x=0,y=0,z=0,A=0.0;var B=0;var C=0;var D=0;var E=0;var F=0;var G=0;var H=0;var I=0;var J=0;var K=0;var L=global.Math.floor;var M=global.Math.abs;var N=global.Math.sqrt;var O=global.Math.pow;var P=global.Math.cos;var Q=global.Math.sin;var R=global.Math.tan;var S=global.Math.acos;var T=global.Math.asin;var U=global.Math.atan;var V=global.Math.atan2;var W=global.Math.exp;var X=global.Math.log;var Y=global.Math.ceil;var Z=global.Math.imul;var _=env.abort;var $=env.assert;var aa=env.asmPrintInt;var ab=env.asmPrintFloat;var ac=env.min;var ad=env.invoke_ii;var ae=env.invoke_vi;var af=env.invoke_vii;var ag=env.invoke_iiii;var ah=env.invoke_v;var ai=env.invoke_iii;var aj=env._llvm_lifetime_end;var ak=env._lseek;var al=env._sysconf;var am=env._abort;var an=env._close;var ao=env._pread;var ap=env._open;var aq=env.___setErrNo;var ar=env._send;var as=env._write;var at=env._read;var au=env._recv;var av=env._pwrite;var aw=env._sbrk;var ax=env._strerror_r;var ay=env.___errno_location;var az=env._strerror;var aA=env._llvm_lifetime_start;var aB=env._llvm_bswap_i32;var aC=env._time;
// EMSCRIPTEN_START_FUNCS
function aJ(a){a=a|0;var b=0;b=i;i=i+a|0;i=i+7>>3<<3;return b|0}function aK(){return i|0}function aL(a){a=a|0;i=a}function aM(a,b){a=a|0;b=b|0;if((o|0)==0){o=a;p=b}}function aN(b){b=b|0;a[k]=a[b];a[k+1|0]=a[b+1|0];a[k+2|0]=a[b+2|0];a[k+3|0]=a[b+3|0]}function aO(b){b=b|0;a[k]=a[b];a[k+1|0]=a[b+1|0];a[k+2|0]=a[b+2|0];a[k+3|0]=a[b+3|0];a[k+4|0]=a[b+4|0];a[k+5|0]=a[b+5|0];a[k+6|0]=a[b+6|0];a[k+7|0]=a[b+7|0]}function aP(a){a=a|0;B=a}function aQ(a){a=a|0;C=a}function aR(a){a=a|0;D=a}function aS(a){a=a|0;E=a}function aT(a){a=a|0;F=a}function aU(a){a=a|0;G=a}function aV(a){a=a|0;H=a}function aW(a){a=a|0;I=a}function aX(a){a=a|0;J=a}function aY(a){a=a|0;K=a}function aZ(){}function a_(a,b,c){a=a|0;b=b|0;c=c|0;var e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0;e=a>>>16;f=a&65535;if((c|0)==1){a=(d[b]|0)+f|0;g=a>>>0>65520?a-65521|0:a;a=g+e|0;h=(a>>>0>65520?a+15|0:a)<<16|g;return h|0}if((b|0)==0){h=1;return h|0}if(c>>>0<16){if((c|0)==0){i=f;j=e}else{g=f;a=b;k=c;l=e;while(1){m=k-1|0;n=(d[a]|0)+g|0;o=n+l|0;if((m|0)==0){i=n;j=o;break}else{g=n;a=a+1|0;k=m;l=o}}}h=((j>>>0)%65521|0)<<16|(i>>>0>65520?i-65521|0:i);return h|0}do{if(c>>>0>5551){i=f;j=b;l=c;k=e;do{l=l-5552|0;a=347;g=k;o=j;m=i;while(1){n=(d[o]|0)+m|0;p=n+(d[o+1|0]|0)|0;q=p+(d[o+2|0]|0)|0;r=q+(d[o+3|0]|0)|0;s=r+(d[o+4|0]|0)|0;t=s+(d[o+5|0]|0)|0;u=t+(d[o+6|0]|0)|0;v=u+(d[o+7|0]|0)|0;w=v+(d[o+8|0]|0)|0;x=w+(d[o+9|0]|0)|0;y=x+(d[o+10|0]|0)|0;z=y+(d[o+11|0]|0)|0;A=z+(d[o+12|0]|0)|0;B=A+(d[o+13|0]|0)|0;C=B+(d[o+14|0]|0)|0;D=C+(d[o+15|0]|0)|0;E=n+g+p+q+r+s+t+u+v+w+x+y+z+A+B+C+D|0;C=a-1|0;if((C|0)==0){break}else{a=C;g=E;o=o+16|0;m=D}}j=j+5552|0;i=(D>>>0)%65521|0;k=(E>>>0)%65521|0;}while(l>>>0>5551);if((l|0)==0){F=k;G=i;break}if(l>>>0>15){H=i;I=j;J=l;K=k;L=15}else{M=i;N=j;O=l;P=k;L=16}}else{H=f;I=b;J=c;K=e;L=15}}while(0);if((L|0)==15){while(1){L=0;Q=J-16|0;e=(d[I]|0)+H|0;c=e+(d[I+1|0]|0)|0;b=c+(d[I+2|0]|0)|0;f=b+(d[I+3|0]|0)|0;E=f+(d[I+4|0]|0)|0;D=E+(d[I+5|0]|0)|0;m=D+(d[I+6|0]|0)|0;o=m+(d[I+7|0]|0)|0;g=o+(d[I+8|0]|0)|0;a=g+(d[I+9|0]|0)|0;C=a+(d[I+10|0]|0)|0;B=C+(d[I+11|0]|0)|0;A=B+(d[I+12|0]|0)|0;z=A+(d[I+13|0]|0)|0;y=z+(d[I+14|0]|0)|0;R=y+(d[I+15|0]|0)|0;S=e+K+c+b+f+E+D+m+o+g+a+C+B+A+z+y+R|0;T=I+16|0;if(Q>>>0>15){H=R;I=T;J=Q;K=S;L=15}else{break}}if((Q|0)==0){U=R;V=S;L=17}else{M=R;N=T;O=Q;P=S;L=16}}if((L|0)==16){while(1){L=0;S=O-1|0;Q=(d[N]|0)+M|0;T=Q+P|0;if((S|0)==0){U=Q;V=T;L=17;break}else{M=Q;N=N+1|0;O=S;P=T;L=16}}}if((L|0)==17){F=(V>>>0)%65521|0;G=(U>>>0)%65521|0}h=F<<16|G;return h|0}function a$(a,b,e){a=a|0;b=b|0;e=e|0;var f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0;if((b|0)==0){f=0;return f|0}g=~a;L39:do{if((e|0)==0){h=g}else{a=b;i=e;j=g;while(1){if((a&3|0)==0){break}k=c[4200+(((d[a]|0)^j&255)<<2)>>2]^j>>>8;l=i-1|0;if((l|0)==0){h=k;break L39}else{a=a+1|0;i=l;j=k}}k=a;if(i>>>0>31){l=i;m=j;n=k;while(1){o=c[n>>2]^m;p=c[6248+((o>>>8&255)<<2)>>2]^c[7272+((o&255)<<2)>>2]^c[5224+((o>>>16&255)<<2)>>2]^c[4200+(o>>>24<<2)>>2]^c[n+4>>2];o=c[6248+((p>>>8&255)<<2)>>2]^c[7272+((p&255)<<2)>>2]^c[5224+((p>>>16&255)<<2)>>2]^c[4200+(p>>>24<<2)>>2]^c[n+8>>2];p=c[6248+((o>>>8&255)<<2)>>2]^c[7272+((o&255)<<2)>>2]^c[5224+((o>>>16&255)<<2)>>2]^c[4200+(o>>>24<<2)>>2]^c[n+12>>2];o=c[6248+((p>>>8&255)<<2)>>2]^c[7272+((p&255)<<2)>>2]^c[5224+((p>>>16&255)<<2)>>2]^c[4200+(p>>>24<<2)>>2]^c[n+16>>2];p=c[6248+((o>>>8&255)<<2)>>2]^c[7272+((o&255)<<2)>>2]^c[5224+((o>>>16&255)<<2)>>2]^c[4200+(o>>>24<<2)>>2]^c[n+20>>2];o=c[6248+((p>>>8&255)<<2)>>2]^c[7272+((p&255)<<2)>>2]^c[5224+((p>>>16&255)<<2)>>2]^c[4200+(p>>>24<<2)>>2]^c[n+24>>2];p=n+32|0;q=c[6248+((o>>>8&255)<<2)>>2]^c[7272+((o&255)<<2)>>2]^c[5224+((o>>>16&255)<<2)>>2]^c[4200+(o>>>24<<2)>>2]^c[n+28>>2];o=c[6248+((q>>>8&255)<<2)>>2]^c[7272+((q&255)<<2)>>2]^c[5224+((q>>>16&255)<<2)>>2]^c[4200+(q>>>24<<2)>>2];q=l-32|0;if(q>>>0>31){l=q;m=o;n=p}else{r=q;s=o;t=p;break}}}else{r=i;s=j;t=k}if(r>>>0>3){n=r;m=s;l=t;while(1){a=l+4|0;p=c[l>>2]^m;o=c[6248+((p>>>8&255)<<2)>>2]^c[7272+((p&255)<<2)>>2]^c[5224+((p>>>16&255)<<2)>>2]^c[4200+(p>>>24<<2)>>2];p=n-4|0;if(p>>>0>3){n=p;m=o;l=a}else{u=p;v=o;w=a;break}}}else{u=r;v=s;w=t}if((u|0)==0){h=v;break}l=v;m=u;n=w;while(1){k=c[4200+(((d[n]|0)^l&255)<<2)>>2]^l>>>8;j=m-1|0;if((j|0)==0){h=k;break}else{l=k;m=j;n=n+1|0}}}}while(0);f=~h;return f|0}function a0(b,d,e,f,g,h,i,j){b=b|0;d=d|0;e=e|0;f=f|0;g=g|0;h=h|0;i=i|0;j=j|0;var k=0,l=0,m=0,n=0,o=0,p=0;if((i|0)==0){k=-6;return k|0}if(!((a[i]|0)==49&(j|0)==56)){k=-6;return k|0}if((b|0)==0){k=-2;return k|0}j=b+24|0;c[j>>2]=0;i=b+32|0;l=c[i>>2]|0;if((l|0)==0){c[i>>2]=6;c[b+40>>2]=0;m=6}else{m=l}l=b+36|0;if((c[l>>2]|0)==0){c[l>>2]=2}l=(d|0)==-1?6:d;if((f|0)<0){n=0;o=-f|0}else{d=(f|0)>15;n=d?2:1;o=d?f-16|0:f}if(!((g-1|0)>>>0<9&(e|0)==8)){k=-2;return k|0}if((o-8|0)>>>0>7|l>>>0>9|h>>>0>4){k=-2;return k|0}e=(o|0)==8?9:o;o=b+40|0;f=aG[m&7](c[o>>2]|0,1,5828)|0;if((f|0)==0){k=-4;return k|0}c[b+28>>2]=f;c[f>>2]=b;c[f+24>>2]=n;c[f+28>>2]=0;c[f+48>>2]=e;n=1<<e;e=f+44|0;c[e>>2]=n;c[f+52>>2]=n-1;m=g+7|0;c[f+80>>2]=m;d=1<<m;m=f+76|0;c[m>>2]=d;c[f+84>>2]=d-1;c[f+88>>2]=((g+9|0)>>>0)/3|0;d=f+56|0;c[d>>2]=aG[c[i>>2]&7](c[o>>2]|0,n,2)|0;n=aG[c[i>>2]&7](c[o>>2]|0,c[e>>2]|0,2)|0;p=f+64|0;c[p>>2]=n;bI(n|0,0,c[e>>2]<<1|0);e=f+68|0;c[e>>2]=aG[c[i>>2]&7](c[o>>2]|0,c[m>>2]|0,2)|0;c[f+5824>>2]=0;m=1<<g+6;g=f+5788|0;c[g>>2]=m;n=aG[c[i>>2]&7](c[o>>2]|0,m,4)|0;m=n;c[f+8>>2]=n;o=c[g>>2]|0;c[f+12>>2]=o<<2;do{if((c[d>>2]|0)!=0){if((c[p>>2]|0)==0){break}if((c[e>>2]|0)==0|(n|0)==0){break}c[f+5796>>2]=m+(o>>>1<<1);c[f+5784>>2]=n+(o*3|0);c[f+132>>2]=l;c[f+136>>2]=h;a[f+36|0]=8;k=a2(b)|0;return k|0}}while(0);c[f+4>>2]=666;c[j>>2]=c[8];a1(b)|0;k=-4;return k|0}function a1(a){a=a|0;var b=0,d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0;if((a|0)==0){b=-2;return b|0}d=a+28|0;e=c[d>>2]|0;if((e|0)==0){b=-2;return b|0}f=c[e+4>>2]|0;if(!((f|0)==666|(f|0)==113|(f|0)==103|(f|0)==91|(f|0)==73|(f|0)==69|(f|0)==42)){b=-2;return b|0}g=c[e+8>>2]|0;if((g|0)==0){h=e}else{aF[c[a+36>>2]&3](c[a+40>>2]|0,g);h=c[d>>2]|0}g=c[h+68>>2]|0;if((g|0)==0){i=h}else{aF[c[a+36>>2]&3](c[a+40>>2]|0,g);i=c[d>>2]|0}g=c[i+64>>2]|0;if((g|0)==0){j=i}else{aF[c[a+36>>2]&3](c[a+40>>2]|0,g);j=c[d>>2]|0}g=c[j+56>>2]|0;i=a+36|0;if((g|0)==0){k=j;l=a+40|0}else{j=a+40|0;aF[c[i>>2]&3](c[j>>2]|0,g);k=c[d>>2]|0;l=j}aF[c[i>>2]&3](c[l>>2]|0,k);c[d>>2]=0;b=(f|0)==113?-3:0;return b|0}function a2(a){a=a|0;var d=0,f=0,g=0,h=0,i=0,j=0,k=0;if((a|0)==0){d=-2;return d|0}f=c[a+28>>2]|0;if((f|0)==0){d=-2;return d|0}if((c[a+32>>2]|0)==0){d=-2;return d|0}if((c[a+36>>2]|0)==0){d=-2;return d|0}c[a+20>>2]=0;c[a+8>>2]=0;c[a+24>>2]=0;c[a+44>>2]=2;c[f+20>>2]=0;c[f+16>>2]=c[f+8>>2];g=f+24|0;h=c[g>>2]|0;if((h|0)<0){i=-h|0;c[g>>2]=i;j=i}else{j=h}c[f+4>>2]=(j|0)!=0?42:113;if((j|0)==2){k=a$(0,0,0)|0}else{k=a_(0,0,0)|0}c[a+48>>2]=k;c[f+40>>2]=0;bw(f);c[f+60>>2]=c[f+44>>2]<<1;k=f+76|0;a=f+68|0;b[(c[a>>2]|0)+((c[k>>2]|0)-1<<1)>>1]=0;bI(c[a>>2]|0,0,(c[k>>2]<<1)-2|0);k=c[f+132>>2]|0;c[f+128>>2]=e[12394+(k*12|0)>>1]|0;c[f+140>>2]=e[12392+(k*12|0)>>1]|0;c[f+144>>2]=e[12396+(k*12|0)>>1]|0;c[f+124>>2]=e[12398+(k*12|0)>>1]|0;c[f+108>>2]=0;c[f+92>>2]=0;c[f+116>>2]=0;c[f+120>>2]=2;c[f+96>>2]=2;c[f+112>>2]=0;c[f+104>>2]=0;c[f+72>>2]=0;d=0;return d|0}function a3(e,f){e=e|0;f=f|0;var g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0,X=0,Y=0,Z=0,_=0,$=0,aa=0,ab=0,ac=0,ad=0,ae=0,af=0,ag=0,ah=0,ai=0,aj=0,ak=0,al=0,am=0,an=0,ao=0,ap=0,aq=0,ar=0,as=0,at=0,au=0,av=0,aw=0,ax=0,ay=0,az=0,aA=0,aB=0;if((e|0)==0){g=-2;return g|0}h=e+28|0;i=c[h>>2]|0;if((i|0)==0|f>>>0>5){g=-2;return g|0}j=e+12|0;do{if((c[j>>2]|0)!=0){if((c[e>>2]|0)==0){if((c[e+4>>2]|0)!=0){break}}k=i+4|0;l=c[k>>2]|0;m=(f|0)==4;if(!((l|0)!=666|m)){break}n=e+16|0;if((c[n>>2]|0)==0){c[e+24>>2]=c[9];g=-5;return g|0}o=i|0;c[o>>2]=e;p=i+40|0;q=c[p>>2]|0;c[p>>2]=f;do{if((l|0)==42){if((c[i+24>>2]|0)!=2){r=(c[i+48>>2]<<12)-30720|0;do{if((c[i+136>>2]|0)>1){s=0}else{t=c[i+132>>2]|0;if((t|0)<2){s=0;break}if((t|0)<6){s=64;break}s=(t|0)==6?128:192}}while(0);t=s|r;u=i+108|0;v=(c[u>>2]|0)==0?t:t|32;t=(v|((v>>>0)%31|0))^31;c[k>>2]=113;v=i+20|0;w=c[v>>2]|0;c[v>>2]=w+1;x=i+8|0;a[(c[x>>2]|0)+w|0]=t>>>8&255;w=c[v>>2]|0;c[v>>2]=w+1;a[(c[x>>2]|0)+w|0]=t&255;t=e+48|0;if((c[u>>2]|0)!=0){u=c[t>>2]|0;w=c[v>>2]|0;c[v>>2]=w+1;a[(c[x>>2]|0)+w|0]=u>>>24&255;w=c[v>>2]|0;c[v>>2]=w+1;a[(c[x>>2]|0)+w|0]=u>>>16&255;u=c[t>>2]|0;w=c[v>>2]|0;c[v>>2]=w+1;a[(c[x>>2]|0)+w|0]=u>>>8&255;w=c[v>>2]|0;c[v>>2]=w+1;a[(c[x>>2]|0)+w|0]=u&255}c[t>>2]=a_(0,0,0)|0;y=c[k>>2]|0;z=131;break}t=e+48|0;c[t>>2]=a$(0,0,0)|0;u=i+20|0;w=c[u>>2]|0;c[u>>2]=w+1;x=i+8|0;a[(c[x>>2]|0)+w|0]=31;w=c[u>>2]|0;c[u>>2]=w+1;a[(c[x>>2]|0)+w|0]=-117;w=c[u>>2]|0;c[u>>2]=w+1;a[(c[x>>2]|0)+w|0]=8;w=i+28|0;v=c[w>>2]|0;if((v|0)==0){A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=0;A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=0;A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=0;A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=0;A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=0;A=c[i+132>>2]|0;do{if((A|0)==9){B=2}else{if((c[i+136>>2]|0)>1){B=4;break}B=(A|0)<2?4:0}}while(0);A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=B;A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=3;c[k>>2]=113;break}A=((c[v+44>>2]|0)!=0?2:0)|(c[v>>2]|0)!=0|((c[v+16>>2]|0)==0?0:4)|((c[v+28>>2]|0)==0?0:8)|((c[v+36>>2]|0)==0?0:16);r=c[u>>2]|0;c[u>>2]=r+1;a[(c[x>>2]|0)+r|0]=A;A=c[(c[w>>2]|0)+4>>2]&255;r=c[u>>2]|0;c[u>>2]=r+1;a[(c[x>>2]|0)+r|0]=A;A=(c[(c[w>>2]|0)+4>>2]|0)>>>8&255;r=c[u>>2]|0;c[u>>2]=r+1;a[(c[x>>2]|0)+r|0]=A;A=(c[(c[w>>2]|0)+4>>2]|0)>>>16&255;r=c[u>>2]|0;c[u>>2]=r+1;a[(c[x>>2]|0)+r|0]=A;A=(c[(c[w>>2]|0)+4>>2]|0)>>>24&255;r=c[u>>2]|0;c[u>>2]=r+1;a[(c[x>>2]|0)+r|0]=A;A=c[i+132>>2]|0;do{if((A|0)==9){C=2}else{if((c[i+136>>2]|0)>1){C=4;break}C=(A|0)<2?4:0}}while(0);A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=C;A=c[(c[w>>2]|0)+12>>2]&255;v=c[u>>2]|0;c[u>>2]=v+1;a[(c[x>>2]|0)+v|0]=A;A=c[w>>2]|0;if((c[A+16>>2]|0)==0){D=A}else{v=c[A+20>>2]&255;A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=v;v=(c[(c[w>>2]|0)+20>>2]|0)>>>8&255;A=c[u>>2]|0;c[u>>2]=A+1;a[(c[x>>2]|0)+A|0]=v;D=c[w>>2]|0}if((c[D+44>>2]|0)!=0){c[t>>2]=a$(c[t>>2]|0,c[x>>2]|0,c[u>>2]|0)|0}c[i+32>>2]=0;c[k>>2]=69;E=w;z=133}else{y=l;z=131}}while(0);do{if((z|0)==131){if((y|0)!=69){F=y;z=154;break}E=i+28|0;z=133}}while(0);do{if((z|0)==133){l=c[E>>2]|0;if((c[l+16>>2]|0)==0){c[k>>2]=73;G=l;z=156;break}v=i+20|0;A=c[v>>2]|0;r=i+32|0;H=c[r>>2]|0;L193:do{if(H>>>0<(c[l+20>>2]&65535)>>>0){I=i+12|0;J=e+48|0;K=i+8|0;L=e+20|0;M=A;N=l;O=A;P=H;while(1){if((O|0)==(c[I>>2]|0)){if((c[N+44>>2]|0)!=0&O>>>0>M>>>0){c[J>>2]=a$(c[J>>2]|0,(c[K>>2]|0)+M|0,O-M|0)|0}Q=c[h>>2]|0;R=c[Q+20>>2]|0;S=c[n>>2]|0;T=R>>>0>S>>>0?S:R;do{if((T|0)!=0){R=c[j>>2]|0;S=c[Q+16>>2]|0;bJ(R|0,S|0,T)|0;c[j>>2]=(c[j>>2]|0)+T;S=(c[h>>2]|0)+16|0;c[S>>2]=(c[S>>2]|0)+T;c[L>>2]=(c[L>>2]|0)+T;c[n>>2]=(c[n>>2]|0)-T;S=(c[h>>2]|0)+20|0;c[S>>2]=(c[S>>2]|0)-T;S=c[h>>2]|0;if((c[S+20>>2]|0)!=0){break}c[S+16>>2]=c[S+8>>2]}}while(0);U=c[v>>2]|0;if((U|0)==(c[I>>2]|0)){break}V=U;W=U;X=c[r>>2]|0;Y=c[E>>2]|0}else{V=M;W=O;X=P;Y=N}T=a[(c[Y+16>>2]|0)+X|0]|0;c[v>>2]=W+1;a[(c[K>>2]|0)+W|0]=T;T=(c[r>>2]|0)+1|0;c[r>>2]=T;Q=c[E>>2]|0;if(T>>>0>=(c[Q+20>>2]&65535)>>>0){Z=V;_=Q;break L193}M=V;N=Q;O=c[v>>2]|0;P=T}Z=U;_=c[E>>2]|0}else{Z=A;_=l}}while(0);do{if((c[_+44>>2]|0)==0){$=_}else{l=c[v>>2]|0;if(l>>>0<=Z>>>0){$=_;break}A=e+48|0;c[A>>2]=a$(c[A>>2]|0,(c[i+8>>2]|0)+Z|0,l-Z|0)|0;$=c[E>>2]|0}}while(0);if((c[r>>2]|0)==(c[$+20>>2]|0)){c[r>>2]=0;c[k>>2]=73;G=$;z=156;break}else{F=c[k>>2]|0;z=154;break}}}while(0);do{if((z|0)==154){if((F|0)!=73){aa=F;z=175;break}G=c[i+28>>2]|0;z=156}}while(0);do{if((z|0)==156){v=i+28|0;if((c[G+28>>2]|0)==0){c[k>>2]=91;ab=v;z=177;break}l=i+20|0;A=c[l>>2]|0;H=i+12|0;w=e+48|0;u=i+8|0;x=e+20|0;t=i+32|0;P=A;O=A;while(1){if((O|0)==(c[H>>2]|0)){if((c[(c[v>>2]|0)+44>>2]|0)!=0&O>>>0>P>>>0){c[w>>2]=a$(c[w>>2]|0,(c[u>>2]|0)+P|0,O-P|0)|0}A=c[h>>2]|0;N=c[A+20>>2]|0;M=c[n>>2]|0;K=N>>>0>M>>>0?M:N;do{if((K|0)!=0){N=c[j>>2]|0;M=c[A+16>>2]|0;bJ(N|0,M|0,K)|0;c[j>>2]=(c[j>>2]|0)+K;M=(c[h>>2]|0)+16|0;c[M>>2]=(c[M>>2]|0)+K;c[x>>2]=(c[x>>2]|0)+K;c[n>>2]=(c[n>>2]|0)-K;M=(c[h>>2]|0)+20|0;c[M>>2]=(c[M>>2]|0)-K;M=c[h>>2]|0;if((c[M+20>>2]|0)!=0){break}c[M+16>>2]=c[M+8>>2]}}while(0);K=c[l>>2]|0;if((K|0)==(c[H>>2]|0)){ac=1;ad=K;break}else{ae=K;af=K}}else{ae=P;af=O}K=c[t>>2]|0;c[t>>2]=K+1;ag=a[(c[(c[v>>2]|0)+28>>2]|0)+K|0]|0;c[l>>2]=af+1;a[(c[u>>2]|0)+af|0]=ag;if(ag<<24>>24==0){z=167;break}P=ae;O=c[l>>2]|0}if((z|0)==167){ac=ag&255;ad=ae}do{if((c[(c[v>>2]|0)+44>>2]|0)!=0){O=c[l>>2]|0;if(O>>>0<=ad>>>0){break}c[w>>2]=a$(c[w>>2]|0,(c[u>>2]|0)+ad|0,O-ad|0)|0}}while(0);if((ac|0)==0){c[t>>2]=0;c[k>>2]=91;ab=v;z=177;break}else{aa=c[k>>2]|0;z=175;break}}}while(0);do{if((z|0)==175){if((aa|0)!=91){ah=aa;z=196;break}ab=i+28|0;z=177}}while(0);do{if((z|0)==177){if((c[(c[ab>>2]|0)+36>>2]|0)==0){c[k>>2]=103;ai=ab;z=198;break}u=i+20|0;w=c[u>>2]|0;l=i+12|0;O=e+48|0;P=i+8|0;H=e+20|0;x=i+32|0;r=w;K=w;while(1){if((K|0)==(c[l>>2]|0)){if((c[(c[ab>>2]|0)+44>>2]|0)!=0&K>>>0>r>>>0){c[O>>2]=a$(c[O>>2]|0,(c[P>>2]|0)+r|0,K-r|0)|0}w=c[h>>2]|0;A=c[w+20>>2]|0;M=c[n>>2]|0;N=A>>>0>M>>>0?M:A;do{if((N|0)!=0){A=c[j>>2]|0;M=c[w+16>>2]|0;bJ(A|0,M|0,N)|0;c[j>>2]=(c[j>>2]|0)+N;M=(c[h>>2]|0)+16|0;c[M>>2]=(c[M>>2]|0)+N;c[H>>2]=(c[H>>2]|0)+N;c[n>>2]=(c[n>>2]|0)-N;M=(c[h>>2]|0)+20|0;c[M>>2]=(c[M>>2]|0)-N;M=c[h>>2]|0;if((c[M+20>>2]|0)!=0){break}c[M+16>>2]=c[M+8>>2]}}while(0);N=c[u>>2]|0;if((N|0)==(c[l>>2]|0)){aj=1;ak=N;break}else{al=N;am=N}}else{al=r;am=K}N=c[x>>2]|0;c[x>>2]=N+1;an=a[(c[(c[ab>>2]|0)+36>>2]|0)+N|0]|0;c[u>>2]=am+1;a[(c[P>>2]|0)+am|0]=an;if(an<<24>>24==0){z=188;break}r=al;K=c[u>>2]|0}if((z|0)==188){aj=an&255;ak=al}do{if((c[(c[ab>>2]|0)+44>>2]|0)!=0){K=c[u>>2]|0;if(K>>>0<=ak>>>0){break}c[O>>2]=a$(c[O>>2]|0,(c[P>>2]|0)+ak|0,K-ak|0)|0}}while(0);if((aj|0)==0){c[k>>2]=103;ai=ab;z=198;break}else{ah=c[k>>2]|0;z=196;break}}}while(0);do{if((z|0)==196){if((ah|0)!=103){break}ai=i+28|0;z=198}}while(0);do{if((z|0)==198){if((c[(c[ai>>2]|0)+44>>2]|0)==0){c[k>>2]=113;break}P=i+20|0;O=i+12|0;do{if(((c[P>>2]|0)+2|0)>>>0>(c[O>>2]|0)>>>0){u=c[h>>2]|0;K=c[u+20>>2]|0;r=c[n>>2]|0;x=K>>>0>r>>>0?r:K;if((x|0)==0){break}K=c[j>>2]|0;r=c[u+16>>2]|0;bJ(K|0,r|0,x)|0;c[j>>2]=(c[j>>2]|0)+x;r=(c[h>>2]|0)+16|0;c[r>>2]=(c[r>>2]|0)+x;r=e+20|0;c[r>>2]=(c[r>>2]|0)+x;c[n>>2]=(c[n>>2]|0)-x;r=(c[h>>2]|0)+20|0;c[r>>2]=(c[r>>2]|0)-x;x=c[h>>2]|0;if((c[x+20>>2]|0)!=0){break}c[x+16>>2]=c[x+8>>2]}}while(0);x=c[P>>2]|0;if((x+2|0)>>>0>(c[O>>2]|0)>>>0){break}r=e+48|0;K=c[r>>2]&255;c[P>>2]=x+1;u=i+8|0;a[(c[u>>2]|0)+x|0]=K;K=(c[r>>2]|0)>>>8&255;x=c[P>>2]|0;c[P>>2]=x+1;a[(c[u>>2]|0)+x|0]=K;c[r>>2]=a$(0,0,0)|0;c[k>>2]=113}}while(0);r=i+20|0;do{if((c[r>>2]|0)==0){if((c[e+4>>2]|0)!=0){break}if(!((q|0)>=(f|0)&(f|0)!=4)){break}c[e+24>>2]=c[9];g=-5;return g|0}else{K=c[h>>2]|0;x=c[K+20>>2]|0;u=c[n>>2]|0;l=x>>>0>u>>>0?u:x;if((l|0)==0){ao=u}else{u=c[j>>2]|0;x=c[K+16>>2]|0;bJ(u|0,x|0,l)|0;c[j>>2]=(c[j>>2]|0)+l;x=(c[h>>2]|0)+16|0;c[x>>2]=(c[x>>2]|0)+l;x=e+20|0;c[x>>2]=(c[x>>2]|0)+l;c[n>>2]=(c[n>>2]|0)-l;x=(c[h>>2]|0)+20|0;c[x>>2]=(c[x>>2]|0)-l;l=c[h>>2]|0;if((c[l+20>>2]|0)==0){c[l+16>>2]=c[l+8>>2]}ao=c[n>>2]|0}if((ao|0)!=0){break}c[p>>2]=-1;g=0;return g|0}}while(0);q=(c[k>>2]|0)==666;l=(c[e+4>>2]|0)==0;do{if(q){if(l){z=220;break}c[e+24>>2]=c[9];g=-5;return g|0}else{if(l){z=220}else{z=223}}}while(0);do{if((z|0)==220){if((c[i+116>>2]|0)!=0){z=223;break}if((f|0)==0){g=0;return g|0}else{if(q){break}else{z=223;break}}}}while(0);L321:do{if((z|0)==223){q=c[i+136>>2]|0;L323:do{if((q|0)==2){l=i+116|0;x=i+96|0;u=i+108|0;K=i+56|0;H=i+5792|0;v=i+5796|0;t=i+5784|0;N=i+5788|0;w=i+92|0;while(1){if((c[l>>2]|0)==0){a4(i);if((c[l>>2]|0)==0){break}}c[x>>2]=0;M=a[(c[K>>2]|0)+(c[u>>2]|0)|0]|0;b[(c[v>>2]|0)+(c[H>>2]<<1)>>1]=0;A=c[H>>2]|0;c[H>>2]=A+1;a[(c[t>>2]|0)+A|0]=M;A=i+148+((M&255)<<2)|0;b[A>>1]=(b[A>>1]|0)+1&65535;A=(c[H>>2]|0)==((c[N>>2]|0)-1|0);c[l>>2]=(c[l>>2]|0)-1;M=(c[u>>2]|0)+1|0;c[u>>2]=M;if(!A){continue}A=c[w>>2]|0;if((A|0)>-1){ap=(c[K>>2]|0)+A|0}else{ap=0}by(i,ap,M-A|0,0);c[w>>2]=c[u>>2];A=c[o>>2]|0;M=A+28|0;I=c[M>>2]|0;L=c[I+20>>2]|0;J=A+16|0;T=c[J>>2]|0;Q=L>>>0>T>>>0?T:L;do{if((Q|0)!=0){L=A+12|0;T=c[L>>2]|0;S=c[I+16>>2]|0;bJ(T|0,S|0,Q)|0;c[L>>2]=(c[L>>2]|0)+Q;L=(c[M>>2]|0)+16|0;c[L>>2]=(c[L>>2]|0)+Q;L=A+20|0;c[L>>2]=(c[L>>2]|0)+Q;c[J>>2]=(c[J>>2]|0)-Q;L=(c[M>>2]|0)+20|0;c[L>>2]=(c[L>>2]|0)-Q;L=c[M>>2]|0;if((c[L+20>>2]|0)!=0){break}c[L+16>>2]=c[L+8>>2]}}while(0);if((c[(c[o>>2]|0)+16>>2]|0)==0){break L323}}if((f|0)==0){break}l=c[w>>2]|0;if((l|0)>-1){aq=(c[K>>2]|0)+l|0}else{aq=0}by(i,aq,(c[u>>2]|0)-l|0,m&1);c[w>>2]=c[u>>2];l=c[o>>2]|0;N=l+28|0;H=c[N>>2]|0;t=c[H+20>>2]|0;v=l+16|0;x=c[v>>2]|0;M=t>>>0>x>>>0?x:t;do{if((M|0)!=0){t=l+12|0;x=c[t>>2]|0;Q=c[H+16>>2]|0;bJ(x|0,Q|0,M)|0;c[t>>2]=(c[t>>2]|0)+M;t=(c[N>>2]|0)+16|0;c[t>>2]=(c[t>>2]|0)+M;t=l+20|0;c[t>>2]=(c[t>>2]|0)+M;c[v>>2]=(c[v>>2]|0)-M;t=(c[N>>2]|0)+20|0;c[t>>2]=(c[t>>2]|0)-M;t=c[N>>2]|0;if((c[t+20>>2]|0)!=0){break}c[t+16>>2]=c[t+8>>2]}}while(0);if((c[(c[o>>2]|0)+16>>2]|0)==0){ar=m?2:0;z=281;break}else{ar=m?3:1;z=281;break}}else if((q|0)==3){N=i+116|0;M=(f|0)==0;v=i+96|0;l=i+108|0;H=i+5792|0;u=i+5796|0;w=i+5784|0;K=i+2440+(d[14184]<<2)|0;t=i+5788|0;Q=i+56|0;x=i+92|0;L351:while(1){J=c[N>>2]|0;do{if(J>>>0<258){a4(i);A=c[N>>2]|0;if(A>>>0<258&M){break L323}if((A|0)==0){break L351}c[v>>2]=0;if(A>>>0>2){as=A;z=250;break}at=c[l>>2]|0;z=265}else{c[v>>2]=0;as=J;z=250}}while(0);do{if((z|0)==250){z=0;J=c[l>>2]|0;if((J|0)==0){at=0;z=265;break}A=c[Q>>2]|0;I=a[A+(J-1)|0]|0;if(I<<24>>24!=(a[A+J|0]|0)){at=J;z=265;break}if(I<<24>>24!=(a[A+(J+1)|0]|0)){at=J;z=265;break}L=A+(J+2)|0;if(I<<24>>24!=(a[L]|0)){at=J;z=265;break}S=A+(J+258)|0;A=L;while(1){L=A+1|0;if(I<<24>>24!=(a[L]|0)){au=L;break}L=A+2|0;if(I<<24>>24!=(a[L]|0)){au=L;break}L=A+3|0;if(I<<24>>24!=(a[L]|0)){au=L;break}L=A+4|0;if(I<<24>>24!=(a[L]|0)){au=L;break}L=A+5|0;if(I<<24>>24!=(a[L]|0)){au=L;break}L=A+6|0;if(I<<24>>24!=(a[L]|0)){au=L;break}L=A+7|0;if(I<<24>>24!=(a[L]|0)){au=L;break}L=A+8|0;if(I<<24>>24==(a[L]|0)&L>>>0<S>>>0){A=L}else{au=L;break}}A=au-S+258|0;I=A>>>0>as>>>0?as:A;c[v>>2]=I;if(I>>>0<=2){at=J;z=265;break}A=I+253|0;b[(c[u>>2]|0)+(c[H>>2]<<1)>>1]=1;I=c[H>>2]|0;c[H>>2]=I+1;a[(c[w>>2]|0)+I|0]=A&255;I=i+148+((d[13928+(A&255)|0]|256)+1<<2)|0;b[I>>1]=(b[I>>1]|0)+1&65535;b[K>>1]=(b[K>>1]|0)+1&65535;I=(c[H>>2]|0)==((c[t>>2]|0)-1|0);A=c[v>>2]|0;c[N>>2]=(c[N>>2]|0)-A;L=(c[l>>2]|0)+A|0;c[l>>2]=L;c[v>>2]=0;if(I){av=L}else{continue L351}}}while(0);if((z|0)==265){z=0;L=a[(c[Q>>2]|0)+at|0]|0;b[(c[u>>2]|0)+(c[H>>2]<<1)>>1]=0;I=c[H>>2]|0;c[H>>2]=I+1;a[(c[w>>2]|0)+I|0]=L;I=i+148+((L&255)<<2)|0;b[I>>1]=(b[I>>1]|0)+1&65535;I=(c[H>>2]|0)==((c[t>>2]|0)-1|0);c[N>>2]=(c[N>>2]|0)-1;L=(c[l>>2]|0)+1|0;c[l>>2]=L;if(I){av=L}else{continue}}L=c[x>>2]|0;if((L|0)>-1){aw=(c[Q>>2]|0)+L|0}else{aw=0}by(i,aw,av-L|0,0);c[x>>2]=c[l>>2];L=c[o>>2]|0;I=L+28|0;A=c[I>>2]|0;T=c[A+20>>2]|0;R=L+16|0;ax=c[R>>2]|0;ay=T>>>0>ax>>>0?ax:T;do{if((ay|0)!=0){T=L+12|0;ax=c[T>>2]|0;az=c[A+16>>2]|0;bJ(ax|0,az|0,ay)|0;c[T>>2]=(c[T>>2]|0)+ay;T=(c[I>>2]|0)+16|0;c[T>>2]=(c[T>>2]|0)+ay;T=L+20|0;c[T>>2]=(c[T>>2]|0)+ay;c[R>>2]=(c[R>>2]|0)-ay;T=(c[I>>2]|0)+20|0;c[T>>2]=(c[T>>2]|0)-ay;T=c[I>>2]|0;if((c[T+20>>2]|0)!=0){break}c[T+16>>2]=c[T+8>>2]}}while(0);if((c[(c[o>>2]|0)+16>>2]|0)==0){break L323}}N=c[x>>2]|0;if((N|0)>-1){aA=(c[Q>>2]|0)+N|0}else{aA=0}by(i,aA,(c[l>>2]|0)-N|0,m&1);c[x>>2]=c[l>>2];N=c[o>>2]|0;t=N+28|0;H=c[t>>2]|0;w=c[H+20>>2]|0;u=N+16|0;v=c[u>>2]|0;K=w>>>0>v>>>0?v:w;do{if((K|0)!=0){w=N+12|0;v=c[w>>2]|0;M=c[H+16>>2]|0;bJ(v|0,M|0,K)|0;c[w>>2]=(c[w>>2]|0)+K;w=(c[t>>2]|0)+16|0;c[w>>2]=(c[w>>2]|0)+K;w=N+20|0;c[w>>2]=(c[w>>2]|0)+K;c[u>>2]=(c[u>>2]|0)-K;w=(c[t>>2]|0)+20|0;c[w>>2]=(c[w>>2]|0)-K;w=c[t>>2]|0;if((c[w+20>>2]|0)!=0){break}c[w+16>>2]=c[w+8>>2]}}while(0);if((c[(c[o>>2]|0)+16>>2]|0)==0){ar=m?2:0;z=281;break}else{ar=m?3:1;z=281;break}}else{ar=aI[c[12400+((c[i+132>>2]|0)*12|0)>>2]&15](i,f)|0;z=281}}while(0);do{if((z|0)==281){if((ar-2|0)>>>0<2){c[k>>2]=666}if((ar|0)==2|(ar|0)==0){break}else if((ar|0)!=1){break L321}do{if((f|0)==1){bx(i)}else if((f|0)!=5){bs(i,0,0,0);if((f|0)!=3){break}q=i+76|0;P=i+68|0;b[(c[P>>2]|0)+((c[q>>2]|0)-1<<1)>>1]=0;bI(c[P>>2]|0,0,(c[q>>2]<<1)-2|0);if((c[i+116>>2]|0)!=0){break}c[i+108>>2]=0;c[i+92>>2]=0}}while(0);q=c[h>>2]|0;P=c[q+20>>2]|0;O=c[n>>2]|0;t=P>>>0>O>>>0?O:P;if((t|0)==0){aB=O}else{O=c[j>>2]|0;P=c[q+16>>2]|0;bJ(O|0,P|0,t)|0;c[j>>2]=(c[j>>2]|0)+t;P=(c[h>>2]|0)+16|0;c[P>>2]=(c[P>>2]|0)+t;P=e+20|0;c[P>>2]=(c[P>>2]|0)+t;c[n>>2]=(c[n>>2]|0)-t;P=(c[h>>2]|0)+20|0;c[P>>2]=(c[P>>2]|0)-t;t=c[h>>2]|0;if((c[t+20>>2]|0)==0){c[t+16>>2]=c[t+8>>2]}aB=c[n>>2]|0}if((aB|0)!=0){break L321}c[p>>2]=-1;g=0;return g|0}}while(0);if((c[n>>2]|0)!=0){g=0;return g|0}c[p>>2]=-1;g=0;return g|0}}while(0);if(!m){g=0;return g|0}p=i+24|0;k=c[p>>2]|0;if((k|0)<1){g=1;return g|0}o=e+48|0;t=c[o>>2]|0;if((k|0)==2){k=c[r>>2]|0;c[r>>2]=k+1;P=i+8|0;a[(c[P>>2]|0)+k|0]=t&255;k=(c[o>>2]|0)>>>8&255;O=c[r>>2]|0;c[r>>2]=O+1;a[(c[P>>2]|0)+O|0]=k;k=(c[o>>2]|0)>>>16&255;O=c[r>>2]|0;c[r>>2]=O+1;a[(c[P>>2]|0)+O|0]=k;k=(c[o>>2]|0)>>>24&255;O=c[r>>2]|0;c[r>>2]=O+1;a[(c[P>>2]|0)+O|0]=k;k=e+8|0;O=c[k>>2]&255;q=c[r>>2]|0;c[r>>2]=q+1;a[(c[P>>2]|0)+q|0]=O;O=(c[k>>2]|0)>>>8&255;q=c[r>>2]|0;c[r>>2]=q+1;a[(c[P>>2]|0)+q|0]=O;O=(c[k>>2]|0)>>>16&255;q=c[r>>2]|0;c[r>>2]=q+1;a[(c[P>>2]|0)+q|0]=O;O=(c[k>>2]|0)>>>24&255;k=c[r>>2]|0;c[r>>2]=k+1;a[(c[P>>2]|0)+k|0]=O}else{O=c[r>>2]|0;c[r>>2]=O+1;k=i+8|0;a[(c[k>>2]|0)+O|0]=t>>>24&255;O=c[r>>2]|0;c[r>>2]=O+1;a[(c[k>>2]|0)+O|0]=t>>>16&255;t=c[o>>2]|0;o=c[r>>2]|0;c[r>>2]=o+1;a[(c[k>>2]|0)+o|0]=t>>>8&255;o=c[r>>2]|0;c[r>>2]=o+1;a[(c[k>>2]|0)+o|0]=t&255}t=c[h>>2]|0;o=c[t+20>>2]|0;k=c[n>>2]|0;O=o>>>0>k>>>0?k:o;do{if((O|0)!=0){o=c[j>>2]|0;k=c[t+16>>2]|0;bJ(o|0,k|0,O)|0;c[j>>2]=(c[j>>2]|0)+O;k=(c[h>>2]|0)+16|0;c[k>>2]=(c[k>>2]|0)+O;k=e+20|0;c[k>>2]=(c[k>>2]|0)+O;c[n>>2]=(c[n>>2]|0)-O;k=(c[h>>2]|0)+20|0;c[k>>2]=(c[k>>2]|0)-O;k=c[h>>2]|0;if((c[k+20>>2]|0)!=0){break}c[k+16>>2]=c[k+8>>2]}}while(0);O=c[p>>2]|0;if((O|0)>0){c[p>>2]=-O}g=(c[r>>2]|0)==0|0;return g|0}}while(0);c[e+24>>2]=c[6];g=-2;return g|0}function a4(a){a=a|0;var f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0;f=a+44|0;g=c[f>>2]|0;h=a+60|0;i=a+116|0;j=a+108|0;k=g-262|0;l=a|0;m=a+56|0;n=a+72|0;o=a+88|0;p=a+84|0;q=a+112|0;r=a+92|0;s=a+76|0;t=a+68|0;u=a+64|0;v=c[i>>2]|0;w=g;while(1){x=c[j>>2]|0;y=(c[h>>2]|0)-v-x|0;if(x>>>0<(k+w|0)>>>0){z=y}else{x=c[m>>2]|0;A=x+g|0;bJ(x|0,A|0,g)|0;c[q>>2]=(c[q>>2]|0)-g;c[j>>2]=(c[j>>2]|0)-g;c[r>>2]=(c[r>>2]|0)-g;A=c[s>>2]|0;x=A;B=(c[t>>2]|0)+(A<<1)|0;do{B=B-2|0;A=e[B>>1]|0;if(A>>>0<g>>>0){C=0}else{C=A-g&65535}b[B>>1]=C;x=x-1|0;}while((x|0)!=0);x=g;B=(c[u>>2]|0)+(g<<1)|0;do{B=B-2|0;A=e[B>>1]|0;if(A>>>0<g>>>0){D=0}else{D=A-g&65535}b[B>>1]=D;x=x-1|0;}while((x|0)!=0);z=y+g|0}x=c[l>>2]|0;B=x+4|0;A=c[B>>2]|0;if((A|0)==0){E=355;break}F=c[i>>2]|0;G=(c[m>>2]|0)+(F+(c[j>>2]|0))|0;H=A>>>0>z>>>0?z:A;if((H|0)==0){I=0;J=F}else{c[B>>2]=A-H;A=c[(c[x+28>>2]|0)+24>>2]|0;if((A|0)==1){B=x+48|0;F=x|0;c[B>>2]=a_(c[B>>2]|0,c[F>>2]|0,H)|0;K=F}else if((A|0)==2){A=x+48|0;F=x|0;c[A>>2]=a$(c[A>>2]|0,c[F>>2]|0,H)|0;K=F}else{K=x|0}F=c[K>>2]|0;bJ(G|0,F|0,H)|0;c[K>>2]=(c[K>>2]|0)+H;F=x+8|0;c[F>>2]=(c[F>>2]|0)+H;I=H;J=c[i>>2]|0}L=J+I|0;c[i>>2]=L;if(L>>>0>2){H=c[j>>2]|0;F=c[m>>2]|0;x=d[F+H|0]|0;c[n>>2]=x;c[n>>2]=((d[F+(H+1)|0]|0)^x<<c[o>>2])&c[p>>2];if(L>>>0>=262){break}}if((c[(c[l>>2]|0)+4>>2]|0)==0){break}v=L;w=c[f>>2]|0}if((E|0)==355){return}E=a+5824|0;a=c[E>>2]|0;f=c[h>>2]|0;if(a>>>0>=f>>>0){return}h=L+(c[j>>2]|0)|0;if(a>>>0<h>>>0){j=f-h|0;L=j>>>0>258?258:j;bI((c[m>>2]|0)+h|0,0,L|0);c[E>>2]=L+h;return}L=h+258|0;if(a>>>0>=L>>>0){return}h=L-a|0;L=f-a|0;f=h>>>0>L>>>0?L:h;bI((c[m>>2]|0)+a|0,0,f|0);c[E>>2]=(c[E>>2]|0)+f;return}function a5(a,b){a=a|0;b=b|0;var d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0;d=(c[a+12>>2]|0)-5|0;e=d>>>0<65535?d:65535;d=a+116|0;f=a+108|0;g=a+92|0;h=a+44|0;i=a+56|0;j=a|0;while(1){k=c[d>>2]|0;if(k>>>0<2){a4(a);l=c[d>>2]|0;if((l|b|0)==0){m=0;n=384;break}if((l|0)==0){n=375;break}else{o=l}}else{o=k}k=(c[f>>2]|0)+o|0;c[f>>2]=k;c[d>>2]=0;l=c[g>>2]|0;p=l+e|0;if((k|0)!=0&k>>>0<p>>>0){q=k;r=l}else{c[d>>2]=k-p;c[f>>2]=p;if((l|0)>-1){s=(c[i>>2]|0)+l|0}else{s=0}by(a,s,e,0);c[g>>2]=c[f>>2];l=c[j>>2]|0;p=l+28|0;k=c[p>>2]|0;t=c[k+20>>2]|0;u=l+16|0;v=c[u>>2]|0;w=t>>>0>v>>>0?v:t;do{if((w|0)!=0){t=l+12|0;v=c[t>>2]|0;x=c[k+16>>2]|0;bJ(v|0,x|0,w)|0;c[t>>2]=(c[t>>2]|0)+w;t=(c[p>>2]|0)+16|0;c[t>>2]=(c[t>>2]|0)+w;t=l+20|0;c[t>>2]=(c[t>>2]|0)+w;c[u>>2]=(c[u>>2]|0)-w;t=(c[p>>2]|0)+20|0;c[t>>2]=(c[t>>2]|0)-w;t=c[p>>2]|0;if((c[t+20>>2]|0)!=0){break}c[t+16>>2]=c[t+8>>2]}}while(0);if((c[(c[j>>2]|0)+16>>2]|0)==0){m=0;n=385;break}q=c[f>>2]|0;r=c[g>>2]|0}p=q-r|0;if(p>>>0<((c[h>>2]|0)-262|0)>>>0){continue}if((r|0)>-1){y=(c[i>>2]|0)+r|0}else{y=0}by(a,y,p,0);c[g>>2]=c[f>>2];p=c[j>>2]|0;w=p+28|0;u=c[w>>2]|0;l=c[u+20>>2]|0;k=p+16|0;t=c[k>>2]|0;x=l>>>0>t>>>0?t:l;do{if((x|0)!=0){l=p+12|0;t=c[l>>2]|0;v=c[u+16>>2]|0;bJ(t|0,v|0,x)|0;c[l>>2]=(c[l>>2]|0)+x;l=(c[w>>2]|0)+16|0;c[l>>2]=(c[l>>2]|0)+x;l=p+20|0;c[l>>2]=(c[l>>2]|0)+x;c[k>>2]=(c[k>>2]|0)-x;l=(c[w>>2]|0)+20|0;c[l>>2]=(c[l>>2]|0)-x;l=c[w>>2]|0;if((c[l+20>>2]|0)!=0){break}c[l+16>>2]=c[l+8>>2]}}while(0);if((c[(c[j>>2]|0)+16>>2]|0)==0){m=0;n=386;break}}if((n|0)==384){return m|0}else if((n|0)==386){return m|0}else if((n|0)==385){return m|0}else if((n|0)==375){n=c[g>>2]|0;if((n|0)>-1){z=(c[i>>2]|0)+n|0}else{z=0}i=(b|0)==4;by(a,z,(c[f>>2]|0)-n|0,i&1);c[g>>2]=c[f>>2];f=c[j>>2]|0;g=f+28|0;n=c[g>>2]|0;z=c[n+20>>2]|0;a=f+16|0;b=c[a>>2]|0;y=z>>>0>b>>>0?b:z;do{if((y|0)!=0){z=f+12|0;b=c[z>>2]|0;r=c[n+16>>2]|0;bJ(b|0,r|0,y)|0;c[z>>2]=(c[z>>2]|0)+y;z=(c[g>>2]|0)+16|0;c[z>>2]=(c[z>>2]|0)+y;z=f+20|0;c[z>>2]=(c[z>>2]|0)+y;c[a>>2]=(c[a>>2]|0)-y;z=(c[g>>2]|0)+20|0;c[z>>2]=(c[z>>2]|0)-y;z=c[g>>2]|0;if((c[z+20>>2]|0)!=0){break}c[z+16>>2]=c[z+8>>2]}}while(0);if((c[(c[j>>2]|0)+16>>2]|0)==0){m=i?2:0;return m|0}else{m=i?3:1;return m|0}}return 0}function a6(e,f){e=e|0;f=f|0;var g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0;g=e+116|0;h=(f|0)==0;i=e+72|0;j=e+88|0;k=e+108|0;l=e+56|0;m=e+84|0;n=e+68|0;o=e+52|0;p=e+64|0;q=e+44|0;r=e+96|0;s=e+112|0;t=e+5792|0;u=e+5796|0;v=e+5784|0;w=e+5788|0;x=e+128|0;y=e+92|0;z=e|0;while(1){if((c[g>>2]|0)>>>0<262){a4(e);A=c[g>>2]|0;if(A>>>0<262&h){B=0;C=426;break}if((A|0)==0){C=414;break}if(A>>>0>2){C=394}else{C=397}}else{C=394}do{if((C|0)==394){C=0;A=c[k>>2]|0;D=((d[(c[l>>2]|0)+(A+2)|0]|0)^c[i>>2]<<c[j>>2])&c[m>>2];c[i>>2]=D;E=b[(c[n>>2]|0)+(D<<1)>>1]|0;b[(c[p>>2]|0)+((c[o>>2]&A)<<1)>>1]=E;A=E&65535;b[(c[n>>2]|0)+(c[i>>2]<<1)>>1]=c[k>>2]&65535;if(E<<16>>16==0){C=397;break}if(((c[k>>2]|0)-A|0)>>>0>((c[q>>2]|0)-262|0)>>>0){C=397;break}E=a7(e,A)|0;c[r>>2]=E;F=E}}while(0);if((C|0)==397){C=0;F=c[r>>2]|0}do{if(F>>>0>2){E=F+253|0;A=(c[k>>2]|0)-(c[s>>2]|0)&65535;b[(c[u>>2]|0)+(c[t>>2]<<1)>>1]=A;D=c[t>>2]|0;c[t>>2]=D+1;a[(c[v>>2]|0)+D|0]=E&255;D=A-1&65535;A=e+148+((d[13928+(E&255)|0]|0|256)+1<<2)|0;b[A>>1]=(b[A>>1]|0)+1&65535;A=D&65535;if((D&65535)<256){G=A}else{G=(A>>>7)+256|0}A=e+2440+((d[14184+G|0]|0)<<2)|0;b[A>>1]=(b[A>>1]|0)+1&65535;A=(c[t>>2]|0)==((c[w>>2]|0)-1|0)|0;D=c[r>>2]|0;E=(c[g>>2]|0)-D|0;c[g>>2]=E;if(!(D>>>0<=(c[x>>2]|0)>>>0&E>>>0>2)){E=(c[k>>2]|0)+D|0;c[k>>2]=E;c[r>>2]=0;H=c[l>>2]|0;I=d[H+E|0]|0;c[i>>2]=I;c[i>>2]=((d[H+(E+1)|0]|0)^I<<c[j>>2])&c[m>>2];J=A;K=E;break}c[r>>2]=D-1;do{D=c[k>>2]|0;E=D+1|0;c[k>>2]=E;I=((d[(c[l>>2]|0)+(D+3)|0]|0)^c[i>>2]<<c[j>>2])&c[m>>2];c[i>>2]=I;b[(c[p>>2]|0)+((c[o>>2]&E)<<1)>>1]=b[(c[n>>2]|0)+(I<<1)>>1]|0;b[(c[n>>2]|0)+(c[i>>2]<<1)>>1]=c[k>>2]&65535;I=(c[r>>2]|0)-1|0;c[r>>2]=I;}while((I|0)!=0);I=(c[k>>2]|0)+1|0;c[k>>2]=I;J=A;K=I}else{I=a[(c[l>>2]|0)+(c[k>>2]|0)|0]|0;b[(c[u>>2]|0)+(c[t>>2]<<1)>>1]=0;E=c[t>>2]|0;c[t>>2]=E+1;a[(c[v>>2]|0)+E|0]=I;E=e+148+((I&255)<<2)|0;b[E>>1]=(b[E>>1]|0)+1&65535;E=(c[t>>2]|0)==((c[w>>2]|0)-1|0)|0;c[g>>2]=(c[g>>2]|0)-1;I=(c[k>>2]|0)+1|0;c[k>>2]=I;J=E;K=I}}while(0);if((J|0)==0){continue}I=c[y>>2]|0;if((I|0)>-1){L=(c[l>>2]|0)+I|0}else{L=0}by(e,L,K-I|0,0);c[y>>2]=c[k>>2];I=c[z>>2]|0;E=I+28|0;D=c[E>>2]|0;H=c[D+20>>2]|0;M=I+16|0;N=c[M>>2]|0;O=H>>>0>N>>>0?N:H;do{if((O|0)!=0){H=I+12|0;N=c[H>>2]|0;P=c[D+16>>2]|0;bJ(N|0,P|0,O)|0;c[H>>2]=(c[H>>2]|0)+O;H=(c[E>>2]|0)+16|0;c[H>>2]=(c[H>>2]|0)+O;H=I+20|0;c[H>>2]=(c[H>>2]|0)+O;c[M>>2]=(c[M>>2]|0)-O;H=(c[E>>2]|0)+20|0;c[H>>2]=(c[H>>2]|0)-O;H=c[E>>2]|0;if((c[H+20>>2]|0)!=0){break}c[H+16>>2]=c[H+8>>2]}}while(0);if((c[(c[z>>2]|0)+16>>2]|0)==0){B=0;C=425;break}}if((C|0)==414){K=c[y>>2]|0;if((K|0)>-1){Q=(c[l>>2]|0)+K|0}else{Q=0}l=(f|0)==4;by(e,Q,(c[k>>2]|0)-K|0,l&1);c[y>>2]=c[k>>2];k=c[z>>2]|0;y=k+28|0;K=c[y>>2]|0;Q=c[K+20>>2]|0;e=k+16|0;f=c[e>>2]|0;L=Q>>>0>f>>>0?f:Q;do{if((L|0)!=0){Q=k+12|0;f=c[Q>>2]|0;J=c[K+16>>2]|0;bJ(f|0,J|0,L)|0;c[Q>>2]=(c[Q>>2]|0)+L;Q=(c[y>>2]|0)+16|0;c[Q>>2]=(c[Q>>2]|0)+L;Q=k+20|0;c[Q>>2]=(c[Q>>2]|0)+L;c[e>>2]=(c[e>>2]|0)-L;Q=(c[y>>2]|0)+20|0;c[Q>>2]=(c[Q>>2]|0)-L;Q=c[y>>2]|0;if((c[Q+20>>2]|0)!=0){break}c[Q+16>>2]=c[Q+8>>2]}}while(0);if((c[(c[z>>2]|0)+16>>2]|0)==0){B=l?2:0;return B|0}else{B=l?3:1;return B|0}}else if((C|0)==425){return B|0}else if((C|0)==426){return B|0}return 0}function a7(b,d){b=b|0;d=d|0;var f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0;f=c[b+124>>2]|0;g=c[b+56>>2]|0;h=c[b+108>>2]|0;i=g+h|0;j=c[b+120>>2]|0;k=c[b+144>>2]|0;l=(c[b+44>>2]|0)-262|0;m=h>>>0>l>>>0?h-l|0:0;l=c[b+64>>2]|0;n=c[b+52>>2]|0;o=g+(h+258)|0;p=c[b+116>>2]|0;q=k>>>0>p>>>0?p:k;k=b+112|0;r=g+(h+1)|0;s=g+(h+2)|0;t=o;u=h+257|0;v=a[g+(j+h)|0]|0;w=a[g+(h-1+j)|0]|0;x=d;d=j>>>0<(c[b+140>>2]|0)>>>0?f:f>>>2;f=j;L584:while(1){j=g+x|0;do{if((a[g+(x+f)|0]|0)==v<<24>>24){if((a[g+(f-1+x)|0]|0)!=w<<24>>24){y=v;z=w;A=f;break}if((a[j]|0)!=(a[i]|0)){y=v;z=w;A=f;break}if((a[g+(x+1)|0]|0)!=(a[r]|0)){y=v;z=w;A=f;break}b=s;B=g+(x+2)|0;while(1){C=b+1|0;if((a[C]|0)!=(a[B+1|0]|0)){D=C;break}C=b+2|0;if((a[C]|0)!=(a[B+2|0]|0)){D=C;break}C=b+3|0;if((a[C]|0)!=(a[B+3|0]|0)){D=C;break}C=b+4|0;if((a[C]|0)!=(a[B+4|0]|0)){D=C;break}C=b+5|0;if((a[C]|0)!=(a[B+5|0]|0)){D=C;break}C=b+6|0;if((a[C]|0)!=(a[B+6|0]|0)){D=C;break}C=b+7|0;if((a[C]|0)!=(a[B+7|0]|0)){D=C;break}C=b+8|0;E=B+8|0;if((a[C]|0)==(a[E]|0)&C>>>0<o>>>0){b=C;B=E}else{D=C;break}}B=D-t|0;b=B+258|0;if((b|0)<=(f|0)){y=v;z=w;A=f;break}c[k>>2]=x;if((b|0)>=(q|0)){F=b;G=447;break L584}y=a[g+(b+h)|0]|0;z=a[g+(u+B)|0]|0;A=b}else{y=v;z=w;A=f}}while(0);j=e[l+((x&n)<<1)>>1]|0;if(j>>>0<=m>>>0){F=A;G=448;break}b=d-1|0;if((b|0)==0){F=A;G=449;break}else{v=y;w=z;x=j;d=b;f=A}}if((G|0)==447){H=F>>>0>p>>>0;I=H?p:F;return I|0}else if((G|0)==448){H=F>>>0>p>>>0;I=H?p:F;return I|0}else if((G|0)==449){H=F>>>0>p>>>0;I=H?p:F;return I|0}return 0}function a8(e,f){e=e|0;f=f|0;var g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0,X=0;g=e+116|0;h=(f|0)==0;i=e+72|0;j=e+88|0;k=e+108|0;l=e+56|0;m=e+84|0;n=e+68|0;o=e+52|0;p=e+64|0;q=e+96|0;r=e+120|0;s=e+112|0;t=e+100|0;u=e+5792|0;v=e+5796|0;w=e+5784|0;x=e+5788|0;y=e+104|0;z=e+92|0;A=e|0;B=e+128|0;C=e+44|0;D=e+136|0;L610:while(1){E=c[g>>2]|0;while(1){do{if(E>>>0<262){a4(e);F=c[g>>2]|0;if(F>>>0<262&h){G=0;H=500;break L610}if((F|0)==0){H=489;break L610}if(F>>>0>2){H=457;break}c[r>>2]=c[q>>2];c[t>>2]=c[s>>2];c[q>>2]=2;I=2;H=465}else{H=457}}while(0);do{if((H|0)==457){H=0;F=c[k>>2]|0;J=((d[(c[l>>2]|0)+(F+2)|0]|0)^c[i>>2]<<c[j>>2])&c[m>>2];c[i>>2]=J;K=b[(c[n>>2]|0)+(J<<1)>>1]|0;b[(c[p>>2]|0)+((c[o>>2]&F)<<1)>>1]=K;F=K&65535;b[(c[n>>2]|0)+(c[i>>2]<<1)>>1]=c[k>>2]&65535;J=c[q>>2]|0;c[r>>2]=J;c[t>>2]=c[s>>2];c[q>>2]=2;if(K<<16>>16==0){I=2;H=465;break}if(J>>>0>=(c[B>>2]|0)>>>0){L=J;M=2;break}if(((c[k>>2]|0)-F|0)>>>0>((c[C>>2]|0)-262|0)>>>0){I=2;H=465;break}J=a7(e,F)|0;c[q>>2]=J;if(J>>>0>=6){I=J;H=465;break}if((c[D>>2]|0)!=1){if((J|0)!=3){I=J;H=465;break}if(((c[k>>2]|0)-(c[s>>2]|0)|0)>>>0<=4096){I=3;H=465;break}}c[q>>2]=2;I=2;H=465}}while(0);if((H|0)==465){H=0;L=c[r>>2]|0;M=I}if(!(L>>>0<3|M>>>0>L>>>0)){break}if((c[y>>2]|0)==0){c[y>>2]=1;c[k>>2]=(c[k>>2]|0)+1;J=(c[g>>2]|0)-1|0;c[g>>2]=J;E=J;continue}J=a[(c[l>>2]|0)+((c[k>>2]|0)-1)|0]|0;b[(c[v>>2]|0)+(c[u>>2]<<1)>>1]=0;F=c[u>>2]|0;c[u>>2]=F+1;a[(c[w>>2]|0)+F|0]=J;F=e+148+((J&255)<<2)|0;b[F>>1]=(b[F>>1]|0)+1&65535;do{if((c[u>>2]|0)==((c[x>>2]|0)-1|0)){F=c[z>>2]|0;if((F|0)>-1){N=(c[l>>2]|0)+F|0}else{N=0}by(e,N,(c[k>>2]|0)-F|0,0);c[z>>2]=c[k>>2];F=c[A>>2]|0;J=F+28|0;K=c[J>>2]|0;O=c[K+20>>2]|0;P=F+16|0;Q=c[P>>2]|0;R=O>>>0>Q>>>0?Q:O;if((R|0)==0){break}O=F+12|0;Q=c[O>>2]|0;S=c[K+16>>2]|0;bJ(Q|0,S|0,R)|0;c[O>>2]=(c[O>>2]|0)+R;O=(c[J>>2]|0)+16|0;c[O>>2]=(c[O>>2]|0)+R;O=F+20|0;c[O>>2]=(c[O>>2]|0)+R;c[P>>2]=(c[P>>2]|0)-R;P=(c[J>>2]|0)+20|0;c[P>>2]=(c[P>>2]|0)-R;R=c[J>>2]|0;if((c[R+20>>2]|0)!=0){break}c[R+16>>2]=c[R+8>>2]}}while(0);c[k>>2]=(c[k>>2]|0)+1;R=(c[g>>2]|0)-1|0;c[g>>2]=R;if((c[(c[A>>2]|0)+16>>2]|0)==0){G=0;H=502;break L610}else{E=R}}E=c[k>>2]|0;R=E-3+(c[g>>2]|0)|0;J=L+253|0;P=E+65535-(c[t>>2]|0)&65535;b[(c[v>>2]|0)+(c[u>>2]<<1)>>1]=P;E=c[u>>2]|0;c[u>>2]=E+1;a[(c[w>>2]|0)+E|0]=J&255;E=P-1&65535;P=e+148+((d[13928+(J&255)|0]|0|256)+1<<2)|0;b[P>>1]=(b[P>>1]|0)+1&65535;P=E&65535;if((E&65535)<256){T=P}else{T=(P>>>7)+256|0}P=e+2440+((d[14184+T|0]|0)<<2)|0;b[P>>1]=(b[P>>1]|0)+1&65535;P=c[u>>2]|0;E=(c[x>>2]|0)-1|0;J=c[r>>2]|0;c[g>>2]=1-J+(c[g>>2]|0);O=J-2|0;c[r>>2]=O;J=O;do{O=c[k>>2]|0;F=O+1|0;c[k>>2]=F;if(F>>>0>R>>>0){U=J}else{S=((d[(c[l>>2]|0)+(O+3)|0]|0)^c[i>>2]<<c[j>>2])&c[m>>2];c[i>>2]=S;b[(c[p>>2]|0)+((c[o>>2]&F)<<1)>>1]=b[(c[n>>2]|0)+(S<<1)>>1]|0;b[(c[n>>2]|0)+(c[i>>2]<<1)>>1]=c[k>>2]&65535;U=c[r>>2]|0}J=U-1|0;c[r>>2]=J;}while((J|0)!=0);c[y>>2]=0;c[q>>2]=2;J=(c[k>>2]|0)+1|0;c[k>>2]=J;if((P|0)!=(E|0)){continue}R=c[z>>2]|0;if((R|0)>-1){V=(c[l>>2]|0)+R|0}else{V=0}by(e,V,J-R|0,0);c[z>>2]=c[k>>2];R=c[A>>2]|0;J=R+28|0;S=c[J>>2]|0;F=c[S+20>>2]|0;O=R+16|0;Q=c[O>>2]|0;K=F>>>0>Q>>>0?Q:F;do{if((K|0)!=0){F=R+12|0;Q=c[F>>2]|0;W=c[S+16>>2]|0;bJ(Q|0,W|0,K)|0;c[F>>2]=(c[F>>2]|0)+K;F=(c[J>>2]|0)+16|0;c[F>>2]=(c[F>>2]|0)+K;F=R+20|0;c[F>>2]=(c[F>>2]|0)+K;c[O>>2]=(c[O>>2]|0)-K;F=(c[J>>2]|0)+20|0;c[F>>2]=(c[F>>2]|0)-K;F=c[J>>2]|0;if((c[F+20>>2]|0)!=0){break}c[F+16>>2]=c[F+8>>2]}}while(0);if((c[(c[A>>2]|0)+16>>2]|0)==0){G=0;H=501;break}}if((H|0)==489){if((c[y>>2]|0)!=0){V=a[(c[l>>2]|0)+((c[k>>2]|0)-1)|0]|0;b[(c[v>>2]|0)+(c[u>>2]<<1)>>1]=0;v=c[u>>2]|0;c[u>>2]=v+1;a[(c[w>>2]|0)+v|0]=V;v=e+148+((V&255)<<2)|0;b[v>>1]=(b[v>>1]|0)+1&65535;c[y>>2]=0}y=c[z>>2]|0;if((y|0)>-1){X=(c[l>>2]|0)+y|0}else{X=0}l=(f|0)==4;by(e,X,(c[k>>2]|0)-y|0,l&1);c[z>>2]=c[k>>2];k=c[A>>2]|0;z=k+28|0;y=c[z>>2]|0;X=c[y+20>>2]|0;e=k+16|0;f=c[e>>2]|0;v=X>>>0>f>>>0?f:X;do{if((v|0)!=0){X=k+12|0;f=c[X>>2]|0;V=c[y+16>>2]|0;bJ(f|0,V|0,v)|0;c[X>>2]=(c[X>>2]|0)+v;X=(c[z>>2]|0)+16|0;c[X>>2]=(c[X>>2]|0)+v;X=k+20|0;c[X>>2]=(c[X>>2]|0)+v;c[e>>2]=(c[e>>2]|0)-v;X=(c[z>>2]|0)+20|0;c[X>>2]=(c[X>>2]|0)-v;X=c[z>>2]|0;if((c[X+20>>2]|0)!=0){break}c[X+16>>2]=c[X+8>>2]}}while(0);if((c[(c[A>>2]|0)+16>>2]|0)==0){G=l?2:0;return G|0}else{G=l?3:1;return G|0}}else if((H|0)==500){return G|0}else if((H|0)==501){return G|0}else if((H|0)==502){return G|0}return 0}function a9(a){a=a|0;var b=0;do{if((a|0)==0){b=-2}else{if((c[a>>2]|0)==7247){b=bg(a)|0;break}else{b=bn(a)|0;break}}}while(0);return b|0}function ba(a,b){a=a|0;b=b|0;return bb(a,-1,b)|0}function bb(b,d,e){b=b|0;d=d|0;e=e|0;var f=0,g=0,h=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,t=0,u=0,v=0,w=0,x=0,y=0;f=i;g=bF(140)|0;if((g|0)==0){h=0;i=f;return h|0}c[g+16>>2]=0;c[g+20>>2]=8192;j=g+80|0;c[j>>2]=0;k=g;c[k>>2]=0;l=g+60|0;c[l>>2]=-1;m=g+64|0;c[m>>2]=0;n=a[e]|0;do{if(n<<24>>24!=0){o=e;p=n;q=0;L695:while(1){r=p<<24>>24;do{if((p-48&255)<10){c[l>>2]=r-48;t=q}else{if((r|0)==114){c[k>>2]=7247;t=7247;break}else if((r|0)==119){c[k>>2]=31153;t=31153;break}else if((r|0)==97){c[k>>2]=1;t=1;break}else if((r|0)==43){u=519;break L695}else if((r|0)==102){c[m>>2]=1;t=q;break}else if((r|0)==104){c[m>>2]=2;t=q;break}else if((r|0)==82){c[m>>2]=3;t=q;break}else if((r|0)==70){c[m>>2]=4;t=q;break}else{t=q;break}}}while(0);r=o+1|0;v=a[r]|0;if(v<<24>>24==0){break}else{o=r;p=v;q=t}}if((u|0)==519){bG(g);h=0;i=f;return h|0}if((t|0)==0){break}q=bF((bK(b|0)|0)+1|0)|0;p=g+8|0;c[p>>2]=q;if((q|0)==0){bG(g);h=0;i=f;return h|0}bL(q|0,b|0)|0;do{if((d|0)==-1){q=c[k>>2]|0;if((q|0)==7247){w=0}else{w=(q|0)==31153?1537:521}q=ap(b|0,w|0,(s=i,i=i+8|0,c[s>>2]=438,s)|0)|0;c[g+4>>2]=q;if((q|0)!=-1){x=q;break}bG(c[p>>2]|0);bG(g);h=0;i=f;return h|0}else{c[g+4>>2]=d;x=d}}while(0);p=c[k>>2]|0;do{if((p|0)==1){c[k>>2]=31153}else if((p|0)==7247){q=ak(x|0,0,1)|0;if((q|0)==-1){y=0}else{y=q}c[g+44>>2]=y;if((c[k>>2]|0)!=7247){break}c[g+36>>2]=0;c[g+40>>2]=0;c[g+52>>2]=0;c[g+56>>2]=1}}while(0);c[g+72>>2]=0;p=c[j>>2]|0;q=g+76|0;if((p|0)!=0){if((c[q>>2]|0)!=-4){bG(p)}c[j>>2]=0}c[q>>2]=0;c[g+12>>2]=0;c[g+88>>2]=0;h=g;i=f;return h|0}}while(0);bG(g);h=0;i=f;return h|0}function bc(b,d,e){b=b|0;d=d|0;e=e|0;var f=0,g=0,h=0;f=b+80|0;g=c[f>>2]|0;h=b+76|0;if((g|0)!=0){if((c[h>>2]|0)!=-4){bG(g)}c[f>>2]=0}c[h>>2]=d;if((e|0)==0){return}if((d|0)==-4){c[f>>2]=e;return}d=c[b+8>>2]|0;b=bK(d|0)|0;g=bF(b+3+(bK(e|0)|0)|0)|0;c[f>>2]=g;if((g|0)==0){c[h>>2]=-4;c[f>>2]=13480;return}else{bL(g|0,d|0)|0;d=c[f>>2]|0;g=d+(bK(d|0)|0)|0;a[g]=a[13376]|0;a[g+1|0]=a[13377|0]|0;a[g+2|0]=a[13378|0]|0;g=c[f>>2]|0;bM(g|0,e|0)|0;return}}function bd(a,b,d){a=a|0;b=b|0;d=d|0;var e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0;if((a|0)==0){e=-1;return e|0}f=a;if((c[a>>2]|0)!=7247){e=-1;return e|0}if((c[a+76>>2]|0)!=0){e=-1;return e|0}if((d|0)<0){bc(f,-5,12832);e=-1;return e|0}if((d|0)==0){e=0;return e|0}g=a+72|0;L780:do{if((c[g>>2]|0)==0){h=a+36|0;i=a+40|0;j=a+32|0;k=a+88|0;l=a+12|0}else{c[g>>2]=0;m=c[a+68>>2]|0;n=a+36|0;o=a+40|0;p=a+88|0;q=a+32|0;r=a+12|0;if((m|0)==0){h=n;i=o;j=q;k=p;l=r;break}s=a+52|0;t=a+28|0;u=a+16|0;v=a+4|0;w=a+100|0;x=a+96|0;y=m;L784:while(1){L786:while(1){z=c[n>>2]|0;while(1){if((z|0)!=0){break L786}if((c[o>>2]|0)!=0){if((c[p>>2]|0)==0){h=n;i=o;j=q;k=p;l=r;break L780}}m=c[s>>2]|0;if((m|0)!=0){A=m;break}if((bf(f)|0)==-1){e=-1;B=631;break L784}m=c[n>>2]|0;if((m|0)==0){B=588;break}else{z=m}}if((B|0)==588){B=0;A=c[s>>2]|0}if((A|0)==2){c[w>>2]=c[u>>2]<<1;c[x>>2]=c[t>>2];if((be(f)|0)==-1){e=-1;B=633;break L784}else{continue}}else if((A|0)!=1){continue}m=c[t>>2]|0;C=c[u>>2]<<1;c[n>>2]=0;D=0;do{E=at(c[v>>2]|0,m+D|0,C-D|0)|0;if((E|0)<1){B=593;break}D=(c[n>>2]|0)+E|0;c[n>>2]=D;}while(D>>>0<C>>>0);if((B|0)==593){B=0;if((E|0)<0){B=595;break L784}c[o>>2]=1}c[q>>2]=c[t>>2]}C=(z|0)<0|(z|0)>(y|0)?y:z;c[n>>2]=z-C;c[q>>2]=(c[q>>2]|0)+C;c[r>>2]=(c[r>>2]|0)+C;if((y|0)==(C|0)){h=n;i=o;j=q;k=p;l=r;break L780}else{y=y-C|0}}if((B|0)==595){bc(f,-1,az(c[(ay()|0)>>2]|0)|0);e=-1;return e|0}else if((B|0)==631){return e|0}else if((B|0)==633){return e|0}}}while(0);z=a+52|0;E=a+28|0;A=a+16|0;g=a+4|0;y=a+100|0;r=a+96|0;a=b;b=0;p=d;L816:while(1){d=c[h>>2]|0;L818:do{if((d|0)==0){if((c[i>>2]|0)!=0){if((c[k>>2]|0)==0){e=b;B=634;break L816}}q=c[z>>2]|0;do{if((q|0)==0){if((bf(f)|0)==-1){e=-1;B=635;break L816}if((c[h>>2]|0)!=0){F=a;G=b;H=p;break L818}I=c[z>>2]|0}else{if(p>>>0<c[A>>2]<<1>>>0){I=q;break}if((q|0)==1){J=0}else{c[y>>2]=p;c[r>>2]=a;if((be(f)|0)==-1){e=-1;B=639;break L816}o=c[h>>2]|0;c[h>>2]=0;K=o;B=624;break L818}while(1){L=at(c[g>>2]|0,a+J|0,p-J|0)|0;if((L|0)<1){break}o=L+J|0;if(o>>>0<p>>>0){J=o}else{K=o;B=624;break L818}}if((L|0)<0){B=621;break L816}c[i>>2]=1;K=J;B=624;break L818}}while(0);if((I|0)==2){c[y>>2]=c[A>>2]<<1;c[r>>2]=c[E>>2];if((be(f)|0)==-1){e=-1;B=637;break L816}else{F=a;G=b;H=p;break}}else if((I|0)!=1){F=a;G=b;H=p;break}q=c[E>>2]|0;o=c[A>>2]<<1;c[h>>2]=0;n=0;do{M=at(c[g>>2]|0,q+n|0,o-n|0)|0;if((M|0)<1){B=611;break}n=(c[h>>2]|0)+M|0;c[h>>2]=n;}while(n>>>0<o>>>0);if((B|0)==611){B=0;if((M|0)<0){B=613;break L816}c[i>>2]=1}c[j>>2]=c[E>>2];F=a;G=b;H=p}else{o=d>>>0>p>>>0?p:d;n=c[j>>2]|0;bJ(a|0,n|0,o)|0;c[j>>2]=(c[j>>2]|0)+o;c[h>>2]=(c[h>>2]|0)-o;K=o;B=624}}while(0);if((B|0)==624){B=0;c[l>>2]=(c[l>>2]|0)+K;F=a+K|0;G=K+b|0;H=p-K|0}if((H|0)==0){e=G;B=640;break}else{a=F;b=G;p=H}}if((B|0)==613){bc(f,-1,az(c[(ay()|0)>>2]|0)|0);e=-1;return e|0}else if((B|0)==621){bc(f,-1,az(c[(ay()|0)>>2]|0)|0);e=-1;return e|0}else if((B|0)==634){return e|0}else if((B|0)==635){return e|0}else if((B|0)==637){return e|0}else if((B|0)==639){return e|0}else if((B|0)==640){return e|0}return 0}function be(a){a=a|0;var b=0,d=0,e=0,f=0,g=0,h=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0;b=i;i=i+16|0;d=b|0;e=b+8|0;f=a+84|0;g=a+100|0;h=c[g>>2]|0;j=a+88|0;k=a+76|0;l=a+40|0;m=a+24|0;n=a+16|0;o=a+4|0;p=a+84|0;while(1){if((c[j>>2]|0)==0){if((c[k>>2]|0)!=0){q=-1;r=669;break}if((c[l>>2]|0)!=0){r=653;break}s=c[m>>2]|0;t=c[n>>2]|0;c[j>>2]=0;u=0;while(1){v=at(c[o>>2]|0,s+u|0,t-u|0)|0;if((v|0)<1){r=649;break}w=(c[j>>2]|0)+v|0;c[j>>2]=w;if(w>>>0<t>>>0){u=w}else{x=w;break}}if((r|0)==649){r=0;if((v|0)<0){r=651;break}c[l>>2]=1;x=c[j>>2]|0}c[p>>2]=c[m>>2];if((x|0)==0){r=653;break}}u=bq(f,0)|0;if((u|0)==(-2|0)|(u|0)==2){r=655;break}else if((u|0)==(-4|0)){r=656;break}else if((u|0)==(-3|0)){r=657;break}y=c[g>>2]|0;z=(u|0)==1;if((y|0)==0|z){r=659;break}}if((r|0)==651){bc(a,-1,az(c[(ay()|0)>>2]|0)|0);q=-1;i=b;return q|0}else if((r|0)==653){bc(a,-3,13112);q=-1;i=b;return q|0}else if((r|0)==655){bc(a,-2,13024);q=-1;i=b;return q|0}else if((r|0)==656){bc(a,-4,13464);q=-1;i=b;return q|0}else if((r|0)==657){g=c[a+108>>2]|0;bc(a,-3,(g|0)==0?12936:g);q=-1;i=b;return q|0}else if((r|0)==659){g=h-y|0;c[a+36>>2]=g;y=(c[a+96>>2]|0)+(-g|0)|0;c[a+32>>2]=y;h=a+132|0;c[h>>2]=a$(c[h>>2]|0,y,g)|0;if(!z){q=0;i=b;return q|0}do{if((bh(a,d)|0)!=-1){if((bh(a,e)|0)==-1){break}if((c[d>>2]|0)!=(c[h>>2]|0)){bc(a,-3,12808);q=-1;i=b;return q|0}if((c[e>>2]|0)==(c[a+104>>2]|0)){c[a+52>>2]=0;q=0;i=b;return q|0}else{bc(a,-3,13904);q=-1;i=b;return q|0}}}while(0);bc(a,-3,13112);q=-1;i=b;return q|0}else if((r|0)==669){i=b;return q|0}return 0}function bf(b){b=b|0;var e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0,X=0,Y=0,Z=0,_=0,$=0,aa=0,ab=0,ac=0,ad=0,ae=0,af=0,ag=0,ah=0,ai=0,aj=0,ak=0,al=0,am=0,an=0,ao=0,ap=0,aq=0,ar=0,as=0,au=0,av=0,aw=0,ax=0,aA=0,aB=0,aC=0,aD=0,aE=0,aF=0,aG=0,aH=0,aI=0,aJ=0,aK=0,aL=0,aM=0,aN=0,aO=0,aP=0,aQ=0,aR=0,aS=0;e=b+84|0;f=b+16|0;do{if((c[f>>2]|0)==0){g=c[b+20>>2]|0;h=bF(g)|0;i=b+24|0;c[i>>2]=h;j=bF(g<<1)|0;k=b+28|0;c[k>>2]=j;l=(j|0)==0;if(!((h|0)==0|l)){c[f>>2]=g;c[b+116>>2]=0;c[b+120>>2]=0;c[b+124>>2]=0;g=b+88|0;c[g>>2]=0;c[e>>2]=0;if((bp(e,-15,13368,56)|0)==0){m=g;break}bG(c[k>>2]|0);bG(c[i>>2]|0);c[f>>2]=0;bc(b,-4,13464);n=-1;return n|0}if(l){o=h}else{bG(j);o=c[i>>2]|0}if((o|0)!=0){bG(o)}bc(b,-4,13464);n=-1;return n|0}else{m=b+88|0}}while(0);o=c[m>>2]|0;do{if((o|0)==0){if((c[b+76>>2]|0)!=0){n=-1;return n|0}i=b+40|0;if((c[i>>2]|0)!=0){n=0;return n|0}j=b+24|0;h=c[j>>2]|0;l=c[f>>2]|0;c[m>>2]=0;k=b+4|0;g=0;while(1){p=at(c[k>>2]|0,h+g|0,l-g|0)|0;if((p|0)<1){q=696;break}r=(c[m>>2]|0)+p|0;c[m>>2]=r;if(r>>>0<l>>>0){g=r}else{s=r;break}}do{if((q|0)==696){if((p|0)>=0){c[i>>2]=1;s=c[m>>2]|0;break}bc(b,-1,az(c[(ay()|0)>>2]|0)|0);n=-1;return n|0}}while(0);c[b+84>>2]=c[j>>2];if((s|0)==0){n=0}else{t=s;break}return n|0}else{t=o}}while(0);o=e|0;s=c[o>>2]|0;if((a[s]|0)==31){p=t-1|0;c[m>>2]=p;i=s+1|0;c[o>>2]=i;do{if((p|0)==0){if((c[b+76>>2]|0)!=0){n=-1;return n|0}s=b+40|0;if((c[s>>2]|0)!=0){break}g=b+24|0;l=c[g>>2]|0;h=c[f>>2]|0;c[m>>2]=0;k=b+4|0;r=0;while(1){u=at(c[k>>2]|0,l+r|0,h-r|0)|0;if((u|0)<1){q=708;break}v=(c[m>>2]|0)+u|0;c[m>>2]=v;if(v>>>0<h>>>0){r=v}else{w=v;break}}do{if((q|0)==708){if((u|0)>=0){c[s>>2]=1;w=c[m>>2]|0;break}bc(b,-1,az(c[(ay()|0)>>2]|0)|0);n=-1;return n|0}}while(0);s=c[g>>2]|0;c[b+84>>2]=s;if((w|0)!=0){x=w;y=s;q=712}}else{x=p;y=i;q=712}}while(0);do{if((q|0)==712){if((a[y]|0)!=-117){break}i=x-1|0;c[m>>2]=i;p=y+1|0;c[o>>2]=p;L967:do{if((i|0)==0){if((c[b+76>>2]|0)!=0){break}w=b+40|0;if((c[w>>2]|0)!=0){break}u=b+24|0;s=c[u>>2]|0;r=c[f>>2]|0;c[m>>2]=0;h=b+4|0;l=0;while(1){z=at(c[h>>2]|0,s+l|0,r-l|0)|0;if((z|0)<1){q=719;break}k=(c[m>>2]|0)+z|0;c[m>>2]=k;if(k>>>0<r>>>0){l=k}else{A=k;break}}do{if((q|0)==719){if((z|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);break L967}else{c[w>>2]=1;A=c[m>>2]|0;break}}}while(0);w=c[u>>2]|0;c[b+84>>2]=w;if((A|0)!=0){B=A;C=w;q=723}}else{B=i;C=p;q=723}}while(0);do{if((q|0)==723){p=B-1|0;c[m>>2]=p;i=C+1|0;c[o>>2]=i;if((a[C]|0)!=8){break}L983:do{if((p|0)==0){if((c[b+76>>2]|0)!=0){break}g=b+40|0;if((c[g>>2]|0)!=0){break}w=b+24|0;l=c[w>>2]|0;r=c[f>>2]|0;c[m>>2]=0;s=b+4|0;h=0;while(1){D=at(c[s>>2]|0,l+h|0,r-h|0)|0;if((D|0)<1){q=731;break}k=(c[m>>2]|0)+D|0;c[m>>2]=k;if(k>>>0<r>>>0){h=k}else{E=k;break}}do{if((q|0)==731){if((D|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);break L983}else{c[g>>2]=1;E=c[m>>2]|0;break}}}while(0);g=c[w>>2]|0;c[b+84>>2]=g;if((E|0)!=0){F=E;G=g;q=735}}else{F=p;G=i;q=735}}while(0);do{if((q|0)==735){i=F-1|0;c[m>>2]=i;p=G+1|0;c[o>>2]=p;u=d[G]|0;if((u&224|0)!=0){break}L999:do{if((i|0)==0){g=b+76|0;if((c[g>>2]|0)!=0){q=749;break}h=b+40|0;if((c[h>>2]|0)!=0){H=g;q=750;break}r=b+24|0;l=c[r>>2]|0;s=c[f>>2]|0;c[m>>2]=0;k=b+4|0;j=0;while(1){I=at(c[k>>2]|0,l+j|0,s-j|0)|0;if((I|0)<1){q=743;break}v=(c[m>>2]|0)+I|0;c[m>>2]=v;if(v>>>0<s>>>0){j=v}else{J=v;break}}do{if((q|0)==743){if((I|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);K=c[m>>2]|0;q=748;break L999}else{c[h>>2]=1;J=c[m>>2]|0;break}}}while(0);h=c[r>>2]|0;c[b+84>>2]=h;if((J|0)==0){H=g;q=750}else{L=J;M=h;q=747}}else{L=i;M=p;q=747}}while(0);if((q|0)==747){p=L-1|0;c[m>>2]=p;c[o>>2]=M+1;K=p;q=748}if((q|0)==748){if((K|0)==0){q=749}else{N=K;q=759}}if((q|0)==749){H=b+76|0;q=750}L1018:do{if((q|0)==750){if((c[H>>2]|0)!=0){q=761;break}p=b+40|0;if((c[p>>2]|0)!=0){O=H;q=762;break}i=b+24|0;w=c[i>>2]|0;h=c[f>>2]|0;c[m>>2]=0;j=b+4|0;s=0;while(1){P=at(c[j>>2]|0,w+s|0,h-s|0)|0;if((P|0)<1){q=755;break}l=(c[m>>2]|0)+P|0;c[m>>2]=l;if(l>>>0<h>>>0){s=l}else{Q=l;break}}do{if((q|0)==755){if((P|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);R=c[m>>2]|0;q=760;break L1018}else{c[p>>2]=1;Q=c[m>>2]|0;break}}}while(0);c[b+84>>2]=c[i>>2];if((Q|0)==0){O=H;q=762}else{N=Q;q=759}}}while(0);if((q|0)==759){p=N-1|0;c[m>>2]=p;c[o>>2]=(c[o>>2]|0)+1;R=p;q=760}if((q|0)==760){if((R|0)==0){q=761}else{S=R;q=771}}if((q|0)==761){O=b+76|0;q=762}L1037:do{if((q|0)==762){if((c[O>>2]|0)!=0){q=773;break}p=b+40|0;if((c[p>>2]|0)!=0){T=O;q=774;break}s=b+24|0;h=c[s>>2]|0;w=c[f>>2]|0;c[m>>2]=0;j=b+4|0;g=0;while(1){U=at(c[j>>2]|0,h+g|0,w-g|0)|0;if((U|0)<1){q=767;break}r=(c[m>>2]|0)+U|0;c[m>>2]=r;if(r>>>0<w>>>0){g=r}else{V=r;break}}do{if((q|0)==767){if((U|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);W=c[m>>2]|0;q=772;break L1037}else{c[p>>2]=1;V=c[m>>2]|0;break}}}while(0);c[b+84>>2]=c[s>>2];if((V|0)==0){T=O;q=774}else{S=V;q=771}}}while(0);if((q|0)==771){p=S-1|0;c[m>>2]=p;c[o>>2]=(c[o>>2]|0)+1;W=p;q=772}if((q|0)==772){if((W|0)==0){q=773}else{X=W;q=783}}if((q|0)==773){T=b+76|0;q=774}L1056:do{if((q|0)==774){if((c[T>>2]|0)!=0){q=785;break}p=b+40|0;if((c[p>>2]|0)!=0){Y=T;q=786;break}g=b+24|0;w=c[g>>2]|0;h=c[f>>2]|0;c[m>>2]=0;j=b+4|0;i=0;while(1){Z=at(c[j>>2]|0,w+i|0,h-i|0)|0;if((Z|0)<1){q=779;break}r=(c[m>>2]|0)+Z|0;c[m>>2]=r;if(r>>>0<h>>>0){i=r}else{_=r;break}}do{if((q|0)==779){if((Z|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);$=c[m>>2]|0;q=784;break L1056}else{c[p>>2]=1;_=c[m>>2]|0;break}}}while(0);c[b+84>>2]=c[g>>2];if((_|0)==0){Y=T;q=786}else{X=_;q=783}}}while(0);if((q|0)==783){p=X-1|0;c[m>>2]=p;c[o>>2]=(c[o>>2]|0)+1;$=p;q=784}if((q|0)==784){if(($|0)==0){q=785}else{aa=$;q=795}}if((q|0)==785){Y=b+76|0;q=786}L1075:do{if((q|0)==786){if((c[Y>>2]|0)!=0){q=797;break}p=b+40|0;if((c[p>>2]|0)!=0){ab=Y;q=798;break}i=b+24|0;h=c[i>>2]|0;w=c[f>>2]|0;c[m>>2]=0;j=b+4|0;s=0;while(1){ac=at(c[j>>2]|0,h+s|0,w-s|0)|0;if((ac|0)<1){q=791;break}r=(c[m>>2]|0)+ac|0;c[m>>2]=r;if(r>>>0<w>>>0){s=r}else{ad=r;break}}do{if((q|0)==791){if((ac|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);ae=c[m>>2]|0;q=796;break L1075}else{c[p>>2]=1;ad=c[m>>2]|0;break}}}while(0);c[b+84>>2]=c[i>>2];if((ad|0)==0){ab=Y;q=798}else{aa=ad;q=795}}}while(0);if((q|0)==795){p=aa-1|0;c[m>>2]=p;c[o>>2]=(c[o>>2]|0)+1;ae=p;q=796}if((q|0)==796){if((ae|0)==0){q=797}else{af=ae;q=807}}if((q|0)==797){ab=b+76|0;q=798}L1094:do{if((q|0)==798){if((c[ab>>2]|0)!=0){break}p=b+40|0;if((c[p>>2]|0)!=0){break}s=b+24|0;w=c[s>>2]|0;h=c[f>>2]|0;c[m>>2]=0;j=b+4|0;g=0;while(1){ag=at(c[j>>2]|0,w+g|0,h-g|0)|0;if((ag|0)<1){q=803;break}r=(c[m>>2]|0)+ag|0;c[m>>2]=r;if(r>>>0<h>>>0){g=r}else{ah=r;break}}do{if((q|0)==803){if((ag|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);break L1094}else{c[p>>2]=1;ah=c[m>>2]|0;break}}}while(0);c[b+84>>2]=c[s>>2];if((ah|0)!=0){af=ah;q=807}}}while(0);if((q|0)==807){c[m>>2]=af-1;c[o>>2]=(c[o>>2]|0)+1}L1110:do{if((u&4|0)!=0){p=c[m>>2]|0;L1112:do{if((p|0)==0){g=b+76|0;if((c[g>>2]|0)!=0){ai=-1;q=821;break}h=b+40|0;if((c[h>>2]|0)!=0){aj=-1;ak=g;q=822;break}w=b+24|0;j=c[w>>2]|0;i=c[f>>2]|0;c[m>>2]=0;r=b+4|0;l=0;while(1){al=at(c[r>>2]|0,j+l|0,i-l|0)|0;if((al|0)<1){q=815;break}k=(c[m>>2]|0)+al|0;c[m>>2]=k;if(k>>>0<i>>>0){l=k}else{am=k;break}}do{if((q|0)==815){if((al|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);an=-1;ao=c[m>>2]|0;q=820;break L1112}else{c[h>>2]=1;am=c[m>>2]|0;break}}}while(0);c[b+84>>2]=c[w>>2];if((am|0)==0){aj=-1;ak=g;q=822}else{ap=am;q=819}}else{ap=p;q=819}}while(0);if((q|0)==819){p=ap-1|0;c[m>>2]=p;s=c[o>>2]|0;c[o>>2]=s+1;an=d[s]|0;ao=p;q=820}if((q|0)==820){if((ao|0)==0){ai=an;q=821}else{aq=ao;ar=an;q=831}}if((q|0)==821){aj=ai;ak=b+76|0;q=822}L1131:do{if((q|0)==822){if((c[ak>>2]|0)!=0){as=-256;au=aj;break}p=b+40|0;if((c[p>>2]|0)!=0){as=-256;au=aj;break}s=b+24|0;h=c[s>>2]|0;l=c[f>>2]|0;c[m>>2]=0;i=b+4|0;j=0;while(1){av=at(c[i>>2]|0,h+j|0,l-j|0)|0;if((av|0)<1){q=827;break}r=(c[m>>2]|0)+av|0;c[m>>2]=r;if(r>>>0<l>>>0){j=r}else{aw=r;break}}do{if((q|0)==827){if((av|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);as=-256;au=aj;break L1131}else{c[p>>2]=1;aw=c[m>>2]|0;break}}}while(0);c[b+84>>2]=c[s>>2];if((aw|0)==0){as=-256;au=aj}else{aq=aw;ar=aj;q=831}}}while(0);if((q|0)==831){c[m>>2]=aq-1;p=c[o>>2]|0;c[o>>2]=p+1;as=d[p]<<8;au=ar}p=as+au|0;if((p|0)==0){break}j=b+76|0;l=b+40|0;h=b+24|0;i=b+4|0;g=b+84|0;w=p;p=c[m>>2]|0;while(1){w=w-1|0;if((p|0)==0){if((c[j>>2]|0)!=0){break L1110}if((c[l>>2]|0)!=0){break L1110}r=c[h>>2]|0;k=c[f>>2]|0;c[m>>2]=0;v=0;while(1){ax=at(c[i>>2]|0,r+v|0,k-v|0)|0;if((ax|0)<1){q=840;break}aA=(c[m>>2]|0)+ax|0;c[m>>2]=aA;if(aA>>>0<k>>>0){v=aA}else{aB=aA;break}}if((q|0)==840){q=0;if((ax|0)<0){break}c[l>>2]=1;aB=c[m>>2]|0}c[g>>2]=c[h>>2];if((aB|0)==0){break L1110}else{aC=aB}}else{aC=p}p=aC-1|0;c[m>>2]=p;c[o>>2]=(c[o>>2]|0)+1;if((w|0)==0){break L1110}}bc(b,-1,az(c[(ay()|0)>>2]|0)|0)}}while(0);L1164:do{if((u&8|0)!=0){w=b+76|0;p=b+40|0;h=b+24|0;g=b+4|0;l=b+84|0;i=c[m>>2]|0;while(1){if((i|0)==0){if((c[w>>2]|0)!=0){break L1164}if((c[p>>2]|0)!=0){break L1164}j=c[h>>2]|0;v=c[f>>2]|0;c[m>>2]=0;k=0;while(1){aD=at(c[g>>2]|0,j+k|0,v-k|0)|0;if((aD|0)<1){q=853;break}r=(c[m>>2]|0)+aD|0;c[m>>2]=r;if(r>>>0<v>>>0){k=r}else{aE=r;break}}if((q|0)==853){q=0;if((aD|0)<0){break}c[p>>2]=1;aE=c[m>>2]|0}c[l>>2]=c[h>>2];if((aE|0)==0){break L1164}else{aF=aE}}else{aF=i}i=aF-1|0;c[m>>2]=i;k=c[o>>2]|0;c[o>>2]=k+1;if((a[k]|0)==0){break L1164}}bc(b,-1,az(c[(ay()|0)>>2]|0)|0)}}while(0);L1182:do{if((u&16|0)!=0){i=b+76|0;h=b+40|0;l=b+24|0;p=b+4|0;g=b+84|0;w=c[m>>2]|0;while(1){if((w|0)==0){if((c[i>>2]|0)!=0){break L1182}if((c[h>>2]|0)!=0){break L1182}k=c[l>>2]|0;v=c[f>>2]|0;c[m>>2]=0;j=0;while(1){aG=at(c[p>>2]|0,k+j|0,v-j|0)|0;if((aG|0)<1){q=866;break}r=(c[m>>2]|0)+aG|0;c[m>>2]=r;if(r>>>0<v>>>0){j=r}else{aH=r;break}}if((q|0)==866){q=0;if((aG|0)<0){break}c[h>>2]=1;aH=c[m>>2]|0}c[g>>2]=c[l>>2];if((aH|0)==0){break L1182}else{aI=aH}}else{aI=w}w=aI-1|0;c[m>>2]=w;j=c[o>>2]|0;c[o>>2]=j+1;if((a[j]|0)==0){break L1182}}bc(b,-1,az(c[(ay()|0)>>2]|0)|0)}}while(0);L1200:do{if((u&2|0)!=0){w=c[m>>2]|0;L1202:do{if((w|0)==0){l=b+76|0;if((c[l>>2]|0)!=0){q=884;break}g=b+40|0;if((c[g>>2]|0)!=0){aJ=l;q=885;break}h=b+24|0;p=c[h>>2]|0;i=c[f>>2]|0;c[m>>2]=0;j=b+4|0;v=0;while(1){aK=at(c[j>>2]|0,p+v|0,i-v|0)|0;if((aK|0)<1){q=878;break}k=(c[m>>2]|0)+aK|0;c[m>>2]=k;if(k>>>0<i>>>0){v=k}else{aL=k;break}}do{if((q|0)==878){if((aK|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);aM=c[m>>2]|0;q=883;break L1202}else{c[g>>2]=1;aL=c[m>>2]|0;break}}}while(0);c[b+84>>2]=c[h>>2];if((aL|0)==0){aJ=l;q=885}else{aN=aL;q=882}}else{aN=w;q=882}}while(0);if((q|0)==882){w=aN-1|0;c[m>>2]=w;c[o>>2]=(c[o>>2]|0)+1;aM=w;q=883}if((q|0)==883){if((aM|0)==0){q=884}else{aO=aM}}if((q|0)==884){aJ=b+76|0;q=885}if((q|0)==885){if((c[aJ>>2]|0)!=0){break}w=b+40|0;if((c[w>>2]|0)!=0){break}g=b+24|0;v=c[g>>2]|0;i=c[f>>2]|0;c[m>>2]=0;p=b+4|0;j=0;while(1){aP=at(c[p>>2]|0,v+j|0,i-j|0)|0;if((aP|0)<1){q=890;break}k=(c[m>>2]|0)+aP|0;c[m>>2]=k;if(k>>>0<i>>>0){j=k}else{aQ=k;break}}do{if((q|0)==890){if((aP|0)<0){bc(b,-1,az(c[(ay()|0)>>2]|0)|0);break L1200}else{c[w>>2]=1;aQ=c[m>>2]|0;break}}}while(0);c[b+84>>2]=c[g>>2];if((aQ|0)==0){break}else{aO=aQ}}c[m>>2]=aO-1;c[o>>2]=(c[o>>2]|0)+1}}while(0);bm(e)|0;c[b+132>>2]=a$(0,0,0)|0;c[b+52>>2]=2;c[b+56>>2]=0;n=0;return n|0}}while(0);bc(b,-3,13176);n=-1;return n|0}}while(0);bc(b,-3,13256);n=-1;return n|0}}while(0);e=b+28|0;a[c[e>>2]|0]=31;c[b+36>>2]=1;aR=c[m>>2]|0;aS=e}else{aR=t;aS=b+28|0}c[b+48>>2]=c[b+12>>2];t=c[aS>>2]|0;c[b+32>>2]=t;if((aR|0)!=0){aS=b+36|0;e=t+(c[aS>>2]|0)|0;t=c[o>>2]|0;bJ(e|0,t|0,aR)|0;c[aS>>2]=(c[aS>>2]|0)+(c[m>>2]|0);c[m>>2]=0}c[b+52>>2]=1;c[b+56>>2]=1;n=0;return n|0}function bg(a){a=a|0;var b=0,d=0;if((a|0)==0){b=-2;return b|0}if((c[a>>2]|0)!=7247){b=-2;return b|0}if((c[a+16>>2]|0)!=0){d=a+84|0;bu(d)|0;bG(c[a+28>>2]|0);bG(c[a+24>>2]|0)}bc(a,0,0);bG(c[a+8>>2]|0);d=an(c[a+4>>2]|0)|0;bG(a);b=((d|0)!=0)<<31>>31;return b|0}function bh(a,b){a=a|0;b=b|0;var e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0;e=a+84|0;f=a+88|0;g=c[f>>2]|0;L1259:do{if((g|0)==0){h=a+76|0;if((c[h>>2]|0)!=0){i=-1;j=934;break}k=a+40|0;if((c[k>>2]|0)!=0){l=-1;m=h;j=935;break}n=a+24|0;o=c[n>>2]|0;p=c[a+16>>2]|0;c[f>>2]=0;q=a+4|0;r=0;while(1){s=at(c[q>>2]|0,o+r|0,p-r|0)|0;if((s|0)<1){j=928;break}t=(c[f>>2]|0)+s|0;c[f>>2]=t;if(t>>>0<p>>>0){r=t}else{u=t;break}}do{if((j|0)==928){if((s|0)<0){bc(a,-1,az(c[(ay()|0)>>2]|0)|0);v=-1;w=c[f>>2]|0;j=933;break L1259}else{c[k>>2]=1;u=c[f>>2]|0;break}}}while(0);c[a+84>>2]=c[n>>2];if((u|0)==0){l=-1;m=h;j=935}else{x=u;j=932}}else{x=g;j=932}}while(0);if((j|0)==932){g=x-1|0;c[f>>2]=g;x=e|0;u=c[x>>2]|0;c[x>>2]=u+1;v=d[u]|0;w=g;j=933}if((j|0)==933){if((w|0)==0){i=v;j=934}else{y=w;z=v;j=946}}if((j|0)==934){l=i;m=a+76|0;j=935}L1278:do{if((j|0)==935){if((c[m>>2]|0)!=0){A=l-256|0;j=948;break}i=a+40|0;if((c[i>>2]|0)==0){v=a+24|0;w=c[v>>2]|0;g=c[a+16>>2]|0;c[f>>2]=0;u=a+4|0;x=0;while(1){B=at(c[u>>2]|0,w+x|0,g-x|0)|0;if((B|0)<1){j=941;break}s=(c[f>>2]|0)+B|0;c[f>>2]=s;if(s>>>0<g>>>0){x=s}else{C=s;break}}do{if((j|0)==941){if((B|0)<0){bc(a,-1,az(c[(ay()|0)>>2]|0)|0);D=-256;E=l;F=c[f>>2]|0;j=947;break L1278}else{c[i>>2]=1;C=c[f>>2]|0;break}}}while(0);c[a+84>>2]=c[v>>2];if((C|0)!=0){y=C;z=l;j=946;break}}G=l-256|0;H=m;j=949}}while(0);if((j|0)==946){m=y-1|0;c[f>>2]=m;y=e|0;l=c[y>>2]|0;c[y>>2]=l+1;D=(d[l]|0)<<8;E=z;F=m;j=947}if((j|0)==947){m=D+E|0;if((F|0)==0){A=m;j=948}else{I=F;J=m;j=960}}if((j|0)==948){G=A;H=a+76|0;j=949}L1301:do{if((j|0)==949){if((c[H>>2]|0)!=0){K=G-65536|0;j=962;break}A=a+40|0;if((c[A>>2]|0)==0){m=a+24|0;F=c[m>>2]|0;E=c[a+16>>2]|0;c[f>>2]=0;D=a+4|0;z=0;while(1){L=at(c[D>>2]|0,F+z|0,E-z|0)|0;if((L|0)<1){j=955;break}l=(c[f>>2]|0)+L|0;c[f>>2]=l;if(l>>>0<E>>>0){z=l}else{M=l;break}}do{if((j|0)==955){if((L|0)<0){bc(a,-1,az(c[(ay()|0)>>2]|0)|0);N=-65536;O=G;P=c[f>>2]|0;j=961;break L1301}else{c[A>>2]=1;M=c[f>>2]|0;break}}}while(0);c[a+84>>2]=c[m>>2];if((M|0)!=0){I=M;J=G;j=960;break}}Q=G-65536|0;R=H;j=963}}while(0);if((j|0)==960){H=I-1|0;c[f>>2]=H;I=e|0;G=c[I>>2]|0;c[I>>2]=G+1;N=(d[G]|0)<<16;O=J;P=H;j=961}if((j|0)==961){H=N+O|0;if((P|0)==0){K=H;j=962}else{S=P;T=H}}if((j|0)==962){Q=K;R=a+76|0;j=963}do{if((j|0)==963){if((c[R>>2]|0)!=0){U=-1;return U|0}K=a+40|0;if((c[K>>2]|0)!=0){U=-1;return U|0}H=a+24|0;P=c[H>>2]|0;O=c[a+16>>2]|0;c[f>>2]=0;N=a+4|0;J=0;while(1){V=at(c[N>>2]|0,P+J|0,O-J|0)|0;if((V|0)<1){j=968;break}G=(c[f>>2]|0)+V|0;c[f>>2]=G;if(G>>>0<O>>>0){J=G}else{W=G;break}}do{if((j|0)==968){if((V|0)>=0){c[K>>2]=1;W=c[f>>2]|0;break}bc(a,-1,az(c[(ay()|0)>>2]|0)|0);U=-1;return U|0}}while(0);c[a+84>>2]=c[H>>2];if((W|0)==0){U=-1}else{S=W;T=Q;break}return U|0}}while(0);c[f>>2]=S-1;S=e|0;e=c[S>>2]|0;c[S>>2]=e+1;c[b>>2]=((d[e]|0)<<24)+T;U=0;return U|0}function bi(a,b,d){a=a|0;b=b|0;d=d|0;var e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0;if((a|0)==0){e=0;return e|0}f=a;g=a+84|0;if((c[a>>2]|0)!=31153){e=0;return e|0}if((c[a+76>>2]|0)!=0){e=0;return e|0}if((d|0)<0){bc(f,-5,13520);e=0;return e|0}if((d|0)==0){e=0;return e|0}h=a+16|0;do{if((c[h>>2]|0)==0){if((bj(f)|0)==-1){e=0}else{break}return e|0}}while(0);i=a+72|0;L1366:do{if((c[i>>2]|0)!=0){c[i>>2]=0;j=c[a+68>>2]|0;k=a+88|0;do{if((c[k>>2]|0)!=0){if((bk(f,0)|0)==-1){e=0}else{break}return e|0}}while(0);l=a+24|0;m=g;n=a+12|0;o=j;p=1;while(1){if((o|0)==0){break L1366}q=c[h>>2]|0;r=(q|0)<0|(q|0)>(o|0)?o:q;if((p|0)!=0){bI(c[l>>2]|0,0,r|0)}c[k>>2]=r;c[m>>2]=c[l>>2];c[n>>2]=(c[n>>2]|0)+r;if((bk(f,0)|0)==-1){e=0;break}else{o=o-r|0;p=0}}return e|0}}while(0);i=a+88|0;L1380:do{if((c[h>>2]|0)>>>0>d>>>0){p=a+24|0;o=g;n=a+12|0;l=b;m=d;while(1){k=c[i>>2]|0;if((k|0)==0){j=c[p>>2]|0;c[o>>2]=j;s=j}else{s=c[o>>2]|0}j=(c[h>>2]|0)-k|0;r=j>>>0>m>>>0?m:j;j=s+k|0;bJ(j|0,l|0,r)|0;c[i>>2]=(c[i>>2]|0)+r;c[n>>2]=(c[n>>2]|0)+r;if((m|0)==(r|0)){break L1380}if((bk(f,0)|0)==-1){e=0;break}l=l+r|0;m=m-r|0}return e|0}else{do{if((c[i>>2]|0)!=0){if((bk(f,0)|0)==-1){e=0}else{break}return e|0}}while(0);c[i>>2]=d;c[g>>2]=b;m=a+12|0;c[m>>2]=(c[m>>2]|0)+d;if((bk(f,0)|0)==-1){e=0}else{break}return e|0}}while(0);e=d;return e|0}function bj(a){a=a|0;var b=0,d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0;b=a+20|0;d=c[b>>2]|0;e=bF(d)|0;f=a+24|0;c[f>>2]=e;g=bF(d)|0;d=a+28|0;c[d>>2]=g;h=(g|0)==0;if(!((e|0)==0|h)){c[a+116>>2]=0;c[a+120>>2]=0;c[a+124>>2]=0;if((a0(a+84|0,c[a+60>>2]|0,8,31,8,c[a+64>>2]|0,13360,56)|0)==0){i=c[b>>2]|0;c[a+16>>2]=i;c[a+100>>2]=i;i=c[d>>2]|0;c[a+96>>2]=i;c[a+32>>2]=i;j=0;return j|0}else{bG(c[f>>2]|0);bc(a,-4,13448);j=-1;return j|0}}if(h){k=e}else{bG(g);k=c[f>>2]|0}if((k|0)!=0){bG(k)}bc(a,-4,13448);j=-1;return j|0}function bk(a,b){a=a|0;b=b|0;var d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0;d=a+84|0;e=a+16|0;do{if((c[e>>2]|0)==0){if((bj(a)|0)==-1){f=-1}else{break}return f|0}}while(0);g=a+100|0;h=a+96|0;i=a+32|0;j=a+28|0;k=a+4|0;L1420:do{if((b|0)==4){l=0;m=c[g>>2]|0;while(1){if((m|0)==0|(l|0)==1){n=c[h>>2]|0;o=c[i>>2]|0;p=n-o|0;if((n|0)==(o|0)){q=m}else{n=as(c[k>>2]|0,o|0,p|0)|0;if(!((n|0)>-1&(n|0)==(p|0))){r=1062;break L1420}q=c[g>>2]|0}if((q|0)==0){p=c[e>>2]|0;c[g>>2]=p;n=c[j>>2]|0;c[h>>2]=n;s=p;t=n}else{s=q;t=c[h>>2]|0}c[i>>2]=t;u=s}else{u=m}n=a3(d,4)|0;if((n|0)==-2){r=1068;break L1420}p=c[g>>2]|0;if((u|0)==(p|0)){r=1070;break}else{l=n;m=p}}}else if((b|0)==0){m=c[g>>2]|0;while(1){if((m|0)==0){l=c[h>>2]|0;p=c[i>>2]|0;n=l-p|0;do{if((l|0)==(p|0)){r=1042}else{o=as(c[k>>2]|0,p|0,n|0)|0;if(!((o|0)>-1&(o|0)==(n|0))){r=1062;break L1420}o=c[g>>2]|0;if((o|0)==0){r=1042;break}v=o;w=c[h>>2]|0}}while(0);if((r|0)==1042){r=0;n=c[e>>2]|0;c[g>>2]=n;p=c[j>>2]|0;c[h>>2]=p;v=n;w=p}c[i>>2]=w;x=v}else{x=m}if((a3(d,0)|0)==-2){r=1068;break L1420}p=c[g>>2]|0;if((x|0)==(p|0)){r=1070;break}else{m=p}}}else{m=c[g>>2]|0;while(1){if((m|0)!=0&(b|0)==0){y=m}else{p=c[h>>2]|0;n=c[i>>2]|0;l=p-n|0;if((p|0)==(n|0)){z=m}else{p=as(c[k>>2]|0,n|0,l|0)|0;if(!((p|0)>-1&(p|0)==(l|0))){r=1062;break L1420}z=c[g>>2]|0}if((z|0)==0){l=c[e>>2]|0;c[g>>2]=l;p=c[j>>2]|0;c[h>>2]=p;A=l;B=p}else{A=z;B=c[h>>2]|0}c[i>>2]=B;y=A}if((a3(d,b)|0)==-2){r=1068;break L1420}p=c[g>>2]|0;if((y|0)==(p|0)){r=1070;break}else{m=p}}}}while(0);if((r|0)==1062){bc(a,-1,az(c[(ay()|0)>>2]|0)|0);f=-1;return f|0}else if((r|0)==1070){if((b|0)!=4){f=0;return f|0}a2(d)|0;f=0;return f|0}else if((r|0)==1068){bc(a,-2,13792);f=-1;return f|0}return 0}function bl(e,f){e=e|0;f=f|0;var g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0,X=0,Y=0,Z=0,_=0,$=0,aa=0,ab=0,ac=0,ad=0,ae=0,af=0,ag=0,ah=0,ai=0,aj=0,ak=0,al=0,am=0,an=0,ao=0,ap=0,aq=0,ar=0,as=0,at=0,au=0,av=0,aw=0,ax=0,ay=0,az=0,aA=0,aB=0,aC=0,aD=0,aE=0,aF=0,aG=0,aH=0,aI=0,aJ=0,aK=0,aL=0,aM=0,aN=0,aO=0,aP=0,aQ=0,aR=0,aS=0,aT=0,aU=0,aV=0,aW=0,aX=0;g=c[e+28>>2]|0;h=g;i=e|0;j=c[i>>2]|0;k=e+4|0;l=j+((c[k>>2]|0)-6)|0;m=e+12|0;n=c[m>>2]|0;o=e+16|0;p=c[o>>2]|0;q=n+(p-258)|0;r=c[h+44>>2]|0;s=c[h+48>>2]|0;t=c[h+52>>2]|0;u=h+56|0;v=h+60|0;w=c[h+76>>2]|0;x=c[h+80>>2]|0;y=(1<<c[h+84>>2])-1|0;z=(1<<c[h+88>>2])-1|0;A=n+(p+~f)|0;f=h+7104|0;p=t-1|0;B=(s|0)==0;C=(c[h+40>>2]|0)-1|0;h=C+s|0;D=s-1|0;E=A-1|0;F=A-s|0;G=j-1|0;j=n-1|0;n=c[u>>2]|0;H=c[v>>2]|0;L1476:while(1){if(H>>>0<15){I=G+2|0;J=I;K=((d[G+1|0]|0)<<H)+n+((d[I]|0)<<H+8)|0;L=H+16|0}else{J=G;K=n;L=H}I=K&y;M=a[w+(I<<2)|0]|0;N=b[w+(I<<2)+2>>1]|0;O=d[w+(I<<2)+1|0]|0;I=K>>>(O>>>0);P=L-O|0;do{if(M<<24>>24==0){Q=N&255;R=I;S=P;T=1083}else{O=N;U=I;V=P;W=M;while(1){X=W&255;if((X&16|0)!=0){break}if((X&64|0)!=0){T=1131;break L1476}Y=(U&(1<<X)-1)+(O&65535)|0;Z=a[w+(Y<<2)|0]|0;_=b[w+(Y<<2)+2>>1]|0;$=d[w+(Y<<2)+1|0]|0;aa=U>>>($>>>0);ab=V-$|0;if(Z<<24>>24==0){T=1082;break}else{O=_;U=aa;V=ab;W=Z}}if((T|0)==1082){T=0;Q=_&255;R=aa;S=ab;T=1083;break}W=O&65535;Z=X&15;if((Z|0)==0){ac=W;ad=J;ae=U;af=V}else{if(V>>>0<Z>>>0){$=J+1|0;ag=$;ah=((d[$]|0)<<V)+U|0;ai=V+8|0}else{ag=J;ah=U;ai=V}ac=(ah&(1<<Z)-1)+W|0;ad=ag;ae=ah>>>(Z>>>0);af=ai-Z|0}if(af>>>0<15){Z=ad+2|0;aj=Z;ak=((d[ad+1|0]|0)<<af)+ae+((d[Z]|0)<<af+8)|0;al=af+16|0}else{aj=ad;ak=ae;al=af}Z=ak&z;W=b[x+(Z<<2)+2>>1]|0;$=d[x+(Z<<2)+1|0]|0;Y=ak>>>($>>>0);am=al-$|0;$=d[x+(Z<<2)|0]|0;if(($&16|0)==0){Z=W;an=Y;ao=am;ap=$;while(1){if((ap&64|0)!=0){T=1128;break L1476}aq=(an&(1<<ap)-1)+(Z&65535)|0;ar=b[x+(aq<<2)+2>>1]|0;as=d[x+(aq<<2)+1|0]|0;at=an>>>(as>>>0);au=ao-as|0;as=d[x+(aq<<2)|0]|0;if((as&16|0)==0){Z=ar;an=at;ao=au;ap=as}else{av=ar;aw=at;ax=au;ay=as;break}}}else{av=W;aw=Y;ax=am;ay=$}ap=av&65535;Z=ay&15;do{if(ax>>>0<Z>>>0){O=aj+1|0;as=((d[O]|0)<<ax)+aw|0;au=ax+8|0;if(au>>>0>=Z>>>0){az=O;aA=as;aB=au;break}O=aj+2|0;az=O;aA=((d[O]|0)<<au)+as|0;aB=ax+16|0}else{az=aj;aA=aw;aB=ax}}while(0);$=(aA&(1<<Z)-1)+ap|0;aC=aA>>>(Z>>>0);aD=aB-Z|0;am=j;Y=am-A|0;if($>>>0<=Y>>>0){W=j+(-$|0)|0;as=ac;au=j;while(1){a[au+1|0]=a[W+1|0]|0;a[au+2|0]=a[W+2|0]|0;O=W+3|0;aE=au+3|0;a[aE]=a[O]|0;aF=as-3|0;if(aF>>>0>2){W=O;as=aF;au=aE}else{break}}if((aF|0)==0){aG=az;aH=aE;aI=aC;aJ=aD;break}as=au+4|0;a[as]=a[W+4|0]|0;if(aF>>>0<=1){aG=az;aH=as;aI=aC;aJ=aD;break}as=au+5|0;a[as]=a[W+5|0]|0;aG=az;aH=as;aI=aC;aJ=aD;break}as=$-Y|0;if(as>>>0>r>>>0){if((c[f>>2]|0)!=0){T=1098;break L1476}}do{if(B){Z=t+(C-as)|0;if(as>>>0>=ac>>>0){aK=Z;aL=ac;aM=j;break}ap=ac-as|0;O=$-am|0;at=Z;Z=as;ar=j;do{at=at+1|0;ar=ar+1|0;a[ar]=a[at]|0;Z=Z-1|0;}while((Z|0)!=0);aK=j+(E+O+(1-$))|0;aL=ap;aM=j+(A+O)|0}else{if(s>>>0>=as>>>0){Z=t+(D-as)|0;if(as>>>0>=ac>>>0){aK=Z;aL=ac;aM=j;break}at=ac-as|0;ar=$-am|0;aq=Z;Z=as;aN=j;do{aq=aq+1|0;aN=aN+1|0;a[aN]=a[aq]|0;Z=Z-1|0;}while((Z|0)!=0);aK=j+(E+ar+(1-$))|0;aL=at;aM=j+(A+ar)|0;break}Z=t+(h-as)|0;aq=as-s|0;if(aq>>>0>=ac>>>0){aK=Z;aL=ac;aM=j;break}aN=ac-aq|0;O=$-am|0;ap=Z;Z=aq;aq=j;do{ap=ap+1|0;aq=aq+1|0;a[aq]=a[ap]|0;Z=Z-1|0;}while((Z|0)!=0);Z=j+(F+O)|0;if(s>>>0>=aN>>>0){aK=p;aL=aN;aM=Z;break}ap=aN-s|0;aq=p;ar=s;at=Z;do{aq=aq+1|0;at=at+1|0;a[at]=a[aq]|0;ar=ar-1|0;}while((ar|0)!=0);aK=j+(E+O+(1-$))|0;aL=ap;aM=j+(A+O)|0}}while(0);if(aL>>>0>2){$=aM;am=aL;as=aK;while(1){a[$+1|0]=a[as+1|0]|0;a[$+2|0]=a[as+2|0]|0;Y=as+3|0;W=$+3|0;a[W]=a[Y]|0;au=am-3|0;if(au>>>0>2){$=W;am=au;as=Y}else{aO=W;aP=au;aQ=Y;break}}}else{aO=aM;aP=aL;aQ=aK}if((aP|0)==0){aG=az;aH=aO;aI=aC;aJ=aD;break}as=aO+1|0;a[as]=a[aQ+1|0]|0;if(aP>>>0<=1){aG=az;aH=as;aI=aC;aJ=aD;break}as=aO+2|0;a[as]=a[aQ+2|0]|0;aG=az;aH=as;aI=aC;aJ=aD}}while(0);if((T|0)==1083){T=0;M=j+1|0;a[M]=Q;aG=J;aH=M;aI=R;aJ=S}if(aG>>>0<l>>>0&aH>>>0<q>>>0){G=aG;j=aH;n=aI;H=aJ}else{aR=aG;aS=aH;aT=aI;aU=aJ;break}}do{if((T|0)==1128){c[e+24>>2]=13712;c[g>>2]=29;aR=aj;aS=j;aT=an;aU=ao}else if((T|0)==1131){if((X&32|0)==0){c[e+24>>2]=13416;c[g>>2]=29;aR=J;aS=j;aT=U;aU=V;break}else{c[g>>2]=11;aR=J;aS=j;aT=U;aU=V;break}}else if((T|0)==1098){c[e+24>>2]=13304;c[g>>2]=29;aR=az;aS=j;aT=aC;aU=aD}}while(0);aD=aU>>>3;aC=aR+(-aD|0)|0;j=aU-(aD<<3)|0;aU=(1<<j)-1&aT;c[i>>2]=aR+(1-aD);c[m>>2]=aS+1;if(aC>>>0<l>>>0){aV=l-aC|0}else{aV=l-aC|0}c[k>>2]=aV+5;if(aS>>>0<q>>>0){aW=q-aS|0;aX=aW+257|0;c[o>>2]=aX;c[u>>2]=aU;c[v>>2]=j;return}else{aW=q-aS|0;aX=aW+257|0;c[o>>2]=aX;c[u>>2]=aU;c[v>>2]=j;return}}function bm(a){a=a|0;var b=0,d=0,e=0;if((a|0)==0){b=-2;return b|0}d=c[a+28>>2]|0;if((d|0)==0){b=-2;return b|0}e=d;c[e+28>>2]=0;c[a+20>>2]=0;c[a+8>>2]=0;c[a+24>>2]=0;c[a+48>>2]=1;c[d>>2]=0;c[e+4>>2]=0;c[e+12>>2]=0;c[e+20>>2]=32768;c[e+32>>2]=0;c[e+40>>2]=0;c[e+44>>2]=0;c[e+48>>2]=0;c[e+56>>2]=0;c[e+60>>2]=0;d=e+1328|0;c[e+108>>2]=d;c[e+80>>2]=d;c[e+76>>2]=d;c[e+7104>>2]=1;c[e+7108>>2]=-1;b=0;return b|0}function bn(a){a=a|0;var b=0,d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0;if((a|0)==0){b=-2;return b|0}d=a;if((c[a>>2]|0)!=31153){b=-2;return b|0}e=a+72|0;L1582:do{if((c[e>>2]|0)==0){f=0}else{c[e>>2]=0;g=c[a+68>>2]|0;h=a+88|0;if((c[h>>2]|0)!=0){if((bk(d,0)|0)==-1){f=-1;break}}i=a+16|0;j=a+24|0;k=a+84|0;l=a+12|0;m=g;g=1;while(1){if((m|0)==0){f=0;break L1582}n=c[i>>2]|0;o=(n|0)<0|(n|0)>(m|0)?m:n;if((g|0)!=0){bI(c[j>>2]|0,0,o|0)}c[h>>2]=o;c[k>>2]=c[j>>2];c[l>>2]=(c[l>>2]|0)+o;if((bk(d,0)|0)==-1){f=-1;break}else{m=m-o|0;g=0}}}}while(0);e=(bk(d,4)|0)+f|0;a1(a+84|0)|0;bG(c[a+28>>2]|0);bG(c[a+24>>2]|0);bc(d,0,0);bG(c[a+8>>2]|0);d=an(c[a+4>>2]|0)|0;bG(a);b=((e|0)!=(-d|0))<<31>>31;return b|0}function bo(a,b){a=a|0;b=b|0;var d=0,e=0,f=0,g=0,h=0,i=0,j=0;if((a|0)==0){d=-2;return d|0}e=a+28|0;f=c[e>>2]|0;if((f|0)==0){d=-2;return d|0}g=f;if((b|0)<0){h=0;i=-b|0}else{h=(b>>4)+1|0;i=(b|0)<48?b&15:b}if((i|0)!=0&(i-8|0)>>>0>7){d=-2;return d|0}b=g+52|0;f=c[b>>2]|0;j=g+36|0;do{if((f|0)!=0){if((c[j>>2]|0)==(i|0)){break}aF[c[a+36>>2]&3](c[a+40>>2]|0,f);c[b>>2]=0}}while(0);c[g+8>>2]=h;c[j>>2]=i;i=c[e>>2]|0;if((i|0)==0){d=-2;return d|0}e=i;c[e+28>>2]=0;c[a+20>>2]=0;c[a+8>>2]=0;c[a+24>>2]=0;c[a+48>>2]=1;c[i>>2]=0;c[e+4>>2]=0;c[e+12>>2]=0;c[e+20>>2]=32768;c[e+32>>2]=0;c[e+40>>2]=0;c[e+44>>2]=0;c[e+48>>2]=0;c[e+56>>2]=0;c[e+60>>2]=0;i=e+1328|0;c[e+108>>2]=i;c[e+80>>2]=i;c[e+76>>2]=i;c[e+7104>>2]=1;c[e+7108>>2]=-1;d=0;return d|0}function bp(b,d,e,f){b=b|0;d=d|0;e=e|0;f=f|0;var g=0,h=0,i=0,j=0;if((e|0)==0){g=-6;return g|0}if(!((a[e]|0)==49&(f|0)==56)){g=-6;return g|0}if((b|0)==0){g=-2;return g|0}c[b+24>>2]=0;f=b+32|0;e=c[f>>2]|0;if((e|0)==0){c[f>>2]=6;c[b+40>>2]=0;h=6}else{h=e}e=b+36|0;if((c[e>>2]|0)==0){c[e>>2]=2}f=b+40|0;i=aG[h&7](c[f>>2]|0,1,7116)|0;if((i|0)==0){g=-4;return g|0}h=b+28|0;c[h>>2]=i;c[i+52>>2]=0;j=bo(b,d)|0;if((j|0)==0){g=0;return g|0}aF[c[e>>2]&3](c[f>>2]|0,i);c[h>>2]=0;g=j;return g|0}function bq(f,g){f=f|0;g=g|0;var h=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0,X=0,Y=0,Z=0,_=0,$=0,aa=0,ab=0,ac=0,ad=0,ae=0,af=0,ag=0,ah=0,ai=0,aj=0,ak=0,al=0,am=0,an=0,ao=0,ap=0,aq=0,ar=0,as=0,at=0,au=0,av=0,aw=0,ax=0,ay=0,az=0,aA=0,aC=0,aD=0,aE=0,aF=0,aG=0,aH=0,aI=0,aJ=0,aK=0,aL=0,aM=0,aN=0,aO=0,aP=0,aQ=0,aR=0,aS=0,aT=0,aU=0,aV=0,aW=0,aX=0,aY=0,aZ=0,a0=0,a1=0,a2=0,a3=0,a4=0,a5=0,a6=0,a7=0,a8=0,a9=0,ba=0,bb=0,bc=0,bd=0,be=0,bf=0,bg=0,bh=0,bi=0,bj=0,bk=0,bm=0,bn=0,bo=0,bp=0,bq=0,br=0,bs=0,bu=0,bw=0,bx=0,by=0,bz=0,bA=0,bB=0,bC=0,bD=0,bE=0,bF=0,bG=0,bH=0,bI=0,bK=0,bL=0,bM=0,bN=0,bO=0,bP=0,bQ=0,bR=0,bS=0,bT=0,bU=0,bV=0,bW=0,bX=0,bY=0,bZ=0,b_=0,b$=0,b0=0,b1=0,b2=0,b3=0,b4=0,b5=0,b6=0,b7=0,b8=0,b9=0,ca=0,cb=0,cc=0,cd=0,ce=0,cf=0,cg=0,ch=0,ci=0,cj=0,ck=0,cl=0,cm=0,cn=0,co=0,cp=0,cq=0,cr=0,cs=0,ct=0,cu=0,cv=0,cw=0,cx=0,cy=0,cz=0,cA=0,cB=0,cC=0,cD=0,cE=0,cF=0,cG=0,cH=0,cI=0,cJ=0,cK=0,cL=0,cM=0,cN=0,cO=0,cP=0,cQ=0,cR=0,cS=0,cT=0,cU=0,cV=0,cW=0,cX=0,cY=0,cZ=0,c_=0,c$=0,c0=0,c1=0,c2=0,c3=0,c4=0,c5=0,c6=0,c7=0,c8=0,c9=0,da=0,db=0,dc=0,dd=0,de=0,df=0,dg=0,dh=0,di=0,dj=0,dk=0,dl=0,dm=0,dn=0,dp=0,dq=0,dr=0,ds=0,dt=0,du=0,dv=0,dw=0,dx=0,dy=0,dz=0,dA=0,dB=0,dC=0,dD=0,dE=0,dF=0,dG=0,dH=0,dI=0,dJ=0,dK=0,dL=0,dM=0,dN=0,dO=0,dP=0,dQ=0,dR=0,dS=0,dT=0,dU=0,dV=0,dW=0,dX=0,dY=0,dZ=0,d_=0,d$=0,d0=0,d1=0,d2=0,d3=0,d4=0,d5=0,d6=0,d7=0,d8=0,d9=0,ea=0,eb=0,ec=0,ed=0,ee=0,ef=0,eg=0,eh=0,ei=0,ej=0,ek=0,el=0,em=0,en=0,eo=0,ep=0,eq=0,er=0,es=0,et=0,eu=0,ev=0,ew=0,ex=0,ey=0,ez=0,eA=0,eB=0,eC=0,eD=0,eE=0,eF=0,eG=0,eH=0,eI=0,eJ=0,eK=0,eL=0,eM=0,eN=0,eO=0,eP=0,eQ=0,eR=0,eS=0,eT=0,eU=0,eV=0,eW=0,eX=0,eY=0,eZ=0,e_=0,e$=0,e0=0,e1=0,e2=0,e3=0,e4=0,e5=0,e6=0,e7=0,e8=0,e9=0,fa=0,fb=0,fc=0,fd=0,fe=0,ff=0,fg=0,fh=0,fi=0,fj=0,fk=0,fl=0,fm=0,fn=0,fo=0,fp=0,fq=0,fr=0,fs=0,ft=0,fu=0,fv=0,fw=0,fx=0,fy=0,fz=0,fA=0,fB=0,fC=0,fD=0,fE=0,fF=0,fG=0,fH=0,fI=0,fJ=0,fK=0,fL=0,fM=0,fN=0,fO=0,fP=0,fQ=0,fR=0;h=i;i=i+8|0;j=h|0;if((f|0)==0){k=-2;i=h;return k|0}l=c[f+28>>2]|0;if((l|0)==0){k=-2;i=h;return k|0}m=f+12|0;n=c[m>>2]|0;if((n|0)==0){k=-2;i=h;return k|0}o=f|0;p=c[o>>2]|0;do{if((p|0)==0){if((c[f+4>>2]|0)==0){break}else{k=-2}i=h;return k|0}}while(0);q=l;r=l;l=c[r>>2]|0;if((l|0)==11){c[r>>2]=12;s=c[m>>2]|0;t=c[o>>2]|0;u=12}else{s=n;t=p;u=l}l=f+16|0;p=c[l>>2]|0;n=f+4|0;v=c[n>>2]|0;w=q+56|0;x=q+60|0;y=q+8|0;z=q+24|0;A=j|0;B=j+1|0;C=q+16|0;D=q+32|0;E=f+24|0;F=q+36|0;G=q+20|0;H=f+48|0;I=q+64|0;J=q+12|0;K=(g-5|0)>>>0<2;L=q+4|0;M=q+76|0;N=q+84|0;O=q+80|0;P=q+88|0;Q=(g|0)==6;R=q+7108|0;S=q+72|0;T=q+7112|0;U=q+68|0;V=q+44|0;W=q+7104|0;X=q+48|0;Y=q+52|0;Z=q+40|0;_=f+20|0;$=q+28|0;aa=q+96|0;ab=q+100|0;ac=q+92|0;ad=q+104|0;ae=q+1328|0;af=q+108|0;ag=q+112|0;ah=q+752|0;ai=q+624|0;aj=j+2|0;ak=j+3|0;j=0;al=p;am=c[x>>2]|0;an=c[w>>2]|0;ao=p;p=v;ap=s;s=t;t=u;L1657:while(1){L1659:do{if((t|0)==2){if(am>>>0<32){aq=s;ar=p;as=an;at=am;au=1246}else{av=s;aw=p;ax=an;au=1248}}else if((t|0)==7){ay=am;az=an;aA=p;aC=s;au=1295}else if((t|0)==9){if(am>>>0<32){u=s;aD=p;aE=an;aF=am;while(1){if((aD|0)==0){aG=j;aH=al;aI=aF;aJ=aE;aK=0;aL=u;aM=ao;break L1657}aN=aD-1|0;aO=u+1|0;aP=(d[u]<<aF)+aE|0;aQ=aF+8|0;if(aQ>>>0<32){u=aO;aD=aN;aE=aP;aF=aQ}else{aR=aO;aS=aN;aT=aP;break}}}else{aR=s;aS=p;aT=an}aF=aB(aT|0)|0;c[z>>2]=aF;c[H>>2]=aF;c[r>>2]=10;aU=0;aV=0;aW=aS;aX=aR;au=1320}else if((t|0)==1){if(am>>>0<16){aF=s;aE=p;aD=an;u=am;while(1){if((aE|0)==0){aG=j;aH=al;aI=u;aJ=aD;aK=0;aL=aF;aM=ao;break L1657}aP=aE-1|0;aN=aF+1|0;aO=(d[aF]<<u)+aD|0;aQ=u+8|0;if(aQ>>>0<16){aF=aN;aE=aP;aD=aO;u=aQ}else{aY=aN;aZ=aP;a0=aO;a1=aQ;break}}}else{aY=s;aZ=p;a0=an;a1=am}c[C>>2]=a0;if((a0&255|0)!=8){c[E>>2]=13384;c[r>>2]=29;a2=j;a3=al;a4=a1;a5=a0;a6=ao;a7=aZ;a8=ap;a9=aY;break}if((a0&57344|0)!=0){c[E>>2]=13224;c[r>>2]=29;a2=j;a3=al;a4=a1;a5=a0;a6=ao;a7=aZ;a8=ap;a9=aY;break}u=c[D>>2]|0;if((u|0)==0){ba=a0}else{c[u>>2]=a0>>>8&1;ba=c[C>>2]|0}if((ba&512|0)!=0){a[A]=a0&255;a[B]=a0>>>8&255;c[z>>2]=a$(c[z>>2]|0,A,2)|0}c[r>>2]=2;aq=aY;ar=aZ;as=0;at=0;au=1246}else if((t|0)==5){bb=am;bc=an;bd=p;be=s;au=1272}else if((t|0)==21){bf=j;bg=am;bh=an;bi=p;bj=s;bk=c[S>>2]|0;au=1419}else if((t|0)==17){u=c[ad>>2]|0;if(u>>>0<(c[ac>>2]|0)>>>0){bm=s;bn=p;bo=an;bp=am;bq=u;au=1353}else{br=s;bs=p;bu=an;bw=am;bx=u;au=1357}}else if((t|0)==18){by=j;bz=am;bA=an;bB=p;bC=s;bD=c[ad>>2]|0;au=1362}else if((t|0)==12){bE=am;bF=an;bG=p;bH=s;au=1324}else if((t|0)==25){if((ao|0)==0){bI=j;bK=am;bL=an;bM=p;bN=s;bO=0;au=1483;break L1657}a[ap]=c[I>>2]&255;c[r>>2]=20;a2=j;a3=al;a4=am;a5=an;a6=ao-1|0;a7=p;a8=ap+1|0;a9=s}else if((t|0)==26){do{if((c[y>>2]|0)==0){bP=al;bQ=am;bR=an;bS=p;bT=s}else{if(am>>>0<32){u=s;aD=p;aE=an;aF=am;while(1){if((aD|0)==0){aG=j;aH=al;aI=aF;aJ=aE;aK=0;aL=u;aM=ao;break L1657}aQ=aD-1|0;aO=u+1|0;aP=(d[u]<<aF)+aE|0;aN=aF+8|0;if(aN>>>0<32){u=aO;aD=aQ;aE=aP;aF=aN}else{bU=aO;bV=aQ;bW=aP;bX=aN;break}}}else{bU=s;bV=p;bW=an;bX=am}aF=al-ao|0;c[_>>2]=(c[_>>2]|0)+aF;c[$>>2]=(c[$>>2]|0)+aF;if((al|0)!=(ao|0)){aE=c[z>>2]|0;aD=ap+(-aF|0)|0;if((c[C>>2]|0)==0){bY=a_(aE,aD,aF)|0}else{bY=a$(aE,aD,aF)|0}c[z>>2]=bY;c[H>>2]=bY}if((c[C>>2]|0)==0){bZ=aB(bW|0)|0}else{bZ=bW}if((bZ|0)==(c[z>>2]|0)){bP=ao;bQ=0;bR=0;bS=bV;bT=bU;break}c[E>>2]=13576;c[r>>2]=29;a2=j;a3=ao;a4=bX;a5=bW;a6=ao;a7=bV;a8=ap;a9=bU;break L1659}}while(0);c[r>>2]=27;b_=bP;b$=bQ;b0=bR;b1=bS;b2=bT;au=1475}else if((t|0)==27){b_=al;b$=am;b0=an;b1=p;b2=s;au=1475}else if((t|0)==28){bI=1;bK=am;bL=an;bM=p;bN=s;bO=ao;au=1483;break L1657}else if((t|0)==29){aG=-3;aH=al;aI=am;aJ=an;aK=p;aL=s;aM=ao;break L1657}else if((t|0)==0){aF=c[y>>2]|0;if((aF|0)==0){c[r>>2]=12;a2=j;a3=al;a4=am;a5=an;a6=ao;a7=p;a8=ap;a9=s;break}if(am>>>0<16){aD=s;aE=p;u=an;aN=am;while(1){if((aE|0)==0){aG=j;aH=al;aI=aN;aJ=u;aK=0;aL=aD;aM=ao;break L1657}aP=aE-1|0;aQ=aD+1|0;aO=(d[aD]<<aN)+u|0;b3=aN+8|0;if(b3>>>0<16){aD=aQ;aE=aP;u=aO;aN=b3}else{b4=aQ;b5=aP;b6=aO;b7=b3;break}}}else{b4=s;b5=p;b6=an;b7=am}if((aF&2|0)!=0&(b6|0)==35615){c[z>>2]=a$(0,0,0)|0;a[A]=31;a[B]=-117;c[z>>2]=a$(c[z>>2]|0,A,2)|0;c[r>>2]=1;a2=j;a3=al;a4=0;a5=0;a6=ao;a7=b5;a8=ap;a9=b4;break}c[C>>2]=0;aN=c[D>>2]|0;if((aN|0)==0){b8=aF}else{c[aN+48>>2]=-1;b8=c[y>>2]|0}do{if((b8&1|0)!=0){if(((((b6<<8&65280)+(b6>>>8)|0)>>>0)%31|0|0)!=0){break}if((b6&15|0)!=8){c[E>>2]=13384;c[r>>2]=29;a2=j;a3=al;a4=b7;a5=b6;a6=ao;a7=b5;a8=ap;a9=b4;break L1659}aN=b6>>>4;u=b7-4|0;aE=(aN&15)+8|0;aD=c[F>>2]|0;do{if((aD|0)==0){c[F>>2]=aE}else{if(aE>>>0<=aD>>>0){break}c[E>>2]=13336;c[r>>2]=29;a2=j;a3=al;a4=u;a5=aN;a6=ao;a7=b5;a8=ap;a9=b4;break L1659}}while(0);c[G>>2]=1<<aE;aN=a_(0,0,0)|0;c[z>>2]=aN;c[H>>2]=aN;c[r>>2]=b6>>>12&2^11;a2=j;a3=al;a4=0;a5=0;a6=ao;a7=b5;a8=ap;a9=b4;break L1659}}while(0);c[E>>2]=13656;c[r>>2]=29;a2=j;a3=al;a4=b7;a5=b6;a6=ao;a7=b5;a8=ap;a9=b4}else if((t|0)==16){if(am>>>0<14){aF=s;aN=p;u=an;aD=am;while(1){if((aN|0)==0){aG=j;aH=al;aI=aD;aJ=u;aK=0;aL=aF;aM=ao;break L1657}b3=aN-1|0;aO=aF+1|0;aP=(d[aF]<<aD)+u|0;aQ=aD+8|0;if(aQ>>>0<14){aF=aO;aN=b3;u=aP;aD=aQ}else{b9=aO;ca=b3;cb=aP;cc=aQ;break}}}else{b9=s;ca=p;cb=an;cc=am}aD=(cb&31)+257|0;c[aa>>2]=aD;u=(cb>>>5&31)+1|0;c[ab>>2]=u;c[ac>>2]=(cb>>>10&15)+4;aN=cb>>>14;aF=cc-14|0;if(aD>>>0>286|u>>>0>30){c[E>>2]=12896;c[r>>2]=29;a2=j;a3=al;a4=aF;a5=aN;a6=ao;a7=ca;a8=ap;a9=b9;break}else{c[ad>>2]=0;c[r>>2]=17;bm=b9;bn=ca;bo=aN;bp=aF;bq=0;au=1353;break}}else if((t|0)==3){if(am>>>0<16){cd=s;ce=p;cf=an;cg=am;au=1254}else{ch=s;ci=p;cj=an;au=1256}}else if((t|0)==8){ck=am;cl=an;cm=p;cn=s;au=1308}else if((t|0)==23){co=j;cp=am;cq=an;cr=p;cs=s;ct=c[S>>2]|0;au=1438}else if((t|0)==4){cu=am;cv=an;cw=p;cx=s;au=1261}else if((t|0)==30){au=1497;break L1657}else if((t|0)==22){cy=j;cz=am;cA=an;cB=p;cC=s;au=1426}else if((t|0)==24){cD=j;cE=am;cF=an;cG=p;cH=s;au=1444}else if((t|0)==14){cI=am;cJ=an;cK=p;cL=s;au=1342}else if((t|0)==15){cM=am;cN=an;cO=p;cP=s;au=1343}else if((t|0)==19){cQ=j;cR=am;cS=an;cT=p;cU=s;au=1399}else if((t|0)==20){cV=j;cW=am;cX=an;cY=p;cZ=s;au=1400}else if((t|0)==13){aF=am&7;aN=an>>>(aF>>>0);u=am-aF|0;if(u>>>0<32){aF=s;aD=p;aQ=aN;aP=u;while(1){if((aD|0)==0){aG=j;aH=al;aI=aP;aJ=aQ;aK=0;aL=aF;aM=ao;break L1657}b3=aD-1|0;aO=aF+1|0;c_=(d[aF]<<aP)+aQ|0;c$=aP+8|0;if(c$>>>0<32){aF=aO;aD=b3;aQ=c_;aP=c$}else{c0=aO;c1=b3;c2=c_;c3=c$;break}}}else{c0=s;c1=p;c2=aN;c3=u}aP=c2&65535;if((aP|0)==(c2>>>16^65535|0)){c[I>>2]=aP;c[r>>2]=14;if(Q){bI=j;bK=0;bL=0;bM=c1;bN=c0;bO=ao;au=1483;break L1657}else{cI=0;cJ=0;cK=c1;cL=c0;au=1342;break}}else{c[E>>2]=12992;c[r>>2]=29;a2=j;a3=al;a4=c3;a5=c2;a6=ao;a7=c1;a8=ap;a9=c0;break}}else if((t|0)==10){aU=am;aV=an;aW=p;aX=s;au=1320}else if((t|0)==11){c4=am;c5=an;c6=p;c7=s;au=1323}else if((t|0)==6){c8=am;c9=an;da=p;db=s;au=1282}else{k=-2;au=1500;break L1657}}while(0);if((au|0)==1246){while(1){au=0;if((ar|0)==0){aG=j;aH=al;aI=at;aJ=as;aK=0;aL=aq;aM=ao;break L1657}aP=ar-1|0;aQ=aq+1|0;aD=(d[aq]<<at)+as|0;aF=at+8|0;if(aF>>>0<32){aq=aQ;ar=aP;as=aD;at=aF;au=1246}else{av=aQ;aw=aP;ax=aD;au=1248;break}}}else if((au|0)==1353){while(1){au=0;if(bp>>>0<3){aD=bm;aP=bn;aQ=bo;aF=bp;while(1){if((aP|0)==0){aG=j;aH=al;aI=aF;aJ=aQ;aK=0;aL=aD;aM=ao;break L1657}c$=aP-1|0;c_=aD+1|0;b3=(d[aD]<<aF)+aQ|0;aO=aF+8|0;if(aO>>>0<3){aD=c_;aP=c$;aQ=b3;aF=aO}else{dc=c_;dd=c$;de=b3;df=aO;break}}}else{dc=bm;dd=bn;de=bo;df=bp}c[ad>>2]=bq+1;b[q+112+(e[1664+(bq<<1)>>1]<<1)>>1]=de&7;aF=de>>>3;aQ=df-3|0;aP=c[ad>>2]|0;if(aP>>>0<(c[ac>>2]|0)>>>0){bm=dc;bn=dd;bo=aF;bp=aQ;bq=aP;au=1353}else{br=dc;bs=dd;bu=aF;bw=aQ;bx=aP;au=1357;break}}}else if((au|0)==1475){au=0;if((c[y>>2]|0)==0){dg=b$;dh=b0;di=b1;dj=b2;au=1482;break}if((c[C>>2]|0)==0){dg=b$;dh=b0;di=b1;dj=b2;au=1482;break}if(b$>>>0<32){aP=b2;aQ=b1;aF=b0;aD=b$;while(1){if((aQ|0)==0){aG=j;aH=b_;aI=aD;aJ=aF;aK=0;aL=aP;aM=ao;break L1657}u=aQ-1|0;aN=aP+1|0;aO=(d[aP]<<aD)+aF|0;b3=aD+8|0;if(b3>>>0<32){aP=aN;aQ=u;aF=aO;aD=b3}else{dk=aN;dl=u;dm=aO;dn=b3;break}}}else{dk=b2;dl=b1;dm=b0;dn=b$}if((dm|0)==(c[$>>2]|0)){dg=0;dh=0;di=dl;dj=dk;au=1482;break}c[E>>2]=13496;c[r>>2]=29;a2=j;a3=b_;a4=dn;a5=dm;a6=ao;a7=dl;a8=ap;a9=dk}else if((au|0)==1342){au=0;c[r>>2]=15;cM=cI;cN=cJ;cO=cK;cP=cL;au=1343}else if((au|0)==1320){au=0;if((c[J>>2]|0)==0){au=1321;break}aD=a_(0,0,0)|0;c[z>>2]=aD;c[H>>2]=aD;c[r>>2]=11;c4=aU;c5=aV;c6=aW;c7=aX;au=1323}do{if((au|0)==1357){au=0;if(bx>>>0<19){aD=bx;do{c[ad>>2]=aD+1;b[q+112+(e[1664+(aD<<1)>>1]<<1)>>1]=0;aD=c[ad>>2]|0;}while(aD>>>0<19)}c[af>>2]=ae;c[M>>2]=ae;c[N>>2]=7;aD=bv(0,ag,19,af,N,ah)|0;if((aD|0)==0){c[ad>>2]=0;c[r>>2]=18;by=0;bz=bw;bA=bu;bB=bs;bC=br;bD=0;au=1362;break}else{c[E>>2]=12776;c[r>>2]=29;a2=aD;a3=al;a4=bw;a5=bu;a6=ao;a7=bs;a8=ap;a9=br;break}}else if((au|0)==1343){au=0;aD=c[I>>2]|0;if((aD|0)==0){c[r>>2]=11;a2=j;a3=al;a4=cM;a5=cN;a6=ao;a7=cO;a8=ap;a9=cP;break}aF=aD>>>0>cO>>>0?cO:aD;aD=aF>>>0>ao>>>0?ao:aF;if((aD|0)==0){bI=j;bK=cM;bL=cN;bM=cO;bN=cP;bO=ao;au=1483;break L1657}bJ(ap|0,cP|0,aD)|0;c[I>>2]=(c[I>>2]|0)-aD;a2=j;a3=al;a4=cM;a5=cN;a6=ao-aD|0;a7=cO-aD|0;a8=ap+aD|0;a9=cP+aD|0}else if((au|0)==1323){au=0;if(K){bI=j;bK=c4;bL=c5;bM=c6;bN=c7;bO=ao;au=1483;break L1657}else{bE=c4;bF=c5;bG=c6;bH=c7;au=1324}}else if((au|0)==1248){au=0;aD=c[D>>2]|0;if((aD|0)!=0){c[aD+4>>2]=ax}if((c[C>>2]&512|0)!=0){a[A]=ax&255;a[B]=ax>>>8&255;a[aj]=ax>>>16&255;a[ak]=ax>>>24&255;c[z>>2]=a$(c[z>>2]|0,A,4)|0}c[r>>2]=3;cd=av;ce=aw;cf=0;cg=0;au=1254}}while(0);L1802:do{if((au|0)==1324){au=0;if((c[L>>2]|0)!=0){aD=bE&7;c[r>>2]=26;a2=j;a3=al;a4=bE-aD|0;a5=bF>>>(aD>>>0);a6=ao;a7=bG;a8=ap;a9=bH;break}if(bE>>>0<3){aD=bH;aF=bG;aQ=bF;aP=bE;while(1){if((aF|0)==0){aG=j;aH=al;aI=aP;aJ=aQ;aK=0;aL=aD;aM=ao;break L1657}b3=aF-1|0;aO=aD+1|0;u=(d[aD]<<aP)+aQ|0;aN=aP+8|0;if(aN>>>0<3){aD=aO;aF=b3;aQ=u;aP=aN}else{dp=aO;dq=b3;dr=u;ds=aN;break}}}else{dp=bH;dq=bG;dr=bF;ds=bE}c[L>>2]=dr&1;aP=dr>>>1&3;if((aP|0)==0){c[r>>2]=13}else if((aP|0)==1){c[M>>2]=1704;c[N>>2]=9;c[O>>2]=3752;c[P>>2]=5;c[r>>2]=19;if(Q){au=1332;break L1657}}else if((aP|0)==2){c[r>>2]=16}else if((aP|0)==3){c[E>>2]=13088;c[r>>2]=29}a2=j;a3=al;a4=ds-3|0;a5=dr>>>3;a6=ao;a7=dq;a8=ap;a9=dp}else if((au|0)==1362){au=0;aP=c[aa>>2]|0;aQ=c[ab>>2]|0;do{if(bD>>>0<(aQ+aP|0)>>>0){aF=bC;aD=bB;aN=bA;u=bz;b3=bD;aO=aP;c$=aQ;L1820:while(1){c_=(1<<c[N>>2])-1|0;dt=c_&aN;du=c[M>>2]|0;dv=d[du+(dt<<2)+1|0]|0;if(dv>>>0>u>>>0){dw=aF;dx=aD;dy=aN;dz=u;while(1){if((dx|0)==0){aG=by;aH=al;aI=dz;aJ=dy;aK=0;aL=dw;aM=ao;break L1657}dA=dx-1|0;dB=dw+1|0;dC=(d[dw]<<dz)+dy|0;dD=dz+8|0;dE=c_&dC;dF=d[du+(dE<<2)+1|0]|0;if(dF>>>0>dD>>>0){dw=dB;dx=dA;dy=dC;dz=dD}else{dG=dB;dH=dA;dI=dC;dJ=dD;dK=dE;dL=dF;break}}}else{dG=aF;dH=aD;dI=aN;dJ=u;dK=dt;dL=dv}dz=b[du+(dK<<2)+2>>1]|0;L1827:do{if((dz&65535)<16){if(dJ>>>0<dL>>>0){dy=dG;dx=dH;dw=dI;c_=dJ;while(1){if((dx|0)==0){aG=by;aH=al;aI=c_;aJ=dw;aK=0;aL=dy;aM=ao;break L1657}dF=dx-1|0;dE=dy+1|0;dD=(d[dy]<<c_)+dw|0;dC=c_+8|0;if(dC>>>0<dL>>>0){dy=dE;dx=dF;dw=dD;c_=dC}else{dM=dE;dN=dF;dO=dD;dP=dC;break}}}else{dM=dG;dN=dH;dO=dI;dP=dJ}c[ad>>2]=b3+1;b[q+112+(b3<<1)>>1]=dz;dQ=dP-dL|0;dR=dO>>>(dL>>>0);dS=dN;dT=dM}else{if((dz<<16>>16|0)==16){c_=dL+2|0;if(dJ>>>0<c_>>>0){dw=dG;dx=dH;dy=dI;dC=dJ;while(1){if((dx|0)==0){aG=by;aH=al;aI=dC;aJ=dy;aK=0;aL=dw;aM=ao;break L1657}dD=dx-1|0;dF=dw+1|0;dE=(d[dw]<<dC)+dy|0;dA=dC+8|0;if(dA>>>0<c_>>>0){dw=dF;dx=dD;dy=dE;dC=dA}else{dU=dF;dV=dD;dW=dE;dX=dA;break}}}else{dU=dG;dV=dH;dW=dI;dX=dJ}dY=dW>>>(dL>>>0);dZ=dX-dL|0;if((b3|0)==0){au=1379;break L1820}d_=b[q+112+(b3-1<<1)>>1]|0;d$=(dY&3)+3|0;d0=dZ-2|0;d1=dY>>>2;d2=dV;d3=dU}else if((dz<<16>>16|0)==17){dC=dL+3|0;if(dJ>>>0<dC>>>0){dy=dG;dx=dH;dw=dI;c_=dJ;while(1){if((dx|0)==0){aG=by;aH=al;aI=c_;aJ=dw;aK=0;aL=dy;aM=ao;break L1657}dA=dx-1|0;dE=dy+1|0;dD=(d[dy]<<c_)+dw|0;dF=c_+8|0;if(dF>>>0<dC>>>0){dy=dE;dx=dA;dw=dD;c_=dF}else{d4=dE;d5=dA;d6=dD;d7=dF;break}}}else{d4=dG;d5=dH;d6=dI;d7=dJ}c_=d6>>>(dL>>>0);d_=0;d$=(c_&7)+3|0;d0=-3-dL+d7|0;d1=c_>>>3;d2=d5;d3=d4}else{c_=dL+7|0;if(dJ>>>0<c_>>>0){dw=dG;dx=dH;dy=dI;dC=dJ;while(1){if((dx|0)==0){aG=by;aH=al;aI=dC;aJ=dy;aK=0;aL=dw;aM=ao;break L1657}dF=dx-1|0;dD=dw+1|0;dA=(d[dw]<<dC)+dy|0;dE=dC+8|0;if(dE>>>0<c_>>>0){dw=dD;dx=dF;dy=dA;dC=dE}else{d8=dD;d9=dF;ea=dA;eb=dE;break}}}else{d8=dG;d9=dH;ea=dI;eb=dJ}dC=ea>>>(dL>>>0);d_=0;d$=(dC&127)+11|0;d0=-7-dL+eb|0;d1=dC>>>7;d2=d9;d3=d8}if((b3+d$|0)>>>0>(c$+aO|0)>>>0){au=1388;break L1820}else{ec=d$;ed=b3}while(1){dC=ec-1|0;c[ad>>2]=ed+1;b[q+112+(ed<<1)>>1]=d_;if((dC|0)==0){dQ=d0;dR=d1;dS=d2;dT=d3;break L1827}ec=dC;ed=c[ad>>2]|0}}}while(0);dz=c[ad>>2]|0;ee=c[aa>>2]|0;du=c[ab>>2]|0;if(dz>>>0<(du+ee|0)>>>0){aF=dT;aD=dS;aN=dR;u=dQ;b3=dz;aO=ee;c$=du}else{au=1391;break}}if((au|0)==1379){au=0;c[E>>2]=13872;c[r>>2]=29;a2=by;a3=al;a4=dZ;a5=dY;a6=ao;a7=dV;a8=ap;a9=dU;break L1802}else if((au|0)==1388){au=0;c[E>>2]=13872;c[r>>2]=29;a2=by;a3=al;a4=d0;a5=d1;a6=ao;a7=d2;a8=ap;a9=d3;break L1802}else if((au|0)==1391){au=0;if((c[r>>2]|0)==29){a2=by;a3=al;a4=dQ;a5=dR;a6=ao;a7=dS;a8=ap;a9=dT;break L1802}else{ef=ee;eg=dQ;eh=dR;ei=dS;ej=dT;break}}}else{ef=aP;eg=bz;eh=bA;ei=bB;ej=bC}}while(0);if((b[ai>>1]|0)==0){c[E>>2]=13832;c[r>>2]=29;a2=by;a3=al;a4=eg;a5=eh;a6=ao;a7=ei;a8=ap;a9=ej;break}c[af>>2]=ae;c[M>>2]=ae;c[N>>2]=9;aP=bv(1,ag,ef,af,N,ah)|0;if((aP|0)!=0){c[E>>2]=13760;c[r>>2]=29;a2=aP;a3=al;a4=eg;a5=eh;a6=ao;a7=ei;a8=ap;a9=ej;break}c[O>>2]=c[af>>2];c[P>>2]=6;aP=bv(2,q+112+(c[aa>>2]<<1)|0,c[ab>>2]|0,af,P,ah)|0;if((aP|0)==0){c[r>>2]=19;if(Q){bI=0;bK=eg;bL=eh;bM=ei;bN=ej;bO=ao;au=1483;break L1657}else{cQ=0;cR=eg;cS=eh;cT=ei;cU=ej;au=1399;break}}else{c[E>>2]=13736;c[r>>2]=29;a2=aP;a3=al;a4=eg;a5=eh;a6=ao;a7=ei;a8=ap;a9=ej;break}}else if((au|0)==1254){while(1){au=0;if((ce|0)==0){aG=j;aH=al;aI=cg;aJ=cf;aK=0;aL=cd;aM=ao;break L1657}aP=ce-1|0;aQ=cd+1|0;c$=(d[cd]<<cg)+cf|0;aO=cg+8|0;if(aO>>>0<16){cd=aQ;ce=aP;cf=c$;cg=aO;au=1254}else{ch=aQ;ci=aP;cj=c$;au=1256;break}}}}while(0);if((au|0)==1256){au=0;c$=c[D>>2]|0;if((c$|0)!=0){c[c$+8>>2]=cj&255;c[(c[D>>2]|0)+12>>2]=cj>>>8}if((c[C>>2]&512|0)!=0){a[A]=cj&255;a[B]=cj>>>8&255;c[z>>2]=a$(c[z>>2]|0,A,2)|0}c[r>>2]=4;cu=0;cv=0;cw=ci;cx=ch;au=1261}else if((au|0)==1399){au=0;c[r>>2]=20;cV=cQ;cW=cR;cX=cS;cY=cT;cZ=cU;au=1400}do{if((au|0)==1261){au=0;c$=c[C>>2]|0;do{if((c$&1024|0)==0){aP=c[D>>2]|0;if((aP|0)==0){ek=cu;el=cv;em=cw;en=cx;break}c[aP+16>>2]=0;ek=cu;el=cv;em=cw;en=cx}else{if(cu>>>0<16){aP=cx;aQ=cw;aO=cv;b3=cu;while(1){if((aQ|0)==0){aG=j;aH=al;aI=b3;aJ=aO;aK=0;aL=aP;aM=ao;break L1657}u=aQ-1|0;aN=aP+1|0;aD=(d[aP]<<b3)+aO|0;aF=b3+8|0;if(aF>>>0<16){aP=aN;aQ=u;aO=aD;b3=aF}else{eo=aN;ep=u;eq=aD;break}}}else{eo=cx;ep=cw;eq=cv}c[I>>2]=eq;b3=c[D>>2]|0;if((b3|0)==0){er=c$}else{c[b3+20>>2]=eq;er=c[C>>2]|0}if((er&512|0)==0){ek=0;el=0;em=ep;en=eo;break}a[A]=eq&255;a[B]=eq>>>8&255;c[z>>2]=a$(c[z>>2]|0,A,2)|0;ek=0;el=0;em=ep;en=eo}}while(0);c[r>>2]=5;bb=ek;bc=el;bd=em;be=en;au=1272}else if((au|0)==1400){au=0;if(cY>>>0>5&ao>>>0>257){c[m>>2]=ap;c[l>>2]=ao;c[o>>2]=cZ;c[n>>2]=cY;c[w>>2]=cX;c[x>>2]=cW;bl(f,al);c$=c[m>>2]|0;b3=c[l>>2]|0;aO=c[o>>2]|0;aQ=c[n>>2]|0;aP=c[w>>2]|0;aD=c[x>>2]|0;if((c[r>>2]|0)!=11){a2=cV;a3=al;a4=aD;a5=aP;a6=b3;a7=aQ;a8=c$;a9=aO;break}c[R>>2]=-1;a2=cV;a3=al;a4=aD;a5=aP;a6=b3;a7=aQ;a8=c$;a9=aO;break}c[R>>2]=0;aO=(1<<c[N>>2])-1|0;c$=aO&cX;aQ=c[M>>2]|0;b3=a[aQ+(c$<<2)+1|0]|0;aP=b3&255;if(aP>>>0>cW>>>0){aD=cZ;u=cY;aN=cX;aF=cW;while(1){if((u|0)==0){aG=cV;aH=al;aI=aF;aJ=aN;aK=0;aL=aD;aM=ao;break L1657}aE=u-1|0;du=aD+1|0;dz=(d[aD]<<aF)+aN|0;dv=aF+8|0;dt=aO&dz;dC=a[aQ+(dt<<2)+1|0]|0;dy=dC&255;if(dy>>>0>dv>>>0){aD=du;u=aE;aN=dz;aF=dv}else{es=du;et=aE;eu=dz;ev=dv;ew=dC;ex=dt;ey=dy;break}}}else{es=cZ;et=cY;eu=cX;ev=cW;ew=b3;ex=c$;ey=aP}aF=a[aQ+(ex<<2)|0]|0;aN=b[aQ+(ex<<2)+2>>1]|0;u=aF&255;do{if(aF<<24>>24==0){ez=0;eA=ew;eB=aN;eC=ev;eD=eu;eE=et;eF=es;eG=0}else{if((u&240|0)!=0){ez=aF;eA=ew;eB=aN;eC=ev;eD=eu;eE=et;eF=es;eG=0;break}aD=aN&65535;aO=(1<<ey+u)-1|0;dy=((eu&aO)>>>(ey>>>0))+aD|0;dt=a[aQ+(dy<<2)+1|0]|0;if(((dt&255)+ey|0)>>>0>ev>>>0){dC=es;dv=et;dz=eu;aE=ev;while(1){if((dv|0)==0){aG=cV;aH=al;aI=aE;aJ=dz;aK=0;aL=dC;aM=ao;break L1657}du=dv-1|0;dx=dC+1|0;dw=(d[dC]<<aE)+dz|0;c_=aE+8|0;dE=((dw&aO)>>>(ey>>>0))+aD|0;dA=a[aQ+(dE<<2)+1|0]|0;if(((dA&255)+ey|0)>>>0>c_>>>0){dC=dx;dv=du;dz=dw;aE=c_}else{eH=dx;eI=du;eJ=dw;eK=c_;eL=dE;eM=dA;break}}}else{eH=es;eI=et;eJ=eu;eK=ev;eL=dy;eM=dt}aE=b[aQ+(eL<<2)+2>>1]|0;dz=a[aQ+(eL<<2)|0]|0;c[R>>2]=ey;ez=dz;eA=eM;eB=aE;eC=eK-ey|0;eD=eJ>>>(ey>>>0);eE=eI;eF=eH;eG=ey}}while(0);aQ=eA&255;u=eD>>>(aQ>>>0);aN=eC-aQ|0;c[R>>2]=eG+aQ;c[I>>2]=eB&65535;aQ=ez&255;if(ez<<24>>24==0){c[r>>2]=25;a2=cV;a3=al;a4=aN;a5=u;a6=ao;a7=eE;a8=ap;a9=eF;break}if((aQ&32|0)!=0){c[R>>2]=-1;c[r>>2]=11;a2=cV;a3=al;a4=aN;a5=u;a6=ao;a7=eE;a8=ap;a9=eF;break}if((aQ&64|0)==0){aF=aQ&15;c[S>>2]=aF;c[r>>2]=21;bf=cV;bg=aN;bh=u;bi=eE;bj=eF;bk=aF;au=1419;break}else{c[E>>2]=13680;c[r>>2]=29;a2=cV;a3=al;a4=aN;a5=u;a6=ao;a7=eE;a8=ap;a9=eF;break}}}while(0);if((au|0)==1272){au=0;u=c[C>>2]|0;if((u&1024|0)==0){eN=bd;eO=be}else{aN=c[I>>2]|0;aF=aN>>>0>bd>>>0?bd:aN;if((aF|0)==0){eP=bd;eQ=be;eR=aN}else{aQ=c[D>>2]|0;do{if((aQ|0)==0){eS=u}else{aP=c[aQ+16>>2]|0;if((aP|0)==0){eS=u;break}c$=(c[aQ+20>>2]|0)-aN|0;b3=aP+c$|0;aP=c[aQ+24>>2]|0;aE=(c$+aF|0)>>>0>aP>>>0?aP-c$|0:aF;bJ(b3|0,be|0,aE)|0;eS=c[C>>2]|0}}while(0);if((eS&512|0)!=0){c[z>>2]=a$(c[z>>2]|0,be,aF)|0}aQ=(c[I>>2]|0)-aF|0;c[I>>2]=aQ;eP=bd-aF|0;eQ=be+aF|0;eR=aQ}if((eR|0)==0){eN=eP;eO=eQ}else{bI=j;bK=bb;bL=bc;bM=eP;bN=eQ;bO=ao;au=1483;break}}c[I>>2]=0;c[r>>2]=6;c8=bb;c9=bc;da=eN;db=eO;au=1282}else if((au|0)==1419){au=0;if((bk|0)==0){eT=bg;eU=bh;eV=bi;eW=bj;eX=c[I>>2]|0}else{if(bg>>>0<bk>>>0){aQ=bj;aN=bi;u=bh;aE=bg;while(1){if((aN|0)==0){aG=bf;aH=al;aI=aE;aJ=u;aK=0;aL=aQ;aM=ao;break L1657}b3=aN-1|0;c$=aQ+1|0;aP=(d[aQ]<<aE)+u|0;dz=aE+8|0;if(dz>>>0<bk>>>0){aQ=c$;aN=b3;u=aP;aE=dz}else{eY=c$;eZ=b3;e_=aP;e$=dz;break}}}else{eY=bj;eZ=bi;e_=bh;e$=bg}aE=(c[I>>2]|0)+((1<<bk)-1&e_)|0;c[I>>2]=aE;c[R>>2]=(c[R>>2]|0)+bk;eT=e$-bk|0;eU=e_>>>(bk>>>0);eV=eZ;eW=eY;eX=aE}c[T>>2]=eX;c[r>>2]=22;cy=bf;cz=eT;cA=eU;cB=eV;cC=eW;au=1426}do{if((au|0)==1426){au=0;aE=(1<<c[P>>2])-1|0;u=aE&cA;aN=c[O>>2]|0;aQ=a[aN+(u<<2)+1|0]|0;aF=aQ&255;if(aF>>>0>cz>>>0){dz=cC;aP=cB;b3=cA;c$=cz;while(1){if((aP|0)==0){aG=cy;aH=al;aI=c$;aJ=b3;aK=0;aL=dz;aM=ao;break L1657}dv=aP-1|0;dC=dz+1|0;aD=(d[dz]<<c$)+b3|0;aO=c$+8|0;dA=aE&aD;dE=a[aN+(dA<<2)+1|0]|0;c_=dE&255;if(c_>>>0>aO>>>0){dz=dC;aP=dv;b3=aD;c$=aO}else{e0=dC;e1=dv;e2=aD;e3=aO;e4=dE;e5=dA;e6=c_;break}}}else{e0=cC;e1=cB;e2=cA;e3=cz;e4=aQ;e5=u;e6=aF}c$=a[aN+(e5<<2)|0]|0;b3=b[aN+(e5<<2)+2>>1]|0;aP=c$&255;if((aP&240|0)==0){dz=b3&65535;aE=(1<<e6+aP)-1|0;aP=((e2&aE)>>>(e6>>>0))+dz|0;c_=a[aN+(aP<<2)+1|0]|0;if(((c_&255)+e6|0)>>>0>e3>>>0){dA=e0;dE=e1;aO=e2;aD=e3;while(1){if((dE|0)==0){aG=cy;aH=al;aI=aD;aJ=aO;aK=0;aL=dA;aM=ao;break L1657}dv=dE-1|0;dC=dA+1|0;dw=(d[dA]<<aD)+aO|0;du=aD+8|0;dx=((dw&aE)>>>(e6>>>0))+dz|0;dF=a[aN+(dx<<2)+1|0]|0;if(((dF&255)+e6|0)>>>0>du>>>0){dA=dC;dE=dv;aO=dw;aD=du}else{e7=dC;e8=dv;e9=dw;fa=du;fb=dx;fc=dF;break}}}else{e7=e0;e8=e1;e9=e2;fa=e3;fb=aP;fc=c_}aD=b[aN+(fb<<2)+2>>1]|0;aO=a[aN+(fb<<2)|0]|0;dE=(c[R>>2]|0)+e6|0;c[R>>2]=dE;fd=aO;fe=fc;ff=aD;fg=fa-e6|0;fh=e9>>>(e6>>>0);fi=e8;fj=e7;fk=dE}else{fd=c$;fe=e4;ff=b3;fg=e3;fh=e2;fi=e1;fj=e0;fk=c[R>>2]|0}dE=fe&255;aD=fh>>>(dE>>>0);aO=fg-dE|0;c[R>>2]=fk+dE;dE=fd&255;if((dE&64|0)==0){c[U>>2]=ff&65535;dA=dE&15;c[S>>2]=dA;c[r>>2]=23;co=cy;cp=aO;cq=aD;cr=fi;cs=fj;ct=dA;au=1438;break}else{c[E>>2]=13632;c[r>>2]=29;a2=cy;a3=al;a4=aO;a5=aD;a6=ao;a7=fi;a8=ap;a9=fj;break}}else if((au|0)==1282){au=0;do{if((c[C>>2]&2048|0)==0){aD=c[D>>2]|0;if((aD|0)==0){fl=da;fm=db;break}c[aD+28>>2]=0;fl=da;fm=db}else{if((da|0)==0){bI=j;bK=c8;bL=c9;bM=0;bN=db;bO=ao;au=1483;break L1657}else{fn=0}while(1){fo=fn+1|0;aD=a[db+fn|0]|0;aO=c[D>>2]|0;do{if((aO|0)!=0){dA=aO+28|0;if((c[dA>>2]|0)==0){break}dE=c[I>>2]|0;if(dE>>>0>=(c[aO+32>>2]|0)>>>0){break}c[I>>2]=dE+1;a[(c[dA>>2]|0)+dE|0]=aD}}while(0);fp=aD<<24>>24!=0;if(fp&fo>>>0<da>>>0){fn=fo}else{break}}if((c[C>>2]&512|0)!=0){c[z>>2]=a$(c[z>>2]|0,db,fo)|0}dt=da-fo|0;dy=db+fo|0;if(fp){bI=j;bK=c8;bL=c9;bM=dt;bN=dy;bO=ao;au=1483;break L1657}else{fl=dt;fm=dy}}}while(0);c[I>>2]=0;c[r>>2]=7;ay=c8;az=c9;aA=fl;aC=fm;au=1295}}while(0);if((au|0)==1295){au=0;do{if((c[C>>2]&4096|0)==0){b3=c[D>>2]|0;if((b3|0)==0){fq=aA;fr=aC;break}c[b3+36>>2]=0;fq=aA;fr=aC}else{if((aA|0)==0){bI=j;bK=ay;bL=az;bM=0;bN=aC;bO=ao;au=1483;break L1657}else{fs=0}while(1){ft=fs+1|0;b3=a[aC+fs|0]|0;c$=c[D>>2]|0;do{if((c$|0)!=0){aN=c$+36|0;if((c[aN>>2]|0)==0){break}c_=c[I>>2]|0;if(c_>>>0>=(c[c$+40>>2]|0)>>>0){break}c[I>>2]=c_+1;a[(c[aN>>2]|0)+c_|0]=b3}}while(0);fu=b3<<24>>24!=0;if(fu&ft>>>0<aA>>>0){fs=ft}else{break}}if((c[C>>2]&512|0)!=0){c[z>>2]=a$(c[z>>2]|0,aC,ft)|0}c$=aA-ft|0;c_=aC+ft|0;if(fu){bI=j;bK=ay;bL=az;bM=c$;bN=c_;bO=ao;au=1483;break L1657}else{fq=c$;fr=c_}}}while(0);c[r>>2]=8;ck=ay;cl=az;cm=fq;cn=fr;au=1308}else if((au|0)==1438){au=0;if((ct|0)==0){fv=cp;fw=cq;fx=cr;fy=cs}else{if(cp>>>0<ct>>>0){c_=cs;c$=cr;aN=cq;aP=cp;while(1){if((c$|0)==0){aG=co;aH=al;aI=aP;aJ=aN;aK=0;aL=c_;aM=ao;break L1657}dy=c$-1|0;dt=c_+1|0;aO=(d[c_]<<aP)+aN|0;dE=aP+8|0;if(dE>>>0<ct>>>0){c_=dt;c$=dy;aN=aO;aP=dE}else{fz=dt;fA=dy;fB=aO;fC=dE;break}}}else{fz=cs;fA=cr;fB=cq;fC=cp}c[U>>2]=(c[U>>2]|0)+((1<<ct)-1&fB);c[R>>2]=(c[R>>2]|0)+ct;fv=fC-ct|0;fw=fB>>>(ct>>>0);fx=fA;fy=fz}c[r>>2]=24;cD=co;cE=fv;cF=fw;cG=fx;cH=fy;au=1444}L2018:do{if((au|0)==1308){au=0;aP=c[C>>2]|0;do{if((aP&512|0)==0){fD=ck;fE=cl;fF=cm;fG=cn}else{if(ck>>>0<16){aN=cn;c$=cm;c_=cl;dE=ck;while(1){if((c$|0)==0){aG=j;aH=al;aI=dE;aJ=c_;aK=0;aL=aN;aM=ao;break L1657}aO=c$-1|0;dy=aN+1|0;dt=(d[aN]<<dE)+c_|0;dA=dE+8|0;if(dA>>>0<16){aN=dy;c$=aO;c_=dt;dE=dA}else{fH=dy;fI=aO;fJ=dt;fK=dA;break}}}else{fH=cn;fI=cm;fJ=cl;fK=ck}if((fJ|0)==(c[z>>2]&65535|0)){fD=0;fE=0;fF=fI;fG=fH;break}c[E>>2]=13152;c[r>>2]=29;a2=j;a3=al;a4=fK;a5=fJ;a6=ao;a7=fI;a8=ap;a9=fH;break L2018}}while(0);dE=c[D>>2]|0;if((dE|0)!=0){c[dE+44>>2]=aP>>>9&1;c[(c[D>>2]|0)+48>>2]=1}dE=a$(0,0,0)|0;c[z>>2]=dE;c[H>>2]=dE;c[r>>2]=11;a2=j;a3=al;a4=fD;a5=fE;a6=ao;a7=fF;a8=ap;a9=fG}else if((au|0)==1444){au=0;if((ao|0)==0){bI=cD;bK=cE;bL=cF;bM=cG;bN=cH;bO=0;au=1483;break L1657}dE=al-ao|0;c_=c[U>>2]|0;if(c_>>>0>dE>>>0){c$=c_-dE|0;do{if(c$>>>0>(c[V>>2]|0)>>>0){if((c[W>>2]|0)==0){break}c[E>>2]=13600;c[r>>2]=29;a2=cD;a3=al;a4=cE;a5=cF;a6=ao;a7=cG;a8=ap;a9=cH;break L2018}}while(0);aP=c[X>>2]|0;if(c$>>>0>aP>>>0){dE=c$-aP|0;fL=(c[Y>>2]|0)+((c[Z>>2]|0)-dE)|0;fM=dE}else{fL=(c[Y>>2]|0)+(aP-c$)|0;fM=c$}aP=c[I>>2]|0;fN=fL;fO=fM>>>0>aP>>>0?aP:fM;fP=aP}else{aP=c[I>>2]|0;fN=ap+(-c_|0)|0;fO=aP;fP=aP}aP=fO>>>0>ao>>>0?ao:fO;c[I>>2]=fP-aP;dE=~ao;aN=~fO;b3=dE>>>0>aN>>>0?dE:aN;aN=fN;dE=aP;dA=ap;while(1){a[dA]=a[aN]|0;dt=dE-1|0;if((dt|0)==0){break}else{aN=aN+1|0;dE=dt;dA=dA+1|0}}dA=ao-aP|0;dE=ap+~b3|0;if((c[I>>2]|0)!=0){a2=cD;a3=al;a4=cE;a5=cF;a6=dA;a7=cG;a8=dE;a9=cH;break}c[r>>2]=20;a2=cD;a3=al;a4=cE;a5=cF;a6=dA;a7=cG;a8=dE;a9=cH}}while(0);j=a2;al=a3;am=a4;an=a5;ao=a6;p=a7;ap=a8;s=a9;t=c[r>>2]|0}if((au|0)==1482){c[r>>2]=28;aG=1;aH=b_;aI=dg;aJ=dh;aK=di;aL=dj;aM=ao}else if((au|0)==1483){aG=bI;aH=al;aI=bK;aJ=bL;aK=bM;aL=bN;aM=bO}else if((au|0)==1497){k=-4;i=h;return k|0}else if((au|0)==1332){aG=j;aH=al;aI=ds-3|0;aJ=dr>>>3;aK=dq;aL=dp;aM=ao}else if((au|0)==1321){c[m>>2]=ap;c[l>>2]=ao;c[o>>2]=aX;c[n>>2]=aW;c[w>>2]=aV;c[x>>2]=aU;k=2;i=h;return k|0}else if((au|0)==1500){i=h;return k|0}c[m>>2]=ap;c[l>>2]=aM;c[o>>2]=aL;c[n>>2]=aK;c[w>>2]=aJ;c[x>>2]=aI;do{if((c[Z>>2]|0)==0){if((c[r>>2]|0)>>>0>=26){break}if((aH|0)!=(c[l>>2]|0)){au=1487}}else{au=1487}}while(0);do{if((au|0)==1487){if((bt(f,aH)|0)==0){break}c[r>>2]=30;k=-4;i=h;return k|0}}while(0);au=c[n>>2]|0;n=c[l>>2]|0;l=aH-n|0;Z=f+8|0;c[Z>>2]=v-au+(c[Z>>2]|0);c[_>>2]=(c[_>>2]|0)+l;c[$>>2]=(c[$>>2]|0)+l;$=(aH|0)==(n|0);if(!((c[y>>2]|0)==0|$)){y=c[z>>2]|0;n=(c[m>>2]|0)+(-l|0)|0;if((c[C>>2]|0)==0){fQ=a_(y,n,l)|0}else{fQ=a$(y,n,l)|0}c[z>>2]=fQ;c[H>>2]=fQ}fQ=c[r>>2]|0;if((fQ|0)==19){fR=256}else{fR=(fQ|0)==14?256:0}c[f+44>>2]=((c[L>>2]|0)!=0?64:0)+(c[x>>2]|0)+((fQ|0)==11?128:0)+fR;k=((v|0)==(au|0)&$|(g|0)==4)&(aG|0)==0?-5:aG;i=h;return k|0}function br(a){a=a|0;var d=0;d=0;do{b[a+148+(d<<2)>>1]=0;d=d+1|0;}while((d|0)<286);b[a+2440>>1]=0;b[a+2444>>1]=0;b[a+2448>>1]=0;b[a+2452>>1]=0;b[a+2456>>1]=0;b[a+2460>>1]=0;b[a+2464>>1]=0;b[a+2468>>1]=0;b[a+2472>>1]=0;b[a+2476>>1]=0;b[a+2480>>1]=0;b[a+2484>>1]=0;b[a+2488>>1]=0;b[a+2492>>1]=0;b[a+2496>>1]=0;b[a+2500>>1]=0;b[a+2504>>1]=0;b[a+2508>>1]=0;b[a+2512>>1]=0;b[a+2516>>1]=0;b[a+2520>>1]=0;b[a+2524>>1]=0;b[a+2528>>1]=0;b[a+2532>>1]=0;b[a+2536>>1]=0;b[a+2540>>1]=0;b[a+2544>>1]=0;b[a+2548>>1]=0;b[a+2552>>1]=0;b[a+2556>>1]=0;b[a+2684>>1]=0;b[a+2688>>1]=0;b[a+2692>>1]=0;b[a+2696>>1]=0;b[a+2700>>1]=0;b[a+2704>>1]=0;b[a+2708>>1]=0;b[a+2712>>1]=0;b[a+2716>>1]=0;b[a+2720>>1]=0;b[a+2724>>1]=0;b[a+2728>>1]=0;b[a+2732>>1]=0;b[a+2736>>1]=0;b[a+2740>>1]=0;b[a+2744>>1]=0;b[a+2748>>1]=0;b[a+2752>>1]=0;b[a+2756>>1]=0;b[a+1172>>1]=1;c[a+5804>>2]=0;c[a+5800>>2]=0;c[a+5808>>2]=0;c[a+5792>>2]=0;return}function bs(d,f,g,h){d=d|0;f=f|0;g=g|0;h=h|0;var i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0;i=d+5820|0;j=c[i>>2]|0;k=h&65535;h=d+5816|0;l=e[h>>1]|0|k<<j;b[h>>1]=l&65535;if((j|0)>13){m=d+20|0;n=c[m>>2]|0;c[m>>2]=n+1;o=d+8|0;a[(c[o>>2]|0)+n|0]=l&255;n=(e[h>>1]|0)>>>8&255;p=c[m>>2]|0;c[m>>2]=p+1;a[(c[o>>2]|0)+p|0]=n;n=c[i>>2]|0;p=k>>>((16-n|0)>>>0);b[h>>1]=p&65535;q=n-13|0;r=p&255}else{q=j+3|0;r=l&255}c[i>>2]=q;do{if((q|0)>8){l=d+20|0;j=c[l>>2]|0;c[l>>2]=j+1;p=d+8|0;a[(c[p>>2]|0)+j|0]=r;j=(e[h>>1]|0)>>>8&255;n=c[l>>2]|0;c[l>>2]=n+1;a[(c[p>>2]|0)+n|0]=j;s=l;t=p}else{p=d+20|0;if((q|0)>0){l=c[p>>2]|0;c[p>>2]=l+1;j=d+8|0;a[(c[j>>2]|0)+l|0]=r;s=p;t=j;break}else{s=p;t=d+8|0;break}}}while(0);b[h>>1]=0;c[i>>2]=0;c[d+5812>>2]=8;d=c[s>>2]|0;c[s>>2]=d+1;a[(c[t>>2]|0)+d|0]=g&255;d=c[s>>2]|0;c[s>>2]=d+1;a[(c[t>>2]|0)+d|0]=g>>>8&255;d=g&65535^65535;i=c[s>>2]|0;c[s>>2]=i+1;a[(c[t>>2]|0)+i|0]=d&255;i=c[s>>2]|0;c[s>>2]=i+1;a[(c[t>>2]|0)+i|0]=d>>>8&255;if((g|0)==0){return}else{u=g;v=f}while(1){f=u-1|0;g=a[v]|0;d=c[s>>2]|0;c[s>>2]=d+1;a[(c[t>>2]|0)+d|0]=g;if((f|0)==0){break}else{u=f;v=v+1|0}}return}function bt(a,b){a=a|0;b=b|0;var d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0;d=c[a+28>>2]|0;e=d+52|0;f=c[e>>2]|0;do{if((f|0)==0){g=aG[c[a+32>>2]&7](c[a+40>>2]|0,1<<c[d+36>>2],1)|0;c[e>>2]=g;if((g|0)==0){h=1}else{i=g;break}return h|0}else{i=f}}while(0);f=d+40|0;g=c[f>>2]|0;if((g|0)==0){j=1<<c[d+36>>2];c[f>>2]=j;c[d+48>>2]=0;c[d+44>>2]=0;k=j}else{k=g}g=b-(c[a+16>>2]|0)|0;if(g>>>0>=k>>>0){b=(c[a+12>>2]|0)+(-k|0)|0;bJ(i|0,b|0,k)|0;c[d+48>>2]=0;c[d+44>>2]=c[f>>2];h=0;return h|0}b=d+48|0;j=c[b>>2]|0;l=k-j|0;k=l>>>0>g>>>0?g:l;l=i+j|0;j=a+12|0;a=(c[j>>2]|0)+(-g|0)|0;bJ(l|0,a|0,k)|0;a=g-k|0;if((g|0)!=(k|0)){k=c[e>>2]|0;e=(c[j>>2]|0)+(-a|0)|0;bJ(k|0,e|0,a)|0;c[b>>2]=a;c[d+44>>2]=c[f>>2];h=0;return h|0}a=(c[b>>2]|0)+g|0;c[b>>2]=a;e=c[f>>2]|0;if((a|0)==(e|0)){c[b>>2]=0}b=d+44|0;d=c[b>>2]|0;if(d>>>0>=e>>>0){h=0;return h|0}c[b>>2]=d+g;h=0;return h|0}function bu(a){a=a|0;var b=0,d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0;if((a|0)==0){b=-2;return b|0}d=a+28|0;e=c[d>>2]|0;if((e|0)==0){b=-2;return b|0}f=a+36|0;g=c[f>>2]|0;if((g|0)==0){b=-2;return b|0}h=c[e+52>>2]|0;i=a+40|0;if((h|0)==0){j=g;k=e}else{aF[g&3](c[i>>2]|0,h);j=c[f>>2]|0;k=c[d>>2]|0}aF[j&3](c[i>>2]|0,k);c[d>>2]=0;b=0;return b|0}function bv(d,f,g,h,j,k){d=d|0;f=f|0;g=g|0;h=h|0;j=j|0;k=k|0;var l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0,X=0,Y=0,Z=0,_=0;l=i;i=i+32|0;m=l|0;n=i;i=i+32|0;bI(m|0,0,32);o=(g|0)==0;if(!o){p=0;do{q=m+(e[f+(p<<1)>>1]<<1)|0;b[q>>1]=(b[q>>1]|0)+1&65535;p=p+1|0;}while(p>>>0<g>>>0)}p=c[j>>2]|0;q=15;while(1){if((q|0)==0){r=1559;break}if((b[m+(q<<1)>>1]|0)==0){q=q-1|0}else{break}}if((r|0)==1559){s=c[h>>2]|0;c[h>>2]=s+4;a[s|0]=64;a[s+1|0]=1;b[s+2>>1]=0;s=c[h>>2]|0;c[h>>2]=s+4;a[s|0]=64;a[s+1|0]=1;b[s+2>>1]=0;c[j>>2]=1;t=0;i=l;return t|0}s=p>>>0>q>>>0?q:p;p=1;while(1){if(p>>>0>=q>>>0){break}if((b[m+(p<<1)>>1]|0)==0){p=p+1|0}else{break}}u=s>>>0<p>>>0?p:s;s=1;v=1;while(1){if(v>>>0>=16){break}w=(s<<1)-(e[m+(v<<1)>>1]|0)|0;if((w|0)<0){t=-1;r=1612;break}else{s=w;v=v+1|0}}if((r|0)==1612){i=l;return t|0}do{if((s|0)>0){if((d|0)!=0&(q|0)==1){break}else{t=-1}i=l;return t|0}}while(0);b[n+2>>1]=0;s=b[m+2>>1]|0;b[n+4>>1]=s;v=(b[m+4>>1]|0)+s&65535;b[n+6>>1]=v;s=(b[m+6>>1]|0)+v&65535;b[n+8>>1]=s;v=(b[m+8>>1]|0)+s&65535;b[n+10>>1]=v;s=(b[m+10>>1]|0)+v&65535;b[n+12>>1]=s;v=(b[m+12>>1]|0)+s&65535;b[n+14>>1]=v;s=(b[m+14>>1]|0)+v&65535;b[n+16>>1]=s;v=(b[m+16>>1]|0)+s&65535;b[n+18>>1]=v;s=(b[m+18>>1]|0)+v&65535;b[n+20>>1]=s;v=(b[m+20>>1]|0)+s&65535;b[n+22>>1]=v;s=(b[m+22>>1]|0)+v&65535;b[n+24>>1]=s;v=(b[m+24>>1]|0)+s&65535;b[n+26>>1]=v;s=(b[m+26>>1]|0)+v&65535;b[n+28>>1]=s;b[n+30>>1]=(b[m+28>>1]|0)+s&65535;if(!o){o=0;do{s=b[f+(o<<1)>>1]|0;if(s<<16>>16!=0){v=n+((s&65535)<<1)|0;s=b[v>>1]|0;b[v>>1]=s+1&65535;b[k+((s&65535)<<1)>>1]=o&65535}o=o+1|0;}while(o>>>0<g>>>0)}do{if((d|0)==0){x=0;y=1<<u;z=19;A=k;B=k;C=0}else if((d|0)==1){g=1<<u;if(g>>>0>851){t=1}else{x=1;y=g;z=256;A=894;B=958;C=0;break}i=l;return t|0}else{g=1<<u;o=(d|0)==2;if(o&g>>>0>591){t=1}else{x=0;y=g;z=-1;A=1536;B=1600;C=o;break}i=l;return t|0}}while(0);d=y-1|0;o=u&255;g=c[h>>2]|0;n=-1;s=0;v=y;y=0;w=u;D=0;E=p;L2178:while(1){p=1<<w;F=s;G=D;H=E;while(1){I=H-y|0;J=I&255;K=b[k+(G<<1)>>1]|0;L=K&65535;do{if((L|0)<(z|0)){M=0;N=K}else{if((L|0)<=(z|0)){M=96;N=0;break}M=b[A+(L<<1)>>1]&255;N=b[B+(L<<1)>>1]|0}}while(0);L=1<<I;K=F>>>(y>>>0);O=p;while(1){P=O-L|0;Q=P+K|0;a[g+(Q<<2)|0]=M;a[g+(Q<<2)+1|0]=J;b[g+(Q<<2)+2>>1]=N;if((O|0)==(L|0)){break}else{O=P}}O=1<<H-1;while(1){if((O&F|0)==0){break}else{O=O>>>1}}if((O|0)==0){R=0}else{R=(O-1&F)+O|0}S=G+1|0;L=m+(H<<1)|0;K=(b[L>>1]|0)-1&65535;b[L>>1]=K;if(K<<16>>16==0){if((H|0)==(q|0)){break L2178}T=e[f+(e[k+(S<<1)>>1]<<1)>>1]|0}else{T=H}if(T>>>0<=u>>>0){F=R;G=S;H=T;continue}U=R&d;if((U|0)==(n|0)){F=R;G=S;H=T}else{break}}H=(y|0)==0?u:y;G=g+(p<<2)|0;F=T-H|0;L2201:do{if(T>>>0<q>>>0){K=F;L=1<<F;I=T;while(1){P=L-(e[m+(I<<1)>>1]|0)|0;if((P|0)<1){V=K;break L2201}Q=K+1|0;W=Q+H|0;if(W>>>0<q>>>0){K=Q;L=P<<1;I=W}else{V=Q;break}}}else{V=F}}while(0);F=(1<<V)+v|0;if(x&F>>>0>851|C&F>>>0>591){t=1;r=1613;break}a[(c[h>>2]|0)+(U<<2)|0]=V&255;a[(c[h>>2]|0)+(U<<2)+1|0]=o;p=c[h>>2]|0;b[p+(U<<2)+2>>1]=(G-p|0)>>>2&65535;g=G;n=U;s=R;v=F;y=H;w=V;D=S;E=T}if((r|0)==1613){i=l;return t|0}L2211:do{if((R|0)!=0){r=q;T=y;E=R;S=J;D=g;while(1){do{if((T|0)==0){X=D;Y=S;Z=0;_=r}else{if((E&d|0)==(n|0)){X=D;Y=S;Z=T;_=r;break}X=c[h>>2]|0;Y=o;Z=0;_=u}}while(0);V=E>>>(Z>>>0);a[X+(V<<2)|0]=64;a[X+(V<<2)+1|0]=Y;b[X+(V<<2)+2>>1]=0;V=1<<_-1;while(1){if((V&E|0)==0){break}else{V=V>>>1}}if((V|0)==0){break L2211}w=(V-1&E)+V|0;if((w|0)==0){break}else{r=_;T=Z;E=w;S=Y;D=X}}}}while(0);c[h>>2]=(c[h>>2]|0)+(v<<2);c[j>>2]=u;t=0;i=l;return t|0}function bw(a){a=a|0;c[a+2840>>2]=a+148;c[a+2848>>2]=1200;c[a+2852>>2]=a+2440;c[a+2860>>2]=1344;c[a+2864>>2]=a+2684;c[a+2872>>2]=1368;b[a+5816>>1]=0;c[a+5820>>2]=0;c[a+5812>>2]=8;br(a);return}function bx(d){d=d|0;var f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0;f=d+5820|0;g=c[f>>2]|0;h=d+5816|0;i=e[h>>1]|0|2<<g;j=i&65535;b[h>>1]=j;if((g|0)>13){k=d+20|0;l=c[k>>2]|0;c[k>>2]=l+1;m=d+8|0;a[(c[m>>2]|0)+l|0]=i&255;i=(e[h>>1]|0)>>>8&255;l=c[k>>2]|0;c[k>>2]=l+1;a[(c[m>>2]|0)+l|0]=i;i=c[f>>2]|0;l=2>>>((16-i|0)>>>0)&65535;b[h>>1]=l;n=i-13|0;o=l}else{n=g+3|0;o=j}c[f>>2]=n;if((n|0)>9){j=d+20|0;g=c[j>>2]|0;c[j>>2]=g+1;l=d+8|0;a[(c[l>>2]|0)+g|0]=o&255;g=(e[h>>1]|0)>>>8&255;i=c[j>>2]|0;c[j>>2]=i+1;a[(c[l>>2]|0)+i|0]=g;b[h>>1]=0;p=(c[f>>2]|0)-9|0;q=0}else{p=n+7|0;q=o}c[f>>2]=p;do{if((p|0)==16){o=d+20|0;n=c[o>>2]|0;c[o>>2]=n+1;g=d+8|0;a[(c[g>>2]|0)+n|0]=q&255;n=(e[h>>1]|0)>>>8&255;i=c[o>>2]|0;c[o>>2]=i+1;a[(c[g>>2]|0)+i|0]=n;b[h>>1]=0;c[f>>2]=0;r=0;s=0}else{if((p|0)<=7){r=p;s=q;break}n=d+20|0;i=c[n>>2]|0;c[n>>2]=i+1;a[(c[d+8>>2]|0)+i|0]=q&255;i=(e[h>>1]|0)>>>8;b[h>>1]=i;n=(c[f>>2]|0)-8|0;c[f>>2]=n;r=n;s=i}}while(0);q=d+5812|0;if((11-r+(c[q>>2]|0)|0)>=9){c[q>>2]=7;return}p=s&65535|2<<r;b[h>>1]=p&65535;if((r|0)>13){s=d+20|0;i=c[s>>2]|0;c[s>>2]=i+1;n=d+8|0;a[(c[n>>2]|0)+i|0]=p&255;i=(e[h>>1]|0)>>>8&255;g=c[s>>2]|0;c[s>>2]=g+1;a[(c[n>>2]|0)+g|0]=i;i=c[f>>2]|0;g=2>>>((16-i|0)>>>0);b[h>>1]=g&65535;t=i-13|0;u=g&255}else{t=r+3|0;u=p&255}c[f>>2]=t;if((t|0)>9){p=d+20|0;r=c[p>>2]|0;c[p>>2]=r+1;g=d+8|0;a[(c[g>>2]|0)+r|0]=u;r=(e[h>>1]|0)>>>8&255;i=c[p>>2]|0;c[p>>2]=i+1;a[(c[g>>2]|0)+i|0]=r;b[h>>1]=0;v=(c[f>>2]|0)-9|0;w=0}else{v=t+7|0;w=u}c[f>>2]=v;if((v|0)==16){u=d+20|0;t=c[u>>2]|0;c[u>>2]=t+1;r=d+8|0;a[(c[r>>2]|0)+t|0]=w;t=(e[h>>1]|0)>>>8&255;i=c[u>>2]|0;c[u>>2]=i+1;a[(c[r>>2]|0)+i|0]=t;b[h>>1]=0;c[f>>2]=0;c[q>>2]=7;return}if((v|0)<=7){c[q>>2]=7;return}v=d+20|0;t=c[v>>2]|0;c[v>>2]=t+1;a[(c[d+8>>2]|0)+t|0]=w;b[h>>1]=(e[h>>1]|0)>>>8;c[f>>2]=(c[f>>2]|0)-8;c[q>>2]=7;return}function by(f,g,h,i){f=f|0;g=g|0;h=h|0;i=i|0;var j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0;if((c[f+132>>2]|0)>0){j=(c[f>>2]|0)+44|0;if((c[j>>2]|0)==2){k=-201342849;l=0;while(1){if((k&1|0)!=0){if((b[f+148+(l<<2)>>1]|0)!=0){m=0;break}}n=l+1|0;if((n|0)<32){k=k>>>1;l=n}else{o=1646;break}}L2268:do{if((o|0)==1646){if((b[f+184>>1]|0)!=0){m=1;break}if((b[f+188>>1]|0)!=0){m=1;break}if((b[f+200>>1]|0)==0){p=32}else{m=1;break}while(1){if((p|0)>=256){m=0;break L2268}if((b[f+148+(p<<2)>>1]|0)==0){p=p+1|0}else{m=1;break}}}}while(0);c[j>>2]=m}bz(f,f+2840|0);bz(f,f+2852|0);bC(f,f+148|0,c[f+2844>>2]|0);bC(f,f+2440|0,c[f+2856>>2]|0);bz(f,f+2864|0);m=18;while(1){if((m|0)<=2){break}if((b[f+2684+(d[12512+m|0]<<2)+2>>1]|0)==0){m=m-1|0}else{break}}j=f+5800|0;p=(m*3|0)+17+(c[j>>2]|0)|0;c[j>>2]=p;j=(p+10|0)>>>3;p=((c[f+5804>>2]|0)+10|0)>>>3;q=p>>>0>j>>>0?j:p;r=p;s=m}else{m=h+5|0;q=m;r=m;s=0}do{if((h+4|0)>>>0>q>>>0|(g|0)==0){m=f+5820|0;p=c[m>>2]|0;j=(p|0)>13;if((c[f+136>>2]|0)==4|(r|0)==(q|0)){o=i+2&65535;l=f+5816|0;k=e[l>>1]|o<<p;b[l>>1]=k&65535;if(j){n=f+20|0;t=c[n>>2]|0;c[n>>2]=t+1;u=f+8|0;a[(c[u>>2]|0)+t|0]=k&255;k=(e[l>>1]|0)>>>8&255;t=c[n>>2]|0;c[n>>2]=t+1;a[(c[u>>2]|0)+t|0]=k;k=c[m>>2]|0;b[l>>1]=o>>>((16-k|0)>>>0)&65535;v=k-13|0}else{v=p+3|0}c[m>>2]=v;bA(f,48,1224);break}k=i+4&65535;o=f+5816|0;l=e[o>>1]|k<<p;t=l&65535;b[o>>1]=t;if(j){j=f+20|0;u=c[j>>2]|0;c[j>>2]=u+1;n=f+8|0;a[(c[n>>2]|0)+u|0]=l&255;l=(e[o>>1]|0)>>>8&255;u=c[j>>2]|0;c[j>>2]=u+1;a[(c[n>>2]|0)+u|0]=l;l=c[m>>2]|0;u=k>>>((16-l|0)>>>0)&65535;b[o>>1]=u;w=l-13|0;x=u}else{w=p+3|0;x=t}c[m>>2]=w;t=c[f+2844>>2]|0;p=c[f+2856>>2]|0;u=s+1|0;l=t+65280&65535;k=x&65535|l<<w;n=k&65535;b[o>>1]=n;if((w|0)>11){j=f+20|0;y=c[j>>2]|0;c[j>>2]=y+1;z=f+8|0;a[(c[z>>2]|0)+y|0]=k&255;k=(e[o>>1]|0)>>>8&255;y=c[j>>2]|0;c[j>>2]=y+1;a[(c[z>>2]|0)+y|0]=k;k=c[m>>2]|0;y=l>>>((16-k|0)>>>0)&65535;b[o>>1]=y;A=k-11|0;B=y}else{A=w+5|0;B=n}c[m>>2]=A;n=p&65535;y=n<<A|B&65535;k=y&65535;b[o>>1]=k;if((A|0)>11){l=f+20|0;z=c[l>>2]|0;c[l>>2]=z+1;j=f+8|0;a[(c[j>>2]|0)+z|0]=y&255;y=(e[o>>1]|0)>>>8&255;z=c[l>>2]|0;c[l>>2]=z+1;a[(c[j>>2]|0)+z|0]=y;y=c[m>>2]|0;z=n>>>((16-y|0)>>>0)&65535;b[o>>1]=z;C=y-11|0;D=z}else{C=A+5|0;D=k}c[m>>2]=C;k=s+65533&65535;z=k<<C|D&65535;y=z&65535;b[o>>1]=y;if((C|0)>12){n=f+20|0;j=c[n>>2]|0;c[n>>2]=j+1;l=f+8|0;a[(c[l>>2]|0)+j|0]=z&255;z=(e[o>>1]|0)>>>8&255;j=c[n>>2]|0;c[n>>2]=j+1;a[(c[l>>2]|0)+j|0]=z;z=c[m>>2]|0;j=k>>>((16-z|0)>>>0)&65535;b[o>>1]=j;E=z-12|0;F=j}else{E=C+4|0;F=y}c[m>>2]=E;if((u|0)>0){y=f+20|0;j=f+8|0;z=0;k=E;l=F;while(1){n=e[f+2684+(d[12512+z|0]<<2)+2>>1]|0;G=n<<k|l&65535;H=G&65535;b[o>>1]=H;if((k|0)>13){I=c[y>>2]|0;c[y>>2]=I+1;a[(c[j>>2]|0)+I|0]=G&255;G=(e[o>>1]|0)>>>8&255;I=c[y>>2]|0;c[y>>2]=I+1;a[(c[j>>2]|0)+I|0]=G;G=c[m>>2]|0;I=n>>>((16-G|0)>>>0)&65535;b[o>>1]=I;J=G-13|0;K=I}else{J=k+3|0;K=H}c[m>>2]=J;H=z+1|0;if((H|0)<(u|0)){z=H;k=J;l=K}else{break}}}l=f+148|0;bB(f,l,t);k=f+2440|0;bB(f,k,p);bA(f,l,k)}else{bs(f,g,h,i)}}while(0);br(f);if((i|0)==0){return}i=f+5820|0;h=c[i>>2]|0;do{if((h|0)>8){g=f+5816|0;K=b[g>>1]&255;J=f+20|0;F=c[J>>2]|0;c[J>>2]=F+1;E=f+8|0;a[(c[E>>2]|0)+F|0]=K;K=(e[g>>1]|0)>>>8&255;F=c[J>>2]|0;c[J>>2]=F+1;a[(c[E>>2]|0)+F|0]=K;L=g}else{g=f+5816|0;if((h|0)<=0){L=g;break}K=b[g>>1]&255;F=f+20|0;E=c[F>>2]|0;c[F>>2]=E+1;a[(c[f+8>>2]|0)+E|0]=K;L=g}}while(0);b[L>>1]=0;c[i>>2]=0;return}function bz(f,g){f=f|0;g=g|0;var h=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0,X=0,Y=0,_=0;h=i;i=i+32|0;j=h|0;k=g|0;l=c[k>>2]|0;m=g+8|0;n=c[m>>2]|0;o=c[n>>2]|0;p=c[n+12>>2]|0;n=f+5200|0;c[n>>2]=0;q=f+5204|0;c[q>>2]=573;if((p|0)>0){r=0;s=-1;while(1){if((b[l+(r<<2)>>1]|0)==0){b[l+(r<<2)+2>>1]=0;t=s}else{u=(c[n>>2]|0)+1|0;c[n>>2]=u;c[f+2908+(u<<2)>>2]=r;a[f+5208+r|0]=0;t=r}u=r+1|0;if((u|0)<(p|0)){r=u;s=t}else{break}}s=c[n>>2]|0;if((s|0)<2){v=s;w=t;x=1694}else{y=t}}else{v=0;w=-1;x=1694}if((x|0)==1694){x=f+5800|0;t=f+5804|0;if((o|0)==0){s=w;r=v;while(1){u=(s|0)<2;z=s+1|0;A=u?z:s;B=u?z:0;z=r+1|0;c[n>>2]=z;c[f+2908+(z<<2)>>2]=B;b[l+(B<<2)>>1]=1;a[f+5208+B|0]=0;c[x>>2]=(c[x>>2]|0)-1;B=c[n>>2]|0;if((B|0)<2){s=A;r=B}else{y=A;break}}}else{r=w;w=v;while(1){v=(r|0)<2;s=r+1|0;A=v?s:r;B=v?s:0;s=w+1|0;c[n>>2]=s;c[f+2908+(s<<2)>>2]=B;b[l+(B<<2)>>1]=1;a[f+5208+B|0]=0;c[x>>2]=(c[x>>2]|0)-1;c[t>>2]=(c[t>>2]|0)-(e[o+(B<<2)+2>>1]|0);B=c[n>>2]|0;if((B|0)<2){r=A;w=B}else{y=A;break}}}}w=g+4|0;c[w>>2]=y;g=c[n>>2]|0;if((g|0)>1){r=(g|0)/2|0;o=g;while(1){t=c[f+2908+(r<<2)>>2]|0;x=f+5208+t|0;A=r<<1;L2348:do{if((A|0)>(o|0)){C=r}else{B=l+(t<<2)|0;s=r;v=A;z=o;while(1){do{if((v|0)<(z|0)){u=v|1;D=c[f+2908+(u<<2)>>2]|0;E=b[l+(D<<2)>>1]|0;F=c[f+2908+(v<<2)>>2]|0;G=b[l+(F<<2)>>1]|0;if((E&65535)>=(G&65535)){if(E<<16>>16!=G<<16>>16){H=v;break}if((d[f+5208+D|0]|0)>(d[f+5208+F|0]|0)){H=v;break}}H=u}else{H=v}}while(0);u=b[B>>1]|0;F=c[f+2908+(H<<2)>>2]|0;D=b[l+(F<<2)>>1]|0;if((u&65535)<(D&65535)){C=s;break L2348}if(u<<16>>16==D<<16>>16){if((d[x]|0)<=(d[f+5208+F|0]|0)){C=s;break L2348}}c[f+2908+(s<<2)>>2]=F;F=H<<1;D=c[n>>2]|0;if((F|0)>(D|0)){C=H;break}else{s=H;v=F;z=D}}}}while(0);c[f+2908+(C<<2)>>2]=t;x=r-1|0;A=c[n>>2]|0;if((x|0)>0){r=x;o=A}else{I=A;break}}}else{I=g}g=f+2912|0;o=p;p=I;while(1){I=c[g>>2]|0;r=p-1|0;c[n>>2]=r;C=c[f+2908+(p<<2)>>2]|0;c[g>>2]=C;H=f+5208+C|0;L2367:do{if((r|0)<2){J=1}else{A=l+(C<<2)|0;x=1;z=2;v=r;while(1){do{if((z|0)<(v|0)){s=z|1;B=c[f+2908+(s<<2)>>2]|0;D=b[l+(B<<2)>>1]|0;F=c[f+2908+(z<<2)>>2]|0;u=b[l+(F<<2)>>1]|0;if((D&65535)>=(u&65535)){if(D<<16>>16!=u<<16>>16){K=z;break}if((d[f+5208+B|0]|0)>(d[f+5208+F|0]|0)){K=z;break}}K=s}else{K=z}}while(0);s=b[A>>1]|0;F=c[f+2908+(K<<2)>>2]|0;B=b[l+(F<<2)>>1]|0;if((s&65535)<(B&65535)){J=x;break L2367}if(s<<16>>16==B<<16>>16){if((d[H]|0)<=(d[f+5208+F|0]|0)){J=x;break L2367}}c[f+2908+(x<<2)>>2]=F;F=K<<1;B=c[n>>2]|0;if((F|0)>(B|0)){J=K;break}else{x=K;z=F;v=B}}}}while(0);c[f+2908+(J<<2)>>2]=C;H=c[g>>2]|0;r=(c[q>>2]|0)-1|0;c[q>>2]=r;c[f+2908+(r<<2)>>2]=I;r=(c[q>>2]|0)-1|0;c[q>>2]=r;c[f+2908+(r<<2)>>2]=H;r=l+(o<<2)|0;b[r>>1]=(b[l+(H<<2)>>1]|0)+(b[l+(I<<2)>>1]|0)&65535;t=a[f+5208+I|0]|0;v=a[f+5208+H|0]|0;z=f+5208+o|0;a[z]=((t&255)<(v&255)?v:t)+1&255;t=o&65535;b[l+(H<<2)+2>>1]=t;b[l+(I<<2)+2>>1]=t;t=o+1|0;c[g>>2]=o;H=c[n>>2]|0;L2383:do{if((H|0)<2){L=1}else{v=1;x=2;A=H;while(1){do{if((x|0)<(A|0)){B=x|1;F=c[f+2908+(B<<2)>>2]|0;s=b[l+(F<<2)>>1]|0;u=c[f+2908+(x<<2)>>2]|0;D=b[l+(u<<2)>>1]|0;if((s&65535)>=(D&65535)){if(s<<16>>16!=D<<16>>16){M=x;break}if((d[f+5208+F|0]|0)>(d[f+5208+u|0]|0)){M=x;break}}M=B}else{M=x}}while(0);B=b[r>>1]|0;u=c[f+2908+(M<<2)>>2]|0;F=b[l+(u<<2)>>1]|0;if((B&65535)<(F&65535)){L=v;break L2383}if(B<<16>>16==F<<16>>16){if((d[z]|0)<=(d[f+5208+u|0]|0)){L=v;break L2383}}c[f+2908+(v<<2)>>2]=u;u=M<<1;F=c[n>>2]|0;if((u|0)>(F|0)){L=M;break}else{v=M;x=u;A=F}}}}while(0);c[f+2908+(L<<2)>>2]=o;z=c[n>>2]|0;if((z|0)>1){o=t;p=z}else{break}}p=c[g>>2]|0;g=(c[q>>2]|0)-1|0;c[q>>2]=g;c[f+2908+(g<<2)>>2]=p;p=c[k>>2]|0;k=c[w>>2]|0;w=c[m>>2]|0;m=c[w>>2]|0;g=c[w+4>>2]|0;o=c[w+8>>2]|0;n=c[w+16>>2]|0;w=f+2876|0;bI(w|0,0,32);b[p+(c[f+2908+(c[q>>2]<<2)>>2]<<2)+2>>1]=0;L=(c[q>>2]|0)+1|0;L2399:do{if((L|0)<573){q=f+5800|0;M=f+5804|0;if((m|0)==0){J=0;K=L;while(1){z=c[f+2908+(K<<2)>>2]|0;r=p+(z<<2)+2|0;H=(e[p+(e[r>>1]<<2)+2>>1]|0)+1|0;I=(H|0)>(n|0);C=I?n:H;H=(I&1)+J|0;b[r>>1]=C&65535;if((z|0)<=(k|0)){r=f+2876+(C<<1)|0;b[r>>1]=(b[r>>1]|0)+1&65535;if((z|0)<(o|0)){N=0}else{N=c[g+(z-o<<2)>>2]|0}r=Z(e[p+(z<<2)>>1]|0,N+C|0)|0;c[q>>2]=r+(c[q>>2]|0)}r=K+1|0;if((r|0)<573){J=H;K=r}else{O=H;break}}}else{K=0;J=L;while(1){t=c[f+2908+(J<<2)>>2]|0;H=p+(t<<2)+2|0;r=(e[p+(e[H>>1]<<2)+2>>1]|0)+1|0;C=(r|0)>(n|0);z=C?n:r;r=(C&1)+K|0;b[H>>1]=z&65535;if((t|0)<=(k|0)){H=f+2876+(z<<1)|0;b[H>>1]=(b[H>>1]|0)+1&65535;if((t|0)<(o|0)){P=0}else{P=c[g+(t-o<<2)>>2]|0}H=e[p+(t<<2)>>1]|0;C=Z(H,P+z|0)|0;c[q>>2]=C+(c[q>>2]|0);C=Z((e[m+(t<<2)+2>>1]|0)+P|0,H)|0;c[M>>2]=C+(c[M>>2]|0)}C=J+1|0;if((C|0)<573){K=r;J=C}else{O=r;break}}}if((O|0)==0){break}J=f+2876+(n<<1)|0;K=O;do{M=n;while(1){r=M-1|0;Q=f+2876+(r<<1)|0;R=b[Q>>1]|0;if(R<<16>>16==0){M=r}else{break}}b[Q>>1]=R-1&65535;r=f+2876+(M<<1)|0;b[r>>1]=(b[r>>1]|0)+2&65535;S=(b[J>>1]|0)-1&65535;b[J>>1]=S;K=K-2|0;}while((K|0)>0);if((n|0)==0){break}else{T=n;U=573;V=S}while(1){K=T&65535;if(V<<16>>16==0){W=U}else{J=V&65535;r=U;while(1){C=r;do{C=C-1|0;X=c[f+2908+(C<<2)>>2]|0;}while((X|0)>(k|0));H=p+(X<<2)+2|0;t=e[H>>1]|0;if((t|0)!=(T|0)){z=Z(e[p+(X<<2)>>1]|0,T-t|0)|0;c[q>>2]=z+(c[q>>2]|0);b[H>>1]=K}H=J-1|0;if((H|0)==0){W=C;break}else{J=H;r=C}}}r=T-1|0;if((r|0)==0){break L2399}T=r;U=W;V=b[f+2876+(r<<1)>>1]|0}}}while(0);V=b[w>>1]<<1;b[j+2>>1]=V;w=((b[f+2878>>1]|0)+V&65535)<<1;b[j+4>>1]=w;V=(w+(b[f+2880>>1]|0)&65535)<<1;b[j+6>>1]=V;w=(V+(b[f+2882>>1]|0)&65535)<<1;b[j+8>>1]=w;V=(w+(b[f+2884>>1]|0)&65535)<<1;b[j+10>>1]=V;w=(V+(b[f+2886>>1]|0)&65535)<<1;b[j+12>>1]=w;V=(w+(b[f+2888>>1]|0)&65535)<<1;b[j+14>>1]=V;w=(V+(b[f+2890>>1]|0)&65535)<<1;b[j+16>>1]=w;V=(w+(b[f+2892>>1]|0)&65535)<<1;b[j+18>>1]=V;w=(V+(b[f+2894>>1]|0)&65535)<<1;b[j+20>>1]=w;V=(w+(b[f+2896>>1]|0)&65535)<<1;b[j+22>>1]=V;w=(V+(b[f+2898>>1]|0)&65535)<<1;b[j+24>>1]=w;V=(w+(b[f+2900>>1]|0)&65535)<<1;b[j+26>>1]=V;w=(V+(b[f+2902>>1]|0)&65535)<<1;b[j+28>>1]=w;b[j+30>>1]=(w+(b[f+2904>>1]|0)&65535)<<1;if((y|0)<0){i=h;return}else{Y=0}do{f=b[l+(Y<<2)+2>>1]|0;w=f&65535;if(f<<16>>16!=0){f=j+(w<<1)|0;V=b[f>>1]|0;b[f>>1]=V+1&65535;f=0;W=w;w=V&65535;while(1){_=f|w&1;V=W-1|0;if((V|0)>0){f=_<<1;W=V;w=w>>>1}else{break}}b[l+(Y<<2)>>1]=_&65535}Y=Y+1|0;}while((Y|0)<=(y|0));i=h;return}function bA(f,g,h){f=f|0;g=g|0;h=h|0;var i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0;i=f+5792|0;if((c[i>>2]|0)==0){j=c[f+5820>>2]|0;k=b[f+5816>>1]|0}else{l=f+5796|0;m=f+5784|0;n=f+5820|0;o=f+5816|0;p=f+20|0;q=f+8|0;r=0;while(1){s=b[(c[l>>2]|0)+(r<<1)>>1]|0;t=s&65535;u=r+1|0;v=d[(c[m>>2]|0)+r|0]|0;do{if(s<<16>>16==0){w=e[g+(v<<2)+2>>1]|0;x=c[n>>2]|0;y=e[g+(v<<2)>>1]|0;z=e[o>>1]|0|y<<x;A=z&65535;b[o>>1]=A;if((x|0)>(16-w|0)){B=c[p>>2]|0;c[p>>2]=B+1;a[(c[q>>2]|0)+B|0]=z&255;z=(e[o>>1]|0)>>>8&255;B=c[p>>2]|0;c[p>>2]=B+1;a[(c[q>>2]|0)+B|0]=z;z=c[n>>2]|0;B=y>>>((16-z|0)>>>0)&65535;b[o>>1]=B;y=w-16+z|0;c[n>>2]=y;C=y;D=B;break}else{B=x+w|0;c[n>>2]=B;C=B;D=A;break}}else{A=d[13928+v|0]|0;B=(A|256)+1|0;w=e[g+(B<<2)+2>>1]|0;x=c[n>>2]|0;y=e[g+(B<<2)>>1]|0;B=e[o>>1]|0|y<<x;z=B&65535;b[o>>1]=z;if((x|0)>(16-w|0)){E=c[p>>2]|0;c[p>>2]=E+1;a[(c[q>>2]|0)+E|0]=B&255;B=(e[o>>1]|0)>>>8&255;E=c[p>>2]|0;c[p>>2]=E+1;a[(c[q>>2]|0)+E|0]=B;B=c[n>>2]|0;E=y>>>((16-B|0)>>>0)&65535;b[o>>1]=E;F=w-16+B|0;G=E}else{F=x+w|0;G=z}c[n>>2]=F;z=c[3880+(A<<2)>>2]|0;do{if((A-8|0)>>>0<20){w=v-(c[12536+(A<<2)>>2]|0)&65535;x=w<<F|G&65535;E=x&65535;b[o>>1]=E;if((F|0)>(16-z|0)){B=c[p>>2]|0;c[p>>2]=B+1;a[(c[q>>2]|0)+B|0]=x&255;x=(e[o>>1]|0)>>>8&255;B=c[p>>2]|0;c[p>>2]=B+1;a[(c[q>>2]|0)+B|0]=x;x=c[n>>2]|0;B=w>>>((16-x|0)>>>0)&65535;b[o>>1]=B;w=z-16+x|0;c[n>>2]=w;H=w;I=B;break}else{B=F+z|0;c[n>>2]=B;H=B;I=E;break}}else{H=F;I=G}}while(0);z=t-1|0;if(z>>>0<256){J=z}else{J=(z>>>7)+256|0}A=d[14184+J|0]|0;E=e[h+(A<<2)+2>>1]|0;B=e[h+(A<<2)>>1]|0;w=I&65535|B<<H;x=w&65535;b[o>>1]=x;if((H|0)>(16-E|0)){y=c[p>>2]|0;c[p>>2]=y+1;a[(c[q>>2]|0)+y|0]=w&255;w=(e[o>>1]|0)>>>8&255;y=c[p>>2]|0;c[p>>2]=y+1;a[(c[q>>2]|0)+y|0]=w;w=c[n>>2]|0;y=B>>>((16-w|0)>>>0)&65535;b[o>>1]=y;K=E-16+w|0;L=y}else{K=H+E|0;L=x}c[n>>2]=K;x=c[4e3+(A<<2)>>2]|0;if((A-4|0)>>>0>=26){C=K;D=L;break}E=z-(c[12656+(A<<2)>>2]|0)&65535;A=E<<K|L&65535;z=A&65535;b[o>>1]=z;if((K|0)>(16-x|0)){y=c[p>>2]|0;c[p>>2]=y+1;a[(c[q>>2]|0)+y|0]=A&255;A=(e[o>>1]|0)>>>8&255;y=c[p>>2]|0;c[p>>2]=y+1;a[(c[q>>2]|0)+y|0]=A;A=c[n>>2]|0;y=E>>>((16-A|0)>>>0)&65535;b[o>>1]=y;E=x-16+A|0;c[n>>2]=E;C=E;D=y;break}else{y=K+x|0;c[n>>2]=y;C=y;D=z;break}}}while(0);if(u>>>0<(c[i>>2]|0)>>>0){r=u}else{j=C;k=D;break}}}D=g+1026|0;C=e[D>>1]|0;r=f+5820|0;i=e[g+1024>>1]|0;g=f+5816|0;n=k&65535|i<<j;b[g>>1]=n&65535;if((j|0)>(16-C|0)){k=f+20|0;K=c[k>>2]|0;c[k>>2]=K+1;o=f+8|0;a[(c[o>>2]|0)+K|0]=n&255;n=(e[g>>1]|0)>>>8&255;K=c[k>>2]|0;c[k>>2]=K+1;a[(c[o>>2]|0)+K|0]=n;n=c[r>>2]|0;b[g>>1]=i>>>((16-n|0)>>>0)&65535;M=C-16+n|0;c[r>>2]=M;N=b[D>>1]|0;O=N&65535;P=f+5812|0;c[P>>2]=O;return}else{M=j+C|0;c[r>>2]=M;N=b[D>>1]|0;O=N&65535;P=f+5812|0;c[P>>2]=O;return}}function bB(d,f,g){d=d|0;f=f|0;g=g|0;var h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0,X=0,Y=0;h=b[f+2>>1]|0;i=h<<16>>16==0;j=d+2754|0;k=d+5820|0;l=d+2752|0;m=d+5816|0;n=d+20|0;o=d+8|0;p=d+2758|0;q=d+2756|0;r=d+2750|0;s=d+2748|0;t=0;u=-1;v=h&65535;h=i?138:7;w=i?3:4;L2493:while(1){i=t;x=0;while(1){if((i|0)>(g|0)){break L2493}y=i+1|0;z=b[f+(y<<2)+2>>1]|0;A=z&65535;B=x+1|0;C=(v|0)==(A|0);if((B|0)<(h|0)&C){i=y;x=B}else{break}}do{if((B|0)<(w|0)){i=d+2684+(v<<2)+2|0;D=d+2684+(v<<2)|0;E=B;F=c[k>>2]|0;G=b[m>>1]|0;while(1){H=e[i>>1]|0;I=e[D>>1]|0;J=G&65535|I<<F;K=J&65535;b[m>>1]=K;if((F|0)>(16-H|0)){L=c[n>>2]|0;c[n>>2]=L+1;a[(c[o>>2]|0)+L|0]=J&255;J=(e[m>>1]|0)>>>8&255;L=c[n>>2]|0;c[n>>2]=L+1;a[(c[o>>2]|0)+L|0]=J;J=c[k>>2]|0;L=I>>>((16-J|0)>>>0)&65535;b[m>>1]=L;M=H-16+J|0;N=L}else{M=F+H|0;N=K}c[k>>2]=M;K=E-1|0;if((K|0)==0){break}else{E=K;F=M;G=N}}}else{if((v|0)!=0){if((v|0)==(u|0)){O=B;P=c[k>>2]|0;Q=b[m>>1]|0}else{G=e[d+2684+(v<<2)+2>>1]|0;F=c[k>>2]|0;E=e[d+2684+(v<<2)>>1]|0;D=e[m>>1]|0|E<<F;i=D&65535;b[m>>1]=i;if((F|0)>(16-G|0)){K=c[n>>2]|0;c[n>>2]=K+1;a[(c[o>>2]|0)+K|0]=D&255;D=(e[m>>1]|0)>>>8&255;K=c[n>>2]|0;c[n>>2]=K+1;a[(c[o>>2]|0)+K|0]=D;D=c[k>>2]|0;K=E>>>((16-D|0)>>>0)&65535;b[m>>1]=K;R=G-16+D|0;S=K}else{R=F+G|0;S=i}c[k>>2]=R;O=x;P=R;Q=S}i=e[r>>1]|0;G=e[s>>1]|0;F=Q&65535|G<<P;K=F&65535;b[m>>1]=K;if((P|0)>(16-i|0)){D=c[n>>2]|0;c[n>>2]=D+1;a[(c[o>>2]|0)+D|0]=F&255;F=(e[m>>1]|0)>>>8&255;D=c[n>>2]|0;c[n>>2]=D+1;a[(c[o>>2]|0)+D|0]=F;F=c[k>>2]|0;D=G>>>((16-F|0)>>>0)&65535;b[m>>1]=D;T=i-16+F|0;U=D}else{T=P+i|0;U=K}c[k>>2]=T;K=O+65533&65535;i=U&65535|K<<T;b[m>>1]=i&65535;if((T|0)>14){D=c[n>>2]|0;c[n>>2]=D+1;a[(c[o>>2]|0)+D|0]=i&255;i=(e[m>>1]|0)>>>8&255;D=c[n>>2]|0;c[n>>2]=D+1;a[(c[o>>2]|0)+D|0]=i;i=c[k>>2]|0;b[m>>1]=K>>>((16-i|0)>>>0)&65535;c[k>>2]=i-14;break}else{c[k>>2]=T+2;break}}if((B|0)<11){i=e[j>>1]|0;K=c[k>>2]|0;D=e[l>>1]|0;F=e[m>>1]|0|D<<K;G=F&65535;b[m>>1]=G;if((K|0)>(16-i|0)){E=c[n>>2]|0;c[n>>2]=E+1;a[(c[o>>2]|0)+E|0]=F&255;F=(e[m>>1]|0)>>>8&255;E=c[n>>2]|0;c[n>>2]=E+1;a[(c[o>>2]|0)+E|0]=F;F=c[k>>2]|0;E=D>>>((16-F|0)>>>0)&65535;b[m>>1]=E;V=i-16+F|0;W=E}else{V=K+i|0;W=G}c[k>>2]=V;G=x+65534&65535;i=W&65535|G<<V;b[m>>1]=i&65535;if((V|0)>13){K=c[n>>2]|0;c[n>>2]=K+1;a[(c[o>>2]|0)+K|0]=i&255;i=(e[m>>1]|0)>>>8&255;K=c[n>>2]|0;c[n>>2]=K+1;a[(c[o>>2]|0)+K|0]=i;i=c[k>>2]|0;b[m>>1]=G>>>((16-i|0)>>>0)&65535;c[k>>2]=i-13;break}else{c[k>>2]=V+3;break}}else{i=e[p>>1]|0;G=c[k>>2]|0;K=e[q>>1]|0;E=e[m>>1]|0|K<<G;F=E&65535;b[m>>1]=F;if((G|0)>(16-i|0)){D=c[n>>2]|0;c[n>>2]=D+1;a[(c[o>>2]|0)+D|0]=E&255;E=(e[m>>1]|0)>>>8&255;D=c[n>>2]|0;c[n>>2]=D+1;a[(c[o>>2]|0)+D|0]=E;E=c[k>>2]|0;D=K>>>((16-E|0)>>>0)&65535;b[m>>1]=D;X=i-16+E|0;Y=D}else{X=G+i|0;Y=F}c[k>>2]=X;F=x+65526&65535;i=Y&65535|F<<X;b[m>>1]=i&65535;if((X|0)>9){G=c[n>>2]|0;c[n>>2]=G+1;a[(c[o>>2]|0)+G|0]=i&255;i=(e[m>>1]|0)>>>8&255;G=c[n>>2]|0;c[n>>2]=G+1;a[(c[o>>2]|0)+G|0]=i;i=c[k>>2]|0;b[m>>1]=F>>>((16-i|0)>>>0)&65535;c[k>>2]=i-9;break}else{c[k>>2]=X+7;break}}}}while(0);if(z<<16>>16==0){t=y;u=v;v=A;h=138;w=3;continue}t=y;u=v;v=A;h=C?6:7;w=C?3:4}return}function bC(a,c,d){a=a|0;c=c|0;d=d|0;var f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0;f=b[c+2>>1]|0;g=f<<16>>16==0;b[c+(d+1<<2)+2>>1]=-1;h=a+2752|0;i=a+2756|0;j=a+2748|0;k=g?3:4;l=g?138:7;g=f&65535;f=0;m=-1;L2547:while(1){n=0;o=f;do{if((o|0)>(d|0)){break L2547}o=o+1|0;p=b[c+(o<<2)+2>>1]|0;q=p&65535;n=n+1|0;r=(g|0)==(q|0);}while((n|0)<(l|0)&r);do{if((n|0)<(k|0)){s=a+2684+(g<<2)|0;b[s>>1]=(e[s>>1]|0)+n&65535}else{if((g|0)==0){if((n|0)<11){b[h>>1]=(b[h>>1]|0)+1&65535;break}else{b[i>>1]=(b[i>>1]|0)+1&65535;break}}else{if((g|0)!=(m|0)){s=a+2684+(g<<2)|0;b[s>>1]=(b[s>>1]|0)+1&65535}b[j>>1]=(b[j>>1]|0)+1&65535;break}}}while(0);if(p<<16>>16==0){k=3;l=138;m=g;g=q;f=o;continue}k=r?3:4;l=r?6:7;m=g;g=q;f=o}return}function bD(a,b,c){a=a|0;b=b|0;c=c|0;return bF(Z(c,b)|0)|0}function bE(a,b){a=a|0;b=b|0;bG(b);return}function bF(a){a=a|0;var b=0,d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0,P=0,Q=0,R=0,S=0,T=0,U=0,V=0,W=0,X=0,Y=0,Z=0,_=0,$=0,aa=0,ab=0,ac=0,ad=0,ae=0,af=0,ag=0,ah=0,ai=0,aj=0,ak=0,an=0,ao=0,ap=0,aq=0,ar=0,as=0,at=0,au=0,av=0,ax=0,az=0,aA=0,aB=0,aD=0,aE=0,aF=0,aG=0,aH=0,aI=0,aJ=0,aK=0,aL=0;do{if(a>>>0<245){if(a>>>0<11){b=16}else{b=a+11&-8}d=b>>>3;e=c[3682]|0;f=e>>>(d>>>0);if((f&3|0)!=0){g=(f&1^1)+d|0;h=g<<1;i=14768+(h<<2)|0;j=14768+(h+2<<2)|0;h=c[j>>2]|0;k=h+8|0;l=c[k>>2]|0;do{if((i|0)==(l|0)){c[3682]=e&~(1<<g)}else{if(l>>>0<(c[3686]|0)>>>0){am();return 0}m=l+12|0;if((c[m>>2]|0)==(h|0)){c[m>>2]=i;c[j>>2]=l;break}else{am();return 0}}}while(0);l=g<<3;c[h+4>>2]=l|3;j=h+(l|4)|0;c[j>>2]=c[j>>2]|1;n=k;return n|0}if(b>>>0<=(c[3684]|0)>>>0){o=b;break}if((f|0)!=0){j=2<<d;l=f<<d&(j|-j);j=(l&-l)-1|0;l=j>>>12&16;i=j>>>(l>>>0);j=i>>>5&8;m=i>>>(j>>>0);i=m>>>2&4;p=m>>>(i>>>0);m=p>>>1&2;q=p>>>(m>>>0);p=q>>>1&1;r=(j|l|i|m|p)+(q>>>(p>>>0))|0;p=r<<1;q=14768+(p<<2)|0;m=14768+(p+2<<2)|0;p=c[m>>2]|0;i=p+8|0;l=c[i>>2]|0;do{if((q|0)==(l|0)){c[3682]=e&~(1<<r)}else{if(l>>>0<(c[3686]|0)>>>0){am();return 0}j=l+12|0;if((c[j>>2]|0)==(p|0)){c[j>>2]=q;c[m>>2]=l;break}else{am();return 0}}}while(0);l=r<<3;m=l-b|0;c[p+4>>2]=b|3;q=p;e=q+b|0;c[q+(b|4)>>2]=m|1;c[q+l>>2]=m;l=c[3684]|0;if((l|0)!=0){q=c[3687]|0;d=l>>>3;l=d<<1;f=14768+(l<<2)|0;k=c[3682]|0;h=1<<d;do{if((k&h|0)==0){c[3682]=k|h;s=f;t=14768+(l+2<<2)|0}else{d=14768+(l+2<<2)|0;g=c[d>>2]|0;if(g>>>0>=(c[3686]|0)>>>0){s=g;t=d;break}am();return 0}}while(0);c[t>>2]=q;c[s+12>>2]=q;c[q+8>>2]=s;c[q+12>>2]=f}c[3684]=m;c[3687]=e;n=i;return n|0}l=c[3683]|0;if((l|0)==0){o=b;break}h=(l&-l)-1|0;l=h>>>12&16;k=h>>>(l>>>0);h=k>>>5&8;p=k>>>(h>>>0);k=p>>>2&4;r=p>>>(k>>>0);p=r>>>1&2;d=r>>>(p>>>0);r=d>>>1&1;g=c[15032+((h|l|k|p|r)+(d>>>(r>>>0))<<2)>>2]|0;r=g;d=g;p=(c[g+4>>2]&-8)-b|0;while(1){g=c[r+16>>2]|0;if((g|0)==0){k=c[r+20>>2]|0;if((k|0)==0){break}else{u=k}}else{u=g}g=(c[u+4>>2]&-8)-b|0;k=g>>>0<p>>>0;r=u;d=k?u:d;p=k?g:p}r=d;i=c[3686]|0;if(r>>>0<i>>>0){am();return 0}e=r+b|0;m=e;if(r>>>0>=e>>>0){am();return 0}e=c[d+24>>2]|0;f=c[d+12>>2]|0;do{if((f|0)==(d|0)){q=d+20|0;g=c[q>>2]|0;if((g|0)==0){k=d+16|0;l=c[k>>2]|0;if((l|0)==0){v=0;break}else{w=l;x=k}}else{w=g;x=q}while(1){q=w+20|0;g=c[q>>2]|0;if((g|0)!=0){w=g;x=q;continue}q=w+16|0;g=c[q>>2]|0;if((g|0)==0){break}else{w=g;x=q}}if(x>>>0<i>>>0){am();return 0}else{c[x>>2]=0;v=w;break}}else{q=c[d+8>>2]|0;if(q>>>0<i>>>0){am();return 0}g=q+12|0;if((c[g>>2]|0)!=(d|0)){am();return 0}k=f+8|0;if((c[k>>2]|0)==(d|0)){c[g>>2]=f;c[k>>2]=q;v=f;break}else{am();return 0}}}while(0);L2648:do{if((e|0)!=0){f=d+28|0;i=15032+(c[f>>2]<<2)|0;do{if((d|0)==(c[i>>2]|0)){c[i>>2]=v;if((v|0)!=0){break}c[3683]=c[3683]&~(1<<c[f>>2]);break L2648}else{if(e>>>0<(c[3686]|0)>>>0){am();return 0}q=e+16|0;if((c[q>>2]|0)==(d|0)){c[q>>2]=v}else{c[e+20>>2]=v}if((v|0)==0){break L2648}}}while(0);if(v>>>0<(c[3686]|0)>>>0){am();return 0}c[v+24>>2]=e;f=c[d+16>>2]|0;do{if((f|0)!=0){if(f>>>0<(c[3686]|0)>>>0){am();return 0}else{c[v+16>>2]=f;c[f+24>>2]=v;break}}}while(0);f=c[d+20>>2]|0;if((f|0)==0){break}if(f>>>0<(c[3686]|0)>>>0){am();return 0}else{c[v+20>>2]=f;c[f+24>>2]=v;break}}}while(0);if(p>>>0<16){e=p+b|0;c[d+4>>2]=e|3;f=r+(e+4)|0;c[f>>2]=c[f>>2]|1}else{c[d+4>>2]=b|3;c[r+(b|4)>>2]=p|1;c[r+(p+b)>>2]=p;f=c[3684]|0;if((f|0)!=0){e=c[3687]|0;i=f>>>3;f=i<<1;q=14768+(f<<2)|0;k=c[3682]|0;g=1<<i;do{if((k&g|0)==0){c[3682]=k|g;y=q;z=14768+(f+2<<2)|0}else{i=14768+(f+2<<2)|0;l=c[i>>2]|0;if(l>>>0>=(c[3686]|0)>>>0){y=l;z=i;break}am();return 0}}while(0);c[z>>2]=e;c[y+12>>2]=e;c[e+8>>2]=y;c[e+12>>2]=q}c[3684]=p;c[3687]=m}f=d+8|0;if((f|0)==0){o=b;break}else{n=f}return n|0}else{if(a>>>0>4294967231){o=-1;break}f=a+11|0;g=f&-8;k=c[3683]|0;if((k|0)==0){o=g;break}r=-g|0;i=f>>>8;do{if((i|0)==0){A=0}else{if(g>>>0>16777215){A=31;break}f=(i+1048320|0)>>>16&8;l=i<<f;h=(l+520192|0)>>>16&4;j=l<<h;l=(j+245760|0)>>>16&2;B=14-(h|f|l)+(j<<l>>>15)|0;A=g>>>((B+7|0)>>>0)&1|B<<1}}while(0);i=c[15032+(A<<2)>>2]|0;L2696:do{if((i|0)==0){C=0;D=r;E=0}else{if((A|0)==31){F=0}else{F=25-(A>>>1)|0}d=0;m=r;p=i;q=g<<F;e=0;while(1){B=c[p+4>>2]&-8;l=B-g|0;if(l>>>0<m>>>0){if((B|0)==(g|0)){C=p;D=l;E=p;break L2696}else{G=p;H=l}}else{G=d;H=m}l=c[p+20>>2]|0;B=c[p+16+(q>>>31<<2)>>2]|0;j=(l|0)==0|(l|0)==(B|0)?e:l;if((B|0)==0){C=G;D=H;E=j;break}else{d=G;m=H;p=B;q=q<<1;e=j}}}}while(0);if((E|0)==0&(C|0)==0){i=2<<A;r=k&(i|-i);if((r|0)==0){o=g;break}i=(r&-r)-1|0;r=i>>>12&16;e=i>>>(r>>>0);i=e>>>5&8;q=e>>>(i>>>0);e=q>>>2&4;p=q>>>(e>>>0);q=p>>>1&2;m=p>>>(q>>>0);p=m>>>1&1;I=c[15032+((i|r|e|q|p)+(m>>>(p>>>0))<<2)>>2]|0}else{I=E}if((I|0)==0){J=D;K=C}else{p=I;m=D;q=C;while(1){e=(c[p+4>>2]&-8)-g|0;r=e>>>0<m>>>0;i=r?e:m;e=r?p:q;r=c[p+16>>2]|0;if((r|0)!=0){p=r;m=i;q=e;continue}r=c[p+20>>2]|0;if((r|0)==0){J=i;K=e;break}else{p=r;m=i;q=e}}}if((K|0)==0){o=g;break}if(J>>>0>=((c[3684]|0)-g|0)>>>0){o=g;break}q=K;m=c[3686]|0;if(q>>>0<m>>>0){am();return 0}p=q+g|0;k=p;if(q>>>0>=p>>>0){am();return 0}e=c[K+24>>2]|0;i=c[K+12>>2]|0;do{if((i|0)==(K|0)){r=K+20|0;d=c[r>>2]|0;if((d|0)==0){j=K+16|0;B=c[j>>2]|0;if((B|0)==0){L=0;break}else{M=B;N=j}}else{M=d;N=r}while(1){r=M+20|0;d=c[r>>2]|0;if((d|0)!=0){M=d;N=r;continue}r=M+16|0;d=c[r>>2]|0;if((d|0)==0){break}else{M=d;N=r}}if(N>>>0<m>>>0){am();return 0}else{c[N>>2]=0;L=M;break}}else{r=c[K+8>>2]|0;if(r>>>0<m>>>0){am();return 0}d=r+12|0;if((c[d>>2]|0)!=(K|0)){am();return 0}j=i+8|0;if((c[j>>2]|0)==(K|0)){c[d>>2]=i;c[j>>2]=r;L=i;break}else{am();return 0}}}while(0);L2746:do{if((e|0)!=0){i=K+28|0;m=15032+(c[i>>2]<<2)|0;do{if((K|0)==(c[m>>2]|0)){c[m>>2]=L;if((L|0)!=0){break}c[3683]=c[3683]&~(1<<c[i>>2]);break L2746}else{if(e>>>0<(c[3686]|0)>>>0){am();return 0}r=e+16|0;if((c[r>>2]|0)==(K|0)){c[r>>2]=L}else{c[e+20>>2]=L}if((L|0)==0){break L2746}}}while(0);if(L>>>0<(c[3686]|0)>>>0){am();return 0}c[L+24>>2]=e;i=c[K+16>>2]|0;do{if((i|0)!=0){if(i>>>0<(c[3686]|0)>>>0){am();return 0}else{c[L+16>>2]=i;c[i+24>>2]=L;break}}}while(0);i=c[K+20>>2]|0;if((i|0)==0){break}if(i>>>0<(c[3686]|0)>>>0){am();return 0}else{c[L+20>>2]=i;c[i+24>>2]=L;break}}}while(0);do{if(J>>>0<16){e=J+g|0;c[K+4>>2]=e|3;i=q+(e+4)|0;c[i>>2]=c[i>>2]|1}else{c[K+4>>2]=g|3;c[q+(g|4)>>2]=J|1;c[q+(J+g)>>2]=J;i=J>>>3;if(J>>>0<256){e=i<<1;m=14768+(e<<2)|0;r=c[3682]|0;j=1<<i;do{if((r&j|0)==0){c[3682]=r|j;O=m;P=14768+(e+2<<2)|0}else{i=14768+(e+2<<2)|0;d=c[i>>2]|0;if(d>>>0>=(c[3686]|0)>>>0){O=d;P=i;break}am();return 0}}while(0);c[P>>2]=k;c[O+12>>2]=k;c[q+(g+8)>>2]=O;c[q+(g+12)>>2]=m;break}e=p;j=J>>>8;do{if((j|0)==0){Q=0}else{if(J>>>0>16777215){Q=31;break}r=(j+1048320|0)>>>16&8;i=j<<r;d=(i+520192|0)>>>16&4;B=i<<d;i=(B+245760|0)>>>16&2;l=14-(d|r|i)+(B<<i>>>15)|0;Q=J>>>((l+7|0)>>>0)&1|l<<1}}while(0);j=15032+(Q<<2)|0;c[q+(g+28)>>2]=Q;c[q+(g+20)>>2]=0;c[q+(g+16)>>2]=0;m=c[3683]|0;l=1<<Q;if((m&l|0)==0){c[3683]=m|l;c[j>>2]=e;c[q+(g+24)>>2]=j;c[q+(g+12)>>2]=e;c[q+(g+8)>>2]=e;break}if((Q|0)==31){R=0}else{R=25-(Q>>>1)|0}l=J<<R;m=c[j>>2]|0;while(1){if((c[m+4>>2]&-8|0)==(J|0)){break}S=m+16+(l>>>31<<2)|0;j=c[S>>2]|0;if((j|0)==0){T=2011;break}else{l=l<<1;m=j}}if((T|0)==2011){if(S>>>0<(c[3686]|0)>>>0){am();return 0}else{c[S>>2]=e;c[q+(g+24)>>2]=m;c[q+(g+12)>>2]=e;c[q+(g+8)>>2]=e;break}}l=m+8|0;j=c[l>>2]|0;i=c[3686]|0;if(m>>>0<i>>>0){am();return 0}if(j>>>0<i>>>0){am();return 0}else{c[j+12>>2]=e;c[l>>2]=e;c[q+(g+8)>>2]=j;c[q+(g+12)>>2]=m;c[q+(g+24)>>2]=0;break}}}while(0);q=K+8|0;if((q|0)==0){o=g;break}else{n=q}return n|0}}while(0);K=c[3684]|0;if(o>>>0<=K>>>0){S=K-o|0;J=c[3687]|0;if(S>>>0>15){R=J;c[3687]=R+o;c[3684]=S;c[R+(o+4)>>2]=S|1;c[R+K>>2]=S;c[J+4>>2]=o|3}else{c[3684]=0;c[3687]=0;c[J+4>>2]=K|3;S=J+(K+4)|0;c[S>>2]=c[S>>2]|1}n=J+8|0;return n|0}J=c[3685]|0;if(o>>>0<J>>>0){S=J-o|0;c[3685]=S;J=c[3688]|0;K=J;c[3688]=K+o;c[K+(o+4)>>2]=S|1;c[J+4>>2]=o|3;n=J+8|0;return n|0}do{if((c[3674]|0)==0){J=al(8)|0;if((J-1&J|0)==0){c[3676]=J;c[3675]=J;c[3677]=-1;c[3678]=2097152;c[3679]=0;c[3793]=0;c[3674]=(aC(0)|0)&-16^1431655768;break}else{am();return 0}}}while(0);J=o+48|0;S=c[3676]|0;K=o+47|0;R=S+K|0;Q=-S|0;S=R&Q;if(S>>>0<=o>>>0){n=0;return n|0}O=c[3792]|0;do{if((O|0)!=0){P=c[3790]|0;L=P+S|0;if(L>>>0<=P>>>0|L>>>0>O>>>0){n=0}else{break}return n|0}}while(0);L2838:do{if((c[3793]&4|0)==0){O=c[3688]|0;L2840:do{if((O|0)==0){T=2041}else{L=O;P=15176;while(1){U=P|0;M=c[U>>2]|0;if(M>>>0<=L>>>0){V=P+4|0;if((M+(c[V>>2]|0)|0)>>>0>L>>>0){break}}M=c[P+8>>2]|0;if((M|0)==0){T=2041;break L2840}else{P=M}}if((P|0)==0){T=2041;break}L=R-(c[3685]|0)&Q;if(L>>>0>=2147483647){W=0;break}m=aw(L|0)|0;e=(m|0)==((c[U>>2]|0)+(c[V>>2]|0)|0);X=e?m:-1;Y=e?L:0;Z=m;_=L;T=2050}}while(0);do{if((T|0)==2041){O=aw(0)|0;if((O|0)==-1){W=0;break}g=O;L=c[3675]|0;m=L-1|0;if((m&g|0)==0){$=S}else{$=S-g+(m+g&-L)|0}L=c[3790]|0;g=L+$|0;if(!($>>>0>o>>>0&$>>>0<2147483647)){W=0;break}m=c[3792]|0;if((m|0)!=0){if(g>>>0<=L>>>0|g>>>0>m>>>0){W=0;break}}m=aw($|0)|0;g=(m|0)==(O|0);X=g?O:-1;Y=g?$:0;Z=m;_=$;T=2050}}while(0);L2860:do{if((T|0)==2050){m=-_|0;if((X|0)!=-1){aa=Y;ab=X;T=2061;break L2838}do{if((Z|0)!=-1&_>>>0<2147483647&_>>>0<J>>>0){g=c[3676]|0;O=K-_+g&-g;if(O>>>0>=2147483647){ac=_;break}if((aw(O|0)|0)==-1){aw(m|0)|0;W=Y;break L2860}else{ac=O+_|0;break}}else{ac=_}}while(0);if((Z|0)==-1){W=Y}else{aa=ac;ab=Z;T=2061;break L2838}}}while(0);c[3793]=c[3793]|4;ad=W;T=2058}else{ad=0;T=2058}}while(0);do{if((T|0)==2058){if(S>>>0>=2147483647){break}W=aw(S|0)|0;Z=aw(0)|0;if(!((Z|0)!=-1&(W|0)!=-1&W>>>0<Z>>>0)){break}ac=Z-W|0;Z=ac>>>0>(o+40|0)>>>0;Y=Z?W:-1;if((Y|0)!=-1){aa=Z?ac:ad;ab=Y;T=2061}}}while(0);do{if((T|0)==2061){ad=(c[3790]|0)+aa|0;c[3790]=ad;if(ad>>>0>(c[3791]|0)>>>0){c[3791]=ad}ad=c[3688]|0;L2880:do{if((ad|0)==0){S=c[3686]|0;if((S|0)==0|ab>>>0<S>>>0){c[3686]=ab}c[3794]=ab;c[3795]=aa;c[3797]=0;c[3691]=c[3674];c[3690]=-1;S=0;do{Y=S<<1;ac=14768+(Y<<2)|0;c[14768+(Y+3<<2)>>2]=ac;c[14768+(Y+2<<2)>>2]=ac;S=S+1|0;}while(S>>>0<32);S=ab+8|0;if((S&7|0)==0){ae=0}else{ae=-S&7}S=aa-40-ae|0;c[3688]=ab+ae;c[3685]=S;c[ab+(ae+4)>>2]=S|1;c[ab+(aa-36)>>2]=40;c[3689]=c[3678]}else{S=15176;while(1){af=c[S>>2]|0;ag=S+4|0;ah=c[ag>>2]|0;if((ab|0)==(af+ah|0)){T=2073;break}ac=c[S+8>>2]|0;if((ac|0)==0){break}else{S=ac}}do{if((T|0)==2073){if((c[S+12>>2]&8|0)!=0){break}ac=ad;if(!(ac>>>0>=af>>>0&ac>>>0<ab>>>0)){break}c[ag>>2]=ah+aa;ac=c[3688]|0;Y=(c[3685]|0)+aa|0;Z=ac;W=ac+8|0;if((W&7|0)==0){ai=0}else{ai=-W&7}W=Y-ai|0;c[3688]=Z+ai;c[3685]=W;c[Z+(ai+4)>>2]=W|1;c[Z+(Y+4)>>2]=40;c[3689]=c[3678];break L2880}}while(0);if(ab>>>0<(c[3686]|0)>>>0){c[3686]=ab}S=ab+aa|0;Y=15176;while(1){aj=Y|0;if((c[aj>>2]|0)==(S|0)){T=2083;break}Z=c[Y+8>>2]|0;if((Z|0)==0){break}else{Y=Z}}do{if((T|0)==2083){if((c[Y+12>>2]&8|0)!=0){break}c[aj>>2]=ab;S=Y+4|0;c[S>>2]=(c[S>>2]|0)+aa;S=ab+8|0;if((S&7|0)==0){ak=0}else{ak=-S&7}S=ab+(aa+8)|0;if((S&7|0)==0){an=0}else{an=-S&7}S=ab+(an+aa)|0;Z=S;W=ak+o|0;ac=ab+W|0;_=ac;K=S-(ab+ak)-o|0;c[ab+(ak+4)>>2]=o|3;do{if((Z|0)==(c[3688]|0)){J=(c[3685]|0)+K|0;c[3685]=J;c[3688]=_;c[ab+(W+4)>>2]=J|1}else{if((Z|0)==(c[3687]|0)){J=(c[3684]|0)+K|0;c[3684]=J;c[3687]=_;c[ab+(W+4)>>2]=J|1;c[ab+(J+W)>>2]=J;break}J=aa+4|0;X=c[ab+(J+an)>>2]|0;if((X&3|0)==1){$=X&-8;V=X>>>3;L2915:do{if(X>>>0<256){U=c[ab+((an|8)+aa)>>2]|0;Q=c[ab+(aa+12+an)>>2]|0;R=14768+(V<<1<<2)|0;do{if((U|0)!=(R|0)){if(U>>>0<(c[3686]|0)>>>0){am();return 0}if((c[U+12>>2]|0)==(Z|0)){break}am();return 0}}while(0);if((Q|0)==(U|0)){c[3682]=c[3682]&~(1<<V);break}do{if((Q|0)==(R|0)){ao=Q+8|0}else{if(Q>>>0<(c[3686]|0)>>>0){am();return 0}m=Q+8|0;if((c[m>>2]|0)==(Z|0)){ao=m;break}am();return 0}}while(0);c[U+12>>2]=Q;c[ao>>2]=U}else{R=S;m=c[ab+((an|24)+aa)>>2]|0;P=c[ab+(aa+12+an)>>2]|0;do{if((P|0)==(R|0)){O=an|16;g=ab+(J+O)|0;L=c[g>>2]|0;if((L|0)==0){e=ab+(O+aa)|0;O=c[e>>2]|0;if((O|0)==0){ap=0;break}else{aq=O;ar=e}}else{aq=L;ar=g}while(1){g=aq+20|0;L=c[g>>2]|0;if((L|0)!=0){aq=L;ar=g;continue}g=aq+16|0;L=c[g>>2]|0;if((L|0)==0){break}else{aq=L;ar=g}}if(ar>>>0<(c[3686]|0)>>>0){am();return 0}else{c[ar>>2]=0;ap=aq;break}}else{g=c[ab+((an|8)+aa)>>2]|0;if(g>>>0<(c[3686]|0)>>>0){am();return 0}L=g+12|0;if((c[L>>2]|0)!=(R|0)){am();return 0}e=P+8|0;if((c[e>>2]|0)==(R|0)){c[L>>2]=P;c[e>>2]=g;ap=P;break}else{am();return 0}}}while(0);if((m|0)==0){break}P=ab+(aa+28+an)|0;U=15032+(c[P>>2]<<2)|0;do{if((R|0)==(c[U>>2]|0)){c[U>>2]=ap;if((ap|0)!=0){break}c[3683]=c[3683]&~(1<<c[P>>2]);break L2915}else{if(m>>>0<(c[3686]|0)>>>0){am();return 0}Q=m+16|0;if((c[Q>>2]|0)==(R|0)){c[Q>>2]=ap}else{c[m+20>>2]=ap}if((ap|0)==0){break L2915}}}while(0);if(ap>>>0<(c[3686]|0)>>>0){am();return 0}c[ap+24>>2]=m;R=an|16;P=c[ab+(R+aa)>>2]|0;do{if((P|0)!=0){if(P>>>0<(c[3686]|0)>>>0){am();return 0}else{c[ap+16>>2]=P;c[P+24>>2]=ap;break}}}while(0);P=c[ab+(J+R)>>2]|0;if((P|0)==0){break}if(P>>>0<(c[3686]|0)>>>0){am();return 0}else{c[ap+20>>2]=P;c[P+24>>2]=ap;break}}}while(0);as=ab+(($|an)+aa)|0;at=$+K|0}else{as=Z;at=K}J=as+4|0;c[J>>2]=c[J>>2]&-2;c[ab+(W+4)>>2]=at|1;c[ab+(at+W)>>2]=at;J=at>>>3;if(at>>>0<256){V=J<<1;X=14768+(V<<2)|0;P=c[3682]|0;m=1<<J;do{if((P&m|0)==0){c[3682]=P|m;au=X;av=14768+(V+2<<2)|0}else{J=14768+(V+2<<2)|0;U=c[J>>2]|0;if(U>>>0>=(c[3686]|0)>>>0){au=U;av=J;break}am();return 0}}while(0);c[av>>2]=_;c[au+12>>2]=_;c[ab+(W+8)>>2]=au;c[ab+(W+12)>>2]=X;break}V=ac;m=at>>>8;do{if((m|0)==0){ax=0}else{if(at>>>0>16777215){ax=31;break}P=(m+1048320|0)>>>16&8;$=m<<P;J=($+520192|0)>>>16&4;U=$<<J;$=(U+245760|0)>>>16&2;Q=14-(J|P|$)+(U<<$>>>15)|0;ax=at>>>((Q+7|0)>>>0)&1|Q<<1}}while(0);m=15032+(ax<<2)|0;c[ab+(W+28)>>2]=ax;c[ab+(W+20)>>2]=0;c[ab+(W+16)>>2]=0;X=c[3683]|0;Q=1<<ax;if((X&Q|0)==0){c[3683]=X|Q;c[m>>2]=V;c[ab+(W+24)>>2]=m;c[ab+(W+12)>>2]=V;c[ab+(W+8)>>2]=V;break}if((ax|0)==31){az=0}else{az=25-(ax>>>1)|0}Q=at<<az;X=c[m>>2]|0;while(1){if((c[X+4>>2]&-8|0)==(at|0)){break}aA=X+16+(Q>>>31<<2)|0;m=c[aA>>2]|0;if((m|0)==0){T=2156;break}else{Q=Q<<1;X=m}}if((T|0)==2156){if(aA>>>0<(c[3686]|0)>>>0){am();return 0}else{c[aA>>2]=V;c[ab+(W+24)>>2]=X;c[ab+(W+12)>>2]=V;c[ab+(W+8)>>2]=V;break}}Q=X+8|0;m=c[Q>>2]|0;$=c[3686]|0;if(X>>>0<$>>>0){am();return 0}if(m>>>0<$>>>0){am();return 0}else{c[m+12>>2]=V;c[Q>>2]=V;c[ab+(W+8)>>2]=m;c[ab+(W+12)>>2]=X;c[ab+(W+24)>>2]=0;break}}}while(0);n=ab+(ak|8)|0;return n|0}}while(0);Y=ad;W=15176;while(1){aB=c[W>>2]|0;if(aB>>>0<=Y>>>0){aD=c[W+4>>2]|0;aE=aB+aD|0;if(aE>>>0>Y>>>0){break}}W=c[W+8>>2]|0}W=aB+(aD-39)|0;if((W&7|0)==0){aF=0}else{aF=-W&7}W=aB+(aD-47+aF)|0;ac=W>>>0<(ad+16|0)>>>0?Y:W;W=ac+8|0;_=ab+8|0;if((_&7|0)==0){aG=0}else{aG=-_&7}_=aa-40-aG|0;c[3688]=ab+aG;c[3685]=_;c[ab+(aG+4)>>2]=_|1;c[ab+(aa-36)>>2]=40;c[3689]=c[3678];c[ac+4>>2]=27;c[W>>2]=c[3794];c[W+4>>2]=c[15180>>2];c[W+8>>2]=c[15184>>2];c[W+12>>2]=c[15188>>2];c[3794]=ab;c[3795]=aa;c[3797]=0;c[3796]=W;W=ac+28|0;c[W>>2]=7;if((ac+32|0)>>>0<aE>>>0){_=W;while(1){W=_+4|0;c[W>>2]=7;if((_+8|0)>>>0<aE>>>0){_=W}else{break}}}if((ac|0)==(Y|0)){break}_=ac-ad|0;W=Y+(_+4)|0;c[W>>2]=c[W>>2]&-2;c[ad+4>>2]=_|1;c[Y+_>>2]=_;W=_>>>3;if(_>>>0<256){K=W<<1;Z=14768+(K<<2)|0;S=c[3682]|0;m=1<<W;do{if((S&m|0)==0){c[3682]=S|m;aH=Z;aI=14768+(K+2<<2)|0}else{W=14768+(K+2<<2)|0;Q=c[W>>2]|0;if(Q>>>0>=(c[3686]|0)>>>0){aH=Q;aI=W;break}am();return 0}}while(0);c[aI>>2]=ad;c[aH+12>>2]=ad;c[ad+8>>2]=aH;c[ad+12>>2]=Z;break}K=ad;m=_>>>8;do{if((m|0)==0){aJ=0}else{if(_>>>0>16777215){aJ=31;break}S=(m+1048320|0)>>>16&8;Y=m<<S;ac=(Y+520192|0)>>>16&4;W=Y<<ac;Y=(W+245760|0)>>>16&2;Q=14-(ac|S|Y)+(W<<Y>>>15)|0;aJ=_>>>((Q+7|0)>>>0)&1|Q<<1}}while(0);m=15032+(aJ<<2)|0;c[ad+28>>2]=aJ;c[ad+20>>2]=0;c[ad+16>>2]=0;Z=c[3683]|0;Q=1<<aJ;if((Z&Q|0)==0){c[3683]=Z|Q;c[m>>2]=K;c[ad+24>>2]=m;c[ad+12>>2]=ad;c[ad+8>>2]=ad;break}if((aJ|0)==31){aK=0}else{aK=25-(aJ>>>1)|0}Q=_<<aK;Z=c[m>>2]|0;while(1){if((c[Z+4>>2]&-8|0)==(_|0)){break}aL=Z+16+(Q>>>31<<2)|0;m=c[aL>>2]|0;if((m|0)==0){T=2191;break}else{Q=Q<<1;Z=m}}if((T|0)==2191){if(aL>>>0<(c[3686]|0)>>>0){am();return 0}else{c[aL>>2]=K;c[ad+24>>2]=Z;c[ad+12>>2]=ad;c[ad+8>>2]=ad;break}}Q=Z+8|0;_=c[Q>>2]|0;m=c[3686]|0;if(Z>>>0<m>>>0){am();return 0}if(_>>>0<m>>>0){am();return 0}else{c[_+12>>2]=K;c[Q>>2]=K;c[ad+8>>2]=_;c[ad+12>>2]=Z;c[ad+24>>2]=0;break}}}while(0);ad=c[3685]|0;if(ad>>>0<=o>>>0){break}_=ad-o|0;c[3685]=_;ad=c[3688]|0;Q=ad;c[3688]=Q+o;c[Q+(o+4)>>2]=_|1;c[ad+4>>2]=o|3;n=ad+8|0;return n|0}}while(0);c[(ay()|0)>>2]=12;n=0;return n|0}function bG(a){a=a|0;var b=0,d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0,p=0,q=0,r=0,s=0,t=0,u=0,v=0,w=0,x=0,y=0,z=0,A=0,B=0,C=0,D=0,E=0,F=0,G=0,H=0,I=0,J=0,K=0,L=0,M=0,N=0,O=0;if((a|0)==0){return}b=a-8|0;d=b;e=c[3686]|0;if(b>>>0<e>>>0){am()}f=c[a-4>>2]|0;g=f&3;if((g|0)==1){am()}h=f&-8;i=a+(h-8)|0;j=i;L3097:do{if((f&1|0)==0){k=c[b>>2]|0;if((g|0)==0){return}l=-8-k|0;m=a+l|0;n=m;o=k+h|0;if(m>>>0<e>>>0){am()}if((n|0)==(c[3687]|0)){p=a+(h-4)|0;if((c[p>>2]&3|0)!=3){q=n;r=o;break}c[3684]=o;c[p>>2]=c[p>>2]&-2;c[a+(l+4)>>2]=o|1;c[i>>2]=o;return}p=k>>>3;if(k>>>0<256){k=c[a+(l+8)>>2]|0;s=c[a+(l+12)>>2]|0;t=14768+(p<<1<<2)|0;do{if((k|0)!=(t|0)){if(k>>>0<e>>>0){am()}if((c[k+12>>2]|0)==(n|0)){break}am()}}while(0);if((s|0)==(k|0)){c[3682]=c[3682]&~(1<<p);q=n;r=o;break}do{if((s|0)==(t|0)){u=s+8|0}else{if(s>>>0<e>>>0){am()}v=s+8|0;if((c[v>>2]|0)==(n|0)){u=v;break}am()}}while(0);c[k+12>>2]=s;c[u>>2]=k;q=n;r=o;break}t=m;p=c[a+(l+24)>>2]|0;v=c[a+(l+12)>>2]|0;do{if((v|0)==(t|0)){w=a+(l+20)|0;x=c[w>>2]|0;if((x|0)==0){y=a+(l+16)|0;z=c[y>>2]|0;if((z|0)==0){A=0;break}else{B=z;C=y}}else{B=x;C=w}while(1){w=B+20|0;x=c[w>>2]|0;if((x|0)!=0){B=x;C=w;continue}w=B+16|0;x=c[w>>2]|0;if((x|0)==0){break}else{B=x;C=w}}if(C>>>0<e>>>0){am()}else{c[C>>2]=0;A=B;break}}else{w=c[a+(l+8)>>2]|0;if(w>>>0<e>>>0){am()}x=w+12|0;if((c[x>>2]|0)!=(t|0)){am()}y=v+8|0;if((c[y>>2]|0)==(t|0)){c[x>>2]=v;c[y>>2]=w;A=v;break}else{am()}}}while(0);if((p|0)==0){q=n;r=o;break}v=a+(l+28)|0;m=15032+(c[v>>2]<<2)|0;do{if((t|0)==(c[m>>2]|0)){c[m>>2]=A;if((A|0)!=0){break}c[3683]=c[3683]&~(1<<c[v>>2]);q=n;r=o;break L3097}else{if(p>>>0<(c[3686]|0)>>>0){am()}k=p+16|0;if((c[k>>2]|0)==(t|0)){c[k>>2]=A}else{c[p+20>>2]=A}if((A|0)==0){q=n;r=o;break L3097}}}while(0);if(A>>>0<(c[3686]|0)>>>0){am()}c[A+24>>2]=p;t=c[a+(l+16)>>2]|0;do{if((t|0)!=0){if(t>>>0<(c[3686]|0)>>>0){am()}else{c[A+16>>2]=t;c[t+24>>2]=A;break}}}while(0);t=c[a+(l+20)>>2]|0;if((t|0)==0){q=n;r=o;break}if(t>>>0<(c[3686]|0)>>>0){am()}else{c[A+20>>2]=t;c[t+24>>2]=A;q=n;r=o;break}}else{q=d;r=h}}while(0);d=q;if(d>>>0>=i>>>0){am()}A=a+(h-4)|0;e=c[A>>2]|0;if((e&1|0)==0){am()}do{if((e&2|0)==0){if((j|0)==(c[3688]|0)){B=(c[3685]|0)+r|0;c[3685]=B;c[3688]=q;c[q+4>>2]=B|1;if((q|0)==(c[3687]|0)){c[3687]=0;c[3684]=0}if(B>>>0<=(c[3689]|0)>>>0){return}bH(0)|0;return}if((j|0)==(c[3687]|0)){B=(c[3684]|0)+r|0;c[3684]=B;c[3687]=q;c[q+4>>2]=B|1;c[d+B>>2]=B;return}B=(e&-8)+r|0;C=e>>>3;L3203:do{if(e>>>0<256){u=c[a+h>>2]|0;g=c[a+(h|4)>>2]|0;b=14768+(C<<1<<2)|0;do{if((u|0)!=(b|0)){if(u>>>0<(c[3686]|0)>>>0){am()}if((c[u+12>>2]|0)==(j|0)){break}am()}}while(0);if((g|0)==(u|0)){c[3682]=c[3682]&~(1<<C);break}do{if((g|0)==(b|0)){D=g+8|0}else{if(g>>>0<(c[3686]|0)>>>0){am()}f=g+8|0;if((c[f>>2]|0)==(j|0)){D=f;break}am()}}while(0);c[u+12>>2]=g;c[D>>2]=u}else{b=i;f=c[a+(h+16)>>2]|0;t=c[a+(h|4)>>2]|0;do{if((t|0)==(b|0)){p=a+(h+12)|0;v=c[p>>2]|0;if((v|0)==0){m=a+(h+8)|0;k=c[m>>2]|0;if((k|0)==0){E=0;break}else{F=k;G=m}}else{F=v;G=p}while(1){p=F+20|0;v=c[p>>2]|0;if((v|0)!=0){F=v;G=p;continue}p=F+16|0;v=c[p>>2]|0;if((v|0)==0){break}else{F=v;G=p}}if(G>>>0<(c[3686]|0)>>>0){am()}else{c[G>>2]=0;E=F;break}}else{p=c[a+h>>2]|0;if(p>>>0<(c[3686]|0)>>>0){am()}v=p+12|0;if((c[v>>2]|0)!=(b|0)){am()}m=t+8|0;if((c[m>>2]|0)==(b|0)){c[v>>2]=t;c[m>>2]=p;E=t;break}else{am()}}}while(0);if((f|0)==0){break}t=a+(h+20)|0;u=15032+(c[t>>2]<<2)|0;do{if((b|0)==(c[u>>2]|0)){c[u>>2]=E;if((E|0)!=0){break}c[3683]=c[3683]&~(1<<c[t>>2]);break L3203}else{if(f>>>0<(c[3686]|0)>>>0){am()}g=f+16|0;if((c[g>>2]|0)==(b|0)){c[g>>2]=E}else{c[f+20>>2]=E}if((E|0)==0){break L3203}}}while(0);if(E>>>0<(c[3686]|0)>>>0){am()}c[E+24>>2]=f;b=c[a+(h+8)>>2]|0;do{if((b|0)!=0){if(b>>>0<(c[3686]|0)>>>0){am()}else{c[E+16>>2]=b;c[b+24>>2]=E;break}}}while(0);b=c[a+(h+12)>>2]|0;if((b|0)==0){break}if(b>>>0<(c[3686]|0)>>>0){am()}else{c[E+20>>2]=b;c[b+24>>2]=E;break}}}while(0);c[q+4>>2]=B|1;c[d+B>>2]=B;if((q|0)!=(c[3687]|0)){H=B;break}c[3684]=B;return}else{c[A>>2]=e&-2;c[q+4>>2]=r|1;c[d+r>>2]=r;H=r}}while(0);r=H>>>3;if(H>>>0<256){d=r<<1;e=14768+(d<<2)|0;A=c[3682]|0;E=1<<r;do{if((A&E|0)==0){c[3682]=A|E;I=e;J=14768+(d+2<<2)|0}else{r=14768+(d+2<<2)|0;h=c[r>>2]|0;if(h>>>0>=(c[3686]|0)>>>0){I=h;J=r;break}am()}}while(0);c[J>>2]=q;c[I+12>>2]=q;c[q+8>>2]=I;c[q+12>>2]=e;return}e=q;I=H>>>8;do{if((I|0)==0){K=0}else{if(H>>>0>16777215){K=31;break}J=(I+1048320|0)>>>16&8;d=I<<J;E=(d+520192|0)>>>16&4;A=d<<E;d=(A+245760|0)>>>16&2;r=14-(E|J|d)+(A<<d>>>15)|0;K=H>>>((r+7|0)>>>0)&1|r<<1}}while(0);I=15032+(K<<2)|0;c[q+28>>2]=K;c[q+20>>2]=0;c[q+16>>2]=0;r=c[3683]|0;d=1<<K;do{if((r&d|0)==0){c[3683]=r|d;c[I>>2]=e;c[q+24>>2]=I;c[q+12>>2]=q;c[q+8>>2]=q}else{if((K|0)==31){L=0}else{L=25-(K>>>1)|0}A=H<<L;J=c[I>>2]|0;while(1){if((c[J+4>>2]&-8|0)==(H|0)){break}M=J+16+(A>>>31<<2)|0;E=c[M>>2]|0;if((E|0)==0){N=2370;break}else{A=A<<1;J=E}}if((N|0)==2370){if(M>>>0<(c[3686]|0)>>>0){am()}else{c[M>>2]=e;c[q+24>>2]=J;c[q+12>>2]=q;c[q+8>>2]=q;break}}A=J+8|0;B=c[A>>2]|0;E=c[3686]|0;if(J>>>0<E>>>0){am()}if(B>>>0<E>>>0){am()}else{c[B+12>>2]=e;c[A>>2]=e;c[q+8>>2]=B;c[q+12>>2]=J;c[q+24>>2]=0;break}}}while(0);q=(c[3690]|0)-1|0;c[3690]=q;if((q|0)==0){O=15184}else{return}while(1){q=c[O>>2]|0;if((q|0)==0){break}else{O=q+8|0}}c[3690]=-1;return}function bH(a){a=a|0;var b=0,d=0,e=0,f=0,g=0,h=0,i=0,j=0,k=0,l=0,m=0,n=0,o=0;do{if((c[3674]|0)==0){b=al(8)|0;if((b-1&b|0)==0){c[3676]=b;c[3675]=b;c[3677]=-1;c[3678]=2097152;c[3679]=0;c[3793]=0;c[3674]=(aC(0)|0)&-16^1431655768;break}else{am();return 0}}}while(0);if(a>>>0>=4294967232){d=0;return d|0}b=c[3688]|0;if((b|0)==0){d=0;return d|0}e=c[3685]|0;do{if(e>>>0>(a+40|0)>>>0){f=c[3676]|0;g=Z((((-40-a-1+e+f|0)>>>0)/(f>>>0)|0)-1|0,f)|0;h=b;i=15176;while(1){j=c[i>>2]|0;if(j>>>0<=h>>>0){if((j+(c[i+4>>2]|0)|0)>>>0>h>>>0){k=i;break}}j=c[i+8>>2]|0;if((j|0)==0){k=0;break}else{i=j}}if((c[k+12>>2]&8|0)!=0){break}i=aw(0)|0;h=k+4|0;if((i|0)!=((c[k>>2]|0)+(c[h>>2]|0)|0)){break}j=aw(-(g>>>0>2147483646?-2147483648-f|0:g)|0)|0;l=aw(0)|0;if(!((j|0)!=-1&l>>>0<i>>>0)){break}j=i-l|0;if((i|0)==(l|0)){break}c[h>>2]=(c[h>>2]|0)-j;c[3790]=(c[3790]|0)-j;h=c[3688]|0;m=(c[3685]|0)-j|0;j=h;n=h+8|0;if((n&7|0)==0){o=0}else{o=-n&7}n=m-o|0;c[3688]=j+o;c[3685]=n;c[j+(o+4)>>2]=n|1;c[j+(m+4)>>2]=40;c[3689]=c[3678];d=(i|0)!=(l|0)|0;return d|0}}while(0);if((c[3685]|0)>>>0<=(c[3689]|0)>>>0){d=0;return d|0}c[3689]=-1;d=0;return d|0}function bI(b,d,e){b=b|0;d=d|0;e=e|0;var f=0,g=0,h=0;f=b+e|0;if((e|0)>=20){d=d&255;e=b&3;g=d|d<<8|d<<16|d<<24;h=f&~3;if(e){e=b+4-e|0;while((b|0)<(e|0)){a[b]=d;b=b+1|0}}while((b|0)<(h|0)){c[b>>2]=g;b=b+4|0}}while((b|0)<(f|0)){a[b]=d;b=b+1|0}}function bJ(b,d,e){b=b|0;d=d|0;e=e|0;var f=0;f=b|0;if((b&3)==(d&3)){while(b&3){if((e|0)==0)return f|0;a[b]=a[d]|0;b=b+1|0;d=d+1|0;e=e-1|0}while((e|0)>=4){c[b>>2]=c[d>>2];b=b+4|0;d=d+4|0;e=e-4|0}}while((e|0)>0){a[b]=a[d]|0;b=b+1|0;d=d+1|0;e=e-1|0}return f|0}function bK(b){b=b|0;var c=0;c=b;while(a[c]|0){c=c+1|0}return c-b|0}function bL(b,c){b=b|0;c=c|0;var d=0;do{a[b+d|0]=a[c+d|0];d=d+1|0}while(a[c+(d-1)|0]|0);return b|0}function bM(b,c){b=b|0;c=c|0;var d=0,e=0;d=b+(bK(b)|0)|0;do{a[d+e|0]=a[c+e|0];e=e+1|0}while(a[c+(e-1)|0]|0);return b|0}function bN(a,b){a=a|0;b=b|0;return aD[a&3](b|0)|0}function bO(a,b){a=a|0;b=b|0;aE[a&1](b|0)}function bP(a,b,c){a=a|0;b=b|0;c=c|0;aF[a&3](b|0,c|0)}function bQ(a,b,c,d){a=a|0;b=b|0;c=c|0;d=d|0;return aG[a&7](b|0,c|0,d|0)|0}function bR(a){a=a|0;aH[a&1]()}function bS(a,b,c){a=a|0;b=b|0;c=c|0;return aI[a&15](b|0,c|0)|0}function bT(a){a=a|0;_(0);return 0}function bU(a){a=a|0;_(1)}function bV(a,b){a=a|0;b=b|0;_(2)}function bW(a,b,c){a=a|0;b=b|0;c=c|0;_(3);return 0}function bX(){_(4)}function bY(a,b){a=a|0;b=b|0;_(5);return 0}
// EMSCRIPTEN_END_FUNCS
var aD=[bT,bT,a9,bT];var aE=[bU,bU];var aF=[bV,bV,bE,bV];var aG=[bW,bW,bi,bW,bd,bW,bD,bW];var aH=[bX,bX];var aI=[bY,bY,a5,bY,ba,bY,a8,bY,a6,bY,bY,bY,bY,bY,bY,bY];return{_gzwrite:bi,_strlen:bK,_strcat:bM,_free:bG,_gzread:bd,_gzopen:ba,_memset:bI,_malloc:bF,_memcpy:bJ,_gzclose:a9,_strcpy:bL,runPostSets:aZ,stackAlloc:aJ,stackSave:aK,stackRestore:aL,setThrew:aM,setTempRet0:aP,setTempRet1:aQ,setTempRet2:aR,setTempRet3:aS,setTempRet4:aT,setTempRet5:aU,setTempRet6:aV,setTempRet7:aW,setTempRet8:aX,setTempRet9:aY,dynCall_ii:bN,dynCall_vi:bO,dynCall_vii:bP,dynCall_iiii:bQ,dynCall_v:bR,dynCall_iii:bS}})
// EMSCRIPTEN_END_ASM
({ "Math": Math, "Int8Array": Int8Array, "Int16Array": Int16Array, "Int32Array": Int32Array, "Uint8Array": Uint8Array, "Uint16Array": Uint16Array, "Uint32Array": Uint32Array, "Float32Array": Float32Array, "Float64Array": Float64Array }, { "abort": abort, "assert": assert, "asmPrintInt": asmPrintInt, "asmPrintFloat": asmPrintFloat, "min": Math_min, "invoke_ii": invoke_ii, "invoke_vi": invoke_vi, "invoke_vii": invoke_vii, "invoke_iiii": invoke_iiii, "invoke_v": invoke_v, "invoke_iii": invoke_iii, "_llvm_lifetime_end": _llvm_lifetime_end, "_lseek": _lseek, "_sysconf": _sysconf, "_abort": _abort, "_close": _close, "_pread": _pread, "_open": _open, "___setErrNo": ___setErrNo, "_send": _send, "_write": _write, "_read": _read, "_recv": _recv, "_pwrite": _pwrite, "_sbrk": _sbrk, "_strerror_r": _strerror_r, "___errno_location": ___errno_location, "_strerror": _strerror, "_llvm_lifetime_start": _llvm_lifetime_start, "_llvm_bswap_i32": _llvm_bswap_i32, "_time": _time, "STACKTOP": STACKTOP, "STACK_MAX": STACK_MAX, "tempDoublePtr": tempDoublePtr, "ABORT": ABORT, "NaN": NaN, "Infinity": Infinity }, buffer);
var _gzwrite = Module["_gzwrite"] = asm["_gzwrite"];
var _strlen = Module["_strlen"] = asm["_strlen"];
var _strcat = Module["_strcat"] = asm["_strcat"];
var _free = Module["_free"] = asm["_free"];
var _gzread = Module["_gzread"] = asm["_gzread"];
var _gzopen = Module["_gzopen"] = asm["_gzopen"];
var _memset = Module["_memset"] = asm["_memset"];
var _malloc = Module["_malloc"] = asm["_malloc"];
var _memcpy = Module["_memcpy"] = asm["_memcpy"];
var _gzclose = Module["_gzclose"] = asm["_gzclose"];
var _strcpy = Module["_strcpy"] = asm["_strcpy"];
var runPostSets = Module["runPostSets"] = asm["runPostSets"];
var dynCall_ii = Module["dynCall_ii"] = asm["dynCall_ii"];
var dynCall_vi = Module["dynCall_vi"] = asm["dynCall_vi"];
var dynCall_vii = Module["dynCall_vii"] = asm["dynCall_vii"];
var dynCall_iiii = Module["dynCall_iiii"] = asm["dynCall_iiii"];
var dynCall_v = Module["dynCall_v"] = asm["dynCall_v"];
var dynCall_iii = Module["dynCall_iii"] = asm["dynCall_iii"];
Runtime.stackAlloc = function(size) { return asm['stackAlloc'](size) };
Runtime.stackSave = function() { return asm['stackSave']() };
Runtime.stackRestore = function(top) { asm['stackRestore'](top) };
// Warning: printing of i64 values may be slightly rounded! No deep i64 math used, so precise i64 code not included
var i64Math = null;
// === Auto-generated postamble setup entry stuff ===
Module['callMain'] = function callMain(args) {
  assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on __ATMAIN__)');
  assert(!Module['preRun'] || Module['preRun'].length == 0, 'cannot call main when preRun functions remain to be called');
  args = args || [];
  ensureInitRuntime();
  var argc = args.length+1;
  function pad() {
    for (var i = 0; i < 4-1; i++) {
      argv.push(0);
    }
  }
  var argv = [allocate(intArrayFromString("/bin/this.program"), 'i8', ALLOC_NORMAL) ];
  pad();
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(allocate(intArrayFromString(args[i]), 'i8', ALLOC_NORMAL));
    pad();
  }
  argv.push(0);
  argv = allocate(argv, 'i32', ALLOC_NORMAL);
  var ret;
  var initialStackTop = STACKTOP;
  try {
    ret = Module['_main'](argc, argv, 0);
  }
  catch(e) {
    if (e.name == 'ExitStatus') {
      return e.status;
    } else if (e == 'SimulateInfiniteLoop') {
      Module['noExitRuntime'] = true;
    } else {
      throw e;
    }
  } finally {
    STACKTOP = initialStackTop;
  }
  return ret;
}
function run(args) {
  args = args || Module['arguments'];
  if (runDependencies > 0) {
    Module.printErr('run() called, but dependencies remain, so not running');
    return 0;
  }
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    var toRun = Module['preRun'];
    Module['preRun'] = [];
    for (var i = toRun.length-1; i >= 0; i--) {
      toRun[i]();
    }
    if (runDependencies > 0) {
      // a preRun added a dependency, run will be called later
      return 0;
    }
  }
  function doRun() {
    ensureInitRuntime();
    preMain();
    var ret = 0;
    calledRun = true;
    if (Module['_main'] && shouldRunNow) {
      ret = Module['callMain'](args);
      if (!Module['noExitRuntime']) {
        exitRuntime();
      }
    }
    if (Module['postRun']) {
      if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
      while (Module['postRun'].length > 0) {
        Module['postRun'].pop()();
      }
    }
    return ret;
  }
  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(function() {
      setTimeout(function() {
        Module['setStatus']('');
      }, 1);
      if (!ABORT) doRun();
    }, 1);
    return 0;
  } else {
    return doRun();
  }
}
Module['run'] = Module.run = run;
// {{PRE_RUN_ADDITIONS}}
if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}
// shouldRunNow refers to calling main(), not run().
var shouldRunNow = true;
if (Module['noInitialRun']) {
  shouldRunNow = false;
}
run();
// {{POST_RUN_ADDITIONS}}
  // {{MODULE_ADDITIONS}}
Module['gzcompress'] = function(data) { // TODO: Accept strings
  var gzFile = ccall('gzopen', 'number', ['string', 'string'], ['output.gz', 'wb']);
  var buffer = _malloc(data.length);
  HEAPU8.set(data, buffer);
  ccall('gzwrite', 'number', ['number', 'number', 'number'], [gzFile, buffer, data.length]);
  ccall('gzclose', 'number', ['number'], [gzFile]);
  _free(buffer);
  var ret = new Uint8Array(FS.root.contents['output.gz'].contents);
  FS.deleteFile('output.gz');
  return ret;
};
Module['gzdecompress'] = function(data) {
  var BUFSIZE = 1024*1024;
  FS.createDataFile('/', 'input.gz', data, true, true);
  var gzFile = ccall('gzopen', 'number', ['string', 'string'], ['input.gz', 'rb']);
  var buffer = _malloc(BUFSIZE);
  var chunks = [];
  var total = 0;
  var len;
  while( (len = ccall('gzread', 'number', ['number', 'number', 'number'], [gzFile, buffer, BUFSIZE])) > 0) {
    chunks.push(new Uint8Array(len));
    chunks[chunks.length-1].set(HEAPU8.subarray(buffer, buffer+len));
    total += len;
  }
  ccall('gzclose', 'number', ['number'], [gzFile]);
  FS.deleteFile('input.gz');
  _free(buffer);
  var ret = new Uint8Array(total);
  var curr = 0;
  for (var i = 0; i < chunks.length; i++) {
    ret.set(chunks[i], curr);
    curr += chunks[i].length;
  }
  return ret;
};

  return {
    compress: Module['gzcompress'],
    decompress: Module['gzdecompress']
  };
})();


onmessage = function(msg) {
  var start = Date.now();
  var data = Zee.decompress(new Uint8Array(msg.data.data));
  postMessage({
    filename: msg.data.filename,
    data: data,
    callbackID: msg.data.callbackID,
    time: Date.now() - start
  });
};

