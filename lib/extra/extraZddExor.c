/**CFile****************************************************************

  FileName    [extraZddExor.c]

  PackageName [extra]

  Synopsis    [ZDD based manipulation of ESOP covers.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraZddExor.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

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

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static DdNode * extraZddFastEsopCover( DdManager * dd, DdNode * bF, st_table * Table );

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Computes the Exclusive-OR-type union of two cube sets.]

  Description [Given the two sets of cubes, computes the union of these two sets
  minus those cubes that occur in both sets at the same time. ]

  SideEffects []

  SeeAlso     []

******************************************************************************/

DdNode  *
Extra_zddUnionExor(
  DdManager * dd,
  DdNode * S,
  DdNode * T)
{
    DdNode * zInter, * zUnion, * zRes;

    zInter = Cudd_zddIntersect( dd, S, T );     Cudd_Ref( zInter );
    zUnion = Cudd_zddUnion( dd, S, T );         Cudd_Ref( zUnion );
    zRes = Cudd_zddDiff( dd, zUnion, zInter );  Cudd_Ref( zRes );
    Cudd_RecursiveDerefZdd(dd, zUnion);
    Cudd_RecursiveDerefZdd(dd, zInter);

    Cudd_Deref( zRes );
    return(zRes);

} /* end of Extra_zddUnionExor */


