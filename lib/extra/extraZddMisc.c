/**CFile****************************************************************

  FileName    [extraZddMisc.c]

  PackageName [extra]

  Synopsis    [Various ZDD manipulating procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraZddMisc.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

const int s_LargeNum = 1000000;

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

// traversal structure for the enumeration of paths in the ZDD
// (can be used for both printing and ZDD remapping)
typedef struct
{
    FILE *fp;       /* the file to write graph */
    DdManager *dd;  /* the pointer to the DD manager */
    int Lev[2];     /* levels of the first and the second variables on the path */
    int nIter;      /* the counter of cubes traversed */
} userdata;

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
static int zddCountVars (DdManager *dd, DdNode* S);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Creates the combination composed of a single ZDD variable.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
DdNode * Extra_zddVariable( DdManager * dd, int iVar )
{
    DdNode * zRes;
    do {
        dd->reordered = 0;
        zRes = cuddZddGetNode( dd, iVar, z1, z0 ); 
    } while (dd->reordered == 1);
    return zRes;
}

/**Function********************************************************************

  Synopsis    [Creates ZDD of the combination containing given variables.]

  Description [Creates ZDD of the combination containing given variables.
               VarValues contains 1 for a variable that belongs to the 
               combination and 0 for a varible that does not belong. 
               nVars is number of ZDD variables in the array.]

  SideEffects [New ZDD variables are created if indices of the variables
               present in the combination are larger than the currently
               allocated number of ZDD variables.]

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddCombination( 
  DdManager *dd, 
  int* VarValues, 
  int nVars )
{
    DdNode  *res;
    do {
    dd->reordered = 0;
    res = extraZddCombination(dd, VarValues, nVars);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddCombination */


