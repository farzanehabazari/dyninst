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

#if !defined (_R_E_AST_H_)
#define _R_E_AST_H_

#include "Relocation/Atoms/Atom.h"

class AstNode;
typedef dyn_detail::boost::shared_ptr<AstNode> AstNodePtr;

namespace Dyninst {
namespace Relocation {

class ASTAtom : public Atom {
 public:
  typedef dyn_detail::boost::shared_ptr<ASTAtom> Ptr;

  static Ptr create(AstNodePtr, instPoint *);

  ASTAtom(AstNodePtr a, instPoint *p) : ast_(a), point_(p) {};

  bool generate(Trace &, GenStack &);
  
  virtual ~ASTAtom() {};

  virtual std::string format() const;

 private:

  AstNodePtr ast_;
  // We need this for liveness
  instPoint *point_;
};

struct AstPatch : public Patch {
 AstPatch(AstNodePtr a, instPoint *b) : ast(a), point(b) {};
  
  virtual bool apply(codeGen &gen, int iteration, int shift);
  virtual bool preapply(codeGen &gen);
  virtual ~AstPatch();

  AstNodePtr ast;
  instPoint *point;
};



};
};
#endif