/**Function********************************************************************

  Synopsis    [Given two sets of cubes, computes the set of pair-wise supercubes.]

  Description [Computes the set of pair-wise supercubes of each cube in zA 
  with each cube in zB. If sets zA and zB are the same set zC, the supercubes 
  will also contain all cubes. In this case, if zC is a set of primes, all 
  supercubes with overlap with OFF-set except those that are cubes themselves...]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode* Extra_zddSupercubes( 
  DdManager * dd,     /* the DD manager */
  DdNode * zA,        /* the first cover */
  DdNode * zB)        /* the second cover */
{
    DdNode  *res;
    do {
        dd->reordered = 0;
        res = extraZddSupercubes(dd, zA, zB);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddSupercubes */


/**Function********************************************************************

  Synopsis    [Selects cubes from zA that have a distance-1 cube in zB.]

  Description [Selects all the cubes in zA such that for them there is 
  at least one cube in B that is distance-1 removed from this cube.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode* Extra_zddSelectDist1Cubes( 
  DdManager * dd,     /* the DD manager */
  DdNode * zA,        /* the first cover */
  DdNode * zB)        /* the second cover */
{
    DdNode  *res;
    do {
        dd->reordered = 0;
        res = extraZddSelectDist1Cubes(dd, zA, zB);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddSelectDist1Cubes */


/**Function********************************************************************

  Synopsis    [Computes the set of fast ESOP covers for the multi-output function.]

  Description [Returns the total number of cubes in all the covers. Returns
  the referenced covers in the array zCs. The array zCs should be allocated by 
  the user. The ZDD variables should be allocated two per each BDD variable.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_zddFastEsopCoverArray( DdManager * dd, DdNode ** bFs, DdNode ** zCs, int nFs )
{
    st_table * Table;
    int nCubesTotal;
    int nCubesCur;
    int i;

    // start the general hash-table mapping non-complemented nodes 
    // into the number of cubes in the fast ESOP covers
    Table = st_init_table(st_ptrcmp,st_ptrhash);

    // perform the computation for each output
    nCubesTotal = 0;
    for ( i = 0; i < nFs; i++ )
    {
        zCs[i] = Extra_zddFastEsopCover( dd, bFs[i], Table, &nCubesCur ); Cudd_Ref( zCs[i] );
        nCubesTotal += nCubesCur;
    }

    // free the table
    st_free_table( Table );

    return nCubesTotal;
}


/**Function********************************************************************

  Synopsis    [Computes the fast ESOP cover for the function.]

  Description [Implements the fast computation of the pseudo-Kronecker cover,
  which is known to be the minimum pseudo-Kronecker ESOP for symmetric functoin. 
  The approach has been presented in R. Drechsler, "Pseudo-Kronecker Expressions for 
  Symmetric Functions". IEEE Trans. Comp. Vol. 48. No. 8, Sept. 1999, pp. 987-990.
  The hash table (Visited) may be given. In this case, it is assumed that the table
  contains the mapping of non-complemented BDD nodes into the number of cubes in 
  their fast EXOR covers. The last variable (pnCubes) if given contains the number 
  of cubes in the resulting cover. Two ZDD variables per each functional variable 
  should be allocated by the user, for this function to work correctly.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddFastEsopCover( 
  DdManager * dd,     /* the DD manager */
  DdNode * bF,        /* the function */
  st_table * Visited, /* the table to map the node into its number of cubes */
  int * pnCubes)      /* the returned value: the number of cubes in the cover */
{
    DdNode * zRes, * zTemp;
    DdNode * bFR;
    DdNode * aF;
    st_table * Table;
    int nCubes;

    // solve the trivial cases
    if ( bF == b0 )
    {
        if ( pnCubes )
            *pnCubes = 0;
        return z0;
    }
    if ( bF == b1 )
    {
        if ( pnCubes )
            *pnCubes = 1;
        return z1;
    }

    // get the regular pointer
    bFR = Cudd_Regular(bF);

    aF = Cudd_BddToAdd( dd, bF );  Cudd_Ref( aF );

    // solve the problem recursively
    do 
    {
        // start the table, if not given
        if ( Visited == NULL )
            Table = st_init_table(st_ptrcmp,st_ptrhash);
        else
            Table = Visited;

        dd->reordered = 0;
//      zRes = extraZddFastEsopCover(dd, bFR, Table);
        zRes = extraZddFastEsopCover(dd, aF, Table);

        // get the number of cubes from the table
        if ( zRes != NULL )
        {
            nCubes = -1;
//          st_lookup( Table, (char*)bFR, (char**)&nCubes );
            st_lookup( Table, (char*)aF, (char**)&nCubes );
            assert( nCubes != -1 );
        }

        // delete the table, if it was allocated locally
        if ( Visited == NULL )
            st_free_table( Table );
    } 
    while (dd->reordered == 1);

    Cudd_Ref( zRes );
    Cudd_RecursiveDeref( dd, aF );
    Cudd_Deref( zRes );

    // add the tautology cube if the function was complemented
//  if ( bFR != bF ) 
    if ( 0 ) 
    { 
        Cudd_Ref( zRes );

        if ( Extra_zddEmptyBelongs( dd, zRes ) )
        {
            zRes = Cudd_zddDiff( dd, zTemp = zRes, z1 );  
            nCubes--;
        }
        else 
        {
            zRes = Cudd_zddUnion( dd, zTemp = zRes, z1 );  
            nCubes++;
        }
        Cudd_Ref( zRes );

        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_Deref( zRes );
    }

/*
    // verification: make sure that the cover indeed represents the function
#ifndef NDEBUG
    {
        DdNode * bFunc;
        int fVer;

        Cudd_Ref( zRes );
        bFunc = Extra_zddConvertEsopToBdd( dd, zRes );  Cudd_Ref( bFunc );
        fVer = (int)( bFunc == bF );
        Cudd_RecursiveDeref( dd, bFunc );
        Cudd_Deref( zRes );
        if ( !fVer )
            printf( "\nExtra_zddFastEsopCover(): Internal cover verification failed!!!\n\n" );
    }
#endif
*/

    if ( pnCubes )
        *pnCubes = nCubes;

    return zRes;

} /* end of Extra_zddFastEsopCover */


/*---------------------------------------------------------------------------*/
/* Definition of Internal Functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_zddSupercubes.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

DdNode* extraZddSupercubes( 
  DdManager * dd,     /* the DD manager */
  DdNode * zA,        /* the first cover */
  DdNode * zB)        /* the second cover */
{
    DdNode * zRes;
    int topLevA, topLevB;
;
    // terminal cases
    // if at least one cover is empty, the result is empty, because there no pairs
    if ( zA == z0 || zB == z0 )     return z0;
    // if they are not zero, if any of the covers is full, then the result is full
    // because the supercube of any cube with the full cube, is a full cube
    if ( zA == z1 || zB == z1 )     return z1;

    /* the operation is commutative - normalize the problem */
    topLevA = dd->permZ[zA->index];
    topLevB = dd->permZ[zB->index];

    if ( topLevA > topLevB )
        return extraZddSupercubes(dd, zB, zA);

    // check cache
    if ( zRes = cuddCacheLookup2Zdd( dd, extraZddSupercubes, zA, zB ) )
        return zRes;
    else
    {
        DdNode * zA0, * zA1, * zA2; 
        DdNode * zB0, * zB1, * zB2; 
        DdNode * zC0, * zC1, * zC2; 
        DdNode * zR0, * zR1, * zR2, * zTemp; 

        // get the top most zdd variable 
        int topVarZdd = (zA->index >> 1) << 1;
        // switch from the ZDD-level variables to BDD-level variables
        topLevA /= 2;
        topLevB /= 2;

        if ( topLevA < topLevB )
        {
            extraDecomposeCover( dd, zA, &zA0, &zA1, &zA2 );
            zB0 = z0;
            zB1 = z0;
            zB2 = zB;
        }
        else // if ( topLevA == topLevB )
        {
            extraDecomposeCover( dd, zA, &zA0, &zA1, &zA2 );
            extraDecomposeCover( dd, zB, &zB0, &zB1, &zB2 );
        }

        // this is the main idea of the algorithm
        // Sup( zA0, zB0 )  -> zR0  x
        // Sup( zA0, zB1 )  -> zR2     y
        // Sup( zA0, zB2 )  -> zR2     y

        // Sup( zA1, zB0 )  -> zR2  
        // Sup( zA1, zB1 )  -> zR1  x
        // Sup( zA1, zB2 )  -> zR2

        // Sup( zA2, zB0 )  -> zR2
        // Sup( zA2, zB1 )  -> zR2     y
        // Sup( zA2, zB2 )  -> zR2     y

        // in other words
        // Sup( zA0, zB0 )  -> zR0
        // Sup( zA1, zB1 )  -> zR1
        // Sup( zA0, zB1 + zB2 )  -> zR2           zC0 = zB1 + zB2
        // Sup( zA1, zB0 + zB2 )  -> zR2           zC1 = zB0 + zB2
        // Sup( zA2, zB0 + zB1 + zB2 )  -> zR2     zC2 = zB0 + zC0

        zC0 = cuddZddUnion( dd, zB1, zB2 );
        if ( zC0 == NULL )
            return NULL;
        cuddRef( zC0 );

        zC1 = cuddZddUnion( dd, zB0, zB2 );
        if ( zC1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zC0 );
            return NULL;
        }
        cuddRef( zC1 );

        zC2 = cuddZddUnion( dd, zB0, zC0 );
        if ( zC2 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zC0 );
            Cudd_RecursiveDerefZdd( dd, zC1 );
            return NULL;
        }
        cuddRef( zC2 );
        // at this point zC0, zC1, zC2 are computed


        // compute suppercubes fro zC0, zC1, and zC2 and reuse the variables
        zC0 = extraZddSupercubes( dd, zTemp = zC0, zA0 );
        if ( zC0 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zC1 );
            Cudd_RecursiveDerefZdd( dd, zC2 );
            return NULL;
        }
        cuddRef( zC0 );
        Cudd_RecursiveDerefZdd( dd, zTemp );

        zC1 = extraZddSupercubes( dd, zTemp = zC1, zA1 );
        if ( zC1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zC0 );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zC2 );
            return NULL;
        }
        cuddRef( zC1 );
        Cudd_RecursiveDerefZdd( dd, zTemp );

        zC2 = extraZddSupercubes( dd, zTemp = zC2, zA2 );
        if ( zC2 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zC0 );
            Cudd_RecursiveDerefZdd( dd, zC1 );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            return NULL;
        }
        cuddRef( zC2 );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        // at this point zC0, zC1, zC2 are computed and represent 3 parts of zR2


        zR2 = cuddZddUnion( dd, zC0, zC1 );
        if ( zR2 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zC0 );
            Cudd_RecursiveDerefZdd( dd, zC1 );
            Cudd_RecursiveDerefZdd( dd, zC2 );
            return NULL;
        }
        cuddRef( zR2 );
        Cudd_RecursiveDerefZdd( dd, zC0 );
        Cudd_RecursiveDerefZdd( dd, zC1 );


        zR2 = cuddZddUnion( dd, zTemp = zR2, zC2 );
        if ( zR2 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zC2 );
            return NULL;
        }
        cuddRef( zR2 );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zC2 );
        // at this point zR2 is computed

        // solve other subproblems
        
        zR0 = extraZddSupercubes( dd, zA0, zB0 );
        if ( zR0 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR2 );
            return NULL;
        }
        cuddRef( zR0 );
        
        zR1 = extraZddSupercubes( dd, zA1, zB1 );
        if ( zR1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            return NULL;
        }
        cuddRef( zR1 );


        // at this point, only zR0, zR1, zR2 are referenced

        // lookup the unique table of the Zdd component of the manager
        // and return (or create) a node that has given 1 and 0 cofactors
        zTemp = cuddZddGetNode( dd, topVarZdd+1, zR0, zR2 );
        if ( zTemp == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            return NULL;
        }
        cuddRef( zTemp );
        cuddDeref( zR0 );
        cuddDeref( zR2 );

        zRes = cuddZddGetNode( dd, topVarZdd, zR1, zTemp );
        if ( zRes == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            return NULL;
        }
        cuddDeref( zR1 );
        cuddDeref( zTemp );

        // insert the result into cache
        cuddCacheInsert2( dd, extraZddSupercubes, zA, zB, zRes );
        return zRes;
    }       
} /* extraZddSupercubes */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_zddSelectDist1Cubes.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