/**Function********************************************************************

  Synopsis    [Creates all possible combinations of given variables.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddUniverse( 
  DdManager * dd,    /* the manager */
  DdNode * VarSet)   /* the variables whose universe it to be built */
{
    DdNode  *res;
    do {
    dd->reordered = 0;
    res = extraZddUniverse(dd, VarSet);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddUniverse */


/**Function********************************************************************

  Synopsis    [Builds ZDD representing the set of fixed-size variable tuples.]

  Description [Creates ZDD of all combinations of variables in Support that
  is represented by a ZDD.]

  SideEffects [New ZDD variables are created if indices of the variables
               present in the combination are larger than the currently
               allocated number of ZDD variables.]

  SeeAlso     []

******************************************************************************/
DdNode* Extra_zddTuples( 
  DdManager * dd,   /* the DD manager */
  int K,            /* the number of variables in tuples */
  DdNode * zVarsN)   /* the set of all variables represented as a ZDD */
{
    DdNode  *zRes;
    int     autoDynZ;

    autoDynZ = dd->autoDynZ;
    dd->autoDynZ = 0;

    do {
        /* transform the numeric arguments (K) into a DdNode* argument;
         * this allows us to use the standard internal CUDD cache */
        DdNode *zVarSet = zVarsN, *zVarsK = zVarsN;
        int     nVars = 0, i;

        /* determine the number of variables in VarSet */
        while ( zVarSet != z1 )
        {
            nVars++;
            /* make sure that the VarSet is a cube */
            if ( cuddE( zVarSet ) != z0 )
                return NULL;
            zVarSet = cuddT( zVarSet );
        }
        /* make sure that the number of variables in VarSet is less or equal 
           that the number of variables that should be present in the tuples
        */
        if ( K > nVars )
            return NULL;

        /* the second argument in the recursive call stannds for <n>;
        /* reate the first argument, which stands for <k> 
         * as when we are talking about the tuple of <k> out of <n> */
        for ( i = 0; i < nVars-K; i++ )
            zVarsK = cuddT( zVarsK );

        dd->reordered = 0;
        zRes = extraZddTuples(dd, zVarsK, zVarsN );

    } while (dd->reordered == 1);
    dd->autoDynZ = autoDynZ;
    return zRes;

} /* end of Extra_zddTuples */


/**Function********************************************************************

  Synopsis    [Builds ZDD representing the set of fixed-size variable tuples.]

  Description [Creates ZDD of all combinations of variables in Support that
  is represented by a BDD.]

  SideEffects [New ZDD variables are created if indices of the variables
               present in the combination are larger than the currently
               allocated number of ZDD variables.]

  SeeAlso     []

******************************************************************************/
DdNode* Extra_zddTuplesFromBdd( 
  DdManager * dd,   /* the DD manager */
  int K,            /* the number of variables in tuples */
  DdNode * bVarsN)   /* the set of all variables represented as a BDD */
{
    DdNode  *zRes;
    int     autoDynZ;

    autoDynZ = dd->autoDynZ;
    dd->autoDynZ = 0;

    do {
        /* transform the numeric arguments (K) into a DdNode* argument;
         * this allows us to use the standard internal CUDD cache */
        DdNode *bVarSet = bVarsN, *bVarsK = bVarsN;
        int     nVars = 0, i;

        /* determine the number of variables in VarSet */
        while ( bVarSet != b1 )
        {
            nVars++;
            /* make sure that the VarSet is a cube */
            if ( cuddE( bVarSet ) != b0 )
                return NULL;
            bVarSet = cuddT( bVarSet );
        }
        /* make sure that the number of variables in VarSet is less or equal 
           that the number of variables that should be present in the tuples
        */
        if ( K > nVars )
            return NULL;

        /* the second argument in the recursive call stannds for <n>;
        /* reate the first argument, which stands for <k> 
         * as when we are talking about the tuple of <k> out of <n> */
        for ( i = 0; i < nVars-K; i++ )
            bVarsK = cuddT( bVarsK );

        dd->reordered = 0;
        zRes = extraZddTuplesFromBdd(dd, bVarsK, bVarsN );

    } while (dd->reordered == 1);
    dd->autoDynZ = autoDynZ;
    return zRes;

} /* end of Extra_zddTuplesFromBdd */


/**Function********************************************************************

  Synopsis    [Converts the set of singleton combinations into one combination.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode* Extra_zddSinglesToComb( 
  DdManager * dd,   /* the DD manager */
  DdNode * Singles) /* the set of singleton combinations */
{
    DdNode  *res;
    int     autoDynZ;

    autoDynZ = dd->autoDynZ;
    dd->autoDynZ = 0;

    do {
    dd->reordered = 0;
    res = extraZddSinglesToComb(dd, Singles);
    } while (dd->reordered == 1);
    dd->autoDynZ = autoDynZ;
    return(res);

} /* end of Extra_zddSinglesToComb */


/**Function********************************************************************

  Synopsis    [Returns all combinations containing the maximum number of elements.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddMaximal]

******************************************************************************/
DdNode* Extra_zddMaximum( 
  DdManager * dd,   /* the DD manager */
  DdNode * S,       /* the set of combinations whose maximum is sought */
  int * nVars)      /* if non-zero, stores the number of elements in max combinations */
{
    DdNode  *res;
    int NumVars;
    do {
        dd->reordered = 0;
        res = extraZddMaximum(dd, S, &NumVars);
    } while (dd->reordered == 1);

    /* count the number of elements in max combinations */
    if ( nVars )
        *nVars = NumVars;
    return(res);

} /* end of Extra_zddMaximum */


/**Function********************************************************************

  Synopsis    [Returns all combinations containing the minimum number of elements.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddMinimal]

******************************************************************************/
DdNode* Extra_zddMinimum( 
  DdManager * dd,   /* the DD manager */
  DdNode * S,       /* the set of combinations whose maximum is sought */
  int * nVars)      /* if non-zero, stores the number of elements in max combinations */
{
    DdNode  *res;
    int NumVars;
    do {
        dd->reordered = 0;
        res = extraZddMinimum(dd, S, &NumVars);
    } while (dd->reordered == 1);

    /* count the number of elements in max combinations */
    if ( nVars )
        *nVars = NumVars;
    return(res);

} /* end of Extra_zddMinimum */


/**Function********************************************************************

  Synopsis    [Generates a random set of combinations.]

  Description [Given a set of n elements, each of which is encoded using one
               ZDD variable, this function generates a random set of k subsets 
               (combinations of elements) with density d. Assumes that k and n
               are positive integers. Returns NULL if density is less than 0.0 
               or more than 1.0.]

  SideEffects [Allocates new ZDD variables if their current number is less than n.]

  SeeAlso     []

******************************************************************************/
DdNode* Extra_zddRandomSet( 
  DdManager * dd,   /* the DD manager */
  int n,            /* the number of elements */
  int k,            /* the number of combinations (subsets) */
  double d)         /* average density of elements in combinations */
{
    DdNode *Result, *TempComb, *Aux;
    int c, v, Limit, *VarValues;

    /* sanity check the parameters */
    if ( n <= 0 || k <= 0 || d < 0.0 || d > 1.0 )
        return NULL;

    /* allocate temporary storage for variable values */
    VarValues = ALLOC( int, n );
    if (VarValues == NULL) 
    {
        dd->errorCode = CUDD_MEMORY_OUT;
        return NULL;
    }

    /* start the new set */
    Result = dd->zero;
    Cudd_Ref( Result );

    /* seed random number generator */
    Cudd_Srandom( dd, time(NULL) );
//  Cudd_Srandom( 4 );
    /* determine the limit below which var belongs to the combination */
    Limit = (int)(d * 2147483561.0);

    /* add combinations one by one */
    for ( c = 0; c < k; c++ )
    {
        for ( v = 0; v < n; v++ )
            if ( Cudd_Random( dd ) <= Limit )
                VarValues[v] = 1;
            else
                VarValues[v] = 0;

        TempComb = Extra_zddCombination( dd, VarValues, n );
        Cudd_Ref( TempComb );

        /* make sure that this combination is not already in the set */
        if ( c )
        { /* at least one combination is already included */

            Aux = Cudd_zddDiff( dd, Result, TempComb );
            Cudd_Ref( Aux );
            if ( Aux != Result )
            {
                Cudd_RecursiveDerefZdd( dd, Aux );
                Cudd_RecursiveDerefZdd( dd, TempComb );
                c--;
                continue;
            }
            else 
            { /* Aux is the same node as Result */
                Cudd_Deref( Aux );
            }
        }

        Result = Cudd_zddUnion( dd, Aux = Result, TempComb );
        Cudd_Ref( Result );
        Cudd_RecursiveDerefZdd( dd, Aux );
        Cudd_RecursiveDerefZdd( dd, TempComb );
    }

    FREE( VarValues );
    Cudd_Deref( Result );
    return Result;

} /* end of Extra_zddRandomSet */

/**Function********************************************************************

  Synopsis    [Selects cubes from the cover that are completely contained in the area.]

  Description [This function is similar to Extra_zddSubSet(X,Y) which selects 
  those subsets of X that are completely contained in at least one subset of Y. 
  Extra_zddCoveredByArea() filters the cover of cubes and returns only those cubes 
  that are completely contained in the area. The cover is given as a ZDD, the 
  area is a BDD. Returns the reduced cube set on success; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Extra_zddSubSet]

******************************************************************************/
DdNode  *
Extra_zddCoveredByArea(
  DdManager * dd,
  DdNode * zC,
  DdNode * bA)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddCoveredByArea(dd, zC, bA);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddCoveredByArea */


/**Function********************************************************************

  Synopsis    [Selects cubes from the cover that are not completely covered by other cover.]

  Description [This function is equivalent to first converting the second cover 
  into BDD, calling Extra_zddCoveredByArea(), and then subtracting from the
  first cover the result of the latter operation. However, it is more efficient.
  Returns the reduced cube set on success; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Extra_zddSubSet]

******************************************************************************/
DdNode  *
Extra_zddNotCoveredByCover(
  DdManager * dd,
  DdNode * zC,
  DdNode * zD)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddNotCoveredByCover(dd, zC, zD);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddNotCoveredByCover */


/**Function********************************************************************

  Synopsis    [Selects cubes from the cover that are overlapping with the area.]

  Description [This function is similar to Extra_zddNotSubSet(X,Y) which selects 
  those subsets of X that are completely contained in at least one subset of Y. 
  Extra_zddOverlappingWithArea() filters the cover of cubes and returns only those 
  cubes that overlap with the area. The completely contained cubes are counted as
  overlapping. The cover is given as a ZDD, the area is a BDD. Returns the reduced 
  cube set on success; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Extra_zddSubSet]

******************************************************************************/
DdNode  *
Extra_zddOverlappingWithArea(
  DdManager * dd,
  DdNode * zC,
  DdNode * bA)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddOverlappingWithArea(dd, zC, bA);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddOverlappingWithArea */


/**Function********************************************************************

  Synopsis    [Converts the ZDD into the BDD.]

  Description [This function is a reimplementation of Cudd_MakeBddFromZddCover 
  for the case when BDD variable and ZDD variable orders are synchronized. 
  It is the user's responsibility to ensure the synchronization. This function is
  more efficient than Cudd_MakeBddFromZddCover because it does not require
  referencing cofactors of the cover and because it used cuddUniqueInterO() 
  instead of cuddUniqueInterIVO(). Returns the bdd on success; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_MakeBddFromZddCover]

******************************************************************************/
DdNode  *
Extra_zddConvertToBdd(
  DdManager * dd,
  DdNode * zC)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddConvertToBdd(dd, zC);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddConvertToBdd */

/**Function********************************************************************

  Synopsis    [Converts the ZDD of the positive unate function into the BDD.]

  Description []

  SideEffects [None]

  SeeAlso     [Cudd_MakeBddFromZddCover Extra_zddConvertToBdd]

******************************************************************************/
DdNode  *
Extra_zddConvertToBddUnate(
  DdManager * dd,
  DdNode * zC)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddConvertToBddUnate(dd, zC);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddConvertToBdd */

/**Function********************************************************************

  Synopsis    [Converts the ZDD for ESOP into the BDD.]

  Description [Returns the bdd on success; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_MakeBddFromZddCover Extra_zddConvertToBdd]

******************************************************************************/
DdNode  *
Extra_zddConvertEsopToBdd(
  DdManager * dd,
  DdNode * zC)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddConvertEsopToBdd(dd, zC);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddConvertEsopToBdd */


/**Function********************************************************************

  Synopsis    [Converts ZDD into BDD while at the same time adding another BDD to it.]

  Description [This function is equivalent to first calling Extra_zddConvertToBdd()
  and then Cudd_bddOr() but is more efficient. Returns the reduced cube set on 
  success; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Extra_zddConvertToBdd]

******************************************************************************/
DdNode  *
Extra_zddConvertToBddAndAdd(
  DdManager * dd,
  DdNode * zC,
  DdNode * bA)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddConvertToBddAndAdd(dd, zC, bA);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddConvertToBddAndAdd */


/**Function********************************************************************

  Synopsis    [Finds the area covered by only one cube from cover.]

  Description [This function computes the BDD of the area covered by only one cube
  from the give cover. Returns the reduced cube set on success; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
Extra_zddSingleCoveredArea(
  DdManager * dd,
  DdNode * zC)
{
    DdNode  *res;

    do {
    dd->reordered = 0;
    res = extraZddSingleCoveredArea(dd, zC);
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddSingleCoveredArea */

/**Function********************************************************************

  Synopsis    [Converts the BDD cube into the ZDD cube.]

  Description [Returns the pointer to the ZDD on success; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
Extra_zddConvertBddCubeIntoZddCube(
  DdManager * dd,
  DdNode * bCube)
{
    DdNode  *zCube;
    int *VarValues, i;

    /* allocate temporary storage for variable values */
    assert( dd->sizeZ == 2 * dd->size );
    VarValues = (int*) malloc( dd->sizeZ * sizeof(int) );
    if ( VarValues == NULL ) 
    {
        dd->errorCode = CUDD_MEMORY_OUT;
        return NULL;
    }
    /* clean the storage */
    for ( i = 0; i < dd->sizeZ; i++ )
        VarValues[i] = 0;
    /* get the variable values */
    while ( bCube != b1 )
    {
        assert( !Cudd_IsComplement(bCube) );
        if ( Cudd_E(bCube) == b0 ) /* positive literal */
            VarValues[2*bCube->index] = 1, bCube = Cudd_T(bCube);
        else if ( Cudd_T(bCube) == b0 ) /* negative literal */
            VarValues[2*bCube->index+1] = 1, bCube = Cudd_E(bCube);
        else
            assert(0);
        assert( bCube != b0 );
    }
    /* get the ZDD cube */
    zCube = Extra_zddCombination( dd, VarValues, dd->sizeZ );
    free(VarValues);
    return zCube;
} /* end of Extra_zddConvertBddCubeIntoZddCube */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_zddCombination().]

  Description [Generates in a bottom-up fashion ZDD for one combination 
               whose var values are given in the array VarValues. If necessary,
               creates new variables on the fly.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraZddCombination(
  DdManager* dd, 
  int* VarValues, 
  int nVars )
{
    int lev, index;
    DdNode *zRes, *zTemp;

    /* transform the combination from the array VarValues into a ZDD cube. */
    zRes = dd->one;
    cuddRef(zRes);

    /*  go through levels starting bottom-up and create nodes 
     *  if these variables are present in the comb
     */
    for (lev = nVars - 1; lev >= 0; lev--) 
    { 
        index = (lev >= dd->sizeZ) ? lev : dd->invpermZ[lev];
        if (VarValues[index] == 1) 
        {
            /* compose zRes with ZERO for the given ZDD variable */
            zRes = cuddZddGetNode( dd, index, zTemp = zRes, dd->zero );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zTemp );
                return NULL;
            }
            cuddRef( zRes );
            cuddDeref( zTemp );
        }
    }
    cuddDeref( zRes );
    return zRes;

} /* end of extraZddCombination */



