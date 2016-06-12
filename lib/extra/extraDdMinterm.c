/**CFile****************************************************************

  FileName    [extraDdMinterm.c]

  PackageName [extra]

  Synopsis    [Procedures to count minterms in a DD.]

  Author      [Alan Mishchenko, Jordi Cortadella]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraDdMinterm.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

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

// the cache used for counting minterms in the three procedures
// if F2 is NULL, the cache entry is used by the first procedure
// if F3 is NULL, the cache entry is used by the second procedure
// otherwise, the cache entry is used by the third procedure

typedef struct Extra_MintEntry_t_ Extra_MintEntry_t;

struct Extra_MintCache_t_
{
    int            nSize;     // the table size
    Extra_MintEntry_t * pTable;    // the table
    DdManager *    dd;        // the manager
    DdNode *       bdd1;      // the constant one node of the manager
};

struct Extra_MintEntry_t_
{
    DdNode *       F1;
    DdNode *       F2;
    DdNode *       F3;
    double         dRes;
};

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

// the three procedures for counting minterms
static double extraCountMinterm( Extra_MintCache_t * pCache, DdNode * F );
static double extraCountMintermProduct( Extra_MintCache_t * pCache, DdNode * F, DdNode * G );
static double extraCountMintermExorCare( Extra_MintCache_t * pCache, DdNode * F, DdNode * G, DdNode * C );

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Starts the minterm counting cache.]

  Description [This procedure should be called before there is the need
  to run minterm counting procedures. The size is the largest 
  expected size of the decision diagrams, for which minterms counting 
  will be performed. If 0 is given as size, the procedure allocates
  the cache composed of 10000 entries, which seems to perform well 
  enough for a variety of medium-sized problems.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Extra_MintCache_t * Extra_MintermCacheStart( DdManager * dd, int nSize )
{
    Extra_MintCache_t * pCache;
    assert( nSize >= 0 );
    pCache = ALLOC( Extra_MintCache_t, 1 );
    memset( pCache, 0, sizeof(Extra_MintCache_t) );
    pCache->nSize = ((nSize > 0)? nSize : 10000);
    pCache->pTable = ALLOC( Extra_MintEntry_t, pCache->nSize );
    pCache->dd = dd;
    pCache->bdd1 = b1;
    Extra_MintermCacheClean( pCache );
    return pCache;
}

/**Function*************************************************************

  Synopsis    [Cleans the minterm counting cache.]

  Description [This procedure should be called after dynamic variable
  reordering was performed.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_MintermCacheClean( Extra_MintCache_t * pCache )
{
    int i;
    for ( i = 0; i < pCache->nSize; i++ )
        pCache->pTable[i].F1 = NULL;
}

/**Function*************************************************************

  Synopsis    [Stops the cache.]

  Description [This procedure should be called when there is no need
  to run other minterm counting procedures.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_MintermCacheStop( Extra_MintCache_t * pCache )
{
    FREE( pCache->pTable );
    FREE( pCache );
}

/**Function*************************************************************

  Synopsis    [Counts minterms in the BDD or ADD.]

  Description [Assumes that the function depends on the given number 
  of variables.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
double Extra_CountMinterm( Extra_MintCache_t * pCache, DdNode * F, int nVars )
{
    double dRes;
    assert( pCache ); // should allocate cache first
    dRes  = extraCountMinterm( pCache, F );
    return dRes * pow (2.0, nVars);
} /* end of Extra_CountMinterm */


/**Function*************************************************************

  Synopsis    [Counts minterms in the product of two BDDs or ADDs.]

  Description [Computes the following: Minterms( f (+) g, care ).
  Assumes that the function depends on the given number of variables.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
double Extra_CountMintermProduct( Extra_MintCache_t * pCache, DdNode * F, DdNode * G, int nVars )
{
    double dRes;
    assert( pCache ); // should allocate cache first
    dRes  = extraCountMintermProduct( pCache, F, G );
    return dRes * pow (2.0, nVars);
} /* end of Extra_CountMinterm */

