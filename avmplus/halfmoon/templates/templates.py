#!/usr/bin/env python
# -*- Mode: Python; indent-tabs-mode: nil -*-
# vi: set ts=2 sw=2 expandtab:

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import StringIO

from hrdefs import hrdefs
from sexp import parse
from ast import *
from mangle import *

# -----------------------------------------------------
#
# generate template builder code
#

# format argument value
#
def fmtArg(expr):
  if expr.kind == 'TypeConst':
    # return 'tb.typeConst(%s)' % expr.value.cg()
    return expr.value.cgTraits()
  elif expr.kind == 'NumConst':
    if type(expr.value) == int:
      return 'tb.intConst(%i)' % expr.value
    elif type(expr.value) == float:
      return 'tb.doubleConst(%d)' % expr.value
    else:
      raise ParseError("unsupported constant '%s'" % expr.value)
  elif expr.kind == 'VarRef':
    vartype = expr.type()
    if vartype == int_type:
      return 'tb.intConst(%s)' % expr.value
    elif vartype == number_type:
      return 'tb.doubleConst(%s)' % expr.value
    else:
      raise ParseError("var refs of type '%s' not yet supported" % vartype.dump())
  else:
    return expr.dump()

# generate IfCall code.
# optionally bind the result to a local with the given name.
# numresults specifies the number of results generated by the
# ifcall--we use this only if we generate a merge label.
# merge_label specifies an existing label to use instead of
# generating one. (TODO param count agreement is assumed--verify.)
#
# if we generated a merge label, return its C++ name, otherwise None.
#
def fmtIfCall(ifcall, lhsname, numresults, use_merge_label, indent):
  args = [ifcall.condexpr] + ifcall.args
  # argc = len(args)

  # build arglist expr. collect varargs in local array if needed
  # fixc = 1
  # varc = argc - fixc
  # if varc > 0:
  #   vaname = '%s_args' % (lhsname if lhsname else '')
  #   varargs = ', '.join([fmtArg(args[i]) for i in range(fixc, argc)])
  #   print '%sDef* %s[] = { %s };' % (indent, vaname, varargs)
  #   arglist = ', '.join([fmtArg(args[i]) for i in range(0, fixc)] + [str(varc), vaname])
  # else:
  #   arglist = ', '.join([fmtArg(args[i]) for i in range(0, fixc)] + ['0', 'NULL'])

  arglist = fmtArglist(lhsname, ifcall.base, args, indent)

  # create IfInstr and add to IR
  repname = "IfInstr"
  lhs = '' if lhsname is None else '%s* %s = (%s*)' % (repname, lhsname, repname)
  print '%s%stb.addInstr(new%s(%s));' % (indent, lhs, repname, arglist)

  # an (if ...) may have a mixture of (goto ...) and plain exprs at its leaves.
  # we need to generate a synthetic label for any plain exprs to jump to.
  need_merge = not(ifcall.iftrue.allPathsEscape() and
                  ifcall.iffalse.allPathsEscape())

  # for any of our paths which terminate without an explicit goto,
  # we need to generate a goto to a synthetic merge label.
  # our caller may have supplied a candidate in use_merge_label;
  # if not, generate one now.
  #
  if need_merge:
    if use_merge_label is None:
      # create LabelInstr, but don't add to IR
      merge_label = lhsname + '_merge'
      print '%sLabelInstr* %s = newLabelInstr(%i);' % (indent, merge_label, numresults)
    else:
      merge_label = use_merge_label
  else:
    merge_label = None

  print ''

  fmtArm(ifcall.iftrue, lhsname, merge_label, indent)
  fmtArm(ifcall.iffalse, lhsname, merge_label, indent)

  return merge_label if use_merge_label is None else None

# helper - generate arm code
#
def fmtArm(arm, lhsname, merge_label, indent):
  repname = 'ArmInstr'
  armname = lhsname + '_' + arm.name
  print '%s{ // %s %s arm' % (indent, lhsname, arm.name)
  indent += '  '

  # create ArmInstr, add to IR, save to local
  # note: "(void)arm;" prevents unused variable warnings
  print '%s%s* %s = (%s*)tb.addInstr(%s->arm(%s)); (void)%s;' % (
    indent, repname, armname, repname, lhsname, arm.name, armname)
  
  # create a local for each arm param
  for i in range(0, len(arm.parnames)):
    parname = arm.parnames[i]
    print '%sDef* %s = &%s->params[%i]; (void)%s;' % (
      indent, parname, armname, i, parname)
  print ''
  
  # generate arm body statements, up to final instruction
  genTemStmts(arm, indent)
  
  # if needed, generate a final goto to synthetic label
  if not arm.allPathsEscape():
    body = arm.body
    retinstr = body[len(body) - 1]
    repname = 'GotoInstr'
    gotoname = armname + '_exit'
    print '%s%s* %s = newGotoStmt(%s); ' % (
      indent, repname, gotoname, merge_label)
    for i in range(0, len(retinstr.expr.args)):
      print '%s%s->args[%i] = %s;' % (
        indent, gotoname, i, fmtArg(retinstr.expr.args[i]))
    print '%stb.addInstr(%s); ' % (indent, gotoname)
    
  # close the c++ block for this arm
  indent = indent[:len(indent) - 2]
  print '%s}' % indent
  print ''