/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_zddUniverse().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraZddUniverse(
  DdManager * dd, 
  DdNode * VarSet)
{
    DdNode *zRes, *zPart;
    statLine(dd); 

    if ( VarSet == dd->zero )
    {
        printf("\nextraZddUniverse(): Singles is not a ZDD!\n");
        return NULL;
    }
    if ( VarSet == dd->one )
        return dd->one;

    /* check cache */
    zRes = cuddCacheLookup1Zdd(dd, extraZddUniverse, VarSet);
    if (zRes)
        return(zRes);

    /* make sure that VarSet is a single combination */
    if ( cuddE( VarSet ) != dd->zero )
    {
        printf("\nextraZddUniverse(): VarSet is not a single combination!\n");
        return NULL;
    }

    /* solve the problem recursively */
    zPart = extraZddUniverse( dd, cuddT( VarSet ) );
    if ( zPart == NULL ) 
        return NULL;
    cuddRef( zPart );

    /* create new node with this variable */
    zRes = cuddZddGetNode( dd, VarSet->index, zPart, zPart );
    if ( zRes == NULL ) 
    {
        Cudd_RecursiveDerefZdd( dd, zPart );
        return NULL;
    }
    cuddDeref( zPart );

    cuddCacheInsert1(dd, extraZddUniverse, VarSet, zRes);
    return zRes;

} /* end of extraZddUniverse */


/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_zddTuples().]

  Description [Generates in a bottom-up fashion ZDD for all combinations
               composed of k variables out of variables belonging to Support.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode* extraZddTuples( 
  DdManager * dd,   /* the DD manager */
  DdNode * zVarsK,   /* the number of variables in tuples */
  DdNode * zVarsN)   /* the set of all variables */
{
    DdNode *zRes, *zRes0, *zRes1;
    statLine(dd); 

    /* terminal cases */
/*  if ( k < 0 || k > n )
 *      return dd->zero;
 *  if ( n == 0 )
 *      return dd->one; 
 */
    if ( cuddIZ( dd, zVarsK->index ) < cuddIZ( dd, zVarsN->index ) )
        return z0;
    if ( zVarsN == z1 )
        return z1;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddTuples, zVarsK, zVarsN);
    if (zRes)
        return(zRes);

    /* ZDD in which this variable is 0 */
/*  zRes0 = extraZddTuples( dd, k,     n-1 ); */
    zRes0 = extraZddTuples( dd, zVarsK, cuddT(zVarsN) );
    if ( zRes0 == NULL ) 
        return NULL;
    cuddRef( zRes0 );

    /* ZDD in which this variable is 1 */
/*  zRes1 = extraZddTuples( dd, k-1,          n-1 ); */
    if ( zVarsK == z1 )
    {
        zRes1 = z0;
        cuddRef( zRes1 );
    }
    else
    {
        zRes1 = extraZddTuples( dd, cuddT(zVarsK), cuddT(zVarsN) );
        if ( zRes1 == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            return NULL;
        }
        cuddRef( zRes1 );
    }

    /* compose Res0 and Res1 with the given ZDD variable */
    zRes = cuddZddGetNode( dd, zVarsN->index, zRes1, zRes0 );
    if ( zRes == NULL ) 
    {
        Cudd_RecursiveDerefZdd( dd, zRes0 );
        Cudd_RecursiveDerefZdd( dd, zRes1 );
        return NULL;
    }
    cuddDeref( zRes0 );
    cuddDeref( zRes1 );

    /* insert the result into cache */
    cuddCacheInsert2(dd, extraZddTuples, zVarsK, zVarsN, zRes);
    return zRes;

} /* end of extraZddTuples */


/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_zddTupleFromBdd().]

  Description [Generates in a bottom-up fashion ZDD for all combinations
               composed of k variables out of variables belonging to Support.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode* extraZddTuplesFromBdd( 
  DdManager * dd,   /* the DD manager */
  DdNode * bVarsK,   /* the number of variables in tuples */
  DdNode * bVarsN)   /* the set of all variables */
{
    DdNode *zRes, *zRes0, *zRes1;
    statLine(dd); 

    /* terminal cases */
/*  if ( k < 0 || k > n )
 *      return dd->zero;
 *  if ( n == 0 )
 *      return dd->one; 
 */
    if ( cuddI( dd, bVarsK->index ) < cuddI( dd, bVarsN->index ) )
        return z0;
    if ( bVarsN == b1 )
        return z1;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddTuplesFromBdd, bVarsK, bVarsN);
    if (zRes)
        return(zRes);

    /* ZDD in which this variable is 0 */
/*  zRes0 = extraZddTuplesFromBdd( dd, k,     n-1 ); */
    zRes0 = extraZddTuplesFromBdd( dd, bVarsK, cuddT(bVarsN) );
    if ( zRes0 == NULL ) 
        return NULL;
    cuddRef( zRes0 );

    /* ZDD in which this variable is 1 */
/*  zRes1 = extraZddTuplesFromBdd( dd, k-1,          n-1 ); */
    if ( bVarsK == b1 )
    {
        zRes1 = z0;
        cuddRef( zRes1 );
    }
    else
    {
        zRes1 = extraZddTuplesFromBdd( dd, cuddT(bVarsK), cuddT(bVarsN) );
        if ( zRes1 == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            return NULL;
        }
        cuddRef( zRes1 );
    }

    /* compose Res0 and Res1 with the given ZDD variable */
    zRes = cuddZddGetNode( dd, 2*bVarsN->index, zRes1, zRes0 );
    if ( zRes == NULL ) 
    {
        Cudd_RecursiveDerefZdd( dd, zRes0 );
        Cudd_RecursiveDerefZdd( dd, zRes1 );
        return NULL;
    }
    cuddDeref( zRes0 );
    cuddDeref( zRes1 );

    /* insert the result into cache */
    cuddCacheInsert2(dd, extraZddTuplesFromBdd, bVarsK, bVarsN, zRes);
    return zRes;

} /* end of extraZddTuplesFromBdd */

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_zddSinglesToComb().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraZddSinglesToComb(
  DdManager * dd,   /* the DD manager */
  DdNode * Singles) /* the set of singleton combinations */
{
    DdNode *zRes, *zPart;
    statLine(dd); 

    if ( Singles == dd->one )
    {
        printf("\nextraZddSinglesToComb(): Singles is not a ZDD!\n");
        return NULL;
    }
    if ( Singles == dd->zero )
        return dd->one;

    /* check cache */
    zRes = cuddCacheLookup1Zdd(dd, extraZddSinglesToComb, Singles);
    if (zRes)
        return(zRes);

    /* make sure that this is the set of singletons */
    if ( cuddT( Singles ) != dd->one )
    {
        printf("\nextraZddSinglesToComb(): Singles is not a set of singletons!\n");
        return NULL;
    }

    /* solve the problem recursively */
    zPart = extraZddSinglesToComb( dd, cuddE( Singles ) );
    if ( zPart == NULL )
        return NULL;
    cuddRef( zPart );

    /* create new node with this variable */
    /* compose Res0 and Res1 with the given ZDD variable */
    zRes = cuddZddGetNode( dd, Singles->index, zPart, dd->zero );
    if ( zRes == NULL ) 
    {
        Cudd_RecursiveDerefZdd( dd, zPart );
        return NULL;
    }
    cuddDeref( zPart );

    cuddCacheInsert1(dd, extraZddSinglesToComb, Singles, zRes);
    return zRes;

} /* end of extraZddSinglesToComb */


