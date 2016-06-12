/**CFile****************************************************************

  FileName    [extraZddCover.c]

  PackageName [extra]

  Synopsis    [Procedures to manipulate covers represented as ZDDs.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraZddCover.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define DD_ZDD_BDD_SUBSET_TAG           0x82

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

  Synopsis    [Computes the prime product of two covers represented by ZDDs.]

  Description [Computes the prime product of two covers represented by
  ZDDs. The prime product means that the final result contains only 
  prime cubes. The cubes subsumed in larger cubes are deleted on the fly. 
  The result is also a ZDD. Returns a pointer to the result if
  successful; NULL otherwise.  The covers on which Extra_zddProduct
  operates use two ZDD variables for each function variable (one ZDD
  variable for each literal of the variable). These two ZDD variables
  should be adjacent in the order.]

  SideEffects [None]

  SeeAlso     [Extra_zddProduct Extra_zddUnateProduct]

******************************************************************************/
DdNode  *
Extra_zddPrimeProduct(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddPrimeProduct(dd, f, g);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddPrimeProduct */


/**Function********************************************************************

  Synopsis    [Computes the product of two covers represented by ZDDs.]

  Description [Alternative implementation of Extra_zddProduct.]

  SideEffects [None]

  SeeAlso     [Extra_zddProduct]

******************************************************************************/
DdNode  *
Extra_zddProductAlt(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddProductAlt(dd, f, g);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddProduct */

/**Function********************************************************************

  Synopsis    [Returns all cubes compatible with the given cube.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddMinimal]

******************************************************************************/
DdNode* Extra_zddCompatible( 
  DdManager * dd,     /* the DD manager */
  DdNode * zCover,    /* the cover */
  DdNode * zCube)     /* the cube */
{
    DdNode  *res;
    do {
        dd->reordered = 0;
        res = extraZddCompatible(dd, zCover, zCube);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddCompatible */



/**Function********************************************************************

  Synopsis    [Computes resolvents of the set of clauses w.r.t. given variables.]

  Description [Computes resolvents of the set of clauses. Clause sets are 
  represented in ZDD using the same technique as covers: one ZDD variable encodes
  positive polarity variable, another ZDD variable encodes negative ZDD variable.
  The set of variables is specified by a combination Vars, containing exactly one 
  ZDD variable (either positive or negative polarity) for each clause variable.
  Resolution is performed only w.r.t. variables in the given set. The resulting 
  clause set is minimal but may include clauses that are contained (or contain) 
  clauses from the original set.]

  SideEffects [None]

  SeeAlso     [Extra_zddPrimeProduct]

******************************************************************************/
DdNode  *
Extra_zddResolve(
  DdManager * dd,
  DdNode * S,
  DdNode * Vars)
{
    DdNode  *Res, *Set0, *Set1, *ResOne, *Temp; 
    int VarIndex;

    /* make sure that the set of vars in not empty */
    if ( S == dd->zero || S == dd->one )
        return dd->zero;
    if ( Vars == dd->one )
        return dd->zero;
    if ( Vars == dd->zero )
        return NULL;

    /* start the result */
    Res = dd->zero;
    Cudd_Ref( Res );

    /* go through variables in Vars, perform resolution and accumulate results */
    for ( ; Vars != dd->one; Vars = cuddT(Vars) )
    {
        /* make sure that Vars is a cube */
        if ( cuddE( Vars ) != dd->zero )
            return NULL;

        /* find the variable number in terms of original (not ZDD) variables */
        VarIndex = (Vars->index >> 1);

        /* compute subsets of combinations with positive/negative clause var */
        /* these subsets may not be minimal!!! */
        Set0 = Cudd_zddSubset1( dd, S, 2*VarIndex + 1 );
        if ( Set0 == NULL )
            return NULL;
        Cudd_Ref( Set0 );
        Set1 = Cudd_zddSubset1( dd, S, 2*VarIndex + 0 );
        if ( Set1 == NULL )
            return NULL;
        Cudd_Ref( Set1 );

        /* resolve the subsets */
        ResOne = Extra_zddPrimeProduct( dd, Set0, Set1 );
//      ResOne = Extra_zddProduct( dd, Set0, Set1 );
        if ( ResOne == NULL )
            return NULL;
        Cudd_Ref( ResOne );
        Cudd_RecursiveDerefZdd( dd, Set0 );
        Cudd_RecursiveDerefZdd( dd, Set1 );

        /* add these resolvents to the accumulator */
        Res = Extra_zddMinUnion( dd, Temp = Res, ResOne );
        if ( Res == NULL )
            return NULL;
        Cudd_Ref( Res );
        Cudd_RecursiveDerefZdd( dd, Temp );
        Cudd_RecursiveDerefZdd( dd, ResOne );
    }

    Cudd_Deref( Res );
    return (Res);

} /* end of Extra_zddResolve */

/**Function********************************************************************

  Synopsis    [Returns irredundant sum-of-products as a ZDD.]

  Description []

  SideEffects [None]

  SeeAlso     [Extra_zddIsop]

******************************************************************************/

DdNode *
Extra_zddIsopCover(
  DdManager * dd,  /* the manager */
  DdNode * F1,     /* the BDD for the On-Set */
  DdNode * F12)    /* the BDD for the On-Set and Dc-Set */
{
    DdNode *bIrrCover, *zIrrCover;

    /* call the irredundant cover computation */
    bIrrCover = Cudd_zddIsop( dd, F1, F12, &zIrrCover );
    // both IrrCoverBdd and IrrCoverZdd are not referenced
    if ( bIrrCover == NULL ) 
        return NULL;
    Cudd_Ref( zIrrCover );
    Cudd_Ref( bIrrCover );
    Cudd_IterDerefBdd( dd, bIrrCover );

    Cudd_Deref( zIrrCover );
    return zIrrCover;
}

/**Function********************************************************************

  Synopsis    [Returns irredundant sum-of-products as a ZDD.]

  Description []

  SideEffects [None]

  SeeAlso     [Extra_zddIsop]

******************************************************************************/

void
Extra_zddIsopPrintCover(
  DdManager * dd,  /* the manager */
  DdNode * F1,     /* the BDD for the On-Set */
  DdNode * F12)    /* the BDD for the On-Set and Dc-Set */
{
    DdNode * zIrrCover;
    /* call the irredundant cover computation */
    zIrrCover = Extra_zddIsopCover( dd, F1, F12 );  Cudd_Ref( zIrrCover );
    /* call the cover print out */
    Cudd_zddPrintCover( dd, zIrrCover );
    /* dereference */
    Cudd_RecursiveDerefZdd( dd, zIrrCover );
}


/**Function********************************************************************

  Synopsis    [The procedure to count the number of cubes in the ISOP.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCoverAlt]

******************************************************************************/
DdNode * Extra_zddSimplify( 
  DdManager * dd,     /* the DD manager */
  DdNode * bFuncOn,    /* the on-set */
  DdNode * bFuncOnDc)  /* the on-set + dc-set */
{
    DdNode * res;
    do {
        dd->reordered = 0;
        res = extraZddSimplify(dd, bFuncOn, bFuncOnDc);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddSimplify */

/**Function********************************************************************

  Synopsis    [The alternative implementation of irredundant sum-of-products.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddMinimal]

******************************************************************************/
DdNode* Extra_zddIsopCoverAlt( 
  DdManager * dd,     /* the DD manager */
  DdNode * bFuncOn,    /* the on-set */
  DdNode * bFuncOnDc)  /* the on-set + dc-set */
{
    DdNode * res;
    do {
        dd->reordered = 0;
        res = extraZddIsopCoverAlt(dd, bFuncOn, bFuncOnDc);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddIsopCoverAlt */

/**Function********************************************************************

  Synopsis    [The procedure to count the number of cubes in the ISOP.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCoverAlt]

******************************************************************************/
int Extra_zddIsopCubeNum( 
  DdManager * dd,     /* the DD manager */
  DdNode * bFuncOn,    /* the on-set */
  DdNode * bFuncOnDc)  /* the on-set + dc-set */
{
    DdNode * bRes;
    int nCubes;
    do {
        dd->reordered = 0;
        bRes = extraZddIsopCubeNum(dd, bFuncOn, bFuncOnDc, &nCubes);
    } while (dd->reordered == 1);
    Cudd_Ref( bRes );
//printf( "Original BDD = %d. Final BDD = %d. Cubes = %d.\n", 
//       Cudd_DagSize(bFuncOn), Cudd_DagSize(bRes), nCubes );
    Cudd_RecursiveDeref( dd, bRes );
    return nCubes;

} /* end of Extra_zddIsopCubeNum */


/**Function********************************************************************

  Synopsis    [Selecting important cubes.]

  Description [Given two covers, C and D, and an area of boolean space, Area, 
  this procedure returns the set of all such cubes c in C, for which there 
  DOES NOT EXIST a cube d in D, such that the overlap of c and Area 
  (c intersection Area) is completely contained.]

  SideEffects []

  SeeAlso     [Extra_zddNotSub]

******************************************************************************/
DdNode* Extra_zddNotContainedCubesOverArea( 
  DdManager * dd,   /* the DD manager */
  DdNode * zC,      /* the on-set */
  DdNode * zD,      /* the on-set */
  DdNode * bA)      /* the on-set + dc-set */
{
    DdNode  *res;
    do {
        dd->reordered = 0;
        res = extraZddNotContainedCubesOverArea(dd, zC, zD, bA);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddNotContainedCubesOverArea */


/**Function********************************************************************

  Synopsis    [Returns a disjoint cover as a ZDD.]

  Description []

  SideEffects [None]

  SeeAlso     [Extra_zddIsop]

******************************************************************************/

DdNode *
Extra_zddDisjointCover(
  DdManager * dd,  /* the manager */
  DdNode * F)      /* the BDD whose cubes are enumerated to get the cover */
{
    DdGen * Gen;
    int * Cube;
    CUDD_VALUE_TYPE Value;
    int nVars = dd->size;
    int *VarValues, i;
    DdNode * zDisjCover, * zCube, * zTemp;

    if ( F == Cudd_ReadLogicZero(dd) )
        return dd->zero;
    if ( F == Cudd_ReadOne(dd)  )
        return dd->one;

    /* allocate temporary storage for variable values */
    assert( dd->sizeZ == 2 * dd->size );
    VarValues = (int*) malloc( dd->sizeZ * sizeof(int) );
    if ( VarValues == NULL ) 
    {
        dd->errorCode = CUDD_MEMORY_OUT;
        return NULL;
    }

    /* start the cover */
    zDisjCover = dd->zero;
    Cudd_Ref(zDisjCover);
    Cudd_ForeachCube( dd, F, Gen, Cube, Value )
    {
        /* clean the storage */
        for ( i = 0; i < dd->sizeZ; i++ )
            VarValues[i] = 0;

        /* clean the variable values */
        for ( i = 0; i < nVars; i++ )
            if ( Cube[i] == 0 )
//              printf( "[%d]'", i ),
                VarValues[2*i+1] = 1;
            else if ( Cube[i] == 1 )
//              printf( "[%d]", i ),
                VarValues[2*i] = 1;

        /* create the cube */
        zCube = Extra_zddCombination( dd, VarValues, dd->sizeZ );
        Cudd_Ref(zCube);

        zDisjCover = Cudd_zddUnion( dd, zTemp = zDisjCover, zCube );
        Cudd_Ref(zDisjCover);
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zCube );
    }
    free(VarValues);
    Cudd_Deref( zDisjCover );
    return zDisjCover;
}

/**Function********************************************************************

  Synopsis    [Selects one cube from a ZDD representing the cube cover.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode* Extra_zddSelectOneCube( 
  DdManager * dd,   /* the DD manager */
  DdNode * zS)      /* the cover */
{
    DdNode  *res;
    do {
        dd->reordered = 0;
        res = extraZddSelectOneCube(dd, zS);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddSelectOneCube */


/**Function********************************************************************

  Synopsis    [Selects one subset from the set of subsets represented by a ZDD.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode* Extra_zddSelectOneSubset( 
  DdManager * dd,   /* the DD manager */
  DdNode * zS)      /* the ZDD */
{
    DdNode  *res;
    do {
        dd->reordered = 0;
        res = extraZddSelectOneSubset(dd, zS);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddSelectOneSubset */



/**Function********************************************************************

  Synopsis    [Checks unateness of the cover.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int Extra_zddCheckUnateness( 
  DdManager * dd, 
  DdNode *zCover)
{
    int i;
    int * s_pVarValues = (int*) malloc( dd->sizeZ * sizeof(int) );
    Extra_SupportArray( dd, zCover, s_pVarValues );
    for ( i = 0; i < dd->sizeZ/2; i++ )
        if ( s_pVarValues[ 2*i ] == 1 && s_pVarValues[ 2*i+1 ] == 1 )
        {
            free( s_pVarValues );
            return 0;
        }
    free( s_pVarValues );
    return 1;
} /* end of Extra_zddCheckUnateness */

/**Function********************************************************************

  Synopsis [Computes the BDD of the area covered by the max number of cubes in a ZDD.]

  Description [Given a cover represented as a ZDD, this function computes the
  area of the on-set where the largest number of cubes overlap. The optional argument
  nOverlaps, if it is non-NULL, contains the number of overlaps. Returns the 
  BDD of the area if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
Extra_zddGetMostCoveredArea(
  DdManager * dd,
  DdNode * zCover,
  int * nOverlaps)
{
    DdNode *bRes, *aAreas, *aMaxNode;

    do {
        dd->reordered = 0;
        aAreas = extraZddGetMostCoveredArea(dd, zCover);
    } while (dd->reordered == 1);

    cuddRef( aAreas );

    /* get the maximal value area of this ADD */
    aMaxNode = Cudd_addFindMax( dd, aAreas );    Cudd_Ref( aMaxNode );

    if ( nOverlaps )
        *nOverlaps = (int)cuddV(aMaxNode);

    /* get the BDD representing the max value */
    bRes = Cudd_addBddThreshold( dd, aAreas, cuddV(aMaxNode) ); Cudd_Ref( bRes );

    /* dereference */
    Cudd_RecursiveDeref( dd, aMaxNode );
    Cudd_RecursiveDeref( dd, aAreas );
    cuddDeref( bRes );

    return(bRes);

} /* end of Extra_zddGetMostCoveredArea */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Finds three cofactors of the cover w.r.t. to the topmost variable.]

  Description [Finds three cofactors of the cover w.r.t. to the topmost variable.
  Does not check the cover for being a constant. Assumes that ZDD variables encoding 
  positive and negative polarities are adjacent in the variable order. Is different 
  from cuddZddGetCofactors3() in that it does not compute the cofactors w.r.t. the 
  given variable but takes the cofactors with respent to the topmost variable. 
  This function is more efficient when used in recursive procedures because it does 
  not require referencing of the resulting cofactors (compare cuddZddProduct() 
  and extraZddPrimeProduct()).]

  SideEffects [None]

  SeeAlso     [cuddZddGetCofactors3]

******************************************************************************/
void 
extraDecomposeCover( 
  DdManager* dd,    /* the manager */
  DdNode*  zC,      /* the cover */
  DdNode** zC0,     /* the pointer to the negative var cofactor */ 
  DdNode** zC1,     /* the pointer to the positive var cofactor */ 
  DdNode** zC2 )    /* the pointer to the cofactor without var */ 
{
    if ( (zC->index & 1) == 0 ) 
    { /* the top variable is present in positive polarity and maybe in negative */

        DdNode *Temp = cuddE( zC );
        *zC1  = cuddT( zC );
        if ( cuddIZ(dd,Temp->index) == cuddIZ(dd,zC->index) + 1 )
        {   /* Temp is not a terminal node 
             * top var is present in negative polarity */
            *zC2 = cuddE( Temp );
            *zC0 = cuddT( Temp );
        }
        else
        {   /* top var is not present in negative polarity */
            *zC2 = Temp;
            *zC0 = dd->zero;
        }
    }
    else 
    { /* the top variable is present only in negative */
        *zC1 = dd->zero;
        *zC2 = cuddE( zC );
        *zC0 = cuddT( zC );
    }
} /* extraDecomposeCover */


/**Function********************************************************************

  Synopsis    [Composed three subcovers into one ZDD.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
extraComposeCover( 
  DdManager* dd,    /* the manager */
  DdNode* zC0,     /* the pointer to the negative var cofactor */ 
  DdNode* zC1,     /* the pointer to the positive var cofactor */ 
  DdNode* zC2,     /* the pointer to the cofactor without var */ 
  int TopVar)    /* the index of the positive ZDD var */
{
    DdNode * zRes, * zTemp;
    /* compose with-neg-var and without-var using the neg ZDD var */
    zTemp = cuddZddGetNode( dd, 2*TopVar + 1, zC0, zC2 );
    if ( zTemp == NULL ) 
    {
        Cudd_RecursiveDerefZdd(dd, zC0);
        Cudd_RecursiveDerefZdd(dd, zC1);
        Cudd_RecursiveDerefZdd(dd, zC2);
        return NULL;
    }
    cuddRef( zTemp );
    cuddDeref( zC0 );
    cuddDeref( zC2 );

    /* compose with-pos-var and previous result using the pos ZDD var */
    zRes = cuddZddGetNode( dd, 2*TopVar, zC1, zTemp );
    if ( zRes == NULL ) 
    {
        Cudd_RecursiveDerefZdd(dd, zC1);
        Cudd_RecursiveDerefZdd(dd, zTemp);
        return NULL;
    }
    cuddDeref( zC1 );
    cuddDeref( zTemp );
    return zRes;
} /* extraComposeCover */


/**Function********************************************************************

  Synopsis [Performs the recursive step of Extra_zddPrimeProduct.]

  Description []

  SideEffects [None]

  SeeAlso     [Extra_zddProduct]

******************************************************************************/
DdNode  *
extraZddPrimeProduct(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode  *zRes;
    int     LevelF, LevelG;
    statLine(dd); 

    /* terminal cases */
    if (f == dd->zero || g == dd->zero)
        return dd->zero;
    if (f == dd->one)
        return g;
    if (g == dd->one)
        return f;
    if (f == g)
        return f;

    /* level in terms of original variables, not ZDD variables */
    LevelF = dd->permZ[f->index] >> 1;
    LevelG = dd->permZ[g->index] >> 1;

    /* normalize */
    if (LevelF > LevelG)
        return extraZddPrimeProduct(dd, g, f);

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddPrimeProduct, f, g);
    if (zRes)
        return zRes;
    else
    {
        DdNode *zRes0, *zRes1, *zRes2, *zTemp;
        int TopZddVar;

        if ( LevelF < LevelG )
        {
            DdNode *zF0, *zF1, *zF2;
            extraDecomposeCover(dd, f, &zF0, &zF1, &zF2);

            /* (zF0 + zF1 + zF2) * g = // the expanded product
             *
             *  zF0 * g +              // cofactors with negative var
             *  zF1 * g +              // cofactors with positive var
             *  zF2 * g =              // cofactors without var
             */
            
            /* ------  cofactor with negative variable ------ */
            /* compute (zF0 * g) */
            zRes0 = extraZddPrimeProduct( dd, zF0, g );
            if ( zRes0 == NULL )
                return NULL;
            cuddRef( zRes0 );

            /* ------- cofactor with positive variable ------ */
            /* compute (zF1 * g) */
            zRes1 = extraZddPrimeProduct( dd, zF1, g );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            /* --------- cofactor without variable ------------ */
            /* compute (zF2 + g) */
            zRes2 = extraZddPrimeProduct( dd, zF2, g );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddRef( zRes2 );
            /* at this point, only zRes0, zRes2, and zRes2 are referenced */
        }
        else /* if ( LevelF == LevelG ) */
        {
            DdNode *zF0, *zF1, *zF2;
            DdNode *zG0, *zG1, *zG2, *zG20, *zG21;
            DdNode *zTerm1, *zTerm2;

            extraDecomposeCover(dd, f, &zF0, &zF1, &zF2);
            extraDecomposeCover(dd, g, &zG0, &zG1, &zG2);

            /* (zF0 + zF1 + zF2) * (zG0 + zG1 + zG2) = // the expanded product
             *
             * (zF2 * zG0 + zF0 * zG2 + zF0 * zG0) +   // cofactors with negative var
             * (zF2 * zG1 + zF1 * zG2 + zF1 * zG1) +   // cofactors with positive var
             *  zF2 * zG2 =                            // cofactors without var
             *
             * (zF2 * zG0 + zF0 * (zG2 + zG0)) +       // cofactors with negative var
             * (zF2 * zG1 + zF1 * (zG2 + zG1)) +       // cofactors with positive var
             *  zF2 * zG2                              // cofactors without var
             */

            /* ------  cofactors with negative variable ------ */
            /* compute (zG2 + zG0) */
            zG20 = extraZddMinUnion( dd, zG2, zG0 );
            if ( zG20 == NULL )
                return NULL;
            cuddRef( zG20 );

            /* compute zF0 * (zG2 + zG0) */
            zTerm1 = extraZddPrimeProduct( dd, zF0, zG20 );
            if ( zTerm1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zG20 );
                return NULL;
            }
            cuddRef( zTerm1 );
            Cudd_RecursiveDerefZdd( dd, zG20 );

            /* compute (zF2 * zG0) */
            zTerm2 = extraZddPrimeProduct( dd, zF2, zG0 );
            if ( zTerm2 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zTerm1 );
                return NULL;
            }
            cuddRef( zTerm2 );

            /* compute the sum of these two parts */
            zRes0 = extraZddMinUnion( dd, zTerm1, zTerm2 );
            if ( zRes0 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zTerm1 );
                Cudd_RecursiveDerefZdd( dd, zTerm2 );
                return NULL;
            }
            cuddRef( zRes0 );
            Cudd_RecursiveDerefZdd( dd, zTerm1 );
            Cudd_RecursiveDerefZdd( dd, zTerm2 );


            /* ------- cofactors with positive variable ------ */
            /* compute (zG2 + zG1) */
            zG21 = extraZddMinUnion( dd, zG2, zG1 );
            if ( zG21 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zG21 );

            /* compute zF0 * (zG2 + zG0) */
            zTerm1 = extraZddPrimeProduct( dd, zF1, zG21 );
            if ( zTerm1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zG21 );
                return NULL;
            }
            cuddRef( zTerm1 );
            Cudd_RecursiveDerefZdd( dd, zG21 );

            /* compute (zF2 * zG0) */
            zTerm2 = extraZddPrimeProduct( dd, zF2, zG1 );
            if ( zTerm2 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zTerm1 );
                return NULL;
            }
            cuddRef( zTerm2 );

            /* compute the sum of these two parts */
            zRes1 = extraZddMinUnion( dd, zTerm1, zTerm2 );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zTerm1 );
                Cudd_RecursiveDerefZdd( dd, zTerm2 );
                return NULL;
            }
            cuddRef( zRes1 );
            Cudd_RecursiveDerefZdd( dd, zTerm1 );
            Cudd_RecursiveDerefZdd( dd, zTerm2 );

            /* --------- cofactor without variable ------------ */
            /* compute (zF2 + zG2) */
            zRes2 = extraZddPrimeProduct( dd, zF2, zG2 );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddRef( zRes2 );
            /* at this point, only zRes0, zRes2, and zRes2 are referenced */
        }

        /* ---- filtering cofactors with/without variable --- */
        /* remove negative var cubes contained in cubes without var */
        zRes0 = extraZddNotSupSet(dd, zTemp = zRes0, zRes2);
        if ( zRes0 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zTemp);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            Cudd_RecursiveDerefZdd(dd, zRes2);
            return NULL;
        }
        cuddRef( zRes0 );
        Cudd_RecursiveDerefZdd(dd, zTemp);

        /* remove positive var cubes contained in cubes without var */
        zRes1 = extraZddNotSupSet(dd, zTemp = zRes1, zRes2);
        if ( zRes1 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zTemp);
            Cudd_RecursiveDerefZdd(dd, zRes2);
            return NULL;
        }
        cuddRef( zRes1 );
        Cudd_RecursiveDerefZdd(dd, zTemp);

        /* --------------- composing the result ------------------ */
        /* the index of the positive ZDD variable in F */
        TopZddVar = (f->index >> 1) << 1;

        /* compose with-neg-var and without-var using the neg ZDD var */
        zTemp = cuddZddGetNode(dd, TopZddVar + 1, zRes0, zRes2 );
        if ( zTemp == NULL ) 
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            Cudd_RecursiveDerefZdd(dd, zRes2);
            return NULL;
        }
        cuddRef( zTemp );
        cuddDeref( zRes0 );
        cuddDeref( zRes2 );

        /* compose with-pos-var and previous result using the pos ZDD var */
        zRes = cuddZddGetNode(dd, TopZddVar, zRes1, zTemp );
        if ( zRes == NULL ) 
        {
            Cudd_RecursiveDerefZdd(dd, zRes1);
            Cudd_RecursiveDerefZdd(dd, zTemp);
            return NULL;
        }
        cuddDeref( zRes1 );
        cuddDeref( zTemp );

        /* insert the result into cache */
        cuddCacheInsert2(dd, extraZddPrimeProduct, f, g, zRes);
        return zRes;
    }
} /* end of extraZddPrimeProduct */


