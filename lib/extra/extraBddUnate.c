/**CFile****************************************************************

  FileName    [extraBddUnate.c]

  PackageName [extra]

  Synopsis    [Procedures to compute unate variables of a function.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraBddUnate.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

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

static int extraBddUnateVerify( DdManager * dd, DdNode * bFunc, DdNode * zUnate );
static int extraConstCofVarsVerify( DdManager * dd, DdNode * bFunc, int fCofValue, DdNode * zConstCofVar );

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Checks unateness of one variable.]

  Description [Returns b0(b1) if iVar is negative(positive) unate, dd->vars[0]
  if iVars is not unate, dd->vars[1] if it is both neg and pos unate (in other 
  words, if it does not depend on iVar).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddCheckVarUnate( 
  DdManager * dd,   /* the DD manager */
  DdNode * bF,
  int iVar) 
{
    // we need at least two variables for caching return values
    assert( dd->size >= 2 );
    assert( iVar < dd->size );
    return extraBddCheckVarUnate( dd, bF, dd->vars[iVar] );
} /* end of Extra_bddCheckVarUnate */


/**Function********************************************************************

  Synopsis [Returns the set of variables that are unate in all functions.]

  Description [Detect all unate variables of the set of functions function and 
  returns them as a set of ZDD singletons. A positive ZDD variable stands for a
  positive unate variable of the functions, while the negative ZDD variable stands
  for the negative unate varibale of the functions. The double set of ZDD 
  variables should be defined before this procedure is called.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode *
Extra_bddUnateSharing(
  DdManager * dd,   /* the manager */
  DdNode * pbFuncs[], /* the BDDs of the functions */
  int nFuncs )      /* the number of functions in the array */
{
    DdNode * zRes, * zTemp, * zUnate;
    DdNode * bVars;
    int i;

    assert( nFuncs > 0 );

//  bVars = Cudd_VectorSupport( dd, pbFuncs, nFuncs );  Cudd_Ref( bVars );
    bVars = Extra_VectorSupport( dd, pbFuncs, nFuncs );  Cudd_Ref( bVars );

    zRes  = NULL;
    for ( i = 0; i < nFuncs; i++ )
    if ( pbFuncs[i] )
    {
        zUnate = Extra_bddUnateSupport( dd, pbFuncs[i], bVars );  Cudd_Ref( zUnate );
        if ( zRes == NULL )
            zRes = zUnate; // takes ref
        else
        {
            zRes = Cudd_zddIntersect( dd, zTemp = zRes, zUnate ); Cudd_Ref( zRes );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zUnate );
        }
        if ( zRes == z0 )
            break;
    }
    Cudd_RecursiveDeref( dd, bVars );
    Cudd_Deref(zRes);
    return zRes;
} /* end of Extra_bddUnateSharing */


/**Function********************************************************************

  Synopsis [Returns the set of unate variables of the given function.]

  Description [Detect all unate variables of the given function and returns
  them as a set of ZDD singletons. A positive ZDD variable stands for a
  positive unate variable of the function, while the negative ZDD variable stands
  for the negative unate varibale of the function. The doubled set of ZDD 
  variables should be defined before this procedure is called.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode *
Extra_bddUnate(
  DdManager * dd,   /* the manager */
  DdNode * bFunc )  /* the function whose symmetries are computed */
{
    DdNode * zRes;
    DdNode * bVars;

    bVars = Cudd_Support( dd, bFunc );                 Cudd_Ref( bVars );
    zRes  = Extra_bddUnateSupport( dd, bFunc, bVars ); Cudd_Ref( zRes );
    Cudd_RecursiveDeref( dd, bVars );
    assert( extraBddUnateVerify( dd, bFunc, zRes ) );
    Cudd_Deref(zRes);
    return zRes;
} /* end of Extra_bddUnate */