# helper - indicates the presence of a fixed-arg factory
# method for variable-arg instructions of a given shape.
# NOTE: carnal knowledge of InstrFactory API.
#
def hasFixedArgFactoryMethod(repname, argc):
  return (repname == 'StopInstr' and argc == 2) or \
    (repname == 'CallStmt2' and (argc == 4 or argc == 3)) or \
    (repname == 'CallStmt3' and argc == 5) or \
    (repname == 'IfInstr' and (argc == 0 or argc == 1 or argc == 2))

# generate argument list code. For instructions with fixed input
# shape, and common instances of some variable-input instructions,
# a single-shot factory method is available. In these cases we build
# a simple expression list of args.
# 
# Other variable-input instructions have factory methods that take 
# a count and an array for their variable args (they may still have
# a nonzero number of fixed args; these come first). For these, we 
# generate code to create an array of args and assign it to a local, 
# then return an expression list containing an arg count and a reference
# to the local. 
# 
def fmtArglist(lhsname, base, args, indent):
  rep = getRep(base)
  fixc = rep.shape[EFFECT_IN] + rep.shape[DATA_IN] # fixed arg count
  argc = len(args)
  varc = argc - fixc
  
  if rep.isvarin() and not hasFixedArgFactoryMethod(rep.name, len(args)):
    # build arglist expr. collect va_value in local array if needed
    if varc > 0:
      va_name = '%s_args' % (lhsname if lhsname else '')
      va_value = ', '.join([fmtArg(args[i]) for i in range(fixc, argc)])
      print '%sDef* %s[] = { %s };' % (indent, va_name, va_value)
      arglist = ', '.join([fmtArg(args[i]) for i in range(0, fixc)] + [str(varc), va_name])
    else:
      arglist = ', '.join([fmtArg(args[i]) for i in range(0, fixc)] + ['0', 'NULL'])
      
  else:
    arglist = ', '.join([fmtArg(arg) for arg in args]) # simple arg list
    
  return arglist

# generate Call code.
# optionally bind the result to a local with the given name.
#
def fmtCall(call, defs, indent, lhsname = None):
  base, args = call.base, call.args

  # format argument list, possibly generating locals along the way
  arglist = fmtArglist(lhsname, base, args, indent)

  # add kind arg if needed
  rep = getRep(base)
  if getRepCount(rep, defs) > 1:
    arglist = base.hrname() + ', ' + arglist

  # create and add instr, maybe store to local
  repname = rep.name
  lhs = '' if lhsname is None else '%s* %s = (%s*)' % (repname, lhsname, repname)
  print '%s%stb.addInstr(new%s(%s));' % (indent, lhs, repname, arglist)

# format call to access the given output of a call.
# Instr API has effect_out(), value_out() for instrs which
# have only one effect or data output (respectively),
# and effectOut(i), value_out(i) for the > 1 cases. Here
# we take an index into the combined list of outputs.
#
def fmtAccessor(call, i):
  effects = [t.isEffect() for t in call.types()]
  effect = effects[i]
  fmtix = '' if effects.count(effect) == 1 else '%s' % effects[:i].count(effect)
  return '%s(%s)' % ('effect_out' if effect else 'value_out', fmtix)

# emit user-defined labelled (local) definitions for the given template.
# syntactic restrictions on templates say that gotos must always terminate
# execution paths, which makes our job here simple--all user-defined labelled
# defs end by jumping to a common endpoint label, given here by endlabelname.
#
def genLabelDefSection(tem, end_label_name, indent):

  for label in tem.labels.values():
    labelname = 'label_%s' % label.name.lstrip('@')
    print '%s{ // label %s in %s' % (indent, labelname, tem.name)
    indent += '  '
    print '%stb.addInstr(%s);' % (indent, labelname)

    # extract label params
    for i in range(0, len(label.parnames)):
      pname = label.parnames[i]
      print '%sDef* %s = &%s->params[%i]; (void)%s;' % (
        indent, pname, labelname, i, pname)
    print ''

    # label body
    genTemStmts(label, indent)

    # build goto end from return
    retinstr = label.body[len(label.body) - 1]
    repname = 'GotoInstr'
    gotoname = labelname + '_exit'
    print '%s%s* %s = newGotoStmt(%s);' % (
      indent, repname, gotoname, end_label_name)
    for i in range(0, len(retinstr.expr.args)):
      print '%s%s->args[%i] = %s;' % (
        indent, gotoname, i, fmtArg(retinstr.expr.args[i]))
    print '%stb.addInstr(%s); ' % (indent, gotoname)

    indent = indent[:len(indent) - 2]
    print '%s}' % indent
    print ''