/**Function********************************************************************

  Synopsis [Performs the recursive step of Extra_zddProductAlt.]

  Description [Alternative implementation of Extra_zddProduct]

  SideEffects [None]

  SeeAlso     [Extra_zddProduct]

******************************************************************************/
DdNode  *
extraZddProductAlt(
  DdManager * dd,
  DdNode * f,
  DdNode * g)
{
    DdNode  *zRes;
    int     LevelF, LevelG;
    statLine(dd); 

    /* terminal cases */
    if (f == dd->zero || g == dd->zero)
        return dd->zero;
    if (f == dd->one)
        return g;
    if (g == dd->one)
        return f;
    if (f == g)
        return f;

    /* level in terms of original variables, not ZDD variables */
    LevelF = dd->permZ[f->index] >> 1;
    LevelG = dd->permZ[g->index] >> 1;

    /* normalize */
    if (LevelF > LevelG)
        return extraZddProductAlt(dd, g, f);

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddProductAlt, f, g);
    if (zRes)
        return zRes;
    else
    {
        DdNode *zRes0, *zRes1, *zRes2, *zTemp;
        int TopZddVar;

        if ( LevelF < LevelG )
        {
            DdNode *zF0, *zF1, *zF2;
            extraDecomposeCover(dd, f, &zF0, &zF1, &zF2);

            /* (zF0 + zF1 + zF2) * g = // the expanded product
             *
             *  zF0 * g +              // cofactors with negative var
             *  zF1 * g +              // cofactors with positive var
             *  zF2 * g =              // cofactors without var
             */
            
            /* ------  cofactor with negative variable ------ */
            /* compute (zF0 * g) */
            zRes0 = extraZddProductAlt( dd, zF0, g );
            if ( zRes0 == NULL )
                return NULL;
            cuddRef( zRes0 );

            /* ------- cofactor with positive variable ------ */
            /* compute (zF1 * g) */
            zRes1 = extraZddProductAlt( dd, zF1, g );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            /* --------- cofactor without variable ------------ */
            /* compute (zF2 + g) */
            zRes2 = extraZddProductAlt( dd, zF2, g );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddRef( zRes2 );
            /* at this point, only zRes0, zRes2, and zRes2 are referenced */
        }
        else /* if ( LevelF == LevelG ) */
        {
            DdNode *zF0, *zF1, *zF2;
            DdNode *zG0, *zG1, *zG2, *zG20, *zG21;
            DdNode *zTerm1, *zTerm2;

            extraDecomposeCover(dd, f, &zF0, &zF1, &zF2);
            extraDecomposeCover(dd, g, &zG0, &zG1, &zG2);

            /* (zF0 + zF1 + zF2) * (zG0 + zG1 + zG2) = // the expanded product
             *
             * (zF2 * zG0 + zF0 * zG2 + zF0 * zG0) +   // cofactors with negative var
             * (zF2 * zG1 + zF1 * zG2 + zF1 * zG1) +   // cofactors with positive var
             *  zF2 * zG2 =                            // cofactors without var
             *
             * (zF2 * zG0 + zF0 * (zG2 + zG0)) +       // cofactors with negative var
             * (zF2 * zG1 + zF1 * (zG2 + zG1)) +       // cofactors with positive var
             *  zF2 * zG2                              // cofactors without var
             */

            /* ------  cofactors with negative variable ------ */
            /* compute (zG2 + zG0) */
            zG20 = cuddZddUnion( dd, zG2, zG0 );
            if ( zG20 == NULL )
                return NULL;
            cuddRef( zG20 );

            /* compute zF0 * (zG2 + zG0) */
            zTerm1 = extraZddProductAlt( dd, zF0, zG20 );
            if ( zTerm1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zG20 );
                return NULL;
            }
            cuddRef( zTerm1 );
            Cudd_RecursiveDerefZdd( dd, zG20 );

            /* compute (zF2 * zG0) */
            zTerm2 = extraZddProductAlt( dd, zF2, zG0 );
            if ( zTerm2 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zTerm1 );
                return NULL;
            }
            cuddRef( zTerm2 );

            /* compute the sum of these two parts */
            zRes0 = cuddZddUnion( dd, zTerm1, zTerm2 );
            if ( zRes0 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zTerm1 );
                Cudd_RecursiveDerefZdd( dd, zTerm2 );
                return NULL;
            }
            cuddRef( zRes0 );
            Cudd_RecursiveDerefZdd( dd, zTerm1 );
            Cudd_RecursiveDerefZdd( dd, zTerm2 );


            /* ------- cofactors with positive variable ------ */
            /* compute (zG2 + zG1) */
            zG21 = cuddZddUnion( dd, zG2, zG1 );
            if ( zG21 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zG21 );

            /* compute zF0 * (zG2 + zG0) */
            zTerm1 = extraZddProductAlt( dd, zF1, zG21 );
            if ( zTerm1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zG21 );
                return NULL;
            }
            cuddRef( zTerm1 );
            Cudd_RecursiveDerefZdd( dd, zG21 );

            /* compute (zF2 * zG0) */
            zTerm2 = extraZddProductAlt( dd, zF2, zG1 );
            if ( zTerm2 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zTerm1 );
                return NULL;
            }
            cuddRef( zTerm2 );

            /* compute the sum of these two parts */
            zRes1 = cuddZddUnion( dd, zTerm1, zTerm2 );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zTerm1 );
                Cudd_RecursiveDerefZdd( dd, zTerm2 );
                return NULL;
            }
            cuddRef( zRes1 );
            Cudd_RecursiveDerefZdd( dd, zTerm1 );
            Cudd_RecursiveDerefZdd( dd, zTerm2 );

            /* --------- cofactor without variable ------------ */
            /* compute (zF2 + zG2) */
            zRes2 = extraZddProductAlt( dd, zF2, zG2 );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddRef( zRes2 );
            /* at this point, only zRes0, zRes2, and zRes2 are referenced */
        }

        /* --------------- composing the result ------------------ */
        /* the index of the positive ZDD variable in F */
        TopZddVar = (f->index >> 1) << 1;

        /* compose with-neg-var and without-var using the neg ZDD var */
        zTemp = cuddZddGetNode(dd, TopZddVar + 1, zRes0, zRes2 );
        if ( zTemp == NULL ) 
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            Cudd_RecursiveDerefZdd(dd, zRes2);
            return NULL;
        }
        cuddRef( zTemp );
        cuddDeref( zRes0 );
        cuddDeref( zRes2 );

        /* compose with-pos-var and previous result using the pos ZDD var */
        zRes = cuddZddGetNode(dd, TopZddVar, zRes1, zTemp );
        if ( zRes == NULL ) 
        {
            Cudd_RecursiveDerefZdd(dd, zRes1);
            Cudd_RecursiveDerefZdd(dd, zTemp);
            return NULL;
        }
        cuddDeref( zRes1 );
        cuddDeref( zTemp );

        /* insert the result into cache */
        cuddCacheInsert2(dd, extraZddProductAlt, f, g, zRes);
        return zRes;
    }
} /* end of extraZddProductAlt */


