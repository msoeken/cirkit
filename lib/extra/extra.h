/**CFile****************************************************************

  FileName    [extra.h]

  PackageName [extra]

  Synopsis    [Experimental version of some DD-based procedures.]

  Description [This library contains a number of operators and 
  traversal routines developed to extend the functionality of 
  CUDD v.2.3.x, by Fabio Somenzi (http://vlsi.colorado.edu/~fabio/)
  To compile your code with the library, #include "extra.h" 
  in your source files and link your project to CUDD and this 
  library. Use the library at your own risk and with caution. 
  Note that debugging of some operators still continues.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extra.h,v 1.10 2003/09/01 04:56:18 alanmi Exp $]

***********************************************************************/

#ifndef __EXTRA_H__
#define __EXTRA_H__

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include <string.h>
#include <time.h>
#include "util.h"
#include "st.h"
#include "cuddInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

// an assuption about the maximum number of BDD variables
#define MAXINPUTS   1000       

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

/* constants of the manager */
#define     b0     Cudd_Not((dd)->one)
#define     b1              (dd)->one
#define     z0              (dd)->zero
#define     z1              (dd)->one
#define     a0              (dd)->zero
#define     a1              (dd)->one


// hash key macros
#define hashKey1(a,TSIZE) \
((unsigned)(a) % TSIZE)

#define hashKey2(a,b,TSIZE) \
(((unsigned)(a) + (unsigned)(b) * DD_P1) % TSIZE)

#define hashKey3(a,b,c,TSIZE) \
(((((unsigned)(a) + (unsigned)(b)) * DD_P1 + (unsigned)(c)) * DD_P2 ) % TSIZE)

#define hashKey4(a,b,c,d,TSIZE) \
((((((unsigned)(a) + (unsigned)(b)) * DD_P1 + (unsigned)(c)) * DD_P2 + \
   (unsigned)(d)) * DD_P3) % TSIZE)

#define hashKey5(a,b,c,d,e,TSIZE) \
(((((((unsigned)(a) + (unsigned)(b)) * DD_P1 + (unsigned)(c)) * DD_P2 + \
   (unsigned)(d)) * DD_P3 + (unsigned)(e)) * DD_P1) % TSIZE)


// complementation and testing for pointers for hash entries
#define Hash_IsComplement(p)  ((int)((long) (p) & 02))
#define Hash_Regular(p)       ((DdNode*)((unsigned)(p) & ~02))
#define Hash_Not(p)           ((DdNode*)((long)(p) ^ 02))
#define Hash_NotCond(p,c)     ((DdNode*)((long)(p) ^ (02*c)))


// print the BDD of the function in the SOP form
#define PRB(dd,f)       printf("%s = ", #f); Extra_bddPrint(dd,f); printf("\n")
// print the ZDD of the function in the SOP form
#define PRZ(dd,f)       printf("%s = \n", #f); Cudd_zddPrintCover(dd,f); 
// print the number of nodes in the BDD
#define PRN(f)          printf("Nodes in %s = %d\n", #f, Cudd_DagSize(f))
// print the support of the BDD
#define PRS(dd,f)       printf("Support of %s = ", #f ); Extra_PrintSupport( (dd),(f) ); printf("\n")
// print the Karnaugh map of the completely-specified function
#define PRK(dd,f,n)     printf("K-map for function <%s> is:\n", #f); Extra_PrintKMap(stdout,dd,(f),Cudd_Not(f),(n),NULL,0)
// print the Karnaugh map of the incompletely-specified function
#define PRK2(dd,f,g,n)  printf("K-map for function <%s> is:\n", #f); Extra_PrintKMap(stdout,dd,(f),(g),(n),NULL,0)
// print the time spent by the process "a"
#define PRT(a,t)        printf( "%s = ", (a) ); printf( "%6.2f sec\n", (float)(t)/(float)(CLOCKS_PER_SEC) )

#define CALLOC(type, num)  ((type *) calloc((long)(num), (long)sizeof(type)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


/*===========================================================================*/
/*    ADD-based procedures                                                   */
/*===========================================================================*/

/*=== extraAddMisc.c =============================================================*/

/* remaps the function to depend on the topmost variables on the manager. */
extern DdNode *    Extra_addRemapUp( DdManager * dd, DdNode * aF );
/* counts the number of different constant nodes of the ADD */
extern int         Extra_addCountConst( DdManager * dd, DdNode * aFunc );
/* counts the the number of different constant nodes of the array of ADDs */
extern int         Extra_addCountConstArray( DdManager * dd, DdNode ** paFuncs, int nFuncs );
/* finds the minimum value terminal node in the array of ADDs */
extern DdNode *    Extra_addFindMinArray( DdManager * dd, DdNode ** paFuncs, int nFuncs );
/* finds the maximum value terminal node in the array of ADDs */
extern DdNode *    Extra_addFindMaxArray( DdManager * dd, DdNode ** paFuncs, int nFuncs );
/* absolute value of an ADD */
extern DdNode *    Extra_addAbs( DdManager * dd, DdNode * f );
/* the encoded set of absolute values of the constant nodes of an ADD */
extern DdNode *    Extra_bddAddConstants( DdManager * dd, DdNode * aFunc );
/* determines whether this is an ADD or a BDD */
extern int         Extra_addDetectAdd( DdManager * dd, DdNode * Func );
/* restructure the ADD by replacing negative terminals with their abs value */
extern DdNode *    Extra_addAbsCudd( DdManager * dd, DdNode * f );
/* Coudert's restrict applicable to true ADDs (takes the don't-care set, not the care set). */
extern DdNode *    Extra_addRestrictAdd( DdManager * dd, DdNode * aF, DdNode * aD );
extern DdNode *     extraAddRestrictAdd( DdManager * dd, DdNode * aF, DdNode * aD );
/* intersection, union, and sharp of paths for each terminal */
extern DdNode *    Extra_addForeachTerminalAnd( DdManager * dd, DdNode ** f, DdNode ** g );
extern DdNode *    Extra_addForeachTerminalOr( DdManager * dd, DdNode ** f, DdNode ** g );
extern DdNode *    Extra_addForeachTerminalSharp( DdManager * dd, DdNode ** f, DdNode ** g );
/* swapping a given terminal node with zero terminal node */
extern DdNode *    Extra_addSwapTerminals( DdManager * dd, DdNode * aFunc, DdNode * aTerm );
extern DdNode *     extraAddSwapTerminals( DdManager * dd, DdNode * aFunc, DdNode * aTerm );

/*=== extraAddSpectra.c =============================================================*/