# generate builder code from template statements
# note that stmtlist is currently either the template body, or
# the template body without the final (return) statement.
#
def genTemStmts(tem, indent):

  # first, create label instrs. we need to have them around for gotos,
  # but they don't go into the IR until the end of the template.
  # note that if we have any labels, we'll need common endpoint label
  if len(tem.labels) > 0:
    print '%s// labels defined in %s, plus final endpoint' % (
      indent, tem.name)
    for label in tem.labels.values():
      labelname = 'label_%s' % label.name.lstrip('@')
      print '%sLabelInstr* %s = newLabelInstr(%i);' % (
        indent, labelname, label.numParams())
    # endpoint label
    # note: we get the number of label params from the number
    # of returned results. 
    end_label = 'label_%s' % tem.genLocalName('end')
    print '%sLabelInstr* %s = newLabelInstr(%i);' % (
      indent, end_label, tem.body[len(tem.body) - 1].expr.base.numParams())
    print ''
  else:
    end_label = None

  # emit mainline statement list
  # for stmt in tem.body[:len(tem.body) - 1]:
  for i in range(0, len(tem.body) - 1):
    stmt = tem.body[i]
    
    if stmt.kind == 'LocalDefs':
      # lhs = ...
      rhs = stmt.expr

      if rhs.kind == 'Call':
        # lhs = if/goto/call
        basename = rhs.base.name
        iname = tem.genLocalName(basename) if len(stmt.names) > 0 else None

        if basename == 'if':
          # if this if-stmt is the terminal statement of the template,
          # and we have user-defined labelled defs, then the if can use our
          # end_label as a post-if merge point, if needed.
          use_merge_label = end_label if i == len(tem.body) - 2 else None

          # fmt_ifcall will return the name of merge label to generate
          new_merge_label = fmtIfCall(rhs, iname, len(stmt.names), use_merge_label, indent)

          # add generated merge label to IR and extract vars
          if new_merge_label:
            print '%s// %s merge label, defs' % (indent, iname)
            print '%stb.addInstr(%s);' % (indent, new_merge_label)
            # create C++ local for each LocalDefs binding
            for i in range(0, len(stmt.names)):
              varname = stmt.names[i]
              print '%sDef* %s = &%s->params[%i]; (void)%s;' % (
                indent, varname, new_merge_label, i, varname)

        elif basename.startswith('@'):
          # goto label
          labelname = 'label_%s' % rhs.base.name.lstrip('@')
          repname = 'GotoInstr'
          gotoname = tem.genLocalName('goto')
          print '%sGotoInstr* %s = newGotoStmt(%s);' % (
            indent, gotoname, labelname)
          for i in range(0, len(rhs.args)):
            print '%s%s->args[%i] = %s;' % (
              indent, gotoname, i, fmtArg(rhs.args[i]))
          print '%stb.addInstr(%s);' % (indent, gotoname)

        else:
          # call
          fmtCall(rhs, defs, indent, iname)
          # create local for each LocalDefs binding
          for i in range(0, len(stmt.names)):
            labelname = stmt.names[i]
            accessor = fmtAccessor(rhs, i)
            print '%sDef* %s = %s->%s; (void)%s;' % (
              indent, labelname, iname, accessor, labelname)
        print ''

      else:
        # lhs = non-call
        labelname = stmt.names[0]
        ldef = stmt.defs[stmt.names[0]]
        print '%sDef* %s = %s;' % (indent, labelname, fmtArg(ldef))
        print ''

    elif stmt.kind == 'Call':
      # otherwise it's just an unbound call, no lhs
      # NOTE: this doesn't happen at the moment
      fmtCall(stmt, defs, indent)

    else:
      # nothing else at the top level of a template body
      raise ParseError('unknown statement type in body list: %s' % stmt.dump())

  # wrap it up

  # if needed, emit defined label code, plus final endpoint
  if len(tem.labels) > 0:
    genLabelDefSection(tem, end_label, indent)

    # add endpoint label
    print '%s// common endpoint block' % indent
    print '%stb.addInstr(%s);' % (indent, end_label)

    # create C++ local for each LocalDefs binding fromt
    # terminal statement
    term_stmt = tem.body[len(tem.body) - 2]
    for i in range(0, len(term_stmt.names)):
      varname = term_stmt.names[i]
      print '%sDef* %s = &%s->params[%i]; (void)%s;' % (
        indent, varname, end_label, i, varname)
    print ''

  # finally, add return instr for top-level templates
  # TODO verify handling of extracted vars (above) in nested tems
  if tem.parent is None:
    ret_stmt = tem.body[len(tem.body) - 1]
    fmtCall(ret_stmt.expr, defs, indent)

# generate builder switch case for a template
#
def genTemBuilderCase(tem):
  hrname = tem.hrname()
  print '  case %s: {' % hrname
  print '/***'
  print tem.dump()
  print '***/'
  print ''

  print '    const Type* in_types[] = { %s };' % ', '.join([t.cgType() for t in tem.partypes])
  print '    tb.start(%s, %i, in_types);' % (hrname, len(tem.partypes))
  print ''

  for i in range(0, len(tem.parnames)):
    pname = tem.parnames[i]
    print '    Def* %s = tb.paramRef(%i); (void)%s;' % (pname, i, pname)
  print ''

  genTemStmts(tem, '    ')

  # emit labels
  # emit return

  print ''
  print '    break;'
  print '  }'
  print ''