/**Function********************************************************************

  Synopsis [Performs the recursive step of Extra_zddCompatible.]

  Description []

  SideEffects [None]

  SeeAlso     [Extra_zddCompatibleCount]

******************************************************************************/
DdNode  *
extraZddCompatible(
  DdManager * dd,     /* the DD manager */
  DdNode * zCover,    /* the cover */
  DdNode * zCube)     /* the cube */
{
    DdNode  *zRes;
    statLine(dd); 

    /* terminal cases */
    assert( zCube != dd->zero );
    if ( zCube == dd->one || zCover == dd->zero || zCover == dd->one )
        return zCover;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddCompatible, zCover, zCube);
    if (zRes)
        return zRes;
    else
    {
        DdNode *zRes0, *zRes1, *zRes2, *zTemp;
        int TopZddVar;

        /* level in terms of original variables, not ZDD variables */
        int LevelCover = dd->permZ[zCover->index] >> 1;
        int LevelCube  = dd->permZ[zCube->index]  >> 1;

        if ( LevelCover > LevelCube )
            return extraZddCompatible( dd, zCover, cuddT(zCube) );
        else if ( LevelCover < LevelCube )
        {
            /* decompose the cover */
            DdNode *zCover0, *zCover1, *zCover2;
            extraDecomposeCover(dd, zCover, &zCover0, &zCover1, &zCover2);

            /* cover with negative literal */
            zRes0 = extraZddCompatible( dd, zCover0, zCube );
            if ( zRes0 == NULL )
                return NULL;
            cuddRef( zRes0 );

            /* cover with positive literal */
            zRes1 = extraZddCompatible( dd, zCover1, zCube );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes0);
                return NULL;
            }
            cuddRef( zRes1 );
        
            /* cover without literal */
            zRes2 = extraZddCompatible( dd, zCover2, zCube );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
            cuddRef( zRes2 );
        }
        else /* if ( LevelCover == LevelCube ) */
        {
            /* decompose the cover */
            DdNode *zCover0, *zCover1, *zCover2;
            extraDecomposeCover(dd, zCover, &zCover0, &zCover1, &zCover2);

            if ( 2 * LevelCube == dd->permZ[zCube->index] )
            { /* cube has positive literal */

                /* cover with positive literal */
                zRes1 = extraZddCompatible( dd, zCover1, cuddT(zCube) );
                if ( zRes1 == NULL )
                    return NULL;
                cuddRef( zRes1 );

                /* cover with negative literal */
                zRes0 = dd->zero;
                cuddRef( zRes0 );
            }
            else if ( 2 * LevelCube + 1 == dd->permZ[zCube->index] )
            { /* cube has negative literal */

                /* cover with negative literal */
                zRes0 = extraZddCompatible( dd, zCover0, cuddT(zCube) );
                if ( zRes0 == NULL )
                    return NULL;
                cuddRef( zRes0 );

                /* cover with positive literal */
                zRes1 = dd->zero;
                cuddRef( zRes1 );
            }
            else
                assert(0);
        
            /* cover without literal */
            zRes2 = extraZddCompatible( dd, zCover2, cuddT(zCube) );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
            cuddRef( zRes2 );
        }

        /* --------------- composing the result ------------------ */
        /* the index of the positive ZDD variable in zCover */
        TopZddVar = (zCover->index >> 1) << 1;

        /* compose with-neg-var and without-var using the neg ZDD var */
        zTemp = cuddZddGetNode(dd, TopZddVar + 1, zRes0, zRes2 );
        if ( zTemp == NULL ) 
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            Cudd_RecursiveDerefZdd(dd, zRes2);
            return NULL;
        }
        cuddRef( zTemp );
        cuddDeref( zRes0 );
        cuddDeref( zRes2 );

        /* compose with-pos-var and previous result using the pos ZDD var */
        zRes = cuddZddGetNode(dd, TopZddVar, zRes1, zTemp );
        if ( zRes == NULL ) 
        {
            Cudd_RecursiveDerefZdd(dd, zRes1);
            Cudd_RecursiveDerefZdd(dd, zTemp);
            return NULL;
        }
        cuddDeref( zRes1 );
        cuddDeref( zTemp );

        /* insert the result into cache */
        cuddCacheInsert2(dd, extraZddCompatible, zCover, zCube, zRes);
        return zRes;
    }
} /* end of extraZddCompatible */



