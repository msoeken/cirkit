/**CFile****************************************************************

  FileName    [extraBddMisc.c]

  PackageName [extra]

  Synopsis    [Various BDD manipulating procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraBddMisc.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

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
static void ddProfileNode( DdManager * dd, DdNode * F, int * Profile, st_table * Visited );
static void ddProfileEdge( DdManager * dd, DdNode * F, int * Profile, st_table * Visited );
static DdNode * extraBddPermuteToAdd( DdManager * dd, DdNode * bFunc, int * Permute, st_table * Visited, DdNode ** paVars );
static DdNode * cuddBddPermuteRecur ( DdManager * manager, DdHashTable * table, DdNode * node, int *permut );

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Returns 1 if the BDD has at least one complemented edge.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_WithComplementedEdges( DdNode * bFunc )
{
    if ( Cudd_IsComplement(bFunc) )
        return 1;
    if ( cuddIsConstant(bFunc) )
        return 0;
    if ( Extra_WithComplementedEdges( cuddE(bFunc) ) )
        return 1;
    return Extra_WithComplementedEdges( cuddT(bFunc) );
}

/**Function********************************************************************

  Synopsis    [Remaps the function to depend on the topmost variables on the manager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddRemapUp(
  DdManager * dd,
  DdNode * bF )
{
    static int Permute[MAXINPUTS];
    DdNode * bSupp, * bTemp, * bRes;
    int Counter;

    // get support
    bSupp = Cudd_Support( dd, bF );    Cudd_Ref( bSupp );

    // create the variable map
    // to remap the DD into the upper part of the manager
    Counter = 0;
    for ( bTemp = bSupp; bTemp != b1; bTemp = cuddT(bTemp) )
        Permute[bTemp->index] = dd->invperm[Counter++];

    // transfer the BDD and remap it
    bRes = Cudd_bddPermute( dd, bF, Permute );  Cudd_Ref( bRes );

    // remove support
    Cudd_RecursiveDeref( dd, bSupp );

    // return
    Cudd_Deref( bRes );
    return bRes;
}


/**Function********************************************************************

  Synopsis    [Picks one on-set cube from the given DD.]

  Description [Picks the on-set cube from the given DD with the minimum integer 
  value. The cube is written into an array of characters.  The array must have at
  least as many entries as there are variables. Returns 1 if
  successful; 0 otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_bddPickOneMinterm]

******************************************************************************/
int
Extra_bddPickOneCube(
  DdManager * dd,
  DdNode * node,
  char * string)
{
    DdNode * N, * T, * E;
    int    i;

    if (string == NULL || node == NULL) 
        return 0;

    /* The constant 0 function has no on-set cubes. */
    if ( node == b0 ) 
        return 0;

    for ( i = 0; i < dd->size; i++ ) 
        string[i] = 2;

    while ( 1 )
    {
        if ( node == b1 ) 
            break;
        N = Cudd_Regular(node);
        T = cuddT(N); 
        E = cuddE(N);
        if ( Cudd_IsComplement(node) ) 
        {
            T = Cudd_Not(T); 
            E = Cudd_Not(E);
        }
        if ( T != b0 ) 
        {
            string[N->index] = 1;
            node = T;
        } 
        else 
        {
            string[N->index] = 0;
            node = E;
        } 
    }
    return 1;

} /* end of Extra_bddPickOneCube */


/**Function********************************************************************

  Synopsis    [Find any cube belonging to the on-set of the function.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode *  Extra_bddFindOneCube( DdManager * dd, DdNode * bF )
{
    static char s_Temp[MAXINPUTS];
    DdNode * bCube, * bTemp;
    int v;

    // get the vector of variables in the cube
    Cudd_bddPickOneCube( dd, bF, s_Temp );

    // start the cube
    bCube = b1; Cudd_Ref( bCube );

    for ( v = 0; v < dd->size; v++ )
        if ( s_Temp[v] == 0 )
        {
//          Cube &= !s_XVars[v];
            bCube = Cudd_bddAnd( dd, bTemp = bCube, Cudd_Not(dd->vars[v]) ); Cudd_Ref( bCube );
            Cudd_RecursiveDeref( dd, bTemp );
        }
        else if ( s_Temp[v] == 1 )
        {
//          Cube &=  s_XVars[v];
            bCube = Cudd_bddAnd( dd, bTemp = bCube,          dd->vars[v]  ); Cudd_Ref( bCube );
            Cudd_RecursiveDeref( dd, bTemp );
        }
    Cudd_Deref(bCube);
    return bCube;
}


/**Function********************************************************************

  Synopsis    [Returns one minterm contained in the given BDD.]

  Description [This function differs from Cudd_bddPickOneMinterm in that it 
  returns the minterm with the smallest bits-to-integer value.]

  SideEffects []

******************************************************************************/
DdNode * Extra_bddGetOneMinterm( DdManager * dd, DdNode * bFunc, DdNode * bSupp )
{
    DdNode * bSuppReal, * bSuppCube, * bSuppDiff, * bSuppTemp;
    DdNode * bCube, * bTemp;

    // make sure the support of bFunc is contained in bSupp
    bSuppReal = Cudd_Support( dd, bFunc );                      Cudd_Ref( bSuppReal );
    bTemp     = Cudd_bddExistAbstract( dd, bSuppReal, bSupp );  Cudd_Ref( bTemp );
    if ( bTemp != b1 )
    {
        printf( "Extra_bddGetOneMinterm(): Support is not valid!\n" );
        fflush( stdout );
        assert(0);
    }
    Cudd_RecursiveDeref( dd, bTemp );
    Cudd_RecursiveDeref( dd, bSuppReal );

    // get one cube
    bCube     = Extra_bddGetOneCube( dd, bFunc );               Cudd_Ref( bCube );
    // find the cube support
    bSuppCube = Cudd_Support( dd, bCube );                      Cudd_Ref( bSuppCube );
    // get the support difference (vars in bSupp and not in bCube )
    bSuppDiff = Cudd_bddExistAbstract( dd, bSupp, bSuppCube );  Cudd_Ref( bSuppDiff );
    // these vars should be added to the cube in the negative polarity
    for ( bSuppTemp = bSuppDiff; bSuppTemp != b1; bSuppTemp = cuddT(bSuppTemp) )
    {
        bCube = Cudd_bddAnd( dd, bTemp = bCube, Cudd_Not(dd->vars[bSuppTemp->index]) );    Cudd_Ref( bCube );
        Cudd_RecursiveDeref( dd, bTemp );
    }
    Cudd_RecursiveDeref( dd, bSuppCube );
    Cudd_RecursiveDeref( dd, bSuppDiff );
    Cudd_Deref(bCube);
    return bCube;
}