/**Function********************************************************************

  Synopsis [Returns the set of unate variables of the function with the given support.]

  Description [Detect all unate variables of the given function and returns
  them as a set of ZDD singletons. A positive ZDD variable stands for a
  positive unate variable of the function, while the negative ZDD variable stands
  for the negative unate varibale of the function. The doubled set of ZDD 
  variables should be defined before this procedure is called.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * 
Extra_bddUnateSupport( 
  DdManager * dd,   /* the manager */
  DdNode * bFunc,   /* the function whose symmetries are computed */
  DdNode * bVars )  /* the set of variables on which this function depends */
{
    DdNode * zRes;
    do {
        dd->reordered = 0;
        zRes = extraBddUnate( dd, bFunc, bVars );
    } while (dd->reordered == 1);
    Cudd_Ref( zRes );
    assert( extraBddUnateVerify( dd, bFunc, zRes ) );
    Cudd_Deref( zRes );
    return zRes;
} /* end of Extra_bddUnateSupport */

/**Function*************************************************************

  Synopsis    [Compute the set of variables which have constant cofactors.]

  Description [Should work for any type of diagrams.]

  SideEffects []

  SeeAlso     []

***********************************************************************/
DdNode * Extra_ConstCofVarsSharing( 
  DdManager * dd,     /* the manager */
  DdNode * pbFuncs[], /* the functions whose constant cofactor variables are computed */
  int nFuncs,         /* the number of functions */
  int fCofValue )     /* the flag which shows which constant we are interested in */
{
    DdNode * zRes, * zTemp, * zConstCofVars;
    DdNode * bVars;
    int i;

    assert( nFuncs > 0 );

//  bVars = Cudd_VectorSupport( dd, pbFuncs, nFuncs );  Cudd_Ref( bVars );
    bVars = Extra_VectorSupport( dd, pbFuncs, nFuncs );  Cudd_Ref( bVars );

    zRes  = NULL;
    for ( i = 0; i < nFuncs; i++ )
    if ( pbFuncs[i] )
    {
        zConstCofVars = Extra_ConstCofVarsSupport( dd, pbFuncs[i], bVars, fCofValue );  Cudd_Ref( zConstCofVars );
        if ( zRes == NULL )
            zRes = zConstCofVars; // takes ref
        else
        {
            zRes = Cudd_zddIntersect( dd, zTemp = zRes, zConstCofVars ); Cudd_Ref( zRes );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zConstCofVars );
        }
        if ( zRes == z0 )
            break;
    }
    Cudd_RecursiveDeref( dd, bVars );
    Cudd_Deref(zRes);
    return zRes;
} /* end of Extra_ConstCofVarsSharing */

/**Function*************************************************************

  Synopsis    [Compute the set of variables which have constant cofactors.]

  Description [Should work for any type of diagrams.]

  SideEffects []

  SeeAlso     []

***********************************************************************/
DdNode *
Extra_ConstCofVars(
  DdManager * dd,   /* the manager */
  DdNode * Func,    /* the function whose constant cofactor variables are computed */
  int fCofValue )   /* the flag which shows which constant we are interested in */
{
    DdNode * zRes;
    DdNode * bSupp;
    bSupp = Cudd_Support( dd, Func );                                 Cudd_Ref( bSupp );
    zRes  = Extra_ConstCofVarsSupport( dd, Func, bSupp, fCofValue );  Cudd_Ref( zRes );
    Cudd_RecursiveDeref( dd, bSupp );
    Cudd_Deref( zRes );
    return zRes;
} /* end of Extra_ConstCofVars */