/**Function*************************************************************

  Synopsis    [Counts minterms in the EXOR of two function over the care set.]

  Description [Computes the following formula: Minterms( f (+) g, care ).
  Assumes that the function depends on the given number of variables.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
double Extra_CountMintermExorCare( Extra_MintCache_t * pCache, DdNode * F, DdNode * G, DdNode * C, int nVars )
{
    double dRes;
    assert( pCache ); // should allocate cache first
    dRes  = extraCountMintermExorCare( pCache, F, G, C );
    return dRes * pow (2.0, nVars);
} /* end of Extra_CountMinterm */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
double extraCountMinterm( Extra_MintCache_t * pCache, DdNode * F )
{
    unsigned HKey;
    // normalize the complement
    if ( Cudd_IsComplement(F) )
        return 1.0 - extraCountMinterm( pCache, Cudd_Not(F) );

    // now it is known that the function is not complemented
    if ( cuddIsConstant(F) ) 
        return 1.0;

    // lookup the result in cache
    HKey = hashKey1( F, pCache->nSize );
    if ( pCache->pTable[HKey].F1 == F && 
         pCache->pTable[HKey].F2 == NULL )
        return pCache->pTable[HKey].dRes;
    else
    {
        // Res = Res0/2 + Res1/2;
        double Res = (extraCountMinterm( pCache, cuddE(F) ) / 2) +
                     (extraCountMinterm( pCache, cuddT(F) ) / 2);
        // insert the result into cache
        pCache->pTable[HKey].F1 = F;
        pCache->pTable[HKey].F2 = NULL;
        pCache->pTable[HKey].dRes = Res;
        return Res;
    }
}   /* end of extraCountMinterm */

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
double extraCountMintermProduct( Extra_MintCache_t * pCache, DdNode * F, DdNode * G )
{
    unsigned HKey;
    // consider the trivial cases
    if ( Cudd_IsConstant(F) )
    {
        if ( F == pCache->bdd1 )
            return extraCountMinterm( pCache, G );
        return 0.0;
    }
    if ( Cudd_IsConstant(G) )
    {
        if ( G == pCache->bdd1 )
            return extraCountMinterm( pCache, F );
        return 0.0;
    }
    // now it is known that the functions are not constants
    // normalize the arguments
    if ( F > G )
    {
        DdNode * Temp = F; F = G; G = Temp;
    }
    // lookup the result in cache
    HKey = hashKey2( F, G, pCache->nSize );
    if ( pCache->pTable[HKey].F1 == F && 
         pCache->pTable[HKey].F2 == G && 
         pCache->pTable[HKey].F3 == NULL )
        return pCache->pTable[HKey].dRes;
    else
    {
        DdNode * FR, * GR;
        DdNode * F0, * F1;
        DdNode * G0, * G1;
        int LevelF, LevelG;
        double Res0, Res1, Res;

        FR = Cudd_Regular( F ); 
        GR = Cudd_Regular( G ); 

        LevelF = pCache->dd->perm[FR->index];
        LevelG = pCache->dd->perm[GR->index];

        if ( LevelF <= LevelG )
        {
            if ( FR != F )
            {
                F0 = Cudd_Not( cuddE(FR) );
                F1 = Cudd_Not( cuddT(FR) );
            }
            else
            {
                F0 = cuddE(FR);
                F1 = cuddT(FR);
            }
        }
        else
        {
            F0 = F1 = F;
        }

        if ( LevelG <= LevelF )
        {
            if ( GR != G )
            {
                G0 = Cudd_Not( cuddE(GR) );
                G1 = Cudd_Not( cuddT(GR) );
            }
            else
            {
                G0 = cuddE(GR);
                G1 = cuddT(GR);
            }
        }
        else
        {
            G0 = G1 = G;
        }

        Res0 = extraCountMintermProduct( pCache, F0, G0 );
        Res1 = extraCountMintermProduct( pCache, F1, G1 );
        Res  = Res0 / 2 + Res1 / 2;

        // insert the result into cache
        pCache->pTable[HKey].F1 = F;
        pCache->pTable[HKey].F2 = G;
        pCache->pTable[HKey].F3 = NULL;
        pCache->pTable[HKey].dRes = Res;
        return Res;
    }
}   /* end of extraCountMintermProduct */

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
double extraCountMintermExorCare( Extra_MintCache_t * pCache, DdNode * F, DdNode * G, DdNode * C )
{
    unsigned HKey;
    // consider the trivial cases
    if ( Cudd_IsConstant(F) )
    {
        if ( Cudd_IsComplement(F) ) // F == 0
            return extraCountMintermProduct( pCache, G, C );
        return extraCountMintermProduct( pCache, Cudd_Not(G), C );
    }
    if ( Cudd_IsConstant(G) )
    {
        if ( Cudd_IsComplement(G) ) // G == 0
            return extraCountMintermProduct( pCache, F, C );
        return extraCountMintermProduct( pCache, Cudd_Not(F), C );
    }
    if ( Cudd_IsConstant(C) )
    {
        if ( Cudd_IsComplement(C) ) // C == 0
            return 0.0;
    }
    // now it is known that the functions are not constants, except C
    // normalize the arguments
    if ( F > G )
    {
        DdNode * Temp = F; F = G; G = Temp;
    }

    // lookup the result in cache
    HKey = hashKey3( F, G, C, pCache->nSize );
    if ( pCache->pTable[HKey].F1 == F && 
         pCache->pTable[HKey].F2 == G && 
         pCache->pTable[HKey].F3 == C )
        return pCache->pTable[HKey].dRes;
    else
    {
        DdNode * FR, * GR, * CR;
        DdNode * F0, * F1;
        DdNode * G0, * G1;
        DdNode * C0, * C1;
        int LevelF, LevelG, LevelC, Level;
        double Res0, Res1, Res;

        FR = Cudd_Regular( F ); 
        GR = Cudd_Regular( G ); 
        CR = Cudd_Regular( C ); 

        LevelF = pCache->dd->perm[FR->index];
        LevelG = pCache->dd->perm[GR->index];
        LevelC = cuddI(pCache->dd, CR->index);

        Level = LevelF;
        if ( Level > LevelG )
            Level = LevelG;
        if ( Level > LevelC )
            Level = LevelC;

        if ( LevelF == Level )
        {
            if ( FR != F )
            {
                F0 = Cudd_Not( cuddE(FR) );
                F1 = Cudd_Not( cuddT(FR) );
            }
            else
            {
                F0 = cuddE(FR);
                F1 = cuddT(FR);
            }
        }
        else
        {
            F0 = F1 = F;
        }

        if ( LevelG <= Level )
        {
            if ( GR != G )
            {
                G0 = Cudd_Not( cuddE(GR) );
                G1 = Cudd_Not( cuddT(GR) );
            }
            else
            {
                G0 = cuddE(GR);
                G1 = cuddT(GR);
            }
        }
        else
        {
            G0 = G1 = G;
        }

        if ( LevelC <= Level )
        {
            if ( CR != C )
            {
                C0 = Cudd_Not( cuddE(CR) );
                C1 = Cudd_Not( cuddT(CR) );
            }
            else
            {
                C0 = cuddE(CR);
                C1 = cuddT(CR);
            }
        }
        else
        {
            C0 = C1 = C;
        }

        Res0 = extraCountMintermExorCare( pCache, F0, G0, C0 );
        Res1 = extraCountMintermExorCare( pCache, F1, G1, C1 );
        Res  = Res0 / 2 + Res1 / 2;

        // insert the result into cache
        pCache->pTable[HKey].F1 = F;
        pCache->pTable[HKey].F2 = G;
        pCache->pTable[HKey].F3 = C;
        pCache->pTable[HKey].dRes = Res;
        return Res;
    }
}   /* end of extraCountMintermExorCare */

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


