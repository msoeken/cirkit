/**CFile****************************************************************

  FileName    [extraZddFactor.c]

  PackageName [extra]

  Synopsis    [Factorization procedures based on ZDD representation of covers.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraZddFactor.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

// static variables used by Extra_zddCommonLiterals()
int   s_nLitsFoundInAll;   // the number of variables, which still can appear in all cubes
int   s_nCubes;            // the number of processed cubes
int   s_iZVar;             // the number of a special variable 

// static variables used by Extra_zddMoreThanOneLiteralSet()
int * s_LitCounters;
int   s_nLitsFoundLessThanTwice;
int   s_StartLevel; 


// temporary arrays (should be fixed!)
static int pArray1[2000];
static int pArray2[2000];

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void ddMoreThanOneLiteralSet( DdManager * dd, DdNode * zCover, int * pVars, int nVars );
static void ddCommonLiterals( DdManager * dd, DdNode * zCover, int * pVars, int nVars );

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Counts the number of literals in a factored form.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_bddFactoredFormLiterals( DdManager * dd, DdNode * bOnSet, DdNode * bOnDcSet )
{
    DdNode * zCover;
    int nResult;
    DdNode * aRes;
    zCover = Extra_zddIsopCover( dd, bOnSet, bOnDcSet );   Cudd_Ref( zCover );
    aRes   = Extra_zddFactoredFormLiterals( dd, zCover );  Cudd_Ref( aRes );
    nResult = (int)cuddV( aRes );
    Cudd_RecursiveDeref( dd, aRes );
    Cudd_RecursiveDerefZdd( dd, zCover );
    return nResult;
}

/**Function********************************************************************

  Synopsis    [Counts the number of literals in a factored form.]

  Description [Returns the constant ADD node with the value corresponding to 
  the number of literals in the FF. The source code follows the pseudo-code
  discusses in G.Hachtel, F.Somenzi "Logic synthesis and verification algorithms",
  pp. 432.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddFactoredFormLiterals( DdManager * dd, DdNode * zCover )
{
    DdNode * aRes;
    if ( aRes = cuddCacheLookup1(dd, Extra_zddFactoredFormLiterals, zCover) )
        return aRes;
    else
    {
        int iValue;
        DdNode * zDiv;

        zDiv = Extra_zddQuickDivisor( dd, zCover );   Cudd_Ref( zDiv );

        if ( zDiv == zCover )
        {
            iValue = Extra_zddCountLiteralsSimple( dd, zDiv );
            aRes = cuddUniqueConst( dd, iValue );
        }
        else
        {
            DdNode * zQuo, * zRem;
            Extra_zddDivision( dd, zCover, zDiv, &zQuo, &zRem );
            Cudd_Ref( zQuo );
            Cudd_Ref( zRem );
            assert( zQuo != z0 );

            if ( !Extra_zddMoreThanOneCube( dd, zQuo ) )
                aRes = Extra_zddLFLiterals( dd, zCover, zQuo );
            else
            {
                DdNode * zQuo2, * zRem2, * zTemp;
                DdNode * zQuoFree, * zCommonCube;

                zQuoFree = Extra_zddMakeCubeFree( dd, zQuo, CUDD_CONST_INDEX ); Cudd_Ref( zQuoFree );

                // divide to see whether the new quotient is cube-free
//              zQuo2 = Cudd_zddWeakDiv( dd, zCover, zQuoFree );               Cudd_Ref( zQuo2 );
                zQuo2 = Cudd_zddWeakDivF( dd, zCover, zQuoFree );               Cudd_Ref( zQuo2 );

                if ( Extra_zddTestCubeFree( dd, zQuo2 ) )
                {
                    DdNode * aF1, * aF2, * aF3;

                    // continue the division
                    zTemp = Cudd_zddProduct( dd, zQuoFree, zQuo2 );            Cudd_Ref( zTemp );
                    zRem2 = Cudd_zddDiff( dd, zCover, zTemp );                 Cudd_Ref( zRem2 );

                    aF1 = Extra_zddFactoredFormLiterals( dd, zQuoFree );  Cudd_Ref( aF1 );
                    aF2 = Extra_zddFactoredFormLiterals( dd, zQuo2 );     Cudd_Ref( aF2 );
                    aF3 = Extra_zddFactoredFormLiterals( dd, zRem2 );     Cudd_Ref( aF3 );

                    iValue = (int)(cuddV(aF1) + cuddV(aF2) + cuddV(aF3));
                    aRes = cuddUniqueConst( dd, iValue );

                    Cudd_RecursiveDerefZdd( dd, zTemp );
                    Cudd_RecursiveDerefZdd( dd, zRem2 );

                    Cudd_RecursiveDeref( dd, aF1 );
                    Cudd_RecursiveDeref( dd, aF2 );
                    Cudd_RecursiveDeref( dd, aF3 );
                }
                else
                {
                    zCommonCube = Extra_zddCommonCubeFast( dd, zQuo2 );   Cudd_Ref( zCommonCube );

                    aRes = Extra_zddLFLiterals( dd, zCover, zCommonCube );

                    Cudd_RecursiveDerefZdd( dd, zCommonCube );
                }

                Cudd_RecursiveDerefZdd( dd, zQuo2 );
                Cudd_RecursiveDerefZdd( dd, zQuoFree );
            }

            Cudd_RecursiveDerefZdd( dd, zQuo );
            Cudd_RecursiveDerefZdd( dd, zRem );
        }

        Cudd_RecursiveDerefZdd( dd, zDiv );

        // insert it into cache
        cuddCacheInsert1(dd, Extra_zddFactoredFormLiterals, zCover, aRes);
        return aRes;
    }

} /* end of Extra_zddFactoredFormLiterals */