/* Walsh spectrum computation */
extern DdNode *    Extra_bddWalsh( DdManager * dd, DdNode * bFunc, DdNode * bVars );
extern DdNode *     extraBddWalsh( DdManager * dd, DdNode * bFunc, DdNode * bVars );
/* Reed-Muller spectrum computation */
extern DdNode *    Extra_bddReedMuller( DdManager * dd, DdNode * bFunc, DdNode * bVars );
extern DdNode *     extraBddReedMuller( DdManager * dd, DdNode * bFunc, DdNode * bVars );
/* Haar spectrum computation */
extern DdNode *    Extra_bddHaar( DdManager * dd, DdNode * bFunc, DdNode * bVars );
extern DdNode *     extraBddHaar( DdManager * dd, DdNode * bFunc, DdNode * bVars );
/* inverse Haar spectrum computation */
extern DdNode *    Extra_bddHaarInverse( DdManager * dd, DdNode * aFunc, DdNode * bVars );
extern DdNode *     extraBddHaarInverse( DdManager * dd, DdNode * aFunc, DdNode * aSteps, DdNode * bVars, DdNode * bVarsAll, int nVarsAll, int * InverseMap );
/* remapping from the natural order into the sequential order */
extern DdNode *    Extra_addRemapNatural2Sequential( DdManager * dd, DdNode * aSource, DdNode * bVars );
/* addition function to update the negative-variable-assignment cube value in the ADD */
extern DdNode *     extraAddUpdateZeroCubeValue( DdManager * dd, DdNode * aFunc, DdNode * bVars, DdNode * aNode );
/* the generalized ITE for ADDs */
extern DdNode *     extraAddIteRecurGeneral( DdManager * dd, DdNode * bX, DdNode * aF, DdNode * aG );



/*===========================================================================*/
/*    BDD-based procedures                                                   */
/*===========================================================================*/

/*=== extraBddAuto.c =================================================================*/

/* computes the linear space from the BDD representing the function */
extern DdNode *    Extra_bddSpaceFromFunctionFast( DdManager * dd, DdNode * bFunc );
extern DdNode *    Extra_bddSpaceFromFunction( DdManager * dd, DdNode * bF, DdNode * bG );
extern DdNode *     extraBddSpaceFromFunction( DdManager * dd, DdNode * bF, DdNode * bG );
extern DdNode *    Extra_bddSpaceFromFunctionPos( DdManager * dd, DdNode * bFunc );
extern DdNode *     extraBddSpaceFromFunctionPos( DdManager * dd, DdNode * bFunc );
extern DdNode *    Extra_bddSpaceFromFunctionNeg( DdManager * dd, DdNode * bFunc );
extern DdNode *     extraBddSpaceFromFunctionNeg( DdManager * dd, DdNode * bFunc );
/* computes the set of canonical variables of the linear space */
extern DdNode *    Extra_bddSpaceCanonVars( DdManager * dd, DdNode * bSpace );
extern DdNode *     extraBddSpaceCanonVars( DdManager * dd, DdNode * bSpace );
/* computes the reduction equations */
extern DdNode *    Extra_bddSpaceEquations( DdManager * dd, DdNode * bSpace );
extern DdNode *    Extra_bddSpaceEquationsNeg( DdManager * dd, DdNode * bSpace );
extern DdNode *     extraBddSpaceEquationsNeg( DdManager * dd, DdNode * bSpace );
extern DdNode *    Extra_bddSpaceEquationsPos( DdManager * dd, DdNode * bSpace );
extern DdNode *     extraBddSpaceEquationsPos( DdManager * dd, DdNode * bSpace );
/* computes the linear space from matrix */
extern DdNode *    Extra_bddSpaceFromMatrixPos( DdManager * dd, DdNode * zA );
extern DdNode *     extraBddSpaceFromMatrixPos( DdManager * dd, DdNode * zA );
extern DdNode *    Extra_bddSpaceFromMatrixNeg( DdManager * dd, DdNode * zA );
extern DdNode *     extraBddSpaceFromMatrixNeg( DdManager * dd, DdNode * zA );
/* other utilities */
extern DdNode *    Extra_bddSpaceReduce( DdManager * dd, DdNode * bFunc, DdNode * bCanonVars );
extern DdNode *    * Extra_bddSpaceExorGates( DdManager * dd, DdNode * bFuncRed, DdNode * zEquations );
/* ZDD based procedures */
extern DdNode *    Extra_zddSpaceFromMatrixPos( DdManager * dd, DdNode * zA );
extern DdNode *     extraZddSpaceFromMatrixPos( DdManager * dd, DdNode * zA );
extern DdNode *    Extra_zddSpaceFromMatrixNeg( DdManager * dd, DdNode * zA );
extern DdNode *     extraZddSpaceFromMatrixNeg( DdManager * dd, DdNode * zA );
extern DdNode *    Extra_zddSpaceFromMatrixPosInternal( DdManager * dd, DdNode * zA, int levelPrev );
extern DdNode *    Extra_zddSpaceFromMatrixNegInternal( DdManager * dd, DdNode * zA, int levelPrev );

/*=== extraBddBoundSet.c =================================================================*/

typedef struct Extra_VarSets_t_ Extra_VarSets_t;
struct Extra_VarSets_t_
{
    int         nSets;  // the number of bound sets in this collection
    unsigned *  pSets;  // the bound sets represented as bit strings
    char *      pSizes; // the number of different cofactors for each bound set
    int         iSet;   // the set the is being added
};

extern Extra_VarSets_t * Extra_BoundSetsCompute( DdManager * ddInit, DdNode * bFuncInit, DdNode * bVarsToUse, int nVarsBSet );
extern void        Extra_BoundSetsStart( int nSuppMax );
extern void        Extra_BoundSetsStop();
extern void        Extra_BoundSetsFree( Extra_VarSets_t * p );

/*=== extraBddDistance.c =============================================================*/

/* decomposition of the function into a set of functions that have Humming distance > 1. */
extern int         Extra_bddDistanceDecompose( DdManager * dd, DdNode * bFunc, DdNode * pbRes[], int resLimit );
extern DdNode *    Extra_bddDistance1Iter( DdManager * dd, DdNode * bFunc, DdNode * bCore );
extern DdNode *    Extra_bddDistance1( DdManager * dd, DdNode * bFunc, DdNode * bCore );
extern DdNode *     extraBddDistance1( DdManager * dd, DdNode * bFunc, DdNode * bCore );

/*=== extraBddKmap.c ================================================================*/

/* displays the Karnaugh Map of a function */
extern void        Extra_PrintKMap( FILE * pFile, DdManager * dd, DdNode * OnSet, DdNode * OffSet, int nVars, DdNode ** XVars, int fSuppType, char ** pVarNames );
/* displays the Karnaugh Map of a relation */
extern void        Extra_PrintKMapRelation( FILE * pFile, DdManager * dd, DdNode * OnSet, DdNode * OffSet, int nXVars, int nYVars, DdNode ** XVars, DdNode ** YVars );

/*=== extraBddMisc.c ===============================================================*/

/* remaps the function to depend on the topmost variables of the manager. */
extern DdNode *    Extra_bddRemapUp( DdManager * dd, DdNode * bF );
/* finds one cube with the smallest integer value */
extern int         Extra_bddPickOneCube( DdManager * dd, DdNode * bF, char * string );
/* finds one cube belonging to the on-set of the function */
extern DdNode *    Extra_bddFindOneCube( DdManager * dd, DdNode * bF );
/* finds one minterm belonging to the on-set of the function */
extern DdNode *    Extra_bddFindOneMinterm( DdManager * dd, DdNode * bF, int nVars );
/* finds one cube belonging to the on-set, with the smallest bits-to-integer value */
extern DdNode *    Extra_bddGetOneCube( DdManager * dd, DdNode * bFunc );
/* finds one minterm belonging to the on-set, with the smallest bits-to-integer value */
extern DdNode *    Extra_bddGetOneMinterm( DdManager * dd, DdNode * bFunc, DdNode * bSupp );