/**Function********************************************************************

  Synopsis    [Returns one cube contained in the given BDD.]

  Description [This function returns the cube with the smallest 
  bits-to-integer value.]

  SideEffects []

******************************************************************************/
DdNode * Extra_bddGetOneCube( DdManager * dd, DdNode * bFunc )
{
    DdNode * bFuncR, * bFunc0, * bFunc1;
    DdNode * bRes0,  * bRes1,  * bRes;

    bFuncR = Cudd_Regular(bFunc);
    if ( cuddIsConstant(bFuncR) )
        return bFunc;

    // cofactor
    if ( Cudd_IsComplement(bFunc) )
    {
        bFunc0 = Cudd_Not( cuddE(bFuncR) );
        bFunc1 = Cudd_Not( cuddT(bFuncR) );
    }
    else
    {
        bFunc0 = cuddE(bFuncR);
        bFunc1 = cuddT(bFuncR);
    }

    // try to find the cube with the negative literal
    bRes0 = Extra_bddGetOneCube( dd, bFunc0 );  Cudd_Ref( bRes0 );

    if ( bRes0 != b0 )
    {
        bRes = Cudd_bddAnd( dd, bRes0, Cudd_Not(dd->vars[bFuncR->index]) ); Cudd_Ref( bRes );
        Cudd_RecursiveDeref( dd, bRes0 );
    }
    else
    {
        Cudd_RecursiveDeref( dd, bRes0 );
        // try to find the cube with the positive literal
        bRes1 = Extra_bddGetOneCube( dd, bFunc1 );  Cudd_Ref( bRes1 );
        assert( bRes1 != b0 );
        bRes = Cudd_bddAnd( dd, bRes1, dd->vars[bFuncR->index] ); Cudd_Ref( bRes );
        Cudd_RecursiveDeref( dd, bRes1 );
    }

    Cudd_Deref( bRes );
    return bRes;
}