/**Function********************************************************************

  Synopsis    [Counts the number of literals in the factored form (particular case).]

  Description [Returns the constant ADD node with the value corresponding to the number of literals in the FF]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddLFLiterals( DdManager * dd, DdNode * zCover, DdNode * zCube )
{
    DdNode * aRes;
    if ( aRes = cuddCacheLookup2(dd, Extra_zddLFLiterals, zCover, zCube) )
        return aRes;
    else
    {
        DdNode * zQuo,  * zRem;
        DdNode * aF1, * aF2;
        int nLits;
        int iValue;

        int OccurMax = 0;
        int OccurLit = -1;
        DdNode * zOccurLit;

        // find the literal from the cube that occurs most often in the cover
        nLits = Extra_zddMoreThanOneLiteralSet( dd, zCover, dd->permZ[zCube->index], pArray1, pArray2 );
        for ( ; zCube != z1; zCube = cuddT(zCube) )
            if ( OccurMax < pArray2[ zCube->index ] )
            {
                OccurMax = pArray2[ zCube->index ];
                OccurLit = zCube->index;
            }
        assert( OccurLit != -1 );
        // OccurLit = zCube->index;

        zOccurLit = cuddZddGetNode( dd, OccurLit, z1, z0 );   Cudd_Ref( zOccurLit );

        Extra_zddDivision( dd, zCover, zOccurLit, &zQuo, &zRem );
        Cudd_Ref( zQuo );
        Cudd_Ref( zRem );
        assert( zQuo != z0 );

        Cudd_RecursiveDerefZdd( dd, zOccurLit );

        aF1 = Extra_zddFactoredFormLiterals( dd, zQuo );  Cudd_Ref( aF1 );
        aF2 = Extra_zddFactoredFormLiterals( dd, zRem );  Cudd_Ref( aF2 );

        Cudd_RecursiveDerefZdd( dd, zQuo );
        Cudd_RecursiveDerefZdd( dd, zRem );

        iValue = 1 + (int)(cuddV(aF1) + cuddV(aF2));
        aRes = cuddUniqueConst( dd, iValue );

        Cudd_RecursiveDeref( dd, aF1 );
        Cudd_RecursiveDeref( dd, aF2 );

        // insert it into cache
        cuddCacheInsert2( dd, Extra_zddLFLiterals, zCover, zCube, aRes );
        return aRes;
    }

} /* end of Extra_zddLFLiterals */


