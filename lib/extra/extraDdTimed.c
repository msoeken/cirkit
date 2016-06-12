/**CFile****************************************************************

  FileName    [extraDdTimed.c]

  PackageName [extra]

  Synopsis    [Procedures with timeout.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraDdTimed.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

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

// the signature counter
int s_Signature = 1;

// the universal hash table
typedef struct HashEntry_t
{
    int Sign;           // signature of the current cache operation
    unsigned Arg1;      // the first argument
    unsigned Arg2;      // the second argument
    unsigned Arg3;      // the second argument
} HashEntry;


// HASH TABLE data structure and variables
#define g_TABLESIZE 11111
//int g_TableSize  = 11111;
HashEntry HTable[g_TABLESIZE];
static int HashSuccess = 0;
static int HashFailure = 0;

// lists of nodes to be referenced
static DdNode ** s_pRefDDs = NULL;
static int s_nRefDDs = 0;
static int s_nRefDDsAlloc = 0;



static int s_KeysAbsoluteLimit   = 1000000000;
static int s_KeysAllowedIncrease = 0;
static int s_Timeout             = 0;

static int s_TimeLimit = 0; 
static int s_KeysLimit = 0; 

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

// static functions
static DdNode * extraBddVectorCompose_rec( DdManager * dd, DdNode * f, DdNode ** vector, int deepest );
static void Extra_RefListAdd( DdNode * bF );
static void Extra_RefListRecursiveDeref( DdManager * dd );
static DdNode * extraZddIsopCoverAltTimed( DdManager * dd, DdNode * bA, DdNode * bB);
static DdNode * extraZddIsopCoverAltLimited( DdManager * dd, DdNode * bA, DdNode * bB, int *pnBTracks);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Sets the timeout.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void Extra_OperationTimeoutSet( int timeout )
{
    s_Timeout = timeout;
}

/**Function********************************************************************

  Synopsis    [Sets the space.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void Extra_OperationSpaceoutSet( int MaxNodeIncrease )
{
    s_KeysAllowedIncrease = MaxNodeIncrease;
}


/**Function********************************************************************

  Synopsis    [Resets the timeout.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void Extra_OperationTimeoutReset()
{
    s_Timeout = 0;
}

/**Function********************************************************************

  Synopsis    [Resets the spaceout.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void Extra_OperationSpaceoutReset()
{
    s_KeysAllowedIncrease = 0;
}



/**Function********************************************************************

  Synopsis    [Computes the conjunction of two BDDs f and g.]

  Description [Computes the conjunction of two BDDs f and g. Returns a
  pointer to the resulting BDD if successful; NULL if the intermediate
  result blows up.]

  SideEffects [None]

  SeeAlso     [Cudd_bddIte Cudd_addApply Cudd_bddAndAbstract Cudd_bddIntersect
  Cudd_bddOr Cudd_bddNand Cudd_bddNor Cudd_bddXor Cudd_bddXnor]

******************************************************************************/
DdNode * Extra_bddAnd( DdManager * dd, DdNode * f, DdNode * g )
{
    DdNode *res;

    s_TimeLimit = (int)(s_Timeout /* in miliseconds*/ * (float)(CLOCKS_PER_SEC) / 1000 ) + clock();
    s_KeysLimit = ddMin( (int)dd->keys + s_KeysAllowedIncrease, s_KeysAbsoluteLimit );

    do
    {
        dd->reordered = 0;
        res = extraBddAndRecur( dd, f, g );
    }
    while ( dd->reordered == 1 );
    return ( res );

}                               /* end of Extra_bddAnd */


/**Function********************************************************************

  Synopsis    [Computes the disjunction of two BDDs f and g.]

  Description [Computes the disjunction of two BDDs f and g. Returns a
  pointer to the resulting BDD if successful; NULL if the intermediate
  result blows up.]

  SideEffects [None]

  SeeAlso     [Cudd_bddIte Cudd_addApply Cudd_bddAnd Cudd_bddNand Cudd_bddNor
  Cudd_bddXor Cudd_bddXnor]

******************************************************************************/
DdNode * Extra_bddOr( DdManager * dd, DdNode * f, DdNode * g )
{
    DdNode *res;

    s_TimeLimit = (int)(s_Timeout /* in miliseconds*/ * (float)(CLOCKS_PER_SEC) / 1000 ) + clock();
    s_KeysLimit = ddMin( (int)dd->keys + s_KeysAllowedIncrease, s_KeysAbsoluteLimit );

    do
    {
        dd->reordered = 0;
        res = extraBddAndRecur( dd, Cudd_Not( f ), Cudd_Not( g ) );
    }
    while ( dd->reordered == 1 );
    res = Cudd_NotCond( res, res != NULL );
    return ( res );

}                               /* end of Extra_bddOr */