# generate template builder function impl
def genTemBuilderCases(defs):
  for d in templates_only(defs):
    genTemBuilderCase(d)

# -----------------------------------------------------
#
# generate type signature builder code
#

# generate input signature builder function
#
def genInputSigBuilder(defs):
  print '/// return input type signature for given instruction'
  print '///'
  print 'const Type** InstrFactory::buildInputSignature(InstrKind kind) {'
  print '  switch (kind) {'

  for d in defs:
    hrname = d.hrname()
    print '  case %s: {' % hrname
    print '    /* %s */' % d.dumpSig()
    siglen = len(d.partypes)
    if siglen == 0:
      print '    return NULL;'
    elif siglen == 1:
      print '    return copySig(%s);' % d.partypes[0].cgType()
    else:
      print '    const Type* input_sig[] = { %s };' % ', '.join([t.cgType() for t in d.partypes])
      print '    return copySig(%i, input_sig);' % len(d.partypes)
    print '  }'

  print '  default: {'
  print '    assert(false && "unsupported opcode");'
  print '    return NULL;'
  print '  }'
  print '  } // switch'
  print '}'

# generate output signature builder function
#
def genOutputSigBuilder(defs):
  print '/// return output type signature for given instruction'
  print '///'
  print 'const Type** InstrFactory::buildOutputSignature(InstrKind kind) {'
  print '  switch (kind) {'

  for d in defs:
    hrname = d.hrname()
    print '  case %s: {' % hrname
    print '    /* %s */' % d.dumpSig()
    siglen = len(d.rettypes)
    if siglen == 0:
      print '    return NULL;'
    elif siglen == 1:
      print '    return copySig(%s);' % d.rettypes[0].cgType()
    else:
      print '    const Type* output_sig[] = { %s };' % ', '.join([t.cgType() for t in d.rettypes])
      print '    return copySig(%i, output_sig);' % len(d.rettypes)
    print '  }'

  print '  default: {'
  print '    assert(false && "unsupported opcode");'
  print '    return NULL;'
  print '  }'
  print '  } // switch'
  print '}'

def genSigBuildersImpl(defs):
  genInputSigBuilder(defs)
  print ''
  genOutputSigBuilder(defs)

# ------------------------------------------------------
#
# generate predicate methods
#

# helper: sorted list of all distinct reps
def allReps():
  repset = set(replist +
                 instr_rep_overrides.values() +
                 shape_rep_overrides.values())
  return sorted(repset, key=lambda rep: (rep.shape, rep.name))

# generate is-shape function for a given RepInfo
def genIsShape(defs, shapeinfo, proto = False):
  print '/// true if given InstrKind is instance of %s' % shapeinfo.name
  if proto:
    print 'static bool is%s(InstrKind k);' % shapeinfo.name
  else:
    print 'bool InstrFactory::is%s(InstrKind k) {' % shapeinfo.name
    print '  return instr_attrs[k].shape == %s;' % shapeinfo.enum()
    print '}'
  print ''

# generate has-template pred
def genHasTemplate(defs, proto = False):
  print '/// true if given InstrKind has a template'
  if proto:
    print 'static bool hasTemplate(InstrKind k);'
  else:
    print 'bool InstrFactory::hasTemplate(InstrKind k) {'
    print '  return instr_attrs[k].hastem;'
    print '}'
  print ''

# generate InstrFactory predicate impls
def genPredsImpl(defs):
  genHasTemplate(defs, False)
  for sh in allReps():
    genIsShape(defs, sh)

# generate InstrFactory predicate protos
def genPredsProto(defs):
  genHasTemplate(defs, True)
  for sh in allReps():
    genIsShape(defs, sh, True)

# --------------------------------------------------------
#
# generate shape and instr enums
#

# helper: return map of rep names to counts.
# CAUTION - relies on unique rep names. Can fail, but not silently.
# CAUTION - cache assumes single def list over lifetime of CG
rep_counts = None
def getRepCount(rep, defs):
  global rep_counts
  if rep_counts is None:
    repnames = [r.name for r in allReps()]
    defrepnames = [getRep(d).name for d in defs]
    rep_counts = dict(zip(repnames, [defrepnames.count(repname) for repname in repnames]))
  return rep_counts[rep.name]

#
def shapeData(sh):
  return "%i, %i, %s" % (
          sh[0] + sh[1], sh[2] + sh[3], vararg_names[sh[4]])