/* compute the random function with the given density */
extern DdNode *    Extra_bddRandomFunc( DdManager * dd, int n, double d );
/* build the set of all tuples of K variables out of N */
extern DdNode *    Extra_bddTuples( DdManager * dd, int K, DdNode * bVarsN );
extern DdNode *     extraBddTuples( DdManager * dd, DdNode * bVarsK, DdNode * bVarsN );
/* changes the polarity of vars listed in the cube */
extern DdNode *    Extra_bddChangePolarity( DdManager * dd, DdNode * bFunc, DdNode * bVars );
extern DdNode *     extraBddChangePolarity( DdManager * dd, DdNode * bFunc, DdNode * bVars );

/* converts the bit-string (nbits <= 32) into the BDD cube */
extern DdNode *    Extra_bddBitsToCube( DdManager * dd, int Code, int CodeWidth, DdNode ** pbVars );
/* computes the positive polarty cube composed of the first vars in the array */
extern DdNode *    Extra_bddComputeCube( DdManager * dd, DdNode ** bXVars, int nVars );
/* performs the boolean difference w.r.t to a cube (AKA unique quontifier) */
extern DdNode *    Extra_bddBooleanDiffCube( DdManager * dd, DdNode * bFunc, DdNode * bCube );
extern DdNode *     extraBddBooleanDiffCube( DdManager * dd, DdNode * bFunc, DdNode * bCube );


/* find the profile of a DD (the number of nodes on each level) */
extern int *       Extra_ProfileNode( DdManager * dd, DdNode * F, int * Profile );
/* find the profile of a shared set of DDs (the number of nodes on each level) */
extern int *       Extra_ProfileNodeSharing( DdManager * dd, DdNode ** pFuncs, int nFuncs, int * Profile );

/* find the profile of a DD (the number of edges of each length) */
extern int *       Extra_ProfileEdge( DdManager * dd, DdNode * F, int * Profile );
/* find the profile of a shared set of DDs (the number of edges of each length) */
extern int *       Extra_ProfileEdgeSharing( DdManager * dd, DdNode ** pFuncs, int nFuncs, int * Profile );
/* permutes variables the array of functions */
extern void        Extra_bddPermuteArray( DdManager * dd, DdNode ** bNodesIn, DdNode ** bNodesOut, int nNodes, int *permut );
/* permutes variables in the BDD and returns an ADD */
extern DdNode *    Extra_bddPermuteToAdd( DdManager * dd, DdNode * bFunc, int * Permute );

/* checks if the DD has at least one complement edge */
extern int         Extra_WithComplementedEdges( DdNode * bFunc );

/*=== extraBddPermute.c =================================================================*/

typedef struct Extra_PermMan_t_  Extra_PermMan_t; // the permutation manager data structure

/* starting and stopping the permutation manager */
extern Extra_PermMan_t * Extra_PermutationManagerInit();
extern void        Extra_PermutationManagerQuit( Extra_PermMan_t * p );
/* set/reset the key increase limit */
extern void        Extra_PermutationManagerKeyIncreaseLimitSet( Extra_PermMan_t * p, int Limit );
extern void        Extra_PermutationManagerKeyIncreaseLimitReset( Extra_PermMan_t * p );

/* permuting the BDD in a way that is more efficient than Cudd_bddPermute() */
extern DdNode *    Extra_Permute( Extra_PermMan_t * pMan, DdManager * dd, DdNode * Func, int * pPermute );
extern DdNode *    Extra_RemapUp( Extra_PermMan_t * pMan, DdManager * dd, DdNode * Func );

/*=== extraBddSigma.c =================================================================*/

extern void        Extra_SigmaCountMinterm( DdManager * dd, DdNode * F, unsigned pSigma0[], unsigned pSigma1[] );
extern DdNode *    Extra_bddSigma1Total( DdManager * dd, DdNode * bFunc, int Level );
extern DdNode *     extraBddSigma1Total( DdManager * dd, DdNode * bFunc, DdNode * bLevel );

/*=== extraBddSymm.c =================================================================*/

typedef struct Extra_SymmInfo_t_  Extra_SymmInfo_t;
struct Extra_SymmInfo_t_ {
    int nVars;      // the number of variables in the support
    int nVarsMax;   // the number of variables in the DD manager
    int nSymms;     // the number of pair-wise symmetries
    int nNodes;     // the number of nodes in a ZDD (if applicable)
    int * pVars;    // the list of all variables present in the support
    char ** pSymms; // the symmetry information
};

/* computes the classical symmetry information for the function - recursive */
extern Extra_SymmInfo_t *  Extra_SymmPairsCompute( DdManager * dd, DdNode * bFunc );
/* computes the classical symmetry information for the function - using naive approach */
extern Extra_SymmInfo_t *  Extra_SymmPairsComputeNaive( DdManager * dd, DdNode * bFunc );
extern int         Extra_bddCheckVarsSymmetricNaive( DdManager * dd, DdNode * bF, int iVar1, int iVar2 );

/* allocates the data structure */
extern Extra_SymmInfo_t *  Extra_SymmPairsAllocate( int nVars );
/* deallocates the data structure */
extern void        Extra_SymmPairsDissolve( Extra_SymmInfo_t * );
/* print the contents the data structure */
extern void        Extra_SymmPairsPrint( Extra_SymmInfo_t * );
/* converts the ZDD into the Extra_SymmInfo_t structure */
extern Extra_SymmInfo_t *  Extra_SymmPairsCreateFromZdd( DdManager * dd, DdNode * zPairs, DdNode * bVars );

/* computes the classical symmetry information as a ZDD */
extern DdNode *    Extra_zddSymmPairsCompute( DdManager * dd, DdNode * bF, DdNode * bVars );
extern DdNode *     extraZddSymmPairsCompute( DdManager * dd, DdNode * bF, DdNode * bVars );
/* returns a singleton-set ZDD containing all variables that are symmetric with the given one */
extern DdNode *    Extra_zddGetSymmetricVars( DdManager * dd, DdNode * bF, DdNode * bG, DdNode * bVars );
extern DdNode *     extraZddGetSymmetricVars( DdManager * dd, DdNode * bF, DdNode * bG, DdNode * bVars );
/* converts a set of variables into a set of singleton subsets */
extern DdNode *    Extra_zddGetSingletons( DdManager * dd, DdNode * bVars );
extern DdNode *     extraZddGetSingletons( DdManager * dd, DdNode * bVars );
/* filters the set of variables using the support of the function */
extern DdNode *    Extra_bddReduceVarSet( DdManager * dd, DdNode * bVars, DdNode * bF );
extern DdNode *     extraBddReduceVarSet( DdManager * dd, DdNode * bVars, DdNode * bF );

