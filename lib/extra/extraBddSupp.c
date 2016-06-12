/**CFile****************************************************************

  FileName    [extraBddSupp.c]

  PackageName [extra]

  Synopsis    [Various support manipulating procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraBddSupp.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

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

// these static variables are used to detect support size 
// at the same time as the BDD nodes are counted
// in the procedure Extra_DagSizeSuppSize()
unsigned * s_pArray = NULL;     // storage for the support
int        s_nArrayAlloc  = 0;  // the size of the allocated array
unsigned   s_UniqueCounter= 0;  // to make each call to Extra_DagSizeSuppSize unique

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int ddDagSizeSuppSizeStep (DdNode * f, int * pnSuppSize);
static void ddSupportStep (DdNode *f, int *support);
static void ddClearFlag (DdNode *f);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Returns the size of the support.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_bddSuppSize( DdManager * dd, DdNode * bSupp )
{
    int Counter = 0;
    while ( bSupp != b1 )
    {
        assert( !Cudd_IsComplement(bSupp) );
        assert( cuddE(bSupp) == b0 );

        bSupp = cuddT(bSupp);
        Counter++;
    }
    return Counter;
}

/**Function********************************************************************

  Synopsis    [Returns 1 if the support contains the given BDD variable.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_bddSuppContainVar( DdManager * dd, DdNode * bS, DdNode * bVar )   
{ 
    for( ; bS != b1; bS = cuddT(bS) )
        if ( bS->index == bVar->index )
            return 1;
    return 0;
}

/**Function********************************************************************

  Synopsis    [Returns 1 if two supports represented as BDD cubes are overlapping.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_bddSuppOverlapping( DdManager * dd, DdNode * S1, DdNode * S2 )
{
    while ( S1->index != CUDD_CONST_INDEX && S2->index != CUDD_CONST_INDEX )
    {
        // if the top vars are the same, they intersect
        if ( S1->index == S2->index )
            return 1;
        // if the top vars are different, skip the one, which is higher
        if ( dd->perm[S1->index] < dd->perm[S2->index] )
            S1 = cuddT(S1);
        else
            S2 = cuddT(S2);
    }
    return 0;
}

/**Function********************************************************************

  Synopsis    [Returns the number of different vars in two supports.]

  Description [Counts the number of variables that appear in one support and 
  does not appear in other support. If the number exceeds DiffMax, returns DiffMax.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_bddSuppDifferentVars( DdManager * dd, DdNode * S1, DdNode * S2, int DiffMax )
{
    int Result = 0;
    while ( S1->index != CUDD_CONST_INDEX && S2->index != CUDD_CONST_INDEX )
    {
        // if the top vars are the same, this var is the same
        if ( S1->index == S2->index )
        {
            S1 = cuddT(S1);
            S2 = cuddT(S2);
            continue;
        }
        // the top var is different
        Result++;

        if ( Result >= DiffMax )
            return DiffMax;

        // if the top vars are different, skip the one, which is higher
        if ( dd->perm[S1->index] < dd->perm[S2->index] )
            S1 = cuddT(S1);
        else
            S2 = cuddT(S2);
    }

    // consider the remaining variables
    if ( S1->index != CUDD_CONST_INDEX )
        Result += Extra_bddSuppSize(dd,S1);
    else if ( S2->index != CUDD_CONST_INDEX )
        Result += Extra_bddSuppSize(dd,S2);

    if ( Result >= DiffMax )
        return DiffMax;
    return Result;
}


/**Function********************************************************************

  Synopsis    [Checks the support containment.]

  Description [This function returns 1 if one support is contained in another.
  In this case, bLarge (bSmall) is assigned to point to the larger (smaller) support.
  If the supports are identical, return 0 and does not assign the supports!]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_bddSuppCheckContainment( DdManager * dd, DdNode * bL, DdNode * bH, DdNode ** bLarge, DdNode ** bSmall )
{
    DdNode * bSL = bL;
    DdNode * bSH = bH;
    int fLcontainsH = 1;
    int fHcontainsL = 1;
    int TopVar;
    
    if ( bSL == bSH )
        return 0;

    while ( bSL != b1 || bSH != b1 )
    {
        if ( bSL == b1 )
        { // Low component has no vars; High components has some vars
            fLcontainsH = 0;
            if ( fHcontainsL == 0 )
                return 0;
            else
                break;
        }

        if ( bSH == b1 )
        { // similarly
            fHcontainsL = 0;
            if ( fLcontainsH == 0 )
                return 0;
            else
                break;
        }

        // determine the topmost var of the supports by comparing their levels
        if ( dd->perm[bSL->index] < dd->perm[bSH->index] )
            TopVar = bSL->index;
        else
            TopVar = bSH->index;

        if ( TopVar == bSL->index && TopVar == bSH->index ) 
        { // they are on the same level
            // it does not tell us anything about their containment
            // skip this var
            bSL = cuddT(bSL);
            bSH = cuddT(bSH);
        }
        else if ( TopVar == bSL->index ) // and TopVar != bSH->index
        { // Low components is higher and contains more vars
            // it is not possible that High component contains Low
            fHcontainsL = 0;
            // skip this var
            bSL = cuddT(bSL);
        }
        else // if ( TopVar == bSH->index ) // and TopVar != bSL->index
        { // similarly
            fLcontainsH = 0;
            // skip this var
            bSH = cuddT(bSH);
        }

        // check the stopping condition
        if ( !fHcontainsL && !fLcontainsH )
            return 0;
    }
    // only one of them can be true at the same time
    assert( !fHcontainsL || !fLcontainsH );
    if ( fHcontainsL )
    {
        *bLarge = bH;
        *bSmall = bL;
    }
    else // fLcontainsH
    {
        *bLarge = bL;
        *bSmall = bH;
    }
    return 1;
}


/**Function********************************************************************

  Synopsis    [Finds variables on which the DD depends and returns them as am array.]

  Description [Finds the variables on which the DD depends. Returns an array 
  with entries set to 1 for those variables that belong to the support; 
  NULL otherwise. The array is allocated by the user and should have at least
  as many entries as the maximum number of variables in BDD and ZDD parts of 
  the manager.]

  SideEffects [None]

  SeeAlso     [Cudd_Support Cudd_VectorSupport Cudd_ClassifySupport]

******************************************************************************/
int * 
Extra_SupportArray(
  DdManager * dd, /* manager */
  DdNode * f,     /* DD whose support is sought */
  int * support ) /* array allocated by the user */
{
    int i, size;

    /* Initialize support array for ddSupportStep. */
    size = ddMax(dd->size, dd->sizeZ);
    for (i = 0; i < size; i++) 
        support[i] = 0;
    
    /* Compute support and clean up markers. */
    ddSupportStep(Cudd_Regular(f),support);
    ddClearFlag(Cudd_Regular(f));

    return(support);

} /* end of Extra_SupportArray */