#
def genEnums(defs, proto = False):
  reps = allReps()

  if proto:
    print '/// High level intermediate representation (HR) opcodes'
    print '///'
    print 'enum InstrKind {'
    for i in range(0, len(defs)):
      d = defs[i]
      print '  %s, %s// %s %s' % (
            d.hrname(), ' ' * max(0, 24 - len(d.hrname())),
            getRep(d).name, 'template' if d.isTemplate() else '')
    print '  HR_MAX = %s + 1' % defs[len(defs) - 1].hrname()
    print '};'
    print ''
    print '/// VarargKind designates variability in at most one'
    print "/// of an instruction's four argument groups."
    print '///'
    print 'enum VarargKind {'
    for i in [DATA_IN, DATA_OUT, NONE]:
      print '  %s,' % vararg_names[i]
    print '  VARARGKIND_MAX = %s' % vararg_names[NONE]
    print '};'
    print ''
    print '/// ShapeRep describes the representation of an instruction shape.'
    print '/// Note that when varargs are specified, the corresponding'
    print '/// member gives a minimum, rather than exact, quantity.'
    print '/// For example, a ShapeRep with vararg == %s and datain == 2' % vararg_names[DATA_IN]
    print '/// describes instructions with *at least* 2 data inputs.'
    print '///'
    print 'struct ShapeRep {'
    print '  int num_uses;            // number of Use inputs'
    print '  int num_defs;            // number of Def outputs'
    print '  VarargKind vararg;       // vararg position, if any'
    print '};'
    print ''
    print '/// InstrShape is an enumeration of HR instruction shapes.'
    print '/// The representation details of each InstrShape s is described by'
    print '/// shape_reps[s].'
    print '///'
    print 'enum InstrShape {'
    for i in range(0, len(reps)):
      rep = reps[i]
      shapedump = shapeData(rep.shape)
      print '  %s, %s// %s %s%i instrs' % (rep.enum(),
            ' ' * max(0, 24 - len(rep.enum())), shapedump,
            ' ' * max(0, 24 - len(shapedump)), getRepCount(rep, defs))
    print '  SHAPE_MAX = %s + 1' % reps[len(reps) - 1].enum()
    print '};'
    print ''

  if proto:
    print '/// shape_reps[] gives the representations of'
    print '/// the shapes enumerated by InstrShape.'
    print '///'
    print 'extern const ShapeRep shape_reps[SHAPE_MAX];'
    print ''
  else:
    print '/// shape_reps[] gives the representations of'
    print '/// the shapes enumerated by InstrShape.'
    print '///'
    print 'extern const ShapeRep shape_reps[SHAPE_MAX] = {'
    for rep in reps:
      sh = rep.shape
      print '  { %s }, %s// %s' % (shapeData(sh),
            ' ' * max(0, 10 - len(vararg_names[sh[4]])), rep.enum())
    print '};'
    print ''

  if proto:
    print '/// InstrAttrs contains attributes specific to (and universal'
    print '/// across all instances of) a particular HR instruction.'
    print '///'
    print 'struct InstrAttrs {'
    print '  const char* name;  // printable name'
    print '  InstrShape shape;  // shape (const)'
    print '  bool hastem;       // true if instruction has a template (const)'
    print '};'
    print ''
    print '/// instr_attrs describes the instructions enumerated in InstrKind.'
    print '///'
    print 'extern const InstrAttrs instr_attrs[HR_MAX];'
    print ''
  else:
    print '/// instr_attrs describes the instructions enumerated in InstrKind.'
    print '///'
    print 'extern const InstrAttrs instr_attrs[HR_MAX] = {'
    for d in defs:
      print '  { "%s", %s%s, %s },' % (d.name, ' ' * max(0, 24 - len(d.hrname())),
            getRep(d).enum(), 'true' if d.isTemplate() else 'false')
    print '};'
    print ''

# generate enum declarations
def genEnumsProto(defs):
  genEnums(defs, True)

# generate enum data definitions
def genEnumsImpl(defs):
  genEnums(defs, False)

# -----------------------------------------------------
#
# generate kind-driven dispatcher infrastructure
#

# generate kind adapter methods 
#
def genKindAdapterMethods(defs):
  for d in defs:
    rep = getRep(d)
    print ('RETURN_TYPE do_%s(%s* i) { return static_cast<SELF_CLASS*>(this)->do_default(i); }'
		  % (d.name, rep.name))

# generate dispatch function switch cases
#
def genKindAdapterCases(defs):
  for d in defs:
    hrname = d.hrname()
    rep = getRep(d)
    print 'case %s: ' % hrname
    print '  return a->do_%s(cast<%s>(instr));' % (d.name, rep.name)

# -----------------------------------------------------
#
# generate shape-driven dispatcher infrastructure
#

# generate shape adapter methods 
#
def genShapeAdapterMethods(defs):
  reps = allReps()
  for rep in reps:
    print ('RETURN_TYPE do_%s(%s* i) { return static_cast<SELF_CLASS*>(this)->do_default(i); }'
	      % (rep.name, rep.name))

# generate dispatch function switch cases
#
def genShapeAdapterCases(defs):
  reps = allReps()
  for rep in reps:
    print 'case %s: ' % rep.enum()
    print '  return a->do_%s(cast<%s>(instr));' % (rep.name, rep.name)

# -----------------------------------------------------
#
# generate C++ definitions of runtime helper functions
#

