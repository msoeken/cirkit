/**CFile****************************************************************

  FileName    [extraAddSpectra.c]

  PackageName [extra]

  Synopsis    [Computation of Walsh, Haar, and Reed-Muller spectra from 
  the BDD. The theory underlying the current implementation is developed in 
  M.A.Thornton, D.M.Miller, R.Drechsler, "Transformations amongst the Walsh, 
  Haar, Arithmetic and Reed-Muller Spectral Domains". Proc. of RM-01.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraAddSpectra.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

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

static int s_CacheHit;
static int s_CacheMiss;

static long s_TimeLimit;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define DD_ADD_UPDATE_ZERO_CUBE_TAG	  0x2e /* former DD_BDD_COMPOSE_RECUR_TAG */
#define DD_ADD_WALSH_SUBSET_TAG   	  0x6a /* former DD_BDD_ITE_CONSTANT_TAG */
#define DD_ADD_HAAR_INVERSE_TAG       0x06 /* former DD_BDD_AND_ABSTRACT_TAG */
#define DD_ADD_ITE_GENERAL_TAG 	      0x0a /* former DD_BDD_XOR_EXIST_ABSTRACT_TAG */
// there is no harm in reusing these tags, because originally they are supposed
// to work for BDDs; even if used in their original role in the same BDD manager,
// there is no risk of cache collision, because in this file they are used for ADDs, 
// which are topologically different from BDDs


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/
static DdNode * extraCachingCube( DdManager * dd, DdNode * bVarsAll, DdNode * bVars );

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Returns the ADD of the Reed-Muller spectrum of the function represented as a BDD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddWalsh( 
  DdManager * dd,    /* the manager */
  DdNode * bFunc,    /* the function whose spectrum is being computed */
  DdNode * bVars)    /* the variables on which the function depends */
{
    DdNode	*aRes;

	s_TimeLimit = (int)(1.0 /* time in secs */ * (float)(CLOCKS_PER_SEC)) + clock();

    do 
	{
		dd->reordered = 0;
		aRes = extraBddWalsh(dd, bFunc, bVars);
		if ( clock() > s_TimeLimit )
			break;
    } 
	while (dd->reordered == 1);
    return(aRes);

} /* end of Extra_bddWalsh */


/**Function********************************************************************

  Synopsis    [Returns the ADD of the Reed-Muller spectrum of the function represented as a BDD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddReedMuller( 
  DdManager * dd,    /* the manager */
  DdNode * bFunc,    /* the function whose spectrum is being computed */
  DdNode * bVars)    /* the variables on which the function depends */
{
    DdNode	*aRes;
    do 
	{
		dd->reordered = 0;
		aRes = extraBddReedMuller(dd, bFunc, bVars);
    } 
	while (dd->reordered == 1);
    return(aRes);

} /* end of Extra_bddReedMuller */


/**Function********************************************************************

  Synopsis    [Returns the ADD of the Haar spectrum of the function represented as a BDD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddHaar( 
  DdManager * dd,    /* the manager */
  DdNode * bFunc,    /* the function whose spectrum is being computed */
  DdNode * bVars)    /* the variables on which the function depends */
{
    DdNode	*aRes;
    do 
	{
		dd->reordered = 0;
		aRes = extraBddHaar(dd, bFunc, bVars);
    } 
	while (dd->reordered == 1);
    return(aRes);

} /* end of Extra_bddHaar */

