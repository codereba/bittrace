#include "sqlite32.h"


/************** Begin file vdbeaux.c *****************************************/
/*
** 2003 September 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used for creating, destroying, and populating
** a VDBE (or an "sqlite3_stmt" as it is known to the outside world.)  Prior
** to version 2.8.7, all this code was combined into the vdbe.c source file.
** But that file was getting too big so this subroutines were split out.
*/



/*
** When debugging the code generator in a symbolic debugger, one can
** set the sqlite3VdbeAddopTrace to 1 and all opcodes will be printed
** as they are added to the instruction stream.
*/
#ifdef SQLITE_DEBUG
SQLITE_PRIVATE int sqlite3VdbeAddopTrace = 0;
#endif


/*
** Create a new virtual database engine.
*/
SQLITE_PRIVATE Vdbe *sqlite3VdbeCreate(sqlite3 *db){
  Vdbe *p;
  p = sqlite3DbMallocZero(db, sizeof(Vdbe) );
  if( p==0 ) return 0;
  p->db = db;
  if( db->pVdbe ){
    db->pVdbe->pPrev = p;
  }
  p->pNext = db->pVdbe;
  p->pPrev = 0;
  db->pVdbe = p;
  p->magic = VDBE_MAGIC_INIT;
  return p;
}

/*
** Remember the SQL string for a prepared statement.
*/
SQLITE_PRIVATE void sqlite3VdbeSetSql(Vdbe *p, const char *z, int n, int isPrepareV2){
  assert( isPrepareV2==1 || isPrepareV2==0 );
  if( p==0 ) return;
#ifdef SQLITE_OMIT_TRACE
  if( !isPrepareV2 ) return;
#endif
  assert( p->zSql==0 );
  p->zSql = sqlite3DbStrNDup(p->db, z, n);
  p->isPrepareV2 = (u8)isPrepareV2;
}

/*
** Return the SQL associated with a prepared statement
*/
SQLITE_API const char *sqlite3_sql(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe *)pStmt;
  return (p && p->isPrepareV2) ? p->zSql : 0;
}

/*
** Swap all content between two VDBE structures.
*/
SQLITE_PRIVATE void sqlite3VdbeSwap(Vdbe *pA, Vdbe *pB){
  Vdbe tmp, *pTmp;
  char *zTmp;
  tmp = *pA;
  *pA = *pB;
  *pB = tmp;
  pTmp = pA->pNext;
  pA->pNext = pB->pNext;
  pB->pNext = pTmp;
  pTmp = pA->pPrev;
  pA->pPrev = pB->pPrev;
  pB->pPrev = pTmp;
  zTmp = pA->zSql;
  pA->zSql = pB->zSql;
  pB->zSql = zTmp;
  pB->isPrepareV2 = pA->isPrepareV2;
}

#ifdef SQLITE_DEBUG
/*
** Turn tracing on or off
*/
SQLITE_PRIVATE void sqlite3VdbeTrace(Vdbe *p, FILE *trace){
  p->trace = trace;
}
#endif

/*
** Resize the Vdbe.aOp array so that it is at least one op larger than 
** it was.
**
** If an out-of-memory error occurs while resizing the array, return
** SQLITE_NOMEM. In this case Vdbe.aOp and Vdbe.nOpAlloc remain 
** unchanged (this is so that any opcodes already allocated can be 
** correctly deallocated along with the rest of the Vdbe).
*/
static int growOpArray(Vdbe *p){
  VdbeOp *pNew;
  int nNew = (p->nOpAlloc ? p->nOpAlloc*2 : (int)(1024/sizeof(Op)));
  pNew = sqlite3DbRealloc(p->db, p->aOp, nNew*sizeof(Op));
  if( pNew ){
    p->nOpAlloc = sqlite3DbMallocSize(p->db, pNew)/sizeof(Op);
    p->aOp = pNew;
  }
  return (pNew ? SQLITE_OK : SQLITE_NOMEM);
}

/*
** Add a new instruction to the list of instructions current in the
** VDBE.  Return the address of the new instruction.
**
** Parameters:
**
**    p               Pointer to the VDBE
**
**    op              The opcode for this instruction
**
**    p1, p2, p3      Operands
**
** Use the sqlite3VdbeResolveLabel() function to fix an address and
** the sqlite3VdbeChangeP4() function to change the value of the P4
** operand.
*/
SQLITE_PRIVATE int sqlite3VdbeAddOp3(Vdbe *p, int op, int p1, int p2, int p3){
  int i;
  VdbeOp *pOp;

  i = p->nOp;
  assert( p->magic==VDBE_MAGIC_INIT );
  assert( op>0 && op<0xff );
  if( p->nOpAlloc<=i ){
    if( growOpArray(p) ){
      return 1;
    }
  }
  p->nOp++;
  pOp = &p->aOp[i];
  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.p = 0;
  pOp->p4type = P4_NOTUSED;
#ifdef SQLITE_DEBUG
  pOp->zComment = 0;
  if( sqlite3VdbeAddopTrace ) sqlite3VdbePrintOp(0, i, &p->aOp[i]);
#endif
#ifdef VDBE_PROFILE
  pOp->cycles = 0;
  pOp->cnt = 0;
#endif
  return i;
}
SQLITE_PRIVATE int sqlite3VdbeAddOp0(Vdbe *p, int op){
  return sqlite3VdbeAddOp3(p, op, 0, 0, 0);
}
SQLITE_PRIVATE int sqlite3VdbeAddOp1(Vdbe *p, int op, int p1){
  return sqlite3VdbeAddOp3(p, op, p1, 0, 0);
}
SQLITE_PRIVATE int sqlite3VdbeAddOp2(Vdbe *p, int op, int p1, int p2){
  return sqlite3VdbeAddOp3(p, op, p1, p2, 0);
}


/*
** Add an opcode that includes the p4 value as a pointer.
*/
SQLITE_PRIVATE int sqlite3VdbeAddOp4(
  Vdbe *p,            /* Add the opcode to this VM */
  int op,             /* The new opcode */
  int p1,             /* The P1 operand */
  int p2,             /* The P2 operand */
  int p3,             /* The P3 operand */
  const char *zP4,    /* The P4 operand */
  int p4type          /* P4 operand type */
){
  int addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  return addr;
}

/*
** Add an OP_ParseSchema opcode.  This routine is broken out from
** sqlite3VdbeAddOp4() since it needs to also local all btrees.
**
** The zWhere string must have been obtained from sqlite3_malloc().
** This routine will take ownership of the allocated memory.
*/
SQLITE_PRIVATE void sqlite3VdbeAddParseSchemaOp(Vdbe *p, int iDb, char *zWhere){
  int j;
  int addr = sqlite3VdbeAddOp3(p, OP_ParseSchema, iDb, 0, 0);
  sqlite3VdbeChangeP4(p, addr, zWhere, P4_DYNAMIC);
  for(j=0; j<p->db->nDb; j++) sqlite3VdbeUsesBtree(p, j);
}

/*
** Add an opcode that includes the p4 value as an integer.
*/
SQLITE_PRIVATE int sqlite3VdbeAddOp4Int(
  Vdbe *p,            /* Add the opcode to this VM */
  int op,             /* The new opcode */
  int p1,             /* The P1 operand */
  int p2,             /* The P2 operand */
  int p3,             /* The P3 operand */
  int p4              /* The P4 operand as an integer */
){
  int addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, SQLITE_INT_TO_PTR(p4), P4_INT32);
  return addr;
}

/*
** Create a new symbolic label for an instruction that has yet to be
** coded.  The symbolic label is really just a negative number.  The
** label can be used as the P2 value of an operation.  Later, when
** the label is resolved to a specific address, the VDBE will scan
** through its operation list and change all values of P2 which match
** the label into the resolved address.
**
** The VDBE knows that a P2 value is a label because labels are
** always negative and P2 values are suppose to be non-negative.
** Hence, a negative P2 value is a label that has yet to be resolved.
**
** Zero is returned if a malloc() fails.
*/
SQLITE_PRIVATE int sqlite3VdbeMakeLabel(Vdbe *p){
  int i;
  i = p->nLabel++;
  assert( p->magic==VDBE_MAGIC_INIT );
  if( i>=p->nLabelAlloc ){
    int n = p->nLabelAlloc*2 + 5;
    p->aLabel = sqlite3DbReallocOrFree(p->db, p->aLabel,
                                       n*sizeof(p->aLabel[0]));
    p->nLabelAlloc = sqlite3DbMallocSize(p->db, p->aLabel)/sizeof(p->aLabel[0]);
  }
  if( p->aLabel ){
    p->aLabel[i] = -1;
  }
  return -1-i;
}

/*
** Resolve label "x" to be the address of the next instruction to
** be inserted.  The parameter "x" must have been obtained from
** a prior call to sqlite3VdbeMakeLabel().
*/
SQLITE_PRIVATE void sqlite3VdbeResolveLabel(Vdbe *p, int x){
  int j = -1-x;
  assert( p->magic==VDBE_MAGIC_INIT );
  assert( j>=0 && j<p->nLabel );
  if( p->aLabel ){
    p->aLabel[j] = p->nOp;
  }
}

/*
** Mark the VDBE as one that can only be run one time.
*/
SQLITE_PRIVATE void sqlite3VdbeRunOnlyOnce(Vdbe *p){
  p->runOnlyOnce = 1;
}

#ifdef SQLITE_DEBUG /* sqlite3AssertMayAbort() logic */

/*
** The following type and function are used to iterate through all opcodes
** in a Vdbe main program and each of the sub-programs (triggers) it may 
** invoke directly or indirectly. It should be used as follows:
**
**   Op *pOp;
**   VdbeOpIter sIter;
**
**   memset(&sIter, 0, sizeof(sIter));
**   sIter.v = v;                            // v is of type Vdbe* 
**   while( (pOp = opIterNext(&sIter)) ){
**     // Do something with pOp
**   }
**   sqlite3DbFree(v->db, sIter.apSub);
** 
*/
typedef struct VdbeOpIter VdbeOpIter;
struct VdbeOpIter {
  Vdbe *v;                   /* Vdbe to iterate through the opcodes of */
  SubProgram **apSub;        /* Array of subprograms */
  int nSub;                  /* Number of entries in apSub */
  int iAddr;                 /* Address of next instruction to return */
  int iSub;                  /* 0 = main program, 1 = first sub-program etc. */
};
static Op *opIterNext(VdbeOpIter *p){
  Vdbe *v = p->v;
  Op *pRet = 0;
  Op *aOp;
  int nOp;

  if( p->iSub<=p->nSub ){

    if( p->iSub==0 ){
      aOp = v->aOp;
      nOp = v->nOp;
    }else{
      aOp = p->apSub[p->iSub-1]->aOp;
      nOp = p->apSub[p->iSub-1]->nOp;
    }
    assert( p->iAddr<nOp );

    pRet = &aOp[p->iAddr];
    p->iAddr++;
    if( p->iAddr==nOp ){
      p->iSub++;
      p->iAddr = 0;
    }
  
    if( pRet->p4type==P4_SUBPROGRAM ){
      int nByte = (p->nSub+1)*sizeof(SubProgram*);
      int j;
      for(j=0; j<p->nSub; j++){
        if( p->apSub[j]==pRet->p4.pProgram ) break;
      }
      if( j==p->nSub ){
        p->apSub = sqlite3DbReallocOrFree(v->db, p->apSub, nByte);
        if( !p->apSub ){
          pRet = 0;
        }else{
          p->apSub[p->nSub++] = pRet->p4.pProgram;
        }
      }
    }
  }

  return pRet;
}

/*
** Check if the program stored in the VM associated with pParse may
** throw an ABORT exception (causing the statement, but not entire transaction
** to be rolled back). This condition is true if the main program or any
** sub-programs contains any of the following:
**
**   *  OP_Halt with P1=SQLITE_CONSTRAINT and P2=OE_Abort.
**   *  OP_HaltIfNull with P1=SQLITE_CONSTRAINT and P2=OE_Abort.
**   *  OP_Destroy
**   *  OP_VUpdate
**   *  OP_VRename
**   *  OP_FkCounter with P2==0 (immediate foreign key constraint)
**
** Then check that the value of Parse.mayAbort is true if an
** ABORT may be thrown, or false otherwise. Return true if it does
** match, or false otherwise. This function is intended to be used as
** part of an assert statement in the compiler. Similar to:
**
**   assert( sqlite3VdbeAssertMayAbort(pParse->pVdbe, pParse->mayAbort) );
*/
SQLITE_PRIVATE int sqlite3VdbeAssertMayAbort(Vdbe *v, int mayAbort){
  int hasAbort = 0;
  Op *pOp;
  VdbeOpIter sIter;
  memset(&sIter, 0, sizeof(sIter));
  sIter.v = v;

  while( (pOp = opIterNext(&sIter))!=0 ){
    int opcode = pOp->opcode;
    if( opcode==OP_Destroy || opcode==OP_VUpdate || opcode==OP_VRename 
#ifndef SQLITE_OMIT_FOREIGN_KEY
     || (opcode==OP_FkCounter && pOp->p1==0 && pOp->p2==1) 
#endif
     || ((opcode==OP_Halt || opcode==OP_HaltIfNull) 
      && (pOp->p1==SQLITE_CONSTRAINT && pOp->p2==OE_Abort))
    ){
      hasAbort = 1;
      break;
    }
  }
  sqlite3DbFree(v->db, sIter.apSub);

  /* Return true if hasAbort==mayAbort. Or if a malloc failure occured.
  ** If malloc failed, then the while() loop above may not have iterated
  ** through all opcodes and hasAbort may be set incorrectly. Return
  ** true for this case to prevent the assert() in the callers frame
  ** from failing.  */
  return ( v->db->mallocFailed || hasAbort==mayAbort );
}
#endif /* SQLITE_DEBUG - the sqlite3AssertMayAbort() function */

/*
** Loop through the program looking for P2 values that are negative
** on jump instructions.  Each such value is a label.  Resolve the
** label by setting the P2 value to its correct non-zero value.
**
** This routine is called once after all opcodes have been inserted.
**
** Variable *pMaxFuncArgs is set to the maximum value of any P2 argument 
** to an OP_Function, OP_AggStep or OP_VFilter opcode. This is used by 
** sqlite3VdbeMakeReady() to size the Vdbe.apArg[] array.
**
** The Op.opflags field is set on all opcodes.
*/
static void resolveP2Values(Vdbe *p, int *pMaxFuncArgs){
  int i;
  int nMaxArgs = *pMaxFuncArgs;
  Op *pOp;
  int *aLabel = p->aLabel;
  p->readOnly = 1;
  for(pOp=p->aOp, i=p->nOp-1; i>=0; i--, pOp++){
    u8 opcode = pOp->opcode;

    pOp->opflags = sqlite3OpcodeProperty[opcode];
    if( opcode==OP_Function || opcode==OP_AggStep ){
      if( pOp->p5>nMaxArgs ) nMaxArgs = pOp->p5;
    }else if( (opcode==OP_Transaction && pOp->p2!=0) || opcode==OP_Vacuum ){
      p->readOnly = 0;
#ifndef SQLITE_OMIT_VIRTUALTABLE
    }else if( opcode==OP_VUpdate ){
      if( pOp->p2>nMaxArgs ) nMaxArgs = pOp->p2;
    }else if( opcode==OP_VFilter ){
      int n;
      assert( p->nOp - i >= 3 );
      assert( pOp[-1].opcode==OP_Integer );
      n = pOp[-1].p1;
      if( n>nMaxArgs ) nMaxArgs = n;
#endif
    }

    if( (pOp->opflags & OPFLG_JUMP)!=0 && pOp->p2<0 ){
      assert( -1-pOp->p2<p->nLabel );
      pOp->p2 = aLabel[-1-pOp->p2];
    }
  }
  sqlite3DbFree(p->db, p->aLabel);
  p->aLabel = 0;

  *pMaxFuncArgs = nMaxArgs;
}

/*
** Return the address of the next instruction to be inserted.
*/
SQLITE_PRIVATE int sqlite3VdbeCurrentAddr(Vdbe *p){
  assert( p->magic==VDBE_MAGIC_INIT );
  return p->nOp;
}

/*
** This function returns a pointer to the array of opcodes associated with
** the Vdbe passed as the first argument. It is the callers responsibility
** to arrange for the returned array to be eventually freed using the 
** vdbeFreeOpArray() function.
**
** Before returning, *pnOp is set to the number of entries in the returned
** array. Also, *pnMaxArg is set to the larger of its current value and 
** the number of entries in the Vdbe.apArg[] array required to execute the 
** returned program.
*/
SQLITE_PRIVATE VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, int *pnOp, int *pnMaxArg){
  VdbeOp *aOp = p->aOp;
  assert( aOp && !p->db->mallocFailed );

  /* Check that sqlite3VdbeUsesBtree() was not called on this VM */
  assert( p->btreeMask==0 );

  resolveP2Values(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  return aOp;
}

/*
** Add a whole list of operations to the operation stack.  Return the
** address of the first operation added.
*/
SQLITE_PRIVATE int sqlite3VdbeAddOpList(Vdbe *p, int nOp, VdbeOpList const *aOp){
  int addr;
  assert( p->magic==VDBE_MAGIC_INIT );
  if( p->nOp + nOp > p->nOpAlloc && growOpArray(p) ){
    return 0;
  }
  addr = p->nOp;
  if( ALWAYS(nOp>0) ){
    int i;
    VdbeOpList const *pIn = aOp;
    for(i=0; i<nOp; i++, pIn++){
      int p2 = pIn->p2;
      VdbeOp *pOut = &p->aOp[i+addr];
      pOut->opcode = pIn->opcode;
      pOut->p1 = pIn->p1;
      if( p2<0 && (sqlite3OpcodeProperty[pOut->opcode] & OPFLG_JUMP)!=0 ){
        pOut->p2 = addr + ADDR(p2);
      }else{
        pOut->p2 = p2;
      }
      pOut->p3 = pIn->p3;
      pOut->p4type = P4_NOTUSED;
      pOut->p4.p = 0;
      pOut->p5 = 0;
#ifdef SQLITE_DEBUG
      pOut->zComment = 0;
      if( sqlite3VdbeAddopTrace ){
        sqlite3VdbePrintOp(0, i+addr, &p->aOp[i+addr]);
      }
#endif
    }
    p->nOp += nOp;
  }
  return addr;
}

/*
** Change the value of the P1 operand for a specific instruction.
** This routine is useful when a large program is loaded from a
** static array using sqlite3VdbeAddOpList but we want to make a
** few minor changes to the program.
*/
SQLITE_PRIVATE void sqlite3VdbeChangeP1(Vdbe *p, int addr, int val){
  assert( p!=0 );
  assert( addr>=0 );
  if( p->nOp>addr ){
    p->aOp[addr].p1 = val;
  }
}

/*
** Change the value of the P2 operand for a specific instruction.
** This routine is useful for setting a jump destination.
*/
SQLITE_PRIVATE void sqlite3VdbeChangeP2(Vdbe *p, int addr, int val){
  assert( p!=0 );
  assert( addr>=0 );
  if( p->nOp>addr ){
    p->aOp[addr].p2 = val;
  }
}

/*
** Change the value of the P3 operand for a specific instruction.
*/
SQLITE_PRIVATE void sqlite3VdbeChangeP3(Vdbe *p, int addr, int val){
  assert( p!=0 );
  assert( addr>=0 );
  if( p->nOp>addr ){
    p->aOp[addr].p3 = val;
  }
}

/*
** Change the value of the P5 operand for the most recently
** added operation.
*/
SQLITE_PRIVATE void sqlite3VdbeChangeP5(Vdbe *p, u8 val){
  assert( p!=0 );
  if( p->aOp ){
    assert( p->nOp>0 );
    p->aOp[p->nOp-1].p5 = val;
  }
}

/*
** Change the P2 operand of instruction addr so that it points to
** the address of the next instruction to be coded.
*/
SQLITE_PRIVATE void sqlite3VdbeJumpHere(Vdbe *p, int addr){
  assert( addr>=0 );
  sqlite3VdbeChangeP2(p, addr, p->nOp);
}


/*
** If the input FuncDef structure is ephemeral, then free it.  If
** the FuncDef is not ephermal, then do nothing.
*/
static void freeEphemeralFunction(sqlite3 *db, FuncDef *pDef){
  if( ALWAYS(pDef) && (pDef->flags & SQLITE_FUNC_EPHEM)!=0 ){
    sqlite3DbFree(db, pDef);
  }
}

static void vdbeFreeOpArray(sqlite3 *, Op *, int);

/*
** Delete a P4 value if necessary.
*/
static void freeP4(sqlite3 *db, int p4type, void *p4){
  if( p4 ){
    assert( db );
    switch( p4type ){
      case P4_REAL:
      case P4_INT64:
      case P4_DYNAMIC:
      case P4_KEYINFO:
      case P4_INTARRAY:
      case P4_KEYINFO_HANDOFF: {
        sqlite3DbFree(db, p4);
        break;
      }
      case P4_MPRINTF: {
        if( db->pnBytesFreed==0 ) sqlite3_free(p4);
        break;
      }
      case P4_VDBEFUNC: {
        VdbeFunc *pVdbeFunc = (VdbeFunc *)p4;
        freeEphemeralFunction(db, pVdbeFunc->pFunc);
        if( db->pnBytesFreed==0 ) sqlite3VdbeDeleteAuxData(pVdbeFunc, 0);
        sqlite3DbFree(db, pVdbeFunc);
        break;
      }
      case P4_FUNCDEF: {
        freeEphemeralFunction(db, (FuncDef*)p4);
        break;
      }
      case P4_MEM: {
        if( db->pnBytesFreed==0 ){
          sqlite3ValueFree((sqlite3_value*)p4);
        }else{
          Mem *p = (Mem*)p4;
          sqlite3DbFree(db, p->zMalloc);
          sqlite3DbFree(db, p);
        }
        break;
      }
      case P4_VTAB : {
        if( db->pnBytesFreed==0 ) sqlite3VtabUnlock((VTable *)p4);
        break;
      }
    }
  }
}

/*
** Free the space allocated for aOp and any p4 values allocated for the
** opcodes contained within. If aOp is not NULL it is assumed to contain 
** nOp entries. 
*/
static void vdbeFreeOpArray(sqlite3 *db, Op *aOp, int nOp){
  if( aOp ){
    Op *pOp;
    for(pOp=aOp; pOp<&aOp[nOp]; pOp++){
      freeP4(db, pOp->p4type, pOp->p4.p);
#ifdef SQLITE_DEBUG
      sqlite3DbFree(db, pOp->zComment);
#endif     
    }
  }
  sqlite3DbFree(db, aOp);
}

/*
** Link the SubProgram object passed as the second argument into the linked
** list at Vdbe.pSubProgram. This list is used to delete all sub-program
** objects when the VM is no longer required.
*/
SQLITE_PRIVATE void sqlite3VdbeLinkSubProgram(Vdbe *pVdbe, SubProgram *p){
  p->pNext = pVdbe->pProgram;
  pVdbe->pProgram = p;
}

/*
** Change N opcodes starting at addr to No-ops.
*/
SQLITE_PRIVATE void sqlite3VdbeChangeToNoop(Vdbe *p, int addr, int N){
  if( p->aOp ){
    VdbeOp *pOp = &p->aOp[addr];
    sqlite3 *db = p->db;
    while( N-- ){
      freeP4(db, pOp->p4type, pOp->p4.p);
      memset(pOp, 0, sizeof(pOp[0]));
      pOp->opcode = OP_Noop;
      pOp++;
    }
  }
}

/*
** Change the value of the P4 operand for a specific instruction.
** This routine is useful when a large program is loaded from a
** static array using sqlite3VdbeAddOpList but we want to make a
** few minor changes to the program.
**
** If n>=0 then the P4 operand is dynamic, meaning that a copy of
** the string is made into memory obtained from sqlite3_malloc().
** A value of n==0 means copy bytes of zP4 up to and including the
** first null byte.  If n>0 then copy n+1 bytes of zP4.
**
** If n==P4_KEYINFO it means that zP4 is a pointer to a KeyInfo structure.
** A copy is made of the KeyInfo structure into memory obtained from
** sqlite3_malloc, to be freed when the Vdbe is finalized.
** n==P4_KEYINFO_HANDOFF indicates that zP4 points to a KeyInfo structure
** stored in memory that the caller has obtained from sqlite3_malloc. The 
** caller should not free the allocation, it will be freed when the Vdbe is
** finalized.
** 
** Other values of n (P4_STATIC, P4_COLLSEQ etc.) indicate that zP4 points
** to a string or structure that is guaranteed to exist for the lifetime of
** the Vdbe. In these cases we can just copy the pointer.
**
** If addr<0 then change P4 on the most recently inserted instruction.
*/
SQLITE_PRIVATE void sqlite3VdbeChangeP4(Vdbe *p, int addr, const char *zP4, int n){
  Op *pOp;
  sqlite3 *db;
  assert( p!=0 );
  db = p->db;
  assert( p->magic==VDBE_MAGIC_INIT );
  if( p->aOp==0 || db->mallocFailed ){
    if ( n!=P4_KEYINFO && n!=P4_VTAB ) {
      freeP4(db, n, (void*)*(char**)&zP4);
    }
    return;
  }
  assert( p->nOp>0 );
  assert( addr<p->nOp );
  if( addr<0 ){
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  freeP4(db, pOp->p4type, pOp->p4.p);
  pOp->p4.p = 0;
  if( n==P4_INT32 ){
    /* Note: this cast is safe, because the origin data point was an int
    ** that was cast to a (const char *). */
    pOp->p4.i = SQLITE_PTR_TO_INT(zP4);
    pOp->p4type = P4_INT32;
  }else if( zP4==0 ){
    pOp->p4.p = 0;
    pOp->p4type = P4_NOTUSED;
  }else if( n==P4_KEYINFO ){
    KeyInfo *pKeyInfo;
    int nField, nByte;

    nField = ((KeyInfo*)zP4)->nField;
    nByte = sizeof(*pKeyInfo) + (nField-1)*sizeof(pKeyInfo->aColl[0]) + nField;
    pKeyInfo = sqlite3DbMallocRaw(0, nByte);
    pOp->p4.pKeyInfo = pKeyInfo;
    if( pKeyInfo ){
      u8 *aSortOrder;
      memcpy((char*)pKeyInfo, zP4, nByte - nField);
      aSortOrder = pKeyInfo->aSortOrder;
      if( aSortOrder ){
        pKeyInfo->aSortOrder = (unsigned char*)&pKeyInfo->aColl[nField];
        memcpy(pKeyInfo->aSortOrder, aSortOrder, nField);
      }
      pOp->p4type = P4_KEYINFO;
    }else{
      p->db->mallocFailed = 1;
      pOp->p4type = P4_NOTUSED;
    }
  }else if( n==P4_KEYINFO_HANDOFF ){
    pOp->p4.p = (void*)zP4;
    pOp->p4type = P4_KEYINFO;
  }else if( n==P4_VTAB ){
    pOp->p4.p = (void*)zP4;
    pOp->p4type = P4_VTAB;
    sqlite3VtabLock((VTable *)zP4);
    assert( ((VTable *)zP4)->db==p->db );
  }else if( n<0 ){
    pOp->p4.p = (void*)zP4;
    pOp->p4type = (signed char)n;
  }else{
    if( n==0 ) n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = P4_DYNAMIC;
  }
}

#ifndef NDEBUG
/*
** Change the comment on the the most recently coded instruction.  Or
** insert a No-op and add the comment to that new instruction.  This
** makes the code easier to read during debugging.  None of this happens
** in a production build.
*/
SQLITE_PRIVATE void sqlite3VdbeComment(Vdbe *p, const char *zFormat, ...){
  va_list ap;
  if( !p ) return;
  assert( p->nOp>0 || p->aOp==0 );
  assert( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->db->mallocFailed );
  if( p->nOp ){
    char **pz = &p->aOp[p->nOp-1].zComment;
    va_start(ap, zFormat);
    sqlite3DbFree(p->db, *pz);
    *pz = sqlite3VMPrintf(p->db, zFormat, ap);
    va_end(ap);
  }
}
SQLITE_PRIVATE void sqlite3VdbeNoopComment(Vdbe *p, const char *zFormat, ...){
  va_list ap;
  if( !p ) return;
  sqlite3VdbeAddOp0(p, OP_Noop);
  assert( p->nOp>0 || p->aOp==0 );
  assert( p->aOp==0 || p->aOp[p->nOp-1].zComment==0 || p->db->mallocFailed );
  if( p->nOp ){
    char **pz = &p->aOp[p->nOp-1].zComment;
    va_start(ap, zFormat);
    sqlite3DbFree(p->db, *pz);
    *pz = sqlite3VMPrintf(p->db, zFormat, ap);
    va_end(ap);
  }
}
#endif  /* NDEBUG */

/*
** Return the opcode for a given address.  If the address is -1, then
** return the most recently inserted opcode.
**
** If a memory allocation error has occurred prior to the calling of this
** routine, then a pointer to a dummy VdbeOp will be returned.  That opcode
** is readable but not writable, though it is cast to a writable value.
** The return of a dummy opcode allows the call to continue functioning
** after a OOM fault without having to check to see if the return from 
** this routine is a valid pointer.  But because the dummy.opcode is 0,
** dummy will never be written to.  This is verified by code inspection and
** by running with Valgrind.
**
** About the #ifdef SQLITE_OMIT_TRACE:  Normally, this routine is never called
** unless p->nOp>0.  This is because in the absense of SQLITE_OMIT_TRACE,
** an OP_Trace instruction is always inserted by sqlite3VdbeGet() as soon as
** a new VDBE is created.  So we are free to set addr to p->nOp-1 without
** having to double-check to make sure that the result is non-negative. But
** if SQLITE_OMIT_TRACE is defined, the OP_Trace is omitted and we do need to
** check the value of p->nOp-1 before continuing.
*/
SQLITE_PRIVATE VdbeOp *sqlite3VdbeGetOp(Vdbe *p, int addr){
  /* C89 specifies that the constant "dummy" will be initialized to all
  ** zeros, which is correct.  MSVC generates a warning, nevertheless. */
  static const VdbeOp dummy;  /* Ignore the MSVC warning about no initializer */
  assert( p->magic==VDBE_MAGIC_INIT );
  if( addr<0 ){
#ifdef SQLITE_OMIT_TRACE
    if( p->nOp==0 ) return (VdbeOp*)&dummy;
#endif
    addr = p->nOp - 1;
  }
  assert( (addr>=0 && addr<p->nOp) || p->db->mallocFailed );
  if( p->db->mallocFailed ){
    return (VdbeOp*)&dummy;
  }else{
    return &p->aOp[addr];
  }
}

#if !defined(SQLITE_OMIT_EXPLAIN) || !defined(NDEBUG) \
     || defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Compute a string that describes the P4 parameter for an opcode.
** Use zTemp for any required temporary buffer space.
*/
static char *displayP4(Op *pOp, char *zTemp, int nTemp){
  char *zP4 = zTemp;
  assert( nTemp>=20 );
  switch( pOp->p4type ){
    case P4_KEYINFO_STATIC:
    case P4_KEYINFO: {
      int i, j;
      KeyInfo *pKeyInfo = pOp->p4.pKeyInfo;
      sqlite3_snprintf(nTemp, zTemp, "keyinfo(%d", pKeyInfo->nField);
      i = sqlite3Strlen30(zTemp);
      for(j=0; j<pKeyInfo->nField; j++){
        CollSeq *pColl = pKeyInfo->aColl[j];
        if( pColl ){
          int n = sqlite3Strlen30(pColl->zName);
          if( i+n>nTemp-6 ){
            memcpy(&zTemp[i],",...",4);
            break;
          }
          zTemp[i++] = ',';
          if( pKeyInfo->aSortOrder && pKeyInfo->aSortOrder[j] ){
            zTemp[i++] = '-';
          }
          memcpy(&zTemp[i], pColl->zName,n+1);
          i += n;
        }else if( i+4<nTemp-6 ){
          memcpy(&zTemp[i],",nil",4);
          i += 4;
        }
      }
      zTemp[i++] = ')';
      zTemp[i] = 0;
      assert( i<nTemp );
      break;
    }
    case P4_COLLSEQ: {
      CollSeq *pColl = pOp->p4.pColl;
      sqlite3_snprintf(nTemp, zTemp, "collseq(%.20s)", pColl->zName);
      break;
    }
    case P4_FUNCDEF: {
      FuncDef *pDef = pOp->p4.pFunc;
      sqlite3_snprintf(nTemp, zTemp, "%s(%d)", pDef->zName, pDef->nArg);
      break;
    }
    case P4_INT64: {
      sqlite3_snprintf(nTemp, zTemp, "%lld", *pOp->p4.pI64);
      break;
    }
    case P4_INT32: {
      sqlite3_snprintf(nTemp, zTemp, "%d", pOp->p4.i);
      break;
    }
    case P4_REAL: {
      sqlite3_snprintf(nTemp, zTemp, "%.16g", *pOp->p4.pReal);
      break;
    }
    case P4_MEM: {
      Mem *pMem = pOp->p4.pMem;
      assert( (pMem->flags & MEM_Null)==0 );
      if( pMem->flags & MEM_Str ){
        zP4 = pMem->z;
      }else if( pMem->flags & MEM_Int ){
        sqlite3_snprintf(nTemp, zTemp, "%lld", pMem->u.i);
      }else if( pMem->flags & MEM_Real ){
        sqlite3_snprintf(nTemp, zTemp, "%.16g", pMem->r);
      }else{
        assert( pMem->flags & MEM_Blob );
        zP4 = "(blob)";
      }
      break;
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    case P4_VTAB: {
      sqlite3_vtab *pVtab = pOp->p4.pVtab->pVtab;
      sqlite3_snprintf(nTemp, zTemp, "vtab:%p:%p", pVtab, pVtab->pModule);
      break;
    }
#endif
    case P4_INTARRAY: {
      sqlite3_snprintf(nTemp, zTemp, "intarray");
      break;
    }
    case P4_SUBPROGRAM: {
      sqlite3_snprintf(nTemp, zTemp, "program");
      break;
    }
    default: {
      zP4 = pOp->p4.z;
      if( zP4==0 ){
        zP4 = zTemp;
        zTemp[0] = 0;
      }
    }
  }
  assert( zP4!=0 );
  return zP4;
}
#endif

/*
** Declare to the Vdbe that the BTree object at db->aDb[i] is used.
**
** The prepared statements need to know in advance the complete set of
** attached databases that they will be using.  A mask of these databases
** is maintained in p->btreeMask and is used for locking and other purposes.
*/
SQLITE_PRIVATE void sqlite3VdbeUsesBtree(Vdbe *p, int i){
  assert( i>=0 && i<p->db->nDb && i<(int)sizeof(yDbMask)*8 );
  assert( i<(int)sizeof(p->btreeMask)*8 );
  p->btreeMask |= ((yDbMask)1)<<i;
  if( i!=1 && sqlite3BtreeSharable(p->db->aDb[i].pBt) ){
    p->lockMask |= ((yDbMask)1)<<i;
  }
}

#if !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** If SQLite is compiled to support shared-cache mode and to be threadsafe,
** this routine obtains the mutex associated with each BtShared structure
** that may be accessed by the VM passed as an argument. In doing so it also
** sets the BtShared.db member of each of the BtShared structures, ensuring
** that the correct busy-handler callback is invoked if required.
**
** If SQLite is not threadsafe but does support shared-cache mode, then
** sqlite3BtreeEnter() is invoked to set the BtShared.db variables
** of all of BtShared structures accessible via the database handle 
** associated with the VM.
**
** If SQLite is not threadsafe and does not support shared-cache mode, this
** function is a no-op.
**
** The p->btreeMask field is a bitmask of all btrees that the prepared 
** statement p will ever use.  Let N be the number of bits in p->btreeMask
** corresponding to btrees that use shared cache.  Then the runtime of
** this routine is N*N.  But as N is rarely more than 1, this should not
** be a problem.
*/
SQLITE_PRIVATE void sqlite3VdbeEnter(Vdbe *p){
  int i;
  yDbMask mask;
  sqlite3 *db;
  Db *aDb;
  int nDb;
  if( p->lockMask==0 ) return;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  for(i=0, mask=1; i<nDb; i++, mask += mask){
    if( i!=1 && (mask & p->lockMask)!=0 && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}
#endif

#if !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE>0
/*
** Unlock all of the btrees previously locked by a call to sqlite3VdbeEnter().
*/
SQLITE_PRIVATE void sqlite3VdbeLeave(Vdbe *p){
  int i;
  yDbMask mask;
  sqlite3 *db;
  Db *aDb;
  int nDb;
  if( p->lockMask==0 ) return;  /* The common case */
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  for(i=0, mask=1; i<nDb; i++, mask += mask){
    if( i!=1 && (mask & p->lockMask)!=0 && ALWAYS(aDb[i].pBt!=0) ){
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}
#endif

#if defined(VDBE_PROFILE) || defined(SQLITE_DEBUG)
/*
** Print a single opcode.  This routine is used for debugging only.
*/
SQLITE_PRIVATE void sqlite3VdbePrintOp(FILE *pOut, int pc, Op *pOp){
  char *zP4;
  char zPtr[50];
  static const char *zFormat1 = "%4d %-13s %4d %4d %4d %-4s %.2X %s\n";
  if( pOut==0 ) pOut = stdout;
  zP4 = displayP4(pOp, zPtr, sizeof(zPtr));
  fprintf(pOut, zFormat1, pc, 
      sqlite3OpcodeName(pOp->opcode), pOp->p1, pOp->p2, pOp->p3, zP4, pOp->p5,
#ifdef SQLITE_DEBUG
      pOp->zComment ? pOp->zComment : ""
#else
      ""
#endif
  );
  fflush(pOut);
}
#endif

/*
** Release an array of N Mem elements
*/
static void releaseMemArray(Mem *p, int N){
  if( p && N ){
    Mem *pEnd;
    sqlite3 *db = p->db;
    u8 malloc_failed = db->mallocFailed;
    if( db->pnBytesFreed ){
      for(pEnd=&p[N]; p<pEnd; p++){
        sqlite3DbFree(db, p->zMalloc);
      }
      return;
    }
    for(pEnd=&p[N]; p<pEnd; p++){
      assert( (&p[1])==pEnd || p[0].db==p[1].db );

      /* This block is really an inlined version of sqlite3VdbeMemRelease()
      ** that takes advantage of the fact that the memory cell value is 
      ** being set to NULL after releasing any dynamic resources.
      **
      ** The justification for duplicating code is that according to 
      ** callgrind, this causes a certain test case to hit the CPU 4.7 
      ** percent less (x86 linux, gcc version 4.1.2, -O6) than if 
      ** sqlite3MemRelease() were called from here. With -O2, this jumps
      ** to 6.6 percent. The test case is inserting 1000 rows into a table 
      ** with no indexes using a single prepared INSERT statement, bind() 
      ** and reset(). Inserts are grouped into a transaction.
      */
      if( p->flags&(MEM_Agg|MEM_Dyn|MEM_Frame|MEM_RowSet) ){
        sqlite3VdbeMemRelease(p);
      }else if( p->zMalloc ){
        sqlite3DbFree(db, p->zMalloc);
        p->zMalloc = 0;
      }

      p->flags = MEM_Null;
    }
    db->mallocFailed = malloc_failed;
  }
}

/*
** Delete a VdbeFrame object and its contents. VdbeFrame objects are
** allocated by the OP_Program opcode in sqlite3VdbeExec().
*/
SQLITE_PRIVATE void sqlite3VdbeFrameDelete(VdbeFrame *p){
  int i;
  Mem *aMem = VdbeFrameMem(p);
  VdbeCursor **apCsr = (VdbeCursor **)&aMem[p->nChildMem];
  for(i=0; i<p->nChildCsr; i++){
    sqlite3VdbeFreeCursor(p->v, apCsr[i]);
  }
  releaseMemArray(aMem, p->nChildMem);
  sqlite3DbFree(p->v->db, p);
}

#ifndef SQLITE_OMIT_EXPLAIN
/*
** Give a listing of the program in the virtual machine.
**
** The interface is the same as sqlite3VdbeExec().  But instead of
** running the code, it invokes the callback once for each instruction.
** This feature is used to implement "EXPLAIN".
**
** When p->explain==1, each instruction is listed.  When
** p->explain==2, only OP_Explain instructions are listed and these
** are shown in a different format.  p->explain==2 is used to implement
** EXPLAIN QUERY PLAN.
**
** When p->explain==1, first the main program is listed, then each of
** the trigger subprograms are listed one by one.
*/
SQLITE_PRIVATE int sqlite3VdbeList(
  Vdbe *p                   /* The VDBE */
){
  int nRow;                            /* Stop when row count reaches this */
  int nSub = 0;                        /* Number of sub-vdbes seen so far */
  SubProgram **apSub = 0;              /* Array of sub-vdbes */
  Mem *pSub = 0;                       /* Memory cell hold array of subprogs */
  sqlite3 *db = p->db;                 /* The database connection */
  int i;                               /* Loop counter */
  int rc = SQLITE_OK;                  /* Return code */
  Mem *pMem = p->pResultSet = &p->aMem[1];  /* First Mem of result set */

  assert( p->explain );
  assert( p->magic==VDBE_MAGIC_RUN );
  assert( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY || p->rc==SQLITE_NOMEM );

  /* Even though this opcode does not use dynamic strings for
  ** the result, result columns may become dynamic if the user calls
  ** sqlite3_column_text16(), causing a translation to UTF-16 encoding.
  */
  releaseMemArray(pMem, 8);

  if( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    db->mallocFailed = 1;
    return SQLITE_ERROR;
  }

  /* When the number of output rows reaches nRow, that means the
  ** listing has finished and sqlite3_step() should return SQLITE_DONE.
  ** nRow is the sum of the number of rows in the main program, plus
  ** the sum of the number of rows in all trigger subprograms encountered
  ** so far.  The nRow value will increase as new trigger subprograms are
  ** encountered, but p->pc will eventually catch up to nRow.
  */
  nRow = p->nOp;
  if( p->explain==1 ){
    /* The first 8 memory cells are used for the result set.  So we will
    ** commandeer the 9th cell to use as storage for an array of pointers
    ** to trigger subprograms.  The VDBE is guaranteed to have at least 9
    ** cells.  */
    assert( p->nMem>9 );
    pSub = &p->aMem[9];
    if( pSub->flags&MEM_Blob ){
      /* On the first call to sqlite3_step(), pSub will hold a NULL.  It is
      ** initialized to a BLOB by the P4_SUBPROGRAM processing logic below */
      nSub = pSub->n/sizeof(Vdbe*);
      apSub = (SubProgram **)pSub->z;
    }
    for(i=0; i<nSub; i++){
      nRow += apSub[i]->nOp;
    }
  }

  do{
    i = p->pc++;
  }while( i<nRow && p->explain==2 && p->aOp[i].opcode!=OP_Explain );
  if( i>=nRow ){
    p->rc = SQLITE_OK;
    rc = SQLITE_DONE;
  }else if( db->u1.isInterrupted ){
    p->rc = SQLITE_INTERRUPT;
    rc = SQLITE_ERROR;
    sqlite3SetString(&p->zErrMsg, db, "%s", sqlite3ErrStr(p->rc));
  }else{
    char *z;
    Op *pOp;
    if( i<p->nOp ){
      /* The output line number is small enough that we are still in the
      ** main program. */
      pOp = &p->aOp[i];
    }else{
      /* We are currently listing subprograms.  Figure out which one and
      ** pick up the appropriate opcode. */
      int j;
      i -= p->nOp;
      for(j=0; i>=apSub[j]->nOp; j++){
        i -= apSub[j]->nOp;
      }
      pOp = &apSub[j]->aOp[i];
    }
    if( p->explain==1 ){
      pMem->flags = MEM_Int;
      pMem->type = SQLITE_INTEGER;
      pMem->u.i = i;                                /* Program counter */
      pMem++;
  
      pMem->flags = MEM_Static|MEM_Str|MEM_Term;
      pMem->z = (char*)sqlite3OpcodeName(pOp->opcode);  /* Opcode */
      assert( pMem->z!=0 );
      pMem->n = sqlite3Strlen30(pMem->z);
      pMem->type = SQLITE_TEXT;
      pMem->enc = SQLITE_UTF8;
      pMem++;

      /* When an OP_Program opcode is encounter (the only opcode that has
      ** a P4_SUBPROGRAM argument), expand the size of the array of subprograms
      ** kept in p->aMem[9].z to hold the new program - assuming this subprogram
      ** has not already been seen.
      */
      if( pOp->p4type==P4_SUBPROGRAM ){
        int nByte = (nSub+1)*sizeof(SubProgram*);
        int j;
        for(j=0; j<nSub; j++){
          if( apSub[j]==pOp->p4.pProgram ) break;
        }
        if( j==nSub && SQLITE_OK==sqlite3VdbeMemGrow(pSub, nByte, 1) ){
          apSub = (SubProgram **)pSub->z;
          apSub[nSub++] = pOp->p4.pProgram;
          pSub->flags |= MEM_Blob;
          pSub->n = nSub*sizeof(SubProgram*);
        }
      }
    }

    pMem->flags = MEM_Int;
    pMem->u.i = pOp->p1;                          /* P1 */
    pMem->type = SQLITE_INTEGER;
    pMem++;

    pMem->flags = MEM_Int;
    pMem->u.i = pOp->p2;                          /* P2 */
    pMem->type = SQLITE_INTEGER;
    pMem++;

    pMem->flags = MEM_Int;
    pMem->u.i = pOp->p3;                          /* P3 */
    pMem->type = SQLITE_INTEGER;
    pMem++;

    if( sqlite3VdbeMemGrow(pMem, 32, 0) ){            /* P4 */
      assert( p->db->mallocFailed );
      return SQLITE_ERROR;
    }
    pMem->flags = MEM_Dyn|MEM_Str|MEM_Term;
    z = displayP4(pOp, pMem->z, 32);
    if( z!=pMem->z ){
      sqlite3VdbeMemSetStr(pMem, z, -1, SQLITE_UTF8, 0);
    }else{
      assert( pMem->z!=0 );
      pMem->n = sqlite3Strlen30(pMem->z);
      pMem->enc = SQLITE_UTF8;
    }
    pMem->type = SQLITE_TEXT;
    pMem++;

    if( p->explain==1 ){
      if( sqlite3VdbeMemGrow(pMem, 4, 0) ){
        assert( p->db->mallocFailed );
        return SQLITE_ERROR;
      }
      pMem->flags = MEM_Dyn|MEM_Str|MEM_Term;
      pMem->n = 2;
      sqlite3_snprintf(3, pMem->z, "%.2x", pOp->p5);   /* P5 */
      pMem->type = SQLITE_TEXT;
      pMem->enc = SQLITE_UTF8;
      pMem++;
  
#ifdef SQLITE_DEBUG
      if( pOp->zComment ){
        pMem->flags = MEM_Str|MEM_Term;
        pMem->z = pOp->zComment;
        pMem->n = sqlite3Strlen30(pMem->z);
        pMem->enc = SQLITE_UTF8;
        pMem->type = SQLITE_TEXT;
      }else
#endif
      {
        pMem->flags = MEM_Null;                       /* Comment */
        pMem->type = SQLITE_NULL;
      }
    }

    p->nResColumn = 8 - 4*(p->explain-1);
    p->rc = SQLITE_OK;
    rc = SQLITE_ROW;
  }
  return rc;
}
#endif /* SQLITE_OMIT_EXPLAIN */

#ifdef SQLITE_DEBUG
/*
** Print the SQL that was used to generate a VDBE program.
*/
SQLITE_PRIVATE void sqlite3VdbePrintSql(Vdbe *p){
  int nOp = p->nOp;
  VdbeOp *pOp;
  if( nOp<1 ) return;
  pOp = &p->aOp[0];
  if( pOp->opcode==OP_Trace && pOp->p4.z!=0 ){
    const char *z = pOp->p4.z;
    while( sqlite3Isspace(*z) ) z++;
    printf("SQL: [%s]\n", z);
  }
}
#endif

#if !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** Print an IOTRACE message showing SQL content.
*/
SQLITE_PRIVATE void sqlite3VdbeIOTraceSql(Vdbe *p){
  int nOp = p->nOp;
  VdbeOp *pOp;
  if( sqlite3IoTrace==0 ) return;
  if( nOp<1 ) return;
  pOp = &p->aOp[0];
  if( pOp->opcode==OP_Trace && pOp->p4.z!=0 ){
    int i, j;
    char z[1000];
    sqlite3_snprintf(sizeof(z), z, "%s", pOp->p4.z);
    for(i=0; sqlite3Isspace(z[i]); i++){}
    for(j=0; z[i]; i++){
      if( sqlite3Isspace(z[i]) ){
        if( z[i-1]!=' ' ){
          z[j++] = ' ';
        }
      }else{
        z[j++] = z[i];
      }
    }
    z[j] = 0;
    sqlite3IoTrace("SQL %s\n", z);
  }
}
#endif /* !SQLITE_OMIT_TRACE && SQLITE_ENABLE_IOTRACE */

/*
** Allocate space from a fixed size buffer and return a pointer to
** that space.  If insufficient space is available, return NULL.
**
** The pBuf parameter is the initial value of a pointer which will
** receive the new memory.  pBuf is normally NULL.  If pBuf is not
** NULL, it means that memory space has already been allocated and that
** this routine should not allocate any new memory.  When pBuf is not
** NULL simply return pBuf.  Only allocate new memory space when pBuf
** is NULL.
**
** nByte is the number of bytes of space needed.
**
** *ppFrom points to available space and pEnd points to the end of the
** available space.  When space is allocated, *ppFrom is advanced past
** the end of the allocated space.
**
** *pnByte is a counter of the number of bytes of space that have failed
** to allocate.  If there is insufficient space in *ppFrom to satisfy the
** request, then increment *pnByte by the amount of the request.
*/
static void *allocSpace(
  void *pBuf,          /* Where return pointer will be stored */
  int nByte,           /* Number of bytes to allocate */
  u8 **ppFrom,         /* IN/OUT: Allocate from *ppFrom */
  u8 *pEnd,            /* Pointer to 1 byte past the end of *ppFrom buffer */
  int *pnByte          /* If allocation cannot be made, increment *pnByte */
){
  assert( EIGHT_BYTE_ALIGNMENT(*ppFrom) );
  if( pBuf ) return pBuf;
  nByte = ROUND8(nByte);
  if( &(*ppFrom)[nByte] <= pEnd ){
    pBuf = (void*)*ppFrom;
    *ppFrom += nByte;
  }else{
    *pnByte += nByte;
  }
  return pBuf;
}

/*
** Rewind the VDBE back to the beginning in preparation for
** running it.
*/
SQLITE_PRIVATE void sqlite3VdbeRewind(Vdbe *p){
#if defined(SQLITE_DEBUG) || defined(VDBE_PROFILE)
  int i;
#endif
  assert( p!=0 );
  assert( p->magic==VDBE_MAGIC_INIT );

  /* There should be at least one opcode.
  */
  assert( p->nOp>0 );

  /* Set the magic to VDBE_MAGIC_RUN sooner rather than later. */
  p->magic = VDBE_MAGIC_RUN;

#ifdef SQLITE_DEBUG
  for(i=1; i<p->nMem; i++){
    assert( p->aMem[i].db==p->db );
  }
#endif
  p->pc = -1;
  p->rc = SQLITE_OK;
  p->errorAction = OE_Abort;
  p->magic = VDBE_MAGIC_RUN;
  p->nChange = 0;
  p->cacheCtr = 1;
  p->minWriteFileFormat = 255;
  p->iStatement = 0;
  p->nFkConstraint = 0;
#ifdef VDBE_PROFILE
  for(i=0; i<p->nOp; i++){
    p->aOp[i].cnt = 0;
    p->aOp[i].cycles = 0;
  }
#endif
}

/*
** Prepare a virtual machine for execution for the first time after
** creating the virtual machine.  This involves things such
** as allocating stack space and initializing the program counter.
** After the VDBE has be prepped, it can be executed by one or more
** calls to sqlite3VdbeExec().  
**
** This function may be called exact once on a each virtual machine.
** After this routine is called the VM has been "packaged" and is ready
** to run.  After this routine is called, futher calls to 
** sqlite3VdbeAddOp() functions are prohibited.  This routine disconnects
** the Vdbe from the Parse object that helped generate it so that the
** the Vdbe becomes an independent entity and the Parse object can be
** destroyed.
**
** Use the sqlite3VdbeRewind() procedure to restore a virtual machine back
** to its initial state after it has been run.
*/
SQLITE_PRIVATE void sqlite3VdbeMakeReady(
  Vdbe *p,                       /* The VDBE */
  Parse *pParse                  /* Parsing context */
){
  sqlite3 *db;                   /* The database connection */
  int nVar;                      /* Number of parameters */
  int nMem;                      /* Number of VM memory registers */
  int nCursor;                   /* Number of cursors required */
  int nArg;                      /* Number of arguments in subprograms */
  int n;                         /* Loop counter */
  u8 *zCsr;                      /* Memory available for allocation */
  u8 *zEnd;                      /* First byte past allocated memory */
  int nByte;                     /* How much extra memory is needed */

  assert( p!=0 );
  assert( p->nOp>0 );
  assert( pParse!=0 );
  assert( p->magic==VDBE_MAGIC_INIT );
  db = p->db;
  assert( db->mallocFailed==0 );
  nVar = pParse->nVar;
  nMem = pParse->nMem;
  nCursor = pParse->nTab;
  nArg = pParse->nMaxArg;
  
  /* For each cursor required, also allocate a memory cell. Memory
  ** cells (nMem+1-nCursor)..nMem, inclusive, will never be used by
  ** the vdbe program. Instead they are used to allocate space for
  ** VdbeCursor/BtCursor structures. The blob of memory associated with 
  ** cursor 0 is stored in memory cell nMem. Memory cell (nMem-1)
  ** stores the blob of memory associated with cursor 1, etc.
  **
  ** See also: allocateCursor().
  */
  nMem += nCursor;

  /* Allocate space for memory registers, SQL variables, VDBE cursors and 
  ** an array to marshal SQL function arguments in.
  */
  zCsr = (u8*)&p->aOp[p->nOp];       /* Memory avaliable for allocation */
  zEnd = (u8*)&p->aOp[p->nOpAlloc];  /* First byte past end of zCsr[] */

  resolveP2Values(p, &nArg);
  p->usesStmtJournal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  if( pParse->explain && nMem<10 ){
    nMem = 10;
  }
  memset(zCsr, 0, zEnd-zCsr);
  zCsr += (zCsr - (u8*)0)&7;
  assert( EIGHT_BYTE_ALIGNMENT(zCsr) );
  p->expired = 0;

  /* Memory for registers, parameters, cursor, etc, is allocated in two
  ** passes.  On the first pass, we try to reuse unused space at the 
  ** end of the opcode array.  If we are unable to satisfy all memory
  ** requirements by reusing the opcode array tail, then the second
  ** pass will fill in the rest using a fresh allocation.  
  **
  ** This two-pass approach that reuses as much memory as possible from
  ** the leftover space at the end of the opcode array can significantly
  ** reduce the amount of memory held by a prepared statement.
  */
  do {
    nByte = 0;
    p->aMem = allocSpace(p->aMem, nMem*sizeof(Mem), &zCsr, zEnd, &nByte);
    p->aVar = allocSpace(p->aVar, nVar*sizeof(Mem), &zCsr, zEnd, &nByte);
    p->apArg = allocSpace(p->apArg, nArg*sizeof(Mem*), &zCsr, zEnd, &nByte);
    p->azVar = allocSpace(p->azVar, nVar*sizeof(char*), &zCsr, zEnd, &nByte);
    p->apCsr = allocSpace(p->apCsr, nCursor*sizeof(VdbeCursor*),
                          &zCsr, zEnd, &nByte);
    if( nByte ){
      p->pFree = sqlite3DbMallocZero(db, nByte);
    }
    zCsr = p->pFree;
    zEnd = &zCsr[nByte];
  }while( nByte && !db->mallocFailed );

  p->nCursor = (u16)nCursor;
  if( p->aVar ){
    p->nVar = (ynVar)nVar;
    for(n=0; n<nVar; n++){
      p->aVar[n].flags = MEM_Null;
      p->aVar[n].db = db;
    }
  }
  if( p->azVar ){
    p->nzVar = pParse->nzVar;
    memcpy(p->azVar, pParse->azVar, p->nzVar*sizeof(p->azVar[0]));
    memset(pParse->azVar, 0, pParse->nzVar*sizeof(pParse->azVar[0]));
  }
  if( p->aMem ){
    p->aMem--;                      /* aMem[] goes from 1..nMem */
    p->nMem = nMem;                 /*       not from 0..nMem-1 */
    for(n=1; n<=nMem; n++){
      p->aMem[n].flags = MEM_Null;
      p->aMem[n].db = db;
    }
  }
  p->explain = pParse->explain;
  sqlite3VdbeRewind(p);
}

/*
** Close a VDBE cursor and release all the resources that cursor 
** happens to hold.
*/
SQLITE_PRIVATE void sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx){
  if( pCx==0 ){
    return;
  }
  if( pCx->pBt ){
    sqlite3BtreeClose(pCx->pBt);
    /* The pCx->pCursor will be close automatically, if it exists, by
    ** the call above. */
  }else if( pCx->pCursor ){
    sqlite3BtreeCloseCursor(pCx->pCursor);
  }
#ifndef SQLITE_OMIT_VIRTUALTABLE
  if( pCx->pVtabCursor ){
    sqlite3_vtab_cursor *pVtabCursor = pCx->pVtabCursor;
    const sqlite3_module *pModule = pCx->pModule;
    p->inVtabMethod = 1;
    pModule->xClose(pVtabCursor);
    p->inVtabMethod = 0;
  }
#endif
}

/*
** Copy the values stored in the VdbeFrame structure to its Vdbe. This
** is used, for example, when a trigger sub-program is halted to restore
** control to the main program.
*/
SQLITE_PRIVATE int sqlite3VdbeFrameRestore(VdbeFrame *pFrame){
  Vdbe *v = pFrame->v;
  v->aOp = pFrame->aOp;
  v->nOp = pFrame->nOp;
  v->aMem = pFrame->aMem;
  v->nMem = pFrame->nMem;
  v->apCsr = pFrame->apCsr;
  v->nCursor = pFrame->nCursor;
  v->db->lastRowid = pFrame->lastRowid;
  v->nChange = pFrame->nChange;
  return pFrame->pc;
}

/*
** Close all cursors.
**
** Also release any dynamic memory held by the VM in the Vdbe.aMem memory 
** cell array. This is necessary as the memory cell array may contain
** pointers to VdbeFrame objects, which may in turn contain pointers to
** open cursors.
*/
static void closeAllCursors(Vdbe *p){
  if( p->pFrame ){
    VdbeFrame *pFrame;
    for(pFrame=p->pFrame; pFrame->pParent; pFrame=pFrame->pParent);
    sqlite3VdbeFrameRestore(pFrame);
  }
  p->pFrame = 0;
  p->nFrame = 0;

  if( p->apCsr ){
    int i;
    for(i=0; i<p->nCursor; i++){
      VdbeCursor *pC = p->apCsr[i];
      if( pC ){
        sqlite3VdbeFreeCursor(p, pC);
        p->apCsr[i] = 0;
      }
    }
  }
  if( p->aMem ){
    releaseMemArray(&p->aMem[1], p->nMem);
  }
  while( p->pDelFrame ){
    VdbeFrame *pDel = p->pDelFrame;
    p->pDelFrame = pDel->pParent;
    sqlite3VdbeFrameDelete(pDel);
  }
}

/*
** Clean up the VM after execution.
**
** This routine will automatically close any cursors, lists, and/or
** sorters that were left open.  It also deletes the values of
** variables in the aVar[] array.
*/
static void Cleanup(Vdbe *p){
  sqlite3 *db = p->db;

#ifdef SQLITE_DEBUG
  /* Execute assert() statements to ensure that the Vdbe.apCsr[] and 
  ** Vdbe.aMem[] arrays have already been cleaned up.  */
  int i;
  for(i=0; i<p->nCursor; i++) assert( p->apCsr==0 || p->apCsr[i]==0 );
  for(i=1; i<=p->nMem; i++) assert( p->aMem==0 || p->aMem[i].flags==MEM_Null );
#endif

  sqlite3DbFree(db, p->zErrMsg);
  p->zErrMsg = 0;
  p->pResultSet = 0;
}

/*
** Set the number of result columns that will be returned by this SQL
** statement. This is now set at compile time, rather than during
** execution of the vdbe program so that sqlite3_column_count() can
** be called on an SQL statement before sqlite3_step().
*/
SQLITE_PRIVATE void sqlite3VdbeSetNumCols(Vdbe *p, int nResColumn){
  Mem *pColName;
  int n;
  sqlite3 *db = p->db;

  releaseMemArray(p->aColName, p->nResColumn*COLNAME_N);
  sqlite3DbFree(db, p->aColName);
  n = nResColumn*COLNAME_N;
  p->nResColumn = (u16)nResColumn;
  p->aColName = pColName = (Mem*)sqlite3DbMallocZero(db, sizeof(Mem)*n );
  if( p->aColName==0 ) return;
  while( n-- > 0 ){
    pColName->flags = MEM_Null;
    pColName->db = p->db;
    pColName++;
  }
}

/*
** Set the name of the idx'th column to be returned by the SQL statement.
** zName must be a pointer to a nul terminated string.
**
** This call must be made after a call to sqlite3VdbeSetNumCols().
**
** The final parameter, xDel, must be one of SQLITE_DYNAMIC, SQLITE_STATIC
** or SQLITE_TRANSIENT. If it is SQLITE_DYNAMIC, then the buffer pointed
** to by zName will be freed by sqlite3DbFree() when the vdbe is destroyed.
*/
SQLITE_PRIVATE int sqlite3VdbeSetColName(
  Vdbe *p,                         /* Vdbe being configured */
  int idx,                         /* Index of column zName applies to */
  int var,                         /* One of the COLNAME_* constants */
  const char *zName,               /* Pointer to buffer containing name */
  void (*xDel)(void*)              /* Memory management strategy for zName */
){
  int rc;
  Mem *pColName;
  assert( idx<p->nResColumn );
  assert( var<COLNAME_N );
  if( p->db->mallocFailed ){
    assert( !zName || xDel!=SQLITE_DYNAMIC );
    return SQLITE_NOMEM;
  }
  assert( p->aColName!=0 );
  pColName = &(p->aColName[idx+var*p->nResColumn]);
  rc = sqlite3VdbeMemSetStr(pColName, zName, -1, SQLITE_UTF8, xDel);
  assert( rc!=0 || !zName || (pColName->flags&MEM_Term)!=0 );
  return rc;
}

/*
** A read or write transaction may or may not be active on database handle
** db. If a transaction is active, commit it. If there is a
** write-transaction spanning more than one database file, this routine
** takes care of the master journal trickery.
*/
static int vdbeCommit(sqlite3 *db, Vdbe *p){
  int i;
  int nTrans = 0;  /* Number of databases with an active write-transaction */
  int rc = SQLITE_OK;
  int needXcommit = 0;

#ifdef SQLITE_OMIT_VIRTUALTABLE
  /* With this option, sqlite3VtabSync() is defined to be simply 
  ** SQLITE_OK so p is not used. 
  */
  UNUSED_PARAMETER(p);
#endif

  /* Before doing anything else, call the xSync() callback for any
  ** virtual module tables written in this transaction. This has to
  ** be done before determining whether a master journal file is 
  ** required, as an xSync() callback may add an attached database
  ** to the transaction.
  */
  rc = sqlite3VtabSync(db, &p->zErrMsg);

  /* This loop determines (a) if the commit hook should be invoked and
  ** (b) how many database files have open write transactions, not 
  ** including the temp database. (b) is important because if more than 
  ** one database file has an open write transaction, a master journal
  ** file is required for an atomic commit.
  */ 
  for(i=0; rc==SQLITE_OK && i<db->nDb; i++){ 
    Btree *pBt = db->aDb[i].pBt;
    if( sqlite3BtreeIsInTrans(pBt) ){
      needXcommit = 1;
      if( i!=1 ) nTrans++;
      rc = sqlite3PagerExclusiveLock(sqlite3BtreePager(pBt));
    }
  }
  if( rc!=SQLITE_OK ){
    return rc;
  }

  /* If there are any write-transactions at all, invoke the commit hook */
  if( needXcommit && db->xCommitCallback ){
    rc = db->xCommitCallback(db->pCommitArg);
    if( rc ){
      return SQLITE_CONSTRAINT;
    }
  }

  /* The simple case - no more than one database file (not counting the
  ** TEMP database) has a transaction active.   There is no need for the
  ** master-journal.
  **
  ** If the return value of sqlite3BtreeGetFilename() is a zero length
  ** string, it means the main database is :memory: or a temp file.  In 
  ** that case we do not support atomic multi-file commits, so use the 
  ** simple case then too.
  */
  if( 0==sqlite3Strlen30(sqlite3BtreeGetFilename(db->aDb[0].pBt))
   || nTrans<=1
  ){
    for(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      if( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, 0);
      }
    }

    /* Do the commit only if all databases successfully complete phase 1. 
    ** If one of the BtreeCommitPhaseOne() calls fails, this indicates an
    ** IO error while deleting or truncating a journal file. It is unlikely,
    ** but could happen. In this case abandon processing and return the error.
    */
    for(i=0; rc==SQLITE_OK && i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      if( pBt ){
        rc = sqlite3BtreeCommitPhaseTwo(pBt, 0);
      }
    }
    if( rc==SQLITE_OK ){
      sqlite3VtabCommit(db);
    }
  }

  /* The complex case - There is a multi-file write-transaction active.
  ** This requires a master journal file to ensure the transaction is
  ** committed atomicly.
  */
#ifndef SQLITE_OMIT_DISKIO
  else{
    sqlite3_vfs *pVfs = db->pVfs;
    int needSync = 0;
    char *zMaster = 0;   /* File-name for the master journal */
    char const *zMainFile = sqlite3BtreeGetFilename(db->aDb[0].pBt);
    sqlite3_file *pMaster = 0;
    i64 offset = 0;
    int res;

    /* Select a master journal file name */
    do {
      u32 iRandom;
      sqlite3DbFree(db, zMaster);
      sqlite3_randomness(sizeof(iRandom), &iRandom);
      zMaster = sqlite3MPrintf(db, "%s-mj%08X", zMainFile, iRandom&0x7fffffff);
      if( !zMaster ){
        return SQLITE_NOMEM;
      }
      sqlite3FileSuffix3(zMainFile, zMaster);
      rc = sqlite3OsAccess(pVfs, zMaster, SQLITE_ACCESS_EXISTS, &res);
    }while( rc==SQLITE_OK && res );
    if( rc==SQLITE_OK ){
      /* Open the master journal. */
      rc = sqlite3OsOpenMalloc(pVfs, zMaster, &pMaster, 
          SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|
          SQLITE_OPEN_EXCLUSIVE|SQLITE_OPEN_MASTER_JOURNAL, 0
      );
    }
    if( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zMaster);
      return rc;
    }
 
    /* Write the name of each database file in the transaction into the new
    ** master journal file. If an error occurs at this point close
    ** and delete the master journal file. All the individual journal files
    ** still have 'null' as the master journal pointer, so they will roll
    ** back independently if a failure occurs.
    */
    for(i=0; i<db->nDb; i++){
      Btree *pBt = db->aDb[i].pBt;
      if( sqlite3BtreeIsInTrans(pBt) ){
        char const *zFile = sqlite3BtreeGetJournalname(pBt);
        if( zFile==0 ){
          continue;  /* Ignore TEMP and :memory: databases */
        }
        assert( zFile[0]!=0 );
        if( !needSync && !sqlite3BtreeSyncDisabled(pBt) ){
          needSync = 1;
        }
        rc = sqlite3OsWrite(pMaster, zFile, sqlite3Strlen30(zFile)+1, offset);
        offset += sqlite3Strlen30(zFile)+1;
        if( rc!=SQLITE_OK ){
          sqlite3OsCloseFree(pMaster);
          sqlite3OsDelete(pVfs, zMaster, 0);
          sqlite3DbFree(db, zMaster);
          return rc;
        }
      }
    }

    /* Sync the master journal file. If the IOCAP_SEQUENTIAL device
    ** flag is set this is not required.
    */
    if( needSync 
     && 0==(sqlite3OsDeviceCharacteristics(pMaster)&SQLITE_IOCAP_SEQUENTIAL)
     && SQLITE_OK!=(rc = sqlite3OsSync(pMaster, SQLITE_SYNC_NORMAL))
    ){
      sqlite3OsCloseFree(pMaster);
      sqlite3OsDelete(pVfs, zMaster, 0);
      sqlite3DbFree(db, zMaster);
      return rc;
    }

    /* Sync all the db files involved in the transaction. The same call
    ** sets the master journal pointer in each individual journal. If
    ** an error occurs here, do not delete the master journal file.
    **
    ** If the error occurs during the first call to
    ** sqlite3BtreeCommitPhaseOne(), then there is a chance that the
    ** master journal file will be orphaned. But we cannot delete it,
    ** in case the master journal file name was written into the journal
    ** file before the failure occurred.
    */
    for(i=0; rc==SQLITE_OK && i<db->nDb; i++){ 
      Btree *pBt = db->aDb[i].pBt;
      if( pBt ){
        rc = sqlite3BtreeCommitPhaseOne(pBt, zMaster);
      }
    }
    sqlite3OsCloseFree(pMaster);
    assert( rc!=SQLITE_BUSY );
    if( rc!=SQLITE_OK ){
      sqlite3DbFree(db, zMaster);
      return rc;
    }

    /* Delete the master journal file. This commits the transaction. After
    ** doing this the directory is synced again before any individual
    ** transaction files are deleted.
    */
    rc = sqlite3OsDelete(pVfs, zMaster, 1);
    sqlite3DbFree(db, zMaster);
    zMaster = 0;
    if( rc ){
      return rc;
    }

    /* All files and directories have already been synced, so the following
    ** calls to sqlite3BtreeCommitPhaseTwo() are only closing files and
    ** deleting or truncating journals. If something goes wrong while
    ** this is happening we don't really care. The integrity of the
    ** transaction is already guaranteed, but some stray 'cold' journals
    ** may be lying around. Returning an error code won't help matters.
    */
    disable_simulated_io_errors();
    sqlite3BeginBenignMalloc();
    for(i=0; i<db->nDb; i++){ 
      Btree *pBt = db->aDb[i].pBt;
      if( pBt ){
        sqlite3BtreeCommitPhaseTwo(pBt, 1);
      }
    }
    sqlite3EndBenignMalloc();
    enable_simulated_io_errors();

    sqlite3VtabCommit(db);
  }
#endif

  return rc;
}

/* 
** This routine checks that the sqlite3.activeVdbeCnt count variable
** matches the number of vdbe's in the list sqlite3.pVdbe that are
** currently active. An assertion fails if the two counts do not match.
** This is an internal self-check only - it is not an essential processing
** step.
**
** This is a no-op if NDEBUG is defined.
*/
#ifndef NDEBUG
static void checkActiveVdbeCnt(sqlite3 *db){
  Vdbe *p;
  int cnt = 0;
  int nWrite = 0;
  p = db->pVdbe;
  while( p ){
    if( p->magic==VDBE_MAGIC_RUN && p->pc>=0 ){
      cnt++;
      if( p->readOnly==0 ) nWrite++;
    }
    p = p->pNext;
  }
  assert( cnt==db->activeVdbeCnt );
  assert( nWrite==db->writeVdbeCnt );
}
#else
#define checkActiveVdbeCnt(x)
#endif

/*
** For every Btree that in database connection db which 
** has been modified, "trip" or invalidate each cursor in
** that Btree might have been modified so that the cursor
** can never be used again.  This happens when a rollback
*** occurs.  We have to trip all the other cursors, even
** cursor from other VMs in different database connections,
** so that none of them try to use the data at which they
** were pointing and which now may have been changed due
** to the rollback.
**
** Remember that a rollback can delete tables complete and
** reorder rootpages.  So it is not sufficient just to save
** the state of the cursor.  We have to invalidate the cursor
** so that it is never used again.
*/
static void invalidateCursorsOnModifiedBtrees(sqlite3 *db){
  int i;
  for(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    if( p && sqlite3BtreeIsInTrans(p) ){
      sqlite3BtreeTripAllCursors(p, SQLITE_ABORT);
    }
  }
}

/*
** If the Vdbe passed as the first argument opened a statement-transaction,
** close it now. Argument eOp must be either SAVEPOINT_ROLLBACK or
** SAVEPOINT_RELEASE. If it is SAVEPOINT_ROLLBACK, then the statement
** transaction is rolled back. If eOp is SAVEPOINT_RELEASE, then the 
** statement transaction is commtted.
**
** If an IO error occurs, an SQLITE_IOERR_XXX error code is returned. 
** Otherwise SQLITE_OK.
*/
SQLITE_PRIVATE int sqlite3VdbeCloseStatement(Vdbe *p, int eOp){
  sqlite3 *const db = p->db;
  int rc = SQLITE_OK;

  /* If p->iStatement is greater than zero, then this Vdbe opened a 
  ** statement transaction that should be closed here. The only exception
  ** is that an IO error may have occured, causing an emergency rollback.
  ** In this case (db->nStatement==0), and there is nothing to do.
  */
  if( db->nStatement && p->iStatement ){
    int i;
    const int iSavepoint = p->iStatement-1;

    assert( eOp==SAVEPOINT_ROLLBACK || eOp==SAVEPOINT_RELEASE);
    assert( db->nStatement>0 );
    assert( p->iStatement==(db->nStatement+db->nSavepoint) );

    for(i=0; i<db->nDb; i++){ 
      int rc2 = SQLITE_OK;
      Btree *pBt = db->aDb[i].pBt;
      if( pBt ){
        if( eOp==SAVEPOINT_ROLLBACK ){
          rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_ROLLBACK, iSavepoint);
        }
        if( rc2==SQLITE_OK ){
          rc2 = sqlite3BtreeSavepoint(pBt, SAVEPOINT_RELEASE, iSavepoint);
        }
        if( rc==SQLITE_OK ){
          rc = rc2;
        }
      }
    }
    db->nStatement--;
    p->iStatement = 0;

    if( rc==SQLITE_OK ){
      if( eOp==SAVEPOINT_ROLLBACK ){
        rc = sqlite3VtabSavepoint(db, SAVEPOINT_ROLLBACK, iSavepoint);
      }
      if( rc==SQLITE_OK ){
        rc = sqlite3VtabSavepoint(db, SAVEPOINT_RELEASE, iSavepoint);
      }
    }

    /* If the statement transaction is being rolled back, also restore the 
    ** database handles deferred constraint counter to the value it had when 
    ** the statement transaction was opened.  */
    if( eOp==SAVEPOINT_ROLLBACK ){
      db->nDeferredCons = p->nStmtDefCons;
    }
  }
  return rc;
}

/*
** This function is called when a transaction opened by the database 
** handle associated with the VM passed as an argument is about to be 
** committed. If there are outstanding deferred foreign key constraint
** violations, return SQLITE_ERROR. Otherwise, SQLITE_OK.
**
** If there are outstanding FK violations and this function returns 
** SQLITE_ERROR, set the result of the VM to SQLITE_CONSTRAINT and write
** an error message to it. Then return SQLITE_ERROR.
*/
#ifndef SQLITE_OMIT_FOREIGN_KEY
SQLITE_PRIVATE int sqlite3VdbeCheckFk(Vdbe *p, int deferred){
  sqlite3 *db = p->db;
  if( (deferred && db->nDeferredCons>0) || (!deferred && p->nFkConstraint>0) ){
    p->rc = SQLITE_CONSTRAINT;
    p->errorAction = OE_Abort;
    sqlite3SetString(&p->zErrMsg, db, "foreign key constraint failed");
    return SQLITE_ERROR;
  }
  return SQLITE_OK;
}
#endif

/*
** This routine is called the when a VDBE tries to halt.  If the VDBE
** has made changes and is in autocommit mode, then commit those
** changes.  If a rollback is needed, then do the rollback.
**
** This routine is the only way to move the state of a VM from
** SQLITE_MAGIC_RUN to SQLITE_MAGIC_HALT.  It is harmless to
** call this on a VM that is in the SQLITE_MAGIC_HALT state.
**
** Return an error code.  If the commit could not complete because of
** lock contention, return SQLITE_BUSY.  If SQLITE_BUSY is returned, it
** means the close did not happen and needs to be repeated.
*/
SQLITE_PRIVATE int sqlite3VdbeHalt(Vdbe *p){
  int rc;                         /* Used to store transient return codes */
  sqlite3 *db = p->db;

  /* This function contains the logic that determines if a statement or
  ** transaction will be committed or rolled back as a result of the
  ** execution of this virtual machine. 
  **
  ** If any of the following errors occur:
  **
  **     SQLITE_NOMEM
  **     SQLITE_IOERR
  **     SQLITE_FULL
  **     SQLITE_INTERRUPT
  **
  ** Then the internal cache might have been left in an inconsistent
  ** state.  We need to rollback the statement transaction, if there is
  ** one, or the complete transaction if there is no statement transaction.
  */

  if( p->db->mallocFailed ){
    p->rc = SQLITE_NOMEM;
  }
  closeAllCursors(p);
  if( p->magic!=VDBE_MAGIC_RUN ){
    return SQLITE_OK;
  }
  checkActiveVdbeCnt(db);

  /* No commit or rollback needed if the program never started */
  if( p->pc>=0 ){
    int mrc;   /* Primary error code from p->rc */
    int eStatementOp = 0;
    int isSpecialError;            /* Set to true if a 'special' error */

    /* Lock all btrees used by the statement */
    sqlite3VdbeEnter(p);

    /* Check for one of the special errors */
    mrc = p->rc & 0xff;
    assert( p->rc!=SQLITE_IOERR_BLOCKED );  /* This error no longer exists */
    isSpecialError = mrc==SQLITE_NOMEM || mrc==SQLITE_IOERR
                     || mrc==SQLITE_INTERRUPT || mrc==SQLITE_FULL;
    if( isSpecialError ){
      /* If the query was read-only and the error code is SQLITE_INTERRUPT, 
      ** no rollback is necessary. Otherwise, at least a savepoint 
      ** transaction must be rolled back to restore the database to a 
      ** consistent state.
      **
      ** Even if the statement is read-only, it is important to perform
      ** a statement or transaction rollback operation. If the error 
      ** occured while writing to the journal, sub-journal or database
      ** file as part of an effort to free up cache space (see function
      ** pagerStress() in pager.c), the rollback is required to restore 
      ** the pager to a consistent state.
      */
      if( !p->readOnly || mrc!=SQLITE_INTERRUPT ){
        if( (mrc==SQLITE_NOMEM || mrc==SQLITE_FULL) && p->usesStmtJournal ){
          eStatementOp = SAVEPOINT_ROLLBACK;
        }else{
          /* We are forced to roll back the active transaction. Before doing
          ** so, abort any other statements this handle currently has active.
          */
          invalidateCursorsOnModifiedBtrees(db);
          sqlite3RollbackAll(db);
          sqlite3CloseSavepoints(db);
          db->autoCommit = 1;
        }
      }
    }

    /* Check for immediate foreign key violations. */
    if( p->rc==SQLITE_OK ){
      sqlite3VdbeCheckFk(p, 0);
    }
  
    /* If the auto-commit flag is set and this is the only active writer 
    ** VM, then we do either a commit or rollback of the current transaction. 
    **
    ** Note: This block also runs if one of the special errors handled 
    ** above has occurred. 
    */
    if( !sqlite3VtabInSync(db) 
     && db->autoCommit 
     && db->writeVdbeCnt==(p->readOnly==0) 
    ){
      if( p->rc==SQLITE_OK || (p->errorAction==OE_Fail && !isSpecialError) ){
        rc = sqlite3VdbeCheckFk(p, 1);
        if( rc!=SQLITE_OK ){
          if( NEVER(p->readOnly) ){
            sqlite3VdbeLeave(p);
            return SQLITE_ERROR;
          }
          rc = SQLITE_CONSTRAINT;
        }else{ 
          /* The auto-commit flag is true, the vdbe program was successful 
          ** or hit an 'OR FAIL' constraint and there are no deferred foreign
          ** key constraints to hold up the transaction. This means a commit 
          ** is required. */
          rc = vdbeCommit(db, p);
        }
        if( rc==SQLITE_BUSY && p->readOnly ){
          sqlite3VdbeLeave(p);
          return SQLITE_BUSY;
        }else if( rc!=SQLITE_OK ){
          p->rc = rc;
          sqlite3RollbackAll(db);
        }else{
          db->nDeferredCons = 0;
          sqlite3CommitInternalChanges(db);
        }
      }else{
        sqlite3RollbackAll(db);
      }
      db->nStatement = 0;
    }else if( eStatementOp==0 ){
      if( p->rc==SQLITE_OK || p->errorAction==OE_Fail ){
        eStatementOp = SAVEPOINT_RELEASE;
      }else if( p->errorAction==OE_Abort ){
        eStatementOp = SAVEPOINT_ROLLBACK;
      }else{
        invalidateCursorsOnModifiedBtrees(db);
        sqlite3RollbackAll(db);
        sqlite3CloseSavepoints(db);
        db->autoCommit = 1;
      }
    }
  
    /* If eStatementOp is non-zero, then a statement transaction needs to
    ** be committed or rolled back. Call sqlite3VdbeCloseStatement() to
    ** do so. If this operation returns an error, and the current statement
    ** error code is SQLITE_OK or SQLITE_CONSTRAINT, then promote the
    ** current statement error code.
    */
    if( eStatementOp ){
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      if( rc ){
        if( p->rc==SQLITE_OK || p->rc==SQLITE_CONSTRAINT ){
          p->rc = rc;
          sqlite3DbFree(db, p->zErrMsg);
          p->zErrMsg = 0;
        }
        invalidateCursorsOnModifiedBtrees(db);
        sqlite3RollbackAll(db);
        sqlite3CloseSavepoints(db);
        db->autoCommit = 1;
      }
    }
  
    /* If this was an INSERT, UPDATE or DELETE and no statement transaction
    ** has been rolled back, update the database connection change-counter. 
    */
    if( p->changeCntOn ){
      if( eStatementOp!=SAVEPOINT_ROLLBACK ){
        sqlite3VdbeSetChanges(db, p->nChange);
      }else{
        sqlite3VdbeSetChanges(db, 0);
      }
      p->nChange = 0;
    }
  
    /* Rollback or commit any schema changes that occurred. */
    if( p->rc!=SQLITE_OK && db->flags&SQLITE_InternChanges ){
      sqlite3ResetInternalSchema(db, -1);
      db->flags = (db->flags | SQLITE_InternChanges);
    }

    /* Release the locks */
    sqlite3VdbeLeave(p);
  }

  /* We have successfully halted and closed the VM.  Record this fact. */
  if( p->pc>=0 ){
    db->activeVdbeCnt--;
    if( !p->readOnly ){
      db->writeVdbeCnt--;
    }
    assert( db->activeVdbeCnt>=db->writeVdbeCnt );
  }
  p->magic = VDBE_MAGIC_HALT;
  checkActiveVdbeCnt(db);
  if( p->db->mallocFailed ){
    p->rc = SQLITE_NOMEM;
  }

  /* If the auto-commit flag is set to true, then any locks that were held
  ** by connection db have now been released. Call sqlite3ConnectionUnlocked() 
  ** to invoke any required unlock-notify callbacks.
  */
  if( db->autoCommit ){
    sqlite3ConnectionUnlocked(db);
  }

  assert( db->activeVdbeCnt>0 || db->autoCommit==0 || db->nStatement==0 );
  return (p->rc==SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}


/*
** Each VDBE holds the result of the most recent sqlite3_step() call
** in p->rc.  This routine sets that result back to SQLITE_OK.
*/
SQLITE_PRIVATE void sqlite3VdbeResetStepResult(Vdbe *p){
  p->rc = SQLITE_OK;
}

/*
** Clean up a VDBE after execution but do not delete the VDBE just yet.
** Write any error messages into *pzErrMsg.  Return the result code.
**
** After this routine is run, the VDBE should be ready to be executed
** again.
**
** To look at it another way, this routine resets the state of the
** virtual machine from VDBE_MAGIC_RUN or VDBE_MAGIC_HALT back to
** VDBE_MAGIC_INIT.
*/
SQLITE_PRIVATE int sqlite3VdbeReset(Vdbe *p){
  sqlite3 *db;
  db = p->db;

  /* If the VM did not run to completion or if it encountered an
  ** error, then it might not have been halted properly.  So halt
  ** it now.
  */
  sqlite3VdbeHalt(p);

  /* If the VDBE has be run even partially, then transfer the error code
  ** and error message from the VDBE into the main database structure.  But
  ** if the VDBE has just been set to run but has not actually executed any
  ** instructions yet, leave the main database error information unchanged.
  */
  if( p->pc>=0 ){
    if( p->zErrMsg ){
      sqlite3BeginBenignMalloc();
      sqlite3ValueSetStr(db->pErr,-1,p->zErrMsg,SQLITE_UTF8,SQLITE_TRANSIENT);
      sqlite3EndBenignMalloc();
      db->errCode = p->rc;
      sqlite3DbFree(db, p->zErrMsg);
      p->zErrMsg = 0;
    }else if( p->rc ){
      sqlite3Error(db, p->rc, 0);
    }else{
      sqlite3Error(db, SQLITE_OK, 0);
    }
    if( p->runOnlyOnce ) p->expired = 1;
  }else if( p->rc && p->expired ){
    /* The expired flag was set on the VDBE before the first call
    ** to sqlite3_step(). For consistency (since sqlite3_step() was
    ** called), set the database error in this case as well.
    */
    sqlite3Error(db, p->rc, 0);
    sqlite3ValueSetStr(db->pErr, -1, p->zErrMsg, SQLITE_UTF8, SQLITE_TRANSIENT);
    sqlite3DbFree(db, p->zErrMsg);
    p->zErrMsg = 0;
  }

  /* Reclaim all memory used by the VDBE
  */
  Cleanup(p);

  /* Save profiling information from this VDBE run.
  */
#ifdef VDBE_PROFILE
  {
    FILE *out = fopen("vdbe_profile.out", "a");
    if( out ){
      int i;
      fprintf(out, "---- ");
      for(i=0; i<p->nOp; i++){
        fprintf(out, "%02x", p->aOp[i].opcode);
      }
      fprintf(out, "\n");
      for(i=0; i<p->nOp; i++){
        fprintf(out, "%6d %10lld %8lld ",
           p->aOp[i].cnt,
           p->aOp[i].cycles,
           p->aOp[i].cnt>0 ? p->aOp[i].cycles/p->aOp[i].cnt : 0
        );
        sqlite3VdbePrintOp(out, i, &p->aOp[i]);
      }
      fclose(out);
    }
  }
#endif
  p->magic = VDBE_MAGIC_INIT;
  return p->rc & db->errMask;
}
 
/*
** Clean up and delete a VDBE after execution.  Return an integer which is
** the result code.  Write any error message text into *pzErrMsg.
*/
SQLITE_PRIVATE int sqlite3VdbeFinalize(Vdbe *p){
  int rc = SQLITE_OK;
  if( p->magic==VDBE_MAGIC_RUN || p->magic==VDBE_MAGIC_HALT ){
    rc = sqlite3VdbeReset(p);
    assert( (rc & p->db->errMask)==rc );
  }
  sqlite3VdbeDelete(p);
  return rc;
}

/*
** Call the destructor for each auxdata entry in pVdbeFunc for which
** the corresponding bit in mask is clear.  Auxdata entries beyond 31
** are always destroyed.  To destroy all auxdata entries, call this
** routine with mask==0.
*/
SQLITE_PRIVATE void sqlite3VdbeDeleteAuxData(VdbeFunc *pVdbeFunc, int mask){
  int i;
  for(i=0; i<pVdbeFunc->nAux; i++){
    struct AuxData *pAux = &pVdbeFunc->apAux[i];
    if( (i>31 || !(mask&(((u32)1)<<i))) && pAux->pAux ){
      if( pAux->xDelete ){
        pAux->xDelete(pAux->pAux);
      }
      pAux->pAux = 0;
    }
  }
}

/*
** Free all memory associated with the Vdbe passed as the second argument.
** The difference between this function and sqlite3VdbeDelete() is that
** VdbeDelete() also unlinks the Vdbe from the list of VMs associated with
** the database connection.
*/
SQLITE_PRIVATE void sqlite3VdbeDeleteObject(sqlite3 *db, Vdbe *p){
  SubProgram *pSub, *pNext;
  int i;
  assert( p->db==0 || p->db==db );
  releaseMemArray(p->aVar, p->nVar);
  releaseMemArray(p->aColName, p->nResColumn*COLNAME_N);
  for(pSub=p->pProgram; pSub; pSub=pNext){
    pNext = pSub->pNext;
    vdbeFreeOpArray(db, pSub->aOp, pSub->nOp);
    sqlite3DbFree(db, pSub);
  }
  for(i=p->nzVar-1; i>=0; i--) sqlite3DbFree(db, p->azVar[i]);
  vdbeFreeOpArray(db, p->aOp, p->nOp);
  sqlite3DbFree(db, p->aLabel);
  sqlite3DbFree(db, p->aColName);
  sqlite3DbFree(db, p->zSql);
  sqlite3DbFree(db, p->pFree);
  sqlite3DbFree(db, p);
}

/*
** Delete an entire VDBE.
*/
SQLITE_PRIVATE void sqlite3VdbeDelete(Vdbe *p){
  sqlite3 *db;

  if( NEVER(p==0) ) return;
  db = p->db;
  if( p->pPrev ){
    p->pPrev->pNext = p->pNext;
  }else{
    assert( db->pVdbe==p );
    db->pVdbe = p->pNext;
  }
  if( p->pNext ){
    p->pNext->pPrev = p->pPrev;
  }
  p->magic = VDBE_MAGIC_DEAD;
  p->db = 0;
  sqlite3VdbeDeleteObject(db, p);
}

/*
** Make sure the cursor p is ready to read or write the row to which it
** was last positioned.  Return an error code if an OOM fault or I/O error
** prevents us from positioning the cursor to its correct position.
**
** If a MoveTo operation is pending on the given cursor, then do that
** MoveTo now.  If no move is pending, check to see if the row has been
** deleted out from under the cursor and if it has, mark the row as
** a NULL row.
**
** If the cursor is already pointing to the correct row and that row has
** not been deleted out from under the cursor, then this routine is a no-op.
*/
SQLITE_PRIVATE int sqlite3VdbeCursorMoveto(VdbeCursor *p){
  if( p->deferredMoveto ){
    int res, rc;
#ifdef SQLITE_TEST
    extern int sqlite3_search_count;
#endif
    assert( p->isTable );
    rc = sqlite3BtreeMovetoUnpacked(p->pCursor, 0, p->movetoTarget, 0, &res);
    if( rc ) return rc;
    p->lastRowid = p->movetoTarget;
    if( res!=0 ) return SQLITE_CORRUPT_BKPT;
    p->rowidIsValid = 1;
#ifdef SQLITE_TEST
    sqlite3_search_count++;
#endif
    p->deferredMoveto = 0;
    p->cacheStatus = CACHE_STALE;
  }else if( ALWAYS(p->pCursor) ){
    int hasMoved;
    int rc = sqlite3BtreeCursorHasMoved(p->pCursor, &hasMoved);
    if( rc ) return rc;
    if( hasMoved ){
      p->cacheStatus = CACHE_STALE;
      p->nullRow = 1;
    }
  }
  return SQLITE_OK;
}

/*
** The following functions:
**
** sqlite3VdbeSerialType()
** sqlite3VdbeSerialTypeLen()
** sqlite3VdbeSerialLen()
** sqlite3VdbeSerialPut()
** sqlite3VdbeSerialGet()
**
** encapsulate the code that serializes values for storage in SQLite
** data and index records. Each serialized value consists of a
** 'serial-type' and a blob of data. The serial type is an 8-byte unsigned
** integer, stored as a varint.
**
** In an SQLite index record, the serial type is stored directly before
** the blob of data that it corresponds to. In a table record, all serial
** types are stored at the start of the record, and the blobs of data at
** the end. Hence these functions allow the caller to handle the
** serial-type and data blob seperately.
**
** The following table describes the various storage classes for data:
**
**   serial type        bytes of data      type
**   --------------     ---------------    ---------------
**      0                     0            NULL
**      1                     1            signed integer
**      2                     2            signed integer
**      3                     3            signed integer
**      4                     4            signed integer
**      5                     6            signed integer
**      6                     8            signed integer
**      7                     8            IEEE float
**      8                     0            Integer constant 0
**      9                     0            Integer constant 1
**     10,11                               reserved for expansion
**    N>=12 and even       (N-12)/2        BLOB
**    N>=13 and odd        (N-13)/2        text
**
** The 8 and 9 types were added in 3.3.0, file format 4.  Prior versions
** of SQLite will not understand those serial types.
*/

/*
** Return the serial-type for the value stored in pMem.
*/
SQLITE_PRIVATE u32 sqlite3VdbeSerialType(Mem *pMem, int file_format){
  int flags = pMem->flags;
  int n;

  if( flags&MEM_Null ){
    return 0;
  }
  if( flags&MEM_Int ){
    /* Figure out whether to use 1, 2, 4, 6 or 8 bytes. */
#   define MAX_6BYTE ((((i64)0x00008000)<<32)-1)
    i64 i = pMem->u.i;
    u64 u;
    if( file_format>=4 && (i&1)==i ){
      return 8+(u32)i;
    }
    if( i<0 ){
      if( i<(-MAX_6BYTE) ) return 6;
      /* Previous test prevents:  u = -(-9223372036854775808) */
      u = -i;
    }else{
      u = i;
    }
    if( u<=127 ) return 1;
    if( u<=32767 ) return 2;
    if( u<=8388607 ) return 3;
    if( u<=2147483647 ) return 4;
    if( u<=MAX_6BYTE ) return 5;
    return 6;
  }
  if( flags&MEM_Real ){
    return 7;
  }
  assert( pMem->db->mallocFailed || flags&(MEM_Str|MEM_Blob) );
  n = pMem->n;
  if( flags & MEM_Zero ){
    n += pMem->u.nZero;
  }
  assert( n>=0 );
  return ((n*2) + 12 + ((flags&MEM_Str)!=0));
}

/*
** Return the length of the data corresponding to the supplied serial-type.
*/
SQLITE_PRIVATE u32 sqlite3VdbeSerialTypeLen(u32 serial_type){
  if( serial_type>=12 ){
    return (serial_type-12)/2;
  }else{
    static const u8 aSize[] = { 0, 1, 2, 3, 4, 6, 8, 8, 0, 0, 0, 0 };
    return aSize[serial_type];
  }
}

/*
** If we are on an architecture with mixed-endian floating 
** points (ex: ARM7) then swap the lower 4 bytes with the 
** upper 4 bytes.  Return the result.
**
** For most architectures, this is a no-op.
**
** (later):  It is reported to me that the mixed-endian problem
** on ARM7 is an issue with GCC, not with the ARM7 chip.  It seems
** that early versions of GCC stored the two words of a 64-bit
** float in the wrong order.  And that error has been propagated
** ever since.  The blame is not necessarily with GCC, though.
** GCC might have just copying the problem from a prior compiler.
** I am also told that newer versions of GCC that follow a different
** ABI get the byte order right.
**
** Developers using SQLite on an ARM7 should compile and run their
** application using -DSQLITE_DEBUG=1 at least once.  With DEBUG
** enabled, some asserts below will ensure that the byte order of
** floating point values is correct.
**
** (2007-08-30)  Frank van Vugt has studied this problem closely
** and has send his findings to the SQLite developers.  Frank
** writes that some Linux kernels offer floating point hardware
** emulation that uses only 32-bit mantissas instead of a full 
** 48-bits as required by the IEEE standard.  (This is the
** CONFIG_FPE_FASTFPE option.)  On such systems, floating point
** byte swapping becomes very complicated.  To avoid problems,
** the necessary byte swapping is carried out using a 64-bit integer
** rather than a 64-bit float.  Frank assures us that the code here
** works for him.  We, the developers, have no way to independently
** verify this, but Frank seems to know what he is talking about
** so we trust him.
*/
#ifdef SQLITE_MIXED_ENDIAN_64BIT_FLOAT
static u64 floatSwap(u64 in){
  union {
    u64 r;
    u32 i[2];
  } u;
  u32 t;

  u.r = in;
  t = u.i[0];
  u.i[0] = u.i[1];
  u.i[1] = t;
  return u.r;
}
# define swapMixedEndianFloat(X)  X = floatSwap(X)
#else
# define swapMixedEndianFloat(X)
#endif

/*
** Write the serialized data blob for the value stored in pMem into 
** buf. It is assumed that the caller has allocated sufficient space.
** Return the number of bytes written.
**
** nBuf is the amount of space left in buf[].  nBuf must always be
** large enough to hold the entire field.  Except, if the field is
** a blob with a zero-filled tail, then buf[] might be just the right
** size to hold everything except for the zero-filled tail.  If buf[]
** is only big enough to hold the non-zero prefix, then only write that
** prefix into buf[].  But if buf[] is large enough to hold both the
** prefix and the tail then write the prefix and set the tail to all
** zeros.
**
** Return the number of bytes actually written into buf[].  The number
** of bytes in the zero-filled tail is included in the return value only
** if those bytes were zeroed in buf[].
*/ 
SQLITE_PRIVATE u32 sqlite3VdbeSerialPut(u8 *buf, int nBuf, Mem *pMem, int file_format){
  u32 serial_type = sqlite3VdbeSerialType(pMem, file_format);
  u32 len;

  /* Integer and Real */
  if( serial_type<=7 && serial_type>0 ){
    u64 v;
    u32 i;
    if( serial_type==7 ){
      assert( sizeof(v)==sizeof(pMem->r) );
      memcpy(&v, &pMem->r, sizeof(v));
      swapMixedEndianFloat(v);
    }else{
      v = pMem->u.i;
    }
    len = i = sqlite3VdbeSerialTypeLen(serial_type);
    assert( len<=(u32)nBuf );
    while( i-- ){
      buf[i] = (u8)(v&0xFF);
      v >>= 8;
    }
    return len;
  }

  /* String or blob */
  if( serial_type>=12 ){
    assert( pMem->n + ((pMem->flags & MEM_Zero)?pMem->u.nZero:0)
             == (int)sqlite3VdbeSerialTypeLen(serial_type) );
    assert( pMem->n<=nBuf );
    len = pMem->n;
    memcpy(buf, pMem->z, len);
    if( pMem->flags & MEM_Zero ){
      len += pMem->u.nZero;
      assert( nBuf>=0 );
      if( len > (u32)nBuf ){
        len = (u32)nBuf;
      }
      memset(&buf[pMem->n], 0, len-pMem->n);
    }
    return len;
  }

  /* NULL or constants 0 or 1 */
  return 0;
}

/*
** Deserialize the data blob pointed to by buf as serial type serial_type
** and store the result in pMem.  Return the number of bytes read.
*/ 
SQLITE_PRIVATE u32 sqlite3VdbeSerialGet(
  const unsigned char *buf,     /* Buffer to deserialize from */
  u32 serial_type,              /* Serial type to deserialize */
  Mem *pMem                     /* Memory cell to write value into */
){
  switch( serial_type ){
    case 10:   /* Reserved for future use */
    case 11:   /* Reserved for future use */
    case 0: {  /* NULL */
      pMem->flags = MEM_Null;
      break;
    }
    case 1: { /* 1-byte signed integer */
      pMem->u.i = (signed char)buf[0];
      pMem->flags = MEM_Int;
      return 1;
    }
    case 2: { /* 2-byte signed integer */
      pMem->u.i = (((signed char)buf[0])<<8) | buf[1];
      pMem->flags = MEM_Int;
      return 2;
    }
    case 3: { /* 3-byte signed integer */
      pMem->u.i = (((signed char)buf[0])<<16) | (buf[1]<<8) | buf[2];
      pMem->flags = MEM_Int;
      return 3;
    }
    case 4: { /* 4-byte signed integer */
      pMem->u.i = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
      pMem->flags = MEM_Int;
      return 4;
    }
    case 5: { /* 6-byte signed integer */
      u64 x = (((signed char)buf[0])<<8) | buf[1];
      u32 y = (buf[2]<<24) | (buf[3]<<16) | (buf[4]<<8) | buf[5];
      x = (x<<32) | y;
      pMem->u.i = *(i64*)&x;
      pMem->flags = MEM_Int;
      return 6;
    }
    case 6:   /* 8-byte signed integer */
    case 7: { /* IEEE floating point */
      u64 x;
      u32 y;
#if !defined(NDEBUG) && !defined(SQLITE_OMIT_FLOATING_POINT)
      /* Verify that integers and floating point values use the same
      ** byte order.  Or, that if SQLITE_MIXED_ENDIAN_64BIT_FLOAT is
      ** defined that 64-bit floating point values really are mixed
      ** endian.
      */
      static const u64 t1 = ((u64)0x3ff00000)<<32;
      static const double r1 = 1.0;
      u64 t2 = t1;
      swapMixedEndianFloat(t2);
      assert( sizeof(r1)==sizeof(t2) && memcmp(&r1, &t2, sizeof(r1))==0 );
#endif

      x = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
      y = (buf[4]<<24) | (buf[5]<<16) | (buf[6]<<8) | buf[7];
      x = (x<<32) | y;
      if( serial_type==6 ){
        pMem->u.i = *(i64*)&x;
        pMem->flags = MEM_Int;
      }else{
        assert( sizeof(x)==8 && sizeof(pMem->r)==8 );
        swapMixedEndianFloat(x);
        memcpy(&pMem->r, &x, sizeof(x));
        pMem->flags = sqlite3IsNaN(pMem->r) ? MEM_Null : MEM_Real;
      }
      return 8;
    }
    case 8:    /* Integer 0 */
    case 9: {  /* Integer 1 */
      pMem->u.i = serial_type-8;
      pMem->flags = MEM_Int;
      return 0;
    }
    default: {
      u32 len = (serial_type-12)/2;
      pMem->z = (char *)buf;
      pMem->n = len;
      pMem->xDel = 0;
      if( serial_type&0x01 ){
        pMem->flags = MEM_Str | MEM_Ephem;
      }else{
        pMem->flags = MEM_Blob | MEM_Ephem;
      }
      return len;
    }
  }
  return 0;
}


/*
** Given the nKey-byte encoding of a record in pKey[], parse the
** record into a UnpackedRecord structure.  Return a pointer to
** that structure.
**
** The calling function might provide szSpace bytes of memory
** space at pSpace.  This space can be used to hold the returned
** VDbeParsedRecord structure if it is large enough.  If it is
** not big enough, space is obtained from sqlite3_malloc().
**
** The returned structure should be closed by a call to
** sqlite3VdbeDeleteUnpackedRecord().
*/ 
SQLITE_PRIVATE UnpackedRecord *sqlite3VdbeRecordUnpack(
  KeyInfo *pKeyInfo,     /* Information about the record format */
  int nKey,              /* Size of the binary record */
  const void *pKey,      /* The binary record */
  char *pSpace,          /* Unaligned space available to hold the object */
  int szSpace            /* Size of pSpace[] in bytes */
){
  const unsigned char *aKey = (const unsigned char *)pKey;
  UnpackedRecord *p;  /* The unpacked record that we will return */
  int nByte;          /* Memory space needed to hold p, in bytes */
  int d;
  u32 idx;
  u16 u;              /* Unsigned loop counter */
  u32 szHdr;
  Mem *pMem;
  int nOff;           /* Increase pSpace by this much to 8-byte align it */
  
  /*
  ** We want to shift the pointer pSpace up such that it is 8-byte aligned.
  ** Thus, we need to calculate a value, nOff, between 0 and 7, to shift 
  ** it by.  If pSpace is already 8-byte aligned, nOff should be zero.
  */
  nOff = (8 - (SQLITE_PTR_TO_INT(pSpace) & 7)) & 7;
  pSpace += nOff;
  szSpace -= nOff;
  nByte = ROUND8(sizeof(UnpackedRecord)) + sizeof(Mem)*(pKeyInfo->nField+1);
  if( nByte>szSpace ){
    p = sqlite3DbMallocRaw(pKeyInfo->db, nByte);
    if( p==0 ) return 0;
    p->flags = UNPACKED_NEED_FREE | UNPACKED_NEED_DESTROY;
  }else{
    p = (UnpackedRecord*)pSpace;
    p->flags = UNPACKED_NEED_DESTROY;
  }
  p->pKeyInfo = pKeyInfo;
  p->nField = pKeyInfo->nField + 1;
  p->aMem = pMem = (Mem*)&((char*)p)[ROUND8(sizeof(UnpackedRecord))];
  assert( EIGHT_BYTE_ALIGNMENT(pMem) );
  idx = getVarint32(aKey, szHdr);
  d = szHdr;
  u = 0;
  while( idx<szHdr && u<p->nField && d<=nKey ){
    u32 serial_type;

    idx += getVarint32(&aKey[idx], serial_type);
    pMem->enc = pKeyInfo->enc;
    pMem->db = pKeyInfo->db;
    /* pMem->flags = 0; // sqlite3VdbeSerialGet() will set this for us */
    pMem->zMalloc = 0;
    d += sqlite3VdbeSerialGet(&aKey[d], serial_type, pMem);
    pMem++;
    u++;
  }
  assert( u<=pKeyInfo->nField + 1 );
  p->nField = u;
  return (void*)p;
}

/*
** This routine destroys a UnpackedRecord object.
*/
SQLITE_PRIVATE void sqlite3VdbeDeleteUnpackedRecord(UnpackedRecord *p){
#ifdef SQLITE_DEBUG
  int i;
  Mem *pMem;

  assert( p!=0 );
  assert( p->flags & UNPACKED_NEED_DESTROY );
  for(i=0, pMem=p->aMem; i<p->nField; i++, pMem++){
    /* The unpacked record is always constructed by the
    ** sqlite3VdbeUnpackRecord() function above, which makes all
    ** strings and blobs static.  And none of the elements are
    ** ever transformed, so there is never anything to delete.
    */
    if( NEVER(pMem->zMalloc) ) sqlite3VdbeMemRelease(pMem);
  }
#endif
  if( p->flags & UNPACKED_NEED_FREE ){
    sqlite3DbFree(p->pKeyInfo->db, p);
  }
}

/*
** This function compares the two table rows or index records
** specified by {nKey1, pKey1} and pPKey2.  It returns a negative, zero
** or positive integer if key1 is less than, equal to or 
** greater than key2.  The {nKey1, pKey1} key must be a blob
** created by th OP_MakeRecord opcode of the VDBE.  The pPKey2
** key must be a parsed key such as obtained from
** sqlite3VdbeParseRecord.
**
** Key1 and Key2 do not have to contain the same number of fields.
** The key with fewer fields is usually compares less than the 
** longer key.  However if the UNPACKED_INCRKEY flags in pPKey2 is set
** and the common prefixes are equal, then key1 is less than key2.
** Or if the UNPACKED_MATCH_PREFIX flag is set and the prefixes are
** equal, then the keys are considered to be equal and
** the parts beyond the common prefix are ignored.
**
** If the UNPACKED_IGNORE_ROWID flag is set, then the last byte of
** the header of pKey1 is ignored.  It is assumed that pKey1 is
** an index key, and thus ends with a rowid value.  The last byte
** of the header will therefore be the serial type of the rowid:
** one of 1, 2, 3, 4, 5, 6, 8, or 9 - the integer serial types.
** The serial type of the final rowid will always be a single byte.
** By ignoring this last byte of the header, we force the comparison
** to ignore the rowid at the end of key1.
*/
SQLITE_PRIVATE int sqlite3VdbeRecordCompare(
  int nKey1, const void *pKey1, /* Left key */
  UnpackedRecord *pPKey2        /* Right key */
){
  int d1;            /* Offset into aKey[] of next data element */
  u32 idx1;          /* Offset into aKey[] of next header element */
  u32 szHdr1;        /* Number of bytes in header */
  int i = 0;
  int nField;
  int rc = 0;
  const unsigned char *aKey1 = (const unsigned char *)pKey1;
  KeyInfo *pKeyInfo;
  Mem mem1;

  pKeyInfo = pPKey2->pKeyInfo;
  mem1.enc = pKeyInfo->enc;
  mem1.db = pKeyInfo->db;
  /* mem1.flags = 0;  // Will be initialized by sqlite3VdbeSerialGet() */
  VVA_ONLY( mem1.zMalloc = 0; ) /* Only needed by assert() statements */

  /* Compilers may complain that mem1.u.i is potentially uninitialized.
  ** We could initialize it, as shown here, to silence those complaints.
  ** But in fact, mem1.u.i will never actually be used uninitialized, and doing 
  ** the unnecessary initialization has a measurable negative performance
  ** impact, since this routine is a very high runner.  And so, we choose
  ** to ignore the compiler warnings and leave this variable uninitialized.
  */
  /*  mem1.u.i = 0;  // not needed, here to silence compiler warning */
  
  idx1 = getVarint32(aKey1, szHdr1);
  d1 = szHdr1;
  if( pPKey2->flags & UNPACKED_IGNORE_ROWID ){
    szHdr1--;
  }
  nField = pKeyInfo->nField;
  while( idx1<szHdr1 && i<pPKey2->nField ){
    u32 serial_type1;

    /* Read the serial types for the next element in each key. */
    idx1 += getVarint32( aKey1+idx1, serial_type1 );
    if( d1>=nKey1 && sqlite3VdbeSerialTypeLen(serial_type1)>0 ) break;

    /* Extract the values to be compared.
    */
    d1 += sqlite3VdbeSerialGet(&aKey1[d1], serial_type1, &mem1);

    /* Do the comparison
    */
    rc = sqlite3MemCompare(&mem1, &pPKey2->aMem[i],
                           i<nField ? pKeyInfo->aColl[i] : 0);
    if( rc!=0 ){
      assert( mem1.zMalloc==0 );  /* See comment below */

      /* Invert the result if we are using DESC sort order. */
      if( pKeyInfo->aSortOrder && i<nField && pKeyInfo->aSortOrder[i] ){
        rc = -rc;
      }
    
      /* If the PREFIX_SEARCH flag is set and all fields except the final
      ** rowid field were equal, then clear the PREFIX_SEARCH flag and set 
      ** pPKey2->rowid to the value of the rowid field in (pKey1, nKey1).
      ** This is used by the OP_IsUnique opcode.
      */
      if( (pPKey2->flags & UNPACKED_PREFIX_SEARCH) && i==(pPKey2->nField-1) ){
        assert( idx1==szHdr1 && rc );
        assert( mem1.flags & MEM_Int );
        pPKey2->flags &= ~UNPACKED_PREFIX_SEARCH;
        pPKey2->rowid = mem1.u.i;
      }
    
      return rc;
    }
    i++;
  }

  /* No memory allocation is ever used on mem1.  Prove this using
  ** the following assert().  If the assert() fails, it indicates a
  ** memory leak and a need to call sqlite3VdbeMemRelease(&mem1).
  */
  assert( mem1.zMalloc==0 );

  /* rc==0 here means that one of the keys ran out of fields and
  ** all the fields up to that point were equal. If the UNPACKED_INCRKEY
  ** flag is set, then break the tie by treating key2 as larger.
  ** If the UPACKED_PREFIX_MATCH flag is set, then keys with common prefixes
  ** are considered to be equal.  Otherwise, the longer key is the 
  ** larger.  As it happens, the pPKey2 will always be the longer
  ** if there is a difference.
  */
  assert( rc==0 );
  if( pPKey2->flags & UNPACKED_INCRKEY ){
    rc = -1;
  }else if( pPKey2->flags & UNPACKED_PREFIX_MATCH ){
    /* Leave rc==0 */
  }else if( idx1<szHdr1 ){
    rc = 1;
  }
  return rc;
}
 

/*
** pCur points at an index entry created using the OP_MakeRecord opcode.
** Read the rowid (the last field in the record) and store it in *rowid.
** Return SQLITE_OK if everything works, or an error code otherwise.
**
** pCur might be pointing to text obtained from a corrupt database file.
** So the content cannot be trusted.  Do appropriate checks on the content.
*/
SQLITE_PRIVATE int sqlite3VdbeIdxRowid(sqlite3 *db, BtCursor *pCur, i64 *rowid){
  i64 nCellKey = 0;
  int rc;
  u32 szHdr;        /* Size of the header */
  u32 typeRowid;    /* Serial type of the rowid */
  u32 lenRowid;     /* Size of the rowid */
  Mem m, v;

  UNUSED_PARAMETER(db);

  /* Get the size of the index entry.  Only indices entries of less
  ** than 2GiB are support - anything large must be database corruption.
  ** Any corruption is detected in sqlite3BtreeParseCellPtr(), though, so
  ** this code can safely assume that nCellKey is 32-bits  
  */
  assert( sqlite3BtreeCursorIsValid(pCur) );
  rc = sqlite3BtreeKeySize(pCur, &nCellKey);
  assert( rc==SQLITE_OK );     /* pCur is always valid so KeySize cannot fail */
  assert( (nCellKey & SQLITE_MAX_U32)==(u64)nCellKey );

  /* Read in the complete content of the index entry */
  memset(&m, 0, sizeof(m));
  rc = sqlite3VdbeMemFromBtree(pCur, 0, (int)nCellKey, 1, &m);
  if( rc ){
    return rc;
  }

  /* The index entry must begin with a header size */
  (void)getVarint32((u8*)m.z, szHdr);
  testcase( szHdr==3 );
  testcase( szHdr==m.n );
  if( unlikely(szHdr<3 || (int)szHdr>m.n) ){
    goto idx_rowid_corruption;
  }

  /* The last field of the index should be an integer - the ROWID.
  ** Verify that the last entry really is an integer. */
  (void)getVarint32((u8*)&m.z[szHdr-1], typeRowid);
  testcase( typeRowid==1 );
  testcase( typeRowid==2 );
  testcase( typeRowid==3 );
  testcase( typeRowid==4 );
  testcase( typeRowid==5 );
  testcase( typeRowid==6 );
  testcase( typeRowid==8 );
  testcase( typeRowid==9 );
  if( unlikely(typeRowid<1 || typeRowid>9 || typeRowid==7) ){
    goto idx_rowid_corruption;
  }
  lenRowid = sqlite3VdbeSerialTypeLen(typeRowid);
  testcase( (u32)m.n==szHdr+lenRowid );
  if( unlikely((u32)m.n<szHdr+lenRowid) ){
    goto idx_rowid_corruption;
  }

  /* Fetch the integer off the end of the index record */
  sqlite3VdbeSerialGet((u8*)&m.z[m.n-lenRowid], typeRowid, &v);
  *rowid = v.u.i;
  sqlite3VdbeMemRelease(&m);
  return SQLITE_OK;

  /* Jump here if database corruption is detected after m has been
  ** allocated.  Free the m object and return SQLITE_CORRUPT. */
idx_rowid_corruption:
  testcase( m.zMalloc!=0 );
  sqlite3VdbeMemRelease(&m);
  return SQLITE_CORRUPT_BKPT;
}

/*
** Compare the key of the index entry that cursor pC is pointing to against
** the key string in pUnpacked.  Write into *pRes a number
** that is negative, zero, or positive if pC is less than, equal to,
** or greater than pUnpacked.  Return SQLITE_OK on success.
**
** pUnpacked is either created without a rowid or is truncated so that it
** omits the rowid at the end.  The rowid at the end of the index entry
** is ignored as well.  Hence, this routine only compares the prefixes 
** of the keys prior to the final rowid, not the entire key.
*/
SQLITE_PRIVATE int sqlite3VdbeIdxKeyCompare(
  VdbeCursor *pC,             /* The cursor to compare against */
  UnpackedRecord *pUnpacked,  /* Unpacked version of key to compare against */
  int *res                    /* Write the comparison result here */
){
  i64 nCellKey = 0;
  int rc;
  BtCursor *pCur = pC->pCursor;
  Mem m;

  assert( sqlite3BtreeCursorIsValid(pCur) );
  rc = sqlite3BtreeKeySize(pCur, &nCellKey);
  assert( rc==SQLITE_OK );    /* pCur is always valid so KeySize cannot fail */
  /* nCellKey will always be between 0 and 0xffffffff because of the say
  ** that btreeParseCellPtr() and sqlite3GetVarint32() are implemented */
  if( nCellKey<=0 || nCellKey>0x7fffffff ){
    *res = 0;
    return SQLITE_CORRUPT_BKPT;
  }
  memset(&m, 0, sizeof(m));
  rc = sqlite3VdbeMemFromBtree(pC->pCursor, 0, (int)nCellKey, 1, &m);
  if( rc ){
    return rc;
  }
  assert( pUnpacked->flags & UNPACKED_IGNORE_ROWID );
  *res = sqlite3VdbeRecordCompare(m.n, m.z, pUnpacked);
  sqlite3VdbeMemRelease(&m);
  return SQLITE_OK;
}

/*
** This routine sets the value to be returned by subsequent calls to
** sqlite3_changes() on the database handle 'db'. 
*/
SQLITE_PRIVATE void sqlite3VdbeSetChanges(sqlite3 *db, int nChange){
  assert( sqlite3_mutex_held(db->mutex) );
  db->nChange = nChange;
  db->nTotalChange += nChange;
}

/*
** Set a flag in the vdbe to update the change counter when it is finalised
** or reset.
*/
SQLITE_PRIVATE void sqlite3VdbeCountChanges(Vdbe *v){
  v->changeCntOn = 1;
}

/*
** Mark every prepared statement associated with a database connection
** as expired.
**
** An expired statement means that recompilation of the statement is
** recommend.  Statements expire when things happen that make their
** programs obsolete.  Removing user-defined functions or collating
** sequences, or changing an authorization function are the types of
** things that make prepared statements obsolete.
*/
SQLITE_PRIVATE void sqlite3ExpirePreparedStatements(sqlite3 *db){
  Vdbe *p;
  for(p = db->pVdbe; p; p=p->pNext){
    p->expired = 1;
  }
}

/*
** Return the database associated with the Vdbe.
*/
SQLITE_PRIVATE sqlite3 *sqlite3VdbeDb(Vdbe *v){
  return v->db;
}

/*
** Return a pointer to an sqlite3_value structure containing the value bound
** parameter iVar of VM v. Except, if the value is an SQL NULL, return 
** 0 instead. Unless it is NULL, apply affinity aff (one of the SQLITE_AFF_*
** constants) to the value before returning it.
**
** The returned value must be freed by the caller using sqlite3ValueFree().
*/
SQLITE_PRIVATE sqlite3_value *sqlite3VdbeGetValue(Vdbe *v, int iVar, u8 aff){
  assert( iVar>0 );
  if( v ){
    Mem *pMem = &v->aVar[iVar-1];
    if( 0==(pMem->flags & MEM_Null) ){
      sqlite3_value *pRet = sqlite3ValueNew(v->db);
      if( pRet ){
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3ValueApplyAffinity(pRet, aff, SQLITE_UTF8);
        sqlite3VdbeMemStoreType((Mem *)pRet);
      }
      return pRet;
    }
  }
  return 0;
}

/*
** Configure SQL variable iVar so that binding a new value to it signals
** to sqlite3_reoptimize() that re-preparing the statement may result
** in a better query plan.
*/
SQLITE_PRIVATE void sqlite3VdbeSetVarmask(Vdbe *v, int iVar){
  assert( iVar>0 );
  if( iVar>32 ){
    v->expmask = 0xffffffff;
  }else{
    v->expmask |= ((u32)1 << (iVar-1));
  }
}

/************** End of vdbeaux.c *********************************************/
/************** Begin file vdbeapi.c *****************************************/
/*
** 2004 May 26
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains code use to implement APIs that are part of the
** VDBE.
*/

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return TRUE (non-zero) of the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
*/
SQLITE_API int sqlite3_expired(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  return p==0 || p->expired;
}
#endif

/*
** Check on a Vdbe to make sure it has not been finalized.  Log
** an error and return true if it has been finalized (or is otherwise
** invalid).  Return false if it is ok.
*/
static int vdbeSafety(Vdbe *p){
  if( p->db==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    return 1;
  }else{
    return 0;
  }
}
static int vdbeSafetyNotNull(Vdbe *p){
  if( p==0 ){
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    return 1;
  }else{
    return vdbeSafety(p);
  }
}

/*
** The following routine destroys a virtual machine that is created by
** the sqlite3_compile() routine. The integer returned is an SQLITE_
** success/failure code that describes the result of executing the virtual
** machine.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
SQLITE_API int sqlite3_finalize(sqlite3_stmt *pStmt){
  int rc;
  if( pStmt==0 ){
    /* IMPLEMENTATION-OF: R-57228-12904 Invoking sqlite3_finalize() on a NULL
    ** pointer is a harmless no-op. */
    rc = SQLITE_OK;
  }else{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3 *db = v->db;
#if SQLITE_THREADSAFE
    sqlite3_mutex *mutex;
#endif
    if( vdbeSafety(v) ) return SQLITE_MISUSE_BKPT;
#if SQLITE_THREADSAFE
    mutex = v->db->mutex;
#endif
    sqlite3_mutex_enter(mutex);
    rc = sqlite3VdbeFinalize(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(mutex);
  }
  return rc;
}

/*
** Terminate the current execution of an SQL statement and reset it
** back to its starting state so that it can be reused. A success code from
** the prior execution is returned.
**
** This routine sets the error code and string returned by
** sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16().
*/
SQLITE_API int sqlite3_reset(sqlite3_stmt *pStmt){
  int rc;
  if( pStmt==0 ){
    rc = SQLITE_OK;
  }else{
    Vdbe *v = (Vdbe*)pStmt;
    sqlite3_mutex_enter(v->db->mutex);
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);
    assert( (rc & (v->db->errMask))==rc );
    rc = sqlite3ApiExit(v->db, rc);
    sqlite3_mutex_leave(v->db->mutex);
  }
  return rc;
}

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
SQLITE_API int sqlite3_clear_bindings(sqlite3_stmt *pStmt){
  int i;
  int rc = SQLITE_OK;
  Vdbe *p = (Vdbe*)pStmt;
#if SQLITE_THREADSAFE
  sqlite3_mutex *mutex = ((Vdbe*)pStmt)->db->mutex;
#endif
  sqlite3_mutex_enter(mutex);
  for(i=0; i<p->nVar; i++){
    sqlite3VdbeMemRelease(&p->aVar[i]);
    p->aVar[i].flags = MEM_Null;
  }
  if( p->isPrepareV2 && p->expmask ){
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  return rc;
}


/**************************** sqlite3_value_  *******************************
** The following routines extract information from a Mem or sqlite3_value
** structure.
*/
SQLITE_API const void *sqlite3_value_blob(sqlite3_value *pVal){
  Mem *p = (Mem*)pVal;
  if( p->flags & (MEM_Blob|MEM_Str) ){
    sqlite3VdbeMemExpandBlob(p);
    p->flags &= ~MEM_Str;
    p->flags |= MEM_Blob;
    return p->n ? p->z : 0;
  }else{
    return sqlite3_value_text(pVal);
  }
}
SQLITE_API int sqlite3_value_bytes(sqlite3_value *pVal){
  return sqlite3ValueBytes(pVal, SQLITE_UTF8);
}
SQLITE_API int sqlite3_value_bytes16(sqlite3_value *pVal){
  return sqlite3ValueBytes(pVal, SQLITE_UTF16NATIVE);
}
SQLITE_API double sqlite3_value_double(sqlite3_value *pVal){
  return sqlite3VdbeRealValue((Mem*)pVal);
}
SQLITE_API int sqlite3_value_int(sqlite3_value *pVal){
  return (int)sqlite3VdbeIntValue((Mem*)pVal);
}
SQLITE_API sqlite_int64 sqlite3_value_int64(sqlite3_value *pVal){
  return sqlite3VdbeIntValue((Mem*)pVal);
}
SQLITE_API const unsigned char *sqlite3_value_text(sqlite3_value *pVal){
  return (const unsigned char *)sqlite3ValueText(pVal, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API const void *sqlite3_value_text16(sqlite3_value* pVal){
  return sqlite3ValueText(pVal, SQLITE_UTF16NATIVE);
}
SQLITE_API const void *sqlite3_value_text16be(sqlite3_value *pVal){
  return sqlite3ValueText(pVal, SQLITE_UTF16BE);
}
SQLITE_API const void *sqlite3_value_text16le(sqlite3_value *pVal){
  return sqlite3ValueText(pVal, SQLITE_UTF16LE);
}
#endif /* SQLITE_OMIT_UTF16 */
SQLITE_API int sqlite3_value_type(sqlite3_value* pVal){
  return pVal->type;
}

/**************************** sqlite3_result_  *******************************
** The following routines are used by user-defined functions to specify
** the function result.
**
** The setStrOrError() funtion calls sqlite3VdbeMemSetStr() to store the
** result as a string or blob but if the string or blob is too large, it
** then sets the error code to SQLITE_TOOBIG
*/
static void setResultStrOrError(
  sqlite3_context *pCtx,  /* Function context */
  const char *z,          /* String pointer */
  int n,                  /* Bytes in string, or negative */
  u8 enc,                 /* Encoding of z.  0 for BLOBs */
  void (*xDel)(void*)     /* Destructor function */
){
  if( sqlite3VdbeMemSetStr(&pCtx->s, z, n, enc, xDel)==SQLITE_TOOBIG ){
    sqlite3_result_error_toobig(pCtx);
  }
}
SQLITE_API void sqlite3_result_blob(
  sqlite3_context *pCtx, 
  const void *z, 
  int n, 
  void (*xDel)(void *)
){
  assert( n>=0 );
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  setResultStrOrError(pCtx, z, n, 0, xDel);
}
SQLITE_API void sqlite3_result_double(sqlite3_context *pCtx, double rVal){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  sqlite3VdbeMemSetDouble(&pCtx->s, rVal);
}
SQLITE_API void sqlite3_result_error(sqlite3_context *pCtx, const char *z, int n){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(&pCtx->s, z, n, SQLITE_UTF8, SQLITE_TRANSIENT);
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API void sqlite3_result_error16(sqlite3_context *pCtx, const void *z, int n){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  pCtx->isError = SQLITE_ERROR;
  sqlite3VdbeMemSetStr(&pCtx->s, z, n, SQLITE_UTF16NATIVE, SQLITE_TRANSIENT);
}
#endif
SQLITE_API void sqlite3_result_int(sqlite3_context *pCtx, int iVal){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  sqlite3VdbeMemSetInt64(&pCtx->s, (i64)iVal);
}
SQLITE_API void sqlite3_result_int64(sqlite3_context *pCtx, i64 iVal){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  sqlite3VdbeMemSetInt64(&pCtx->s, iVal);
}
SQLITE_API void sqlite3_result_null(sqlite3_context *pCtx){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  sqlite3VdbeMemSetNull(&pCtx->s);
}
SQLITE_API void sqlite3_result_text(
  sqlite3_context *pCtx, 
  const char *z, 
  int n,
  void (*xDel)(void *)
){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF8, xDel);
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API void sqlite3_result_text16(
  sqlite3_context *pCtx, 
  const void *z, 
  int n, 
  void (*xDel)(void *)
){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF16NATIVE, xDel);
}
SQLITE_API void sqlite3_result_text16be(
  sqlite3_context *pCtx, 
  const void *z, 
  int n, 
  void (*xDel)(void *)
){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF16BE, xDel);
}
SQLITE_API void sqlite3_result_text16le(
  sqlite3_context *pCtx, 
  const void *z, 
  int n, 
  void (*xDel)(void *)
){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  setResultStrOrError(pCtx, z, n, SQLITE_UTF16LE, xDel);
}
#endif /* SQLITE_OMIT_UTF16 */
SQLITE_API void sqlite3_result_value(sqlite3_context *pCtx, sqlite3_value *pValue){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  sqlite3VdbeMemCopy(&pCtx->s, pValue);
}
SQLITE_API void sqlite3_result_zeroblob(sqlite3_context *pCtx, int n){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  sqlite3VdbeMemSetZeroBlob(&pCtx->s, n);
}
SQLITE_API void sqlite3_result_error_code(sqlite3_context *pCtx, int errCode){
  pCtx->isError = errCode;
  if( pCtx->s.flags & MEM_Null ){
    sqlite3VdbeMemSetStr(&pCtx->s, sqlite3ErrStr(errCode), -1, 
                         SQLITE_UTF8, SQLITE_STATIC);
  }
}

/* Force an SQLITE_TOOBIG error. */
SQLITE_API void sqlite3_result_error_toobig(sqlite3_context *pCtx){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  pCtx->isError = SQLITE_TOOBIG;
  sqlite3VdbeMemSetStr(&pCtx->s, "string or blob too big", -1, 
                       SQLITE_UTF8, SQLITE_STATIC);
}

/* An SQLITE_NOMEM error. */
SQLITE_API void sqlite3_result_error_nomem(sqlite3_context *pCtx){
  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  sqlite3VdbeMemSetNull(&pCtx->s);
  pCtx->isError = SQLITE_NOMEM;
  pCtx->s.db->mallocFailed = 1;
}

/*
** This function is called after a transaction has been committed. It 
** invokes callbacks registered with sqlite3_wal_hook() as required.
*/
static int doWalCallbacks(sqlite3 *db){
  int rc = SQLITE_OK;
#ifndef SQLITE_OMIT_WAL
  int i;
  for(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    if( pBt ){
      int nEntry = sqlite3PagerWalCallback(sqlite3BtreePager(pBt));
      if( db->xWalCallback && nEntry>0 && rc==SQLITE_OK ){
        rc = db->xWalCallback(db->pWalArg, db, db->aDb[i].zName, nEntry);
      }
    }
  }
#endif
  return rc;
}

/*
** Execute the statement pStmt, either until a row of data is ready, the
** statement is completely executed or an error occurs.
**
** This routine implements the bulk of the logic behind the sqlite_step()
** API.  The only thing omitted is the automatic recompile if a 
** schema change has occurred.  That detail is handled by the
** outer sqlite3_step() wrapper procedure.
*/
static int sqlite3Step(Vdbe *p){
  sqlite3 *db;
  int rc;

  assert(p);
  if( p->magic!=VDBE_MAGIC_RUN ){
    /* We used to require that sqlite3_reset() be called before retrying
    ** sqlite3_step() after any error or after SQLITE_DONE.  But beginning
    ** with version 3.7.0, we changed this so that sqlite3_reset() would
    ** be called automatically instead of throwing the SQLITE_MISUSE error.
    ** This "automatic-reset" change is not technically an incompatibility, 
    ** since any application that receives an SQLITE_MISUSE is broken by
    ** definition.
    **
    ** Nevertheless, some published applications that were originally written
    ** for version 3.6.23 or earlier do in fact depend on SQLITE_MISUSE 
    ** returns, and the so were broken by the automatic-reset change.  As a
    ** a work-around, the SQLITE_OMIT_AUTORESET compile-time restores the
    ** legacy behavior of returning SQLITE_MISUSE for cases where the 
    ** previous sqlite3_step() returned something other than a SQLITE_LOCKED
    ** or SQLITE_BUSY error.
    */
#ifdef SQLITE_OMIT_AUTORESET
    if( p->rc==SQLITE_BUSY || p->rc==SQLITE_LOCKED ){
      sqlite3_reset((sqlite3_stmt*)p);
    }else{
      return SQLITE_MISUSE_BKPT;
    }
#else
    sqlite3_reset((sqlite3_stmt*)p);
#endif
  }

  /* Check that malloc() has not failed. If it has, return early. */
  db = p->db;
  if( db->mallocFailed ){
    p->rc = SQLITE_NOMEM;
    return SQLITE_NOMEM;
  }

  if( p->pc<=0 && p->expired ){
    p->rc = SQLITE_SCHEMA;
    rc = SQLITE_ERROR;
    goto end_of_step;
  }
  if( p->pc<0 ){
    /* If there are no other statements currently running, then
    ** reset the interrupt flag.  This prevents a call to sqlite3_interrupt
    ** from interrupting a statement that has not yet started.
    */
    if( db->activeVdbeCnt==0 ){
      db->u1.isInterrupted = 0;
    }

    assert( db->writeVdbeCnt>0 || db->autoCommit==0 || db->nDeferredCons==0 );

#ifndef SQLITE_OMIT_TRACE
    if( db->xProfile && !db->init.busy ){
      sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
    }
#endif

    db->activeVdbeCnt++;
    if( p->readOnly==0 ) db->writeVdbeCnt++;
    p->pc = 0;
  }
#ifndef SQLITE_OMIT_EXPLAIN
  if( p->explain ){
    rc = sqlite3VdbeList(p);
  }else
#endif /* SQLITE_OMIT_EXPLAIN */
  {
    db->vdbeExecCnt++;
    rc = sqlite3VdbeExec(p);
    db->vdbeExecCnt--;
  }

#ifndef SQLITE_OMIT_TRACE
  /* Invoke the profile callback if there is one
  */
  if( rc!=SQLITE_ROW && db->xProfile && !db->init.busy && p->zSql ){
    sqlite3_int64 iNow;
    sqlite3OsCurrentTimeInt64(db->pVfs, &iNow);
    db->xProfile(db->pProfileArg, p->zSql, (iNow - p->startTime)*1000000);
  }
#endif

  if( rc==SQLITE_DONE ){
    assert( p->rc==SQLITE_OK );
    p->rc = doWalCallbacks(db);
    if( p->rc!=SQLITE_OK ){
      rc = SQLITE_ERROR;
    }
  }

  db->errCode = rc;
  if( SQLITE_NOMEM==sqlite3ApiExit(p->db, p->rc) ){
    p->rc = SQLITE_NOMEM;
  }
end_of_step:
  /* At this point local variable rc holds the value that should be 
  ** returned if this statement was compiled using the legacy 
  ** sqlite3_prepare() interface. According to the docs, this can only
  ** be one of the values in the first assert() below. Variable p->rc 
  ** contains the value that would be returned if sqlite3_finalize() 
  ** were called on statement p.
  */
  assert( rc==SQLITE_ROW  || rc==SQLITE_DONE   || rc==SQLITE_ERROR 
       || rc==SQLITE_BUSY || rc==SQLITE_MISUSE
  );
  assert( p->rc!=SQLITE_ROW && p->rc!=SQLITE_DONE );
  if( p->isPrepareV2 && rc!=SQLITE_ROW && rc!=SQLITE_DONE ){
    /* If this statement was prepared using sqlite3_prepare_v2(), and an
    ** error has occured, then return the error code in p->rc to the
    ** caller. Set the error code in the database handle to the same value.
    */ 
    rc = db->errCode = p->rc;
  }
  return (rc&db->errMask);
}

/*
** The maximum number of times that a statement will try to reparse
** itself before giving up and returning SQLITE_SCHEMA.
*/
#ifndef SQLITE_MAX_SCHEMA_RETRY
# define SQLITE_MAX_SCHEMA_RETRY 5
#endif

/*
** This is the top-level implementation of sqlite3_step().  Call
** sqlite3Step() to do most of the work.  If a schema error occurs,
** call sqlite3Reprepare() and try again.
*/
SQLITE_API int sqlite3_step(sqlite3_stmt *pStmt){
  int rc = SQLITE_OK;      /* Result from sqlite3Step() */
  int rc2 = SQLITE_OK;     /* Result from sqlite3Reprepare() */
  Vdbe *v = (Vdbe*)pStmt;  /* the prepared statement */
  int cnt = 0;             /* Counter to prevent infinite loop of reprepares */
  sqlite3 *db;             /* The database connection */

  if( vdbeSafetyNotNull(v) ){
    return SQLITE_MISUSE_BKPT;
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  while( (rc = sqlite3Step(v))==SQLITE_SCHEMA
         && cnt++ < SQLITE_MAX_SCHEMA_RETRY
         && (rc2 = rc = sqlite3Reprepare(v))==SQLITE_OK ){
    sqlite3_reset(pStmt);
    v->expired = 0;
  }
  if( rc2!=SQLITE_OK && ALWAYS(v->isPrepareV2) && ALWAYS(db->pErr) ){
    /* This case occurs after failing to recompile an sql statement. 
    ** The error message from the SQL compiler has already been loaded 
    ** into the database handle. This block copies the error message 
    ** from the database handle into the statement and sets the statement
    ** program counter to 0 to ensure that when the statement is 
    ** finalized or reset the parser error message is available via
    ** sqlite3_errmsg() and sqlite3_errcode().
    */
    const char *zErr = (const char *)sqlite3_value_text(db->pErr); 
    sqlite3DbFree(db, v->zErrMsg);
    if( !db->mallocFailed ){
      v->zErrMsg = sqlite3DbStrDup(db, zErr);
      v->rc = rc2;
    } else {
      v->zErrMsg = 0;
      v->rc = rc = SQLITE_NOMEM;
    }
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
*/
SQLITE_API void *sqlite3_user_data(sqlite3_context *p){
  assert( p && p->pFunc );
  return p->pFunc->pUserData;
}

/*
** Extract the user data from a sqlite3_context structure and return a
** pointer to it.
**
** IMPLEMENTATION-OF: R-46798-50301 The sqlite3_context_db_handle() interface
** returns a copy of the pointer to the database connection (the 1st
** parameter) of the sqlite3_create_function() and
** sqlite3_create_function16() routines that originally registered the
** application defined function.
*/
SQLITE_API sqlite3 *sqlite3_context_db_handle(sqlite3_context *p){
  assert( p && p->pFunc );
  return p->s.db;
}

/*
** The following is the implementation of an SQL function that always
** fails with an error message stating that the function is used in the
** wrong context.  The sqlite3_overload_function() API might construct
** SQL function that use this routine so that the functions will exist
** for name resolution but are actually overloaded by the xFindFunction
** method of virtual tables.
*/
SQLITE_PRIVATE void sqlite3InvalidFunction(
  sqlite3_context *context,  /* The function calling context */
  int NotUsed,               /* Number of arguments to the function */
  sqlite3_value **NotUsed2   /* Value of each argument */
){
  const char *zName = context->pFunc->zName;
  char *zErr;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  zErr = sqlite3_mprintf(
      "unable to use function %s in the requested context", zName);
  sqlite3_result_error(context, zErr, -1);
  sqlite3_free(zErr);
}

/*
** Allocate or return the aggregate context for a user function.  A new
** context is allocated on the first call.  Subsequent calls return the
** same context that was returned on prior calls.
*/
SQLITE_API void *sqlite3_aggregate_context(sqlite3_context *p, int nByte){
  Mem *pMem;
  assert( p && p->pFunc && p->pFunc->xStep );
  assert( sqlite3_mutex_held(p->s.db->mutex) );
  pMem = p->pMem;
  testcase( nByte<0 );
  if( (pMem->flags & MEM_Agg)==0 ){
    if( nByte<=0 ){
      sqlite3VdbeMemReleaseExternal(pMem);
      pMem->flags = MEM_Null;
      pMem->z = 0;
    }else{
      sqlite3VdbeMemGrow(pMem, nByte, 0);
      pMem->flags = MEM_Agg;
      pMem->u.pDef = p->pFunc;
      if( pMem->z ){
        memset(pMem->z, 0, nByte);
      }
    }
  }
  return (void*)pMem->z;
}

/*
** Return the auxilary data pointer, if any, for the iArg'th argument to
** the user-function defined by pCtx.
*/
SQLITE_API void *sqlite3_get_auxdata(sqlite3_context *pCtx, int iArg){
  VdbeFunc *pVdbeFunc;

  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  pVdbeFunc = pCtx->pVdbeFunc;
  if( !pVdbeFunc || iArg>=pVdbeFunc->nAux || iArg<0 ){
    return 0;
  }
  return pVdbeFunc->apAux[iArg].pAux;
}

/*
** Set the auxilary data pointer and delete function, for the iArg'th
** argument to the user-function defined by pCtx. Any previous value is
** deleted by calling the delete function specified when it was set.
*/
SQLITE_API void sqlite3_set_auxdata(
  sqlite3_context *pCtx, 
  int iArg, 
  void *pAux, 
  void (*xDelete)(void*)
){
  struct AuxData *pAuxData;
  VdbeFunc *pVdbeFunc;
  if( iArg<0 ) goto failed;

  assert( sqlite3_mutex_held(pCtx->s.db->mutex) );
  pVdbeFunc = pCtx->pVdbeFunc;
  if( !pVdbeFunc || pVdbeFunc->nAux<=iArg ){
    int nAux = (pVdbeFunc ? pVdbeFunc->nAux : 0);
    int nMalloc = sizeof(VdbeFunc) + sizeof(struct AuxData)*iArg;
    pVdbeFunc = sqlite3DbRealloc(pCtx->s.db, pVdbeFunc, nMalloc);
    if( !pVdbeFunc ){
      goto failed;
    }
    pCtx->pVdbeFunc = pVdbeFunc;
    memset(&pVdbeFunc->apAux[nAux], 0, sizeof(struct AuxData)*(iArg+1-nAux));
    pVdbeFunc->nAux = iArg+1;
    pVdbeFunc->pFunc = pCtx->pFunc;
  }

  pAuxData = &pVdbeFunc->apAux[iArg];
  if( pAuxData->pAux && pAuxData->xDelete ){
    pAuxData->xDelete(pAuxData->pAux);
  }
  pAuxData->pAux = pAux;
  pAuxData->xDelete = xDelete;
  return;

failed:
  if( xDelete ){
    xDelete(pAux);
  }
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Return the number of times the Step function of a aggregate has been 
** called.
**
** This function is deprecated.  Do not use it for new code.  It is
** provide only to avoid breaking legacy code.  New aggregate function
** implementations should keep their own counts within their aggregate
** context.
*/
SQLITE_API int sqlite3_aggregate_count(sqlite3_context *p){
  assert( p && p->pMem && p->pFunc && p->pFunc->xStep );
  return p->pMem->n;
}
#endif

/*
** Return the number of columns in the result set for the statement pStmt.
*/
SQLITE_API int sqlite3_column_count(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  return pVm ? pVm->nResColumn : 0;
}

/*
** Return the number of values available from the current row of the
** currently executing statement pStmt.
*/
SQLITE_API int sqlite3_data_count(sqlite3_stmt *pStmt){
  Vdbe *pVm = (Vdbe *)pStmt;
  if( pVm==0 || pVm->pResultSet==0 ) return 0;
  return pVm->nResColumn;
}


/*
** Check to see if column iCol of the given statement is valid.  If
** it is, return a pointer to the Mem for the value of that column.
** If iCol is not valid, return a pointer to a Mem which has a value
** of NULL.
*/
static Mem *columnMem(sqlite3_stmt *pStmt, int i){
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  if( pVm && pVm->pResultSet!=0 && i<pVm->nResColumn && i>=0 ){
    sqlite3_mutex_enter(pVm->db->mutex);
    pOut = &pVm->pResultSet[i];
  }else{
    /* If the value passed as the second argument is out of range, return
    ** a pointer to the following static Mem object which contains the
    ** value SQL NULL. Even though the Mem structure contains an element
    ** of type i64, on certain architecture (x86) with certain compiler
    ** switches (-Os), gcc may align this Mem object on a 4-byte boundary
    ** instead of an 8-byte one. This all works fine, except that when
    ** running with SQLITE_DEBUG defined the SQLite code sometimes assert()s
    ** that a Mem structure is located on an 8-byte boundary. To prevent
    ** this assert() from failing, when building with SQLITE_DEBUG defined
    ** using gcc, force nullMem to be 8-byte aligned using the magical
    ** __attribute__((aligned(8))) macro.  */
    static const Mem nullMem 
#if defined(SQLITE_DEBUG) && defined(__GNUC__)
      __attribute__((aligned(8))) 
#endif
      = {0, "", (double)0, {0}, 0, MEM_Null, SQLITE_NULL, 0,
#ifdef SQLITE_DEBUG
         0, 0,  /* pScopyFrom, pFiller */
#endif
         0, 0 };

    if( pVm && ALWAYS(pVm->db) ){
      sqlite3_mutex_enter(pVm->db->mutex);
      sqlite3Error(pVm->db, SQLITE_RANGE, 0);
    }
    pOut = (Mem*)&nullMem;
  }
  return pOut;
}

/*
** This function is called after invoking an sqlite3_value_XXX function on a 
** column value (i.e. a value returned by evaluating an SQL expression in the
** select list of a SELECT statement) that may cause a malloc() failure. If 
** malloc() has failed, the threads mallocFailed flag is cleared and the result
** code of statement pStmt set to SQLITE_NOMEM.
**
** Specifically, this is called from within:
**
**     sqlite3_column_int()
**     sqlite3_column_int64()
**     sqlite3_column_text()
**     sqlite3_column_text16()
**     sqlite3_column_real()
**     sqlite3_column_bytes()
**     sqlite3_column_bytes16()
**     sqiite3_column_blob()
*/
static void columnMallocFailure(sqlite3_stmt *pStmt)
{
  /* If malloc() failed during an encoding conversion within an
  ** sqlite3_column_XXX API, then set the return code of the statement to
  ** SQLITE_NOMEM. The next call to _step() (if any) will return SQLITE_ERROR
  ** and _finalize() will return NOMEM.
  */
  Vdbe *p = (Vdbe *)pStmt;
  if( p ){
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

/**************************** sqlite3_column_  *******************************
** The following routines are used to access elements of the current row
** in the result set.
*/
SQLITE_API const void *sqlite3_column_blob(sqlite3_stmt *pStmt, int i){
  const void *val;
  val = sqlite3_value_blob( columnMem(pStmt,i) );
  /* Even though there is no encoding conversion, value_blob() might
  ** need to call malloc() to expand the result of a zeroblob() 
  ** expression. 
  */
  columnMallocFailure(pStmt);
  return val;
}
SQLITE_API int sqlite3_column_bytes(sqlite3_stmt *pStmt, int i){
  int val = sqlite3_value_bytes( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  return val;
}
SQLITE_API int sqlite3_column_bytes16(sqlite3_stmt *pStmt, int i){
  int val = sqlite3_value_bytes16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  return val;
}
SQLITE_API double sqlite3_column_double(sqlite3_stmt *pStmt, int i){
  double val = sqlite3_value_double( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  return val;
}
SQLITE_API int sqlite3_column_int(sqlite3_stmt *pStmt, int i){
  int val = sqlite3_value_int( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  return val;
}
SQLITE_API sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, int i){
  sqlite_int64 val = sqlite3_value_int64( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  return val;
}
SQLITE_API const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, int i){
  const unsigned char *val = sqlite3_value_text( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  return val;
}
SQLITE_API sqlite3_value *sqlite3_column_value(sqlite3_stmt *pStmt, int i){
  Mem *pOut = columnMem(pStmt, i);
  if( pOut->flags&MEM_Static ){
    pOut->flags &= ~MEM_Static;
    pOut->flags |= MEM_Ephem;
  }
  columnMallocFailure(pStmt);
  return (sqlite3_value *)pOut;
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API const void *sqlite3_column_text16(sqlite3_stmt *pStmt, int i){
  const void *val = sqlite3_value_text16( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  return val;
}
#endif /* SQLITE_OMIT_UTF16 */
SQLITE_API int sqlite3_column_type(sqlite3_stmt *pStmt, int i){
  int iType = sqlite3_value_type( columnMem(pStmt,i) );
  columnMallocFailure(pStmt);
  return iType;
}

/* The following function is experimental and subject to change or
** removal */
/*int sqlite3_column_numeric_type(sqlite3_stmt *pStmt, int i){
**  return sqlite3_value_numeric_type( columnMem(pStmt,i) );
**}
*/

/*
** Convert the N-th element of pStmt->pColName[] into a string using
** xFunc() then return that string.  If N is out of range, return 0.
**
** There are up to 5 names for each column.  useType determines which
** name is returned.  Here are the names:
**
**    0      The column name as it should be displayed for output
**    1      The datatype name for the column
**    2      The name of the database that the column derives from
**    3      The name of the table that the column derives from
**    4      The name of the table column that the result column derives from
**
** If the result is not a simple column reference (if it is an expression
** or a constant) then useTypes 2, 3, and 4 return NULL.
*/
static const void *columnName(
  sqlite3_stmt *pStmt,
  int N,
  const void *(*xFunc)(Mem*),
  int useType
){
  const void *ret = 0;
  Vdbe *p = (Vdbe *)pStmt;
  int n;
  sqlite3 *db = p->db;
  
  assert( db!=0 );
  n = sqlite3_column_count(pStmt);
  if( N<n && N>=0 ){
    N += useType*n;
    sqlite3_mutex_enter(db->mutex);
    assert( db->mallocFailed==0 );
    ret = xFunc(&p->aColName[N]);
     /* A malloc may have failed inside of the xFunc() call. If this
    ** is the case, clear the mallocFailed flag and return NULL.
    */
    if( db->mallocFailed ){
      db->mallocFailed = 0;
      ret = 0;
    }
    sqlite3_mutex_leave(db->mutex);
  }
  return ret;
}

/*
** Return the name of the Nth column of the result set returned by SQL
** statement pStmt.
*/
SQLITE_API const char *sqlite3_column_name(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text, COLNAME_NAME);
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API const void *sqlite3_column_name16(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text16, COLNAME_NAME);
}
#endif

/*
** Constraint:  If you have ENABLE_COLUMN_METADATA then you must
** not define OMIT_DECLTYPE.
*/
#if defined(SQLITE_OMIT_DECLTYPE) && defined(SQLITE_ENABLE_COLUMN_METADATA)
# error "Must not define both SQLITE_OMIT_DECLTYPE \
         and SQLITE_ENABLE_COLUMN_METADATA"
#endif

#ifndef SQLITE_OMIT_DECLTYPE
/*
** Return the column declaration type (if applicable) of the 'i'th column
** of the result set of SQL statement pStmt.
*/
SQLITE_API const char *sqlite3_column_decltype(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text, COLNAME_DECLTYPE);
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API const void *sqlite3_column_decltype16(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text16, COLNAME_DECLTYPE);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_DECLTYPE */

#ifdef SQLITE_ENABLE_COLUMN_METADATA
/*
** Return the name of the database from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unabiguous reference to a database column.
*/
SQLITE_API const char *sqlite3_column_database_name(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text, COLNAME_DATABASE);
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API const void *sqlite3_column_database_name16(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text16, COLNAME_DATABASE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unabiguous reference to a database column.
*/
SQLITE_API const char *sqlite3_column_table_name(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text, COLNAME_TABLE);
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API const void *sqlite3_column_table_name16(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text16, COLNAME_TABLE);
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the name of the table column from which a result column derives.
** NULL is returned if the result column is an expression or constant or
** anything else which is not an unabiguous reference to a database column.
*/
SQLITE_API const char *sqlite3_column_origin_name(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text, COLNAME_COLUMN);
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API const void *sqlite3_column_origin_name16(sqlite3_stmt *pStmt, int N){
  return columnName(
      pStmt, N, (const void*(*)(Mem*))sqlite3_value_text16, COLNAME_COLUMN);
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_ENABLE_COLUMN_METADATA */


/******************************* sqlite3_bind_  ***************************
** 
** Routines used to attach values to wildcards in a compiled SQL statement.
*/
/*
** Unbind the value bound to variable i in virtual machine p. This is the 
** the same as binding a NULL value to the column. If the "i" parameter is
** out of range, then SQLITE_RANGE is returned. Othewise SQLITE_OK.
**
** A successful evaluation of this routine acquires the mutex on p.
** the mutex is released if any kind of error occurs.
**
** The error code stored in database p->db is overwritten with the return
** value in any case.
*/
static int vdbeUnbind(Vdbe *p, int i){
  Mem *pVar;
  if( vdbeSafetyNotNull(p) ){
    return SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(p->db->mutex);
  if( p->magic!=VDBE_MAGIC_RUN || p->pc>=0 ){
    sqlite3Error(p->db, SQLITE_MISUSE, 0);
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE, 
        "bind on a busy prepared statement: [%s]", p->zSql);
    return SQLITE_MISUSE_BKPT;
  }
  if( i<1 || i>p->nVar ){
    sqlite3Error(p->db, SQLITE_RANGE, 0);
    sqlite3_mutex_leave(p->db->mutex);
    return SQLITE_RANGE;
  }
  i--;
  pVar = &p->aVar[i];
  sqlite3VdbeMemRelease(pVar);
  pVar->flags = MEM_Null;
  sqlite3Error(p->db, SQLITE_OK, 0);

  /* If the bit corresponding to this variable in Vdbe.expmask is set, then 
  ** binding a new value to this variable invalidates the current query plan.
  **
  ** IMPLEMENTATION-OF: R-48440-37595 If the specific value bound to host
  ** parameter in the WHERE clause might influence the choice of query plan
  ** for a statement, then the statement will be automatically recompiled,
  ** as if there had been a schema change, on the first sqlite3_step() call
  ** following any change to the bindings of that parameter.
  */
  if( p->isPrepareV2 &&
     ((i<32 && p->expmask & ((u32)1 << i)) || p->expmask==0xffffffff)
  ){
    p->expired = 1;
  }
  return SQLITE_OK;
}

/*
** Bind a text or BLOB value.
*/
static int bindText(
  sqlite3_stmt *pStmt,   /* The statement to bind against */
  int i,                 /* Index of the parameter to bind */
  const void *zData,     /* Pointer to the data to be bound */
  int nData,             /* Number of bytes of data to be bound */
  void (*xDel)(void*),   /* Destructor for the data */
  u8 encoding            /* Encoding for the data */
){
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  int rc;

  rc = vdbeUnbind(p, i);
  if( rc==SQLITE_OK ){
    if( zData!=0 ){
      pVar = &p->aVar[i-1];
      rc = sqlite3VdbeMemSetStr(pVar, zData, nData, encoding, xDel);
      if( rc==SQLITE_OK && encoding!=0 ){
        rc = sqlite3VdbeChangeEncoding(pVar, ENC(p->db));
      }
      sqlite3Error(p->db, rc, 0);
      rc = sqlite3ApiExit(p->db, rc);
    }
    sqlite3_mutex_leave(p->db->mutex);
  }else if( xDel!=SQLITE_STATIC && xDel!=SQLITE_TRANSIENT ){
    xDel((void*)zData);
  }
  return rc;
}


/*
** Bind a blob value to an SQL statement variable.
*/
SQLITE_API int sqlite3_bind_blob(
  sqlite3_stmt *pStmt, 
  int i, 
  const void *zData, 
  int nData, 
  void (*xDel)(void*)
){
  return bindText(pStmt, i, zData, nData, xDel, 0);
}
SQLITE_API int sqlite3_bind_double(sqlite3_stmt *pStmt, int i, double rValue){
  int rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, i);
  if( rc==SQLITE_OK ){
    sqlite3VdbeMemSetDouble(&p->aVar[i-1], rValue);
    sqlite3_mutex_leave(p->db->mutex);
  }
  return rc;
}
SQLITE_API int sqlite3_bind_int(sqlite3_stmt *p, int i, int iValue){
  return sqlite3_bind_int64(p, i, (i64)iValue);
}
SQLITE_API int sqlite3_bind_int64(sqlite3_stmt *pStmt, int i, sqlite_int64 iValue){
  int rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, i);
  if( rc==SQLITE_OK ){
    sqlite3VdbeMemSetInt64(&p->aVar[i-1], iValue);
    sqlite3_mutex_leave(p->db->mutex);
  }
  return rc;
}
SQLITE_API int sqlite3_bind_null(sqlite3_stmt *pStmt, int i){
  int rc;
  Vdbe *p = (Vdbe*)pStmt;
  rc = vdbeUnbind(p, i);
  if( rc==SQLITE_OK ){
    sqlite3_mutex_leave(p->db->mutex);
  }
  return rc;
}
SQLITE_API int sqlite3_bind_text( 
  sqlite3_stmt *pStmt, 
  int i, 
  const char *zData, 
  int nData, 
  void (*xDel)(void*)
){
  return bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF8);
}
#ifndef SQLITE_OMIT_UTF16
SQLITE_API int sqlite3_bind_text16(
  sqlite3_stmt *pStmt, 
  int i, 
  const void *zData, 
  int nData, 
  void (*xDel)(void*)
){
  return bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF16NATIVE);
}
#endif /* SQLITE_OMIT_UTF16 */
SQLITE_API int sqlite3_bind_value(sqlite3_stmt *pStmt, int i, const sqlite3_value *pValue){
  int rc;
  switch( pValue->type ){
    case SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, pValue->u.i);
      break;
    }
    case SQLITE_FLOAT: {
      rc = sqlite3_bind_double(pStmt, i, pValue->r);
      break;
    }
    case SQLITE_BLOB: {
      if( pValue->flags & MEM_Zero ){
        rc = sqlite3_bind_zeroblob(pStmt, i, pValue->u.nZero);
      }else{
        rc = sqlite3_bind_blob(pStmt, i, pValue->z, pValue->n,SQLITE_TRANSIENT);
      }
      break;
    }
    case SQLITE_TEXT: {
      rc = bindText(pStmt,i,  pValue->z, pValue->n, SQLITE_TRANSIENT,
                              pValue->enc);
      break;
    }
    default: {
      rc = sqlite3_bind_null(pStmt, i);
      break;
    }
  }
  return rc;
}
SQLITE_API int sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, int i, int n){
  int rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, i);
  if( rc==SQLITE_OK ){
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i-1], n);
    sqlite3_mutex_leave(p->db->mutex);
  }
  return rc;
}

/*
** Return the number of wildcards that can be potentially bound to.
** This routine is added to support DBD::SQLite.  
*/
SQLITE_API int sqlite3_bind_parameter_count(sqlite3_stmt *pStmt){
  Vdbe *p = (Vdbe*)pStmt;
  return p ? p->nVar : 0;
}

/*
** Return the name of a wildcard parameter.  Return NULL if the index
** is out of range or if the wildcard is unnamed.
**
** The result is always UTF-8.
*/
SQLITE_API const char *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, int i){
  Vdbe *p = (Vdbe*)pStmt;
  if( p==0 || i<1 || i>p->nzVar ){
    return 0;
  }
  return p->azVar[i-1];
}

/*
** Given a wildcard parameter name, return the index of the variable
** with that name.  If there is no variable with the given name,
** return 0.
*/
SQLITE_PRIVATE int sqlite3VdbeParameterIndex(Vdbe *p, const char *zName, int nName){
  int i;
  if( p==0 ){
    return 0;
  }
  if( zName ){
    for(i=0; i<p->nzVar; i++){
      const char *z = p->azVar[i];
      if( z && memcmp(z,zName,nName)==0 && z[nName]==0 ){
        return i+1;
      }
    }
  }
  return 0;
}
SQLITE_API int sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, const char *zName){
  return sqlite3VdbeParameterIndex((Vdbe*)pStmt, zName, sqlite3Strlen30(zName));
}

/*
** Transfer all bindings from the first statement over to the second.
*/
SQLITE_PRIVATE int sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  int i;
  assert( pTo->db==pFrom->db );
  assert( pTo->nVar==pFrom->nVar );
  sqlite3_mutex_enter(pTo->db->mutex);
  for(i=0; i<pFrom->nVar; i++){
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  return SQLITE_OK;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Deprecated external interface.  Internal/core SQLite code
** should call sqlite3TransferBindings.
**
** Is is misuse to call this routine with statements from different
** database connections.  But as this is a deprecated interface, we
** will not bother to check for that condition.
**
** If the two statements contain a different number of bindings, then
** an SQLITE_ERROR is returned.  Nothing else can go wrong, so otherwise
** SQLITE_OK is returned.
*/
SQLITE_API int sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt){
  Vdbe *pFrom = (Vdbe*)pFromStmt;
  Vdbe *pTo = (Vdbe*)pToStmt;
  if( pFrom->nVar!=pTo->nVar ){
    return SQLITE_ERROR;
  }
  if( pTo->isPrepareV2 && pTo->expmask ){
    pTo->expired = 1;
  }
  if( pFrom->isPrepareV2 && pFrom->expmask ){
    pFrom->expired = 1;
  }
  return sqlite3TransferBindings(pFromStmt, pToStmt);
}
#endif

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
SQLITE_API sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt){
  return pStmt ? ((Vdbe*)pStmt)->db : 0;
}

/*
** Return true if the prepared statement is guaranteed to not modify the
** database.
*/
SQLITE_API int sqlite3_stmt_readonly(sqlite3_stmt *pStmt){
  return pStmt ? ((Vdbe*)pStmt)->readOnly : 1;
}

/*
** Return a pointer to the next prepared statement after pStmt associated
** with database connection pDb.  If pStmt is NULL, return the first
** prepared statement for the database connection.  Return NULL if there
** are no more.
*/
SQLITE_API sqlite3_stmt *sqlite3_next_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt){
  sqlite3_stmt *pNext;
  sqlite3_mutex_enter(pDb->mutex);
  if( pStmt==0 ){
    pNext = (sqlite3_stmt*)pDb->pVdbe;
  }else{
    pNext = (sqlite3_stmt*)((Vdbe*)pStmt)->pNext;
  }
  sqlite3_mutex_leave(pDb->mutex);
  return pNext;
}

/*
** Return the value of a status counter for a prepared statement
*/
SQLITE_API int sqlite3_stmt_status(sqlite3_stmt *pStmt, int op, int resetFlag){
  Vdbe *pVdbe = (Vdbe*)pStmt;
  int v = pVdbe->aCounter[op-1];
  if( resetFlag ) pVdbe->aCounter[op-1] = 0;
  return v;
}

/************** End of vdbeapi.c *********************************************/
/************** Begin file vdbetrace.c ***************************************/
/*
** 2009 November 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains code used to insert the values of host parameters
** (aka "wildcards") into the SQL text output by sqlite3_trace().
*/

#ifndef SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
static int findNextHostParameter(const char *zSql, int *pnToken){
  int tokenType;
  int nTotal = 0;
  int n;

  *pnToken = 0;
  while( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    assert( n>0 && tokenType!=TK_ILLEGAL );
    if( tokenType==TK_VARIABLE ){
      *pnToken = n;
      break;
    }
    nTotal += n;
    zSql += n;
  }
  return nTotal;
}

/*
** This function returns a pointer to a nul-terminated string in memory
** obtained from sqlite3DbMalloc(). If sqlite3.vdbeExecCnt is 1, then the
** string contains a copy of zRawSql but with host parameters expanded to 
** their current bindings. Or, if sqlite3.vdbeExecCnt is greater than 1, 
** then the returned string holds a copy of zRawSql with "-- " prepended
** to each line of text.
**
** The calling function is responsible for making sure the memory returned
** is eventually freed.
**
** ALGORITHM:  Scan the input string looking for host parameters in any of
** these forms:  ?, ?N, $A, @A, :A.  Take care to avoid text within
** string literals, quoted identifier names, and comments.  For text forms,
** the host parameter index is found by scanning the perpared
** statement for the corresponding OP_Variable opcode.  Once the host
** parameter index is known, locate the value in p->aVar[].  Then render
** the value as a literal in place of the host parameter name.
*/
SQLITE_PRIVATE char *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  const char *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  int idx = 0;             /* Index of a host parameter */
  int nextIndex = 1;       /* Index of next ? host parameter */
  int n;                   /* Length of a token prefix */
  int nToken;              /* Length of the parameter token */
  int i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum out;            /* Accumulate the output here */
  char zBase[100];         /* Initial working space */

  db = p->db;
  sqlite3StrAccumInit(&out, zBase, sizeof(zBase), 
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  out.db = db;
  if( db->vdbeExecCnt>1 ){
    while( *zRawSql ){
      const char *zStart = zRawSql;
      while( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3StrAccumAppend(&out, "-- ", 3);
      sqlite3StrAccumAppend(&out, zStart, (int)(zRawSql-zStart));
    }
  }else{
    while( zRawSql[0] ){
      n = findNextHostParameter(zRawSql, &nToken);
      assert( n>0 );
      sqlite3StrAccumAppend(&out, zRawSql, n);
      zRawSql += n;
      assert( zRawSql[0] || nToken==0 );
      if( nToken==0 ) break;
      if( zRawSql[0]=='?' ){
        if( nToken>1 ){
          assert( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }else{
          idx = nextIndex;
        }
      }else{
        assert( zRawSql[0]==':' || zRawSql[0]=='$' || zRawSql[0]=='@' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        idx = sqlite3VdbeParameterIndex(p, zRawSql, nToken);
        assert( idx>0 );
      }
      zRawSql += nToken;
      nextIndex = idx + 1;
      assert( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      if( pVar->flags & MEM_Null ){
        sqlite3StrAccumAppend(&out, "NULL", 4);
      }else if( pVar->flags & MEM_Int ){
        sqlite3XPrintf(&out, "%lld", pVar->u.i);
      }else if( pVar->flags & MEM_Real ){
        sqlite3XPrintf(&out, "%!.15g", pVar->r);
      }else if( pVar->flags & MEM_Str ){
#ifndef SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        if( enc!=SQLITE_UTF8 ){
          Mem utf8;
          memset(&utf8, 0, sizeof(utf8));
          utf8.db = db;
          sqlite3VdbeMemSetStr(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8);
          sqlite3XPrintf(&out, "'%.*q'", utf8.n, utf8.z);
          sqlite3VdbeMemRelease(&utf8);
        }else
#endif
        {
          sqlite3XPrintf(&out, "'%.*q'", pVar->n, pVar->z);
        }
      }else if( pVar->flags & MEM_Zero ){
        sqlite3XPrintf(&out, "zeroblob(%d)", pVar->u.nZero);
      }else{
        assert( pVar->flags & MEM_Blob );
        sqlite3StrAccumAppend(&out, "x'", 2);
        for(i=0; i<pVar->n; i++){
          sqlite3XPrintf(&out, "%02x", pVar->z[i]&0xff);
        }
        sqlite3StrAccumAppend(&out, "'", 1);
      }
    }
  }
  return sqlite3StrAccumFinish(&out);
}

#endif /* #ifndef SQLITE_OMIT_TRACE */

/************** End of vdbetrace.c *******************************************/
/************** Begin file vdbe.c ********************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** The code in this file implements execution method of the 
** Virtual Database Engine (VDBE).  A separate file ("vdbeaux.c")
** handles housekeeping details such as creating and deleting
** VDBE instances.  This file is solely interested in executing
** the VDBE program.
**
** In the external interface, an "sqlite3_stmt*" is an opaque pointer
** to a VDBE.
**
** The SQL parser generates a program which is then executed by
** the VDBE to do the work of the SQL statement.  VDBE programs are 
** similar in form to assembly language.  The program consists of
** a linear sequence of operations.  Each operation has an opcode 
** and 5 operands.  Operands P1, P2, and P3 are integers.  Operand P4 
** is a null-terminated string.  Operand P5 is an unsigned character.
** Few opcodes use all 5 operands.
**
** Computation results are stored on a set of registers numbered beginning
** with 1 and going up to Vdbe.nMem.  Each register can store
** either an integer, a null-terminated string, a floating point
** number, or the SQL "NULL" value.  An implicit conversion from one
** type to the other occurs as necessary.
** 
** Most of the code in this file is taken up by the sqlite3VdbeExec()
** function which does the work of interpreting a VDBE program.
** But other routines are also provided to help in building up
** a program instruction by instruction.
**
** Various scripts scan this source file in order to generate HTML
** documentation, headers files, or other derived files.  The formatting
** of the code in this file is, therefore, important.  See other comments
** in this file for details.  If in doubt, do not deviate from existing
** commenting and indentation practices when changing or adding code.
*/

/*
** Invoke this macro on memory cells just prior to changing the
** value of the cell.  This macro verifies that shallow copies are
** not misused.
*/
#ifdef SQLITE_DEBUG
# define memAboutToChange(P,M) sqlite3VdbeMemPrepareToChange(P,M)
#else
# define memAboutToChange(P,M)
#endif

/*
** The following global variable is incremented every time a cursor
** moves, either by the OP_SeekXX, OP_Next, or OP_Prev opcodes.  The test
** procedures use this information to make sure that indices are
** working correctly.  This variable has no function other than to
** help verify the correct operation of the library.
*/
#ifdef SQLITE_TEST
SQLITE_API int sqlite3_search_count = 0;
#endif

/*
** When this global variable is positive, it gets decremented once before
** each instruction in the VDBE.  When reaches zero, the u1.isInterrupted
** field of the sqlite3 structure is set in order to simulate and interrupt.
**
** This facility is used for testing purposes only.  It does not function
** in an ordinary build.
*/
#ifdef SQLITE_TEST
SQLITE_API int sqlite3_interrupt_count = 0;
#endif

/*
** The next global variable is incremented each type the OP_Sort opcode
** is executed.  The test procedures use this information to make sure that
** sorting is occurring or not occurring at appropriate times.   This variable
** has no function other than to help verify the correct operation of the
** library.
*/
#ifdef SQLITE_TEST
SQLITE_API int sqlite3_sort_count = 0;
#endif

/*
** The next global variable records the size of the largest MEM_Blob
** or MEM_Str that has been used by a VDBE opcode.  The test procedures
** use this information to make sure that the zero-blob functionality
** is working correctly.   This variable has no function other than to
** help verify the correct operation of the library.
*/
#ifdef SQLITE_TEST
SQLITE_API int sqlite3_max_blobsize = 0;
static void updateMaxBlobsize(Mem *p){
  if( (p->flags & (MEM_Str|MEM_Blob))!=0 && p->n>sqlite3_max_blobsize ){
    sqlite3_max_blobsize = p->n;
  }
}
#endif

/*
** The next global variable is incremented each type the OP_Found opcode
** is executed. This is used to test whether or not the foreign key
** operation implemented using OP_FkIsZero is working. This variable
** has no function other than to help verify the correct operation of the
** library.
*/
#ifdef SQLITE_TEST
SQLITE_API int sqlite3_found_count = 0;
#endif

/*
** Test a register to see if it exceeds the current maximum blob size.
** If it does, record the new maximum blob size.
*/
#if defined(SQLITE_TEST) && !defined(SQLITE_OMIT_BUILTIN_TEST)
# define UPDATE_MAX_BLOBSIZE(P)  updateMaxBlobsize(P)
#else
# define UPDATE_MAX_BLOBSIZE(P)
#endif

/*
** Convert the given register into a string if it isn't one
** already. Return non-zero if a malloc() fails.
*/
#define Stringify(P, enc) \
   if(((P)->flags&(MEM_Str|MEM_Blob))==0 && sqlite3VdbeMemStringify(P,enc)) \
     { goto no_mem; }

/*
** An ephemeral string value (signified by the MEM_Ephem flag) contains
** a pointer to a dynamically allocated string where some other entity
** is responsible for deallocating that string.  Because the register
** does not control the string, it might be deleted without the register
** knowing it.
**
** This routine converts an ephemeral string into a dynamically allocated
** string that the register itself controls.  In other words, it
** converts an MEM_Ephem string into an MEM_Dyn string.
*/
#define Deephemeralize(P) \
   if( ((P)->flags&MEM_Ephem)!=0 \
       && sqlite3VdbeMemMakeWriteable(P) ){ goto no_mem;}

/*
** Call sqlite3VdbeMemExpandBlob() on the supplied value (type Mem*)
** P if required.
*/
#define ExpandBlob(P) (((P)->flags&MEM_Zero)?sqlite3VdbeMemExpandBlob(P):0)

/*
** Argument pMem points at a register that will be passed to a
** user-defined function or returned to the user as the result of a query.
** This routine sets the pMem->type variable used by the sqlite3_value_*() 
** routines.
*/
SQLITE_PRIVATE void sqlite3VdbeMemStoreType(Mem *pMem){
  int flags = pMem->flags;
  if( flags & MEM_Null ){
    pMem->type = SQLITE_NULL;
  }
  else if( flags & MEM_Int ){
    pMem->type = SQLITE_INTEGER;
  }
  else if( flags & MEM_Real ){
    pMem->type = SQLITE_FLOAT;
  }
  else if( flags & MEM_Str ){
    pMem->type = SQLITE_TEXT;
  }else{
    pMem->type = SQLITE_BLOB;
  }
}

/*
** Allocate VdbeCursor number iCur.  Return a pointer to it.  Return NULL
** if we run out of memory.
*/
static VdbeCursor *allocateCursor(
  Vdbe *p,              /* The virtual machine */
  int iCur,             /* Index of the new VdbeCursor */
  int nField,           /* Number of fields in the table or index */
  int iDb,              /* When database the cursor belongs to, or -1 */
  int isBtreeCursor     /* True for B-Tree.  False for pseudo-table or vtab */
){
  /* Find the memory cell that will be used to store the blob of memory
  ** required for this VdbeCursor structure. It is convenient to use a 
  ** vdbe memory cell to manage the memory allocation required for a
  ** VdbeCursor structure for the following reasons:
  **
  **   * Sometimes cursor numbers are used for a couple of different
  **     purposes in a vdbe program. The different uses might require
  **     different sized allocations. Memory cells provide growable
  **     allocations.
  **
  **   * When using ENABLE_MEMORY_MANAGEMENT, memory cell buffers can
  **     be freed lazily via the sqlite3_release_memory() API. This
  **     minimizes the number of malloc calls made by the system.
  **
  ** Memory cells for cursors are allocated at the top of the address
  ** space. Memory cell (p->nMem) corresponds to cursor 0. Space for
  ** cursor 1 is managed by memory cell (p->nMem-1), etc.
  */
  Mem *pMem = &p->aMem[p->nMem-iCur];

  int nByte;
  VdbeCursor *pCx = 0;
  nByte = 
      ROUND8(sizeof(VdbeCursor)) + 
      (isBtreeCursor?sqlite3BtreeCursorSize():0) + 
      2*nField*sizeof(u32);

  assert( iCur<p->nCursor );
  if( p->apCsr[iCur] ){
    sqlite3VdbeFreeCursor(p, p->apCsr[iCur]);
    p->apCsr[iCur] = 0;
  }
  if( SQLITE_OK==sqlite3VdbeMemGrow(pMem, nByte, 0) ){
    p->apCsr[iCur] = pCx = (VdbeCursor*)pMem->z;
    memset(pCx, 0, sizeof(VdbeCursor));
    pCx->iDb = iDb;
    pCx->nField = nField;
    if( nField ){
      pCx->aType = (u32 *)&pMem->z[ROUND8(sizeof(VdbeCursor))];
    }
    if( isBtreeCursor ){
      pCx->pCursor = (BtCursor*)
          &pMem->z[ROUND8(sizeof(VdbeCursor))+2*nField*sizeof(u32)];
      sqlite3BtreeCursorZero(pCx->pCursor);
    }
  }
  return pCx;
}

/*
** Try to convert a value into a numeric representation if we can
** do so without loss of information.  In other words, if the string
** looks like a number, convert it into a number.  If it does not
** look like a number, leave it alone.
*/
static void applyNumericAffinity(Mem *pRec){
  if( (pRec->flags & (MEM_Real|MEM_Int))==0 ){
    double rValue;
    i64 iValue;
    u8 enc = pRec->enc;
    if( (pRec->flags&MEM_Str)==0 ) return;
    if( sqlite3AtoF(pRec->z, &rValue, pRec->n, enc)==0 ) return;
    if( 0==sqlite3Atoi64(pRec->z, &iValue, pRec->n, enc) ){
      pRec->u.i = iValue;
      pRec->flags |= MEM_Int;
    }else{
      pRec->r = rValue;
      pRec->flags |= MEM_Real;
    }
  }
}

/*
** Processing is determine by the affinity parameter:
**
** SQLITE_AFF_INTEGER:
** SQLITE_AFF_REAL:
** SQLITE_AFF_NUMERIC:
**    Try to convert pRec to an integer representation or a 
**    floating-point representation if an integer representation
**    is not possible.  Note that the integer representation is
**    always preferred, even if the affinity is REAL, because
**    an integer representation is more space efficient on disk.
**
** SQLITE_AFF_TEXT:
**    Convert pRec to a text representation.
**
** SQLITE_AFF_NONE:
**    No-op.  pRec is unchanged.
*/
static void applyAffinity(
  Mem *pRec,          /* The value to apply affinity to */
  char affinity,      /* The affinity to be applied */
  u8 enc              /* Use this text encoding */
){
  if( affinity==SQLITE_AFF_TEXT ){
    /* Only attempt the conversion to TEXT if there is an integer or real
    ** representation (blob and NULL do not get converted) but no string
    ** representation.
    */
    if( 0==(pRec->flags&MEM_Str) && (pRec->flags&(MEM_Real|MEM_Int)) ){
      sqlite3VdbeMemStringify(pRec, enc);
    }
    pRec->flags &= ~(MEM_Real|MEM_Int);
  }else if( affinity!=SQLITE_AFF_NONE ){
    assert( affinity==SQLITE_AFF_INTEGER || affinity==SQLITE_AFF_REAL
             || affinity==SQLITE_AFF_NUMERIC );
    applyNumericAffinity(pRec);
    if( pRec->flags & MEM_Real ){
      sqlite3VdbeIntegerAffinity(pRec);
    }
  }
}

/*
** Try to convert the type of a function argument or a result column
** into a numeric representation.  Use either INTEGER or REAL whichever
** is appropriate.  But only do the conversion if it is possible without
** loss of information and return the revised type of the argument.
*/
SQLITE_API int sqlite3_value_numeric_type(sqlite3_value *pVal){
  Mem *pMem = (Mem*)pVal;
  if( pMem->type==SQLITE_TEXT ){
    applyNumericAffinity(pMem);
    sqlite3VdbeMemStoreType(pMem);
  }
  return pMem->type;
}

/*
** Exported version of applyAffinity(). This one works on sqlite3_value*, 
** not the internal Mem* type.
*/
SQLITE_PRIVATE void sqlite3ValueApplyAffinity(
  sqlite3_value *pVal, 
  u8 affinity, 
  u8 enc
){
  applyAffinity((Mem *)pVal, affinity, enc);
}

#ifdef SQLITE_DEBUG
/*
** Write a nice string representation of the contents of cell pMem
** into buffer zBuf, length nBuf.
*/
SQLITE_PRIVATE void sqlite3VdbeMemPrettyPrint(Mem *pMem, char *zBuf){
  char *zCsr = zBuf;
  int f = pMem->flags;

  static const char *const encnames[] = {"(X)", "(8)", "(16LE)", "(16BE)"};

  if( f&MEM_Blob ){
    int i;
    char c;
    if( f & MEM_Dyn ){
      c = 'z';
      assert( (f & (MEM_Static|MEM_Ephem))==0 );
    }else if( f & MEM_Static ){
      c = 't';
      assert( (f & (MEM_Dyn|MEM_Ephem))==0 );
    }else if( f & MEM_Ephem ){
      c = 'e';
      assert( (f & (MEM_Static|MEM_Dyn))==0 );
    }else{
      c = 's';
    }

    sqlite3_snprintf(100, zCsr, "%c", c);
    zCsr += sqlite3Strlen30(zCsr);
    sqlite3_snprintf(100, zCsr, "%d[", pMem->n);
    zCsr += sqlite3Strlen30(zCsr);
    for(i=0; i<16 && i<pMem->n; i++){
      sqlite3_snprintf(100, zCsr, "%02X", ((int)pMem->z[i] & 0xFF));
      zCsr += sqlite3Strlen30(zCsr);
    }
    for(i=0; i<16 && i<pMem->n; i++){
      char z = pMem->z[i];
      if( z<32 || z>126 ) *zCsr++ = '.';
      else *zCsr++ = z;
    }

    sqlite3_snprintf(100, zCsr, "]%s", encnames[pMem->enc]);
    zCsr += sqlite3Strlen30(zCsr);
    if( f & MEM_Zero ){
      sqlite3_snprintf(100, zCsr,"+%dz",pMem->u.nZero);
      zCsr += sqlite3Strlen30(zCsr);
    }
    *zCsr = '\0';
  }else if( f & MEM_Str ){
    int j, k;
    zBuf[0] = ' ';
    if( f & MEM_Dyn ){
      zBuf[1] = 'z';
      assert( (f & (MEM_Static|MEM_Ephem))==0 );
    }else if( f & MEM_Static ){
      zBuf[1] = 't';
      assert( (f & (MEM_Dyn|MEM_Ephem))==0 );
    }else if( f & MEM_Ephem ){
      zBuf[1] = 'e';
      assert( (f & (MEM_Static|MEM_Dyn))==0 );
    }else{
      zBuf[1] = 's';
    }
    k = 2;
    sqlite3_snprintf(100, &zBuf[k], "%d", pMem->n);
    k += sqlite3Strlen30(&zBuf[k]);
    zBuf[k++] = '[';
    for(j=0; j<15 && j<pMem->n; j++){
      u8 c = pMem->z[j];
      if( c>=0x20 && c<0x7f ){
        zBuf[k++] = c;
      }else{
        zBuf[k++] = '.';
      }
    }
    zBuf[k++] = ']';
    sqlite3_snprintf(100,&zBuf[k], encnames[pMem->enc]);
    k += sqlite3Strlen30(&zBuf[k]);
    zBuf[k++] = 0;
  }
}
#endif

#ifdef SQLITE_DEBUG
/*
** Print the value of a register for tracing purposes:
*/
static void memTracePrint(FILE *out, Mem *p){
  if( p->flags & MEM_Null ){
    fprintf(out, " NULL");
  }else if( (p->flags & (MEM_Int|MEM_Str))==(MEM_Int|MEM_Str) ){
    fprintf(out, " si:%lld", p->u.i);
  }else if( p->flags & MEM_Int ){
    fprintf(out, " i:%lld", p->u.i);
#ifndef SQLITE_OMIT_FLOATING_POINT
  }else if( p->flags & MEM_Real ){
    fprintf(out, " r:%g", p->r);
#endif
  }else if( p->flags & MEM_RowSet ){
    fprintf(out, " (rowset)");
  }else{
    char zBuf[200];
    sqlite3VdbeMemPrettyPrint(p, zBuf);
    fprintf(out, " ");
    fprintf(out, "%s", zBuf);
  }
}
static void registerTrace(FILE *out, int iReg, Mem *p){
  fprintf(out, "REG[%d] = ", iReg);
  memTracePrint(out, p);
  fprintf(out, "\n");
}
#endif

#ifdef SQLITE_DEBUG
#  define REGISTER_TRACE(R,M) if(p->trace)registerTrace(p->trace,R,M)
#else
#  define REGISTER_TRACE(R,M)
#endif


#ifdef VDBE_PROFILE

/* 
** hwtime.h contains inline assembler code for implementing 
** high-performance timing routines.
*/
/************** Include hwtime.h in the middle of vdbe.c *********************/
/************** Begin file hwtime.h ******************************************/
/*
** 2008 May 27
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains inline asm code for retrieving "high-performance"
** counters for x86 class CPUs.
*/
#ifndef _HWTIME_H_
#define _HWTIME_H_

/*
** The following routine only works on pentium-class (or newer) processors.
** It uses the RDTSC opcode to read the cycle count value out of the
** processor and returns that value.  This can be used for high-res
** profiling.
*/
#if (defined(__GNUC__) || defined(_MSC_VER)) && \
      (defined(i386) || defined(__i386__) || defined(_M_IX86))

  #if defined(__GNUC__)

  __inline__ sqlite_uint64 sqlite3Hwtime(void){
     unsigned int lo, hi;
     __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
     return (sqlite_uint64)hi << 32 | lo;
  }

  #elif defined(_MSC_VER)

  __declspec(naked) __inline sqlite_uint64 __cdecl sqlite3Hwtime(void){
     __asm {
        rdtsc
        ret       ; return value at EDX:EAX
     }
  }

  #endif

#elif (defined(__GNUC__) && defined(__x86_64__))

  __inline__ sqlite_uint64 sqlite3Hwtime(void){
      unsigned long val;
      __asm__ __volatile__ ("rdtsc" : "=A" (val));
      return val;
  }
 
#elif (defined(__GNUC__) && defined(__ppc__))

  __inline__ sqlite_uint64 sqlite3Hwtime(void){
      unsigned long long retval;
      unsigned long junk;
      __asm__ __volatile__ ("\n\
          1:      mftbu   %1\n\
                  mftb    %L0\n\
                  mftbu   %0\n\
                  cmpw    %0,%1\n\
                  bne     1b"
                  : "=r" (retval), "=r" (junk));
      return retval;
  }

#else

  #error Need implementation of sqlite3Hwtime() for your platform.

  /*
  ** To compile without implementing sqlite3Hwtime() for your platform,
  ** you can remove the above #error and use the following
  ** stub function.  You will lose timing support for many
  ** of the debugging and testing utilities, but it should at
  ** least compile and run.
  */
SQLITE_PRIVATE   sqlite_uint64 sqlite3Hwtime(void){ return ((sqlite_uint64)0); }

#endif

#endif /* !defined(_HWTIME_H_) */

/************** End of hwtime.h **********************************************/
/************** Continuing where we left off in vdbe.c ***********************/

#endif

/*
** The CHECK_FOR_INTERRUPT macro defined here looks to see if the
** sqlite3_interrupt() routine has been called.  If it has been, then
** processing of the VDBE program is interrupted.
**
** This macro added to every instruction that does a jump in order to
** implement a loop.  This test used to be on every single instruction,
** but that meant we more testing that we needed.  By only testing the
** flag on jump instructions, we get a (small) speed improvement.
*/
#define CHECK_FOR_INTERRUPT \
   if( db->u1.isInterrupted ) goto abort_due_to_interrupt;


#ifndef NDEBUG
/*
** This function is only called from within an assert() expression. It
** checks that the sqlite3.nTransaction variable is correctly set to
** the number of non-transaction savepoints currently in the 
** linked list starting at sqlite3.pSavepoint.
** 
** Usage:
**
**     assert( checkSavepointCount(db) );
*/
static int checkSavepointCount(sqlite3 *db){
  int n = 0;
  Savepoint *p;
  for(p=db->pSavepoint; p; p=p->pNext) n++;
  assert( n==(db->nSavepoint + db->isTransactionSavepoint) );
  return 1;
}
#endif

/*
** Transfer error message text from an sqlite3_vtab.zErrMsg (text stored
** in memory obtained from sqlite3_malloc) into a Vdbe.zErrMsg (text stored
** in memory obtained from sqlite3DbMalloc).
*/
static void importVtabErrMsg(Vdbe *p, sqlite3_vtab *pVtab){
  sqlite3 *db = p->db;
  sqlite3DbFree(db, p->zErrMsg);
  p->zErrMsg = sqlite3DbStrDup(db, pVtab->zErrMsg);
  sqlite3_free(pVtab->zErrMsg);
  pVtab->zErrMsg = 0;
}


/*
** Execute as much of a VDBE program as we can then return.
**
** sqlite3VdbeMakeReady() must be called before this routine in order to
** close the program with a final OP_Halt and to set up the callbacks
** and the error message pointer.
**
** Whenever a row or result data is available, this routine will either
** invoke the result callback (if there is one) or return with
** SQLITE_ROW.
**
** If an attempt is made to open a locked database, then this routine
** will either invoke the busy callback (if there is one) or it will
** return SQLITE_BUSY.
**
** If an error occurs, an error message is written to memory obtained
** from sqlite3_malloc() and p->zErrMsg is made to point to that memory.
** The error code is stored in p->rc and this routine returns SQLITE_ERROR.
**
** If the callback ever returns non-zero, then the program exits
** immediately.  There will be no error message but the p->rc field is
** set to SQLITE_ABORT and this routine will return SQLITE_ERROR.
**
** A memory allocation error causes p->rc to be set to SQLITE_NOMEM and this
** routine to return SQLITE_ERROR.
**
** Other fatal errors return SQLITE_ERROR.
**
** After this routine has finished, sqlite3VdbeFinalize() should be
** used to clean up the mess that was left behind.
*/
SQLITE_PRIVATE int sqlite3VdbeExec(
  Vdbe *p                    /* The VDBE */
){
  int pc=0;                  /* The program counter */
  Op *aOp = p->aOp;          /* Copy of p->aOp */
  Op *pOp;                   /* Current operation */
  int rc = SQLITE_OK;        /* Value to return */
  sqlite3 *db = p->db;       /* The database */
  u8 resetSchemaOnFault = 0; /* Reset schema after an error if positive */
  u8 encoding = ENC(db);     /* The database encoding */
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  int checkProgress;         /* True if progress callbacks are enabled */
  int nProgressOps = 0;      /* Opcodes executed since progress callback. */
#endif
  Mem *aMem = p->aMem;       /* Copy of p->aMem */
  Mem *pIn1 = 0;             /* 1st input operand */
  Mem *pIn2 = 0;             /* 2nd input operand */
  Mem *pIn3 = 0;             /* 3rd input operand */
  Mem *pOut = 0;             /* Output operand */
  int iCompare = 0;          /* Result of last OP_Compare operation */
  int *aPermute = 0;         /* Permutation of columns for OP_Compare */
  i64 lastRowid = db->lastRowid;  /* Saved value of the last insert ROWID */
#ifdef VDBE_PROFILE
  u64 start;                 /* CPU clock count at start of opcode */
  int origPc;                /* Program counter at start of opcode */
#endif
  /********************************************************************
  ** Automatically generated code
  **
  ** The following union is automatically generated by the
  ** vdbe-compress.tcl script.  The purpose of this union is to
  ** reduce the amount of stack space required by this function.
  ** See comments in the vdbe-compress.tcl script for details.
  */
  union vdbeExecUnion {
    struct OP_Yield_stack_vars {
      int pcDest;
    } aa;
    struct OP_Variable_stack_vars {
      Mem *pVar;       /* Value being transferred */
    } ab;
    struct OP_Move_stack_vars {
      char *zMalloc;   /* Holding variable for allocated memory */
      int n;           /* Number of registers left to copy */
      int p1;          /* Register to copy from */
      int p2;          /* Register to copy to */
    } ac;
    struct OP_ResultRow_stack_vars {
      Mem *pMem;
      int i;
    } ad;
    struct OP_Concat_stack_vars {
      i64 nByte;
    } ae;
    struct OP_Remainder_stack_vars {
      int flags;      /* Combined MEM_* flags from both inputs */
      i64 iA;         /* Integer value of left operand */
      i64 iB;         /* Integer value of right operand */
      double rA;      /* Real value of left operand */
      double rB;      /* Real value of right operand */
    } af;
    struct OP_Function_stack_vars {
      int i;
      Mem *pArg;
      sqlite3_context ctx;
      sqlite3_value **apVal;
      int n;
    } ag;
    struct OP_ShiftRight_stack_vars {
      i64 iA;
      u64 uA;
      i64 iB;
      u8 op;
    } ah;
    struct OP_Ge_stack_vars {
      int res;            /* Result of the comparison of pIn1 against pIn3 */
      char affinity;      /* Affinity to use for comparison */
      u16 flags1;         /* Copy of initial value of pIn1->flags */
      u16 flags3;         /* Copy of initial value of pIn3->flags */
    } ai;
    struct OP_Compare_stack_vars {
      int n;
      int i;
      int p1;
      int p2;
      const KeyInfo *pKeyInfo;
      int idx;
      CollSeq *pColl;    /* Collating sequence to use on this term */
      int bRev;          /* True for DESCENDING sort order */
    } aj;
    struct OP_Or_stack_vars {
      int v1;    /* Left operand:  0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */
      int v2;    /* Right operand: 0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */
    } ak;
    struct OP_IfNot_stack_vars {
      int c;
    } al;
    struct OP_Column_stack_vars {
      u32 payloadSize;   /* Number of bytes in the record */
      i64 payloadSize64; /* Number of bytes in the record */
      int p1;            /* P1 value of the opcode */
      int p2;            /* column number to retrieve */
      VdbeCursor *pC;    /* The VDBE cursor */
      char *zRec;        /* Pointer to complete record-data */
      BtCursor *pCrsr;   /* The BTree cursor */
      u32 *aType;        /* aType[i] holds the numeric type of the i-th column */
      u32 *aOffset;      /* aOffset[i] is offset to start of data for i-th column */
      int nField;        /* number of fields in the record */
      int len;           /* The length of the serialized data for the column */
      int i;             /* Loop counter */
      char *zData;       /* Part of the record being decoded */
      Mem *pDest;        /* Where to write the extracted value */
      Mem sMem;          /* For storing the record being decoded */
      u8 *zIdx;          /* Index into header */
      u8 *zEndHdr;       /* Pointer to first byte after the header */
      u32 offset;        /* Offset into the data */
      u32 szField;       /* Number of bytes in the content of a field */
      int szHdr;         /* Size of the header size field at start of record */
      int avail;         /* Number of bytes of available data */
      Mem *pReg;         /* PseudoTable input register */
    } am;
    struct OP_Affinity_stack_vars {
      const char *zAffinity;   /* The affinity to be applied */
      char cAff;               /* A single character of affinity */
    } an;
    struct OP_MakeRecord_stack_vars {
      u8 *zNewRecord;        /* A buffer to hold the data for the new record */
      Mem *pRec;             /* The new record */
      u64 nData;             /* Number of bytes of data space */
      int nHdr;              /* Number of bytes of header space */
      i64 nByte;             /* Data space required for this record */
      int nZero;             /* Number of zero bytes at the end of the record */
      int nVarint;           /* Number of bytes in a varint */
      u32 serial_type;       /* Type field */
      Mem *pData0;           /* First field to be combined into the record */
      Mem *pLast;            /* Last field of the record */
      int nField;            /* Number of fields in the record */
      char *zAffinity;       /* The affinity string for the record */
      int file_format;       /* File format to use for encoding */
      int i;                 /* Space used in zNewRecord[] */
      int len;               /* Length of a field */
    } ao;
    struct OP_Count_stack_vars {
      i64 nEntry;
      BtCursor *pCrsr;
    } ap;
    struct OP_Savepoint_stack_vars {
      int p1;                         /* Value of P1 operand */
      char *zName;                    /* Name of savepoint */
      int nName;
      Savepoint *pNew;
      Savepoint *pSavepoint;
      Savepoint *pTmp;
      int iSavepoint;
      int ii;
    } aq;
    struct OP_AutoCommit_stack_vars {
      int desiredAutoCommit;
      int iRollback;
      int turnOnAC;
    } ar;
    struct OP_Transaction_stack_vars {
      Btree *pBt;
    } as;
    struct OP_ReadCookie_stack_vars {
      int iMeta;
      int iDb;
      int iCookie;
    } at;
    struct OP_SetCookie_stack_vars {
      Db *pDb;
    } au;
    struct OP_VerifyCookie_stack_vars {
      int iMeta;
      int iGen;
      Btree *pBt;
    } av;
    struct OP_OpenWrite_stack_vars {
      int nField;
      KeyInfo *pKeyInfo;
      int p2;
      int iDb;
      int wrFlag;
      Btree *pX;
      VdbeCursor *pCur;
      Db *pDb;
    } aw;
    struct OP_OpenEphemeral_stack_vars {
      VdbeCursor *pCx;
    } ax;
    struct OP_OpenPseudo_stack_vars {
      VdbeCursor *pCx;
    } ay;
    struct OP_SeekGt_stack_vars {
      int res;
      int oc;
      VdbeCursor *pC;
      UnpackedRecord r;
      int nField;
      i64 iKey;      /* The rowid we are to seek to */
    } az;
    struct OP_Seek_stack_vars {
      VdbeCursor *pC;
    } ba;
    struct OP_Found_stack_vars {
      int alreadyExists;
      VdbeCursor *pC;
      int res;
      UnpackedRecord *pIdxKey;
      UnpackedRecord r;
      char aTempRec[ROUND8(sizeof(UnpackedRecord)) + sizeof(Mem)*3 + 7];
    } bb;
    struct OP_IsUnique_stack_vars {
      u16 ii;
      VdbeCursor *pCx;
      BtCursor *pCrsr;
      u16 nField;
      Mem *aMx;
      UnpackedRecord r;                  /* B-Tree index search key */
      i64 R;                             /* Rowid stored in register P3 */
    } bc;
    struct OP_NotExists_stack_vars {
      VdbeCursor *pC;
      BtCursor *pCrsr;
      int res;
      u64 iKey;
    } bd;
    struct OP_NewRowid_stack_vars {
      i64 v;                 /* The new rowid */
      VdbeCursor *pC;        /* Cursor of table to get the new rowid */
      int res;               /* Result of an sqlite3BtreeLast() */
      int cnt;               /* Counter to limit the number of searches */
      Mem *pMem;             /* Register holding largest rowid for AUTOINCREMENT */
      VdbeFrame *pFrame;     /* Root frame of VDBE */
    } be;
    struct OP_InsertInt_stack_vars {
      Mem *pData;       /* MEM cell holding data for the record to be inserted */
      Mem *pKey;        /* MEM cell holding key  for the record */
      i64 iKey;         /* The integer ROWID or key for the record to be inserted */
      VdbeCursor *pC;   /* Cursor to table into which insert is written */
      int nZero;        /* Number of zero-bytes to append */
      int seekResult;   /* Result of prior seek or 0 if no USESEEKRESULT flag */
      const char *zDb;  /* database name - used by the update hook */
      const char *zTbl; /* Table name - used by the opdate hook */
      int op;           /* Opcode for update hook: SQLITE_UPDATE or SQLITE_INSERT */
    } bf;
    struct OP_Delete_stack_vars {
      i64 iKey;
      VdbeCursor *pC;
    } bg;
    struct OP_RowData_stack_vars {
      VdbeCursor *pC;
      BtCursor *pCrsr;
      u32 n;
      i64 n64;
    } bh;
    struct OP_Rowid_stack_vars {
      VdbeCursor *pC;
      i64 v;
      sqlite3_vtab *pVtab;
      const sqlite3_module *pModule;
    } bi;
    struct OP_NullRow_stack_vars {
      VdbeCursor *pC;
    } bj;
    struct OP_Last_stack_vars {
      VdbeCursor *pC;
      BtCursor *pCrsr;
      int res;
    } bk;
    struct OP_Rewind_stack_vars {
      VdbeCursor *pC;
      BtCursor *pCrsr;
      int res;
    } bl;
    struct OP_Next_stack_vars {
      VdbeCursor *pC;
      BtCursor *pCrsr;
      int res;
    } bm;
    struct OP_IdxInsert_stack_vars {
      VdbeCursor *pC;
      BtCursor *pCrsr;
      int nKey;
      const char *zKey;
    } bn;
    struct OP_IdxDelete_stack_vars {
      VdbeCursor *pC;
      BtCursor *pCrsr;
      int res;
      UnpackedRecord r;
    } bo;
    struct OP_IdxRowid_stack_vars {
      BtCursor *pCrsr;
      VdbeCursor *pC;
      i64 rowid;
    } bp;
    struct OP_IdxGE_stack_vars {
      VdbeCursor *pC;
      int res;
      UnpackedRecord r;
    } bq;
    struct OP_Destroy_stack_vars {
      int iMoved;
      int iCnt;
      Vdbe *pVdbe;
      int iDb;
    } br;
    struct OP_Clear_stack_vars {
      int nChange;
    } bs;
    struct OP_CreateTable_stack_vars {
      int pgno;
      int flags;
      Db *pDb;
    } bt;
    struct OP_ParseSchema_stack_vars {
      int iDb;
      const char *zMaster;
      char *zSql;
      InitData initData;
    } bu;
    struct OP_IntegrityCk_stack_vars {
      int nRoot;      /* Number of tables to check.  (Number of root pages.) */
      int *aRoot;     /* Array of rootpage numbers for tables to be checked */
      int j;          /* Loop counter */
      int nErr;       /* Number of errors reported */
      char *z;        /* Text of the error report */
      Mem *pnErr;     /* Register keeping track of errors remaining */
    } bv;
    struct OP_RowSetRead_stack_vars {
      i64 val;
    } bw;
    struct OP_RowSetTest_stack_vars {
      int iSet;
      int exists;
    } bx;
    struct OP_Program_stack_vars {
      int nMem;               /* Number of memory registers for sub-program */
      int nByte;              /* Bytes of runtime space required for sub-program */
      Mem *pRt;               /* Register to allocate runtime space */
      Mem *pMem;              /* Used to iterate through memory cells */
      Mem *pEnd;              /* Last memory cell in new array */
      VdbeFrame *pFrame;      /* New vdbe frame to execute in */
      SubProgram *pProgram;   /* Sub-program to execute */
      void *t;                /* Token identifying trigger */
    } by;
    struct OP_Param_stack_vars {
      VdbeFrame *pFrame;
      Mem *pIn;
    } bz;
    struct OP_MemMax_stack_vars {
      Mem *pIn1;
      VdbeFrame *pFrame;
    } ca;
    struct OP_AggStep_stack_vars {
      int n;
      int i;
      Mem *pMem;
      Mem *pRec;
      sqlite3_context ctx;
      sqlite3_value **apVal;
    } cb;
    struct OP_AggFinal_stack_vars {
      Mem *pMem;
    } cc;
    struct OP_Checkpoint_stack_vars {
      int i;                          /* Loop counter */
      int aRes[3];                    /* Results */
      Mem *pMem;                      /* Write results here */
    } cd;
    struct OP_JournalMode_stack_vars {
      Btree *pBt;                     /* Btree to change journal mode of */
      Pager *pPager;                  /* Pager associated with pBt */
      int eNew;                       /* New journal mode */
      int eOld;                       /* The old journal mode */
      const char *zFilename;          /* Name of database file for pPager */
    } ce;
    struct OP_IncrVacuum_stack_vars {
      Btree *pBt;
    } cf;
    struct OP_VBegin_stack_vars {
      VTable *pVTab;
    } cg;
    struct OP_VOpen_stack_vars {
      VdbeCursor *pCur;
      sqlite3_vtab_cursor *pVtabCursor;
      sqlite3_vtab *pVtab;
      sqlite3_module *pModule;
    } ch;
    struct OP_VFilter_stack_vars {
      int nArg;
      int iQuery;
      const sqlite3_module *pModule;
      Mem *pQuery;
      Mem *pArgc;
      sqlite3_vtab_cursor *pVtabCursor;
      sqlite3_vtab *pVtab;
      VdbeCursor *pCur;
      int res;
      int i;
      Mem **apArg;
    } ci;
    struct OP_VColumn_stack_vars {
      sqlite3_vtab *pVtab;
      const sqlite3_module *pModule;
      Mem *pDest;
      sqlite3_context sContext;
    } cj;
    struct OP_VNext_stack_vars {
      sqlite3_vtab *pVtab;
      const sqlite3_module *pModule;
      int res;
      VdbeCursor *pCur;
    } ck;
    struct OP_VRename_stack_vars {
      sqlite3_vtab *pVtab;
      Mem *pName;
    } cl;
    struct OP_VUpdate_stack_vars {
      sqlite3_vtab *pVtab;
      sqlite3_module *pModule;
      int nArg;
      int i;
      sqlite_int64 rowid;
      Mem **apArg;
      Mem *pX;
    } cm;
    struct OP_Trace_stack_vars {
      char *zTrace;
      char *z;
    } cn;
  } u;
  /* End automatically generated code
  ********************************************************************/

  assert( p->magic==VDBE_MAGIC_RUN );  /* sqlite3_step() verifies this */
  sqlite3VdbeEnter(p);
  if( p->rc==SQLITE_NOMEM ){
    /* This happens if a malloc() inside a call to sqlite3_column_text() or
    ** sqlite3_column_text16() failed.  */
    goto no_mem;
  }
  assert( p->rc==SQLITE_OK || p->rc==SQLITE_BUSY );
  p->rc = SQLITE_OK;
  assert( p->explain==0 );
  p->pResultSet = 0;
  db->busyHandler.nBusy = 0;
  CHECK_FOR_INTERRUPT;
  sqlite3VdbeIOTraceSql(p);
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  checkProgress = db->xProgress!=0;
#endif
#ifdef SQLITE_DEBUG
  sqlite3BeginBenignMalloc();
  if( p->pc==0  && (p->db->flags & SQLITE_VdbeListing)!=0 ){
    int i;
    printf("VDBE Program Listing:\n");
    sqlite3VdbePrintSql(p);
    for(i=0; i<p->nOp; i++){
      sqlite3VdbePrintOp(stdout, i, &aOp[i]);
    }
  }
  sqlite3EndBenignMalloc();
#endif
  for(pc=p->pc; rc==SQLITE_OK; pc++){
    assert( pc>=0 && pc<p->nOp );
    if( db->mallocFailed ) goto no_mem;
#ifdef VDBE_PROFILE
    origPc = pc;
    start = sqlite3Hwtime();
#endif
    pOp = &aOp[pc];

    /* Only allow tracing if SQLITE_DEBUG is defined.
    */
#ifdef SQLITE_DEBUG
    if( p->trace ){
      if( pc==0 ){
        printf("VDBE Execution Trace:\n");
        sqlite3VdbePrintSql(p);
      }
      sqlite3VdbePrintOp(p->trace, pc, pOp);
    }
#endif
      

    /* Check to see if we need to simulate an interrupt.  This only happens
    ** if we have a special test build.
    */
#ifdef SQLITE_TEST
    if( sqlite3_interrupt_count>0 ){
      sqlite3_interrupt_count--;
      if( sqlite3_interrupt_count==0 ){
        sqlite3_interrupt(db);
      }
    }
#endif

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
    /* Call the progress callback if it is configured and the required number
    ** of VDBE ops have been executed (either since this invocation of
    ** sqlite3VdbeExec() or since last time the progress callback was called).
    ** If the progress callback returns non-zero, exit the virtual machine with
    ** a return code SQLITE_ABORT.
    */
    if( checkProgress ){
      if( db->nProgressOps==nProgressOps ){
        int prc;
        prc = db->xProgress(db->pProgressArg);
        if( prc!=0 ){
          rc = SQLITE_INTERRUPT;
          goto vdbe_error_halt;
        }
        nProgressOps = 0;
      }
      nProgressOps++;
    }
#endif

    /* On any opcode with the "out2-prerelase" tag, free any
    ** external allocations out of mem[p2] and set mem[p2] to be
    ** an undefined integer.  Opcodes will either fill in the integer
    ** value or convert mem[p2] to a different type.
    */
    assert( pOp->opflags==sqlite3OpcodeProperty[pOp->opcode] );
    if( pOp->opflags & OPFLG_OUT2_PRERELEASE ){
      assert( pOp->p2>0 );
      assert( pOp->p2<=p->nMem );
      pOut = &aMem[pOp->p2];
      memAboutToChange(p, pOut);
      sqlite3VdbeMemReleaseExternal(pOut);
      pOut->flags = MEM_Int;
    }

    /* Sanity checking on other operands */
#ifdef SQLITE_DEBUG
    if( (pOp->opflags & OPFLG_IN1)!=0 ){
      assert( pOp->p1>0 );
      assert( pOp->p1<=p->nMem );
      assert( memIsValid(&aMem[pOp->p1]) );
      REGISTER_TRACE(pOp->p1, &aMem[pOp->p1]);
    }
    if( (pOp->opflags & OPFLG_IN2)!=0 ){
      assert( pOp->p2>0 );
      assert( pOp->p2<=p->nMem );
      assert( memIsValid(&aMem[pOp->p2]) );
      REGISTER_TRACE(pOp->p2, &aMem[pOp->p2]);
    }
    if( (pOp->opflags & OPFLG_IN3)!=0 ){
      assert( pOp->p3>0 );
      assert( pOp->p3<=p->nMem );
      assert( memIsValid(&aMem[pOp->p3]) );
      REGISTER_TRACE(pOp->p3, &aMem[pOp->p3]);
    }
    if( (pOp->opflags & OPFLG_OUT2)!=0 ){
      assert( pOp->p2>0 );
      assert( pOp->p2<=p->nMem );
      memAboutToChange(p, &aMem[pOp->p2]);
    }
    if( (pOp->opflags & OPFLG_OUT3)!=0 ){
      assert( pOp->p3>0 );
      assert( pOp->p3<=p->nMem );
      memAboutToChange(p, &aMem[pOp->p3]);
    }
#endif
  
    switch( pOp->opcode ){

/*****************************************************************************
** What follows is a massive switch statement where each case implements a
** separate instruction in the virtual machine.  If we follow the usual
** indentation conventions, each case should be indented by 6 spaces.  But
** that is a lot of wasted space on the left margin.  So the code within
** the switch statement will break with convention and be flush-left. Another
** big comment (similar to this one) will mark the point in the code where
** we transition back to normal indentation.
**
** The formatting of each case is important.  The makefile for SQLite
** generates two C files "opcodes.h" and "opcodes.c" by scanning this
** file looking for lines that begin with "case OP_".  The opcodes.h files
** will be filled with #defines that give unique integer values to each
** opcode and the opcodes.c file is filled with an array of strings where
** each string is the symbolic name for the corresponding opcode.  If the
** case statement is followed by a comment of the form "/# same as ... #/"
** that comment is used to determine the particular value of the opcode.
**
** Other keywords in the comment that follows each case are used to
** construct the OPFLG_INITIALIZER value that initializes opcodeProperty[].
** Keywords include: in1, in2, in3, out2_prerelease, out2, out3.  See
** the mkopcodeh.awk script for additional information.
**
** Documentation about VDBE opcodes is generated by scanning this file
** for lines of that contain "Opcode:".  That line and all subsequent
** comment lines are used in the generation of the opcode.html documentation
** file.
**
** SUMMARY:
**
**     Formatting is important to scripts that scan this file.
**     Do not deviate from the formatting style currently in use.
**
*****************************************************************************/

/* Opcode:  Goto * P2 * * *
**
** An unconditional jump to address P2.
** The next instruction executed will be 
** the one at index P2 from the beginning of
** the program.
*/
case OP_Goto: {             /* jump */
  CHECK_FOR_INTERRUPT;
  pc = pOp->p2 - 1;
  break;
}

/* Opcode:  Gosub P1 P2 * * *
**
** Write the current address onto register P1
** and then jump to address P2.
*/
case OP_Gosub: {            /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  assert( (pIn1->flags & MEM_Dyn)==0 );
  memAboutToChange(p, pIn1);
  pIn1->flags = MEM_Int;
  pIn1->u.i = pc;
  REGISTER_TRACE(pOp->p1, pIn1);
  pc = pOp->p2 - 1;
  break;
}

/* Opcode:  Return P1 * * * *
**
** Jump to the next instruction after the address in register P1.
*/
case OP_Return: {           /* in1 */
  pIn1 = &aMem[pOp->p1];
  assert( pIn1->flags & MEM_Int );
  pc = (int)pIn1->u.i;
  break;
}

/* Opcode:  Yield P1 * * * *
**
** Swap the program counter with the value in register P1.
*/
case OP_Yield: {            /* in1 */
#if 0  /* local variables moved into u.aa */
  int pcDest;
#endif /* local variables moved into u.aa */
  pIn1 = &aMem[pOp->p1];
  assert( (pIn1->flags & MEM_Dyn)==0 );
  pIn1->flags = MEM_Int;
  u.aa.pcDest = (int)pIn1->u.i;
  pIn1->u.i = pc;
  REGISTER_TRACE(pOp->p1, pIn1);
  pc = u.aa.pcDest;
  break;
}

/* Opcode:  HaltIfNull  P1 P2 P3 P4 *
**
** Check the value in register P3.  If it is NULL then Halt using
** parameter P1, P2, and P4 as if this were a Halt instruction.  If the
** value in register P3 is not NULL, then this routine is a no-op.
*/
case OP_HaltIfNull: {      /* in3 */
  pIn3 = &aMem[pOp->p3];
  if( (pIn3->flags & MEM_Null)==0 ) break;
  /* Fall through into OP_Halt */
}

/* Opcode:  Halt P1 P2 * P4 *
**
** Exit immediately.  All open cursors, etc are closed
** automatically.
**
** P1 is the result code returned by sqlite3_exec(), sqlite3_reset(),
** or sqlite3_finalize().  For a normal halt, this should be SQLITE_OK (0).
** For errors, it can be some other value.  If P1!=0 then P2 will determine
** whether or not to rollback the current transaction.  Do not rollback
** if P2==OE_Fail. Do the rollback if P2==OE_Rollback.  If P2==OE_Abort,
** then back out all changes that have occurred during this execution of the
** VDBE, but do not rollback the transaction. 
**
** If P4 is not null then it is an error message string.
**
** There is an implied "Halt 0 0 0" instruction inserted at the very end of
** every program.  So a jump past the last instruction of the program
** is the same as executing Halt.
*/
case OP_Halt: {
  if( pOp->p1==SQLITE_OK && p->pFrame ){
    /* Halt the sub-program. Return control to the parent frame. */
    VdbeFrame *pFrame = p->pFrame;
    p->pFrame = pFrame->pParent;
    p->nFrame--;
    sqlite3VdbeSetChanges(db, p->nChange);
    pc = sqlite3VdbeFrameRestore(pFrame);
    lastRowid = db->lastRowid;
    if( pOp->p2==OE_Ignore ){
      /* Instruction pc is the OP_Program that invoked the sub-program 
      ** currently being halted. If the p2 instruction of this OP_Halt
      ** instruction is set to OE_Ignore, then the sub-program is throwing
      ** an IGNORE exception. In this case jump to the address specified
      ** as the p2 of the calling OP_Program.  */
      pc = p->aOp[pc].p2-1;
    }
    aOp = p->aOp;
    aMem = p->aMem;
    break;
  }

  p->rc = pOp->p1;
  p->errorAction = (u8)pOp->p2;
  p->pc = pc;
  if( pOp->p4.z ){
    assert( p->rc!=SQLITE_OK );
    sqlite3SetString(&p->zErrMsg, db, "%s", pOp->p4.z);
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(pOp->p1, "abort at %d in [%s]: %s", pc, p->zSql, pOp->p4.z);
  }else if( p->rc ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(pOp->p1, "constraint failed at %d in [%s]", pc, p->zSql);
  }
  rc = sqlite3VdbeHalt(p);
  assert( rc==SQLITE_BUSY || rc==SQLITE_OK || rc==SQLITE_ERROR );
  if( rc==SQLITE_BUSY ){
    p->rc = rc = SQLITE_BUSY;
  }else{
    assert( rc==SQLITE_OK || p->rc==SQLITE_CONSTRAINT );
    assert( rc==SQLITE_OK || db->nDeferredCons>0 );
    rc = p->rc ? SQLITE_ERROR : SQLITE_DONE;
  }
  goto vdbe_return;
}

/* Opcode: Integer P1 P2 * * *
**
** The 32-bit integer value P1 is written into register P2.
*/
case OP_Integer: {         /* out2-prerelease */
  pOut->u.i = pOp->p1;
  break;
}

/* Opcode: Int64 * P2 * P4 *
**
** P4 is a pointer to a 64-bit integer value.
** Write that value into register P2.
*/
case OP_Int64: {           /* out2-prerelease */
  assert( pOp->p4.pI64!=0 );
  pOut->u.i = *pOp->p4.pI64;
  break;
}

#ifndef SQLITE_OMIT_FLOATING_POINT
/* Opcode: Real * P2 * P4 *
**
** P4 is a pointer to a 64-bit floating point value.
** Write that value into register P2.
*/
case OP_Real: {            /* same as TK_FLOAT, out2-prerelease */
  pOut->flags = MEM_Real;
  assert( !sqlite3IsNaN(*pOp->p4.pReal) );
  pOut->r = *pOp->p4.pReal;
  break;
}
#endif

/* Opcode: String8 * P2 * P4 *
**
** P4 points to a nul terminated UTF-8 string. This opcode is transformed 
** into an OP_String before it is executed for the first time.
*/
case OP_String8: {         /* same as TK_STRING, out2-prerelease */
  assert( pOp->p4.z!=0 );
  pOp->opcode = OP_String;
  pOp->p1 = sqlite3Strlen30(pOp->p4.z);

#ifndef SQLITE_OMIT_UTF16
  if( encoding!=SQLITE_UTF8 ){
    rc = sqlite3VdbeMemSetStr(pOut, pOp->p4.z, -1, SQLITE_UTF8, SQLITE_STATIC);
    if( rc==SQLITE_TOOBIG ) goto too_big;
    if( SQLITE_OK!=sqlite3VdbeChangeEncoding(pOut, encoding) ) goto no_mem;
    assert( pOut->zMalloc==pOut->z );
    assert( pOut->flags & MEM_Dyn );
    pOut->zMalloc = 0;
    pOut->flags |= MEM_Static;
    pOut->flags &= ~MEM_Dyn;
    if( pOp->p4type==P4_DYNAMIC ){
      sqlite3DbFree(db, pOp->p4.z);
    }
    pOp->p4type = P4_DYNAMIC;
    pOp->p4.z = pOut->z;
    pOp->p1 = pOut->n;
  }
#endif
  if( pOp->p1>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    goto too_big;
  }
  /* Fall through to the next case, OP_String */
}
  
/* Opcode: String P1 P2 * P4 *
**
** The string value P4 of length P1 (bytes) is stored in register P2.
*/
case OP_String: {          /* out2-prerelease */
  assert( pOp->p4.z!=0 );
  pOut->flags = MEM_Str|MEM_Static|MEM_Term;
  pOut->z = pOp->p4.z;
  pOut->n = pOp->p1;
  pOut->enc = encoding;
  UPDATE_MAX_BLOBSIZE(pOut);
  break;
}

/* Opcode: Null * P2 * * *
**
** Write a NULL into register P2.
*/
case OP_Null: {           /* out2-prerelease */
  pOut->flags = MEM_Null;
  break;
}


/* Opcode: Blob P1 P2 * P4
**
** P4 points to a blob of data P1 bytes long.  Store this
** blob in register P2.
*/
case OP_Blob: {                /* out2-prerelease */
  assert( pOp->p1 <= SQLITE_MAX_LENGTH );
  sqlite3VdbeMemSetStr(pOut, pOp->p4.z, pOp->p1, 0, 0);
  pOut->enc = encoding;
  UPDATE_MAX_BLOBSIZE(pOut);
  break;
}

/* Opcode: Variable P1 P2 * P4 *
**
** Transfer the values of bound parameter P1 into register P2
**
** If the parameter is named, then its name appears in P4 and P3==1.
** The P4 value is used by sqlite3_bind_parameter_name().
*/
case OP_Variable: {            /* out2-prerelease */
#if 0  /* local variables moved into u.ab */
  Mem *pVar;       /* Value being transferred */
#endif /* local variables moved into u.ab */

  assert( pOp->p1>0 && pOp->p1<=p->nVar );
  assert( pOp->p4.z==0 || pOp->p4.z==p->azVar[pOp->p1-1] );
  u.ab.pVar = &p->aVar[pOp->p1 - 1];
  if( sqlite3VdbeMemTooBig(u.ab.pVar) ){
    goto too_big;
  }
  sqlite3VdbeMemShallowCopy(pOut, u.ab.pVar, MEM_Static);
  UPDATE_MAX_BLOBSIZE(pOut);
  break;
}

/* Opcode: Move P1 P2 P3 * *
**
** Move the values in register P1..P1+P3-1 over into
** registers P2..P2+P3-1.  Registers P1..P1+P1-1 are
** left holding a NULL.  It is an error for register ranges
** P1..P1+P3-1 and P2..P2+P3-1 to overlap.
*/
case OP_Move: {
#if 0  /* local variables moved into u.ac */
  char *zMalloc;   /* Holding variable for allocated memory */
  int n;           /* Number of registers left to copy */
  int p1;          /* Register to copy from */
  int p2;          /* Register to copy to */
#endif /* local variables moved into u.ac */

  u.ac.n = pOp->p3;
  u.ac.p1 = pOp->p1;
  u.ac.p2 = pOp->p2;
  assert( u.ac.n>0 && u.ac.p1>0 && u.ac.p2>0 );
  assert( u.ac.p1+u.ac.n<=u.ac.p2 || u.ac.p2+u.ac.n<=u.ac.p1 );

  pIn1 = &aMem[u.ac.p1];
  pOut = &aMem[u.ac.p2];
  while( u.ac.n-- ){
    assert( pOut<=&aMem[p->nMem] );
    assert( pIn1<=&aMem[p->nMem] );
    assert( memIsValid(pIn1) );
    memAboutToChange(p, pOut);
    u.ac.zMalloc = pOut->zMalloc;
    pOut->zMalloc = 0;
    sqlite3VdbeMemMove(pOut, pIn1);
    pIn1->zMalloc = u.ac.zMalloc;
    REGISTER_TRACE(u.ac.p2++, pOut);
    pIn1++;
    pOut++;
  }
  break;
}

/* Opcode: Copy P1 P2 * * *
**
** Make a copy of register P1 into register P2.
**
** This instruction makes a deep copy of the value.  A duplicate
** is made of any string or blob constant.  See also OP_SCopy.
*/
case OP_Copy: {             /* in1, out2 */
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  assert( pOut!=pIn1 );
  sqlite3VdbeMemShallowCopy(pOut, pIn1, MEM_Ephem);
  Deephemeralize(pOut);
  REGISTER_TRACE(pOp->p2, pOut);
  break;
}

/* Opcode: SCopy P1 P2 * * *
**
** Make a shallow copy of register P1 into register P2.
**
** This instruction makes a shallow copy of the value.  If the value
** is a string or blob, then the copy is only a pointer to the
** original and hence if the original changes so will the copy.
** Worse, if the original is deallocated, the copy becomes invalid.
** Thus the program must guarantee that the original will not change
** during the lifetime of the copy.  Use OP_Copy to make a complete
** copy.
*/
case OP_SCopy: {            /* in1, out2 */
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  assert( pOut!=pIn1 );
  sqlite3VdbeMemShallowCopy(pOut, pIn1, MEM_Ephem);
#ifdef SQLITE_DEBUG
  if( pOut->pScopyFrom==0 ) pOut->pScopyFrom = pIn1;
#endif
  REGISTER_TRACE(pOp->p2, pOut);
  break;
}

/* Opcode: ResultRow P1 P2 * * *
**
** The registers P1 through P1+P2-1 contain a single row of
** results. This opcode causes the sqlite3_step() call to terminate
** with an SQLITE_ROW return code and it sets up the sqlite3_stmt
** structure to provide access to the top P1 values as the result
** row.
*/
case OP_ResultRow: {
#if 0  /* local variables moved into u.ad */
  Mem *pMem;
  int i;
#endif /* local variables moved into u.ad */
  assert( p->nResColumn==pOp->p2 );
  assert( pOp->p1>0 );
  assert( pOp->p1+pOp->p2<=p->nMem+1 );

  /* If this statement has violated immediate foreign key constraints, do
  ** not return the number of rows modified. And do not RELEASE the statement
  ** transaction. It needs to be rolled back.  */
  if( SQLITE_OK!=(rc = sqlite3VdbeCheckFk(p, 0)) ){
    assert( db->flags&SQLITE_CountRows );
    assert( p->usesStmtJournal );
    break;
  }

  /* If the SQLITE_CountRows flag is set in sqlite3.flags mask, then
  ** DML statements invoke this opcode to return the number of rows
  ** modified to the user. This is the only way that a VM that
  ** opens a statement transaction may invoke this opcode.
  **
  ** In case this is such a statement, close any statement transaction
  ** opened by this VM before returning control to the user. This is to
  ** ensure that statement-transactions are always nested, not overlapping.
  ** If the open statement-transaction is not closed here, then the user
  ** may step another VM that opens its own statement transaction. This
  ** may lead to overlapping statement transactions.
  **
  ** The statement transaction is never a top-level transaction.  Hence
  ** the RELEASE call below can never fail.
  */
  assert( p->iStatement==0 || db->flags&SQLITE_CountRows );
  rc = sqlite3VdbeCloseStatement(p, SAVEPOINT_RELEASE);
  if( NEVER(rc!=SQLITE_OK) ){
    break;
  }

  /* Invalidate all ephemeral cursor row caches */
  p->cacheCtr = (p->cacheCtr + 2)|1;

  /* Make sure the results of the current row are \000 terminated
  ** and have an assigned type.  The results are de-ephemeralized as
  ** as side effect.
  */
  u.ad.pMem = p->pResultSet = &aMem[pOp->p1];
  for(u.ad.i=0; u.ad.i<pOp->p2; u.ad.i++){
    assert( memIsValid(&u.ad.pMem[u.ad.i]) );
    Deephemeralize(&u.ad.pMem[u.ad.i]);
    assert( (u.ad.pMem[u.ad.i].flags & MEM_Ephem)==0
            || (u.ad.pMem[u.ad.i].flags & (MEM_Str|MEM_Blob))==0 );
    sqlite3VdbeMemNulTerminate(&u.ad.pMem[u.ad.i]);
    sqlite3VdbeMemStoreType(&u.ad.pMem[u.ad.i]);
    REGISTER_TRACE(pOp->p1+u.ad.i, &u.ad.pMem[u.ad.i]);
  }
  if( db->mallocFailed ) goto no_mem;

  /* Return SQLITE_ROW
  */
  p->pc = pc + 1;
  rc = SQLITE_ROW;
  goto vdbe_return;
}

/* Opcode: Concat P1 P2 P3 * *
**
** Add the text in register P1 onto the end of the text in
** register P2 and store the result in register P3.
** If either the P1 or P2 text are NULL then store NULL in P3.
**
**   P3 = P2 || P1
**
** It is illegal for P1 and P3 to be the same register. Sometimes,
** if P3 is the same register as P2, the implementation is able
** to avoid a memcpy().
*/
case OP_Concat: {           /* same as TK_CONCAT, in1, in2, out3 */
#if 0  /* local variables moved into u.ae */
  i64 nByte;
#endif /* local variables moved into u.ae */

  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  pOut = &aMem[pOp->p3];
  assert( pIn1!=pOut );
  if( (pIn1->flags | pIn2->flags) & MEM_Null ){
    sqlite3VdbeMemSetNull(pOut);
    break;
  }
  if( ExpandBlob(pIn1) || ExpandBlob(pIn2) ) goto no_mem;
  Stringify(pIn1, encoding);
  Stringify(pIn2, encoding);
  u.ae.nByte = pIn1->n + pIn2->n;
  if( u.ae.nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    goto too_big;
  }
  MemSetTypeFlag(pOut, MEM_Str);
  if( sqlite3VdbeMemGrow(pOut, (int)u.ae.nByte+2, pOut==pIn2) ){
    goto no_mem;
  }
  if( pOut!=pIn2 ){
    memcpy(pOut->z, pIn2->z, pIn2->n);
  }
  memcpy(&pOut->z[pIn2->n], pIn1->z, pIn1->n);
  pOut->z[u.ae.nByte] = 0;
  pOut->z[u.ae.nByte+1] = 0;
  pOut->flags |= MEM_Term;
  pOut->n = (int)u.ae.nByte;
  pOut->enc = encoding;
  UPDATE_MAX_BLOBSIZE(pOut);
  break;
}

/* Opcode: Add P1 P2 P3 * *
**
** Add the value in register P1 to the value in register P2
** and store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: Multiply P1 P2 P3 * *
**
**
** Multiply the value in register P1 by the value in register P2
** and store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: Subtract P1 P2 P3 * *
**
** Subtract the value in register P1 from the value in register P2
** and store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: Divide P1 P2 P3 * *
**
** Divide the value in register P1 by the value in register P2
** and store the result in register P3 (P3=P2/P1). If the value in 
** register P1 is zero, then the result is NULL. If either input is 
** NULL, the result is NULL.
*/
/* Opcode: Remainder P1 P2 P3 * *
**
** Compute the remainder after integer division of the value in
** register P1 by the value in register P2 and store the result in P3. 
** If the value in register P2 is zero the result is NULL.
** If either operand is NULL, the result is NULL.
*/
case OP_Add:                   /* same as TK_PLUS, in1, in2, out3 */
case OP_Subtract:              /* same as TK_MINUS, in1, in2, out3 */
case OP_Multiply:              /* same as TK_STAR, in1, in2, out3 */
case OP_Divide:                /* same as TK_SLASH, in1, in2, out3 */
case OP_Remainder: {           /* same as TK_REM, in1, in2, out3 */
#if 0  /* local variables moved into u.af */
  int flags;      /* Combined MEM_* flags from both inputs */
  i64 iA;         /* Integer value of left operand */
  i64 iB;         /* Integer value of right operand */
  double rA;      /* Real value of left operand */
  double rB;      /* Real value of right operand */
#endif /* local variables moved into u.af */

  pIn1 = &aMem[pOp->p1];
  applyNumericAffinity(pIn1);
  pIn2 = &aMem[pOp->p2];
  applyNumericAffinity(pIn2);
  pOut = &aMem[pOp->p3];
  u.af.flags = pIn1->flags | pIn2->flags;
  if( (u.af.flags & MEM_Null)!=0 ) goto arithmetic_result_is_null;
  if( (pIn1->flags & pIn2->flags & MEM_Int)==MEM_Int ){
    u.af.iA = pIn1->u.i;
    u.af.iB = pIn2->u.i;
    switch( pOp->opcode ){
      case OP_Add:       if( sqlite3AddInt64(&u.af.iB,u.af.iA) ) goto fp_math;  break;
      case OP_Subtract:  if( sqlite3SubInt64(&u.af.iB,u.af.iA) ) goto fp_math;  break;
      case OP_Multiply:  if( sqlite3MulInt64(&u.af.iB,u.af.iA) ) goto fp_math;  break;
      case OP_Divide: {
        if( u.af.iA==0 ) goto arithmetic_result_is_null;
        if( u.af.iA==-1 && u.af.iB==SMALLEST_INT64 ) goto fp_math;
        u.af.iB /= u.af.iA;
        break;
      }
      default: {
        if( u.af.iA==0 ) goto arithmetic_result_is_null;
        if( u.af.iA==-1 ) u.af.iA = 1;
        u.af.iB %= u.af.iA;
        break;
      }
    }
    pOut->u.i = u.af.iB;
    MemSetTypeFlag(pOut, MEM_Int);
  }else{
fp_math:
    u.af.rA = sqlite3VdbeRealValue(pIn1);
    u.af.rB = sqlite3VdbeRealValue(pIn2);
    switch( pOp->opcode ){
      case OP_Add:         u.af.rB += u.af.rA;       break;
      case OP_Subtract:    u.af.rB -= u.af.rA;       break;
      case OP_Multiply:    u.af.rB *= u.af.rA;       break;
      case OP_Divide: {
        /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
        if( u.af.rA==(double)0 ) goto arithmetic_result_is_null;
        u.af.rB /= u.af.rA;
        break;
      }
      default: {
        u.af.iA = (i64)u.af.rA;
        u.af.iB = (i64)u.af.rB;
        if( u.af.iA==0 ) goto arithmetic_result_is_null;
        if( u.af.iA==-1 ) u.af.iA = 1;
        u.af.rB = (double)(u.af.iB % u.af.iA);
        break;
      }
    }
#ifdef SQLITE_OMIT_FLOATING_POINT
    pOut->u.i = u.af.rB;
    MemSetTypeFlag(pOut, MEM_Int);
#else
    if( sqlite3IsNaN(u.af.rB) ){
      goto arithmetic_result_is_null;
    }
    pOut->r = u.af.rB;
    MemSetTypeFlag(pOut, MEM_Real);
    if( (u.af.flags & MEM_Real)==0 ){
      sqlite3VdbeIntegerAffinity(pOut);
    }
#endif
  }
  break;

arithmetic_result_is_null:
  sqlite3VdbeMemSetNull(pOut);
  break;
}

/* Opcode: CollSeq * * P4
**
** P4 is a pointer to a CollSeq struct. If the next call to a user function
** or aggregate calls sqlite3GetFuncCollSeq(), this collation sequence will
** be returned. This is used by the built-in min(), max() and nullif()
** functions.
**
** The interface used by the implementation of the aforementioned functions
** to retrieve the collation sequence set by this opcode is not available
** publicly, only to user functions defined in func.c.
*/
case OP_CollSeq: {
  assert( pOp->p4type==P4_COLLSEQ );
  break;
}

/* Opcode: Function P1 P2 P3 P4 P5
**
** Invoke a user function (P4 is a pointer to a Function structure that
** defines the function) with P5 arguments taken from register P2 and
** successors.  The result of the function is stored in register P3.
** Register P3 must not be one of the function inputs.
**
** P1 is a 32-bit bitmask indicating whether or not each argument to the 
** function was determined to be constant at compile time. If the first
** argument was constant then bit 0 of P1 is set. This is used to determine
** whether meta data associated with a user function argument using the
** sqlite3_set_auxdata() API may be safely retained until the next
** invocation of this opcode.
**
** See also: AggStep and AggFinal
*/
case OP_Function: {
#if 0  /* local variables moved into u.ag */
  int i;
  Mem *pArg;
  sqlite3_context ctx;
  sqlite3_value **apVal;
  int n;
#endif /* local variables moved into u.ag */

  u.ag.n = pOp->p5;
  u.ag.apVal = p->apArg;
  assert( u.ag.apVal || u.ag.n==0 );
  assert( pOp->p3>0 && pOp->p3<=p->nMem );
  pOut = &aMem[pOp->p3];
  memAboutToChange(p, pOut);

  assert( u.ag.n==0 || (pOp->p2>0 && pOp->p2+u.ag.n<=p->nMem+1) );
  assert( pOp->p3<pOp->p2 || pOp->p3>=pOp->p2+u.ag.n );
  u.ag.pArg = &aMem[pOp->p2];
  for(u.ag.i=0; u.ag.i<u.ag.n; u.ag.i++, u.ag.pArg++){
    assert( memIsValid(u.ag.pArg) );
    u.ag.apVal[u.ag.i] = u.ag.pArg;
    Deephemeralize(u.ag.pArg);
    sqlite3VdbeMemStoreType(u.ag.pArg);
    REGISTER_TRACE(pOp->p2+u.ag.i, u.ag.pArg);
  }

  assert( pOp->p4type==P4_FUNCDEF || pOp->p4type==P4_VDBEFUNC );
  if( pOp->p4type==P4_FUNCDEF ){
    u.ag.ctx.pFunc = pOp->p4.pFunc;
    u.ag.ctx.pVdbeFunc = 0;
  }else{
    u.ag.ctx.pVdbeFunc = (VdbeFunc*)pOp->p4.pVdbeFunc;
    u.ag.ctx.pFunc = u.ag.ctx.pVdbeFunc->pFunc;
  }

  u.ag.ctx.s.flags = MEM_Null;
  u.ag.ctx.s.db = db;
  u.ag.ctx.s.xDel = 0;
  u.ag.ctx.s.zMalloc = 0;

  /* The output cell may already have a buffer allocated. Move
  ** the pointer to u.ag.ctx.s so in case the user-function can use
  ** the already allocated buffer instead of allocating a new one.
  */
  sqlite3VdbeMemMove(&u.ag.ctx.s, pOut);
  MemSetTypeFlag(&u.ag.ctx.s, MEM_Null);

  u.ag.ctx.isError = 0;
  if( u.ag.ctx.pFunc->flags & SQLITE_FUNC_NEEDCOLL ){
    assert( pOp>aOp );
    assert( pOp[-1].p4type==P4_COLLSEQ );
    assert( pOp[-1].opcode==OP_CollSeq );
    u.ag.ctx.pColl = pOp[-1].p4.pColl;
  }
  db->lastRowid = lastRowid;
  (*u.ag.ctx.pFunc->xFunc)(&u.ag.ctx, u.ag.n, u.ag.apVal); /* IMP: R-24505-23230 */
  lastRowid = db->lastRowid;

  /* If any auxiliary data functions have been called by this user function,
  ** immediately call the destructor for any non-static values.
  */
  if( u.ag.ctx.pVdbeFunc ){
    sqlite3VdbeDeleteAuxData(u.ag.ctx.pVdbeFunc, pOp->p1);
    pOp->p4.pVdbeFunc = u.ag.ctx.pVdbeFunc;
    pOp->p4type = P4_VDBEFUNC;
  }

  if( db->mallocFailed ){
    /* Even though a malloc() has failed, the implementation of the
    ** user function may have called an sqlite3_result_XXX() function
    ** to return a value. The following call releases any resources
    ** associated with such a value.
    */
    sqlite3VdbeMemRelease(&u.ag.ctx.s);
    goto no_mem;
  }

  /* If the function returned an error, throw an exception */
  if( u.ag.ctx.isError ){
    sqlite3SetString(&p->zErrMsg, db, "%s", sqlite3_value_text(&u.ag.ctx.s));
    rc = u.ag.ctx.isError;
  }

  /* Copy the result of the function into register P3 */
  sqlite3VdbeChangeEncoding(&u.ag.ctx.s, encoding);
  sqlite3VdbeMemMove(pOut, &u.ag.ctx.s);
  if( sqlite3VdbeMemTooBig(pOut) ){
    goto too_big;
  }

#if 0
  /* The app-defined function has done something that as caused this
  ** statement to expire.  (Perhaps the function called sqlite3_exec()
  ** with a CREATE TABLE statement.)
  */
  if( p->expired ) rc = SQLITE_ABORT;
#endif

  REGISTER_TRACE(pOp->p3, pOut);
  UPDATE_MAX_BLOBSIZE(pOut);
  break;
}

/* Opcode: BitAnd P1 P2 P3 * *
**
** Take the bit-wise AND of the values in register P1 and P2 and
** store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: BitOr P1 P2 P3 * *
**
** Take the bit-wise OR of the values in register P1 and P2 and
** store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: ShiftLeft P1 P2 P3 * *
**
** Shift the integer value in register P2 to the left by the
** number of bits specified by the integer in register P1.
** Store the result in register P3.
** If either input is NULL, the result is NULL.
*/
/* Opcode: ShiftRight P1 P2 P3 * *
**
** Shift the integer value in register P2 to the right by the
** number of bits specified by the integer in register P1.
** Store the result in register P3.
** If either input is NULL, the result is NULL.
*/
case OP_BitAnd:                 /* same as TK_BITAND, in1, in2, out3 */
case OP_BitOr:                  /* same as TK_BITOR, in1, in2, out3 */
case OP_ShiftLeft:              /* same as TK_LSHIFT, in1, in2, out3 */
case OP_ShiftRight: {           /* same as TK_RSHIFT, in1, in2, out3 */
#if 0  /* local variables moved into u.ah */
  i64 iA;
  u64 uA;
  i64 iB;
  u8 op;
#endif /* local variables moved into u.ah */

  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  pOut = &aMem[pOp->p3];
  if( (pIn1->flags | pIn2->flags) & MEM_Null ){
    sqlite3VdbeMemSetNull(pOut);
    break;
  }
  u.ah.iA = sqlite3VdbeIntValue(pIn2);
  u.ah.iB = sqlite3VdbeIntValue(pIn1);
  u.ah.op = pOp->opcode;
  if( u.ah.op==OP_BitAnd ){
    u.ah.iA &= u.ah.iB;
  }else if( u.ah.op==OP_BitOr ){
    u.ah.iA |= u.ah.iB;
  }else if( u.ah.iB!=0 ){
    assert( u.ah.op==OP_ShiftRight || u.ah.op==OP_ShiftLeft );

    /* If shifting by a negative amount, shift in the other direction */
    if( u.ah.iB<0 ){
      assert( OP_ShiftRight==OP_ShiftLeft+1 );
      u.ah.op = 2*OP_ShiftLeft + 1 - u.ah.op;
      u.ah.iB = u.ah.iB>(-64) ? -u.ah.iB : 64;
    }

    if( u.ah.iB>=64 ){
      u.ah.iA = (u.ah.iA>=0 || u.ah.op==OP_ShiftLeft) ? 0 : -1;
    }else{
      memcpy(&u.ah.uA, &u.ah.iA, sizeof(u.ah.uA));
      if( u.ah.op==OP_ShiftLeft ){
        u.ah.uA <<= u.ah.iB;
      }else{
        u.ah.uA >>= u.ah.iB;
        /* Sign-extend on a right shift of a negative number */
        if( u.ah.iA<0 ) u.ah.uA |= ((((u64)0xffffffff)<<32)|0xffffffff) << (64-u.ah.iB);
      }
      memcpy(&u.ah.iA, &u.ah.uA, sizeof(u.ah.iA));
    }
  }
  pOut->u.i = u.ah.iA;
  MemSetTypeFlag(pOut, MEM_Int);
  break;
}

/* Opcode: AddImm  P1 P2 * * *
** 
** Add the constant P2 to the value in register P1.
** The result is always an integer.
**
** To force any register to be an integer, just add 0.
*/
case OP_AddImm: {            /* in1 */
  pIn1 = &aMem[pOp->p1];
  memAboutToChange(p, pIn1);
  sqlite3VdbeMemIntegerify(pIn1);
  pIn1->u.i += pOp->p2;
  break;
}

/* Opcode: MustBeInt P1 P2 * * *
** 
** Force the value in register P1 to be an integer.  If the value
** in P1 is not an integer and cannot be converted into an integer
** without data loss, then jump immediately to P2, or if P2==0
** raise an SQLITE_MISMATCH exception.
*/
case OP_MustBeInt: {            /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  applyAffinity(pIn1, SQLITE_AFF_NUMERIC, encoding);
  if( (pIn1->flags & MEM_Int)==0 ){
    if( pOp->p2==0 ){
      rc = SQLITE_MISMATCH;
      goto abort_due_to_error;
    }else{
      pc = pOp->p2 - 1;
    }
  }else{
    MemSetTypeFlag(pIn1, MEM_Int);
  }
  break;
}

#ifndef SQLITE_OMIT_FLOATING_POINT
/* Opcode: RealAffinity P1 * * * *
**
** If register P1 holds an integer convert it to a real value.
**
** This opcode is used when extracting information from a column that
** has REAL affinity.  Such column values may still be stored as
** integers, for space efficiency, but after extraction we want them
** to have only a real value.
*/
case OP_RealAffinity: {                  /* in1 */
  pIn1 = &aMem[pOp->p1];
  if( pIn1->flags & MEM_Int ){
    sqlite3VdbeMemRealify(pIn1);
  }
  break;
}
#endif

#ifndef SQLITE_OMIT_CAST
/* Opcode: ToText P1 * * * *
**
** Force the value in register P1 to be text.
** If the value is numeric, convert it to a string using the
** equivalent of printf().  Blob values are unchanged and
** are afterwards simply interpreted as text.
**
** A NULL value is not changed by this routine.  It remains NULL.
*/
case OP_ToText: {                  /* same as TK_TO_TEXT, in1 */
  pIn1 = &aMem[pOp->p1];
  memAboutToChange(p, pIn1);
  if( pIn1->flags & MEM_Null ) break;
  assert( MEM_Str==(MEM_Blob>>3) );
  pIn1->flags |= (pIn1->flags&MEM_Blob)>>3;
  applyAffinity(pIn1, SQLITE_AFF_TEXT, encoding);
  rc = ExpandBlob(pIn1);
  assert( pIn1->flags & MEM_Str || db->mallocFailed );
  pIn1->flags &= ~(MEM_Int|MEM_Real|MEM_Blob|MEM_Zero);
  UPDATE_MAX_BLOBSIZE(pIn1);
  break;
}

/* Opcode: ToBlob P1 * * * *
**
** Force the value in register P1 to be a BLOB.
** If the value is numeric, convert it to a string first.
** Strings are simply reinterpreted as blobs with no change
** to the underlying data.
**
** A NULL value is not changed by this routine.  It remains NULL.
*/
case OP_ToBlob: {                  /* same as TK_TO_BLOB, in1 */
  pIn1 = &aMem[pOp->p1];
  if( pIn1->flags & MEM_Null ) break;
  if( (pIn1->flags & MEM_Blob)==0 ){
    applyAffinity(pIn1, SQLITE_AFF_TEXT, encoding);
    assert( pIn1->flags & MEM_Str || db->mallocFailed );
    MemSetTypeFlag(pIn1, MEM_Blob);
  }else{
    pIn1->flags &= ~(MEM_TypeMask&~MEM_Blob);
  }
  UPDATE_MAX_BLOBSIZE(pIn1);
  break;
}

/* Opcode: ToNumeric P1 * * * *
**
** Force the value in register P1 to be numeric (either an
** integer or a floating-point number.)
** If the value is text or blob, try to convert it to an using the
** equivalent of atoi() or atof() and store 0 if no such conversion 
** is possible.
**
** A NULL value is not changed by this routine.  It remains NULL.
*/
case OP_ToNumeric: {                  /* same as TK_TO_NUMERIC, in1 */
  pIn1 = &aMem[pOp->p1];
  sqlite3VdbeMemNumerify(pIn1);
  break;
}
#endif /* SQLITE_OMIT_CAST */

/* Opcode: ToInt P1 * * * *
**
** Force the value in register P1 to be an integer.  If
** The value is currently a real number, drop its fractional part.
** If the value is text or blob, try to convert it to an integer using the
** equivalent of atoi() and store 0 if no such conversion is possible.
**
** A NULL value is not changed by this routine.  It remains NULL.
*/
case OP_ToInt: {                  /* same as TK_TO_INT, in1 */
  pIn1 = &aMem[pOp->p1];
  if( (pIn1->flags & MEM_Null)==0 ){
    sqlite3VdbeMemIntegerify(pIn1);
  }
  break;
}

#if !defined(SQLITE_OMIT_CAST) && !defined(SQLITE_OMIT_FLOATING_POINT)
/* Opcode: ToReal P1 * * * *
**
** Force the value in register P1 to be a floating point number.
** If The value is currently an integer, convert it.
** If the value is text or blob, try to convert it to an integer using the
** equivalent of atoi() and store 0.0 if no such conversion is possible.
**
** A NULL value is not changed by this routine.  It remains NULL.
*/
case OP_ToReal: {                  /* same as TK_TO_REAL, in1 */
  pIn1 = &aMem[pOp->p1];
  memAboutToChange(p, pIn1);
  if( (pIn1->flags & MEM_Null)==0 ){
    sqlite3VdbeMemRealify(pIn1);
  }
  break;
}
#endif /* !defined(SQLITE_OMIT_CAST) && !defined(SQLITE_OMIT_FLOATING_POINT) */

/* Opcode: Lt P1 P2 P3 P4 P5
**
** Compare the values in register P1 and P3.  If reg(P3)<reg(P1) then
** jump to address P2.  
**
** If the SQLITE_JUMPIFNULL bit of P5 is set and either reg(P1) or
** reg(P3) is NULL then take the jump.  If the SQLITE_JUMPIFNULL 
** bit is clear then fall through if either operand is NULL.
**
** The SQLITE_AFF_MASK portion of P5 must be an affinity character -
** SQLITE_AFF_TEXT, SQLITE_AFF_INTEGER, and so forth. An attempt is made 
** to coerce both inputs according to this affinity before the
** comparison is made. If the SQLITE_AFF_MASK is 0x00, then numeric
** affinity is used. Note that the affinity conversions are stored
** back into the input registers P1 and P3.  So this opcode can cause
** persistent changes to registers P1 and P3.
**
** Once any conversions have taken place, and neither value is NULL, 
** the values are compared. If both values are blobs then memcmp() is
** used to determine the results of the comparison.  If both values
** are text, then the appropriate collating function specified in
** P4 is  used to do the comparison.  If P4 is not specified then
** memcmp() is used to compare text string.  If both values are
** numeric, then a numeric comparison is used. If the two values
** are of different types, then numbers are considered less than
** strings and strings are considered less than blobs.
**
** If the SQLITE_STOREP2 bit of P5 is set, then do not jump.  Instead,
** store a boolean result (either 0, or 1, or NULL) in register P2.
*/
/* Opcode: Ne P1 P2 P3 P4 P5
**
** This works just like the Lt opcode except that the jump is taken if
** the operands in registers P1 and P3 are not equal.  See the Lt opcode for
** additional information.
**
** If SQLITE_NULLEQ is set in P5 then the result of comparison is always either
** true or false and is never NULL.  If both operands are NULL then the result
** of comparison is false.  If either operand is NULL then the result is true.
** If neither operand is NULL the result is the same as it would be if
** the SQLITE_NULLEQ flag were omitted from P5.
*/
/* Opcode: Eq P1 P2 P3 P4 P5
**
** This works just like the Lt opcode except that the jump is taken if
** the operands in registers P1 and P3 are equal.
** See the Lt opcode for additional information.
**
** If SQLITE_NULLEQ is set in P5 then the result of comparison is always either
** true or false and is never NULL.  If both operands are NULL then the result
** of comparison is true.  If either operand is NULL then the result is false.
** If neither operand is NULL the result is the same as it would be if
** the SQLITE_NULLEQ flag were omitted from P5.
*/
/* Opcode: Le P1 P2 P3 P4 P5
**
** This works just like the Lt opcode except that the jump is taken if
** the content of register P3 is less than or equal to the content of
** register P1.  See the Lt opcode for additional information.
*/
/* Opcode: Gt P1 P2 P3 P4 P5
**
** This works just like the Lt opcode except that the jump is taken if
** the content of register P3 is greater than the content of
** register P1.  See the Lt opcode for additional information.
*/
/* Opcode: Ge P1 P2 P3 P4 P5
**
** This works just like the Lt opcode except that the jump is taken if
** the content of register P3 is greater than or equal to the content of
** register P1.  See the Lt opcode for additional information.
*/
case OP_Eq:               /* same as TK_EQ, jump, in1, in3 */
case OP_Ne:               /* same as TK_NE, jump, in1, in3 */
case OP_Lt:               /* same as TK_LT, jump, in1, in3 */
case OP_Le:               /* same as TK_LE, jump, in1, in3 */
case OP_Gt:               /* same as TK_GT, jump, in1, in3 */
case OP_Ge: {             /* same as TK_GE, jump, in1, in3 */
#if 0  /* local variables moved into u.ai */
  int res;            /* Result of the comparison of pIn1 against pIn3 */
  char affinity;      /* Affinity to use for comparison */
  u16 flags1;         /* Copy of initial value of pIn1->flags */
  u16 flags3;         /* Copy of initial value of pIn3->flags */
#endif /* local variables moved into u.ai */

  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  u.ai.flags1 = pIn1->flags;
  u.ai.flags3 = pIn3->flags;
  if( (u.ai.flags1 | u.ai.flags3)&MEM_Null ){
    /* One or both operands are NULL */
    if( pOp->p5 & SQLITE_NULLEQ ){
      /* If SQLITE_NULLEQ is set (which will only happen if the operator is
      ** OP_Eq or OP_Ne) then take the jump or not depending on whether
      ** or not both operands are null.
      */
      assert( pOp->opcode==OP_Eq || pOp->opcode==OP_Ne );
      u.ai.res = (u.ai.flags1 & u.ai.flags3 & MEM_Null)==0;
    }else{
      /* SQLITE_NULLEQ is clear and at least one operand is NULL,
      ** then the result is always NULL.
      ** The jump is taken if the SQLITE_JUMPIFNULL bit is set.
      */
      if( pOp->p5 & SQLITE_STOREP2 ){
        pOut = &aMem[pOp->p2];
        MemSetTypeFlag(pOut, MEM_Null);
        REGISTER_TRACE(pOp->p2, pOut);
      }else if( pOp->p5 & SQLITE_JUMPIFNULL ){
        pc = pOp->p2-1;
      }
      break;
    }
  }else{
    /* Neither operand is NULL.  Do a comparison. */
    u.ai.affinity = pOp->p5 & SQLITE_AFF_MASK;
    if( u.ai.affinity ){
      applyAffinity(pIn1, u.ai.affinity, encoding);
      applyAffinity(pIn3, u.ai.affinity, encoding);
      if( db->mallocFailed ) goto no_mem;
    }

    assert( pOp->p4type==P4_COLLSEQ || pOp->p4.pColl==0 );
    ExpandBlob(pIn1);
    ExpandBlob(pIn3);
    u.ai.res = sqlite3MemCompare(pIn3, pIn1, pOp->p4.pColl);
  }
  switch( pOp->opcode ){
    case OP_Eq:    u.ai.res = u.ai.res==0;     break;
    case OP_Ne:    u.ai.res = u.ai.res!=0;     break;
    case OP_Lt:    u.ai.res = u.ai.res<0;      break;
    case OP_Le:    u.ai.res = u.ai.res<=0;     break;
    case OP_Gt:    u.ai.res = u.ai.res>0;      break;
    default:       u.ai.res = u.ai.res>=0;     break;
  }

  if( pOp->p5 & SQLITE_STOREP2 ){
    pOut = &aMem[pOp->p2];
    memAboutToChange(p, pOut);
    MemSetTypeFlag(pOut, MEM_Int);
    pOut->u.i = u.ai.res;
    REGISTER_TRACE(pOp->p2, pOut);
  }else if( u.ai.res ){
    pc = pOp->p2-1;
  }

  /* Undo any changes made by applyAffinity() to the input registers. */
  pIn1->flags = (pIn1->flags&~MEM_TypeMask) | (u.ai.flags1&MEM_TypeMask);
  pIn3->flags = (pIn3->flags&~MEM_TypeMask) | (u.ai.flags3&MEM_TypeMask);
  break;
}

/* Opcode: Permutation * * * P4 *
**
** Set the permutation used by the OP_Compare operator to be the array
** of integers in P4.
**
** The permutation is only valid until the next OP_Permutation, OP_Compare,
** OP_Halt, or OP_ResultRow.  Typically the OP_Permutation should occur
** immediately prior to the OP_Compare.
*/
case OP_Permutation: {
  assert( pOp->p4type==P4_INTARRAY );
  assert( pOp->p4.ai );
  aPermute = pOp->p4.ai;
  break;
}

/* Opcode: Compare P1 P2 P3 P4 *
**
** Compare two vectors of registers in reg(P1)..reg(P1+P3-1) (call this
** vector "A") and in reg(P2)..reg(P2+P3-1) ("B").  Save the result of
** the comparison for use by the next OP_Jump instruct.
**
** P4 is a KeyInfo structure that defines collating sequences and sort
** orders for the comparison.  The permutation applies to registers
** only.  The KeyInfo elements are used sequentially.
**
** The comparison is a sort comparison, so NULLs compare equal,
** NULLs are less than numbers, numbers are less than strings,
** and strings are less than blobs.
*/
case OP_Compare: {
#if 0  /* local variables moved into u.aj */
  int n;
  int i;
  int p1;
  int p2;
  const KeyInfo *pKeyInfo;
  int idx;
  CollSeq *pColl;    /* Collating sequence to use on this term */
  int bRev;          /* True for DESCENDING sort order */
#endif /* local variables moved into u.aj */

  u.aj.n = pOp->p3;
  u.aj.pKeyInfo = pOp->p4.pKeyInfo;
  assert( u.aj.n>0 );
  assert( u.aj.pKeyInfo!=0 );
  u.aj.p1 = pOp->p1;
  u.aj.p2 = pOp->p2;
#if SQLITE_DEBUG
  if( aPermute ){
    int k, mx = 0;
    for(k=0; k<u.aj.n; k++) if( aPermute[k]>mx ) mx = aPermute[k];
    assert( u.aj.p1>0 && u.aj.p1+mx<=p->nMem+1 );
    assert( u.aj.p2>0 && u.aj.p2+mx<=p->nMem+1 );
  }else{
    assert( u.aj.p1>0 && u.aj.p1+u.aj.n<=p->nMem+1 );
    assert( u.aj.p2>0 && u.aj.p2+u.aj.n<=p->nMem+1 );
  }
#endif /* SQLITE_DEBUG */
  for(u.aj.i=0; u.aj.i<u.aj.n; u.aj.i++){
    u.aj.idx = aPermute ? aPermute[u.aj.i] : u.aj.i;
    assert( memIsValid(&aMem[u.aj.p1+u.aj.idx]) );
    assert( memIsValid(&aMem[u.aj.p2+u.aj.idx]) );
    REGISTER_TRACE(u.aj.p1+u.aj.idx, &aMem[u.aj.p1+u.aj.idx]);
    REGISTER_TRACE(u.aj.p2+u.aj.idx, &aMem[u.aj.p2+u.aj.idx]);
    assert( u.aj.i<u.aj.pKeyInfo->nField );
    u.aj.pColl = u.aj.pKeyInfo->aColl[u.aj.i];
    u.aj.bRev = u.aj.pKeyInfo->aSortOrder[u.aj.i];
    iCompare = sqlite3MemCompare(&aMem[u.aj.p1+u.aj.idx], &aMem[u.aj.p2+u.aj.idx], u.aj.pColl);
    if( iCompare ){
      if( u.aj.bRev ) iCompare = -iCompare;
      break;
    }
  }
  aPermute = 0;
  break;
}

/* Opcode: Jump P1 P2 P3 * *
**
** Jump to the instruction at address P1, P2, or P3 depending on whether
** in the most recent OP_Compare instruction the P1 vector was less than
** equal to, or greater than the P2 vector, respectively.
*/
case OP_Jump: {             /* jump */
  if( iCompare<0 ){
    pc = pOp->p1 - 1;
  }else if( iCompare==0 ){
    pc = pOp->p2 - 1;
  }else{
    pc = pOp->p3 - 1;
  }
  break;
}

/* Opcode: And P1 P2 P3 * *
**
** Take the logical AND of the values in registers P1 and P2 and
** write the result into register P3.
**
** If either P1 or P2 is 0 (false) then the result is 0 even if
** the other input is NULL.  A NULL and true or two NULLs give
** a NULL output.
*/
/* Opcode: Or P1 P2 P3 * *
**
** Take the logical OR of the values in register P1 and P2 and
** store the answer in register P3.
**
** If either P1 or P2 is nonzero (true) then the result is 1 (true)
** even if the other input is NULL.  A NULL and false or two NULLs
** give a NULL output.
*/
case OP_And:              /* same as TK_AND, in1, in2, out3 */
case OP_Or: {             /* same as TK_OR, in1, in2, out3 */
#if 0  /* local variables moved into u.ak */
  int v1;    /* Left operand:  0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */
  int v2;    /* Right operand: 0==FALSE, 1==TRUE, 2==UNKNOWN or NULL */
#endif /* local variables moved into u.ak */

  pIn1 = &aMem[pOp->p1];
  if( pIn1->flags & MEM_Null ){
    u.ak.v1 = 2;
  }else{
    u.ak.v1 = sqlite3VdbeIntValue(pIn1)!=0;
  }
  pIn2 = &aMem[pOp->p2];
  if( pIn2->flags & MEM_Null ){
    u.ak.v2 = 2;
  }else{
    u.ak.v2 = sqlite3VdbeIntValue(pIn2)!=0;
  }
  if( pOp->opcode==OP_And ){
    static const unsigned char and_logic[] = { 0, 0, 0, 0, 1, 2, 0, 2, 2 };
    u.ak.v1 = and_logic[u.ak.v1*3+u.ak.v2];
  }else{
    static const unsigned char or_logic[] = { 0, 1, 2, 1, 1, 1, 2, 1, 2 };
    u.ak.v1 = or_logic[u.ak.v1*3+u.ak.v2];
  }
  pOut = &aMem[pOp->p3];
  if( u.ak.v1==2 ){
    MemSetTypeFlag(pOut, MEM_Null);
  }else{
    pOut->u.i = u.ak.v1;
    MemSetTypeFlag(pOut, MEM_Int);
  }
  break;
}

/* Opcode: Not P1 P2 * * *
**
** Interpret the value in register P1 as a boolean value.  Store the
** boolean complement in register P2.  If the value in register P1 is 
** NULL, then a NULL is stored in P2.
*/
case OP_Not: {                /* same as TK_NOT, in1, out2 */
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  if( pIn1->flags & MEM_Null ){
    sqlite3VdbeMemSetNull(pOut);
  }else{
    sqlite3VdbeMemSetInt64(pOut, !sqlite3VdbeIntValue(pIn1));
  }
  break;
}

/* Opcode: BitNot P1 P2 * * *
**
** Interpret the content of register P1 as an integer.  Store the
** ones-complement of the P1 value into register P2.  If P1 holds
** a NULL then store a NULL in P2.
*/
case OP_BitNot: {             /* same as TK_BITNOT, in1, out2 */
  pIn1 = &aMem[pOp->p1];
  pOut = &aMem[pOp->p2];
  if( pIn1->flags & MEM_Null ){
    sqlite3VdbeMemSetNull(pOut);
  }else{
    sqlite3VdbeMemSetInt64(pOut, ~sqlite3VdbeIntValue(pIn1));
  }
  break;
}

/* Opcode: If P1 P2 P3 * *
**
** Jump to P2 if the value in register P1 is true.  The value
** is considered true if it is numeric and non-zero.  If the value
** in P1 is NULL then take the jump if P3 is true.
*/
/* Opcode: IfNot P1 P2 P3 * *
**
** Jump to P2 if the value in register P1 is False.  The value
** is considered true if it has a numeric value of zero.  If the value
** in P1 is NULL then take the jump if P3 is true.
*/
case OP_If:                 /* jump, in1 */
case OP_IfNot: {            /* jump, in1 */
#if 0  /* local variables moved into u.al */
  int c;
#endif /* local variables moved into u.al */
  pIn1 = &aMem[pOp->p1];
  if( pIn1->flags & MEM_Null ){
    u.al.c = pOp->p3;
  }else{
#ifdef SQLITE_OMIT_FLOATING_POINT
    u.al.c = sqlite3VdbeIntValue(pIn1)!=0;
#else
    u.al.c = sqlite3VdbeRealValue(pIn1)!=0.0;
#endif
    if( pOp->opcode==OP_IfNot ) u.al.c = !u.al.c;
  }
  if( u.al.c ){
    pc = pOp->p2-1;
  }
  break;
}

/* Opcode: IsNull P1 P2 * * *
**
** Jump to P2 if the value in register P1 is NULL.
*/
case OP_IsNull: {            /* same as TK_ISNULL, jump, in1 */
  pIn1 = &aMem[pOp->p1];
  if( (pIn1->flags & MEM_Null)!=0 ){
    pc = pOp->p2 - 1;
  }
  break;
}

/* Opcode: NotNull P1 P2 * * *
**
** Jump to P2 if the value in register P1 is not NULL.  
*/
case OP_NotNull: {            /* same as TK_NOTNULL, jump, in1 */
  pIn1 = &aMem[pOp->p1];
  if( (pIn1->flags & MEM_Null)==0 ){
    pc = pOp->p2 - 1;
  }
  break;
}

/* Opcode: Column P1 P2 P3 P4 P5
**
** Interpret the data that cursor P1 points to as a structure built using
** the MakeRecord instruction.  (See the MakeRecord opcode for additional
** information about the format of the data.)  Extract the P2-th column
** from this record.  If there are less that (P2+1) 
** values in the record, extract a NULL.
**
** The value extracted is stored in register P3.
**
** If the column contains fewer than P2 fields, then extract a NULL.  Or,
** if the P4 argument is a P4_MEM use the value of the P4 argument as
** the result.
**
** If the OPFLAG_CLEARCACHE bit is set on P5 and P1 is a pseudo-table cursor,
** then the cache of the cursor is reset prior to extracting the column.
** The first OP_Column against a pseudo-table after the value of the content
** register has changed should have this bit set.
*/
case OP_Column: {
#if 0  /* local variables moved into u.am */
  u32 payloadSize;   /* Number of bytes in the record */
  i64 payloadSize64; /* Number of bytes in the record */
  int p1;            /* P1 value of the opcode */
  int p2;            /* column number to retrieve */
  VdbeCursor *pC;    /* The VDBE cursor */
  char *zRec;        /* Pointer to complete record-data */
  BtCursor *pCrsr;   /* The BTree cursor */
  u32 *aType;        /* aType[i] holds the numeric type of the i-th column */
  u32 *aOffset;      /* aOffset[i] is offset to start of data for i-th column */
  int nField;        /* number of fields in the record */
  int len;           /* The length of the serialized data for the column */
  int i;             /* Loop counter */
  char *zData;       /* Part of the record being decoded */
  Mem *pDest;        /* Where to write the extracted value */
  Mem sMem;          /* For storing the record being decoded */
  u8 *zIdx;          /* Index into header */
  u8 *zEndHdr;       /* Pointer to first byte after the header */
  u32 offset;        /* Offset into the data */
  u32 szField;       /* Number of bytes in the content of a field */
  int szHdr;         /* Size of the header size field at start of record */
  int avail;         /* Number of bytes of available data */
  Mem *pReg;         /* PseudoTable input register */
#endif /* local variables moved into u.am */


  u.am.p1 = pOp->p1;
  u.am.p2 = pOp->p2;
  u.am.pC = 0;
  memset(&u.am.sMem, 0, sizeof(u.am.sMem));
  assert( u.am.p1<p->nCursor );
  assert( pOp->p3>0 && pOp->p3<=p->nMem );
  u.am.pDest = &aMem[pOp->p3];
  memAboutToChange(p, u.am.pDest);
  MemSetTypeFlag(u.am.pDest, MEM_Null);
  u.am.zRec = 0;

  /* This block sets the variable u.am.payloadSize to be the total number of
  ** bytes in the record.
  **
  ** u.am.zRec is set to be the complete text of the record if it is available.
  ** The complete record text is always available for pseudo-tables
  ** If the record is stored in a cursor, the complete record text
  ** might be available in the  u.am.pC->aRow cache.  Or it might not be.
  ** If the data is unavailable,  u.am.zRec is set to NULL.
  **
  ** We also compute the number of columns in the record.  For cursors,
  ** the number of columns is stored in the VdbeCursor.nField element.
  */
  u.am.pC = p->apCsr[u.am.p1];
  assert( u.am.pC!=0 );
#ifndef SQLITE_OMIT_VIRTUALTABLE
  assert( u.am.pC->pVtabCursor==0 );
#endif
  u.am.pCrsr = u.am.pC->pCursor;
  if( u.am.pCrsr!=0 ){
    /* The record is stored in a B-Tree */
    rc = sqlite3VdbeCursorMoveto(u.am.pC);
    if( rc ) goto abort_due_to_error;
    if( u.am.pC->nullRow ){
      u.am.payloadSize = 0;
    }else if( u.am.pC->cacheStatus==p->cacheCtr ){
      u.am.payloadSize = u.am.pC->payloadSize;
      u.am.zRec = (char*)u.am.pC->aRow;
    }else if( u.am.pC->isIndex ){
      assert( sqlite3BtreeCursorIsValid(u.am.pCrsr) );
      rc = sqlite3BtreeKeySize(u.am.pCrsr, &u.am.payloadSize64);
      assert( rc==SQLITE_OK );   /* True because of CursorMoveto() call above */
      /* sqlite3BtreeParseCellPtr() uses getVarint32() to extract the
      ** payload size, so it is impossible for u.am.payloadSize64 to be
      ** larger than 32 bits. */
      assert( (u.am.payloadSize64 & SQLITE_MAX_U32)==(u64)u.am.payloadSize64 );
      u.am.payloadSize = (u32)u.am.payloadSize64;
    }else{
      assert( sqlite3BtreeCursorIsValid(u.am.pCrsr) );
      rc = sqlite3BtreeDataSize(u.am.pCrsr, &u.am.payloadSize);
      assert( rc==SQLITE_OK );   /* DataSize() cannot fail */
    }
  }else if( u.am.pC->pseudoTableReg>0 ){
    u.am.pReg = &aMem[u.am.pC->pseudoTableReg];
    assert( u.am.pReg->flags & MEM_Blob );
    assert( memIsValid(u.am.pReg) );
    u.am.payloadSize = u.am.pReg->n;
    u.am.zRec = u.am.pReg->z;
    u.am.pC->cacheStatus = (pOp->p5&OPFLAG_CLEARCACHE) ? CACHE_STALE : p->cacheCtr;
    assert( u.am.payloadSize==0 || u.am.zRec!=0 );
  }else{
    /* Consider the row to be NULL */
    u.am.payloadSize = 0;
  }

  /* If u.am.payloadSize is 0, then just store a NULL */
  if( u.am.payloadSize==0 ){
    assert( u.am.pDest->flags&MEM_Null );
    goto op_column_out;
  }
  assert( db->aLimit[SQLITE_LIMIT_LENGTH]>=0 );
  if( u.am.payloadSize > (u32)db->aLimit[SQLITE_LIMIT_LENGTH] ){
    goto too_big;
  }

  u.am.nField = u.am.pC->nField;
  assert( u.am.p2<u.am.nField );

  /* Read and parse the table header.  Store the results of the parse
  ** into the record header cache fields of the cursor.
  */
  u.am.aType = u.am.pC->aType;
  if( u.am.pC->cacheStatus==p->cacheCtr ){
    u.am.aOffset = u.am.pC->aOffset;
  }else{
    assert(u.am.aType);
    u.am.avail = 0;
    u.am.pC->aOffset = u.am.aOffset = &u.am.aType[u.am.nField];
    u.am.pC->payloadSize = u.am.payloadSize;
    u.am.pC->cacheStatus = p->cacheCtr;

    /* Figure out how many bytes are in the header */
    if( u.am.zRec ){
      u.am.zData = u.am.zRec;
    }else{
      if( u.am.pC->isIndex ){
        u.am.zData = (char*)sqlite3BtreeKeyFetch(u.am.pCrsr, &u.am.avail);
      }else{
        u.am.zData = (char*)sqlite3BtreeDataFetch(u.am.pCrsr, &u.am.avail);
      }
      /* If KeyFetch()/DataFetch() managed to get the entire payload,
      ** save the payload in the u.am.pC->aRow cache.  That will save us from
      ** having to make additional calls to fetch the content portion of
      ** the record.
      */
      assert( u.am.avail>=0 );
      if( u.am.payloadSize <= (u32)u.am.avail ){
        u.am.zRec = u.am.zData;
        u.am.pC->aRow = (u8*)u.am.zData;
      }else{
        u.am.pC->aRow = 0;
      }
    }
    /* The following assert is true in all cases accept when
    ** the database file has been corrupted externally.
    **    assert( u.am.zRec!=0 || u.am.avail>=u.am.payloadSize || u.am.avail>=9 ); */
    u.am.szHdr = getVarint32((u8*)u.am.zData, u.am.offset);

    /* Make sure a corrupt database has not given us an oversize header.
    ** Do this now to avoid an oversize memory allocation.
    **
    ** Type entries can be between 1 and 5 bytes each.  But 4 and 5 byte
    ** types use so much data space that there can only be 4096 and 32 of
    ** them, respectively.  So the maximum header length results from a
    ** 3-byte type for each of the maximum of 32768 columns plus three
    ** extra bytes for the header length itself.  32768*3 + 3 = 98307.
    */
    if( u.am.offset > 98307 ){
      rc = SQLITE_CORRUPT_BKPT;
      goto op_column_out;
    }

    /* Compute in u.am.len the number of bytes of data we need to read in order
    ** to get u.am.nField type values.  u.am.offset is an upper bound on this.  But
    ** u.am.nField might be significantly less than the true number of columns
    ** in the table, and in that case, 5*u.am.nField+3 might be smaller than u.am.offset.
    ** We want to minimize u.am.len in order to limit the size of the memory
    ** allocation, especially if a corrupt database file has caused u.am.offset
    ** to be oversized. Offset is limited to 98307 above.  But 98307 might
    ** still exceed Robson memory allocation limits on some configurations.
    ** On systems that cannot tolerate large memory allocations, u.am.nField*5+3
    ** will likely be much smaller since u.am.nField will likely be less than
    ** 20 or so.  This insures that Robson memory allocation limits are
    ** not exceeded even for corrupt database files.
    */
    u.am.len = u.am.nField*5 + 3;
    if( u.am.len > (int)u.am.offset ) u.am.len = (int)u.am.offset;

    /* The KeyFetch() or DataFetch() above are fast and will get the entire
    ** record header in most cases.  But they will fail to get the complete
    ** record header if the record header does not fit on a single page
    ** in the B-Tree.  When that happens, use sqlite3VdbeMemFromBtree() to
    ** acquire the complete header text.
    */
    if( !u.am.zRec && u.am.avail<u.am.len ){
      u.am.sMem.flags = 0;
      u.am.sMem.db = 0;
      rc = sqlite3VdbeMemFromBtree(u.am.pCrsr, 0, u.am.len, u.am.pC->isIndex, &u.am.sMem);
      if( rc!=SQLITE_OK ){
        goto op_column_out;
      }
      u.am.zData = u.am.sMem.z;
    }
    u.am.zEndHdr = (u8 *)&u.am.zData[u.am.len];
    u.am.zIdx = (u8 *)&u.am.zData[u.am.szHdr];

    /* Scan the header and use it to fill in the u.am.aType[] and u.am.aOffset[]
    ** arrays.  u.am.aType[u.am.i] will contain the type integer for the u.am.i-th
    ** column and u.am.aOffset[u.am.i] will contain the u.am.offset from the beginning
    ** of the record to the start of the data for the u.am.i-th column
    */
    for(u.am.i=0; u.am.i<u.am.nField; u.am.i++){
      if( u.am.zIdx<u.am.zEndHdr ){
        u.am.aOffset[u.am.i] = u.am.offset;
        u.am.zIdx += getVarint32(u.am.zIdx, u.am.aType[u.am.i]);
        u.am.szField = sqlite3VdbeSerialTypeLen(u.am.aType[u.am.i]);
        u.am.offset += u.am.szField;
        if( u.am.offset<u.am.szField ){  /* True if u.am.offset overflows */
          u.am.zIdx = &u.am.zEndHdr[1];  /* Forces SQLITE_CORRUPT return below */
          break;
        }
      }else{
        /* If u.am.i is less that u.am.nField, then there are less fields in this
        ** record than SetNumColumns indicated there are columns in the
        ** table. Set the u.am.offset for any extra columns not present in
        ** the record to 0. This tells code below to store a NULL
        ** instead of deserializing a value from the record.
        */
        u.am.aOffset[u.am.i] = 0;
      }
    }
    sqlite3VdbeMemRelease(&u.am.sMem);
    u.am.sMem.flags = MEM_Null;

    /* If we have read more header data than was contained in the header,
    ** or if the end of the last field appears to be past the end of the
    ** record, or if the end of the last field appears to be before the end
    ** of the record (when all fields present), then we must be dealing
    ** with a corrupt database.
    */
    if( (u.am.zIdx > u.am.zEndHdr) || (u.am.offset > u.am.payloadSize)
         || (u.am.zIdx==u.am.zEndHdr && u.am.offset!=u.am.payloadSize) ){
      rc = SQLITE_CORRUPT_BKPT;
      goto op_column_out;
    }
  }

  /* Get the column information. If u.am.aOffset[u.am.p2] is non-zero, then
  ** deserialize the value from the record. If u.am.aOffset[u.am.p2] is zero,
  ** then there are not enough fields in the record to satisfy the
  ** request.  In this case, set the value NULL or to P4 if P4 is
  ** a pointer to a Mem object.
  */
  if( u.am.aOffset[u.am.p2] ){
    assert( rc==SQLITE_OK );
    if( u.am.zRec ){
      sqlite3VdbeMemReleaseExternal(u.am.pDest);
      sqlite3VdbeSerialGet((u8 *)&u.am.zRec[u.am.aOffset[u.am.p2]], u.am.aType[u.am.p2], u.am.pDest);
    }else{
      u.am.len = sqlite3VdbeSerialTypeLen(u.am.aType[u.am.p2]);
      sqlite3VdbeMemMove(&u.am.sMem, u.am.pDest);
      rc = sqlite3VdbeMemFromBtree(u.am.pCrsr, u.am.aOffset[u.am.p2], u.am.len, u.am.pC->isIndex, &u.am.sMem);
      if( rc!=SQLITE_OK ){
        goto op_column_out;
      }
      u.am.zData = u.am.sMem.z;
      sqlite3VdbeSerialGet((u8*)u.am.zData, u.am.aType[u.am.p2], u.am.pDest);
    }
    u.am.pDest->enc = encoding;
  }else{
    if( pOp->p4type==P4_MEM ){
      sqlite3VdbeMemShallowCopy(u.am.pDest, pOp->p4.pMem, MEM_Static);
    }else{
      assert( u.am.pDest->flags&MEM_Null );
    }
  }

  /* If we dynamically allocated space to hold the data (in the
  ** sqlite3VdbeMemFromBtree() call above) then transfer control of that
  ** dynamically allocated space over to the u.am.pDest structure.
  ** This prevents a memory copy.
  */
  if( u.am.sMem.zMalloc ){
    assert( u.am.sMem.z==u.am.sMem.zMalloc );
    assert( !(u.am.pDest->flags & MEM_Dyn) );
    assert( !(u.am.pDest->flags & (MEM_Blob|MEM_Str)) || u.am.pDest->z==u.am.sMem.z );
    u.am.pDest->flags &= ~(MEM_Ephem|MEM_Static);
    u.am.pDest->flags |= MEM_Term;
    u.am.pDest->z = u.am.sMem.z;
    u.am.pDest->zMalloc = u.am.sMem.zMalloc;
  }

  rc = sqlite3VdbeMemMakeWriteable(u.am.pDest);

op_column_out:
  UPDATE_MAX_BLOBSIZE(u.am.pDest);
  REGISTER_TRACE(pOp->p3, u.am.pDest);
  break;
}

/* Opcode: Affinity P1 P2 * P4 *
**
** Apply affinities to a range of P2 registers starting with P1.
**
** P4 is a string that is P2 characters long. The nth character of the
** string indicates the column affinity that should be used for the nth
** memory cell in the range.
*/
case OP_Affinity: {
#if 0  /* local variables moved into u.an */
  const char *zAffinity;   /* The affinity to be applied */
  char cAff;               /* A single character of affinity */
#endif /* local variables moved into u.an */

  u.an.zAffinity = pOp->p4.z;
  assert( u.an.zAffinity!=0 );
  assert( u.an.zAffinity[pOp->p2]==0 );
  pIn1 = &aMem[pOp->p1];
  while( (u.an.cAff = *(u.an.zAffinity++))!=0 ){
    assert( pIn1 <= &p->aMem[p->nMem] );
    assert( memIsValid(pIn1) );
    ExpandBlob(pIn1);
    applyAffinity(pIn1, u.an.cAff, encoding);
    pIn1++;
  }
  break;
}

/* Opcode: MakeRecord P1 P2 P3 P4 *
**
** Convert P2 registers beginning with P1 into the [record format]
** use as a data record in a database table or as a key
** in an index.  The OP_Column opcode can decode the record later.
**
** P4 may be a string that is P2 characters long.  The nth character of the
** string indicates the column affinity that should be used for the nth
** field of the index key.
**
** The mapping from character to affinity is given by the SQLITE_AFF_
** macros defined in sqliteInt.h.
**
** If P4 is NULL then all index fields have the affinity NONE.
*/
case OP_MakeRecord: {
#if 0  /* local variables moved into u.ao */
  u8 *zNewRecord;        /* A buffer to hold the data for the new record */
  Mem *pRec;             /* The new record */
  u64 nData;             /* Number of bytes of data space */
  int nHdr;              /* Number of bytes of header space */
  i64 nByte;             /* Data space required for this record */
  int nZero;             /* Number of zero bytes at the end of the record */
  int nVarint;           /* Number of bytes in a varint */
  u32 serial_type;       /* Type field */
  Mem *pData0;           /* First field to be combined into the record */
  Mem *pLast;            /* Last field of the record */
  int nField;            /* Number of fields in the record */
  char *zAffinity;       /* The affinity string for the record */
  int file_format;       /* File format to use for encoding */
  int i;                 /* Space used in zNewRecord[] */
  int len;               /* Length of a field */
#endif /* local variables moved into u.ao */

  /* Assuming the record contains N fields, the record format looks
  ** like this:
  **
  ** ------------------------------------------------------------------------
  ** | hdr-size | type 0 | type 1 | ... | type N-1 | data0 | ... | data N-1 |
  ** ------------------------------------------------------------------------
  **
  ** Data(0) is taken from register P1.  Data(1) comes from register P1+1
  ** and so froth.
  **
  ** Each type field is a varint representing the serial type of the
  ** corresponding data element (see sqlite3VdbeSerialType()). The
  ** hdr-size field is also a varint which is the offset from the beginning
  ** of the record to data0.
  */
  u.ao.nData = 0;         /* Number of bytes of data space */
  u.ao.nHdr = 0;          /* Number of bytes of header space */
  u.ao.nZero = 0;         /* Number of zero bytes at the end of the record */
  u.ao.nField = pOp->p1;
  u.ao.zAffinity = pOp->p4.z;
  assert( u.ao.nField>0 && pOp->p2>0 && pOp->p2+u.ao.nField<=p->nMem+1 );
  u.ao.pData0 = &aMem[u.ao.nField];
  u.ao.nField = pOp->p2;
  u.ao.pLast = &u.ao.pData0[u.ao.nField-1];
  u.ao.file_format = p->minWriteFileFormat;

  /* Identify the output register */
  assert( pOp->p3<pOp->p1 || pOp->p3>=pOp->p1+pOp->p2 );
  pOut = &aMem[pOp->p3];
  memAboutToChange(p, pOut);

  /* Loop through the elements that will make up the record to figure
  ** out how much space is required for the new record.
  */
  for(u.ao.pRec=u.ao.pData0; u.ao.pRec<=u.ao.pLast; u.ao.pRec++){
    assert( memIsValid(u.ao.pRec) );
    if( u.ao.zAffinity ){
      applyAffinity(u.ao.pRec, u.ao.zAffinity[u.ao.pRec-u.ao.pData0], encoding);
    }
    if( u.ao.pRec->flags&MEM_Zero && u.ao.pRec->n>0 ){
      sqlite3VdbeMemExpandBlob(u.ao.pRec);
    }
    u.ao.serial_type = sqlite3VdbeSerialType(u.ao.pRec, u.ao.file_format);
    u.ao.len = sqlite3VdbeSerialTypeLen(u.ao.serial_type);
    u.ao.nData += u.ao.len;
    u.ao.nHdr += sqlite3VarintLen(u.ao.serial_type);
    if( u.ao.pRec->flags & MEM_Zero ){
      /* Only pure zero-filled BLOBs can be input to this Opcode.
      ** We do not allow blobs with a prefix and a zero-filled tail. */
      u.ao.nZero += u.ao.pRec->u.nZero;
    }else if( u.ao.len ){
      u.ao.nZero = 0;
    }
  }

  /* Add the initial header varint and total the size */
  u.ao.nHdr += u.ao.nVarint = sqlite3VarintLen(u.ao.nHdr);
  if( u.ao.nVarint<sqlite3VarintLen(u.ao.nHdr) ){
    u.ao.nHdr++;
  }
  u.ao.nByte = u.ao.nHdr+u.ao.nData-u.ao.nZero;
  if( u.ao.nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    goto too_big;
  }

  /* Make sure the output register has a buffer large enough to store
  ** the new record. The output register (pOp->p3) is not allowed to
  ** be one of the input registers (because the following call to
  ** sqlite3VdbeMemGrow() could clobber the value before it is used).
  */
  if( sqlite3VdbeMemGrow(pOut, (int)u.ao.nByte, 0) ){
    goto no_mem;
  }
  u.ao.zNewRecord = (u8 *)pOut->z;

  /* Write the record */
  u.ao.i = putVarint32(u.ao.zNewRecord, u.ao.nHdr);
  for(u.ao.pRec=u.ao.pData0; u.ao.pRec<=u.ao.pLast; u.ao.pRec++){
    u.ao.serial_type = sqlite3VdbeSerialType(u.ao.pRec, u.ao.file_format);
    u.ao.i += putVarint32(&u.ao.zNewRecord[u.ao.i], u.ao.serial_type);      /* serial type */
  }
  for(u.ao.pRec=u.ao.pData0; u.ao.pRec<=u.ao.pLast; u.ao.pRec++){  /* serial data */
    u.ao.i += sqlite3VdbeSerialPut(&u.ao.zNewRecord[u.ao.i], (int)(u.ao.nByte-u.ao.i), u.ao.pRec,u.ao.file_format);
  }
  assert( u.ao.i==u.ao.nByte );

  assert( pOp->p3>0 && pOp->p3<=p->nMem );
  pOut->n = (int)u.ao.nByte;
  pOut->flags = MEM_Blob | MEM_Dyn;
  pOut->xDel = 0;
  if( u.ao.nZero ){
    pOut->u.nZero = u.ao.nZero;
    pOut->flags |= MEM_Zero;
  }
  pOut->enc = SQLITE_UTF8;  /* In case the blob is ever converted to text */
  REGISTER_TRACE(pOp->p3, pOut);
  UPDATE_MAX_BLOBSIZE(pOut);
  break;
}

/* Opcode: Count P1 P2 * * *
**
** Store the number of entries (an integer value) in the table or index 
** opened by cursor P1 in register P2
*/
#ifndef SQLITE_OMIT_BTREECOUNT
case OP_Count: {         /* out2-prerelease */
#if 0  /* local variables moved into u.ap */
  i64 nEntry;
  BtCursor *pCrsr;
#endif /* local variables moved into u.ap */

  u.ap.pCrsr = p->apCsr[pOp->p1]->pCursor;
  if( u.ap.pCrsr ){
    rc = sqlite3BtreeCount(u.ap.pCrsr, &u.ap.nEntry);
  }else{
    u.ap.nEntry = 0;
  }
  pOut->u.i = u.ap.nEntry;
  break;
}
#endif

/* Opcode: Savepoint P1 * * P4 *
**
** Open, release or rollback the savepoint named by parameter P4, depending
** on the value of P1. To open a new savepoint, P1==0. To release (commit) an
** existing savepoint, P1==1, or to rollback an existing savepoint P1==2.
*/
case OP_Savepoint: {
#if 0  /* local variables moved into u.aq */
  int p1;                         /* Value of P1 operand */
  char *zName;                    /* Name of savepoint */
  int nName;
  Savepoint *pNew;
  Savepoint *pSavepoint;
  Savepoint *pTmp;
  int iSavepoint;
  int ii;
#endif /* local variables moved into u.aq */

  u.aq.p1 = pOp->p1;
  u.aq.zName = pOp->p4.z;

  /* Assert that the u.aq.p1 parameter is valid. Also that if there is no open
  ** transaction, then there cannot be any savepoints.
  */
  assert( db->pSavepoint==0 || db->autoCommit==0 );
  assert( u.aq.p1==SAVEPOINT_BEGIN||u.aq.p1==SAVEPOINT_RELEASE||u.aq.p1==SAVEPOINT_ROLLBACK );
  assert( db->pSavepoint || db->isTransactionSavepoint==0 );
  assert( checkSavepointCount(db) );

  if( u.aq.p1==SAVEPOINT_BEGIN ){
    if( db->writeVdbeCnt>0 ){
      /* A new savepoint cannot be created if there are active write
      ** statements (i.e. open read/write incremental blob handles).
      */
      sqlite3SetString(&p->zErrMsg, db, "cannot open savepoint - "
        "SQL statements in progress");
      rc = SQLITE_BUSY;
    }else{
      u.aq.nName = sqlite3Strlen30(u.aq.zName);

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* This call is Ok even if this savepoint is actually a transaction
      ** savepoint (and therefore should not prompt xSavepoint()) callbacks.
      ** If this is a transaction savepoint being opened, it is guaranteed
      ** that the db->aVTrans[] array is empty.  */
      assert( db->autoCommit==0 || db->nVTrans==0 );
      rc = sqlite3VtabSavepoint(db, SAVEPOINT_BEGIN,
                                db->nStatement+db->nSavepoint);
      if( rc!=SQLITE_OK ) goto abort_due_to_error;
#endif

      /* Create a new savepoint structure. */
      u.aq.pNew = sqlite3DbMallocRaw(db, sizeof(Savepoint)+u.aq.nName+1);
      if( u.aq.pNew ){
        u.aq.pNew->zName = (char *)&u.aq.pNew[1];
        memcpy(u.aq.pNew->zName, u.aq.zName, u.aq.nName+1);

        /* If there is no open transaction, then mark this as a special
        ** "transaction savepoint". */
        if( db->autoCommit ){
          db->autoCommit = 0;
          db->isTransactionSavepoint = 1;
        }else{
          db->nSavepoint++;
        }

        /* Link the new savepoint into the database handle's list. */
        u.aq.pNew->pNext = db->pSavepoint;
        db->pSavepoint = u.aq.pNew;
        u.aq.pNew->nDeferredCons = db->nDeferredCons;
      }
    }
  }else{
    u.aq.iSavepoint = 0;

    /* Find the named savepoint. If there is no such savepoint, then an
    ** an error is returned to the user.  */
    for(
      u.aq.pSavepoint = db->pSavepoint;
      u.aq.pSavepoint && sqlite3StrICmp(u.aq.pSavepoint->zName, u.aq.zName);
      u.aq.pSavepoint = u.aq.pSavepoint->pNext
    ){
      u.aq.iSavepoint++;
    }
    if( !u.aq.pSavepoint ){
      sqlite3SetString(&p->zErrMsg, db, "no such savepoint: %s", u.aq.zName);
      rc = SQLITE_ERROR;
    }else if(
        db->writeVdbeCnt>0 || (u.aq.p1==SAVEPOINT_ROLLBACK && db->activeVdbeCnt>1)
    ){
      /* It is not possible to release (commit) a savepoint if there are
      ** active write statements. It is not possible to rollback a savepoint
      ** if there are any active statements at all.
      */
      sqlite3SetString(&p->zErrMsg, db,
        "cannot %s savepoint - SQL statements in progress",
        (u.aq.p1==SAVEPOINT_ROLLBACK ? "rollback": "release")
      );
      rc = SQLITE_BUSY;
    }else{

      /* Determine whether or not this is a transaction savepoint. If so,
      ** and this is a RELEASE command, then the current transaction
      ** is committed.
      */
      int isTransaction = u.aq.pSavepoint->pNext==0 && db->isTransactionSavepoint;
      if( isTransaction && u.aq.p1==SAVEPOINT_RELEASE ){
        if( (rc = sqlite3VdbeCheckFk(p, 1))!=SQLITE_OK ){
          goto vdbe_return;
        }
        db->autoCommit = 1;
        if( sqlite3VdbeHalt(p)==SQLITE_BUSY ){
          p->pc = pc;
          db->autoCommit = 0;
          p->rc = rc = SQLITE_BUSY;
          goto vdbe_return;
        }
        db->isTransactionSavepoint = 0;
        rc = p->rc;
      }else{
        u.aq.iSavepoint = db->nSavepoint - u.aq.iSavepoint - 1;
        for(u.aq.ii=0; u.aq.ii<db->nDb; u.aq.ii++){
          rc = sqlite3BtreeSavepoint(db->aDb[u.aq.ii].pBt, u.aq.p1, u.aq.iSavepoint);
          if( rc!=SQLITE_OK ){
            goto abort_due_to_error;
          }
        }
        if( u.aq.p1==SAVEPOINT_ROLLBACK && (db->flags&SQLITE_InternChanges)!=0 ){
          sqlite3ExpirePreparedStatements(db);
          sqlite3ResetInternalSchema(db, -1);
          db->flags = (db->flags | SQLITE_InternChanges);
        }
      }

      /* Regardless of whether this is a RELEASE or ROLLBACK, destroy all
      ** savepoints nested inside of the savepoint being operated on. */
      while( db->pSavepoint!=u.aq.pSavepoint ){
        u.aq.pTmp = db->pSavepoint;
        db->pSavepoint = u.aq.pTmp->pNext;
        sqlite3DbFree(db, u.aq.pTmp);
        db->nSavepoint--;
      }

      /* If it is a RELEASE, then destroy the savepoint being operated on
      ** too. If it is a ROLLBACK TO, then set the number of deferred
      ** constraint violations present in the database to the value stored
      ** when the savepoint was created.  */
      if( u.aq.p1==SAVEPOINT_RELEASE ){
        assert( u.aq.pSavepoint==db->pSavepoint );
        db->pSavepoint = u.aq.pSavepoint->pNext;
        sqlite3DbFree(db, u.aq.pSavepoint);
        if( !isTransaction ){
          db->nSavepoint--;
        }
      }else{
        db->nDeferredCons = u.aq.pSavepoint->nDeferredCons;
      }

      if( !isTransaction ){
        rc = sqlite3VtabSavepoint(db, u.aq.p1, u.aq.iSavepoint);
        if( rc!=SQLITE_OK ) goto abort_due_to_error;
      }
    }
  }

  break;
}

/* Opcode: AutoCommit P1 P2 * * *
**
** Set the database auto-commit flag to P1 (1 or 0). If P2 is true, roll
** back any currently active btree transactions. If there are any active
** VMs (apart from this one), then a ROLLBACK fails.  A COMMIT fails if
** there are active writing VMs or active VMs that use shared cache.
**
** This instruction causes the VM to halt.
*/
case OP_AutoCommit: {
#if 0  /* local variables moved into u.ar */
  int desiredAutoCommit;
  int iRollback;
  int turnOnAC;
#endif /* local variables moved into u.ar */

  u.ar.desiredAutoCommit = pOp->p1;
  u.ar.iRollback = pOp->p2;
  u.ar.turnOnAC = u.ar.desiredAutoCommit && !db->autoCommit;
  assert( u.ar.desiredAutoCommit==1 || u.ar.desiredAutoCommit==0 );
  assert( u.ar.desiredAutoCommit==1 || u.ar.iRollback==0 );
  assert( db->activeVdbeCnt>0 );  /* At least this one VM is active */

  if( u.ar.turnOnAC && u.ar.iRollback && db->activeVdbeCnt>1 ){
    /* If this instruction implements a ROLLBACK and other VMs are
    ** still running, and a transaction is active, return an error indicating
    ** that the other VMs must complete first.
    */
    sqlite3SetString(&p->zErrMsg, db, "cannot rollback transaction - "
        "SQL statements in progress");
    rc = SQLITE_BUSY;
  }else if( u.ar.turnOnAC && !u.ar.iRollback && db->writeVdbeCnt>0 ){
    /* If this instruction implements a COMMIT and other VMs are writing
    ** return an error indicating that the other VMs must complete first.
    */
    sqlite3SetString(&p->zErrMsg, db, "cannot commit transaction - "
        "SQL statements in progress");
    rc = SQLITE_BUSY;
  }else if( u.ar.desiredAutoCommit!=db->autoCommit ){
    if( u.ar.iRollback ){
      assert( u.ar.desiredAutoCommit==1 );
      sqlite3RollbackAll(db);
      db->autoCommit = 1;
    }else if( (rc = sqlite3VdbeCheckFk(p, 1))!=SQLITE_OK ){
      goto vdbe_return;
    }else{
      db->autoCommit = (u8)u.ar.desiredAutoCommit;
      if( sqlite3VdbeHalt(p)==SQLITE_BUSY ){
        p->pc = pc;
        db->autoCommit = (u8)(1-u.ar.desiredAutoCommit);
        p->rc = rc = SQLITE_BUSY;
        goto vdbe_return;
      }
    }
    assert( db->nStatement==0 );
    sqlite3CloseSavepoints(db);
    if( p->rc==SQLITE_OK ){
      rc = SQLITE_DONE;
    }else{
      rc = SQLITE_ERROR;
    }
    goto vdbe_return;
  }else{
    sqlite3SetString(&p->zErrMsg, db,
        (!u.ar.desiredAutoCommit)?"cannot start a transaction within a transaction":(
        (u.ar.iRollback)?"cannot rollback - no transaction is active":
                   "cannot commit - no transaction is active"));

    rc = SQLITE_ERROR;
  }
  break;
}

/* Opcode: Transaction P1 P2 * * *
**
** Begin a transaction.  The transaction ends when a Commit or Rollback
** opcode is encountered.  Depending on the ON CONFLICT setting, the
** transaction might also be rolled back if an error is encountered.
**
** P1 is the index of the database file on which the transaction is
** started.  Index 0 is the main database file and index 1 is the
** file used for temporary tables.  Indices of 2 or more are used for
** attached databases.
**
** If P2 is non-zero, then a write-transaction is started.  A RESERVED lock is
** obtained on the database file when a write-transaction is started.  No
** other process can start another write transaction while this transaction is
** underway.  Starting a write transaction also creates a rollback journal. A
** write transaction must be started before any changes can be made to the
** database.  If P2 is 2 or greater then an EXCLUSIVE lock is also obtained
** on the file.
**
** If a write-transaction is started and the Vdbe.usesStmtJournal flag is
** true (this flag is set if the Vdbe may modify more than one row and may
** throw an ABORT exception), a statement transaction may also be opened.
** More specifically, a statement transaction is opened iff the database
** connection is currently not in autocommit mode, or if there are other
** active statements. A statement transaction allows the affects of this
** VDBE to be rolled back after an error without having to roll back the
** entire transaction. If no error is encountered, the statement transaction
** will automatically commit when the VDBE halts.
**
** If P2 is zero, then a read-lock is obtained on the database file.
*/
case OP_Transaction: {
#if 0  /* local variables moved into u.as */
  Btree *pBt;
#endif /* local variables moved into u.as */

  assert( pOp->p1>=0 && pOp->p1<db->nDb );
  assert( (p->btreeMask & (((yDbMask)1)<<pOp->p1))!=0 );
  u.as.pBt = db->aDb[pOp->p1].pBt;

  if( u.as.pBt ){
    rc = sqlite3BtreeBeginTrans(u.as.pBt, pOp->p2);
    if( rc==SQLITE_BUSY ){
      p->pc = pc;
      p->rc = rc = SQLITE_BUSY;
      goto vdbe_return;
    }
    if( rc!=SQLITE_OK ){
      goto abort_due_to_error;
    }

    if( pOp->p2 && p->usesStmtJournal
     && (db->autoCommit==0 || db->activeVdbeCnt>1)
    ){
      assert( sqlite3BtreeIsInTrans(u.as.pBt) );
      if( p->iStatement==0 ){
        assert( db->nStatement>=0 && db->nSavepoint>=0 );
        db->nStatement++;
        p->iStatement = db->nSavepoint + db->nStatement;
      }

      rc = sqlite3VtabSavepoint(db, SAVEPOINT_BEGIN, p->iStatement-1);
      if( rc==SQLITE_OK ){
        rc = sqlite3BtreeBeginStmt(u.as.pBt, p->iStatement);
      }

      /* Store the current value of the database handles deferred constraint
      ** counter. If the statement transaction needs to be rolled back,
      ** the value of this counter needs to be restored too.  */
      p->nStmtDefCons = db->nDeferredCons;
    }
  }
  break;
}

/* Opcode: ReadCookie P1 P2 P3 * *
**
** Read cookie number P3 from database P1 and write it into register P2.
** P3==1 is the schema version.  P3==2 is the database format.
** P3==3 is the recommended pager cache size, and so forth.  P1==0 is
** the main database file and P1==1 is the database file used to store
** temporary tables.
**
** There must be a read-lock on the database (either a transaction
** must be started or there must be an open cursor) before
** executing this instruction.
*/
case OP_ReadCookie: {               /* out2-prerelease */
#if 0  /* local variables moved into u.at */
  int iMeta;
  int iDb;
  int iCookie;
#endif /* local variables moved into u.at */

  u.at.iDb = pOp->p1;
  u.at.iCookie = pOp->p3;
  assert( pOp->p3<SQLITE_N_BTREE_META );
  assert( u.at.iDb>=0 && u.at.iDb<db->nDb );
  assert( db->aDb[u.at.iDb].pBt!=0 );
  assert( (p->btreeMask & (((yDbMask)1)<<u.at.iDb))!=0 );

  sqlite3BtreeGetMeta(db->aDb[u.at.iDb].pBt, u.at.iCookie, (u32 *)&u.at.iMeta);
  pOut->u.i = u.at.iMeta;
  break;
}

/* Opcode: SetCookie P1 P2 P3 * *
**
** Write the content of register P3 (interpreted as an integer)
** into cookie number P2 of database P1.  P2==1 is the schema version.  
** P2==2 is the database format. P2==3 is the recommended pager cache 
** size, and so forth.  P1==0 is the main database file and P1==1 is the 
** database file used to store temporary tables.
**
** A transaction must be started before executing this opcode.
*/
case OP_SetCookie: {       /* in3 */
#if 0  /* local variables moved into u.au */
  Db *pDb;
#endif /* local variables moved into u.au */
  assert( pOp->p2<SQLITE_N_BTREE_META );
  assert( pOp->p1>=0 && pOp->p1<db->nDb );
  assert( (p->btreeMask & (((yDbMask)1)<<pOp->p1))!=0 );
  u.au.pDb = &db->aDb[pOp->p1];
  assert( u.au.pDb->pBt!=0 );
  assert( sqlite3SchemaMutexHeld(db, pOp->p1, 0) );
  pIn3 = &aMem[pOp->p3];
  sqlite3VdbeMemIntegerify(pIn3);
  /* See note about index shifting on OP_ReadCookie */
  rc = sqlite3BtreeUpdateMeta(u.au.pDb->pBt, pOp->p2, (int)pIn3->u.i);
  if( pOp->p2==BTREE_SCHEMA_VERSION ){
    /* When the schema cookie changes, record the new cookie internally */
    u.au.pDb->pSchema->schema_cookie = (int)pIn3->u.i;
    db->flags |= SQLITE_InternChanges;
  }else if( pOp->p2==BTREE_FILE_FORMAT ){
    /* Record changes in the file format */
    u.au.pDb->pSchema->file_format = (u8)pIn3->u.i;
  }
  if( pOp->p1==1 ){
    /* Invalidate all prepared statements whenever the TEMP database
    ** schema is changed.  Ticket #1644 */
    sqlite3ExpirePreparedStatements(db);
    p->expired = 0;
  }
  break;
}

/* Opcode: VerifyCookie P1 P2 P3 * *
**
** Check the value of global database parameter number 0 (the
** schema version) and make sure it is equal to P2 and that the
** generation counter on the local schema parse equals P3.
**
** P1 is the database number which is 0 for the main database file
** and 1 for the file holding temporary tables and some higher number
** for auxiliary databases.
**
** The cookie changes its value whenever the database schema changes.
** This operation is used to detect when that the cookie has changed
** and that the current process needs to reread the schema.
**
** Either a transaction needs to have been started or an OP_Open needs
** to be executed (to establish a read lock) before this opcode is
** invoked.
*/
case OP_VerifyCookie: {
#if 0  /* local variables moved into u.av */
  int iMeta;
  int iGen;
  Btree *pBt;
#endif /* local variables moved into u.av */

  assert( pOp->p1>=0 && pOp->p1<db->nDb );
  assert( (p->btreeMask & (((yDbMask)1)<<pOp->p1))!=0 );
  assert( sqlite3SchemaMutexHeld(db, pOp->p1, 0) );
  u.av.pBt = db->aDb[pOp->p1].pBt;
  if( u.av.pBt ){
    sqlite3BtreeGetMeta(u.av.pBt, BTREE_SCHEMA_VERSION, (u32 *)&u.av.iMeta);
    u.av.iGen = db->aDb[pOp->p1].pSchema->iGeneration;
  }else{
    u.av.iGen = u.av.iMeta = 0;
  }
  if( u.av.iMeta!=pOp->p2 || u.av.iGen!=pOp->p3 ){
    sqlite3DbFree(db, p->zErrMsg);
    p->zErrMsg = sqlite3DbStrDup(db, "database schema has changed");
    /* If the schema-cookie from the database file matches the cookie
    ** stored with the in-memory representation of the schema, do
    ** not reload the schema from the database file.
    **
    ** If virtual-tables are in use, this is not just an optimization.
    ** Often, v-tables store their data in other SQLite tables, which
    ** are queried from within xNext() and other v-table methods using
    ** prepared queries. If such a query is out-of-date, we do not want to
    ** discard the database schema, as the user code implementing the
    ** v-table would have to be ready for the sqlite3_vtab structure itself
    ** to be invalidated whenever sqlite3_step() is called from within
    ** a v-table method.
    */
    if( db->aDb[pOp->p1].pSchema->schema_cookie!=u.av.iMeta ){
      sqlite3ResetInternalSchema(db, pOp->p1);
    }

    p->expired = 1;
    rc = SQLITE_SCHEMA;
  }
  break;
}

/* Opcode: OpenRead P1 P2 P3 P4 P5
**
** Open a read-only cursor for the database table whose root page is
** P2 in a database file.  The database file is determined by P3. 
** P3==0 means the main database, P3==1 means the database used for 
** temporary tables, and P3>1 means used the corresponding attached
** database.  Give the new cursor an identifier of P1.  The P1
** values need not be contiguous but all P1 values should be small integers.
** It is an error for P1 to be negative.
**
** If P5!=0 then use the content of register P2 as the root page, not
** the value of P2 itself.
**
** There will be a read lock on the database whenever there is an
** open cursor.  If the database was unlocked prior to this instruction
** then a read lock is acquired as part of this instruction.  A read
** lock allows other processes to read the database but prohibits
** any other process from modifying the database.  The read lock is
** released when all cursors are closed.  If this instruction attempts
** to get a read lock but fails, the script terminates with an
** SQLITE_BUSY error code.
**
** The P4 value may be either an integer (P4_INT32) or a pointer to
** a KeyInfo structure (P4_KEYINFO). If it is a pointer to a KeyInfo 
** structure, then said structure defines the content and collating 
** sequence of the index being opened. Otherwise, if P4 is an integer 
** value, it is set to the number of columns in the table.
**
** See also OpenWrite.
*/
/* Opcode: OpenWrite P1 P2 P3 P4 P5
**
** Open a read/write cursor named P1 on the table or index whose root
** page is P2.  Or if P5!=0 use the content of register P2 to find the
** root page.
**
** The P4 value may be either an integer (P4_INT32) or a pointer to
** a KeyInfo structure (P4_KEYINFO). If it is a pointer to a KeyInfo 
** structure, then said structure defines the content and collating 
** sequence of the index being opened. Otherwise, if P4 is an integer 
** value, it is set to the number of columns in the table, or to the
** largest index of any column of the table that is actually used.
**
** This instruction works just like OpenRead except that it opens the cursor
** in read/write mode.  For a given table, there can be one or more read-only
** cursors or a single read/write cursor but not both.
**
** See also OpenRead.
*/
case OP_OpenRead:
case OP_OpenWrite: {
#if 0  /* local variables moved into u.aw */
  int nField;
  KeyInfo *pKeyInfo;
  int p2;
  int iDb;
  int wrFlag;
  Btree *pX;
  VdbeCursor *pCur;
  Db *pDb;
#endif /* local variables moved into u.aw */

  if( p->expired ){
    rc = SQLITE_ABORT;
    break;
  }

  u.aw.nField = 0;
  u.aw.pKeyInfo = 0;
  u.aw.p2 = pOp->p2;
  u.aw.iDb = pOp->p3;
  assert( u.aw.iDb>=0 && u.aw.iDb<db->nDb );
  assert( (p->btreeMask & (((yDbMask)1)<<u.aw.iDb))!=0 );
  u.aw.pDb = &db->aDb[u.aw.iDb];
  u.aw.pX = u.aw.pDb->pBt;
  assert( u.aw.pX!=0 );
  if( pOp->opcode==OP_OpenWrite ){
    u.aw.wrFlag = 1;
    assert( sqlite3SchemaMutexHeld(db, u.aw.iDb, 0) );
    if( u.aw.pDb->pSchema->file_format < p->minWriteFileFormat ){
      p->minWriteFileFormat = u.aw.pDb->pSchema->file_format;
    }
  }else{
    u.aw.wrFlag = 0;
  }
  if( pOp->p5 ){
    assert( u.aw.p2>0 );
    assert( u.aw.p2<=p->nMem );
    pIn2 = &aMem[u.aw.p2];
    assert( memIsValid(pIn2) );
    assert( (pIn2->flags & MEM_Int)!=0 );
    sqlite3VdbeMemIntegerify(pIn2);
    u.aw.p2 = (int)pIn2->u.i;
    /* The u.aw.p2 value always comes from a prior OP_CreateTable opcode and
    ** that opcode will always set the u.aw.p2 value to 2 or more or else fail.
    ** If there were a failure, the prepared statement would have halted
    ** before reaching this instruction. */
    if( NEVER(u.aw.p2<2) ) {
      rc = SQLITE_CORRUPT_BKPT;
      goto abort_due_to_error;
    }
  }
  if( pOp->p4type==P4_KEYINFO ){
    u.aw.pKeyInfo = pOp->p4.pKeyInfo;
    u.aw.pKeyInfo->enc = ENC(p->db);
    u.aw.nField = u.aw.pKeyInfo->nField+1;
  }else if( pOp->p4type==P4_INT32 ){
    u.aw.nField = pOp->p4.i;
  }
  assert( pOp->p1>=0 );
  u.aw.pCur = allocateCursor(p, pOp->p1, u.aw.nField, u.aw.iDb, 1);
  if( u.aw.pCur==0 ) goto no_mem;
  u.aw.pCur->nullRow = 1;
  u.aw.pCur->isOrdered = 1;
  rc = sqlite3BtreeCursor(u.aw.pX, u.aw.p2, u.aw.wrFlag, u.aw.pKeyInfo, u.aw.pCur->pCursor);
  u.aw.pCur->pKeyInfo = u.aw.pKeyInfo;

  /* Since it performs no memory allocation or IO, the only values that
  ** sqlite3BtreeCursor() may return are SQLITE_EMPTY and SQLITE_OK.
  ** SQLITE_EMPTY is only returned when attempting to open the table
  ** rooted at page 1 of a zero-byte database.  */
  assert( rc==SQLITE_EMPTY || rc==SQLITE_OK );
  if( rc==SQLITE_EMPTY ){
    u.aw.pCur->pCursor = 0;
    rc = SQLITE_OK;
  }

  /* Set the VdbeCursor.isTable and isIndex variables. Previous versions of
  ** SQLite used to check if the root-page flags were sane at this point
  ** and report database corruption if they were not, but this check has
  ** since moved into the btree layer.  */
  u.aw.pCur->isTable = pOp->p4type!=P4_KEYINFO;
  u.aw.pCur->isIndex = !u.aw.pCur->isTable;
  break;
}

/* Opcode: OpenEphemeral P1 P2 * P4 *
**
** Open a new cursor P1 to a transient table.
** The cursor is always opened read/write even if 
** the main database is read-only.  The ephemeral
** table is deleted automatically when the cursor is closed.
**
** P2 is the number of columns in the ephemeral table.
** The cursor points to a BTree table if P4==0 and to a BTree index
** if P4 is not 0.  If P4 is not NULL, it points to a KeyInfo structure
** that defines the format of keys in the index.
**
** This opcode was once called OpenTemp.  But that created
** confusion because the term "temp table", might refer either
** to a TEMP table at the SQL level, or to a table opened by
** this opcode.  Then this opcode was call OpenVirtual.  But
** that created confusion with the whole virtual-table idea.
*/
/* Opcode: OpenAutoindex P1 P2 * P4 *
**
** This opcode works the same as OP_OpenEphemeral.  It has a
** different name to distinguish its use.  Tables created using
** by this opcode will be used for automatically created transient
** indices in joins.
*/
case OP_OpenAutoindex: 
case OP_OpenEphemeral: {
#if 0  /* local variables moved into u.ax */
  VdbeCursor *pCx;
#endif /* local variables moved into u.ax */
  static const int vfsFlags =
      SQLITE_OPEN_READWRITE |
      SQLITE_OPEN_CREATE |
      SQLITE_OPEN_EXCLUSIVE |
      SQLITE_OPEN_DELETEONCLOSE |
      SQLITE_OPEN_TRANSIENT_DB;

  assert( pOp->p1>=0 );
  u.ax.pCx = allocateCursor(p, pOp->p1, pOp->p2, -1, 1);
  if( u.ax.pCx==0 ) goto no_mem;
  u.ax.pCx->nullRow = 1;
  rc = sqlite3BtreeOpen(db->pVfs, 0, db, &u.ax.pCx->pBt,
                        BTREE_OMIT_JOURNAL | BTREE_SINGLE | pOp->p5, vfsFlags);
  if( rc==SQLITE_OK ){
    rc = sqlite3BtreeBeginTrans(u.ax.pCx->pBt, 1);
  }
  if( rc==SQLITE_OK ){
    /* If a transient index is required, create it by calling
    ** sqlite3BtreeCreateTable() with the BTREE_BLOBKEY flag before
    ** opening it. If a transient table is required, just use the
    ** automatically created table with root-page 1 (an BLOB_INTKEY table).
    */
    if( pOp->p4.pKeyInfo ){
      int pgno;
      assert( pOp->p4type==P4_KEYINFO );
      rc = sqlite3BtreeCreateTable(u.ax.pCx->pBt, &pgno, BTREE_BLOBKEY);
      if( rc==SQLITE_OK ){
        assert( pgno==MASTER_ROOT+1 );
        rc = sqlite3BtreeCursor(u.ax.pCx->pBt, pgno, 1,
                                (KeyInfo*)pOp->p4.z, u.ax.pCx->pCursor);
        u.ax.pCx->pKeyInfo = pOp->p4.pKeyInfo;
        u.ax.pCx->pKeyInfo->enc = ENC(p->db);
      }
      u.ax.pCx->isTable = 0;
    }else{
      rc = sqlite3BtreeCursor(u.ax.pCx->pBt, MASTER_ROOT, 1, 0, u.ax.pCx->pCursor);
      u.ax.pCx->isTable = 1;
    }
  }
  u.ax.pCx->isOrdered = (pOp->p5!=BTREE_UNORDERED);
  u.ax.pCx->isIndex = !u.ax.pCx->isTable;
  break;
}

/* Opcode: OpenPseudo P1 P2 P3 * *
**
** Open a new cursor that points to a fake table that contains a single
** row of data.  The content of that one row in the content of memory
** register P2.  In other words, cursor P1 becomes an alias for the 
** MEM_Blob content contained in register P2.
**
** A pseudo-table created by this opcode is used to hold a single
** row output from the sorter so that the row can be decomposed into
** individual columns using the OP_Column opcode.  The OP_Column opcode
** is the only cursor opcode that works with a pseudo-table.
**
** P3 is the number of fields in the records that will be stored by
** the pseudo-table.
*/
case OP_OpenPseudo: {
#if 0  /* local variables moved into u.ay */
  VdbeCursor *pCx;
#endif /* local variables moved into u.ay */

  assert( pOp->p1>=0 );
  u.ay.pCx = allocateCursor(p, pOp->p1, pOp->p3, -1, 0);
  if( u.ay.pCx==0 ) goto no_mem;
  u.ay.pCx->nullRow = 1;
  u.ay.pCx->pseudoTableReg = pOp->p2;
  u.ay.pCx->isTable = 1;
  u.ay.pCx->isIndex = 0;
  break;
}

/* Opcode: Close P1 * * * *
**
** Close a cursor previously opened as P1.  If P1 is not
** currently open, this instruction is a no-op.
*/
case OP_Close: {
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  sqlite3VdbeFreeCursor(p, p->apCsr[pOp->p1]);
  p->apCsr[pOp->p1] = 0;
  break;
}

/* Opcode: SeekGe P1 P2 P3 P4 *
**
** If cursor P1 refers to an SQL table (B-Tree that uses integer keys), 
** use the value in register P3 as the key.  If cursor P1 refers 
** to an SQL index, then P3 is the first in an array of P4 registers 
** that are used as an unpacked index key. 
**
** Reposition cursor P1 so that  it points to the smallest entry that 
** is greater than or equal to the key value. If there are no records 
** greater than or equal to the key and P2 is not zero, then jump to P2.
**
** See also: Found, NotFound, Distinct, SeekLt, SeekGt, SeekLe
*/
/* Opcode: SeekGt P1 P2 P3 P4 *
**
** If cursor P1 refers to an SQL table (B-Tree that uses integer keys), 
** use the value in register P3 as a key. If cursor P1 refers 
** to an SQL index, then P3 is the first in an array of P4 registers 
** that are used as an unpacked index key. 
**
** Reposition cursor P1 so that  it points to the smallest entry that 
** is greater than the key value. If there are no records greater than 
** the key and P2 is not zero, then jump to P2.
**
** See also: Found, NotFound, Distinct, SeekLt, SeekGe, SeekLe
*/
/* Opcode: SeekLt P1 P2 P3 P4 * 
**
** If cursor P1 refers to an SQL table (B-Tree that uses integer keys), 
** use the value in register P3 as a key. If cursor P1 refers 
** to an SQL index, then P3 is the first in an array of P4 registers 
** that are used as an unpacked index key. 
**
** Reposition cursor P1 so that  it points to the largest entry that 
** is less than the key value. If there are no records less than 
** the key and P2 is not zero, then jump to P2.
**
** See also: Found, NotFound, Distinct, SeekGt, SeekGe, SeekLe
*/
/* Opcode: SeekLe P1 P2 P3 P4 *
**
** If cursor P1 refers to an SQL table (B-Tree that uses integer keys), 
** use the value in register P3 as a key. If cursor P1 refers 
** to an SQL index, then P3 is the first in an array of P4 registers 
** that are used as an unpacked index key. 
**
** Reposition cursor P1 so that it points to the largest entry that 
** is less than or equal to the key value. If there are no records 
** less than or equal to the key and P2 is not zero, then jump to P2.
**
** See also: Found, NotFound, Distinct, SeekGt, SeekGe, SeekLt
*/
case OP_SeekLt:         /* jump, in3 */
case OP_SeekLe:         /* jump, in3 */
case OP_SeekGe:         /* jump, in3 */
case OP_SeekGt: {       /* jump, in3 */
#if 0  /* local variables moved into u.az */
  int res;
  int oc;
  VdbeCursor *pC;
  UnpackedRecord r;
  int nField;
  i64 iKey;      /* The rowid we are to seek to */
#endif /* local variables moved into u.az */

  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  assert( pOp->p2!=0 );
  u.az.pC = p->apCsr[pOp->p1];
  assert( u.az.pC!=0 );
  assert( u.az.pC->pseudoTableReg==0 );
  assert( OP_SeekLe == OP_SeekLt+1 );
  assert( OP_SeekGe == OP_SeekLt+2 );
  assert( OP_SeekGt == OP_SeekLt+3 );
  assert( u.az.pC->isOrdered );
  if( u.az.pC->pCursor!=0 ){
    u.az.oc = pOp->opcode;
    u.az.pC->nullRow = 0;
    if( u.az.pC->isTable ){
      /* The input value in P3 might be of any type: integer, real, string,
      ** blob, or NULL.  But it needs to be an integer before we can do
      ** the seek, so covert it. */
      pIn3 = &aMem[pOp->p3];
      applyNumericAffinity(pIn3);
      u.az.iKey = sqlite3VdbeIntValue(pIn3);
      u.az.pC->rowidIsValid = 0;

      /* If the P3 value could not be converted into an integer without
      ** loss of information, then special processing is required... */
      if( (pIn3->flags & MEM_Int)==0 ){
        if( (pIn3->flags & MEM_Real)==0 ){
          /* If the P3 value cannot be converted into any kind of a number,
          ** then the seek is not possible, so jump to P2 */
          pc = pOp->p2 - 1;
          break;
        }
        /* If we reach this point, then the P3 value must be a floating
        ** point number. */
        assert( (pIn3->flags & MEM_Real)!=0 );

        if( u.az.iKey==SMALLEST_INT64 && (pIn3->r<(double)u.az.iKey || pIn3->r>0) ){
          /* The P3 value is too large in magnitude to be expressed as an
          ** integer. */
          u.az.res = 1;
          if( pIn3->r<0 ){
            if( u.az.oc>=OP_SeekGe ){  assert( u.az.oc==OP_SeekGe || u.az.oc==OP_SeekGt );
              rc = sqlite3BtreeFirst(u.az.pC->pCursor, &u.az.res);
              if( rc!=SQLITE_OK ) goto abort_due_to_error;
            }
          }else{
            if( u.az.oc<=OP_SeekLe ){  assert( u.az.oc==OP_SeekLt || u.az.oc==OP_SeekLe );
              rc = sqlite3BtreeLast(u.az.pC->pCursor, &u.az.res);
              if( rc!=SQLITE_OK ) goto abort_due_to_error;
            }
          }
          if( u.az.res ){
            pc = pOp->p2 - 1;
          }
          break;
        }else if( u.az.oc==OP_SeekLt || u.az.oc==OP_SeekGe ){
          /* Use the ceiling() function to convert real->int */
          if( pIn3->r > (double)u.az.iKey ) u.az.iKey++;
        }else{
          /* Use the floor() function to convert real->int */
          assert( u.az.oc==OP_SeekLe || u.az.oc==OP_SeekGt );
          if( pIn3->r < (double)u.az.iKey ) u.az.iKey--;
        }
      }
      rc = sqlite3BtreeMovetoUnpacked(u.az.pC->pCursor, 0, (u64)u.az.iKey, 0, &u.az.res);
      if( rc!=SQLITE_OK ){
        goto abort_due_to_error;
      }
      if( u.az.res==0 ){
        u.az.pC->rowidIsValid = 1;
        u.az.pC->lastRowid = u.az.iKey;
      }
    }else{
      u.az.nField = pOp->p4.i;
      assert( pOp->p4type==P4_INT32 );
      assert( u.az.nField>0 );
      u.az.r.pKeyInfo = u.az.pC->pKeyInfo;
      u.az.r.nField = (u16)u.az.nField;

      /* The next line of code computes as follows, only faster:
      **   if( u.az.oc==OP_SeekGt || u.az.oc==OP_SeekLe ){
      **     u.az.r.flags = UNPACKED_INCRKEY;
      **   }else{
      **     u.az.r.flags = 0;
      **   }
      */
      u.az.r.flags = (u16)(UNPACKED_INCRKEY * (1 & (u.az.oc - OP_SeekLt)));
      assert( u.az.oc!=OP_SeekGt || u.az.r.flags==UNPACKED_INCRKEY );
      assert( u.az.oc!=OP_SeekLe || u.az.r.flags==UNPACKED_INCRKEY );
      assert( u.az.oc!=OP_SeekGe || u.az.r.flags==0 );
      assert( u.az.oc!=OP_SeekLt || u.az.r.flags==0 );

      u.az.r.aMem = &aMem[pOp->p3];
#ifdef SQLITE_DEBUG
      { int i; for(i=0; i<u.az.r.nField; i++) assert( memIsValid(&u.az.r.aMem[i]) ); }
#endif
      ExpandBlob(u.az.r.aMem);
      rc = sqlite3BtreeMovetoUnpacked(u.az.pC->pCursor, &u.az.r, 0, 0, &u.az.res);
      if( rc!=SQLITE_OK ){
        goto abort_due_to_error;
      }
      u.az.pC->rowidIsValid = 0;
    }
    u.az.pC->deferredMoveto = 0;
    u.az.pC->cacheStatus = CACHE_STALE;
#ifdef SQLITE_TEST
    sqlite3_search_count++;
#endif
    if( u.az.oc>=OP_SeekGe ){  assert( u.az.oc==OP_SeekGe || u.az.oc==OP_SeekGt );
      if( u.az.res<0 || (u.az.res==0 && u.az.oc==OP_SeekGt) ){
        rc = sqlite3BtreeNext(u.az.pC->pCursor, &u.az.res);
        if( rc!=SQLITE_OK ) goto abort_due_to_error;
        u.az.pC->rowidIsValid = 0;
      }else{
        u.az.res = 0;
      }
    }else{
      assert( u.az.oc==OP_SeekLt || u.az.oc==OP_SeekLe );
      if( u.az.res>0 || (u.az.res==0 && u.az.oc==OP_SeekLt) ){
        rc = sqlite3BtreePrevious(u.az.pC->pCursor, &u.az.res);
        if( rc!=SQLITE_OK ) goto abort_due_to_error;
        u.az.pC->rowidIsValid = 0;
      }else{
        /* u.az.res might be negative because the table is empty.  Check to
        ** see if this is the case.
        */
        u.az.res = sqlite3BtreeEof(u.az.pC->pCursor);
      }
    }
    assert( pOp->p2>0 );
    if( u.az.res ){
      pc = pOp->p2 - 1;
    }
  }else{
    /* This happens when attempting to open the sqlite3_master table
    ** for read access returns SQLITE_EMPTY. In this case always
    ** take the jump (since there are no records in the table).
    */
    pc = pOp->p2 - 1;
  }
  break;
}

/* Opcode: Seek P1 P2 * * *
**
** P1 is an open table cursor and P2 is a rowid integer.  Arrange
** for P1 to move so that it points to the rowid given by P2.
**
** This is actually a deferred seek.  Nothing actually happens until
** the cursor is used to read a record.  That way, if no reads
** occur, no unnecessary I/O happens.
*/
case OP_Seek: {    /* in2 */
#if 0  /* local variables moved into u.ba */
  VdbeCursor *pC;
#endif /* local variables moved into u.ba */

  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.ba.pC = p->apCsr[pOp->p1];
  assert( u.ba.pC!=0 );
  if( ALWAYS(u.ba.pC->pCursor!=0) ){
    assert( u.ba.pC->isTable );
    u.ba.pC->nullRow = 0;
    pIn2 = &aMem[pOp->p2];
    u.ba.pC->movetoTarget = sqlite3VdbeIntValue(pIn2);
    u.ba.pC->rowidIsValid = 0;
    u.ba.pC->deferredMoveto = 1;
  }
  break;
}
  

/* Opcode: Found P1 P2 P3 P4 *
**
** If P4==0 then register P3 holds a blob constructed by MakeRecord.  If
** P4>0 then register P3 is the first of P4 registers that form an unpacked
** record.
**
** Cursor P1 is on an index btree.  If the record identified by P3 and P4
** is a prefix of any entry in P1 then a jump is made to P2 and
** P1 is left pointing at the matching entry.
*/
/* Opcode: NotFound P1 P2 P3 P4 *
**
** If P4==0 then register P3 holds a blob constructed by MakeRecord.  If
** P4>0 then register P3 is the first of P4 registers that form an unpacked
** record.
** 
** Cursor P1 is on an index btree.  If the record identified by P3 and P4
** is not the prefix of any entry in P1 then a jump is made to P2.  If P1 
** does contain an entry whose prefix matches the P3/P4 record then control
** falls through to the next instruction and P1 is left pointing at the
** matching entry.
**
** See also: Found, NotExists, IsUnique
*/
case OP_NotFound:       /* jump, in3 */
case OP_Found: {        /* jump, in3 */
#if 0  /* local variables moved into u.bb */
  int alreadyExists;
  VdbeCursor *pC;
  int res;
  UnpackedRecord *pIdxKey;
  UnpackedRecord r;
  char aTempRec[ROUND8(sizeof(UnpackedRecord)) + sizeof(Mem)*3 + 7];
#endif /* local variables moved into u.bb */

#ifdef SQLITE_TEST
  sqlite3_found_count++;
#endif

  u.bb.alreadyExists = 0;
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  assert( pOp->p4type==P4_INT32 );
  u.bb.pC = p->apCsr[pOp->p1];
  assert( u.bb.pC!=0 );
  pIn3 = &aMem[pOp->p3];
  if( ALWAYS(u.bb.pC->pCursor!=0) ){

    assert( u.bb.pC->isTable==0 );
    if( pOp->p4.i>0 ){
      u.bb.r.pKeyInfo = u.bb.pC->pKeyInfo;
      u.bb.r.nField = (u16)pOp->p4.i;
      u.bb.r.aMem = pIn3;
#ifdef SQLITE_DEBUG
      { int i; for(i=0; i<u.bb.r.nField; i++) assert( memIsValid(&u.bb.r.aMem[i]) ); }
#endif
      u.bb.r.flags = UNPACKED_PREFIX_MATCH;
      u.bb.pIdxKey = &u.bb.r;
    }else{
      assert( pIn3->flags & MEM_Blob );
      assert( (pIn3->flags & MEM_Zero)==0 );  /* zeroblobs already expanded */
      u.bb.pIdxKey = sqlite3VdbeRecordUnpack(u.bb.pC->pKeyInfo, pIn3->n, pIn3->z,
                                        u.bb.aTempRec, sizeof(u.bb.aTempRec));
      if( u.bb.pIdxKey==0 ){
        goto no_mem;
      }
      u.bb.pIdxKey->flags |= UNPACKED_PREFIX_MATCH;
    }
    rc = sqlite3BtreeMovetoUnpacked(u.bb.pC->pCursor, u.bb.pIdxKey, 0, 0, &u.bb.res);
    if( pOp->p4.i==0 ){
      sqlite3VdbeDeleteUnpackedRecord(u.bb.pIdxKey);
    }
    if( rc!=SQLITE_OK ){
      break;
    }
    u.bb.alreadyExists = (u.bb.res==0);
    u.bb.pC->deferredMoveto = 0;
    u.bb.pC->cacheStatus = CACHE_STALE;
  }
  if( pOp->opcode==OP_Found ){
    if( u.bb.alreadyExists ) pc = pOp->p2 - 1;
  }else{
    if( !u.bb.alreadyExists ) pc = pOp->p2 - 1;
  }
  break;
}

/* Opcode: IsUnique P1 P2 P3 P4 *
**
** Cursor P1 is open on an index b-tree - that is to say, a btree which
** no data and where the key are records generated by OP_MakeRecord with
** the list field being the integer ROWID of the entry that the index
** entry refers to.
**
** The P3 register contains an integer record number. Call this record 
** number R. Register P4 is the first in a set of N contiguous registers
** that make up an unpacked index key that can be used with cursor P1.
** The value of N can be inferred from the cursor. N includes the rowid
** value appended to the end of the index record. This rowid value may
** or may not be the same as R.
**
** If any of the N registers beginning with register P4 contains a NULL
** value, jump immediately to P2.
**
** Otherwise, this instruction checks if cursor P1 contains an entry
** where the first (N-1) fields match but the rowid value at the end
** of the index entry is not R. If there is no such entry, control jumps
** to instruction P2. Otherwise, the rowid of the conflicting index
** entry is copied to register P3 and control falls through to the next
** instruction.
**
** See also: NotFound, NotExists, Found
*/
case OP_IsUnique: {        /* jump, in3 */
#if 0  /* local variables moved into u.bc */
  u16 ii;
  VdbeCursor *pCx;
  BtCursor *pCrsr;
  u16 nField;
  Mem *aMx;
  UnpackedRecord r;                  /* B-Tree index search key */
  i64 R;                             /* Rowid stored in register P3 */
#endif /* local variables moved into u.bc */

  pIn3 = &aMem[pOp->p3];
  u.bc.aMx = &aMem[pOp->p4.i];
  /* Assert that the values of parameters P1 and P4 are in range. */
  assert( pOp->p4type==P4_INT32 );
  assert( pOp->p4.i>0 && pOp->p4.i<=p->nMem );
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );

  /* Find the index cursor. */
  u.bc.pCx = p->apCsr[pOp->p1];
  assert( u.bc.pCx->deferredMoveto==0 );
  u.bc.pCx->seekResult = 0;
  u.bc.pCx->cacheStatus = CACHE_STALE;
  u.bc.pCrsr = u.bc.pCx->pCursor;

  /* If any of the values are NULL, take the jump. */
  u.bc.nField = u.bc.pCx->pKeyInfo->nField;
  for(u.bc.ii=0; u.bc.ii<u.bc.nField; u.bc.ii++){
    if( u.bc.aMx[u.bc.ii].flags & MEM_Null ){
      pc = pOp->p2 - 1;
      u.bc.pCrsr = 0;
      break;
    }
  }
  assert( (u.bc.aMx[u.bc.nField].flags & MEM_Null)==0 );

  if( u.bc.pCrsr!=0 ){
    /* Populate the index search key. */
    u.bc.r.pKeyInfo = u.bc.pCx->pKeyInfo;
    u.bc.r.nField = u.bc.nField + 1;
    u.bc.r.flags = UNPACKED_PREFIX_SEARCH;
    u.bc.r.aMem = u.bc.aMx;
#ifdef SQLITE_DEBUG
    { int i; for(i=0; i<u.bc.r.nField; i++) assert( memIsValid(&u.bc.r.aMem[i]) ); }
#endif

    /* Extract the value of u.bc.R from register P3. */
    sqlite3VdbeMemIntegerify(pIn3);
    u.bc.R = pIn3->u.i;

    /* Search the B-Tree index. If no conflicting record is found, jump
    ** to P2. Otherwise, copy the rowid of the conflicting record to
    ** register P3 and fall through to the next instruction.  */
    rc = sqlite3BtreeMovetoUnpacked(u.bc.pCrsr, &u.bc.r, 0, 0, &u.bc.pCx->seekResult);
    if( (u.bc.r.flags & UNPACKED_PREFIX_SEARCH) || u.bc.r.rowid==u.bc.R ){
      pc = pOp->p2 - 1;
    }else{
      pIn3->u.i = u.bc.r.rowid;
    }
  }
  break;
}

/* Opcode: NotExists P1 P2 P3 * *
**
** Use the content of register P3 as an integer key.  If a record 
** with that key does not exist in table of P1, then jump to P2. 
** If the record does exist, then fall through.  The cursor is left 
** pointing to the record if it exists.
**
** The difference between this operation and NotFound is that this
** operation assumes the key is an integer and that P1 is a table whereas
** NotFound assumes key is a blob constructed from MakeRecord and
** P1 is an index.
**
** See also: Found, NotFound, IsUnique
*/
case OP_NotExists: {        /* jump, in3 */
#if 0  /* local variables moved into u.bd */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  int res;
  u64 iKey;
#endif /* local variables moved into u.bd */

  pIn3 = &aMem[pOp->p3];
  assert( pIn3->flags & MEM_Int );
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bd.pC = p->apCsr[pOp->p1];
  assert( u.bd.pC!=0 );
  assert( u.bd.pC->isTable );
  assert( u.bd.pC->pseudoTableReg==0 );
  u.bd.pCrsr = u.bd.pC->pCursor;
  if( u.bd.pCrsr!=0 ){
    u.bd.res = 0;
    u.bd.iKey = pIn3->u.i;
    rc = sqlite3BtreeMovetoUnpacked(u.bd.pCrsr, 0, u.bd.iKey, 0, &u.bd.res);
    u.bd.pC->lastRowid = pIn3->u.i;
    u.bd.pC->rowidIsValid = u.bd.res==0 ?1:0;
    u.bd.pC->nullRow = 0;
    u.bd.pC->cacheStatus = CACHE_STALE;
    u.bd.pC->deferredMoveto = 0;
    if( u.bd.res!=0 ){
      pc = pOp->p2 - 1;
      assert( u.bd.pC->rowidIsValid==0 );
    }
    u.bd.pC->seekResult = u.bd.res;
  }else{
    /* This happens when an attempt to open a read cursor on the
    ** sqlite_master table returns SQLITE_EMPTY.
    */
    pc = pOp->p2 - 1;
    assert( u.bd.pC->rowidIsValid==0 );
    u.bd.pC->seekResult = 0;
  }
  break;
}

/* Opcode: Sequence P1 P2 * * *
**
** Find the next available sequence number for cursor P1.
** Write the sequence number into register P2.
** The sequence number on the cursor is incremented after this
** instruction.  
*/
case OP_Sequence: {           /* out2-prerelease */
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  assert( p->apCsr[pOp->p1]!=0 );
  pOut->u.i = p->apCsr[pOp->p1]->seqCount++;
  break;
}


/* Opcode: NewRowid P1 P2 P3 * *
**
** Get a new integer record number (a.k.a "rowid") used as the key to a table.
** The record number is not previously used as a key in the database
** table that cursor P1 points to.  The new record number is written
** written to register P2.
**
** If P3>0 then P3 is a register in the root frame of this VDBE that holds 
** the largest previously generated record number. No new record numbers are
** allowed to be less than this value. When this value reaches its maximum, 
** an SQLITE_FULL error is generated. The P3 register is updated with the '
** generated record number. This P3 mechanism is used to help implement the
** AUTOINCREMENT feature.
*/
case OP_NewRowid: {           /* out2-prerelease */
#if 0  /* local variables moved into u.be */
  i64 v;                 /* The new rowid */
  VdbeCursor *pC;        /* Cursor of table to get the new rowid */
  int res;               /* Result of an sqlite3BtreeLast() */
  int cnt;               /* Counter to limit the number of searches */
  Mem *pMem;             /* Register holding largest rowid for AUTOINCREMENT */
  VdbeFrame *pFrame;     /* Root frame of VDBE */
#endif /* local variables moved into u.be */

  u.be.v = 0;
  u.be.res = 0;
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.be.pC = p->apCsr[pOp->p1];
  assert( u.be.pC!=0 );
  if( NEVER(u.be.pC->pCursor==0) ){
    /* The zero initialization above is all that is needed */
  }else{
    /* The next rowid or record number (different terms for the same
    ** thing) is obtained in a two-step algorithm.
    **
    ** First we attempt to find the largest existing rowid and add one
    ** to that.  But if the largest existing rowid is already the maximum
    ** positive integer, we have to fall through to the second
    ** probabilistic algorithm
    **
    ** The second algorithm is to select a rowid at random and see if
    ** it already exists in the table.  If it does not exist, we have
    ** succeeded.  If the random rowid does exist, we select a new one
    ** and try again, up to 100 times.
    */
    assert( u.be.pC->isTable );

#ifdef SQLITE_32BIT_ROWID
#   define MAX_ROWID 0x7fffffff
#else
    /* Some compilers complain about constants of the form 0x7fffffffffffffff.
    ** Others complain about 0x7ffffffffffffffffLL.  The following macro seems
    ** to provide the constant while making all compilers happy.
    */
#   define MAX_ROWID  (i64)( (((u64)0x7fffffff)<<32) | (u64)0xffffffff )
#endif

    if( !u.be.pC->useRandomRowid ){
      u.be.v = sqlite3BtreeGetCachedRowid(u.be.pC->pCursor);
      if( u.be.v==0 ){
        rc = sqlite3BtreeLast(u.be.pC->pCursor, &u.be.res);
        if( rc!=SQLITE_OK ){
          goto abort_due_to_error;
        }
        if( u.be.res ){
          u.be.v = 1;   /* IMP: R-61914-48074 */
        }else{
          assert( sqlite3BtreeCursorIsValid(u.be.pC->pCursor) );
          rc = sqlite3BtreeKeySize(u.be.pC->pCursor, &u.be.v);
          assert( rc==SQLITE_OK );   /* Cannot fail following BtreeLast() */
          if( u.be.v==MAX_ROWID ){
            u.be.pC->useRandomRowid = 1;
          }else{
            u.be.v++;   /* IMP: R-29538-34987 */
          }
        }
      }

#ifndef SQLITE_OMIT_AUTOINCREMENT
      if( pOp->p3 ){
        /* Assert that P3 is a valid memory cell. */
        assert( pOp->p3>0 );
        if( p->pFrame ){
          for(u.be.pFrame=p->pFrame; u.be.pFrame->pParent; u.be.pFrame=u.be.pFrame->pParent);
          /* Assert that P3 is a valid memory cell. */
          assert( pOp->p3<=u.be.pFrame->nMem );
          u.be.pMem = &u.be.pFrame->aMem[pOp->p3];
        }else{
          /* Assert that P3 is a valid memory cell. */
          assert( pOp->p3<=p->nMem );
          u.be.pMem = &aMem[pOp->p3];
          memAboutToChange(p, u.be.pMem);
        }
        assert( memIsValid(u.be.pMem) );

        REGISTER_TRACE(pOp->p3, u.be.pMem);
        sqlite3VdbeMemIntegerify(u.be.pMem);
        assert( (u.be.pMem->flags & MEM_Int)!=0 );  /* mem(P3) holds an integer */
        if( u.be.pMem->u.i==MAX_ROWID || u.be.pC->useRandomRowid ){
          rc = SQLITE_FULL;   /* IMP: R-12275-61338 */
          goto abort_due_to_error;
        }
        if( u.be.v<u.be.pMem->u.i+1 ){
          u.be.v = u.be.pMem->u.i + 1;
        }
        u.be.pMem->u.i = u.be.v;
      }
#endif

      sqlite3BtreeSetCachedRowid(u.be.pC->pCursor, u.be.v<MAX_ROWID ? u.be.v+1 : 0);
    }
    if( u.be.pC->useRandomRowid ){
      /* IMPLEMENTATION-OF: R-07677-41881 If the largest ROWID is equal to the
      ** largest possible integer (9223372036854775807) then the database
      ** engine starts picking positive candidate ROWIDs at random until
      ** it finds one that is not previously used. */
      assert( pOp->p3==0 );  /* We cannot be in random rowid mode if this is
                             ** an AUTOINCREMENT table. */
      /* on the first attempt, simply do one more than previous */
      u.be.v = lastRowid;
      u.be.v &= (MAX_ROWID>>1); /* ensure doesn't go negative */
      u.be.v++; /* ensure non-zero */
      u.be.cnt = 0;
      while(   ((rc = sqlite3BtreeMovetoUnpacked(u.be.pC->pCursor, 0, (u64)u.be.v,
                                                 0, &u.be.res))==SQLITE_OK)
            && (u.be.res==0)
            && (++u.be.cnt<100)){
        /* collision - try another random rowid */
        sqlite3_randomness(sizeof(u.be.v), &u.be.v);
        if( u.be.cnt<5 ){
          /* try "small" random rowids for the initial attempts */
          u.be.v &= 0xffffff;
        }else{
          u.be.v &= (MAX_ROWID>>1); /* ensure doesn't go negative */
        }
        u.be.v++; /* ensure non-zero */
      }
      if( rc==SQLITE_OK && u.be.res==0 ){
        rc = SQLITE_FULL;   /* IMP: R-38219-53002 */
        goto abort_due_to_error;
      }
      assert( u.be.v>0 );  /* EV: R-40812-03570 */
    }
    u.be.pC->rowidIsValid = 0;
    u.be.pC->deferredMoveto = 0;
    u.be.pC->cacheStatus = CACHE_STALE;
  }
  pOut->u.i = u.be.v;
  break;
}

/* Opcode: Insert P1 P2 P3 P4 P5
**
** Write an entry into the table of cursor P1.  A new entry is
** created if it doesn't already exist or the data for an existing
** entry is overwritten.  The data is the value MEM_Blob stored in register
** number P2. The key is stored in register P3. The key must
** be a MEM_Int.
**
** If the OPFLAG_NCHANGE flag of P5 is set, then the row change count is
** incremented (otherwise not).  If the OPFLAG_LASTROWID flag of P5 is set,
** then rowid is stored for subsequent return by the
** sqlite3_last_insert_rowid() function (otherwise it is unmodified).
**
** If the OPFLAG_USESEEKRESULT flag of P5 is set and if the result of
** the last seek operation (OP_NotExists) was a success, then this
** operation will not attempt to find the appropriate row before doing
** the insert but will instead overwrite the row that the cursor is
** currently pointing to.  Presumably, the prior OP_NotExists opcode
** has already positioned the cursor correctly.  This is an optimization
** that boosts performance by avoiding redundant seeks.
**
** If the OPFLAG_ISUPDATE flag is set, then this opcode is part of an
** UPDATE operation.  Otherwise (if the flag is clear) then this opcode
** is part of an INSERT operation.  The difference is only important to
** the update hook.
**
** Parameter P4 may point to a string containing the table-name, or
** may be NULL. If it is not NULL, then the update-hook 
** (sqlite3.xUpdateCallback) is invoked following a successful insert.
**
** (WARNING/TODO: If P1 is a pseudo-cursor and P2 is dynamically
** allocated, then ownership of P2 is transferred to the pseudo-cursor
** and register P2 becomes ephemeral.  If the cursor is changed, the
** value of register P2 will then change.  Make sure this does not
** cause any problems.)
**
** This instruction only works on tables.  The equivalent instruction
** for indices is OP_IdxInsert.
*/
/* Opcode: InsertInt P1 P2 P3 P4 P5
**
** This works exactly like OP_Insert except that the key is the
** integer value P3, not the value of the integer stored in register P3.
*/
case OP_Insert: 
case OP_InsertInt: {
#if 0  /* local variables moved into u.bf */
  Mem *pData;       /* MEM cell holding data for the record to be inserted */
  Mem *pKey;        /* MEM cell holding key  for the record */
  i64 iKey;         /* The integer ROWID or key for the record to be inserted */
  VdbeCursor *pC;   /* Cursor to table into which insert is written */
  int nZero;        /* Number of zero-bytes to append */
  int seekResult;   /* Result of prior seek or 0 if no USESEEKRESULT flag */
  const char *zDb;  /* database name - used by the update hook */
  const char *zTbl; /* Table name - used by the opdate hook */
  int op;           /* Opcode for update hook: SQLITE_UPDATE or SQLITE_INSERT */
#endif /* local variables moved into u.bf */

  u.bf.pData = &aMem[pOp->p2];
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  assert( memIsValid(u.bf.pData) );
  u.bf.pC = p->apCsr[pOp->p1];
  assert( u.bf.pC!=0 );
  assert( u.bf.pC->pCursor!=0 );
  assert( u.bf.pC->pseudoTableReg==0 );
  assert( u.bf.pC->isTable );
  REGISTER_TRACE(pOp->p2, u.bf.pData);

  if( pOp->opcode==OP_Insert ){
    u.bf.pKey = &aMem[pOp->p3];
    assert( u.bf.pKey->flags & MEM_Int );
    assert( memIsValid(u.bf.pKey) );
    REGISTER_TRACE(pOp->p3, u.bf.pKey);
    u.bf.iKey = u.bf.pKey->u.i;
  }else{
    assert( pOp->opcode==OP_InsertInt );
    u.bf.iKey = pOp->p3;
  }

  if( pOp->p5 & OPFLAG_NCHANGE ) p->nChange++;
  if( pOp->p5 & OPFLAG_LASTROWID ) db->lastRowid = lastRowid = u.bf.iKey;
  if( u.bf.pData->flags & MEM_Null ){
    u.bf.pData->z = 0;
    u.bf.pData->n = 0;
  }else{
    assert( u.bf.pData->flags & (MEM_Blob|MEM_Str) );
  }
  u.bf.seekResult = ((pOp->p5 & OPFLAG_USESEEKRESULT) ? u.bf.pC->seekResult : 0);
  if( u.bf.pData->flags & MEM_Zero ){
    u.bf.nZero = u.bf.pData->u.nZero;
  }else{
    u.bf.nZero = 0;
  }
  sqlite3BtreeSetCachedRowid(u.bf.pC->pCursor, 0);
  rc = sqlite3BtreeInsert(u.bf.pC->pCursor, 0, u.bf.iKey,
                          u.bf.pData->z, u.bf.pData->n, u.bf.nZero,
                          pOp->p5 & OPFLAG_APPEND, u.bf.seekResult
  );
  u.bf.pC->rowidIsValid = 0;
  u.bf.pC->deferredMoveto = 0;
  u.bf.pC->cacheStatus = CACHE_STALE;

  /* Invoke the update-hook if required. */
  if( rc==SQLITE_OK && db->xUpdateCallback && pOp->p4.z ){
    u.bf.zDb = db->aDb[u.bf.pC->iDb].zName;
    u.bf.zTbl = pOp->p4.z;
    u.bf.op = ((pOp->p5 & OPFLAG_ISUPDATE) ? SQLITE_UPDATE : SQLITE_INSERT);
    assert( u.bf.pC->isTable );
    db->xUpdateCallback(db->pUpdateArg, u.bf.op, u.bf.zDb, u.bf.zTbl, u.bf.iKey);
    assert( u.bf.pC->iDb>=0 );
  }
  break;
}

/* Opcode: Delete P1 P2 * P4 *
**
** Delete the record at which the P1 cursor is currently pointing.
**
** The cursor will be left pointing at either the next or the previous
** record in the table. If it is left pointing at the next record, then
** the next Next instruction will be a no-op.  Hence it is OK to delete
** a record from within an Next loop.
**
** If the OPFLAG_NCHANGE flag of P2 is set, then the row change count is
** incremented (otherwise not).
**
** P1 must not be pseudo-table.  It has to be a real table with
** multiple rows.
**
** If P4 is not NULL, then it is the name of the table that P1 is
** pointing to.  The update hook will be invoked, if it exists.
** If P4 is not NULL then the P1 cursor must have been positioned
** using OP_NotFound prior to invoking this opcode.
*/
case OP_Delete: {
#if 0  /* local variables moved into u.bg */
  i64 iKey;
  VdbeCursor *pC;
#endif /* local variables moved into u.bg */

  u.bg.iKey = 0;
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bg.pC = p->apCsr[pOp->p1];
  assert( u.bg.pC!=0 );
  assert( u.bg.pC->pCursor!=0 );  /* Only valid for real tables, no pseudotables */

  /* If the update-hook will be invoked, set u.bg.iKey to the rowid of the
  ** row being deleted.
  */
  if( db->xUpdateCallback && pOp->p4.z ){
    assert( u.bg.pC->isTable );
    assert( u.bg.pC->rowidIsValid );  /* lastRowid set by previous OP_NotFound */
    u.bg.iKey = u.bg.pC->lastRowid;
  }

  /* The OP_Delete opcode always follows an OP_NotExists or OP_Last or
  ** OP_Column on the same table without any intervening operations that
  ** might move or invalidate the cursor.  Hence cursor u.bg.pC is always pointing
  ** to the row to be deleted and the sqlite3VdbeCursorMoveto() operation
  ** below is always a no-op and cannot fail.  We will run it anyhow, though,
  ** to guard against future changes to the code generator.
  **/
  assert( u.bg.pC->deferredMoveto==0 );
  rc = sqlite3VdbeCursorMoveto(u.bg.pC);
  if( NEVER(rc!=SQLITE_OK) ) goto abort_due_to_error;

  sqlite3BtreeSetCachedRowid(u.bg.pC->pCursor, 0);
  rc = sqlite3BtreeDelete(u.bg.pC->pCursor);
  u.bg.pC->cacheStatus = CACHE_STALE;

  /* Invoke the update-hook if required. */
  if( rc==SQLITE_OK && db->xUpdateCallback && pOp->p4.z ){
    const char *zDb = db->aDb[u.bg.pC->iDb].zName;
    const char *zTbl = pOp->p4.z;
    db->xUpdateCallback(db->pUpdateArg, SQLITE_DELETE, zDb, zTbl, u.bg.iKey);
    assert( u.bg.pC->iDb>=0 );
  }
  if( pOp->p2 & OPFLAG_NCHANGE ) p->nChange++;
  break;
}
/* Opcode: ResetCount * * * * *
**
** The value of the change counter is copied to the database handle
** change counter (returned by subsequent calls to sqlite3_changes()).
** Then the VMs internal change counter resets to 0.
** This is used by trigger programs.
*/
case OP_ResetCount: {
  sqlite3VdbeSetChanges(db, p->nChange);
  p->nChange = 0;
  break;
}

/* Opcode: RowData P1 P2 * * *
**
** Write into register P2 the complete row data for cursor P1.
** There is no interpretation of the data.  
** It is just copied onto the P2 register exactly as 
** it is found in the database file.
**
** If the P1 cursor must be pointing to a valid row (not a NULL row)
** of a real table, not a pseudo-table.
*/
/* Opcode: RowKey P1 P2 * * *
**
** Write into register P2 the complete row key for cursor P1.
** There is no interpretation of the data.  
** The key is copied onto the P3 register exactly as 
** it is found in the database file.
**
** If the P1 cursor must be pointing to a valid row (not a NULL row)
** of a real table, not a pseudo-table.
*/
case OP_RowKey:
case OP_RowData: {
#if 0  /* local variables moved into u.bh */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  u32 n;
  i64 n64;
#endif /* local variables moved into u.bh */

  pOut = &aMem[pOp->p2];
  memAboutToChange(p, pOut);

  /* Note that RowKey and RowData are really exactly the same instruction */
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bh.pC = p->apCsr[pOp->p1];
  assert( u.bh.pC->isTable || pOp->opcode==OP_RowKey );
  assert( u.bh.pC->isIndex || pOp->opcode==OP_RowData );
  assert( u.bh.pC!=0 );
  assert( u.bh.pC->nullRow==0 );
  assert( u.bh.pC->pseudoTableReg==0 );
  assert( u.bh.pC->pCursor!=0 );
  u.bh.pCrsr = u.bh.pC->pCursor;
  assert( sqlite3BtreeCursorIsValid(u.bh.pCrsr) );

  /* The OP_RowKey and OP_RowData opcodes always follow OP_NotExists or
  ** OP_Rewind/Op_Next with no intervening instructions that might invalidate
  ** the cursor.  Hence the following sqlite3VdbeCursorMoveto() call is always
  ** a no-op and can never fail.  But we leave it in place as a safety.
  */
  assert( u.bh.pC->deferredMoveto==0 );
  rc = sqlite3VdbeCursorMoveto(u.bh.pC);
  if( NEVER(rc!=SQLITE_OK) ) goto abort_due_to_error;

  if( u.bh.pC->isIndex ){
    assert( !u.bh.pC->isTable );
    rc = sqlite3BtreeKeySize(u.bh.pCrsr, &u.bh.n64);
    assert( rc==SQLITE_OK );    /* True because of CursorMoveto() call above */
    if( u.bh.n64>db->aLimit[SQLITE_LIMIT_LENGTH] ){
      goto too_big;
    }
    u.bh.n = (u32)u.bh.n64;
  }else{
    rc = sqlite3BtreeDataSize(u.bh.pCrsr, &u.bh.n);
    assert( rc==SQLITE_OK );    /* DataSize() cannot fail */
    if( u.bh.n>(u32)db->aLimit[SQLITE_LIMIT_LENGTH] ){
      goto too_big;
    }
  }
  if( sqlite3VdbeMemGrow(pOut, u.bh.n, 0) ){
    goto no_mem;
  }
  pOut->n = u.bh.n;
  MemSetTypeFlag(pOut, MEM_Blob);
  if( u.bh.pC->isIndex ){
    rc = sqlite3BtreeKey(u.bh.pCrsr, 0, u.bh.n, pOut->z);
  }else{
    rc = sqlite3BtreeData(u.bh.pCrsr, 0, u.bh.n, pOut->z);
  }
  pOut->enc = SQLITE_UTF8;  /* In case the blob is ever cast to text */
  UPDATE_MAX_BLOBSIZE(pOut);
  break;
}

/* Opcode: Rowid P1 P2 * * *
**
** Store in register P2 an integer which is the key of the table entry that
** P1 is currently point to.
**
** P1 can be either an ordinary table or a virtual table.  There used to
** be a separate OP_VRowid opcode for use with virtual tables, but this
** one opcode now works for both table types.
*/
case OP_Rowid: {                 /* out2-prerelease */
#if 0  /* local variables moved into u.bi */
  VdbeCursor *pC;
  i64 v;
  sqlite3_vtab *pVtab;
  const sqlite3_module *pModule;
#endif /* local variables moved into u.bi */

  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bi.pC = p->apCsr[pOp->p1];
  assert( u.bi.pC!=0 );
  assert( u.bi.pC->pseudoTableReg==0 );
  if( u.bi.pC->nullRow ){
    pOut->flags = MEM_Null;
    break;
  }else if( u.bi.pC->deferredMoveto ){
    u.bi.v = u.bi.pC->movetoTarget;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  }else if( u.bi.pC->pVtabCursor ){
    u.bi.pVtab = u.bi.pC->pVtabCursor->pVtab;
    u.bi.pModule = u.bi.pVtab->pModule;
    assert( u.bi.pModule->xRowid );
    rc = u.bi.pModule->xRowid(u.bi.pC->pVtabCursor, &u.bi.v);
    importVtabErrMsg(p, u.bi.pVtab);
#endif /* SQLITE_OMIT_VIRTUALTABLE */
  }else{
    assert( u.bi.pC->pCursor!=0 );
    rc = sqlite3VdbeCursorMoveto(u.bi.pC);
    if( rc ) goto abort_due_to_error;
    if( u.bi.pC->rowidIsValid ){
      u.bi.v = u.bi.pC->lastRowid;
    }else{
      rc = sqlite3BtreeKeySize(u.bi.pC->pCursor, &u.bi.v);
      assert( rc==SQLITE_OK );  /* Always so because of CursorMoveto() above */
    }
  }
  pOut->u.i = u.bi.v;
  break;
}

/* Opcode: NullRow P1 * * * *
**
** Move the cursor P1 to a null row.  Any OP_Column operations
** that occur while the cursor is on the null row will always
** write a NULL.
*/
case OP_NullRow: {
#if 0  /* local variables moved into u.bj */
  VdbeCursor *pC;
#endif /* local variables moved into u.bj */

  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bj.pC = p->apCsr[pOp->p1];
  assert( u.bj.pC!=0 );
  u.bj.pC->nullRow = 1;
  u.bj.pC->rowidIsValid = 0;
  if( u.bj.pC->pCursor ){
    sqlite3BtreeClearCursor(u.bj.pC->pCursor);
  }
  break;
}

/* Opcode: Last P1 P2 * * *
**
** The next use of the Rowid or Column or Next instruction for P1 
** will refer to the last entry in the database table or index.
** If the table or index is empty and P2>0, then jump immediately to P2.
** If P2 is 0 or if the table or index is not empty, fall through
** to the following instruction.
*/
case OP_Last: {        /* jump */
#if 0  /* local variables moved into u.bk */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  int res;
#endif /* local variables moved into u.bk */

  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bk.pC = p->apCsr[pOp->p1];
  assert( u.bk.pC!=0 );
  u.bk.pCrsr = u.bk.pC->pCursor;
  if( u.bk.pCrsr==0 ){
    u.bk.res = 1;
  }else{
    rc = sqlite3BtreeLast(u.bk.pCrsr, &u.bk.res);
  }
  u.bk.pC->nullRow = (u8)u.bk.res;
  u.bk.pC->deferredMoveto = 0;
  u.bk.pC->rowidIsValid = 0;
  u.bk.pC->cacheStatus = CACHE_STALE;
  if( pOp->p2>0 && u.bk.res ){
    pc = pOp->p2 - 1;
  }
  break;
}


/* Opcode: Sort P1 P2 * * *
**
** This opcode does exactly the same thing as OP_Rewind except that
** it increments an undocumented global variable used for testing.
**
** Sorting is accomplished by writing records into a sorting index,
** then rewinding that index and playing it back from beginning to
** end.  We use the OP_Sort opcode instead of OP_Rewind to do the
** rewinding so that the global variable will be incremented and
** regression tests can determine whether or not the optimizer is
** correctly optimizing out sorts.
*/
case OP_Sort: {        /* jump */
#ifdef SQLITE_TEST
  sqlite3_sort_count++;
  sqlite3_search_count--;
#endif
  p->aCounter[SQLITE_STMTSTATUS_SORT-1]++;
  /* Fall through into OP_Rewind */
}
/* Opcode: Rewind P1 P2 * * *
**
** The next use of the Rowid or Column or Next instruction for P1 
** will refer to the first entry in the database table or index.
** If the table or index is empty and P2>0, then jump immediately to P2.
** If P2 is 0 or if the table or index is not empty, fall through
** to the following instruction.
*/
case OP_Rewind: {        /* jump */
#if 0  /* local variables moved into u.bl */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  int res;
#endif /* local variables moved into u.bl */

  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bl.pC = p->apCsr[pOp->p1];
  assert( u.bl.pC!=0 );
  u.bl.res = 1;
  if( (u.bl.pCrsr = u.bl.pC->pCursor)!=0 ){
    rc = sqlite3BtreeFirst(u.bl.pCrsr, &u.bl.res);
    u.bl.pC->atFirst = u.bl.res==0 ?1:0;
    u.bl.pC->deferredMoveto = 0;
    u.bl.pC->cacheStatus = CACHE_STALE;
    u.bl.pC->rowidIsValid = 0;
  }
  u.bl.pC->nullRow = (u8)u.bl.res;
  assert( pOp->p2>0 && pOp->p2<p->nOp );
  if( u.bl.res ){
    pc = pOp->p2 - 1;
  }
  break;
}

/* Opcode: Next P1 P2 * * P5
**
** Advance cursor P1 so that it points to the next key/data pair in its
** table or index.  If there are no more key/value pairs then fall through
** to the following instruction.  But if the cursor advance was successful,
** jump immediately to P2.
**
** The P1 cursor must be for a real table, not a pseudo-table.
**
** If P5 is positive and the jump is taken, then event counter
** number P5-1 in the prepared statement is incremented.
**
** See also: Prev
*/
/* Opcode: Prev P1 P2 * * P5
**
** Back up cursor P1 so that it points to the previous key/data pair in its
** table or index.  If there is no previous key/value pairs then fall through
** to the following instruction.  But if the cursor backup was successful,
** jump immediately to P2.
**
** The P1 cursor must be for a real table, not a pseudo-table.
**
** If P5 is positive and the jump is taken, then event counter
** number P5-1 in the prepared statement is incremented.
*/
case OP_Prev:          /* jump */
case OP_Next: {        /* jump */
#if 0  /* local variables moved into u.bm */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  int res;
#endif /* local variables moved into u.bm */

  CHECK_FOR_INTERRUPT;
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  assert( pOp->p5<=ArraySize(p->aCounter) );
  u.bm.pC = p->apCsr[pOp->p1];
  if( u.bm.pC==0 ){
    break;  /* See ticket #2273 */
  }
  u.bm.pCrsr = u.bm.pC->pCursor;
  if( u.bm.pCrsr==0 ){
    u.bm.pC->nullRow = 1;
    break;
  }
  u.bm.res = 1;
  assert( u.bm.pC->deferredMoveto==0 );
  rc = pOp->opcode==OP_Next ? sqlite3BtreeNext(u.bm.pCrsr, &u.bm.res) :
                              sqlite3BtreePrevious(u.bm.pCrsr, &u.bm.res);
  u.bm.pC->nullRow = (u8)u.bm.res;
  u.bm.pC->cacheStatus = CACHE_STALE;
  if( u.bm.res==0 ){
    pc = pOp->p2 - 1;
    if( pOp->p5 ) p->aCounter[pOp->p5-1]++;
#ifdef SQLITE_TEST
    sqlite3_search_count++;
#endif
  }
  u.bm.pC->rowidIsValid = 0;
  break;
}

/* Opcode: IdxInsert P1 P2 P3 * P5
**
** Register P2 holds an SQL index key made using the
** MakeRecord instructions.  This opcode writes that key
** into the index P1.  Data for the entry is nil.
**
** P3 is a flag that provides a hint to the b-tree layer that this
** insert is likely to be an append.
**
** This instruction only works for indices.  The equivalent instruction
** for tables is OP_Insert.
*/
case OP_IdxInsert: {        /* in2 */
#if 0  /* local variables moved into u.bn */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  int nKey;
  const char *zKey;
#endif /* local variables moved into u.bn */

  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bn.pC = p->apCsr[pOp->p1];
  assert( u.bn.pC!=0 );
  pIn2 = &aMem[pOp->p2];
  assert( pIn2->flags & MEM_Blob );
  u.bn.pCrsr = u.bn.pC->pCursor;
  if( ALWAYS(u.bn.pCrsr!=0) ){
    assert( u.bn.pC->isTable==0 );
    rc = ExpandBlob(pIn2);
    if( rc==SQLITE_OK ){
      u.bn.nKey = pIn2->n;
      u.bn.zKey = pIn2->z;
      rc = sqlite3BtreeInsert(u.bn.pCrsr, u.bn.zKey, u.bn.nKey, "", 0, 0, pOp->p3,
          ((pOp->p5 & OPFLAG_USESEEKRESULT) ? u.bn.pC->seekResult : 0)
      );
      assert( u.bn.pC->deferredMoveto==0 );
      u.bn.pC->cacheStatus = CACHE_STALE;
    }
  }
  break;
}

/* Opcode: IdxDelete P1 P2 P3 * *
**
** The content of P3 registers starting at register P2 form
** an unpacked index key. This opcode removes that entry from the 
** index opened by cursor P1.
*/
case OP_IdxDelete: {
#if 0  /* local variables moved into u.bo */
  VdbeCursor *pC;
  BtCursor *pCrsr;
  int res;
  UnpackedRecord r;
#endif /* local variables moved into u.bo */

  assert( pOp->p3>0 );
  assert( pOp->p2>0 && pOp->p2+pOp->p3<=p->nMem+1 );
  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bo.pC = p->apCsr[pOp->p1];
  assert( u.bo.pC!=0 );
  u.bo.pCrsr = u.bo.pC->pCursor;
  if( ALWAYS(u.bo.pCrsr!=0) ){
    u.bo.r.pKeyInfo = u.bo.pC->pKeyInfo;
    u.bo.r.nField = (u16)pOp->p3;
    u.bo.r.flags = 0;
    u.bo.r.aMem = &aMem[pOp->p2];
#ifdef SQLITE_DEBUG
    { int i; for(i=0; i<u.bo.r.nField; i++) assert( memIsValid(&u.bo.r.aMem[i]) ); }
#endif
    rc = sqlite3BtreeMovetoUnpacked(u.bo.pCrsr, &u.bo.r, 0, 0, &u.bo.res);
    if( rc==SQLITE_OK && u.bo.res==0 ){
      rc = sqlite3BtreeDelete(u.bo.pCrsr);
    }
    assert( u.bo.pC->deferredMoveto==0 );
    u.bo.pC->cacheStatus = CACHE_STALE;
  }
  break;
}

/* Opcode: IdxRowid P1 P2 * * *
**
** Write into register P2 an integer which is the last entry in the record at
** the end of the index key pointed to by cursor P1.  This integer should be
** the rowid of the table entry to which this index entry points.
**
** See also: Rowid, MakeRecord.
*/
case OP_IdxRowid: {              /* out2-prerelease */
#if 0  /* local variables moved into u.bp */
  BtCursor *pCrsr;
  VdbeCursor *pC;
  i64 rowid;
#endif /* local variables moved into u.bp */

  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bp.pC = p->apCsr[pOp->p1];
  assert( u.bp.pC!=0 );
  u.bp.pCrsr = u.bp.pC->pCursor;
  pOut->flags = MEM_Null;
  if( ALWAYS(u.bp.pCrsr!=0) ){
    rc = sqlite3VdbeCursorMoveto(u.bp.pC);
    if( NEVER(rc) ) goto abort_due_to_error;
    assert( u.bp.pC->deferredMoveto==0 );
    assert( u.bp.pC->isTable==0 );
    if( !u.bp.pC->nullRow ){
      rc = sqlite3VdbeIdxRowid(db, u.bp.pCrsr, &u.bp.rowid);
      if( rc!=SQLITE_OK ){
        goto abort_due_to_error;
      }
      pOut->u.i = u.bp.rowid;
      pOut->flags = MEM_Int;
    }
  }
  break;
}

/* Opcode: IdxGE P1 P2 P3 P4 P5
**
** The P4 register values beginning with P3 form an unpacked index 
** key that omits the ROWID.  Compare this key value against the index 
** that P1 is currently pointing to, ignoring the ROWID on the P1 index.
**
** If the P1 index entry is greater than or equal to the key value
** then jump to P2.  Otherwise fall through to the next instruction.
**
** If P5 is non-zero then the key value is increased by an epsilon 
** prior to the comparison.  This make the opcode work like IdxGT except
** that if the key from register P3 is a prefix of the key in the cursor,
** the result is false whereas it would be true with IdxGT.
*/
/* Opcode: IdxLT P1 P2 P3 P4 P5
**
** The P4 register values beginning with P3 form an unpacked index 
** key that omits the ROWID.  Compare this key value against the index 
** that P1 is currently pointing to, ignoring the ROWID on the P1 index.
**
** If the P1 index entry is less than the key value then jump to P2.
** Otherwise fall through to the next instruction.
**
** If P5 is non-zero then the key value is increased by an epsilon prior 
** to the comparison.  This makes the opcode work like IdxLE.
*/
case OP_IdxLT:          /* jump */
case OP_IdxGE: {        /* jump */
#if 0  /* local variables moved into u.bq */
  VdbeCursor *pC;
  int res;
  UnpackedRecord r;
#endif /* local variables moved into u.bq */

  assert( pOp->p1>=0 && pOp->p1<p->nCursor );
  u.bq.pC = p->apCsr[pOp->p1];
  assert( u.bq.pC!=0 );
  assert( u.bq.pC->isOrdered );
  if( ALWAYS(u.bq.pC->pCursor!=0) ){
    assert( u.bq.pC->deferredMoveto==0 );
    assert( pOp->p5==0 || pOp->p5==1 );
    assert( pOp->p4type==P4_INT32 );
    u.bq.r.pKeyInfo = u.bq.pC->pKeyInfo;
    u.bq.r.nField = (u16)pOp->p4.i;
    if( pOp->p5 ){
      u.bq.r.flags = UNPACKED_INCRKEY | UNPACKED_IGNORE_ROWID;
    }else{
      u.bq.r.flags = UNPACKED_IGNORE_ROWID;
    }
    u.bq.r.aMem = &aMem[pOp->p3];
#ifdef SQLITE_DEBUG
    { int i; for(i=0; i<u.bq.r.nField; i++) assert( memIsValid(&u.bq.r.aMem[i]) ); }
#endif
    rc = sqlite3VdbeIdxKeyCompare(u.bq.pC, &u.bq.r, &u.bq.res);
    if( pOp->opcode==OP_IdxLT ){
      u.bq.res = -u.bq.res;
    }else{
      assert( pOp->opcode==OP_IdxGE );
      u.bq.res++;
    }
    if( u.bq.res>0 ){
      pc = pOp->p2 - 1 ;
    }
  }
  break;
}

/* Opcode: Destroy P1 P2 P3 * *
**
** Delete an entire database table or index whose root page in the database
** file is given by P1.
**
** The table being destroyed is in the main database file if P3==0.  If
** P3==1 then the table to be clear is in the auxiliary database file
** that is used to store tables create using CREATE TEMPORARY TABLE.
**
** If AUTOVACUUM is enabled then it is possible that another root page
** might be moved into the newly deleted root page in order to keep all
** root pages contiguous at the beginning of the database.  The former
** value of the root page that moved - its value before the move occurred -
** is stored in register P2.  If no page 
** movement was required (because the table being dropped was already 
** the last one in the database) then a zero is stored in register P2.
** If AUTOVACUUM is disabled then a zero is stored in register P2.
**
** See also: Clear
*/
case OP_Destroy: {     /* out2-prerelease */
#if 0  /* local variables moved into u.br */
  int iMoved;
  int iCnt;
  Vdbe *pVdbe;
  int iDb;
#endif /* local variables moved into u.br */
#ifndef SQLITE_OMIT_VIRTUALTABLE
  u.br.iCnt = 0;
  for(u.br.pVdbe=db->pVdbe; u.br.pVdbe; u.br.pVdbe = u.br.pVdbe->pNext){
    if( u.br.pVdbe->magic==VDBE_MAGIC_RUN && u.br.pVdbe->inVtabMethod<2 && u.br.pVdbe->pc>=0 ){
      u.br.iCnt++;
    }
  }
#else
  u.br.iCnt = db->activeVdbeCnt;
#endif
  pOut->flags = MEM_Null;
  if( u.br.iCnt>1 ){
    rc = SQLITE_LOCKED;
    p->errorAction = OE_Abort;
  }else{
    u.br.iDb = pOp->p3;
    assert( u.br.iCnt==1 );
    assert( (p->btreeMask & (((yDbMask)1)<<u.br.iDb))!=0 );
    rc = sqlite3BtreeDropTable(db->aDb[u.br.iDb].pBt, pOp->p1, &u.br.iMoved);
    pOut->flags = MEM_Int;
    pOut->u.i = u.br.iMoved;
#ifndef SQLITE_OMIT_AUTOVACUUM
    if( rc==SQLITE_OK && u.br.iMoved!=0 ){
      sqlite3RootPageMoved(db, u.br.iDb, u.br.iMoved, pOp->p1);
      /* All OP_Destroy operations occur on the same btree */
      assert( resetSchemaOnFault==0 || resetSchemaOnFault==u.br.iDb+1 );
      resetSchemaOnFault = u.br.iDb+1;
    }
#endif
  }
  break;
}

/* Opcode: Clear P1 P2 P3
**
** Delete all contents of the database table or index whose root page
** in the database file is given by P1.  But, unlike Destroy, do not
** remove the table or index from the database file.
**
** The table being clear is in the main database file if P2==0.  If
** P2==1 then the table to be clear is in the auxiliary database file
** that is used to store tables create using CREATE TEMPORARY TABLE.
**
** If the P3 value is non-zero, then the table referred to must be an
** intkey table (an SQL table, not an index). In this case the row change 
** count is incremented by the number of rows in the table being cleared. 
** If P3 is greater than zero, then the value stored in register P3 is
** also incremented by the number of rows in the table being cleared.
**
** See also: Destroy
*/
case OP_Clear: {
#if 0  /* local variables moved into u.bs */
  int nChange;
#endif /* local variables moved into u.bs */

  u.bs.nChange = 0;
  assert( (p->btreeMask & (((yDbMask)1)<<pOp->p2))!=0 );
  rc = sqlite3BtreeClearTable(
      db->aDb[pOp->p2].pBt, pOp->p1, (pOp->p3 ? &u.bs.nChange : 0)
  );
  if( pOp->p3 ){
    p->nChange += u.bs.nChange;
    if( pOp->p3>0 ){
      assert( memIsValid(&aMem[pOp->p3]) );
      memAboutToChange(p, &aMem[pOp->p3]);
      aMem[pOp->p3].u.i += u.bs.nChange;
    }
  }
  break;
}

/* Opcode: CreateTable P1 P2 * * *
**
** Allocate a new table in the main database file if P1==0 or in the
** auxiliary database file if P1==1 or in an attached database if
** P1>1.  Write the root page number of the new table into
** register P2
**
** The difference between a table and an index is this:  A table must
** have a 4-byte integer key and can have arbitrary data.  An index
** has an arbitrary key but no data.
**
** See also: CreateIndex
*/
/* Opcode: CreateIndex P1 P2 * * *
**
** Allocate a new index in the main database file if P1==0 or in the
** auxiliary database file if P1==1 or in an attached database if
** P1>1.  Write the root page number of the new table into
** register P2.
**
** See documentation on OP_CreateTable for additional information.
*/
case OP_CreateIndex:            /* out2-prerelease */
case OP_CreateTable: {          /* out2-prerelease */
#if 0  /* local variables moved into u.bt */
  int pgno;
  int flags;
  Db *pDb;
#endif /* local variables moved into u.bt */

  u.bt.pgno = 0;
  assert( pOp->p1>=0 && pOp->p1<db->nDb );
  assert( (p->btreeMask & (((yDbMask)1)<<pOp->p1))!=0 );
  u.bt.pDb = &db->aDb[pOp->p1];
  assert( u.bt.pDb->pBt!=0 );
  if( pOp->opcode==OP_CreateTable ){
    /* u.bt.flags = BTREE_INTKEY; */
    u.bt.flags = BTREE_INTKEY;
  }else{
    u.bt.flags = BTREE_BLOBKEY;
  }
  rc = sqlite3BtreeCreateTable(u.bt.pDb->pBt, &u.bt.pgno, u.bt.flags);
  pOut->u.i = u.bt.pgno;
  break;
}

/* Opcode: ParseSchema P1 * * P4 *
**
** Read and parse all entries from the SQLITE_MASTER table of database P1
** that match the WHERE clause P4. 
**
** This opcode invokes the parser to create a new virtual machine,
** then runs the new virtual machine.  It is thus a re-entrant opcode.
*/
case OP_ParseSchema: {
#if 0  /* local variables moved into u.bu */
  int iDb;
  const char *zMaster;
  char *zSql;
  InitData initData;
#endif /* local variables moved into u.bu */

  /* Any prepared statement that invokes this opcode will hold mutexes
  ** on every btree.  This is a prerequisite for invoking
  ** sqlite3InitCallback().
  */
#ifdef SQLITE_DEBUG
  for(u.bu.iDb=0; u.bu.iDb<db->nDb; u.bu.iDb++){
    assert( u.bu.iDb==1 || sqlite3BtreeHoldsMutex(db->aDb[u.bu.iDb].pBt) );
  }
#endif

  u.bu.iDb = pOp->p1;
  assert( u.bu.iDb>=0 && u.bu.iDb<db->nDb );
  assert( DbHasProperty(db, u.bu.iDb, DB_SchemaLoaded) );
  /* Used to be a conditional */ {
    u.bu.zMaster = SCHEMA_TABLE(u.bu.iDb);
    u.bu.initData.db = db;
    u.bu.initData.iDb = pOp->p1;
    u.bu.initData.pzErrMsg = &p->zErrMsg;
    u.bu.zSql = sqlite3MPrintf(db,
       "SELECT name, rootpage, sql FROM '%q'.%s WHERE %s ORDER BY rowid",
       db->aDb[u.bu.iDb].zName, u.bu.zMaster, pOp->p4.z);
    if( u.bu.zSql==0 ){
      rc = SQLITE_NOMEM;
    }else{
      assert( db->init.busy==0 );
      db->init.busy = 1;
      u.bu.initData.rc = SQLITE_OK;
      assert( !db->mallocFailed );
      rc = sqlite3_exec(db, u.bu.zSql, sqlite3InitCallback, &u.bu.initData, 0);
      if( rc==SQLITE_OK ) rc = u.bu.initData.rc;
      sqlite3DbFree(db, u.bu.zSql);
      db->init.busy = 0;
    }
  }
  if( rc==SQLITE_NOMEM ){
    goto no_mem;
  }
  break;
}

#if !defined(SQLITE_OMIT_ANALYZE)
/* Opcode: LoadAnalysis P1 * * * *
**
** Read the sqlite_stat1 table for database P1 and load the content
** of that table into the internal index hash table.  This will cause
** the analysis to be used when preparing all subsequent queries.
*/
case OP_LoadAnalysis: {
  assert( pOp->p1>=0 && pOp->p1<db->nDb );
  rc = sqlite3AnalysisLoad(db, pOp->p1);
  break;  
}
#endif /* !defined(SQLITE_OMIT_ANALYZE) */

/* Opcode: DropTable P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the table named P4 in database P1.  This is called after a table
** is dropped in order to keep the internal representation of the
** schema consistent with what is on disk.
*/
case OP_DropTable: {
  sqlite3UnlinkAndDeleteTable(db, pOp->p1, pOp->p4.z);
  break;
}

/* Opcode: DropIndex P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the index named P4 in database P1.  This is called after an index
** is dropped in order to keep the internal representation of the
** schema consistent with what is on disk.
*/
case OP_DropIndex: {
  sqlite3UnlinkAndDeleteIndex(db, pOp->p1, pOp->p4.z);
  break;
}

/* Opcode: DropTrigger P1 * * P4 *
**
** Remove the internal (in-memory) data structures that describe
** the trigger named P4 in database P1.  This is called after a trigger
** is dropped in order to keep the internal representation of the
** schema consistent with what is on disk.
*/
case OP_DropTrigger: {
  sqlite3UnlinkAndDeleteTrigger(db, pOp->p1, pOp->p4.z);
  break;
}


#ifndef SQLITE_OMIT_INTEGRITY_CHECK
/* Opcode: IntegrityCk P1 P2 P3 * P5
**
** Do an analysis of the currently open database.  Store in
** register P1 the text of an error message describing any problems.
** If no problems are found, store a NULL in register P1.
**
** The register P3 contains the maximum number of allowed errors.
** At most reg(P3) errors will be reported.
** In other words, the analysis stops as soon as reg(P1) errors are 
** seen.  Reg(P1) is updated with the number of errors remaining.
**
** The root page numbers of all tables in the database are integer
** stored in reg(P1), reg(P1+1), reg(P1+2), ....  There are P2 tables
** total.
**
** If P5 is not zero, the check is done on the auxiliary database
** file, not the main database file.
**
** This opcode is used to implement the integrity_check pragma.
*/
case OP_IntegrityCk: {
#if 0  /* local variables moved into u.bv */
  int nRoot;      /* Number of tables to check.  (Number of root pages.) */
  int *aRoot;     /* Array of rootpage numbers for tables to be checked */
  int j;          /* Loop counter */
  int nErr;       /* Number of errors reported */
  char *z;        /* Text of the error report */
  Mem *pnErr;     /* Register keeping track of errors remaining */
#endif /* local variables moved into u.bv */

  u.bv.nRoot = pOp->p2;
  assert( u.bv.nRoot>0 );
  u.bv.aRoot = sqlite3DbMallocRaw(db, sizeof(int)*(u.bv.nRoot+1) );
  if( u.bv.aRoot==0 ) goto no_mem;
  assert( pOp->p3>0 && pOp->p3<=p->nMem );
  u.bv.pnErr = &aMem[pOp->p3];
  assert( (u.bv.pnErr->flags & MEM_Int)!=0 );
  assert( (u.bv.pnErr->flags & (MEM_Str|MEM_Blob))==0 );
  pIn1 = &aMem[pOp->p1];
  for(u.bv.j=0; u.bv.j<u.bv.nRoot; u.bv.j++){
    u.bv.aRoot[u.bv.j] = (int)sqlite3VdbeIntValue(&pIn1[u.bv.j]);
  }
  u.bv.aRoot[u.bv.j] = 0;
  assert( pOp->p5<db->nDb );
  assert( (p->btreeMask & (((yDbMask)1)<<pOp->p5))!=0 );
  u.bv.z = sqlite3BtreeIntegrityCheck(db->aDb[pOp->p5].pBt, u.bv.aRoot, u.bv.nRoot,
                                 (int)u.bv.pnErr->u.i, &u.bv.nErr);
  sqlite3DbFree(db, u.bv.aRoot);
  u.bv.pnErr->u.i -= u.bv.nErr;
  sqlite3VdbeMemSetNull(pIn1);
  if( u.bv.nErr==0 ){
    assert( u.bv.z==0 );
  }else if( u.bv.z==0 ){
    goto no_mem;
  }else{
    sqlite3VdbeMemSetStr(pIn1, u.bv.z, -1, SQLITE_UTF8, sqlite3_free);
  }
  UPDATE_MAX_BLOBSIZE(pIn1);
  sqlite3VdbeChangeEncoding(pIn1, encoding);
  break;
}
#endif /* SQLITE_OMIT_INTEGRITY_CHECK */

/* Opcode: RowSetAdd P1 P2 * * *
**
** Insert the integer value held by register P2 into a boolean index
** held in register P1.
**
** An assertion fails if P2 is not an integer.
*/
case OP_RowSetAdd: {       /* in1, in2 */
  pIn1 = &aMem[pOp->p1];
  pIn2 = &aMem[pOp->p2];
  assert( (pIn2->flags & MEM_Int)!=0 );
  if( (pIn1->flags & MEM_RowSet)==0 ){
    sqlite3VdbeMemSetRowSet(pIn1);
    if( (pIn1->flags & MEM_RowSet)==0 ) goto no_mem;
  }
  sqlite3RowSetInsert(pIn1->u.pRowSet, pIn2->u.i);
  break;
}

/* Opcode: RowSetRead P1 P2 P3 * *
**
** Extract the smallest value from boolean index P1 and put that value into
** register P3.  Or, if boolean index P1 is initially empty, leave P3
** unchanged and jump to instruction P2.
*/
case OP_RowSetRead: {       /* jump, in1, out3 */
#if 0  /* local variables moved into u.bw */
  i64 val;
#endif /* local variables moved into u.bw */
  CHECK_FOR_INTERRUPT;
  pIn1 = &aMem[pOp->p1];
  if( (pIn1->flags & MEM_RowSet)==0
   || sqlite3RowSetNext(pIn1->u.pRowSet, &u.bw.val)==0
  ){
    /* The boolean index is empty */
    sqlite3VdbeMemSetNull(pIn1);
    pc = pOp->p2 - 1;
  }else{
    /* A value was pulled from the index */
    sqlite3VdbeMemSetInt64(&aMem[pOp->p3], u.bw.val);
  }
  break;
}

/* Opcode: RowSetTest P1 P2 P3 P4
**
** Register P3 is assumed to hold a 64-bit integer value. If register P1
** contains a RowSet object and that RowSet object contains
** the value held in P3, jump to register P2. Otherwise, insert the
** integer in P3 into the RowSet and continue on to the
** next opcode.
**
** The RowSet object is optimized for the case where successive sets
** of integers, where each set contains no duplicates. Each set
** of values is identified by a unique P4 value. The first set
** must have P4==0, the final set P4=-1.  P4 must be either -1 or
** non-negative.  For non-negative values of P4 only the lower 4
** bits are significant.
**
** This allows optimizations: (a) when P4==0 there is no need to test
** the rowset object for P3, as it is guaranteed not to contain it,
** (b) when P4==-1 there is no need to insert the value, as it will
** never be tested for, and (c) when a value that is part of set X is
** inserted, there is no need to search to see if the same value was
** previously inserted as part of set X (only if it was previously
** inserted as part of some other set).
*/
case OP_RowSetTest: {                     /* jump, in1, in3 */
#if 0  /* local variables moved into u.bx */
  int iSet;
  int exists;
#endif /* local variables moved into u.bx */

  pIn1 = &aMem[pOp->p1];
  pIn3 = &aMem[pOp->p3];
  u.bx.iSet = pOp->p4.i;
  assert( pIn3->flags&MEM_Int );

  /* If there is anything other than a rowset object in memory cell P1,
  ** delete it now and initialize P1 with an empty rowset
  */
  if( (pIn1->flags & MEM_RowSet)==0 ){
    sqlite3VdbeMemSetRowSet(pIn1);
    if( (pIn1->flags & MEM_RowSet)==0 ) goto no_mem;
  }

  assert( pOp->p4type==P4_INT32 );
  assert( u.bx.iSet==-1 || u.bx.iSet>=0 );
  if( u.bx.iSet ){
    u.bx.exists = sqlite3RowSetTest(pIn1->u.pRowSet,
                               (u8)(u.bx.iSet>=0 ? u.bx.iSet & 0xf : 0xff),
                               pIn3->u.i);
    if( u.bx.exists ){
      pc = pOp->p2 - 1;
      break;
    }
  }
  if( u.bx.iSet>=0 ){
    sqlite3RowSetInsert(pIn1->u.pRowSet, pIn3->u.i);
  }
  break;
}


#ifndef SQLITE_OMIT_TRIGGER

/* Opcode: Program P1 P2 P3 P4 *
**
** Execute the trigger program passed as P4 (type P4_SUBPROGRAM). 
**
** P1 contains the address of the memory cell that contains the first memory 
** cell in an array of values used as arguments to the sub-program. P2 
** contains the address to jump to if the sub-program throws an IGNORE 
** exception using the RAISE() function. Register P3 contains the address 
** of a memory cell in this (the parent) VM that is used to allocate the 
** memory required by the sub-vdbe at runtime.
**
** P4 is a pointer to the VM containing the trigger program.
*/
case OP_Program: {        /* jump */
#if 0  /* local variables moved into u.by */
  int nMem;               /* Number of memory registers for sub-program */
  int nByte;              /* Bytes of runtime space required for sub-program */
  Mem *pRt;               /* Register to allocate runtime space */
  Mem *pMem;              /* Used to iterate through memory cells */
  Mem *pEnd;              /* Last memory cell in new array */
  VdbeFrame *pFrame;      /* New vdbe frame to execute in */
  SubProgram *pProgram;   /* Sub-program to execute */
  void *t;                /* Token identifying trigger */
#endif /* local variables moved into u.by */

  u.by.pProgram = pOp->p4.pProgram;
  u.by.pRt = &aMem[pOp->p3];
  assert( memIsValid(u.by.pRt) );
  assert( u.by.pProgram->nOp>0 );

  /* If the p5 flag is clear, then recursive invocation of triggers is
  ** disabled for backwards compatibility (p5 is set if this sub-program
  ** is really a trigger, not a foreign key action, and the flag set
  ** and cleared by the "PRAGMA recursive_triggers" command is clear).
  **
  ** It is recursive invocation of triggers, at the SQL level, that is
  ** disabled. In some cases a single trigger may generate more than one
  ** SubProgram (if the trigger may be executed with more than one different
  ** ON CONFLICT algorithm). SubProgram structures associated with a
  ** single trigger all have the same value for the SubProgram.token
  ** variable.  */
  if( pOp->p5 ){
    u.by.t = u.by.pProgram->token;
    for(u.by.pFrame=p->pFrame; u.by.pFrame && u.by.pFrame->token!=u.by.t; u.by.pFrame=u.by.pFrame->pParent);
    if( u.by.pFrame ) break;
  }

  if( p->nFrame>=db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH] ){
    rc = SQLITE_ERROR;
    sqlite3SetString(&p->zErrMsg, db, "too many levels of trigger recursion");
    break;
  }

  /* Register u.by.pRt is used to store the memory required to save the state
  ** of the current program, and the memory required at runtime to execute
  ** the trigger program. If this trigger has been fired before, then u.by.pRt
  ** is already allocated. Otherwise, it must be initialized.  */
  if( (u.by.pRt->flags&MEM_Frame)==0 ){
    /* SubProgram.nMem is set to the number of memory cells used by the
    ** program stored in SubProgram.aOp. As well as these, one memory
    ** cell is required for each cursor used by the program. Set local
    ** variable u.by.nMem (and later, VdbeFrame.nChildMem) to this value.
    */
    u.by.nMem = u.by.pProgram->nMem + u.by.pProgram->nCsr;
    u.by.nByte = ROUND8(sizeof(VdbeFrame))
              + u.by.nMem * sizeof(Mem)
              + u.by.pProgram->nCsr * sizeof(VdbeCursor *);
    u.by.pFrame = sqlite3DbMallocZero(db, u.by.nByte);
    if( !u.by.pFrame ){
      goto no_mem;
    }
    sqlite3VdbeMemRelease(u.by.pRt);
    u.by.pRt->flags = MEM_Frame;
    u.by.pRt->u.pFrame = u.by.pFrame;

    u.by.pFrame->v = p;
    u.by.pFrame->nChildMem = u.by.nMem;
    u.by.pFrame->nChildCsr = u.by.pProgram->nCsr;
    u.by.pFrame->pc = pc;
    u.by.pFrame->aMem = p->aMem;
    u.by.pFrame->nMem = p->nMem;
    u.by.pFrame->apCsr = p->apCsr;
    u.by.pFrame->nCursor = p->nCursor;
    u.by.pFrame->aOp = p->aOp;
    u.by.pFrame->nOp = p->nOp;
    u.by.pFrame->token = u.by.pProgram->token;

    u.by.pEnd = &VdbeFrameMem(u.by.pFrame)[u.by.pFrame->nChildMem];
    for(u.by.pMem=VdbeFrameMem(u.by.pFrame); u.by.pMem!=u.by.pEnd; u.by.pMem++){
      u.by.pMem->flags = MEM_Null;
      u.by.pMem->db = db;
    }
  }else{
    u.by.pFrame = u.by.pRt->u.pFrame;
    assert( u.by.pProgram->nMem+u.by.pProgram->nCsr==u.by.pFrame->nChildMem );
    assert( u.by.pProgram->nCsr==u.by.pFrame->nChildCsr );
    assert( pc==u.by.pFrame->pc );
  }

  p->nFrame++;
  u.by.pFrame->pParent = p->pFrame;
  u.by.pFrame->lastRowid = lastRowid;
  u.by.pFrame->nChange = p->nChange;
  p->nChange = 0;
  p->pFrame = u.by.pFrame;
  p->aMem = aMem = &VdbeFrameMem(u.by.pFrame)[-1];
  p->nMem = u.by.pFrame->nChildMem;
  p->nCursor = (u16)u.by.pFrame->nChildCsr;
  p->apCsr = (VdbeCursor **)&aMem[p->nMem+1];
  p->aOp = aOp = u.by.pProgram->aOp;
  p->nOp = u.by.pProgram->nOp;
  pc = -1;

  break;
}

/* Opcode: Param P1 P2 * * *
**
** This opcode is only ever present in sub-programs called via the 
** OP_Program instruction. Copy a value currently stored in a memory 
** cell of the calling (parent) frame to cell P2 in the current frames 
** address space. This is used by trigger programs to access the new.* 
** and old.* values.
**
** The address of the cell in the parent frame is determined by adding
** the value of the P1 argument to the value of the P1 argument to the
** calling OP_Program instruction.
*/
case OP_Param: {           /* out2-prerelease */
#if 0  /* local variables moved into u.bz */
  VdbeFrame *pFrame;
  Mem *pIn;
#endif /* local variables moved into u.bz */
  u.bz.pFrame = p->pFrame;
  u.bz.pIn = &u.bz.pFrame->aMem[pOp->p1 + u.bz.pFrame->aOp[u.bz.pFrame->pc].p1];
  sqlite3VdbeMemShallowCopy(pOut, u.bz.pIn, MEM_Ephem);
  break;
}

#endif /* #ifndef SQLITE_OMIT_TRIGGER */

#ifndef SQLITE_OMIT_FOREIGN_KEY
/* Opcode: FkCounter P1 P2 * * *
**
** Increment a "constraint counter" by P2 (P2 may be negative or positive).
** If P1 is non-zero, the database constraint counter is incremented 
** (deferred foreign key constraints). Otherwise, if P1 is zero, the 
** statement counter is incremented (immediate foreign key constraints).
*/
case OP_FkCounter: {
  if( pOp->p1 ){
    db->nDeferredCons += pOp->p2;
  }else{
    p->nFkConstraint += pOp->p2;
  }
  break;
}

/* Opcode: FkIfZero P1 P2 * * *
**
** This opcode tests if a foreign key constraint-counter is currently zero.
** If so, jump to instruction P2. Otherwise, fall through to the next 
** instruction.
**
** If P1 is non-zero, then the jump is taken if the database constraint-counter
** is zero (the one that counts deferred constraint violations). If P1 is
** zero, the jump is taken if the statement constraint-counter is zero
** (immediate foreign key constraint violations).
*/
case OP_FkIfZero: {         /* jump */
  if( pOp->p1 ){
    if( db->nDeferredCons==0 ) pc = pOp->p2-1;
  }else{
    if( p->nFkConstraint==0 ) pc = pOp->p2-1;
  }
  break;
}
#endif /* #ifndef SQLITE_OMIT_FOREIGN_KEY */

#ifndef SQLITE_OMIT_AUTOINCREMENT
/* Opcode: MemMax P1 P2 * * *
**
** P1 is a register in the root frame of this VM (the root frame is
** different from the current frame if this instruction is being executed
** within a sub-program). Set the value of register P1 to the maximum of 
** its current value and the value in register P2.
**
** This instruction throws an error if the memory cell is not initially
** an integer.
*/
case OP_MemMax: {        /* in2 */
#if 0  /* local variables moved into u.ca */
  Mem *pIn1;
  VdbeFrame *pFrame;
#endif /* local variables moved into u.ca */
  if( p->pFrame ){
    for(u.ca.pFrame=p->pFrame; u.ca.pFrame->pParent; u.ca.pFrame=u.ca.pFrame->pParent);
    u.ca.pIn1 = &u.ca.pFrame->aMem[pOp->p1];
  }else{
    u.ca.pIn1 = &aMem[pOp->p1];
  }
  assert( memIsValid(u.ca.pIn1) );
  sqlite3VdbeMemIntegerify(u.ca.pIn1);
  pIn2 = &aMem[pOp->p2];
  sqlite3VdbeMemIntegerify(pIn2);
  if( u.ca.pIn1->u.i<pIn2->u.i){
    u.ca.pIn1->u.i = pIn2->u.i;
  }
  break;
}
#endif /* SQLITE_OMIT_AUTOINCREMENT */

/* Opcode: IfPos P1 P2 * * *
**
** If the value of register P1 is 1 or greater, jump to P2.
**
** It is illegal to use this instruction on a register that does
** not contain an integer.  An assertion fault will result if you try.
*/
case OP_IfPos: {        /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  assert( pIn1->flags&MEM_Int );
  if( pIn1->u.i>0 ){
     pc = pOp->p2 - 1;
  }
  break;
}

/* Opcode: IfNeg P1 P2 * * *
**
** If the value of register P1 is less than zero, jump to P2. 
**
** It is illegal to use this instruction on a register that does
** not contain an integer.  An assertion fault will result if you try.
*/
case OP_IfNeg: {        /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  assert( pIn1->flags&MEM_Int );
  if( pIn1->u.i<0 ){
     pc = pOp->p2 - 1;
  }
  break;
}

/* Opcode: IfZero P1 P2 P3 * *
**
** The register P1 must contain an integer.  Add literal P3 to the
** value in register P1.  If the result is exactly 0, jump to P2. 
**
** It is illegal to use this instruction on a register that does
** not contain an integer.  An assertion fault will result if you try.
*/
case OP_IfZero: {        /* jump, in1 */
  pIn1 = &aMem[pOp->p1];
  assert( pIn1->flags&MEM_Int );
  pIn1->u.i += pOp->p3;
  if( pIn1->u.i==0 ){
     pc = pOp->p2 - 1;
  }
  break;
}

/* Opcode: AggStep * P2 P3 P4 P5
**
** Execute the step function for an aggregate.  The
** function has P5 arguments.   P4 is a pointer to the FuncDef
** structure that specifies the function.  Use register
** P3 as the accumulator.
**
** The P5 arguments are taken from register P2 and its
** successors.
*/
case OP_AggStep: {
#if 0  /* local variables moved into u.cb */
  int n;
  int i;
  Mem *pMem;
  Mem *pRec;
  sqlite3_context ctx;
  sqlite3_value **apVal;
#endif /* local variables moved into u.cb */

  u.cb.n = pOp->p5;
  assert( u.cb.n>=0 );
  u.cb.pRec = &aMem[pOp->p2];
  u.cb.apVal = p->apArg;
  assert( u.cb.apVal || u.cb.n==0 );
  for(u.cb.i=0; u.cb.i<u.cb.n; u.cb.i++, u.cb.pRec++){
    assert( memIsValid(u.cb.pRec) );
    u.cb.apVal[u.cb.i] = u.cb.pRec;
    memAboutToChange(p, u.cb.pRec);
    sqlite3VdbeMemStoreType(u.cb.pRec);
  }
  u.cb.ctx.pFunc = pOp->p4.pFunc;
  assert( pOp->p3>0 && pOp->p3<=p->nMem );
  u.cb.ctx.pMem = u.cb.pMem = &aMem[pOp->p3];
  u.cb.pMem->n++;
  u.cb.ctx.s.flags = MEM_Null;
  u.cb.ctx.s.z = 0;
  u.cb.ctx.s.zMalloc = 0;
  u.cb.ctx.s.xDel = 0;
  u.cb.ctx.s.db = db;
  u.cb.ctx.isError = 0;
  u.cb.ctx.pColl = 0;
  if( u.cb.ctx.pFunc->flags & SQLITE_FUNC_NEEDCOLL ){
    assert( pOp>p->aOp );
    assert( pOp[-1].p4type==P4_COLLSEQ );
    assert( pOp[-1].opcode==OP_CollSeq );
    u.cb.ctx.pColl = pOp[-1].p4.pColl;
  }
  (u.cb.ctx.pFunc->xStep)(&u.cb.ctx, u.cb.n, u.cb.apVal); /* IMP: R-24505-23230 */
  if( u.cb.ctx.isError ){
    sqlite3SetString(&p->zErrMsg, db, "%s", sqlite3_value_text(&u.cb.ctx.s));
    rc = u.cb.ctx.isError;
  }

  sqlite3VdbeMemRelease(&u.cb.ctx.s);

  break;
}

/* Opcode: AggFinal P1 P2 * P4 *
**
** Execute the finalizer function for an aggregate.  P1 is
** the memory location that is the accumulator for the aggregate.
**
** P2 is the number of arguments that the step function takes and
** P4 is a pointer to the FuncDef for this function.  The P2
** argument is not used by this opcode.  It is only there to disambiguate
** functions that can take varying numbers of arguments.  The
** P4 argument is only needed for the degenerate case where
** the step function was not previously called.
*/
case OP_AggFinal: {
#if 0  /* local variables moved into u.cc */
  Mem *pMem;
#endif /* local variables moved into u.cc */
  assert( pOp->p1>0 && pOp->p1<=p->nMem );
  u.cc.pMem = &aMem[pOp->p1];
  assert( (u.cc.pMem->flags & ~(MEM_Null|MEM_Agg))==0 );
  rc = sqlite3VdbeMemFinalize(u.cc.pMem, pOp->p4.pFunc);
  if( rc ){
    sqlite3SetString(&p->zErrMsg, db, "%s", sqlite3_value_text(u.cc.pMem));
  }
  sqlite3VdbeChangeEncoding(u.cc.pMem, encoding);
  UPDATE_MAX_BLOBSIZE(u.cc.pMem);
  if( sqlite3VdbeMemTooBig(u.cc.pMem) ){
    goto too_big;
  }
  break;
}

#ifndef SQLITE_OMIT_WAL
/* Opcode: Checkpoint P1 P2 P3 * *
**
** Checkpoint database P1. This is a no-op if P1 is not currently in
** WAL mode. Parameter P2 is one of SQLITE_CHECKPOINT_PASSIVE, FULL
** or RESTART.  Write 1 or 0 into mem[P3] if the checkpoint returns
** SQLITE_BUSY or not, respectively.  Write the number of pages in the
** WAL after the checkpoint into mem[P3+1] and the number of pages
** in the WAL that have been checkpointed after the checkpoint
** completes into mem[P3+2].  However on an error, mem[P3+1] and
** mem[P3+2] are initialized to -1.
*/
case OP_Checkpoint: {
#if 0  /* local variables moved into u.cd */
  int i;                          /* Loop counter */
  int aRes[3];                    /* Results */
  Mem *pMem;                      /* Write results here */
#endif /* local variables moved into u.cd */

  u.cd.aRes[0] = 0;
  u.cd.aRes[1] = u.cd.aRes[2] = -1;
  assert( pOp->p2==SQLITE_CHECKPOINT_PASSIVE
       || pOp->p2==SQLITE_CHECKPOINT_FULL
       || pOp->p2==SQLITE_CHECKPOINT_RESTART
  );
  rc = sqlite3Checkpoint(db, pOp->p1, pOp->p2, &u.cd.aRes[1], &u.cd.aRes[2]);
  if( rc==SQLITE_BUSY ){
    rc = SQLITE_OK;
    u.cd.aRes[0] = 1;
  }
  for(u.cd.i=0, u.cd.pMem = &aMem[pOp->p3]; u.cd.i<3; u.cd.i++, u.cd.pMem++){
    sqlite3VdbeMemSetInt64(u.cd.pMem, (i64)u.cd.aRes[u.cd.i]);
  }
  break;
};  
#endif

#ifndef SQLITE_OMIT_PRAGMA
/* Opcode: JournalMode P1 P2 P3 * P5
**
** Change the journal mode of database P1 to P3. P3 must be one of the
** PAGER_JOURNALMODE_XXX values. If changing between the various rollback
** modes (delete, truncate, persist, off and memory), this is a simple
** operation. No IO is required.
**
** If changing into or out of WAL mode the procedure is more complicated.
**
** Write a string containing the final journal-mode to register P2.
*/
case OP_JournalMode: {    /* out2-prerelease */
#if 0  /* local variables moved into u.ce */
  Btree *pBt;                     /* Btree to change journal mode of */
  Pager *pPager;                  /* Pager associated with pBt */
  int eNew;                       /* New journal mode */
  int eOld;                       /* The old journal mode */
  const char *zFilename;          /* Name of database file for pPager */
#endif /* local variables moved into u.ce */

  u.ce.eNew = pOp->p3;
  assert( u.ce.eNew==PAGER_JOURNALMODE_DELETE
       || u.ce.eNew==PAGER_JOURNALMODE_TRUNCATE
       || u.ce.eNew==PAGER_JOURNALMODE_PERSIST
       || u.ce.eNew==PAGER_JOURNALMODE_OFF
       || u.ce.eNew==PAGER_JOURNALMODE_MEMORY
       || u.ce.eNew==PAGER_JOURNALMODE_WAL
       || u.ce.eNew==PAGER_JOURNALMODE_QUERY
  );
  assert( pOp->p1>=0 && pOp->p1<db->nDb );

  u.ce.pBt = db->aDb[pOp->p1].pBt;
  u.ce.pPager = sqlite3BtreePager(u.ce.pBt);
  u.ce.eOld = sqlite3PagerGetJournalMode(u.ce.pPager);
  if( u.ce.eNew==PAGER_JOURNALMODE_QUERY ) u.ce.eNew = u.ce.eOld;
  if( !sqlite3PagerOkToChangeJournalMode(u.ce.pPager) ) u.ce.eNew = u.ce.eOld;

#ifndef SQLITE_OMIT_WAL
  u.ce.zFilename = sqlite3PagerFilename(u.ce.pPager);

  /* Do not allow a transition to journal_mode=WAL for a database
  ** in temporary storage or if the VFS does not support shared memory
  */
  if( u.ce.eNew==PAGER_JOURNALMODE_WAL
   && (u.ce.zFilename[0]==0                         /* Temp file */
       || !sqlite3PagerWalSupported(u.ce.pPager))   /* No shared-memory support */
  ){
    u.ce.eNew = u.ce.eOld;
  }

  if( (u.ce.eNew!=u.ce.eOld)
   && (u.ce.eOld==PAGER_JOURNALMODE_WAL || u.ce.eNew==PAGER_JOURNALMODE_WAL)
  ){
    if( !db->autoCommit || db->activeVdbeCnt>1 ){
      rc = SQLITE_ERROR;
      sqlite3SetString(&p->zErrMsg, db,
          "cannot change %s wal mode from within a transaction",
          (u.ce.eNew==PAGER_JOURNALMODE_WAL ? "into" : "out of")
      );
      break;
    }else{

      if( u.ce.eOld==PAGER_JOURNALMODE_WAL ){
        /* If leaving WAL mode, close the log file. If successful, the call
        ** to PagerCloseWal() checkpoints and deletes the write-ahead-log
        ** file. An EXCLUSIVE lock may still be held on the database file
        ** after a successful return.
        */
        rc = sqlite3PagerCloseWal(u.ce.pPager);
        if( rc==SQLITE_OK ){
          sqlite3PagerSetJournalMode(u.ce.pPager, u.ce.eNew);
        }
      }else if( u.ce.eOld==PAGER_JOURNALMODE_MEMORY ){
        /* Cannot transition directly from MEMORY to WAL.  Use mode OFF
        ** as an intermediate */
        sqlite3PagerSetJournalMode(u.ce.pPager, PAGER_JOURNALMODE_OFF);
      }

      /* Open a transaction on the database file. Regardless of the journal
      ** mode, this transaction always uses a rollback journal.
      */
      assert( sqlite3BtreeIsInTrans(u.ce.pBt)==0 );
      if( rc==SQLITE_OK ){
        rc = sqlite3BtreeSetVersion(u.ce.pBt, (u.ce.eNew==PAGER_JOURNALMODE_WAL ? 2 : 1));
      }
    }
  }
#endif /* ifndef SQLITE_OMIT_WAL */

  if( rc ){
    u.ce.eNew = u.ce.eOld;
  }
  u.ce.eNew = sqlite3PagerSetJournalMode(u.ce.pPager, u.ce.eNew);

  pOut = &aMem[pOp->p2];
  pOut->flags = MEM_Str|MEM_Static|MEM_Term;
  pOut->z = (char *)sqlite3JournalModename(u.ce.eNew);
  pOut->n = sqlite3Strlen30(pOut->z);
  pOut->enc = SQLITE_UTF8;
  sqlite3VdbeChangeEncoding(pOut, encoding);
  break;
};
#endif /* SQLITE_OMIT_PRAGMA */

#if !defined(SQLITE_OMIT_VACUUM) && !defined(SQLITE_OMIT_ATTACH)
/* Opcode: Vacuum * * * * *
**
** Vacuum the entire database.  This opcode will cause other virtual
** machines to be created and run.  It may not be called from within
** a transaction.
*/
case OP_Vacuum: {
  rc = sqlite3RunVacuum(&p->zErrMsg, db);
  break;
}
#endif

#if !defined(SQLITE_OMIT_AUTOVACUUM)
/* Opcode: IncrVacuum P1 P2 * * *
**
** Perform a single step of the incremental vacuum procedure on
** the P1 database. If the vacuum has finished, jump to instruction
** P2. Otherwise, fall through to the next instruction.
*/
case OP_IncrVacuum: {        /* jump */
#if 0  /* local variables moved into u.cf */
  Btree *pBt;
#endif /* local variables moved into u.cf */

  assert( pOp->p1>=0 && pOp->p1<db->nDb );
  assert( (p->btreeMask & (((yDbMask)1)<<pOp->p1))!=0 );
  u.cf.pBt = db->aDb[pOp->p1].pBt;
  rc = sqlite3BtreeIncrVacuum(u.cf.pBt);
  if( rc==SQLITE_DONE ){
    pc = pOp->p2 - 1;
    rc = SQLITE_OK;
  }
  break;
}
#endif

/* Opcode: Expire P1 * * * *
**
** Cause precompiled statements to become expired. An expired statement
** fails with an error code of SQLITE_SCHEMA if it is ever executed 
** (via sqlite3_step()).
** 
** If P1 is 0, then all SQL statements become expired. If P1 is non-zero,
** then only the currently executing statement is affected. 
*/
case OP_Expire: {
  if( !pOp->p1 ){
    sqlite3ExpirePreparedStatements(db);
  }else{
    p->expired = 1;
  }
  break;
}

#ifndef SQLITE_OMIT_SHARED_CACHE
/* Opcode: TableLock P1 P2 P3 P4 *
**
** Obtain a lock on a particular table. This instruction is only used when
** the shared-cache feature is enabled. 
**
** P1 is the index of the database in sqlite3.aDb[] of the database
** on which the lock is acquired.  A readlock is obtained if P3==0 or
** a write lock if P3==1.
**
** P2 contains the root-page of the table to lock.
**
** P4 contains a pointer to the name of the table being locked. This is only
** used to generate an error message if the lock cannot be obtained.
*/
case OP_TableLock: {
  u8 isWriteLock = (u8)pOp->p3;
  if( isWriteLock || 0==(db->flags&SQLITE_ReadUncommitted) ){
    int p1 = pOp->p1; 
    assert( p1>=0 && p1<db->nDb );
    assert( (p->btreeMask & (((yDbMask)1)<<p1))!=0 );
    assert( isWriteLock==0 || isWriteLock==1 );
    rc = sqlite3BtreeLockTable(db->aDb[p1].pBt, pOp->p2, isWriteLock);
    if( (rc&0xFF)==SQLITE_LOCKED ){
      const char *z = pOp->p4.z;
      sqlite3SetString(&p->zErrMsg, db, "database table is locked: %s", z);
    }
  }
  break;
}
#endif /* SQLITE_OMIT_SHARED_CACHE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VBegin * * * P4 *
**
** P4 may be a pointer to an sqlite3_vtab structure. If so, call the 
** xBegin method for that table.
**
** Also, whether or not P4 is set, check that this is not being called from
** within a callback to a virtual table xSync() method. If it is, the error
** code will be set to SQLITE_LOCKED.
*/
case OP_VBegin: {
#if 0  /* local variables moved into u.cg */
  VTable *pVTab;
#endif /* local variables moved into u.cg */
  u.cg.pVTab = pOp->p4.pVtab;
  rc = sqlite3VtabBegin(db, u.cg.pVTab);
  if( u.cg.pVTab ) importVtabErrMsg(p, u.cg.pVTab->pVtab);
  break;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VCreate P1 * * P4 *
**
** P4 is the name of a virtual table in database P1. Call the xCreate method
** for that table.
*/
case OP_VCreate: {
  rc = sqlite3VtabCallCreate(db, pOp->p1, pOp->p4.z, &p->zErrMsg);
  break;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VDestroy P1 * * P4 *
**
** P4 is the name of a virtual table in database P1.  Call the xDestroy method
** of that table.
*/
case OP_VDestroy: {
  p->inVtabMethod = 2;
  rc = sqlite3VtabCallDestroy(db, pOp->p1, pOp->p4.z);
  p->inVtabMethod = 0;
  break;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VOpen P1 * * P4 *
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** P1 is a cursor number.  This opcode opens a cursor to the virtual
** table and stores that cursor in P1.
*/
case OP_VOpen: {
#if 0  /* local variables moved into u.ch */
  VdbeCursor *pCur;
  sqlite3_vtab_cursor *pVtabCursor;
  sqlite3_vtab *pVtab;
  sqlite3_module *pModule;
#endif /* local variables moved into u.ch */

  u.ch.pCur = 0;
  u.ch.pVtabCursor = 0;
  u.ch.pVtab = pOp->p4.pVtab->pVtab;
  u.ch.pModule = (sqlite3_module *)u.ch.pVtab->pModule;
  assert(u.ch.pVtab && u.ch.pModule);
  rc = u.ch.pModule->xOpen(u.ch.pVtab, &u.ch.pVtabCursor);
  importVtabErrMsg(p, u.ch.pVtab);
  if( SQLITE_OK==rc ){
    /* Initialize sqlite3_vtab_cursor base class */
    u.ch.pVtabCursor->pVtab = u.ch.pVtab;

    /* Initialise vdbe cursor object */
    u.ch.pCur = allocateCursor(p, pOp->p1, 0, -1, 0);
    if( u.ch.pCur ){
      u.ch.pCur->pVtabCursor = u.ch.pVtabCursor;
      u.ch.pCur->pModule = u.ch.pVtabCursor->pVtab->pModule;
    }else{
      db->mallocFailed = 1;
      u.ch.pModule->xClose(u.ch.pVtabCursor);
    }
  }
  break;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VFilter P1 P2 P3 P4 *
**
** P1 is a cursor opened using VOpen.  P2 is an address to jump to if
** the filtered result set is empty.
**
** P4 is either NULL or a string that was generated by the xBestIndex
** method of the module.  The interpretation of the P4 string is left
** to the module implementation.
**
** This opcode invokes the xFilter method on the virtual table specified
** by P1.  The integer query plan parameter to xFilter is stored in register
** P3. Register P3+1 stores the argc parameter to be passed to the
** xFilter method. Registers P3+2..P3+1+argc are the argc
** additional parameters which are passed to
** xFilter as argv. Register P3+2 becomes argv[0] when passed to xFilter.
**
** A jump is made to P2 if the result set after filtering would be empty.
*/
case OP_VFilter: {   /* jump */
#if 0  /* local variables moved into u.ci */
  int nArg;
  int iQuery;
  const sqlite3_module *pModule;
  Mem *pQuery;
  Mem *pArgc;
  sqlite3_vtab_cursor *pVtabCursor;
  sqlite3_vtab *pVtab;
  VdbeCursor *pCur;
  int res;
  int i;
  Mem **apArg;
#endif /* local variables moved into u.ci */

  u.ci.pQuery = &aMem[pOp->p3];
  u.ci.pArgc = &u.ci.pQuery[1];
  u.ci.pCur = p->apCsr[pOp->p1];
  assert( memIsValid(u.ci.pQuery) );
  REGISTER_TRACE(pOp->p3, u.ci.pQuery);
  assert( u.ci.pCur->pVtabCursor );
  u.ci.pVtabCursor = u.ci.pCur->pVtabCursor;
  u.ci.pVtab = u.ci.pVtabCursor->pVtab;
  u.ci.pModule = u.ci.pVtab->pModule;

  /* Grab the index number and argc parameters */
  assert( (u.ci.pQuery->flags&MEM_Int)!=0 && u.ci.pArgc->flags==MEM_Int );
  u.ci.nArg = (int)u.ci.pArgc->u.i;
  u.ci.iQuery = (int)u.ci.pQuery->u.i;

  /* Invoke the xFilter method */
  {
    u.ci.res = 0;
    u.ci.apArg = p->apArg;
    for(u.ci.i = 0; u.ci.i<u.ci.nArg; u.ci.i++){
      u.ci.apArg[u.ci.i] = &u.ci.pArgc[u.ci.i+1];
      sqlite3VdbeMemStoreType(u.ci.apArg[u.ci.i]);
    }

    p->inVtabMethod = 1;
    rc = u.ci.pModule->xFilter(u.ci.pVtabCursor, u.ci.iQuery, pOp->p4.z, u.ci.nArg, u.ci.apArg);
    p->inVtabMethod = 0;
    importVtabErrMsg(p, u.ci.pVtab);
    if( rc==SQLITE_OK ){
      u.ci.res = u.ci.pModule->xEof(u.ci.pVtabCursor);
    }

    if( u.ci.res ){
      pc = pOp->p2 - 1;
    }
  }
  u.ci.pCur->nullRow = 0;

  break;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VColumn P1 P2 P3 * *
**
** Store the value of the P2-th column of
** the row of the virtual-table that the 
** P1 cursor is pointing to into register P3.
*/
case OP_VColumn: {
#if 0  /* local variables moved into u.cj */
  sqlite3_vtab *pVtab;
  const sqlite3_module *pModule;
  Mem *pDest;
  sqlite3_context sContext;
#endif /* local variables moved into u.cj */

  VdbeCursor *pCur = p->apCsr[pOp->p1];
  assert( pCur->pVtabCursor );
  assert( pOp->p3>0 && pOp->p3<=p->nMem );
  u.cj.pDest = &aMem[pOp->p3];
  memAboutToChange(p, u.cj.pDest);
  if( pCur->nullRow ){
    sqlite3VdbeMemSetNull(u.cj.pDest);
    break;
  }
  u.cj.pVtab = pCur->pVtabCursor->pVtab;
  u.cj.pModule = u.cj.pVtab->pModule;
  assert( u.cj.pModule->xColumn );
  memset(&u.cj.sContext, 0, sizeof(u.cj.sContext));

  /* The output cell may already have a buffer allocated. Move
  ** the current contents to u.cj.sContext.s so in case the user-function
  ** can use the already allocated buffer instead of allocating a
  ** new one.
  */
  sqlite3VdbeMemMove(&u.cj.sContext.s, u.cj.pDest);
  MemSetTypeFlag(&u.cj.sContext.s, MEM_Null);

  rc = u.cj.pModule->xColumn(pCur->pVtabCursor, &u.cj.sContext, pOp->p2);
  importVtabErrMsg(p, u.cj.pVtab);
  if( u.cj.sContext.isError ){
    rc = u.cj.sContext.isError;
  }

  /* Copy the result of the function to the P3 register. We
  ** do this regardless of whether or not an error occurred to ensure any
  ** dynamic allocation in u.cj.sContext.s (a Mem struct) is  released.
  */
  sqlite3VdbeChangeEncoding(&u.cj.sContext.s, encoding);
  sqlite3VdbeMemMove(u.cj.pDest, &u.cj.sContext.s);
  REGISTER_TRACE(pOp->p3, u.cj.pDest);
  UPDATE_MAX_BLOBSIZE(u.cj.pDest);

  if( sqlite3VdbeMemTooBig(u.cj.pDest) ){
    goto too_big;
  }
  break;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VNext P1 P2 * * *
**
** Advance virtual table P1 to the next row in its result set and
** jump to instruction P2.  Or, if the virtual table has reached
** the end of its result set, then fall through to the next instruction.
*/
case OP_VNext: {   /* jump */
#if 0  /* local variables moved into u.ck */
  sqlite3_vtab *pVtab;
  const sqlite3_module *pModule;
  int res;
  VdbeCursor *pCur;
#endif /* local variables moved into u.ck */

  u.ck.res = 0;
  u.ck.pCur = p->apCsr[pOp->p1];
  assert( u.ck.pCur->pVtabCursor );
  if( u.ck.pCur->nullRow ){
    break;
  }
  u.ck.pVtab = u.ck.pCur->pVtabCursor->pVtab;
  u.ck.pModule = u.ck.pVtab->pModule;
  assert( u.ck.pModule->xNext );

  /* Invoke the xNext() method of the module. There is no way for the
  ** underlying implementation to return an error if one occurs during
  ** xNext(). Instead, if an error occurs, true is returned (indicating that
  ** data is available) and the error code returned when xColumn or
  ** some other method is next invoked on the save virtual table cursor.
  */
  p->inVtabMethod = 1;
  rc = u.ck.pModule->xNext(u.ck.pCur->pVtabCursor);
  p->inVtabMethod = 0;
  importVtabErrMsg(p, u.ck.pVtab);
  if( rc==SQLITE_OK ){
    u.ck.res = u.ck.pModule->xEof(u.ck.pCur->pVtabCursor);
  }

  if( !u.ck.res ){
    /* If there is data, jump to P2 */
    pc = pOp->p2 - 1;
  }
  break;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VRename P1 * * P4 *
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** This opcode invokes the corresponding xRename method. The value
** in register P1 is passed as the zName argument to the xRename method.
*/
case OP_VRename: {
#if 0  /* local variables moved into u.cl */
  sqlite3_vtab *pVtab;
  Mem *pName;
#endif /* local variables moved into u.cl */

  u.cl.pVtab = pOp->p4.pVtab->pVtab;
  u.cl.pName = &aMem[pOp->p1];
  assert( u.cl.pVtab->pModule->xRename );
  assert( memIsValid(u.cl.pName) );
  REGISTER_TRACE(pOp->p1, u.cl.pName);
  assert( u.cl.pName->flags & MEM_Str );
  rc = u.cl.pVtab->pModule->xRename(u.cl.pVtab, u.cl.pName->z);
  importVtabErrMsg(p, u.cl.pVtab);
  p->expired = 0;

  break;
}
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
/* Opcode: VUpdate P1 P2 P3 P4 *
**
** P4 is a pointer to a virtual table object, an sqlite3_vtab structure.
** This opcode invokes the corresponding xUpdate method. P2 values
** are contiguous memory cells starting at P3 to pass to the xUpdate 
** invocation. The value in register (P3+P2-1) corresponds to the 
** p2th element of the argv array passed to xUpdate.
**
** The xUpdate method will do a DELETE or an INSERT or both.
** The argv[0] element (which corresponds to memory cell P3)
** is the rowid of a row to delete.  If argv[0] is NULL then no 
** deletion occurs.  The argv[1] element is the rowid of the new 
** row.  This can be NULL to have the virtual table select the new 
** rowid for itself.  The subsequent elements in the array are 
** the values of columns in the new row.
**
** If P2==1 then no insert is performed.  argv[0] is the rowid of
** a row to delete.
**
** P1 is a boolean flag. If it is set to true and the xUpdate call
** is successful, then the value returned by sqlite3_last_insert_rowid() 
** is set to the value of the rowid for the row just inserted.
*/
case OP_VUpdate: {
#if 0  /* local variables moved into u.cm */
  sqlite3_vtab *pVtab;
  sqlite3_module *pModule;
  int nArg;
  int i;
  sqlite_int64 rowid;
  Mem **apArg;
  Mem *pX;
#endif /* local variables moved into u.cm */

  assert( pOp->p2==1        || pOp->p5==OE_Fail   || pOp->p5==OE_Rollback
       || pOp->p5==OE_Abort || pOp->p5==OE_Ignore || pOp->p5==OE_Replace
  );
  u.cm.pVtab = pOp->p4.pVtab->pVtab;
  u.cm.pModule = (sqlite3_module *)u.cm.pVtab->pModule;
  u.cm.nArg = pOp->p2;
  assert( pOp->p4type==P4_VTAB );
  if( ALWAYS(u.cm.pModule->xUpdate) ){
    u8 vtabOnConflict = db->vtabOnConflict;
    u.cm.apArg = p->apArg;
    u.cm.pX = &aMem[pOp->p3];
    for(u.cm.i=0; u.cm.i<u.cm.nArg; u.cm.i++){
      assert( memIsValid(u.cm.pX) );
      memAboutToChange(p, u.cm.pX);
      sqlite3VdbeMemStoreType(u.cm.pX);
      u.cm.apArg[u.cm.i] = u.cm.pX;
      u.cm.pX++;
    }
    db->vtabOnConflict = pOp->p5;
    rc = u.cm.pModule->xUpdate(u.cm.pVtab, u.cm.nArg, u.cm.apArg, &u.cm.rowid);
    db->vtabOnConflict = vtabOnConflict;
    importVtabErrMsg(p, u.cm.pVtab);
    if( rc==SQLITE_OK && pOp->p1 ){
      assert( u.cm.nArg>1 && u.cm.apArg[0] && (u.cm.apArg[0]->flags&MEM_Null) );
      db->lastRowid = lastRowid = u.cm.rowid;
    }
    if( rc==SQLITE_CONSTRAINT && pOp->p4.pVtab->bConstraint ){
      if( pOp->p5==OE_Ignore ){
        rc = SQLITE_OK;
      }else{
        p->errorAction = ((pOp->p5==OE_Replace) ? OE_Abort : pOp->p5);
      }
    }else{
      p->nChange++;
    }
  }
  break;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifndef  SQLITE_OMIT_PAGER_PRAGMAS
/* Opcode: Pagecount P1 P2 * * *
**
** Write the current number of pages in database P1 to memory cell P2.
*/
case OP_Pagecount: {            /* out2-prerelease */
  pOut->u.i = sqlite3BtreeLastPage(db->aDb[pOp->p1].pBt);
  break;
}
#endif


#ifndef  SQLITE_OMIT_PAGER_PRAGMAS
/* Opcode: MaxPgcnt P1 P2 P3 * *
**
** Try to set the maximum page count for database P1 to the value in P3.
** Do not let the maximum page count fall below the current page count and
** do not change the maximum page count value if P3==0.
**
** Store the maximum page count after the change in register P2.
*/
case OP_MaxPgcnt: {            /* out2-prerelease */
  unsigned int newMax;
  Btree *pBt;

  pBt = db->aDb[pOp->p1].pBt;
  newMax = 0;
  if( pOp->p3 ){
    newMax = sqlite3BtreeLastPage(pBt);
    if( newMax < (unsigned)pOp->p3 ) newMax = (unsigned)pOp->p3;
  }
  pOut->u.i = sqlite3BtreeMaxPageCount(pBt, newMax);
  break;
}
#endif


#ifndef SQLITE_OMIT_TRACE
/* Opcode: Trace * * * P4 *
**
** If tracing is enabled (by the sqlite3_trace()) interface, then
** the UTF-8 string contained in P4 is emitted on the trace callback.
*/
case OP_Trace: {
#if 0  /* local variables moved into u.cn */
  char *zTrace;
  char *z;
#endif /* local variables moved into u.cn */

  if( db->xTrace && (u.cn.zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql))!=0 ){
    u.cn.z = sqlite3VdbeExpandSql(p, u.cn.zTrace);
    db->xTrace(db->pTraceArg, u.cn.z);
    sqlite3DbFree(db, u.cn.z);
  }
#ifdef SQLITE_DEBUG
  if( (db->flags & SQLITE_SqlTrace)!=0
   && (u.cn.zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql))!=0
  ){
    sqlite3DebugPrintf("SQL-trace: %s\n", u.cn.zTrace);
  }
#endif /* SQLITE_DEBUG */
  break;
}
#endif


/* Opcode: Noop * * * * *
**
** Do nothing.  This instruction is often useful as a jump
** destination.
*/
/*
** The magic Explain opcode are only inserted when explain==2 (which
** is to say when the EXPLAIN QUERY PLAN syntax is used.)
** This opcode records information from the optimizer.  It is the
** the same as a no-op.  This opcodesnever appears in a real VM program.
*/
default: {          /* This is really OP_Noop and OP_Explain */
  assert( pOp->opcode==OP_Noop || pOp->opcode==OP_Explain );
  break;
}

/*****************************************************************************
** The cases of the switch statement above this line should all be indented
** by 6 spaces.  But the left-most 6 spaces have been removed to improve the
** readability.  From this point on down, the normal indentation rules are
** restored.
*****************************************************************************/
    }

#ifdef VDBE_PROFILE
    {
      u64 elapsed = sqlite3Hwtime() - start;
      pOp->cycles += elapsed;
      pOp->cnt++;
#if 0
        fprintf(stdout, "%10llu ", elapsed);
        sqlite3VdbePrintOp(stdout, origPc, &aOp[origPc]);
#endif
    }
#endif

    /* The following code adds nothing to the actual functionality
    ** of the program.  It is only here for testing and debugging.
    ** On the other hand, it does burn CPU cycles every time through
    ** the evaluator loop.  So we can leave it out when NDEBUG is defined.
    */
#ifndef NDEBUG
    assert( pc>=-1 && pc<p->nOp );

#ifdef SQLITE_DEBUG
    if( p->trace ){
      if( rc!=0 ) fprintf(p->trace,"rc=%d\n",rc);
      if( pOp->opflags & (OPFLG_OUT2_PRERELEASE|OPFLG_OUT2) ){
        registerTrace(p->trace, pOp->p2, &aMem[pOp->p2]);
      }
      if( pOp->opflags & OPFLG_OUT3 ){
        registerTrace(p->trace, pOp->p3, &aMem[pOp->p3]);
      }
    }
#endif  /* SQLITE_DEBUG */
#endif  /* NDEBUG */
  }  /* The end of the for(;;) loop the loops through opcodes */

  /* If we reach this point, it means that execution is finished with
  ** an error of some kind.
  */
vdbe_error_halt:
  assert( rc );
  p->rc = rc;
  testcase( sqlite3GlobalConfig.xLog!=0 );
  sqlite3_log(rc, "statement aborts at %d: [%s] %s", 
                   pc, p->zSql, p->zErrMsg);
  sqlite3VdbeHalt(p);
  if( rc==SQLITE_IOERR_NOMEM ) db->mallocFailed = 1;
  rc = SQLITE_ERROR;
  if( resetSchemaOnFault>0 ){
    sqlite3ResetInternalSchema(db, resetSchemaOnFault-1);
  }

  /* This is the only way out of this procedure.  We have to
  ** release the mutexes on btrees that were acquired at the
  ** top. */
vdbe_return:
  db->lastRowid = lastRowid;
  sqlite3VdbeLeave(p);
  return rc;

  /* Jump to here if a string or blob larger than SQLITE_MAX_LENGTH
  ** is encountered.
  */
too_big:
  sqlite3SetString(&p->zErrMsg, db, "string or blob too big");
  rc = SQLITE_TOOBIG;
  goto vdbe_error_halt;

  /* Jump to here if a malloc() fails.
  */
no_mem:
  db->mallocFailed = 1;
  sqlite3SetString(&p->zErrMsg, db, "out of memory");
  rc = SQLITE_NOMEM;
  goto vdbe_error_halt;

  /* Jump to here for any other kind of fatal error.  The "rc" variable
  ** should hold the error number.
  */
abort_due_to_error:
  assert( p->zErrMsg==0 );
  if( db->mallocFailed ) rc = SQLITE_NOMEM;
  if( rc!=SQLITE_IOERR_NOMEM ){
    sqlite3SetString(&p->zErrMsg, db, "%s", sqlite3ErrStr(rc));
  }
  goto vdbe_error_halt;

  /* Jump to here if the sqlite3_interrupt() API sets the interrupt
  ** flag.
  */
abort_due_to_interrupt:
  assert( db->u1.isInterrupted );
  rc = SQLITE_INTERRUPT;
  p->rc = rc;
  sqlite3SetString(&p->zErrMsg, db, "%s", sqlite3ErrStr(rc));
  goto vdbe_error_halt;
}

/************** End of vdbe.c ************************************************/
/************** Begin file vdbeblob.c ****************************************/
/*
** 2007 May 1
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains code used to implement incremental BLOB I/O.
*/


#ifndef SQLITE_OMIT_INCRBLOB

/*
** Valid sqlite3_blob* handles point to Incrblob structures.
*/
typedef struct Incrblob Incrblob;
struct Incrblob {
  int flags;              /* Copy of "flags" passed to sqlite3_blob_open() */
  int nByte;              /* Size of open blob, in bytes */
  int iOffset;            /* Byte offset of blob in cursor data */
  int iCol;               /* Table column this handle is open on */
  BtCursor *pCsr;         /* Cursor pointing at blob row */
  sqlite3_stmt *pStmt;    /* Statement holding cursor open */
  sqlite3 *db;            /* The associated database */
};


/*
** This function is used by both blob_open() and blob_reopen(). It seeks
** the b-tree cursor associated with blob handle p to point to row iRow.
** If successful, SQLITE_OK is returned and subsequent calls to
** sqlite3_blob_read() or sqlite3_blob_write() access the specified row.
**
** If an error occurs, or if the specified row does not exist or does not
** contain a value of type TEXT or BLOB in the column nominated when the
** blob handle was opened, then an error code is returned and *pzErr may
** be set to point to a buffer containing an error message. It is the
** responsibility of the caller to free the error message buffer using
** sqlite3DbFree().
**
** If an error does occur, then the b-tree cursor is closed. All subsequent
** calls to sqlite3_blob_read(), blob_write() or blob_reopen() will 
** immediately return SQLITE_ABORT.
*/
static int blobSeekToRow(Incrblob *p, sqlite3_int64 iRow, char **pzErr){
  int rc;                         /* Error code */
  char *zErr = 0;                 /* Error message */
  Vdbe *v = (Vdbe *)p->pStmt;

  /* Set the value of the SQL statements only variable to integer iRow. 
  ** This is done directly instead of using sqlite3_bind_int64() to avoid 
  ** triggering asserts related to mutexes.
  */
  assert( v->aVar[0].flags&MEM_Int );
  v->aVar[0].u.i = iRow;

  rc = sqlite3_step(p->pStmt);
  if( rc==SQLITE_ROW ){
    u32 type = v->apCsr[0]->aType[p->iCol];
    if( type<12 ){
      zErr = sqlite3MPrintf(p->db, "cannot open value of type %s",
          type==0?"null": type==7?"real": "integer"
      );
      rc = SQLITE_ERROR;
      sqlite3_finalize(p->pStmt);
      p->pStmt = 0;
    }else{
      p->iOffset = v->apCsr[0]->aOffset[p->iCol];
      p->nByte = sqlite3VdbeSerialTypeLen(type);
      p->pCsr =  v->apCsr[0]->pCursor;
      sqlite3BtreeEnterCursor(p->pCsr);
      sqlite3BtreeCacheOverflow(p->pCsr);
      sqlite3BtreeLeaveCursor(p->pCsr);
    }
  }

  if( rc==SQLITE_ROW ){
    rc = SQLITE_OK;
  }else if( p->pStmt ){
    rc = sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
    if( rc==SQLITE_OK ){
      zErr = sqlite3MPrintf(p->db, "no such rowid: %lld", iRow);
      rc = SQLITE_ERROR;
    }else{
      zErr = sqlite3MPrintf(p->db, "%s", sqlite3_errmsg(p->db));
    }
  }

  assert( rc!=SQLITE_OK || zErr==0 );
  assert( rc!=SQLITE_ROW && rc!=SQLITE_DONE );

  *pzErr = zErr;
  return rc;
}

/*
** Open a blob handle.
*/
SQLITE_API int sqlite3_blob_open(
  sqlite3* db,            /* The database connection */
  const char *zDb,        /* The attached database containing the blob */
  const char *zTable,     /* The table containing the blob */
  const char *zColumn,    /* The column containing the blob */
  sqlite_int64 iRow,      /* The row containing the glob */
  int flags,              /* True -> read/write access, false -> read-only */
  sqlite3_blob **ppBlob   /* Handle for accessing the blob returned here */
){
  int nAttempt = 0;
  int iCol;               /* Index of zColumn in row-record */

  /* This VDBE program seeks a btree cursor to the identified 
  ** db/table/row entry. The reason for using a vdbe program instead
  ** of writing code to use the b-tree layer directly is that the
  ** vdbe program will take advantage of the various transaction,
  ** locking and error handling infrastructure built into the vdbe.
  **
  ** After seeking the cursor, the vdbe executes an OP_ResultRow.
  ** Code external to the Vdbe then "borrows" the b-tree cursor and
  ** uses it to implement the blob_read(), blob_write() and 
  ** blob_bytes() functions.
  **
  ** The sqlite3_blob_close() function finalizes the vdbe program,
  ** which closes the b-tree cursor and (possibly) commits the 
  ** transaction.
  */
  static const VdbeOpList openBlob[] = {
    {OP_Transaction, 0, 0, 0},     /* 0: Start a transaction */
    {OP_VerifyCookie, 0, 0, 0},    /* 1: Check the schema cookie */
    {OP_TableLock, 0, 0, 0},       /* 2: Acquire a read or write lock */

    /* One of the following two instructions is replaced by an OP_Noop. */
    {OP_OpenRead, 0, 0, 0},        /* 3: Open cursor 0 for reading */
    {OP_OpenWrite, 0, 0, 0},       /* 4: Open cursor 0 for read/write */

    {OP_Variable, 1, 1, 1},        /* 5: Push the rowid to the stack */
    {OP_NotExists, 0, 10, 1},      /* 6: Seek the cursor */
    {OP_Column, 0, 0, 1},          /* 7  */
    {OP_ResultRow, 1, 0, 0},       /* 8  */
    {OP_Goto, 0, 5, 0},            /* 9  */
    {OP_Close, 0, 0, 0},           /* 10 */
    {OP_Halt, 0, 0, 0},            /* 11 */
  };

  int rc = SQLITE_OK;
  char *zErr = 0;
  Table *pTab;
  Parse *pParse = 0;
  Incrblob *pBlob = 0;

  flags = !!flags;                /* flags = (flags ? 1 : 0); */
  *ppBlob = 0;

  sqlite3_mutex_enter(db->mutex);

  pBlob = (Incrblob *)sqlite3DbMallocZero(db, sizeof(Incrblob));
  if( !pBlob ) goto blob_open_out;
  pParse = sqlite3StackAllocRaw(db, sizeof(*pParse));
  if( !pParse ) goto blob_open_out;

  do {
    memset(pParse, 0, sizeof(Parse));
    pParse->db = db;
    sqlite3DbFree(db, zErr);
    zErr = 0;

    sqlite3BtreeEnterAll(db);
    pTab = sqlite3LocateTable(pParse, 0, zTable, zDb);
    if( pTab && IsVirtual(pTab) ){
      pTab = 0;
      sqlite3ErrorMsg(pParse, "cannot open virtual table: %s", zTable);
    }
#ifndef SQLITE_OMIT_VIEW
    if( pTab && pTab->pSelect ){
      pTab = 0;
      sqlite3ErrorMsg(pParse, "cannot open view: %s", zTable);
    }
#endif
    if( !pTab ){
      if( pParse->zErrMsg ){
        sqlite3DbFree(db, zErr);
        zErr = pParse->zErrMsg;
        pParse->zErrMsg = 0;
      }
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      goto blob_open_out;
    }

    /* Now search pTab for the exact column. */
    for(iCol=0; iCol<pTab->nCol; iCol++) {
      if( sqlite3StrICmp(pTab->aCol[iCol].zName, zColumn)==0 ){
        break;
      }
    }
    if( iCol==pTab->nCol ){
      sqlite3DbFree(db, zErr);
      zErr = sqlite3MPrintf(db, "no such column: \"%s\"", zColumn);
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      goto blob_open_out;
    }

    /* If the value is being opened for writing, check that the
    ** column is not indexed, and that it is not part of a foreign key. 
    ** It is against the rules to open a column to which either of these
    ** descriptions applies for writing.  */
    if( flags ){
      const char *zFault = 0;
      Index *pIdx;
#ifndef SQLITE_OMIT_FOREIGN_KEY
      if( db->flags&SQLITE_ForeignKeys ){
        /* Check that the column is not part of an FK child key definition. It
        ** is not necessary to check if it is part of a parent key, as parent
        ** key columns must be indexed. The check below will pick up this 
        ** case.  */
        FKey *pFKey;
        for(pFKey=pTab->pFKey; pFKey; pFKey=pFKey->pNextFrom){
          int j;
          for(j=0; j<pFKey->nCol; j++){
            if( pFKey->aCol[j].iFrom==iCol ){
              zFault = "foreign key";
            }
          }
        }
      }
#endif
      for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        int j;
        for(j=0; j<pIdx->nColumn; j++){
          if( pIdx->aiColumn[j]==iCol ){
            zFault = "indexed";
          }
        }
      }
      if( zFault ){
        sqlite3DbFree(db, zErr);
        zErr = sqlite3MPrintf(db, "cannot open %s column for writing", zFault);
        rc = SQLITE_ERROR;
        sqlite3BtreeLeaveAll(db);
        goto blob_open_out;
      }
    }

    pBlob->pStmt = (sqlite3_stmt *)sqlite3VdbeCreate(db);
    assert( pBlob->pStmt || db->mallocFailed );
    if( pBlob->pStmt ){
      Vdbe *v = (Vdbe *)pBlob->pStmt;
      int iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

      sqlite3VdbeAddOpList(v, sizeof(openBlob)/sizeof(VdbeOpList), openBlob);


      /* Configure the OP_Transaction */
      sqlite3VdbeChangeP1(v, 0, iDb);
      sqlite3VdbeChangeP2(v, 0, flags);

      /* Configure the OP_VerifyCookie */
      sqlite3VdbeChangeP1(v, 1, iDb);
      sqlite3VdbeChangeP2(v, 1, pTab->pSchema->schema_cookie);
      sqlite3VdbeChangeP3(v, 1, pTab->pSchema->iGeneration);

      /* Make sure a mutex is held on the table to be accessed */
      sqlite3VdbeUsesBtree(v, iDb); 

      /* Configure the OP_TableLock instruction */
#ifdef SQLITE_OMIT_SHARED_CACHE
      sqlite3VdbeChangeToNoop(v, 2, 1);
#else
      sqlite3VdbeChangeP1(v, 2, iDb);
      sqlite3VdbeChangeP2(v, 2, pTab->tnum);
      sqlite3VdbeChangeP3(v, 2, flags);
      sqlite3VdbeChangeP4(v, 2, pTab->zName, P4_TRANSIENT);
#endif

      /* Remove either the OP_OpenWrite or OpenRead. Set the P2 
      ** parameter of the other to pTab->tnum.  */
      sqlite3VdbeChangeToNoop(v, 4 - flags, 1);
      sqlite3VdbeChangeP2(v, 3 + flags, pTab->tnum);
      sqlite3VdbeChangeP3(v, 3 + flags, iDb);

      /* Configure the number of columns. Configure the cursor to
      ** think that the table has one more column than it really
      ** does. An OP_Column to retrieve this imaginary column will
      ** always return an SQL NULL. This is useful because it means
      ** we can invoke OP_Column to fill in the vdbe cursors type 
      ** and offset cache without causing any IO.
      */
      sqlite3VdbeChangeP4(v, 3+flags, SQLITE_INT_TO_PTR(pTab->nCol+1),P4_INT32);
      sqlite3VdbeChangeP2(v, 7, pTab->nCol);
      if( !db->mallocFailed ){
        pParse->nVar = 1;
        pParse->nMem = 1;
        pParse->nTab = 1;
        sqlite3VdbeMakeReady(v, pParse);
      }
    }
   
    pBlob->flags = flags;
    pBlob->iCol = iCol;
    pBlob->db = db;
    sqlite3BtreeLeaveAll(db);
    if( db->mallocFailed ){
      goto blob_open_out;
    }
    sqlite3_bind_int64(pBlob->pStmt, 1, iRow);
    rc = blobSeekToRow(pBlob, iRow, &zErr);
  } while( (++nAttempt)<5 && rc==SQLITE_SCHEMA );

blob_open_out:
  if( rc==SQLITE_OK && db->mallocFailed==0 ){
    *ppBlob = (sqlite3_blob *)pBlob;
  }else{
    if( pBlob && pBlob->pStmt ) sqlite3VdbeFinalize((Vdbe *)pBlob->pStmt);
    sqlite3DbFree(db, pBlob);
  }
  sqlite3Error(db, rc, (zErr ? "%s" : 0), zErr);
  sqlite3DbFree(db, zErr);
  sqlite3StackFree(db, pParse);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

/*
** Close a blob handle that was previously created using
** sqlite3_blob_open().
*/
SQLITE_API int sqlite3_blob_close(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  int rc;
  sqlite3 *db;

  if( p ){
    db = p->db;
    sqlite3_mutex_enter(db->mutex);
    rc = sqlite3_finalize(p->pStmt);
    sqlite3DbFree(db, p);
    sqlite3_mutex_leave(db->mutex);
  }else{
    rc = SQLITE_OK;
  }
  return rc;
}

/*
** Perform a read or write operation on a blob
*/
static int blobReadWrite(
  sqlite3_blob *pBlob, 
  void *z, 
  int n, 
  int iOffset, 
  int (*xCall)(BtCursor*, u32, u32, void*)
){
  int rc;
  Incrblob *p = (Incrblob *)pBlob;
  Vdbe *v;
  sqlite3 *db;

  if( p==0 ) return SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);
  v = (Vdbe*)p->pStmt;

  if( n<0 || iOffset<0 || (iOffset+n)>p->nByte ){
    /* Request is out of range. Return a transient error. */
    rc = SQLITE_ERROR;
    sqlite3Error(db, SQLITE_ERROR, 0);
  }else if( v==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }else{
    /* Call either BtreeData() or BtreePutData(). If SQLITE_ABORT is
    ** returned, clean-up the statement handle.
    */
    assert( db == v->db );
    sqlite3BtreeEnterCursor(p->pCsr);
    rc = xCall(p->pCsr, iOffset+p->iOffset, n, z);
    sqlite3BtreeLeaveCursor(p->pCsr);
    if( rc==SQLITE_ABORT ){
      sqlite3VdbeFinalize(v);
      p->pStmt = 0;
    }else{
      db->errCode = rc;
      v->rc = rc;
    }
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

/*
** Read data from a blob handle.
*/
SQLITE_API int sqlite3_blob_read(sqlite3_blob *pBlob, void *z, int n, int iOffset){
  return blobReadWrite(pBlob, z, n, iOffset, sqlite3BtreeData);
}

/*
** Write data to a blob handle.
*/
SQLITE_API int sqlite3_blob_write(sqlite3_blob *pBlob, const void *z, int n, int iOffset){
  return blobReadWrite(pBlob, (void *)z, n, iOffset, sqlite3BtreePutData);
}

/*
** Query a blob handle for the size of the data.
**
** The Incrblob.nByte field is fixed for the lifetime of the Incrblob
** so no mutex is required for access.
*/
SQLITE_API int sqlite3_blob_bytes(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  return (p && p->pStmt) ? p->nByte : 0;
}

/*
** Move an existing blob handle to point to a different row of the same
** database table.
**
** If an error occurs, or if the specified row does not exist or does not
** contain a blob or text value, then an error code is returned and the
** database handle error code and message set. If this happens, then all 
** subsequent calls to sqlite3_blob_xxx() functions (except blob_close()) 
** immediately return SQLITE_ABORT.
*/
SQLITE_API int sqlite3_blob_reopen(sqlite3_blob *pBlob, sqlite3_int64 iRow){
  int rc;
  Incrblob *p = (Incrblob *)pBlob;
  sqlite3 *db;

  if( p==0 ) return SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);

  if( p->pStmt==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }else{
    char *zErr;
    rc = blobSeekToRow(p, iRow, &zErr);
    if( rc!=SQLITE_OK ){
      sqlite3Error(db, rc, (zErr ? "%s" : 0), zErr);
      sqlite3DbFree(db, zErr);
    }
    assert( rc!=SQLITE_SCHEMA );
  }

  rc = sqlite3ApiExit(db, rc);
  assert( rc==SQLITE_OK || p->pStmt==0 );
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

#endif /* #ifndef SQLITE_OMIT_INCRBLOB */

/************** End of vdbeblob.c ********************************************/
/************** Begin file journal.c *****************************************/
/*
** 2007 August 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements a special kind of sqlite3_file object used
** by SQLite to create journal files if the atomic-write optimization
** is enabled.
**
** The distinctive characteristic of this sqlite3_file is that the
** actual on disk file is created lazily. When the file is created,
** the caller specifies a buffer size for an in-memory buffer to
** be used to service read() and write() requests. The actual file
** on disk is not created or populated until either:
**
**   1) The in-memory representation grows too large for the allocated 
**      buffer, or
**   2) The sqlite3JournalCreate() function is called.
*/
#ifdef SQLITE_ENABLE_ATOMIC_WRITE


/*
** A JournalFile object is a subclass of sqlite3_file used by
** as an open file handle for journal files.
*/
struct JournalFile {
  sqlite3_io_methods *pMethod;    /* I/O methods on journal files */
  int nBuf;                       /* Size of zBuf[] in bytes */
  char *zBuf;                     /* Space to buffer journal writes */
  int iSize;                      /* Amount of zBuf[] currently used */
  int flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  sqlite3_file *pReal;            /* The "real" underlying file descriptor */
  const char *zJournal;           /* Name of the journal file */
};
typedef struct JournalFile JournalFile;

/*
** If it does not already exists, create and populate the on-disk file 
** for JournalFile p.
*/
static int createFile(JournalFile *p){
  int rc = SQLITE_OK;
  if( !p->pReal ){
    sqlite3_file *pReal = (sqlite3_file *)&p[1];
    rc = sqlite3OsOpen(p->pVfs, p->zJournal, pReal, p->flags, 0);
    if( rc==SQLITE_OK ){
      p->pReal = pReal;
      if( p->iSize>0 ){
        assert(p->iSize<=p->nBuf);
        rc = sqlite3OsWrite(p->pReal, p->zBuf, p->iSize, 0);
      }
    }
  }
  return rc;
}

/*
** Close the file.
*/
static int jrnlClose(sqlite3_file *pJfd){
  JournalFile *p = (JournalFile *)pJfd;
  if( p->pReal ){
    sqlite3OsClose(p->pReal);
  }
  sqlite3_free(p->zBuf);
  return SQLITE_OK;
}

/*
** Read data from the file.
*/
static int jrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  void *zBuf,            /* Put the results here */
  int iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  int rc = SQLITE_OK;
  JournalFile *p = (JournalFile *)pJfd;
  if( p->pReal ){
    rc = sqlite3OsRead(p->pReal, zBuf, iAmt, iOfst);
  }else if( (iAmt+iOfst)>p->iSize ){
    rc = SQLITE_IOERR_SHORT_READ;
  }else{
    memcpy(zBuf, &p->zBuf[iOfst], iAmt);
  }
  return rc;
}

/*
** Write data to the file.
*/
static int jrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  const void *zBuf,      /* Take data to be written from here */
  int iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  int rc = SQLITE_OK;
  JournalFile *p = (JournalFile *)pJfd;
  if( !p->pReal && (iOfst+iAmt)>p->nBuf ){
    rc = createFile(p);
  }
  if( rc==SQLITE_OK ){
    if( p->pReal ){
      rc = sqlite3OsWrite(p->pReal, zBuf, iAmt, iOfst);
    }else{
      memcpy(&p->zBuf[iOfst], zBuf, iAmt);
      if( p->iSize<(iOfst+iAmt) ){
        p->iSize = (iOfst+iAmt);
      }
    }
  }
  return rc;
}

/*
** Truncate the file.
*/
static int jrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size){
  int rc = SQLITE_OK;
  JournalFile *p = (JournalFile *)pJfd;
  if( p->pReal ){
    rc = sqlite3OsTruncate(p->pReal, size);
  }else if( size<p->iSize ){
    p->iSize = size;
  }
  return rc;
}

/*
** Sync the file.
*/
static int jrnlSync(sqlite3_file *pJfd, int flags){
  int rc;
  JournalFile *p = (JournalFile *)pJfd;
  if( p->pReal ){
    rc = sqlite3OsSync(p->pReal, flags);
  }else{
    rc = SQLITE_OK;
  }
  return rc;
}

/*
** Query the size of the file in bytes.
*/
static int jrnlFileSize(sqlite3_file *pJfd, sqlite_int64 *pSize){
  int rc = SQLITE_OK;
  JournalFile *p = (JournalFile *)pJfd;
  if( p->pReal ){
    rc = sqlite3OsFileSize(p->pReal, pSize);
  }else{
    *pSize = (sqlite_int64) p->iSize;
  }
  return rc;
}

/*
** Table of methods for JournalFile sqlite3_file object.
*/
static struct sqlite3_io_methods JournalFileMethods = {
  1,             /* iVersion */
  jrnlClose,     /* xClose */
  jrnlRead,      /* xRead */
  jrnlWrite,     /* xWrite */
  jrnlTruncate,  /* xTruncate */
  jrnlSync,      /* xSync */
  jrnlFileSize,  /* xFileSize */
  0,             /* xLock */
  0,             /* xUnlock */
  0,             /* xCheckReservedLock */
  0,             /* xFileControl */
  0,             /* xSectorSize */
  0,             /* xDeviceCharacteristics */
  0,             /* xShmMap */
  0,             /* xShmLock */
  0,             /* xShmBarrier */
  0              /* xShmUnmap */
};

/* 
** Open a journal file.
*/
SQLITE_PRIVATE int sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  const char *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  int flags,                 /* Opening flags */
  int nBuf                   /* Bytes buffered before opening the file */
){
  JournalFile *p = (JournalFile *)pJfd;
  memset(p, 0, sqlite3JournalSize(pVfs));
  if( nBuf>0 ){
    p->zBuf = sqlite3MallocZero(nBuf);
    if( !p->zBuf ){
      return SQLITE_NOMEM;
    }
  }else{
    return sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }
  p->pMethod = &JournalFileMethods;
  p->nBuf = nBuf;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  return SQLITE_OK;
}

/*
** If the argument p points to a JournalFile structure, and the underlying
** file has not yet been created, create it now.
*/
SQLITE_PRIVATE int sqlite3JournalCreate(sqlite3_file *p){
  if( p->pMethods!=&JournalFileMethods ){
    return SQLITE_OK;
  }
  return createFile((JournalFile *)p);
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
SQLITE_PRIVATE int sqlite3JournalSize(sqlite3_vfs *pVfs){
  return (pVfs->szOsFile+sizeof(JournalFile));
}
#endif

/************** End of journal.c *********************************************/
/************** Begin file memjournal.c **************************************/
/*
** 2008 October 7
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains code use to implement an in-memory rollback journal.
** The in-memory rollback journal is used to journal transactions for
** ":memory:" databases and when the journal_mode=MEMORY pragma is used.
*/

/* Forward references to internal structures */
typedef struct MemJournal MemJournal;
typedef struct FilePoint FilePoint;
typedef struct FileChunk FileChunk;

/* Space to hold the rollback journal is allocated in increments of
** this many bytes.
**
** The size chosen is a little less than a power of two.  That way,
** the FileChunk object will have a size that almost exactly fills
** a power-of-two allocation.  This mimimizes wasted space in power-of-two
** memory allocators.
*/
#define JOURNAL_CHUNKSIZE ((int)(1024-sizeof(FileChunk*)))

/* Macro to find the minimum of two numeric values.
*/
#ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
#endif

/*
** The rollback journal is composed of a linked list of these structures.
*/
struct FileChunk {
  FileChunk *pNext;               /* Next chunk in the journal */
  u8 zChunk[JOURNAL_CHUNKSIZE];   /* Content of this chunk */
};

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
struct FilePoint {
  sqlite3_int64 iOffset;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This subclass is a subclass of sqlite3_file.  Each open memory-journal
** is an instance of this class.
*/
struct MemJournal {
  sqlite3_io_methods *pMethod;    /* Parent class. MUST BE FIRST */
  FileChunk *pFirst;              /* Head of in-memory chunk-list */
  FilePoint endpoint;             /* Pointer to the end of the file */
  FilePoint readpoint;            /* Pointer to the end of the last xRead() */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
static int memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  void *zBuf,            /* Put the results here */
  int iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *zOut = zBuf;
  int nRead = iAmt;
  int iChunkOffset;
  FileChunk *pChunk;

  /* SQLite never tries to read past the end of a rollback journal file */
  assert( iOfst+iAmt<=p->endpoint.iOffset );

  if( p->readpoint.iOffset!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    for(pChunk=p->pFirst; 
        ALWAYS(pChunk) && (iOff+JOURNAL_CHUNKSIZE)<=iOfst;
        pChunk=pChunk->pNext
    ){
      iOff += JOURNAL_CHUNKSIZE;
    }
  }else{
    pChunk = p->readpoint.pChunk;
  }

  iChunkOffset = (int)(iOfst%JOURNAL_CHUNKSIZE);
  do {
    int iSpace = JOURNAL_CHUNKSIZE - iChunkOffset;
    int nCopy = MIN(nRead, (JOURNAL_CHUNKSIZE - iChunkOffset));
    memcpy(zOut, &pChunk->zChunk[iChunkOffset], nCopy);
    zOut += nCopy;
    nRead -= iSpace;
    iChunkOffset = 0;
  } while( nRead>=0 && (pChunk=pChunk->pNext)!=0 && nRead>0 );
  p->readpoint.iOffset = iOfst+iAmt;
  p->readpoint.pChunk = pChunk;

  return SQLITE_OK;
}

/*
** Write data to the file.
*/
static int memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  const void *zBuf,      /* Take data to be written from here */
  int iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  int nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* An in-memory journal file should only ever be appended to. Random
  ** access writes are not required by sqlite.
  */
  assert( iOfst==p->endpoint.iOffset );
  UNUSED_PARAMETER(iOfst);

  while( nWrite>0 ){
    FileChunk *pChunk = p->endpoint.pChunk;
    int iChunkOffset = (int)(p->endpoint.iOffset%JOURNAL_CHUNKSIZE);
    int iSpace = MIN(nWrite, JOURNAL_CHUNKSIZE - iChunkOffset);

    if( iChunkOffset==0 ){
      /* New chunk is required to extend the file. */
      FileChunk *pNew = sqlite3_malloc(sizeof(FileChunk));
      if( !pNew ){
        return SQLITE_IOERR_NOMEM;
      }
      pNew->pNext = 0;
      if( pChunk ){
        assert( p->pFirst );
        pChunk->pNext = pNew;
      }else{
        assert( !p->pFirst );
        p->pFirst = pNew;
      }
      p->endpoint.pChunk = pNew;
    }

    memcpy(&p->endpoint.pChunk->zChunk[iChunkOffset], zWrite, iSpace);
    zWrite += iSpace;
    nWrite -= iSpace;
    p->endpoint.iOffset += iSpace;
  }

  return SQLITE_OK;
}

/*
** Truncate the file.
*/
static int memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size){
  MemJournal *p = (MemJournal *)pJfd;
  FileChunk *pChunk;
  assert(size==0);
  UNUSED_PARAMETER(size);
  pChunk = p->pFirst;
  while( pChunk ){
    FileChunk *pTmp = pChunk;
    pChunk = pChunk->pNext;
    sqlite3_free(pTmp);
  }
  sqlite3MemJournalOpen(pJfd);
  return SQLITE_OK;
}

/*
** Close the file.
*/
static int memjrnlClose(sqlite3_file *pJfd){
  memjrnlTruncate(pJfd, 0);
  return SQLITE_OK;
}


/*
** Sync the file.
**
** Syncing an in-memory journal is a no-op.  And, in fact, this routine
** is never called in a working implementation.  This implementation
** exists purely as a contingency, in case some malfunction in some other
** part of SQLite causes Sync to be called by mistake.
*/
static int memjrnlSync(sqlite3_file *NotUsed, int NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  return SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
static int memjrnlFileSize(sqlite3_file *pJfd, sqlite_int64 *pSize){
  MemJournal *p = (MemJournal *)pJfd;
  *pSize = (sqlite_int64) p->endpoint.iOffset;
  return SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
static const struct sqlite3_io_methods MemJournalMethods = {
  1,                /* iVersion */
  memjrnlClose,     /* xClose */
  memjrnlRead,      /* xRead */
  memjrnlWrite,     /* xWrite */
  memjrnlTruncate,  /* xTruncate */
  memjrnlSync,      /* xSync */
  memjrnlFileSize,  /* xFileSize */
  0,                /* xLock */
  0,                /* xUnlock */
  0,                /* xCheckReservedLock */
  0,                /* xFileControl */
  0,                /* xSectorSize */
  0,                /* xDeviceCharacteristics */
  0,                /* xShmMap */
  0,                /* xShmLock */
  0,                /* xShmBarrier */
  0                 /* xShmUnlock */
};

/* 
** Open a journal file.
*/
SQLITE_PRIVATE void sqlite3MemJournalOpen(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  assert( EIGHT_BYTE_ALIGNMENT(p) );
  memset(p, 0, sqlite3MemJournalSize());
  p->pMethod = (sqlite3_io_methods*)&MemJournalMethods;
}

/*
** Return true if the file-handle passed as an argument is 
** an in-memory journal 
*/
SQLITE_PRIVATE int sqlite3IsMemJournal(sqlite3_file *pJfd){
  return pJfd->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a MemJournal file descriptor.
*/
SQLITE_PRIVATE int sqlite3MemJournalSize(void){
  return sizeof(MemJournal);
}

/************** End of memjournal.c ******************************************/
/************** Begin file walker.c ******************************************/
/*
** 2008 August 16
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains routines used for walking the parser tree for
** an SQL statement.
*/


/*
** Walk an expression tree.  Invoke the callback once for each node
** of the expression, while decending.  (In other words, the callback
** is invoked before visiting children.)
**
** The return value from the callback should be one of the WRC_*
** constants to specify how to proceed with the walk.
**
**    WRC_Continue      Continue descending down the tree.
**
**    WRC_Prune         Do not descend into child nodes.  But allow
**                      the walk to continue with sibling nodes.
**
**    WRC_Abort         Do no more callbacks.  Unwind the stack and
**                      return the top-level walk call.
**
** The return value from this routine is WRC_Abort to abandon the tree walk
** and WRC_Continue to continue.
*/
SQLITE_PRIVATE int sqlite3WalkExpr(Walker *pWalker, Expr *pExpr){
  int rc;
  if( pExpr==0 ) return WRC_Continue;
  testcase( ExprHasProperty(pExpr, EP_TokenOnly) );
  testcase( ExprHasProperty(pExpr, EP_Reduced) );
  rc = pWalker->xExprCallback(pWalker, pExpr);
  if( rc==WRC_Continue
              && !ExprHasAnyProperty(pExpr,EP_TokenOnly) ){
    if( sqlite3WalkExpr(pWalker, pExpr->pLeft) ) return WRC_Abort;
    if( sqlite3WalkExpr(pWalker, pExpr->pRight) ) return WRC_Abort;
    if( ExprHasProperty(pExpr, EP_xIsSelect) ){
      if( sqlite3WalkSelect(pWalker, pExpr->x.pSelect) ) return WRC_Abort;
    }else{
      if( sqlite3WalkExprList(pWalker, pExpr->x.pList) ) return WRC_Abort;
    }
  }
  return rc & WRC_Abort;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
SQLITE_PRIVATE int sqlite3WalkExprList(Walker *pWalker, ExprList *p){
  int i;
  struct ExprList_item *pItem;
  if( p ){
    for(i=p->nExpr, pItem=p->a; i>0; i--, pItem++){
      if( sqlite3WalkExpr(pWalker, pItem->pExpr) ) return WRC_Abort;
    }
  }
  return WRC_Continue;
}

/*
** Walk all expressions associated with SELECT statement p.  Do
** not invoke the SELECT callback on p, but do (of course) invoke
** any expr callbacks and SELECT callbacks that come from subqueries.
** Return WRC_Abort or WRC_Continue.
*/
SQLITE_PRIVATE int sqlite3WalkSelectExpr(Walker *pWalker, Select *p){
  if( sqlite3WalkExprList(pWalker, p->pEList) ) return WRC_Abort;
  if( sqlite3WalkExpr(pWalker, p->pWhere) ) return WRC_Abort;
  if( sqlite3WalkExprList(pWalker, p->pGroupBy) ) return WRC_Abort;
  if( sqlite3WalkExpr(pWalker, p->pHaving) ) return WRC_Abort;
  if( sqlite3WalkExprList(pWalker, p->pOrderBy) ) return WRC_Abort;
  if( sqlite3WalkExpr(pWalker, p->pLimit) ) return WRC_Abort;
  if( sqlite3WalkExpr(pWalker, p->pOffset) ) return WRC_Abort;
  return WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
SQLITE_PRIVATE int sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  int i;
  struct SrcList_item *pItem;

  pSrc = p->pSrc;
  if( ALWAYS(pSrc) ){
    for(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      if( sqlite3WalkSelect(pWalker, pItem->pSelect) ){
        return WRC_Abort;
      }
    }
  }
  return WRC_Continue;
} 

/*
** Call sqlite3WalkExpr() for every expression in Select statement p.
** Invoke sqlite3WalkSelect() for subqueries in the FROM clause and
** on the compound select chain, p->pPrior.
**
** Return WRC_Continue under normal conditions.  Return WRC_Abort if
** there is an abort request.
**
** If the Walker does not have an xSelectCallback() then this routine
** is a no-op returning WRC_Continue.
*/
SQLITE_PRIVATE int sqlite3WalkSelect(Walker *pWalker, Select *p){
  int rc;
  if( p==0 || pWalker->xSelectCallback==0 ) return WRC_Continue;
  rc = WRC_Continue;
  while( p  ){
    rc = pWalker->xSelectCallback(pWalker, p);
    if( rc ) break;
    if( sqlite3WalkSelectExpr(pWalker, p) ) return WRC_Abort;
    if( sqlite3WalkSelectFrom(pWalker, p) ) return WRC_Abort;
    p = p->pPrior;
  }
  return rc & WRC_Abort;
}

/************** End of walker.c **********************************************/
/************** Begin file resolve.c *****************************************/
/*
** 2008 August 18
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains routines used for walking the parser tree and
** resolve all identifiers by associating them with a particular
** table and column.
*/

/*
** Turn the pExpr expression into an alias for the iCol-th column of the
** result set in pEList.
**
** If the result set column is a simple column reference, then this routine
** makes an exact copy.  But for any other kind of expression, this
** routine make a copy of the result set column as the argument to the
** TK_AS operator.  The TK_AS operator causes the expression to be
** evaluated just once and then reused for each alias.
**
** The reason for suppressing the TK_AS term when the expression is a simple
** column reference is so that the column reference will be recognized as
** usable by indices within the WHERE clause processing logic. 
**
** Hack:  The TK_AS operator is inhibited if zType[0]=='G'.  This means
** that in a GROUP BY clause, the expression is evaluated twice.  Hence:
**
**     SELECT random()%5 AS x, count(*) FROM tab GROUP BY x
**
** Is equivalent to:
**
**     SELECT random()%5 AS x, count(*) FROM tab GROUP BY random()%5
**
** The result of random()%5 in the GROUP BY clause is probably different
** from the result in the result-set.  We might fix this someday.  Or
** then again, we might not...
*/
static void resolveAlias(
  Parse *pParse,         /* Parsing context */
  ExprList *pEList,      /* A result set */
  int iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  Expr *pExpr,           /* Transform this into an alias to the result set */
  const char *zType      /* "GROUP" or "ORDER" or "" */
){
  Expr *pOrig;           /* The iCol-th column of the result set */
  Expr *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  assert( iCol>=0 && iCol<pEList->nExpr );
  pOrig = pEList->a[iCol].pExpr;
  assert( pOrig!=0 );
  assert( pOrig->flags & EP_Resolved );
  db = pParse->db;
  if( pOrig->op!=TK_COLUMN && zType[0]!='G' ){
    pDup = sqlite3ExprDup(db, pOrig, 0);
    pDup = sqlite3PExpr(pParse, TK_AS, pDup, 0, 0);
    if( pDup==0 ) return;
    if( pEList->a[iCol].iAlias==0 ){
      pEList->a[iCol].iAlias = (u16)(++pParse->nAlias);
    }
    pDup->iTable = pEList->a[iCol].iAlias;
  }else if( ExprHasProperty(pOrig, EP_IntValue) || pOrig->u.zToken==0 ){
    pDup = sqlite3ExprDup(db, pOrig, 0);
    if( pDup==0 ) return;
  }else{
    char *zToken = pOrig->u.zToken;
    assert( zToken!=0 );
    pOrig->u.zToken = 0;
    pDup = sqlite3ExprDup(db, pOrig, 0);
    pOrig->u.zToken = zToken;
    if( pDup==0 ) return;
    assert( (pDup->flags & (EP_Reduced|EP_TokenOnly))==0 );
    pDup->flags2 |= EP2_MallocedToken;
    pDup->u.zToken = sqlite3DbStrDup(db, zToken);
  }
  if( pExpr->flags & EP_ExpCollate ){
    pDup->pColl = pExpr->pColl;
    pDup->flags |= EP_ExpCollate;
  }

  /* Before calling sqlite3ExprDelete(), set the EP_Static flag. This 
  ** prevents ExprDelete() from deleting the Expr structure itself,
  ** allowing it to be repopulated by the memcpy() on the following line.
  */
  ExprSetProperty(pExpr, EP_Static);
  sqlite3ExprDelete(db, pExpr);
  memcpy(pExpr, pDup, sizeof(*pExpr));
  sqlite3DbFree(db, pDup);
}

/*
** Given the name of a column of the form X.Y.Z or Y.Z or just Z, look up
** that name in the set of source tables in pSrcList and make the pExpr 
** expression node refer back to that source column.  The following changes
** are made to pExpr:
**
**    pExpr->iDb           Set the index in db->aDb[] of the database X
**                         (even if X is implied).
**    pExpr->iTable        Set to the cursor number for the table obtained
**                         from pSrcList.
**    pExpr->pTab          Points to the Table structure of X.Y (even if
**                         X and/or Y are implied.)
**    pExpr->iColumn       Set to the column number within the table.
**    pExpr->op            Set to TK_COLUMN.
**    pExpr->pLeft         Any expression this points to is deleted
**    pExpr->pRight        Any expression this points to is deleted.
**
** The zDb variable is the name of the database (the "X").  This value may be
** NULL meaning that name is of the form Y.Z or Z.  Any available database
** can be used.  The zTable variable is the name of the table (the "Y").  This
** value can be NULL if zDb is also NULL.  If zTable is NULL it
** means that the form of the name is Z and that columns from any table
** can be used.
**
** If the name cannot be resolved unambiguously, leave an error message
** in pParse and return WRC_Abort.  Return WRC_Prune on success.
*/
static int lookupName(
  Parse *pParse,       /* The parsing context */
  const char *zDb,     /* Name of the database containing table, or NULL */
  const char *zTab,    /* Name of table containing column, or NULL */
  const char *zCol,    /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  Expr *pExpr          /* Make this EXPR node point to the selected column */
){
  int i, j;            /* Loop counters */
  int cnt = 0;                      /* Number of matching column names */
  int cntTab = 0;                   /* Number of matching table names */
  sqlite3 *db = pParse->db;         /* The database connection */
  struct SrcList_item *pItem;       /* Use for looping over pSrcList items */
  struct SrcList_item *pMatch = 0;  /* The matching pSrcList item */
  NameContext *pTopNC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  int isTrigger = 0;

  assert( pNC );     /* the name context cannot be NULL. */
  assert( zCol );    /* The Z in X.Y.Z cannot be NULL */
  assert( ~ExprHasAnyProperty(pExpr, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  pExpr->iTable = -1;
  pExpr->pTab = 0;
  ExprSetIrreducible(pExpr);

  /* Start at the inner-most context and move outward until a match is found */
  while( pNC && cnt==0 ){
    ExprList *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    if( pSrcList ){
      for(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        Table *pTab;
        int iDb;
        Column *pCol;
  
        pTab = pItem->pTab;
        assert( pTab!=0 && pTab->zName!=0 );
        iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        assert( pTab->nCol>0 );
        if( zTab ){
          if( pItem->zAlias ){
            char *zTabName = pItem->zAlias;
            if( sqlite3StrICmp(zTabName, zTab)!=0 ) continue;
          }else{
            char *zTabName = pTab->zName;
            if( NEVER(zTabName==0) || sqlite3StrICmp(zTabName, zTab)!=0 ){
              continue;
            }
            if( zDb!=0 && sqlite3StrICmp(db->aDb[iDb].zName, zDb)!=0 ){
              continue;
            }
          }
        }
        if( 0==(cntTab++) ){
          pExpr->iTable = pItem->iCursor;
          pExpr->pTab = pTab;
          pSchema = pTab->pSchema;
          pMatch = pItem;
        }
        for(j=0, pCol=pTab->aCol; j<pTab->nCol; j++, pCol++){
          if( sqlite3StrICmp(pCol->zName, zCol)==0 ){
            IdList *pUsing;
            cnt++;
            pExpr->iTable = pItem->iCursor;
            pExpr->pTab = pTab;
            pMatch = pItem;
            pSchema = pTab->pSchema;
            /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
            pExpr->iColumn = j==pTab->iPKey ? -1 : (i16)j;
            if( i<pSrcList->nSrc-1 ){
              if( pItem[1].jointype & JT_NATURAL ){
                /* If this match occurred in the left table of a natural join,
                ** then skip the right table to avoid a duplicate match */
                pItem++;
                i++;
              }else if( (pUsing = pItem[1].pUsing)!=0 ){
                /* If this match occurs on a column that is in the USING clause
                ** of a join, skip the search of the right table of the join
                ** to avoid a duplicate match there. */
                int k;
                for(k=0; k<pUsing->nId; k++){
                  if( sqlite3StrICmp(pUsing->a[k].zName, zCol)==0 ){
                    pItem++;
                    i++;
                    break;
                  }
                }
              }
            }
            break;
          }
        }
      }
    }

#ifndef SQLITE_OMIT_TRIGGER
    /* If we have not already resolved the name, then maybe 
    ** it is a new.* or old.* trigger argument reference
    */
    if( zDb==0 && zTab!=0 && cnt==0 && pParse->pTriggerTab!=0 ){
      int op = pParse->eTriggerOp;
      Table *pTab = 0;
      assert( op==TK_DELETE || op==TK_UPDATE || op==TK_INSERT );
      if( op!=TK_DELETE && sqlite3StrICmp("new",zTab) == 0 ){
        pExpr->iTable = 1;
        pTab = pParse->pTriggerTab;
      }else if( op!=TK_INSERT && sqlite3StrICmp("old",zTab)==0 ){
        pExpr->iTable = 0;
        pTab = pParse->pTriggerTab;
      }

      if( pTab ){ 
        int iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        for(iCol=0; iCol<pTab->nCol; iCol++){
          Column *pCol = &pTab->aCol[iCol];
          if( sqlite3StrICmp(pCol->zName, zCol)==0 ){
            if( iCol==pTab->iPKey ){
              iCol = -1;
            }
            break;
          }
        }
        if( iCol>=pTab->nCol && sqlite3IsRowid(zCol) ){
          iCol = -1;        /* IMP: R-44911-55124 */
        }
        if( iCol<pTab->nCol ){
          cnt++;
          if( iCol<0 ){
            pExpr->affinity = SQLITE_AFF_INTEGER;
          }else if( pExpr->iTable==0 ){
            testcase( iCol==31 );
            testcase( iCol==32 );
            pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
          }else{
            testcase( iCol==31 );
            testcase( iCol==32 );
            pParse->newmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
          }
          pExpr->iColumn = (i16)iCol;
          pExpr->pTab = pTab;
          isTrigger = 1;
        }
      }
    }
#endif /* !defined(SQLITE_OMIT_TRIGGER) */

    /*
    ** Perhaps the name is a reference to the ROWID
    */
    if( cnt==0 && cntTab==1 && sqlite3IsRowid(zCol) ){
      cnt = 1;
      pExpr->iColumn = -1;     /* IMP: R-44911-55124 */
      pExpr->affinity = SQLITE_AFF_INTEGER;
    }

    /*
    ** If the input is of the form Z (not Y.Z or X.Y.Z) then the name Z
    ** might refer to an result-set alias.  This happens, for example, when
    ** we are resolving names in the WHERE clause of the following command:
    **
    **     SELECT a+b AS x FROM table WHERE x<10;
    **
    ** In cases like this, replace pExpr with a copy of the expression that
    ** forms the result set entry ("a+b" in the example) and return immediately.
    ** Note that the expression in the result set should have already been
    ** resolved by the time the WHERE clause is resolved.
    */
    if( cnt==0 && (pEList = pNC->pEList)!=0 && zTab==0 ){
      for(j=0; j<pEList->nExpr; j++){
        char *zAs = pEList->a[j].zName;
        if( zAs!=0 && sqlite3StrICmp(zAs, zCol)==0 ){
          Expr *pOrig;
          assert( pExpr->pLeft==0 && pExpr->pRight==0 );
          assert( pExpr->x.pList==0 );
          assert( pExpr->x.pSelect==0 );
          pOrig = pEList->a[j].pExpr;
          if( !pNC->allowAgg && ExprHasProperty(pOrig, EP_Agg) ){
            sqlite3ErrorMsg(pParse, "misuse of aliased aggregate %s", zAs);
            return WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, pExpr, "");
          cnt = 1;
          pMatch = 0;
          assert( zTab==0 && zDb==0 );
          goto lookupname_end;
        }
      } 
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    if( cnt==0 ){
      pNC = pNC->pNext;
    }
  }

  /*
  ** If X and Y are NULL (in other words if only the column name Z is
  ** supplied) and the value of Z is enclosed in double-quotes, then
  ** Z is a string literal if it doesn't match any column names.  In that
  ** case, we need to return right away and not make any changes to
  ** pExpr.
  **
  ** Because no reference was made to outer contexts, the pNC->nRef
  ** fields are not changed in any context.
  */
  if( cnt==0 && zTab==0 && ExprHasProperty(pExpr,EP_DblQuoted) ){
    pExpr->op = TK_STRING;
    pExpr->pTab = 0;
    return WRC_Prune;
  }

  /*
  ** cnt==0 means there was not match.  cnt>1 means there were two or
  ** more matches.  Either way, we have an error.
  */
  if( cnt!=1 ){
    const char *zErr;
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    if( zDb ){
      sqlite3ErrorMsg(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }else if( zTab ){
      sqlite3ErrorMsg(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }else{
      sqlite3ErrorMsg(pParse, "%s: %s", zErr, zCol);
    }
    pParse->checkSchema = 1;
    pTopNC->nErr++;
  }

  /* If a column from a table in pSrcList is referenced, then record
  ** this fact in the pSrcList.a[].colUsed bitmask.  Column 0 causes
  ** bit 0 to be set.  Column 1 sets bit 1.  And so forth.  If the
  ** column number is greater than the number of bits in the bitmask
  ** then set the high-order bit of the bitmask.
  */
  if( pExpr->iColumn>=0 && pMatch!=0 ){
    int n = pExpr->iColumn;
    testcase( n==BMS-1 );
    if( n>=BMS ){
      n = BMS-1;
    }
    assert( pMatch->iCursor==pExpr->iTable );
    pMatch->colUsed |= ((Bitmask)1)<<n;
  }

  /* Clean up and return
  */
  sqlite3ExprDelete(db, pExpr->pLeft);
  pExpr->pLeft = 0;
  sqlite3ExprDelete(db, pExpr->pRight);
  pExpr->pRight = 0;
  pExpr->op = (isTrigger ? TK_TRIGGER : TK_COLUMN);
lookupname_end:
  if( cnt==1 ){
    assert( pNC!=0 );
    sqlite3AuthRead(pParse, pExpr, pSchema, pNC->pSrcList);
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    for(;;){
      assert( pTopNC!=0 );
      pTopNC->nRef++;
      if( pTopNC==pNC ) break;
      pTopNC = pTopNC->pNext;
    }
    return WRC_Prune;
  } else {
    return WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
SQLITE_PRIVATE Expr *sqlite3CreateColumnExpr(sqlite3 *db, SrcList *pSrc, int iSrc, int iCol){
  Expr *p = sqlite3ExprAlloc(db, TK_COLUMN, 0, 0);
  if( p ){
    struct SrcList_item *pItem = &pSrc->a[iSrc];
    p->pTab = pItem->pTab;
    p->iTable = pItem->iCursor;
    if( p->pTab->iPKey==iCol ){
      p->iColumn = -1;
    }else{
      p->iColumn = (ynVar)iCol;
      testcase( iCol==BMS );
      testcase( iCol==BMS-1 );
      pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
    }
    ExprSetProperty(p, EP_Resolved);
  }
  return p;
}

/*
** This routine is callback for sqlite3WalkExpr().
**
** Resolve symbolic names into TK_COLUMN operators for the current
** node in the expression tree.  Return 0 to continue the search down
** the tree or 2 to abort the tree walk.
**
** This routine also does error checking and name resolution for
** function names.  The operator for aggregate functions is changed
** to TK_AGG_FUNCTION.
*/
static int resolveExprStep(Walker *pWalker, Expr *pExpr){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  assert( pNC!=0 );
  pParse = pNC->pParse;
  assert( pParse==pWalker->pParse );

  if( ExprHasAnyProperty(pExpr, EP_Resolved) ) return WRC_Prune;
  ExprSetProperty(pExpr, EP_Resolved);
#ifndef NDEBUG
  if( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    int i;
    for(i=0; i<pNC->pSrcList->nSrc; i++){
      assert( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  switch( pExpr->op ){

#if defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) && !defined(SQLITE_OMIT_SUBQUERY)
    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements.
    */
    case TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      struct SrcList_item *pItem;
      assert( pSrcList && pSrcList->nSrc==1 );
      pItem = pSrcList->a; 
      pExpr->op = TK_COLUMN;
      pExpr->pTab = pItem->pTab;
      pExpr->iTable = pItem->iCursor;
      pExpr->iColumn = -1;
      pExpr->affinity = SQLITE_AFF_INTEGER;
      break;
    }
#endif /* defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) && !defined(SQLITE_OMIT_SUBQUERY) */

    /* A lone identifier is the name of a column.
    */
    case TK_ID: {
      return lookupName(pParse, 0, 0, pExpr->u.zToken, pNC, pExpr);
    }
  
    /* A table name and column name:     ID.ID
    ** Or a database, table and column:  ID.ID.ID
    */
    case TK_DOT: {
      const char *zColumn;
      const char *zTable;
      const char *zDb;
      Expr *pRight;

      /* if( pSrcList==0 ) break; */
      pRight = pExpr->pRight;
      if( pRight->op==TK_ID ){
        zDb = 0;
        zTable = pExpr->pLeft->u.zToken;
        zColumn = pRight->u.zToken;
      }else{
        assert( pRight->op==TK_DOT );
        zDb = pExpr->pLeft->u.zToken;
        zTable = pRight->pLeft->u.zToken;
        zColumn = pRight->pRight->u.zToken;
      }
      return lookupName(pParse, zDb, zTable, zColumn, pNC, pExpr);
    }

    /* Resolve function names
    */
    case TK_CONST_FUNC:
    case TK_FUNCTION: {
      ExprList *pList = pExpr->x.pList;    /* The argument list */
      int n = pList ? pList->nExpr : 0;    /* Number of arguments */
      int no_such_func = 0;       /* True if no such function exists */
      int wrong_num_args = 0;     /* True if wrong number of arguments */
      int is_agg = 0;             /* True if is an aggregate function */
      int auth;                   /* Authorization to use the function */
      int nId;                    /* Number of characters in function name */
      const char *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */

      testcase( pExpr->op==TK_CONST_FUNC );
      assert( !ExprHasProperty(pExpr, EP_xIsSelect) );
      zId = pExpr->u.zToken;
      nId = sqlite3Strlen30(zId);
      pDef = sqlite3FindFunction(pParse->db, zId, nId, n, enc, 0);
      if( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, nId, -1, enc, 0);
        if( pDef==0 ){
          no_such_func = 1;
        }else{
          wrong_num_args = 1;
        }
      }else{
        is_agg = pDef->xFunc==0;
      }
#ifndef SQLITE_OMIT_AUTHORIZATION
      if( pDef ){
        auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0, pDef->zName, 0);
        if( auth!=SQLITE_OK ){
          if( auth==SQLITE_DENY ){
            sqlite3ErrorMsg(pParse, "not authorized to use function: %s",
                                    pDef->zName);
            pNC->nErr++;
          }
          pExpr->op = TK_NULL;
          return WRC_Prune;
        }
      }
#endif
      if( is_agg && !pNC->allowAgg ){
        sqlite3ErrorMsg(pParse, "misuse of aggregate function %.*s()", nId,zId);
        pNC->nErr++;
        is_agg = 0;
      }else if( no_such_func ){
        sqlite3ErrorMsg(pParse, "no such function: %.*s", nId, zId);
        pNC->nErr++;
      }else if( wrong_num_args ){
        sqlite3ErrorMsg(pParse,"wrong number of arguments to function %.*s()",
             nId, zId);
        pNC->nErr++;
      }
      if( is_agg ){
        pExpr->op = TK_AGG_FUNCTION;
        pNC->hasAgg = 1;
      }
      if( is_agg ) pNC->allowAgg = 0;
      sqlite3WalkExprList(pWalker, pList);
      if( is_agg ) pNC->allowAgg = 1;
      /* FIX ME:  Compute pExpr->affinity based on the expected return
      ** type of the function 
      */
      return WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_SELECT:
    case TK_EXISTS:  testcase( pExpr->op==TK_EXISTS );
#endif
    case TK_IN: {
      testcase( pExpr->op==TK_IN );
      if( ExprHasProperty(pExpr, EP_xIsSelect) ){
        int nRef = pNC->nRef;
#ifndef SQLITE_OMIT_CHECK
        if( pNC->isCheck ){
          sqlite3ErrorMsg(pParse,"subqueries prohibited in CHECK constraints");
        }
#endif
        sqlite3WalkSelect(pWalker, pExpr->x.pSelect);
        assert( pNC->nRef>=nRef );
        if( nRef!=pNC->nRef ){
          ExprSetProperty(pExpr, EP_VarSelect);
        }
      }
      break;
    }
#ifndef SQLITE_OMIT_CHECK
    case TK_VARIABLE: {
      if( pNC->isCheck ){
        sqlite3ErrorMsg(pParse,"parameters prohibited in CHECK constraints");
      }
      break;
    }
#endif
  }
  return (pParse->nErr || pParse->db->mallocFailed) ? WRC_Abort : WRC_Continue;
}

/*
** pEList is a list of expressions which are really the result set of the
** a SELECT statement.  pE is a term in an ORDER BY or GROUP BY clause.
** This routine checks to see if pE is a simple identifier which corresponds
** to the AS-name of one of the terms of the expression list.  If it is,
** this routine return an integer between 1 and N where N is the number of
** elements in pEList, corresponding to the matching entry.  If there is
** no match, or if pE is not a simple identifier, then this routine
** return 0.
**
** pEList has been resolved.  pE has not.
*/
static int resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  ExprList *pEList,  /* List of expressions to scan */
  Expr *pE           /* Expression we are trying to match */
){
  int i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  if( pE->op==TK_ID ){
    char *zCol = pE->u.zToken;
    for(i=0; i<pEList->nExpr; i++){
      char *zAs = pEList->a[i].zName;
      if( zAs!=0 && sqlite3StrICmp(zAs, zCol)==0 ){
        return i+1;
      }
    }
  }
  return 0;
}

/*
** pE is a pointer to an expression which is a single term in the
** ORDER BY of a compound SELECT.  The expression has not been
** name resolved.
**
** At the point this routine is called, we already know that the
** ORDER BY term is not an integer index into the result set.  That
** case is handled by the calling routine.
**
** Attempt to match pE against result set columns in the left-most
** SELECT statement.  Return the index i of the matching column,
** as an indication to the caller that it should sort by the i-th column.
** The left-most column is 1.  In other words, the value returned is the
** same integer value that would be used in the SQL statement to indicate
** the column.
**
** If there is no match, return 0.  Return -1 if an error occurs.
*/
static int resolveOrderByTermToExprList(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  Expr *pE           /* The specific ORDER BY term */
){
  int i;             /* Loop counter */
  ExprList *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  int rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  assert( sqlite3ExprIsInteger(pE, &i)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, sizeof(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.pEList = pEList;
  nc.allowAgg = 1;
  nc.nErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3ResolveExprNames(&nc, pE);
  db->suppressErr = savedSuppErr;
  if( rc ) return 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  for(i=0; i<pEList->nExpr; i++){
    if( sqlite3ExprCompare(pEList->a[i].pExpr, pE)<2 ){
      return i+1;
    }
  }

  /* If no match, return 0. */
  return 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
static void resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  const char *zType,     /* "ORDER" or "GROUP" */
  int i,                 /* The index (1-based) of the term out of range */
  int mx                 /* Largest permissible value of i */
){
  sqlite3ErrorMsg(pParse, 
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
}

/*
** Analyze the ORDER BY clause in a compound SELECT statement.   Modify
** each term of the ORDER BY clause is a constant integer between 1
** and N where N is the number of columns in the compound SELECT.
**
** ORDER BY terms that are already an integer between 1 and N are
** unmodified.  ORDER BY terms that are integers outside the range of
** 1 through N generate an error.  ORDER BY terms that are expressions
** are matched against result set expressions of compound SELECT
** beginning with the left-most SELECT and working toward the right.
** At the first match, the ORDER BY expression is transformed into
** the integer column number.
**
** Return the number of errors seen.
*/
static int resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  int i;
  ExprList *pOrderBy;
  ExprList *pEList;
  sqlite3 *db;
  int moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  if( pOrderBy==0 ) return 0;
  db = pParse->db;
#if SQLITE_MAX_COLUMN
  if( pOrderBy->nExpr>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3ErrorMsg(pParse, "too many terms in ORDER BY clause");
    return 1;
  }
#endif
  for(i=0; i<pOrderBy->nExpr; i++){
    pOrderBy->a[i].done = 0;
  }
  pSelect->pNext = 0;
  while( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  while( pSelect && moreToDo ){
    struct ExprList_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    assert( pEList!=0 );
    for(i=0, pItem=pOrderBy->a; i<pOrderBy->nExpr; i++, pItem++){
      int iCol = -1;
      Expr *pE, *pDup;
      if( pItem->done ) continue;
      pE = pItem->pExpr;
      if( sqlite3ExprIsInteger(pE, &iCol) ){
        if( iCol<=0 || iCol>pEList->nExpr ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->nExpr);
          return 1;
        }
      }else{
        iCol = resolveAsName(pParse, pEList, pE);
        if( iCol==0 ){
          pDup = sqlite3ExprDup(db, pE, 0);
          if( !db->mallocFailed ){
            assert(pDup);
            iCol = resolveOrderByTermToExprList(pParse, pSelect, pDup);
          }
          sqlite3ExprDelete(db, pDup);
        }
      }
      if( iCol>0 ){
        CollSeq *pColl = pE->pColl;
        int flags = pE->flags & EP_ExpCollate;
        sqlite3ExprDelete(db, pE);
        pItem->pExpr = pE = sqlite3Expr(db, TK_INTEGER, 0);
        if( pE==0 ) return 1;
        pE->pColl = pColl;
        pE->flags |= EP_IntValue | flags;
        pE->u.iValue = iCol;
        pItem->iCol = (u16)iCol;
        pItem->done = 1;
      }else{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  for(i=0; i<pOrderBy->nExpr; i++){
    if( pOrderBy->a[i].done==0 ){
      sqlite3ErrorMsg(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      return 1;
    }
  }
  return 0;
}

/*
** Check every term in the ORDER BY or GROUP BY clause pOrderBy of
** the SELECT statement pSelect.  If any term is reference to a
** result set expression (as determined by the ExprList.a.iCol field)
** then convert that term into a copy of the corresponding result set
** column.
**
** If any errors are detected, add an error message to pParse and
** return non-zero.  Return zero if no errors are seen.
*/
SQLITE_PRIVATE int sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  ExprList *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  const char *zType     /* "ORDER" or "GROUP" */
){
  int i;
  sqlite3 *db = pParse->db;
  ExprList *pEList;
  struct ExprList_item *pItem;

  if( pOrderBy==0 || pParse->db->mallocFailed ) return 0;
#if SQLITE_MAX_COLUMN
  if( pOrderBy->nExpr>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3ErrorMsg(pParse, "too many terms in %s BY clause", zType);
    return 1;
  }
#endif
  pEList = pSelect->pEList;
  assert( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  for(i=0, pItem=pOrderBy->a; i<pOrderBy->nExpr; i++, pItem++){
    if( pItem->iCol ){
      if( pItem->iCol>pEList->nExpr ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->nExpr);
        return 1;
      }
      resolveAlias(pParse, pEList, pItem->iCol-1, pItem->pExpr, zType);
    }
  }
  return 0;
}

/*
** pOrderBy is an ORDER BY or GROUP BY clause in SELECT statement pSelect.
** The Name context of the SELECT statement is pNC.  zType is either
** "ORDER" or "GROUP" depending on which type of clause pOrderBy is.
**
** This routine resolves each term of the clause into an expression.
** If the order-by term is an integer I between 1 and N (where N is the
** number of columns in the result set of the SELECT) then the expression
** in the resolution is a copy of the I-th result-set expression.  If
** the order-by term is an identify that corresponds to the AS-name of
** a result-set expression, then the term resolves to a copy of the
** result-set expression.  Otherwise, the expression is resolved in
** the usual way - using sqlite3ResolveExprNames().
**
** This routine returns the number of errors.  If errors occur, then
** an appropriate error message might be left in pParse.  (OOM errors
** excepted.)
*/
static int resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  ExprList *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  const char *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  int i;                         /* Loop counter */
  int iCol;                      /* Column number */
  struct ExprList_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  int nResult;                   /* Number of terms in the result set */

  if( pOrderBy==0 ) return 0;
  nResult = pSelect->pEList->nExpr;
  pParse = pNC->pParse;
  for(i=0, pItem=pOrderBy->a; i<pOrderBy->nExpr; i++, pItem++){
    Expr *pE = pItem->pExpr;
    iCol = resolveAsName(pParse, pSelect->pEList, pE);
    if( iCol>0 ){
      /* If an AS-name match is found, mark this ORDER BY column as being
      ** a copy of the iCol-th result-set column.  The subsequent call to
      ** sqlite3ResolveOrderGroupBy() will convert the expression to a
      ** copy of the iCol-th result-set expression. */
      pItem->iCol = (u16)iCol;
      continue;
    }
    if( sqlite3ExprIsInteger(pE, &iCol) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      if( iCol<1 ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult);
        return 1;
      }
      pItem->iCol = (u16)iCol;
      continue;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->iCol = 0;
    if( sqlite3ResolveExprNames(pNC, pE) ){
      return 1;
    }
  }
  return sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendents.
*/
static int resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  int isCompound;         /* True if p is a compound select */
  int nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  ExprList *pEList;       /* Result set expression list */
  int i;                  /* Loop counter */
  ExprList *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
  

  assert( p!=0 );
  if( p->selFlags & SF_Resolved ){
    return WRC_Prune;
  }
  pOuterNC = pWalker->u.pNC;
  pParse = pWalker->pParse;
  db = pParse->db;

  /* Normally sqlite3SelectExpand() will be called first and will have
  ** already expanded this SELECT.  However, if this is a subquery within
  ** an expression, sqlite3ResolveExprNames() will be called without a
  ** prior call to sqlite3SelectExpand().  When that happens, let
  ** sqlite3SelectPrep() do all of the processing for this SELECT.
  ** sqlite3SelectPrep() will invoke both sqlite3SelectExpand() and
  ** this routine in the correct order.
  */
  if( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    return (pParse->nErr || db->mallocFailed) ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  while( p ){
    assert( (p->selFlags & SF_Expanded)!=0 );
    assert( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, sizeof(sNC));
    sNC.pParse = pParse;
    if( sqlite3ResolveExprNames(&sNC, p->pLimit) ||
        sqlite3ResolveExprNames(&sNC, p->pOffset) ){
      return WRC_Abort;
    }
  
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.allowAgg = 1;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
  
    /* Resolve names in the result set. */
    pEList = p->pEList;
    assert( pEList!=0 );
    for(i=0; i<pEList->nExpr; i++){
      Expr *pX = pEList->a[i].pExpr;
      if( sqlite3ResolveExprNames(&sNC, pX) ){
        return WRC_Abort;
      }
    }
  
    /* Recursively resolve names in all subqueries
    */
    for(i=0; i<p->pSrc->nSrc; i++){
      struct SrcList_item *pItem = &p->pSrc->a[i];
      if( pItem->pSelect ){
        const char *zSavedContext = pParse->zAuthContext;
        if( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        if( pParse->nErr || db->mallocFailed ) return WRC_Abort;
      }
    }
  
    /* If there are no aggregate functions in the result-set, and no GROUP BY 
    ** expression, do not allow aggregates in any of the other expressions.
    */
    assert( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    if( pGroupBy || sNC.hasAgg ){
      p->selFlags |= SF_Aggregate;
    }else{
      sNC.allowAgg = 0;
    }
  
    /* If a HAVING clause is present, then there must be a GROUP BY clause.
    */
    if( p->pHaving && !pGroupBy ){
      sqlite3ErrorMsg(pParse, "a GROUP BY clause is required before HAVING");
      return WRC_Abort;
    }
  
    /* Add the expression list to the name-context before parsing the
    ** other expressions in the SELECT statement. This is so that
    ** expressions in the WHERE clause (etc.) can refer to expressions by
    ** aliases in the result set.
    **
    ** Minor point: If this is the case, then the expression will be
    ** re-evaluated for each reference to it.
    */
    sNC.pEList = p->pEList;
    if( sqlite3ResolveExprNames(&sNC, p->pWhere) ||
       sqlite3ResolveExprNames(&sNC, p->pHaving)
    ){
      return WRC_Abort;
    }

    /* The ORDER BY and GROUP BY clauses may not refer to terms in
    ** outer queries 
    */
    sNC.pNext = 0;
    sNC.allowAgg = 1;

    /* Process the ORDER BY clause for singleton SELECT statements.
    ** The ORDER BY clause for compounds SELECT statements is handled
    ** below, after all of the result-sets for all of the elements of
    ** the compound have been resolved.
    */
    if( !isCompound && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER") ){
      return WRC_Abort;
    }
    if( db->mallocFailed ){
      return WRC_Abort;
    }
  
    /* Resolve the GROUP BY clause.  At the same time, make sure 
    ** the GROUP BY clause does not contain aggregate functions.
    */
    if( pGroupBy ){
      struct ExprList_item *pItem;
    
      if( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        return WRC_Abort;
      }
      for(i=0, pItem=pGroupBy->a; i<pGroupBy->nExpr; i++, pItem++){
        if( ExprHasProperty(pItem->pExpr, EP_Agg) ){
          sqlite3ErrorMsg(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          return WRC_Abort;
        }
      }
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  if( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    return WRC_Abort;
  }

  return WRC_Prune;
}

/*
** This routine walks an expression tree and resolves references to
** table columns and result-set columns.  At the same time, do error
** checking on function usage and set a flag if any aggregate functions
** are seen.
**
** To resolve table columns references we look for nodes (or subtrees) of the 
** form X.Y.Z or Y.Z or just Z where
**
**      X:   The name of a database.  Ex:  "main" or "temp" or
**           the symbolic name assigned to an ATTACH-ed database.
**
**      Y:   The name of a table in a FROM clause.  Or in a trigger
**           one of the special names "old" or "new".
**
**      Z:   The name of a column in table Y.
**
** The node at the root of the subtree is modified as follows:
**
**    Expr.op        Changed to TK_COLUMN
**    Expr.pTab      Points to the Table object for X.Y
**    Expr.iColumn   The column index in X.Y.  -1 for the rowid.
**    Expr.iTable    The VDBE cursor number for X.Y
**
**
** To resolve result-set references, look for expression nodes of the
** form Z (with no X and Y prefix) where the Z matches the right-hand
** size of an AS clause in the result-set of a SELECT.  The Z expression
** is replaced by a copy of the left-hand side of the result-set expression.
** Table-name and function resolution occurs on the substituted expression
** tree.  For example, in:
**
**      SELECT a+b AS x, c+d AS y FROM t1 ORDER BY x;
**
** The "x" term of the order by is replaced by "a+b" to render:
**
**      SELECT a+b AS x, c+d AS y FROM t1 ORDER BY a+b;
**
** Function calls are checked to make sure that the function is 
** defined and that the correct number of arguments are specified.
** If the function is an aggregate function, then the pNC->hasAgg is
** set and the opcode is changed from TK_FUNCTION to TK_AGG_FUNCTION.
** If an expression contains aggregate functions then the EP_Agg
** property on the expression is set.
**
** An error message is left in pParse if anything is amiss.  The number
** if errors is returned.
*/
SQLITE_PRIVATE int sqlite3ResolveExprNames( 
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  Expr *pExpr             /* The expression to be analyzed. */
){
  int savedHasAgg;
  Walker w;

  if( pExpr==0 ) return 0;
#if SQLITE_MAX_EXPR_DEPTH>0
  {
    Parse *pParse = pNC->pParse;
    if( sqlite3ExprCheckHeight(pParse, pExpr->nHeight+pNC->pParse->nHeight) ){
      return 1;
    }
    pParse->nHeight += pExpr->nHeight;
  }
#endif
  savedHasAgg = pNC->hasAgg;
  pNC->hasAgg = 0;
  w.xExprCallback = resolveExprStep;
  w.xSelectCallback = resolveSelectStep;
  w.pParse = pNC->pParse;
  w.u.pNC = pNC;
  sqlite3WalkExpr(&w, pExpr);
#if SQLITE_MAX_EXPR_DEPTH>0
  pNC->pParse->nHeight -= pExpr->nHeight;
#endif
  if( pNC->nErr>0 || w.pParse->nErr>0 ){
    ExprSetProperty(pExpr, EP_Error);
  }
  if( pNC->hasAgg ){
    ExprSetProperty(pExpr, EP_Agg);
  }else if( savedHasAgg ){
    pNC->hasAgg = 1;
  }
  return ExprHasProperty(pExpr, EP_Error);
}


/*
** Resolve all names in all expressions of a SELECT and in all
** decendents of the SELECT, including compounds off of p->pPrior,
** subqueries in expressions, and subqueries used as FROM clause
** terms.
**
** See sqlite3ResolveExprNames() for a description of the kinds of
** transformations that occur.
**
** All SELECT statements should have been expanded using
** sqlite3SelectExpand() prior to invoking this routine.
*/
SQLITE_PRIVATE void sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  assert( p!=0 );
  w.xExprCallback = resolveExprStep;
  w.xSelectCallback = resolveSelectStep;
  w.pParse = pParse;
  w.u.pNC = pOuterNC;
  sqlite3WalkSelect(&w, p);
}

/************** End of resolve.c *********************************************/
/************** Begin file expr.c ********************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains routines used for analyzing expressions and
** for generating VDBE code that evaluates expressions in SQLite.
*/

/*
** Return the 'affinity' of the expression pExpr if any.
**
** If pExpr is a column, a reference to a column via an 'AS' alias,
** or a sub-select with a column as the return value, then the 
** affinity of that column is returned. Otherwise, 0x00 is returned,
** indicating no affinity for the expression.
**
** i.e. the WHERE clause expresssions in the following statements all
** have an affinity:
**
** CREATE TABLE t1(a);
** SELECT * FROM t1 WHERE a;
** SELECT a AS b FROM t1 WHERE b;
** SELECT * FROM t1 WHERE (select a from t1);
*/
SQLITE_PRIVATE char sqlite3ExprAffinity(Expr *pExpr){
  int op = pExpr->op;
  if( op==TK_SELECT ){
    assert( pExpr->flags&EP_xIsSelect );
    return sqlite3ExprAffinity(pExpr->x.pSelect->pEList->a[0].pExpr);
  }
#ifndef SQLITE_OMIT_CAST
  if( op==TK_CAST ){
    assert( !ExprHasProperty(pExpr, EP_IntValue) );
    return sqlite3AffinityType(pExpr->u.zToken);
  }
#endif
  if( (op==TK_AGG_COLUMN || op==TK_COLUMN || op==TK_REGISTER) 
   && pExpr->pTab!=0
  ){
    /* op==TK_REGISTER && pExpr->pTab!=0 happens when pExpr was originally
    ** a TK_COLUMN but was previously evaluated and cached in a register */
    int j = pExpr->iColumn;
    if( j<0 ) return SQLITE_AFF_INTEGER;
    assert( pExpr->pTab && j<pExpr->pTab->nCol );
    return pExpr->pTab->aCol[j].affinity;
  }
  return pExpr->affinity;
}

/*
** Set the explicit collating sequence for an expression to the
** collating sequence supplied in the second argument.
*/
SQLITE_PRIVATE Expr *sqlite3ExprSetColl(Expr *pExpr, CollSeq *pColl){
  if( pExpr && pColl ){
    pExpr->pColl = pColl;
    pExpr->flags |= EP_ExpCollate;
  }
  return pExpr;
}

/*
** Set the collating sequence for expression pExpr to be the collating
** sequence named by pToken.   Return a pointer to the revised expression.
** The collating sequence is marked as "explicit" using the EP_ExpCollate
** flag.  An explicit collating sequence will override implicit
** collating sequences.
*/
SQLITE_PRIVATE Expr *sqlite3ExprSetCollByToken(Parse *pParse, Expr *pExpr, Token *pCollName){
  char *zColl = 0;            /* Dequoted name of collation sequence */
  CollSeq *pColl;
  sqlite3 *db = pParse->db;
  zColl = sqlite3NameFromToken(db, pCollName);
  pColl = sqlite3LocateCollSeq(pParse, zColl);
  sqlite3ExprSetColl(pExpr, pColl);
  sqlite3DbFree(db, zColl);
  return pExpr;
}

/*
** Return the default collation sequence for the expression pExpr. If
** there is no default collation type, return 0.
*/
SQLITE_PRIVATE CollSeq *sqlite3ExprCollSeq(Parse *pParse, Expr *pExpr){
  CollSeq *pColl = 0;
  Expr *p = pExpr;
  while( p ){
    int op;
    pColl = p->pColl;
    if( pColl ) break;
    op = p->op;
    if( p->pTab!=0 && (
        op==TK_AGG_COLUMN || op==TK_COLUMN || op==TK_REGISTER || op==TK_TRIGGER
    )){
      /* op==TK_REGISTER && p->pTab!=0 happens when pExpr was originally
      ** a TK_COLUMN but was previously evaluated and cached in a register */
      const char *zColl;
      int j = p->iColumn;
      if( j>=0 ){
        sqlite3 *db = pParse->db;
        zColl = p->pTab->aCol[j].zColl;
        pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
        pExpr->pColl = pColl;
      }
      break;
    }
    if( op!=TK_CAST && op!=TK_UPLUS ){
      break;
    }
    p = p->pLeft;
  }
  if( sqlite3CheckCollSeq(pParse, pColl) ){ 
    pColl = 0;
  }
  return pColl;
}

/*
** pExpr is an operand of a comparison operator.  aff2 is the
** type affinity of the other operand.  This routine returns the
** type affinity that should be used for the comparison operator.
*/
SQLITE_PRIVATE char sqlite3CompareAffinity(Expr *pExpr, char aff2){
  char aff1 = sqlite3ExprAffinity(pExpr);
  if( aff1 && aff2 ){
    /* Both sides of the comparison are columns. If one has numeric
    ** affinity, use that. Otherwise use no affinity.
    */
    if( sqlite3IsNumericAffinity(aff1) || sqlite3IsNumericAffinity(aff2) ){
      return SQLITE_AFF_NUMERIC;
    }else{
      return SQLITE_AFF_NONE;
    }
  }else if( !aff1 && !aff2 ){
    /* Neither side of the comparison is a column.  Compare the
    ** results directly.
    */
    return SQLITE_AFF_NONE;
  }else{
    /* One side is a column, the other is not. Use the columns affinity. */
    assert( aff1==0 || aff2==0 );
    return (aff1 + aff2);
  }
}

/*
** pExpr is a comparison operator.  Return the type affinity that should
** be applied to both operands prior to doing the comparison.
*/
static char comparisonAffinity(Expr *pExpr){
  char aff;
  assert( pExpr->op==TK_EQ || pExpr->op==TK_IN || pExpr->op==TK_LT ||
          pExpr->op==TK_GT || pExpr->op==TK_GE || pExpr->op==TK_LE ||
          pExpr->op==TK_NE || pExpr->op==TK_IS || pExpr->op==TK_ISNOT );
  assert( pExpr->pLeft );
  aff = sqlite3ExprAffinity(pExpr->pLeft);
  if( pExpr->pRight ){
    aff = sqlite3CompareAffinity(pExpr->pRight, aff);
  }else if( ExprHasProperty(pExpr, EP_xIsSelect) ){
    aff = sqlite3CompareAffinity(pExpr->x.pSelect->pEList->a[0].pExpr, aff);
  }else if( !aff ){
    aff = SQLITE_AFF_NONE;
  }
  return aff;
}

/*
** pExpr is a comparison expression, eg. '=', '<', IN(...) etc.
** idx_affinity is the affinity of an indexed column. Return true
** if the index with affinity idx_affinity may be used to implement
** the comparison in pExpr.
*/
SQLITE_PRIVATE int sqlite3IndexAffinityOk(Expr *pExpr, char idx_affinity){
  char aff = comparisonAffinity(pExpr);
  switch( aff ){
    case SQLITE_AFF_NONE:
      return 1;
    case SQLITE_AFF_TEXT:
      return idx_affinity==SQLITE_AFF_TEXT;
    default:
      return sqlite3IsNumericAffinity(idx_affinity);
  }
}

/*
** Return the P5 value that should be used for a binary comparison
** opcode (OP_Eq, OP_Ge etc.) used to compare pExpr1 and pExpr2.
*/
static u8 binaryCompareP5(Expr *pExpr1, Expr *pExpr2, int jumpIfNull){
  u8 aff = (char)sqlite3ExprAffinity(pExpr2);
  aff = (u8)sqlite3CompareAffinity(pExpr1, aff) | (u8)jumpIfNull;
  return aff;
}

/*
** Return a pointer to the collation sequence that should be used by
** a binary comparison operator comparing pLeft and pRight.
**
** If the left hand expression has a collating sequence type, then it is
** used. Otherwise the collation sequence for the right hand expression
** is used, or the default (BINARY) if neither expression has a collating
** type.
**
** Argument pRight (but not pLeft) may be a null pointer. In this case,
** it is not considered.
*/
SQLITE_PRIVATE CollSeq *sqlite3BinaryCompareCollSeq(
  Parse *pParse, 
  Expr *pLeft, 
  Expr *pRight
){
  CollSeq *pColl;
  assert( pLeft );
  if( pLeft->flags & EP_ExpCollate ){
    assert( pLeft->pColl );
    pColl = pLeft->pColl;
  }else if( pRight && pRight->flags & EP_ExpCollate ){
    assert( pRight->pColl );
    pColl = pRight->pColl;
  }else{
    pColl = sqlite3ExprCollSeq(pParse, pLeft);
    if( !pColl ){
      pColl = sqlite3ExprCollSeq(pParse, pRight);
    }
  }
  return pColl;
}

/*
** Generate code for a comparison operator.
*/
static int codeCompare(
  Parse *pParse,    /* The parsing (and code generating) context */
  Expr *pLeft,      /* The left operand */
  Expr *pRight,     /* The right operand */
  int opcode,       /* The comparison opcode */
  int in1, int in2, /* Register holding operands */
  int dest,         /* Jump here if true.  */
  int jumpIfNull    /* If true, jump if either operand is NULL */
){
  int p5;
  int addr;
  CollSeq *p4;

  p4 = sqlite3BinaryCompareCollSeq(pParse, pLeft, pRight);
  p5 = binaryCompareP5(pLeft, pRight, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1,
                           (void*)p4, P4_COLLSEQ);
  sqlite3VdbeChangeP5(pParse->pVdbe, (u8)p5);
  return addr;
}

#if SQLITE_MAX_EXPR_DEPTH>0
/*
** Check that argument nHeight is less than or equal to the maximum
** expression depth allowed. If it is not, leave an error message in
** pParse.
*/
SQLITE_PRIVATE int sqlite3ExprCheckHeight(Parse *pParse, int nHeight){
  int rc = SQLITE_OK;
  int mxHeight = pParse->db->aLimit[SQLITE_LIMIT_EXPR_DEPTH];
  if( nHeight>mxHeight ){
    sqlite3ErrorMsg(pParse, 
       "Expression tree is too large (maximum depth %d)", mxHeight
    );
    rc = SQLITE_ERROR;
  }
  return rc;
}

/* The following three functions, heightOfExpr(), heightOfExprList()
** and heightOfSelect(), are used to determine the maximum height
** of any expression tree referenced by the structure passed as the
** first argument.
**
** If this maximum height is greater than the current value pointed
** to by pnHeight, the second parameter, then set *pnHeight to that
** value.
*/
static void heightOfExpr(Expr *p, int *pnHeight){
  if( p ){
    if( p->nHeight>*pnHeight ){
      *pnHeight = p->nHeight;
    }
  }
}
static void heightOfExprList(ExprList *p, int *pnHeight){
  if( p ){
    int i;
    for(i=0; i<p->nExpr; i++){
      heightOfExpr(p->a[i].pExpr, pnHeight);
    }
  }
}
static void heightOfSelect(Select *p, int *pnHeight){
  if( p ){
    heightOfExpr(p->pWhere, pnHeight);
    heightOfExpr(p->pHaving, pnHeight);
    heightOfExpr(p->pLimit, pnHeight);
    heightOfExpr(p->pOffset, pnHeight);
    heightOfExprList(p->pEList, pnHeight);
    heightOfExprList(p->pGroupBy, pnHeight);
    heightOfExprList(p->pOrderBy, pnHeight);
    heightOfSelect(p->pPrior, pnHeight);
  }
}

/*
** Set the Expr.nHeight variable in the structure passed as an 
** argument. An expression with no children, Expr.pList or 
** Expr.pSelect member has a height of 1. Any other expression
** has a height equal to the maximum height of any other 
** referenced Expr plus one.
*/
static void exprSetHeight(Expr *p){
  int nHeight = 0;
  heightOfExpr(p->pLeft, &nHeight);
  heightOfExpr(p->pRight, &nHeight);
  if( ExprHasProperty(p, EP_xIsSelect) ){
    heightOfSelect(p->x.pSelect, &nHeight);
  }else{
    heightOfExprList(p->x.pList, &nHeight);
  }
  p->nHeight = nHeight + 1;
}

/*
** Set the Expr.nHeight variable using the exprSetHeight() function. If
** the height is greater than the maximum allowed expression depth,
** leave an error in pParse.
*/
SQLITE_PRIVATE void sqlite3ExprSetHeight(Parse *pParse, Expr *p){
  exprSetHeight(p);
  sqlite3ExprCheckHeight(pParse, p->nHeight);
}

/*
** Return the maximum height of any expression tree referenced
** by the select statement passed as an argument.
*/
SQLITE_PRIVATE int sqlite3SelectExprHeight(Select *p){
  int nHeight = 0;
  heightOfSelect(p, &nHeight);
  return nHeight;
}
#else
  #define exprSetHeight(y)
#endif /* SQLITE_MAX_EXPR_DEPTH>0 */

/*
** This routine is the core allocator for Expr nodes.
**
** Construct a new expression node and return a pointer to it.  Memory
** for this node and for the pToken argument is a single allocation
** obtained from sqlite3DbMalloc().  The calling function
** is responsible for making sure the node eventually gets freed.
**
** If dequote is true, then the token (if it exists) is dequoted.
** If dequote is false, no dequoting is performance.  The deQuote
** parameter is ignored if pToken is NULL or if the token does not
** appear to be quoted.  If the quotes were of the form "..." (double-quotes)
** then the EP_DblQuoted flag is set on the expression node.
**
** Special case:  If op==TK_INTEGER and pToken points to a string that
** can be translated into a 32-bit integer, then the token is not
** stored in u.zToken.  Instead, the integer values is written
** into u.iValue and the EP_IntValue flag is set.  No extra storage
** is allocated to hold the integer text and the dequote flag is ignored.
*/
SQLITE_PRIVATE Expr *sqlite3ExprAlloc(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  int op,                 /* Expression opcode */
  const Token *pToken,    /* Token argument.  Might be NULL */
  int dequote             /* True to dequote */
){
  Expr *pNew;
  int nExtra = 0;
  int iValue = 0;

  if( pToken ){
    if( op!=TK_INTEGER || pToken->z==0
          || sqlite3GetInt32(pToken->z, &iValue)==0 ){
      nExtra = pToken->n+1;
      assert( iValue>=0 );
    }
  }
  pNew = sqlite3DbMallocZero(db, sizeof(Expr)+nExtra);
  if( pNew ){
    pNew->op = (u8)op;
    pNew->iAgg = -1;
    if( pToken ){
      if( nExtra==0 ){
        pNew->flags |= EP_IntValue;
        pNew->u.iValue = iValue;
      }else{
        int c;
        pNew->u.zToken = (char*)&pNew[1];
        memcpy(pNew->u.zToken, pToken->z, pToken->n);
        pNew->u.zToken[pToken->n] = 0;
        if( dequote && nExtra>=3 
             && ((c = pToken->z[0])=='\'' || c=='"' || c=='[' || c=='`') ){
          sqlite3Dequote(pNew->u.zToken);
          if( c=='"' ) pNew->flags |= EP_DblQuoted;
        }
      }
    }
#if SQLITE_MAX_EXPR_DEPTH>0
    pNew->nHeight = 1;
#endif  
  }
  return pNew;
}

/*
** Allocate a new expression node from a zero-terminated token that has
** already been dequoted.
*/
SQLITE_PRIVATE Expr *sqlite3Expr(
  sqlite3 *db,            /* Handle for sqlite3DbMallocZero() (may be null) */
  int op,                 /* Expression opcode */
  const char *zToken      /* Token argument.  Might be NULL */
){
  Token x;
  x.z = zToken;
  x.n = zToken ? sqlite3Strlen30(zToken) : 0;
  return sqlite3ExprAlloc(db, op, &x, 0);
}

/*
** Attach subtrees pLeft and pRight to the Expr node pRoot.
**
** If pRoot==NULL that means that a memory allocation error has occurred.
** In that case, delete the subtrees pLeft and pRight.
*/
SQLITE_PRIVATE void sqlite3ExprAttachSubtrees(
  sqlite3 *db,
  Expr *pRoot,
  Expr *pLeft,
  Expr *pRight
){
  if( pRoot==0 ){
    assert( db->mallocFailed );
    sqlite3ExprDelete(db, pLeft);
    sqlite3ExprDelete(db, pRight);
  }else{
    if( pRight ){
      pRoot->pRight = pRight;
      if( pRight->flags & EP_ExpCollate ){
        pRoot->flags |= EP_ExpCollate;
        pRoot->pColl = pRight->pColl;
      }
    }
    if( pLeft ){
      pRoot->pLeft = pLeft;
      if( pLeft->flags & EP_ExpCollate ){
        pRoot->flags |= EP_ExpCollate;
        pRoot->pColl = pLeft->pColl;
      }
    }
    exprSetHeight(pRoot);
  }
}

/*
** Allocate a Expr node which joins as many as two subtrees.
**
** One or both of the subtrees can be NULL.  Return a pointer to the new
** Expr node.  Or, if an OOM error occurs, set pParse->db->mallocFailed,
** free the subtrees and return NULL.
*/
SQLITE_PRIVATE Expr *sqlite3PExpr(
  Parse *pParse,          /* Parsing context */
  int op,                 /* Expression opcode */
  Expr *pLeft,            /* Left operand */
  Expr *pRight,           /* Right operand */
  const Token *pToken     /* Argument token */
){
  Expr *p = sqlite3ExprAlloc(pParse->db, op, pToken, 1);
  sqlite3ExprAttachSubtrees(pParse->db, p, pLeft, pRight);
  if( p ) {
    sqlite3ExprCheckHeight(pParse, p->nHeight);
  }
  return p;
}

/*
** Join two expressions using an AND operator.  If either expression is
** NULL, then just return the other expression.
*/
SQLITE_PRIVATE Expr *sqlite3ExprAnd(sqlite3 *db, Expr *pLeft, Expr *pRight){
  if( pLeft==0 ){
    return pRight;
  }else if( pRight==0 ){
    return pLeft;
  }else{
    Expr *pNew = sqlite3ExprAlloc(db, TK_AND, 0, 0);
    sqlite3ExprAttachSubtrees(db, pNew, pLeft, pRight);
    return pNew;
  }
}

/*
** Construct a new expression node for a function with multiple
** arguments.
*/
SQLITE_PRIVATE Expr *sqlite3ExprFunction(Parse *pParse, ExprList *pList, Token *pToken){
  Expr *pNew;
  sqlite3 *db = pParse->db;
  assert( pToken );
  pNew = sqlite3ExprAlloc(db, TK_FUNCTION, pToken, 1);
  if( pNew==0 ){
    sqlite3ExprListDelete(db, pList); /* Avoid memory leak when malloc fails */
    return 0;
  }
  pNew->x.pList = pList;
  assert( !ExprHasProperty(pNew, EP_xIsSelect) );
  sqlite3ExprSetHeight(pParse, pNew);
  return pNew;
}

/*
** Assign a variable number to an expression that encodes a wildcard
** in the original SQL statement.  
**
** Wildcards consisting of a single "?" are assigned the next sequential
** variable number.
**
** Wildcards of the form "?nnn" are assigned the number "nnn".  We make
** sure "nnn" is not too be to avoid a denial of service attack when
** the SQL statement comes from an external source.
**
** Wildcards of the form ":aaa", "@aaa", or "$aaa" are assigned the same number
** as the previous instance of the same wildcard.  Or if this is the first
** instance of the wildcard, the next sequenial variable number is
** assigned.
*/
SQLITE_PRIVATE void sqlite3ExprAssignVarNumber(Parse *pParse, Expr *pExpr){
  sqlite3 *db = pParse->db;
  const char *z;

  if( pExpr==0 ) return;
  assert( !ExprHasAnyProperty(pExpr, EP_IntValue|EP_Reduced|EP_TokenOnly) );
  z = pExpr->u.zToken;
  assert( z!=0 );
  assert( z[0]!=0 );
  if( z[1]==0 ){
    /* Wildcard of the form "?".  Assign the next variable number */
    assert( z[0]=='?' );
    pExpr->iColumn = (ynVar)(++pParse->nVar);
  }else{
    ynVar x = 0;
    u32 n = sqlite3Strlen30(z);
    if( z[0]=='?' ){
      /* Wildcard of the form "?nnn".  Convert "nnn" to an integer and
      ** use it as the variable number */
      i64 i;
      int bOk = 0==sqlite3Atoi64(&z[1], &i, n-1, SQLITE_UTF8);
      pExpr->iColumn = x = (ynVar)i;
      testcase( i==0 );
      testcase( i==1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER]-1 );
      testcase( i==db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER] );
      if( bOk==0 || i<1 || i>db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER] ){
        sqlite3ErrorMsg(pParse, "variable number must be between ?1 and ?%d",
            db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER]);
        x = 0;
      }
      if( i>pParse->nVar ){
        pParse->nVar = (int)i;
      }
    }else{
      /* Wildcards like ":aaa", "$aaa" or "@aaa".  Reuse the same variable
      ** number as the prior appearance of the same name, or if the name
      ** has never appeared before, reuse the same variable number
      */
      ynVar i;
      for(i=0; i<pParse->nzVar; i++){
        if( pParse->azVar[i] && memcmp(pParse->azVar[i],z,n+1)==0 ){
          pExpr->iColumn = x = (ynVar)i+1;
          break;
        }
      }
      if( x==0 ) x = pExpr->iColumn = (ynVar)(++pParse->nVar);
    }
    if( x>0 ){
      if( x>pParse->nzVar ){
        char **a;
        a = sqlite3DbRealloc(db, pParse->azVar, x*sizeof(a[0]));
        if( a==0 ) return;  /* Error reported through db->mallocFailed */
        pParse->azVar = a;
        memset(&a[pParse->nzVar], 0, (x-pParse->nzVar)*sizeof(a[0]));
        pParse->nzVar = x;
      }
      if( z[0]!='?' || pParse->azVar[x-1]==0 ){
        sqlite3DbFree(db, pParse->azVar[x-1]);
        pParse->azVar[x-1] = sqlite3DbStrNDup(db, z, n);
      }
    }
  } 
  if( !pParse->nErr && pParse->nVar>db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER] ){
    sqlite3ErrorMsg(pParse, "too many SQL variables");
  }
}

/*
** Recursively delete an expression tree.
*/
SQLITE_PRIVATE void sqlite3ExprDelete(sqlite3 *db, Expr *p){
  if( p==0 ) return;
  /* Sanity check: Assert that the IntValue is non-negative if it exists */
  assert( !ExprHasProperty(p, EP_IntValue) || p->u.iValue>=0 );
  if( !ExprHasAnyProperty(p, EP_TokenOnly) ){
    sqlite3ExprDelete(db, p->pLeft);
    sqlite3ExprDelete(db, p->pRight);
    if( !ExprHasProperty(p, EP_Reduced) && (p->flags2 & EP2_MallocedToken)!=0 ){
      sqlite3DbFree(db, p->u.zToken);
    }
    if( ExprHasProperty(p, EP_xIsSelect) ){
      sqlite3SelectDelete(db, p->x.pSelect);
    }else{
      sqlite3ExprListDelete(db, p->x.pList);
    }
  }
  if( !ExprHasProperty(p, EP_Static) ){
    sqlite3DbFree(db, p);
  }
}

/*
** Return the number of bytes allocated for the expression structure 
** passed as the first argument. This is always one of EXPR_FULLSIZE,
** EXPR_REDUCEDSIZE or EXPR_TOKENONLYSIZE.
*/
static int exprStructSize(Expr *p){
  if( ExprHasProperty(p, EP_TokenOnly) ) return EXPR_TOKENONLYSIZE;
  if( ExprHasProperty(p, EP_Reduced) ) return EXPR_REDUCEDSIZE;
  return EXPR_FULLSIZE;
}

/*
** The dupedExpr*Size() routines each return the number of bytes required
** to store a copy of an expression or expression tree.  They differ in
** how much of the tree is measured.
**
**     dupedExprStructSize()     Size of only the Expr structure 
**     dupedExprNodeSize()       Size of Expr + space for token
**     dupedExprSize()           Expr + token + subtree components
**
***************************************************************************
**
** The dupedExprStructSize() function returns two values OR-ed together:  
** (1) the space required for a copy of the Expr structure only and 
** (2) the EP_xxx flags that indicate what the structure size should be.
** The return values is always one of:
**
**      EXPR_FULLSIZE
**      EXPR_REDUCEDSIZE   | EP_Reduced
**      EXPR_TOKENONLYSIZE | EP_TokenOnly
**
** The size of the structure can be found by masking the return value
** of this routine with 0xfff.  The flags can be found by masking the
** return value with EP_Reduced|EP_TokenOnly.
**
** Note that with flags==EXPRDUP_REDUCE, this routines works on full-size
** (unreduced) Expr objects as they or originally constructed by the parser.
** During expression analysis, extra information is computed and moved into
** later parts of teh Expr object and that extra information might get chopped
** off if the expression is reduced.  Note also that it does not work to
** make a EXPRDUP_REDUCE copy of a reduced expression.  It is only legal
** to reduce a pristine expression tree from the parser.  The implementation
** of dupedExprStructSize() contain multiple assert() statements that attempt
** to enforce this constraint.
*/
static int dupedExprStructSize(Expr *p, int flags){
  int nSize;
  assert( flags==EXPRDUP_REDUCE || flags==0 ); /* Only one flag value allowed */
  if( 0==(flags&EXPRDUP_REDUCE) ){
    nSize = EXPR_FULLSIZE;
  }else{
    assert( !ExprHasAnyProperty(p, EP_TokenOnly|EP_Reduced) );
    assert( !ExprHasProperty(p, EP_FromJoin) ); 
    assert( (p->flags2 & EP2_MallocedToken)==0 );
    assert( (p->flags2 & EP2_Irreducible)==0 );
    if( p->pLeft || p->pRight || p->pColl || p->x.pList ){
      nSize = EXPR_REDUCEDSIZE | EP_Reduced;
    }else{
      nSize = EXPR_TOKENONLYSIZE | EP_TokenOnly;
    }
  }
  return nSize;
}

/*
** This function returns the space in bytes required to store the copy 
** of the Expr structure and a copy of the Expr.u.zToken string (if that
** string is defined.)
*/
static int dupedExprNodeSize(Expr *p, int flags){
  int nByte = dupedExprStructSize(p, flags) & 0xfff;
  if( !ExprHasProperty(p, EP_IntValue) && p->u.zToken ){
    nByte += sqlite3Strlen30(p->u.zToken)+1;
  }
  return ROUND8(nByte);
}

/*
** Return the number of bytes required to create a duplicate of the 
** expression passed as the first argument. The second argument is a
** mask containing EXPRDUP_XXX flags.
**
** The value returned includes space to create a copy of the Expr struct
** itself and the buffer referred to by Expr.u.zToken, if any.
**
** If the EXPRDUP_REDUCE flag is set, then the return value includes 
** space to duplicate all Expr nodes in the tree formed by Expr.pLeft 
** and Expr.pRight variables (but not for any structures pointed to or 
** descended from the Expr.x.pList or Expr.x.pSelect variables).
*/
static int dupedExprSize(Expr *p, int flags){
  int nByte = 0;
  if( p ){
    nByte = dupedExprNodeSize(p, flags);
    if( flags&EXPRDUP_REDUCE ){
      nByte += dupedExprSize(p->pLeft, flags) + dupedExprSize(p->pRight, flags);
    }
  }
  return nByte;
}

/*
** This function is similar to sqlite3ExprDup(), except that if pzBuffer 
** is not NULL then *pzBuffer is assumed to point to a buffer large enough 
** to store the copy of expression p, the copies of p->u.zToken
** (if applicable), and the copies of the p->pLeft and p->pRight expressions,
** if any. Before returning, *pzBuffer is set to the first byte passed the
** portion of the buffer copied into by this function.
*/
static Expr *exprDup(sqlite3 *db, Expr *p, int flags, u8 **pzBuffer){
  Expr *pNew = 0;                      /* Value to return */
  if( p ){
    const int isReduced = (flags&EXPRDUP_REDUCE);
    u8 *zAlloc;
    u32 staticFlag = 0;

    assert( pzBuffer==0 || isReduced );

    /* Figure out where to write the new Expr structure. */
    if( pzBuffer ){
      zAlloc = *pzBuffer;
      staticFlag = EP_Static;
    }else{
      zAlloc = sqlite3DbMallocRaw(db, dupedExprSize(p, flags));
    }
    pNew = (Expr *)zAlloc;

    if( pNew ){
      /* Set nNewSize to the size allocated for the structure pointed to
      ** by pNew. This is either EXPR_FULLSIZE, EXPR_REDUCEDSIZE or
      ** EXPR_TOKENONLYSIZE. nToken is set to the number of bytes consumed
      ** by the copy of the p->u.zToken string (if any).
      */
      const unsigned nStructSize = dupedExprStructSize(p, flags);
      const int nNewSize = nStructSize & 0xfff;
      int nToken;
      if( !ExprHasProperty(p, EP_IntValue) && p->u.zToken ){
        nToken = sqlite3Strlen30(p->u.zToken) + 1;
      }else{
        nToken = 0;
      }
      if( isReduced ){
        assert( ExprHasProperty(p, EP_Reduced)==0 );
        memcpy(zAlloc, p, nNewSize);
      }else{
        int nSize = exprStructSize(p);
        memcpy(zAlloc, p, nSize);
        memset(&zAlloc[nSize], 0, EXPR_FULLSIZE-nSize);
      }

      /* Set the EP_Reduced, EP_TokenOnly, and EP_Static flags appropriately. */
      pNew->flags &= ~(EP_Reduced|EP_TokenOnly|EP_Static);
      pNew->flags |= nStructSize & (EP_Reduced|EP_TokenOnly);
      pNew->flags |= staticFlag;

      /* Copy the p->u.zToken string, if any. */
      if( nToken ){
        char *zToken = pNew->u.zToken = (char*)&zAlloc[nNewSize];
        memcpy(zToken, p->u.zToken, nToken);
      }

      if( 0==((p->flags|pNew->flags) & EP_TokenOnly) ){
        /* Fill in the pNew->x.pSelect or pNew->x.pList member. */
        if( ExprHasProperty(p, EP_xIsSelect) ){
          pNew->x.pSelect = sqlite3SelectDup(db, p->x.pSelect, isReduced);
        }else{
          pNew->x.pList = sqlite3ExprListDup(db, p->x.pList, isReduced);
        }
      }

      /* Fill in pNew->pLeft and pNew->pRight. */
      if( ExprHasAnyProperty(pNew, EP_Reduced|EP_TokenOnly) ){
        zAlloc += dupedExprNodeSize(p, flags);
        if( ExprHasProperty(pNew, EP_Reduced) ){
          pNew->pLeft = exprDup(db, p->pLeft, EXPRDUP_REDUCE, &zAlloc);
          pNew->pRight = exprDup(db, p->pRight, EXPRDUP_REDUCE, &zAlloc);
        }
        if( pzBuffer ){
          *pzBuffer = zAlloc;
        }
      }else{
        pNew->flags2 = 0;
        if( !ExprHasAnyProperty(p, EP_TokenOnly) ){
          pNew->pLeft = sqlite3ExprDup(db, p->pLeft, 0);
          pNew->pRight = sqlite3ExprDup(db, p->pRight, 0);
        }
      }

    }
  }
  return pNew;
}

/*
** The following group of routines make deep copies of expressions,
** expression lists, ID lists, and select statements.  The copies can
** be deleted (by being passed to their respective ...Delete() routines)
** without effecting the originals.
**
** The expression list, ID, and source lists return by sqlite3ExprListDup(),
** sqlite3IdListDup(), and sqlite3SrcListDup() can not be further expanded 
** by subsequent calls to sqlite*ListAppend() routines.
**
** Any tables that the SrcList might point to are not duplicated.
**
** The flags parameter contains a combination of the EXPRDUP_XXX flags.
** If the EXPRDUP_REDUCE flag is set, then the structure returned is a
** truncated version of the usual Expr structure that will be stored as
** part of the in-memory representation of the database schema.
*/
SQLITE_PRIVATE Expr *sqlite3ExprDup(sqlite3 *db, Expr *p, int flags){
  return exprDup(db, p, flags, 0);
}
SQLITE_PRIVATE ExprList *sqlite3ExprListDup(sqlite3 *db, ExprList *p, int flags){
  ExprList *pNew;
  struct ExprList_item *pItem, *pOldItem;
  int i;
  if( p==0 ) return 0;
  pNew = sqlite3DbMallocRaw(db, sizeof(*pNew) );
  if( pNew==0 ) return 0;
  pNew->iECursor = 0;
  pNew->nExpr = pNew->nAlloc = p->nExpr;
  pNew->a = pItem = sqlite3DbMallocRaw(db,  p->nExpr*sizeof(p->a[0]) );
  if( pItem==0 ){
    sqlite3DbFree(db, pNew);
    return 0;
  } 
  pOldItem = p->a;
  for(i=0; i<p->nExpr; i++, pItem++, pOldItem++){
    Expr *pOldExpr = pOldItem->pExpr;
    pItem->pExpr = sqlite3ExprDup(db, pOldExpr, flags);
    pItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pItem->zSpan = sqlite3DbStrDup(db, pOldItem->zSpan);
    pItem->sortOrder = pOldItem->sortOrder;
    pItem->done = 0;
    pItem->iCol = pOldItem->iCol;
    pItem->iAlias = pOldItem->iAlias;
  }
  return pNew;
}

/*
** If cursors, triggers, views and subqueries are all omitted from
** the build, then none of the following routines, except for 
** sqlite3SelectDup(), can be called. sqlite3SelectDup() is sometimes
** called with a NULL argument.
*/
#if !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER) \
 || !defined(SQLITE_OMIT_SUBQUERY)
SQLITE_PRIVATE SrcList *sqlite3SrcListDup(sqlite3 *db, SrcList *p, int flags){
  SrcList *pNew;
  int i;
  int nByte;
  if( p==0 ) return 0;
  nByte = sizeof(*p) + (p->nSrc>0 ? sizeof(p->a[0]) * (p->nSrc-1) : 0);
  pNew = sqlite3DbMallocRaw(db, nByte );
  if( pNew==0 ) return 0;
  pNew->nSrc = pNew->nAlloc = p->nSrc;
  for(i=0; i<p->nSrc; i++){
    struct SrcList_item *pNewItem = &pNew->a[i];
    struct SrcList_item *pOldItem = &p->a[i];
    Table *pTab;
    pNewItem->zDatabase = sqlite3DbStrDup(db, pOldItem->zDatabase);
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->zAlias = sqlite3DbStrDup(db, pOldItem->zAlias);
    pNewItem->jointype = pOldItem->jointype;
    pNewItem->iCursor = pOldItem->iCursor;
    pNewItem->isPopulated = pOldItem->isPopulated;
    pNewItem->zIndex = sqlite3DbStrDup(db, pOldItem->zIndex);
    pNewItem->notIndexed = pOldItem->notIndexed;
    pNewItem->pIndex = pOldItem->pIndex;
    pTab = pNewItem->pTab = pOldItem->pTab;
    if( pTab ){
      pTab->nRef++;
    }
    pNewItem->pSelect = sqlite3SelectDup(db, pOldItem->pSelect, flags);
    pNewItem->pOn = sqlite3ExprDup(db, pOldItem->pOn, flags);
    pNewItem->pUsing = sqlite3IdListDup(db, pOldItem->pUsing);
    pNewItem->colUsed = pOldItem->colUsed;
  }
  return pNew;
}
SQLITE_PRIVATE IdList *sqlite3IdListDup(sqlite3 *db, IdList *p){
  IdList *pNew;
  int i;
  if( p==0 ) return 0;
  pNew = sqlite3DbMallocRaw(db, sizeof(*pNew) );
  if( pNew==0 ) return 0;
  pNew->nId = pNew->nAlloc = p->nId;
  pNew->a = sqlite3DbMallocRaw(db, p->nId*sizeof(p->a[0]) );
  if( pNew->a==0 ){
    sqlite3DbFree(db, pNew);
    return 0;
  }
  for(i=0; i<p->nId; i++){
    struct IdList_item *pNewItem = &pNew->a[i];
    struct IdList_item *pOldItem = &p->a[i];
    pNewItem->zName = sqlite3DbStrDup(db, pOldItem->zName);
    pNewItem->idx = pOldItem->idx;
  }
  return pNew;
}
SQLITE_PRIVATE Select *sqlite3SelectDup(sqlite3 *db, Select *p, int flags){
  Select *pNew;
  if( p==0 ) return 0;
  pNew = sqlite3DbMallocRaw(db, sizeof(*p) );
  if( pNew==0 ) return 0;
  pNew->pEList = sqlite3ExprListDup(db, p->pEList, flags);
  pNew->pSrc = sqlite3SrcListDup(db, p->pSrc, flags);
  pNew->pWhere = sqlite3ExprDup(db, p->pWhere, flags);
  pNew->pGroupBy = sqlite3ExprListDup(db, p->pGroupBy, flags);
  pNew->pHaving = sqlite3ExprDup(db, p->pHaving, flags);
  pNew->pOrderBy = sqlite3ExprListDup(db, p->pOrderBy, flags);
  pNew->op = p->op;
  pNew->pPrior = sqlite3SelectDup(db, p->pPrior, flags);
  pNew->pLimit = sqlite3ExprDup(db, p->pLimit, flags);
  pNew->pOffset = sqlite3ExprDup(db, p->pOffset, flags);
  pNew->iLimit = 0;
  pNew->iOffset = 0;
  pNew->selFlags = p->selFlags & ~SF_UsesEphemeral;
  pNew->pRightmost = 0;
  pNew->addrOpenEphm[0] = -1;
  pNew->addrOpenEphm[1] = -1;
  pNew->addrOpenEphm[2] = -1;
  return pNew;
}
#else
SQLITE_PRIVATE Select *sqlite3SelectDup(sqlite3 *db, Select *p, int flags){
  assert( p==0 );
  return 0;
}
#endif


/*
** Add a new element to the end of an expression list.  If pList is
** initially NULL, then create a new expression list.
**
** If a memory allocation error occurs, the entire list is freed and
** NULL is returned.  If non-NULL is returned, then it is guaranteed
** that the new entry was successfully appended.
*/
SQLITE_PRIVATE ExprList *sqlite3ExprListAppend(
  Parse *pParse,          /* Parsing context */
  ExprList *pList,        /* List to which to append. Might be NULL */
  Expr *pExpr             /* Expression to be appended. Might be NULL */
){
  sqlite3 *db = pParse->db;
  if( pList==0 ){
    pList = sqlite3DbMallocZero(db, sizeof(ExprList) );
    if( pList==0 ){
      goto no_mem;
    }
    assert( pList->nAlloc==0 );
  }
  if( pList->nAlloc<=pList->nExpr ){
    struct ExprList_item *a;
    int n = pList->nAlloc*2 + 4;
    a = sqlite3DbRealloc(db, pList->a, n*sizeof(pList->a[0]));
    if( a==0 ){
      goto no_mem;
    }
    pList->a = a;
    pList->nAlloc = sqlite3DbMallocSize(db, a)/sizeof(a[0]);
  }
  assert( pList->a!=0 );
  if( 1 ){
    struct ExprList_item *pItem = &pList->a[pList->nExpr++];
    memset(pItem, 0, sizeof(*pItem));
    pItem->pExpr = pExpr;
  }
  return pList;

no_mem:     
  /* Avoid leaking memory if malloc has failed. */
  sqlite3ExprDelete(db, pExpr);
  sqlite3ExprListDelete(db, pList);
  return 0;
}

/*
** Set the ExprList.a[].zName element of the most recently added item
** on the expression list.
**
** pList might be NULL following an OOM error.  But pName should never be
** NULL.  If a memory allocation fails, the pParse->db->mallocFailed flag
** is set.
*/
SQLITE_PRIVATE void sqlite3ExprListSetName(
  Parse *pParse,          /* Parsing context */
  ExprList *pList,        /* List to which to add the span. */
  Token *pName,           /* Name to be added */
  int dequote             /* True to cause the name to be dequoted */
){
  assert( pList!=0 || pParse->db->mallocFailed!=0 );
  if( pList ){
    struct ExprList_item *pItem;
    assert( pList->nExpr>0 );
    pItem = &pList->a[pList->nExpr-1];
    assert( pItem->zName==0 );
    pItem->zName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    if( dequote && pItem->zName ) sqlite3Dequote(pItem->zName);
  }
}

/*
** Set the ExprList.a[].zSpan element of the most recently added item
** on the expression list.
**
** pList might be NULL following an OOM error.  But pSpan should never be
** NULL.  If a memory allocation fails, the pParse->db->mallocFailed flag
** is set.
*/
SQLITE_PRIVATE void sqlite3ExprListSetSpan(
  Parse *pParse,          /* Parsing context */
  ExprList *pList,        /* List to which to add the span. */
  ExprSpan *pSpan         /* The span to be added */
){
  sqlite3 *db = pParse->db;
  assert( pList!=0 || db->mallocFailed!=0 );
  if( pList ){
    struct ExprList_item *pItem = &pList->a[pList->nExpr-1];
    assert( pList->nExpr>0 );
    assert( db->mallocFailed || pItem->pExpr==pSpan->pExpr );
    sqlite3DbFree(db, pItem->zSpan);
    pItem->zSpan = sqlite3DbStrNDup(db, (char*)pSpan->zStart,
                                    (int)(pSpan->zEnd - pSpan->zStart));
  }
}

/*
** If the expression list pEList contains more than iLimit elements,
** leave an error message in pParse.
*/
SQLITE_PRIVATE void sqlite3ExprListCheckLength(
  Parse *pParse,
  ExprList *pEList,
  const char *zObject
){
  int mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  testcase( pEList && pEList->nExpr==mx );
  testcase( pEList && pEList->nExpr==mx+1 );
  if( pEList && pEList->nExpr>mx ){
    sqlite3ErrorMsg(pParse, "too many columns in %s", zObject);
  }
}

/*
** Delete an entire expression list.
*/
SQLITE_PRIVATE void sqlite3ExprListDelete(sqlite3 *db, ExprList *pList){
  int i;
  struct ExprList_item *pItem;
  if( pList==0 ) return;
  assert( pList->a!=0 || (pList->nExpr==0 && pList->nAlloc==0) );
  assert( pList->nExpr<=pList->nAlloc );
  for(pItem=pList->a, i=0; i<pList->nExpr; i++, pItem++){
    sqlite3ExprDelete(db, pItem->pExpr);
    sqlite3DbFree(db, pItem->zName);
    sqlite3DbFree(db, pItem->zSpan);
  }
  sqlite3DbFree(db, pList->a);
  sqlite3DbFree(db, pList);
}

/*
** These routines are Walker callbacks.  Walker.u.pi is a pointer
** to an integer.  These routines are checking an expression to see
** if it is a constant.  Set *Walker.u.pi to 0 if the expression is
** not constant.
**
** These callback routines are used to implement the following:
**
**     sqlite3ExprIsConstant()
**     sqlite3ExprIsConstantNotJoin()
**     sqlite3ExprIsConstantOrFunction()
**
*/
static int exprNodeIsConstant(Walker *pWalker, Expr *pExpr){

  /* If pWalker->u.i is 3 then any term of the expression that comes from
  ** the ON or USING clauses of a join disqualifies the expression
  ** from being considered constant. */
  if( pWalker->u.i==3 && ExprHasAnyProperty(pExpr, EP_FromJoin) ){
    pWalker->u.i = 0;
    return WRC_Abort;
  }

  switch( pExpr->op ){
    /* Consider functions to be constant if all their arguments are constant
    ** and pWalker->u.i==2 */
    case TK_FUNCTION:
      if( pWalker->u.i==2 ) return 0;
      /* Fall through */
    case TK_ID:
    case TK_COLUMN:
    case TK_AGG_FUNCTION:
    case TK_AGG_COLUMN:
      testcase( pExpr->op==TK_ID );
      testcase( pExpr->op==TK_COLUMN );
      testcase( pExpr->op==TK_AGG_FUNCTION );
      testcase( pExpr->op==TK_AGG_COLUMN );
      pWalker->u.i = 0;
      return WRC_Abort;
    default:
      testcase( pExpr->op==TK_SELECT ); /* selectNodeIsConstant will disallow */
      testcase( pExpr->op==TK_EXISTS ); /* selectNodeIsConstant will disallow */
      return WRC_Continue;
  }
}
static int selectNodeIsConstant(Walker *pWalker, Select *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  pWalker->u.i = 0;
  return WRC_Abort;
}
static int exprIsConst(Expr *p, int initFlag){
  Walker w;
  w.u.i = initFlag;
  w.xExprCallback = exprNodeIsConstant;
  w.xSelectCallback = selectNodeIsConstant;
  sqlite3WalkExpr(&w, p);
  return w.u.i;
}

/*
** Walk an expression tree.  Return 1 if the expression is constant
** and 0 if it involves variables or function calls.
**
** For the purposes of this function, a double-quoted string (ex: "abc")
** is considered a variable but a single-quoted string (ex: 'abc') is
** a constant.
*/
SQLITE_PRIVATE int sqlite3ExprIsConstant(Expr *p){
  return exprIsConst(p, 1);
}

/*
** Walk an expression tree.  Return 1 if the expression is constant
** that does no originate from the ON or USING clauses of a join.
** Return 0 if it involves variables or function calls or terms from
** an ON or USING clause.
*/
SQLITE_PRIVATE int sqlite3ExprIsConstantNotJoin(Expr *p){
  return exprIsConst(p, 3);
}

/*
** Walk an expression tree.  Return 1 if the expression is constant
** or a function call with constant arguments.  Return and 0 if there
** are any variables.
**
** For the purposes of this function, a double-quoted string (ex: "abc")
** is considered a variable but a single-quoted string (ex: 'abc') is
** a constant.
*/
SQLITE_PRIVATE int sqlite3ExprIsConstantOrFunction(Expr *p){
  return exprIsConst(p, 2);
}

/*
** If the expression p codes a constant integer that is small enough
** to fit in a 32-bit integer, return 1 and put the value of the integer
** in *pValue.  If the expression is not an integer or if it is too big
** to fit in a signed 32-bit integer, return 0 and leave *pValue unchanged.
*/
SQLITE_PRIVATE int sqlite3ExprIsInteger(Expr *p, int *pValue){
  int rc = 0;

  /* If an expression is an integer literal that fits in a signed 32-bit
  ** integer, then the EP_IntValue flag will have already been set */
  assert( p->op!=TK_INTEGER || (p->flags & EP_IntValue)!=0
           || sqlite3GetInt32(p->u.zToken, &rc)==0 );

  if( p->flags & EP_IntValue ){
    *pValue = p->u.iValue;
    return 1;
  }
  switch( p->op ){
    case TK_UPLUS: {
      rc = sqlite3ExprIsInteger(p->pLeft, pValue);
      break;
    }
    case TK_UMINUS: {
      int v;
      if( sqlite3ExprIsInteger(p->pLeft, &v) ){
        *pValue = -v;
        rc = 1;
      }
      break;
    }
    default: break;
  }
  return rc;
}

/*
** Return FALSE if there is no chance that the expression can be NULL.
**
** If the expression might be NULL or if the expression is too complex
** to tell return TRUE.  
**
** This routine is used as an optimization, to skip OP_IsNull opcodes
** when we know that a value cannot be NULL.  Hence, a false positive
** (returning TRUE when in fact the expression can never be NULL) might
** be a small performance hit but is otherwise harmless.  On the other
** hand, a false negative (returning FALSE when the result could be NULL)
** will likely result in an incorrect answer.  So when in doubt, return
** TRUE.
*/
SQLITE_PRIVATE int sqlite3ExprCanBeNull(const Expr *p){
  u8 op;
  while( p->op==TK_UPLUS || p->op==TK_UMINUS ){ p = p->pLeft; }
  op = p->op;
  if( op==TK_REGISTER ) op = p->op2;
  switch( op ){
    case TK_INTEGER:
    case TK_STRING:
    case TK_FLOAT:
    case TK_BLOB:
      return 0;
    default:
      return 1;
  }
}

/*
** Generate an OP_IsNull instruction that tests register iReg and jumps
** to location iDest if the value in iReg is NULL.  The value in iReg 
** was computed by pExpr.  If we can look at pExpr at compile-time and
** determine that it can never generate a NULL, then the OP_IsNull operation
** can be omitted.
*/
SQLITE_PRIVATE void sqlite3ExprCodeIsNullJump(
  Vdbe *v,            /* The VDBE under construction */
  const Expr *pExpr,  /* Only generate OP_IsNull if this expr can be NULL */
  int iReg,           /* Test the value in this register for NULL */
  int iDest           /* Jump here if the value is null */
){
  if( sqlite3ExprCanBeNull(pExpr) ){
    sqlite3VdbeAddOp2(v, OP_IsNull, iReg, iDest);
  }
}

/*
** Return TRUE if the given expression is a constant which would be
** unchanged by OP_Affinity with the affinity given in the second
** argument.
**
** This routine is used to determine if the OP_Affinity operation
** can be omitted.  When in doubt return FALSE.  A false negative
** is harmless.  A false positive, however, can result in the wrong
** answer.
*/
SQLITE_PRIVATE int sqlite3ExprNeedsNoAffinityChange(const Expr *p, char aff){
  u8 op;
  if( aff==SQLITE_AFF_NONE ) return 1;
  while( p->op==TK_UPLUS || p->op==TK_UMINUS ){ p = p->pLeft; }
  op = p->op;
  if( op==TK_REGISTER ) op = p->op2;
  switch( op ){
    case TK_INTEGER: {
      return aff==SQLITE_AFF_INTEGER || aff==SQLITE_AFF_NUMERIC;
    }
    case TK_FLOAT: {
      return aff==SQLITE_AFF_REAL || aff==SQLITE_AFF_NUMERIC;
    }
    case TK_STRING: {
      return aff==SQLITE_AFF_TEXT;
    }
    case TK_BLOB: {
      return 1;
    }
    case TK_COLUMN: {
      assert( p->iTable>=0 );  /* p cannot be part of a CHECK constraint */
      return p->iColumn<0
          && (aff==SQLITE_AFF_INTEGER || aff==SQLITE_AFF_NUMERIC);
    }
    default: {
      return 0;
    }
  }
}

/*
** Return TRUE if the given string is a row-id column name.
*/
SQLITE_PRIVATE int sqlite3IsRowid(const char *z){
  if( sqlite3StrICmp(z, "_ROWID_")==0 ) return 1;
  if( sqlite3StrICmp(z, "ROWID")==0 ) return 1;
  if( sqlite3StrICmp(z, "OID")==0 ) return 1;
  return 0;
}

/*
** Return true if we are able to the IN operator optimization on a
** query of the form
**
**       x IN (SELECT ...)
**
** Where the SELECT... clause is as specified by the parameter to this
** routine.
**
** The Select object passed in has already been preprocessed and no
** errors have been found.
*/
#ifndef SQLITE_OMIT_SUBQUERY
static int isCandidateForInOpt(Select *p){
  SrcList *pSrc;
  ExprList *pEList;
  Table *pTab;
  if( p==0 ) return 0;                   /* right-hand side of IN is SELECT */
  if( p->pPrior ) return 0;              /* Not a compound SELECT */
  if( p->selFlags & (SF_Distinct|SF_Aggregate) ){
    testcase( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
    testcase( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
    return 0; /* No DISTINCT keyword and no aggregate functions */
  }
  assert( p->pGroupBy==0 );              /* Has no GROUP BY clause */
  if( p->pLimit ) return 0;              /* Has no LIMIT clause */
  assert( p->pOffset==0 );               /* No LIMIT means no OFFSET */
  if( p->pWhere ) return 0;              /* Has no WHERE clause */
  pSrc = p->pSrc;
  assert( pSrc!=0 );
  if( pSrc->nSrc!=1 ) return 0;          /* Single term in FROM clause */
  if( pSrc->a[0].pSelect ) return 0;     /* FROM is not a subquery or view */
  pTab = pSrc->a[0].pTab;
  if( NEVER(pTab==0) ) return 0;
  assert( pTab->pSelect==0 );            /* FROM clause is not a view */
  if( IsVirtual(pTab) ) return 0;        /* FROM clause not a virtual table */
  pEList = p->pEList;
  if( pEList->nExpr!=1 ) return 0;       /* One column in the result set */
  if( pEList->a[0].pExpr->op!=TK_COLUMN ) return 0; /* Result is a column */
  return 1;
}
#endif /* SQLITE_OMIT_SUBQUERY */

/*
** This function is used by the implementation of the IN (...) operator.
** It's job is to find or create a b-tree structure that may be used
** either to test for membership of the (...) set or to iterate through
** its members, skipping duplicates.
**
** The index of the cursor opened on the b-tree (database table, database index 
** or ephermal table) is stored in pX->iTable before this function returns.
** The returned value of this function indicates the b-tree type, as follows:
**
**   IN_INDEX_ROWID - The cursor was opened on a database table.
**   IN_INDEX_INDEX - The cursor was opened on a database index.
**   IN_INDEX_EPH -   The cursor was opened on a specially created and
**                    populated epheremal table.
**
** An existing b-tree may only be used if the SELECT is of the simple
** form:
**
**     SELECT <column> FROM <table>
**
** If the prNotFound parameter is 0, then the b-tree will be used to iterate
** through the set members, skipping any duplicates. In this case an
** epheremal table must be used unless the selected <column> is guaranteed
** to be unique - either because it is an INTEGER PRIMARY KEY or it
** has a UNIQUE constraint or UNIQUE index.
**
** If the prNotFound parameter is not 0, then the b-tree will be used 
** for fast set membership tests. In this case an epheremal table must 
** be used unless <column> is an INTEGER PRIMARY KEY or an index can 
** be found with <column> as its left-most column.
**
** When the b-tree is being used for membership tests, the calling function
** needs to know whether or not the structure contains an SQL NULL 
** value in order to correctly evaluate expressions like "X IN (Y, Z)".
** If there is any chance that the (...) might contain a NULL value at
** runtime, then a register is allocated and the register number written
** to *prNotFound. If there is no chance that the (...) contains a
** NULL value, then *prNotFound is left unchanged.
**
** If a register is allocated and its location stored in *prNotFound, then
** its initial value is NULL.  If the (...) does not remain constant
** for the duration of the query (i.e. the SELECT within the (...)
** is a correlated subquery) then the value of the allocated register is
** reset to NULL each time the subquery is rerun. This allows the
** caller to use vdbe code equivalent to the following:
**
**   if( register==NULL ){
**     has_null = <test if data structure contains null>
**     register = 1
**   }
**
** in order to avoid running the <test if data structure contains null>
** test more often than is necessary.
*/
#ifndef SQLITE_OMIT_SUBQUERY
SQLITE_PRIVATE int sqlite3FindInIndex(Parse *pParse, Expr *pX, int *prNotFound){
  Select *p;                            /* SELECT to the right of IN operator */
  int eType = 0;                        /* Type of RHS table. IN_INDEX_* */
  int iTab = pParse->nTab++;            /* Cursor of the RHS table */
  int mustBeUnique = (prNotFound==0);   /* True if RHS must be unique */

  assert( pX->op==TK_IN );

  /* Check to see if an existing table or index can be used to
  ** satisfy the query.  This is preferable to generating a new 
  ** ephemeral table.
  */
  p = (ExprHasProperty(pX, EP_xIsSelect) ? pX->x.pSelect : 0);
  if( ALWAYS(pParse->nErr==0) && isCandidateForInOpt(p) ){
    sqlite3 *db = pParse->db;              /* Database connection */
    Expr *pExpr = p->pEList->a[0].pExpr;   /* Expression <column> */
    int iCol = pExpr->iColumn;             /* Index of column <column> */
    Vdbe *v = sqlite3GetVdbe(pParse);      /* Virtual machine being coded */
    Table *pTab = p->pSrc->a[0].pTab;      /* Table <table>. */
    int iDb;                               /* Database idx for pTab */
   
    /* Code an OP_VerifyCookie and OP_TableLock for <table>. */
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    /* This function is only called from two places. In both cases the vdbe
    ** has already been allocated. So assume sqlite3GetVdbe() is always
    ** successful here.
    */
    assert(v);
    if( iCol<0 ){
      int iMem = ++pParse->nMem;
      int iAddr;

      iAddr = sqlite3VdbeAddOp1(v, OP_If, iMem);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, iMem);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
      eType = IN_INDEX_ROWID;

      sqlite3VdbeJumpHere(v, iAddr);
    }else{
      Index *pIdx;                         /* Iterator variable */

      /* The collation sequence used by the comparison. If an index is to
      ** be used in place of a temp-table, it must be ordered according
      ** to this collation sequence.  */
      CollSeq *pReq = sqlite3BinaryCompareCollSeq(pParse, pX->pLeft, pExpr);

      /* Check that the affinity that will be used to perform the 
      ** comparison is the same as the affinity of the column. If
      ** it is not, it is not possible to use any index.
      */
      char aff = comparisonAffinity(pX);
      int affinity_ok = (pTab->aCol[iCol].affinity==aff||aff==SQLITE_AFF_NONE);

      for(pIdx=pTab->pIndex; pIdx && eType==0 && affinity_ok; pIdx=pIdx->pNext){
        if( (pIdx->aiColumn[0]==iCol)
         && sqlite3FindCollSeq(db, ENC(db), pIdx->azColl[0], 0)==pReq
         && (!mustBeUnique || (pIdx->nColumn==1 && pIdx->onError!=OE_None))
        ){
          int iMem = ++pParse->nMem;
          int iAddr;
          char *pKey;
  
          pKey = (char *)sqlite3IndexKeyinfo(pParse, pIdx);
          iAddr = sqlite3VdbeAddOp1(v, OP_If, iMem);
          sqlite3VdbeAddOp2(v, OP_Integer, 1, iMem);
  
          sqlite3VdbeAddOp4(v, OP_OpenRead, iTab, pIdx->tnum, iDb,
                               pKey,P4_KEYINFO_HANDOFF);
          VdbeComment((v, "%s", pIdx->zName));
          eType = IN_INDEX_INDEX;

          sqlite3VdbeJumpHere(v, iAddr);
          if( prNotFound && !pTab->aCol[iCol].notNull ){
            *prNotFound = ++pParse->nMem;
          }
        }
      }
    }
  }

  if( eType==0 ){
    /* Could not found an existing table or index to use as the RHS b-tree.
    ** We will have to generate an ephemeral table to do the job.
    */
    double savedNQueryLoop = pParse->nQueryLoop;
    int rMayHaveNull = 0;
    eType = IN_INDEX_EPH;
    if( prNotFound ){
      *prNotFound = rMayHaveNull = ++pParse->nMem;
    }else{
      testcase( pParse->nQueryLoop>(double)1 );
      pParse->nQueryLoop = (double)1;
      if( pX->pLeft->iColumn<0 && !ExprHasAnyProperty(pX, EP_xIsSelect) ){
        eType = IN_INDEX_ROWID;
      }
    }
    sqlite3CodeSubselect(pParse, pX, rMayHaveNull, eType==IN_INDEX_ROWID);
    pParse->nQueryLoop = savedNQueryLoop;
  }else{
    pX->iTable = iTab;
  }
  return eType;
}
#endif

/*
** Generate code for scalar subqueries used as a subquery expression, EXISTS,
** or IN operators.  Examples:
**
**     (SELECT a FROM b)          -- subquery
**     EXISTS (SELECT a FROM b)   -- EXISTS subquery
**     x IN (4,5,11)              -- IN operator with list on right-hand side
**     x IN (SELECT a FROM b)     -- IN operator with subquery on the right
**
** The pExpr parameter describes the expression that contains the IN
** operator or subquery.
**
** If parameter isRowid is non-zero, then expression pExpr is guaranteed
** to be of the form "<rowid> IN (?, ?, ?)", where <rowid> is a reference
** to some integer key column of a table B-Tree. In this case, use an
** intkey B-Tree to store the set of IN(...) values instead of the usual
** (slower) variable length keys B-Tree.
**
** If rMayHaveNull is non-zero, that means that the operation is an IN
** (not a SELECT or EXISTS) and that the RHS might contains NULLs.
** Furthermore, the IN is in a WHERE clause and that we really want
** to iterate over the RHS of the IN operator in order to quickly locate
** all corresponding LHS elements.  All this routine does is initialize
** the register given by rMayHaveNull to NULL.  Calling routines will take
** care of changing this register value to non-NULL if the RHS is NULL-free.
**
** If rMayHaveNull is zero, that means that the subquery is being used
** for membership testing only.  There is no need to initialize any
** registers to indicate the presense or absence of NULLs on the RHS.
**
** For a SELECT or EXISTS operator, return the register that holds the
** result.  For IN operators or if an error occurs, the return value is 0.
*/
#ifndef SQLITE_OMIT_SUBQUERY
SQLITE_PRIVATE int sqlite3CodeSubselect(
  Parse *pParse,          /* Parsing context */
  Expr *pExpr,            /* The IN, SELECT, or EXISTS operator */
  int rMayHaveNull,       /* Register that records whether NULLs exist in RHS */
  int isRowid             /* If true, LHS of IN operator is a rowid */
){
  int testAddr = 0;                       /* One-time test address */
  int rReg = 0;                           /* Register storing resulting */
  Vdbe *v = sqlite3GetVdbe(pParse);
  if( NEVER(v==0) ) return 0;
  sqlite3ExprCachePush(pParse);

  /* This code must be run in its entirety every time it is encountered
  ** if any of the following is true:
  **
  **    *  The right-hand side is a correlated subquery
  **    *  The right-hand side is an expression list containing variables
  **    *  We are inside a trigger
  **
  ** If all of the above are false, then we can run this code just once
  ** save the results, and reuse the same result on subsequent invocations.
  */
  if( !ExprHasAnyProperty(pExpr, EP_VarSelect) && !pParse->pTriggerTab ){
    int mem = ++pParse->nMem;
    sqlite3VdbeAddOp1(v, OP_If, mem);
    testAddr = sqlite3VdbeAddOp2(v, OP_Integer, 1, mem);
    assert( testAddr>0 || pParse->db->mallocFailed );
  }

#ifndef SQLITE_OMIT_EXPLAIN
  if( pParse->explain==2 ){
    char *zMsg = sqlite3MPrintf(
        pParse->db, "EXECUTE %s%s SUBQUERY %d", testAddr?"":"CORRELATED ",
        pExpr->op==TK_IN?"LIST":"SCALAR", pParse->iNextSelectId
    );
    sqlite3VdbeAddOp4(v, OP_Explain, pParse->iSelectId, 0, 0, zMsg, P4_DYNAMIC);
  }
#endif

  switch( pExpr->op ){
    case TK_IN: {
      char affinity;              /* Affinity of the LHS of the IN */
      KeyInfo keyInfo;            /* Keyinfo for the generated table */
      int addr;                   /* Address of OP_OpenEphemeral instruction */
      Expr *pLeft = pExpr->pLeft; /* the LHS of the IN operator */

      if( rMayHaveNull ){
        sqlite3VdbeAddOp2(v, OP_Null, 0, rMayHaveNull);
      }

      affinity = sqlite3ExprAffinity(pLeft);

      /* Whether this is an 'x IN(SELECT...)' or an 'x IN(<exprlist>)'
      ** expression it is handled the same way.  An ephemeral table is 
      ** filled with single-field index keys representing the results
      ** from the SELECT or the <exprlist>.
      **
      ** If the 'x' expression is a column value, or the SELECT...
      ** statement returns a column value, then the affinity of that
      ** column is used to build the index keys. If both 'x' and the
      ** SELECT... statement are columns, then numeric affinity is used
      ** if either column has NUMERIC or INTEGER affinity. If neither
      ** 'x' nor the SELECT... statement are columns, then numeric affinity
      ** is used.
      */
      pExpr->iTable = pParse->nTab++;
      addr = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pExpr->iTable, !isRowid);
      if( rMayHaveNull==0 ) sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
      memset(&keyInfo, 0, sizeof(keyInfo));
      keyInfo.nField = 1;

      if( ExprHasProperty(pExpr, EP_xIsSelect) ){
        /* Case 1:     expr IN (SELECT ...)
        **
        ** Generate code to write the results of the select into the temporary
        ** table allocated and opened above.
        */
        SelectDest dest;
        ExprList *pEList;

        assert( !isRowid );
        sqlite3SelectDestInit(&dest, SRT_Set, pExpr->iTable);
        dest.affinity = (u8)affinity;
        assert( (pExpr->iTable&0x0000FFFF)==pExpr->iTable );
        pExpr->x.pSelect->iLimit = 0;
        if( sqlite3Select(pParse, pExpr->x.pSelect, &dest) ){
          return 0;
        }
        pEList = pExpr->x.pSelect->pEList;
        if( ALWAYS(pEList!=0 && pEList->nExpr>0) ){ 
          keyInfo.aColl[0] = sqlite3BinaryCompareCollSeq(pParse, pExpr->pLeft,
              pEList->a[0].pExpr);
        }
      }else if( ALWAYS(pExpr->x.pList!=0) ){
        /* Case 2:     expr IN (exprlist)
        **
        ** For each expression, build an index key from the evaluation and
        ** store it in the temporary table. If <expr> is a column, then use
        ** that columns affinity when building index keys. If <expr> is not
        ** a column, use numeric affinity.
        */
        int i;
        ExprList *pList = pExpr->x.pList;
        struct ExprList_item *pItem;
        int r1, r2, r3;

        if( !affinity ){
          affinity = SQLITE_AFF_NONE;
        }
        keyInfo.aColl[0] = sqlite3ExprCollSeq(pParse, pExpr->pLeft);

        /* Loop through each expression in <exprlist>. */
        r1 = sqlite3GetTempReg(pParse);
        r2 = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp2(v, OP_Null, 0, r2);
        for(i=pList->nExpr, pItem=pList->a; i>0; i--, pItem++){
          Expr *pE2 = pItem->pExpr;
          int iValToIns;

          /* If the expression is not constant then we will need to
          ** disable the test that was generated above that makes sure
          ** this code only executes once.  Because for a non-constant
          ** expression we need to rerun this code each time.
          */
          if( testAddr && !sqlite3ExprIsConstant(pE2) ){
            sqlite3VdbeChangeToNoop(v, testAddr-1, 2);
            testAddr = 0;
          }

          /* Evaluate the expression and insert it into the temp table */
          if( isRowid && sqlite3ExprIsInteger(pE2, &iValToIns) ){
            sqlite3VdbeAddOp3(v, OP_InsertInt, pExpr->iTable, r2, iValToIns);
          }else{
            r3 = sqlite3ExprCodeTarget(pParse, pE2, r1);
            if( isRowid ){
              sqlite3VdbeAddOp2(v, OP_MustBeInt, r3,
                                sqlite3VdbeCurrentAddr(v)+2);
              sqlite3VdbeAddOp3(v, OP_Insert, pExpr->iTable, r2, r3);
            }else{
              sqlite3VdbeAddOp4(v, OP_MakeRecord, r3, 1, r2, &affinity, 1);
              sqlite3ExprCacheAffinityChange(pParse, r3, 1);
              sqlite3VdbeAddOp2(v, OP_IdxInsert, pExpr->iTable, r2);
            }
          }
        }
        sqlite3ReleaseTempReg(pParse, r1);
        sqlite3ReleaseTempReg(pParse, r2);
      }
      if( !isRowid ){
        sqlite3VdbeChangeP4(v, addr, (void *)&keyInfo, P4_KEYINFO);
      }
      break;
    }

    case TK_EXISTS:
    case TK_SELECT:
    default: {
      /* If this has to be a scalar SELECT.  Generate code to put the
      ** value of this select in a memory cell and record the number
      ** of the memory cell in iColumn.  If this is an EXISTS, write
      ** an integer 0 (not exists) or 1 (exists) into a memory cell
      ** and record that memory cell in iColumn.
      */
      Select *pSel;                         /* SELECT statement to encode */
      SelectDest dest;                      /* How to deal with SELECt result */

      testcase( pExpr->op==TK_EXISTS );
      testcase( pExpr->op==TK_SELECT );
      assert( pExpr->op==TK_EXISTS || pExpr->op==TK_SELECT );

      assert( ExprHasProperty(pExpr, EP_xIsSelect) );
      pSel = pExpr->x.pSelect;
      sqlite3SelectDestInit(&dest, 0, ++pParse->nMem);
      if( pExpr->op==TK_SELECT ){
        dest.eDest = SRT_Mem;
        sqlite3VdbeAddOp2(v, OP_Null, 0, dest.iParm);
        VdbeComment((v, "Init subquery result"));
      }else{
        dest.eDest = SRT_Exists;
        sqlite3VdbeAddOp2(v, OP_Integer, 0, dest.iParm);
        VdbeComment((v, "Init EXISTS result"));
      }
      sqlite3ExprDelete(pParse->db, pSel->pLimit);
      pSel->pLimit = sqlite3PExpr(pParse, TK_INTEGER, 0, 0,
                                  &sqlite3IntTokens[1]);
      pSel->iLimit = 0;
      if( sqlite3Select(pParse, pSel, &dest) ){
        return 0;
      }
      rReg = dest.iParm;
      ExprSetIrreducible(pExpr);
      break;
    }
  }

  if( testAddr ){
    sqlite3VdbeJumpHere(v, testAddr-1);
  }
  sqlite3ExprCachePop(pParse, 1);

  return rReg;
}
#endif /* SQLITE_OMIT_SUBQUERY */

#ifndef SQLITE_OMIT_SUBQUERY
/*
** Generate code for an IN expression.
**
**      x IN (SELECT ...)
**      x IN (value, value, ...)
**
** The left-hand side (LHS) is a scalar expression.  The right-hand side (RHS)
** is an array of zero or more values.  The expression is true if the LHS is
** contained within the RHS.  The value of the expression is unknown (NULL)
** if the LHS is NULL or if the LHS is not contained within the RHS and the
** RHS contains one or more NULL values.
**
** This routine generates code will jump to destIfFalse if the LHS is not 
** contained within the RHS.  If due to NULLs we cannot determine if the LHS
** is contained in the RHS then jump to destIfNull.  If the LHS is contained
** within the RHS then fall through.
*/
static void sqlite3ExprCodeIN(
  Parse *pParse,        /* Parsing and code generating context */
  Expr *pExpr,          /* The IN expression */
  int destIfFalse,      /* Jump here if LHS is not contained in the RHS */
  int destIfNull        /* Jump here if the results are unknown due to NULLs */
){
  int rRhsHasNull = 0;  /* Register that is true if RHS contains NULL values */
  char affinity;        /* Comparison affinity to use */
  int eType;            /* Type of the RHS */
  int r1;               /* Temporary use register */
  Vdbe *v;              /* Statement under construction */

  /* Compute the RHS.   After this step, the table with cursor
  ** pExpr->iTable will contains the values that make up the RHS.
  */
  v = pParse->pVdbe;
  assert( v!=0 );       /* OOM detected prior to this routine */
  VdbeNoopComment((v, "begin IN expr"));
  eType = sqlite3FindInIndex(pParse, pExpr, &rRhsHasNull);

  /* Figure out the affinity to use to create a key from the results
  ** of the expression. affinityStr stores a static string suitable for
  ** P4 of OP_MakeRecord.
  */
  affinity = comparisonAffinity(pExpr);

  /* Code the LHS, the <expr> from "<expr> IN (...)".
  */
  sqlite3ExprCachePush(pParse);
  r1 = sqlite3GetTempReg(pParse);
  sqlite3ExprCode(pParse, pExpr->pLeft, r1);

  /* If the LHS is NULL, then the result is either false or NULL depending
  ** on whether the RHS is empty or not, respectively.
  */
  if( destIfNull==destIfFalse ){
    /* Shortcut for the common case where the false and NULL outcomes are
    ** the same. */
    sqlite3VdbeAddOp2(v, OP_IsNull, r1, destIfNull);
  }else{
    int addr1 = sqlite3VdbeAddOp1(v, OP_NotNull, r1);
    sqlite3VdbeAddOp2(v, OP_Rewind, pExpr->iTable, destIfFalse);
    sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfNull);
    sqlite3VdbeJumpHere(v, addr1);
  }

  if( eType==IN_INDEX_ROWID ){
    /* In this case, the RHS is the ROWID of table b-tree
    */
    sqlite3VdbeAddOp2(v, OP_MustBeInt, r1, destIfFalse);
    sqlite3VdbeAddOp3(v, OP_NotExists, pExpr->iTable, destIfFalse, r1);
  }else{
    /* In this case, the RHS is an index b-tree.
    */
    sqlite3VdbeAddOp4(v, OP_Affinity, r1, 1, 0, &affinity, 1);

    /* If the set membership test fails, then the result of the 
    ** "x IN (...)" expression must be either 0 or NULL. If the set
    ** contains no NULL values, then the result is 0. If the set 
    ** contains one or more NULL values, then the result of the
    ** expression is also NULL.
    */
    if( rRhsHasNull==0 || destIfFalse==destIfNull ){
      /* This branch runs if it is known at compile time that the RHS
      ** cannot contain NULL values. This happens as the result
      ** of a "NOT NULL" constraint in the database schema.
      **
      ** Also run this branch if NULL is equivalent to FALSE
      ** for this particular IN operator.
      */
      sqlite3VdbeAddOp4Int(v, OP_NotFound, pExpr->iTable, destIfFalse, r1, 1);

    }else{
      /* In this branch, the RHS of the IN might contain a NULL and
      ** the presence of a NULL on the RHS makes a difference in the
      ** outcome.
      */
      int j1, j2, j3;

      /* First check to see if the LHS is contained in the RHS.  If so,
      ** then the presence of NULLs in the RHS does not matter, so jump
      ** over all of the code that follows.
      */
      j1 = sqlite3VdbeAddOp4Int(v, OP_Found, pExpr->iTable, 0, r1, 1);

      /* Here we begin generating code that runs if the LHS is not
      ** contained within the RHS.  Generate additional code that
      ** tests the RHS for NULLs.  If the RHS contains a NULL then
      ** jump to destIfNull.  If there are no NULLs in the RHS then
      ** jump to destIfFalse.
      */
      j2 = sqlite3VdbeAddOp1(v, OP_NotNull, rRhsHasNull);
      j3 = sqlite3VdbeAddOp4Int(v, OP_Found, pExpr->iTable, 0, rRhsHasNull, 1);
      sqlite3VdbeAddOp2(v, OP_Integer, -1, rRhsHasNull);
      sqlite3VdbeJumpHere(v, j3);
      sqlite3VdbeAddOp2(v, OP_AddImm, rRhsHasNull, 1);
      sqlite3VdbeJumpHere(v, j2);

      /* Jump to the appropriate target depending on whether or not
      ** the RHS contains a NULL
      */
      sqlite3VdbeAddOp2(v, OP_If, rRhsHasNull, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Goto, 0, destIfFalse);

      /* The OP_Found at the top of this branch jumps here when true, 
      ** causing the overall IN expression evaluation to fall through.
      */
      sqlite3VdbeJumpHere(v, j1);
    }
  }
  sqlite3ReleaseTempReg(pParse, r1);
  sqlite3ExprCachePop(pParse, 1);
  VdbeComment((v, "end IN expr"));
}
#endif /* SQLITE_OMIT_SUBQUERY */

/*
** Duplicate an 8-byte value
*/
static char *dup8bytes(Vdbe *v, const char *in){
  char *out = sqlite3DbMallocRaw(sqlite3VdbeDb(v), 8);
  if( out ){
    memcpy(out, in, 8);
  }
  return out;
}

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Generate an instruction that will put the floating point
** value described by z[0..n-1] into register iMem.
**
** The z[] string will probably not be zero-terminated.  But the 
** z[n] character is guaranteed to be something that does not look
** like the continuation of the number.
*/
static void codeReal(Vdbe *v, const char *z, int negateFlag, int iMem){
  if( ALWAYS(z!=0) ){
    double value;
    char *zV;
    sqlite3AtoF(z, &value, sqlite3Strlen30(z), SQLITE_UTF8);
    assert( !sqlite3IsNaN(value) ); /* The new AtoF never returns NaN */
    if( negateFlag ) value = -value;
    zV = dup8bytes(v, (char*)&value);
    sqlite3VdbeAddOp4(v, OP_Real, 0, iMem, 0, zV, P4_REAL);
  }
}
#endif


/*
** Generate an instruction that will put the integer describe by
** text z[0..n-1] into register iMem.
**
** Expr.u.zToken is always UTF8 and zero-terminated.
*/
static void codeInteger(Parse *pParse, Expr *pExpr, int negFlag, int iMem){
  Vdbe *v = pParse->pVdbe;
  if( pExpr->flags & EP_IntValue ){
    int i = pExpr->u.iValue;
    assert( i>=0 );
    if( negFlag ) i = -i;
    sqlite3VdbeAddOp2(v, OP_Integer, i, iMem);
  }else{
    int c;
    i64 value;
    const char *z = pExpr->u.zToken;
    assert( z!=0 );
    c = sqlite3Atoi64(z, &value, sqlite3Strlen30(z), SQLITE_UTF8);
    if( c==0 || (c==2 && negFlag) ){
      char *zV;
      if( negFlag ){ value = c==2 ? SMALLEST_INT64 : -value; }
      zV = dup8bytes(v, (char*)&value);
      sqlite3VdbeAddOp4(v, OP_Int64, 0, iMem, 0, zV, P4_INT64);
    }else{
#ifdef SQLITE_OMIT_FLOATING_POINT
      sqlite3ErrorMsg(pParse, "oversized integer: %s%s", negFlag ? "-" : "", z);
#else
      codeReal(v, z, negFlag, iMem);
#endif
    }
  }
}

/*
** Clear a cache entry.
*/
static void cacheEntryClear(Parse *pParse, struct yColCache *p){
  if( p->tempReg ){
    if( pParse->nTempReg<ArraySize(pParse->aTempReg) ){
      pParse->aTempReg[pParse->nTempReg++] = p->iReg;
    }
    p->tempReg = 0;
  }
}


/*
** Record in the column cache that a particular column from a
** particular table is stored in a particular register.
*/
SQLITE_PRIVATE void sqlite3ExprCacheStore(Parse *pParse, int iTab, int iCol, int iReg){
  int i;
  int minLru;
  int idxLru;
  struct yColCache *p;

  assert( iReg>0 );  /* Register numbers are always positive */
  assert( iCol>=-1 && iCol<32768 );  /* Finite column numbers */

  /* The SQLITE_ColumnCache flag disables the column cache.  This is used
  ** for testing only - to verify that SQLite always gets the same answer
  ** with and without the column cache.
  */
  if( pParse->db->flags & SQLITE_ColumnCache ) return;

  /* First replace any existing entry.
  **
  ** Actually, the way the column cache is currently used, we are guaranteed
  ** that the object will never already be in cache.  Verify this guarantee.
  */
#ifndef NDEBUG
  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
#if 0 /* This code wold remove the entry from the cache if it existed */
    if( p->iReg && p->iTable==iTab && p->iColumn==iCol ){
      cacheEntryClear(pParse, p);
      p->iLevel = pParse->iCacheLevel;
      p->iReg = iReg;
      p->lru = pParse->iCacheCnt++;
      return;
    }
#endif
    assert( p->iReg==0 || p->iTable!=iTab || p->iColumn!=iCol );
  }
#endif

  /* Find an empty slot and replace it */
  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
    if( p->iReg==0 ){
      p->iLevel = pParse->iCacheLevel;
      p->iTable = iTab;
      p->iColumn = iCol;
      p->iReg = iReg;
      p->tempReg = 0;
      p->lru = pParse->iCacheCnt++;
      return;
    }
  }

  /* Replace the last recently used */
  minLru = 0x7fffffff;
  idxLru = -1;
  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
    if( p->lru<minLru ){
      idxLru = i;
      minLru = p->lru;
    }
  }
  if( ALWAYS(idxLru>=0) ){
    p = &pParse->aColCache[idxLru];
    p->iLevel = pParse->iCacheLevel;
    p->iTable = iTab;
    p->iColumn = iCol;
    p->iReg = iReg;
    p->tempReg = 0;
    p->lru = pParse->iCacheCnt++;
    return;
  }
}

/*
** Indicate that registers between iReg..iReg+nReg-1 are being overwritten.
** Purge the range of registers from the column cache.
*/
SQLITE_PRIVATE void sqlite3ExprCacheRemove(Parse *pParse, int iReg, int nReg){
  int i;
  int iLast = iReg + nReg - 1;
  struct yColCache *p;
  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
    int r = p->iReg;
    if( r>=iReg && r<=iLast ){
      cacheEntryClear(pParse, p);
      p->iReg = 0;
    }
  }
}

/*
** Remember the current column cache context.  Any new entries added
** added to the column cache after this call are removed when the
** corresponding pop occurs.
*/
SQLITE_PRIVATE void sqlite3ExprCachePush(Parse *pParse){
  pParse->iCacheLevel++;
}

/*
** Remove from the column cache any entries that were added since the
** the previous N Push operations.  In other words, restore the cache
** to the state it was in N Pushes ago.
*/
SQLITE_PRIVATE void sqlite3ExprCachePop(Parse *pParse, int N){
  int i;
  struct yColCache *p;
  assert( N>0 );
  assert( pParse->iCacheLevel>=N );
  pParse->iCacheLevel -= N;
  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
    if( p->iReg && p->iLevel>pParse->iCacheLevel ){
      cacheEntryClear(pParse, p);
      p->iReg = 0;
    }
  }
}

/*
** When a cached column is reused, make sure that its register is
** no longer available as a temp register.  ticket #3879:  that same
** register might be in the cache in multiple places, so be sure to
** get them all.
*/
static void sqlite3ExprCachePinRegister(Parse *pParse, int iReg){
  int i;
  struct yColCache *p;
  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
    if( p->iReg==iReg ){
      p->tempReg = 0;
    }
  }
}

/*
** Generate code to extract the value of the iCol-th column of a table.
*/
SQLITE_PRIVATE void sqlite3ExprCodeGetColumnOfTable(
  Vdbe *v,        /* The VDBE under construction */
  Table *pTab,    /* The table containing the value */
  int iTabCur,    /* The cursor for this table */
  int iCol,       /* Index of the column to extract */
  int regOut      /* Extract the valud into this register */
){
  if( iCol<0 || iCol==pTab->iPKey ){
    sqlite3VdbeAddOp2(v, OP_Rowid, iTabCur, regOut);
  }else{
    int op = IsVirtual(pTab) ? OP_VColumn : OP_Column;
    sqlite3VdbeAddOp3(v, op, iTabCur, iCol, regOut);
  }
  if( iCol>=0 ){
    sqlite3ColumnDefault(v, pTab, iCol, regOut);
  }
}

/*
** Generate code that will extract the iColumn-th column from
** table pTab and store the column value in a register.  An effort
** is made to store the column value in register iReg, but this is
** not guaranteed.  The location of the column value is returned.
**
** There must be an open cursor to pTab in iTable when this routine
** is called.  If iColumn<0 then code is generated that extracts the rowid.
*/
SQLITE_PRIVATE int sqlite3ExprCodeGetColumn(
  Parse *pParse,   /* Parsing and code generating context */
  Table *pTab,     /* Description of the table we are reading from */
  int iColumn,     /* Index of the table column */
  int iTable,      /* The cursor pointing to the table */
  int iReg         /* Store results here */
){
  Vdbe *v = pParse->pVdbe;
  int i;
  struct yColCache *p;

  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
    if( p->iReg>0 && p->iTable==iTable && p->iColumn==iColumn ){
      p->lru = pParse->iCacheCnt++;
      sqlite3ExprCachePinRegister(pParse, p->iReg);
      return p->iReg;
    }
  }  
  assert( v!=0 );
  sqlite3ExprCodeGetColumnOfTable(v, pTab, iTable, iColumn, iReg);
  sqlite3ExprCacheStore(pParse, iTable, iColumn, iReg);
  return iReg;
}

/*
** Clear all column cache entries.
*/
SQLITE_PRIVATE void sqlite3ExprCacheClear(Parse *pParse){
  int i;
  struct yColCache *p;

  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
    if( p->iReg ){
      cacheEntryClear(pParse, p);
      p->iReg = 0;
    }
  }
}

/*
** Record the fact that an affinity change has occurred on iCount
** registers starting with iStart.
*/
SQLITE_PRIVATE void sqlite3ExprCacheAffinityChange(Parse *pParse, int iStart, int iCount){
  sqlite3ExprCacheRemove(pParse, iStart, iCount);
}

/*
** Generate code to move content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1. Keep the column cache up-to-date.
*/
SQLITE_PRIVATE void sqlite3ExprCodeMove(Parse *pParse, int iFrom, int iTo, int nReg){
  int i;
  struct yColCache *p;
  if( NEVER(iFrom==iTo) ) return;
  sqlite3VdbeAddOp3(pParse->pVdbe, OP_Move, iFrom, iTo, nReg);
  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
    int x = p->iReg;
    if( x>=iFrom && x<iFrom+nReg ){
      p->iReg += iTo-iFrom;
    }
  }
}

/*
** Generate code to copy content from registers iFrom...iFrom+nReg-1
** over to iTo..iTo+nReg-1.
*/
SQLITE_PRIVATE void sqlite3ExprCodeCopy(Parse *pParse, int iFrom, int iTo, int nReg){
  int i;
  if( NEVER(iFrom==iTo) ) return;
  for(i=0; i<nReg; i++){
    sqlite3VdbeAddOp2(pParse->pVdbe, OP_Copy, iFrom+i, iTo+i);
  }
}

#if defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
/*
** Return true if any register in the range iFrom..iTo (inclusive)
** is used as part of the column cache.
**
** This routine is used within assert() and testcase() macros only
** and does not appear in a normal build.
*/
static int usedAsColumnCache(Parse *pParse, int iFrom, int iTo){
  int i;
  struct yColCache *p;
  for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
    int r = p->iReg;
    if( r>=iFrom && r<=iTo ) return 1;    /*NO_TEST*/
  }
  return 0;
}
#endif /* SQLITE_DEBUG || SQLITE_COVERAGE_TEST */

/*
** Generate code into the current Vdbe to evaluate the given
** expression.  Attempt to store the results in register "target".
** Return the register where results are stored.
**
** With this routine, there is no guarantee that results will
** be stored in target.  The result might be stored in some other
** register if it is convenient to do so.  The calling function
** must check the return code and move the results to the desired
** register.
*/
SQLITE_PRIVATE int sqlite3ExprCodeTarget(Parse *pParse, Expr *pExpr, int target){
  Vdbe *v = pParse->pVdbe;  /* The VM under construction */
  int op;                   /* The opcode being coded */
  int inReg = target;       /* Results stored in register inReg */
  int regFree1 = 0;         /* If non-zero free this temporary register */
  int regFree2 = 0;         /* If non-zero free this temporary register */
  int r1, r2, r3, r4;       /* Various register numbers */
  sqlite3 *db = pParse->db; /* The database connection */

  assert( target>0 && target<=pParse->nMem );
  if( v==0 ){
    assert( pParse->db->mallocFailed );
    return 0;
  }

  if( pExpr==0 ){
    op = TK_NULL;
  }else{
    op = pExpr->op;
  }
  switch( op ){
    case TK_AGG_COLUMN: {
      AggInfo *pAggInfo = pExpr->pAggInfo;
      struct AggInfo_col *pCol = &pAggInfo->aCol[pExpr->iAgg];
      if( !pAggInfo->directMode ){
        assert( pCol->iMem>0 );
        inReg = pCol->iMem;
        break;
      }else if( pAggInfo->useSortingIdx ){
        sqlite3VdbeAddOp3(v, OP_Column, pAggInfo->sortingIdx,
                              pCol->iSorterColumn, target);
        break;
      }
      /* Otherwise, fall thru into the TK_COLUMN case */
    }
    case TK_COLUMN: {
      if( pExpr->iTable<0 ){
        /* This only happens when coding check constraints */
        assert( pParse->ckBase>0 );
        inReg = pExpr->iColumn + pParse->ckBase;
      }else{
        inReg = sqlite3ExprCodeGetColumn(pParse, pExpr->pTab,
                                 pExpr->iColumn, pExpr->iTable, target);
      }
      break;
    }
    case TK_INTEGER: {
      codeInteger(pParse, pExpr, 0, target);
      break;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    case TK_FLOAT: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      codeReal(v, pExpr->u.zToken, 0, target);
      break;
    }
#endif
    case TK_STRING: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3VdbeAddOp4(v, OP_String8, 0, target, 0, pExpr->u.zToken, 0);
      break;
    }
    case TK_NULL: {
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      break;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    case TK_BLOB: {
      int n;
      const char *z;
      char *zBlob;
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      assert( pExpr->u.zToken[0]=='x' || pExpr->u.zToken[0]=='X' );
      assert( pExpr->u.zToken[1]=='\'' );
      z = &pExpr->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;
      assert( z[n]=='\'' );
      zBlob = sqlite3HexToBlob(sqlite3VdbeDb(v), z, n);
      sqlite3VdbeAddOp4(v, OP_Blob, n/2, target, 0, zBlob, P4_DYNAMIC);
      break;
    }
#endif
    case TK_VARIABLE: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      assert( pExpr->u.zToken!=0 );
      assert( pExpr->u.zToken[0]!=0 );
      sqlite3VdbeAddOp2(v, OP_Variable, pExpr->iColumn, target);
      if( pExpr->u.zToken[1]!=0 ){
        assert( pExpr->u.zToken[0]=='?' 
             || strcmp(pExpr->u.zToken, pParse->azVar[pExpr->iColumn-1])==0 );
        sqlite3VdbeChangeP4(v, -1, pParse->azVar[pExpr->iColumn-1], P4_STATIC);
      }
      break;
    }
    case TK_REGISTER: {
      inReg = pExpr->iTable;
      break;
    }
    case TK_AS: {
      inReg = sqlite3ExprCodeTarget(pParse, pExpr->pLeft, target);
      break;
    }
#ifndef SQLITE_OMIT_CAST
    case TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      int aff, to_op;
      inReg = sqlite3ExprCodeTarget(pParse, pExpr->pLeft, target);
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      aff = sqlite3AffinityType(pExpr->u.zToken);
      to_op = aff - SQLITE_AFF_TEXT + OP_ToText;
      assert( to_op==OP_ToText    || aff!=SQLITE_AFF_TEXT    );
      assert( to_op==OP_ToBlob    || aff!=SQLITE_AFF_NONE    );
      assert( to_op==OP_ToNumeric || aff!=SQLITE_AFF_NUMERIC );
      assert( to_op==OP_ToInt     || aff!=SQLITE_AFF_INTEGER );
      assert( to_op==OP_ToReal    || aff!=SQLITE_AFF_REAL    );
      testcase( to_op==OP_ToText );
      testcase( to_op==OP_ToBlob );
      testcase( to_op==OP_ToNumeric );
      testcase( to_op==OP_ToInt );
      testcase( to_op==OP_ToReal );
      if( inReg!=target ){
        sqlite3VdbeAddOp2(v, OP_SCopy, inReg, target);
        inReg = target;
      }
      sqlite3VdbeAddOp1(v, to_op, inReg);
      testcase( usedAsColumnCache(pParse, inReg, inReg) );
      sqlite3ExprCacheAffinityChange(pParse, inReg, 1);
      break;
    }
#endif /* SQLITE_OMIT_CAST */
    case TK_LT:
    case TK_LE:
    case TK_GT:
    case TK_GE:
    case TK_NE:
    case TK_EQ: {
      assert( TK_LT==OP_Lt );
      assert( TK_LE==OP_Le );
      assert( TK_GT==OP_Gt );
      assert( TK_GE==OP_Ge );
      assert( TK_EQ==OP_Eq );
      assert( TK_NE==OP_Ne );
      testcase( op==TK_LT );
      testcase( op==TK_LE );
      testcase( op==TK_GT );
      testcase( op==TK_GE );
      testcase( op==TK_EQ );
      testcase( op==TK_NE );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, inReg, SQLITE_STOREP2);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      break;
    }
    case TK_IS:
    case TK_ISNOT: {
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, inReg, SQLITE_STOREP2 | SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      break;
    }
    case TK_AND:
    case TK_OR:
    case TK_PLUS:
    case TK_STAR:
    case TK_MINUS:
    case TK_REM:
    case TK_BITAND:
    case TK_BITOR:
    case TK_SLASH:
    case TK_LSHIFT:
    case TK_RSHIFT: 
    case TK_CONCAT: {
      assert( TK_AND==OP_And );
      assert( TK_OR==OP_Or );
      assert( TK_PLUS==OP_Add );
      assert( TK_MINUS==OP_Subtract );
      assert( TK_REM==OP_Remainder );
      assert( TK_BITAND==OP_BitAnd );
      assert( TK_BITOR==OP_BitOr );
      assert( TK_SLASH==OP_Divide );
      assert( TK_LSHIFT==OP_ShiftLeft );
      assert( TK_RSHIFT==OP_ShiftRight );
      assert( TK_CONCAT==OP_Concat );
      testcase( op==TK_AND );
      testcase( op==TK_OR );
      testcase( op==TK_PLUS );
      testcase( op==TK_MINUS );
      testcase( op==TK_REM );
      testcase( op==TK_BITAND );
      testcase( op==TK_BITOR );
      testcase( op==TK_SLASH );
      testcase( op==TK_LSHIFT );
      testcase( op==TK_RSHIFT );
      testcase( op==TK_CONCAT );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      break;
    }
    case TK_UMINUS: {
      Expr *pLeft = pExpr->pLeft;
      assert( pLeft );
      if( pLeft->op==TK_INTEGER ){
        codeInteger(pParse, pLeft, 1, target);
#ifndef SQLITE_OMIT_FLOATING_POINT
      }else if( pLeft->op==TK_FLOAT ){
        assert( !ExprHasProperty(pExpr, EP_IntValue) );
        codeReal(v, pLeft->u.zToken, 1, target);
#endif
      }else{
        regFree1 = r1 = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp2(v, OP_Integer, 0, r1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree2);
        sqlite3VdbeAddOp3(v, OP_Subtract, r2, r1, target);
        testcase( regFree2==0 );
      }
      inReg = target;
      break;
    }
    case TK_BITNOT:
    case TK_NOT: {
      assert( TK_BITNOT==OP_BitNot );
      assert( TK_NOT==OP_Not );
      testcase( op==TK_BITNOT );
      testcase( op==TK_NOT );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      testcase( regFree1==0 );
      inReg = target;
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      break;
    }
    case TK_ISNULL:
    case TK_NOTNULL: {
      int addr;
      assert( TK_ISNULL==OP_IsNull );
      assert( TK_NOTNULL==OP_NotNull );
      testcase( op==TK_ISNULL );
      testcase( op==TK_NOTNULL );
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      testcase( regFree1==0 );
      addr = sqlite3VdbeAddOp1(v, op, r1);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, -1);
      sqlite3VdbeJumpHere(v, addr);
      break;
    }
    case TK_AGG_FUNCTION: {
      AggInfo *pInfo = pExpr->pAggInfo;
      if( pInfo==0 ){
        assert( !ExprHasProperty(pExpr, EP_IntValue) );
        sqlite3ErrorMsg(pParse, "misuse of aggregate: %s()", pExpr->u.zToken);
      }else{
        inReg = pInfo->aFunc[pExpr->iAgg].iMem;
      }
      break;
    }
    case TK_CONST_FUNC:
    case TK_FUNCTION: {
      ExprList *pFarg;       /* List of function arguments */
      int nFarg;             /* Number of function arguments */
      FuncDef *pDef;         /* The function definition object */
      int nId;               /* Length of the function name in bytes */
      const char *zId;       /* The function name */
      int constMask = 0;     /* Mask of function arguments that are constant */
      int i;                 /* Loop counter */
      u8 enc = ENC(db);      /* The text encoding used by this database */
      CollSeq *pColl = 0;    /* A collating sequence */

      assert( !ExprHasProperty(pExpr, EP_xIsSelect) );
      testcase( op==TK_CONST_FUNC );
      testcase( op==TK_FUNCTION );
      if( ExprHasAnyProperty(pExpr, EP_TokenOnly) ){
        pFarg = 0;
      }else{
        pFarg = pExpr->x.pList;
      }
      nFarg = pFarg ? pFarg->nExpr : 0;
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      zId = pExpr->u.zToken;
      nId = sqlite3Strlen30(zId);
      pDef = sqlite3FindFunction(db, zId, nId, nFarg, enc, 0);
      if( pDef==0 ){
        sqlite3ErrorMsg(pParse, "unknown function: %.*s()", nId, zId);
        break;
      }

      /* Attempt a direct implementation of the built-in COALESCE() and
      ** IFNULL() functions.  This avoids unnecessary evalation of
      ** arguments past the first non-NULL argument.
      */
      if( pDef->flags & SQLITE_FUNC_COALESCE ){
        int endCoalesce = sqlite3VdbeMakeLabel(v);
        assert( nFarg>=2 );
        sqlite3ExprCode(pParse, pFarg->a[0].pExpr, target);
        for(i=1; i<nFarg; i++){
          sqlite3VdbeAddOp2(v, OP_NotNull, target, endCoalesce);
          sqlite3ExprCacheRemove(pParse, target, 1);
          sqlite3ExprCachePush(pParse);
          sqlite3ExprCode(pParse, pFarg->a[i].pExpr, target);
          sqlite3ExprCachePop(pParse, 1);
        }
        sqlite3VdbeResolveLabel(v, endCoalesce);
        break;
      }


      if( pFarg ){
        r1 = sqlite3GetTempRange(pParse, nFarg);
        sqlite3ExprCachePush(pParse);     /* Ticket 2ea2425d34be */
        sqlite3ExprCodeExprList(pParse, pFarg, r1, 1);
        sqlite3ExprCachePop(pParse, 1);   /* Ticket 2ea2425d34be */
      }else{
        r1 = 0;
      }
#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Possibly overload the function if the first argument is
      ** a virtual table column.
      **
      ** For infix functions (LIKE, GLOB, REGEXP, and MATCH) use the
      ** second argument, not the first, as the argument to test to
      ** see if it is a column in a virtual table.  This is done because
      ** the left operand of infix functions (the operand we want to
      ** control overloading) ends up as the second argument to the
      ** function.  The expression "A glob B" is equivalent to 
      ** "glob(B,A).  We want to use the A in "A glob B" to test
      ** for function overloading.  But we use the B term in "glob(B,A)".
      */
      if( nFarg>=2 && (pExpr->flags & EP_InfixFunc) ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].pExpr);
      }else if( nFarg>0 ){
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].pExpr);
      }
#endif
      for(i=0; i<nFarg; i++){
        if( i<32 && sqlite3ExprIsConstant(pFarg->a[i].pExpr) ){
          constMask |= (1<<i);
        }
        if( (pDef->flags & SQLITE_FUNC_NEEDCOLL)!=0 && !pColl ){
          pColl = sqlite3ExprCollSeq(pParse, pFarg->a[i].pExpr);
        }
      }
      if( pDef->flags & SQLITE_FUNC_NEEDCOLL ){
        if( !pColl ) pColl = db->pDfltColl; 
        sqlite3VdbeAddOp4(v, OP_CollSeq, 0, 0, 0, (char *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddOp4(v, OP_Function, constMask, r1, target,
                        (char*)pDef, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u8)nFarg);
      if( nFarg ){
        sqlite3ReleaseTempRange(pParse, r1, nFarg);
      }
      break;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_EXISTS:
    case TK_SELECT: {
      testcase( op==TK_EXISTS );
      testcase( op==TK_SELECT );
      inReg = sqlite3CodeSubselect(pParse, pExpr, 0, 0);
      break;
    }
    case TK_IN: {
      int destIfFalse = sqlite3VdbeMakeLabel(v);
      int destIfNull = sqlite3VdbeMakeLabel(v);
      sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      sqlite3ExprCodeIN(pParse, pExpr, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, OP_AddImm, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      break;
    }
#endif /* SQLITE_OMIT_SUBQUERY */


    /*
    **    x BETWEEN y AND z
    **
    ** This is equivalent to
    **
    **    x>=y AND x<=z
    **
    ** X is stored in pExpr->pLeft.
    ** Y is stored in pExpr->pList->a[0].pExpr.
    ** Z is stored in pExpr->pList->a[1].pExpr.
    */
    case TK_BETWEEN: {
      Expr *pLeft = pExpr->pLeft;
      struct ExprList_item *pLItem = pExpr->x.pList->a;
      Expr *pRight = pLItem->pExpr;

      r1 = sqlite3ExprCodeTemp(pParse, pLeft, &regFree1);
      r2 = sqlite3ExprCodeTemp(pParse, pRight, &regFree2);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      r3 = sqlite3GetTempReg(pParse);
      r4 = sqlite3GetTempReg(pParse);
      codeCompare(pParse, pLeft, pRight, OP_Ge,
                  r1, r2, r3, SQLITE_STOREP2);
      pLItem++;
      pRight = pLItem->pExpr;
      sqlite3ReleaseTempReg(pParse, regFree2);
      r2 = sqlite3ExprCodeTemp(pParse, pRight, &regFree2);
      testcase( regFree2==0 );
      codeCompare(pParse, pLeft, pRight, OP_Le, r1, r2, r4, SQLITE_STOREP2);
      sqlite3VdbeAddOp3(v, OP_And, r3, r4, target);
      sqlite3ReleaseTempReg(pParse, r3);
      sqlite3ReleaseTempReg(pParse, r4);
      break;
    }
    case TK_UPLUS: {
      inReg = sqlite3ExprCodeTarget(pParse, pExpr->pLeft, target);
      break;
    }

    case TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      **
      ** The expression is implemented using an OP_Param opcode. The p1
      ** parameter is set to 0 for an old.rowid reference, or to (i+1)
      ** to reference another column of the old.* pseudo-table, where 
      ** i is the index of the column. For a new.rowid reference, p1 is
      ** set to (n+1), where n is the number of columns in each pseudo-table.
      ** For a reference to any other column in the new.* pseudo-table, p1
      ** is set to (n+2+i), where n and i are as defined previously. For
      ** example, if the table on which triggers are being fired is
      ** declared as:
      **
      **   CREATE TABLE t1(a, b);
      **
      ** Then p1 is interpreted as follows:
      **
      **   p1==0   ->    old.rowid     p1==3   ->    new.rowid
      **   p1==1   ->    old.a         p1==4   ->    new.a
      **   p1==2   ->    old.b         p1==5   ->    new.b       
      */
      Table *pTab = pExpr->pTab;
      int p1 = pExpr->iTable * (pTab->nCol+1) + 1 + pExpr->iColumn;

      assert( pExpr->iTable==0 || pExpr->iTable==1 );
      assert( pExpr->iColumn>=-1 && pExpr->iColumn<pTab->nCol );
      assert( pTab->iPKey<0 || pExpr->iColumn!=pTab->iPKey );
      assert( p1>=0 && p1<(pTab->nCol*2+2) );

      sqlite3VdbeAddOp2(v, OP_Param, p1, target);
      VdbeComment((v, "%s.%s -> $%d",
        (pExpr->iTable ? "new" : "old"),
        (pExpr->iColumn<0 ? "rowid" : pExpr->pTab->aCol[pExpr->iColumn].zName),
        target
      ));

#ifndef SQLITE_OMIT_FLOATING_POINT
      /* If the column has REAL affinity, it may currently be stored as an
      ** integer. Use OP_RealAffinity to make sure it is really real.  */
      if( pExpr->iColumn>=0 
       && pTab->aCol[pExpr->iColumn].affinity==SQLITE_AFF_REAL
      ){
        sqlite3VdbeAddOp1(v, OP_RealAffinity, target);
      }
#endif
      break;
    }


    /*
    ** Form A:
    **   CASE x WHEN e1 THEN r1 WHEN e2 THEN r2 ... WHEN eN THEN rN ELSE y END
    **
    ** Form B:
    **   CASE WHEN e1 THEN r1 WHEN e2 THEN r2 ... WHEN eN THEN rN ELSE y END
    **
    ** Form A is can be transformed into the equivalent form B as follows:
    **   CASE WHEN x=e1 THEN r1 WHEN x=e2 THEN r2 ...
    **        WHEN x=eN THEN rN ELSE y END
    **
    ** X (if it exists) is in pExpr->pLeft.
    ** Y is in pExpr->pRight.  The Y is also optional.  If there is no
    ** ELSE clause and no other term matches, then the result of the
    ** exprssion is NULL.
    ** Ei is in pExpr->pList->a[i*2] and Ri is pExpr->pList->a[i*2+1].
    **
    ** The result of the expression is the Ri for the first matching Ei,
    ** or if there is no matching Ei, the ELSE term Y, or if there is
    ** no ELSE term, NULL.
    */
    default: assert( op==TK_CASE ); {
      int endLabel;                     /* GOTO label for end of CASE stmt */
      int nextCase;                     /* GOTO label for next WHEN clause */
      int nExpr;                        /* 2x number of WHEN terms */
      int i;                            /* Loop counter */
      ExprList *pEList;                 /* List of WHEN terms */
      struct ExprList_item *aListelem;  /* Array of WHEN terms */
      Expr opCompare;                   /* The X==Ei expression */
      Expr cacheX;                      /* Cached expression X */
      Expr *pX;                         /* The X expression */
      Expr *pTest = 0;                  /* X==Ei (form A) or just Ei (form B) */
      VVA_ONLY( int iCacheLevel = pParse->iCacheLevel; )

      assert( !ExprHasProperty(pExpr, EP_xIsSelect) && pExpr->x.pList );
      assert((pExpr->x.pList->nExpr % 2) == 0);
      assert(pExpr->x.pList->nExpr > 0);
      pEList = pExpr->x.pList;
      aListelem = pEList->a;
      nExpr = pEList->nExpr;
      endLabel = sqlite3VdbeMakeLabel(v);
      if( (pX = pExpr->pLeft)!=0 ){
        cacheX = *pX;
        testcase( pX->op==TK_COLUMN );
        testcase( pX->op==TK_REGISTER );
        cacheX.iTable = sqlite3ExprCodeTemp(pParse, pX, &regFree1);
        testcase( regFree1==0 );
        cacheX.op = TK_REGISTER;
        opCompare.op = TK_EQ;
        opCompare.pLeft = &cacheX;
        pTest = &opCompare;
        /* Ticket b351d95f9cd5ef17e9d9dbae18f5ca8611190001:
        ** The value in regFree1 might get SCopy-ed into the file result.
        ** So make sure that the regFree1 register is not reused for other
        ** purposes and possibly overwritten.  */
        regFree1 = 0;
      }
      for(i=0; i<nExpr; i=i+2){
        sqlite3ExprCachePush(pParse);
        if( pX ){
          assert( pTest!=0 );
          opCompare.pRight = aListelem[i].pExpr;
        }else{
          pTest = aListelem[i].pExpr;
        }
        nextCase = sqlite3VdbeMakeLabel(v);
        testcase( pTest->op==TK_COLUMN );
        sqlite3ExprIfFalse(pParse, pTest, nextCase, SQLITE_JUMPIFNULL);
        testcase( aListelem[i+1].pExpr->op==TK_COLUMN );
        testcase( aListelem[i+1].pExpr->op==TK_REGISTER );
        sqlite3ExprCode(pParse, aListelem[i+1].pExpr, target);
        sqlite3VdbeAddOp2(v, OP_Goto, 0, endLabel);
        sqlite3ExprCachePop(pParse, 1);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      if( pExpr->pRight ){
        sqlite3ExprCachePush(pParse);
        sqlite3ExprCode(pParse, pExpr->pRight, target);
        sqlite3ExprCachePop(pParse, 1);
      }else{
        sqlite3VdbeAddOp2(v, OP_Null, 0, target);
      }
      assert( db->mallocFailed || pParse->nErr>0 
           || pParse->iCacheLevel==iCacheLevel );
      sqlite3VdbeResolveLabel(v, endLabel);
      break;
    }
#ifndef SQLITE_OMIT_TRIGGER
    case TK_RAISE: {
      assert( pExpr->affinity==OE_Rollback 
           || pExpr->affinity==OE_Abort
           || pExpr->affinity==OE_Fail
           || pExpr->affinity==OE_Ignore
      );
      if( !pParse->pTriggerTab ){
        sqlite3ErrorMsg(pParse,
                       "RAISE() may only be used within a trigger-program");
        return 0;
      }
      if( pExpr->affinity==OE_Abort ){
        sqlite3MayAbort(pParse);
      }
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      if( pExpr->affinity==OE_Ignore ){
        sqlite3VdbeAddOp4(
            v, OP_Halt, SQLITE_OK, OE_Ignore, 0, pExpr->u.zToken,0);
      }else{
        sqlite3HaltConstraint(pParse, pExpr->affinity, pExpr->u.zToken, 0);
      }

      break;
    }
#endif
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  return inReg;
}

/*
** Generate code to evaluate an expression and store the results
** into a register.  Return the register number where the results
** are stored.
**
** If the register is a temporary register that can be deallocated,
** then write its number into *pReg.  If the result register is not
** a temporary, then set *pReg to zero.
*/
SQLITE_PRIVATE int sqlite3ExprCodeTemp(Parse *pParse, Expr *pExpr, int *pReg){
  int r1 = sqlite3GetTempReg(pParse);
  int r2 = sqlite3ExprCodeTarget(pParse, pExpr, r1);
  if( r2==r1 ){
    *pReg = r1;
  }else{
    sqlite3ReleaseTempReg(pParse, r1);
    *pReg = 0;
  }
  return r2;
}

/*
** Generate code that will evaluate expression pExpr and store the
** results in register target.  The results are guaranteed to appear
** in register target.
*/
SQLITE_PRIVATE int sqlite3ExprCode(Parse *pParse, Expr *pExpr, int target){
  int inReg;

  assert( target>0 && target<=pParse->nMem );
  if( pExpr && pExpr->op==TK_REGISTER ){
    sqlite3VdbeAddOp2(pParse->pVdbe, OP_Copy, pExpr->iTable, target);
  }else{
    inReg = sqlite3ExprCodeTarget(pParse, pExpr, target);
    assert( pParse->pVdbe || pParse->db->mallocFailed );
    if( inReg!=target && pParse->pVdbe ){
      sqlite3VdbeAddOp2(pParse->pVdbe, OP_SCopy, inReg, target);
    }
  }
  return target;
}

/*
** Generate code that evalutes the given expression and puts the result
** in register target.
**
** Also make a copy of the expression results into another "cache" register
** and modify the expression so that the next time it is evaluated,
** the result is a copy of the cache register.
**
** This routine is used for expressions that are used multiple 
** times.  They are evaluated once and the results of the expression
** are reused.
*/
SQLITE_PRIVATE int sqlite3ExprCodeAndCache(Parse *pParse, Expr *pExpr, int target){
  Vdbe *v = pParse->pVdbe;
  int inReg;
  inReg = sqlite3ExprCode(pParse, pExpr, target);
  assert( target>0 );
  /* This routine is called for terms to INSERT or UPDATE.  And the only
  ** other place where expressions can be converted into TK_REGISTER is
  ** in WHERE clause processing.  So as currently implemented, there is
  ** no way for a TK_REGISTER to exist here.  But it seems prudent to
  ** keep the ALWAYS() in case the conditions above change with future
  ** modifications or enhancements. */
  if( ALWAYS(pExpr->op!=TK_REGISTER) ){  
    int iMem;
    iMem = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Copy, inReg, iMem);
    pExpr->iTable = iMem;
    pExpr->op2 = pExpr->op;
    pExpr->op = TK_REGISTER;
  }
  return inReg;
}

/*
** Return TRUE if pExpr is an constant expression that is appropriate
** for factoring out of a loop.  Appropriate expressions are:
**
**    *  Any expression that evaluates to two or more opcodes.
**
**    *  Any OP_Integer, OP_Real, OP_String, OP_Blob, OP_Null, 
**       or OP_Variable that does not need to be placed in a 
**       specific register.
**
** There is no point in factoring out single-instruction constant
** expressions that need to be placed in a particular register.  
** We could factor them out, but then we would end up adding an
** OP_SCopy instruction to move the value into the correct register
** later.  We might as well just use the original instruction and
** avoid the OP_SCopy.
*/
static int isAppropriateForFactoring(Expr *p){
  if( !sqlite3ExprIsConstantNotJoin(p) ){
    return 0;  /* Only constant expressions are appropriate for factoring */
  }
  if( (p->flags & EP_FixedDest)==0 ){
    return 1;  /* Any constant without a fixed destination is appropriate */
  }
  while( p->op==TK_UPLUS ) p = p->pLeft;
  switch( p->op ){
#ifndef SQLITE_OMIT_BLOB_LITERAL
    case TK_BLOB:
#endif
    case TK_VARIABLE:
    case TK_INTEGER:
    case TK_FLOAT:
    case TK_NULL:
    case TK_STRING: {
      testcase( p->op==TK_BLOB );
      testcase( p->op==TK_VARIABLE );
      testcase( p->op==TK_INTEGER );
      testcase( p->op==TK_FLOAT );
      testcase( p->op==TK_NULL );
      testcase( p->op==TK_STRING );
      /* Single-instruction constants with a fixed destination are
      ** better done in-line.  If we factor them, they will just end
      ** up generating an OP_SCopy to move the value to the destination
      ** register. */
      return 0;
    }
    case TK_UMINUS: {
      if( p->pLeft->op==TK_FLOAT || p->pLeft->op==TK_INTEGER ){
        return 0;
      }
      break;
    }
    default: {
      break;
    }
  }
  return 1;
}

/*
** If pExpr is a constant expression that is appropriate for
** factoring out of a loop, then evaluate the expression
** into a register and convert the expression into a TK_REGISTER
** expression.
*/
static int evalConstExpr(Walker *pWalker, Expr *pExpr){
  Parse *pParse = pWalker->pParse;
  switch( pExpr->op ){
    case TK_IN:
    case TK_REGISTER: {
      return WRC_Prune;
    }
    case TK_FUNCTION:
    case TK_AGG_FUNCTION:
    case TK_CONST_FUNC: {
      /* The arguments to a function have a fixed destination.
      ** Mark them this way to avoid generated unneeded OP_SCopy
      ** instructions. 
      */
      ExprList *pList = pExpr->x.pList;
      assert( !ExprHasProperty(pExpr, EP_xIsSelect) );
      if( pList ){
        int i = pList->nExpr;
        struct ExprList_item *pItem = pList->a;
        for(; i>0; i--, pItem++){
          if( ALWAYS(pItem->pExpr) ) pItem->pExpr->flags |= EP_FixedDest;
        }
      }
      break;
    }
  }
  if( isAppropriateForFactoring(pExpr) ){
    int r1 = ++pParse->nMem;
    int r2;
    r2 = sqlite3ExprCodeTarget(pParse, pExpr, r1);
    if( NEVER(r1!=r2) ) sqlite3ReleaseTempReg(pParse, r1);
    pExpr->op2 = pExpr->op;
    pExpr->op = TK_REGISTER;
    pExpr->iTable = r2;
    return WRC_Prune;
  }
  return WRC_Continue;
}

/*
** Preevaluate constant subexpressions within pExpr and store the
** results in registers.  Modify pExpr so that the constant subexpresions
** are TK_REGISTER opcodes that refer to the precomputed values.
**
** This routine is a no-op if the jump to the cookie-check code has
** already occur.  Since the cookie-check jump is generated prior to
** any other serious processing, this check ensures that there is no
** way to accidently bypass the constant initializations.
**
** This routine is also a no-op if the SQLITE_FactorOutConst optimization
** is disabled via the sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS)
** interface.  This allows test logic to verify that the same answer is
** obtained for queries regardless of whether or not constants are
** precomputed into registers or if they are inserted in-line.
*/
SQLITE_PRIVATE void sqlite3ExprCodeConstants(Parse *pParse, Expr *pExpr){
  Walker w;
  if( pParse->cookieGoto ) return;
  if( (pParse->db->flags & SQLITE_FactorOutConst)!=0 ) return;
  w.xExprCallback = evalConstExpr;
  w.xSelectCallback = 0;
  w.pParse = pParse;
  sqlite3WalkExpr(&w, pExpr);
}


/*
** Generate code that pushes the value of every element of the given
** expression list into a sequence of registers beginning at target.
**
** Return the number of elements evaluated.
*/
SQLITE_PRIVATE int sqlite3ExprCodeExprList(
  Parse *pParse,     /* Parsing context */
  ExprList *pList,   /* The expression list to be coded */
  int target,        /* Where to write results */
  int doHardCopy     /* Make a hard copy of every element */
){
  struct ExprList_item *pItem;
  int i, n;
  assert( pList!=0 );
  assert( target>0 );
  assert( pParse->pVdbe!=0 );  /* Never gets this far otherwise */
  n = pList->nExpr;
  for(pItem=pList->a, i=0; i<n; i++, pItem++){
    Expr *pExpr = pItem->pExpr;
    int inReg = sqlite3ExprCodeTarget(pParse, pExpr, target+i);
    if( inReg!=target+i ){
      sqlite3VdbeAddOp2(pParse->pVdbe, doHardCopy ? OP_Copy : OP_SCopy,
                        inReg, target+i);
    }
  }
  return n;
}

/*
** Generate code for a BETWEEN operator.
**
**    x BETWEEN y AND z
**
** The above is equivalent to 
**
**    x>=y AND x<=z
**
** Code it as such, taking care to do the common subexpression
** elementation of x.
*/
static void exprCodeBetween(
  Parse *pParse,    /* Parsing and code generating context */
  Expr *pExpr,      /* The BETWEEN expression */
  int dest,         /* Jump here if the jump is taken */
  int jumpIfTrue,   /* Take the jump if the BETWEEN is true */
  int jumpIfNull    /* Take the jump if the BETWEEN is NULL */
){
  Expr exprAnd;     /* The AND operator in  x>=y AND x<=z  */
  Expr compLeft;    /* The  x>=y  term */
  Expr compRight;   /* The  x<=z  term */
  Expr exprX;       /* The  x  subexpression */
  int regFree1 = 0; /* Temporary use register */

  assert( !ExprHasProperty(pExpr, EP_xIsSelect) );
  exprX = *pExpr->pLeft;
  exprAnd.op = TK_AND;
  exprAnd.pLeft = &compLeft;
  exprAnd.pRight = &compRight;
  compLeft.op = TK_GE;
  compLeft.pLeft = &exprX;
  compLeft.pRight = pExpr->x.pList->a[0].pExpr;
  compRight.op = TK_LE;
  compRight.pLeft = &exprX;
  compRight.pRight = pExpr->x.pList->a[1].pExpr;
  exprX.iTable = sqlite3ExprCodeTemp(pParse, &exprX, &regFree1);
  exprX.op = TK_REGISTER;
  if( jumpIfTrue ){
    sqlite3ExprIfTrue(pParse, &exprAnd, dest, jumpIfNull);
  }else{
    sqlite3ExprIfFalse(pParse, &exprAnd, dest, jumpIfNull);
  }
  sqlite3ReleaseTempReg(pParse, regFree1);

  /* Ensure adequate test coverage */
  testcase( jumpIfTrue==0 && jumpIfNull==0 && regFree1==0 );
  testcase( jumpIfTrue==0 && jumpIfNull==0 && regFree1!=0 );
  testcase( jumpIfTrue==0 && jumpIfNull!=0 && regFree1==0 );
  testcase( jumpIfTrue==0 && jumpIfNull!=0 && regFree1!=0 );
  testcase( jumpIfTrue!=0 && jumpIfNull==0 && regFree1==0 );
  testcase( jumpIfTrue!=0 && jumpIfNull==0 && regFree1!=0 );
  testcase( jumpIfTrue!=0 && jumpIfNull!=0 && regFree1==0 );
  testcase( jumpIfTrue!=0 && jumpIfNull!=0 && regFree1!=0 );
}

/*
** Generate code for a boolean expression such that a jump is made
** to the label "dest" if the expression is true but execution
** continues straight thru if the expression is false.
**
** If the expression evaluates to NULL (neither true nor false), then
** take the jump if the jumpIfNull flag is SQLITE_JUMPIFNULL.
**
** This code depends on the fact that certain token values (ex: TK_EQ)
** are the same as opcode values (ex: OP_Eq) that implement the corresponding
** operation.  Special comments in vdbe.c and the mkopcodeh.awk script in
** the make process cause these values to align.  Assert()s in the code
** below verify that the numbers are aligned correctly.
*/
SQLITE_PRIVATE void sqlite3ExprIfTrue(Parse *pParse, Expr *pExpr, int dest, int jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  int op = 0;
  int regFree1 = 0;
  int regFree2 = 0;
  int r1, r2;

  assert( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  if( NEVER(v==0) )     return;  /* Existance of VDBE checked by caller */
  if( NEVER(pExpr==0) ) return;  /* No way this can happen */
  op = pExpr->op;
  switch( op ){
    case TK_AND: {
      int d2 = sqlite3VdbeMakeLabel(v);
      testcase( jumpIfNull==0 );
      sqlite3ExprCachePush(pParse);
      sqlite3ExprIfFalse(pParse, pExpr->pLeft, d2,jumpIfNull^SQLITE_JUMPIFNULL);
      sqlite3ExprIfTrue(pParse, pExpr->pRight, dest, jumpIfNull);
      sqlite3VdbeResolveLabel(v, d2);
      sqlite3ExprCachePop(pParse, 1);
      break;
    }
    case TK_OR: {
      testcase( jumpIfNull==0 );
      sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest, jumpIfNull);
      sqlite3ExprIfTrue(pParse, pExpr->pRight, dest, jumpIfNull);
      break;
    }
    case TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest, jumpIfNull);
      break;
    }
    case TK_LT:
    case TK_LE:
    case TK_GT:
    case TK_GE:
    case TK_NE:
    case TK_EQ: {
      assert( TK_LT==OP_Lt );
      assert( TK_LE==OP_Le );
      assert( TK_GT==OP_Gt );
      assert( TK_GE==OP_Ge );
      assert( TK_EQ==OP_Eq );
      assert( TK_NE==OP_Ne );
      testcase( op==TK_LT );
      testcase( op==TK_LE );
      testcase( op==TK_GT );
      testcase( op==TK_GE );
      testcase( op==TK_EQ );
      testcase( op==TK_NE );
      testcase( jumpIfNull==0 );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, dest, jumpIfNull);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      break;
    }
    case TK_IS:
    case TK_ISNOT: {
      testcase( op==TK_IS );
      testcase( op==TK_ISNOT );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
      op = (op==TK_IS) ? TK_EQ : TK_NE;
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, dest, SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      break;
    }
    case TK_ISNULL:
    case TK_NOTNULL: {
      assert( TK_ISNULL==OP_IsNull );
      assert( TK_NOTNULL==OP_NotNull );
      testcase( op==TK_ISNULL );
      testcase( op==TK_NOTNULL );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( regFree1==0 );
      break;
    }
    case TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      exprCodeBetween(pParse, pExpr, dest, 1, jumpIfNull);
      break;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_IN: {
      int destIfFalse = sqlite3VdbeMakeLabel(v);
      int destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3ExprCodeIN(pParse, pExpr, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, OP_Goto, 0, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      break;
    }
#endif
    default: {
      r1 = sqlite3ExprCodeTemp(pParse, pExpr, &regFree1);
      sqlite3VdbeAddOp3(v, OP_If, r1, dest, jumpIfNull!=0);
      testcase( regFree1==0 );
      testcase( jumpIfNull==0 );
      break;
    }
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);  
}

/*
** Generate code for a boolean expression such that a jump is made
** to the label "dest" if the expression is false but execution
** continues straight thru if the expression is true.
**
** If the expression evaluates to NULL (neither true nor false) then
** jump if jumpIfNull is SQLITE_JUMPIFNULL or fall through if jumpIfNull
** is 0.
*/
SQLITE_PRIVATE void sqlite3ExprIfFalse(Parse *pParse, Expr *pExpr, int dest, int jumpIfNull){
  Vdbe *v = pParse->pVdbe;
  int op = 0;
  int regFree1 = 0;
  int regFree2 = 0;
  int r1, r2;

  assert( jumpIfNull==SQLITE_JUMPIFNULL || jumpIfNull==0 );
  if( NEVER(v==0) ) return; /* Existance of VDBE checked by caller */
  if( pExpr==0 )    return;

  /* The value of pExpr->op and op are related as follows:
  **
  **       pExpr->op            op
  **       ---------          ----------
  **       TK_ISNULL          OP_NotNull
  **       TK_NOTNULL         OP_IsNull
  **       TK_NE              OP_Eq
  **       TK_EQ              OP_Ne
  **       TK_GT              OP_Le
  **       TK_LE              OP_Gt
  **       TK_GE              OP_Lt
  **       TK_LT              OP_Ge
  **
  ** For other values of pExpr->op, op is undefined and unused.
  ** The value of TK_ and OP_ constants are arranged such that we
  ** can compute the mapping above using the following expression.
  ** Assert()s verify that the computation is correct.
  */
  op = ((pExpr->op+(TK_ISNULL&1))^1)-(TK_ISNULL&1);

  /* Verify correct alignment of TK_ and OP_ constants
  */
  assert( pExpr->op!=TK_ISNULL || op==OP_NotNull );
  assert( pExpr->op!=TK_NOTNULL || op==OP_IsNull );
  assert( pExpr->op!=TK_NE || op==OP_Eq );
  assert( pExpr->op!=TK_EQ || op==OP_Ne );
  assert( pExpr->op!=TK_LT || op==OP_Ge );
  assert( pExpr->op!=TK_LE || op==OP_Gt );
  assert( pExpr->op!=TK_GT || op==OP_Le );
  assert( pExpr->op!=TK_GE || op==OP_Lt );

  switch( pExpr->op ){
    case TK_AND: {
      testcase( jumpIfNull==0 );
      sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest, jumpIfNull);
      sqlite3ExprIfFalse(pParse, pExpr->pRight, dest, jumpIfNull);
      break;
    }
    case TK_OR: {
      int d2 = sqlite3VdbeMakeLabel(v);
      testcase( jumpIfNull==0 );
      sqlite3ExprCachePush(pParse);
      sqlite3ExprIfTrue(pParse, pExpr->pLeft, d2, jumpIfNull^SQLITE_JUMPIFNULL);
      sqlite3ExprIfFalse(pParse, pExpr->pRight, dest, jumpIfNull);
      sqlite3VdbeResolveLabel(v, d2);
      sqlite3ExprCachePop(pParse, 1);
      break;
    }
    case TK_NOT: {
      testcase( jumpIfNull==0 );
      sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest, jumpIfNull);
      break;
    }
    case TK_LT:
    case TK_LE:
    case TK_GT:
    case TK_GE:
    case TK_NE:
    case TK_EQ: {
      testcase( op==TK_LT );
      testcase( op==TK_LE );
      testcase( op==TK_GT );
      testcase( op==TK_GE );
      testcase( op==TK_EQ );
      testcase( op==TK_NE );
      testcase( jumpIfNull==0 );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, dest, jumpIfNull);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      break;
    }
    case TK_IS:
    case TK_ISNOT: {
      testcase( pExpr->op==TK_IS );
      testcase( pExpr->op==TK_ISNOT );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
      op = (pExpr->op==TK_IS) ? TK_NE : TK_EQ;
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op,
                  r1, r2, dest, SQLITE_NULLEQ);
      testcase( regFree1==0 );
      testcase( regFree2==0 );
      break;
    }
    case TK_ISNULL:
    case TK_NOTNULL: {
      testcase( op==TK_ISNULL );
      testcase( op==TK_NOTNULL );
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      testcase( regFree1==0 );
      break;
    }
    case TK_BETWEEN: {
      testcase( jumpIfNull==0 );
      exprCodeBetween(pParse, pExpr, dest, 0, jumpIfNull);
      break;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_IN: {
      if( jumpIfNull ){
        sqlite3ExprCodeIN(pParse, pExpr, dest, dest);
      }else{
        int destIfNull = sqlite3VdbeMakeLabel(v);
        sqlite3ExprCodeIN(pParse, pExpr, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      break;
    }
#endif
    default: {
      r1 = sqlite3ExprCodeTemp(pParse, pExpr, &regFree1);
      sqlite3VdbeAddOp3(v, OP_IfNot, r1, dest, jumpIfNull!=0);
      testcase( regFree1==0 );
      testcase( jumpIfNull==0 );
      break;
    }
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
}

/*
** Do a deep comparison of two expression trees.  Return 0 if the two
** expressions are completely identical.  Return 1 if they differ only
** by a COLLATE operator at the top level.  Return 2 if there are differences
** other than the top-level COLLATE operator.
**
** Sometimes this routine will return 2 even if the two expressions
** really are equivalent.  If we cannot prove that the expressions are
** identical, we return 2 just to be safe.  So if this routine
** returns 2, then you do not really know for certain if the two
** expressions are the same.  But if you get a 0 or 1 return, then you
** can be sure the expressions are the same.  In the places where
** this routine is used, it does not hurt to get an extra 2 - that
** just might result in some slightly slower code.  But returning
** an incorrect 0 or 1 could lead to a malfunction.
*/
SQLITE_PRIVATE int sqlite3ExprCompare(Expr *pA, Expr *pB){
  if( pA==0||pB==0 ){
    return pB==pA ? 0 : 2;
  }
  assert( !ExprHasAnyProperty(pA, EP_TokenOnly|EP_Reduced) );
  assert( !ExprHasAnyProperty(pB, EP_TokenOnly|EP_Reduced) );
  if( ExprHasProperty(pA, EP_xIsSelect) || ExprHasProperty(pB, EP_xIsSelect) ){
    return 2;
  }
  if( (pA->flags & EP_Distinct)!=(pB->flags & EP_Distinct) ) return 2;
  if( pA->op!=pB->op ) return 2;
  if( sqlite3ExprCompare(pA->pLeft, pB->pLeft) ) return 2;
  if( sqlite3ExprCompare(pA->pRight, pB->pRight) ) return 2;
  if( sqlite3ExprListCompare(pA->x.pList, pB->x.pList) ) return 2;
  if( pA->iTable!=pB->iTable || pA->iColumn!=pB->iColumn ) return 2;
  if( ExprHasProperty(pA, EP_IntValue) ){
    if( !ExprHasProperty(pB, EP_IntValue) || pA->u.iValue!=pB->u.iValue ){
      return 2;
    }
  }else if( pA->op!=TK_COLUMN && pA->u.zToken ){
    if( ExprHasProperty(pB, EP_IntValue) || NEVER(pB->u.zToken==0) ) return 2;
    if( sqlite3StrICmp(pA->u.zToken,pB->u.zToken)!=0 ){
      return 2;
    }
  }
  if( (pA->flags & EP_ExpCollate)!=(pB->flags & EP_ExpCollate) ) return 1;
  if( (pA->flags & EP_ExpCollate)!=0 && pA->pColl!=pB->pColl ) return 2;
  return 0;
}

/*
** Compare two ExprList objects.  Return 0 if they are identical and 
** non-zero if they differ in any way.
**
** This routine might return non-zero for equivalent ExprLists.  The
** only consequence will be disabled optimizations.  But this routine
** must never return 0 if the two ExprList objects are different, or
** a malfunction will result.
**
** Two NULL pointers are considered to be the same.  But a NULL pointer
** always differs from a non-NULL pointer.
*/
SQLITE_PRIVATE int sqlite3ExprListCompare(ExprList *pA, ExprList *pB){
  int i;
  if( pA==0 && pB==0 ) return 0;
  if( pA==0 || pB==0 ) return 1;
  if( pA->nExpr!=pB->nExpr ) return 1;
  for(i=0; i<pA->nExpr; i++){
    Expr *pExprA = pA->a[i].pExpr;
    Expr *pExprB = pB->a[i].pExpr;
    if( pA->a[i].sortOrder!=pB->a[i].sortOrder ) return 1;
    if( sqlite3ExprCompare(pExprA, pExprB) ) return 1;
  }
  return 0;
}

/*
** Add a new element to the pAggInfo->aCol[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
static int addAggInfoColumn(sqlite3 *db, AggInfo *pInfo){
  int i;
  pInfo->aCol = sqlite3ArrayAllocate(
       db,
       pInfo->aCol,
       sizeof(pInfo->aCol[0]),
       3,
       &pInfo->nColumn,
       &pInfo->nColumnAlloc,
       &i
  );
  return i;
}    

/*
** Add a new element to the pAggInfo->aFunc[] array.  Return the index of
** the new element.  Return a negative number if malloc fails.
*/
static int addAggInfoFunc(sqlite3 *db, AggInfo *pInfo){
  int i;
  pInfo->aFunc = sqlite3ArrayAllocate(
       db, 
       pInfo->aFunc,
       sizeof(pInfo->aFunc[0]),
       3,
       &pInfo->nFunc,
       &pInfo->nFuncAlloc,
       &i
  );
  return i;
}    

/*
** This is the xExprCallback for a tree walker.  It is used to
** implement sqlite3ExprAnalyzeAggregates().  See sqlite3ExprAnalyzeAggregates
** for additional information.
*/
static int analyzeAggregate(Walker *pWalker, Expr *pExpr){
  int i;
  NameContext *pNC = pWalker->u.pNC;
  Parse *pParse = pNC->pParse;
  SrcList *pSrcList = pNC->pSrcList;
  AggInfo *pAggInfo = pNC->pAggInfo;

  switch( pExpr->op ){
    case TK_AGG_COLUMN:
    case TK_COLUMN: {
      testcase( pExpr->op==TK_AGG_COLUMN );
      testcase( pExpr->op==TK_COLUMN );
      /* Check to see if the column is in one of the tables in the FROM
      ** clause of the aggregate query */
      if( ALWAYS(pSrcList!=0) ){
        struct SrcList_item *pItem = pSrcList->a;
        for(i=0; i<pSrcList->nSrc; i++, pItem++){
          struct AggInfo_col *pCol;
          assert( !ExprHasAnyProperty(pExpr, EP_TokenOnly|EP_Reduced) );
          if( pExpr->iTable==pItem->iCursor ){
            /* If we reach this point, it means that pExpr refers to a table
            ** that is in the FROM clause of the aggregate query.  
            **
            ** Make an entry for the column in pAggInfo->aCol[] if there
            ** is not an entry there already.
            */
            int k;
            pCol = pAggInfo->aCol;
            for(k=0; k<pAggInfo->nColumn; k++, pCol++){
              if( pCol->iTable==pExpr->iTable &&
                  pCol->iColumn==pExpr->iColumn ){
                break;
              }
            }
            if( (k>=pAggInfo->nColumn)
             && (k = addAggInfoColumn(pParse->db, pAggInfo))>=0 
            ){
              pCol = &pAggInfo->aCol[k];
              pCol->pTab = pExpr->pTab;
              pCol->iTable = pExpr->iTable;
              pCol->iColumn = pExpr->iColumn;
              pCol->iMem = ++pParse->nMem;
              pCol->iSorterColumn = -1;
              pCol->pExpr = pExpr;
              if( pAggInfo->pGroupBy ){
                int j, n;
                ExprList *pGB = pAggInfo->pGroupBy;
                struct ExprList_item *pTerm = pGB->a;
                n = pGB->nExpr;
                for(j=0; j<n; j++, pTerm++){
                  Expr *pE = pTerm->pExpr;
                  if( pE->op==TK_COLUMN && pE->iTable==pExpr->iTable &&
                      pE->iColumn==pExpr->iColumn ){
                    pCol->iSorterColumn = j;
                    break;
                  }
                }
              }
              if( pCol->iSorterColumn<0 ){
                pCol->iSorterColumn = pAggInfo->nSortingColumn++;
              }
            }
            /* There is now an entry for pExpr in pAggInfo->aCol[] (either
            ** because it was there before or because we just created it).
            ** Convert the pExpr to be a TK_AGG_COLUMN referring to that
            ** pAggInfo->aCol[] entry.
            */
            ExprSetIrreducible(pExpr);
            pExpr->pAggInfo = pAggInfo;
            pExpr->op = TK_AGG_COLUMN;
            pExpr->iAgg = (i16)k;
            break;
          } /* endif pExpr->iTable==pItem->iCursor */
        } /* end loop over pSrcList */
      }
      return WRC_Prune;
    }
    case TK_AGG_FUNCTION: {
      /* The pNC->nDepth==0 test causes aggregate functions in subqueries
      ** to be ignored */
      if( pNC->nDepth==0 ){
        /* Check to see if pExpr is a duplicate of another aggregate 
        ** function that is already in the pAggInfo structure
        */
        struct AggInfo_func *pItem = pAggInfo->aFunc;
        for(i=0; i<pAggInfo->nFunc; i++, pItem++){
          if( sqlite3ExprCompare(pItem->pExpr, pExpr)==0 ){
            break;
          }
        }
        if( i>=pAggInfo->nFunc ){
          /* pExpr is original.  Make a new entry in pAggInfo->aFunc[]
          */
          u8 enc = ENC(pParse->db);
          i = addAggInfoFunc(pParse->db, pAggInfo);
          if( i>=0 ){
            assert( !ExprHasProperty(pExpr, EP_xIsSelect) );
            pItem = &pAggInfo->aFunc[i];
            pItem->pExpr = pExpr;
            pItem->iMem = ++pParse->nMem;
            assert( !ExprHasProperty(pExpr, EP_IntValue) );
            pItem->pFunc = sqlite3FindFunction(pParse->db,
                   pExpr->u.zToken, sqlite3Strlen30(pExpr->u.zToken),
                   pExpr->x.pList ? pExpr->x.pList->nExpr : 0, enc, 0);
            if( pExpr->flags & EP_Distinct ){
              pItem->iDistinct = pParse->nTab++;
            }else{
              pItem->iDistinct = -1;
            }
          }
        }
        /* Make pExpr point to the appropriate pAggInfo->aFunc[] entry
        */
        assert( !ExprHasAnyProperty(pExpr, EP_TokenOnly|EP_Reduced) );
        ExprSetIrreducible(pExpr);
        pExpr->iAgg = (i16)i;
        pExpr->pAggInfo = pAggInfo;
        return WRC_Prune;
      }
    }
  }
  return WRC_Continue;
}
static int analyzeAggregatesInSelect(Walker *pWalker, Select *pSelect){
  NameContext *pNC = pWalker->u.pNC;
  if( pNC->nDepth==0 ){
    pNC->nDepth++;
    sqlite3WalkSelect(pWalker, pSelect);
    pNC->nDepth--;
    return WRC_Prune;
  }else{
    return WRC_Continue;
  }
}

/*
** Analyze the given expression looking for aggregate functions and
** for variables that need to be added to the pParse->aAgg[] array.
** Make additional entries to the pParse->aAgg[] array as necessary.
**
** This routine should only be called after the expression has been
** analyzed by sqlite3ResolveExprNames().
*/
SQLITE_PRIVATE void sqlite3ExprAnalyzeAggregates(NameContext *pNC, Expr *pExpr){
  Walker w;
  w.xExprCallback = analyzeAggregate;
  w.xSelectCallback = analyzeAggregatesInSelect;
  w.u.pNC = pNC;
  assert( pNC->pSrcList!=0 );
  sqlite3WalkExpr(&w, pExpr);
}

/*
** Call sqlite3ExprAnalyzeAggregates() for every expression in an
** expression list.  Return the number of errors.
**
** If an error is found, the analysis is cut short.
*/
SQLITE_PRIVATE void sqlite3ExprAnalyzeAggList(NameContext *pNC, ExprList *pList){
  struct ExprList_item *pItem;
  int i;
  if( pList ){
    for(pItem=pList->a, i=0; i<pList->nExpr; i++, pItem++){
      sqlite3ExprAnalyzeAggregates(pNC, pItem->pExpr);
    }
  }
}

/*
** Allocate a single new register for use to hold some intermediate result.
*/
SQLITE_PRIVATE int sqlite3GetTempReg(Parse *pParse){
  if( pParse->nTempReg==0 ){
    return ++pParse->nMem;
  }
  return pParse->aTempReg[--pParse->nTempReg];
}

/*
** Deallocate a register, making available for reuse for some other
** purpose.
**
** If a register is currently being used by the column cache, then
** the dallocation is deferred until the column cache line that uses
** the register becomes stale.
*/
SQLITE_PRIVATE void sqlite3ReleaseTempReg(Parse *pParse, int iReg){
  if( iReg && pParse->nTempReg<ArraySize(pParse->aTempReg) ){
    int i;
    struct yColCache *p;
    for(i=0, p=pParse->aColCache; i<SQLITE_N_COLCACHE; i++, p++){
      if( p->iReg==iReg ){
        p->tempReg = 1;
        return;
      }
    }
    pParse->aTempReg[pParse->nTempReg++] = iReg;
  }
}

/*
** Allocate or deallocate a block of nReg consecutive registers
*/
SQLITE_PRIVATE int sqlite3GetTempRange(Parse *pParse, int nReg){
  int i, n;
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  if( nReg<=n ){
    assert( !usedAsColumnCache(pParse, i, i+n-1) );
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  }else{
    i = pParse->nMem+1;
    pParse->nMem += nReg;
  }
  return i;
}
SQLITE_PRIVATE void sqlite3ReleaseTempRange(Parse *pParse, int iReg, int nReg){
  sqlite3ExprCacheRemove(pParse, iReg, nReg);
  if( nReg>pParse->nRangeReg ){
    pParse->nRangeReg = nReg;
    pParse->iRangeReg = iReg;
  }
}

/************** End of expr.c ************************************************/
/************** Begin file alter.c *******************************************/
/*
** 2005 February 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains C code routines that used to generate VDBE code
** that implements the ALTER TABLE command.
*/

/*
** The code in this file only exists if we are not omitting the
** ALTER TABLE logic from the build.
*/
#ifndef SQLITE_OMIT_ALTERTABLE


/*
** This function is used by SQL generated to implement the 
** ALTER TABLE command. The first argument is the text of a CREATE TABLE or
** CREATE INDEX command. The second is a table name. The table name in 
** the CREATE TABLE or CREATE INDEX statement is replaced with the third
** argument and the result returned. Examples:
**
** sqlite_rename_table('CREATE TABLE abc(a, b, c)', 'def')
**     -> 'CREATE TABLE def(a, b, c)'
**
** sqlite_rename_table('CREATE INDEX i ON abc(a)', 'def')
**     -> 'CREATE INDEX i ON def(a, b, c)'
*/
static void renameTableFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **argv
){
  unsigned char const *zSql = sqlite3_value_text(argv[0]);
  unsigned char const *zTableName = sqlite3_value_text(argv[1]);

  int token;
  Token tname;
  unsigned char const *zCsr = zSql;
  int len = 0;
  char *zRet;

  sqlite3 *db = sqlite3_context_db_handle(context);

  UNUSED_PARAMETER(NotUsed);

  /* The principle used to locate the table name in the CREATE TABLE 
  ** statement is that the table name is the first non-space token that
  ** is immediately followed by a TK_LP or TK_USING token.
  */
  if( zSql ){
    do {
      if( !*zCsr ){
        /* Ran out of input before finding an opening bracket. Return NULL. */
        return;
      }

      /* Store the token that zCsr points to in tname. */
      tname.z = (char*)zCsr;
      tname.n = len;

      /* Advance zCsr to the next token. Store that token type in 'token',
      ** and its length in 'len' (to be used next iteration of this loop).
      */
      do {
        zCsr += len;
        len = sqlite3GetToken(zCsr, &token);
      } while( token==TK_SPACE );
      assert( len>0 );
    } while( token!=TK_LP && token!=TK_USING );

    zRet = sqlite3MPrintf(db, "%.*s\"%w\"%s", ((u8*)tname.z) - zSql, zSql, 
       zTableName, tname.z+tname.n);
    sqlite3_result_text(context, zRet, -1, SQLITE_DYNAMIC);
  }
}

/*
** This C function implements an SQL user function that is used by SQL code
** generated by the ALTER TABLE ... RENAME command to modify the definition
** of any foreign key constraints that use the table being renamed as the 
** parent table. It is passed three arguments:
**
**   1) The complete text of the CREATE TABLE statement being modified,
**   2) The old name of the table being renamed, and
**   3) The new name of the table being renamed.
**
** It returns the new CREATE TABLE statement. For example:
**
**   sqlite_rename_parent('CREATE TABLE t1(a REFERENCES t2)', 't2', 't3')
**       -> 'CREATE TABLE t1(a REFERENCES t3)'
*/
#ifndef SQLITE_OMIT_FOREIGN_KEY
static void renameParentFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  char *zOutput = 0;
  char *zResult;
  unsigned char const *zInput = sqlite3_value_text(argv[0]);
  unsigned char const *zOld = sqlite3_value_text(argv[1]);
  unsigned char const *zNew = sqlite3_value_text(argv[2]);

  unsigned const char *z;         /* Pointer to token */
  int n;                          /* Length of token z */
  int token;                      /* Type of token */

  UNUSED_PARAMETER(NotUsed);
  for(z=zInput; *z; z=z+n){
    n = sqlite3GetToken(z, &token);
    if( token==TK_REFERENCES ){
      char *zParent;
      do {
        z += n;
        n = sqlite3GetToken(z, &token);
      }while( token==TK_SPACE );

      zParent = sqlite3DbStrNDup(db, (const char *)z, n);
      if( zParent==0 ) break;
      sqlite3Dequote(zParent);
      if( 0==sqlite3StrICmp((const char *)zOld, zParent) ){
        char *zOut = sqlite3MPrintf(db, "%s%.*s\"%w\"", 
            (zOutput?zOutput:""), z-zInput, zInput, (const char *)zNew
        );
        sqlite3DbFree(db, zOutput);
        zOutput = zOut;
        zInput = &z[n];
      }
      sqlite3DbFree(db, zParent);
    }
  }

  zResult = sqlite3MPrintf(db, "%s%s", (zOutput?zOutput:""), zInput), 
  sqlite3_result_text(context, zResult, -1, SQLITE_DYNAMIC);
  sqlite3DbFree(db, zOutput);
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
/* This function is used by SQL generated to implement the
** ALTER TABLE command. The first argument is the text of a CREATE TRIGGER 
** statement. The second is a table name. The table name in the CREATE 
** TRIGGER statement is replaced with the third argument and the result 
** returned. This is analagous to renameTableFunc() above, except for CREATE
** TRIGGER, not CREATE INDEX and CREATE TABLE.
*/
static void renameTriggerFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **argv
){
  unsigned char const *zSql = sqlite3_value_text(argv[0]);
  unsigned char const *zTableName = sqlite3_value_text(argv[1]);

  int token;
  Token tname;
  int dist = 3;
  unsigned char const *zCsr = zSql;
  int len = 0;
  char *zRet;
  sqlite3 *db = sqlite3_context_db_handle(context);

  UNUSED_PARAMETER(NotUsed);

  /* The principle used to locate the table name in the CREATE TRIGGER 
  ** statement is that the table name is the first token that is immediatedly
  ** preceded by either TK_ON or TK_DOT and immediatedly followed by one
  ** of TK_WHEN, TK_BEGIN or TK_FOR.
  */
  if( zSql ){
    do {

      if( !*zCsr ){
        /* Ran out of input before finding the table name. Return NULL. */
        return;
      }

      /* Store the token that zCsr points to in tname. */
      tname.z = (char*)zCsr;
      tname.n = len;

      /* Advance zCsr to the next token. Store that token type in 'token',
      ** and its length in 'len' (to be used next iteration of this loop).
      */
      do {
        zCsr += len;
        len = sqlite3GetToken(zCsr, &token);
      }while( token==TK_SPACE );
      assert( len>0 );

      /* Variable 'dist' stores the number of tokens read since the most
      ** recent TK_DOT or TK_ON. This means that when a WHEN, FOR or BEGIN 
      ** token is read and 'dist' equals 2, the condition stated above
      ** to be met.
      **
      ** Note that ON cannot be a database, table or column name, so
      ** there is no need to worry about syntax like 
      ** "CREATE TRIGGER ... ON ON.ON BEGIN ..." etc.
      */
      dist++;
      if( token==TK_DOT || token==TK_ON ){
        dist = 0;
      }
    } while( dist!=2 || (token!=TK_WHEN && token!=TK_FOR && token!=TK_BEGIN) );

    /* Variable tname now contains the token that is the old table-name
    ** in the CREATE TRIGGER statement.
    */
    zRet = sqlite3MPrintf(db, "%.*s\"%w\"%s", ((u8*)tname.z) - zSql, zSql, 
       zTableName, tname.z+tname.n);
    sqlite3_result_text(context, zRet, -1, SQLITE_DYNAMIC);
  }
}
#endif   /* !SQLITE_OMIT_TRIGGER */

/*
** Register built-in functions used to help implement ALTER TABLE
*/
SQLITE_PRIVATE void sqlite3AlterFunctions(void){
  static SQLITE_WSD FuncDef aAlterTableFuncs[] = {
    FUNCTION(sqlite_rename_table,   2, 0, 0, renameTableFunc),
#ifndef SQLITE_OMIT_TRIGGER
    FUNCTION(sqlite_rename_trigger, 2, 0, 0, renameTriggerFunc),
#endif
#ifndef SQLITE_OMIT_FOREIGN_KEY
    FUNCTION(sqlite_rename_parent,  3, 0, 0, renameParentFunc),
#endif
  };
  int i;
  FuncDefHash *pHash = &GLOBAL(FuncDefHash, sqlite3GlobalFunctions);
  FuncDef *aFunc = (FuncDef*)&GLOBAL(FuncDef, aAlterTableFuncs);

  for(i=0; i<ArraySize(aAlterTableFuncs); i++){
    sqlite3FuncDefInsert(pHash, &aFunc[i]);
  }
}

/*
** This function is used to create the text of expressions of the form:
**
**   name=<constant1> OR name=<constant2> OR ...
**
** If argument zWhere is NULL, then a pointer string containing the text 
** "name=<constant>" is returned, where <constant> is the quoted version
** of the string passed as argument zConstant. The returned buffer is
** allocated using sqlite3DbMalloc(). It is the responsibility of the
** caller to ensure that it is eventually freed.
**
** If argument zWhere is not NULL, then the string returned is 
** "<where> OR name=<constant>", where <where> is the contents of zWhere.
** In this case zWhere is passed to sqlite3DbFree() before returning.
** 
*/
static char *whereOrName(sqlite3 *db, char *zWhere, char *zConstant){
  char *zNew;
  if( !zWhere ){
    zNew = sqlite3MPrintf(db, "name=%Q", zConstant);
  }else{
    zNew = sqlite3MPrintf(db, "%s OR name=%Q", zWhere, zConstant);
    sqlite3DbFree(db, zWhere);
  }
  return zNew;
}

#if !defined(SQLITE_OMIT_FOREIGN_KEY) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Generate the text of a WHERE expression which can be used to select all
** tables that have foreign key constraints that refer to table pTab (i.e.
** constraints for which pTab is the parent table) from the sqlite_master
** table.
*/
static char *whereForeignKeys(Parse *pParse, Table *pTab){
  FKey *p;
  char *zWhere = 0;
  for(p=sqlite3FkReferences(pTab); p; p=p->pNextTo){
    zWhere = whereOrName(pParse->db, zWhere, p->pFrom->zName);
  }
  return zWhere;
}
#endif

/*
** Generate the text of a WHERE expression which can be used to select all
** temporary triggers on table pTab from the sqlite_temp_master table. If
** table pTab has no temporary triggers, or is itself stored in the 
** temporary database, NULL is returned.
*/
static char *whereTempTriggers(Parse *pParse, Table *pTab){
  Trigger *pTrig;
  char *zWhere = 0;
  const Schema *pTempSchema = pParse->db->aDb[1].pSchema; /* Temp db schema */

  /* If the table is not located in the temp-db (in which case NULL is 
  ** returned, loop through the tables list of triggers. For each trigger
  ** that is not part of the temp-db schema, add a clause to the WHERE 
  ** expression being built up in zWhere.
  */
  if( pTab->pSchema!=pTempSchema ){
    sqlite3 *db = pParse->db;
    for(pTrig=sqlite3TriggerList(pParse, pTab); pTrig; pTrig=pTrig->pNext){
      if( pTrig->pSchema==pTempSchema ){
        zWhere = whereOrName(db, zWhere, pTrig->zName);
      }
    }
  }
  if( zWhere ){
    char *zNew = sqlite3MPrintf(pParse->db, "type='trigger' AND (%s)", zWhere);
    sqlite3DbFree(pParse->db, zWhere);
    zWhere = zNew;
  }
  return zWhere;
}

/*
** Generate code to drop and reload the internal representation of table
** pTab from the database, including triggers and temporary triggers.
** Argument zName is the name of the table in the database schema at
** the time the generated code is executed. This can be different from
** pTab->zName if this function is being called to code part of an 
** "ALTER TABLE RENAME TO" statement.
*/
static void reloadTableSchema(Parse *pParse, Table *pTab, const char *zName){
  Vdbe *v;
  char *zWhere;
  int iDb;                   /* Index of database containing pTab */
#ifndef SQLITE_OMIT_TRIGGER
  Trigger *pTrig;
#endif

  v = sqlite3GetVdbe(pParse);
  if( NEVER(v==0) ) return;
  assert( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  assert( iDb>=0 );

#ifndef SQLITE_OMIT_TRIGGER
  /* Drop any table triggers from the internal schema. */
  for(pTrig=sqlite3TriggerList(pParse, pTab); pTrig; pTrig=pTrig->pNext){
    int iTrigDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);
    assert( iTrigDb==iDb || iTrigDb==1 );
    sqlite3VdbeAddOp4(v, OP_DropTrigger, iTrigDb, 0, 0, pTrig->zName, 0);
  }
#endif

  /* Drop the table and index from the internal schema.  */
  sqlite3VdbeAddOp4(v, OP_DropTable, iDb, 0, 0, pTab->zName, 0);

  /* Reload the table, index and permanent trigger schemas. */
  zWhere = sqlite3MPrintf(pParse->db, "tbl_name=%Q", zName);
  if( !zWhere ) return;
  sqlite3VdbeAddParseSchemaOp(v, iDb, zWhere);

#ifndef SQLITE_OMIT_TRIGGER
  /* Now, if the table is not stored in the temp database, reload any temp 
  ** triggers. Don't use IN(...) in case SQLITE_OMIT_SUBQUERY is defined. 
  */
  if( (zWhere=whereTempTriggers(pParse, pTab))!=0 ){
    sqlite3VdbeAddParseSchemaOp(v, 1, zWhere);
  }
#endif
}

/*
** Parameter zName is the name of a table that is about to be altered
** (either with ALTER TABLE ... RENAME TO or ALTER TABLE ... ADD COLUMN).
** If the table is a system table, this function leaves an error message
** in pParse->zErr (system tables may not be altered) and returns non-zero.
**
** Or, if zName is not a system table, zero is returned.
*/
static int isSystemTable(Parse *pParse, const char *zName){
  if( sqlite3Strlen30(zName)>6 && 0==sqlite3StrNICmp(zName, "sqlite_", 7) ){
    sqlite3ErrorMsg(pParse, "table %s may not be altered", zName);
    return 1;
  }
  return 0;
}

/*
** Generate code to implement the "ALTER TABLE xxx RENAME TO yyy" 
** command. 
*/
SQLITE_PRIVATE void sqlite3AlterRenameTable(
  Parse *pParse,            /* Parser context. */
  SrcList *pSrc,            /* The table to rename. */
  Token *pName              /* The new table name. */
){
  int iDb;                  /* Database that contains the table */
  char *zDb;                /* Name of database iDb */
  Table *pTab;              /* Table being renamed */
  char *zName = 0;          /* NULL-terminated version of pName */ 
  sqlite3 *db = pParse->db; /* Database connection */
  int nTabName;             /* Number of UTF-8 characters in zTabName */
  const char *zTabName;     /* Original name of the table */
  Vdbe *v;
#ifndef SQLITE_OMIT_TRIGGER
  char *zWhere = 0;         /* Where clause to locate temp triggers */
#endif
  VTable *pVTab = 0;        /* Non-zero if this is a v-tab with an xRename() */
  int savedDbFlags;         /* Saved value of db->flags */

  savedDbFlags = db->flags;  
  if( NEVER(db->mallocFailed) ) goto exit_rename_table;
  assert( pSrc->nSrc==1 );
  assert( sqlite3BtreeHoldsAllMutexes(pParse->db) );

  pTab = sqlite3LocateTable(pParse, 0, pSrc->a[0].zName, pSrc->a[0].zDatabase);
  if( !pTab ) goto exit_rename_table;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zName;
  db->flags |= SQLITE_PreferBuiltin;

  /* Get a NULL terminated version of the new table name. */
  zName = sqlite3NameFromToken(db, pName);
  if( !zName ) goto exit_rename_table;

  /* Check that a table or index named 'zName' does not already exist
  ** in database iDb. If so, this is an error.
  */
  if( sqlite3FindTable(db, zName, zDb) || sqlite3FindIndex(db, zName, zDb) ){
    sqlite3ErrorMsg(pParse, 
        "there is already another table or index with this name: %s", zName);
    goto exit_rename_table;
  }

  /* Make sure it is not a system table being altered, or a reserved name
  ** that the table is being renamed to.
  */
  if( SQLITE_OK!=isSystemTable(pParse, pTab->zName) ){
    goto exit_rename_table;
  }
  if( SQLITE_OK!=sqlite3CheckObjectName(pParse, zName) ){ goto
    exit_rename_table;
  }

#ifndef SQLITE_OMIT_VIEW
  if( pTab->pSelect ){
    sqlite3ErrorMsg(pParse, "view %s may not be altered", pTab->zName);
    goto exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  if( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    goto exit_rename_table;
  }
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE
  if( sqlite3ViewGetColumnNames(pParse, pTab) ){
    goto exit_rename_table;
  }
  if( IsVirtual(pTab) ){
    pVTab = sqlite3GetVTable(db, pTab);
    if( pVTab->pVtab->pModule->xRename==0 ){
      pVTab = 0;
    }
  }
#endif

  /* Begin a transaction and code the VerifyCookie for database iDb. 
  ** Then modify the schema cookie (since the ALTER TABLE modifies the
  ** schema). Open a statement transaction if the table is a virtual
  ** table.
  */
  v = sqlite3GetVdbe(pParse);
  if( v==0 ){
    goto exit_rename_table;
  }
  sqlite3BeginWriteOperation(pParse, pVTab!=0, iDb);
  sqlite3ChangeCookie(pParse, iDb);

  /* If this is a virtual table, invoke the xRename() function if
  ** one is defined. The xRename() callback will modify the names
  ** of any resources used by the v-table implementation (including other
  ** SQLite tables) that are identified by the name of the virtual table.
  */
#ifndef SQLITE_OMIT_VIRTUALTABLE
  if( pVTab ){
    int i = ++pParse->nMem;
    sqlite3VdbeAddOp4(v, OP_String8, 0, i, 0, zName, 0);
    sqlite3VdbeAddOp4(v, OP_VRename, i, 0, 0,(const char*)pVTab, P4_VTAB);
    sqlite3MayAbort(pParse);
  }
#endif

  /* figure out how many UTF-8 characters are in zName */
  zTabName = pTab->zName;
  nTabName = sqlite3Utf8CharLen(zTabName, -1);

#if !defined(SQLITE_OMIT_FOREIGN_KEY) && !defined(SQLITE_OMIT_TRIGGER)
  if( db->flags&SQLITE_ForeignKeys ){
    /* If foreign-key support is enabled, rewrite the CREATE TABLE 
    ** statements corresponding to all child tables of foreign key constraints
    ** for which the renamed table is the parent table.  */
    if( (zWhere=whereForeignKeys(pParse, pTab))!=0 ){
      sqlite3NestedParse(pParse, 
          "UPDATE \"%w\".%s SET "
              "sql = sqlite_rename_parent(sql, %Q, %Q) "
              "WHERE %s;", zDb, SCHEMA_TABLE(iDb), zTabName, zName, zWhere);
      sqlite3DbFree(db, zWhere);
    }
  }
#endif

  /* Modify the sqlite_master table to use the new table name. */
  sqlite3NestedParse(pParse,
      "UPDATE %Q.%s SET "
#ifdef SQLITE_OMIT_TRIGGER
          "sql = sqlite_rename_table(sql, %Q), "
#else
          "sql = CASE "
            "WHEN type = 'trigger' THEN sqlite_rename_trigger(sql, %Q)"
            "ELSE sqlite_rename_table(sql, %Q) END, "
#endif
          "tbl_name = %Q, "
          "name = CASE "
            "WHEN type='table' THEN %Q "
            "WHEN name LIKE 'sqlite_autoindex%%' AND type='index' THEN "
             "'sqlite_autoindex_' || %Q || substr(name,%d+18) "
            "ELSE name END "
      "WHERE tbl_name=%Q AND "
          "(type='table' OR type='index' OR type='trigger');", 
      zDb, SCHEMA_TABLE(iDb), zName, zName, zName, 
#ifndef SQLITE_OMIT_TRIGGER
      zName,
#endif
      zName, nTabName, zTabName
  );

#ifndef SQLITE_OMIT_AUTOINCREMENT
  /* If the sqlite_sequence table exists in this database, then update 
  ** it with the new table name.
  */
  if( sqlite3FindTable(db, "sqlite_sequence", zDb) ){
    sqlite3NestedParse(pParse,
        "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q",
        zDb, zName, pTab->zName);
  }
#endif

#ifndef SQLITE_OMIT_TRIGGER
  /* If there are TEMP triggers on this table, modify the sqlite_temp_master
  ** table. Don't do this if the table being ALTERed is itself located in
  ** the temp database.
  */
  if( (zWhere=whereTempTriggers(pParse, pTab))!=0 ){
    sqlite3NestedParse(pParse, 
        "UPDATE sqlite_temp_master SET "
            "sql = sqlite_rename_trigger(sql, %Q), "
            "tbl_name = %Q "
            "WHERE %s;", zName, zName, zWhere);
    sqlite3DbFree(db, zWhere);
  }
#endif

#if !defined(SQLITE_OMIT_FOREIGN_KEY) && !defined(SQLITE_OMIT_TRIGGER)
  if( db->flags&SQLITE_ForeignKeys ){
    FKey *p;
    for(p=sqlite3FkReferences(pTab); p; p=p->pNextTo){
      Table *pFrom = p->pFrom;
      if( pFrom!=pTab ){
        reloadTableSchema(pParse, p->pFrom, pFrom->zName);
      }
    }
  }
#endif

  /* Drop and reload the internal table schema. */
  reloadTableSchema(pParse, pTab, zName);

exit_rename_table:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zName);
  db->flags = savedDbFlags;
}


/*
** Generate code to make sure the file format number is at least minFormat.
** The generated code will increase the file format number if necessary.
*/
SQLITE_PRIVATE void sqlite3MinimumFileFormat(Parse *pParse, int iDb, int minFormat){
  Vdbe *v;
  v = sqlite3GetVdbe(pParse);
  /* The VDBE should have been allocated before this routine is called.
  ** If that allocation failed, we would have quit before reaching this
  ** point */
  if( ALWAYS(v) ){
    int r1 = sqlite3GetTempReg(pParse);
    int r2 = sqlite3GetTempReg(pParse);
    int j1;
    sqlite3VdbeAddOp3(v, OP_ReadCookie, iDb, r1, BTREE_FILE_FORMAT);
    sqlite3VdbeUsesBtree(v, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, minFormat, r2);
    j1 = sqlite3VdbeAddOp3(v, OP_Ge, r2, 0, r1);
    sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_FILE_FORMAT, r2);
    sqlite3VdbeJumpHere(v, j1);
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
}

/*
** This function is called after an "ALTER TABLE ... ADD" statement
** has been parsed. Argument pColDef contains the text of the new
** column definition.
**
** The Table structure pParse->pNewTable was extended to include
** the new column during parsing.
*/
SQLITE_PRIVATE void sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef){
  Table *pNew;              /* Copy of pParse->pNewTable */
  Table *pTab;              /* Table being altered */
  int iDb;                  /* Database number */
  const char *zDb;          /* Database name */
  const char *zTab;         /* Table name */
  char *zCol;               /* Null-terminated column definition */
  Column *pCol;             /* The new column */
  Expr *pDflt;              /* Default value for the new column */
  sqlite3 *db;              /* The database connection; */

  db = pParse->db;
  if( pParse->nErr || db->mallocFailed ) return;
  pNew = pParse->pNewTable;
  assert( pNew );

  assert( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pNew->pSchema);
  zDb = db->aDb[iDb].zName;
  zTab = &pNew->zName[16];  /* Skip the "sqlite_altertab_" prefix on the name */
  pCol = &pNew->aCol[pNew->nCol-1];
  pDflt = pCol->pDflt;
  pTab = sqlite3FindTable(db, zTab, zDb);
  assert( pTab );

#ifndef SQLITE_OMIT_AUTHORIZATION
  /* Invoke the authorization callback. */
  if( sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0) ){
    return;
  }
#endif

  /* If the default value for the new column was specified with a 
  ** literal NULL, then set pDflt to 0. This simplifies checking
  ** for an SQL NULL default below.
  */
  if( pDflt && pDflt->op==TK_NULL ){
    pDflt = 0;
  }

  /* Check that the new column is not specified as PRIMARY KEY or UNIQUE.
  ** If there is a NOT NULL constraint, then the default value for the
  ** column must not be NULL.
  */
  if( pCol->isPrimKey ){
    sqlite3ErrorMsg(pParse, "Cannot add a PRIMARY KEY column");
    return;
  }
  if( pNew->pIndex ){
    sqlite3ErrorMsg(pParse, "Cannot add a UNIQUE column");
    return;
  }
  if( (db->flags&SQLITE_ForeignKeys) && pNew->pFKey && pDflt ){
    sqlite3ErrorMsg(pParse, 
        "Cannot add a REFERENCES column with non-NULL default value");
    return;
  }
  if( pCol->notNull && !pDflt ){
    sqlite3ErrorMsg(pParse, 
        "Cannot add a NOT NULL column with default value NULL");
    return;
  }

  /* Ensure the default expression is something that sqlite3ValueFromExpr()
  ** can handle (i.e. not CURRENT_TIME etc.)
  */
  if( pDflt ){
    sqlite3_value *pVal;
    if( sqlite3ValueFromExpr(db, pDflt, SQLITE_UTF8, SQLITE_AFF_NONE, &pVal) ){
      db->mallocFailed = 1;
      return;
    }
    if( !pVal ){
      sqlite3ErrorMsg(pParse, "Cannot add a column with non-constant default");
      return;
    }
    sqlite3ValueFree(pVal);
  }

  /* Modify the CREATE TABLE statement. */
  zCol = sqlite3DbStrNDup(db, (char*)pColDef->z, pColDef->n);
  if( zCol ){
    char *zEnd = &zCol[pColDef->n-1];
    int savedDbFlags = db->flags;
    while( zEnd>zCol && (*zEnd==';' || sqlite3Isspace(*zEnd)) ){
      *zEnd-- = '\0';
    }
    db->flags |= SQLITE_PreferBuiltin;
    sqlite3NestedParse(pParse, 
        "UPDATE \"%w\".%s SET "
          "sql = substr(sql,1,%d) || ', ' || %Q || substr(sql,%d) "
        "WHERE type = 'table' AND name = %Q", 
      zDb, SCHEMA_TABLE(iDb), pNew->addColOffset, zCol, pNew->addColOffset+1,
      zTab
    );
    sqlite3DbFree(db, zCol);
    db->flags = savedDbFlags;
  }

  /* If the default value of the new column is NULL, then set the file
  ** format to 2. If the default value of the new column is not NULL,
  ** the file format becomes 3.
  */
  sqlite3MinimumFileFormat(pParse, iDb, pDflt ? 3 : 2);

  /* Reload the schema of the modified table. */
  reloadTableSchema(pParse, pTab, pTab->zName);
}

/*
** This function is called by the parser after the table-name in
** an "ALTER TABLE <table-name> ADD" statement is parsed. Argument 
** pSrc is the full-name of the table being altered.
**
** This routine makes a (partial) copy of the Table structure
** for the table being altered and sets Parse.pNewTable to point
** to it. Routines called by the parser as the column definition
** is parsed (i.e. sqlite3AddColumn()) add the new Column data to 
** the copy. The copy of the Table structure is deleted by tokenize.c 
** after parsing is finished.
**
** Routine sqlite3AlterFinishAddColumn() will be called to complete
** coding the "ALTER TABLE ... ADD" statement.
*/
SQLITE_PRIVATE void sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc){
  Table *pNew;
  Table *pTab;
  Vdbe *v;
  int iDb;
  int i;
  int nAlloc;
  sqlite3 *db = pParse->db;

  /* Look up the table being altered. */
  assert( pParse->pNewTable==0 );
  assert( sqlite3BtreeHoldsAllMutexes(db) );
  if( db->mallocFailed ) goto exit_begin_add_column;
  pTab = sqlite3LocateTable(pParse, 0, pSrc->a[0].zName, pSrc->a[0].zDatabase);
  if( !pTab ) goto exit_begin_add_column;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  if( IsVirtual(pTab) ){
    sqlite3ErrorMsg(pParse, "virtual tables may not be altered");
    goto exit_begin_add_column;
  }
#endif

  /* Make sure this is not an attempt to ALTER a view. */
  if( pTab->pSelect ){
    sqlite3ErrorMsg(pParse, "Cannot add a column to a view");
    goto exit_begin_add_column;
  }
  if( SQLITE_OK!=isSystemTable(pParse, pTab->zName) ){
    goto exit_begin_add_column;
  }

  assert( pTab->addColOffset>0 );
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  /* Put a copy of the Table struct in Parse.pNewTable for the
  ** sqlite3AddColumn() function and friends to modify.  But modify
  ** the name by adding an "sqlite_altertab_" prefix.  By adding this
  ** prefix, we insure that the name will not collide with an existing
  ** table because user table are not allowed to have the "sqlite_"
  ** prefix on their name.
  */
  pNew = (Table*)sqlite3DbMallocZero(db, sizeof(Table));
  if( !pNew ) goto exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nRef = 1;
  pNew->nCol = pTab->nCol;
  assert( pNew->nCol>0 );
  nAlloc = (((pNew->nCol-1)/8)*8)+8;
  assert( nAlloc>=pNew->nCol && nAlloc%8==0 && nAlloc-pNew->nCol<8 );
  pNew->aCol = (Column*)sqlite3DbMallocZero(db, sizeof(Column)*nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  if( !pNew->aCol || !pNew->zName ){
    db->mallocFailed = 1;
    goto exit_begin_add_column;
  }
  memcpy(pNew->aCol, pTab->aCol, sizeof(Column)*pNew->nCol);
  for(i=0; i<pNew->nCol; i++){
    Column *pCol = &pNew->aCol[i];
    pCol->zName = sqlite3DbStrDup(db, pCol->zName);
    pCol->zColl = 0;
    pCol->zType = 0;
    pCol->pDflt = 0;
    pCol->zDflt = 0;
  }
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->addColOffset = pTab->addColOffset;
  pNew->nRef = 1;

  /* Begin a transaction and increment the schema cookie.  */
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  v = sqlite3GetVdbe(pParse);
  if( !v ) goto exit_begin_add_column;
  sqlite3ChangeCookie(pParse, iDb);

exit_begin_add_column:
  sqlite3SrcListDelete(db, pSrc);
  return;
}
#endif  /* SQLITE_ALTER_TABLE */

/************** End of alter.c ***********************************************/
/************** Begin file analyze.c *****************************************/
/*
** 2005 July 8
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code associated with the ANALYZE command.
*/
#ifndef SQLITE_OMIT_ANALYZE

/*
** This routine generates code that opens the sqlite_stat1 table for
** writing with cursor iStatCur. If the library was built with the
** SQLITE_ENABLE_STAT2 macro defined, then the sqlite_stat2 table is
** opened for writing using cursor (iStatCur+1)
**
** If the sqlite_stat1 tables does not previously exist, it is created.
** Similarly, if the sqlite_stat2 table does not exist and the library
** is compiled with SQLITE_ENABLE_STAT2 defined, it is created. 
**
** Argument zWhere may be a pointer to a buffer containing a table name,
** or it may be a NULL pointer. If it is not NULL, then all entries in
** the sqlite_stat1 and (if applicable) sqlite_stat2 tables associated
** with the named table are deleted. If zWhere==0, then code is generated
** to delete all stat table entries.
*/
static void openStatTable(
  Parse *pParse,          /* Parsing context */
  int iDb,                /* The database we are looking in */
  int iStatCur,           /* Open the sqlite_stat1 table on this cursor */
  const char *zWhere,     /* Delete entries for this table or index */
  const char *zWhereType  /* Either "tbl" or "idx" */
){
  static const struct {
    const char *zName;
    const char *zCols;
  } aTable[] = {
    { "sqlite_stat1", "tbl,idx,stat" },
#ifdef SQLITE_ENABLE_STAT2
    { "sqlite_stat2", "tbl,idx,sampleno,sample" },
#endif
  };

  int aRoot[] = {0, 0};
  u8 aCreateTbl[] = {0, 0};

  int i;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  if( v==0 ) return;
  assert( sqlite3BtreeHoldsAllMutexes(db) );
  assert( sqlite3VdbeDb(v)==db );
  pDb = &db->aDb[iDb];

  for(i=0; i<ArraySize(aTable); i++){
    const char *zTab = aTable[i].zName;
    Table *pStat;
    if( (pStat = sqlite3FindTable(db, zTab, pDb->zName))==0 ){
      /* The sqlite_stat[12] table does not exist. Create it. Note that a 
      ** side-effect of the CREATE TABLE statement is to leave the rootpage 
      ** of the new table in register pParse->regRoot. This is important 
      ** because the OpenWrite opcode below will be needing it. */
      sqlite3NestedParse(pParse,
          "CREATE TABLE %Q.%s(%s)", pDb->zName, zTab, aTable[i].zCols
      );
      aRoot[i] = pParse->regRoot;
      aCreateTbl[i] = 1;
    }else{
      /* The table already exists. If zWhere is not NULL, delete all entries 
      ** associated with the table zWhere. If zWhere is NULL, delete the
      ** entire contents of the table. */
      aRoot[i] = pStat->tnum;
      sqlite3TableLock(pParse, iDb, aRoot[i], 1, zTab);
      if( zWhere ){
        sqlite3NestedParse(pParse,
           "DELETE FROM %Q.%s WHERE %s=%Q", pDb->zName, zTab, zWhereType, zWhere
        );
      }else{
        /* The sqlite_stat[12] table already exists.  Delete all rows. */
        sqlite3VdbeAddOp2(v, OP_Clear, aRoot[i], iDb);
      }
    }
  }

  /* Open the sqlite_stat[12] tables for writing. */
  for(i=0; i<ArraySize(aTable); i++){
    sqlite3VdbeAddOp3(v, OP_OpenWrite, iStatCur+i, aRoot[i], iDb);
    sqlite3VdbeChangeP4(v, -1, (char *)3, P4_INT32);
    sqlite3VdbeChangeP5(v, aCreateTbl[i]);
  }
}

/*
** Generate code to do an analysis of all indices associated with
** a single table.
*/
static void analyzeOneTable(
  Parse *pParse,   /* Parser context */
  Table *pTab,     /* Table whose indices are to be analyzed */
  Index *pOnlyIdx, /* If not NULL, only analyze this one index */
  int iStatCur,    /* Index of VdbeCursor that writes the sqlite_stat1 table */
  int iMem         /* Available memory locations begin here */
){
  sqlite3 *db = pParse->db;    /* Database handle */
  Index *pIdx;                 /* An index to being analyzed */
  int iIdxCur;                 /* Cursor open on index being analyzed */
  Vdbe *v;                     /* The virtual machine being built up */
  int i;                       /* Loop counter */
  int topOfLoop;               /* The top of the loop */
  int endOfLoop;               /* The end of the loop */
  int jZeroRows = -1;          /* Jump from here if number of rows is zero */
  int iDb;                     /* Index of database containing pTab */
  int regTabname = iMem++;     /* Register containing table name */
  int regIdxname = iMem++;     /* Register containing index name */
  int regSampleno = iMem++;    /* Register containing next sample number */
  int regCol = iMem++;         /* Content of a column analyzed table */
  int regRec = iMem++;         /* Register holding completed record */
  int regTemp = iMem++;        /* Temporary use register */
  int regRowid = iMem++;       /* Rowid for the inserted record */

#ifdef SQLITE_ENABLE_STAT2
  int addr = 0;                /* Instruction address */
  int regTemp2 = iMem++;       /* Temporary use register */
  int regSamplerecno = iMem++; /* Index of next sample to record */
  int regRecno = iMem++;       /* Current sample index */
  int regLast = iMem++;        /* Index of last sample to record */
  int regFirst = iMem++;       /* Index of first sample to record */
#endif

  v = sqlite3GetVdbe(pParse);
  if( v==0 || NEVER(pTab==0) ){
    return;
  }
  if( pTab->tnum==0 ){
    /* Do not gather statistics on views or virtual tables */
    return;
  }
  if( memcmp(pTab->zName, "sqlite_", 7)==0 ){
    /* Do not gather statistics on system tables */
    return;
  }
  assert( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  assert( iDb>=0 );
  assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
#ifndef SQLITE_OMIT_AUTHORIZATION
  if( sqlite3AuthCheck(pParse, SQLITE_ANALYZE, pTab->zName, 0,
      db->aDb[iDb].zName ) ){
    return;
  }
#endif

  /* Establish a read-lock on the table at the shared-cache level. */
  sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

  iIdxCur = pParse->nTab++;
  sqlite3VdbeAddOp4(v, OP_String8, 0, regTabname, 0, pTab->zName, 0);
  for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
    int nCol;
    KeyInfo *pKey;

    if( pOnlyIdx && pOnlyIdx!=pIdx ) continue;
    nCol = pIdx->nColumn;
    pKey = sqlite3IndexKeyinfo(pParse, pIdx);
    if( iMem+1+(nCol*2)>pParse->nMem ){
      pParse->nMem = iMem+1+(nCol*2);
    }

    /* Open a cursor to the index to be analyzed. */
    assert( iDb==sqlite3SchemaToIndex(db, pIdx->pSchema) );
    sqlite3VdbeAddOp4(v, OP_OpenRead, iIdxCur, pIdx->tnum, iDb,
        (char *)pKey, P4_KEYINFO_HANDOFF);
    VdbeComment((v, "%s", pIdx->zName));

    /* Populate the register containing the index name. */
    sqlite3VdbeAddOp4(v, OP_String8, 0, regIdxname, 0, pIdx->zName, 0);

#ifdef SQLITE_ENABLE_STAT2

    /* If this iteration of the loop is generating code to analyze the
    ** first index in the pTab->pIndex list, then register regLast has
    ** not been populated. In this case populate it now.  */
    if( pTab->pIndex==pIdx ){
      sqlite3VdbeAddOp2(v, OP_Integer, SQLITE_INDEX_SAMPLES, regSamplerecno);
      sqlite3VdbeAddOp2(v, OP_Integer, SQLITE_INDEX_SAMPLES*2-1, regTemp);
      sqlite3VdbeAddOp2(v, OP_Integer, SQLITE_INDEX_SAMPLES*2, regTemp2);

      sqlite3VdbeAddOp2(v, OP_Count, iIdxCur, regLast);
      sqlite3VdbeAddOp2(v, OP_Null, 0, regFirst);
      addr = sqlite3VdbeAddOp3(v, OP_Lt, regSamplerecno, 0, regLast);
      sqlite3VdbeAddOp3(v, OP_Divide, regTemp2, regLast, regFirst);
      sqlite3VdbeAddOp3(v, OP_Multiply, regLast, regTemp, regLast);
      sqlite3VdbeAddOp2(v, OP_AddImm, regLast, SQLITE_INDEX_SAMPLES*2-2);
      sqlite3VdbeAddOp3(v, OP_Divide,  regTemp2, regLast, regLast);
      sqlite3VdbeJumpHere(v, addr);
    }

    /* Zero the regSampleno and regRecno registers. */
    sqlite3VdbeAddOp2(v, OP_Integer, 0, regSampleno);
    sqlite3VdbeAddOp2(v, OP_Integer, 0, regRecno);
    sqlite3VdbeAddOp2(v, OP_Copy, regFirst, regSamplerecno);
#endif

    /* The block of memory cells initialized here is used as follows.
    **
    **    iMem:                
    **        The total number of rows in the table.
    **
    **    iMem+1 .. iMem+nCol: 
    **        Number of distinct entries in index considering the 
    **        left-most N columns only, where N is between 1 and nCol, 
    **        inclusive.
    **
    **    iMem+nCol+1 .. Mem+2*nCol:  
    **        Previous value of indexed columns, from left to right.
    **
    ** Cells iMem through iMem+nCol are initialized to 0. The others are 
    ** initialized to contain an SQL NULL.
    */
    for(i=0; i<=nCol; i++){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, iMem+i);
    }
    for(i=0; i<nCol; i++){
      sqlite3VdbeAddOp2(v, OP_Null, 0, iMem+nCol+i+1);
    }

    /* Start the analysis loop. This loop runs through all the entries in
    ** the index b-tree.  */
    endOfLoop = sqlite3VdbeMakeLabel(v);
    sqlite3VdbeAddOp2(v, OP_Rewind, iIdxCur, endOfLoop);
    topOfLoop = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp2(v, OP_AddImm, iMem, 1);

    for(i=0; i<nCol; i++){
      CollSeq *pColl;
      sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, i, regCol);
      if( i==0 ){
#ifdef SQLITE_ENABLE_STAT2
        /* Check if the record that cursor iIdxCur points to contains a
        ** value that should be stored in the sqlite_stat2 table. If so,
        ** store it.  */
        int ne = sqlite3VdbeAddOp3(v, OP_Ne, regRecno, 0, regSamplerecno);
        assert( regTabname+1==regIdxname 
             && regTabname+2==regSampleno
             && regTabname+3==regCol
        );
        sqlite3VdbeChangeP5(v, SQLITE_JUMPIFNULL);
        sqlite3VdbeAddOp4(v, OP_MakeRecord, regTabname, 4, regRec, "aaab", 0);
        sqlite3VdbeAddOp2(v, OP_NewRowid, iStatCur+1, regRowid);
        sqlite3VdbeAddOp3(v, OP_Insert, iStatCur+1, regRec, regRowid);

        /* Calculate new values for regSamplerecno and regSampleno.
        **
        **   sampleno = sampleno + 1
        **   samplerecno = samplerecno+(remaining records)/(remaining samples)
        */
        sqlite3VdbeAddOp2(v, OP_AddImm, regSampleno, 1);
        sqlite3VdbeAddOp3(v, OP_Subtract, regRecno, regLast, regTemp);
        sqlite3VdbeAddOp2(v, OP_AddImm, regTemp, -1);
        sqlite3VdbeAddOp2(v, OP_Integer, SQLITE_INDEX_SAMPLES, regTemp2);
        sqlite3VdbeAddOp3(v, OP_Subtract, regSampleno, regTemp2, regTemp2);
        sqlite3VdbeAddOp3(v, OP_Divide, regTemp2, regTemp, regTemp);
        sqlite3VdbeAddOp3(v, OP_Add, regSamplerecno, regTemp, regSamplerecno);

        sqlite3VdbeJumpHere(v, ne);
        sqlite3VdbeAddOp2(v, OP_AddImm, regRecno, 1);
#endif

        /* Always record the very first row */
        sqlite3VdbeAddOp1(v, OP_IfNot, iMem+1);
      }
      assert( pIdx->azColl!=0 );
      assert( pIdx->azColl[i]!=0 );
      pColl = sqlite3LocateCollSeq(pParse, pIdx->azColl[i]);
      sqlite3VdbeAddOp4(v, OP_Ne, regCol, 0, iMem+nCol+i+1,
                       (char*)pColl, P4_COLLSEQ);
      sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
    }
    if( db->mallocFailed ){
      /* If a malloc failure has occurred, then the result of the expression 
      ** passed as the second argument to the call to sqlite3VdbeJumpHere() 
      ** below may be negative. Which causes an assert() to fail (or an
      ** out-of-bounds write if SQLITE_DEBUG is not defined).  */
      return;
    }
    sqlite3VdbeAddOp2(v, OP_Goto, 0, endOfLoop);
    for(i=0; i<nCol; i++){
      int addr2 = sqlite3VdbeCurrentAddr(v) - (nCol*2);
      if( i==0 ){
        sqlite3VdbeJumpHere(v, addr2-1);  /* Set jump dest for the OP_IfNot */
      }
      sqlite3VdbeJumpHere(v, addr2);      /* Set jump dest for the OP_Ne */
      sqlite3VdbeAddOp2(v, OP_AddImm, iMem+i+1, 1);
      sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, i, iMem+nCol+i+1);
    }

    /* End of the analysis loop. */
    sqlite3VdbeResolveLabel(v, endOfLoop);
    sqlite3VdbeAddOp2(v, OP_Next, iIdxCur, topOfLoop);
    sqlite3VdbeAddOp1(v, OP_Close, iIdxCur);

    /* Store the results in sqlite_stat1.
    **
    ** The result is a single row of the sqlite_stat1 table.  The first
    ** two columns are the names of the table and index.  The third column
    ** is a string composed of a list of integer statistics about the
    ** index.  The first integer in the list is the total number of entries
    ** in the index.  There is one additional integer in the list for each
    ** column of the table.  This additional integer is a guess of how many
    ** rows of the table the index will select.  If D is the count of distinct
    ** values and K is the total number of rows, then the integer is computed
    ** as:
    **
    **        I = (K+D-1)/D
    **
    ** If K==0 then no entry is made into the sqlite_stat1 table.  
    ** If K>0 then it is always the case the D>0 so division by zero
    ** is never possible.
    */
    sqlite3VdbeAddOp2(v, OP_SCopy, iMem, regSampleno);
    if( jZeroRows<0 ){
      jZeroRows = sqlite3VdbeAddOp1(v, OP_IfNot, iMem);
    }
    for(i=0; i<nCol; i++){
      sqlite3VdbeAddOp4(v, OP_String8, 0, regTemp, 0, " ", 0);
      sqlite3VdbeAddOp3(v, OP_Concat, regTemp, regSampleno, regSampleno);
      sqlite3VdbeAddOp3(v, OP_Add, iMem, iMem+i+1, regTemp);
      sqlite3VdbeAddOp2(v, OP_AddImm, regTemp, -1);
      sqlite3VdbeAddOp3(v, OP_Divide, iMem+i+1, regTemp, regTemp);
      sqlite3VdbeAddOp1(v, OP_ToInt, regTemp);
      sqlite3VdbeAddOp3(v, OP_Concat, regTemp, regSampleno, regSampleno);
    }
    sqlite3VdbeAddOp4(v, OP_MakeRecord, regTabname, 3, regRec, "aaa", 0);
    sqlite3VdbeAddOp2(v, OP_NewRowid, iStatCur, regRowid);
    sqlite3VdbeAddOp3(v, OP_Insert, iStatCur, regRec, regRowid);
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
  }

  /* If the table has no indices, create a single sqlite_stat1 entry
  ** containing NULL as the index name and the row count as the content.
  */
  if( pTab->pIndex==0 ){
    sqlite3VdbeAddOp3(v, OP_OpenRead, iIdxCur, pTab->tnum, iDb);
    VdbeComment((v, "%s", pTab->zName));
    sqlite3VdbeAddOp2(v, OP_Count, iIdxCur, regSampleno);
    sqlite3VdbeAddOp1(v, OP_Close, iIdxCur);
    jZeroRows = sqlite3VdbeAddOp1(v, OP_IfNot, regSampleno);
  }else{
    sqlite3VdbeJumpHere(v, jZeroRows);
    jZeroRows = sqlite3VdbeAddOp0(v, OP_Goto);
  }
  sqlite3VdbeAddOp2(v, OP_Null, 0, regIdxname);
  sqlite3VdbeAddOp4(v, OP_MakeRecord, regTabname, 3, regRec, "aaa", 0);
  sqlite3VdbeAddOp2(v, OP_NewRowid, iStatCur, regRowid);
  sqlite3VdbeAddOp3(v, OP_Insert, iStatCur, regRec, regRowid);
  sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
  if( pParse->nMem<regRec ) pParse->nMem = regRec;
  sqlite3VdbeJumpHere(v, jZeroRows);
}

/*
** Generate code that will cause the most recent index analysis to
** be loaded into internal hash tables where is can be used.
*/
static void loadAnalysis(Parse *pParse, int iDb){
  Vdbe *v = sqlite3GetVdbe(pParse);
  if( v ){
    sqlite3VdbeAddOp1(v, OP_LoadAnalysis, iDb);
  }
}

/*
** Generate code that will do an analysis of an entire database
*/
static void analyzeDatabase(Parse *pParse, int iDb){
  sqlite3 *db = pParse->db;
  Schema *pSchema = db->aDb[iDb].pSchema;    /* Schema of database iDb */
  HashElem *k;
  int iStatCur;
  int iMem;

  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 2;
  openStatTable(pParse, iDb, iStatCur, 0, 0);
  iMem = pParse->nMem+1;
  assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
  for(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
    Table *pTab = (Table*)sqliteHashData(k);
    analyzeOneTable(pParse, pTab, 0, iStatCur, iMem);
  }
  loadAnalysis(pParse, iDb);
}

/*
** Generate code that will do an analysis of a single table in
** a database.  If pOnlyIdx is not NULL then it is a single index
** in pTab that should be analyzed.
*/
static void analyzeTable(Parse *pParse, Table *pTab, Index *pOnlyIdx){
  int iDb;
  int iStatCur;

  assert( pTab!=0 );
  assert( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 2;
  if( pOnlyIdx ){
    openStatTable(pParse, iDb, iStatCur, pOnlyIdx->zName, "idx");
  }else{
    openStatTable(pParse, iDb, iStatCur, pTab->zName, "tbl");
  }
  analyzeOneTable(pParse, pTab, pOnlyIdx, iStatCur, pParse->nMem+1);
  loadAnalysis(pParse, iDb);
}

/*
** Generate code for the ANALYZE command.  The parser calls this routine
** when it recognizes an ANALYZE command.
**
**        ANALYZE                            -- 1
**        ANALYZE  <database>                -- 2
**        ANALYZE  ?<database>.?<tablename>  -- 3
**
** Form 1 causes all indices in all attached databases to be analyzed.
** Form 2 analyzes all indices the single database named.
** Form 3 analyzes all indices associated with the named table.
*/
SQLITE_PRIVATE void sqlite3Analyze(Parse *pParse, Token *pName1, Token *pName2){
  sqlite3 *db = pParse->db;
  int iDb;
  int i;
  char *z, *zDb;
  Table *pTab;
  Index *pIdx;
  Token *pTableName;

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  assert( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  if( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    return;
  }

  assert( pName2!=0 || pName1==0 );
  if( pName1==0 ){
    /* Form 1:  Analyze everything */
    for(i=0; i<db->nDb; i++){
      if( i==1 ) continue;  /* Do not analyze the TEMP database */
      analyzeDatabase(pParse, i);
    }
  }else if( pName2->n==0 ){
    /* Form 2:  Analyze the database or table named */
    iDb = sqlite3FindDb(db, pName1);
    if( iDb>=0 ){
      analyzeDatabase(pParse, iDb);
    }else{
      z = sqlite3NameFromToken(db, pName1);
      if( z ){
        if( (pIdx = sqlite3FindIndex(db, z, 0))!=0 ){
          analyzeTable(pParse, pIdx->pTable, pIdx);
        }else if( (pTab = sqlite3LocateTable(pParse, 0, z, 0))!=0 ){
          analyzeTable(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }
  }else{
    /* Form 3: Analyze the fully qualified table name */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pTableName);
    if( iDb>=0 ){
      zDb = db->aDb[iDb].zName;
      z = sqlite3NameFromToken(db, pTableName);
      if( z ){
        if( (pIdx = sqlite3FindIndex(db, z, zDb))!=0 ){
          analyzeTable(pParse, pIdx->pTable, pIdx);
        }else if( (pTab = sqlite3LocateTable(pParse, 0, z, zDb))!=0 ){
          analyzeTable(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }   
  }
}

/*
** Used to pass information from the analyzer reader through to the
** callback routine.
*/
typedef struct analysisInfo analysisInfo;
struct analysisInfo {
  sqlite3 *db;
  const char *zDatabase;
};

/*
** This callback is invoked once for each index when reading the
** sqlite_stat1 table.  
**
**     argv[0] = name of the table
**     argv[1] = name of the index (might be NULL)
**     argv[2] = results of analysis - on integer for each column
**
** Entries for which argv[1]==NULL simply record the number of rows in
** the table.
*/
static int analysisLoader(void *pData, int argc, char **argv, char **NotUsed){
  analysisInfo *pInfo = (analysisInfo*)pData;
  Index *pIndex;
  Table *pTable;
  int i, c, n;
  unsigned int v;
  const char *z;

  assert( argc==3 );
  UNUSED_PARAMETER2(NotUsed, argc);

  if( argv==0 || argv[0]==0 || argv[2]==0 ){
    return 0;
  }
  pTable = sqlite3FindTable(pInfo->db, argv[0], pInfo->zDatabase);
  if( pTable==0 ){
    return 0;
  }
  if( argv[1] ){
    pIndex = sqlite3FindIndex(pInfo->db, argv[1], pInfo->zDatabase);
  }else{
    pIndex = 0;
  }
  n = pIndex ? pIndex->nColumn : 0;
  z = argv[2];
  for(i=0; *z && i<=n; i++){
    v = 0;
    while( (c=z[0])>='0' && c<='9' ){
      v = v*10 + c - '0';
      z++;
    }
    if( i==0 ) pTable->nRowEst = v;
    if( pIndex==0 ) break;
    pIndex->aiRowEst[i] = v;
    if( *z==' ' ) z++;
    if( memcmp(z, "unordered", 10)==0 ){
      pIndex->bUnordered = 1;
      break;
    }
  }
  return 0;
}

/*
** If the Index.aSample variable is not NULL, delete the aSample[] array
** and its contents.
*/
SQLITE_PRIVATE void sqlite3DeleteIndexSamples(sqlite3 *db, Index *pIdx){
#ifdef SQLITE_ENABLE_STAT2
  if( pIdx->aSample ){
    int j;
    for(j=0; j<SQLITE_INDEX_SAMPLES; j++){
      IndexSample *p = &pIdx->aSample[j];
      if( p->eType==SQLITE_TEXT || p->eType==SQLITE_BLOB ){
        sqlite3DbFree(db, p->u.z);
      }
    }
    sqlite3DbFree(db, pIdx->aSample);
  }
#else
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(pIdx);
#endif
}

/*
** Load the content of the sqlite_stat1 and sqlite_stat2 tables. The
** contents of sqlite_stat1 are used to populate the Index.aiRowEst[]
** arrays. The contents of sqlite_stat2 are used to populate the
** Index.aSample[] arrays.
**
** If the sqlite_stat1 table is not present in the database, SQLITE_ERROR
** is returned. In this case, even if SQLITE_ENABLE_STAT2 was defined 
** during compilation and the sqlite_stat2 table is present, no data is 
** read from it.
**
** If SQLITE_ENABLE_STAT2 was defined during compilation and the 
** sqlite_stat2 table is not present in the database, SQLITE_ERROR is
** returned. However, in this case, data is read from the sqlite_stat1
** table (if it is present) before returning.
**
** If an OOM error occurs, this function always sets db->mallocFailed.
** This means if the caller does not care about other errors, the return
** code may be ignored.
*/
SQLITE_PRIVATE int sqlite3AnalysisLoad(sqlite3 *db, int iDb){
  analysisInfo sInfo;
  HashElem *i;
  char *zSql;
  int rc;

  assert( iDb>=0 && iDb<db->nDb );
  assert( db->aDb[iDb].pBt!=0 );

  /* Clear any prior statistics */
  assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
  for(i=sqliteHashFirst(&db->aDb[iDb].pSchema->idxHash);i;i=sqliteHashNext(i)){
    Index *pIdx = sqliteHashData(i);
    sqlite3DefaultRowEst(pIdx);
    sqlite3DeleteIndexSamples(db, pIdx);
    pIdx->aSample = 0;
  }

  /* Check to make sure the sqlite_stat1 table exists */
  sInfo.db = db;
  sInfo.zDatabase = db->aDb[iDb].zName;
  if( sqlite3FindTable(db, "sqlite_stat1", sInfo.zDatabase)==0 ){
    return SQLITE_ERROR;
  }

  /* Load new statistics out of the sqlite_stat1 table */
  zSql = sqlite3MPrintf(db, 
      "SELECT tbl, idx, stat FROM %Q.sqlite_stat1", sInfo.zDatabase);
  if( zSql==0 ){
    rc = SQLITE_NOMEM;
  }else{
    rc = sqlite3_exec(db, zSql, analysisLoader, &sInfo, 0);
    sqlite3DbFree(db, zSql);
  }


  /* Load the statistics from the sqlite_stat2 table. */
#ifdef SQLITE_ENABLE_STAT2
  if( rc==SQLITE_OK && !sqlite3FindTable(db, "sqlite_stat2", sInfo.zDatabase) ){
    rc = SQLITE_ERROR;
  }
  if( rc==SQLITE_OK ){
    sqlite3_stmt *pStmt = 0;

    zSql = sqlite3MPrintf(db, 
        "SELECT idx,sampleno,sample FROM %Q.sqlite_stat2", sInfo.zDatabase);
    if( !zSql ){
      rc = SQLITE_NOMEM;
    }else{
      rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
      sqlite3DbFree(db, zSql);
    }

    if( rc==SQLITE_OK ){
      while( sqlite3_step(pStmt)==SQLITE_ROW ){
        char *zIndex;   /* Index name */
        Index *pIdx;    /* Pointer to the index object */

        zIndex = (char *)sqlite3_column_text(pStmt, 0);
        pIdx = zIndex ? sqlite3FindIndex(db, zIndex, sInfo.zDatabase) : 0;
        if( pIdx ){
          int iSample = sqlite3_column_int(pStmt, 1);
          if( iSample<SQLITE_INDEX_SAMPLES && iSample>=0 ){
            int eType = sqlite3_column_type(pStmt, 2);

            if( pIdx->aSample==0 ){
              static const int sz = sizeof(IndexSample)*SQLITE_INDEX_SAMPLES;
              pIdx->aSample = (IndexSample *)sqlite3DbMallocRaw(0, sz);
              if( pIdx->aSample==0 ){
                db->mallocFailed = 1;
                break;
              }
	      memset(pIdx->aSample, 0, sz);
            }

            assert( pIdx->aSample );
            {
              IndexSample *pSample = &pIdx->aSample[iSample];
              pSample->eType = (u8)eType;
              if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
                pSample->u.r = sqlite3_column_double(pStmt, 2);
              }else if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
                const char *z = (const char *)(
                    (eType==SQLITE_BLOB) ?
                    sqlite3_column_blob(pStmt, 2):
                    sqlite3_column_text(pStmt, 2)
                );
                int n = sqlite3_column_bytes(pStmt, 2);
                if( n>24 ){
                  n = 24;
                }
                pSample->nByte = (u8)n;
                if( n < 1){
                  pSample->u.z = 0;
                }else{
                  pSample->u.z = sqlite3DbStrNDup(0, z, n);
                  if( pSample->u.z==0 ){
                    db->mallocFailed = 1;
                    break;
                  }
                }
              }
            }
          }
        }
      }
      rc = sqlite3_finalize(pStmt);
    }
  }
#endif

  if( rc==SQLITE_NOMEM ){
    db->mallocFailed = 1;
  }
  return rc;
}


#endif /* SQLITE_OMIT_ANALYZE */

/************** End of analyze.c *********************************************/
/************** Begin file attach.c ******************************************/
/*
** 2003 April 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used to implement the ATTACH and DETACH commands.
*/

#ifndef SQLITE_OMIT_ATTACH
/*
** Resolve an expression that was part of an ATTACH or DETACH statement. This
** is slightly different from resolving a normal SQL expression, because simple
** identifiers are treated as strings, not possible column names or aliases.
**
** i.e. if the parser sees:
**
**     ATTACH DATABASE abc AS def
**
** it treats the two expressions as literal strings 'abc' and 'def' instead of
** looking for columns of the same name.
**
** This only applies to the root node of pExpr, so the statement:
**
**     ATTACH DATABASE abc||def AS 'db2'
**
** will fail because neither abc or def can be resolved.
*/
static int resolveAttachExpr(NameContext *pName, Expr *pExpr)
{
  int rc = SQLITE_OK;
  if( pExpr ){
    if( pExpr->op!=TK_ID ){
      rc = sqlite3ResolveExprNames(pName, pExpr);
      if( rc==SQLITE_OK && !sqlite3ExprIsConstant(pExpr) ){
        sqlite3ErrorMsg(pName->pParse, "invalid name: \"%s\"", pExpr->u.zToken);
        return SQLITE_ERROR;
      }
    }else{
      pExpr->op = TK_STRING;
    }
  }
  return rc;
}

/*
** An SQL user-function registered to do the work of an ATTACH statement. The
** three arguments to the function come directly from an attach statement:
**
**     ATTACH DATABASE x AS y KEY z
**
**     SELECT sqlite_attach(x, y, z)
**
** If the optional "KEY z" syntax is omitted, an SQL NULL is passed as the
** third argument.
*/
static void attachFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **argv
){
  int i;
  int rc = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);
  const char *zName;
  const char *zFile;
  char *zPath = 0;
  char *zErr = 0;
  unsigned int flags;
  Db *aNew;
  char *zErrDyn = 0;
  sqlite3_vfs *pVfs;

  UNUSED_PARAMETER(NotUsed);

  zFile = (const char *)sqlite3_value_text(argv[0]);
  zName = (const char *)sqlite3_value_text(argv[1]);
  if( zFile==0 ) zFile = "";
  if( zName==0 ) zName = "";

  /* Check for the following errors:
  **
  **     * Too many attached databases,
  **     * Transaction currently open
  **     * Specified database name already being used.
  */
  if( db->nDb>=db->aLimit[SQLITE_LIMIT_ATTACHED]+2 ){
    zErrDyn = sqlite3MPrintf(db, "too many attached databases - max %d", 
      db->aLimit[SQLITE_LIMIT_ATTACHED]
    );
    goto attach_error;
  }
  if( !db->autoCommit ){
    zErrDyn = sqlite3MPrintf(db, "cannot ATTACH database within transaction");
    goto attach_error;
  }
  for(i=0; i<db->nDb; i++){
    char *z = db->aDb[i].zName;
    assert( z && zName );
    if( sqlite3StrICmp(z, zName)==0 ){
      zErrDyn = sqlite3MPrintf(db, "database %s is already in use", zName);
      goto attach_error;
    }
  }

  /* Allocate the new entry in the db->aDb[] array and initialise the schema
  ** hash tables.
  */
  if( db->aDb==db->aDbStatic ){
    aNew = sqlite3DbMallocRaw(db, sizeof(db->aDb[0])*3 );
    if( aNew==0 ) return;
    memcpy(aNew, db->aDb, sizeof(db->aDb[0])*2);
  }else{
    aNew = sqlite3DbRealloc(db, db->aDb, sizeof(db->aDb[0])*(db->nDb+1) );
    if( aNew==0 ) return;
  }
  db->aDb = aNew;
  aNew = &db->aDb[db->nDb];
  memset(aNew, 0, sizeof(*aNew));

  /* Open the database file. If the btree is successfully opened, use
  ** it to obtain the database schema. At this point the schema may
  ** or may not be initialised.
  */
  flags = db->openFlags;
  rc = sqlite3ParseUri(db->pVfs->zName, zFile, &flags, &pVfs, &zPath, &zErr);
  if( rc!=SQLITE_OK ){
    if( rc==SQLITE_NOMEM ) db->mallocFailed = 1;
    sqlite3_result_error(context, zErr, -1);
    sqlite3_free(zErr);
    return;
  }
  assert( pVfs );
  flags |= SQLITE_OPEN_MAIN_DB;
  rc = sqlite3BtreeOpen(pVfs, zPath, db, &aNew->pBt, 0, flags);
  sqlite3_free( zPath );
  db->nDb++;
  if( rc==SQLITE_CONSTRAINT ){
    rc = SQLITE_ERROR;
    zErrDyn = sqlite3MPrintf(db, "database is already attached");
  }else if( rc==SQLITE_OK ){
    Pager *pPager;
    aNew->pSchema = sqlite3SchemaGet(db, aNew->pBt);
    if( !aNew->pSchema ){
      rc = SQLITE_NOMEM;
    }else if( aNew->pSchema->file_format && aNew->pSchema->enc!=ENC(db) ){
      zErrDyn = sqlite3MPrintf(db, 
        "attached databases must use the same text encoding as main database");
      rc = SQLITE_ERROR;
    }
    pPager = sqlite3BtreePager(aNew->pBt);
    sqlite3PagerLockingMode(pPager, db->dfltLockMode);
    sqlite3BtreeSecureDelete(aNew->pBt,
                             sqlite3BtreeSecureDelete(db->aDb[0].pBt,-1) );
  }
  aNew->safety_level = 3;
  aNew->zName = sqlite3DbStrDup(db, zName);
  if( rc==SQLITE_OK && aNew->zName==0 ){
    rc = SQLITE_NOMEM;
  }


#ifdef SQLITE_HAS_CODEC
  if( rc==SQLITE_OK ){
    extern int sqlite3CodecAttach(sqlite3*, int, const void*, int);
    extern void sqlite3CodecGetKey(sqlite3*, int, void**, int*);
    int nKey;
    char *zKey;
    int t = sqlite3_value_type(argv[2]);
    switch( t ){
      case SQLITE_INTEGER:
      case SQLITE_FLOAT:
        zErrDyn = sqlite3DbStrDup(db, "Invalid key value");
        rc = SQLITE_ERROR;
        break;
        
      case SQLITE_TEXT:
      case SQLITE_BLOB:
        nKey = sqlite3_value_bytes(argv[2]);
        zKey = (char *)sqlite3_value_blob(argv[2]);
        rc = sqlite3CodecAttach(db, db->nDb-1, zKey, nKey);
        break;

      case SQLITE_NULL:
        /* No key specified.  Use the key from the main database */
        sqlite3CodecGetKey(db, 0, (void**)&zKey, &nKey);
        if( nKey>0 || sqlite3BtreeGetReserve(db->aDb[0].pBt)>0 ){
          rc = sqlite3CodecAttach(db, db->nDb-1, zKey, nKey);
        }
        break;
    }
  }
#endif

  /* If the file was opened successfully, read the schema for the new database.
  ** If this fails, or if opening the file failed, then close the file and 
  ** remove the entry from the db->aDb[] array. i.e. put everything back the way
  ** we found it.
  */
  if( rc==SQLITE_OK ){
    sqlite3BtreeEnterAll(db);
    rc = sqlite3Init(db, &zErrDyn);
    sqlite3BtreeLeaveAll(db);
  }
  if( rc ){
    int iDb = db->nDb - 1;
    assert( iDb>=2 );
    if( db->aDb[iDb].pBt ){
      sqlite3BtreeClose(db->aDb[iDb].pBt);
      db->aDb[iDb].pBt = 0;
      db->aDb[iDb].pSchema = 0;
    }
    sqlite3ResetInternalSchema(db, -1);
    db->nDb = iDb;
    if( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ){
      db->mallocFailed = 1;
      sqlite3DbFree(db, zErrDyn);
      zErrDyn = sqlite3MPrintf(db, "out of memory");
    }else if( zErrDyn==0 ){
      zErrDyn = sqlite3MPrintf(db, "unable to open database: %s", zFile);
    }
    goto attach_error;
  }
  
  return;

attach_error:
  /* Return an error if we get here */
  if( zErrDyn ){
    sqlite3_result_error(context, zErrDyn, -1);
    sqlite3DbFree(db, zErrDyn);
  }
  if( rc ) sqlite3_result_error_code(context, rc);
}

/*
** An SQL user-function registered to do the work of an DETACH statement. The
** three arguments to the function come directly from a detach statement:
**
**     DETACH DATABASE x
**
**     SELECT sqlite_detach(x)
*/
static void detachFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **argv
){
  const char *zName = (const char *)sqlite3_value_text(argv[0]);
  sqlite3 *db = sqlite3_context_db_handle(context);
  int i;
  Db *pDb = 0;
  char zErr[128];

  UNUSED_PARAMETER(NotUsed);

  if( zName==0 ) zName = "";
  for(i=0; i<db->nDb; i++){
    pDb = &db->aDb[i];
    if( pDb->pBt==0 ) continue;
    if( sqlite3StrICmp(pDb->zName, zName)==0 ) break;
  }

  if( i>=db->nDb ){
    sqlite3_snprintf(sizeof(zErr),zErr, "no such database: %s", zName);
    goto detach_error;
  }
  if( i<2 ){
    sqlite3_snprintf(sizeof(zErr),zErr, "cannot detach database %s", zName);
    goto detach_error;
  }
  if( !db->autoCommit ){
    sqlite3_snprintf(sizeof(zErr), zErr,
                     "cannot DETACH database within transaction");
    goto detach_error;
  }
  if( sqlite3BtreeIsInReadTrans(pDb->pBt) || sqlite3BtreeIsInBackup(pDb->pBt) ){
    sqlite3_snprintf(sizeof(zErr),zErr, "database %s is locked", zName);
    goto detach_error;
  }

  sqlite3BtreeClose(pDb->pBt);
  pDb->pBt = 0;
  pDb->pSchema = 0;
  sqlite3ResetInternalSchema(db, -1);
  return;

detach_error:
  sqlite3_result_error(context, zErr, -1);
}

/*
** This procedure generates VDBE code for a single invocation of either the
** sqlite_detach() or sqlite_attach() SQL user functions.
*/
static void codeAttach(
  Parse *pParse,       /* The parser context */
  int type,            /* Either SQLITE_ATTACH or SQLITE_DETACH */
  FuncDef const *pFunc,/* FuncDef wrapper for detachFunc() or attachFunc() */
  Expr *pAuthArg,      /* Expression to pass to authorization callback */
  Expr *pFilename,     /* Name of database file */
  Expr *pDbname,       /* Name of the database to use internally */
  Expr *pKey           /* Database key for encryption extension */
){
  int rc;
  NameContext sName;
  Vdbe *v;
  sqlite3* db = pParse->db;
  int regArgs;

  memset(&sName, 0, sizeof(NameContext));
  sName.pParse = pParse;

  if( 
      SQLITE_OK!=(rc = resolveAttachExpr(&sName, pFilename)) ||
      SQLITE_OK!=(rc = resolveAttachExpr(&sName, pDbname)) ||
      SQLITE_OK!=(rc = resolveAttachExpr(&sName, pKey))
  ){
    pParse->nErr++;
    goto attach_end;
  }

#ifndef SQLITE_OMIT_AUTHORIZATION
  if( pAuthArg ){
    char *zAuthArg;
    if( pAuthArg->op==TK_STRING ){
      zAuthArg = pAuthArg->u.zToken;
    }else{
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    if(rc!=SQLITE_OK ){
      goto attach_end;
    }
  }
#endif /* SQLITE_OMIT_AUTHORIZATION */


  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3GetTempRange(pParse, 4);
  sqlite3ExprCode(pParse, pFilename, regArgs);
  sqlite3ExprCode(pParse, pDbname, regArgs+1);
  sqlite3ExprCode(pParse, pKey, regArgs+2);

  assert( v || db->mallocFailed );
  if( v ){
    sqlite3VdbeAddOp3(v, OP_Function, 0, regArgs+3-pFunc->nArg, regArgs+3);
    assert( pFunc->nArg==-1 || (pFunc->nArg&0xff)==pFunc->nArg );
    sqlite3VdbeChangeP5(v, (u8)(pFunc->nArg));
    sqlite3VdbeChangeP4(v, -1, (char *)pFunc, P4_FUNCDEF);

    /* Code an OP_Expire. For an ATTACH statement, set P1 to true (expire this
    ** statement only). For DETACH, set it to false (expire all existing
    ** statements).
    */
    sqlite3VdbeAddOp1(v, OP_Expire, (type==SQLITE_ATTACH));
  }
  
attach_end:
  sqlite3ExprDelete(db, pFilename);
  sqlite3ExprDelete(db, pDbname);
  sqlite3ExprDelete(db, pKey);
}

/*
** Called by the parser to compile a DETACH statement.
**
**     DETACH pDbname
*/
SQLITE_PRIVATE void sqlite3Detach(Parse *pParse, Expr *pDbname){
  static const FuncDef detach_func = {
    1,                /* nArg */
    SQLITE_UTF8,      /* iPrefEnc */
    0,                /* flags */
    0,                /* pUserData */
    0,                /* pNext */
    detachFunc,       /* xFunc */
    0,                /* xStep */
    0,                /* xFinalize */
    "sqlite_detach",  /* zName */
    0,                /* pHash */
    0                 /* pDestructor */
  };
  codeAttach(pParse, SQLITE_DETACH, &detach_func, pDbname, 0, 0, pDbname);
}

/*
** Called by the parser to compile an ATTACH statement.
**
**     ATTACH p AS pDbname KEY pKey
*/
SQLITE_PRIVATE void sqlite3Attach(Parse *pParse, Expr *p, Expr *pDbname, Expr *pKey){
  static const FuncDef attach_func = {
    3,                /* nArg */
    SQLITE_UTF8,      /* iPrefEnc */
    0,                /* flags */
    0,                /* pUserData */
    0,                /* pNext */
    attachFunc,       /* xFunc */
    0,                /* xStep */
    0,                /* xFinalize */
    "sqlite_attach",  /* zName */
    0,                /* pHash */
    0                 /* pDestructor */
  };
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, pKey);
}
#endif /* SQLITE_OMIT_ATTACH */

/*
** Initialize a DbFixer structure.  This routine must be called prior
** to passing the structure to one of the sqliteFixAAAA() routines below.
**
** The return value indicates whether or not fixation is required.  TRUE
** means we do need to fix the database references, FALSE means we do not.
*/
SQLITE_PRIVATE int sqlite3FixInit(
  DbFixer *pFix,      /* The fixer to be initialized */
  Parse *pParse,      /* Error messages will be written here */
  int iDb,            /* This is the database that must be used */
  const char *zType,  /* "view", "trigger", or "index" */
  const Token *pName  /* Name of the view, trigger, or index */
){
  sqlite3 *db;

  if( NEVER(iDb<0) || iDb==1 ) return 0;
  db = pParse->db;
  assert( db->nDb>iDb );
  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zName;
  pFix->zType = zType;
  pFix->pName = pName;
  return 1;
}

/*
** The following set of routines walk through the parse tree and assign
** a specific database to all table references where the database name
** was left unspecified in the original SQL statement.  The pFix structure
** must have been initialized by a prior call to sqlite3FixInit().
**
** These routines are used to make sure that an index, trigger, or
** view in one database does not refer to objects in a different database.
** (Exception: indices, triggers, and views in the TEMP database are
** allowed to refer to anything.)  If a reference is explicitly made
** to an object in a different database, an error message is added to
** pParse->zErrMsg and these routines return non-zero.  If everything
** checks out, these routines return 0.
*/
SQLITE_PRIVATE int sqlite3FixSrcList(
  DbFixer *pFix,       /* Context of the fixation */
  SrcList *pList       /* The Source list to check and modify */
){
  int i;
  const char *zDb;
  struct SrcList_item *pItem;

  if( NEVER(pList==0) ) return 0;
  zDb = pFix->zDb;
  for(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
    if( pItem->zDatabase==0 ){
      pItem->zDatabase = sqlite3DbStrDup(pFix->pParse->db, zDb);
    }else if( sqlite3StrICmp(pItem->zDatabase,zDb)!=0 ){
      sqlite3ErrorMsg(pFix->pParse,
         "%s %T cannot reference objects in database %s",
         pFix->zType, pFix->pName, pItem->zDatabase);
      return 1;
    }
#if !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
    if( sqlite3FixSelect(pFix, pItem->pSelect) ) return 1;
    if( sqlite3FixExpr(pFix, pItem->pOn) ) return 1;
#endif
  }
  return 0;
}
#if !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_TRIGGER)
SQLITE_PRIVATE int sqlite3FixSelect(
  DbFixer *pFix,       /* Context of the fixation */
  Select *pSelect      /* The SELECT statement to be fixed to one database */
){
  while( pSelect ){
    if( sqlite3FixExprList(pFix, pSelect->pEList) ){
      return 1;
    }
    if( sqlite3FixSrcList(pFix, pSelect->pSrc) ){
      return 1;
    }
    if( sqlite3FixExpr(pFix, pSelect->pWhere) ){
      return 1;
    }
    if( sqlite3FixExpr(pFix, pSelect->pHaving) ){
      return 1;
    }
    pSelect = pSelect->pPrior;
  }
  return 0;
}
SQLITE_PRIVATE int sqlite3FixExpr(
  DbFixer *pFix,     /* Context of the fixation */
  Expr *pExpr        /* The expression to be fixed to one database */
){
  while( pExpr ){
    if( ExprHasAnyProperty(pExpr, EP_TokenOnly) ) break;
    if( ExprHasProperty(pExpr, EP_xIsSelect) ){
      if( sqlite3FixSelect(pFix, pExpr->x.pSelect) ) return 1;
    }else{
      if( sqlite3FixExprList(pFix, pExpr->x.pList) ) return 1;
    }
    if( sqlite3FixExpr(pFix, pExpr->pRight) ){
      return 1;
    }
    pExpr = pExpr->pLeft;
  }
  return 0;
}
SQLITE_PRIVATE int sqlite3FixExprList(
  DbFixer *pFix,     /* Context of the fixation */
  ExprList *pList    /* The expression to be fixed to one database */
){
  int i;
  struct ExprList_item *pItem;
  if( pList==0 ) return 0;
  for(i=0, pItem=pList->a; i<pList->nExpr; i++, pItem++){
    if( sqlite3FixExpr(pFix, pItem->pExpr) ){
      return 1;
    }
  }
  return 0;
}
#endif

#ifndef SQLITE_OMIT_TRIGGER
SQLITE_PRIVATE int sqlite3FixTriggerStep(
  DbFixer *pFix,     /* Context of the fixation */
  TriggerStep *pStep /* The trigger step be fixed to one database */
){
  while( pStep ){
    if( sqlite3FixSelect(pFix, pStep->pSelect) ){
      return 1;
    }
    if( sqlite3FixExpr(pFix, pStep->pWhere) ){
      return 1;
    }
    if( sqlite3FixExprList(pFix, pStep->pExprList) ){
      return 1;
    }
    pStep = pStep->pNext;
  }
  return 0;
}
#endif

/************** End of attach.c **********************************************/
/************** Begin file auth.c ********************************************/
/*
** 2003 January 11
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used to implement the sqlite3_set_authorizer()
** API.  This facility is an optional feature of the library.  Embedded
** systems that do not need this facility may omit it by recompiling
** the library with -DSQLITE_OMIT_AUTHORIZATION=1
*/

/*
** All of the code in this file may be omitted by defining a single
** macro.
*/
#ifndef SQLITE_OMIT_AUTHORIZATION

/*
** Set or clear the access authorization function.
**
** The access authorization function is be called during the compilation
** phase to verify that the user has read and/or write access permission on
** various fields of the database.  The first argument to the auth function
** is a copy of the 3rd argument to this routine.  The second argument
** to the auth function is one of these constants:
**
**       SQLITE_CREATE_INDEX
**       SQLITE_CREATE_TABLE
**       SQLITE_CREATE_TEMP_INDEX
**       SQLITE_CREATE_TEMP_TABLE
**       SQLITE_CREATE_TEMP_TRIGGER
**       SQLITE_CREATE_TEMP_VIEW
**       SQLITE_CREATE_TRIGGER
**       SQLITE_CREATE_VIEW
**       SQLITE_DELETE
**       SQLITE_DROP_INDEX
**       SQLITE_DROP_TABLE
**       SQLITE_DROP_TEMP_INDEX
**       SQLITE_DROP_TEMP_TABLE
**       SQLITE_DROP_TEMP_TRIGGER
**       SQLITE_DROP_TEMP_VIEW
**       SQLITE_DROP_TRIGGER
**       SQLITE_DROP_VIEW
**       SQLITE_INSERT
**       SQLITE_PRAGMA
**       SQLITE_READ
**       SQLITE_SELECT
**       SQLITE_TRANSACTION
**       SQLITE_UPDATE
**
** The third and fourth arguments to the auth function are the name of
** the table and the column that are being accessed.  The auth function
** should return either SQLITE_OK, SQLITE_DENY, or SQLITE_IGNORE.  If
** SQLITE_OK is returned, it means that access is allowed.  SQLITE_DENY
** means that the SQL statement will never-run - the sqlite3_exec() call
** will return with an error.  SQLITE_IGNORE means that the SQL statement
** should run but attempts to read the specified column will return NULL
** and attempts to write the column will be ignored.
**
** Setting the auth function to NULL disables this hook.  The default
** setting of the auth function is NULL.
*/
SQLITE_API int sqlite3_set_authorizer(
  sqlite3 *db,
  int (*xAuth)(void*,int,const char*,const char*,const char*,const char*),
  void *pArg
){
  sqlite3_mutex_enter(db->mutex);
  db->xAuth = xAuth;
  db->pAuthArg = pArg;
  sqlite3ExpirePreparedStatements(db);
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

/*
** Write an error message into pParse->zErrMsg that explains that the
** user-supplied authorization function returned an illegal value.
*/
static void sqliteAuthBadReturnCode(Parse *pParse){
  sqlite3ErrorMsg(pParse, "authorizer malfunction");
  pParse->rc = SQLITE_ERROR;
}

/*
** Invoke the authorization callback for permission to read column zCol from
** table zTab in database zDb. This function assumes that an authorization
** callback has been registered (i.e. that sqlite3.xAuth is not NULL).
**
** If SQLITE_IGNORE is returned and pExpr is not NULL, then pExpr is changed
** to an SQL NULL expression. Otherwise, if pExpr is NULL, then SQLITE_IGNORE
** is treated as SQLITE_DENY. In this case an error is left in pParse.
*/
SQLITE_PRIVATE int sqlite3AuthReadCol(
  Parse *pParse,                  /* The parser context */
  const char *zTab,               /* Table name */
  const char *zCol,               /* Column name */
  int iDb                         /* Index of containing database. */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  char *zDb = db->aDb[iDb].zName; /* Name of attached database */
  int rc;                         /* Auth callback return code */

  rc = db->xAuth(db->pAuthArg, SQLITE_READ, zTab,zCol,zDb,pParse->zAuthContext);
  if( rc==SQLITE_DENY ){
    if( db->nDb>2 || iDb!=0 ){
      sqlite3ErrorMsg(pParse, "access to %s.%s.%s is prohibited",zDb,zTab,zCol);
    }else{
      sqlite3ErrorMsg(pParse, "access to %s.%s is prohibited", zTab, zCol);
    }
    pParse->rc = SQLITE_AUTH;
  }else if( rc!=SQLITE_IGNORE && rc!=SQLITE_OK ){
    sqliteAuthBadReturnCode(pParse);
  }
  return rc;
}

/*
** The pExpr should be a TK_COLUMN expression.  The table referred to
** is in pTabList or else it is the NEW or OLD table of a trigger.  
** Check to see if it is OK to read this particular column.
**
** If the auth function returns SQLITE_IGNORE, change the TK_COLUMN 
** instruction into a TK_NULL.  If the auth function returns SQLITE_DENY,
** then generate an error.
*/
SQLITE_PRIVATE void sqlite3AuthRead(
  Parse *pParse,        /* The parser context */
  Expr *pExpr,          /* The expression to check authorization on */
  Schema *pSchema,      /* The schema of the expression */
  SrcList *pTabList     /* All table that pExpr might refer to */
){
  sqlite3 *db = pParse->db;
  Table *pTab = 0;      /* The table being read */
  const char *zCol;     /* Name of the column of the table */
  int iSrc;             /* Index in pTabList->a[] of table being read */
  int iDb;              /* The index of the database the expression refers to */
  int iCol;             /* Index of column in table */

  if( db->xAuth==0 ) return;
  iDb = sqlite3SchemaToIndex(pParse->db, pSchema);
  if( iDb<0 ){
    /* An attempt to read a column out of a subquery or other
    ** temporary table. */
    return;
  }

  assert( pExpr->op==TK_COLUMN || pExpr->op==TK_TRIGGER );
  if( pExpr->op==TK_TRIGGER ){
    pTab = pParse->pTriggerTab;
  }else{
    assert( pTabList );
    for(iSrc=0; ALWAYS(iSrc<pTabList->nSrc); iSrc++){
      if( pExpr->iTable==pTabList->a[iSrc].iCursor ){
        pTab = pTabList->a[iSrc].pTab;
        break;
      }
    }
  }
  iCol = pExpr->iColumn;
  if( NEVER(pTab==0) ) return;

  if( iCol>=0 ){
    assert( iCol<pTab->nCol );
    zCol = pTab->aCol[iCol].zName;
  }else if( pTab->iPKey>=0 ){
    assert( pTab->iPKey<pTab->nCol );
    zCol = pTab->aCol[pTab->iPKey].zName;
  }else{
    zCol = "ROWID";
  }
  assert( iDb>=0 && iDb<db->nDb );
  if( SQLITE_IGNORE==sqlite3AuthReadCol(pParse, pTab->zName, zCol, iDb) ){
    pExpr->op = TK_NULL;
  }
}

/*
** Do an authorization check using the code and arguments given.  Return
** either SQLITE_OK (zero) or SQLITE_IGNORE or SQLITE_DENY.  If SQLITE_DENY
** is returned, then the error count and error message in pParse are
** modified appropriately.
*/
SQLITE_PRIVATE int sqlite3AuthCheck(
  Parse *pParse,
  int code,
  const char *zArg1,
  const char *zArg2,
  const char *zArg3
){
  sqlite3 *db = pParse->db;
  int rc;

  /* Don't do any authorization checks if the database is initialising
  ** or if the parser is being invoked from within sqlite3_declare_vtab.
  */
  if( db->init.busy || IN_DECLARE_VTAB ){
    return SQLITE_OK;
  }

  if( db->xAuth==0 ){
    return SQLITE_OK;
  }
  rc = db->xAuth(db->pAuthArg, code, zArg1, zArg2, zArg3, pParse->zAuthContext);
  if( rc==SQLITE_DENY ){
    sqlite3ErrorMsg(pParse, "not authorized");
    pParse->rc = SQLITE_AUTH;
  }else if( rc!=SQLITE_OK && rc!=SQLITE_IGNORE ){
    rc = SQLITE_DENY;
    sqliteAuthBadReturnCode(pParse);
  }
  return rc;
}

/*
** Push an authorization context.  After this routine is called, the
** zArg3 argument to authorization callbacks will be zContext until
** popped.  Or if pParse==0, this routine is a no-op.
*/
SQLITE_PRIVATE void sqlite3AuthContextPush(
  Parse *pParse,
  AuthContext *pContext, 
  const char *zContext
){
  assert( pParse );
  pContext->pParse = pParse;
  pContext->zAuthContext = pParse->zAuthContext;
  pParse->zAuthContext = zContext;
}

/*
** Pop an authorization context that was previously pushed
** by sqlite3AuthContextPush
*/
SQLITE_PRIVATE void sqlite3AuthContextPop(AuthContext *pContext){
  if( pContext->pParse ){
    pContext->pParse->zAuthContext = pContext->zAuthContext;
    pContext->pParse = 0;
  }
}

#endif /* SQLITE_OMIT_AUTHORIZATION */

/************** End of auth.c ************************************************/
/************** Begin file build.c *******************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains C code routines that are called by the SQLite parser
** when syntax rules are reduced.  The routines in this file handle the
** following kinds of SQL syntax:
**
**     CREATE TABLE
**     DROP TABLE
**     CREATE INDEX
**     DROP INDEX
**     creating ID lists
**     BEGIN TRANSACTION
**     COMMIT
**     ROLLBACK
*/

/*
** This routine is called when a new SQL statement is beginning to
** be parsed.  Initialize the pParse structure as needed.
*/
SQLITE_PRIVATE void sqlite3BeginParse(Parse *pParse, int explainFlag){
  pParse->explain = (u8)explainFlag;
  pParse->nVar = 0;
}

#ifndef SQLITE_OMIT_SHARED_CACHE
/*
** The TableLock structure is only used by the sqlite3TableLock() and
** codeTableLocks() functions.
*/
struct TableLock {
  int iDb;             /* The database containing the table to be locked */
  int iTab;            /* The root page of the table to be locked */
  u8 isWriteLock;      /* True for write lock.  False for a read lock */
  const char *zName;   /* Name of the table */
};

/*
** Record the fact that we want to lock a table at run-time.  
**
** The table to be locked has root page iTab and is found in database iDb.
** A read or a write lock can be taken depending on isWritelock.
**
** This routine just records the fact that the lock is desired.  The
** code to make the lock occur is generated by a later call to
** codeTableLocks() which occurs during sqlite3FinishCoding().
*/
SQLITE_PRIVATE void sqlite3TableLock(
  Parse *pParse,     /* Parsing context */
  int iDb,           /* Index of the database containing the table to lock */
  int iTab,          /* Root page number of the table to be locked */
  u8 isWriteLock,    /* True for a write lock */
  const char *zName  /* Name of the table to be locked */
){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  int i;
  int nBytes;
  TableLock *p;
  assert( iDb>=0 );

  for(i=0; i<pToplevel->nTableLock; i++){
    p = &pToplevel->aTableLock[i];
    if( p->iDb==iDb && p->iTab==iTab ){
      p->isWriteLock = (p->isWriteLock || isWriteLock);
      return;
    }
  }

  nBytes = sizeof(TableLock) * (pToplevel->nTableLock+1);
  pToplevel->aTableLock =
      sqlite3DbReallocOrFree(pToplevel->db, pToplevel->aTableLock, nBytes);
  if( pToplevel->aTableLock ){
    p = &pToplevel->aTableLock[pToplevel->nTableLock++];
    p->iDb = iDb;
    p->iTab = iTab;
    p->isWriteLock = isWriteLock;
    p->zName = zName;
  }else{
    pToplevel->nTableLock = 0;
    pToplevel->db->mallocFailed = 1;
  }
}

/*
** Code an OP_TableLock instruction for each table locked by the
** statement (configured by calls to sqlite3TableLock()).
*/
static void codeTableLocks(Parse *pParse){
  int i;
  Vdbe *pVdbe; 

  pVdbe = sqlite3GetVdbe(pParse);
  assert( pVdbe!=0 ); /* sqlite3GetVdbe cannot fail: VDBE already allocated */

  for(i=0; i<pParse->nTableLock; i++){
    TableLock *p = &pParse->aTableLock[i];
    int p1 = p->iDb;
    sqlite3VdbeAddOp4(pVdbe, OP_TableLock, p1, p->iTab, p->isWriteLock,
                      p->zName, P4_STATIC);
  }
}
#else
  #define codeTableLocks(x)
#endif

/*
** This routine is called after a single SQL statement has been
** parsed and a VDBE program to execute that statement has been
** prepared.  This routine puts the finishing touches on the
** VDBE program and resets the pParse structure for the next
** parse.
**
** Note that if an error occurred, it might be the case that
** no VDBE code was generated.
*/
SQLITE_PRIVATE void sqlite3FinishCoding(Parse *pParse){
  sqlite3 *db;
  Vdbe *v;

  db = pParse->db;
  if( db->mallocFailed ) return;
  if( pParse->nested ) return;
  if( pParse->nErr ) return;

  /* Begin by generating some termination code at the end of the
  ** vdbe program
  */
  v = sqlite3GetVdbe(pParse);
  assert( !pParse->isMultiWrite 
       || sqlite3VdbeAssertMayAbort(v, pParse->mayAbort));
  if( v ){
    sqlite3VdbeAddOp0(v, OP_Halt);

    /* The cookie mask contains one bit for each database file open.
    ** (Bit 0 is for main, bit 1 is for temp, and so forth.)  Bits are
    ** set for each database that is used.  Generate code to start a
    ** transaction on each used database and to verify the schema cookie
    ** on each used database.
    */
    if( pParse->cookieGoto>0 ){
      yDbMask mask;
      int iDb;
      sqlite3VdbeJumpHere(v, pParse->cookieGoto-1);
      for(iDb=0, mask=1; iDb<db->nDb; mask<<=1, iDb++){
        if( (mask & pParse->cookieMask)==0 ) continue;
        sqlite3VdbeUsesBtree(v, iDb);
        sqlite3VdbeAddOp2(v,OP_Transaction, iDb, (mask & pParse->writeMask)!=0);
        if( db->init.busy==0 ){
          assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
          sqlite3VdbeAddOp3(v, OP_VerifyCookie,
                            iDb, pParse->cookieValue[iDb],
                            db->aDb[iDb].pSchema->iGeneration);
        }
      }
#ifndef SQLITE_OMIT_VIRTUALTABLE
      {
        int i;
        for(i=0; i<pParse->nVtabLock; i++){
          char *vtab = (char *)sqlite3GetVTable(db, pParse->apVtabLock[i]);
          sqlite3VdbeAddOp4(v, OP_VBegin, 0, 0, 0, vtab, P4_VTAB);
        }
        pParse->nVtabLock = 0;
      }
#endif

      /* Once all the cookies have been verified and transactions opened, 
      ** obtain the required table-locks. This is a no-op unless the 
      ** shared-cache feature is enabled.
      */
      codeTableLocks(pParse);

      /* Initialize any AUTOINCREMENT data structures required.
      */
      sqlite3AutoincrementBegin(pParse);

      /* Finally, jump back to the beginning of the executable code. */
      sqlite3VdbeAddOp2(v, OP_Goto, 0, pParse->cookieGoto);
    }
  }


  /* Get the VDBE program ready for execution
  */
  if( v && ALWAYS(pParse->nErr==0) && !db->mallocFailed ){
#ifdef SQLITE_DEBUG
    FILE *trace = (db->flags & SQLITE_VdbeTrace)!=0 ? stdout : 0;
    sqlite3VdbeTrace(v, trace);
#endif
    assert( pParse->iCacheLevel==0 );  /* Disables and re-enables match */
    /* A minimum of one cursor is required if autoincrement is used
    *  See ticket [a696379c1f08866] */
    if( pParse->pAinc!=0 && pParse->nTab==0 ) pParse->nTab = 1;
    sqlite3VdbeMakeReady(v, pParse);
    pParse->rc = SQLITE_DONE;
    pParse->colNamesSet = 0;
  }else{
    pParse->rc = SQLITE_ERROR;
  }
  pParse->nTab = 0;
  pParse->nMem = 0;
  pParse->nSet = 0;
  pParse->nVar = 0;
  pParse->cookieMask = 0;
  pParse->cookieGoto = 0;
}

/*
** Run the parser and code generator recursively in order to generate
** code for the SQL statement given onto the end of the pParse context
** currently under construction.  When the parser is run recursively
** this way, the final OP_Halt is not appended and other initialization
** and finalization steps are omitted because those are handling by the
** outermost parser.
**
** Not everything is nestable.  This facility is designed to permit
** INSERT, UPDATE, and DELETE operations against SQLITE_MASTER.  Use
** care if you decide to try to use this routine for some other purposes.
*/
SQLITE_PRIVATE void sqlite3NestedParse(Parse *pParse, const char *zFormat, ...){
  va_list ap;
  char *zSql;
  char *zErrMsg = 0;
  sqlite3 *db = pParse->db;
# define SAVE_SZ  (sizeof(Parse) - offsetof(Parse,nVar))
  char saveBuf[SAVE_SZ];

  if( pParse->nErr ) return;
  assert( pParse->nested<10 );  /* Nesting should only be of limited depth */
  va_start(ap, zFormat);
  zSql = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  if( zSql==0 ){
    return;   /* A malloc must have failed */
  }
  pParse->nested++;
  memcpy(saveBuf, &pParse->nVar, SAVE_SZ);
  memset(&pParse->nVar, 0, SAVE_SZ);
  sqlite3RunParser(pParse, zSql, &zErrMsg);
  sqlite3DbFree(db, zErrMsg);
  sqlite3DbFree(db, zSql);
  memcpy(&pParse->nVar, saveBuf, SAVE_SZ);
  pParse->nested--;
}

/*
** Locate the in-memory structure that describes a particular database
** table given the name of that table and (optionally) the name of the
** database containing the table.  Return NULL if not found.
**
** If zDatabase is 0, all databases are searched for the table and the
** first matching table is returned.  (No checking for duplicate table
** names is done.)  The search order is TEMP first, then MAIN, then any
** auxiliary databases added using the ATTACH command.
**
** See also sqlite3LocateTable().
*/
SQLITE_PRIVATE Table *sqlite3FindTable(sqlite3 *db, const char *zName, const char *zDatabase){
  Table *p = 0;
  int i;
  int nName;
  assert( zName!=0 );
  nName = sqlite3Strlen30(zName);
  /* All mutexes are required for schema access.  Make sure we hold them. */
  assert( zDatabase!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  for(i=OMIT_TEMPDB; i<db->nDb; i++){
    int j = (i<2) ? i^1 : i;   /* Search TEMP before MAIN */
    if( zDatabase!=0 && sqlite3StrICmp(zDatabase, db->aDb[j].zName) ) continue;
    assert( sqlite3SchemaMutexHeld(db, j, 0) );
    p = sqlite3HashFind(&db->aDb[j].pSchema->tblHash, zName, nName);
    if( p ) break;
  }
  return p;
}

/*
** Locate the in-memory structure that describes a particular database
** table given the name of that table and (optionally) the name of the
** database containing the table.  Return NULL if not found.  Also leave an
** error message in pParse->zErrMsg.
**
** The difference between this routine and sqlite3FindTable() is that this
** routine leaves an error message in pParse->zErrMsg where
** sqlite3FindTable() does not.
*/
SQLITE_PRIVATE Table *sqlite3LocateTable(
  Parse *pParse,         /* context in which to report errors */
  int isView,            /* True if looking for a VIEW rather than a TABLE */
  const char *zName,     /* Name of the table we are looking for */
  const char *zDbase     /* Name of the database.  Might be NULL */
){
  Table *p;

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  if( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    return 0;
  }

  p = sqlite3FindTable(pParse->db, zName, zDbase);
  if( p==0 ){
    const char *zMsg = isView ? "no such view" : "no such table";
    if( zDbase ){
      sqlite3ErrorMsg(pParse, "%s: %s.%s", zMsg, zDbase, zName);
    }else{
      sqlite3ErrorMsg(pParse, "%s: %s", zMsg, zName);
    }
    pParse->checkSchema = 1;
  }
  return p;
}

/*
** Locate the in-memory structure that describes 
** a particular index given the name of that index
** and the name of the database that contains the index.
** Return NULL if not found.
**
** If zDatabase is 0, all databases are searched for the
** table and the first matching index is returned.  (No checking
** for duplicate index names is done.)  The search order is
** TEMP first, then MAIN, then any auxiliary databases added
** using the ATTACH command.
*/
SQLITE_PRIVATE Index *sqlite3FindIndex(sqlite3 *db, const char *zName, const char *zDb){
  Index *p = 0;
  int i;
  int nName = sqlite3Strlen30(zName);
  /* All mutexes are required for schema access.  Make sure we hold them. */
  assert( zDb!=0 || sqlite3BtreeHoldsAllMutexes(db) );
  for(i=OMIT_TEMPDB; i<db->nDb; i++){
    int j = (i<2) ? i^1 : i;  /* Search TEMP before MAIN */
    Schema *pSchema = db->aDb[j].pSchema;
    assert( pSchema );
    if( zDb && sqlite3StrICmp(zDb, db->aDb[j].zName) ) continue;
    assert( sqlite3SchemaMutexHeld(db, j, 0) );
    p = sqlite3HashFind(&pSchema->idxHash, zName, nName);
    if( p ) break;
  }
  return p;
}

/*
** Reclaim the memory used by an index
*/
static void freeIndex(sqlite3 *db, Index *p){
#ifndef SQLITE_OMIT_ANALYZE
  sqlite3DeleteIndexSamples(db, p);
#endif
  sqlite3DbFree(db, p->zColAff);
  sqlite3DbFree(db, p);
}

/*
** For the index called zIdxName which is found in the database iDb,
** unlike that index from its Table then remove the index from
** the index hash table and free all memory structures associated
** with the index.
*/
SQLITE_PRIVATE void sqlite3UnlinkAndDeleteIndex(sqlite3 *db, int iDb, const char *zIdxName){
  Index *pIndex;
  int len;
  Hash *pHash;

  assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pHash = &db->aDb[iDb].pSchema->idxHash;
  len = sqlite3Strlen30(zIdxName);
  pIndex = sqlite3HashInsert(pHash, zIdxName, len, 0);
  if( ALWAYS(pIndex) ){
    if( pIndex->pTable->pIndex==pIndex ){
      pIndex->pTable->pIndex = pIndex->pNext;
    }else{
      Index *p;
      /* Justification of ALWAYS();  The index must be on the list of
      ** indices. */
      p = pIndex->pTable->pIndex;
      while( ALWAYS(p) && p->pNext!=pIndex ){ p = p->pNext; }
      if( ALWAYS(p && p->pNext==pIndex) ){
        p->pNext = pIndex->pNext;
      }
    }
    freeIndex(db, pIndex);
  }
  db->flags |= SQLITE_InternChanges;
}

/*
** Erase all schema information from the in-memory hash tables of
** a single database.  This routine is called to reclaim memory
** before the database closes.  It is also called during a rollback
** if there were schema changes during the transaction or if a
** schema-cookie mismatch occurs.
**
** If iDb<0 then reset the internal schema tables for all database
** files.  If iDb>=0 then reset the internal schema for only the
** single file indicated.
*/
SQLITE_PRIVATE void sqlite3ResetInternalSchema(sqlite3 *db, int iDb){
  int i, j;
  assert( iDb<db->nDb );

  if( iDb>=0 ){
    /* Case 1:  Reset the single schema identified by iDb */
    Db *pDb = &db->aDb[iDb];
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    assert( pDb->pSchema!=0 );
    sqlite3SchemaClear(pDb->pSchema);

    /* If any database other than TEMP is reset, then also reset TEMP
    ** since TEMP might be holding triggers that reference tables in the
    ** other database.
    */
    if( iDb!=1 ){
      pDb = &db->aDb[1];
      assert( pDb->pSchema!=0 );
      sqlite3SchemaClear(pDb->pSchema);
    }
    return;
  }
  /* Case 2 (from here to the end): Reset all schemas for all attached
  ** databases. */
  assert( iDb<0 );
  sqlite3BtreeEnterAll(db);
  for(i=0; i<db->nDb; i++){
    Db *pDb = &db->aDb[i];
    if( pDb->pSchema ){
      sqlite3SchemaClear(pDb->pSchema);
    }
  }
  db->flags &= ~SQLITE_InternChanges;
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);

  /* If one or more of the auxiliary database files has been closed,
  ** then remove them from the auxiliary database list.  We take the
  ** opportunity to do this here since we have just deleted all of the
  ** schema hash tables and therefore do not have to make any changes
  ** to any of those tables.
  */
  for(i=j=2; i<db->nDb; i++){
    struct Db *pDb = &db->aDb[i];
    if( pDb->pBt==0 ){
      sqlite3DbFree(db, pDb->zName);
      pDb->zName = 0;
      continue;
    }
    if( j<i ){
      db->aDb[j] = db->aDb[i];
    }
    j++;
  }
  memset(&db->aDb[j], 0, (db->nDb-j)*sizeof(db->aDb[j]));
  db->nDb = j;
  if( db->nDb<=2 && db->aDb!=db->aDbStatic ){
    memcpy(db->aDbStatic, db->aDb, 2*sizeof(db->aDb[0]));
    sqlite3DbFree(db, db->aDb);
    db->aDb = db->aDbStatic;
  }
}

/*
** This routine is called when a commit occurs.
*/
SQLITE_PRIVATE void sqlite3CommitInternalChanges(sqlite3 *db){
  db->flags &= ~SQLITE_InternChanges;
}

/*
** Delete memory allocated for the column names of a table or view (the
** Table.aCol[] array).
*/
static void sqliteDeleteColumnNames(sqlite3 *db, Table *pTable){
  int i;
  Column *pCol;
  assert( pTable!=0 );
  if( (pCol = pTable->aCol)!=0 ){
    for(i=0; i<pTable->nCol; i++, pCol++){
      sqlite3DbFree(db, pCol->zName);
      sqlite3ExprDelete(db, pCol->pDflt);
      sqlite3DbFree(db, pCol->zDflt);
      sqlite3DbFree(db, pCol->zType);
      sqlite3DbFree(db, pCol->zColl);
    }
    sqlite3DbFree(db, pTable->aCol);
  }
}

/*
** Remove the memory data structures associated with the given
** Table.  No changes are made to disk by this routine.
**
** This routine just deletes the data structure.  It does not unlink
** the table data structure from the hash table.  But it does destroy
** memory structures of the indices and foreign keys associated with 
** the table.
*/
SQLITE_PRIVATE void sqlite3DeleteTable(sqlite3 *db, Table *pTable){
  Index *pIndex, *pNext;

  assert( !pTable || pTable->nRef>0 );

  /* Do not delete the table until the reference count reaches zero. */
  if( !pTable ) return;
  if( ((!db || db->pnBytesFreed==0) && (--pTable->nRef)>0) ) return;

  /* Delete all indices associated with this table. */
  for(pIndex = pTable->pIndex; pIndex; pIndex=pNext){
    pNext = pIndex->pNext;
    assert( pIndex->pSchema==pTable->pSchema );
    if( !db || db->pnBytesFreed==0 ){
      char *zName = pIndex->zName; 
      TESTONLY ( Index *pOld = ) sqlite3HashInsert(
	  &pIndex->pSchema->idxHash, zName, sqlite3Strlen30(zName), 0
      );
      assert( db==0 || sqlite3SchemaMutexHeld(db, 0, pIndex->pSchema) );
      assert( pOld==pIndex || pOld==0 );
    }
    freeIndex(db, pIndex);
  }

  /* Delete any foreign keys attached to this table. */
  sqlite3FkDelete(db, pTable);

  /* Delete the Table structure itself.
  */
  sqliteDeleteColumnNames(db, pTable);
  sqlite3DbFree(db, pTable->zName);
  sqlite3DbFree(db, pTable->zColAff);
  sqlite3SelectDelete(db, pTable->pSelect);
#ifndef SQLITE_OMIT_CHECK
  sqlite3ExprDelete(db, pTable->pCheck);
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3VtabClear(db, pTable);
#endif
  sqlite3DbFree(db, pTable);
}

/*
** Unlink the given table from the hash tables and the delete the
** table structure with all its indices and foreign keys.
*/
SQLITE_PRIVATE void sqlite3UnlinkAndDeleteTable(sqlite3 *db, int iDb, const char *zTabName){
  Table *p;
  Db *pDb;

  assert( db!=0 );
  assert( iDb>=0 && iDb<db->nDb );
  assert( zTabName );
  assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
  testcase( zTabName[0]==0 );  /* Zero-length table names are allowed */
  pDb = &db->aDb[iDb];
  p = sqlite3HashInsert(&pDb->pSchema->tblHash, zTabName,
                        sqlite3Strlen30(zTabName),0);
  sqlite3DeleteTable(db, p);
  db->flags |= SQLITE_InternChanges;
}

/*
** Given a token, return a string that consists of the text of that
** token.  Space to hold the returned string
** is obtained from sqliteMalloc() and must be freed by the calling
** function.
**
** Any quotation marks (ex:  "name", 'name', [name], or `name`) that
** surround the body of the token are removed.
**
** Tokens are often just pointers into the original SQL text and so
** are not \000 terminated and are not persistent.  The returned string
** is \000 terminated and is persistent.
*/
SQLITE_PRIVATE char *sqlite3NameFromToken(sqlite3 *db, Token *pName){
  char *zName;
  if( pName ){
    zName = sqlite3DbStrNDup(db, (char*)pName->z, pName->n);
    sqlite3Dequote(zName);
  }else{
    zName = 0;
  }
  return zName;
}

/*
** Open the sqlite_master table stored in database number iDb for
** writing. The table is opened using cursor 0.
*/
SQLITE_PRIVATE void sqlite3OpenMasterTable(Parse *p, int iDb){
  Vdbe *v = sqlite3GetVdbe(p);
  sqlite3TableLock(p, iDb, MASTER_ROOT, 1, SCHEMA_TABLE(iDb));
  sqlite3VdbeAddOp3(v, OP_OpenWrite, 0, MASTER_ROOT, iDb);
  sqlite3VdbeChangeP4(v, -1, (char *)5, P4_INT32);  /* 5 column table */
  if( p->nTab==0 ){
    p->nTab = 1;
  }
}

/*
** Parameter zName points to a nul-terminated buffer containing the name
** of a database ("main", "temp" or the name of an attached db). This
** function returns the index of the named database in db->aDb[], or
** -1 if the named db cannot be found.
*/
SQLITE_PRIVATE int sqlite3FindDbName(sqlite3 *db, const char *zName){
  int i = -1;         /* Database number */
  if( zName ){
    Db *pDb;
    int n = sqlite3Strlen30(zName);
    for(i=(db->nDb-1), pDb=&db->aDb[i]; i>=0; i--, pDb--){
      if( (!OMIT_TEMPDB || i!=1 ) && n==sqlite3Strlen30(pDb->zName) && 
          0==sqlite3StrICmp(pDb->zName, zName) ){
        break;
      }
    }
  }
  return i;
}

/*
** The token *pName contains the name of a database (either "main" or
** "temp" or the name of an attached db). This routine returns the
** index of the named database in db->aDb[], or -1 if the named db 
** does not exist.
*/
SQLITE_PRIVATE int sqlite3FindDb(sqlite3 *db, Token *pName){
  int i;                               /* Database number */
  char *zName;                         /* Name we are searching for */
  zName = sqlite3NameFromToken(db, pName);
  i = sqlite3FindDbName(db, zName);
  sqlite3DbFree(db, zName);
  return i;
}

/* The table or view or trigger name is passed to this routine via tokens
** pName1 and pName2. If the table name was fully qualified, for example:
**
** CREATE TABLE xxx.yyy (...);
** 
** Then pName1 is set to "xxx" and pName2 "yyy". On the other hand if
** the table name is not fully qualified, i.e.:
**
** CREATE TABLE yyy(...);
**
** Then pName1 is set to "yyy" and pName2 is "".
**
** This routine sets the *ppUnqual pointer to point at the token (pName1 or
** pName2) that stores the unqualified table name.  The index of the
** database "xxx" is returned.
*/
SQLITE_PRIVATE int sqlite3TwoPartName(
  Parse *pParse,      /* Parsing and code generating context */
  Token *pName1,      /* The "xxx" in the name "xxx.yyy" or "xxx" */
  Token *pName2,      /* The "yyy" in the name "xxx.yyy" */
  Token **pUnqual     /* Write the unqualified object name here */
){
  int iDb;                    /* Database holding the object */
  sqlite3 *db = pParse->db;

  if( ALWAYS(pName2!=0) && pName2->n>0 ){
    if( db->init.busy ) {
      sqlite3ErrorMsg(pParse, "corrupt database");
      pParse->nErr++;
      return -1;
    }
    *pUnqual = pName2;
    iDb = sqlite3FindDb(db, pName1);
    if( iDb<0 ){
      sqlite3ErrorMsg(pParse, "unknown database %T", pName1);
      pParse->nErr++;
      return -1;
    }
  }else{
    assert( db->init.iDb==0 || db->init.busy );
    iDb = db->init.iDb;
    *pUnqual = pName1;
  }
  return iDb;
}

/*
** This routine is used to check if the UTF-8 string zName is a legal
** unqualified name for a new schema object (table, index, view or
** trigger). All names are legal except those that begin with the string
** "sqlite_" (in upper, lower or mixed case). This portion of the namespace
** is reserved for internal use.
*/
SQLITE_PRIVATE int sqlite3CheckObjectName(Parse *pParse, const char *zName){
  if( !pParse->db->init.busy && pParse->nested==0 
          && (pParse->db->flags & SQLITE_WriteSchema)==0
          && 0==sqlite3StrNICmp(zName, "sqlite_", 7) ){
    sqlite3ErrorMsg(pParse, "object name reserved for internal use: %s", zName);
    return SQLITE_ERROR;
  }
  return SQLITE_OK;
}

/*
** Begin constructing a new table representation in memory.  This is
** the first of several action routines that get called in response
** to a CREATE TABLE statement.  In particular, this routine is called
** after seeing tokens "CREATE" and "TABLE" and the table name. The isTemp
** flag is true if the table should be stored in the auxiliary database
** file instead of in the main database file.  This is normally the case
** when the "TEMP" or "TEMPORARY" keyword occurs in between
** CREATE and TABLE.
**
** The new table record is initialized and put in pParse->pNewTable.
** As more of the CREATE TABLE statement is parsed, additional action
** routines will be called to add more information to this record.
** At the end of the CREATE TABLE statement, the sqlite3EndTable() routine
** is called to complete the construction of the new table record.
*/
SQLITE_PRIVATE void sqlite3StartTable(
  Parse *pParse,   /* Parser context */
  Token *pName1,   /* First part of the name of the table or view */
  Token *pName2,   /* Second part of the name of the table or view */
  int isTemp,      /* True if this is a TEMP table */
  int isView,      /* True if this is a VIEW */
  int isVirtual,   /* True if this is a VIRTUAL table */
  int noErr        /* Do nothing if table already exists */
){
  Table *pTable;
  char *zName = 0; /* The name of the new table */
  sqlite3 *db = pParse->db;
  Vdbe *v;
  int iDb;         /* Database number to create the table in */
  Token *pName;    /* Unqualified name of the table to create */

  /* The table or view name to create is passed to this routine via tokens
  ** pName1 and pName2. If the table name was fully qualified, for example:
  **
  ** CREATE TABLE xxx.yyy (...);
  ** 
  ** Then pName1 is set to "xxx" and pName2 "yyy". On the other hand if
  ** the table name is not fully qualified, i.e.:
  **
  ** CREATE TABLE yyy(...);
  **
  ** Then pName1 is set to "yyy" and pName2 is "".
  **
  ** The call below sets the pName pointer to point at the token (pName1 or
  ** pName2) that stores the unqualified table name. The variable iDb is
  ** set to the index of the database that the table or view is to be
  ** created in.
  */
  iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
  if( iDb<0 ) return;
  if( !OMIT_TEMPDB && isTemp && pName2->n>0 && iDb!=1 ){
    /* If creating a temp table, the name may not be qualified. Unless 
    ** the database name is "temp" anyway.  */
    sqlite3ErrorMsg(pParse, "temporary table name must be unqualified");
    return;
  }
  if( !OMIT_TEMPDB && isTemp ) iDb = 1;

  pParse->sNameToken = *pName;
  zName = sqlite3NameFromToken(db, pName);
  if( zName==0 ) return;
  if( SQLITE_OK!=sqlite3CheckObjectName(pParse, zName) ){
    goto begin_table_error;
  }
  if( db->init.iDb==1 ) isTemp = 1;
#ifndef SQLITE_OMIT_AUTHORIZATION
  assert( (isTemp & 1)==isTemp );
  {
    int code;
    char *zDb = db->aDb[iDb].zName;
    if( sqlite3AuthCheck(pParse, SQLITE_INSERT, SCHEMA_TABLE(isTemp), 0, zDb) ){
      goto begin_table_error;
    }
    if( isView ){
      if( !OMIT_TEMPDB && isTemp ){
        code = SQLITE_CREATE_TEMP_VIEW;
      }else{
        code = SQLITE_CREATE_VIEW;
      }
    }else{
      if( !OMIT_TEMPDB && isTemp ){
        code = SQLITE_CREATE_TEMP_TABLE;
      }else{
        code = SQLITE_CREATE_TABLE;
      }
    }
    if( !isVirtual && sqlite3AuthCheck(pParse, code, zName, 0, zDb) ){
      goto begin_table_error;
    }
  }
#endif

  /* Make sure the new table name does not collide with an existing
  ** index or table name in the same database.  Issue an error message if
  ** it does. The exception is if the statement being parsed was passed
  ** to an sqlite3_declare_vtab() call. In that case only the column names
  ** and types will be used, so there is no need to test for namespace
  ** collisions.
  */
  if( !IN_DECLARE_VTAB ){
    char *zDb = db->aDb[iDb].zName;
    if( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
      goto begin_table_error;
    }
    pTable = sqlite3FindTable(db, zName, zDb);
    if( pTable ){
      if( !noErr ){
        sqlite3ErrorMsg(pParse, "table %T already exists", pName);
      }else{
        assert( !db->init.busy );
        sqlite3CodeVerifySchema(pParse, iDb);
      }
      goto begin_table_error;
    }
    if( sqlite3FindIndex(db, zName, zDb)!=0 ){
      sqlite3ErrorMsg(pParse, "there is already an index named %s", zName);
      goto begin_table_error;
    }
  }

  pTable = sqlite3DbMallocZero(db, sizeof(Table));
  if( pTable==0 ){
    db->mallocFailed = 1;
    pParse->rc = SQLITE_NOMEM;
    pParse->nErr++;
    goto begin_table_error;
  }
  pTable->zName = zName;
  pTable->iPKey = -1;
  pTable->pSchema = db->aDb[iDb].pSchema;
  pTable->nRef = 1;
  pTable->nRowEst = 1000000;
  assert( pParse->pNewTable==0 );
  pParse->pNewTable = pTable;

  /* If this is the magic sqlite_sequence table used by autoincrement,
  ** then record a pointer to this table in the main database structure
  ** so that INSERT can find the table easily.
  */
#ifndef SQLITE_OMIT_AUTOINCREMENT
  if( !pParse->nested && strcmp(zName, "sqlite_sequence")==0 ){
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    pTable->pSchema->pSeqTab = pTable;
  }
#endif

  /* Begin generating the code that will insert the table record into
  ** the SQLITE_MASTER table.  Note in particular that we must go ahead
  ** and allocate the record number for the table entry now.  Before any
  ** PRIMARY KEY or UNIQUE keywords are parsed.  Those keywords will cause
  ** indices to be created and the table record must come before the 
  ** indices.  Hence, the record number for the table must be allocated
  ** now.
  */
  if( !db->init.busy && (v = sqlite3GetVdbe(pParse))!=0 ){
    int j1;
    int fileFormat;
    int reg1, reg2, reg3;
    sqlite3BeginWriteOperation(pParse, 0, iDb);

#ifndef SQLITE_OMIT_VIRTUALTABLE
    if( isVirtual ){
      sqlite3VdbeAddOp0(v, OP_VBegin);
    }
#endif

    /* If the file format and encoding in the database have not been set, 
    ** set them now.
    */
    reg1 = pParse->regRowid = ++pParse->nMem;
    reg2 = pParse->regRoot = ++pParse->nMem;
    reg3 = ++pParse->nMem;
    sqlite3VdbeAddOp3(v, OP_ReadCookie, iDb, reg3, BTREE_FILE_FORMAT);
    sqlite3VdbeUsesBtree(v, iDb);
    j1 = sqlite3VdbeAddOp1(v, OP_If, reg3);
    fileFormat = (db->flags & SQLITE_LegacyFileFmt)!=0 ?
                  1 : SQLITE_MAX_FILE_FORMAT;
    sqlite3VdbeAddOp2(v, OP_Integer, fileFormat, reg3);
    sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_FILE_FORMAT, reg3);
    sqlite3VdbeAddOp2(v, OP_Integer, ENC(db), reg3);
    sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_TEXT_ENCODING, reg3);
    sqlite3VdbeJumpHere(v, j1);

    /* This just creates a place-holder record in the sqlite_master table.
    ** The record created does not contain anything yet.  It will be replaced
    ** by the real entry in code generated at sqlite3EndTable().
    **
    ** The rowid for the new entry is left in register pParse->regRowid.
    ** The root page number of the new table is left in reg pParse->regRoot.
    ** The rowid and root page number values are needed by the code that
    ** sqlite3EndTable will generate.
    */
#if !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
    if( isView || isVirtual ){
      sqlite3VdbeAddOp2(v, OP_Integer, 0, reg2);
    }else
#endif
    {
      sqlite3VdbeAddOp2(v, OP_CreateTable, iDb, reg2);
    }
    sqlite3OpenMasterTable(pParse, iDb);
    sqlite3VdbeAddOp2(v, OP_NewRowid, 0, reg1);
    sqlite3VdbeAddOp2(v, OP_Null, 0, reg3);
    sqlite3VdbeAddOp3(v, OP_Insert, 0, reg3, reg1);
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
    sqlite3VdbeAddOp0(v, OP_Close);
  }

  /* Normal (non-error) return. */
  return;

  /* If an error occurs, we jump here */
begin_table_error:
  sqlite3DbFree(db, zName);
  return;
}

/*
** This macro is used to compare two strings in a case-insensitive manner.
** It is slightly faster than calling sqlite3StrICmp() directly, but
** produces larger code.
**
** WARNING: This macro is not compatible with the strcmp() family. It
** returns true if the two strings are equal, otherwise false.
*/
#define STRICMP(x, y) (\
sqlite3UpperToLower[*(unsigned char *)(x)]==   \
sqlite3UpperToLower[*(unsigned char *)(y)]     \
&& sqlite3StrICmp((x)+1,(y)+1)==0 )

/*
** Add a new column to the table currently being constructed.
**
** The parser calls this routine once for each column declaration
** in a CREATE TABLE statement.  sqlite3StartTable() gets called
** first to get things going.  Then this routine is called for each
** column.
*/
SQLITE_PRIVATE void sqlite3AddColumn(Parse *pParse, Token *pName){
  Table *p;
  int i;
  char *z;
  Column *pCol;
  sqlite3 *db = pParse->db;
  if( (p = pParse->pNewTable)==0 ) return;
#if SQLITE_MAX_COLUMN
  if( p->nCol+1>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3ErrorMsg(pParse, "too many columns on %s", p->zName);
    return;
  }
#endif
  z = sqlite3NameFromToken(db, pName);
  if( z==0 ) return;
  for(i=0; i<p->nCol; i++){
    if( STRICMP(z, p->aCol[i].zName) ){
      sqlite3ErrorMsg(pParse, "duplicate column name: %s", z);
      sqlite3DbFree(db, z);
      return;
    }
  }
  if( (p->nCol & 0x7)==0 ){
    Column *aNew;
    aNew = sqlite3DbRealloc(db,p->aCol,(p->nCol+8)*sizeof(p->aCol[0]));
    if( aNew==0 ){
      sqlite3DbFree(db, z);
      return;
    }
    p->aCol = aNew;
  }
  pCol = &p->aCol[p->nCol];
  memset(pCol, 0, sizeof(p->aCol[0]));
  pCol->zName = z;
 
  /* If there is no type specified, columns have the default affinity
  ** 'NONE'. If there is a type specified, then sqlite3AddColumnType() will
  ** be called next to set pCol->affinity correctly.
  */
  pCol->affinity = SQLITE_AFF_NONE;
  p->nCol++;
}

/*
** This routine is called by the parser while in the middle of
** parsing a CREATE TABLE statement.  A "NOT NULL" constraint has
** been seen on a column.  This routine sets the notNull flag on
** the column currently under construction.
*/
SQLITE_PRIVATE void sqlite3AddNotNull(Parse *pParse, int onError){
  Table *p;
  p = pParse->pNewTable;
  if( p==0 || NEVER(p->nCol<1) ) return;
  p->aCol[p->nCol-1].notNull = (u8)onError;
}

/*
** Scan the column type name zType (length nType) and return the
** associated affinity type.
**
** This routine does a case-independent search of zType for the 
** substrings in the following table. If one of the substrings is
** found, the corresponding affinity is returned. If zType contains
** more than one of the substrings, entries toward the top of 
** the table take priority. For example, if zType is 'BLOBINT', 
** SQLITE_AFF_INTEGER is returned.
**
** Substring     | Affinity
** --------------------------------
** 'INT'         | SQLITE_AFF_INTEGER
** 'CHAR'        | SQLITE_AFF_TEXT
** 'CLOB'        | SQLITE_AFF_TEXT
** 'TEXT'        | SQLITE_AFF_TEXT
** 'BLOB'        | SQLITE_AFF_NONE
** 'REAL'        | SQLITE_AFF_REAL
** 'FLOA'        | SQLITE_AFF_REAL
** 'DOUB'        | SQLITE_AFF_REAL
**
** If none of the substrings in the above table are found,
** SQLITE_AFF_NUMERIC is returned.
*/
SQLITE_PRIVATE char sqlite3AffinityType(const char *zIn){
  u32 h = 0;
  char aff = SQLITE_AFF_NUMERIC;

  if( zIn ) while( zIn[0] ){
    h = (h<<8) + sqlite3UpperToLower[(*zIn)&0xff];
    zIn++;
    if( h==(('c'<<24)+('h'<<16)+('a'<<8)+'r') ){             /* CHAR */
      aff = SQLITE_AFF_TEXT; 
    }else if( h==(('c'<<24)+('l'<<16)+('o'<<8)+'b') ){       /* CLOB */
      aff = SQLITE_AFF_TEXT;
    }else if( h==(('t'<<24)+('e'<<16)+('x'<<8)+'t') ){       /* TEXT */
      aff = SQLITE_AFF_TEXT;
    }else if( h==(('b'<<24)+('l'<<16)+('o'<<8)+'b')          /* BLOB */
        && (aff==SQLITE_AFF_NUMERIC || aff==SQLITE_AFF_REAL) ){
      aff = SQLITE_AFF_NONE;
#ifndef SQLITE_OMIT_FLOATING_POINT
    }else if( h==(('r'<<24)+('e'<<16)+('a'<<8)+'l')          /* REAL */
        && aff==SQLITE_AFF_NUMERIC ){
      aff = SQLITE_AFF_REAL;
    }else if( h==(('f'<<24)+('l'<<16)+('o'<<8)+'a')          /* FLOA */
        && aff==SQLITE_AFF_NUMERIC ){
      aff = SQLITE_AFF_REAL;
    }else if( h==(('d'<<24)+('o'<<16)+('u'<<8)+'b')          /* DOUB */
        && aff==SQLITE_AFF_NUMERIC ){
      aff = SQLITE_AFF_REAL;
#endif
    }else if( (h&0x00FFFFFF)==(('i'<<16)+('n'<<8)+'t') ){    /* INT */
      aff = SQLITE_AFF_INTEGER;
      break;
    }
  }

  return aff;
}

/*
** This routine is called by the parser while in the middle of
** parsing a CREATE TABLE statement.  The pFirst token is the first
** token in the sequence of tokens that describe the type of the
** column currently under construction.   pLast is the last token
** in the sequence.  Use this information to construct a string
** that contains the typename of the column and store that string
** in zType.
*/ 
SQLITE_PRIVATE void sqlite3AddColumnType(Parse *pParse, Token *pType){
  Table *p;
  Column *pCol;

  p = pParse->pNewTable;
  if( p==0 || NEVER(p->nCol<1) ) return;
  pCol = &p->aCol[p->nCol-1];
  assert( pCol->zType==0 );
  pCol->zType = sqlite3NameFromToken(pParse->db, pType);
  pCol->affinity = sqlite3AffinityType(pCol->zType);
}

/*
** The expression is the default value for the most recently added column
** of the table currently under construction.
**
** Default value expressions must be constant.  Raise an exception if this
** is not the case.
**
** This routine is called by the parser while in the middle of
** parsing a CREATE TABLE statement.
*/
SQLITE_PRIVATE void sqlite3AddDefaultValue(Parse *pParse, ExprSpan *pSpan){
  Table *p;
  Column *pCol;
  sqlite3 *db = pParse->db;
  p = pParse->pNewTable;
  if( p!=0 ){
    pCol = &(p->aCol[p->nCol-1]);
    if( !sqlite3ExprIsConstantOrFunction(pSpan->pExpr) ){
      sqlite3ErrorMsg(pParse, "default value of column [%s] is not constant",
          pCol->zName);
    }else{
      /* A copy of pExpr is used instead of the original, as pExpr contains
      ** tokens that point to volatile memory. The 'span' of the expression
      ** is required by pragma table_info.
      */
      sqlite3ExprDelete(db, pCol->pDflt);
      pCol->pDflt = sqlite3ExprDup(db, pSpan->pExpr, EXPRDUP_REDUCE);
      sqlite3DbFree(db, pCol->zDflt);
      pCol->zDflt = sqlite3DbStrNDup(db, (char*)pSpan->zStart,
                                     (int)(pSpan->zEnd - pSpan->zStart));
    }
  }
  sqlite3ExprDelete(db, pSpan->pExpr);
}

/*
** Designate the PRIMARY KEY for the table.  pList is a list of names 
** of columns that form the primary key.  If pList is NULL, then the
** most recently added column of the table is the primary key.
**
** A table can have at most one primary key.  If the table already has
** a primary key (and this is the second primary key) then create an
** error.
**
** If the PRIMARY KEY is on a single column whose datatype is INTEGER,
** then we will try to use that column as the rowid.  Set the Table.iPKey
** field of the table under construction to be the index of the
** INTEGER PRIMARY KEY column.  Table.iPKey is set to -1 if there is
** no INTEGER PRIMARY KEY.
**
** If the key is not an INTEGER PRIMARY KEY, then create a unique
** index for the key.  No index is created for INTEGER PRIMARY KEYs.
*/
SQLITE_PRIVATE void sqlite3AddPrimaryKey(
  Parse *pParse,    /* Parsing context */
  ExprList *pList,  /* List of field names to be indexed */
  int onError,      /* What to do with a uniqueness conflict */
  int autoInc,      /* True if the AUTOINCREMENT keyword is present */
  int sortOrder     /* SQLITE_SO_ASC or SQLITE_SO_DESC */
){
  Table *pTab = pParse->pNewTable;
  char *zType = 0;
  int iCol = -1, i;
  if( pTab==0 || IN_DECLARE_VTAB ) goto primary_key_exit;
  if( pTab->tabFlags & TF_HasPrimaryKey ){
    sqlite3ErrorMsg(pParse, 
      "table \"%s\" has more than one primary key", pTab->zName);
    goto primary_key_exit;
  }
  pTab->tabFlags |= TF_HasPrimaryKey;
  if( pList==0 ){
    iCol = pTab->nCol - 1;
    pTab->aCol[iCol].isPrimKey = 1;
  }else{
    for(i=0; i<pList->nExpr; i++){
      for(iCol=0; iCol<pTab->nCol; iCol++){
        if( sqlite3StrICmp(pList->a[i].zName, pTab->aCol[iCol].zName)==0 ){
          break;
        }
      }
      if( iCol<pTab->nCol ){
        pTab->aCol[iCol].isPrimKey = 1;
      }
    }
    if( pList->nExpr>1 ) iCol = -1;
  }
  if( iCol>=0 && iCol<pTab->nCol ){
    zType = pTab->aCol[iCol].zType;
  }
  if( zType && sqlite3StrICmp(zType, "INTEGER")==0
        && sortOrder==SQLITE_SO_ASC ){
    pTab->iPKey = iCol;
    pTab->keyConf = (u8)onError;
    assert( autoInc==0 || autoInc==1 );
    pTab->tabFlags |= autoInc*TF_Autoincrement;
  }else if( autoInc ){
#ifndef SQLITE_OMIT_AUTOINCREMENT
    sqlite3ErrorMsg(pParse, "AUTOINCREMENT is only allowed on an "
       "INTEGER PRIMARY KEY");
#endif
  }else{
    Index *p;
    p = sqlite3CreateIndex(pParse, 0, 0, 0, pList, onError, 0, 0, sortOrder, 0);
    if( p ){
      p->autoIndex = 2;
    }
    pList = 0;
  }

primary_key_exit:
  sqlite3ExprListDelete(pParse->db, pList);
  return;
}

/*
** Add a new CHECK constraint to the table currently under construction.
*/
SQLITE_PRIVATE void sqlite3AddCheckConstraint(
  Parse *pParse,    /* Parsing context */
  Expr *pCheckExpr  /* The check expression */
){
  sqlite3 *db = pParse->db;
#ifndef SQLITE_OMIT_CHECK
  Table *pTab = pParse->pNewTable;
  if( pTab && !IN_DECLARE_VTAB ){
    pTab->pCheck = sqlite3ExprAnd(db, pTab->pCheck, pCheckExpr);
  }else
#endif
  {
    sqlite3ExprDelete(db, pCheckExpr);
  }
}

/*
** Set the collation function of the most recently parsed table column
** to the CollSeq given.
*/
SQLITE_PRIVATE void sqlite3AddCollateType(Parse *pParse, Token *pToken){
  Table *p;
  int i;
  char *zColl;              /* Dequoted name of collation sequence */
  sqlite3 *db;

  if( (p = pParse->pNewTable)==0 ) return;
  i = p->nCol-1;
  db = pParse->db;
  zColl = sqlite3NameFromToken(db, pToken);
  if( !zColl ) return;

  if( sqlite3LocateCollSeq(pParse, zColl) ){
    Index *pIdx;
    p->aCol[i].zColl = zColl;
  
    /* If the column is declared as "<name> PRIMARY KEY COLLATE <type>",
    ** then an index may have been created on this column before the
    ** collation type was added. Correct this if it is the case.
    */
    for(pIdx=p->pIndex; pIdx; pIdx=pIdx->pNext){
      assert( pIdx->nColumn==1 );
      if( pIdx->aiColumn[0]==i ){
        pIdx->azColl[0] = p->aCol[i].zColl;
      }
    }
  }else{
    sqlite3DbFree(db, zColl);
  }
}

/*
** This function returns the collation sequence for database native text
** encoding identified by the string zName, length nName.
**
** If the requested collation sequence is not available, or not available
** in the database native encoding, the collation factory is invoked to
** request it. If the collation factory does not supply such a sequence,
** and the sequence is available in another text encoding, then that is
** returned instead.
**
** If no versions of the requested collations sequence are available, or
** another error occurs, NULL is returned and an error message written into
** pParse.
**
** This routine is a wrapper around sqlite3FindCollSeq().  This routine
** invokes the collation factory if the named collation cannot be found
** and generates an error message.
**
** See also: sqlite3FindCollSeq(), sqlite3GetCollSeq()
*/
SQLITE_PRIVATE CollSeq *sqlite3LocateCollSeq(Parse *pParse, const char *zName){
  sqlite3 *db = pParse->db;
  u8 enc = ENC(db);
  u8 initbusy = db->init.busy;
  CollSeq *pColl;

  pColl = sqlite3FindCollSeq(db, enc, zName, initbusy);
  if( !initbusy && (!pColl || !pColl->xCmp) ){
    pColl = sqlite3GetCollSeq(db, enc, pColl, zName);
    if( !pColl ){
      sqlite3ErrorMsg(pParse, "no such collation sequence: %s", zName);
    }
  }

  return pColl;
}


/*
** Generate code that will increment the schema cookie.
**
** The schema cookie is used to determine when the schema for the
** database changes.  After each schema change, the cookie value
** changes.  When a process first reads the schema it records the
** cookie.  Thereafter, whenever it goes to access the database,
** it checks the cookie to make sure the schema has not changed
** since it was last read.
**
** This plan is not completely bullet-proof.  It is possible for
** the schema to change multiple times and for the cookie to be
** set back to prior value.  But schema changes are infrequent
** and the probability of hitting the same cookie value is only
** 1 chance in 2^32.  So we're safe enough.
*/
SQLITE_PRIVATE void sqlite3ChangeCookie(Parse *pParse, int iDb){
  int r1 = sqlite3GetTempReg(pParse);
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;
  assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
  sqlite3VdbeAddOp2(v, OP_Integer, db->aDb[iDb].pSchema->schema_cookie+1, r1);
  sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_SCHEMA_VERSION, r1);
  sqlite3ReleaseTempReg(pParse, r1);
}

/*
** Measure the number of characters needed to output the given
** identifier.  The number returned includes any quotes used
** but does not include the null terminator.
**
** The estimate is conservative.  It might be larger that what is
** really needed.
*/
static int identLength(const char *z){
  int n;
  for(n=0; *z; n++, z++){
    if( *z=='"' ){ n++; }
  }
  return n + 2;
}

/*
** The first parameter is a pointer to an output buffer. The second 
** parameter is a pointer to an integer that contains the offset at
** which to write into the output buffer. This function copies the
** nul-terminated string pointed to by the third parameter, zSignedIdent,
** to the specified offset in the buffer and updates *pIdx to refer
** to the first byte after the last byte written before returning.
** 
** If the string zSignedIdent consists entirely of alpha-numeric
** characters, does not begin with a digit and is not an SQL keyword,
** then it is copied to the output buffer exactly as it is. Otherwise,
** it is quoted using double-quotes.
*/
static void identPut(char *z, int *pIdx, char *zSignedIdent){
  unsigned char *zIdent = (unsigned char*)zSignedIdent;
  int i, j, needQuote;
  i = *pIdx;

  for(j=0; zIdent[j]; j++){
    if( !sqlite3Isalnum(zIdent[j]) && zIdent[j]!='_' ) break;
  }
  needQuote = sqlite3Isdigit(zIdent[0]) || sqlite3KeywordCode(zIdent, j)!=TK_ID;
  if( !needQuote ){
    needQuote = zIdent[j];
  }

  if( needQuote ) z[i++] = '"';
  for(j=0; zIdent[j]; j++){
    z[i++] = zIdent[j];
    if( zIdent[j]=='"' ) z[i++] = '"';
  }
  if( needQuote ) z[i++] = '"';
  z[i] = 0;
  *pIdx = i;
}

/*
** Generate a CREATE TABLE statement appropriate for the given
** table.  Memory to hold the text of the statement is obtained
** from sqliteMalloc() and must be freed by the calling function.
*/
static char *createTableStmt(sqlite3 *db, Table *p){
  int i, k, n;
  char *zStmt;
  char *zSep, *zSep2, *zEnd;
  Column *pCol;
  n = 0;
  for(pCol = p->aCol, i=0; i<p->nCol; i++, pCol++){
    n += identLength(pCol->zName) + 5;
  }
  n += identLength(p->zName);
  if( n<50 ){ 
    zSep = "";
    zSep2 = ",";
    zEnd = ")";
  }else{
    zSep = "\n  ";
    zSep2 = ",\n  ";
    zEnd = "\n)";
  }
  n += 35 + 6*p->nCol;
  zStmt = sqlite3DbMallocRaw(0, n);
  if( zStmt==0 ){
    db->mallocFailed = 1;
    return 0;
  }
  sqlite3_snprintf(n, zStmt, "CREATE TABLE ");
  k = sqlite3Strlen30(zStmt);
  identPut(zStmt, &k, p->zName);
  zStmt[k++] = '(';
  for(pCol=p->aCol, i=0; i<p->nCol; i++, pCol++){
    static const char * const azType[] = {
        /* SQLITE_AFF_TEXT    */ " TEXT",
        /* SQLITE_AFF_NONE    */ "",
        /* SQLITE_AFF_NUMERIC */ " NUM",
        /* SQLITE_AFF_INTEGER */ " INT",
        /* SQLITE_AFF_REAL    */ " REAL"
    };
    int len;
    const char *zType;

    sqlite3_snprintf(n-k, &zStmt[k], zSep);
    k += sqlite3Strlen30(&zStmt[k]);
    zSep = zSep2;
    identPut(zStmt, &k, pCol->zName);
    assert( pCol->affinity-SQLITE_AFF_TEXT >= 0 );
    assert( pCol->affinity-SQLITE_AFF_TEXT < ArraySize(azType) );
    testcase( pCol->affinity==SQLITE_AFF_TEXT );
    testcase( pCol->affinity==SQLITE_AFF_NONE );
    testcase( pCol->affinity==SQLITE_AFF_NUMERIC );
    testcase( pCol->affinity==SQLITE_AFF_INTEGER );
    testcase( pCol->affinity==SQLITE_AFF_REAL );
    
    zType = azType[pCol->affinity - SQLITE_AFF_TEXT];
    len = sqlite3Strlen30(zType);
    assert( pCol->affinity==SQLITE_AFF_NONE 
            || pCol->affinity==sqlite3AffinityType(zType) );
    memcpy(&zStmt[k], zType, len);
    k += len;
    assert( k<=n );
  }
  sqlite3_snprintf(n-k, &zStmt[k], "%s", zEnd);
  return zStmt;
}

/*
** This routine is called to report the final ")" that terminates
** a CREATE TABLE statement.
**
** The table structure that other action routines have been building
** is added to the internal hash tables, assuming no errors have
** occurred.
**
** An entry for the table is made in the master table on disk, unless
** this is a temporary table or db->init.busy==1.  When db->init.busy==1
** it means we are reading the sqlite_master table because we just
** connected to the database or because the sqlite_master table has
** recently changed, so the entry for this table already exists in
** the sqlite_master table.  We do not want to create it again.
**
** If the pSelect argument is not NULL, it means that this routine
** was called to create a table generated from a 
** "CREATE TABLE ... AS SELECT ..." statement.  The column names of
** the new table will match the result set of the SELECT.
*/
SQLITE_PRIVATE void sqlite3EndTable(
  Parse *pParse,          /* Parse context */
  Token *pCons,           /* The ',' token after the last column defn. */
  Token *pEnd,            /* The final ')' token in the CREATE TABLE */
  Select *pSelect         /* Select from a "CREATE ... AS SELECT" */
){
  Table *p;
  sqlite3 *db = pParse->db;
  int iDb;

  if( (pEnd==0 && pSelect==0) || db->mallocFailed ){
    return;
  }
  p = pParse->pNewTable;
  if( p==0 ) return;

  assert( !db->init.busy || !pSelect );

  iDb = sqlite3SchemaToIndex(db, p->pSchema);

#ifndef SQLITE_OMIT_CHECK
  /* Resolve names in all CHECK constraint expressions.
  */
  if( p->pCheck ){
    SrcList sSrc;                   /* Fake SrcList for pParse->pNewTable */
    NameContext sNC;                /* Name context for pParse->pNewTable */

    memset(&sNC, 0, sizeof(sNC));
    memset(&sSrc, 0, sizeof(sSrc));
    sSrc.nSrc = 1;
    sSrc.a[0].zName = p->zName;
    sSrc.a[0].pTab = p;
    sSrc.a[0].iCursor = -1;
    sNC.pParse = pParse;
    sNC.pSrcList = &sSrc;
    sNC.isCheck = 1;
    if( sqlite3ResolveExprNames(&sNC, p->pCheck) ){
      return;
    }
  }
#endif /* !defined(SQLITE_OMIT_CHECK) */

  /* If the db->init.busy is 1 it means we are reading the SQL off the
  ** "sqlite_master" or "sqlite_temp_master" table on the disk.
  ** So do not write to the disk again.  Extract the root page number
  ** for the table from the db->init.newTnum field.  (The page number
  ** should have been put there by the sqliteOpenCb routine.)
  */
  if( db->init.busy ){
    p->tnum = db->init.newTnum;
  }

  /* If not initializing, then create a record for the new table
  ** in the SQLITE_MASTER table of the database.
  **
  ** If this is a TEMPORARY table, write the entry into the auxiliary
  ** file instead of into the main database file.
  */
  if( !db->init.busy ){
    int n;
    Vdbe *v;
    char *zType;    /* "view" or "table" */
    char *zType2;   /* "VIEW" or "TABLE" */
    char *zStmt;    /* Text of the CREATE TABLE or CREATE VIEW statement */

    v = sqlite3GetVdbe(pParse);
    if( NEVER(v==0) ) return;

    sqlite3VdbeAddOp1(v, OP_Close, 0);

    /* 
    ** Initialize zType for the new view or table.
    */
    if( p->pSelect==0 ){
      /* A regular table */
      zType = "table";
      zType2 = "TABLE";
#ifndef SQLITE_OMIT_VIEW
    }else{
      /* A view */
      zType = "view";
      zType2 = "VIEW";
#endif
    }

    /* If this is a CREATE TABLE xx AS SELECT ..., execute the SELECT
    ** statement to populate the new table. The root-page number for the
    ** new table is in register pParse->regRoot.
    **
    ** Once the SELECT has been coded by sqlite3Select(), it is in a
    ** suitable state to query for the column names and types to be used
    ** by the new table.
    **
    ** A shared-cache write-lock is not required to write to the new table,
    ** as a schema-lock must have already been obtained to create it. Since
    ** a schema-lock excludes all other database users, the write-lock would
    ** be redundant.
    */
    if( pSelect ){
      SelectDest dest;
      Table *pSelTab;

      assert(pParse->nTab==1);
      sqlite3VdbeAddOp3(v, OP_OpenWrite, 1, pParse->regRoot, iDb);
      sqlite3VdbeChangeP5(v, 1);
      pParse->nTab = 2;
      sqlite3SelectDestInit(&dest, SRT_Table, 1);
      sqlite3Select(pParse, pSelect, &dest);
      sqlite3VdbeAddOp1(v, OP_Close, 1);
      if( pParse->nErr==0 ){
        pSelTab = sqlite3ResultSetOfSelect(pParse, pSelect);
        if( pSelTab==0 ) return;
        assert( p->aCol==0 );
        p->nCol = pSelTab->nCol;
        p->aCol = pSelTab->aCol;
        pSelTab->nCol = 0;
        pSelTab->aCol = 0;
        sqlite3DeleteTable(db, pSelTab);
      }
    }

    /* Compute the complete text of the CREATE statement */
    if( pSelect ){
      zStmt = createTableStmt(db, p);
    }else{
      n = (int)(pEnd->z - pParse->sNameToken.z) + 1;
      zStmt = sqlite3MPrintf(db, 
          "CREATE %s %.*s", zType2, n, pParse->sNameToken.z
      );
    }

    /* A slot for the record has already been allocated in the 
    ** SQLITE_MASTER table.  We just need to update that slot with all
    ** the information we've collected.
    */
    sqlite3NestedParse(pParse,
      "UPDATE %Q.%s "
         "SET type='%s', name=%Q, tbl_name=%Q, rootpage=#%d, sql=%Q "
       "WHERE rowid=#%d",
      db->aDb[iDb].zName, SCHEMA_TABLE(iDb),
      zType,
      p->zName,
      p->zName,
      pParse->regRoot,
      zStmt,
      pParse->regRowid
    );
    sqlite3DbFree(db, zStmt);
    sqlite3ChangeCookie(pParse, iDb);

#ifndef SQLITE_OMIT_AUTOINCREMENT
    /* Check to see if we need to create an sqlite_sequence table for
    ** keeping track of autoincrement keys.
    */
    if( p->tabFlags & TF_Autoincrement ){
      Db *pDb = &db->aDb[iDb];
      assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
      if( pDb->pSchema->pSeqTab==0 ){
        sqlite3NestedParse(pParse,
          "CREATE TABLE %Q.sqlite_sequence(name,seq)",
          pDb->zName
        );
      }
    }
#endif

    /* Reparse everything to update our internal data structures */
    sqlite3VdbeAddParseSchemaOp(v, iDb,
               sqlite3MPrintf(db, "tbl_name='%q'", p->zName));
  }


  /* Add the table to the in-memory representation of the database.
  */
  if( db->init.busy ){
    Table *pOld;
    Schema *pSchema = p->pSchema;
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    pOld = sqlite3HashInsert(&pSchema->tblHash, p->zName,
                             sqlite3Strlen30(p->zName),p);
    if( pOld ){
      assert( p==pOld );  /* Malloc must have failed inside HashInsert() */
      db->mallocFailed = 1;
      return;
    }
    pParse->pNewTable = 0;
    db->nTable++;
    db->flags |= SQLITE_InternChanges;

#ifndef SQLITE_OMIT_ALTERTABLE
    if( !p->pSelect ){
      const char *zName = (const char *)pParse->sNameToken.z;
      int nName;
      assert( !pSelect && pCons && pEnd );
      if( pCons->z==0 ){
        pCons = pEnd;
      }
      nName = (int)((const char *)pCons->z - zName);
      p->addColOffset = 13 + sqlite3Utf8CharLen(zName, nName);
    }
#endif
  }
}

#ifndef SQLITE_OMIT_VIEW
/*
** The parser calls this routine in order to create a new VIEW
*/
SQLITE_PRIVATE void sqlite3CreateView(
  Parse *pParse,     /* The parsing context */
  Token *pBegin,     /* The CREATE token that begins the statement */
  Token *pName1,     /* The token that holds the name of the view */
  Token *pName2,     /* The token that holds the name of the view */
  Select *pSelect,   /* A SELECT statement that will become the new view */
  int isTemp,        /* TRUE for a TEMPORARY view */
  int noErr          /* Suppress error messages if VIEW already exists */
){
  Table *p;
  int n;
  const char *z;
  Token sEnd;
  DbFixer sFix;
  Token *pName;
  int iDb;
  sqlite3 *db = pParse->db;

  if( pParse->nVar>0 ){
    sqlite3ErrorMsg(pParse, "parameters are not allowed in views");
    sqlite3SelectDelete(db, pSelect);
    return;
  }
  sqlite3StartTable(pParse, pName1, pName2, isTemp, 1, 0, noErr);
  p = pParse->pNewTable;
  if( p==0 || pParse->nErr ){
    sqlite3SelectDelete(db, pSelect);
    return;
  }
  sqlite3TwoPartName(pParse, pName1, pName2, &pName);
  iDb = sqlite3SchemaToIndex(db, p->pSchema);
  if( sqlite3FixInit(&sFix, pParse, iDb, "view", pName)
    && sqlite3FixSelect(&sFix, pSelect)
  ){
    sqlite3SelectDelete(db, pSelect);
    return;
  }

  /* Make a copy of the entire SELECT statement that defines the view.
  ** This will force all the Expr.token.z values to be dynamically
  ** allocated rather than point to the input string - which means that
  ** they will persist after the current sqlite3_exec() call returns.
  */
  p->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
  sqlite3SelectDelete(db, pSelect);
  if( db->mallocFailed ){
    return;
  }
  if( !db->init.busy ){
    sqlite3ViewGetColumnNames(pParse, p);
  }

  /* Locate the end of the CREATE VIEW statement.  Make sEnd point to
  ** the end.
  */
  sEnd = pParse->sLastToken;
  if( ALWAYS(sEnd.z[0]!=0) && sEnd.z[0]!=';' ){
    sEnd.z += sEnd.n;
  }
  sEnd.n = 0;
  n = (int)(sEnd.z - pBegin->z);
  z = pBegin->z;
  while( ALWAYS(n>0) && sqlite3Isspace(z[n-1]) ){ n--; }
  sEnd.z = &z[n-1];
  sEnd.n = 1;

  /* Use sqlite3EndTable() to add the view to the SQLITE_MASTER table */
  sqlite3EndTable(pParse, 0, &sEnd, 0);
  return;
}
#endif /* SQLITE_OMIT_VIEW */

#if !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
/*
** The Table structure pTable is really a VIEW.  Fill in the names of
** the columns of the view in the pTable structure.  Return the number
** of errors.  If an error is seen leave an error message in pParse->zErrMsg.
*/
SQLITE_PRIVATE int sqlite3ViewGetColumnNames(Parse *pParse, Table *pTable){
  Table *pSelTab;   /* A fake table from which we get the result set */
  Select *pSel;     /* Copy of the SELECT that implements the view */
  int nErr = 0;     /* Number of errors encountered */
  int n;            /* Temporarily holds the number of cursors assigned */
  sqlite3 *db = pParse->db;  /* Database connection for malloc errors */
  int (*xAuth)(void*,int,const char*,const char*,const char*,const char*);

  assert( pTable );

#ifndef SQLITE_OMIT_VIRTUALTABLE
  if( sqlite3VtabCallConnect(pParse, pTable) ){
    return SQLITE_ERROR;
  }
  if( IsVirtual(pTable) ) return 0;
#endif

#ifndef SQLITE_OMIT_VIEW
  /* A positive nCol means the columns names for this view are
  ** already known.
  */
  if( pTable->nCol>0 ) return 0;

  /* A negative nCol is a special marker meaning that we are currently
  ** trying to compute the column names.  If we enter this routine with
  ** a negative nCol, it means two or more views form a loop, like this:
  **
  **     CREATE VIEW one AS SELECT * FROM two;
  **     CREATE VIEW two AS SELECT * FROM one;
  **
  ** Actually, the error above is now caught prior to reaching this point.
  ** But the following test is still important as it does come up
  ** in the following:
  ** 
  **     CREATE TABLE main.ex1(a);
  **     CREATE TEMP VIEW ex1 AS SELECT a FROM ex1;
  **     SELECT * FROM temp.ex1;
  */
  if( pTable->nCol<0 ){
    sqlite3ErrorMsg(pParse, "view %s is circularly defined", pTable->zName);
    return 1;
  }
  assert( pTable->nCol>=0 );

  /* If we get this far, it means we need to compute the table names.
  ** Note that the call to sqlite3ResultSetOfSelect() will expand any
  ** "*" elements in the results set of the view and will assign cursors
  ** to the elements of the FROM clause.  But we do not want these changes
  ** to be permanent.  So the computation is done on a copy of the SELECT
  ** statement that defines the view.
  */
  assert( pTable->pSelect );
  pSel = sqlite3SelectDup(db, pTable->pSelect, 0);
  if( pSel ){
    u8 enableLookaside = db->lookaside.bEnabled;
    n = pParse->nTab;
    sqlite3SrcListAssignCursors(pParse, pSel->pSrc);
    pTable->nCol = -1;
    db->lookaside.bEnabled = 0;
#ifndef SQLITE_OMIT_AUTHORIZATION
    xAuth = db->xAuth;
    db->xAuth = 0;
    pSelTab = sqlite3ResultSetOfSelect(pParse, pSel);
    db->xAuth = xAuth;
#else
    pSelTab = sqlite3ResultSetOfSelect(pParse, pSel);
#endif
    db->lookaside.bEnabled = enableLookaside;
    pParse->nTab = n;
    if( pSelTab ){
      assert( pTable->aCol==0 );
      pTable->nCol = pSelTab->nCol;
      pTable->aCol = pSelTab->aCol;
      pSelTab->nCol = 0;
      pSelTab->aCol = 0;
      sqlite3DeleteTable(db, pSelTab);
      assert( sqlite3SchemaMutexHeld(db, 0, pTable->pSchema) );
      pTable->pSchema->flags |= DB_UnresetViews;
    }else{
      pTable->nCol = 0;
      nErr++;
    }
    sqlite3SelectDelete(db, pSel);
  } else {
    nErr++;
  }
#endif /* SQLITE_OMIT_VIEW */
  return nErr;  
}
#endif /* !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE) */

#ifndef SQLITE_OMIT_VIEW
/*
** Clear the column names from every VIEW in database idx.
*/
static void sqliteViewResetAll(sqlite3 *db, int idx){
  HashElem *i;
  assert( sqlite3SchemaMutexHeld(db, idx, 0) );
  if( !DbHasProperty(db, idx, DB_UnresetViews) ) return;
  for(i=sqliteHashFirst(&db->aDb[idx].pSchema->tblHash); i;i=sqliteHashNext(i)){
    Table *pTab = sqliteHashData(i);
    if( pTab->pSelect ){
      sqliteDeleteColumnNames(db, pTab);
      pTab->aCol = 0;
      pTab->nCol = 0;
    }
  }
  DbClearProperty(db, idx, DB_UnresetViews);
}
#else
# define sqliteViewResetAll(A,B)
#endif /* SQLITE_OMIT_VIEW */

/*
** This function is called by the VDBE to adjust the internal schema
** used by SQLite when the btree layer moves a table root page. The
** root-page of a table or index in database iDb has changed from iFrom
** to iTo.
**
** Ticket #1728:  The symbol table might still contain information
** on tables and/or indices that are the process of being deleted.
** If you are unlucky, one of those deleted indices or tables might
** have the same rootpage number as the real table or index that is
** being moved.  So we cannot stop searching after the first match 
** because the first match might be for one of the deleted indices
** or tables and not the table/index that is actually being moved.
** We must continue looping until all tables and indices with
** rootpage==iFrom have been converted to have a rootpage of iTo
** in order to be certain that we got the right one.
*/
#ifndef SQLITE_OMIT_AUTOVACUUM
SQLITE_PRIVATE void sqlite3RootPageMoved(sqlite3 *db, int iDb, int iFrom, int iTo){
  HashElem *pElem;
  Hash *pHash;
  Db *pDb;

  assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
  pDb = &db->aDb[iDb];
  pHash = &pDb->pSchema->tblHash;
  for(pElem=sqliteHashFirst(pHash); pElem; pElem=sqliteHashNext(pElem)){
    Table *pTab = sqliteHashData(pElem);
    if( pTab->tnum==iFrom ){
      pTab->tnum = iTo;
    }
  }
  pHash = &pDb->pSchema->idxHash;
  for(pElem=sqliteHashFirst(pHash); pElem; pElem=sqliteHashNext(pElem)){
    Index *pIdx = sqliteHashData(pElem);
    if( pIdx->tnum==iFrom ){
      pIdx->tnum = iTo;
    }
  }
}
#endif

/*
** Write code to erase the table with root-page iTable from database iDb.
** Also write code to modify the sqlite_master table and internal schema
** if a root-page of another table is moved by the btree-layer whilst
** erasing iTable (this can happen with an auto-vacuum database).
*/ 
static void destroyRootPage(Parse *pParse, int iTable, int iDb){
  Vdbe *v = sqlite3GetVdbe(pParse);
  int r1 = sqlite3GetTempReg(pParse);
  sqlite3VdbeAddOp3(v, OP_Destroy, iTable, r1, iDb);
  sqlite3MayAbort(pParse);
#ifndef SQLITE_OMIT_AUTOVACUUM
  /* OP_Destroy stores an in integer r1. If this integer
  ** is non-zero, then it is the root page number of a table moved to
  ** location iTable. The following code modifies the sqlite_master table to
  ** reflect this.
  **
  ** The "#NNN" in the SQL is a special constant that means whatever value
  ** is in register NNN.  See grammar rules associated with the TK_REGISTER
  ** token for additional information.
  */
  sqlite3NestedParse(pParse, 
     "UPDATE %Q.%s SET rootpage=%d WHERE #%d AND rootpage=#%d",
     pParse->db->aDb[iDb].zName, SCHEMA_TABLE(iDb), iTable, r1, r1);
#endif
  sqlite3ReleaseTempReg(pParse, r1);
}

/*
** Write VDBE code to erase table pTab and all associated indices on disk.
** Code to update the sqlite_master tables and internal schema definitions
** in case a root-page belonging to another table is moved by the btree layer
** is also added (this can happen with an auto-vacuum database).
*/
static void destroyTable(Parse *pParse, Table *pTab){
#ifdef SQLITE_OMIT_AUTOVACUUM
  Index *pIdx;
  int iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  destroyRootPage(pParse, pTab->tnum, iDb);
  for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
    destroyRootPage(pParse, pIdx->tnum, iDb);
  }
#else
  /* If the database may be auto-vacuum capable (if SQLITE_OMIT_AUTOVACUUM
  ** is not defined), then it is important to call OP_Destroy on the
  ** table and index root-pages in order, starting with the numerically 
  ** largest root-page number. This guarantees that none of the root-pages
  ** to be destroyed is relocated by an earlier OP_Destroy. i.e. if the
  ** following were coded:
  **
  ** OP_Destroy 4 0
  ** ...
  ** OP_Destroy 5 0
  **
  ** and root page 5 happened to be the largest root-page number in the
  ** database, then root page 5 would be moved to page 4 by the 
  ** "OP_Destroy 4 0" opcode. The subsequent "OP_Destroy 5 0" would hit
  ** a free-list page.
  */
  int iTab = pTab->tnum;
  int iDestroyed = 0;

  while( 1 ){
    Index *pIdx;
    int iLargest = 0;

    if( iDestroyed==0 || iTab<iDestroyed ){
      iLargest = iTab;
    }
    for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      int iIdx = pIdx->tnum;
      assert( pIdx->pSchema==pTab->pSchema );
      if( (iDestroyed==0 || (iIdx<iDestroyed)) && iIdx>iLargest ){
        iLargest = iIdx;
      }
    }
    if( iLargest==0 ){
      return;
    }else{
      int iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
      destroyRootPage(pParse, iLargest, iDb);
      iDestroyed = iLargest;
    }
  }
#endif
}

/*
** This routine is called to do the work of a DROP TABLE statement.
** pName is the name of the table to be dropped.
*/
SQLITE_PRIVATE void sqlite3DropTable(Parse *pParse, SrcList *pName, int isView, int noErr){
  Table *pTab;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  int iDb;

  if( db->mallocFailed ){
    goto exit_drop_table;
  }
  assert( pParse->nErr==0 );
  assert( pName->nSrc==1 );
  if( noErr ) db->suppressErr++;
  pTab = sqlite3LocateTable(pParse, isView, 
                            pName->a[0].zName, pName->a[0].zDatabase);
  if( noErr ) db->suppressErr--;

  if( pTab==0 ){
    if( noErr ) sqlite3CodeVerifyNamedSchema(pParse, pName->a[0].zDatabase);
    goto exit_drop_table;
  }
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  assert( iDb>=0 && iDb<db->nDb );

  /* If pTab is a virtual table, call ViewGetColumnNames() to ensure
  ** it is initialized.
  */
  if( IsVirtual(pTab) && sqlite3ViewGetColumnNames(pParse, pTab) ){
    goto exit_drop_table;
  }
#ifndef SQLITE_OMIT_AUTHORIZATION
  {
    int code;
    const char *zTab = SCHEMA_TABLE(iDb);
    const char *zDb = db->aDb[iDb].zName;
    const char *zArg2 = 0;
    if( sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb)){
      goto exit_drop_table;
    }
    if( isView ){
      if( !OMIT_TEMPDB && iDb==1 ){
        code = SQLITE_DROP_TEMP_VIEW;
      }else{
        code = SQLITE_DROP_VIEW;
      }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    }else if( IsVirtual(pTab) ){
      code = SQLITE_DROP_VTABLE;
      zArg2 = sqlite3GetVTable(db, pTab)->pMod->zName;
#endif
    }else{
      if( !OMIT_TEMPDB && iDb==1 ){
        code = SQLITE_DROP_TEMP_TABLE;
      }else{
        code = SQLITE_DROP_TABLE;
      }
    }
    if( sqlite3AuthCheck(pParse, code, pTab->zName, zArg2, zDb) ){
      goto exit_drop_table;
    }
    if( sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0, zDb) ){
      goto exit_drop_table;
    }
  }
#endif
  if( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 ){
    sqlite3ErrorMsg(pParse, "table %s may not be dropped", pTab->zName);
    goto exit_drop_table;
  }

#ifndef SQLITE_OMIT_VIEW
  /* Ensure DROP TABLE is not used on a view, and DROP VIEW is not used
  ** on a table.
  */
  if( isView && pTab->pSelect==0 ){
    sqlite3ErrorMsg(pParse, "use DROP TABLE to delete table %s", pTab->zName);
    goto exit_drop_table;
  }
  if( !isView && pTab->pSelect ){
    sqlite3ErrorMsg(pParse, "use DROP VIEW to delete view %s", pTab->zName);
    goto exit_drop_table;
  }
#endif

  /* Generate code to remove the table from the master table
  ** on disk.
  */
  v = sqlite3GetVdbe(pParse);
  if( v ){
    Trigger *pTrigger;
    Db *pDb = &db->aDb[iDb];
    sqlite3BeginWriteOperation(pParse, 1, iDb);

#ifndef SQLITE_OMIT_VIRTUALTABLE
    if( IsVirtual(pTab) ){
      sqlite3VdbeAddOp0(v, OP_VBegin);
    }
#endif
    sqlite3FkDropTable(pParse, pName, pTab);

    /* Drop all triggers associated with the table being dropped. Code
    ** is generated to remove entries from sqlite_master and/or
    ** sqlite_temp_master if required.
    */
    pTrigger = sqlite3TriggerList(pParse, pTab);
    while( pTrigger ){
      assert( pTrigger->pSchema==pTab->pSchema || 
          pTrigger->pSchema==db->aDb[1].pSchema );
      sqlite3DropTriggerPtr(pParse, pTrigger);
      pTrigger = pTrigger->pNext;
    }

#ifndef SQLITE_OMIT_AUTOINCREMENT
    /* Remove any entries of the sqlite_sequence table associated with
    ** the table being dropped. This is done before the table is dropped
    ** at the btree level, in case the sqlite_sequence table needs to
    ** move as a result of the drop (can happen in auto-vacuum mode).
    */
    if( pTab->tabFlags & TF_Autoincrement ){
      sqlite3NestedParse(pParse,
        "DELETE FROM %s.sqlite_sequence WHERE name=%Q",
        pDb->zName, pTab->zName
      );
    }
#endif

    /* Drop all SQLITE_MASTER table and index entries that refer to the
    ** table. The program name loops through the master table and deletes
    ** every row that refers to a table of the same name as the one being
    ** dropped. Triggers are handled seperately because a trigger can be
    ** created in the temp database that refers to a table in another
    ** database.
    */
    sqlite3NestedParse(pParse, 
        "DELETE FROM %Q.%s WHERE tbl_name=%Q and type!='trigger'",
        pDb->zName, SCHEMA_TABLE(iDb), pTab->zName);

    /* Drop any statistics from the sqlite_stat1 table, if it exists */
    if( sqlite3FindTable(db, "sqlite_stat1", db->aDb[iDb].zName) ){
      sqlite3NestedParse(pParse,
        "DELETE FROM %Q.sqlite_stat1 WHERE tbl=%Q", pDb->zName, pTab->zName
      );
    }

    if( !isView && !IsVirtual(pTab) ){
      destroyTable(pParse, pTab);
    }

    /* Remove the table entry from SQLite's internal schema and modify
    ** the schema cookie.
    */
    if( IsVirtual(pTab) ){
      sqlite3VdbeAddOp4(v, OP_VDestroy, iDb, 0, 0, pTab->zName, 0);
    }
    sqlite3VdbeAddOp4(v, OP_DropTable, iDb, 0, 0, pTab->zName, 0);
    sqlite3ChangeCookie(pParse, iDb);
  }
  sqliteViewResetAll(db, iDb);

exit_drop_table:
  sqlite3SrcListDelete(db, pName);
}

/*
** This routine is called to create a new foreign key on the table
** currently under construction.  pFromCol determines which columns
** in the current table point to the foreign key.  If pFromCol==0 then
** connect the key to the last column inserted.  pTo is the name of
** the table referred to.  pToCol is a list of tables in the other
** pTo table that the foreign key points to.  flags contains all
** information about the conflict resolution algorithms specified
** in the ON DELETE, ON UPDATE and ON INSERT clauses.
**
** An FKey structure is created and added to the table currently
** under construction in the pParse->pNewTable field.
**
** The foreign key is set for IMMEDIATE processing.  A subsequent call
** to sqlite3DeferForeignKey() might change this to DEFERRED.
*/
SQLITE_PRIVATE void sqlite3CreateForeignKey(
  Parse *pParse,       /* Parsing context */
  ExprList *pFromCol,  /* Columns in this table that point to other table */
  Token *pTo,          /* Name of the other table */
  ExprList *pToCol,    /* Columns in the other table */
  int flags            /* Conflict resolution algorithms. */
){
  sqlite3 *db = pParse->db;
#ifndef SQLITE_OMIT_FOREIGN_KEY
  FKey *pFKey = 0;
  FKey *pNextTo;
  Table *p = pParse->pNewTable;
  int nByte;
  int i;
  int nCol;
  char *z;

  assert( pTo!=0 );
  if( p==0 || IN_DECLARE_VTAB ) goto fk_end;
  if( pFromCol==0 ){
    int iCol = p->nCol-1;
    if( NEVER(iCol<0) ) goto fk_end;
    if( pToCol && pToCol->nExpr!=1 ){
      sqlite3ErrorMsg(pParse, "foreign key on %s"
         " should reference only one column of table %T",
         p->aCol[iCol].zName, pTo);
      goto fk_end;
    }
    nCol = 1;
  }else if( pToCol && pToCol->nExpr!=pFromCol->nExpr ){
    sqlite3ErrorMsg(pParse,
        "number of columns in foreign key does not match the number of "
        "columns in the referenced table");
    goto fk_end;
  }else{
    nCol = pFromCol->nExpr;
  }
  nByte = sizeof(*pFKey) + (nCol-1)*sizeof(pFKey->aCol[0]) + pTo->n + 1;
  if( pToCol ){
    for(i=0; i<pToCol->nExpr; i++){
      nByte += sqlite3Strlen30(pToCol->a[i].zName) + 1;
    }
  }
  pFKey = sqlite3DbMallocZero(db, nByte );
  if( pFKey==0 ){
    goto fk_end;
  }
  pFKey->pFrom = p;
  pFKey->pNextFrom = p->pFKey;
  z = (char*)&pFKey->aCol[nCol];
  pFKey->zTo = z;
  memcpy(z, pTo->z, pTo->n);
  z[pTo->n] = 0;
  sqlite3Dequote(z);
  z += pTo->n+1;
  pFKey->nCol = nCol;
  if( pFromCol==0 ){
    pFKey->aCol[0].iFrom = p->nCol-1;
  }else{
    for(i=0; i<nCol; i++){
      int j;
      for(j=0; j<p->nCol; j++){
        if( sqlite3StrICmp(p->aCol[j].zName, pFromCol->a[i].zName)==0 ){
          pFKey->aCol[i].iFrom = j;
          break;
        }
      }
      if( j>=p->nCol ){
        sqlite3ErrorMsg(pParse, 
          "unknown column \"%s\" in foreign key definition", 
          pFromCol->a[i].zName);
        goto fk_end;
      }
    }
  }
  if( pToCol ){
    for(i=0; i<nCol; i++){
      int n = sqlite3Strlen30(pToCol->a[i].zName);
      pFKey->aCol[i].zCol = z;
      memcpy(z, pToCol->a[i].zName, n);
      z[n] = 0;
      z += n+1;
    }
  }
  pFKey->isDeferred = 0;
  pFKey->aAction[0] = (u8)(flags & 0xff);            /* ON DELETE action */
  pFKey->aAction[1] = (u8)((flags >> 8 ) & 0xff);    /* ON UPDATE action */

  assert( sqlite3SchemaMutexHeld(db, 0, p->pSchema) );
  pNextTo = (FKey *)sqlite3HashInsert(&p->pSchema->fkeyHash, 
      pFKey->zTo, sqlite3Strlen30(pFKey->zTo), (void *)pFKey
  );
  if( pNextTo==pFKey ){
    db->mallocFailed = 1;
    goto fk_end;
  }
  if( pNextTo ){
    assert( pNextTo->pPrevTo==0 );
    pFKey->pNextTo = pNextTo;
    pNextTo->pPrevTo = pFKey;
  }

  /* Link the foreign key to the table as the last step.
  */
  p->pFKey = pFKey;
  pFKey = 0;

fk_end:
  sqlite3DbFree(db, pFKey);
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */
  sqlite3ExprListDelete(db, pFromCol);
  sqlite3ExprListDelete(db, pToCol);
}

/*
** This routine is called when an INITIALLY IMMEDIATE or INITIALLY DEFERRED
** clause is seen as part of a foreign key definition.  The isDeferred
** parameter is 1 for INITIALLY DEFERRED and 0 for INITIALLY IMMEDIATE.
** The behavior of the most recently created foreign key is adjusted
** accordingly.
*/
SQLITE_PRIVATE void sqlite3DeferForeignKey(Parse *pParse, int isDeferred){
#ifndef SQLITE_OMIT_FOREIGN_KEY
  Table *pTab;
  FKey *pFKey;
  if( (pTab = pParse->pNewTable)==0 || (pFKey = pTab->pFKey)==0 ) return;
  assert( isDeferred==0 || isDeferred==1 ); /* EV: R-30323-21917 */
  pFKey->isDeferred = (u8)isDeferred;
#endif
}

/*
** Generate code that will erase and refill index *pIdx.  This is
** used to initialize a newly created index or to recompute the
** content of an index in response to a REINDEX command.
**
** if memRootPage is not negative, it means that the index is newly
** created.  The register specified by memRootPage contains the
** root page number of the index.  If memRootPage is negative, then
** the index already exists and must be cleared before being refilled and
** the root page number of the index is taken from pIndex->tnum.
*/
static void sqlite3RefillIndex(Parse *pParse, Index *pIndex, int memRootPage){
  Table *pTab = pIndex->pTable;  /* The table that is indexed */
  int iTab = pParse->nTab++;     /* Btree cursor used for pTab */
  int iIdx = pParse->nTab++;     /* Btree cursor used for pIndex */
  int addr1;                     /* Address of top of loop */
  int tnum;                      /* Root page of index */
  Vdbe *v;                       /* Generate code into this virtual machine */
  KeyInfo *pKey;                 /* KeyInfo for index */
  int regIdxKey;                 /* Registers containing the index key */
  int regRecord;                 /* Register holding assemblied index record */
  sqlite3 *db = pParse->db;      /* The database connection */
  int iDb = sqlite3SchemaToIndex(db, pIndex->pSchema);

#ifndef SQLITE_OMIT_AUTHORIZATION
  if( sqlite3AuthCheck(pParse, SQLITE_REINDEX, pIndex->zName, 0,
      db->aDb[iDb].zName ) ){
    return;
  }
#endif

  /* Require a write-lock on the table to perform this operation */
  sqlite3TableLock(pParse, iDb, pTab->tnum, 1, pTab->zName);

  v = sqlite3GetVdbe(pParse);
  if( v==0 ) return;
  if( memRootPage>=0 ){
    tnum = memRootPage;
  }else{
    tnum = pIndex->tnum;
    sqlite3VdbeAddOp2(v, OP_Clear, tnum, iDb);
  }
  pKey = sqlite3IndexKeyinfo(pParse, pIndex);
  sqlite3VdbeAddOp4(v, OP_OpenWrite, iIdx, tnum, iDb, 
                    (char *)pKey, P4_KEYINFO_HANDOFF);
  if( memRootPage>=0 ){
    sqlite3VdbeChangeP5(v, 1);
  }
  sqlite3OpenTable(pParse, iTab, iDb, pTab, OP_OpenRead);
  addr1 = sqlite3VdbeAddOp2(v, OP_Rewind, iTab, 0);
  regRecord = sqlite3GetTempReg(pParse);
  regIdxKey = sqlite3GenerateIndexKey(pParse, pIndex, iTab, regRecord, 1);
  if( pIndex->onError!=OE_None ){
    const int regRowid = regIdxKey + pIndex->nColumn;
    const int j2 = sqlite3VdbeCurrentAddr(v) + 2;
    void * const pRegKey = SQLITE_INT_TO_PTR(regIdxKey);

    /* The registers accessed by the OP_IsUnique opcode were allocated
    ** using sqlite3GetTempRange() inside of the sqlite3GenerateIndexKey()
    ** call above. Just before that function was freed they were released
    ** (made available to the compiler for reuse) using 
    ** sqlite3ReleaseTempRange(). So in some ways having the OP_IsUnique
    ** opcode use the values stored within seems dangerous. However, since
    ** we can be sure that no other temp registers have been allocated
    ** since sqlite3ReleaseTempRange() was called, it is safe to do so.
    */
    sqlite3VdbeAddOp4(v, OP_IsUnique, iIdx, j2, regRowid, pRegKey, P4_INT32);
    sqlite3HaltConstraint(
        pParse, OE_Abort, "indexed columns are not unique", P4_STATIC);
  }
  sqlite3VdbeAddOp2(v, OP_IdxInsert, iIdx, regRecord);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  sqlite3ReleaseTempReg(pParse, regRecord);
  sqlite3VdbeAddOp2(v, OP_Next, iTab, addr1+1);
  sqlite3VdbeJumpHere(v, addr1);
  sqlite3VdbeAddOp1(v, OP_Close, iTab);
  sqlite3VdbeAddOp1(v, OP_Close, iIdx);
}

/*
** Create a new index for an SQL table.  pName1.pName2 is the name of the index 
** and pTblList is the name of the table that is to be indexed.  Both will 
** be NULL for a primary key or an index that is created to satisfy a
** UNIQUE constraint.  If pTable and pIndex are NULL, use pParse->pNewTable
** as the table to be indexed.  pParse->pNewTable is a table that is
** currently being constructed by a CREATE TABLE statement.
**
** pList is a list of columns to be indexed.  pList will be NULL if this
** is a primary key or unique-constraint on the most recent column added
** to the table currently under construction.  
**
** If the index is created successfully, return a pointer to the new Index
** structure. This is used by sqlite3AddPrimaryKey() to mark the index
** as the tables primary key (Index.autoIndex==2).
*/
SQLITE_PRIVATE Index *sqlite3CreateIndex(
  Parse *pParse,     /* All information about this parse */
  Token *pName1,     /* First part of index name. May be NULL */
  Token *pName2,     /* Second part of index name. May be NULL */
  SrcList *pTblName, /* Table to index. Use pParse->pNewTable if 0 */
  ExprList *pList,   /* A list of columns to be indexed */
  int onError,       /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  Token *pStart,     /* The CREATE token that begins this statement */
  Token *pEnd,       /* The ")" that closes the CREATE INDEX statement */
  int sortOrder,     /* Sort order of primary key when pList==NULL */
  int ifNotExist     /* Omit error if index already exists */
){
  Index *pRet = 0;     /* Pointer to return */
  Table *pTab = 0;     /* Table to be indexed */
  Index *pIndex = 0;   /* The index to be created */
  char *zName = 0;     /* Name of the index */
  int nName;           /* Number of characters in zName */
  int i, j;
  Token nullId;        /* Fake token for an empty ID list */
  DbFixer sFix;        /* For assigning database names to pTable */
  int sortOrderMask;   /* 1 to honor DESC in index.  0 to ignore. */
  sqlite3 *db = pParse->db;
  Db *pDb;             /* The specific table containing the indexed database */
  int iDb;             /* Index of the database that is being written */
  Token *pName = 0;    /* Unqualified name of the index to create */
  struct ExprList_item *pListItem; /* For looping over pList */
  int nCol;
  int nExtra = 0;
  char *zExtra;

  assert( pStart==0 || pEnd!=0 ); /* pEnd must be non-NULL if pStart is */
  assert( pParse->nErr==0 );      /* Never called with prior errors */
  if( db->mallocFailed || IN_DECLARE_VTAB ){
    goto exit_create_index;
  }
  if( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    goto exit_create_index;
  }

  /*
  ** Find the table that is to be indexed.  Return early if not found.
  */
  if( pTblName!=0 ){

    /* Use the two-part index name to determine the database 
    ** to search for the table. 'Fix' the table name to this db
    ** before looking up the table.
    */
    assert( pName1 && pName2 );
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    if( iDb<0 ) goto exit_create_index;

#ifndef SQLITE_OMIT_TEMPDB
    /* If the index name was unqualified, check if the the table
    ** is a temp table. If so, set the database to 1. Do not do this
    ** if initialising a database schema.
    */
    if( !db->init.busy ){
      pTab = sqlite3SrcListLookup(pParse, pTblName);
      if( pName2->n==0 && pTab && pTab->pSchema==db->aDb[1].pSchema ){
        iDb = 1;
      }
    }
#endif

    if( sqlite3FixInit(&sFix, pParse, iDb, "index", pName) &&
        sqlite3FixSrcList(&sFix, pTblName)
    ){
      /* Because the parser constructs pTblName from a single identifier,
      ** sqlite3FixSrcList can never fail. */
      assert(0);
    }
    pTab = sqlite3LocateTable(pParse, 0, pTblName->a[0].zName, 
        pTblName->a[0].zDatabase);
    if( !pTab || db->mallocFailed ) goto exit_create_index;
    assert( db->aDb[iDb].pSchema==pTab->pSchema );
  }else{
    assert( pName==0 );
    pTab = pParse->pNewTable;
    if( !pTab ) goto exit_create_index;
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  }
  pDb = &db->aDb[iDb];

  assert( pTab!=0 );
  assert( pParse->nErr==0 );
  if( sqlite3StrNICmp(pTab->zName, "sqlite_", 7)==0 
       && memcmp(&pTab->zName[7],"altertab_",9)!=0 ){
    sqlite3ErrorMsg(pParse, "table %s may not be indexed", pTab->zName);
    goto exit_create_index;
  }
#ifndef SQLITE_OMIT_VIEW
  if( pTab->pSelect ){
    sqlite3ErrorMsg(pParse, "views may not be indexed");
    goto exit_create_index;
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  if( IsVirtual(pTab) ){
    sqlite3ErrorMsg(pParse, "virtual tables may not be indexed");
    goto exit_create_index;
  }
#endif

  /*
  ** Find the name of the index.  Make sure there is not already another
  ** index or table with the same name.  
  **
  ** Exception:  If we are reading the names of permanent indices from the
  ** sqlite_master table (because some other process changed the schema) and
  ** one of the index names collides with the name of a temporary table or
  ** index, then we will continue to process this index.
  **
  ** If pName==0 it means that we are
  ** dealing with a primary key or UNIQUE constraint.  We have to invent our
  ** own name.
  */
  if( pName ){
    zName = sqlite3NameFromToken(db, pName);
    if( zName==0 ) goto exit_create_index;
    if( SQLITE_OK!=sqlite3CheckObjectName(pParse, zName) ){
      goto exit_create_index;
    }
    if( !db->init.busy ){
      if( sqlite3FindTable(db, zName, 0)!=0 ){
        sqlite3ErrorMsg(pParse, "there is already a table named %s", zName);
        goto exit_create_index;
      }
    }
    if( sqlite3FindIndex(db, zName, pDb->zName)!=0 ){
      if( !ifNotExist ){
        sqlite3ErrorMsg(pParse, "index %s already exists", zName);
      }else{
        assert( !db->init.busy );
        sqlite3CodeVerifySchema(pParse, iDb);
      }
      goto exit_create_index;
    }
  }else{
    int n;
    Index *pLoop;
    for(pLoop=pTab->pIndex, n=1; pLoop; pLoop=pLoop->pNext, n++){}
    zName = sqlite3MPrintf(db, "sqlite_autoindex_%s_%d", pTab->zName, n);
    if( zName==0 ){
      goto exit_create_index;
    }
  }

  /* Check for authorization to create an index.
  */
#ifndef SQLITE_OMIT_AUTHORIZATION
  {
    const char *zDb = pDb->zName;
    if( sqlite3AuthCheck(pParse, SQLITE_INSERT, SCHEMA_TABLE(iDb), 0, zDb) ){
      goto exit_create_index;
    }
    i = SQLITE_CREATE_INDEX;
    if( !OMIT_TEMPDB && iDb==1 ) i = SQLITE_CREATE_TEMP_INDEX;
    if( sqlite3AuthCheck(pParse, i, zName, pTab->zName, zDb) ){
      goto exit_create_index;
    }
  }
#endif

  /* If pList==0, it means this routine was called to make a primary
  ** key out of the last column added to the table under construction.
  ** So create a fake list to simulate this.
  */
  if( pList==0 ){
    nullId.z = pTab->aCol[pTab->nCol-1].zName;
    nullId.n = sqlite3Strlen30((char*)nullId.z);
    pList = sqlite3ExprListAppend(pParse, 0, 0);
    if( pList==0 ) goto exit_create_index;
    sqlite3ExprListSetName(pParse, pList, &nullId, 0);
    pList->a[0].sortOrder = (u8)sortOrder;
  }

  /* Figure out how many bytes of space are required to store explicitly
  ** specified collation sequence names.
  */
  for(i=0; i<pList->nExpr; i++){
    Expr *pExpr = pList->a[i].pExpr;
    if( pExpr ){
      CollSeq *pColl = pExpr->pColl;
      /* Either pColl!=0 or there was an OOM failure.  But if an OOM
      ** failure we have quit before reaching this point. */
      if( ALWAYS(pColl) ){
        nExtra += (1 + sqlite3Strlen30(pColl->zName));
      }
    }
  }

  /* 
  ** Allocate the index structure. 
  */
  nName = sqlite3Strlen30(zName);
  nCol = pList->nExpr;
  pIndex = sqlite3DbMallocZero(db, 
      sizeof(Index) +              /* Index structure  */
      sizeof(int)*nCol +           /* Index.aiColumn   */
      sizeof(int)*(nCol+1) +       /* Index.aiRowEst   */
      sizeof(char *)*nCol +        /* Index.azColl     */
      sizeof(u8)*nCol +            /* Index.aSortOrder */
      nName + 1 +                  /* Index.zName      */
      nExtra                       /* Collation sequence names */
  );
  if( db->mallocFailed ){
    goto exit_create_index;
  }
  pIndex->azColl = (char**)(&pIndex[1]);
  pIndex->aiColumn = (int *)(&pIndex->azColl[nCol]);
  pIndex->aiRowEst = (unsigned *)(&pIndex->aiColumn[nCol]);
  pIndex->aSortOrder = (u8 *)(&pIndex->aiRowEst[nCol+1]);
  pIndex->zName = (char *)(&pIndex->aSortOrder[nCol]);
  zExtra = (char *)(&pIndex->zName[nName+1]);
  memcpy(pIndex->zName, zName, nName+1);
  pIndex->pTable = pTab;
  pIndex->nColumn = pList->nExpr;
  pIndex->onError = (u8)onError;
  pIndex->autoIndex = (u8)(pName==0);
  pIndex->pSchema = db->aDb[iDb].pSchema;
  assert( sqlite3SchemaMutexHeld(db, iDb, 0) );

  /* Check to see if we should honor DESC requests on index columns
  */
  if( pDb->pSchema->file_format>=4 ){
    sortOrderMask = -1;   /* Honor DESC */
  }else{
    sortOrderMask = 0;    /* Ignore DESC */
  }

  /* Scan the names of the columns of the table to be indexed and
  ** load the column indices into the Index structure.  Report an error
  ** if any column is not found.
  **
  ** TODO:  Add a test to make sure that the same column is not named
  ** more than once within the same index.  Only the first instance of
  ** the column will ever be used by the optimizer.  Note that using the
  ** same column more than once cannot be an error because that would 
  ** break backwards compatibility - it needs to be a warning.
  */
  for(i=0, pListItem=pList->a; i<pList->nExpr; i++, pListItem++){
    const char *zColName = pListItem->zName;
    Column *pTabCol;
    int requestedSortOrder;
    char *zColl;                   /* Collation sequence name */

    for(j=0, pTabCol=pTab->aCol; j<pTab->nCol; j++, pTabCol++){
      if( sqlite3StrICmp(zColName, pTabCol->zName)==0 ) break;
    }
    if( j>=pTab->nCol ){
      sqlite3ErrorMsg(pParse, "table %s has no column named %s",
        pTab->zName, zColName);
      pParse->checkSchema = 1;
      goto exit_create_index;
    }
    pIndex->aiColumn[i] = j;
    /* Justification of the ALWAYS(pListItem->pExpr->pColl):  Because of
    ** the way the "idxlist" non-terminal is constructed by the parser,
    ** if pListItem->pExpr is not null then either pListItem->pExpr->pColl
    ** must exist or else there must have been an OOM error.  But if there
    ** was an OOM error, we would never reach this point. */
    if( pListItem->pExpr && ALWAYS(pListItem->pExpr->pColl) ){
      int nColl;
      zColl = pListItem->pExpr->pColl->zName;
      nColl = sqlite3Strlen30(zColl) + 1;
      assert( nExtra>=nColl );
      memcpy(zExtra, zColl, nColl);
      zColl = zExtra;
      zExtra += nColl;
      nExtra -= nColl;
    }else{
      zColl = pTab->aCol[j].zColl;
      if( !zColl ){
        zColl = db->pDfltColl->zName;
      }
    }
    if( !db->init.busy && !sqlite3LocateCollSeq(pParse, zColl) ){
      goto exit_create_index;
    }
    pIndex->azColl[i] = zColl;
    requestedSortOrder = pListItem->sortOrder & sortOrderMask;
    pIndex->aSortOrder[i] = (u8)requestedSortOrder;
  }
  sqlite3DefaultRowEst(pIndex);

  if( pTab==pParse->pNewTable ){
    /* This routine has been called to create an automatic index as a
    ** result of a PRIMARY KEY or UNIQUE clause on a column definition, or
    ** a PRIMARY KEY or UNIQUE clause following the column definitions.
    ** i.e. one of:
    **
    ** CREATE TABLE t(x PRIMARY KEY, y);
    ** CREATE TABLE t(x, y, UNIQUE(x, y));
    **
    ** Either way, check to see if the table already has such an index. If
    ** so, don't bother creating this one. This only applies to
    ** automatically created indices. Users can do as they wish with
    ** explicit indices.
    **
    ** Two UNIQUE or PRIMARY KEY constraints are considered equivalent
    ** (and thus suppressing the second one) even if they have different
    ** sort orders.
    **
    ** If there are different collating sequences or if the columns of
    ** the constraint occur in different orders, then the constraints are
    ** considered distinct and both result in separate indices.
    */
    Index *pIdx;
    for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      int k;
      assert( pIdx->onError!=OE_None );
      assert( pIdx->autoIndex );
      assert( pIndex->onError!=OE_None );

      if( pIdx->nColumn!=pIndex->nColumn ) continue;
      for(k=0; k<pIdx->nColumn; k++){
        const char *z1;
        const char *z2;
        if( pIdx->aiColumn[k]!=pIndex->aiColumn[k] ) break;
        z1 = pIdx->azColl[k];
        z2 = pIndex->azColl[k];
        if( z1!=z2 && sqlite3StrICmp(z1, z2) ) break;
      }
      if( k==pIdx->nColumn ){
        if( pIdx->onError!=pIndex->onError ){
          /* This constraint creates the same index as a previous
          ** constraint specified somewhere in the CREATE TABLE statement.
          ** However the ON CONFLICT clauses are different. If both this 
          ** constraint and the previous equivalent constraint have explicit
          ** ON CONFLICT clauses this is an error. Otherwise, use the
          ** explicitly specified behaviour for the index.
          */
          if( !(pIdx->onError==OE_Default || pIndex->onError==OE_Default) ){
            sqlite3ErrorMsg(pParse, 
                "conflicting ON CONFLICT clauses specified", 0);
          }
          if( pIdx->onError==OE_Default ){
            pIdx->onError = pIndex->onError;
          }
        }
        goto exit_create_index;
      }
    }
  }

  /* Link the new Index structure to its table and to the other
  ** in-memory database structures. 
  */
  if( db->init.busy ){
    Index *p;
    assert( sqlite3SchemaMutexHeld(db, 0, pIndex->pSchema) );
    p = sqlite3HashInsert(&pIndex->pSchema->idxHash, 
                          pIndex->zName, sqlite3Strlen30(pIndex->zName),
                          pIndex);
    if( p ){
      assert( p==pIndex );  /* Malloc must have failed */
      db->mallocFailed = 1;
      goto exit_create_index;
    }
    db->flags |= SQLITE_InternChanges;
    if( pTblName!=0 ){
      pIndex->tnum = db->init.newTnum;
    }
  }

  /* If the db->init.busy is 0 then create the index on disk.  This
  ** involves writing the index into the master table and filling in the
  ** index with the current table contents.
  **
  ** The db->init.busy is 0 when the user first enters a CREATE INDEX 
  ** command.  db->init.busy is 1 when a database is opened and 
  ** CREATE INDEX statements are read out of the master table.  In
  ** the latter case the index already exists on disk, which is why
  ** we don't want to recreate it.
  **
  ** If pTblName==0 it means this index is generated as a primary key
  ** or UNIQUE constraint of a CREATE TABLE statement.  Since the table
  ** has just been created, it contains no data and the index initialization
  ** step can be skipped.
  */
  else{ /* if( db->init.busy==0 ) */
    Vdbe *v;
    char *zStmt;
    int iMem = ++pParse->nMem;

    v = sqlite3GetVdbe(pParse);
    if( v==0 ) goto exit_create_index;


    /* Create the rootpage for the index
    */
    sqlite3BeginWriteOperation(pParse, 1, iDb);
    sqlite3VdbeAddOp2(v, OP_CreateIndex, iDb, iMem);

    /* Gather the complete text of the CREATE INDEX statement into
    ** the zStmt variable
    */
    if( pStart ){
      assert( pEnd!=0 );
      /* A named index with an explicit CREATE INDEX statement */
      zStmt = sqlite3MPrintf(db, "CREATE%s INDEX %.*s",
        onError==OE_None ? "" : " UNIQUE",
        pEnd->z - pName->z + 1,
        pName->z);
    }else{
      /* An automatic index created by a PRIMARY KEY or UNIQUE constraint */
      /* zStmt = sqlite3MPrintf(""); */
      zStmt = 0;
    }

    /* Add an entry in sqlite_master for this index
    */
    sqlite3NestedParse(pParse, 
        "INSERT INTO %Q.%s VALUES('index',%Q,%Q,#%d,%Q);",
        db->aDb[iDb].zName, SCHEMA_TABLE(iDb),
        pIndex->zName,
        pTab->zName,
        iMem,
        zStmt
    );
    sqlite3DbFree(db, zStmt);

    /* Fill the index with data and reparse the schema. Code an OP_Expire
    ** to invalidate all pre-compiled statements.
    */
    if( pTblName ){
      sqlite3RefillIndex(pParse, pIndex, iMem);
      sqlite3ChangeCookie(pParse, iDb);
      sqlite3VdbeAddParseSchemaOp(v, iDb,
         sqlite3MPrintf(db, "name='%q' AND type='index'", pIndex->zName));
      sqlite3VdbeAddOp1(v, OP_Expire, 0);
    }
  }

  /* When adding an index to the list of indices for a table, make
  ** sure all indices labeled OE_Replace come after all those labeled
  ** OE_Ignore.  This is necessary for the correct constraint check
  ** processing (in sqlite3GenerateConstraintChecks()) as part of
  ** UPDATE and INSERT statements.  
  */
  if( db->init.busy || pTblName==0 ){
    if( onError!=OE_Replace || pTab->pIndex==0
         || pTab->pIndex->onError==OE_Replace){
      pIndex->pNext = pTab->pIndex;
      pTab->pIndex = pIndex;
    }else{
      Index *pOther = pTab->pIndex;
      while( pOther->pNext && pOther->pNext->onError!=OE_Replace ){
        pOther = pOther->pNext;
      }
      pIndex->pNext = pOther->pNext;
      pOther->pNext = pIndex;
    }
    pRet = pIndex;
    pIndex = 0;
  }

  /* Clean up before exiting */
exit_create_index:
  if( pIndex ){
    sqlite3DbFree(db, pIndex->zColAff);
    sqlite3DbFree(db, pIndex);
  }
  sqlite3ExprListDelete(db, pList);
  sqlite3SrcListDelete(db, pTblName);
  sqlite3DbFree(db, zName);
  return pRet;
}

/*
** Fill the Index.aiRowEst[] array with default information - information
** to be used when we have not run the ANALYZE command.
**
** aiRowEst[0] is suppose to contain the number of elements in the index.
** Since we do not know, guess 1 million.  aiRowEst[1] is an estimate of the
** number of rows in the table that match any particular value of the
** first column of the index.  aiRowEst[2] is an estimate of the number
** of rows that match any particular combiniation of the first 2 columns
** of the index.  And so forth.  It must always be the case that
*
**           aiRowEst[N]<=aiRowEst[N-1]
**           aiRowEst[N]>=1
**
** Apart from that, we have little to go on besides intuition as to
** how aiRowEst[] should be initialized.  The numbers generated here
** are based on typical values found in actual indices.
*/
SQLITE_PRIVATE void sqlite3DefaultRowEst(Index *pIdx){
  unsigned *a = pIdx->aiRowEst;
  int i;
  unsigned n;
  assert( a!=0 );
  a[0] = pIdx->pTable->nRowEst;
  if( a[0]<10 ) a[0] = 10;
  n = 10;
  for(i=1; i<=pIdx->nColumn; i++){
    a[i] = n;
    if( n>5 ) n--;
  }
  if( pIdx->onError!=OE_None ){
    a[pIdx->nColumn] = 1;
  }
}

/*
** This routine will drop an existing named index.  This routine
** implements the DROP INDEX statement.
*/
SQLITE_PRIVATE void sqlite3DropIndex(Parse *pParse, SrcList *pName, int ifExists){
  Index *pIndex;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  int iDb;

  assert( pParse->nErr==0 );   /* Never called with prior errors */
  if( db->mallocFailed ){
    goto exit_drop_index;
  }
  assert( pName->nSrc==1 );
  if( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    goto exit_drop_index;
  }
  pIndex = sqlite3FindIndex(db, pName->a[0].zName, pName->a[0].zDatabase);
  if( pIndex==0 ){
    if( !ifExists ){
      sqlite3ErrorMsg(pParse, "no such index: %S", pName, 0);
    }else{
      sqlite3CodeVerifyNamedSchema(pParse, pName->a[0].zDatabase);
    }
    pParse->checkSchema = 1;
    goto exit_drop_index;
  }
  if( pIndex->autoIndex ){
    sqlite3ErrorMsg(pParse, "index associated with UNIQUE "
      "or PRIMARY KEY constraint cannot be dropped", 0);
    goto exit_drop_index;
  }
  iDb = sqlite3SchemaToIndex(db, pIndex->pSchema);
#ifndef SQLITE_OMIT_AUTHORIZATION
  {
    int code = SQLITE_DROP_INDEX;
    Table *pTab = pIndex->pTable;
    const char *zDb = db->aDb[iDb].zName;
    const char *zTab = SCHEMA_TABLE(iDb);
    if( sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb) ){
      goto exit_drop_index;
    }
    if( !OMIT_TEMPDB && iDb ) code = SQLITE_DROP_TEMP_INDEX;
    if( sqlite3AuthCheck(pParse, code, pIndex->zName, pTab->zName, zDb) ){
      goto exit_drop_index;
    }
  }
#endif

  /* Generate code to remove the index and from the master table */
  v = sqlite3GetVdbe(pParse);
  if( v ){
    sqlite3BeginWriteOperation(pParse, 1, iDb);
    sqlite3NestedParse(pParse,
       "DELETE FROM %Q.%s WHERE name=%Q AND type='index'",
       db->aDb[iDb].zName, SCHEMA_TABLE(iDb),
       pIndex->zName
    );
    if( sqlite3FindTable(db, "sqlite_stat1", db->aDb[iDb].zName) ){
      sqlite3NestedParse(pParse,
        "DELETE FROM %Q.sqlite_stat1 WHERE idx=%Q",
        db->aDb[iDb].zName, pIndex->zName
      );
    }
    sqlite3ChangeCookie(pParse, iDb);
    destroyRootPage(pParse, pIndex->tnum, iDb);
    sqlite3VdbeAddOp4(v, OP_DropIndex, iDb, 0, 0, pIndex->zName, 0);
  }

exit_drop_index:
  sqlite3SrcListDelete(db, pName);
}

/*
** pArray is a pointer to an array of objects.  Each object in the
** array is szEntry bytes in size.  This routine allocates a new
** object on the end of the array.
**
** *pnEntry is the number of entries already in use.  *pnAlloc is
** the previously allocated size of the array.  initSize is the
** suggested initial array size allocation.
**
** The index of the new entry is returned in *pIdx.
**
** This routine returns a pointer to the array of objects.  This
** might be the same as the pArray parameter or it might be a different
** pointer if the array was resized.
*/
SQLITE_PRIVATE void *sqlite3ArrayAllocate(
  sqlite3 *db,      /* Connection to notify of malloc failures */
  void *pArray,     /* Array of objects.  Might be reallocated */
  int szEntry,      /* Size of each object in the array */
  int initSize,     /* Suggested initial allocation, in elements */
  int *pnEntry,     /* Number of objects currently in use */
  int *pnAlloc,     /* Current size of the allocation, in elements */
  int *pIdx         /* Write the index of a new slot here */
){
  char *z;
  if( *pnEntry >= *pnAlloc ){
    void *pNew;
    int newSize;
    newSize = (*pnAlloc)*2 + initSize;
    pNew = sqlite3DbRealloc(db, pArray, newSize*szEntry);
    if( pNew==0 ){
      *pIdx = -1;
      return pArray;
    }
    *pnAlloc = sqlite3DbMallocSize(db, pNew)/szEntry;
    pArray = pNew;
  }
  z = (char*)pArray;
  memset(&z[*pnEntry * szEntry], 0, szEntry);
  *pIdx = *pnEntry;
  ++*pnEntry;
  return pArray;
}

/*
** Append a new element to the given IdList.  Create a new IdList if
** need be.
**
** A new IdList is returned, or NULL if malloc() fails.
*/
SQLITE_PRIVATE IdList *sqlite3IdListAppend(sqlite3 *db, IdList *pList, Token *pToken){
  int i;
  if( pList==0 ){
    pList = sqlite3DbMallocZero(db, sizeof(IdList) );
    if( pList==0 ) return 0;
    pList->nAlloc = 0;
  }
  pList->a = sqlite3ArrayAllocate(
      db,
      pList->a,
      sizeof(pList->a[0]),
      5,
      &pList->nId,
      &pList->nAlloc,
      &i
  );
  if( i<0 ){
    sqlite3IdListDelete(db, pList);
    return 0;
  }
  pList->a[i].zName = sqlite3NameFromToken(db, pToken);
  return pList;
}

/*
** Delete an IdList.
*/
SQLITE_PRIVATE void sqlite3IdListDelete(sqlite3 *db, IdList *pList){
  int i;
  if( pList==0 ) return;
  for(i=0; i<pList->nId; i++){
    sqlite3DbFree(db, pList->a[i].zName);
  }
  sqlite3DbFree(db, pList->a);
  sqlite3DbFree(db, pList);
}

/*
** Return the index in pList of the identifier named zId.  Return -1
** if not found.
*/
SQLITE_PRIVATE int sqlite3IdListIndex(IdList *pList, const char *zName){
  int i;
  if( pList==0 ) return -1;
  for(i=0; i<pList->nId; i++){
    if( sqlite3StrICmp(pList->a[i].zName, zName)==0 ) return i;
  }
  return -1;
}

/*
** Expand the space allocated for the given SrcList object by
** creating nExtra new slots beginning at iStart.  iStart is zero based.
** New slots are zeroed.
**
** For example, suppose a SrcList initially contains two entries: A,B.
** To append 3 new entries onto the end, do this:
**
**    sqlite3SrcListEnlarge(db, pSrclist, 3, 2);
**
** After the call above it would contain:  A, B, nil, nil, nil.
** If the iStart argument had been 1 instead of 2, then the result
** would have been:  A, nil, nil, nil, B.  To prepend the new slots,
** the iStart value would be 0.  The result then would
** be: nil, nil, nil, A, B.
**
** If a memory allocation fails the SrcList is unchanged.  The
** db->mallocFailed flag will be set to true.
*/
SQLITE_PRIVATE SrcList *sqlite3SrcListEnlarge(
  sqlite3 *db,       /* Database connection to notify of OOM errors */
  SrcList *pSrc,     /* The SrcList to be enlarged */
  int nExtra,        /* Number of new slots to add to pSrc->a[] */
  int iStart         /* Index in pSrc->a[] of first new slot */
){
  int i;

  /* Sanity checking on calling parameters */
  assert( iStart>=0 );
  assert( nExtra>=1 );
  assert( pSrc!=0 );
  assert( iStart<=pSrc->nSrc );

  /* Allocate additional space if needed */
  if( pSrc->nSrc+nExtra>pSrc->nAlloc ){
    SrcList *pNew;
    int nAlloc = pSrc->nSrc+nExtra;
    int nGot;
    pNew = sqlite3DbRealloc(db, pSrc,
               sizeof(*pSrc) + (nAlloc-1)*sizeof(pSrc->a[0]) );
    if( pNew==0 ){
      assert( db->mallocFailed );
      return pSrc;
    }
    pSrc = pNew;
    nGot = (sqlite3DbMallocSize(db, pNew) - sizeof(*pSrc))/sizeof(pSrc->a[0])+1;
    pSrc->nAlloc = (u16)nGot;
  }

  /* Move existing slots that come after the newly inserted slots
  ** out of the way */
  for(i=pSrc->nSrc-1; i>=iStart; i--){
    pSrc->a[i+nExtra] = pSrc->a[i];
  }
  pSrc->nSrc += (i16)nExtra;

  /* Zero the newly allocated slots */
  memset(&pSrc->a[iStart], 0, sizeof(pSrc->a[0])*nExtra);
  for(i=iStart; i<iStart+nExtra; i++){
    pSrc->a[i].iCursor = -1;
  }

  /* Return a pointer to the enlarged SrcList */
  return pSrc;
}


/*
** Append a new table name to the given SrcList.  Create a new SrcList if
** need be.  A new entry is created in the SrcList even if pTable is NULL.
**
** A SrcList is returned, or NULL if there is an OOM error.  The returned
** SrcList might be the same as the SrcList that was input or it might be
** a new one.  If an OOM error does occurs, then the prior value of pList
** that is input to this routine is automatically freed.
**
** If pDatabase is not null, it means that the table has an optional
** database name prefix.  Like this:  "database.table".  The pDatabase
** points to the table name and the pTable points to the database name.
** The SrcList.a[].zName field is filled with the table name which might
** come from pTable (if pDatabase is NULL) or from pDatabase.  
** SrcList.a[].zDatabase is filled with the database name from pTable,
** or with NULL if no database is specified.
**
** In other words, if call like this:
**
**         sqlite3SrcListAppend(D,A,B,0);
**
** Then B is a table name and the database name is unspecified.  If called
** like this:
**
**         sqlite3SrcListAppend(D,A,B,C);
**
** Then C is the table name and B is the database name.  If C is defined
** then so is B.  In other words, we never have a case where:
**
**         sqlite3SrcListAppend(D,A,0,C);
**
** Both pTable and pDatabase are assumed to be quoted.  They are dequoted
** before being added to the SrcList.
*/
SQLITE_PRIVATE SrcList *sqlite3SrcListAppend(
  sqlite3 *db,        /* Connection to notify of malloc failures */
  SrcList *pList,     /* Append to this SrcList. NULL creates a new SrcList */
  Token *pTable,      /* Table to append */
  Token *pDatabase    /* Database of the table */
){
  struct SrcList_item *pItem;
  assert( pDatabase==0 || pTable!=0 );  /* Cannot have C without B */
  if( pList==0 ){
    pList = sqlite3DbMallocZero(db, sizeof(SrcList) );
    if( pList==0 ) return 0;
    pList->nAlloc = 1;
  }
  pList = sqlite3SrcListEnlarge(db, pList, 1, pList->nSrc);
  if( db->mallocFailed ){
    sqlite3SrcListDelete(db, pList);
    return 0;
  }
  pItem = &pList->a[pList->nSrc-1];
  if( pDatabase && pDatabase->z==0 ){
    pDatabase = 0;
  }
  if( pDatabase ){
    Token *pTemp = pDatabase;
    pDatabase = pTable;
    pTable = pTemp;
  }
  pItem->zName = sqlite3NameFromToken(db, pTable);
  pItem->zDatabase = sqlite3NameFromToken(db, pDatabase);
  return pList;
}

/*
** Assign VdbeCursor index numbers to all tables in a SrcList
*/
SQLITE_PRIVATE void sqlite3SrcListAssignCursors(Parse *pParse, SrcList *pList){
  int i;
  struct SrcList_item *pItem;
  assert(pList || pParse->db->mallocFailed );
  if( pList ){
    for(i=0, pItem=pList->a; i<pList->nSrc; i++, pItem++){
      if( pItem->iCursor>=0 ) break;
      pItem->iCursor = pParse->nTab++;
      if( pItem->pSelect ){
        sqlite3SrcListAssignCursors(pParse, pItem->pSelect->pSrc);
      }
    }
  }
}

/*
** Delete an entire SrcList including all its substructure.
*/
SQLITE_PRIVATE void sqlite3SrcListDelete(sqlite3 *db, SrcList *pList){
  int i;
  struct SrcList_item *pItem;
  if( pList==0 ) return;
  for(pItem=pList->a, i=0; i<pList->nSrc; i++, pItem++){
    sqlite3DbFree(db, pItem->zDatabase);
    sqlite3DbFree(db, pItem->zName);
    sqlite3DbFree(db, pItem->zAlias);
    sqlite3DbFree(db, pItem->zIndex);
    sqlite3DeleteTable(db, pItem->pTab);
    sqlite3SelectDelete(db, pItem->pSelect);
    sqlite3ExprDelete(db, pItem->pOn);
    sqlite3IdListDelete(db, pItem->pUsing);
  }
  sqlite3DbFree(db, pList);
}

/*
** This routine is called by the parser to add a new term to the
** end of a growing FROM clause.  The "p" parameter is the part of
** the FROM clause that has already been constructed.  "p" is NULL
** if this is the first term of the FROM clause.  pTable and pDatabase
** are the name of the table and database named in the FROM clause term.
** pDatabase is NULL if the database name qualifier is missing - the
** usual case.  If the term has a alias, then pAlias points to the
** alias token.  If the term is a subquery, then pSubquery is the
** SELECT statement that the subquery encodes.  The pTable and
** pDatabase parameters are NULL for subqueries.  The pOn and pUsing
** parameters are the content of the ON and USING clauses.
**
** Return a new SrcList which encodes is the FROM with the new
** term added.
*/
SQLITE_PRIVATE SrcList *sqlite3SrcListAppendFromTerm(
  Parse *pParse,          /* Parsing context */
  SrcList *p,             /* The left part of the FROM clause already seen */
  Token *pTable,          /* Name of the table to add to the FROM clause */
  Token *pDatabase,       /* Name of the database containing pTable */
  Token *pAlias,          /* The right-hand side of the AS subexpression */
  Select *pSubquery,      /* A subquery used in place of a table name */
  Expr *pOn,              /* The ON clause of a join */
  IdList *pUsing          /* The USING clause of a join */
){
  struct SrcList_item *pItem;
  sqlite3 *db = pParse->db;
  if( !p && (pOn || pUsing) ){
    sqlite3ErrorMsg(pParse, "a JOIN clause is required before %s", 
      (pOn ? "ON" : "USING")
    );
    goto append_from_error;
  }
  p = sqlite3SrcListAppend(db, p, pTable, pDatabase);
  if( p==0 || NEVER(p->nSrc==0) ){
    goto append_from_error;
  }
  pItem = &p->a[p->nSrc-1];
  assert( pAlias!=0 );
  if( pAlias->n ){
    pItem->zAlias = sqlite3NameFromToken(db, pAlias);
  }
  pItem->pSelect = pSubquery;
  pItem->pOn = pOn;
  pItem->pUsing = pUsing;
  return p;

 append_from_error:
  assert( p==0 );
  sqlite3ExprDelete(db, pOn);
  sqlite3IdListDelete(db, pUsing);
  sqlite3SelectDelete(db, pSubquery);
  return 0;
}

/*
** Add an INDEXED BY or NOT INDEXED clause to the most recently added 
** element of the source-list passed as the second argument.
*/
SQLITE_PRIVATE void sqlite3SrcListIndexedBy(Parse *pParse, SrcList *p, Token *pIndexedBy){
  assert( pIndexedBy!=0 );
  if( p && ALWAYS(p->nSrc>0) ){
    struct SrcList_item *pItem = &p->a[p->nSrc-1];
    assert( pItem->notIndexed==0 && pItem->zIndex==0 );
    if( pIndexedBy->n==1 && !pIndexedBy->z ){
      /* A "NOT INDEXED" clause was supplied. See parse.y 
      ** construct "indexed_opt" for details. */
      pItem->notIndexed = 1;
    }else{
      pItem->zIndex = sqlite3NameFromToken(pParse->db, pIndexedBy);
    }
  }
}

/*
** When building up a FROM clause in the parser, the join operator
** is initially attached to the left operand.  But the code generator
** expects the join operator to be on the right operand.  This routine
** Shifts all join operators from left to right for an entire FROM
** clause.
**
** Example: Suppose the join is like this:
**
**           A natural cross join B
**
** The operator is "natural cross join".  The A and B operands are stored
** in p->a[0] and p->a[1], respectively.  The parser initially stores the
** operator with A.  This routine shifts that operator over to B.
*/
SQLITE_PRIVATE void sqlite3SrcListShiftJoinType(SrcList *p){
  if( p && p->a ){
    int i;
    for(i=p->nSrc-1; i>0; i--){
      p->a[i].jointype = p->a[i-1].jointype;
    }
    p->a[0].jointype = 0;
  }
}

/*
** Begin a transaction
*/
SQLITE_PRIVATE void sqlite3BeginTransaction(Parse *pParse, int type){
  sqlite3 *db;
  Vdbe *v;
  int i;

  assert( pParse!=0 );
  db = pParse->db;
  assert( db!=0 );
/*  if( db->aDb[0].pBt==0 ) return; */
  if( sqlite3AuthCheck(pParse, SQLITE_TRANSACTION, "BEGIN", 0, 0) ){
    return;
  }
  v = sqlite3GetVdbe(pParse);
  if( !v ) return;
  if( type!=TK_DEFERRED ){
    for(i=0; i<db->nDb; i++){
      sqlite3VdbeAddOp2(v, OP_Transaction, i, (type==TK_EXCLUSIVE)+1);
      sqlite3VdbeUsesBtree(v, i);
    }
  }
  sqlite3VdbeAddOp2(v, OP_AutoCommit, 0, 0);
}

/*
** Commit a transaction
*/
SQLITE_PRIVATE void sqlite3CommitTransaction(Parse *pParse){
  sqlite3 *db;
  Vdbe *v;

  assert( pParse!=0 );
  db = pParse->db;
  assert( db!=0 );
/*  if( db->aDb[0].pBt==0 ) return; */
  if( sqlite3AuthCheck(pParse, SQLITE_TRANSACTION, "COMMIT", 0, 0) ){
    return;
  }
  v = sqlite3GetVdbe(pParse);
  if( v ){
    sqlite3VdbeAddOp2(v, OP_AutoCommit, 1, 0);
  }
}

/*
** Rollback a transaction
*/
SQLITE_PRIVATE void sqlite3RollbackTransaction(Parse *pParse){
  sqlite3 *db;
  Vdbe *v;

  assert( pParse!=0 );
  db = pParse->db;
  assert( db!=0 );
/*  if( db->aDb[0].pBt==0 ) return; */
  if( sqlite3AuthCheck(pParse, SQLITE_TRANSACTION, "ROLLBACK", 0, 0) ){
    return;
  }
  v = sqlite3GetVdbe(pParse);
  if( v ){
    sqlite3VdbeAddOp2(v, OP_AutoCommit, 1, 1);
  }
}

/*
** This function is called by the parser when it parses a command to create,
** release or rollback an SQL savepoint. 
*/
SQLITE_PRIVATE void sqlite3Savepoint(Parse *pParse, int op, Token *pName){
  char *zName = sqlite3NameFromToken(pParse->db, pName);
  if( zName ){
    Vdbe *v = sqlite3GetVdbe(pParse);
#ifndef SQLITE_OMIT_AUTHORIZATION
    static const char * const az[] = { "BEGIN", "RELEASE", "ROLLBACK" };
    assert( !SAVEPOINT_BEGIN && SAVEPOINT_RELEASE==1 && SAVEPOINT_ROLLBACK==2 );
#endif
    if( !v || sqlite3AuthCheck(pParse, SQLITE_SAVEPOINT, az[op], zName, 0) ){
      sqlite3DbFree(pParse->db, zName);
      return;
    }
    sqlite3VdbeAddOp4(v, OP_Savepoint, op, 0, 0, zName, P4_DYNAMIC);
  }
}

/*
** Make sure the TEMP database is open and available for use.  Return
** the number of errors.  Leave any error messages in the pParse structure.
*/
SQLITE_PRIVATE int sqlite3OpenTempDatabase(Parse *pParse){
  sqlite3 *db = pParse->db;
  if( db->aDb[1].pBt==0 && !pParse->explain ){
    int rc;
    Btree *pBt;
    static const int flags = 
          SQLITE_OPEN_READWRITE |
          SQLITE_OPEN_CREATE |
          SQLITE_OPEN_EXCLUSIVE |
          SQLITE_OPEN_DELETEONCLOSE |
          SQLITE_OPEN_TEMP_DB;

    rc = sqlite3BtreeOpen(db->pVfs, 0, db, &pBt, 0, flags);
    if( rc!=SQLITE_OK ){
      sqlite3ErrorMsg(pParse, "unable to open a temporary database "
        "file for storing temporary tables");
      pParse->rc = rc;
      return 1;
    }
    db->aDb[1].pBt = pBt;
    assert( db->aDb[1].pSchema );
    if( SQLITE_NOMEM==sqlite3BtreeSetPageSize(pBt, db->nextPagesize, -1, 0) ){
      db->mallocFailed = 1;
      return 1;
    }
  }
  return 0;
}

/*
** Generate VDBE code that will verify the schema cookie and start
** a read-transaction for all named database files.
**
** It is important that all schema cookies be verified and all
** read transactions be started before anything else happens in
** the VDBE program.  But this routine can be called after much other
** code has been generated.  So here is what we do:
**
** The first time this routine is called, we code an OP_Goto that
** will jump to a subroutine at the end of the program.  Then we
** record every database that needs its schema verified in the
** pParse->cookieMask field.  Later, after all other code has been
** generated, the subroutine that does the cookie verifications and
** starts the transactions will be coded and the OP_Goto P2 value
** will be made to point to that subroutine.  The generation of the
** cookie verification subroutine code happens in sqlite3FinishCoding().
**
** If iDb<0 then code the OP_Goto only - don't set flag to verify the
** schema on any databases.  This can be used to position the OP_Goto
** early in the code, before we know if any database tables will be used.
*/
SQLITE_PRIVATE void sqlite3CodeVerifySchema(Parse *pParse, int iDb){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);

  if( pToplevel->cookieGoto==0 ){
    Vdbe *v = sqlite3GetVdbe(pToplevel);
    if( v==0 ) return;  /* This only happens if there was a prior error */
    pToplevel->cookieGoto = sqlite3VdbeAddOp2(v, OP_Goto, 0, 0)+1;
  }
  if( iDb>=0 ){
    sqlite3 *db = pToplevel->db;
    yDbMask mask;

    assert( iDb<db->nDb );
    assert( db->aDb[iDb].pBt!=0 || iDb==1 );
    assert( iDb<SQLITE_MAX_ATTACHED+2 );
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    mask = ((yDbMask)1)<<iDb;
    if( (pToplevel->cookieMask & mask)==0 ){
      pToplevel->cookieMask |= mask;
      pToplevel->cookieValue[iDb] = db->aDb[iDb].pSchema->schema_cookie;
      if( !OMIT_TEMPDB && iDb==1 ){
        sqlite3OpenTempDatabase(pToplevel);
      }
    }
  }
}

/*
** If argument zDb is NULL, then call sqlite3CodeVerifySchema() for each 
** attached database. Otherwise, invoke it for the database named zDb only.
*/
SQLITE_PRIVATE void sqlite3CodeVerifyNamedSchema(Parse *pParse, const char *zDb){
  sqlite3 *db = pParse->db;
  int i;
  for(i=0; i<db->nDb; i++){
    Db *pDb = &db->aDb[i];
    if( pDb->pBt && (!zDb || 0==sqlite3StrICmp(zDb, pDb->zName)) ){
      sqlite3CodeVerifySchema(pParse, i);
    }
  }
}

/*
** Generate VDBE code that prepares for doing an operation that
** might change the database.
**
** This routine starts a new transaction if we are not already within
** a transaction.  If we are already within a transaction, then a checkpoint
** is set if the setStatement parameter is true.  A checkpoint should
** be set for operations that might fail (due to a constraint) part of
** the way through and which will need to undo some writes without having to
** rollback the whole transaction.  For operations where all constraints
** can be checked before any changes are made to the database, it is never
** necessary to undo a write and the checkpoint should not be set.
*/
SQLITE_PRIVATE void sqlite3BeginWriteOperation(Parse *pParse, int setStatement, int iDb){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  sqlite3CodeVerifySchema(pParse, iDb);
  pToplevel->writeMask |= ((yDbMask)1)<<iDb;
  pToplevel->isMultiWrite |= setStatement;
}

/*
** Indicate that the statement currently under construction might write
** more than one entry (example: deleting one row then inserting another,
** inserting multiple rows in a table, or inserting a row and index entries.)
** If an abort occurs after some of these writes have completed, then it will
** be necessary to undo the completed writes.
*/
SQLITE_PRIVATE void sqlite3MultiWrite(Parse *pParse){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  pToplevel->isMultiWrite = 1;
}

/* 
** The code generator calls this routine if is discovers that it is
** possible to abort a statement prior to completion.  In order to 
** perform this abort without corrupting the database, we need to make
** sure that the statement is protected by a statement transaction.
**
** Technically, we only need to set the mayAbort flag if the
** isMultiWrite flag was previously set.  There is a time dependency
** such that the abort must occur after the multiwrite.  This makes
** some statements involving the REPLACE conflict resolution algorithm
** go a little faster.  But taking advantage of this time dependency
** makes it more difficult to prove that the code is correct (in 
** particular, it prevents us from writing an effective
** implementation of sqlite3AssertMayAbort()) and so we have chosen
** to take the safe route and skip the optimization.
*/
SQLITE_PRIVATE void sqlite3MayAbort(Parse *pParse){
  Parse *pToplevel = sqlite3ParseToplevel(pParse);
  pToplevel->mayAbort = 1;
}

/*
** Code an OP_Halt that causes the vdbe to return an SQLITE_CONSTRAINT
** error. The onError parameter determines which (if any) of the statement
** and/or current transaction is rolled back.
*/
SQLITE_PRIVATE void sqlite3HaltConstraint(Parse *pParse, int onError, char *p4, int p4type){
  Vdbe *v = sqlite3GetVdbe(pParse);
  if( onError==OE_Abort ){
    sqlite3MayAbort(pParse);
  }
  sqlite3VdbeAddOp4(v, OP_Halt, SQLITE_CONSTRAINT, onError, 0, p4, p4type);
}

/*
** Check to see if pIndex uses the collating sequence pColl.  Return
** true if it does and false if it does not.
*/
#ifndef SQLITE_OMIT_REINDEX
static int collationMatch(const char *zColl, Index *pIndex){
  int i;
  assert( zColl!=0 );
  for(i=0; i<pIndex->nColumn; i++){
    const char *z = pIndex->azColl[i];
    assert( z!=0 );
    if( 0==sqlite3StrICmp(z, zColl) ){
      return 1;
    }
  }
  return 0;
}
#endif

/*
** Recompute all indices of pTab that use the collating sequence pColl.
** If pColl==0 then recompute all indices of pTab.
*/
#ifndef SQLITE_OMIT_REINDEX
static void reindexTable(Parse *pParse, Table *pTab, char const *zColl){
  Index *pIndex;              /* An index associated with pTab */

  for(pIndex=pTab->pIndex; pIndex; pIndex=pIndex->pNext){
    if( zColl==0 || collationMatch(zColl, pIndex) ){
      int iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3RefillIndex(pParse, pIndex, -1);
    }
  }
}
#endif

/*
** Recompute all indices of all tables in all databases where the
** indices use the collating sequence pColl.  If pColl==0 then recompute
** all indices everywhere.
*/
#ifndef SQLITE_OMIT_REINDEX
static void reindexDatabases(Parse *pParse, char const *zColl){
  Db *pDb;                    /* A single database */
  int iDb;                    /* The database index number */
  sqlite3 *db = pParse->db;   /* The database connection */
  HashElem *k;                /* For looping over tables in pDb */
  Table *pTab;                /* A table in the database */

  assert( sqlite3BtreeHoldsAllMutexes(db) );  /* Needed for schema access */
  for(iDb=0, pDb=db->aDb; iDb<db->nDb; iDb++, pDb++){
    assert( pDb!=0 );
    for(k=sqliteHashFirst(&pDb->pSchema->tblHash);  k; k=sqliteHashNext(k)){
      pTab = (Table*)sqliteHashData(k);
      reindexTable(pParse, pTab, zColl);
    }
  }
}
#endif

/*
** Generate code for the REINDEX command.
**
**        REINDEX                            -- 1
**        REINDEX  <collation>               -- 2
**        REINDEX  ?<database>.?<tablename>  -- 3
**        REINDEX  ?<database>.?<indexname>  -- 4
**
** Form 1 causes all indices in all attached databases to be rebuilt.
** Form 2 rebuilds all indices in all databases that use the named
** collating function.  Forms 3 and 4 rebuild the named index or all
** indices associated with the named table.
*/
#ifndef SQLITE_OMIT_REINDEX
SQLITE_PRIVATE void sqlite3Reindex(Parse *pParse, Token *pName1, Token *pName2){
  CollSeq *pColl;             /* Collating sequence to be reindexed, or NULL */
  char *z;                    /* Name of a table or index */
  const char *zDb;            /* Name of the database */
  Table *pTab;                /* A table in the database */
  Index *pIndex;              /* An index associated with pTab */
  int iDb;                    /* The database index number */
  sqlite3 *db = pParse->db;   /* The database connection */
  Token *pObjName;            /* Name of the table or index to be reindexed */

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  if( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    return;
  }

  if( pName1==0 ){
    reindexDatabases(pParse, 0);
    return;
  }else if( NEVER(pName2==0) || pName2->z==0 ){
    char *zColl;
    assert( pName1->z );
    zColl = sqlite3NameFromToken(pParse->db, pName1);
    if( !zColl ) return;
    pColl = sqlite3FindCollSeq(db, ENC(db), zColl, 0);
    if( pColl ){
      reindexDatabases(pParse, zColl);
      sqlite3DbFree(db, zColl);
      return;
    }
    sqlite3DbFree(db, zColl);
  }
  iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pObjName);
  if( iDb<0 ) return;
  z = sqlite3NameFromToken(db, pObjName);
  if( z==0 ) return;
  zDb = db->aDb[iDb].zName;
  pTab = sqlite3FindTable(db, z, zDb);
  if( pTab ){
    reindexTable(pParse, pTab, 0);
    sqlite3DbFree(db, z);
    return;
  }
  pIndex = sqlite3FindIndex(db, z, zDb);
  sqlite3DbFree(db, z);
  if( pIndex ){
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3RefillIndex(pParse, pIndex, -1);
    return;
  }
  sqlite3ErrorMsg(pParse, "unable to identify the object to be reindexed");
}
#endif

/*
** Return a dynamicly allocated KeyInfo structure that can be used
** with OP_OpenRead or OP_OpenWrite to access database index pIdx.
**
** If successful, a pointer to the new structure is returned. In this case
** the caller is responsible for calling sqlite3DbFree(db, ) on the returned 
** pointer. If an error occurs (out of memory or missing collation 
** sequence), NULL is returned and the state of pParse updated to reflect
** the error.
*/
SQLITE_PRIVATE KeyInfo *sqlite3IndexKeyinfo(Parse *pParse, Index *pIdx){
  int i;
  int nCol = pIdx->nColumn;
  int nBytes = sizeof(KeyInfo) + (nCol-1)*sizeof(CollSeq*) + nCol;
  sqlite3 *db = pParse->db;
  KeyInfo *pKey = (KeyInfo *)sqlite3DbMallocZero(db, nBytes);

  if( pKey ){
    pKey->db = pParse->db;
    pKey->aSortOrder = (u8 *)&(pKey->aColl[nCol]);
    assert( &pKey->aSortOrder[nCol]==&(((u8 *)pKey)[nBytes]) );
    for(i=0; i<nCol; i++){
      char *zColl = pIdx->azColl[i];
      assert( zColl );
      pKey->aColl[i] = sqlite3LocateCollSeq(pParse, zColl);
      pKey->aSortOrder[i] = pIdx->aSortOrder[i];
    }
    pKey->nField = (u16)nCol;
  }

  if( pParse->nErr ){
    sqlite3DbFree(db, pKey);
    pKey = 0;
  }
  return pKey;
}

/************** End of build.c ***********************************************/