cpp_type_map = {
  'Atom': 'Atom',
  'Boolean': 'BoolKind',
  'Class': 'ClassClosure*',
  'Env': 'MethodEnv*',
  'Int': 'int32_t',
  'Method': 'MethodInfo*',
  'Name': 'const Multiname*',
  'Namespace': 'Namespace*',
  'Number': 'double',
  'Ord': 'int',
  'String': 'String*',
  'Effect': 'void',
  'Traits': 'Traits*',
  'Uint': 'uint32_t',
  'Object': 'Atom',
  'ScriptObject': 'ScriptObject*',
  'Array': 'ArrayObject*',
  'VectorInt': 'IntVectorObject*',
  'VectorUInt': 'UIntVectorObject*',
  'VectorDouble': 'DoubleVectorObject*',
  'Function': 'ClassClosure*',
  'Bot': 'void',
}

def cpp_typename(t):
  name = t.name
  if name.endswith('~'):
    name = name[0:len(name)-1]
  return cpp_type_map[name] if name in cpp_type_map else '?'+name

# true if the shape for d treats the last fixed arg as the first vararg
def has_extra_vararg(d):
  return getRep(d).name.startswith('CallStmt')

def make_argsig(d):
    sig = [cpp_typename(t) for t in d.partypes if cpp_typename(t) != 'void']
    if d.isvarin:
      vartype = sig[len(sig)-1]
      fixc = getRep(d).shape[DATA_IN] - (1 if has_extra_vararg(d) else 0)
      sig = sig[0:fixc] + ['int'] + ['%s*' % vartype]
    return ['MethodFrame*'] + sig

def ret_type(d):
    sig = [t for t in d.rettypes if not t.isEffect()]
    return sig[0] if len(sig) == 1 else None

def make_ret_ctype(d):
    t = ret_type(d)
    return cpp_typename(t) if t else 'void'

# Make a list of just primitive instructions
def protos_only(defs):
  return [d for d in defs if not(d.isTemplate())]

# return a list of all template instructions
def templates_only(defs):
  return [d for d in defs if d.isTemplate()]

# Exclude instructions with TopData or Top in their signature,
# or any instruction with 2+ data outputs.
def do_generate_stub(d):
  fullsig = [t.name for t in d.partypes + d.rettypes]
  return not ('TopData' in fullsig or 'Top' in fullsig or 'State' in fullsig)\
         and d.shape[DATA_OUT] in range(2)

# Generate a class with C++ prototypes for each stub.
def gen_stub_protos(defs):
  protos = protos_only(defs)
  stubs = [d for d in protos if do_generate_stub(d)]
  print "namespace halfmoon {"
  print "using namespace avmplus;"
  print "struct Stubs {"
  print "  static const int stub_count = %d;" % len(protos)
  print
  for d in stubs:
    print '  // %s' % d.dumpSig()
    arg_sig = make_argsig(d)
    ret_ctype = make_ret_ctype(d)
    print '  static %s do_%s(%s);' % (ret_ctype, d.name, ', '.join(arg_sig))
    print
  print "};"
  print
  print "/* One-line implementations, for copy/paste convenience:"
  for d in stubs:
    arg_sig = make_argsig(d)
    ret_ctype = make_ret_ctype(d)
    ret_stmt = 'return 0; ' if ret_ctype != 'void' else ''
    print '  %s Stubs::do_%s(%s) { assert(false && "%s not implemented"); %s}' %\
      (ret_ctype, d.name, ', '.join(arg_sig), d.name, ret_stmt)
  print "*/"
  print "}"

# Map C++ type names to nanojit::ArgType enums.
def lir_argtype(ctype):
  if ctype == 'void':
    return 'ARGTYPE_V'
  if ctype == 'double':
    return 'ARGTYPE_D'
  if ctype == 'uint32_t':
    return 'ARGTYPE_UI'
  if ctype in ['int', 'int32_t', 'BoolKind']:
    return 'ARGTYPE_I'
  return 'ARGTYPE_P'

# Generate the LIR typesig builder expression by mapping the C++ 
# paramter types to LIR ArgType enums.
def lir_typesig(d):
  argtypes = [lir_argtype(s) for s in make_argsig(d)]
  sigtypes = [lir_argtype(make_ret_ctype(d))] + argtypes
  return 'CallInfo::typeSig%d(%s)' % (len(argtypes), ', '.join(sigtypes))

# an opcode is pure if it has no side effects.  Since side-effect
# types are mapped to C++ 'void', we scan for void.
def lir_ispure(d):
  return 1 if 'void' not in [cpp_typename(t) for t in d.partypes] else 0

def lir_accset(d):
  return 'ACCSET_NONE' if lir_ispure(d) else 'ACCSET_ALL'

# generate a table of nanojit CallInfo structures; one for each stub.
def gen_stub_lirtable(defs):
  protos = protos_only(defs)
  print "namespace halfmoon {"
  print "const nanojit::CallInfo LirEmitter::lir_table[] = {"
  for d in protos:
    if do_generate_stub(d):
      print '  { (uintptr_t)&Stubs::do_%s, %s, ABI_CDECL, %d, %s verbose_only(, "%s")},' %\
        (d.name, lir_typesig(d), lir_ispure(d), lir_accset(d), d.name)
    else:
      print '  { 0, 0, ABI_CDECL, 0, ACCSET_NONE verbose_only(, "%s")},' % d.name
  print "};"
  print
  print "const int LirEmitter::stub_fixc[] = {"
  for d in protos:
    fixc = (getRep(d).shape[DATA_IN] - (1 if has_extra_vararg(d) else 0)) if d.isvarin\
           else -1 # -1 means stub has fixed arg count despite variadic shape
    print '  %d, // %s' % (fixc, d.name)
  print "};"
  print "}"

