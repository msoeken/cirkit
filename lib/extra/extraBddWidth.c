/**CFile****************************************************************

  FileName    [extraBddWidth.c]

  PackageName [extra]

  Synopsis    [Procedures to compute the width of the BDD.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraBddWidth.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

typedef struct prof_info_ prof_info;

struct prof_info_ 
{
    DdManager *    dd;          // the current manager
    int *          pStarting;   // the number of edges at this level
    int *          pStopping;   // the number of cofactors stopping at this level
    int *          pProfile;    // the resulting profile
    st_table *     tCof2Ref;    // the mapping of cofactors into their top refs
};

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

static void extraCofactorsComputeAtLevel_rec( DdManager * dd, DdNode * Func, int CutLevel, st_table * tVisited, st_table * tNodes );
static void extraProfileWidthCompute_rec( prof_info * p, DdNode * Func );
static int extraProfileWidthCalculate( prof_info * p );
static void extraProfileWidthUpdate( prof_info * p, DdNode * node, int TopLevelNew );
static void extraProfileWidthDelocate( prof_info * p );
static prof_info * extraProfileWidthAllocate( DdManager * dd );

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function*************************************************************

  Synopsis    [Smart computation of the BDD profile (works for complemented edges).]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
st_table * Extra_CofactorsComputeAtLevelSharing( DdManager * dd, DdNode * Funcs[], int nFuncs, int CutLevel )
{
    st_table * tVisited, * tNodes;
    int i;

    tVisited = st_init_table(st_ptrcmp,st_ptrhash);
    tNodes   = st_init_table(st_ptrcmp,st_ptrhash);
    for ( i = 0; i < nFuncs; i++ )
        extraCofactorsComputeAtLevel_rec( dd, Funcs[i], CutLevel, tVisited, tNodes );
    st_free_table( tVisited );
    return tNodes;
}

/**Function*************************************************************

  Synopsis    [Smart computation of the BDD profile (works for complemented edges).]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
st_table * Extra_CofactorsComputeAtLevel( DdManager * dd, DdNode * Func, int CutLevel )
{
    st_table * tVisited, * tNodes;

    tVisited = st_init_table(st_ptrcmp,st_ptrhash);
    tNodes   = st_init_table(st_ptrcmp,st_ptrhash);
    extraCofactorsComputeAtLevel_rec( dd, Func, CutLevel, tVisited, tNodes );
    st_free_table( tVisited );
    return tNodes;
}


/**Function*************************************************************

  Synopsis    [Smart computation of the BDD profile (works for complemented edges).]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Extra_ProfileWidthComputeAtLevelSharing( DdManager * dd, DdNode * Funcs[], int nFuncs, int CutLevel )
{
    st_table * tVisited, * tNodes;
    int RetValue;
    int i;

    tVisited = st_init_table(st_ptrcmp,st_ptrhash);
    tNodes   = st_init_table(st_ptrcmp,st_ptrhash);
    for ( i = 0; i < nFuncs; i++ )
        extraCofactorsComputeAtLevel_rec( dd, Funcs[i], CutLevel, tVisited, tNodes );
    RetValue = st_count( tNodes );
    st_free_table( tNodes );
    st_free_table( tVisited );
    return RetValue;
}

/**Function*************************************************************

  Synopsis    [Smart computation of the BDD profile (works for complemented edges).]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Extra_ProfileWidthComputeAtLevel( DdManager * dd, DdNode * Func, int CutLevel )
{
    st_table * tVisited, * tNodes;
    int RetValue;

    tVisited = st_init_table(st_ptrcmp,st_ptrhash);
    tNodes   = st_init_table(st_ptrcmp,st_ptrhash);
    extraCofactorsComputeAtLevel_rec( dd, Func, CutLevel, tVisited, tNodes );
    RetValue = st_count( tNodes );
    st_free_table( tNodes );
    st_free_table( tVisited );
    return RetValue;
}


/**Function*************************************************************

  Synopsis    [Smart computation of the BDD profile (works for complemented edges).]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Extra_ProfileWidthComputeSharing( DdManager * dd, DdNode * Funcs[], int nFuncs, int ** ppProfile )
{
    prof_info * p;
    int WidthMax;
    int i;

    // create the data structure
    p = extraProfileWidthAllocate( dd );
    for ( i = 0; i < nFuncs; i++ )
    {
        // traverse and add the nodes
        extraProfileWidthCompute_rec( p, Funcs[i] );
        // add the topmost node to the profile
        extraProfileWidthUpdate( p, Funcs[i], 0 );
    }
    // calculate the profile
    WidthMax = extraProfileWidthCalculate( p );
    // prepare the return values
    *ppProfile  = p->pProfile;
    p->pProfile = NULL;
    // remove the data structure
    extraProfileWidthDelocate( p );

    return WidthMax;
} /* end of Extra_ProfileWidthComputeSharing */