/**Function********************************************************************

  Synopsis    [Takes the ADD of Haar and returns the ADD of inverse Haar.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddHaarInverse( 
  DdManager * dd,    /* the manager */
  DdNode * aFunc,    /* the Haar that should be inverted */
  DdNode * bVars)    /* the variables on which the function depends */
{
	static int Vars[MAXINPUTS];
	static int InverseMap[MAXINPUTS];
	int k, nVars = 0;
	DdNode * bTemp;
	DdNode * aStepFirst;
    DdNode * aRes;

	/////////////////////////////////////////////////////////
	// create the array of variables in the set bVars
	for ( bTemp = bVars; bTemp != b1; bTemp = cuddT(bTemp) )
		Vars[nVars++] = bTemp->index;

	// compute the inverse mapping of variable into the one that 
	// has the same distance from the bottom as this one from the top
	k = 0;
	for ( bTemp = bVars; bTemp != b1; bTemp = cuddT(bTemp), k++ )
		InverseMap[bTemp->index] = Vars[nVars-1-k];
	/////////////////////////////////////////////////////////

	// get the constant ADD representing the first step
	aStepFirst = cuddUniqueConst( dd, 0.0 ); Cudd_Ref(aStepFirst);

    do 
	{
		dd->reordered = 0;
		aRes = extraBddHaarInverse(dd, aFunc, aStepFirst, bVars, bVars, nVars, InverseMap);
    } 
	while (dd->reordered == 1);

	Cudd_RecursiveDeref( dd, aStepFirst );
    return(aRes);

} /* end of Extra_bddHaarInverse */

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_addRemapNatural2Sequential( DdManager * dd, DdNode * aSource, DdNode * bVars )
{
	static int Vars[MAXINPUTS];
	static int Permute[MAXINPUTS];
	int k, nVars = 0;
	DdNode * bTemp;

	// create the array of variables in the set bVars
	for ( bTemp = bVars; bTemp != b1; bTemp = cuddT(bTemp) )
		Vars[nVars++] = bTemp->index;

	k = 0;
	for ( bTemp = bVars; bTemp != b1; bTemp = cuddT(bTemp), k++ )
		Permute[bTemp->index] = Vars[nVars-1-k];

	return Cudd_addPermute( dd, aSource, Permute );
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddWalsh().]

  Description [Generates in a bottom-up fashion an ADD for all spectral
               coefficients of the functions represented by a BDD.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode* extraBddWalsh( 
  DdManager * dd,    /* the manager */
  DdNode * bFunc,    /* the function whose spectrum is being computed */
  DdNode * bVars)    /* the variables on which the function depends */
{
	DdNode * aRes;
    statLine(dd); 

	if ( clock() > s_TimeLimit )
		return NULL;

	/* terminal cases */
	if ( bVars == b1 )
	{
		assert( Cudd_IsConstant(bFunc) );
		if ( bFunc == b0 )
			return a1;
		else
			return Cudd_addConst(dd,-1.0);
	}

    /* check cache */
//	if ( bFunc->ref != 1 )
  if ( ( aRes = cuddCacheLookup2(dd, extraBddWalsh, bFunc, bVars) ) )
	{
	    s_CacheHit++;
    	return aRes;
	}
	else
	{
		DdNode * bFunc0,  * bFunc1;   /* cofactors of the function */
		DdNode * aWalsh0, * aWalsh1;   /* partial solutions of the problem */
		DdNode * aRes0,   * aRes1;    /* partial results to be composed by ITE */
		DdNode * bFuncR = Cudd_Regular(bFunc); /* the regular pointer to the function */

	    s_CacheMiss++;

	    /* bFunc cannot depend on a variable that is not in bVars */
		assert( cuddI(dd,bFuncR->index) >= cuddI(dd,bVars->index) );


		/* cofactor the BDD */
		if ( bFuncR->index == bVars->index )
		{
			if ( bFuncR != bFunc ) /* bFunc is complemented */
			{
				bFunc0 = Cudd_Not( cuddE(bFuncR) );
				bFunc1 = Cudd_Not( cuddT(bFuncR) );
			}
			else
			{
				bFunc0 = cuddE(bFuncR);
				bFunc1 = cuddT(bFuncR);
			}
		}
		else /* bVars is higher in the variable order */
			bFunc0 = bFunc1 = bFunc;


		/* solve subproblems */
		aWalsh0 = extraBddWalsh( dd, bFunc0, cuddT(bVars) );
		if ( aWalsh0 == NULL )
			return NULL;
		cuddRef( aWalsh0 );

		aWalsh1 = extraBddWalsh( dd, bFunc1, cuddT(bVars) );
		if ( aWalsh1 == NULL )
		{
			Cudd_RecursiveDeref( dd, aWalsh0 );
			return NULL;
		}
		cuddRef( aWalsh1 );


		/* compute  aRes0 = aWalsh0 + aWalsh1 */
		aRes0 = cuddAddApplyRecur( dd, Cudd_addPlus, aWalsh0, aWalsh1 );
		if ( aRes0 == NULL )
		{
			Cudd_RecursiveDeref( dd, aWalsh0 );
			Cudd_RecursiveDeref( dd, aWalsh1 );
			return NULL;
		}
		cuddRef( aRes0 );

		/* compute  aRes1 = aWalsh0 - aWalsh1 */
		aRes1 = cuddAddApplyRecur( dd, Cudd_addMinus, aWalsh0, aWalsh1 );
		if ( aRes1 == NULL )
		{
			Cudd_RecursiveDeref( dd, aWalsh0 );
			Cudd_RecursiveDeref( dd, aWalsh1 );
			Cudd_RecursiveDeref( dd, aRes0 );
			return NULL;
		}
		cuddRef( aRes1 );

		Cudd_RecursiveDeref(dd, aWalsh0);
		Cudd_RecursiveDeref(dd, aWalsh1);


		/* only aRes0 and aRes1 are referenced at this point */

		/* consider the case when Res0 and Res1 are the same node */
		aRes = (aRes1 == aRes0) ? aRes1 : cuddUniqueInter( dd, bVars->index, aRes1, aRes0 );
		if (aRes == NULL) 
		{
			Cudd_RecursiveDeref(dd, aRes1);
			Cudd_RecursiveDeref(dd, aRes0);
			return NULL;
		}
		cuddDeref(aRes1);
		cuddDeref(aRes0);

		/* insert the result into cache */
//		if ( bFunc->ref != 1 )
		cuddCacheInsert2(dd, extraBddWalsh, bFunc, bVars, aRes);
		return aRes;
	}
} /* end of extraBddWalsh */

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddReedMuller().]

  Description [Generates in a bottom-up fashion an ADD for all spectral
               coefficients of the functions represented by a BDD.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode* extraBddReedMuller( 
  DdManager * dd,    /* the manager */
  DdNode * bFunc,    /* the function whose spectrum is being computed */
  DdNode * bVars)    /* the variables on which the function depends */
{
	DdNode * aRes;
    statLine(dd); 

	/* terminal cases */
	if ( bVars == b1 )
	{
		assert( Cudd_IsConstant(bFunc) );
		if ( bFunc == b0 )
			return a0;
		else
			return a1;
	}

    /* check cache */
  if ( ( aRes = cuddCacheLookup2(dd, extraBddReedMuller, bFunc, bVars) ) )
    	return aRes;
	else
	{
		DdNode * bFunc0, * bFunc1;   /* cofactors of the function */
		DdNode * aRes0,  * aRes1;    /* partial results to be composed by ITE */
		DdNode * bFuncR = Cudd_Regular(bFunc); /* the regular pointer to the function */
		DdNode * aTemp;

		/* bFunc cannot depend on a variable that is not in bVars */
		assert( cuddI(dd,bFuncR->index) >= cuddI(dd,bVars->index) );


		/* cofactor the BDD */
		if ( bFuncR->index == bVars->index )
		{
			if ( bFuncR != bFunc ) /* bFunc is complemented */
			{
				bFunc0 = Cudd_Not( cuddE(bFuncR) );
				bFunc1 = Cudd_Not( cuddT(bFuncR) );
			}
			else
			{
				bFunc0 = cuddE(bFuncR);
				bFunc1 = cuddT(bFuncR);
			}
		}
		else /* bVars is higher in the variable order */
			bFunc0 = bFunc1 = bFunc;


		/* solve subproblems */
		aRes0 = extraBddReedMuller( dd, bFunc0, cuddT(bVars) );
		if ( aRes0 == NULL )
			return NULL;
		cuddRef( aRes0 );

		aRes1 = extraBddReedMuller( dd, bFunc1, cuddT(bVars) );
		if ( aRes1 == NULL )
		{
			Cudd_RecursiveDeref( dd, aRes0 );
			return NULL;
		}
		cuddRef( aRes1 );

		/* compute  aRes1 = aRes1 (+) aRes0 */
		aRes1 = cuddAddApplyRecur( dd, Cudd_addXor, aTemp = aRes1, aRes0 );
		if ( aRes1 == NULL )
		{
			Cudd_RecursiveDeref( dd, aRes0 );
			Cudd_RecursiveDeref( dd, aTemp );
			return NULL;
		}
		cuddRef( aRes1 );
		Cudd_RecursiveDeref(dd, aTemp);


		/* only aRes0 and aRes1 are referenced at this point */

		/* consider the case when Res0 and Res1 are the same node */
		aRes = (aRes1 == aRes0) ? aRes1 : cuddUniqueInter( dd, bVars->index, aRes1, aRes0 );
		if (aRes == NULL) 
		{
			Cudd_RecursiveDeref(dd, aRes1);
			Cudd_RecursiveDeref(dd, aRes0);
			return NULL;
		}
		cuddDeref(aRes1);
		cuddDeref(aRes0);

		/* insert the result into cache */
		cuddCacheInsert2(dd, extraBddReedMuller, bFunc, bVars, aRes);
		return aRes;
	}
} /* end of extraBddReedMuller */

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddHaar().]

  Description [Generates in a bottom-up fashion an ADD for all spectral
               coefficients of the functions represented by a BDD.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode* extraBddHaar( 
  DdManager * dd,    /* the manager */
  DdNode * bFunc,    /* the function whose spectrum is being computed */
  DdNode * bVars)    /* the variables on which the function depends */
{
	DdNode * aRes;
    statLine(dd); 

	/* terminal cases */
	if ( bVars == b1 )
	{
		assert( Cudd_IsConstant(bFunc) );
		if ( bFunc == b0 )
			return a0;
		else
			return a1;
	}

    /* check cache */
//	if ( bFunc->ref != 1 )
    if ( aRes = cuddCacheLookup2(dd, extraBddHaar, bFunc, bVars) )
    	return aRes;
	else
	{
		DdNode * bFunc0, * bFunc1;   /* cofactors of the function */
		DdNode * aHaar0, * aHaar1;   /* partial solutions of the problem */
		DdNode * aNode0, * aNode1;   /* the special terminal nodes */
		DdNode * aRes0,  * aRes1;    /* partial results to be composed by ITE */
		DdNode * bFuncR = Cudd_Regular(bFunc); /* the regular pointer to the function */
		DdNode * aTemp;
		double   dValue0, dValue1;

		/* bFunc cannot depend on a variable that is not in bVars */
		assert( cuddI(dd,bFuncR->index) >= cuddI(dd,bVars->index) );


		/* cofactor the BDD */
		if ( bFuncR->index == bVars->index )
		{
			if ( bFuncR != bFunc ) /* bFunc is complemented */
			{
				bFunc0 = Cudd_Not( cuddE(bFuncR) );
				bFunc1 = Cudd_Not( cuddT(bFuncR) );
			}
			else
			{
				bFunc0 = cuddE(bFuncR);
				bFunc1 = cuddT(bFuncR);
			}
		}
		else /* bVars is higher in the variable order */
			bFunc0 = bFunc1 = bFunc;


		/* solve subproblems */
		aHaar0 = extraBddHaar( dd, bFunc0, cuddT(bVars) );
		if ( aHaar0 == NULL )
			return NULL;
		cuddRef( aHaar0 );

		aHaar1 = extraBddHaar( dd, bFunc1, cuddT(bVars) );
		if ( aHaar1 == NULL )
		{
			Cudd_RecursiveDeref( dd, aHaar0 );
			return NULL;
		}
		cuddRef( aHaar1 );

		/* retrieve the terminal values in aHaar0 and aHaar1 */
		for ( aTemp = aHaar0; aTemp->index != CUDD_CONST_INDEX; aTemp = cuddE(aTemp) );
		dValue0 = cuddV( aTemp );
		for ( aTemp = aHaar1; aTemp->index != CUDD_CONST_INDEX; aTemp = cuddE(aTemp) );
		dValue1 = cuddV( aTemp );

		/* get the new terminal nodes */
		aNode0 = cuddUniqueConst( dd, dValue0 + dValue1 );
		if ( aNode0 == NULL )
		{
			Cudd_RecursiveDeref( dd, aHaar0 );
			Cudd_RecursiveDeref( dd, aHaar1 );
			return NULL;
		}
		cuddRef( aNode0 );

		aNode1 = cuddUniqueConst( dd, dValue0 - dValue1 );
		if ( aNode1 == NULL )
		{
			Cudd_RecursiveDeref( dd, aHaar0 );
			Cudd_RecursiveDeref( dd, aHaar1 );
			Cudd_RecursiveDeref( dd, aNode0 );
			return NULL;
		}
		cuddRef( aNode1 );


		/* replace the terminal nodes in the cofactor ADDs */
		aRes0 = extraAddUpdateZeroCubeValue( dd, aHaar0, cuddT(bVars), aNode0  );
		if ( aRes0 == NULL )
		{
			Cudd_RecursiveDeref( dd, aHaar0 );
			Cudd_RecursiveDeref( dd, aHaar1 );
			Cudd_RecursiveDeref( dd, aNode0 );
			Cudd_RecursiveDeref( dd, aNode1 );
			return NULL;
		}
		cuddRef( aRes0 );

		aRes1 = extraAddUpdateZeroCubeValue( dd, aHaar1, cuddT(bVars), aNode1  );
		if ( aRes1 == NULL )
		{
			Cudd_RecursiveDeref( dd, aHaar0 );
			Cudd_RecursiveDeref( dd, aHaar1 );
			Cudd_RecursiveDeref( dd, aNode0 );
			Cudd_RecursiveDeref( dd, aNode1 );
			Cudd_RecursiveDeref( dd, aRes0 );
			return NULL;
		}
		cuddRef( aRes1 );

		Cudd_RecursiveDeref(dd, aHaar0);
		Cudd_RecursiveDeref(dd, aHaar1);

		Cudd_RecursiveDeref(dd, aNode0);
		Cudd_RecursiveDeref(dd, aNode1);


		/* only aRes0 and aRes1 are referenced at this point */

		/* consider the case when Res0 and Res1 are the same node */
		aRes = (aRes1 == aRes0) ? aRes1 : cuddUniqueInter( dd, bVars->index, aRes1, aRes0 );
		if (aRes == NULL) 
		{
			Cudd_RecursiveDeref(dd, aRes1);
			Cudd_RecursiveDeref(dd, aRes0);
			return NULL;
		}
		cuddDeref(aRes1);
		cuddDeref(aRes0);

		/* insert the result into cache */
//		if ( bFunc->ref != 1 )
		cuddCacheInsert2(dd, extraBddHaar, bFunc, bVars, aRes);
		return aRes;
	}
} /* end of extraBddHaar */

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddHaarInverse().]

  Description [Generates in a bottom-up fashion an ADD for the inverse Haar.]

  SideEffects [The third cached argument (bSteps) is the BDD of the elementary variable
  whose index equal to the number of lazy steps made thus far plus one. On the top-most
  level it is 0, next it is 1, etc.]

  SeeAlso     []