/**Function********************************************************************

  Synopsis    [Performs the recursive step of the alternative ISOP computation.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/
DdNode* extraZddSimplify( 
  DdManager * dd,     /* the DD manager */
  DdNode * bA,        /* the on-set */
  DdNode * bB)        /* the on-set + dc-set */
{
    DdNode  *zRes;
    statLine(dd); 

    /* if there is no area (the cover should be empty), there is nowhere to expand */
    if ( bA == b0 )    return z0;
    /* if the area is the total boolean space, the cover is expanded into a single large cube */
    if ( bB == b1 )    return z1;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddSimplify, bA, bB);
    if (zRes)
        return zRes;
    else
    {
        DdNode *bA0, *bA1, *bB0, *bB1;
        DdNode *zRes0, *zRes1, *zRes2, *zTemp;

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


        /* zRes0 = extraZddSimplify( bA0, bB0 ) */
        zRes0 = extraZddSimplify( dd, bA0, bB0 );
        if ( zRes0 == NULL )
            return NULL;
        cuddRef( zRes0 );

        /* zRes1 = extraZddSimplify( bA1, bB1 ) */
        zRes1 = extraZddSimplify( dd, bA1, bB1 );
        if ( zRes1 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( zRes1 );

        /* zRes2 = cuddZddIntersect( zRes0, zRes1 ) */
        zRes2 = cuddZddIntersect( dd, zRes0, zRes1 );
        if ( zRes2 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( zRes2 );

        /* zRes0 = cuddZddDiff( zRes0, zRes2 ) */
        zRes0 = cuddZddDiff( dd, zTemp = zRes0, zRes2 );
        if ( zRes0 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zTemp);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            Cudd_RecursiveDerefZdd(dd, zRes2);
            return NULL;
        }
        cuddRef( zRes0 );
        Cudd_RecursiveDerefZdd(dd, zTemp);

        /* zRes1 = cuddZddDiff( zRes1, zRes2 ) */
        zRes1 = cuddZddDiff( dd, zTemp = zRes1, zRes2 );
        if ( zRes1 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zTemp);
            Cudd_RecursiveDerefZdd(dd, zRes2);
            return NULL;
        }
        cuddRef( zRes1 );
        Cudd_RecursiveDerefZdd(dd, zTemp);

        /* --------------- compose the result ------------------ */
        zRes = extraComposeCover( dd, zRes0, zRes1, zRes2, TopVar );
        if ( zRes == NULL ) return NULL;

        /* insert the result into cache and return */
        cuddCacheInsert2(dd, extraZddSimplify, bA, bB, zRes);
        return zRes;
    }
} /* end of extraZddSimplify */