/**Function********************************************************************

  Synopsis    [Computes the boolean difference of f with respect to x.]

  Description [Computes the boolean difference of f with respect to the
  variable with index x.  Returns the BDD of the boolean difference if
  successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddBooleanDiff( DdManager * manager, DdNode * f, int x )
{
    DdNode *res, *var;

    s_TimeLimit = (int)(s_Timeout /* in miliseconds*/ * (float)(CLOCKS_PER_SEC) / 1000 ) + clock();
    s_KeysLimit = ddMin( (int)manager->keys + s_KeysAllowedIncrease, s_KeysAbsoluteLimit );

    /* If the variable is not currently in the manager, f cannot
       ** depend on it.
     */
    if ( x >= manager->size )
        return ( Cudd_Not( DD_ONE( manager ) ) );
    var = manager->vars[x];

    do
    {
        manager->reordered = 0;
        res = extraBddBooleanDiffRecur( manager, Cudd_Regular( f ), var );
    }
    while ( manager->reordered == 1 );

    return ( res );

}                               /* end of Extra_bddBooleanDiff */


/**Function********************************************************************

  Synopsis    [The alternative implementation of irredundant sum-of-products.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddMinimal]

******************************************************************************/
DdNode* Extra_zddIsopCoverAltTimed( 
  DdManager * dd,     /* the DD manager */
  DdNode * bFuncOn,    /* the on-set */
  DdNode * bFuncOnDc)  /* the on-set + dc-set */
{
    DdNode  *res;

    s_TimeLimit = (int)(s_Timeout /* in miliseconds*/ * (float)(CLOCKS_PER_SEC) / 1000 ) + clock();
    s_KeysLimit = ddMin( (int)dd->keys + s_KeysAllowedIncrease, s_KeysAbsoluteLimit );

    do {
        dd->reordered = 0;
        res = extraZddIsopCoverAltTimed(dd, bFuncOn, bFuncOnDc);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddIsopCoverAlt */

/**Function********************************************************************

  Synopsis    [ISOP computation with the limit on the number of backtracks.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddMinimal]

******************************************************************************/
DdNode * Extra_zddIsopCoverAltLimited( 
  DdManager * dd,      /* the DD manager */
  DdNode * bFuncOn,    /* the on-set */
  DdNode * bFuncOnDc,  /* the on-set + dc-set */
  int nBTracks )       /* the number of backtracks */
{
    DdNode * res;
    int nBTracksInit = nBTracks;

    do {
        dd->reordered = 0;
        res = extraZddIsopCoverAltLimited(dd, bFuncOn, bFuncOnDc, &nBTracks);
    } while (dd->reordered == 1);

//if ( nBTracksInit-nBTracks > 500 )
//printf( "The cover = %5d.", Cudd_zddCount( dd, res ) );
//printf( "    Performed backtracks = %5d.\n", nBTracksInit-nBTracks );

    return(res);

} /* end of Extra_zddIsopCoverAlt */


/**Function********************************************************************

  Synopsis    [Computes the vector compose with the timeout.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddVectorCompose( DdManager * dd, DdNode * f, DdNode ** vector )
{
    DdNode *res;
    int deepest;
    int i;
    long clk = clock();

//  s_TimeLimit = (int)(s_Timeout /* in secs */       * (float)(CLOCKS_PER_SEC) ) + clock();
    s_TimeLimit = (int)(s_Timeout /* in miliseconds*/ * (float)(CLOCKS_PER_SEC) / 1000 ) + clock();
    s_KeysLimit = ddMin( (int)dd->keys + s_KeysAllowedIncrease, s_KeysAbsoluteLimit );

    do
    {
        dd->reordered = 0;

        // get unique signature
        s_Signature++;

        // set the counter of the referenced DDs to 0
        s_nRefDDs = 0;

        /* Find deepest real substitution. */
        for ( deepest = dd->size - 1; deepest >= 0; deepest-- )
        {
            i = dd->invperm[deepest];
            if ( vector[i] != dd->vars[i] )
                break;
        }

        /* Recursively solve the problem. */
        res = extraBddVectorCompose_rec( dd, f, vector, deepest );
        if ( res != NULL )
            cuddRef( res );

        /* Dispose of local cache. */
        // undo the DDs that were referenced for storing in the cache
        Extra_RefListRecursiveDeref(dd);
    }
    while ( dd->reordered == 1 );

//  printf( "VCtime = %.2f sec.\n", (float)(clock()-clk)/(float)(CLOCKS_PER_SEC) );

    if ( res != NULL )
        cuddDeref( res );
    return ( res );
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis [Implements the recursive step of Cudd_bddAnd.]

  Description [Implements the recursive step of Cudd_bddAnd by taking
  the conjunction of two BDDs.  Returns a pointer to the result is
  successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_bddAnd]

******************************************************************************/
DdNode * extraBddAndRecur( DdManager * manager, DdNode * f, DdNode * g )
{
    DdNode *F, *fv, *fnv, *G, *gv, *gnv;
    DdNode *one, *r, *t, *e;
    unsigned int topf, topg, index;

    // quit because of timeout or spaceout
    if ( s_Timeout && clock() > s_TimeLimit )
    {
//      printf( "t" );
        return NULL;
    }
    if ( s_KeysAllowedIncrease && (int)manager->keys > s_KeysLimit )
    {
//      printf( "s" );
        return NULL;
    }

    statLine( manager );
    one = DD_ONE( manager );

    /* Terminal cases. */
    F = Cudd_Regular( f );
    G = Cudd_Regular( g );
    if ( F == G )
    {
        if ( f == g )
            return ( f );
        else
            return ( Cudd_Not( one ) );
    }
    if ( F == one )
    {
        if ( f == one )
            return ( g );
        else
            return ( f );
    }
    if ( G == one )
    {
        if ( g == one )
            return ( f );
        else
            return ( g );
    }

    /* At this point f and g are not constant. */
    if ( f > g )
    {                           /* Try to increase cache efficiency. */
        DdNode *tmp = f;
        f = g;
        g = tmp;
        F = Cudd_Regular( f );
        G = Cudd_Regular( g );
    }

    /* Check cache. */
    if ( F->ref != 1 || G->ref != 1 )
    {
        r = cuddCacheLookup2( manager, Cudd_bddAnd, f, g );
        if ( r != NULL )
            return ( r );
    }

    /* Here we can skip the use of cuddI, because the operands are known
       ** to be non-constant.
     */
    topf = manager->perm[F->index];
    topg = manager->perm[G->index];

    /* Compute cofactors. */
    if ( topf <= topg )
    {
        index = F->index;
        fv = cuddT( F );
        fnv = cuddE( F );
        if ( Cudd_IsComplement( f ) )
        {
            fv = Cudd_Not( fv );
            fnv = Cudd_Not( fnv );
        }
    }
    else
    {
        index = G->index;
        fv = fnv = f;
    }

    if ( topg <= topf )
    {
        gv = cuddT( G );
        gnv = cuddE( G );
        if ( Cudd_IsComplement( g ) )
        {
            gv = Cudd_Not( gv );
            gnv = Cudd_Not( gnv );
        }
    }
    else
    {
        gv = gnv = g;
    }

    t = extraBddAndRecur( manager, fv, gv );
    if ( t == NULL )
        return ( NULL );
    cuddRef( t );

    e = extraBddAndRecur( manager, fnv, gnv );
    if ( e == NULL )
    {
        Cudd_IterDerefBdd( manager, t );
        return ( NULL );
    }
    cuddRef( e );

    if ( t == e )
    {
        r = t;
    }
    else
    {
        if ( Cudd_IsComplement( t ) )
        {
            r = cuddUniqueInter( manager, ( int ) index, Cudd_Not( t ),
                                 Cudd_Not( e ) );
            if ( r == NULL )
            {
                Cudd_IterDerefBdd( manager, t );
                Cudd_IterDerefBdd( manager, e );
                return ( NULL );
            }
            r = Cudd_Not( r );
        }
        else
        {
            r = cuddUniqueInter( manager, ( int ) index, t, e );
            if ( r == NULL )
            {
                Cudd_IterDerefBdd( manager, t );
                Cudd_IterDerefBdd( manager, e );
                return ( NULL );
            }
        }
    }
    cuddDeref( e );
    cuddDeref( t );
    if ( F->ref != 1 || G->ref != 1 )
        cuddCacheInsert2( manager, Cudd_bddAnd, f, g, r );
    return ( r );

}                               /* end of extraBddAndRecur */

/**Function********************************************************************

  Synopsis    [Performs the recursive steps of Cudd_bddBoleanDiff.]

  Description [Performs the recursive steps of Cudd_bddBoleanDiff.
  Returns the BDD obtained by XORing the cofactors of f with respect to
  var if successful; NULL otherwise. Exploits the fact that dF/dx =
  dF'/dx.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode * extraBddBooleanDiffRecur( DdManager * manager, DdNode * f, DdNode * var )
{
    DdNode *T, *E, *res, *res1, *res2;

    // quit because of timeout or spaceout
    if ( s_Timeout && clock() > s_TimeLimit )
    {
//      printf( "t" );
        return NULL;
    }
    if ( s_KeysAllowedIncrease && (int)manager->keys > s_KeysLimit )
    {
//      printf( "s" );
        return NULL;
    }

    statLine( manager );
    if ( cuddI( manager, f->index ) > manager->perm[var->index] )
    {
        /* f does not depend on var. */
        return ( Cudd_Not( DD_ONE( manager ) ) );
    }

    /* From now on, f is non-constant. */

    /* If the two indices are the same, so are their levels. */
    if ( f->index == var->index )
    {
        res = cuddBddXorRecur( manager, cuddT( f ), cuddE( f ) );
        return ( res );
    }

    /* From now on, cuddI(manager,f->index) < cuddI(manager,cube->index). */

    /* Check the cache. */
    res = cuddCacheLookup2( manager, extraBddBooleanDiffRecur, f, var );
    if ( res != NULL )
    {
        return ( res );
    }

    /* Compute the cofactors of f. */
    T = cuddT( f );
    E = cuddE( f );

    res1 = extraBddBooleanDiffRecur( manager, T, var );
    if ( res1 == NULL )
        return ( NULL );
    cuddRef( res1 );
    res2 = extraBddBooleanDiffRecur( manager, Cudd_Regular( E ), var );
    if ( res2 == NULL )
    {
        Cudd_IterDerefBdd( manager, res1 );
        return ( NULL );
    }
    cuddRef( res2 );
    /* ITE takes care of possible complementation of res1 and of the
       ** case in which res1 == res2. */
    res = cuddBddIteRecur( manager, manager->vars[f->index], res1, res2 );
    if ( res == NULL )
    {
        Cudd_IterDerefBdd( manager, res1 );
        Cudd_IterDerefBdd( manager, res2 );
        return ( NULL );
    }
    cuddDeref( res1 );
    cuddDeref( res2 );
    cuddCacheInsert2( manager, extraBddBooleanDiffRecur, f, var, res );
    return ( res );

}                               /* end of extraBddBooleanDiffRecur */