/**Function********************************************************************

  Synopsis    [Finds quick-divisor of the cover.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddQuickDivisor( DdManager * dd, DdNode * zCover )
{
    if ( !Extra_zddMoreThanOneCube( dd, zCover ) )
        return zCover;
    else
        return Extra_zddLevel0Kernel( dd, zCover );
}

/**Function********************************************************************

  Synopsis    [Finds one level-0 kernel of the cover.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddLevel0Kernel( DdManager * dd, DdNode * zCover )
{
    DdNode * zRes;
    if ( zRes = cuddCacheLookup1Zdd(dd, Extra_zddLevel0Kernel, zCover) )
        return zRes;
    else
    {
        int nLits = Extra_zddMoreThanOneLiteralSet( dd, zCover, 0, pArray1, pArray2 );
        if ( nLits == 0 )
            zRes = zCover;
        else
        {
            int i, iVar;
            DdNode * zCoverNew;

            // take any literal
//          OccurLit  = dd->invpermZ[pArray1[0]];

            // find the literal that occurs most often
            int OccurMax = 0;
            int OccurLit = -1;
            for ( i = 0; i < nLits; i++ )
            {
                iVar = dd->invpermZ[pArray1[i]];
                if ( OccurMax < pArray2[iVar] )
                {
                    OccurMax = pArray2[iVar];
                    OccurLit = iVar;
                }
            }
            assert( OccurLit != -1 );
            // OccurLit = zCube->index;


            // make the cover cube free
            zCoverNew = Extra_zddMakeCubeFree( dd, zCover, OccurLit );   Cudd_Ref( zCoverNew );
            assert( Extra_zddTestCubeFree( dd, zCoverNew ) );

            // call recursively
            zRes = Extra_zddLevel0Kernel( dd, zCoverNew );           Cudd_Ref( zRes );
            Cudd_RecursiveDerefZdd( dd, zCoverNew );
            Cudd_Deref( zRes );
            assert( zRes != z0 );
        }

        // insert it into cache
        cuddCacheInsert1( dd, Extra_zddLevel0Kernel, zCover, zRes );
        return zRes;
    }

} /* end of Extra_zddLevel0Kernel */



/**Function********************************************************************

  Synopsis    [Derives both the quotient and the remainder.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Extra_zddDivision( DdManager * dd, DdNode * zCover, DdNode * zDiv, DdNode ** zQuo, DdNode ** zRem )
{
    DdNode * zTemp;

//  *zQuo = Cudd_zddWeakDiv( dd, zCover, zDiv );  Cudd_Ref( *zQuo );
    *zQuo = Cudd_zddWeakDivF( dd, zCover, zDiv );  Cudd_Ref( *zQuo );
    zTemp = Cudd_zddProduct( dd, zDiv, *zQuo  );  Cudd_Ref( zTemp );
    *zRem = Cudd_zddDiff(    dd, zCover, zTemp ); Cudd_Ref( *zRem );
    Cudd_RecursiveDerefZdd( dd, zTemp );

    Cudd_Deref( *zQuo );
    Cudd_Deref( *zRem );
}


/**Function********************************************************************

  Synopsis    [Computes the common cube as a ZDD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddCommonCubeFast( DdManager * dd, DdNode * zCover )
{
    // get the literals occurring more than once in zCover
    // the literals are will be in the first array
    int nCommLits = Extra_zddCommonLiterals( dd, zCover, CUDD_CONST_INDEX, pArray1, pArray2 );

    // get the cube
    return Extra_zddCombinationFromLevels( dd, pArray1, nCommLits );

} /* Extra_zddCommonCubeFast */