/**Function********************************************************************

  Synopsis    [Finds the variables on which a set of DDs depends.]

  Description [Finds the variables on which a set of DDs depends.
  The set must contain either BDDs and ADDs, or ZDDs.
  Returns a BDD consisting of the product of the variables if
  successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_Support Cudd_ClassifySupport]

******************************************************************************/
int *
Extra_VectorSupportArray( 
  DdManager * dd, /* manager */ 
  DdNode ** F, /* array of DDs whose support is sought */ 
  int n, /* size of the array */  
  int * support ) /* array allocated by the user */
{
    int i, size;

    /* Allocate and initialize support array for ddSupportStep. */
    size = ddMax( dd->size, dd->sizeZ );
    for ( i = 0; i < size; i++ )
        support[i] = 0;

    /* Compute support and clean up markers. */
    for ( i = 0; i < n; i++ )
        ddSupportStep( Cudd_Regular(F[i]), support );
    for ( i = 0; i < n; i++ )
        ddClearFlag( Cudd_Regular(F[i]) );

    return support;
}


/**Function********************************************************************

  Synopsis    [This procedure computes the node count and support size at the same time]

  Description []

  SideEffects [Uses static variables defined on top of the file.]

  SeeAlso     [Cudd_DagSize Cudd_Support Cudd_SupportSize]

******************************************************************************/
int Extra_DagSizeSuppSize( DdNode * node, int * pnSuppSize )
{
    int nNodeCount;
    int nSuppSize;

    // set a new signature to distinquish this call from other calls
    s_UniqueCounter++;
    nSuppSize = 0;
    nNodeCount = ddDagSizeSuppSizeStep( Cudd_Regular(node), &nSuppSize );
    ddClearFlag( Cudd_Regular(node) );

    *pnSuppSize = nSuppSize;
    return nNodeCount;
}                               /* end of Extra_DagSizeSuppSize */