/**Function********************************************************************

  Synopsis    [Performs the recursive step of the alternative ISOP computation.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/
DdNode* extraZddIsopCoverAltTimed( 
  DdManager * dd,     /* the DD manager */
  DdNode * bA,        /* the on-set */
  DdNode * bB)        /* the on-set + dc-set */
{
    DdNode  *zRes;
    statLine(dd); 

    // quit because of timeout or spaceout
    if ( s_Timeout && clock() > s_TimeLimit )
    {
//      printf( "t" );
        return NULL;
    }
    if ( s_KeysAllowedIncrease && (int)dd->keys > s_KeysLimit )
    {
//      printf( "s" );
        return NULL;
    }

    /* if there is no area (the cover should be empty), there is nowhere to expand */
    if ( bA == b0 )    return z0;
    /* if the area is the total boolean space, the cover is expanded into a single large cube */
    if ( bB == b1 )    return z1;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddIsopCoverAltTimed, bA, bB);
    if (zRes)
        return zRes;
    else
    {
        DdNode *bA0, *bA1, *bB0, *bB1, *bG0, *bG1, *bHA, *bHB;
        DdNode *zRes0, *zRes1, *zRes2;

        DdNode *bAA = Cudd_Regular(bA);
        DdNode *bBB = Cudd_Regular(bB);

        int LevA = dd->perm[bAA->index];
        int LevB = dd->perm[bBB->index];

        /* the index of the positive ZDD variable in the result */
        int TopVar;

        if ( LevA <= LevB )
        {
            int fIsComp = (int)(bA != bAA);
            bA0 = Cudd_NotCond( cuddE(bAA), fIsComp );
            bA1 = Cudd_NotCond( cuddT(bAA), fIsComp );
            TopVar = bAA->index;
        }
        else
            bA0 = bA1 = bA;

        if ( LevB <= LevA )
        {
            int fIsComp = (int)(bB != bBB);
            bB0 = Cudd_NotCond( cuddE(bBB), fIsComp );
            bB1 = Cudd_NotCond( cuddT(bBB), fIsComp );
            TopVar = bBB->index;
        }
        else
            bB0 = bB1 = bB;

        /* g0 = F1(x=0) & !F12(x=1) */
        bG0 = cuddBddAndRecur( dd, bA0, Cudd_Not(bB1) );
        if ( bG0 == NULL ) return NULL;
        cuddRef( bG0 );

        /* P0 = IrrCover( g0, F12(x=0) ) */
        zRes0 = extraZddIsopCoverAltTimed( dd, bG0, bB0 );
        if ( zRes0 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            return NULL;
        }
        cuddRef( zRes0 );
        Cudd_IterDerefBdd(dd, bG0);


        /* g0 = F1(x=1) & !F12(x=0) */
        bG1 = cuddBddAndRecur( dd, bA1, Cudd_Not(bB0) );
        if ( bG1 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( bG1 );

        /* P1 = IrrCover( g1, F12(x=1) ) */
        zRes1 = extraZddIsopCoverAltTimed( dd, bG1, bB1 );
        if ( zRes1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( zRes1 );
        Cudd_IterDerefBdd(dd, bG1);

        /* h1 = F1(x=0) & !bdd(P0)  +   F1(x=1) & !bdd(P1) */
        bG0 = extraZddConvertToBddAndAdd( dd, zRes0, Cudd_Not(bA0) );
        if ( bG0 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG0 );

        bG1 = extraZddConvertToBddAndAdd( dd, zRes1, Cudd_Not(bA1) );
        if ( bG1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG1 );

        bHA = cuddBddAndRecur( dd, bG0, bG1 );
        if ( bHA == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHA );
        bHA = Cudd_Not(bHA);
        Cudd_IterDerefBdd(dd, bG0);
        Cudd_IterDerefBdd(dd, bG1);

        /* h12 = F12(x=0) & F12(x=1) */
        bHB = cuddBddAndRecur( dd, bB0, bB1 );
        if ( bHB == NULL )
        {
            Cudd_IterDerefBdd(dd, bHA);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHB );

        /* P1 = IrrCover( h1, h12 ) */
        zRes2 = extraZddIsopCoverAltTimed( dd, bHA, bHB );
        if ( zRes2 == NULL )
        {
            Cudd_IterDerefBdd(dd, bHA);
            Cudd_IterDerefBdd(dd, bHB);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( zRes2 );
        Cudd_IterDerefBdd(dd, bHA);
        Cudd_IterDerefBdd(dd, bHB);

        /* --------------- compose the result ------------------ */
        zRes = extraComposeCover( dd, zRes0, zRes1, zRes2, TopVar );
        if ( zRes == NULL ) return NULL;

        /* insert the result into cache and return */
        cuddCacheInsert2(dd, extraZddIsopCoverAltTimed, bA, bB, zRes);
        return zRes;
    }
} /* end of extraZddIsopCoverAltTimed */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of the alternative ISOP computation.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/
DdNode* extraZddIsopCoverAltLimited( 
  DdManager * dd,     /* the DD manager */
  DdNode * bA,        /* the on-set */
  DdNode * bB,        /* the on-set + dc-set */
  int * pnBTracks)
{
    DdNode  *zRes;
    statLine(dd); 

    /* if there is no area (the cover should be empty), there is nowhere to expand */
    if ( bA == b0 )    return z0;
    /* if the area is the total boolean space, the cover is expanded into a single large cube */
    if ( bB == b1 )    return z1;

    // quit because of the limit on the number of backtracks
    if ( --(*pnBTracks) < 0 )
        return NULL;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddIsopCoverAlt, bA, bB);
    if (zRes)
        return zRes;
    else
    {
        DdNode *bA0, *bA1, *bB0, *bB1, *bG0, *bG1, *bHA, *bHB;
        DdNode *zRes0, *zRes1, *zRes2;

        DdNode *bAA = Cudd_Regular(bA);
        DdNode *bBB = Cudd_Regular(bB);

        int LevA = dd->perm[bAA->index];
        int LevB = dd->perm[bBB->index];

        /* the index of the positive ZDD variable in the result */
        int TopVar;

        if ( LevA <= LevB )
        {
            int fIsComp = (int)(bA != bAA);
            bA0 = Cudd_NotCond( cuddE(bAA), fIsComp );
            bA1 = Cudd_NotCond( cuddT(bAA), fIsComp );
            TopVar = bAA->index;
        }
        else
            bA0 = bA1 = bA;

        if ( LevB <= LevA )
        {
            int fIsComp = (int)(bB != bBB);
            bB0 = Cudd_NotCond( cuddE(bBB), fIsComp );
            bB1 = Cudd_NotCond( cuddT(bBB), fIsComp );
            TopVar = bBB->index;
        }
        else
            bB0 = bB1 = bB;

        /* g0 = F1(x=0) & !F12(x=1) */
        bG0 = cuddBddAndRecur( dd, bA0, Cudd_Not(bB1) );
        if ( bG0 == NULL ) return NULL;
        cuddRef( bG0 );

        /* P0 = IrrCover( g0, F12(x=0) ) */
        zRes0 = extraZddIsopCoverAltLimited( dd, bG0, bB0, pnBTracks );
        if ( zRes0 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            return NULL;
        }
        cuddRef( zRes0 );
        Cudd_IterDerefBdd(dd, bG0);


        /* g0 = F1(x=1) & !F12(x=0) */
        bG1 = cuddBddAndRecur( dd, bA1, Cudd_Not(bB0) );
        if ( bG1 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( bG1 );

        /* P1 = IrrCover( g1, F12(x=1) ) */
        zRes1 = extraZddIsopCoverAltLimited( dd, bG1, bB1, pnBTracks );
        if ( zRes1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( zRes1 );
        Cudd_IterDerefBdd(dd, bG1);

        /* h1 = F1(x=0) & !bdd(P0)  +   F1(x=1) & !bdd(P1) */
        bG0 = extraZddConvertToBddAndAdd( dd, zRes0, Cudd_Not(bA0) );
        if ( bG0 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG0 );

        bG1 = extraZddConvertToBddAndAdd( dd, zRes1, Cudd_Not(bA1) );
        if ( bG1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG1 );

        bHA = cuddBddAndRecur( dd, bG0, bG1 );
        if ( bHA == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHA );
        bHA = Cudd_Not(bHA);
        Cudd_IterDerefBdd(dd, bG0);
        Cudd_IterDerefBdd(dd, bG1);

        /* h12 = F12(x=0) & F12(x=1) */
        bHB = cuddBddAndRecur( dd, bB0, bB1 );
        if ( bHB == NULL )
        {
            Cudd_IterDerefBdd(dd, bHA);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHB );

        /* P1 = IrrCover( h1, h12 ) */
        zRes2 = extraZddIsopCoverAltLimited( dd, bHA, bHB, pnBTracks );
        if ( zRes2 == NULL )
        {
            Cudd_IterDerefBdd(dd, bHA);
            Cudd_IterDerefBdd(dd, bHB);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( zRes2 );
        Cudd_IterDerefBdd(dd, bHA);
        Cudd_IterDerefBdd(dd, bHB);

        /* --------------- compose the result ------------------ */
        zRes = extraComposeCover( dd, zRes0, zRes1, zRes2, TopVar );
        if ( zRes == NULL ) return NULL;

        /* insert the result into cache and return */
        cuddCacheInsert2(dd, extraZddIsopCoverAlt, bA, bB, zRes);
        return zRes;
    }
} /* end of extraZddIsopCoverAltLimited */




/**Function********************************************************************

  Synopsis    [Computes the vector compose with the timeout.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddVectorCompose_rec( DdManager * dd, DdNode * f, DdNode ** vector, int deepest )
{
    DdNode * F, * T, * E;
    DdNode * res;
    unsigned HKey;
    statLine( dd );

    // quit because of timeout or spaceout
    if ( s_Timeout && clock() > s_TimeLimit )
    {
//      printf( "t" );
        return NULL;
    }
    if ( s_KeysAllowedIncrease && (int)dd->keys > s_KeysLimit )
    {
//      printf( "s" );
        return NULL;
    }


    F = Cudd_Regular( f );

    /* If we are past the deepest substitution, return f. */
    if ( cuddI( dd, F->index ) > deepest )
        return ( f );

    /* If problem already solved, look up answer and return. */
//  if ( ( res = cuddHashTableLookup1( table, F ) ) != NULL )
//      return ( Cudd_NotCond( res, F != f ) );
    HKey = hashKey2(s_Signature,F,g_TABLESIZE);
    if ( HTable[HKey].Sign == s_Signature && HTable[HKey].Arg1 == (unsigned)F )
    {
        res = (DdNode*) HTable[HKey].Arg2;  
        assert( res );
        return Cudd_NotCond( res, (int)(F!=f) );
    }


    /* Split and recur on children of this node. */
    T = extraBddVectorCompose_rec( dd, cuddT( F ), vector, deepest );
    if ( T == NULL )
        return ( NULL );
    cuddRef( T );
    E = extraBddVectorCompose_rec( dd, cuddE( F ), vector, deepest );
    if ( E == NULL )
    {
        Cudd_IterDerefBdd( dd, T );
        return ( NULL );
    }
    cuddRef( E );

    /* Call cuddBddIteRecur with the BDD that replaces the current top
       ** variable and the T and E we just created.
     */
    res = cuddBddIteRecur( dd, vector[F->index], T, E );
    if ( res == NULL )
    {
        Cudd_IterDerefBdd( dd, T );
        Cudd_IterDerefBdd( dd, E );
        return ( NULL );
    }
    cuddRef( res );
    Cudd_IterDerefBdd( dd, T );
    Cudd_IterDerefBdd( dd, E );

    /* Do not keep the result if the reference count is only 1, since
       ** it will not be visited again.
     */
//  if ( F->ref != 1 )
//  {
//      ptrint fanout = ( ptrint ) F->ref;
//      cuddSatDec( fanout );
//      if ( !cuddHashTableInsert1( table, F, res, fanout ) )
//      {
//          Cudd_IterDerefBdd( dd, res );
//          return ( NULL );
//      }
//  }

    HTable[HKey].Sign = s_Signature;
    HTable[HKey].Arg1 = (unsigned)F;
    HTable[HKey].Arg2 = (unsigned)res;  
    // add the BDD to the referenced DD list
    Extra_RefListAdd( res );

    cuddDeref( res );
    return ( Cudd_NotCond( res, F != f ) );
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////
///          implementation of the referenced BDD lists              ///
////////////////////////////////////////////////////////////////////////

void Extra_RefListAdd( DdNode * bF )
 // creates new reference to be stored in cache
{
     // add the BDD to the referenced DD list
     if ( s_nRefDDs >= s_nRefDDsAlloc )
     { // resize the list of referenced DDs
        DdNode ** s_pRefDDsNew;
        int nSizeNew, i;

        // get the new size
        if ( s_nRefDDsAlloc > 1000 )
            nSizeNew = s_nRefDDsAlloc * 2;
        else
            nSizeNew = s_nRefDDsAlloc + 1000;

        // allocate the new units
        s_pRefDDsNew = (DdNode**) malloc( nSizeNew * sizeof(DdNode*) );

        // copy the previous units if any
        for ( i = 0; i < s_nRefDDs; i++ )
            s_pRefDDsNew[i] = s_pRefDDs[i];

        // free the previous entries if they were allocated
        if ( s_pRefDDs )
            free( s_pRefDDs );
        s_pRefDDs      = s_pRefDDsNew;
        s_nRefDDsAlloc = nSizeNew;
     }
     s_pRefDDs[s_nRefDDs++] = bF;  Cudd_Ref( bF ); 
}

void Extra_RefListRecursiveDeref( DdManager * dd )
{
    // dereference the referenced DDs
    int i;
    for ( i = 0; i < s_nRefDDs; i++ )
        Cudd_IterDerefBdd( dd, s_pRefDDs[i] );
    s_nRefDDs = 0;
}