/**Function*************************************************************

  Synopsis    [Smart computation of the BDD profile (works for complemented edges).]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Extra_ProfileWidthCompute( DdManager * dd, DdNode * Func, int ** ppProfile )
{
    prof_info * p;
    int WidthMax;

    // create the data structure
    p = extraProfileWidthAllocate( dd );
    // traverse and add the nodes
    extraProfileWidthCompute_rec( p, Func );
    // add the topmost node to the profile
    extraProfileWidthUpdate( p, Func, 0 );
    // calculate the profile
    WidthMax = extraProfileWidthCalculate( p );
    // prepare the return values
    *ppProfile  = p->pProfile;
    p->pProfile = NULL;
    // remove the data structure
    extraProfileWidthDelocate( p );

    return WidthMax;
} /* end of Extra_ProfileWidthCompute */



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraCofactorsComputeAtLevel_rec( DdManager * dd, DdNode * Func, int CutLevel, st_table * tVisited, st_table * tNodes )
{
    DdNode * FuncR, * Func0, * Func1;

    // check if this node has already been visited
    if ( st_find_or_add( tVisited, (char *)Func, NULL ) )
        return;

    FuncR = Cudd_Regular(Func);
    if ( cuddIsConstant(FuncR) || dd->perm[FuncR->index] >= CutLevel )
    {
        st_find_or_add( tNodes, (char *)Func, NULL );
        return;
    }

    // cofactor the function
    if ( FuncR != Func )
    {
        Func0 = Cudd_Not(cuddE(FuncR));
        Func1 = Cudd_Not(cuddT(FuncR));
    }
    else
    {
        Func0 = cuddE(FuncR);
        Func1 = cuddT(FuncR);
    }

    // visit the cofactors
    extraCofactorsComputeAtLevel_rec( dd, Func0, CutLevel, tVisited, tNodes );
    extraCofactorsComputeAtLevel_rec( dd, Func1, CutLevel, tVisited, tNodes );
}



/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraProfileWidthCompute_rec( prof_info * p, DdNode * Func )
{
    DdNode * FuncR, * Func0, * Func1;

    // check if this node has already been visited
    if ( st_is_member( p->tCof2Ref, (char*)Func ) )
        return;
    FuncR = Cudd_Regular(Func);
    if ( cuddIsConstant(FuncR) )
        return;

    // cofactor the function
    if ( FuncR != Func )
    {
        Func0 = Cudd_Not(cuddE(FuncR));
        Func1 = Cudd_Not(cuddT(FuncR));
    }
    else
    {
        Func0 = cuddE(FuncR);
        Func1 = cuddT(FuncR);
    }

    // visit the cofactors
    extraProfileWidthCompute_rec( p, Func0 );
    extraProfileWidthCompute_rec( p, Func1 );
    // update the cofactor entries
    extraProfileWidthUpdate( p, Func0, p->dd->perm[FuncR->index]+1 );
    extraProfileWidthUpdate( p, Func1, p->dd->perm[FuncR->index]+1 );
}