/**Function********************************************************************

  Synopsis    [Finds the support and writes it in cache.]

  Description [Finds the variables on which a DD depends.
  Returns an ADD consisting of the product of the variables in the positive polarity if
  successful; NULL otherwise.]

  SideEffects [Should not be used with ZDD.]

  SeeAlso     [Cudd_VectorSupport Cudd_Support]

******************************************************************************/
DdNode * Extra_SupportCache( DdManager * dd, DdNode * f )
{
    DdNode * bRes;
    if ( bRes = cuddCacheLookup1(dd, Extra_SupportCache, f) )
        return bRes;
    bRes = Cudd_Support( dd, f );
    cuddCacheInsert1(dd, Extra_SupportCache, f, bRes);
    return bRes;
}

/**Function********************************************************************

  Synopsis    [Finds variables on which the DD depends and returns them as a ZDD.]

  Description [Finds the variables on which the DD depends.
  Returns a ZDD consisting of the combination of the variables if
  successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_Support Cudd_VectorSupport Cudd_ClassifySupport]

******************************************************************************/
DdNode *
Extra_zddSupport(
  DdManager * dd /* manager */,
  DdNode * f     /* DD whose support is sought */)
{
    int *support;
    DdNode *res;
    int i, size;

    /* Allocate and initialize support array for ddSupportStep. */
    size = ddMax(dd->size, dd->sizeZ);
    support = ALLOC(int,size);
    if (support == NULL) {
        dd->errorCode = CUDD_MEMORY_OUT;
        return(NULL);
    }
    for (i = 0; i < size; i++) 
        support[i] = 0;
    

    /* Compute support and clean up markers. */
    ddSupportStep(Cudd_Regular(f),support);
    ddClearFlag(Cudd_Regular(f));

    /* Transform support from array to cube. */
    res = Extra_zddCombination( dd, support, size );

    FREE(support);
    return(res);

} /* end of Extra_zddSupport */


  /**Function********************************************************************

  Synopsis    [Finds the support as a negative polarity cube.]

  Description [Finds the variables on which a DD depends. Returns a BDD 
  consisting of the product of the variables in the negative polarity 
  if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_VectorSupport Cudd_Support]

******************************************************************************/
DdNode * Extra_bddSupportNegativeCube( DdManager * dd, DdNode * f )
{
    int *support;
    DdNode *res, *tmp, *var;
    int i, j;
    int size;

    /* Allocate and initialize support array for ddSupportStep. */
    size = ddMax( dd->size, dd->sizeZ );
    support = ALLOC( int, size );
    if ( support == NULL )
    {
        dd->errorCode = CUDD_MEMORY_OUT;
        return ( NULL );
    }
    for ( i = 0; i < size; i++ )
    {
        support[i] = 0;
    }

    /* Compute support and clean up markers. */
    ddSupportStep( Cudd_Regular( f ), support );
    ddClearFlag( Cudd_Regular( f ) );

    /* Transform support from array to cube. */
    do
    {
        dd->reordered = 0;
        res = DD_ONE( dd );
        cuddRef( res );
        for ( j = size - 1; j >= 0; j-- )
        {                       /* for each level bottom-up */
            i = ( j >= dd->size ) ? j : dd->invperm[j];
            if ( support[i] == 1 )
            {
                var = cuddUniqueInter( dd, i, dd->one, Cudd_Not( dd->one ) );
                //////////////////////////////////////////////////////////////////
                var = Cudd_Not(var);
                //////////////////////////////////////////////////////////////////
                cuddRef( var );
                tmp = cuddBddAndRecur( dd, res, var );
                if ( tmp == NULL )
                {
                    Cudd_RecursiveDeref( dd, res );
                    Cudd_RecursiveDeref( dd, var );
                    res = NULL;
                    break;
                }
                cuddRef( tmp );
                Cudd_RecursiveDeref( dd, res );
                Cudd_RecursiveDeref( dd, var );
                res = tmp;
            }
        }
    }
    while ( dd->reordered == 1 );

    FREE( support );
    if ( res != NULL )
        cuddDeref( res );
    return ( res );

}                               /* end of Extra_SupportNeg */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_zddSupport.]

  Description [Performs the recursive step of Extra_Support. Performs a
  DFS from f. The support is accumulated in supp as a side effect. Uses
  the LSB of the then pointer as visited flag.]

  SideEffects [None]

  SeeAlso     [ddClearFlag]