/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_zddMaximum().]

  Description [Generates in a bottom-up fashion ZDD for all combinations
  that have maximum cardinality in the cover.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraZddMaximum(
  DdManager * dd,   /* the DD manager */
  DdNode * S,       /* the set of combinations */
  int * nVars)      /* the pointer where cardinality goes */ 
{
    DdNode *zRes;
    DdNode*(*cacheOp)(DdManager*,DdNode*);
    statLine(dd); 

    /* terminal cases */
    if ( S == dd->zero || S == dd->one )
    {
        *nVars = 0;
        return S;
    }

    /* check cache */
    cacheOp = (DdNode*(*)(DdManager*,DdNode*))extraZddMaximum;

    zRes = cuddCacheLookup1Zdd(dd, cacheOp, S);
    if (zRes)
    {
        *nVars = zddCountVars( dd, zRes );
        return(zRes);
    }
    else
    {
        DdNode *zSet0, *zSet1;
        int     Card0,  Card1;

        /* solve the else problem recursively */
        zSet0 = extraZddMaximum( dd, cuddE( S ), &Card0 );
        if ( zSet0 == NULL )
            return NULL;
        cuddRef( zSet0 );

        /* solve the then problem recursively */
        zSet1 = extraZddMaximum( dd, cuddT( S ), &Card1 );
        if ( zSet1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zSet0 );
            return NULL;
        }
        cuddRef( zSet1 );

        /* compare the solutions */
        if ( Card0 == Card1 + 1 ) 
        {  /* both sets are good */

            /* create new node with this variable */
            zRes = cuddZddGetNode( dd, S->index, zSet1, zSet0 );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zSet0 );
                Cudd_RecursiveDerefZdd( dd, zSet1 );
                return NULL;
            }
            cuddDeref( zSet0 );
            cuddDeref( zSet1 );

            *nVars = Card0;
        }
        else if ( Card0 > Card1 + 1 )
        {  /* take only zSet0 */

            Cudd_RecursiveDerefZdd( dd, zSet1 );
            zRes = zSet0;
            cuddDeref( zRes );

            *nVars = Card0;
        }
        else /* if ( Card0 < Card1 + 1 ) */
        {  /* take only zSet1 */

            Cudd_RecursiveDerefZdd( dd, zSet0 );

            /* create new node with this variable and empty else child */
            zRes = cuddZddGetNode( dd, S->index, zSet1, dd->zero );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zSet1 );
                return NULL;
            }
            cuddDeref( zSet1 );

            *nVars = Card1 + 1;
        }

        cuddCacheInsert1(dd, cacheOp, S, zRes);
        return zRes;
    }
} /* end of extraZddMaximum */


/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_zddMinimum().]

  Description [Generates in a bottom-up fashion ZDD for all combinations
  that have minimum cardinality in the cover.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraZddMinimum(
  DdManager * dd,   /* the DD manager */
  DdNode * S,       /* the set of combinations */
  int * nVars)      /* the pointer where cardinality goes */ 
{
    DdNode *zRes;
    DdNode*(*cacheOp)(DdManager*,DdNode*);
    statLine(dd); 

    /* terminal cases */
    if ( S == dd->zero )
    {
        *nVars = s_LargeNum;
        return S;
    }
    if ( S == dd->one )
    {
        *nVars = 0;
        return S;
    }

    /* check cache */
    cacheOp = (DdNode*(*)(DdManager*,DdNode*))extraZddMinimum;

    zRes = cuddCacheLookup1Zdd(dd, cacheOp, S);
    if (zRes)
    {
        *nVars = zddCountVars( dd, zRes );
        return(zRes);
    }
    else
    {
        DdNode *zSet0, *zSet1;
        int     Card0,  Card1;

        /* solve the else problem recursively */
        zSet0 = extraZddMinimum( dd, cuddE( S ), &Card0 );
        if ( zSet0 == NULL )
            return NULL;
        cuddRef( zSet0 );

        /* solve the then problem recursively */
        zSet1 = extraZddMinimum( dd, cuddT( S ), &Card1 );
        if ( zSet1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zSet0 );
            return NULL;
        }
        cuddRef( zSet1 );

        /* compare the solutions */
        if ( Card0 == Card1 + 1 ) 
        {  /* both sets are good */

            /* create new node with this variable */
            zRes = cuddZddGetNode( dd, S->index, zSet1, zSet0 );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zSet0 );
                Cudd_RecursiveDerefZdd( dd, zSet1 );
                return NULL;
            }
            cuddDeref( zSet0 );
            cuddDeref( zSet1 );

            *nVars = Card0;
        }
        else if ( Card0 < Card1 + 1 )
        {  /* take only zSet0 */

            Cudd_RecursiveDerefZdd( dd, zSet1 );
            zRes = zSet0;
            cuddDeref( zRes );

            *nVars = Card0;
        }
        else /* if ( Card0 > Card1 + 1 ) */
        {  /* take only zSet1 */

            Cudd_RecursiveDerefZdd( dd, zSet0 );

            /* create new node with this variable and empty else child */
            zRes = cuddZddGetNode( dd, S->index, zSet1, dd->zero );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zSet1 );
                return NULL;
            }
            cuddDeref( zSet1 );

            *nVars = Card1 + 1;
        }

        cuddCacheInsert1(dd, cacheOp, S, zRes);
        return zRes;
    }
} /* end of extraZddMinimum */