******************************************************************************/
DdNode * extraBddHaarInverse( 
  DdManager * dd,    /* the manager */
  DdNode * aFunc,    /* the function whose spectrum is being computed */
  DdNode * aSteps,   /* the index of this variable indicates the number of previous lazy recursive calls */
  DdNode * bVars,    /* the variables, on which the function depends */
  DdNode * bVarsAll, /* the set of all variables, which will never change through the calls */
  int      nVarsAll, /* the number of vars in the set */
  int    * InverseMap ) /* the variable map mapping the var index into its inverse var index */
{
	DdNode * aRes;
	DdNode * bCacheCube;
    statLine(dd); 

	/* terminal cases */
	if ( bVars == b1 )
	{ // return a terminal node with a value equal to cuddV(aFunc) * 2^(nSteps-1)
		if ( cuddV(aSteps) == 0.0 )
			return cuddUniqueConst( dd, cuddV(aFunc) ); 
		else
			return cuddUniqueConst( dd, cuddV(aFunc) * Extra_Power2( (int)(cuddV(aSteps)-1) ) ); 
	}

    /* check cache */
    /* the last two arguments are derivitives, therefore there are useless for caching */
	/* the other two arguments (bVars and bVarsAll) can be combined into one argument */
	bCacheCube = extraCachingCube( dd, bVarsAll, bVars );  Cudd_Ref( bCacheCube );
    if ( aRes = cuddCacheLookup(dd, DD_ADD_HAAR_INVERSE_TAG, aFunc, aSteps, bCacheCube) )
	{
		Cudd_RecursiveDeref( dd, bCacheCube );
   		return aRes;
	}
	else
	{
		DdNode * aFunc0, * aFunc1;   /* cofactors of the function */
		DdNode * aInvH0, * aInvH1;   /* partial solutions of the problem */
		DdNode * aRes0,  * aRes1;    /* partial results to be composed by ITE */
		DdNode * aStepNext;

		/* aFunc cannot depend on a variable that is not in bVars */
		assert( cuddI(dd,aFunc->index) >= cuddI(dd,bVars->index) );

		/* cofactor the ADD */
		if ( aFunc->index == bVars->index )
		{
			aFunc0 = cuddE(aFunc);
			aFunc1 = cuddT(aFunc);
		}
		else /* bVars is higher in the variable order */
			aFunc0 = aFunc1 = aFunc;


		if ( cuddV(aSteps) > 0.0 ) /* meaning that it is a lazy call */
		{
			/* solve subproblems */
			aStepNext = cuddUniqueConst( dd, cuddV(aSteps)+1 );
			if ( aStepNext == NULL )
				return NULL;
			cuddRef( aStepNext );

			aInvH0 = extraBddHaarInverse( dd, aFunc0, aStepNext, cuddT(bVars), bVarsAll, nVarsAll, InverseMap );
			if ( aInvH0 == NULL )
			{
				Cudd_RecursiveDeref( dd, aStepNext );
				return NULL;
			}
			cuddRef( aInvH0 );

			aInvH1 = extraBddHaarInverse( dd, aFunc1, aStepNext, cuddT(bVars), bVarsAll, nVarsAll, InverseMap );
			if ( aInvH1 == NULL )
			{
				Cudd_RecursiveDeref( dd, aStepNext );
				Cudd_RecursiveDeref( dd, aInvH0 );
				return NULL;
			}
			cuddRef( aInvH1 );
			Cudd_RecursiveDeref( dd, aStepNext );

			aRes0 = aInvH0;
			aRes1 = aInvH1;
		}
		else // if ( cuddV(aSteps) == 0.0 )
		{
			/* solve subproblems */
			aInvH0 = extraBddHaarInverse( dd, aFunc0, aSteps, cuddT(bVars), bVarsAll, nVarsAll, InverseMap );
			if ( aInvH0 == NULL )
				return NULL;
			cuddRef( aInvH0 );

			aStepNext = cuddUniqueConst( dd, 1.0 );
			if ( aStepNext == NULL )
			{
				Cudd_RecursiveDeref( dd, aInvH0 );
				return NULL;
			}
			cuddRef( aStepNext );

			aInvH1 = extraBddHaarInverse( dd, aFunc1, aStepNext, cuddT(bVars), bVarsAll, nVarsAll, InverseMap );
			if ( aInvH1 == NULL )
			{
				Cudd_RecursiveDeref( dd, aStepNext );
				Cudd_RecursiveDeref( dd, aInvH0 );
				return NULL;
			}
			cuddRef( aInvH1 );
			Cudd_RecursiveDeref( dd, aStepNext );


			/* compute  aRes0 = aWalsh0 + aWalsh1 */
			aRes0 = cuddAddApplyRecur( dd, Cudd_addPlus, aInvH0, aInvH1 );
			if ( aRes0 == NULL )
			{
				Cudd_RecursiveDeref( dd, aInvH0 );
				Cudd_RecursiveDeref( dd, aInvH1 );
				return NULL;
			}
			cuddRef( aRes0 );

			/* compute  aRes1 = aWalsh0 - aWalsh1 */
			aRes1 = cuddAddApplyRecur( dd, Cudd_addMinus, aInvH0, aInvH1 );
			if ( aRes1 == NULL )
			{
				Cudd_RecursiveDeref( dd, aInvH0 );
				Cudd_RecursiveDeref( dd, aInvH1 );
				Cudd_RecursiveDeref( dd, aRes0 );
				return NULL;
			}
			cuddRef( aRes1 );

			Cudd_RecursiveDeref(dd, aInvH0);
			Cudd_RecursiveDeref(dd, aInvH1);
		}

		/* only aRes0 and aRes1 are referenced at this point */

		/* consider the case when Res0 and Res1 are the same node */
		aRes = extraAddIteRecurGeneral( dd, dd->vars[ InverseMap[bVars->index] ], aRes1, aRes0 );
		if (aRes == NULL) 
		{
			Cudd_RecursiveDeref(dd, aRes1);
			Cudd_RecursiveDeref(dd, aRes0);
			return NULL;
		}
		cuddRef( aRes );
		Cudd_RecursiveDeref(dd, aRes1);
		Cudd_RecursiveDeref(dd, aRes0);
		cuddDeref( aRes );

		/* insert the result into cache */
		cuddCacheInsert(dd, DD_ADD_HAAR_INVERSE_TAG, aFunc, aSteps, bCacheCube, aRes);
		Cudd_RecursiveDeref( dd, bCacheCube );
		return aRes;
	}
} /* end of extraBddHaarInverse */