/**Function********************************************************************

  Synopsis []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode *
Extra_ConstCofVarsSupport(
  DdManager * dd,   /* the manager */
  DdNode * Func,    /* the function whose constant cofactor variables are computed */
  DdNode * bSupp,   /* the given support */
  int fCofValue )   /* the flag which shows which constant we are interested in */
{
    DdNode * zRes;
    do {
        dd->reordered = 0;
        if ( fCofValue == 0 )
            zRes = extraConst0CofVars( dd, Func, bSupp );
        else
            zRes = extraConst1CofVars( dd, Func, bSupp );
    } while (dd->reordered == 1);
    Cudd_Ref( zRes );
    assert( extraConstCofVarsVerify(dd, Func, fCofValue, zRes) );
    Cudd_Deref( zRes );
    return zRes;
} /* end of Extra_ConstCofVarsSupport */


/**Function********************************************************************

  Synopsis    [Converts a set of variables into a set of singleton subsets.]

  Description [For each BDD variable, includes both positive and negative 
  ZDD variables into the set of singletons.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddGetSingletonsBoth( 
  DdManager * dd,    /* the DD manager */
  DdNode * bVars)    /* the set of variables */
{
    DdNode * res;
    do {
        dd->reordered = 0;
        res = extraZddGetSingletonsBoth( dd, bVars );
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddGetSingletons */


/**Function*************************************************************

  Synopsis    [Computes support of the BDD as a set of ZDD singletons.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
DdNode *
Extra_SupportToSingletons(
  DdManager * dd,   /* the manager */
  DdNode * Func )   /* the function whose support is computed */
{
    DdNode * zRes;
    do {
        dd->reordered = 0;
        zRes = extraSupportToSingletons( dd, Func );
    } while (dd->reordered == 1);
    return zRes;
} /* end of Extra_SupportToSingletons */

/**Function********************************************************************

  Synopsis    [Computes the support in the case when array has NULL entries.]

  Description [Returns 1 if the function actually depends on the variables 
  in the unate set]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_VectorSupport( DdManager * dd, DdNode * pbFuncs[], int nFuncs )
{
    DdNode ** pbFuncsNew;
    DdNode * bVars;
    int i, Counter;

    Counter = 0;
    pbFuncsNew = ALLOC( DdNode *, nFuncs );
    // collect the non-zero functions into an array
    for ( i = 0; i < nFuncs; i++ )
        if ( pbFuncs[i] )
            pbFuncsNew[ Counter++ ] = pbFuncs[i];
    bVars = Cudd_VectorSupport( dd, pbFuncsNew, Counter );   

    free( pbFuncsNew );
    return bVars;   
}

/**Function*************************************************************

  Synopsis    [Allocates a one-shot array of the given size and cleans it.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int ** Extra_ArrayAllocate( int nRows, int nCols, int fClean )
{
    int ** pArray;
    int i;

    pArray = ALLOC( int *, nRows );
    pArray[0] = ALLOC( int, nRows * nCols );
    for ( i = 1; i < nRows; i++ )
        pArray[i] = pArray[i-1] + nCols;
    if ( fClean )
        memset( pArray[0], 0, sizeof(int) * nRows * nCols );

    return pArray;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs a recursive step of Extra_bddUnate.]

  Description [Returns the set of symmetric variable pairs represented as a set 
  of two-literal ZDD cubes. Both variables always appear in the positive polarity
  in the cubes. This function works without building new BDD nodes. Some relatively 
  small number of ZDD nodes may be built to ensure proper bookkeeping of the 
  symmetry information.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * 
extraBddUnate( 
  DdManager * dd,   /* the manager */
  DdNode * bFunc,   /* the function whose symmetries are computed */
  DdNode * bVars )  /* the set of variables on which this function depends */
{
    DdNode * zRes;
    DdNode * bFR = Cudd_Regular(bFunc); 

    if ( cuddIsConstant(bFR) )
        return extraZddGetSingletonsBoth( dd, bVars );
    assert( bVars != b1 );

    if ( zRes = cuddCacheLookup2Zdd(dd, extraBddUnate, bFunc, bVars) )
        return zRes;
    else
    {
        DdNode * bF0, * bF1;             
        DdNode * zRes0, * zRes1;
        DdNode * zPlus, * zTemp;
        DdNode * bVarsCur, * bVarsTemp;
        int fUnate;

        // scroll through the variables to the level of F
        for ( bVarsCur = bVars; dd->perm[bVarsCur->index] < dd->perm[bFR->index]; bVarsCur = cuddT(bVarsCur) );
        assert( dd->perm[bVarsCur->index] == dd->perm[bFR->index] );

        // cofactor the function
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

        // compute the unate variables for the cofactors
        zRes0 = extraBddUnate( dd, bF0, cuddT(bVarsCur) );
        if ( zRes0 == NULL )
            return NULL;
        cuddRef( zRes0 );

        // if there is no symmetries in the negative cofactor
        // there is no need to test the positive cofactor
        if ( zRes0 == z0 )
            zRes = zRes0;  // zRes takes reference
        else
        {
            zRes1 = extraBddUnate( dd, bF1, cuddT(bVarsCur) );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            // only those variables are pair-wise symmetric 
            // that are pair-wise symmetric in both cofactors
            // therefore, intersect the solutions
            zRes = cuddZddIntersect( dd, zRes0, zRes1 );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            Cudd_RecursiveDerefZdd( dd, zRes1 );
        }

        // determine if this variable is unate
        fUnate = 0;
        if ( Cudd_bddLeq( dd, bF0, bF1 ) )
            fUnate = 1;
        else if ( Cudd_bddLeq( dd, bF1, bF0 ) )
            fUnate = -1;


        // attach the topmost variable to the set, to get the variable pairs
        // use the positive polarity ZDD variable for the purpose

        // there is no need to do so, if zSymmVars is empty
        if ( fUnate )
        {
            zPlus = cuddZddGetNode( dd, ((fUnate>0)? 2*bFR->index: 2*bFR->index+1), z1, z0 );
            if ( zPlus == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zRes );
                return NULL;
            }
            cuddRef( zPlus );

            // add these variable pairs to the result
            zRes = cuddZddUnion( dd, zTemp = zRes, zPlus );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zTemp );
                Cudd_RecursiveDerefZdd( dd, zPlus );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zPlus );
        }

        // add those variables in the variable that are not in the support of the function
        for ( bVarsTemp = bVars; dd->perm[bVarsTemp->index] < dd->perm[bVarsCur->index]; bVarsTemp = cuddT(bVarsTemp) )
        {
            // add this variable in both polarities
            zPlus = cuddZddGetNode( dd, 2*bVarsTemp->index, z1, z0 );
            if ( zPlus == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zRes );
                return NULL;
            }
            cuddRef( zPlus );

            // add these to the result
            zRes = cuddZddUnion( dd, zTemp = zRes, zPlus );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zTemp );
                Cudd_RecursiveDerefZdd( dd, zPlus );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zPlus );

            // add this variable in both polarities
            zPlus = cuddZddGetNode( dd, 2*bVarsTemp->index+1, z1, z0 );
            if ( zPlus == NULL ) 
            {
                Cudd_RecursiveDerefZdd( dd, zRes );
                return NULL;
            }
            cuddRef( zPlus );

            // add these to the result
            zRes = cuddZddUnion( dd, zTemp = zRes, zPlus );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zTemp );
                Cudd_RecursiveDerefZdd( dd, zPlus );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zPlus );
        }
        cuddDeref( zRes );

        /* insert the result into cache */
        cuddCacheInsert2(dd, extraBddUnate, bFunc, bVars, zRes);
        return zRes;
    }
} /* end of extraBddUnate */