/**Function********************************************************************

  Synopsis [Performs the recursive step of Extra_zddCoveredByArea.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
extraZddCoveredByArea(
  DdManager * dd,
  DdNode * zC,   /* the ZDD for the cover */
  DdNode * bA)   /* the BDD for the area */
{
    DdNode  *zRes;
    statLine(dd); 

    /* if there is no area, there are no contained cubes */
    if ( bA == b0 )  return z0;
    /* if the area is the total boolean space, the cover is contained completely */
    if ( bA == b1 )  return zC;
    /* if there is nothing in the cover, nothing is contained */
    if ( zC == z0 )  return z0;
    /* if the cover is one large cube (and the area is less than the total boolean space), 
       nothing is contained */
    if ( zC == z1 )  return z0;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddCoveredByArea, zC, bA);
    if (zRes)
        return zRes;
    else
    {
        DdNode *bA0, *bA1, *bA01;

        /* find the level in terms of the original variable levels, not ZDD variable levels */
        int levCover = dd->permZ[zC->index] >> 1;
        int levArea  = dd->perm[Cudd_Regular(bA)->index];
        /* determine whether the area is a complemented BDD */
        int fIsComp  = Cudd_IsComplement( bA );

        if ( levCover > levArea )
        {
            /* find the parts(cofactors) of the area */
            bA0 = Cudd_NotCond( Cudd_E( bA ), fIsComp );
            bA1 = Cudd_NotCond( Cudd_T( bA ), fIsComp );

            /* find the intersection of cofactors */
            bA01 = cuddBddAndRecur( dd, bA0, bA1 );
            if ( bA01 == NULL ) return NULL;
            cuddRef( bA01 );

            /* only those cubes are contained which are contained in the intersection */
            zRes = extraZddCoveredByArea( dd, zC, bA01 );
            if ( zRes == NULL ) return NULL;
            cuddRef( zRes );
            Cudd_RecursiveDeref( dd, bA01 );
            cuddDeref( zRes );
        }
        else /* if ( levCover <= levArea ) */
        {
            DdNode *zRes0, *zRes1, *zRes2, *zTemp;
            int TopZddVar;

            /* decompose the cover */
            DdNode *zC0, *zC1, *zC2;
            extraDecomposeCover(dd, zC, &zC0, &zC1, &zC2);

            if ( levCover == levArea )
            {
                /* find the parts(cofactors) of the area */
                bA0 = Cudd_NotCond( Cudd_E( bA ), fIsComp );
                bA1 = Cudd_NotCond( Cudd_T( bA ), fIsComp );

                /* find the intersection of cofactors */
                bA01 = cuddBddAndRecur( dd, bA0, bA1 );
                if ( bA01 == NULL ) return NULL;
                cuddRef( bA01 );

            }
            else /* if ( levCover < levArea ) */
            {
                /* assign the cofactors and their intersection */
                bA0 = bA1 = bA01 = bA;
                /* reference the intersection for uniformity with the above case */
                cuddRef( bA01 );
            }

            /* solve subproblems */

            /* cover without literal can only be contained in the intersection */
            zRes2 = extraZddCoveredByArea( dd, zC2, bA01 );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDeref( dd, bA01 );
                return NULL;
            }
            cuddRef( zRes2 );
            Cudd_RecursiveDeref( dd, bA01 );

            /* cover with negative literal can only be contained in bA0 */
            zRes0 = extraZddCoveredByArea( dd, zC0, bA0 );
            if ( zRes0 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes2);
                return NULL;
            }
            cuddRef( zRes0 );

            /* cover with positive literal can only be contained in bA1 */
            zRes1 = extraZddCoveredByArea( dd, zC1, bA1 );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes2);
                return NULL;
            }
            cuddRef( zRes1 );

            /* --------------- compose the result ------------------ */
            /* the index of the positive ZDD variable in zC */
            TopZddVar = (zC->index >> 1) << 1;

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
        }

        /* insert the result into cache and return */
        cuddCacheInsert2(dd, extraZddCoveredByArea, zC, bA, zRes);
        return zRes;
    }
} /* end of extraZddCoveredByArea */



/**Function********************************************************************

  Synopsis [Performs the recursive step of Cudd_zddNotCoveredByCover.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
extraZddNotCoveredByCover(
  DdManager * dd,
  DdNode * zC,   /* the ZDD for the cover */
  DdNode * zD)   /* the ZDD for the second cover */
{
    DdNode  *zRes;
    statLine(dd); 

    /* if the second cover is empty, the first cover is not covered */
    if ( zD == z0 )  return zC;
    /* if the second cover is full, not a part of the first cover is not covered */
    if ( zD == z1 )  return z0;
    /* if there is nothing in the cover, nothing is not covered */
    if ( zC == z0 )  return z0;
    /* if the first cover is full (and the second cover is not empty), 
       the universe is partially covered, so the result is empty */
    if ( zC == z1 )  return z1;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddNotCoveredByCover, zC, zD);
    if (zRes)
        return zRes;
    else
    {
        /* find the level in terms of the original variable levels, not ZDD variable levels */
        int levC = dd->permZ[zC->index] >> 1;
        int levD = dd->permZ[zD->index] >> 1;

        if ( levC > levD )
        {
            DdNode *zD0, *zD1, *zD2;
            extraDecomposeCover(dd, zD, &zD0, &zD1, &zD2);

            /* C cannot be covered by zD0 and zD1 - only zD2 left to consider */
            zRes = extraZddNotCoveredByCover( dd, zC, zD2 );
            if ( zRes == NULL ) return NULL;
        }
        else /* if ( levC <= levD ) */
        {
            DdNode *zRes0, *zRes1, *zRes2;
            DdNode *zC0, *zC1, *zC2;
            DdNode *zD0, *zD1, *zD2, *zTemp;
            int TopZddVar;

            /* decompose covers */
            extraDecomposeCover(dd, zC, &zC0, &zC1, &zC2);
            if ( levC == levD )
                extraDecomposeCover(dd, zD, &zD0, &zD1, &zD2);
            else /* if ( levC < levD ) */
                zD0 = zD1 = z0, zD2 = zD;

            /* solve subproblems */

            /* cover with negative literal can only be contained in zD0 + zD2 */
            zTemp = cuddZddUnion( dd, zD0, zD2 );
            if ( zTemp == NULL )
                return NULL;
            cuddRef( zTemp );

            zRes0 = extraZddNotCoveredByCover( dd, zC0, zTemp );
            if ( zRes0 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zTemp);
                return NULL;
            }
            cuddRef( zRes0 );
            Cudd_RecursiveDerefZdd(dd, zTemp);

            /* cover with positive literal can only be contained in zD1 + zD2 */
            zTemp = cuddZddUnion( dd, zD1, zD2 );
            if ( zTemp == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes0);
                return NULL;
            }
            cuddRef( zTemp );

            zRes1 = extraZddNotCoveredByCover( dd, zC1, zTemp );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zTemp);
                Cudd_RecursiveDerefZdd(dd, zRes0);
                return NULL;
            }
            cuddRef( zRes1 );
            Cudd_RecursiveDerefZdd(dd, zTemp);


            /* cover without literal can only be contained in zD2 */
            zRes2 = extraZddNotCoveredByCover( dd, zC2, zD2 );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
            cuddRef( zRes2 );

            /* --------------- compose the result ------------------ */
            /* the index of the positive ZDD variable in zC */
            TopZddVar = (zC->index >> 1) << 1;

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
        }

        /* insert the result into cache and return */
        cuddCacheInsert2(dd, extraZddNotCoveredByCover, zC, zD, zRes);
        return zRes;
    }
} /* end of extraZddNotCoveredByCover */