/* checks the possibility that the two vars are symmetric */
extern int         Extra_bddCheckVarsSymmetric( DdManager * dd, DdNode * bF, int iVar1, int iVar2 );
extern DdNode *     extraBddCheckVarsSymmetric( DdManager * dd, DdNode * bF, DdNode * bVars );

/*=== extraBddSupp.c =================================================================*/

/* returns the size of the support */
extern int         Extra_bddSuppSize( DdManager * dd, DdNode * bSupp );
/* returns 1 if the support contains the given BDD variable */
extern int         Extra_bddSuppContainVar( DdManager * dd, DdNode * bS, DdNode * bVar );
/* returns 1 if two supports represented as BDD cubes are overlapping */
extern int         Extra_bddSuppOverlapping( DdManager * dd, DdNode * S1, DdNode * S2 );
/* returns the number of different vars in two supports */
extern int         Extra_bddSuppDifferentVars( DdManager * dd, DdNode * S1, DdNode * S2, int DiffMax );
/* checks the support containment */
extern int         Extra_bddSuppCheckContainment( DdManager * dd, DdNode * bL, DdNode * bH, DdNode ** bLarge, DdNode ** bSmall );

/* get support of the DD as an array of integers */
extern int *       Extra_SupportArray( DdManager * dd, DdNode * F, int * support );
/* get support of the array of DDs as an array of integers */
extern int *       Extra_VectorSupportArray( DdManager * dd, DdNode ** F, int n, int * support );
/* get support size and node count at the same time */
extern int         Extra_DagSizeSuppSize( DdNode * node, int * pnSuppSize );
/* get support of support and stores it in cache */
extern DdNode *    Extra_SupportCache( DdManager * dd, DdNode * f );
/* get support of the DD as a ZDD */
extern DdNode *    Extra_zddSupport( DdManager * dd, DdNode * f );
/* get support as a negative polarity BDD cube */
extern DdNode *    Extra_bddSupportNegativeCube( DdManager * dd, DdNode * f );

/*=== extraBddUnate.c =============================================================*/

/* check if the given variable is unate */
extern DdNode *    Extra_bddCheckVarUnate( DdManager * manager, DdNode * bF, int iVar );
extern DdNode *     extraBddCheckVarUnate( DdManager * manager, DdNode * bF, DdNode * bVar );

/* compute the set of unate variables */
extern DdNode *    Extra_bddUnateSharing( DdManager * dd, DdNode * pbFuncs[], int nFuncs );
extern DdNode *    Extra_bddUnateSupport( DdManager * dd, DdNode * bFunc, DdNode * bVars );
extern DdNode *    Extra_bddUnate( DdManager * dd, DdNode * bFunc );
extern DdNode *     extraBddUnate( DdManager * dd, DdNode * bFunc, DdNode * bVars );
/* converts a set of variables into a set of singleton subsets */
extern DdNode *    Extra_zddGetSingletonsBoth( DdManager * dd, DdNode * bVars );
extern DdNode *     extraZddGetSingletonsBoth( DdManager * dd, DdNode * bVars );
/* vector support when array contains NULL entries */
extern DdNode *    Extra_VectorSupport( DdManager * dd, DdNode * pbFuncs[], int nFuncs );

/* compute the set of constant cofactors */
extern DdNode *    Extra_ConstCofVarsSharing( DdManager * dd, DdNode * pFuncs[], int nFuncs, int fCofValue );
extern DdNode *    Extra_ConstCofVars( DdManager * dd, DdNode * Func, int fCofValue  );
extern DdNode *    Extra_ConstCofVarsSupport( DdManager * dd, DdNode * Func, DdNode * bSupp, int fCofValue );
extern DdNode *     extraConst0CofVars( DdManager * dd, DdNode * Func, DdNode * bVars );
extern DdNode *     extraConst1CofVars( DdManager * dd, DdNode * Func, DdNode * bVars );
/* compute the support of the function as the set of ZDD singletons */
extern DdNode *    Extra_SupportToSingletons( DdManager * dd, DdNode * Func );
extern DdNode *     extraSupportToSingletons( DdManager * dd, DdNode * Func );
/* allocate the rectangular arra */
extern int **      Extra_ArrayAllocate( int nRows, int nCols, int fClean );

/*=== extraBddWidth.c =============================================================*/

/* width computation which works for BDDs with complemented edges */
extern int         Extra_ProfileWidthComputeSharing( DdManager * dd, DdNode * Funcs[], int nFuncs, int ** ppProfile );
extern int         Extra_ProfileWidthCompute( DdManager * dd, DdNode * Func, int ** ppProfile );
/* similar computation for only one level in the BDD */
extern int         Extra_ProfileWidthComputeAtLevelSharing( DdManager * dd, DdNode * Funcs[], int nFuncs, int CutLevel );
extern int         Extra_ProfileWidthComputeAtLevel( DdManager * dd, DdNode * Func, int CutLevel );
/* collect the set of different cofactors at the given level in the BDD */
extern st_table *  Extra_CofactorsComputeAtLevelSharing( DdManager * dd, DdNode * Funcs[], int nFuncs, int CutLevel );
extern st_table *  Extra_CofactorsComputeAtLevel( DdManager * dd, DdNode * Func, int CutLevel );

/*===========================================================================*/
/*    Mixed-DD procedures                                                    */
/*===========================================================================*/

/*=== extraDdMinterm.c =============================================================*/

typedef struct Extra_MintCache_t_ Extra_MintCache_t;

extern Extra_MintCache_t * Extra_MintermCacheStart( DdManager * dd, int nSize );
extern void        Extra_MintermCacheClean( Extra_MintCache_t * pCache );
extern void        Extra_MintermCacheStop( Extra_MintCache_t * pCache );
extern double      Extra_CountMinterm( Extra_MintCache_t * pCache, DdNode * F, int nVars );
extern double      Extra_CountMintermProduct( Extra_MintCache_t * pCache, DdNode * F, DdNode * G, int nVars );
extern double      Extra_CountMintermExorCare( Extra_MintCache_t * pCache, DdNode * F, DdNode * G, DdNode * C, int nVars );

/*=== extraDdMisc.c =============================================================*/

/* verification procedures */
extern int         Extra_zddVerifyCover( DdManager * dd, DdNode * zC, DdNode * bFuncOn, DdNode * bFuncOnDc );
extern int         Extra_bddVerifyRange( DdManager * dd, DdNode * bFunc, DdNode * bLower, DdNode * bUpper );
/* procedure to quit the manager and check for any nondereferenced nodes */
extern void        Extra_StopManager( DdManager * dd );
/* collect the nodes in the given DD */
extern st_table *  Extra_CollectNodes( DdNode * Func );
/* collect the nodes in the given set of DDs */
extern st_table *  Extra_CollectNodesSharing( DdNode * Funcs[], int nFuncs );
/* collect the nodes under the cut */
extern st_table *  Extra_CollectNodesUnderCut( DdManager * dd, DdNode * Func, int Level );

/*=== extraDdNodePath.c =================================================================*/