# generate a table of LLVMEmitter StubInfo structures; one for each stub.
def gen_stub_llvmtable(defs):
  return gen_stub_llvmtable_common(defs, 32)

def gen_stub_llvmtable64(defs):
  return gen_stub_llvmtable_common(defs, 64)

def gen_stub_llvmtable_common(defs,arch):
  protos = protos_only(defs)
  type_strings = []
  type_strings_cxx = {}
  def type_string_index(ret, args):
    type_string = ";".join([make_llvm_type_string(nm, getAvmMangleTypedefs(arch)) for nm in [ret] + args])
    if type_string in type_strings:
      return type_strings.index(type_string)
    type_strings.append(type_string)
    type_strings_cxx[type_string] = '%s ()(%s)' % (ret, ', '.join(args))
    return len(type_strings)-1

  save_stdout = sys.stdout
  sys.stdout = buffer = StringIO.StringIO()
  for scheme in mangleSchemes:
    kindIndex = 0;
# We need to print 2 different stub tables.
# One to handle mangles function names when the target OS is Mac or iOS
# And the other to handle mangled function names when the target OS is Windows
# The tables are named based on the target OS on which the packaged app will be running
    print "const LLVMModule::StubInfo %sllvm_stub_table[%d] = {" % (scheme.getCppLatch(),len(protos))
    print

    for d in protos:
      print '  // %d: %s' % (kindIndex, d.dumpSig())
      kindIndex = kindIndex+1
      fixc = (getRep(d).shape[DATA_IN] - (1 if has_extra_vararg(d) else 0)) if d.isvarin\
             else -1 # -1 means stub has fixed arg count despite variadic shape
      if do_generate_stub(d):
        arg_sig = make_argsig(d)
        ret_ctype = make_ret_ctype(d)
        fn_name = 'halfmoon::Stubs::do_'+d.name
        func_attrs = Attribute.STATIC | Attribute.PUBLIC | Attribute.CDECL 
        mgl_name = scheme.mangle(fn_name, ret_ctype, arg_sig, func_attrs, getAvmMangleTypedefs(arch))
        print '  //     %s %s(%s)' % (ret_ctype, fn_name, ', '.join(arg_sig))
        print '  { "%s", "%s", llvm_stub_types[%d], %s, %d },' % ( d.name, mgl_name,
          type_string_index(ret_ctype, arg_sig), 'true' if lir_ispure(d) else 'false', fixc)
        print
      else:
        print '  { "%s", 0, 0, false, %d },' % (d.name, fixc)
        print
    print "};"
    print

  sys.stdout = save_stdout

  print "namespace compile_abc {"
  print "static const int llvm_stub_count = %d;" % len(protos)
  print "static const char* llvm_stub_types[%d] = {" % len(type_strings)
  typeIndex = 0
  for t in type_strings:
    print '  // %d: %s' % (typeIndex, type_strings_cxx[t])
    typeIndex = typeIndex+1
    print '  "%s",' % (t)
    print
  print "};"
  print

  print buffer.getvalue()
  print "}"

# return the interpreter getter expression for type t
interp_getter_name = {
  'double' : 'interp->getDouble',
  'int' : 'interp->getOrdinal',
  'int32_t' : 'interp->getInt',
  'uint32_t' : 'interp->getUint',
  'BoolKind' : 'interp->getBool',
  'String*' : 'interp->getString',
  'Namespace*' : 'interp->getNs',
  'Atom' : 'interp->getAtom',
  'Traits*' : 'interp->getTraits',
  'MethodEnv*' : 'interp->getEnv',
  'MethodInfo*' : 'interp->getMethod',
  'const Multiname*' : 'interp->getName',
  'ScriptObject*' : 'interp->getObject',
  'ArrayObject*' : '(ArrayObject*)interp->getObject',
  'IntVectorObject*' : '(IntVectorObject*)interp->getObject',
  'UIntVectorObject*' : '(UIntVectorObject*)interp->getObject',
  'DoubleVectorObject*' : '(DoubleVectorObject*)interp->getObject',
  'ClassClosure*' : '(ClassClosure*)interp->getObject',
}
def interp_getter(t):
  return interp_getter_name[cpp_typename(t)]

# return the Interpreter Value constructor name for the return type of d
def interp_value(d):
  ct = cpp_typename(ret_type(d))
  return 'AtomValue' if ct == 'Atom'\
    else 'OrdValue' if ct == 'int'\
    else 'Value'