/**Function********************************************************************

  Synopsis    [Find any minterm belonging to the on-set of the function.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode *  Extra_bddFindOneMinterm( DdManager * dd, DdNode * bF, int nVars )
{
    static char s_Temp[MAXINPUTS];
    DdNode * bCube, * bTemp;
    int v;

    // get the vector of variables in the cube
    Cudd_bddPickOneCube( dd, bF, s_Temp );

    // start the cube
    bCube = b1; Cudd_Ref( bCube );

    assert( nVars <= dd->size );
    for ( v = 0; v < nVars; v++ )
        if ( s_Temp[v] == 1 )
        {
//          Cube &=  s_XVars[v];
            bCube = Cudd_bddAnd( dd, bTemp = bCube,          dd->vars[v]  ); Cudd_Ref( bCube );
            Cudd_RecursiveDeref( dd, bTemp );
        }
        else // if ( s_Temp[v] == 0 )
        {
//          Cube &= !s_XVars[v];
            bCube = Cudd_bddAnd( dd, bTemp = bCube, Cudd_Not(dd->vars[v]) ); Cudd_Ref( bCube );
            Cudd_RecursiveDeref( dd, bTemp );
        }
    Cudd_Deref(bCube);
    return bCube;
}


/**Function********************************************************************

  Synopsis    [Generates a random function.]

  Description [Given the number of variables n and the density of on-set minterms,
  this function generates a random function and returns its BDD. The variables are
  taken from the first variables of the DD manager. Returns NULL if density is 
  less than 0.0 or more than 1.0.]

  SideEffects [Allocates new BDD variables if their current number is less than n.]

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddRandomFunc( 
  DdManager * dd,   /* the DD manager */
  int n,            /* the number of variables */
  double d)         /* average density of on-set minterms */
{
    DdNode *Result, *TempCube, *Aux, **VarBdds;
    int c, v, Limit, *VarValues;
    int nMinterms;

    /* sanity check the parameters */
    if ( n <= 0 || d < 0.0 || d > 1.0 )
        return NULL;

    /* allocate temporary storage for variables and variable values */
    VarBdds = ALLOC( DdNode *, n );
    VarValues = ALLOC( int, n );
    if (VarBdds == NULL || VarValues == NULL) 
    {
        dd->errorCode = CUDD_MEMORY_OUT;
        return NULL;
    }
    /* set elementary BDDs corresponding to variables */
    for ( v = 0; v < n; v++ )
        VarBdds[v] = Cudd_bddIthVar( dd, v );

    /* start the function */
    Result = Cudd_Not( dd->one );
    Cudd_Ref( Result );

    /* seed random number generator */
    Cudd_Srandom( dd, time(NULL) );
//  Cudd_Srandom( 4 );
    /* determine the number of minterms */
    nMinterms = (int)(d*(1<<n));
    /* determine the limit below which var belongs to the minterm */
    Limit = (int)(0.5 * 2147483561.0);

    /* add minterms one by one */
    for ( c = 0; c < nMinterms; c++ )
    {
        for ( v = 0; v < n; v++ )
            if ( Cudd_Random( dd ) <= Limit )
                VarValues[v] = 1;
            else
                VarValues[v] = 0;

        TempCube = Cudd_bddComputeCube( dd, VarBdds, VarValues, n );
        Cudd_Ref( TempCube );

        /* make sure that this combination is not already in the set */
        if ( c )
        { /* at least one combination is already included */

            Aux = Cudd_bddAnd( dd, Result, TempCube );
            Cudd_Ref( Aux );
            if ( Aux != Cudd_Not( dd->one ) )
            {
                Cudd_RecursiveDeref( dd, Aux );
                Cudd_RecursiveDeref( dd, TempCube );
                c--;
                continue;
            }
            else 
            { /* Aux is the same node as Result */
                Cudd_Deref( Aux );
            }
        }

        Result = Cudd_bddOr( dd, Aux = Result, TempCube );
        Cudd_Ref( Result );
        Cudd_RecursiveDeref( dd, Aux );
        Cudd_RecursiveDeref( dd, TempCube );
    }

    FREE( VarValues );
    FREE( VarBdds );
    Cudd_Deref( Result );
    return Result;

} /* end of Extra_bddRandomFunc */


/**Function********************************************************************

  Synopsis    [Generates a random function.]

  Description [Given the number of variables n and the density of on-set minterms,
  this function generates a random function and returns its BDD. The variables are
  taken from the first variables of the DD manager. Returns NULL if density is 
  less than 0.0 or more than 1.0.]

  SideEffects [Allocates new BDD variables if their current number is less than n.]

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddGetRandomTransform( DdManager * dd, DdNode ** XVars, DdNode ** YVars, int nVars )
{
    DdNode * bRes, * bTemp, * bCubeX, * bCubeY, * bCube;
    // go through all the minterms
    int m, mRand;
    int nMints = (1<<nVars);
    int * fMintUsed = (int*)malloc( nMints * sizeof(int) );

    assert( nVars <= 10 );

    // set all minterms to be unused
    for ( m = 0; m < nMints; m++ )
        fMintUsed[m] = 0;

    bRes = b0; Cudd_Ref( bRes );
    for ( m = 0; m < nMints; m++ )
    {
        // get a random minterm that is not used
        mRand = Cudd_Random( dd ) % nMints;
        while ( fMintUsed[mRand] )
            mRand = (mRand+1) % nMints;
        // set as used
        fMintUsed[mRand] = 1;

        // create the cubes
        bCubeX = Extra_bddBitsToCube(dd,m,nVars,XVars);      Cudd_Ref( bCubeX );
        bCubeY = Extra_bddBitsToCube(dd,mRand,nVars,YVars);  Cudd_Ref( bCubeY );

        bCube =  Cudd_bddAnd(dd,bCubeX,bCubeY);      Cudd_Ref( bCube );
        Cudd_RecursiveDeref( dd, bCubeX );
        Cudd_RecursiveDeref( dd, bCubeY );

        bRes  = Cudd_bddOr( dd, bTemp = bRes, bCube);Cudd_Ref( bRes );
        Cudd_RecursiveDeref( dd, bTemp );
        Cudd_RecursiveDeref( dd, bCube );
    }

    free( fMintUsed );
    Cudd_Deref( bRes );
    return bRes;
}  /* end of Extra_bddGetRandomTransform */


/**Function********************************************************************

  Synopsis    [Computes the cube of BDD variables corresponding to bits it the bit-code]

  Description [Returns a bdd composed of elementary bdds found in array BddVars[] such 
  that the bdd vars encode the number Value of bit length CodeWidth (most significant bit 
  is encoded with the first bdd variable). If the variables BddVars are not specified,
  takes the first CodeWidth variables of the manager]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddBitsToCube( DdManager * dd, int Code, int CodeWidth, DdNode ** pbVars )
{
    int z;
    DdNode * bTemp, * bVar, * bVarBdd, * bResult;

    bResult = b1;   Cudd_Ref( bResult );
    for ( z = 0; z < CodeWidth; z++ )
    {
        bVarBdd = (pbVars)? pbVars[z]: dd->vars[z];
        bVar = Cudd_NotCond( bVarBdd, (Code & (1 << (CodeWidth-1-z)))==0 );
        bResult = Cudd_bddAnd( dd, bTemp = bResult, bVar );     Cudd_Ref( bResult );
        Cudd_RecursiveDeref( dd, bTemp );
    }
    Cudd_Deref( bResult );

    return bResult;
}  /* end of Extra_bddBitsToCube */