/**Function********************************************************************

  Synopsis [Performs the recursive step of Cudd_zddOverlappingWithArea.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
extraZddOverlappingWithArea(
  DdManager * dd,
  DdNode * zC,   /* the ZDD for the cover */
  DdNode * bA)   /* the BDD for the area */
{
    DdNode  *zRes;
    statLine(dd); 

    /* if there is no area, there are no overlapping cubes */
    if ( bA == b0 )  return z0;
    /* if the area is the total boolean space, all cubes of the cover overlap with area */
    if ( bA == b1 )  return zC;
    /* if there is nothing in the cover, nothing to overlap */
    if ( zC == z0 )  return z0;
    /* if the cover is the universe (and the area is something), the universe overlaps */
    if ( zC == z1 )  return zC;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddOverlappingWithArea, zC, bA);
    if (zRes)
        return zRes;
    else
    {
        DdNode *bA0, *bA1, *bA01;

        /* find the level in terms of the original variable levels, not ZDD variable levels */
        int levCover = dd->permZ[zC->index] >> 1;
        int levArea  = dd->perm[Cudd_Regular(bA)->index];
        /* determine whether the area is a complemented BDD */
        int fIsComp  = Cudd_IsComplement( bA );

        if ( levCover > levArea )
        {
            /* find the parts(cofactors) of the area */
            bA0 = Cudd_NotCond( Cudd_E( bA ), fIsComp );
            bA1 = Cudd_NotCond( Cudd_T( bA ), fIsComp );

            /* find the union of cofactors */
            bA01 = cuddBddAndRecur( dd, Cudd_Not(bA0), Cudd_Not(bA1) );
            if ( bA01 == NULL ) return NULL;
            cuddRef( bA01 );
            bA01 = Cudd_Not(bA01);

            /* those cubes overlap, which overlap with the union of cofactors */
            zRes = extraZddOverlappingWithArea( dd, zC, bA01 );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDeref( dd, bA01 );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDeref( dd, bA01 );
            cuddDeref( zRes );
        }
        else /* if ( levCover <= levArea ) */
        {
            DdNode *zRes0, *zRes1, *zRes2, *zTemp;
            int TopZddVar;

            /* decompose the cover */
            DdNode *zC0, *zC1, *zC2;
            extraDecomposeCover(dd, zC, &zC0, &zC1, &zC2);

            if ( levCover == levArea )
            {
                /* find the parts(cofactors) of the area */
                bA0 = Cudd_NotCond( Cudd_E( bA ), fIsComp );
                bA1 = Cudd_NotCond( Cudd_T( bA ), fIsComp );

                /* find the union of cofactors */
                bA01 = cuddBddAndRecur( dd, Cudd_Not(bA0), Cudd_Not(bA1) );
                if ( bA01 == NULL ) return NULL;
                cuddRef( bA01 );
                bA01 = Cudd_Not(bA01);
            }
            else /* if ( levCover < levArea ) */
            {
                /* assign the cofactors and their union */
                bA0 = bA1 = bA01 = bA;
                /* reference the union for uniformity with the above case */
                cuddRef( bA01 );
            }

            /* solve subproblems */

            /* cover without literal overlaps with the union */
            zRes2 = extraZddOverlappingWithArea( dd, zC2, bA01 );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDeref( dd, bA01 );
                return NULL;
            }
            cuddRef( zRes2 );
            Cudd_RecursiveDeref( dd, bA01 );

            /* cover with negative literal overlaps with bA0 */
            zRes0 = extraZddOverlappingWithArea( dd, zC0, bA0 );
            if ( zRes0 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes2);
                return NULL;
            }
            cuddRef( zRes0 );

            /* cover with positive literal overlaps with bA1 */
            zRes1 = extraZddOverlappingWithArea( dd, zC1, bA1 );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes2);
                return NULL;
            }
            cuddRef( zRes1 );

            /* --------------- compose the result ------------------ */
            /* the index of the positive ZDD variable in zC */
            TopZddVar = (zC->index >> 1) << 1;

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
        }

        /* insert the result into cache and return */
        cuddCacheInsert2(dd, extraZddOverlappingWithArea, zC, bA, zRes);
        return zRes;
    }
} /* end of extraZddOverlappingWithArea */


/**Function********************************************************************

  Synopsis [Performs the recursive step of Cudd_zddConvertToBdd.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
extraZddConvertToBdd(
  DdManager * dd,
  DdNode * zC)
{
    DdNode  *bRes;
    statLine(dd); 

    /* if there is no cover, there is no BDD */
    if ( zC == z0 )  return b0;
    /* if the cover is the universe, the BDD is constant 1 */
    if ( zC == z1 )  return b1;

    /* check cache */
    bRes = cuddCacheLookup1(dd, extraZddConvertToBdd, zC);
    if (bRes)
        return(bRes);
    else
    {
        DdNode * bRes0, * bRes1, * bRes2, * bTemp;
        DdNode * zC0, * zC1, * zC2;
        int TopBddVar = (zC->index >> 1);

        /* cofactor the cover */
        extraDecomposeCover(dd, zC, &zC0, &zC1, &zC2);

        /* compute bdds for the three cofactors of the cover */
        bRes0 = extraZddConvertToBdd(dd, zC0);
        if ( bRes0 == NULL )
            return NULL;
        cuddRef( bRes0 );

        bRes1 = extraZddConvertToBdd(dd, zC1);
        if ( bRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            return NULL;
        }
        cuddRef( bRes1 );

        bRes2 = extraZddConvertToBdd(dd, zC2);
        if ( bRes2 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            Cudd_RecursiveDeref(dd, bRes1);
            return NULL;
        }
        cuddRef( bRes2 );

        /* compute  bdd(zC0)+bdd(zC2) */
        bTemp = bRes0;
        bRes0 = cuddBddAndRecur(dd, Cudd_Not(bRes0), Cudd_Not(bRes2));
        if ( bRes0 == NULL )
        {
            Cudd_RecursiveDeref(dd, bTemp);
            Cudd_RecursiveDeref(dd, bRes1);
            Cudd_RecursiveDeref(dd, bRes2);
            return NULL;
        }
        cuddRef( bRes0 );
        bRes0 = Cudd_Not(bRes0);
        Cudd_RecursiveDeref(dd, bTemp);

        /* compute  bdd(zC1)+bdd(zC2) */
        bTemp = bRes1;
        bRes1 = cuddBddAndRecur(dd, Cudd_Not(bRes1), Cudd_Not(bRes2));
        if ( bRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            Cudd_RecursiveDeref(dd, bTemp);
            Cudd_RecursiveDeref(dd, bRes2);
            return NULL;
        }
        cuddRef( bRes1 );
        bRes1 = Cudd_Not(bRes1);
        Cudd_RecursiveDeref(dd, bTemp);
        Cudd_RecursiveDeref(dd, bRes2);
        /* only bRes0 and bRes1 are referenced at this point */

        /* consider the case when Res0 and Res1 are the same node */
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        /* consider the case when Res1 is complemented */
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, TopBddVar, Cudd_Not(bRes1), Cudd_Not(bRes0));
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
            bRes = cuddUniqueInter( dd, TopBddVar, bRes1, bRes0 );
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
        cuddCacheInsert1(dd, extraZddConvertToBdd, zC, bRes);
        return bRes;
    }
} /* end of extraZddConvertToBdd */


/**Function********************************************************************

  Synopsis [Performs the recursive step of Extra_zddConvertToBddUnate.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
extraZddConvertToBddUnate(
  DdManager * dd,
  DdNode * zC)
{
    DdNode  *bRes;
    statLine(dd); 

    /* if there is no cover, there is no BDD */
    if ( zC == z0 )  return b0;
    /* if the cover is the universe, the BDD is constant 1 */
    if ( zC == z1 )  return b1;

    /* check cache */
    bRes = cuddCacheLookup1(dd, extraZddConvertToBddUnate, zC);
    if (bRes)
        return(bRes);
    else
    {
        DdNode * bRes0, * bRes1, * bTemp;
//      DdNode * zUnion;
/*
        bRes0 = extraZddConvertToBddUnate(dd, cuddE(zC));
        if ( bRes0 == NULL )
            return NULL;
        cuddRef( bRes0 );

        zUnion = cuddZddUnion( dd, cuddE(zC), cuddT(zC) );
        if ( zUnion == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            return NULL;
        }
        cuddRef( zUnion );

        bRes1 = extraZddConvertToBddUnate(dd, zUnion);
        if ( bRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            Cudd_RecursiveDerefZdd(dd, zUnion);
            return NULL;
        }
        cuddRef( bRes1 );
        Cudd_RecursiveDerefZdd(dd, zUnion);
*/



        bRes0 = extraZddConvertToBddUnate(dd, cuddE(zC));
        if ( bRes0 == NULL )
            return NULL;
        cuddRef( bRes0 );

        bRes1 = extraZddConvertToBddUnate(dd, cuddT(zC));
        if ( bRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            return NULL;
        }
        cuddRef( bRes1 );

        bRes1 = cuddBddAndRecur(dd, bTemp = Cudd_Not(bRes1), Cudd_Not(bRes0));
        if ( bRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            Cudd_RecursiveDeref(dd, bRes1);
            return NULL;
        }
        cuddRef( bRes1 );
        Cudd_RecursiveDeref(dd, bTemp);
        bRes1 = Cudd_Not(bRes1);


        /* consider the case when Res0 and Res1 are the same node */
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        /* consider the case when Res1 is complemented */
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, zC->index/2, Cudd_Not(bRes1), Cudd_Not(bRes0));
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
            bRes = cuddUniqueInter( dd, zC->index/2, bRes1, bRes0 );
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
        cuddCacheInsert1(dd, extraZddConvertToBddUnate, zC, bRes);
        return bRes;
    }
} /* end of extraZddConvertToBddUnate */



