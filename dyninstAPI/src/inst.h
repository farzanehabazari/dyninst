/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef INST_HDR
#define INST_HDR

/*
 * inst.h - This file contains the insrumentation functions that are provied
 *   by the instrumentation layer.
 *
 * $Log: inst.h,v $
 * Revision 1.11  1995/03/10 19:30:21  hollings
 * Added getPointCost and getInsnCost.
 *
 * Revision 1.10  1995/02/16  08:53:35  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.9  1995/02/16  08:33:32  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.8  1994/11/02  11:08:52  markc
 * Added prototypes.
 *
 * Revision 1.7  1994/09/22  02:29:18  markc
 * Removed compiler warnings
 *
 * Revision 1.6  1994/08/08  20:13:39  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.5  1994/07/28  22:40:41  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.4  1994/07/20  23:23:38  hollings
 * added insn generated metric.
 *
 * Revision 1.3  1994/07/05  03:26:06  hollings
 * observed cost model
 *
 * Revision 1.2  1994/06/29  02:52:31  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.1  1994/01/27  20:31:25  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.8  1993/10/19  15:29:19  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.7  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.6  1993/08/20  21:59:49  hollings
 * changed typedef to have seperate structs to get around gdb bug.
 *
 * Revision 1.5  1993/08/11  01:32:24  hollings
 * added prototypes for predicated cost model.
 *
 * Revision 1.4  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.3  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.2  1993/04/27  14:39:21  hollings
 * signal forwarding and args tramp.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

class instInstance;
class process;
class instPoint;
class pdFunction;

typedef enum { callNoArgs, callRecordType, callFullArgs } callOptions;
typedef enum { callPreInsn, callPostInsn } callWhen;
typedef enum { orderFirstAtPoint, orderLastAtPoint } callOrder;

class intCounterHandle {
 public:
    intCounterHandle() {
      proc = NULL; sampler = NULL; counterPtr = NULL;
    }
    process *proc;
    intCounter *counterPtr;		/* NOT in our address space !!!! */
    intCounter data;			/* a copy in our address space */
    instInstance *sampler;		/* function to sample value */
};

class timerHandle {
 public:
    timerHandle() {
      proc = NULL; timerPtr = NULL; sampler = NULL;
    }
    process *proc;
    tTimer *timerPtr;			/* NOT in our address space !!!! */
    tTimer data;			/* a copy in our address space !!!! */
    instInstance *sampler;		/* function to sample value */
};

class AstNode;

/*
 * Insert instrumentation a the specificed codeLocation.
 *
 */
instInstance *addInstFunc(process *proc,
			  instPoint *location,
			  AstNode *ast, 
			  callWhen when,
			  callOrder order);

float getPointFrequency(instPoint *point);
int getPointCost(process *proc, instPoint *point);

void deleteInst(instInstance*);

intCounterHandle *createIntCounter(process *proc, int value, bool report);
int getIntCounterValue(intCounterHandle*);
void freeIntCounter(intCounterHandle*);
void addToIntCounter(intCounterHandle*, int amount);

floatCounter *createFloatCounter(int pid);
float getCounterValue(floatCounter*);
void freeFloatCounter(floatCounter*);
void addToFloatCounter(floatCounter*, float amount);

/*
 * Create a triggered timer.  A triggered timer is one that only will start/stop
 *   when the value of the trigger counter is positive.
 *
 *   type indicates if the timer is based on wall or process (virtual) time.
 *
 *   trigger indicates the trigger counter to use.  A null trigger implies no
 *     trigger is required.
 *
 *   pid is the thread that the timer will run in.
 */
timerHandle *createTimer(process*proc,timerType type, bool report);

/*
 * return the current value of the timer.
 *
 */
float getTimerValue(timerHandle *timer);

/*
 * dispose of a timer. 
 *
 */
void freeTimer(timerHandle*);

/*
 * Test if the inst point is for a call to a tahracked function (as opposed to
 *   a function that has been either implictly or explictly suppressed from
 *   instrumentation (i.e. library functions or user suppression of user 
 *   functions).
 */
int callsTrackedFuncP(instPoint *);

/* return the function asociated with a point. */
pdFunction *getFunction(instPoint *point);

/*
 * struct to define a list of inst requests 
 *
 */
#define FUNC_ENTRY      0x1             /* entry to the function */
#define FUNC_EXIT       0x2             /* exit from function */
#define FUNC_CALL       0x4             /* subroutines called from func */
#define FUNC_ARG  	0x8             /* use arg as argument */

class instMapping {
 public:
  instMapping(const string f, const string i, const int w,
	      AstNode *a, bool mr=true)
    : more(mr), func(f), inst(i), where(w), arg(a) { }

  instMapping() : more(true), where(0), arg(NULL) { }

  void set(const string f, const string i, const int w,
	   AstNode *a=NULL, bool mr=true) {
    func = f; inst =i; where = w; arg = a; more = mr;}

  // KLUDGE -- TODO fix this -- marks last element in array
  bool more;
  string func;         /* function to instrument */
  string inst;         /* inst. function to place at func */
  int where;          /* FUNC_ENTRY, FUNC_EXIT, FUNC_CALL */
  AstNode *arg;	    /* what to pass as arg0 */
};

/*
 * Install a list of inst requests in a process.
 *
 */
void installDefaultInst(process *proc, instMapping *initialReq);

/*
 * get information about the cost of pimitives.
 *
 */
void initPrimitiveCost();
void initDefaultPointFrequencyTable();

//
// Return the expected runtime of the passed function in instruction
//   times.
//
unsigned getPrimitiveCost(const string name);

// a register.
typedef int reg;

typedef enum { plusOp,
               minusOp,
               timesOp,
               divOp,
               lessOp,
               leOp,
               greaterOp,
               geOp,
               eqOp,
               neOp,
               loadOp,           
               loadConstOp,
               storeOp,
               ifOp,
	       callOp,
	       trampPreamble,
	       trampTrailer,
	       noOp,
	       orOp,
	       andOp} opCode;

/*
 * Generate an instruction.
 *
 */
unsigned emit(opCode op, reg src1, reg src2, reg dest, char *insn, unsigned &base);

int getInsnCost(opCode t);

/*
 * get the requested parameter into a register.
 *
 */
reg getParameter(reg dest, int param);

#define INFERIOR_HEAP_BASE     "DYNINSTdata"
#define UINFERIOR_HEAP_BASE    "_DYNINSTdata"
#define GLOBAL_HEAP_BASE        "DYNINSTglobalData"
#define U_GLOBAL_HEAP_BASE      "_DYNINSTglobalData"

class point {
 public:
  point() { inst = NULL; }
  instInstance *inst;
};

/*
 * prototype for architecture specific function to restore the
 * original instructions during a steering-detach.
 */

extern void restore_original_instructions(process *, instPoint *);

extern string getProcessStatus(const process *p);

// TODO - what about mangled names ?
// expects the symbol name advanced past the underscore
extern unsigned findTags(const string funcName);

extern float computePauseTimeMetric();

extern process *nodePseudoProcess;

extern void initLibraryFunctions();
extern void forkNodeProcesses(process *curr,
			      traceHeader *hr,
			      traceFork *fr);

extern unsigned getMaxBranch();

// find these internal functions before finding any other functions
extern dictionary_hash<string, unsigned> tagDict;

extern dictionary_hash <string, unsigned> primitiveCosts;
extern dictionary_hash<string, unsigned> tagDict;
extern process *nodePseudoProcess;

#endif
