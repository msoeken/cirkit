/**CFile****************************************************************

  FileName    [extraDdShift.c]

  PackageName [extra]

  Synopsis    [Procedures to shift and stretch BDDs and ADDs.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraDdShift.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

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

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Shifts the BDD up/down by one variable.]

  Description []

  SideEffects []

  SeeAlso     [Extra_bddMove]

******************************************************************************/
DdNode * Extra_bddShift( 
  DdManager * dd,   /* the DD manager */
  DdNode * bF,
  int fShiftUp) 
{
    DdNode * res;
    do {
        dd->reordered = 0;
        res = extraBddShift( dd, bF, ((fShiftUp)? b1: b0) );
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_bddShift */

/**Function********************************************************************

  Synopsis    [Moves the BDD by the given number of variables up or down.]

  Description []

  SideEffects []

  SeeAlso     [Extra_bddShift]

******************************************************************************/
DdNode * Extra_bddMove( 
  DdManager * dd,   /* the DD manager */
  DdNode * bF,
  int nVars) 
{
    DdNode * res;
    DdNode * bVars;
    if ( nVars == 0 )
        return bF;
    assert( nVars <= dd->size );
    if ( nVars > 0 )
        bVars = dd->vars[nVars];
    else
        bVars = Cudd_Not(dd->vars[-nVars]);

    do {
        dd->reordered = 0;
        res = extraBddMove( dd, bF, bVars );
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_bddMove */

/**Function********************************************************************

  Synopsis    [Stretches out the support of the BDD.]

  Description [For example, if the original BDD depends on variables at levels
  {0,2,3} and nTimes = 3, the resulting BDD will be structurally the same
  but it will depend on variables {0*3 = 0, 2*3 = 6, 3*3 = 9}.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddStretch( 
  DdManager * dd,   /* the DD manager */
  DdNode * bF,
  int nTimes) 
{
    DdNode * res;
    do {
        dd->reordered = 0;
        res = extraBddStretch( dd, bF, dd->vars[nTimes] );
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_bddStretch */

/**Function********************************************************************

  Synopsis    [Swaps two variables in the BDD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddSwapVars( 
  DdManager * dd,      /* the DD manager */
  DdNode    * bFunc,   /* the function to be transformed */
  int         iVar1,   /* the first variable */
  int         iVar2)   /* the second variable */
{
    DdNode * bRes;
    DdNode * bCube;

    assert( iVar1 < dd->size );
    assert( iVar2 < dd->size );

    if ( iVar1 == iVar2 )
        return bFunc;

    // create the cube
    bCube = Cudd_bddAnd( dd, dd->vars[iVar1], dd->vars[iVar2] );   Cudd_Ref( bCube );

    do {
        dd->reordered = 0;
        bRes = extraBddSwapVars( dd, bFunc, bCube );
    } while (dd->reordered == 1);

    Cudd_Ref( bRes );
    Cudd_RecursiveDeref( dd, bCube );
    Cudd_Deref( bRes );

    return bRes;

} /* end of Extra_bddSwapVars */


/**Function********************************************************************

  Synopsis    [Shifts the ZDD up/down by one variable.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddShift( 
  DdManager * dd,   /* the DD manager */
  DdNode * zF,
  int fShiftUp) 
{
    DdNode  *res;
    do {
        dd->reordered = 0;
        res = extraZddShift( dd, zF, ((fShiftUp)? z1: z0) );
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddShift */

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddShift().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddShift( 
  DdManager * dd,    /* the DD manager */
  DdNode * bF,
  DdNode * bFlag) 
{
    DdNode * bRes;

    if ( Cudd_IsConstant(bF) )
        return bF;

    if ( bRes = cuddCacheLookup2(dd, extraBddShift, bF, bFlag) )
        return bRes;
    else
    {
        DdNode * bRes0, * bRes1;             
        DdNode * bF0, * bF1;             
        DdNode * bFR = Cudd_Regular(bF); 
        int VarNew;
        
        if ( bFlag == b1 )
            VarNew = dd->invperm[ dd->perm[bFR->index]-1 ];
        else
            VarNew = dd->invperm[ dd->perm[bFR->index]+1 ];

        // cofactor the functions
        if ( bFR != bF ) // bFunc is complemented 
        {
            bF0 = Cudd_Not( cuddE(bFR) );
            bF1 = Cudd_Not( cuddT(bFR) );
        }
        else
        {
            bF0 = cuddE(bFR);
            bF1 = cuddT(bFR);
        }

        bRes0 = extraBddShift( dd, bF0, bFlag );
        if ( bRes0 == NULL ) 
            return NULL;
        cuddRef( bRes0 );

        bRes1 = extraBddShift( dd, bF1, bFlag );
        if ( bRes1 == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            return NULL;
        }
        cuddRef( bRes1 );

        /* only bRes0 and bRes1 are referenced at this point */

        /* consider the case when bRes0 and bRes1 are the same node */
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        /* consider the case when Res1 is complemented */
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, VarNew, Cudd_Not(bRes1), Cudd_Not(bRes0));
            if ( bRes == NULL ) 
            {
                Cudd_RecursiveDeref(dd,bRes0);
                Cudd_RecursiveDeref(dd,bRes1);
                return NULL;
            }
            bRes = Cudd_Not(bRes);
        } 
        else 
        {
            bRes = cuddUniqueInter( dd, VarNew, bRes1, bRes0 );
            if ( bRes == NULL ) 
            {
                Cudd_RecursiveDeref(dd,bRes0);
                Cudd_RecursiveDeref(dd,bRes1);
                return NULL;
            }
        }
        cuddDeref( bRes0 );
        cuddDeref( bRes1 );
            
        /* insert the result into cache */
        cuddCacheInsert2(dd, extraBddShift, bF, bFlag, bRes);
        return bRes;
    }
} /* end of extraBddShift */


/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddMove().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddMove( 
  DdManager * dd,    /* the DD manager */
  DdNode * bF,
  DdNode * bDist) 
{
    DdNode * bRes;

    if ( Cudd_IsConstant(bF) )
        return bF;

    if ( bRes = cuddCacheLookup2(dd, extraBddMove, bF, bDist) )
        return bRes;
    else
    {
        DdNode * bRes0, * bRes1;             
        DdNode * bF0, * bF1;             
        DdNode * bFR = Cudd_Regular(bF); 
        int VarNew;
        
        if ( Cudd_IsComplement(bDist) )
            VarNew = bFR->index - Cudd_Not(bDist)->index;
        else
            VarNew = bFR->index + bDist->index;

        // cofactor the functions
        if ( bFR != bF ) // bFunc is complemented 
        {
            bF0 = Cudd_Not( cuddE(bFR) );
            bF1 = Cudd_Not( cuddT(bFR) );
        }
        else
        {
            bF0 = cuddE(bFR);
            bF1 = cuddT(bFR);
        }

        bRes0 = extraBddMove( dd, bF0, bDist );
        if ( bRes0 == NULL ) 
            return NULL;
        cuddRef( bRes0 );

        bRes1 = extraBddMove( dd, bF1, bDist );
        if ( bRes1 == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            return NULL;
        }
        cuddRef( bRes1 );

        /* only bRes0 and bRes1 are referenced at this point */
        bRes = cuddBddIteRecur( dd, dd->vars[VarNew], bRes1, bRes0 );
        if ( bRes == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            Cudd_RecursiveDeref( dd, bRes1 );
            return NULL;
        }
        cuddRef( bRes );
        Cudd_RecursiveDeref( dd, bRes0 );
        Cudd_RecursiveDeref( dd, bRes1 );

        /* insert the result into cache */
        cuddCacheInsert2( dd, extraBddMove, bF, bDist, bRes );
        cuddDeref( bRes );
        return bRes;
    }
} /* end of extraBddMove */


/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddStretch().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddStretch( 
  DdManager * dd,    /* the DD manager */
  DdNode * bF,
  DdNode * bTimes) 
{
    DdNode * bRes;

    if ( Cudd_IsConstant(bF) )
        return bF;

    if ( bRes = cuddCacheLookup2(dd, extraBddStretch, bF, bTimes) )
        return bRes;
    else
    {
        DdNode * bRes0, * bRes1;             
        DdNode * bF0, * bF1;             
        DdNode * bFR = Cudd_Regular(bF); 
        int VarNew;

        // get the new variable after stretching
        VarNew = dd->invperm[ dd->perm[bFR->index] * bTimes->index ];

        // cofactor the functions
        if ( bFR != bF ) // bFunc is complemented 
        {
            bF0 = Cudd_Not( cuddE(bFR) );
            bF1 = Cudd_Not( cuddT(bFR) );
        }
        else
        {
            bF0 = cuddE(bFR);
            bF1 = cuddT(bFR);
        }

        bRes0 = extraBddStretch( dd, bF0, bTimes );
        if ( bRes0 == NULL ) 
            return NULL;
        cuddRef( bRes0 );

        bRes1 = extraBddStretch( dd, bF1, bTimes );
        if ( bRes1 == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            return NULL;
        }
        cuddRef( bRes1 );

        /* only bRes0 and bRes1 are referenced at this point */

        /* consider the case when bRes0 and bRes1 are the same node */
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        /* consider the case when Res1 is complemented */
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, VarNew, Cudd_Not(bRes1), Cudd_Not(bRes0));
            if ( bRes == NULL ) 
            {
                Cudd_RecursiveDeref(dd,bRes0);
                Cudd_RecursiveDeref(dd,bRes1);
                return NULL;
            }
            bRes = Cudd_Not(bRes);
        } 
        else 
        {
            bRes = cuddUniqueInter( dd, VarNew, bRes1, bRes0 );
            if ( bRes == NULL ) 
            {
                Cudd_RecursiveDeref(dd,bRes0);
                Cudd_RecursiveDeref(dd,bRes1);
                return NULL;
            }
        }
        cuddDeref( bRes0 );
        cuddDeref( bRes1 );
            
        /* insert the result into cache */
        cuddCacheInsert2(dd, extraBddStretch, bF, bTimes, bRes);
        return bRes;
    }
} /* end of extraBddStretch */


/**Function********************************************************************

  Synopsis [Performs the recursive step of Extra_bddSwapVars.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
extraBddSwapVars(
  DdManager * dd,
  DdNode * bF,
  DdNode * bVars)
{
    DdNode * bRes;
    DdNode * bFR;
    int fComp;
    int LevelF;
    int LevelV1;
    int LevelV2;

    statLine(dd);

    // if constant, return
    bFR = Cudd_Regular(bF);
    if ( cuddIsConstant( bFR ) )
        return bF;

    LevelF  = dd->perm[bFR->index];
    LevelV1 = dd->perm[bVars->index];          // V1
    LevelV2 = dd->perm[cuddT(bVars)->index];   // V2

    // if the call is from below the level of bVars, return
    if ( LevelF > LevelV2 )
        return bF;

    fComp = (int)(bFR != bF);

    if ( bRes = cuddCacheLookup2(dd, extraBddSwapVars, bFR, bVars) )
        return Cudd_NotCond( bRes, fComp );
    else
    {
        DdNode * bRes0, * bRes1;

        // solve subproblems
        bRes0 = extraBddSwapVars( dd, cuddE(bFR), bVars );
        if ( bRes0 == NULL )
            return NULL;
        cuddRef( bRes0 );

        bRes1 = extraBddSwapVars( dd, cuddT(bFR), bVars );
        if ( bRes1 == NULL )
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            return NULL;
        }
        cuddRef( bRes1 );

        // if the call is from the level above the topmost var, it is easy
        if ( LevelF < LevelV1 )
        {
            assert( bRes0 != bRes1 );
            if ( Cudd_IsComplement(bRes1) ) 
            {
                bRes = cuddUniqueInter(dd, bFR->index, Cudd_Not(bRes1), Cudd_Not(bRes0));
                if ( bRes == NULL ) 
                {
                    Cudd_RecursiveDeref(dd,bRes0);
                    Cudd_RecursiveDeref(dd,bRes1);
                    return NULL;
                }
                bRes = Cudd_Not(bRes);
            } 
            else 
            {
                bRes = cuddUniqueInter( dd, bFR->index, bRes1, bRes0 );
                if ( bRes == NULL ) 
                {
                    Cudd_RecursiveDeref(dd,bRes0);
                    Cudd_RecursiveDeref(dd,bRes1);
                    return NULL;
                }
            }
            cuddDeref( bRes0 );
            cuddDeref( bRes1 );
        }
        else // if ( LevelF >= LevelV1 )
        {
            // get the new var after permutation
            int iVarNew;
            if ( LevelF == LevelV1 )
                iVarNew = cuddT(bVars)->index; // Var2
            else if ( LevelF == LevelV2 )
                iVarNew = bVars->index;        // Var1
            else
                iVarNew = bFR->index;

            bRes = cuddBddIteRecur( dd, dd->vars[iVarNew], bRes1, bRes0 );
            if ( bRes == NULL )
            {
                Cudd_RecursiveDeref( dd, bRes1 );
                Cudd_RecursiveDeref( dd, bRes0 );
                return ( NULL );
            }
            cuddRef( bRes );
            Cudd_RecursiveDeref( dd, bRes1 );
            Cudd_RecursiveDeref( dd, bRes0 );
            cuddDeref( bRes );
        }
            
        /* insert the result into cache */
        cuddCacheInsert2(dd, extraBddSwapVars, bFR, bVars, bRes);
        return Cudd_NotCond( bRes, fComp );
    }
} /* end of extraBddSwapVars */