/**Function********************************************************************

  Synopsis    [Performs the recursive step of the alternative ISOP computation.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/
DdNode* extraZddIsopCoverAlt( 
  DdManager * dd,     /* the DD manager */
  DdNode * bA,        /* the on-set */
  DdNode * bB)        /* the on-set + dc-set */
{
    DdNode  *zRes;
    statLine(dd); 

    /* if there is no area (the cover should be empty), there is nowhere to expand */
    if ( bA == b0 )    return z0;
    /* if the area is the total boolean space, the cover is expanded into a single large cube */
    if ( bB == b1 )    return z1;

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
        zRes0 = extraZddIsopCoverAlt( dd, bG0, bB0 );
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
        zRes1 = extraZddIsopCoverAlt( dd, bG1, bB1 );
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
        zRes2 = extraZddIsopCoverAlt( dd, bHA, bHB );
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
} /* end of extraZddIsopCoverAlt */




/**Function********************************************************************

  Synopsis    [Performs the recursive step of the alternative ISOP computation.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/
DdNode * extraZddIsopCubeNum( 
  DdManager * dd,     /* the DD manager */
  DdNode * bA,        /* the on-set */
  DdNode * bB,        /* the on-set + dc-set */
  int * pnCubes)      /* the number of cubes (return value) */
{
    DdNode  * bRes;
    DdNode  * aNum;
    DdNode * (*cacheOp1)(DdManager*,DdNode*);
    DdNode * (*cacheOp2)(DdManager*,DdNode*,DdNode*);

    statLine(dd); 

    /* if there is no area (the cover should be empty), there is nowhere to expand */
    if ( bA == b0 )    
    {
        *pnCubes = 0;
        return b0;
    }
    /* if the area is the total boolean space, the cover is expanded into a single large cube */
    if ( bB == b1 )    
    {
        *pnCubes = 1;
        return b1;
    }

    assert( Cudd_bddLeq( dd, bA, bB ) );

    /* check cache */
    cacheOp1 = (DdNode*(*)(DdManager*,DdNode*))extraZddIsopCubeNum;
    cacheOp2 = (DdNode*(*)(DdManager*,DdNode*,DdNode*))Extra_zddIsopCubeNum;

    bRes = cuddCacheLookup2(dd, cacheOp2, bA, bB);
    if ( bRes )
    {
        // get the number of cubes in this ISOP
        aNum = cuddCacheLookup1( dd, cacheOp1, bRes );
        if ( aNum )
        {
            cuddRef( aNum );
            *pnCubes = (int)(cuddV(aNum));
            Cudd_RecursiveDeref( dd, aNum );
            return bRes;
        }
        cuddRef( bRes );
        Cudd_IterDerefBdd( dd, bRes );
    }
    // if the ISOP as a BDD or the number of cubes cannot be found, 
    // perform the regular expansion
    {
        DdNode *bA0, *bA1, *bB0, *bB1, *bG0, *bG1, *bHA, *bHB;
        DdNode *bRes0, *bRes1, *bRes2, *bTemp;

        DdNode *bAA = Cudd_Regular(bA);
        DdNode *bBB = Cudd_Regular(bB);

        int LevA = dd->perm[bAA->index];
        int LevB = dd->perm[bBB->index];

        int nCubes0, nCubes1, nCubes2;

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
        bRes0 = extraZddIsopCubeNum( dd, bG0, bB0, &nCubes0 );
        if ( bRes0 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            return NULL;
        }
        cuddRef( bRes0 );
        Cudd_IterDerefBdd(dd, bG0);


        /* g0 = F1(x=1) & !F12(x=0) */
        bG1 = cuddBddAndRecur( dd, bA1, Cudd_Not(bB0) );
        if ( bG1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bRes0);
            return NULL;
        }
        cuddRef( bG1 );

        /* P1 = IrrCover( g1, F12(x=1) ) */
        bRes1 = extraZddIsopCubeNum( dd, bG1, bB1, &nCubes1 );
        if ( bRes1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_IterDerefBdd(dd, bRes0);
            return NULL;
        }
        cuddRef( bRes1 );
        Cudd_IterDerefBdd(dd, bG1);

        /* h1 = F1(x=0) & !bdd(P0)  +   F1(x=1) & !bdd(P1) */
        bG0 = cuddBddAndRecur( dd, bA0, Cudd_Not(bRes0) );
        if ( bG0 == NULL )
        {
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes1);
            return NULL;
        }
        cuddRef( bG0 );

        bG1 = cuddBddAndRecur( dd, bA1, Cudd_Not(bRes1) );
        if ( bG1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes1);
            return NULL;
        }
        cuddRef( bG1 );

        bHA = cuddBddAndRecur( dd, Cudd_Not(bG0), Cudd_Not(bG1) );
        if ( bHA == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes1);
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
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes1);
            return NULL;
        }
        cuddRef( bHB );

        /* P1 = IrrCover( h1, h12 ) */
        bRes2 = extraZddIsopCubeNum( dd, bHA, bHB, &nCubes2 );
        if ( bRes2 == NULL )
        {
            Cudd_IterDerefBdd(dd, bHA);
            Cudd_IterDerefBdd(dd, bHB);
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes1);
            return NULL;
        }
        cuddRef( bRes2 );
        Cudd_IterDerefBdd(dd, bHA);
        Cudd_IterDerefBdd(dd, bHB);

        // add 2 to 0 and 1
        bRes0 = cuddBddAndRecur( dd, bTemp = Cudd_Not(bRes0), Cudd_Not(bRes2) );
        if ( bRes0 == NULL )
        {
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes1);
            Cudd_IterDerefBdd(dd, bRes2);
            return NULL;
        }
        cuddRef( bRes0 );
        bRes0 = Cudd_Not( bRes0 );
        Cudd_IterDerefBdd(dd, bTemp);

        bRes1 = cuddBddAndRecur( dd, bTemp = Cudd_Not(bRes1), Cudd_Not(bRes2) );
        if ( bRes1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes1);
            Cudd_IterDerefBdd(dd, bRes2);
            return NULL;
        }
        cuddRef( bRes1 );
        bRes1 = Cudd_Not( bRes1 );
        Cudd_IterDerefBdd(dd, bTemp);
        Cudd_IterDerefBdd(dd, bRes2);


        /* --------------- compose the result ------------------ */
        *pnCubes = nCubes0 + nCubes1 + nCubes2;

        /* consider the case when Res0 and Res1 are the same node */
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        /* consider the case when Res1 is complemented */
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, TopVar, Cudd_Not(bRes1), Cudd_Not(bRes0));
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
            bRes = cuddUniqueInter( dd, TopVar, bRes1, bRes0 );
            if ( bRes == NULL ) 
            {
                Cudd_RecursiveDeref(dd,bRes0);
                Cudd_RecursiveDeref(dd,bRes1);
                return NULL;
            }
        }
        cuddDeref( bRes0 );
        cuddDeref( bRes1 );
        cuddRef( bRes );

        // insert the BDD of the ISOP into cache
        cuddCacheInsert2(dd, cacheOp2, bA, bB, bRes);

        // get the ADD node, which represents the number of cubes in the ISOP
        aNum = cuddUniqueConst( dd, (CUDD_VALUE_TYPE)(*pnCubes) );
        if ( aNum == NULL ) 
        {
            Cudd_RecursiveDeref(dd,bRes);
            return NULL;
        }

        // insert the ADD node into cache
        cuddCacheInsert1(dd, cacheOp1, bRes, aNum);

        // return the BDD of the ISOP
        cuddDeref( bRes );
        return bRes;
    }
} /* end of extraZddIsopCubeNum */