typedef struct Extra_NodeSet_t_     Extra_NodeSet_t;
struct Extra_NodeSet_t_
{
    DdManager * dd;          // the DD manager
    int         Level;       // the number of levels for which the profile is computed
    // information about the nodes
    int         nNodes;      // the number of nodes in the diagram
    int *       pnNodes;     // the profile (the number of nodes on each level plus the nodes below the last level)
    int *       pnNodesCopy; // the profile (the number of nodes on each level plus the nodes below the last level)
    DdNode ***  pbNodes;     // the nodes found on each level plus the nodes below the last level 
    // information about the paths
    int         nPaths;      // the number of nodes in the diagram
    int *       pnPaths;     // the profile (the number of nodes on each level plus the nodes below the last level)
    int *       pnPathsCopy; // the profile (the number of nodes on each level plus the nodes below the last level)
    DdNode ***  pbPaths;     // the path to each node plus the paths below the last level (referenced)
    DdNode ***  pbCofs;      // the cofactors for each path plus the paths below the last level 
};

extern int               Extra_WidthAtLevel    ( DdManager * dd, DdNode * bF, int Level );
extern int               Extra_CofactorsAtLevel( DdManager * dd, DdNode * bF, int Level, DdNode * pbCofs[] );
extern Extra_NodeSet_t * Extra_NodeProfile     ( DdManager * dd, DdNode * bF, int Level, int fCollect );
extern Extra_NodeSet_t * Extra_NodePaths       ( DdManager * dd, DdNode * bF, int Level );
extern void              Extra_NodeSetDeref    ( Extra_NodeSet_t * p );

/*=== extraDdPrint.c =================================================================*/

/* prints the bdd in the form of disjoint sum of products */
extern void        Extra_bddPrint( DdManager * dd, DdNode * F );
/* visualize the BDD/ADD/ZDD */
extern void        Extra_bddShow( DdManager * dd, DdNode * bFunc );
extern void        Extra_addShowFromBdd( DdManager * dd, DdNode * bFunc );
extern void        Extra_DumpDot( DdManager * dd, DdNode * pFuncs[], int nFuncs, char * FileName, int fFlagZdd );

enum { 
    EXTRA_GATE_TYPE_NONE, 
    EXTRA_GATE_TYPE_BUF, 
    EXTRA_GATE_TYPE_INV, 
    EXTRA_GATE_TYPE_AND, 
    EXTRA_GATE_TYPE_NAND, 
    EXTRA_GATE_TYPE_OR, 
    EXTRA_GATE_TYPE_NOR, 
    EXTRA_GATE_TYPE_EXOR, 
    EXTRA_GATE_TYPE_NEXOR,
    EXTRA_GATE_TYPE_CROSS 
};

/* write the BDD/ZDD into as the boolean node in the BLIF file. */
extern void        Extra_WriteBlifNode( FILE * pFile, DdManager * dd, DdNode * Func, char * pInputNames[], char * pOutputName );
extern void        Extra_WriteBlifNodeUsingGates( FILE * pFile, DdManager * dd, DdNode * Func, char * pInputNames[], char * pOutputName, int fCascade );
extern void        Extra_WriteBlifGate( FILE * pFile, char * pInputNames[], int Polars[], int nInputs, char * pOutputName, int GateType );
extern void        Extra_WriteBlifGateCascade( FILE * pFile, char * pInputNames[], int Polars[], int nInputs, char * pOutputName, int GateType );

extern FILE *      Extra_WriteBlifStart( char * FileName, char * pInputNames[], int nInputs, char * pOutputNames[], int nOutputs );
extern void        Extra_WriteBlifStop( FILE * pFile );

extern unsigned    Extra_ReadBinary( char * Buffer );
extern void        Extra_PrintBinary( FILE * pFile, unsigned Sign[], int nBits );
extern void        Extra_PrintSupport( DdManager * dd, DdNode * Func );
extern void        Extra_PrintSymbols( FILE * pFile, char Char, int nTimes, int fPrintNewLine );

/* writes the BDD into a BLIF file as one SOP block */
extern void        Extra_WriteFunctionSop( DdManager * dd, DdNode * bFunc, DdNode * bFuncDc, char ** pNames, char * OutputName, char * FileName );
extern void         extraWriteFunctionSop( FILE * pFile, DdManager * dd, DdNode * zCover, int levPrev, int nLevels, const char * AddOn, int * VarMask );
/* writes the {A,B}DD into a BLIF file as a network of MUXes */
extern void        Extra_WriteFunctionMuxes( DdManager * dd, DdNode * bFunc, char ** pNames, char * OutputName, char * FileName );
extern void         extraWriteFunctionMuxes( FILE * pFile, DdNode * Func, char * OutputName, char * Prefix, char ** InputNames );

/*=== extraDdShift.c =================================================================*/

/* shifts the BDD up/down by one variable */
extern DdNode *    Extra_bddShift( DdManager * dd, DdNode * bF, int fShiftUp );
extern DdNode *     extraBddShift( DdManager * dd, DdNode * bF, DdNode * bFlag );
/* moves the BDD up/down by the given number of variables */
extern DdNode *    Extra_bddMove( DdManager * dd, DdNode * bF, int fShiftUp );
extern DdNode *     extraBddMove( DdManager * dd, DdNode * bF, DdNode * bFlag );
/* stretches the BDD the given number of times  */
extern DdNode *    Extra_bddStretch( DdManager * dd, DdNode * bF, int nTimes );
extern DdNode *     extraBddStretch( DdManager * dd, DdNode * bF, DdNode * bTimes );
/* shifts the ZDD up/down by one variable */
extern DdNode *    Extra_zddShift( DdManager * dd, DdNode * zF, int fShiftUp );
extern DdNode *     extraZddShift( DdManager * dd, DdNode * zF, DdNode * zFlag );
/* swaps two variables in the BDD */
extern DdNode *    Extra_bddSwapVars( DdManager * dd, DdNode * bF, int iVar1, int iVar2 );
extern DdNode *     extraBddSwapVars( DdManager * dd, DdNode * bF, DdNode * bVars );

/*=== extraDdTimed.c =================================================================*/

/* sets the timeout and spaceout for Extra_bddAnd(), Extra_bddOr(), and Extra_bddBooleanDiff() */
extern void        Extra_OperationTimeoutSet( int timeout );
extern void        Extra_OperationSpaceoutSet( int MaxNodeIncrease );
extern void        Extra_OperationTimeoutReset();
extern void        Extra_OperationSpaceoutReset();

/* the same as normal operators only with timeout and spaceout */
extern DdNode *    Extra_bddAnd( DdManager * dd, DdNode * f, DdNode * g );
extern DdNode *    Extra_bddOr( DdManager * dd, DdNode * f, DdNode * g );
extern DdNode *     extraBddAndRecur( DdManager * manager, DdNode * f, DdNode * g );
extern DdNode *    Extra_bddBooleanDiff( DdManager * manager, DdNode * f, int x );
extern DdNode *     extraBddBooleanDiffRecur( DdManager * manager, DdNode * f, DdNode * var );
extern DdNode *    Extra_bddVectorCompose( DdManager * dd, DdNode * f, DdNode ** vector );
extern DdNode *    Extra_zddIsopCoverAltTimed( DdManager * dd, DdNode * bFuncOn, DdNode * bFuncOnDc );
extern DdNode *    Extra_zddIsopCoverAltLimited( DdManager * dd, DdNode * bFuncOn, DdNode * bFuncOnDc, int nBTracks );