DdNode* extraZddSelectDist1Cubes( 
  DdManager * dd,     /* the DD manager */
  DdNode * zA,        /* the first cover */
  DdNode * zB)        /* the second cover */
{
    DdNode * zRes;

    // terminal cases
    if ( zA == z0 )             return z0;
    if ( zB == z0 )             return z0;
    if ( zA == z1 && zB == z1 ) return z0;

    // check cache
    if ( zRes = cuddCacheLookup2Zdd( dd, extraZddSelectDist1Cubes, zA, zB ) )
        return zRes;
    else
    {
        DdNode * zA0, * zA1, * zA2; 
        DdNode * zB0, * zB1, * zB2; 
        DdNode * zR0, * zR1, * zR2, * zR3, * zR4, * zTemp; 

        // get the levels of the arguments
        int topLevA = dd->permZ[zA->index]/2;
        int topLevB = dd->permZ[zB->index]/2;
        int topVar;

        if ( topLevA > topLevB )
        {
            topVar = zB->index/2;
            zA0 = z0;
            zA1 = z0;
            zA2 = zA;
            extraDecomposeCover( dd, zB, &zB0, &zB1, &zB2 );
        }
        else if ( topLevA < topLevB )
        {
            topVar = zA->index/2;
            zB0 = z0;
            zB1 = z0;
            zB2 = zB;
            extraDecomposeCover( dd, zA, &zA0, &zA1, &zA2 );
        }
        else // if ( topLevA == topLevB )
        {
            topVar = zA->index/2;
            extraDecomposeCover( dd, zA, &zA0, &zA1, &zA2 );
            extraDecomposeCover( dd, zB, &zB0, &zB1, &zB2 );
        }

        // solve subroblems (find distance-1 cubes with the different variable other than the current one)
        zR0 = extraZddSelectDist1Cubes( dd, zA0, zB0 );
        if ( zR0 == NULL )
            return NULL;
        cuddRef( zR0 );

        zR1 = extraZddSelectDist1Cubes( dd, zA1, zB1 );
        if ( zR1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            return NULL;
        }
        cuddRef( zR1 );

        zR2 = extraZddSelectDist1Cubes( dd, zA2, zB2 );
        if ( zR2 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            return NULL;
        }
        cuddRef( zR2 );

        ////////////////////////////////////////////////////////////////////////
        // find distance-1 cubes the current variable being the different one
        // R3 = Intersect( A0 , B1 )
        zR3 = cuddZddIntersect( dd, zA0, zB1 );
        if ( zR3 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            return NULL;
        }
        cuddRef( zR3 );

        // R4 = Intersect( A1 , B0 )
        zR4 = cuddZddIntersect( dd, zA1, zB0 );
        if ( zR4 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            Cudd_RecursiveDerefZdd( dd, zR3 );
            return NULL;
        }
        cuddRef( zR4 );

        // zR0 = zR0 + zR3
        zR0 = cuddZddUnion( dd, zTemp = zR0, zR3 );
        if ( zR0 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            Cudd_RecursiveDerefZdd( dd, zR3 );
            Cudd_RecursiveDerefZdd( dd, zR4 );
            return NULL;
        }
        cuddRef( zR0 );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zR3 );

        // zR1 = zR1 + zR4
        zR1 = cuddZddUnion( dd, zTemp = zR1, zR4 );
        if ( zR1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            Cudd_RecursiveDerefZdd( dd, zR4 );
            return NULL;
        }
        cuddRef( zR1 );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zR4 );


        ////////////////////////////////////////////////////////////////////////
        // find distance-1 cubes the current variable being the different one
        // R3 = Intersect( A0 , B2 )
        zR3 = cuddZddIntersect( dd, zA0, zB2 );
        if ( zR3 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            return NULL;
        }
        cuddRef( zR3 );

        // R4 = Intersect( A2 , B0 )
        zR4 = cuddZddIntersect( dd, zA2, zB0 );
        if ( zR4 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            Cudd_RecursiveDerefZdd( dd, zR3 );
            return NULL;
        }
        cuddRef( zR4 );

        // zR0 = zR0 + zR3
        zR0 = cuddZddUnion( dd, zTemp = zR0, zR3 );
        if ( zR0 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            Cudd_RecursiveDerefZdd( dd, zR3 );
            Cudd_RecursiveDerefZdd( dd, zR4 );
            return NULL;
        }
        cuddRef( zR0 );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zR3 );

        // zR2 = zR2 + zR4
        zR2 = cuddZddUnion( dd, zTemp = zR2, zR4 );
        if ( zR2 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zR4 );
            return NULL;
        }
        cuddRef( zR2 );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zR4 );

        
        ////////////////////////////////////////////////////////////////////////
        // find distance-1 cubes the current variable being the different one
        // R3 = Intersect( A1 , B2 )
        zR3 = cuddZddIntersect( dd, zA1, zB2 );
        if ( zR3 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            return NULL;
        }
        cuddRef( zR3 );

        // R4 = Intersect( A2 , B1 )
        zR4 = cuddZddIntersect( dd, zA2, zB1 );
        if ( zR4 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            Cudd_RecursiveDerefZdd( dd, zR3 );
            return NULL;
        }
        cuddRef( zR4 );

        // zR1 = zR1 + zR3
        zR1 = cuddZddUnion( dd, zTemp = zR1, zR3 );
        if ( zR1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            Cudd_RecursiveDerefZdd( dd, zR3 );
            Cudd_RecursiveDerefZdd( dd, zR4 );
            return NULL;
        }
        cuddRef( zR1 );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zR3 );

        // zR2 = zR2 + zR4
        zR2 = cuddZddUnion( dd, zTemp = zR2, zR4 );
        if ( zR2 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zR4 );
            return NULL;
        }
        cuddRef( zR2 );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zR4 );


        // at this point, only zR0, zR1, zR2 are referenced

        // lookup the unique table of the Zdd component of the manager
        // and return (or create) a node that has given 1 and 0 cofactors
        zTemp = cuddZddGetNode( dd, 2*topVar+1, zR0, zR2 );
        if ( zTemp == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zR0 );
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zR2 );
            return NULL;
        }
        cuddRef( zTemp );
        cuddDeref( zR0 );
        cuddDeref( zR2 );

        zRes = cuddZddGetNode( dd, 2*topVar, zR1, zTemp );
        if ( zRes == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zR1 );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            return NULL;
        }
        cuddDeref( zR1 );
        cuddDeref( zTemp );

        // insert the result into cache
        cuddCacheInsert2( dd, extraZddSelectDist1Cubes, zA, zB, zRes );

        return zRes;
    }       
} /* extraZddSelectDist1Cubes */

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_zddFastEsopCover.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraZddFastEsopCover(
  DdManager * dd,    /* the DD manager */
  DdNode * bF,       /* the non-complemented BDD */
  st_table * Table)  /* the pointer where cardinality goes */ 
{
    DdNode * zRes;
    DdNode * (*cacheOp)(DdManager*,DdNode*);
    statLine(dd); 

    assert( !Cudd_IsComplement(bF) );
    assert( bF != b1 );

    // check cache 
    cacheOp = (DdNode*(*)(DdManager*,DdNode*))extraZddFastEsopCover;
    if ( zRes = cuddCacheLookup1Zdd( dd, cacheOp, bF ) )
        return zRes;
    else
    {
        DdNode * bFcR;
        DdNode * bFc[3];
        DdNode * zR[3];
        DdNode * zTemp;
        int Cost[3];
        int Worst;
        int CostRes;
        int i, k;

        // get the three Kronecker cofactors
        bFc[0] = cuddE(bF);
        bFc[1] = cuddT(bF);
//      bFc[2] = cuddBddXorRecur( dd, bFc[0], bFc[1] );
        bFc[2] = Cudd_addApply( dd, Cudd_addXor, bFc[0], bFc[1] );
        if ( bFc[2] == NULL )
            return NULL;
        cuddRef( bFc[2] );

        // solve the problem recursively 
        for ( i = 0; i < 3; i++ )
        {
//          if ( bFc[i] == b0 )
            if ( bFc[i] == a0 )
            {
                zR[i] = z0;
                cuddRef( zR[i] );
                Cost[i] = 0;
            }
            else if ( bFc[i] == b1 )
            {
                zR[i] = z1;
                cuddRef( zR[i] );
                Cost[i] = 1;
            }
            else 
            {
                bFcR = Cudd_Regular(bFc[i]);

                zR[i] = extraZddFastEsopCover( dd, bFcR, Table );
                if ( zR[i] == NULL )
                {
                    Cudd_RecursiveDeref( dd, bFc[2] );
                    for ( k = 0; k < i; k++ )
                        Cudd_RecursiveDerefZdd( dd, zR[i] );
                    return NULL;
                }
                cuddRef( zR[i] );

                // get the cost of this cover
                Cost[i] = -1;
                st_lookup( Table, (char*)bFcR, (char**)&Cost[i] );
                assert( Cost[i] != -1 );

                // add the tautology cube to the cover if it was complemented
                if ( bFcR != bFc[i] )
                {
                    if ( Extra_zddEmptyBelongs( dd, zR[i] ) )
                    {
                        zR[i] = cuddZddDiff( dd, zTemp = zR[i], z1 );  
                        Cost[i]--;
                    }
                    else 
                    {
                        zR[i] = cuddZddUnion( dd, zTemp = zR[i], z1 );  
                        Cost[i]++;
                    }
                    if ( zR[i] == NULL )
                    {
                        Cudd_RecursiveDeref( dd, bFc[2] );
                        Cudd_RecursiveDerefZdd( dd, zTemp );
                        for ( k = 0; k < i; k++ )
                            Cudd_RecursiveDerefZdd( dd, zR[i] );
                        return NULL;
                    }
                    cuddRef( zR[i] );
                    Cudd_RecursiveDerefZdd( dd, zTemp );
                }
            }
        }
        Cudd_RecursiveDeref( dd, bFc[2] );

        // only the three covers are referenced as this point

        // compare the solutions
        Worst   = MAX( Cost[0],Cost[1] );
        Worst   = MAX( Worst,  Cost[2] );
        CostRes = Cost[0] + Cost[1] + Cost[2] - Worst;

        // set the best cost
        st_insert( Table, (char*)bF, (char*)CostRes );

        // derive the best cover
        if ( Worst == Cost[0] )
        { // the negative Davio node
            Cudd_RecursiveDerefZdd( dd, zR[0] );

            // take zR[1] as it is and add negative literal to zR[2]
            zRes = cuddZddGetNode( dd, bF->index*2+1, zR[2], zR[1] );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zR[1] );
                Cudd_RecursiveDerefZdd( dd, zR[2] );
                return NULL;
            }
            cuddDeref( zR[1] );
            cuddDeref( zR[2] );
        }
        else if ( Worst == Cost[1] )
        { // the positive Davio node
            Cudd_RecursiveDerefZdd( dd, zR[1] );

            // take zR[0] as it is and add positive literal to zR[2]
            zRes = cuddZddGetNode( dd, bF->index*2, zR[2], zR[0] );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zR[0] );
                Cudd_RecursiveDerefZdd( dd, zR[2] );
                return NULL;
            }
            cuddDeref( zR[0] );
            cuddDeref( zR[2] );
        }
        else if ( Worst == Cost[2] )
        { // the Shannon node
            Cudd_RecursiveDerefZdd( dd, zR[2] );

            // take zR[0] with negative literal and zR[1] with the positive literal
            cuddRef( z0 ); // should be refed because inside this function it will be derefed
            zRes = extraComposeCover( dd, zR[0], zR[1], z0, bF->index );
            if ( zRes == NULL ) 
                return NULL;
        }

        cuddCacheInsert1( dd, cacheOp, bF, zRes );
        return zRes;
    }
} /* end of extraZddFastEsopCover */