/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_zddShift().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraZddShift( 
  DdManager * dd,    /* the DD manager */
  DdNode * zF,
  DdNode * zFlag) 
{
    DdNode * zRes;

    if ( cuddIsConstant(zF) )
        return zF;

    if ( zRes = cuddCacheLookup2Zdd(dd, extraZddShift, zF, zFlag) )
        return zRes;
    else
    {
        DdNode * zRes0, * zRes1;             
        int VarNew;
        
        if ( zFlag == z1 )
            VarNew = dd->invpermZ[ dd->permZ[zF->index]-1 ];
        else
            VarNew = dd->invpermZ[ dd->permZ[zF->index]+1 ];

        zRes0 = extraZddShift( dd, cuddE(zF), zFlag );
        if ( zRes0 == NULL ) 
            return NULL;
        cuddRef( zRes0 );

        zRes1 = extraZddShift( dd, cuddT(zF), zFlag );
        if ( zRes1 == NULL ) 
        {
            Cudd_RecursiveDeref( dd, zRes0 );
            return NULL;
        }
        cuddRef( zRes1 );

        /* only zRes0 and zRes1 are referenced at this point */

        /* create the new node */
        zRes = cuddZddGetNode( dd, VarNew, zRes1, zRes0 );
        if ( zRes == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            Cudd_RecursiveDerefZdd( dd, zRes1 );
            return NULL;
        }
        cuddDeref( zRes0 );
        cuddDeref( zRes1 );

        /* insert the result into cache */
        cuddCacheInsert2(dd, extraZddShift, zF, zFlag, zRes);
        return zRes;
    }
} /* end of extraZddShift */

/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/