/**Function*************************************************************

  Synopsis    [Calculates the profiles and computes the largest width.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int extraProfileWidthCalculate( prof_info * p )
{
    int WidthMax, lev;
    // compute the profile
    WidthMax = -1;
    for ( lev = 0; lev <= p->dd->size; lev++ )
    {
        if ( lev == 0 )
            p->pProfile[lev] = p->pStarting[lev] - p->pStopping[lev];
        else
            p->pProfile[lev] = p->pProfile[lev-1] + p->pStarting[lev] - p->pStopping[lev];

        if ( WidthMax < p->pProfile[lev] )
            WidthMax = p->pProfile[lev];
    }
    assert( WidthMax >= 0 );
    return WidthMax;
}

/**Function*************************************************************

  Synopsis    [Updates the topmost level from which the given node is referenced.]

  Description [Takes the table which maps each BDD nodes (including the constants)
  into the topmost level on which this node counts as a cofactor. Takes the topmost
  level, on which this node counts as a cofactor (see Extra_ProfileWidthFast(). 
  Takes the node, for which the table entry should be updated.]

  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraProfileWidthUpdate( prof_info * p, DdNode * node, int TopLevelNew )
{
    int * pTopLevel;
    DdNode * nodeR;

    if ( st_find_or_add( p->tCof2Ref, (char*)node, (char***)&pTopLevel ) )
    { // the node is already referenced
        // the current top level should be updated if the new level is higher
        if ( TopLevelNew < *pTopLevel ) 
        {
            // update the starting level
            p->pStarting[*pTopLevel]--;
            *pTopLevel = TopLevelNew;
            p->pStarting[*pTopLevel]++;
            // the stopping level does not have to be updated
        }
    }
    else
    { // the node is not referenced
        // its level should be set to the current new level
        *pTopLevel = TopLevelNew;
        // set the starting and the stopping levels
        nodeR = Cudd_Regular(node);
        p->pStarting[TopLevelNew]++;
        if ( !cuddIsConstant(nodeR) )
            p->pStopping[ p->dd->perm[nodeR->index] + 1 ]++;
    }
}

/**Function*************************************************************

  Synopsis    [Allocate the data structure for profile computation.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
prof_info * extraProfileWidthAllocate( DdManager * dd )
{
    prof_info * p;
    int nEntries;
    p = ALLOC( prof_info, 1 );
    memset( p, 0, sizeof(prof_info) );
    p->dd        = dd;
    nEntries     = ddMax(dd->size, dd->sizeZ) + 1;
    p->pProfile  = ALLOC( int, 3 * nEntries );
    memset( p->pProfile, 0, sizeof(int) * 3 * nEntries );
    p->pStarting = p->pProfile + nEntries;
    p->pStopping = p->pProfile + 2 * nEntries;
    p->tCof2Ref  = st_init_table(st_ptrcmp,st_ptrhash);
    return p;
}

/**Function*************************************************************

  Synopsis    [Allocate the data structure for profile computation.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraProfileWidthDelocate( prof_info * p )
{
    if ( p->pProfile )
        free( p->pProfile );
    if ( p->tCof2Ref )
        st_free_table( p->tCof2Ref );
    free( p );
}



#if 0

/**Function*************************************************************

  Synopsis    [Debugging function.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int hybBoundSet( DdManager * dd, DdNode * pbFuncs[], int nFuncs, int pBSVars[], int nBSVars )
{
    DdNode * paFuncs[1000];
    int * ProfileBdd;
//  int ProfileAdd[1000];
    int * ProfileAdd;
    int i;

    for ( i = 0; i < nFuncs; i++ )
    {
        paFuncs[i] = Cudd_BddToAdd( dd, pbFuncs[i] );               Cudd_Ref( paFuncs[i] );
    }
    Extra_ProfileWidthComputeSharing( dd, pbFuncs, nFuncs, &ProfileBdd );
    Extra_ProfileWidthComputeSharing( dd, paFuncs, nFuncs, &ProfileAdd );
//  Extra_ProfileWidthSharing( dd,        paFuncs, nFuncs, ProfileAdd, -1 );

//Extra_DumpDot( dd, pbFuncs, 1, "bdd.dot", 0 );
//Extra_DumpDot( dd, &aFunc,  1, "abd.dot", 0 );

    for ( i = 0; i <= dd->size; i++ )
    {
        assert( ProfileBdd[i] == ProfileAdd[i]  );
        printf( "Level #%3d: Width Bdd = %3d. Width Add = %3d.\n", i, ProfileBdd[i], ProfileAdd[i] );
    }

    for ( i = 0; i < nFuncs; i++ )
    {
        Cudd_RecursiveDeref( dd, paFuncs[i] );
    }

    return 0;
}

#endif