/**Function********************************************************************

  Synopsis    [Computes the positive polarty cube composed of the first vars in the array.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddComputeCube( DdManager * dd, DdNode ** bXVars, int nVars )
{
    DdNode * bRes;
    DdNode * bTemp;
    int i;

    bRes = b1; Cudd_Ref( bRes );
    for ( i = 0; i < nVars; i++ )
    {
        bRes = Cudd_bddAnd( dd, bTemp = bRes, bXVars[i] );  Cudd_Ref( bRes );
        Cudd_RecursiveDeref( dd, bTemp );
    }

    Cudd_Deref( bRes );
    return bRes;
}

/**Function********************************************************************

  Synopsis    [Builds BDD representing the set of fixed-size variable tuples.]

  Description [Creates BDD of all combinations of variables in Support that have k vars in them.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddTuples( 
  DdManager * dd,   /* the DD manager */
  int K,            /* the number of variables in tuples */
  DdNode * VarsN)   /* the set of all variables represented as a BDD */
{
    DdNode  *res;
    int     autoDyn;

    /* it is important that reordering does not happen, 
       otherwise, this methods will not work */

    autoDyn = dd->autoDyn;
    dd->autoDyn = 0;

    do {
        /* transform the numeric arguments (K) into a DdNode * argument;
         * this allows us to use the standard internal CUDD cache */
        DdNode *VarSet = VarsN, *VarsK = VarsN;
        int     nVars = 0, i;

        /* determine the number of variables in VarSet */
        while ( VarSet != b1 )
        {
            nVars++;
            /* make sure that the VarSet is a cube */
            if ( cuddE( VarSet ) != b0 )
                return NULL;
            VarSet = cuddT( VarSet );
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
            VarsK = cuddT( VarsK );

        dd->reordered = 0;
        res = extraBddTuples(dd, VarsK, VarsN );

    } while (dd->reordered == 1);
    dd->autoDyn = autoDyn;
    return(res);

} /* end of Extra_bddTuples */


/**Function********************************************************************

  Synopsis    [Changes the polarity of vars listed in the cube.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddChangePolarity( 
  DdManager * dd,   /* the DD manager */
  DdNode * bFunc,
  DdNode * bVars) 
{
    DdNode  *res;
    do {
        dd->reordered = 0;
        res = extraBddChangePolarity( dd, bFunc, bVars );
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_bddChangePolarity */



/**Function********************************************************************

  Synopsis    [Finds the node profile of the DD.]

  Description [Finds the number of nodes on each level. The array is allocated 
  by the user and should have at least   as many entries as the maximum number 
  of variables in the BDD and ZDD parts of the manager.]

  SideEffects [None]

  SeeAlso     [Cudd_Support]

******************************************************************************/
int * 
Extra_ProfileNode(
  DdManager * dd, /* manager */
  DdNode * F,     /* DD whose profile is sought */
  int * Profile ) /* array allocated by the user */
{
    int i, size;
    st_table * Visited;

    // Initialize support array for ddSupportStep. 
    size = ddMax(dd->size, dd->sizeZ);
    for (i = 0; i < size; i++) 
        Profile[i] = 0;
  
    // start the visited table
    Visited  = st_init_table(st_ptrcmp,st_ptrhash);
  
    // get the profile
    ddProfileNode(dd, Cudd_Regular(F), Profile, Visited);

    st_free_table( Visited );
    return(Profile);

} /* end of Extra_ProfileNode */

/**Function********************************************************************

  Synopsis    [Finds the node profile of the shared DD.]

  Description [Finds the number of nodes on each level. The array is allocated 
  by the user and should have at least as many entries as the maximum number 
  of variables in the BDD and ZDD parts of the manager.]

  SideEffects [None]

  SeeAlso     [Cudd_Support]

******************************************************************************/
int * 
Extra_ProfileNodeSharing(
  DdManager * dd,    /* manager */
  DdNode ** pFuncs,  /* the DDs whose profile is sought */
  int nFuncs,        /* the number of DDs */
  int * Profile )    /* array allocated by the user */
{
    int i, size;
    st_table * Visited;

    // Initialize support array for ddSupportStep. 
    size = ddMax(dd->size, dd->sizeZ);
    for (i = 0; i < size; i++) 
        Profile[i] = 0;
  
    // start the visited table
    Visited  = st_init_table(st_ptrcmp,st_ptrhash);
  
    // get the profile
    for ( i = 0; i < nFuncs; i++ )
        ddProfileNode(dd, Cudd_Regular(pFuncs[i]), Profile, Visited);

    st_free_table( Visited );
    return(Profile);

} /* end of Extra_ProfileNodeSharing */


/**Function********************************************************************

  Synopsis    [Finds the edge profile of the DD.]

  Description [Finds how many edges of each length exists. The array is allocated 
  by the user and should have at least   as many entries as the maximum number 
  of variables in the BDD and ZDD parts of the manager.]

  SideEffects [None]

  SeeAlso     [Cudd_Support]

******************************************************************************/
int * 
Extra_ProfileEdge(
  DdManager * dd, /* manager */
  DdNode * F,     /* DD whose profile is sought */
  int * Profile ) /* array allocated by the user */
{
    int i, size;
    st_table * Visited;

    // Initialize support array for ddSupportStep. 
    size = ddMax(dd->size, dd->sizeZ);
    for (i = 0; i < size; i++) 
        Profile[i] = 0;
  
    // start the visited table
    Visited  = st_init_table(st_ptrcmp,st_ptrhash);
  
    // get the profile
    ddProfileEdge(dd, Cudd_Regular(F), Profile, Visited);

    st_free_table( Visited );
    return(Profile);

} /* end of Extra_ProfileEdge */

/**Function********************************************************************

  Synopsis    [Finds the edge profile of the shared DD.]

  Description [Finds how many edges of each length exists. The array is allocated 
  by the user and should have at least as many entries as the maximum number 
  of variables in the BDD and ZDD parts of the manager.]

  SideEffects [None]

  SeeAlso     [Cudd_Support]

******************************************************************************/
int * 
Extra_ProfileEdgeSharing(
  DdManager * dd,    /* manager */
  DdNode ** pFuncs,  /* the DDs whose profile is sought */
  int nFuncs,        /* the number of DDs */
  int * Profile )    /* array allocated by the user */
{
    int i, size;
    st_table * Visited;

    // Initialize support array for ddSupportStep. 
    size = ddMax(dd->size, dd->sizeZ);
    for (i = 0; i < size; i++) 
        Profile[i] = 0;
  
    // start the visited table
    Visited  = st_init_table(st_ptrcmp,st_ptrhash);
  
    // get the profile
    for ( i = 0; i < nFuncs; i++ )
        ddProfileEdge(dd, Cudd_Regular(pFuncs[i]), Profile, Visited);

    st_free_table( Visited );
    return(Profile);

} /* end of Extra_ProfileEdgeSharing */


/**Function********************************************************************

  Synopsis    [Permutes the variables of the array of BDDs.]

  Description [Given a permutation in array permut, creates a new BDD
  with permuted variables. There should be an entry in array permut
  for each variable in the manager. The i-th entry of permut holds the
  index of the variable that is to substitute the i-th variable.
  The DDs in the resulting array are already referenced.]

  SideEffects [None]

  SeeAlso     [Cudd_addPermute Cudd_bddSwapVariables]

******************************************************************************/
void Extra_bddPermuteArray( DdManager * manager, DdNode ** bNodesIn, DdNode ** bNodesOut, int nNodes, int *permut )
{
    DdHashTable *table;
    int i, k;
    do
    {
        manager->reordered = 0;
        table = cuddHashTableInit( manager, 1, 2 );

        /* permute the output functions one-by-one */
        for ( i = 0; i < nNodes; i++ )
        {
            bNodesOut[i] = cuddBddPermuteRecur( manager, table, bNodesIn[i], permut );
            if ( bNodesOut[i] == NULL )
            {
                /* deref the array of the already computed outputs */
                for ( k = 0; k < i; k++ )
                    Cudd_RecursiveDeref( manager, bNodesOut[k] );
                break;
            }
            cuddRef( bNodesOut[i] );
        }
        /* Dispose of local cache. */
        cuddHashTableQuit( table );
    }
    while ( manager->reordered == 1 );
}   /* end of Extra_bddPermuteArray */


/**Function********************************************************************

  Synopsis    [Performs the boolean difference w.r.t to a cube (AKA unique quontifier).]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode *
Extra_bddBooleanDiffCube( DdManager * dd, DdNode * bFunc, DdNode * bCube )
{
    DdNode * bRes;
    if ( bCube == b1 )
        return bFunc;
    do
    {
        dd->reordered = 0;
        bRes = extraBddBooleanDiffCube( dd, Cudd_Regular(bFunc), bCube );
    }
    while ( dd->reordered == 1 );
    return bRes;
}                               /* end of Extra_bddBooleanDiffCube */


/**Function********************************************************************

  Synopsis    [Takes the BDD and the permutation array. Returns the ADD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * 
Extra_bddPermuteToAdd( DdManager * dd, DdNode * bFunc, int * Permute )
{
    DdNode * aRes;
    st_generator * gen;
    DdNode * bNode, * aNode;
    st_table *  Visited;
    DdNode ** paVars;
    int i;

    // start the visited table
    Visited = st_init_table(st_ptrcmp,st_ptrhash);
    // prepare the array of ADD vars
    paVars = ALLOC( DdNode *, dd->size );
    for ( i = 0; i < dd->size; i++ )
    {
        paVars[i] = cuddUniqueInter( dd, i, a1, a0 ); cuddRef( paVars[i] );
    }

    aRes = extraBddPermuteToAdd( dd, bFunc, Permute, Visited, paVars ); cuddRef( aRes );

//printf( "The number of cache entries = %d\n", st_count(Visited) );
    // remove the ADDs in cache
    st_foreach_item( Visited, gen, (char**)&bNode, (char**)&aNode) 
        Cudd_RecursiveDeref( dd, aNode );
    st_free_table(Visited);
    for ( i = 0; i < dd->size; i++ )
        Cudd_RecursiveDeref( dd, paVars[i] );
    free( paVars );

    cuddDeref( aRes );
    return aRes;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddTuples().]

  Description [Generates in a bottom-up fashion BDD for all combinations
               composed of k variables out of variables belonging to bVarsN.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddTuples( 
  DdManager * dd,    /* the DD manager */
  DdNode * bVarsK,   /* the number of variables in tuples */
  DdNode * bVarsN)   /* the set of all variables */
{
    DdNode *bRes, *bRes0, *bRes1;
    statLine(dd); 

    /* terminal cases */
/*  if ( k < 0 || k > n )
 *      return dd->zero;
 *  if ( n == 0 )
 *      return dd->one; 
 */
    if ( cuddI( dd, bVarsK->index ) < cuddI( dd, bVarsN->index ) )
        return b0;
    if ( bVarsN == b1 )
        return b1;

    /* check cache */
    bRes = cuddCacheLookup2(dd, extraBddTuples, bVarsK, bVarsN);
    if (bRes)
        return(bRes);

    /* ZDD in which is variable is 0 */
/*  bRes0 = extraBddTuples( dd, k,     n-1 ); */
    bRes0 = extraBddTuples( dd, bVarsK, cuddT(bVarsN) );
    if ( bRes0 == NULL ) 
        return NULL;
    cuddRef( bRes0 );

    /* ZDD in which is variable is 1 */
/*  bRes1 = extraBddTuples( dd, k-1,          n-1 ); */
    if ( bVarsK == b1 )
    {
        bRes1 = b0;
        cuddRef( bRes1 );
    }
    else
    {
        bRes1 = extraBddTuples( dd, cuddT(bVarsK), cuddT(bVarsN) );
        if ( bRes1 == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            return NULL;
        }
        cuddRef( bRes1 );
    }

    /* consider the case when Res0 and Res1 are the same node */
    if ( bRes0 == bRes1 )
        bRes = bRes1;
    /* consider the case when Res1 is complemented */
    else if ( Cudd_IsComplement(bRes1) ) 
    {
        bRes = cuddUniqueInter(dd, bVarsN->index, Cudd_Not(bRes1), Cudd_Not(bRes0));
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
        bRes = cuddUniqueInter( dd, bVarsN->index, bRes1, bRes0 );
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
    cuddCacheInsert2(dd, extraBddTuples, bVarsK, bVarsN, bRes);
    return bRes;

} /* end of extraBddTuples */


/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddChangePolarity().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddChangePolarity( 
  DdManager * dd,    /* the DD manager */
  DdNode * bFunc, 
  DdNode * bVars) 
{
    DdNode * bRes;

    if ( bVars == b1 )
        return bFunc;
    if ( Cudd_IsConstant(bFunc) )
        return bFunc;

    if ( bRes = cuddCacheLookup2(dd, extraBddChangePolarity, bFunc, bVars) )
        return bRes;
    else
    {
        DdNode * bFR = Cudd_Regular(bFunc); 
        int LevelF   = dd->perm[bFR->index];
        int LevelV   = dd->perm[bVars->index];

        if ( LevelV < LevelF )
            bRes = extraBddChangePolarity( dd, bFunc, cuddT(bVars) );
        else // if ( LevelF <= LevelV )
        {
            DdNode * bRes0, * bRes1;             
            DdNode * bF0, * bF1;             
            DdNode * bVarsNext;

            // cofactor the functions
            if ( bFR != bFunc ) // bFunc is complemented 
            {
                bF0 = Cudd_Not( cuddE(bFR) );
                bF1 = Cudd_Not( cuddT(bFR) );
            }
            else
            {
                bF0 = cuddE(bFR);
                bF1 = cuddT(bFR);
            }

            if ( LevelF == LevelV )
                bVarsNext = cuddT(bVars);
            else
                bVarsNext = bVars;

            bRes0 = extraBddChangePolarity( dd, bF0, bVarsNext );
            if ( bRes0 == NULL ) 
                return NULL;
            cuddRef( bRes0 );

            bRes1 = extraBddChangePolarity( dd, bF1, bVarsNext );
            if ( bRes1 == NULL ) 
            {
                Cudd_RecursiveDeref( dd, bRes0 );
                return NULL;
            }
            cuddRef( bRes1 );

            if ( LevelF == LevelV )
            { // swap the cofactors
                DdNode * bTemp;
                bTemp = bRes0;
                bRes0 = bRes1;
                bRes1 = bTemp;
            }

            /* only aRes0 and aRes1 are referenced at this point */

            /* consider the case when Res0 and Res1 are the same node */
            if ( bRes0 == bRes1 )
                bRes = bRes1;
            /* consider the case when Res1 is complemented */
            else if ( Cudd_IsComplement(bRes1) ) 
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
            
        /* insert the result into cache */
        cuddCacheInsert2(dd, extraBddChangePolarity, bFunc, bVars, bRes);
        return bRes;
    }
} /* end of extraBddChangePolarity */


/**Function********************************************************************

  Synopsis    [Performs the recursive steps of Extra_bddBooleanDiffCube.]

  Description [Performs the recursive steps of Extra_bddBooleanDiffCube.
  Returns the BDD obtained by XORing the cofactors of bFunc with respect to
  all the variables in bCube; NULL otherwise. Exploits the fact that dF/dx =
  dF'/dx.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode *
extraBddBooleanDiffCube( DdManager * dd, DdNode * bFunc, DdNode * bCube )
{
    DdNode * bRes;
    int LevelF;
    int LevelV;

    statLine( dd );

    assert( bCube != b1 );
    if ( cuddIsConstant(bFunc) )
        return b0;
    // from now on, bFunc and bCube are non-constants

    LevelF = dd->perm[bFunc->index];
    LevelV = dd->perm[bCube->index];
    // if bCube contains a variable not in bFunc, the EXOR of two identical cofactors is zero
    if ( LevelF > LevelV )
        return b0;

    if ( bRes = cuddCacheLookup2( dd, extraBddBooleanDiffCube, bFunc, bCube ) )
        return bRes;
    else
    {
        DdNode * bF0,   * bF1;
        DdNode * bRes0, * bRes1;

        // cofactor the function
        bF0 = cuddE(bFunc);
        bF1 = cuddT(bFunc);

        if ( LevelF == LevelV )
        {
            if ( cuddT(bCube) == b1 )
                bRes0 = bF0;
            else
                bRes0 = extraBddBooleanDiffCube( dd, Cudd_Regular(bF0), cuddT(bCube) );
            if ( bRes0 == NULL ) 
                return NULL;
            cuddRef( bRes0 );

            if ( cuddT(bCube) == b1 )
                bRes1 = bF1;
            else
                bRes1 = extraBddBooleanDiffCube( dd, bF1, cuddT(bCube) );
            if ( bRes1 == NULL ) 
            {
                Cudd_RecursiveDeref( dd, bRes0 );
                return NULL;
            }
            cuddRef( bRes1 );

            bRes = cuddBddXorRecur( dd, bRes0, bRes1 );
            if ( bRes == NULL ) 
            {
                Cudd_RecursiveDeref( dd, bRes0 );
                Cudd_RecursiveDeref( dd, bRes1 );
                return NULL;
            }
            cuddRef( bRes );
            Cudd_RecursiveDeref( dd, bRes0 );
            Cudd_RecursiveDeref( dd, bRes1 );
            cuddDeref( bRes );
        }
        else //if ( LevelF < LevelV )
        {
            bRes0 = extraBddBooleanDiffCube( dd, Cudd_Regular(bF0), bCube );
            if ( bRes0 == NULL ) 
                return NULL;
            cuddRef( bRes0 );

            bRes1 = extraBddBooleanDiffCube( dd, bF1, bCube );
            if ( bRes1 == NULL ) 
            {
                Cudd_RecursiveDeref( dd, bRes0 );
                return NULL;
            }
            cuddRef( bRes1 );

            // consider the case when bRes0 and bRes1 are the same node 
            if ( bRes0 == bRes1 )
                bRes = bRes1;
            // consider the case when Res1 is complemented 
            else if ( Cudd_IsComplement(bRes1) ) 
            {
                bRes = cuddUniqueInter(dd, bFunc->index, Cudd_Not(bRes1), Cudd_Not(bRes0));
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
                bRes = cuddUniqueInter( dd, bFunc->index, bRes1, bRes0 );
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
    }

    cuddCacheInsert2( dd, extraBddBooleanDiffCube, bFunc, bCube, bRes );
    return bRes;

}                               /* end of extraBddBooleanDiffCube */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_ProfileNode().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void ddProfileNode( DdManager * dd, DdNode * F, int * Profile, st_table * Visited )
// call with the regular pointer
{
    if ( F->index != CUDD_CONST_INDEX )
    {
        if ( st_lookup(Visited, (char*)F, NULL) )
        {
            // the entry already exists
            return; 
        }

        // insert the entry
        st_insert(Visited, (char*)F, NULL);

        // this node has not been visited
        Profile[dd->perm[F->index]]++;

        ddProfileNode( dd, Cudd_Regular(cuddE(F)), Profile, Visited );
        ddProfileNode( dd, cuddT(F), Profile, Visited );
    }
} /* end of ddProfileNode */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_ProfileEdge().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void ddProfileEdge( DdManager * dd, DdNode * F, int * Profile, st_table * Visited )
// call with the regular pointer
{
    if ( F->index != CUDD_CONST_INDEX )
    {
        DdNode * bF0R, * bF1R;
        int Lev, Lev0, Lev1;
        int Length0, Length1;

        if ( st_lookup(Visited, (char*)F, NULL) )
        {
            // the entry already exists
            return; 
        }

        // insert the entry
        st_insert(Visited, (char*)F, NULL);


        // compute the cofactors
        bF0R = Cudd_Regular(cuddE(F));
        bF1R = cuddT(F);
        Lev  = cuddI(dd,F->index);
        Lev0 = cuddI(dd,bF0R->index);
        Lev1 = cuddI(dd,bF1R->index);
        Length0 = ddMin( Lev0-Lev, dd->size-1 );
        Length1 = ddMin( Lev1-Lev, dd->size-1 );

        // this node has not been visited
        Profile[Length0]++;
        Profile[Length1]++;

        ddProfileEdge( dd, Cudd_Regular(cuddE(F)), Profile, Visited );
        ddProfileEdge( dd, cuddT(F), Profile, Visited );
    }
} /* end of ddProfileEdge */



/**Function********************************************************************

  Synopsis    [Implements the recursive step of Cudd_bddPermute.]

  Description [ Recursively puts the BDD in the order given in the array permut.
  Checks for trivial cases to terminate recursion, then splits on the
  children of this node.  Once the solutions for the children are
  obtained, it puts into the current position the node from the rest of
  the BDD that should be here. Then returns this BDD.
  The key here is that the node being visited is NOT put in its proper
  place by this instance, but rather is switched when its proper position
  is reached in the recursion tree.<p>
  The DdNode * that is returned is the same BDD as passed in as node,
  but in the new order.]

  SideEffects [None]

  SeeAlso     [Cudd_bddPermute cuddAddPermuteRecur]

******************************************************************************/
static DdNode *
cuddBddPermuteRecur( DdManager * manager /* DD manager */ ,
                     DdHashTable * table /* computed table */ ,
                     DdNode * node /* BDD to be reordered */ ,
                     int *permut /* permutation array */  )
{
    DdNode *N, *T, *E;
    DdNode *res;
    int index;

    statLine( manager );
    N = Cudd_Regular( node );

    /* Check for terminal case of constant node. */
    if ( cuddIsConstant( N ) )
    {
        return ( node );
    }

    /* If problem already solved, look up answer and return. */
    if ( N->ref != 1 && ( res = cuddHashTableLookup1( table, N ) ) != NULL )
        return ( Cudd_NotCond( res, N != node ) );

    /* Split and recur on children of this node. */
    T = cuddBddPermuteRecur( manager, table, cuddT( N ), permut );
    if ( T == NULL )
        return ( NULL );
    cuddRef( T );
    E = cuddBddPermuteRecur( manager, table, cuddE( N ), permut );
    if ( E == NULL )
    {
        Cudd_IterDerefBdd( manager, T );
        return ( NULL );
    }
    cuddRef( E );

    /* Move variable that should be in this position to this position
       ** by retrieving the single var BDD for that variable, and calling
       ** cuddBddIteRecur with the T and E we just created.
     */
    index = permut[N->index];
    res = cuddBddIteRecur( manager, manager->vars[index], T, E );
    if ( res == NULL )
    {
        Cudd_IterDerefBdd( manager, T );
        Cudd_IterDerefBdd( manager, E );
        return ( NULL );
    }
    cuddRef( res );
    Cudd_IterDerefBdd( manager, T );
    Cudd_IterDerefBdd( manager, E );

    /* Do not keep the result if the reference count is only 1, since
       ** it will not be visited again.
     */
    if ( N->ref != 1 )
    {
        ptrint fanout = ( ptrint ) N->ref;
        cuddSatDec( fanout );
        if ( !cuddHashTableInsert1( table, N, res, fanout ) )
        {
            Cudd_IterDerefBdd( manager, res );
            return ( NULL );
        }
    }
    cuddDeref( res );
    return ( Cudd_NotCond( res, N != node ) );

}                               /* end of cuddBddPermuteRecur */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_bddPermuteToAdd.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddPermuteToAdd( DdManager * dd, DdNode * bFunc, int * Permute, st_table * Visited, DdNode ** paVars )
{
    DdNode * aRes, * aRes0, * aRes1;
    DdNode * bFuncR;

    // check for terminals
    if ( bFunc == b0 )
        return a0;
    if ( bFunc == b1 )
        return a1;

    bFuncR = Cudd_Regular( bFunc );
    if ( bFuncR->ref != 1 ) 
    {
        if ( st_lookup( Visited, (char*)bFunc, (char**)&aRes ) )
            return aRes;
    }
    // else, this BDD is pointed only once and we came here, 
    // so we did not visit it before and will not visit it again

    // solve recursively
    aRes0 = extraBddPermuteToAdd( dd, cuddE(bFuncR), Permute, Visited, paVars );
    if ( aRes0 == NULL )
        return NULL;
    cuddRef( aRes0 );
    aRes1 = extraBddPermuteToAdd( dd, cuddT(bFuncR), Permute, Visited, paVars );
    if ( aRes1 == NULL )
    {
        Cudd_RecursiveDeref( dd, aRes0 );
        return NULL;
    }
    cuddRef( aRes1 );

    // compose
    aRes = cuddAddIteRecur( dd, paVars[Permute[bFuncR->index]], aRes1, aRes0 );
    if ( aRes == NULL )
    {
        Cudd_RecursiveDeref( dd, aRes0 );
        Cudd_RecursiveDeref( dd, aRes1 );
        return ( NULL );
    }
    cuddRef( aRes );
    Cudd_RecursiveDeref( dd, aRes0 );
    Cudd_RecursiveDeref( dd, aRes1 );

    if ( bFuncR != bFunc ) 
    {
        DdNode * aTemp;
        aRes = cuddAddCmplRecur( dd, aTemp = aRes );  cuddRef( aRes );
        Cudd_RecursiveDeref( dd, aTemp );
    }

    if ( bFuncR->ref != 1 ) 
        st_insert( Visited, (char*)bFunc, (char*)aRes ); // reference the entry in cache
    else
        cuddDeref( aRes );
    return aRes;
}