/*=== extraDdTransfer.c =================================================================*/

/* convert a {A,B}DD from a manager to another with variable remapping */
extern DdNode *    Extra_TransferPermute( DdManager * ddSource, DdManager * ddDestination, DdNode * f, int * Permute );
extern DdNode *    Extra_TransferLevelByLevel( DdManager * ddSource, DdManager * ddDestination, DdNode * f );

/*===========================================================================*/
/*     Various Utilities                                                     */
/*===========================================================================*/

/*=== extraUtilFile.c ========================================================*/

extern int         Extra_FileNameCheckExtension( char * FileName, char * Extension );
extern char *      Extra_FileNameAppend( char * pBase, char * pSuffix );
extern char *      Extra_FileNameGeneric( char * FileName );

/*=== extraUtilMemory.c ========================================================*/

typedef struct Extra_MmFixed_t_    Extra_MmFixed_t;    
typedef struct Extra_MmFlex_t_     Extra_MmFlex_t;     

// fixed-size-block memory manager
extern Extra_MmFixed_t *  Extra_MmFixedStart( int nEntrySize, int nChunkSize, int nChunksAlloc );
extern void        Extra_MmFixedStop( Extra_MmFixed_t * p, int fVerbose );
extern char *      Extra_MmFixedEntryFetch( Extra_MmFixed_t * p );
extern void        Extra_MmFixedEntryRecycle( Extra_MmFixed_t * p, char * pEntry );
extern void        Extra_MmFixedRestart( Extra_MmFixed_t * p );
extern int         Extra_MmFixedReadMemUsage( Extra_MmFixed_t * p );
// flexible-size-block memory manager
extern Extra_MmFlex_t * Extra_MmFlexStart( int nChunkSize, int nChunksAlloc );
extern void        Extra_MmFlexStop( Extra_MmFlex_t * p, int fVerbose );
extern char *      Extra_MmFlexEntryFetch( Extra_MmFlex_t * p, int nBytes );
extern int         Extra_MmFlexReadMemUsage( Extra_MmFlex_t * p );

/*=== extraUtilMisc.c ========================================================*/

/* finds the smallest integer larger of equal than the logarithm. */
extern int         Extra_Base2Log( unsigned Num );
extern int         Extra_Base2LogDouble( double Num );
/* returns the power of two as a double */
extern double      Extra_Power2( int Num );
extern int         Extra_Power3( int Num );
/* the number of combinations of k elements out of n */
extern int         Extra_NumCombinations( int k, int n  );
extern int *       Extra_DeriveRadixCode( int Number, int Radix, int nDigits );

/*=== extraUtilProgress.c ================================================================*/

typedef struct ProgressBarStruct ProgressBar;

extern ProgressBar * Extra_ProgressBarStart( FILE * pFile, int nItemsTotal );
extern void        Extra_ProgressBarUpdate( ProgressBar * p, int nItemsCur );
extern void        Extra_ProgressBarStop( ProgressBar * p );




/*===========================================================================*/
/*    ZDD-based procedures                                                   */
/*===========================================================================*/

/*=== extraZddCover.c ==============================================================*/

/* the result of this operation is primes contained in the product of cubes */
extern DdNode *    Extra_zddPrimeProduct( DdManager *dd, DdNode * f, DdNode * g );
extern DdNode *     extraZddPrimeProduct( DdManager *dd, DdNode * f, DdNode * g );
/* an alternative implementation of the cover product */
extern DdNode *    Extra_zddProductAlt( DdManager *dd, DdNode * f, DdNode * g );
extern DdNode *     extraZddProductAlt( DdManager *dd, DdNode * f, DdNode * g );
/* returns the set of cubes pair-wise unate with the given cube */
extern DdNode *    Extra_zddCompatible( DdManager * dd, DdNode * zCover, DdNode * zCube );
extern DdNode *     extraZddCompatible( DdManager * dd, DdNode * zCover, DdNode * zCube );
/* a wrapper for the call to Extra_zddIsop() */
extern DdNode *    Extra_zddIsopCover( DdManager * dd, DdNode * F1, DdNode * F12 );
/* a wrapper for the call to Extra_zddIsopCover() and Extra_zddPrintCover() */
extern void        Extra_zddIsopPrintCover( DdManager * dd, DdNode * F1, DdNode * F12 );
/* a simple cover computation (not ISOP) */
extern DdNode *    Extra_zddSimplify( DdManager * dd, DdNode * F1, DdNode * F12 );
extern DdNode *     extraZddSimplify( DdManager * dd, DdNode * F1, DdNode * F12 );
/* an alternative ISOP cover computation (faster than Extra_zddIsop()) */
extern DdNode *    Extra_zddIsopCoverAlt( DdManager * dd, DdNode * F1, DdNode * F12 );
extern DdNode *     extraZddIsopCoverAlt( DdManager * dd, DdNode * F1, DdNode * F12 );
/* count the number of cubes in the ISOP without building the ISOP as a ZDD */
extern int         Extra_zddIsopCubeNum( DdManager * dd, DdNode * F1, DdNode * F12 );
extern DdNode *     extraZddIsopCubeNum( DdManager * dd, DdNode * F1, DdNode * F12, int * pnCubes );

/* computes the disjoint cube cover produced by the bdd paths */
extern DdNode *    Extra_zddDisjointCover( DdManager * dd, DdNode * F );
/* performs resolution on the set of clauses (S) w.r.t. variables in zdd Vars */
extern DdNode *    Extra_zddResolve( DdManager * dd, DdNode * S, DdNode * Vars );
/* cubes from zC that are not contained by cubes from zD over area bA */
extern DdNode *    Extra_zddNotContainedCubesOverArea( DdManager * dd, DdNode * zC, DdNode * zD, DdNode * bA );
extern DdNode *     extraZddNotContainedCubesOverArea( DdManager * dd, DdNode * zC, DdNode * zD, DdNode * bA );
/* finds cofactors of the cover w.r.t. the top-most variable without creating new nodes */
extern void         extraDecomposeCover( DdManager* dd, DdNode * C, DdNode **zC0, DdNode **zC1, DdNode **zC2  );
/* composes the cover from the three subcovers using the given variable (returns NULL = reordering)*/
extern DdNode *     extraComposeCover( DdManager* dd, DdNode * zC0, DdNode * zC1, DdNode * zC2, int TopVar  );
/* selects one cube from a ZDD representing the cube cover */
extern DdNode *    Extra_zddSelectOneCube( DdManager * dd, DdNode * zS );
extern DdNode *     extraZddSelectOneCube( DdManager * dd, DdNode * zS );
/* selects one subset from a ZDD representing the set of subsets */
extern DdNode *    Extra_zddSelectOneSubset( DdManager * dd, DdNode * zS );
extern DdNode *     extraZddSelectOneSubset( DdManager * dd, DdNode * zS );
/* checks unateness of the cover */
extern int         Extra_zddCheckUnateness( DdManager * dd, DdNode * zCover );
/* computes the BDD of the area covered by the max number of cubes in a ZDD. */
extern DdNode *    Extra_zddGetMostCoveredArea( DdManager * dd, DdNode * zC, int * nOverlaps );
extern DdNode *     extraZddGetMostCoveredArea( DdManager * dd, DdNode * zC );