/**Function********************************************************************

  Synopsis    [Replaces the negative variable assignment node in the ADD by the given value.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraAddUpdateZeroCubeValue(
  DdManager * dd, 
  DdNode * aFunc,    /* the ADD to be updated */
  DdNode * bVars,
  DdNode * aNode )  /* the terminal node representing the required value */
{
	DdNode * aRes;
    statLine(dd); 

	/* terminal cases */
	if ( bVars == b1 )
	{
		assert( Cudd_IsConstant(aFunc) );
		return aNode;
	}

    /* check cache */
    if ( aRes = cuddCacheLookup(dd, DD_ADD_UPDATE_ZERO_CUBE_TAG, aFunc, bVars, aNode) )
	{ 
		s_CacheHit++;
   		return aRes;
	}
	else
	{
		DdNode * aFunc0, * aFunc1;    /* cofactors */
		DdNode * aRes0,  * aRes1;     /* partial results to be composed by ITE */

		s_CacheMiss++;

		if ( aFunc->index == bVars->index )
		{
			aFunc0 = cuddE( aFunc );
			aFunc1 = cuddT( aFunc );
		}
		else
			aFunc0 = aFunc1 = aFunc;


		aRes0  = extraAddUpdateZeroCubeValue( dd, aFunc0, cuddT(bVars), aNode );
		if ( aRes0 == NULL )
			return NULL;
		cuddRef( aRes0 );

		aRes1 = aFunc1;
//		cuddRef( aRes1 );

		/* only aRes0 and aRes1 are referenced at this point */

		/* consider the case when Res0 and Res1 are the same node */
		aRes = (aRes1 == aRes0) ? aRes1 : cuddUniqueInter( dd, bVars->index, aRes1, aRes0 );
		if (aRes == NULL) 
		{
//			Cudd_RecursiveDeref(dd, aRes1);
			Cudd_RecursiveDeref(dd, aRes0);
			return NULL;
		}
//		cuddDeref(aRes1);
		cuddDeref(aRes0);

		/* insert the result into cache */
		cuddCacheInsert(dd, DD_ADD_UPDATE_ZERO_CUBE_TAG, aFunc, bVars, aNode, aRes);
		return aRes;
	}
} /* end of extraAddUpdateZeroCubeValue */


