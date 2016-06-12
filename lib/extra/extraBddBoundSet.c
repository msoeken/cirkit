/**CFile****************************************************************

  FileName    [extraBddBoundSet.c]

  PackageName [extra]

  Synopsis    [Characterization of bounds sets in the BDD. This method
  for the implicit computation of all bound sets is described in the paper:
  A. Mishchenko, X. Wang, T. Kam. A New Enhanced Constructive Decomposition 
  and Mapping Algorithm. Proc. Design Automation Conference 2003.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraBddBoundSet.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define MAX_SUPP_SIZE     32

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

// this manager is used for the computation of bound sets
static DdManager * s_dd = NULL;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/

static void extraBddEnumerateBoundSets( DdManager * dd, DdNode * bFunc, int nRange, 
    Extra_VarSets_t * pVarSets, unsigned uVarSet );

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Starts the BDD manager for bound set computation.]

  Description [Currently, these procedures can be applied to the 
  functions whose support does not exceed MAX_SUPP_SIZE = 32 
  variables.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_BoundSetsStart( int nSuppMax )
{
    assert( s_dd == NULL );
    assert( nSuppMax <= MAX_SUPP_SIZE );
    s_dd = Cudd_Init( 3 * nSuppMax, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );
}

/**Function*************************************************************

  Synopsis    [Stops the BDD manager for bound set computation.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_BoundSetsStop()
{
    assert( s_dd );
    Extra_StopManager( s_dd );
    s_dd = NULL;
}

/**Function*************************************************************

  Synopsis    [Computes *all* support-reducing bound sets of the given size.]

  Description [The function (bFuncInit) should depend on all or some 
  variable among the topmost 32 variables in the BDD manager (ddInit).
  The set of variables (bVarsToUse) specifies the variables to be 
  used in the bound sets. Only the bound sets of the given size (nVarsBSet)
  are considered. Returns the data structure (Extra_VarSets_t) containing 
  information about the bound sets. This data structure contains the number
  of all bound sets computed (pVarSets->nSets). The i-th bound set is 
  represented by the unsigned integer (pVarSets->pSets[i]), with those bits 
  set to one that correspond to the variables present in the given bound set 
  (the set of variables that can appear in the bound sets is limited by 32). 
  The number of different cofactors in a bound set is pVarSets->pSizes[i].]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Extra_VarSets_t * Extra_BoundSetsCompute( 
  DdManager * ddInit, // the manager containing the original function 
  DdNode * bFuncInit, // the function whose bound sets are computed
  DdNode * bVarsToUse,// the variables to be included into the bound sets
  int nVarsBSet )     // the number of variables in the bound sets
{
    Extra_VarSets_t * pVarSets;
    DdNode ** pbVarsA, ** pbVarsB, ** pbVarsX, ** pbComps;
    DdNode * bFunc, * bTemp, * bCubeAll, * bTuples;
    int nSupp, nRange, i, clk, clkTot;
    int * pPermute;
    unsigned uVarSet;
clkTot = clock();

    // set the size of the variable range in the computation manager
    nRange = s_dd->size/3;
    // get the number of support variables
    nSupp = Cudd_SupportSize( ddInit, bFuncInit );

    // make sure the support is not too large 
    assert( nSupp <= nRange );
    assert( nSupp < MAX_SUPP_SIZE );   // MAX_SUPP_SIZE = 32
    // here we should check that the support variables of bFuncInit
    // are concentrated among the topmost 32 variables of ddInit

    // make sure the bound set size is okay
    assert( nVarsBSet < nSupp );

    // make sure that enough vars are given
    assert( Cudd_SupportSize(ddInit,bVarsToUse) > nVarsBSet );

    // allocate room for variable sets
    pbVarsA = ALLOC( DdNode *, 6 * nRange );
    pbVarsB = pbVarsA + nRange;
    pbVarsX = pbVarsB + nRange;
    pbComps = pbVarsX + nRange;

    // set up variables and functions for bound set selection
    for ( i = 0; i < s_dd->size; i++ )
        pbComps[i] = s_dd->vars[i];
    for ( i = 0; i < nRange; i++ )
    {
        pbVarsA[i] = s_dd->vars[3*i + 0];
        pbVarsB[i] = s_dd->vars[3*i + 1];
        pbVarsX[i] = s_dd->vars[3*i + 2];
        // remap the variable only if it is allowed
        if ( Cudd_bddLeq( ddInit, bVarsToUse, ddInit->vars[i] ) ) // i-th var is included in bVarsToUse
            pbComps[i] = Cudd_bddIte( s_dd, pbVarsA[i], pbVarsB[i], pbVarsX[i] ); 
        else
            pbComps[i] = pbVarsX[i];
        Cudd_Ref( pbComps[i] );
    }

    // transfer the BDD into the computation manager
    bFunc = Extra_TransferLevelByLevel( ddInit, s_dd, bFuncInit );  Cudd_Ref( bFunc );
    // vector-compose the BDD
    bFunc = Cudd_bddVectorCompose( s_dd, bTemp = bFunc, pbComps );  Cudd_Ref( bFunc );
    Cudd_RecursiveDeref( s_dd, bTemp );     
    for ( i = 0; i < nRange; i++ )
        Cudd_RecursiveDeref( s_dd, pbComps[i] );

    // create the cube of all variables pbVarsA[] that are allowed
    bCubeAll = s_dd->one;  Cudd_Ref( bCubeAll );
    for ( i = 0; i < nRange; i++ )
    {
        if ( Cudd_bddLeq( ddInit, bVarsToUse, ddInit->vars[i] ) ) // i-th var is included in bVarsToUse
        {
            bCubeAll = Cudd_bddAnd( s_dd, bTemp = bCubeAll, pbVarsA[i] ); Cudd_Ref( bCubeAll );
            Cudd_RecursiveDeref( s_dd, bTemp );     
        }
    }
    FREE( pbVarsA );

    // get the tuples
    bTuples = Extra_bddTuples( s_dd, nVarsBSet, bCubeAll );   Cudd_Ref( bTuples );
    Cudd_RecursiveDeref( s_dd, bCubeAll );

    // restrict to the tuples
    bFunc = Cudd_bddAnd( s_dd, bTemp = bFunc, bTuples );      Cudd_Ref( bFunc );
    Cudd_RecursiveDeref( s_dd, bTemp );
    Cudd_RecursiveDeref( s_dd, bTuples );

    // permute
    pPermute = ALLOC( int, 3 * nRange );
    for ( i = 0; i < nRange; i++ )
    {
        pPermute[ 3*i + 0 ] = 0 * nRange + i;
        pPermute[ 3*i + 1 ] = 1 * nRange + i; 
        pPermute[ 3*i + 2 ] = 2 * nRange + i; 
    } 

clk = clock();
PRN( bFunc );
    // this is the most time-consuming step
    bFunc = Cudd_bddPermute( s_dd, bTemp = bFunc, pPermute );      Cudd_Ref( bFunc );
PRN( bFunc );
    Cudd_RecursiveDeref( s_dd, bTemp );
    FREE( pPermute );
PRT( "Permute   ", clock() - clk );

    // find the number of combinations of nVarsBSet out of nSupp vars
    pVarSets = ALLOC( Extra_VarSets_t, 1 );
    memset( pVarSets, 0, sizeof(Extra_VarSets_t) );
    pVarSets->nSets = Extra_NumCombinations( nVarsBSet, nSupp );
    pVarSets->pSets = ALLOC( unsigned, pVarSets->nSets );
    pVarSets->pSizes = ALLOC( char, pVarSets->nSets );

    // enumerate the combinations
    uVarSet = 0;
    pVarSets->iSet = 0;
clk = clock();
    extraBddEnumerateBoundSets( s_dd, bFunc, nRange, pVarSets, uVarSet );
PRT( "Enumerate ", clock() - clk );
//    assert( pVarSets->iSet == pVarSets->nSets );
    Cudd_RecursiveDeref( s_dd, bFunc );
PRT( "Total     ", clock() - clkTot );
    return pVarSets;
}

/**Function*************************************************************

  Synopsis    [Frees the bound set structure.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_BoundSetsFree( Extra_VarSets_t * p )
{
    FREE( p->pSets );
    FREE( p->pSizes );
    FREE( p );
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Enumerates the bound sets and counts the number of cofactors.]

  Description [Relies on the fact that all bound sets are of the same size.
  In this case, the paths written into uVarSet cannot have don't-care 
  variables.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraBddEnumerateBoundSets( DdManager * dd, DdNode * bFunc, int nRange, 
    Extra_VarSets_t * pVarSets, unsigned uVarSet )
{
    DdNode * bFR, * bF0, * bF1;   

    if ( bFunc == b0 )
        return;

    bFR = Cudd_Regular(bFunc); 
    if ( cuddI(dd, bFR->index) >= nRange )
    {
        // add this bound set
        pVarSets->pSets[ pVarSets->iSet ]  = uVarSet;
        pVarSets->pSizes[ pVarSets->iSet ] = Extra_WidthAtLevel(dd,bFunc,2*nRange);
        pVarSets->iSet++;
        return;
    }

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

    // traverse the branches
    extraBddEnumerateBoundSets( dd, bF0, nRange, pVarSets, uVarSet );

    uVarSet |=  (1 << bFR->index);
    extraBddEnumerateBoundSets( dd, bF1, nRange, pVarSets, uVarSet );
    uVarSet &= ~(1 << bFR->index);
}


////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