# generate a class of helper functions for the interpreter.  Each one
# unpacks arguments, invokes the stub, then saves the result Value.
# var-in instructions are handled by passing a count and pointer to
# values.  void stubs are handled by not saving the result.
def gen_stub_callers(defs):
  protos = protos_only(defs)
  stubs = [d for d in protos if do_generate_stub(d)]
  print "namespace halfmoon {"
  print "class StubCaller {"
  print " public:"
  for d in stubs:
    exprs = ['%s(instr->use(%d))' % (interp_getter(d.partypes[i]), i) for i in range(len(d.partypes))\
      if cpp_typename(d.partypes[i]) != 'void']
    print '  // %s' % d.dumpSig()
    print '  static void do_%s(Interpreter* interp, %s* instr) {' % (d.name, getRep(d).name)
    if d.isvarin:
      fixc = getRep(d).shape[DATA_IN] - (1 if has_extra_vararg(d) else 0)
      var_type = d.partypes[len(d.partypes)-1]
      var_ctype = cpp_typename(var_type)
      vargetter = interp_getter(var_type)
      print '    int argc = instr->arg_count();'
      print '    Use* arg_uses = instr->args();'
      print '    %s* args = (%s*)interp->args_out_;' % (var_ctype, var_ctype)
      print '    for (int i = 0; i < argc; ++i)'
      print '      args[i] = %s(arg_uses[i]);' % (vargetter)
      exprs = exprs[0:fixc] + ['argc, args']
    exprs = ['&interp->frame_'] + exprs
    arg_expr = ',\n        '.join(exprs)
    ret_ctype = make_ret_ctype(d)
    if ret_ctype == 'void':
      print '    Stubs::do_%s(%s);' % (d.name, arg_expr)
      print '    (void)interp;'
    else:
      print '    interp->resultVal(instr->value_out()) = %s(Stubs::do_%s(%s));' %\
        (interp_value(d), d.name, arg_expr)
    if len(exprs) == 1 and ret_ctype == 'void':
      print '    (void)instr;'
    print '  }'
    print
  print "};"
  print
  # generate a table with pointers to the helper functions, indexed by InstrKind
  print "const Interpreter::StubCall Interpreter::stub_table[] = {"
  for d in protos:
    if do_generate_stub(d):
      print '  (StubCall)&StubCaller::do_%s,' % d.name
    else:
      print '  0, // %s' % d.name
  print "};"
  print
  print "}"

# End generation of helpers for stubs

# --------------------------------------------------------
#
# generator harness and helpers
#

def printheader():
  print '///'
  print '/// generated by templates.py -- do not edit'
  print '///'
  print

gendir = "../generated"

def genfile(defs, gen, filename):
  if not(os.path.exists(gendir)):
    os.mkdir(gendir)
  f = open('%s/%s' % (gendir, filename), 'wb')
  try:
    sys.stdout = f
    printheader()
    gen(defs)
  finally:
    f.close()
    sys.stdout = sys.__stdout__

# group defs into primitives-then-templates 
def sortdefs(defs):
  return protos_only(defs) + templates_only(defs)

def gendefs(defs):
  defs = sortdefs(defs)
  genfile(defs, genEnumsProto,            "InstrFactory_defs_proto.hh")
  genfile(defs, genEnumsImpl,             "InstrFactory_defs_impl.hh")
  genfile(defs, genPredsProto,            "InstrFactory_preds_proto.hh")
  genfile(defs, genPredsImpl,             "InstrFactory_preds_impl.hh")
  genfile(defs, genSigBuildersImpl,       "InstrFactory_signatures_impl.hh")
  genfile(defs, genTemBuilderCases,       "InstrFactory_buildTemplate_cases.hh")
  genfile(defs, genKindAdapterMethods,    "KindAdapter_methods.hh")
  genfile(defs, genKindAdapterCases,      "KindAdapter_cases.hh")
  genfile(defs, genShapeAdapterMethods,   "ShapeAdapter_methods.hh")
  genfile(defs, genShapeAdapterCases,     "ShapeAdapter_cases.hh")
  genfile(defs, gen_stub_protos,            "Stub_protos.hh")
  genfile(defs, gen_stub_lirtable,          "Stub_lirtable.hh")
  genfile(defs, gen_stub_llvmtable,         "Stub_llvmtable.hh")
  genfile(defs, gen_stub_llvmtable64,         "Stub_llvmtable_64.hh")
  genfile(defs, gen_stub_callers,           "Stub_callers.hh")

def trace(s):
  save = sys.stdout
  sys.stdout = sys.__stdout__
  print s
  sys.stdout = save

# -----------------------------------------------------
#
# main
#

# dump processed defs
def dump(defs):
  for d in defs:
    print '\n' + d.dump()

# generator functions callable from the command line
gens = {
  'defs': gendefs,        # generate code
  'dump': dump            # dump internal reps
}

if len(sys.argv) > 1 and sys.argv[1] in gens:
  gen = gens[sys.argv[1]]
else:
  print "Error: must specify defs or dump as command-line argument"
  sys.exit(1)

try:
  sexprs = [sexpr for sexpr in parse(hrdefs) if isValid(sexpr)]
  defs = [toDef(sexpr) for sexpr in sexprs]
  process(defs)
  gen(defs)
except ParseError as e:
  print 'parse error: %s' % e.message()
  sys.exit(1)