/**Function********************************************************************

  Synopsis    [Computes the ZDD cube of variables that occur more than once.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddMoreThanOnceCubeFast( DdManager * dd, DdNode * zCover )
{
    // get the literals occurring more than once in zCover
    // the literals are will be in the first array
    int nLits = Extra_zddMoreThanOneLiteralSet( dd, zCover, 0, pArray1, pArray2 );

    // get the cube
    return Extra_zddCombinationFromLevels( dd, pArray1, nLits );

} /* Extra_zddCommonCubeFast */


/**Function********************************************************************

  Synopsis    [Makes the cover cube free.]

  Description [If variable iZVar is specified, the cover is first divided by this variable.
  If variable iZVar is more or equal to CUDD_CONST_INDEX, uses the cover without changes.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddMakeCubeFree( DdManager * dd, DdNode * zCover, int iZVar )
{
    int nCommLits;
    DdNode * zCommCube, * zCoverNew;

    // get the literals that occur more than once in F/iZVar (this set contains iZVar)
    // the literals are will be in the first array
    nCommLits = Extra_zddCommonLiterals( dd, zCover, iZVar, pArray1, pArray2 );
//  assert( nCommLits > 0 );

    // get the common cube (this cube contains iZVar)
    zCommCube = Extra_zddCombinationFromLevels( dd, pArray1, nCommLits ); Cudd_Ref( zCommCube );

    // make zCover cube free
//  zCoverNew = Cudd_zddWeakDiv( dd, zCover, zCommCube ); Cudd_Ref( zCoverNew );
    zCoverNew = Cudd_zddWeakDivF( dd, zCover, zCommCube ); Cudd_Ref( zCoverNew );
    Cudd_RecursiveDerefZdd( dd, zCommCube );

    assert( Extra_zddTestCubeFree( dd, zCoverNew ) );

    Cudd_Deref( zCoverNew );
    return zCoverNew;
}


/**Function********************************************************************

  Synopsis    [Checks whether the cover is cube-free.]

  Description [Returns 1 if the cover is cube-free.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_zddTestCubeFree( DdManager * dd, DdNode * zCover )
{
    int nCommLits = Extra_zddCommonLiterals( dd, zCover, CUDD_CONST_INDEX, pArray1, pArray2 );
    return (int)( nCommLits == 0 );
}



/**Function********************************************************************

  Synopsis    [Counts the number of literals in the cover assuming that each literal occurs only once.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_zddCountLiteralsSimple( DdManager * dd, DdNode * zCover )
{
    if ( Cudd_IsConstant( zCover ) )
        return 0;
    else
    {
        // because of the condition that each literal occurs only once,
        // it is not possible that a non-terminal node has two in-coming edges

        // the positive branch has +1, compared to the literals already counted
        int Res = Extra_zddCountLiteralsSimple( dd, cuddT(zCover) ) + 1;
        // the negarive branch adds its own literals
        if ( cuddE(zCover) != z0 )
            Res += Extra_zddCountLiteralsSimple( dd, cuddE(zCover) );
        return Res;
    }
}



/**Function********************************************************************

  Synopsis    [Checks if the cover consists of more than one cube.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_zddMoreThanOneCube( DdManager * dd, DdNode * zCover )
{
    while ( zCover->index != CUDD_CONST_INDEX )
    {
        // cuddT(zCover) cannot point to z0
        if ( cuddE(zCover) != z0 )
            // there is more than one cube
            return 1;
        else // if ( cuddE(zCover) == z0 )
            // there is at least one cube, continue
            zCover = cuddT(zCover);
    }
    return 0;

} /* end of Extra_zddMoreThanOneCube */

