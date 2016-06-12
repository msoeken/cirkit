/**CFile****************************************************************

  FileName    [extraDdTransfer.c]

  PackageName [extra]

  Synopsis    [Procedures to transder a DD into a different manager.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraDdTransfer.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

static s_fAdd;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static DdNode * extraTransferPermuteRecur
(DdManager * ddS, DdManager * ddD, DdNode * f, st_table * table, int * Permute );

static DdNode * extraTransferPermute
(DdManager * ddS, DdManager * ddD, DdNode * f, int * Permute);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Convert a {A,B}DD from a manager to another with variable remapping.]

  Description [Convert a {A,B}DD from a manager to another one. The orders of the
  variables in the two managers may be different. Returns a
  pointer to the {A,B}DD in the destination manager if successful; NULL
  otherwise. The i-th entry in the array Permute tells what is the index
  of the i-th variable from the old manager in the new manager.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode * Extra_TransferPermute( DdManager * ddSource, DdManager * ddDestination, DdNode * f, int * Permute )
{
    DdNode * bRes;
    // determine whether this function is a BDD or an ADD
    s_fAdd = Extra_addDetectAdd( ddSource, f );
    do
    {
        ddDestination->reordered = 0;
        bRes = extraTransferPermute( ddSource, ddDestination, f, Permute );
    }
    while ( ddDestination->reordered == 1 );
    return ( bRes );

}                               /* end of Extra_TransferPermute */


/**Function********************************************************************

  Synopsis    [Transfers the BDD from one manager into another level by level.]

  Description [Transfers the BDD from one manager into another while
  preserving the correspondence between variables level by level.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode * Extra_TransferLevelByLevel( DdManager * ddSource, DdManager * ddDestination, DdNode * f )
{
    DdNode * bRes;
    int * pPermute;
    int nMin, nMax, i;

    nMin = ddMin(ddSource->size, ddDestination->size);
    nMax = ddMax(ddSource->size, ddDestination->size);
    pPermute = ALLOC( int, nMax );
    // set up the variable permutation
    for ( i = 0; i < nMin; i++ )
        pPermute[ ddSource->invperm[i] ] = ddDestination->invperm[i];
    if ( ddSource->size > ddDestination->size )
    {
        for (      ; i < nMax; i++ )
            pPermute[ ddSource->invperm[i] ] = -1;
    }
    bRes = Extra_TransferPermute( ddSource, ddDestination, f, pPermute );
    FREE( pPermute );
    return bRes;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Convert a BDD from a manager to another one.]

  Description [Convert a BDD from a manager to another one. Returns a
  pointer to the BDD in the destination manager if successful; NULL
  otherwise.]

  SideEffects [None]

  SeeAlso     [Extra_TransferPermute]

******************************************************************************/
DdNode * extraTransferPermute( DdManager * ddS, DdManager * ddD, DdNode * f, int * Permute )
{
    DdNode *res;
    st_table *table = NULL;
    st_generator *gen = NULL;
    DdNode *key, *value;

    table = st_init_table( st_ptrcmp, st_ptrhash );
    if ( table == NULL )
        goto failure;
    res = extraTransferPermuteRecur( ddS, ddD, f, table, Permute );
    if ( res != NULL )
        cuddRef( res );

    /* Dereference all elements in the table and dispose of the table.
       ** This must be done also if res is NULL to avoid leaks in case of
       ** reordering. */
    gen = st_init_gen( table );
    if ( gen == NULL )
        goto failure;
    while ( st_gen( gen, ( char ** ) &key, ( char ** ) &value ) )
    {
        Cudd_RecursiveDeref( ddD, value );
    }
    st_free_gen( gen );
    gen = NULL;
    st_free_table( table );
    table = NULL;

    if ( res != NULL )
        cuddDeref( res );
    return ( res );

  failure:
    if ( table != NULL )
        st_free_table( table );
    if ( gen != NULL )
        st_free_gen( gen );
    return ( NULL );

}                               /* end of extraTransferPermute */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_TransferPermute.]

  Description [Performs the recursive step of Extra_TransferPermute.
  Returns a pointer to the result if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [extraTransferPermute]

******************************************************************************/
static DdNode * 
extraTransferPermuteRecur( 
  DdManager * ddS, 
  DdManager * ddD, 
  DdNode * f, 
  st_table * table, 
  int * Permute )
{
    DdNode *ft, *fe, *t, *e, *var, *res;
    DdNode *one, *zero;
    int index;
    int comple = 0;

    statLine( ddD );
    one = DD_ONE( ddD );
    comple = Cudd_IsComplement( f );

    /* Trivial cases. */
//  if ( Cudd_IsConstant( f ) )
//      return ( Cudd_NotCond( one, comple ) );


    // modifies to work with ADDs, too
    if ( f == ddS->zero )
        return ddD->zero;
    if ( f == ddS->one )
        return ddD->one;
    if ( f == Cudd_Not(ddS->one) )
        return Cudd_Not(ddD->one);
    if ( Cudd_IsConstant(f) ) // this is an ADD
        return cuddUniqueConst(ddD, cuddV(f));


    /* Make canonical to increase the utilization of the cache. */
    f = Cudd_NotCond( f, comple );
    /* Now f is a regular pointer to a non-constant node. */

    /* Check the cache. */
    if ( st_lookup( table, ( char * ) f, ( char ** ) &res ) )
        return ( Cudd_NotCond( res, comple ) );

    /* Recursive step. */
    if ( Permute )
        index = Permute[f->index];
    else
        index = f->index;

    ft = cuddT( f );
    fe = cuddE( f );

    t = extraTransferPermuteRecur( ddS, ddD, ft, table, Permute );
    if ( t == NULL )
    {
        return ( NULL );
    }
    cuddRef( t );

    e = extraTransferPermuteRecur( ddS, ddD, fe, table, Permute );
    if ( e == NULL )
    {
        Cudd_RecursiveDeref( ddD, t );
        return ( NULL );
    }
    cuddRef( e );

    zero = Cudd_Not(ddD->one);
    var = cuddUniqueInter( ddD, index, one, zero );
    if ( var == NULL )
    {
        Cudd_RecursiveDeref( ddD, t );
        Cudd_RecursiveDeref( ddD, e );
        return ( NULL );
    }

    // if it is an ADD, call a specialized function
    if ( s_fAdd == 1 )
        res = extraAddIteRecurGeneral( ddD, var, t, e );
    else // it is a BDD
        res = cuddBddIteRecur( ddD, var, t, e );

    if ( res == NULL )
    {
        Cudd_RecursiveDeref( ddD, t );
        Cudd_RecursiveDeref( ddD, e );
        return ( NULL );
    }
    cuddRef( res );
    Cudd_RecursiveDeref( ddD, t );
    Cudd_RecursiveDeref( ddD, e );

    if ( st_add_direct( table, ( char * ) f, ( char * ) res ) ==
         ST_OUT_OF_MEM )
    {
        Cudd_RecursiveDeref( ddD, res );
        return ( NULL );
    }
    return ( Cudd_NotCond( res, comple ) );

}                               /* end of extraTransferPermuteRecur */

/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/