/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_zddNotContainedCubesOverArea().]

  Description [Given two covers, C and D, and an area of boolean space, A, 
  this procedure returns the set of all such cubes c in C, for which there 
  DOES NOT EXIST a cube d in D, such that the overlap of c and A 
  (c intersection A) is completely contained.]

  SideEffects []

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/
DdNode* extraZddNotContainedCubesOverArea( 
  DdManager * dd,     /* the DD manager */
  DdNode * zC,        /* the cover whose cubes are filtered */
  DdNode * zD,        /* the cover whose cubes are used to determine filterning */
  DdNode * bA)        /* the area */
{
    DdNode  *zRes;
    statLine(dd); 

    /* if there is no area, the intersection of any cube with it 
       is empty and therefore is always contained in D (D is not empty!) */
    if ( bA == b0 )    return z0;
    /* if there are no containing cubes, any cover is not contained */
    if ( zD == z0 )    return zC;
    /* if the area is the b-space, only those cubes are not contained
       whose literals are not supersets of cubes in D */
    if ( bA == b1 )    return extraZddNotSupSet( dd, zC, zD );
    /* if D is the tautology cube (and area is not empty), 
       the intersection of cubes in C with area is always contained in D */
    if ( zD == z1 )    return z0;
    /* if there are no cubes in C, none of them are contained or not contained */
    if ( zC == z0 )    return z0;
    /* if C is the tautology cube, it is contained only if
       there is at least one cube in D that completely contains A */
    /* this case is treated by further expansion */
//  if ( zC == z1 )    return ???

    /* check cache */
    zRes = cuddCacheLookup(dd, DD_ZDD_BDD_SUBSET_TAG, zC, zD, bA);
    if (zRes)
        return zRes;
    else
    {
        DdNode *bA0, *bA1, *bA01;
        DdNode *zC0, *zC1, *zC2, *zD0, *zD1, *zD2, *zUnion;
        DdNode *zRes0, *zRes1, *zRes2;

        int LevC = cuddIZ( dd, zC->index ) >> 1;
        int LevD = cuddIZ( dd, zD->index ) >> 1;
        int LevA = cuddI(  dd, Cudd_Regular(bA)->index );

        /* consider the easy case, when the area splits higher */
        if ( LevA < LevC && LevA < LevD ) 
        {
            /* determine whether the area is a complemented BDD */
            int fIsComp  = Cudd_IsComplement( bA );
            /* find the parts(cofactors) of the area */
            bA0 = Cudd_NotCond( Cudd_E( bA ), fIsComp );
            bA1 = Cudd_NotCond( Cudd_T( bA ), fIsComp );

            /* find the union of cofactors */
            bA01 = cuddBddAndRecur( dd, Cudd_Not(bA0), Cudd_Not(bA1) );
            if ( bA01 == NULL ) 
                return NULL;
            cuddRef( bA01 );
            bA01 = Cudd_Not(bA01);

            /* those cubes overlap, which overlap with the union of cofactors */
            zRes = extraZddNotContainedCubesOverArea( dd, zC, zD, bA01 );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDeref( dd, bA01 );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDeref( dd, bA01 );
            cuddDeref( zRes );
        }
        else
        { /* now, either zC or zD (or both) have the top-most variable */

            int TopVar = -1;
            /* cofactor covers and area */
            if ( LevC <= LevD )
            {
                TopVar = zC->index >> 1;
                extraDecomposeCover(dd, zC, &zC0, &zC1, &zC2);
            }
            else
                zC0 = zC1 = z0, zC2 = zC;

            if ( LevD <= LevC )
            {
                TopVar = zD->index >> 1;
                extraDecomposeCover(dd, zD, &zD0, &zD1, &zD2);
            }
            else
                zD0 = zD1 = z0, zD2 = zD;
            assert( TopVar != -1 );

            if ( Cudd_Regular(bA)->index == TopVar )
            {
                /* determine whether the area is a complemented BDD */
                int fIsComp  = Cudd_IsComplement( bA );
                bA0 = Cudd_NotCond( Cudd_E( bA ), fIsComp );
                bA1 = Cudd_NotCond( Cudd_T( bA ), fIsComp );

                /* find the union of cofactors */
                bA01 = cuddBddAndRecur( dd, Cudd_Not(bA0), Cudd_Not(bA1) );
                if ( bA01 == NULL ) 
                    return NULL;
                cuddRef( bA01 );
                bA01 = Cudd_Not(bA01);
            }
            else
            {
                bA0 = bA1 = bA01 = bA;
                cuddRef( bA01 );
            }

            /* cubes without this literal can be covered by cubes without this literal
               as well as by cubes with positive literal if bA0 = 0
               or by cubes with negative literals if bA1 = 0 */
            if ( bA0 == b0 )
            {
                zUnion = cuddZddUnion( dd, zD2, zD1 );
                if ( zUnion == NULL )
                {
                    Cudd_RecursiveDeref( dd, bA01 );
                    return NULL;
                }
                cuddRef( zUnion );
            }
            else if ( bA1 == b0 )
            {
                zUnion = cuddZddUnion( dd, zD2, zD0 );
                if ( zUnion == NULL )
                {
                    Cudd_RecursiveDeref( dd, bA01 );
                    return NULL;
                }
                cuddRef( zUnion );
            }
            else
            {
                zUnion = zD2;
                cuddRef( zUnion );
            }


            /* solve the problem for cubes without literal */
            zRes2 = extraZddNotContainedCubesOverArea( dd, zC2, zUnion, bA01 );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDeref( dd, bA01 );
                Cudd_RecursiveDerefZdd( dd, zUnion );
                return NULL;
            }
            cuddRef( zRes2 );
            Cudd_RecursiveDeref( dd, bA01 );
            Cudd_RecursiveDerefZdd( dd, zUnion );


            /* compute the union of those cubes in D 
               that can potentially contain cubes in C0 */
            zUnion = cuddZddUnion( dd, zD0, zD2 );
            if ( zUnion == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes2 );
                return NULL;
            }
            cuddRef( zUnion );

            /* solve the problem for cubes with negative literals */
            zRes0 = extraZddNotContainedCubesOverArea( dd, zC0, zUnion, bA0 );
            if ( zRes0 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zUnion );
                Cudd_RecursiveDerefZdd( dd, zRes2 );
                return NULL;
            }
            cuddRef( zRes0 );
            Cudd_RecursiveDerefZdd( dd, zUnion );


            /* compute the union of those cubes in D 
               that can potentially contain cubes in C1 */
            zUnion = cuddZddUnion( dd, zD1, zD2 );
            if ( zUnion == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes2 );
                return NULL;
            }
            cuddRef( zUnion );

            /* solve the problem for cubes with negative literals */
            zRes1 = extraZddNotContainedCubesOverArea( dd, zC1, zUnion, bA1 );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zUnion );
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes2 );
                return NULL;
            }
            cuddRef( zRes1 );
            Cudd_RecursiveDerefZdd( dd, zUnion );

            /* --------------- compose the result ------------------ */
            zRes = extraComposeCover( dd, zRes0, zRes1, zRes2, TopVar );
            if ( zRes == NULL ) return NULL;
        }           
        /* insert the result into cache and return */
        cuddCacheInsert(dd, DD_ZDD_BDD_SUBSET_TAG, zC, zD, bA, zRes);
        return zRes;
    }
} /* end of extraZddNotContainedCubesOverArea */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_zddSelectOneCube.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode * extraZddSelectOneCube( 
  DdManager * dd, 
  DdNode * zS )