/**Function********************************************************************

  Synopsis [Performs the recursive step of Cudd_zddConvertEsopToBdd.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
extraZddConvertEsopToBdd(
  DdManager * dd,
  DdNode * zC)
{
    DdNode  *bRes;
    statLine(dd); 

    /* if there is no cover, there is no BDD */
    if ( zC == z0 )  return b0;
    /* if the cover is the universe, the BDD is constant 1 */
    if ( zC == z1 )  return b1;

    /* check cache */
    bRes = cuddCacheLookup1(dd, extraZddConvertEsopToBdd, zC);
    if (bRes)
        return(bRes);
    else
    {
        DdNode * bRes0, * bRes1, * bRes2, * bTemp;
        DdNode * zC0, * zC1, * zC2;
        int TopBddVar = (zC->index >> 1);

        /* cofactor the cover */
        extraDecomposeCover(dd, zC, &zC0, &zC1, &zC2);

        /* compute bdds for the three cofactors of the cover */
        bRes0 = extraZddConvertEsopToBdd(dd, zC0);
        if ( bRes0 == NULL )
            return NULL;
        cuddRef( bRes0 );

        bRes1 = extraZddConvertEsopToBdd(dd, zC1);
        if ( bRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            return NULL;
        }
        cuddRef( bRes1 );

        bRes2 = extraZddConvertEsopToBdd(dd, zC2);
        if ( bRes2 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            Cudd_RecursiveDeref(dd, bRes1);
            return NULL;
        }
        cuddRef( bRes2 );

        /* compute  bdd(zC0) (+) bdd(zC2) */
        bRes0 = cuddBddXorRecur(dd, bTemp = bRes0, bRes2);
        if ( bRes0 == NULL )
        {
            Cudd_RecursiveDeref(dd, bTemp);
            Cudd_RecursiveDeref(dd, bRes1);
            Cudd_RecursiveDeref(dd, bRes2);
            return NULL;
        }
        cuddRef( bRes0 );
        Cudd_RecursiveDeref(dd, bTemp);

        /* compute  bdd(zC1) (+) bdd(zC2) */
        bRes1 = cuddBddXorRecur(dd, bTemp = bRes1, bRes2);
        if ( bRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            Cudd_RecursiveDeref(dd, bTemp);
            Cudd_RecursiveDeref(dd, bRes2);
            return NULL;
        }
        cuddRef( bRes1 );
        Cudd_RecursiveDeref(dd, bTemp);
        Cudd_RecursiveDeref(dd, bRes2);
        /* only bRes0 and bRes1 are referenced at this point */

        /* consider the case when Res0 and Res1 are the same node */
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        /* consider the case when Res1 is complemented */
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, TopBddVar, Cudd_Not(bRes1), Cudd_Not(bRes0));
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
            bRes = cuddUniqueInter( dd, TopBddVar, bRes1, bRes0 );
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
        cuddCacheInsert1(dd, extraZddConvertEsopToBdd, zC, bRes);
        return bRes;
    }
} /* end of extraZddConvertEsopToBdd */



/**Function********************************************************************

  Synopsis [Performs the recursive step of Cudd_zddConvertToBddAndAdd.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
extraZddConvertToBddAndAdd(
  DdManager * dd,
  DdNode * zC,     /* the cover */
  DdNode * bA)     /* the area */
{
    DdNode  *bRes;
    statLine(dd); 

    /* if there is no cover, return the BDD */
    if ( zC == z0 )  return bA;
    /* if the cover is the universe, the sum is constant 1 */
    if ( zC == z1 )  return b1;
    /* if the area is absent, convert cover to BDD */
    if ( bA == b0 )  return extraZddConvertToBdd(dd,zC);
    /* if the area is constant 1, the BDD is also constant 1 */
    if ( bA == b1 )  return b1;

    /* check cache */
    bRes = cuddCacheLookup2(dd, extraZddConvertToBddAndAdd, zC, bA);
    if (bRes)
        return(bRes);
    else
    {
        DdNode * bRes0, * bRes1, * bRes2;
        DdNode * bA0, * bA1, * bTemp;
        int TopBddVar;
        /* find the level in terms of the original variable levels, not ZDD variable levels */
        int levCover = dd->permZ[zC->index] >> 1;
        int levArea  = dd->perm[Cudd_Regular(bA)->index];
        /* determine whether the area is a complemented BDD */
        int fIsComp  = Cudd_IsComplement(bA);

        if ( levCover > levArea )
        {
            TopBddVar = Cudd_Regular(bA)->index;

            /* find the parts(cofactors) of the area */
            bA0 = Cudd_NotCond( Cudd_E(bA), fIsComp );
            bA1 = Cudd_NotCond( Cudd_T(bA), fIsComp );

            /* solve for the Else part */
            bRes0 = extraZddConvertToBddAndAdd( dd, zC, bA0 );
            if ( bRes0 == NULL ) return NULL;
            cuddRef( bRes0 );

            /* solve for the Then part */
            bRes1 = extraZddConvertToBddAndAdd( dd, zC, bA1 );
            if ( bRes1 == NULL ) 
            {
                Cudd_RecursiveDeref( dd, bRes0 );
                return NULL;
            }
            cuddRef( bRes1 );
            /* only bRes0 and bRes1 are referenced at this point */
        }
        else /* if ( levCover <= levArea ) */
        {
            /* decompose the cover */
            DdNode *zC0, *zC1, *zC2;
            extraDecomposeCover(dd, zC, &zC0, &zC1, &zC2);

            /* assign the top-most BDD variable */
            TopBddVar = (zC->index >> 1);

            /* find the parts(cofactors) of the area */
            if ( levCover == levArea )
            { /* find the parts(cofactors) of the area */
                bA0 = Cudd_NotCond( Cudd_E( bA ), fIsComp );
                bA1 = Cudd_NotCond( Cudd_T( bA ), fIsComp );
            }
            else /* if ( levCover < levArea ) */
            { /* assign the cofactors */
                bA0 = bA1 = bA;
            }

            /* start the Else part of the solution */
            bRes0 = extraZddConvertToBddAndAdd(dd, zC0, bA0);
            if ( bRes0 == NULL )
                return NULL;
            cuddRef( bRes0 );

            /* start the Then part of the solution */
            bRes1 = extraZddConvertToBddAndAdd(dd, zC1, bA1);
            if ( bRes1 == NULL )
            {
                Cudd_RecursiveDeref(dd, bRes0);
                return NULL;
            }
            cuddRef( bRes1 );

            /* cover without literals converted into BDD */
            bRes2 = extraZddConvertToBdd(dd, zC2);
            if ( bRes2 == NULL ) 
            {
                Cudd_RecursiveDeref(dd, bRes0);
                Cudd_RecursiveDeref(dd, bRes1);
                return NULL;
            }
            cuddRef( bRes2 );

            /* add this BDD to the Else part */
            bTemp = bRes0;
            bRes0 = cuddBddAndRecur(dd, Cudd_Not(bRes0), Cudd_Not(bRes2));
            if ( bRes0 == NULL )
            {
                Cudd_RecursiveDeref(dd, bTemp);
                Cudd_RecursiveDeref(dd, bRes1);
                Cudd_RecursiveDeref(dd, bRes2);
                return NULL;
            }
            cuddRef( bRes0 );
            bRes0 = Cudd_Not(bRes0);
            Cudd_RecursiveDeref(dd, bTemp);

            /* add this BDD to the Then part */
            bTemp = bRes1;
            bRes1 = cuddBddAndRecur(dd, Cudd_Not(bRes1), Cudd_Not(bRes2));
            if ( bRes1 == NULL )
            {
                Cudd_RecursiveDeref(dd, bRes0);
                Cudd_RecursiveDeref(dd, bTemp);
                Cudd_RecursiveDeref(dd, bRes2);
                return NULL;
            }
            cuddRef( bRes1 );
            bRes1 = Cudd_Not(bRes1);
            Cudd_RecursiveDeref(dd, bTemp);
            Cudd_RecursiveDeref(dd, bRes2);
            /* only bRes0 and bRes1 are referenced at this point */
        }

        /* consider the case when Res0 and Res1 are the same node */
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        /* consider the case when Res1 is complemented */
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, TopBddVar, Cudd_Not(bRes1), Cudd_Not(bRes0));
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
            bRes = cuddUniqueInter( dd, TopBddVar, bRes1, bRes0 );
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
        cuddCacheInsert2(dd, extraZddConvertToBddAndAdd, zC, bA, bRes);
        return bRes;
    }
} /* end of extraZddConvertToBddAndAdd */