/**Function********************************************************************

  Synopsis    [Performs a recursive step of Extra_zddGetSingletons.]

  Description [Returns the set of ZDD singletons, containing those positive 
  polarity ZDD variables that correspond to the BDD variables in bVars.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraZddGetSingletonsBoth( 
  DdManager * dd,    /* the DD manager */
  DdNode * bVars)    /* the set of variables */
{
    DdNode * zRes;

    if ( bVars == b1 )
        return z0;

    if ( zRes = cuddCacheLookup1Zdd(dd, extraZddGetSingletonsBoth, bVars) )
        return zRes;
    else
    {
        DdNode * zTemp, * zPlus;          

        // solve subproblem
        zRes = extraZddGetSingletonsBoth( dd, cuddT(bVars) );
        if ( zRes == NULL ) 
            return NULL;
        cuddRef( zRes );
        
        // add the positive polarity variable
        zPlus = cuddZddGetNode( dd, 2*bVars->index, z1, z0 );
        if ( zPlus == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zRes );
            return NULL;
        }
        cuddRef( zPlus );

        // add these to the result
        zRes = cuddZddUnion( dd, zTemp = zRes, zPlus );
        if ( zRes == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zPlus );
            return NULL;
        }
        cuddRef( zRes );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zPlus );
        
        // add the positive negative variable
        zPlus = cuddZddGetNode( dd, 2*bVars->index+1, z1, z0 );
        if ( zPlus == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zRes );
            return NULL;
        }
        cuddRef( zPlus );

        // add these to the result
        zRes = cuddZddUnion( dd, zTemp = zRes, zPlus );
        if ( zRes == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zTemp );
            Cudd_RecursiveDerefZdd( dd, zPlus );
            return NULL;
        }
        cuddRef( zRes );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zPlus );
        cuddDeref( zRes );

        cuddCacheInsert1( dd, extraZddGetSingletonsBoth, bVars, zRes );
        return zRes;
    }
}   /* end of extraZddGetSingletonsBoth */