/**Function********************************************************************

  Synopsis    [Find the ZDD cube composed of variables found on the given levels.]

  Description [Assumes that the array pLevels contains the list of all levels 
  whose vars should be included in the combination. The levels are given in the 
  increasing order. The total number of entries in pLevels is nVars. 
  Reordering should be disabled.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddCombinationFromLevels( 
  DdManager * dd, 
  int * pLevels, 
  int   nVars )
{
    DdNode * zRes, * zTemp;
    int lev, index;

    /* transform the combination from the array VarValues into a ZDD cube. */
    zRes = z1;    cuddRef(zRes);

    /*  go through levels starting bottom-up and create nodes 
     *  if these variables are present in the comb
     */
    for (lev = nVars - 1; lev >= 0; lev--) 
    { 
        /* get the variable index on this level */
        index = dd->invpermZ[ pLevels[lev] ];

        /* compose zRes with ZERO for the given ZDD variable */
        zRes = cuddZddGetNode( dd, index, zTemp = zRes, z0 );
        if ( zRes == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zTemp );
            return NULL;
        }
        cuddRef( zRes );
        cuddDeref( zTemp );
    }
    cuddDeref( zRes );
    return zRes;

} /* end of Extra_zddCombinationFromLevels */


/**Function********************************************************************

  Synopsis    [Computes variables appearing in all cubes of the ZDD cover divided by the literal.]

  Description [Returns the number of literals appearing in each of the cubes. The literals
  themselves are returned in the array pVars. The array is filled with _levels_ of the 
  literals that appear in all the cubes. The levels are in the increasing order. 
  (To determine the top-variable in the common cube, consider dd->invpermZ[pLevels[0]].)
  The array pCounters is used temporarily. Both arrays pVars and pCounters are provided 
  by the user and should have at least dd->sizeZ entries. The selected variable is included
  in the set of common literals, because, by definition, it is present in all combinations.]

  SideEffects [The worst-case complexity is linear in the number of cubes in the cover.]

  SeeAlso     []

******************************************************************************/
int Extra_zddCommonLiterals( DdManager * dd, DdNode * zCover, int iZVar, int * pVars, int * pCounters )
{
    int i;
    int nTempCounter = 0;

    if ( zCover == z0 )
        return 0;
    if ( dd->permZ[zCover->index] > cuddIZ(dd,iZVar) )
        return 0;

    // set the variable counters to 0
    for ( i = 0; i < dd->sizeZ; i++ )
        pCounters[i] = 0;

    // assign the static variables
    s_LitCounters     = pCounters;
    s_nCubes          = 0;
    s_iZVar           = iZVar;
    s_nLitsFoundInAll = 1;

    // go through the combinations and remove the variables that does not occur in some of them
    ddCommonLiterals( dd, zCover, pVars, 0 );

    // rewrite the array to contains the list of levels in the increasing order
    // on which the variable appears more than once 
    // (the variable occurs in all combinations if its counter is equal to the number of cubes)
    for ( i = 0; i < dd->sizeZ; i++ )
        if ( pCounters[ dd->invpermZ[i] ] == s_nCubes )
            pVars[ nTempCounter++ ] = i;
    assert( nTempCounter == s_nLitsFoundInAll );

    return nTempCounter;

} /* end of Extra_zddCommonLiterals */