/*=== extraZddExor.c ==============================================================*/

/* computes the Exclusive-OR-type union of two cube sets */
extern DdNode *    Extra_zddUnionExor( DdManager * dd, DdNode * S, DdNode * T );
/* given two sets of cubes, computes the set of pair-wise supercubes */
extern DdNode *    Extra_zddSupercubes( DdManager *dd, DdNode * zA, DdNode * zB );
extern DdNode *     extraZddSupercubes( DdManager *dd, DdNode * zA, DdNode * zB );
/* selects cubes from zA that have a distance-1 cube in zB */
extern DdNode *    Extra_zddSelectDist1Cubes( DdManager *dd, DdNode * zA, DdNode * zB );
extern DdNode *     extraZddSelectDist1Cubes( DdManager *dd, DdNode * zA, DdNode * zB );
/* computes the set of fast ESOP covers for the multi-output function */
extern int         Extra_zddFastEsopCoverArray( DdManager * dd, DdNode ** bFs, DdNode ** zCs, int nFs );
/* computes a fast ESOP cover for the single-output function */
extern DdNode *    Extra_zddFastEsopCover( DdManager * dd, DdNode * bF, st_table * Visited, int * pnCubes );

/*=== extraZddFactor.c ================================================================*/

/* counting the number of literals in the factored form */
extern int         Extra_bddFactoredFormLiterals( DdManager * dd, DdNode * bOnSet, DdNode * bOnDcSet );
extern DdNode *    Extra_zddFactoredFormLiterals( DdManager * dd, DdNode * zCover );
extern DdNode *    Extra_zddLFLiterals( DdManager * dd, DdNode * zCover, DdNode * zCube );
/* computing a quick divisor */
extern DdNode *    Extra_zddQuickDivisor( DdManager * dd, DdNode * zCover );
extern DdNode *    Extra_zddLevel0Kernel( DdManager * dd, DdNode * zCover );
/* division with quotient and remainder */
extern void        Extra_zddDivision( DdManager * dd, DdNode * zCover, DdNode * zDiv, DdNode ** zQuo, DdNode ** zRem );
/* the common cube */
extern DdNode *    Extra_zddCommonCubeFast( DdManager * dd, DdNode * zCover );
/* the cube of literals that occur more than once */
extern DdNode *    Extra_zddMoreThanOnceCubeFast( DdManager * dd, DdNode * zCover );
/* making the cover cube-free */
extern DdNode *    Extra_zddMakeCubeFree( DdManager * dd, DdNode * zCover, int iZVar );
/* testing whether the cover is cube-free */
extern int         Extra_zddTestCubeFree( DdManager * dd, DdNode * zCover );

/* counts the number of literals in the simple cover */
extern int         Extra_zddCountLiteralsSimple( DdManager * dd, DdNode * zCover );
/* tests whether the cover contains more than one cube */
extern int         Extra_zddMoreThanOneCube( DdManager * dd, DdNode * zCover );
/* the cube from levels */
extern DdNode *    Extra_zddCombinationFromLevels( DdManager * dd, int * pLevels, int nVars );
/* determines common literals */
extern int         Extra_zddCommonLiterals( DdManager * dd, DdNode * zCover, int iZVar, int * pLevels, int * pLiterals );
/* determines the set of literals that occur more than once */
extern int         Extra_zddMoreThanOneLiteralSet( DdManager * dd, DdNode * zCover, int StartLevel, int * pVars, int * pCounters );
/* tests whether the given literal literal occurs more than once */
extern int         Extra_zddMoreThanOneLiteral( DdManager * dd, DdNode * zCover, int iZVar );
extern DdNode *     extraZddMoreThanOneLiteral( DdManager * dd, DdNode * zCover, DdNode * zVar );

/*=== extraZddGraph.c ==============================================================*/

/* construct the set of cliques */
extern DdNode *    Extra_zddCliques( DdManager *dd, DdNode * G, int fMaximal ); 
/* construct the set of all maximal cliques */
extern DdNode *    Extra_zddMaxCliques( DdManager *dd, DdNode * G ); 
/* incrementally contruct the set of cliques */
extern DdNode *    Extra_zddIncremCliques( DdManager *dd, DdNode * G, DdNode * C ); 
extern DdNode *     extraZddIncremCliques( DdManager *dd, DdNode * G, DdNode * C ); 

/*=== extraZddIsop.c ================================================================*/

/* improvements to the Irredundant Prime Cover computation */
extern DdNode *    Extra_zddIsopCoverAllVars( DdManager * dd, DdNode * F1, DdNode * F12 );
extern DdNode *     extraZddIsopCoverAllVars( DdManager * dd, DdNode * F1, DdNode * F12, DdNode * bS );

extern DdNode *    Extra_zddIsopCoverUnateVars( DdManager * dd, DdNode * F1, DdNode * F12 );
extern DdNode *     extraZddIsopCoverUnateVars( DdManager * dd, DdNode * F1, DdNode * F12, DdNode * bS );

/* computes an ISOP cover with a random ordering of variables */
extern DdNode *    Extra_zddIsopCoverRandom( DdManager * dd, DdNode * F1, DdNode * F12 );

extern DdNode *    Extra_zddIsopCoverReduced( DdManager * dd, DdNode * F1, DdNode * F12 );
extern DdNode *     extraZddIsopCoverReduced( DdManager * dd, DdNode * F1, DdNode * F12 );

/*=== extraZddLitCount.c ==============================================================*/

/* count the number of times each variable occurs in the combinations */
extern int *       Extra_zddLitCount( DdManager * dd, DdNode * Set );
/* count the number of literals in one ZDD combination */
extern int         Extra_zddLitCountComb( DdManager * dd, DdNode * zComb );

/*=== extraZddMaxMin.c ==============================================================*/