/**Function*************************************************************

  Synopsis    [Performs the recursive step of Extra_ConstCofVars().]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
DdNode *
extraConst0CofVars(
  DdManager * dd,            /* the manager */
  DdNode * Func,             /* the function whose constant cofactor variables are computed */
  DdNode * bVars )           /* the set of variables on which this function depends */
{
    DdNode * zRes;
    DdNode * FuncR;

    if ( Func == b1 )
        return z0;
    if ( Func == b0 || Func == a0 )
        return extraZddGetSingletonsBoth( dd, bVars );

    // scroll through the variables to the level of F
    // these variables can be skipped because at this point the function is not constant
    // so all the cofactors w.r.t. the intermediate variables are also not constant
    FuncR = Cudd_Regular(Func);
    for ( ; dd->perm[bVars->index] < dd->perm[FuncR->index]; bVars = cuddT(bVars) );
    assert( dd->perm[bVars->index] == dd->perm[FuncR->index] );

    if ( zRes = cuddCacheLookup2Zdd(dd, extraConst0CofVars, Func, bVars) )
        return zRes;
    else
    {
        DdNode * bFunc0, * bFunc1;
        DdNode * zRes0, * zRes1, * zTemp;

        // cofactor the function
        if ( FuncR != Func ) // bFunc is complemented 
        {
            bFunc0 = Cudd_Not( cuddE(FuncR) );
            bFunc1 = Cudd_Not( cuddT(FuncR) );
        }
        else
        {
            bFunc0 = cuddE(FuncR);
            bFunc1 = cuddT(FuncR);
        }

        // compute the support for cofactors
        zRes0 = extraConst0CofVars( dd, bFunc0, cuddT(bVars) );
        if ( zRes0 == NULL )
            return NULL;
        cuddRef( zRes0 );

        // if there are no constant 0 cofactor variables
        // there is no need to test the positive cofactor
        if ( zRes0 == z0 )
            zRes = zRes0;  // zRes takes reference
        else
        {
            zRes1 = extraConst0CofVars( dd, bFunc1, cuddT(bVars) );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            // make the union of these two sets
            zRes = cuddZddIntersect( dd, zRes0, zRes1 );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            Cudd_RecursiveDerefZdd( dd, zRes1 );
        }

        // add the current element to the set
        if ( bFunc0 == b0 )
        {
            zRes = cuddZddGetNode( dd, 2*FuncR->index+1, z1, zTemp = zRes );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd(dd, zTemp);
                return NULL;
            }
            cuddRef( zRes );
            cuddDeref( zTemp );
        }
        if ( bFunc1 == b0 )
        {
            zRes = cuddZddGetNode( dd, 2*FuncR->index, z1, zTemp = zRes );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd(dd, zTemp);
                return NULL;
            }
            cuddRef( zRes );
            cuddDeref( zTemp );
        }

        cuddDeref( zRes );
        cuddCacheInsert2(dd, extraConst0CofVars, Func, bVars, zRes);
        return zRes;
    }
}