/**Function********************************************************************

  Synopsis    [Determines all the variables that are below the given level and appear more than once in the cubes.]

  Description [Returns the number of literals, n, appearing more than once in the cubes.
  The levels of the literals themselves are returned as integers in the first n entries of the array 
  pVars. The counters of how many times each literal occurs is returned in pCounters.
  The arrays pVars and pCounters should be allocated by the user and
  have at least dd->sizeZ entries.]

  SideEffects [The worst-case complexity is linear in the number of cubes in the cover.]

  SeeAlso     []

******************************************************************************/
int Extra_zddMoreThanOneLiteralSet( DdManager * dd, DdNode * zCover, int StartLevel, int * pVars, int * pCounters )
{
    int i;
    int nSuppSizeBelow = 0;
    int nTempCounter   = 0;

    if ( zCover == z0 )
        return 0;

    // get the number of variables below the given level
    Extra_SupportArray( dd, zCover, pCounters );
    for ( i = StartLevel; i < dd->sizeZ; i++ )
        if ( pCounters[ dd->invpermZ[i] ] )
            nSuppSizeBelow++;

    // set the counters to zero
    for ( i = 0; i < dd->sizeZ; i++ )
        pCounters[i] = 0;

    // assign the static variables
    s_LitCounters             = pCounters;
    s_nLitsFoundLessThanTwice = nSuppSizeBelow;
    s_StartLevel              = StartLevel;


    // mark the variables
    ddMoreThanOneLiteralSet( dd, zCover, pVars, 0 );


    // rewrite the array to contains the list of levels in the increasing order
    // on which the variable appears more than once 
    // and at the same time verify that the number of levels is the same as 
    // the number of counters that are larger than 1
    for ( i = StartLevel; i < dd->sizeZ; i++ )
        if ( pCounters[ dd->invpermZ[i] ] > 1 )
            pVars[ nTempCounter++ ] = i;
    assert( nTempCounter == nSuppSizeBelow - s_nLitsFoundLessThanTwice );

    return nSuppSizeBelow - s_nLitsFoundLessThanTwice;

} /* end of Extra_zddMoreThanOneLiteralSet */



/**Function********************************************************************

  Synopsis    [Checks if the variable appear more than once in the cover.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_zddMoreThanOneLiteral( DdManager * dd, DdNode * zCover, int iZVar )
{
    DdNode * zVar;
    int RetValue;

    do {
        dd->reordered = 0;
        zVar = cuddZddGetNode( dd, iZVar, z1, z0 );  
    } while (dd->reordered == 1);
    cuddRef( zVar );

    RetValue = (int)( extraZddMoreThanOneLiteral( dd, zCover, zVar ) == zVar );
    Cudd_RecursiveDerefZdd( dd, zVar );

    return RetValue;

} /* end of Extra_zddMoreThanOneLiteral */



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the recursive step of checking whether the var appears more than once in the cover.]

  Description [Returns z0 if the var is not encountered, z1 if the var is encountered exactly once, 
  zVar if the var is encountered more than once.]

  SideEffects []

  SeeAlso     [Extra_zddMoreThanOneLiteral]