// selects one cube from the ZDD zS
// returns z0 if and only if zS is an empty set of cubes
{
    DdNode * zRes;

    if ( zS == z0 )    return z0;
    if ( zS == z1 )    return z1;
    
    // check cache
    if ( zRes = cuddCacheLookup1Zdd( dd, extraZddSelectOneCube, zS ) )
        return zRes;
    else
    {
        DdNode * zS0, * zS1, * zS2, * zTemp; 
        int topVar = zS->index/2;

        extraDecomposeCover( dd, zS, &zS0, &zS1, &zS2 );

        if ( zS2 != z0 )
        {
            zRes = extraZddSelectOneCube( dd, zS2 );
            if ( zRes == NULL )
                return NULL;
        }
        else if ( zS0 != z0 )
        {
            zTemp = extraZddSelectOneCube( dd, zS0 );
            if ( zTemp == NULL )
                return NULL;
            cuddRef( zTemp );

            zRes = cuddZddGetNode( dd, 2*topVar+1, zTemp, dd->zero );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zTemp );
                return NULL;
            }
            cuddDeref( zTemp );
        }
        else if ( zS1 != z0 )
        {
            zTemp = extraZddSelectOneCube( dd, zS1 );
            if ( zTemp == NULL )
                return NULL;
            cuddRef( zTemp );

            zRes = cuddZddGetNode( dd, 2*topVar, zTemp, dd->zero );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zTemp );
                return NULL;
            }
            cuddDeref( zTemp );
        }
        else
            assert( 0 );

        // insert the result into cache
        cuddCacheInsert1( dd, extraZddSelectOneCube, zS, zRes );
        return zRes;
    }       
} /* end of extraZddSelectOneCube */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_zddSelectOneSubset.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode * extraZddSelectOneSubset( 
  DdManager * dd, 
  DdNode * zS )
