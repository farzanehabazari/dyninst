/*
 * Copyright (c) 1996-2009 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "function.h"
#include "parse-cfg.h"
#include "process.h"

#include "mapped_object.h"
#include "mapped_module.h"
#include "InstructionDecoder.h"
#include <set>
#include <sstream>
#include "Relocation/Transformers/Movement-analysis.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

block_instance::block_instance(ParseAPI::Block *ib,
                               mapped_object *obj)
  : PatchBlock(ib, obj) {
  // We create edges lazily
};

// Fork constructor
block_instance::block_instance(const block_instance *parent,
                               mapped_object *childObj)
  : PatchBlock(parent, childObj) {
  // We also need to copy edges.
  // Thing is, those blocks may not exist yet...
  // So we wait, and do edges after all blocks have
  // been created
}

// Edges are deleted at the mapped_object layer
block_instance::~block_instance() 
{
}

AddressSpace *block_instance::addrSpace() const {
  return obj()->proc();
}

edge_instance *block_instance::getFallthrough() {
  for (edgelist::const_iterator iter = getTargets().begin(); iter != getTargets().end(); ++iter) {
    if ((*iter)->type() == FALLTHROUGH ||
        (*iter)->type() == CALL_FT ||
        (*iter)->type() == COND_NOT_TAKEN) {
      return SCAST_EI(*iter);
    }
  }
  return NULL;
}

block_instance *block_instance::getFallthroughBlock() {
  edge_instance *ft = getFallthrough();
  if (ft &&
      !ft->sinkEdge())
    return ft->trg();
  else
    return NULL;
}

edge_instance *block_instance::getTarget() {
  for (edgelist::const_iterator iter = getTargets().begin(); iter != getTargets().end(); ++iter) {
    if ((*iter)->type() == CALL ||
        (*iter)->type() == DIRECT ||
        (*iter)->type() == COND_TAKEN) {
      return SCAST_EI(*iter);
    }
  }
  return NULL;
}

block_instance *block_instance::getTargetBlock() {
  edge_instance *t = getFallthrough();
  if (t &&
      !t->sinkEdge())
    return t->trg();
  else
    return NULL;
}

int block_instance::id() const {
  return llb()->id();
}

using namespace Dyninst::Relocation;
void block_instance::triggerModified() {
    //KEVINTODO: implement this
    // Relocation info caching...
   //PCSensitiveTransformer::invalidateCache(this);
}

void block_instance::setNotAbruptEnd()
{
    llb()->setAbruptEnd(false);
    vector<func_instance*> funcs;
    getFuncs(std::back_inserter(funcs));
    for (vector<func_instance*>::iterator fit = funcs.begin();
         fit != funcs.end(); fit++)
    {
        (*fit)->removeAbruptEnd(this);
    }
}

std::string block_instance::calleeName() {
  // How the heck do we do this again?
  return obj()->getCalleeName(this);
}

void block_instance::updateCallTarget(func_instance *func) {
  // Update a sink-typed call edge to
  // have an inter-module target
  edge_instance *e = getTarget();
  assert(e->sinkEdge());
  e->trg_ = func->entryBlock();
}

func_instance *block_instance::entryOfFunc() const {
  parse_block *b = SCAST_PB(llb());
  parse_func *e = b->getEntryFunc();
  if (!e) return NULL;
  return obj()->findFunction(e);
}

bool block_instance::isFuncExit() const {
  parse_block *b = SCAST_PB(llb());
  return b->isExitBlock();
}

func_instance *block_instance::findFunction(ParseAPI::Function *p) {
  return obj()->findFunction(p);
}


void *block_instance::getPtrToInstruction(Address addr) const {
  if (addr < start()) return NULL;
  if (addr > end()) return NULL;
  return obj()->getPtrToInstruction(addr);
}

void block_instance::destroy(block_instance *b) {
  // Put things here that should go away when we destroy a block.
  // Iterate through functions...

  std::vector<ParseAPI::Function *> pFuncs;
  b->llb()->getFuncs(pFuncs);
  for (unsigned i = 0; i < pFuncs.size(); ++i) {
    func_instance *func = b->findFunction(pFuncs[i]);
    func->destroyBlock(b);
  }
  delete b;
}