******************************************************************************/
DdNode * extraZddMoreThanOneLiteral( DdManager * dd, DdNode * zCover, DdNode * zVar )
{
    DdNode * zRes;
    statLine(dd); 

    // if the cover is a constant and we did not encounter the variable so far, return z0
    if ( zCover->index != CUDD_CONST_INDEX )
        return z0;

    // check cache 
    zRes = cuddCacheLookup2Zdd(dd, extraZddMoreThanOneLiteral, zCover, zVar);
    if (zRes)
        return zRes;
    else
    {
        if ( zCover->index == zVar->index )
        {   // the variable is the topmost in the cover

            if ( Extra_zddMoreThanOneCube( dd, cuddT(zCover) ) )
                // if there are more than 2 cubes in the positive branch, return success
                zRes = zVar;
            else // cuddT(zCover) has exactly one cube, because cuddT(zCover) cannot be z0
                zRes = z1;
        }
        else
        {
            DdNode * zRes0, * zRes1;

            zRes1 = extraZddMoreThanOneLiteral( dd, cuddT(zCover), zVar );
            if ( zRes1 == zVar )
                zRes = zVar;
            else
            {
                zRes0 = extraZddMoreThanOneLiteral( dd, cuddE(zCover), zVar );
                if ( zRes0 == zVar || (zRes0 == z1 && zRes1 == z1) )
                    zRes = zVar;
                else if ( zRes0 == z1 || zRes1 == z1 )
                    zRes = z1;
                else
                    zRes = z0;
            }
        }

        // insert the result into cache and return 
        cuddCacheInsert2(dd, extraZddMoreThanOneLiteral, zCover, zVar, zRes);
        return zRes;
    }
} /* end of extraZddMoreThanOneLiteral */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Determines all the variables that appear in all cubes.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void ddCommonLiterals( DdManager * dd, DdNode * zCover, int * pVars, int nVars )
{
    assert( zCover != z0 );
    if ( zCover == z1 )
    {
        int i;

        // increment the counter of cubes
        s_nCubes++;

        // increment the counters of the current set of variables
        // and determine the number of variables that can be common to all the cubes
        s_nLitsFoundInAll = 0;      
        for ( i = 0; i < nVars; i++ )
            if ( ++s_LitCounters[ pVars[i] ] == s_nCubes )
                s_nLitsFoundInAll++;
    }
    else 
    {
        DdNode * zCoverE = cuddE(zCover);
        DdNode * zCoverT = cuddT(zCover);

        int CoverLev = dd->permZ[zCover->index]; 
        int VarLev   = cuddIZ(dd,s_iZVar);

        if ( CoverLev == VarLev )
        { // call only for the positive branch

            if ( s_nLitsFoundInAll )
            {
                // add the current variable variable to the set
                pVars[nVars] = zCover->index;
                ddCommonLiterals( dd, zCoverT, pVars, nVars + 1 );
            }
        }
        else if ( CoverLev > VarLev )
        { // call both branches

            if ( s_nLitsFoundInAll )
            {
                // add the current variable variable to the set
                pVars[nVars] = zCover->index;
                ddCommonLiterals( dd, cuddT(zCover), pVars, nVars + 1 );
            }

            if ( s_nLitsFoundInAll )
            {
                // skip the negative branch, if this is the selected variable
                if ( cuddE(zCover) != z0 )
                    ddCommonLiterals( dd, cuddE(zCover), pVars, nVars );
            }
        }
        else // if ( CoverLev < VarLev )
        { // call for cofactors only if their level is above or equal to the level iZVar

            if ( s_nLitsFoundInAll )
            {
                if ( cuddIZ(dd,zCoverT->index) <= VarLev )
                {
                    // add the current variable variable to the set
                    pVars[nVars] = zCover->index;
                    ddCommonLiterals( dd, cuddT(zCover), pVars, nVars + 1 );
                }
            }

            if ( s_nLitsFoundInAll )
            {
                if ( cuddIZ(dd,zCoverE->index) <= VarLev )
                {
                    // skip the negative branch, if this is the selected variable
                    if ( cuddE(zCover) != z0 )
                        ddCommonLiterals( dd, cuddE(zCover), pVars, nVars );
                }
            }
        }
    }
}

/**Function********************************************************************

  Synopsis    [Determines all the variables that appear more than once in the cubes.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void ddMoreThanOneLiteralSet( DdManager * dd, DdNode * zCover, int * pVars, int nVars )
{
    assert( zCover != z0 );
    if ( zCover == z1 )
    {
        // increment the counters of the current set of variables
        // and if approapriate subtract from the counter of all vars encountered less than once
        int i;
        for ( i = 0; i < nVars; i++ )
            if ( dd->permZ[ pVars[i] ] >= s_StartLevel )
            {
                if ( ++s_LitCounters[ pVars[i] ] == 2 )
                    s_nLitsFoundLessThanTwice--;
            }
    }
    else 
    {
        if ( s_nLitsFoundLessThanTwice )
        {
            // add the current variable variable to the set
            pVars[nVars] = zCover->index;
            ddMoreThanOneLiteralSet( dd, cuddT(zCover), pVars, nVars + 1 );
        }

        if ( s_nLitsFoundLessThanTwice )
            if ( cuddE(zCover) != z0 )
            ddMoreThanOneLiteralSet( dd, cuddE(zCover), pVars, nVars );
    }
}