// selects one subset from the ZDD zS
// returns z0 if and only if zS is an empty set of cubes
{
    DdNode * zRes;

    if ( zS == z0 )    return z0;
    if ( zS == z1 )    return z1;
    
    // check cache
    if ( zRes = cuddCacheLookup1Zdd( dd, extraZddSelectOneSubset, zS ) )
        return zRes;
    else
    {
        DdNode * zS0, * zS1, * zTemp; 

        zS0 = cuddE(zS);
        zS1 = cuddT(zS);

        if ( zS0 != z0 )
        {
            zRes = extraZddSelectOneSubset( dd, zS0 );
            if ( zRes == NULL )
                return NULL;
        }
        else // if ( zS0 == z0 )
        {
            assert( zS1 != z0 );
            zRes = extraZddSelectOneSubset( dd, zS1 );
            if ( zRes == NULL )
                return NULL;
            cuddRef( zRes );

            zRes = cuddZddGetNode( dd, zS->index, zTemp = zRes, z0 );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zTemp );
                return NULL;
            }
            cuddDeref( zTemp );
        }

        // insert the result into cache
        cuddCacheInsert1( dd, extraZddSelectOneSubset, zS, zRes );
        return zRes;
    }       
} /* end of extraZddSelectOneSubset */

/**Function********************************************************************

  Synopsis [Performs the recursive step of Extra_zddGetMostCoveredArea]

  Description [Computes the ADD, which gives for each minterm of the on-set of the
  function, the number of cubes covering this minterm. Returns the ADD 
  if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
extraZddGetMostCoveredArea(
  DdManager * dd,
  DdNode * zC)
{
    DdNode  *aRes;
    statLine(dd); 

    /* if there is no cover, there are no covered minterms */
    if ( zC == z0 )  return dd->zero;
    /* if the cover is the universe, all minterms are covered once */
    if ( zC == z1 )  return dd->one;

    /* check cache */
    aRes = cuddCacheLookup1(dd, extraZddGetMostCoveredArea, zC);
    if (aRes)
        return(aRes);
    else
    {
        DdNode * aRes0, * aRes1, * aRes2, * aTemp;
        DdNode * zC0,   * zC1,   * zC2;
        int TopBddVar = (zC->index >> 1);

        /* cofactor the cover */
        extraDecomposeCover(dd, zC, &zC0, &zC1, &zC2);

        /* compute adds for the three cofactors of the cover */
        aRes0 = extraZddGetMostCoveredArea(dd, zC0);
        if ( aRes0 == NULL )
            return NULL;
        cuddRef( aRes0 );

        aRes1 = extraZddGetMostCoveredArea(dd, zC1);
        if ( aRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, aRes0);
            return NULL;
        }
        cuddRef( aRes1 );

        aRes2 = extraZddGetMostCoveredArea(dd, zC2);
        if ( aRes2 == NULL )
        {
            Cudd_RecursiveDeref(dd, aRes0);
            Cudd_RecursiveDeref(dd, aRes1);
            return NULL;
        }
        cuddRef( aRes2 );

        /* compute  add(zC0)+add(zC2) */
        aRes0 = cuddAddApplyRecur( dd, Cudd_addPlus, aTemp = aRes0, aRes2 );
        if ( aRes0 == NULL )
        {
            Cudd_RecursiveDeref(dd, aTemp);
            Cudd_RecursiveDeref(dd, aRes1);
            Cudd_RecursiveDeref(dd, aRes2);
            return NULL;
        }
        cuddRef( aRes0 );
        Cudd_RecursiveDeref(dd, aTemp);

        /* compute  add(zC1)+add(zC2) */
        aRes1 = cuddAddApplyRecur( dd, Cudd_addPlus, aTemp = aRes1, aRes2 );
        if ( aRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, aRes0);
            Cudd_RecursiveDeref(dd, aTemp);
            Cudd_RecursiveDeref(dd, aRes2);
            return NULL;
        }
        cuddRef( aRes1 );
        Cudd_RecursiveDeref(dd, aTemp);
        Cudd_RecursiveDeref(dd, aRes2);

        /* only aRes0 and aRes1 are referenced at this point */

        /* consider the case when Res0 and Res1 are the same node */
        aRes = (aRes1 == aRes0) ? aRes1 : cuddUniqueInter(dd,TopBddVar,aRes1,aRes0);
        if (aRes == NULL) 
        {
            Cudd_RecursiveDeref(dd, aRes1);
            Cudd_RecursiveDeref(dd, aRes0);
            return(NULL);
        }
        cuddDeref(aRes1);
        cuddDeref(aRes0);

        /* insert the result into cache */
        cuddCacheInsert1(dd, extraZddGetMostCoveredArea, zC, aRes);
        return aRes;
    }
} /* end of extraZddGetMostCoveredArea */


/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/