/* maximal/minimimal */
extern DdNode *    Extra_zddMaximal( DdManager *dd, DdNode * S );
extern DdNode *     extraZddMaximal( DdManager *dd, DdNode * S );
extern DdNode *    Extra_zddMinimal( DdManager *dd, DdNode * S );
extern DdNode *     extraZddMinimal( DdManager *dd, DdNode * S );
/* maximal/minimal of the union of two sets of subsets */
extern DdNode *    Extra_zddMaxUnion( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *     extraZddMaxUnion( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *    Extra_zddMinUnion( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *     extraZddMinUnion( DdManager *dd, DdNode * S, DdNode * T );
/* dot/cross products */
extern DdNode *    Extra_zddDotProduct( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *     extraZddDotProduct( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *    Extra_zddExorProduct( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *     extraZddExorProduct( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *    Extra_zddCrossProduct( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *     extraZddCrossProduct( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *    Extra_zddMaxDotProduct( DdManager *dd, DdNode * S, DdNode * T );
extern DdNode *     extraZddMaxDotProduct( DdManager *dd, DdNode * S, DdNode * T );

/*=== extraZddMisc.c ==============================================================*/

/* create the combination composed of a single ZDD variable */
extern DdNode *    Extra_zddVariable( DdManager * dd, int iVar );
/* build a ZDD for a combination of variables */
extern DdNode *    Extra_zddCombination( DdManager *dd, int* VarValues, int nVars );
extern DdNode *     extraZddCombination( DdManager *dd, int *VarValues, int nVars  );
/* the set of all possible combinations of the given set of variables */
extern DdNode *    Extra_zddUniverse( DdManager * dd, DdNode * VarSet );
extern DdNode *     extraZddUniverse( DdManager * dd, DdNode * VarSet );
/* build the set of all tuples of K variables out of N */
extern DdNode *    Extra_zddTuples( DdManager * dd, int K, DdNode * zVarsN );
extern DdNode *     extraZddTuples( DdManager * dd, DdNode * zVarsK, DdNode * zVarsN );
/* build the set of all tuples of K variables out of N from the BDD cube */
extern DdNode *    Extra_zddTuplesFromBdd( DdManager * dd, int K, DdNode * bVarsN );
extern DdNode *     extraZddTuplesFromBdd( DdManager * dd, DdNode * bVarsK, DdNode * bVarsN );
/* convert the set of singleton combinations into one combination */
extern DdNode *    Extra_zddSinglesToComb( DdManager * dd, DdNode * Singles );
extern DdNode *     extraZddSinglesToComb(  DdManager * dd, DdNode * Singles  );
/* returns the set of combinations containing the max/min number of elements */
extern DdNode *    Extra_zddMaximum( DdManager * dd, DdNode * S, int * nVars );
extern DdNode *     extraZddMaximum( DdManager * dd, DdNode * S, int * nVars );
extern DdNode *    Extra_zddMinimum( DdManager * dd, DdNode * S, int * nVars );
extern DdNode *     extraZddMinimum( DdManager * dd, DdNode * S, int * nVars );
/* returns the random set of k combinations of n elements with average density d */
extern DdNode *    Extra_zddRandomSet( DdManager * dd, int n, int k, double d );
/* other utilities */
extern DdNode *    Extra_zddCoveredByArea( DdManager *dd, DdNode * zC, DdNode * bA );
extern DdNode *     extraZddCoveredByArea( DdManager *dd, DdNode * zC, DdNode * bA );
extern DdNode *    Extra_zddNotCoveredByCover( DdManager *dd, DdNode * zC, DdNode * zD );
extern DdNode *     extraZddNotCoveredByCover( DdManager *dd, DdNode * zC, DdNode * zD );
extern DdNode *    Extra_zddOverlappingWithArea( DdManager *dd, DdNode * zC, DdNode * bA );
extern DdNode *     extraZddOverlappingWithArea( DdManager *dd, DdNode * zC, DdNode * bA );
extern DdNode *    Extra_zddConvertToBdd( DdManager *dd, DdNode * zC );
extern DdNode *     extraZddConvertToBdd( DdManager *dd, DdNode * zC );
extern DdNode *    Extra_zddConvertToBddUnate( DdManager *dd, DdNode * zC );
extern DdNode *     extraZddConvertToBddUnate( DdManager *dd, DdNode * zC );
extern DdNode *    Extra_zddConvertEsopToBdd( DdManager *dd, DdNode * zC );
extern DdNode *     extraZddConvertEsopToBdd( DdManager *dd, DdNode * zC );
extern DdNode *    Extra_zddConvertToBddAndAdd( DdManager *dd, DdNode * zC, DdNode * bA );
extern DdNode *     extraZddConvertToBddAndAdd( DdManager *dd, DdNode * zC, DdNode * bA );
extern DdNode *    Extra_zddSingleCoveredArea( DdManager *dd, DdNode * zC );
extern DdNode *     extraZddSingleCoveredArea( DdManager *dd, DdNode * zC );
extern DdNode *    Extra_zddConvertBddCubeIntoZddCube( DdManager *dd, DdNode * bCube );

/*=== extraZddPermute.c ==============================================================*/

/* quantifications */
extern DdNode *    Extra_zddExistAbstract( DdManager *manager, DdNode * F, DdNode * cube );
extern DdNode *     extraZddExistAbstractRecur( DdManager *manager, DdNode * F, DdNode * cube );
/* changes the value of several variables in the ZDD */
extern DdNode *    Extra_zddChangeVars( DdManager *manager, DdNode * F, DdNode * cube );
extern DdNode *     extraZddChangeVars( DdManager *manager, DdNode * F, DdNode * cube );
/* permutes variables in ZDD */
extern DdNode *    Extra_zddPermute ( DdManager *dd, DdNode * N, int *permut );
/* computes combinations in F with vars in Cube having the negative polarity */
extern DdNode *    Extra_zddCofactor0( DdManager * dd, DdNode * f, DdNode * cube );
extern DdNode *     extraZddCofactor0( DdManager * dd, DdNode * f, DdNode * cube );
/* computes combinations in F with vars in Cube having the positive polarity */
extern DdNode *    Extra_zddCofactor1( DdManager * dd, DdNode * f, DdNode * cube, int fIncludeVars );
extern DdNode *     extraZddCofactor1( DdManager * dd, DdNode * f, DdNode * cube, int fIncludeVars );

/*=== extraZddSubSup.c ==============================================================*/

/* subset/supset operations */
extern DdNode *    Extra_zddSubSet   ( DdManager *dd, DdNode * X, DdNode * Y );
extern DdNode *     extraZddSubSet   ( DdManager *dd, DdNode * X, DdNode * Y );
extern DdNode *    Extra_zddSupSet   ( DdManager *dd, DdNode * X, DdNode * Y );
extern DdNode *     extraZddSupSet   ( DdManager *dd, DdNode * X, DdNode * Y );
extern DdNode *    Extra_zddNotSubSet( DdManager *dd, DdNode * X, DdNode * Y );
extern DdNode *     extraZddNotSubSet( DdManager *dd, DdNode * X, DdNode * Y );
extern DdNode *    Extra_zddNotSupSet( DdManager *dd, DdNode * X, DdNode * Y );
extern DdNode *     extraZddNotSupSet( DdManager *dd, DdNode * X, DdNode * Y );
extern DdNode *    Extra_zddMaxNotSupSet( DdManager *dd, DdNode * X, DdNode * Y );
extern DdNode *     extraZddMaxNotSupSet( DdManager *dd, DdNode * X, DdNode * Y );
/* check whether the empty combination belongs to the set of subsets */
extern int         Extra_zddEmptyBelongs ( DdManager *dd, DdNode* zS  );
/* check whether the set consists of one subset only */
extern int         Extra_zddIsOneSubset( DdManager *dd, DdNode* zS  );



/**AutomaticEnd***************************************************************/

#endif /* __EXTRA_H__ */