/**Function*************************************************************

  Synopsis    [Performs the recursive step of Extra_ConstCofVars().]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
DdNode *
extraConst1CofVars(
  DdManager * dd,            /* the manager */
  DdNode * Func,             /* the function whose constant cofactor variables are computed */
  DdNode * bVars )           /* the set of variables on which this function depends */
{
    DdNode * zRes;
    DdNode * FuncR;

    if ( Func == b0 || Func == a0 )
        return z0;
    if ( Func == b1 )
        return extraZddGetSingletonsBoth( dd, bVars );

    // scroll through the variables to the level of F
    // these variables can be skipped because at this point the function is not constant
    // so all the cofactors w.r.t. the intermediate variables are also not constant
    FuncR = Cudd_Regular(Func);
    for ( ; dd->perm[bVars->index] < dd->perm[FuncR->index]; bVars = cuddT(bVars) );
    assert( dd->perm[bVars->index] == dd->perm[FuncR->index] );

    if ( zRes = cuddCacheLookup2Zdd(dd, extraConst1CofVars, Func, bVars) )
        return zRes;
    else
    {
        DdNode * bFunc0, * bFunc1;
        DdNode * zRes0, * zRes1, * zTemp;

        // cofactor the function
        if ( FuncR != Func ) // bFunc is complemented 
        {
            bFunc0 = Cudd_Not( cuddE(FuncR) );
            bFunc1 = Cudd_Not( cuddT(FuncR) );
        }
        else
        {
            bFunc0 = cuddE(FuncR);
            bFunc1 = cuddT(FuncR);
        }

        // compute the support for cofactors
        zRes0 = extraConst1CofVars( dd, bFunc0, cuddT(bVars) );
        if ( zRes0 == NULL )
            return NULL;
        cuddRef( zRes0 );

        // if there are no constant 0 cofactor variables
        // there is no need to test the positive cofactor
        if ( zRes0 == z0 )
            zRes = zRes0;  // zRes takes reference
        else
        {
            zRes1 = extraConst1CofVars( dd, bFunc1, cuddT(bVars) );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            // make the union of these two sets
            zRes = cuddZddIntersect( dd, zRes0, zRes1 );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            Cudd_RecursiveDerefZdd( dd, zRes1 );
        }

        // add the current element to the set
        if ( bFunc0 == b1 )
        {
            zRes = cuddZddGetNode( dd, 2*FuncR->index+1, z1, zTemp = zRes );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd(dd, zTemp);
                return NULL;
            }
            cuddRef( zRes );
            cuddDeref( zTemp );
        }
        if ( bFunc1 == b1 )
        {
            zRes = cuddZddGetNode( dd, 2*FuncR->index, z1, zTemp = zRes );
            if ( zRes == NULL ) 
            {
                Cudd_RecursiveDerefZdd(dd, zTemp);
                return NULL;
            }
            cuddRef( zRes );
            cuddDeref( zTemp );
        }

        cuddDeref( zRes );
        cuddCacheInsert2(dd, extraConst1CofVars, Func, bVars, zRes);
        return zRes;
    }
}