/**Function********************************************************************

  Synopsis    [Implements the recursive step of Cudd_addIteGeneral(f,g,h).]

  Description [Implements the recursive step of Cudd_addIteGeneral(f,g,h),
  meaning that g and h are not supposed to be 0-1 ADDs but may have more terminals.
  Applying arithmetic addition in the terminal case. Returns a pointer to the 
  resulting ADD if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_addIte]

******************************************************************************/
DdNode * extraAddIteRecurGeneral( DdManager * dd, DdNode * bX, DdNode * aF, DdNode * aG )
{
	DdNode * aRes;
	statLine( dd );

	assert( !Cudd_IsConstant(bX) );
	assert(  cuddE(bX) == b0 && cuddT(bX) == b1 ); /* the elementary variable */

    /* check cache */
    if ( aRes = cuddCacheLookup(dd, DD_ADD_ITE_GENERAL_TAG, bX, aF, aG) )
    	return aRes;
	else
	{
		DdNode * aF0, * aF1, * aG0, * aG1;
		int LevelF, LevelG, LevelX, LevelTop;

		LevelF = cuddI(dd,aF->index);
		LevelG = cuddI(dd,aG->index);
		LevelX = dd->perm[bX->index];
		LevelTop = ddMin(LevelF, LevelG);
		LevelTop = ddMin(LevelX, LevelTop);

		if ( LevelF == LevelTop )
		{
			aF0 = cuddE(aF);
			aF1 = cuddT(aF);
		}
		else
			aF0 = aF1 = aF;

		if ( LevelG == LevelTop )
		{
			aG0 = cuddE(aG);
			aG1 = cuddT(aG);
		}
		else
			aG0 = aG1 = aG;

		if ( LevelX == LevelTop )
		{
			assert( LevelX < LevelF );
			assert( LevelX < LevelG );

			/* consider the case when Res0 and Res1 are the same node */
			aRes = (aF == aG) ? aF : cuddUniqueInter( dd, bX->index, aF, aG );
			if (aRes == NULL) 
				return NULL;
		}
		else
		{
			DdNode * aRes0,  * aRes1;     /* partial results to be composed by ITE */

			aRes0  = extraAddIteRecurGeneral( dd, bX, aF0, aG0 );
			if ( aRes0 == NULL )
				return NULL;
			cuddRef( aRes0 );

			aRes1  = extraAddIteRecurGeneral( dd, bX, aF1, aG1 );
			if ( aRes1 == NULL )
			{
				Cudd_RecursiveDeref(dd, aRes0);
				return NULL;
			}
			cuddRef( aRes1 );

			/* only aRes0 and aRes1 are referenced at this point */

			/* consider the case when Res0 and Res1 are the same node */
			aRes = (aRes1 == aRes0) ? aRes1 : cuddUniqueInter( dd, dd->invperm[LevelTop], aRes1, aRes0 );
			if (aRes == NULL) 
			{
				Cudd_RecursiveDeref(dd, aRes1);
				Cudd_RecursiveDeref(dd, aRes0);
				return NULL;
			}
			cuddDeref(aRes1);
			cuddDeref(aRes0);
		}

		cuddCacheInsert( dd, DD_ADD_ITE_GENERAL_TAG, bX, aF, aG, aRes );
		return aRes;
	}
}	/* end of extraAddIteRecurGeneral */



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Computes a caching cube.]

  Description [It is known that bVars is a cube representing a subset of bVarsAll
  this function computes the canonical representation of the pair (bVarsAll, bVars) 
  by one DD node the convention is the following: if ( bVars == bVarsAll), return bVarsAll,
  otherwise return ITE(x, cuddT(bVarsAll), bVars), where x is the topmost var in bVarsAll.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraCachingCube( DdManager * dd, DdNode * bVarsAll, DdNode * bVars )
{
	if ( bVars == bVarsAll)  
		return bVarsAll;
	else
		return cuddUniqueInter( dd, bVarsAll->index, cuddT(bVarsAll), bVars );
	// because bVarsAll is positive cube, cuddT(bVarsAll) is never complemented
}



/*

void Experiment( BFunc * pFunc )
{
	int i;
	int nVars = pFunc->nInputs;
	int nOuts = pFunc->nOutputs;
	DdManager * dd = pFunc->dd;
	long clk;

	DdNode * aSpectrum[MAXOUTPUTS];
	DdNode * aModified[MAXOUTPUTS];
	DdNode * aInverse[MAXOUTPUTS];
	DdNode * bTest;
	DdNode * bSupp;
//	DdNode * bTemp;
	// introduce an additional variable
	DdNode * bVarTemp = Cudd_bddNewVar(dd);

//	if ( nVars < 5 )
//	{
//		PRK(pFunc->pOutputs[0],nVars);
//	}


	clk = clock();
	for ( i = 0; i < nOuts; i++ )
	{
		bSupp = Cudd_Support( dd, pFunc->pOutputs[i] );  Cudd_Ref( bSupp ); 


		aSpectrum[i] = Extra_bddHaar( dd, pFunc->pOutputs[i], bSupp );   Cudd_Ref( aSpectrum[i] );
		aModified[i] = Extra_addRemapNatural2Sequential( dd, aSpectrum[i], bSupp ); Cudd_Ref( aModified[i] );
		aInverse[i]  = Extra_bddHaarInverse( dd, aModified[i], bSupp );   Cudd_Ref( aInverse[i] );

		bTest = Cudd_addBddPattern( dd, aInverse[i] );  Cudd_Ref( bTest );
		if ( bTest != pFunc->pOutputs[i] )
			printf( "Output #%d: Verification FAILED!\n", i );
		else
			printf( "Output #%d: Verification okay!\n", i );
		Cudd_RecursiveDeref( dd, bTest );


		Cudd_RecursiveDeref( dd, bSupp );
		fprintf( stderr, "." );
		fflush( stderr );
	}


	{ // visualize the ADD 
//		FILE* pFileDot = fopen( "FuncADD.dot", "w" );
//		Cudd_DumpDot( dd, 1, &aSpectrum2[0], NULL, NULL, pFileDot );
//		fclose( pFileDot );
	}


//	printf( "\n" ) << "s_CacheHit = %d", s_CacheHit );
//	printf( "s_CacheMiss = %d", s_CacheMiss );
	printf( "\n" );
//	printf( "\n" );
	printf( "Spectrum generation time = %.2f sec\n", (float)(clock() - clk)/(float)(CLOCKS_PER_SEC) );
	printf( "Shared BDD node count = %d\n", Cudd_SharingSize(pFunc->pOutputs,nOuts) );
	printf( "Shared ADD node count = %d\n", Cudd_SharingSize(aSpectrum,nOuts) );
	printf( "\n" );
	printf( "Statistics for the all outputs:\n" );
	printf( "The number of different coefficients = %d\n", Extra_addCountConstArray(dd,aSpectrum,nOuts) );
	printf( "The smallest coefficient = %.1f\n", cuddV(Extra_addFindMinArray(dd,aSpectrum,nOuts)) );
	printf( "The largest coefficient = %.1f\n", cuddV(Extra_addFindMaxArray(dd,aSpectrum,nOuts)) );
	printf( "\n" );


	for ( i = 0; i < nOuts; i++ )
	{
		Cudd_RecursiveDeref( dd, aSpectrum[i] );
		Cudd_RecursiveDeref( dd, aModified[i] );
		Cudd_RecursiveDeref( dd, aInverse[i] );
	}

	return;
}
*/

////////////////////////////////////////////////////////////////////////
///                           END OF FILE                            ///
////////////////////////////////////////////////////////////////////////