/**Function********************************************************************

  Synopsis [Performs the recursive step of Cudd_zddSingleCoveredArea.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode  *
extraZddSingleCoveredArea(
  DdManager * dd,
  DdNode * zC)
{
    DdNode  *bRes;
    statLine(dd); 

    /* if there is no cover, there is no BDD */
    if ( zC == z0 )  return b0;
    /* if the cover is the universe, the BDD is constant 1 */
    if ( zC == z1 )  return b1;

    /* check cache */
    bRes = cuddCacheLookup1(dd, extraZddSingleCoveredArea, zC);
    if (bRes)
        return(bRes);
    else
    {
        DdNode * bRes0, * bRes1, * bRes2, * bG0, * bG1;
        DdNode * zC0, * zC1, * zC2;
        int TopBddVar = (zC->index >> 1);

        /* cofactor the cover */
        extraDecomposeCover(dd, zC, &zC0, &zC1, &zC2);

        /* compute bdds for the three cofactors of the cover */
        bRes0 = extraZddSingleCoveredArea(dd, zC0);
        if ( bRes0 == NULL )
            return NULL;
        cuddRef( bRes0 );

        bRes1 = extraZddSingleCoveredArea(dd, zC1);
        if ( bRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            return NULL;
        }
        cuddRef( bRes1 );

        bRes2 = extraZddSingleCoveredArea(dd, zC2);
        if ( bRes2 == NULL )
        {
            Cudd_RecursiveDeref(dd, bRes0);
            Cudd_RecursiveDeref(dd, bRes1);
            return NULL;
        }
        cuddRef( bRes2 );
        /* only bRes0, bRes1 and bRes2 are referenced at this point */

        /* bRes0 = bRes0 & !bdd(zC2) + bRes2 & !bdd(zC0) */
        bG0 = extraZddConvertToBddAndAdd( dd, zC2, Cudd_Not(bRes0) );
        if ( bG0 == NULL )
        {
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes1);
            Cudd_IterDerefBdd(dd, bRes2);
            return NULL;
        }
        cuddRef( bG0 );
        Cudd_IterDerefBdd(dd, bRes0);

        bG1 = extraZddConvertToBddAndAdd( dd, zC0, Cudd_Not(bRes2) );
        if ( bG1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bRes1);
            Cudd_IterDerefBdd(dd, bRes2);
            return NULL;
        }
        cuddRef( bG1 );

        bRes0 = cuddBddAndRecur( dd, bG0, bG1 );
        if ( bRes0 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_IterDerefBdd(dd, bRes1);
            Cudd_IterDerefBdd(dd, bRes2);
            return NULL;
        }
        cuddRef( bRes0 );
        bRes0 = Cudd_Not(bRes0);
        Cudd_IterDerefBdd(dd, bG0);
        Cudd_IterDerefBdd(dd, bG1);

        /* bRes1 = bRes1 & !bdd(zC2) + bRes2 & !bdd(zC1) */
        bG0 = extraZddConvertToBddAndAdd( dd, zC2, Cudd_Not(bRes1) );
        if ( bG0 == NULL )
        {
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes1);
            Cudd_IterDerefBdd(dd, bRes2);
            return NULL;
        }
        cuddRef( bG0 );
        Cudd_IterDerefBdd(dd, bRes1);

        bG1 = extraZddConvertToBddAndAdd( dd, zC1, Cudd_Not(bRes2) );
        if ( bG1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bRes0);
            Cudd_IterDerefBdd(dd, bRes2);
            return NULL;
        }
        cuddRef( bG1 );
        Cudd_IterDerefBdd(dd, bRes2);

        bRes1 = cuddBddAndRecur( dd, bG0, bG1 );
        if ( bRes1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_IterDerefBdd(dd, bRes0);
            return NULL;
        }
        cuddRef( bRes1 );
        bRes1 = Cudd_Not(bRes1);
        Cudd_IterDerefBdd(dd, bG0);
        Cudd_IterDerefBdd(dd, bG1);

        /* only bRes0 and bRes1 are referenced at this point */

        /* consider the case when Res0 and Res1 are the same node */
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        /* consider the case when Res1 is complemented */
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, TopBddVar, Cudd_Not(bRes1), Cudd_Not(bRes0));
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
            bRes = cuddUniqueInter( dd, TopBddVar, bRes1, bRes0 );
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
        cuddCacheInsert1(dd, extraZddSingleCoveredArea, zC, bRes);
        return bRes;
    }
} /* end of extraZddSingleCoveredArea */


/**Function********************************************************************

  Synopsis    [Finds three cofactors of the cover w.r.t. to the topmost variable.]

  Description [Finds three cofactors of the cover w.r.t. to the topmost variable.
  Does not check the cover for being a constant. Assumes that ZDD variables encoding 
  positive and negative polarities are adjacent in the variable order. Is different 
  from cuddZddGetCofactors3() in that it does not compute the cofactors w.r.t. the 
  given variable but takes the cofactors with respent to the topmost variable. 
  This function is more efficient when used in recursive procedures because it does 
  not require referencing of the resulting cofactors (compare cuddZddProduct() 
  and cuddZddPrimeProduct()).]

  SideEffects [None]

  SeeAlso     [cuddZddGetCofactors3]

******************************************************************************/

//void 
//extraDecomposeCover( 
//  DdManager* dd,    /* the manager */
//  DdNode*  zC,      /* the cover */
//  DdNode** zC0,     /* the pointer to the negative var cofactor */ 
//  DdNode** zC1,     /* the pointer to the positive var cofactor */ 
//  DdNode** zC2 )    /* the pointer to the cofactor without var */ 
//{
//  if ( (zC->index & 1) == 0 ) 
//  { /* the top variable is present in positive polarity and maybe in negative */
//
//      DdNode *Temp = cuddE( zC );
//      *zC1  = cuddT( zC );
//      if ( cuddIZ(dd,Temp->index) == cuddIZ(dd,zC->index) + 1 )
//      {   /* Temp is not a terminal node 
//           * top var is present in negative polarity */
//          *zC2 = cuddE( Temp );
//          *zC0 = cuddT( Temp );
//      }
//      else
//      {   /* top var is not present in negative polarity */
//          *zC2 = Temp;
//          *zC0 = dd->zero;
//      }
//  }
//  else 
//  { /* the top variable is present only in negative */
//      *zC1 = dd->zero;
//      *zC2 = cuddE( zC );
//      *zC0 = cuddT( zC );
//  }
//} /* extraDecomposeCover */

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Returns the number of elements in a randomly selected combination.]

  Description [Returns the number of elements in a randomly selected combination.
               This number may be useful if the set contains combinations of 
               the same cardinality.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int 
zddCountVars( 
  DdManager *dd, /* the manager */
  DdNode* S)     /* the set */
{
    /* in a ZDD, positive edge never points to dd->zero */
    int Counter;
    assert( S != dd->zero );
    for ( Counter = 0; S != dd->one; Counter++, S = cuddT(S) );
    return Counter;
}