/**Function*************************************************************

  Synopsis    [Performs the recursive step of Extra_SupportToSingletons().]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
DdNode *
extraSupportToSingletons(
  DdManager * dd,   /* the manager */
  DdNode * Func )   /* the function whose support is computed */
{
    DdNode * zRes;
    DdNode * FuncR;

    FuncR = Cudd_Regular(Func);
    if ( cuddIsConstant(FuncR) )
        return z0;

    if ( zRes = cuddCacheLookup1Zdd(dd, extraSupportToSingletons, FuncR) )
        return zRes;
    else
    {
        DdNode * bFunc0, * bFunc1;
        DdNode * zRes0, * zRes1, * zTemp;

        // cofactor the function
        if ( FuncR != Func ) // bFunc is complemented 
        {
            bFunc0 = Cudd_Not( cuddE(FuncR) );
            bFunc1 = Cudd_Not( cuddT(FuncR) );
        }
        else
        {
            bFunc0 = cuddE(FuncR);
            bFunc1 = cuddT(FuncR);
        }

        // compute the support for cofactors
        zRes0 = extraSupportToSingletons(dd, bFunc0);
        if ( zRes0 == NULL )
            return NULL;
        cuddRef( zRes0 );
        zRes1 = extraSupportToSingletons(dd, bFunc1);
        if ( zRes1 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( zRes1 );

        // make the union of these two sets
        zRes = cuddZddUnion(dd, zRes0, zRes1);
        if ( zRes == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( zRes );
        Cudd_RecursiveDerefZdd(dd, zRes0);
        Cudd_RecursiveDerefZdd(dd, zRes1);

        // add the current element to the set
        zRes = cuddZddGetNode( dd, 2*FuncR->index, z1, zTemp = zRes );
        if ( zRes == NULL ) 
        {
            Cudd_RecursiveDerefZdd(dd, zTemp);
            return NULL;
        }
        cuddDeref( zTemp );

        // cache the result
        cuddCacheInsert1(dd, extraSupportToSingletons, FuncR, zRes);
        return zRes;
    }
}



/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_bddCheckVarUnate().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddCheckVarUnate( 
  DdManager * dd,    /* the DD manager */
  DdNode * bF,
  DdNode * bVar) 
{
    DdNode * bRes;
    DdNode * bFR = Cudd_Regular(bF); 
    int LevelF = cuddI(dd,bFR->index);
    int LevelV = dd->perm[bVar->index];

    if ( LevelF > LevelV )
        return dd->vars[1]; // can be both neg/pos unate

    if ( bRes = cuddCacheLookup2(dd, extraBddCheckVarUnate, bF, bVar) )
        return bRes;
    else
    {
        DdNode * bF0, * bF1;             
        DdNode * bRes0, * bRes1;             

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

        if ( LevelF == LevelV )
        {
            if ( Cudd_bddLeq(dd,bF0,bF1) )
                bRes = b1; // pos unate
            else if ( Cudd_bddLeq(dd,bF1,bF0) )
                bRes = b0; // neg unate
            else
                bRes = dd->vars[0]; // not unate
        }
        else // if ( LevelF < LevelV )
        {
            bRes0 = extraBddCheckVarUnate( dd, bF0, bVar );
            if ( bRes0 == dd->vars[0] )
                bRes = bRes0; // not unate
            else
            {
                bRes1 = extraBddCheckVarUnate( dd, bF1, bVar );
                if ( bRes1 == dd->vars[0] ) 
                    bRes = bRes1;  // not unate
                else if ( bRes1 == bRes0 ) 
                    bRes = bRes1;  // unateness polarity is compatible
                else if ( bRes0 == dd->vars[1] )
                    bRes = bRes1;  // neg cof is ANY; pos cof unateness is returned
                else if ( bRes1 == dd->vars[1] )
                    bRes = bRes0;  // pos cof is ANY; neg cof unateness is returned
                else
                    bRes = dd->vars[0]; // not unate
            }
        }

        cuddCacheInsert2(dd, extraBddCheckVarUnate, bF, bVar, bRes);
        return bRes;
    }
} /* end of extraBddCheckVarUnate */

/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Debugging function.]

  Description [Returns 1 if the function actually depends on the variables 
  in the unate set]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int
extraBddUnateVerify( 
  DdManager * dd,   /* the manager */
  DdNode * bFunc,   /* the function whose symmetries are computed */
  DdNode * zUnate ) /* the set of unate variables */
{
    DdNode * bSupp;
    DdNode * zTemp;
    DdNode * bF0, * bF1;
    int flag;

    flag = 1;
    bSupp = Cudd_Support( dd, bFunc ); Cudd_Ref( bSupp );
    for ( zTemp = zUnate; zTemp != z0; zTemp = cuddE(zTemp) )
    {
//      if ( !Cudd_bddLeq( dd, bSupp, dd->vars[zTemp->index/2] ) )
//          flag = 0;

        bF0 = Cudd_Cofactor( dd, bFunc, Cudd_Not(dd->vars[zTemp->index/2]) );  Cudd_Ref( bF0 );
        bF1 = Cudd_Cofactor( dd, bFunc,          dd->vars[zTemp->index/2]  );  Cudd_Ref( bF1 );

        if ( zTemp->index%2 == 0 )
        {
            if ( !Cudd_bddLeq( dd, bF0, bF1 ) )
                flag = 0;
        }
        if ( zTemp->index%2 == 1 )
        {
            if ( !Cudd_bddLeq( dd, bF1, bF0 ) )
                flag = 0;
        }

        Cudd_RecursiveDeref( dd, bF0 );
        Cudd_RecursiveDeref( dd, bF1 );

        if ( flag == 0 )
            break;

    }
    Cudd_RecursiveDeref( dd, bSupp );
    return flag;
}

/**Function********************************************************************

  Synopsis    [Debugging function.]

  Description [Returns 1 if the function actually depends on the variables 
  in the unate set]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int
extraConstCofVarsVerify( 
  DdManager * dd,   /* the manager */
  DdNode * bFunc,   /* the function whose symmetries are computed */
  int fCofValue,    /* const cof value */
  DdNode * zConstCofVar ) /* the set of unate variables */
{
    DdNode * bSupp;
    DdNode * zTemp;
    DdNode * bF0, * bF1;
    int flag;

    flag = 1;
    bSupp = Cudd_Support( dd, bFunc ); Cudd_Ref( bSupp );
    for ( zTemp = zConstCofVar; zTemp != z0; zTemp = cuddE(zTemp) )
    {
//      if ( !Cudd_bddLeq( dd, bSupp, dd->vars[zTemp->index/2] ) )
//          flag = 0;

        bF0 = Cudd_Cofactor( dd, bFunc, Cudd_Not(dd->vars[zTemp->index/2]) );  Cudd_Ref( bF0 );
        bF1 = Cudd_Cofactor( dd, bFunc,          dd->vars[zTemp->index/2]  );  Cudd_Ref( bF1 );

        if ( zTemp->index%2 == 0 )
        {
            if ( fCofValue == 0 )
            {
                if ( bF1 != b0 )
                    flag = 0;
            } 
            else // if ( fCofValue == 1 )
            {
                if ( bF1 != b1 )
                    flag = 0;
            }
        }
        if ( zTemp->index%2 == 1 )
        {
            if ( fCofValue == 0 )
            {
                if ( bF0 != b0 )
                    flag = 0;
            } 
            else // if ( fCofValue == 1 )
            {
                if ( bF0 != b1 )
                    flag = 0;
            }
        }

        Cudd_RecursiveDeref( dd, bF0 );
        Cudd_RecursiveDeref( dd, bF1 );

        if ( flag == 0 )
            break;

    }
    Cudd_RecursiveDeref( dd, bSupp );
    return flag;
}