******************************************************************************/
static int ddDagSizeSuppSizeStep( DdNode * n, int * pnSuppSize )
{
    int tval, eval;

    // check if this node has already been visited
    if ( Cudd_IsComplement( n->next ) )
        return 0;

    // mark this node as visited
    n->next = Cudd_Not( n->next );

    // check if this node is a constant (no support changes in this case)
    if ( cuddIsConstant( n ) )
        return 1;

    tval = ddDagSizeSuppSizeStep( cuddT(n), pnSuppSize );
    eval = ddDagSizeSuppSizeStep( Cudd_Regular( cuddE(n) ), pnSuppSize );

    // add to the support storage if necessary
    if ( s_nArrayAlloc <= n->index )
    { // reallocate the array
        s_pArray      = REALLOC(unsigned, s_pArray, sizeof(unsigned) * (2 * n->index + 1000) );
        // clean the new part
        memset( s_pArray + s_nArrayAlloc, 0, sizeof(unsigned) * (2 * n->index + 1000 - s_nArrayAlloc) );
        // update the pointer to the new block
        s_nArrayAlloc = 2 * n->index + 1000;
    }
    // check if the given variable has been visited
    if ( s_pArray[n->index] != s_UniqueCounter )
    {
        // the variable is new - mark it as visited
        s_pArray[n->index] = s_UniqueCounter;
        // update the support counter
        (*pnSuppSize)++;
    }
    return ( 1 + tval + eval );
}                               /* end of ddDagSizeSuppSizeStep */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_zddSupport.]

  Description [Performs the recursive step of Extra_Support. Performs a
  DFS from f. The support is accumulated in supp as a side effect. Uses
  the LSB of the then pointer as visited flag.]

  SideEffects [None]

  SeeAlso     [ddClearFlag]

******************************************************************************/
static void
ddSupportStep(
  DdNode * f,
  int * support)
{
    if (cuddIsConstant(f) || Cudd_IsComplement(f->next)) {
    return;
    }

    support[f->index] = 1;
    ddSupportStep(cuddT(f),support);
    ddSupportStep(Cudd_Regular(cuddE(f)),support);
    /* Mark as visited. */
    f->next = Cudd_Not(f->next);
    return;

} /* end of ddSupportStep */


/**Function********************************************************************

  Synopsis    [Performs a DFS from f, clearing the LSB of the next
  pointers.]

  Description []

  SideEffects [None]

  SeeAlso     [ddSupportStep ddDagInt]

******************************************************************************/
static void
ddClearFlag(
  DdNode * f)
{
    if (!Cudd_IsComplement(f->next)) {
    return;
    }
    /* Clear visited flag. */
    f->next = Cudd_Regular(f->next);
    if (cuddIsConstant(f)) {
    return;
    }
    ddClearFlag(cuddT(f));
    ddClearFlag(Cudd_Regular(cuddE(f)));
    return;

} /* end of ddClearFlag */

