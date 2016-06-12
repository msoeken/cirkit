/**CFile****************************************************************

  FileName    [extraUtilPrint.c]

  PackageName [extra]

  Synopsis    [Various printing procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraUtilPrint.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

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

static void extraWriteBlifNode( FILE * pFile, DdManager * dd, DdNode * zCover, int * pVarMap, int nVars, int levPrev, int * pVarValues );
static void extraWriteBlifGateCascade( FILE * pFile, char * pInputNames[], int Polars[], int nInputs, char * pOutputName, int GateType, int OutputSigNum, int * pSigNum );
static void extraWriteBlifNodeUsingGates( FILE * pFile, DdManager * dd, DdNode * zCover, char * pInputNames[], char * pOutputName, int fCascade );

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Outputs the BDD in a readable format.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void Extra_bddPrint( DdManager * dd, DdNode * F )
// utility to convert an ADD or a BDD into a string containing set of cubes
{
    DdGen * Gen;
    int * Cube;
    CUDD_VALUE_TYPE Value;
    int nVars = dd->size;
    int fFirstCube = 1;
    int i;

    if ( F == b0 || F == a0 )
    {
        printf("Constant 0");
        return;
    }
    if ( F == b1 || F == a1 )
    {
        printf("Constant 1");
        return;
    }

    Cudd_ForeachCube( dd, F, Gen, Cube, Value )
    {
        if ( fFirstCube )
            fFirstCube = 0;
        else
//          Output << " + ";
            printf( " + " );

        for ( i = 0; i < nVars; i++ )
            if ( Cube[i] == 0 )
                printf( "[%d]'", i );
//              printf( "%c'", (char)('a'+i) );
            else if ( Cube[i] == 1 )
                printf( "[%d]", i );
//              printf( "%c", (char)('a'+i) );
    }

//  printf("\n");
}

/**Function********************************************************************

  Synopsis    [Visualize the BDD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Extra_bddShow( DdManager * dd, DdNode * bFunc )
{ 
    FILE * pFileDot = fopen( "bdd.dot", "w" );
    Cudd_DumpDot( dd, 1, &bFunc, NULL, NULL, pFileDot );
    fclose( pFileDot );
}

/**Function********************************************************************

  Synopsis    [Visualize the BDD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Extra_addShowFromBdd( DdManager * dd, DdNode * bFunc )
{ 
    DdNode * aFunc;
    FILE * pFileDot = fopen( "add.dot", "w" );
    aFunc = Cudd_BddToAdd( dd, bFunc );  Cudd_Ref( aFunc );
    Cudd_DumpDot( dd, 1, &aFunc, NULL, NULL, pFileDot );
    Cudd_RecursiveDeref( dd, aFunc );
    fclose( pFileDot );
}

/**Function*************************************************************

  Synopsis    [Writes the DOT file.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_DumpDot( DdManager * dd, DdNode * pFuncs[], int nFuncs, char * FileName, int fFlagZdd )
{
    FILE * pFileDot;
    pFileDot = fopen( FileName, "w" );
    if ( fFlagZdd )
        Cudd_zddDumpDot( dd, nFuncs, pFuncs, NULL, NULL, pFileDot );
    else
        Cudd_DumpDot( dd, nFuncs, pFuncs, NULL, NULL, pFileDot );
    fclose( pFileDot );
}



/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
unsigned Extra_ReadBinary( char * Buffer )
{
    unsigned Result;
    int i;

    Result = 0;
    for ( i = 0; Buffer[i]; i++ )
        if ( Buffer[i] == '0' || Buffer[i] == '1' )
            Result = Result * 2 + Buffer[i] - '0';
        else
        {
            assert( 0 );
        }
    return Result;
}

/**Function*************************************************************

  Synopsis    [Prints the bit string.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_PrintBinary( FILE * pFile, unsigned Sign[], int nBits )
{
    int Remainder, nWords;
    int w, i;

    Remainder = (nBits%(sizeof(unsigned)*8));
    nWords    = (nBits/(sizeof(unsigned)*8)) + (Remainder>0);

    for ( w = nWords-1; w >= 0; w-- )
        for ( i = ((w == nWords-1)? Remainder-1: 31); i >= 0; i-- )
            fprintf( pFile, "%c", '0' + (int)((Sign[w] & (1<<i)) > 0) );

//  fprintf( pFile, "\n" );
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_PrintSupport( DdManager * dd, DdNode * Func )
{
    DdNode * bSupp;
    bSupp = Cudd_Support( dd, Func );  Cudd_Ref( bSupp );
PRB( dd, bSupp );
    Cudd_RecursiveDeref( dd, bSupp );
    printf( "\n" );
}

/**Function*************************************************************

  Synopsis    [Returns the composite name of the file.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_PrintSymbols( FILE * pFile, char Char, int nTimes, int fPrintNewLine )
{
    int i;
    for ( i = 0; i < nTimes; i++ )
        printf( "%c", Char );
    if ( fPrintNewLine )
        printf( "\n" );
}


/**Function*************************************************************

  Synopsis    []

  Description []
  
  SideEffects []

  SeeAlso     []

***********************************************************************/
FILE * Extra_WriteBlifStart( char * FileName, char * pInputNames[], int nInputs, char * pOutputNames[], int nOutputs )
{
    FILE * pFile;
    int i;
    // start writing the output file
    pFile = fopen( FileName, "w" );
    // write the hearder
    fprintf( pFile, ".model %s", FileName );
    fprintf( pFile, "\n" );

    fprintf( pFile, ".inputs" );
    for ( i = 0; i < nInputs; i++ )
        fprintf( pFile, " %s", pInputNames[i] );
    fprintf( pFile, "\n" );

    fprintf( pFile, ".outputs" );
    for ( i = 0; i < nOutputs; i++ )
        fprintf( pFile, " %s", pOutputNames[i] );
    fprintf( pFile, "\n" );
    return pFile;
}

/**Function*************************************************************

  Synopsis    [Write the BDD/ZDD into as the boolean node in the BLIF file.]

  Description [The file and the DD manager are obvious. The manager can 
  be reordered. The function is the BDD in the given manager. The input
  names in the array are all variable names in the manager. The function
  may actually depend on a subset of them. Whether the manager is reordered
  or not, this function always writes the cover in the natural ordering
  of the variables. The output names is used on the ".names" line.
  If the array of variables is not given, the ".names" line is not written.]
  
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_WriteBlifNode( FILE * pFile, DdManager * dd, DdNode * Func, char * pInputNames[], char * pOutputName )
{
    DdNode * zCover;
    int * pVarMap;
    int * pSupport;
    int * pVarValues;
    int nSupp, v;
    int fThisIsBdd;

    pSupport   = ALLOC( int, ddMax(dd->size,dd->sizeZ) );
    pVarMap    = ALLOC( int, ddMax(dd->size,dd->sizeZ) );
    pVarValues = ALLOC( int, ddMax(dd->size,dd->sizeZ) );

    fThisIsBdd = Extra_WithComplementedEdges(Func);

    // derive the mapping of support variables into the real variables of this function
    nSupp = 0;
    Extra_SupportArray( dd, Func, pSupport );
    if ( fThisIsBdd )
    {
        for ( v = 0; v < dd->size; v++ )
            if ( pSupport[v] )
                pVarMap[nSupp++] = v;
    }
    else // this is a ZDD
    {
        for ( v = 0; v < dd->size; v++ )
            if ( pSupport[2*v] || pSupport[2*v+1] )
                pVarMap[nSupp++] = v;
    }


    if ( pInputNames )
    { 
        // write the ".names" line
        fprintf( pFile, ".names" );
        // write the inputs in the order of increasing variable number
        for ( v = 0; v < nSupp; v++ )
            fprintf( pFile, " %s", pInputNames[pVarMap[v]] );
        // write the output
        fprintf( pFile, "  %s\n", pOutputName );
    }

    // create the ZDD coverk
    if ( fThisIsBdd )
    {
        zCover = Extra_zddIsopCover( dd, Func, Func );  Cudd_Ref( zCover );
    //  zCover = sopEspressoSingle( dd, Func, 1 );  Cudd_Ref( zCover );
        extraWriteBlifNode( pFile, dd, zCover, pVarMap, nSupp, -1, pVarValues );
        Cudd_RecursiveDerefZdd( dd, zCover );
    }
    else
        extraWriteBlifNode( pFile, dd, Func, pVarMap, nSupp, -1, pVarValues );

    free( pSupport );
    free( pVarMap );
    free( pVarValues );
}


/**Function*************************************************************

  Synopsis    [Write the BDD/ZDD into as the boolean node using two-input AND/OR gates.]

  Description []
  
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_WriteBlifNodeUsingGates( FILE * pFile, DdManager * dd, DdNode * Func, char * pInputNames[], char * pOutputName, int fCascade )
{
    DdNode * zCover;
    // create the ZDD cover
    if ( Extra_WithComplementedEdges(Func) )
    {
        zCover = Extra_zddIsopCover( dd, Func, Func );  Cudd_Ref( zCover );
        extraWriteBlifNodeUsingGates( pFile, dd, zCover, pInputNames, pOutputName, fCascade );
        Cudd_RecursiveDerefZdd( dd, zCover );
    }
    else
        extraWriteBlifNodeUsingGates( pFile, dd, Func, pInputNames, pOutputName, fCascade );
}


/**Function*************************************************************

  Synopsis    []

  Description []
  
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_WriteBlifStop( FILE * pFile )
{
    fprintf( pFile, ".end\n\n" );
    fclose( pFile );
}
/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_WriteBlifGate( FILE * pFile, char * pInputNames[], int Polars[], int nInputs, char * pOutputName, int GateType )
{
    int v, i;

    // write the names line
    fprintf( pFile, ".names" );
    for ( v = 0; v < nInputs; v++ )
        fprintf( pFile, " %s", pInputNames[v] );
    fprintf( pFile, " %s\n", pOutputName );

    if ( GateType == EXTRA_GATE_TYPE_BUF )
        fprintf( pFile, "1 1\n" );
    else if ( GateType == EXTRA_GATE_TYPE_INV )
        fprintf( pFile, "0 1\n" );
    else if ( GateType == EXTRA_GATE_TYPE_AND || GateType == EXTRA_GATE_TYPE_NOR )
    {
        for ( v = 0; v < nInputs; v++ )
        {
            if ( GateType == EXTRA_GATE_TYPE_AND )
            {
                if ( Polars && Polars[v] == 0 )
                    fprintf( pFile, "0" );
                else
                    fprintf( pFile, "1" );
            }
            else
            {
                if ( Polars && Polars[v] == 0 )
                    fprintf( pFile, "1" );
                else
                    fprintf( pFile, "0" );
            }
        }
        fprintf( pFile, " 1\n" );
    }
    else if ( GateType == EXTRA_GATE_TYPE_OR || GateType == EXTRA_GATE_TYPE_NAND )
    {
        for ( v = 0; v < nInputs; v++ )
        {
            for ( i = 0; i < nInputs; i++ )
                if ( i == v )
                {
                    if ( GateType == EXTRA_GATE_TYPE_OR )
                    {
                        if ( Polars && Polars[v] == 0 )
                            fprintf( pFile, "0" );
                        else
                            fprintf( pFile, "1" );
                    }
                    else
                    {
                        if ( Polars && Polars[v] == 0 )
                            fprintf( pFile, "1" );
                        else
                            fprintf( pFile, "0" );
                    }
                }
                else
                    fprintf( pFile, "-" );
            fprintf( pFile, " 1\n" );
        }
    }
    else if ( GateType == EXTRA_GATE_TYPE_EXOR || GateType == EXTRA_GATE_TYPE_NEXOR )
    {
        unsigned m, nMints;
        int nOnes;

        // write the cubes
        nMints = (1<<nInputs);
        for ( m = 0; m < nMints; m++ )
        {
            // count the number of ones
            nOnes = 0;
            for ( v = 0; v < nInputs; v++ )
                if ( m & (1<<v) )
                    nOnes++;
            // if the number is ODD, write this minterm
            if (  (GateType == EXTRA_GATE_TYPE_EXOR  &&  (nOnes & 1)) || 
                  (GateType == EXTRA_GATE_TYPE_NEXOR && !(nOnes & 1)) ) 
            {
                Extra_PrintBinary( pFile, &m, nInputs );
                fprintf( pFile, " 1\n" );
            }
        }
    }
    else if ( GateType == EXTRA_GATE_TYPE_CROSS )
    { // write the cross composed of 1's - to denote some strange gate
        for ( v = 0; v < nInputs; v++ )
        {
            for ( i = 0; i < nInputs; i++ )
                if ( i == v || i == nInputs-1-v )
                    fprintf( pFile, "1" );
                else
                    fprintf( pFile, "-" );
            fprintf( pFile, " 1\n" );
        }
    }
    else
    {
        assert( 0 );
    }
}


/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_WriteBlifGateCascade( FILE * pFile, char * pInputNames[], int Polars[], int nInputs, char * pOutputName, int GateType )
{
    int SigNum;
    int fComp0;

    assert( nInputs > 0 );
    if ( nInputs == 1 )
    {
        fprintf( pFile, ".names %s %s\n", pInputNames[0], pOutputName );
        fComp0 = (Polars && Polars[0] == 0)? 1: 0;
        if ( GateType == EXTRA_GATE_TYPE_AND )
            fprintf( pFile, "%d 1\n", 1^fComp0 );
        else if ( GateType == EXTRA_GATE_TYPE_NOR )
            fprintf( pFile, "%d 1\n", 0^fComp0 );
        else if ( GateType == EXTRA_GATE_TYPE_OR )
            fprintf( pFile, "%d 1\n", 1^fComp0 );
        else if ( GateType == EXTRA_GATE_TYPE_NAND )
            fprintf( pFile, "%d 1\n", 0^fComp0 );
        else if ( GateType == EXTRA_GATE_TYPE_EXOR )
            fprintf( pFile, "1 1\n" );
        else if ( GateType == EXTRA_GATE_TYPE_NEXOR )
            fprintf( pFile, "0 1\n" );
        return;
    }
    SigNum = 1;
    extraWriteBlifGateCascade( pFile, pInputNames, Polars, nInputs, pOutputName, GateType, 0, &SigNum );
}

/**Function********************************************************************

  Synopsis    [Writes the BDD into a PLA file as one SOP block.]

  Description [Takes the manager, the function, the array of variable names, 
  the number of variables and the file name. This function is useful, when we need 
  to write into a file the function, for which it is impossible to derive the SOP.]

  SideEffects [None]

  SeeAlso     [Cudd_zddPrintCover]

******************************************************************************/
void Extra_WriteFunctionSop( 
  DdManager * dd, 
  DdNode * bFunc, 
  DdNode * bFuncDc, 
  char ** pNames, 
  char * OutputName, 
  char * FileName )
{
    static int s_pVarMask[MAXINPUTS];
    DdNode * bFuncs[2]; 
    FILE * pFile;
    int nInputCounter;
    int i;
    DdNode * zCover;

    bFuncs[0] = bFunc;
    bFuncs[1] = (bFuncDc)? bFuncDc: b0;

    // create the variable mask
//  Extra_SupportArray( dd, bFunc, s_pVarMask );
    Extra_VectorSupportArray( dd, bFuncs, 2, s_pVarMask );
    nInputCounter = 0;
    for ( i = 0; i < dd->size; i++ )
        if ( s_pVarMask[i] )
            nInputCounter++;


    // start the file
    pFile = fopen( FileName, "w" );
/*
    fprintf( pFile, ".model %s\n", FileName );
    fprintf( pFile, ".inputs" );
    if ( pNames )
    {
        for ( i = 0; i < dd->size; i++ ) // go through levels
            if ( s_pVarMask[ dd->invperm[i] ] ) // if this level's var is present
                fprintf( pFile, " %s", pNames[ dd->invperm[i] ] ); // print its name
    }
    else
    {
        for ( i = 0; i < nInputCounter; i++ )
            fprintf( pFile, " x%d", i );
    }
    fprintf( pFile, "\n" );
    fprintf( pFile, ".outputs %s", OutputName );
    fprintf( pFile, "\n" );
*/
    fprintf( pFile, ".i %d\n", nInputCounter );
    fprintf( pFile, ".o %d\n", 1 );

    // write the on-set into file
    zCover = Extra_zddIsopCover( dd, bFunc, bFunc );       Cudd_Ref( zCover );
    if ( zCover != z0 )
    extraWriteFunctionSop( pFile, dd, zCover, -1, dd->size, "1", s_pVarMask );
    Cudd_RecursiveDerefZdd( dd, zCover );

    // write the dc-set into file
    if ( bFuncDc )
    {
        zCover = Extra_zddIsopCover( dd, bFuncDc, bFuncDc );       Cudd_Ref( zCover );
        if ( zCover != z0 )
        extraWriteFunctionSop( pFile, dd, zCover, -1, dd->size, "-", s_pVarMask );
        Cudd_RecursiveDerefZdd( dd, zCover );
    }

//  fprintf( pFile, ".end\n" );
    fprintf( pFile, ".e\n" );
    fclose( pFile );
}

/**Function********************************************************************

  Synopsis    [Writes the {A,B}DD into a BLIF file as a network of MUXes.]

  Description [Takes the manager, the function, the array of variable names, 
  the number of variables, the output name and the file name. This function is useful, when we need 
  to write into a file the function, for which it is impossible to derive the SOP.]

  SideEffects [None]

  SeeAlso     [Cudd_zddPrintCover]

******************************************************************************/
void Extra_WriteFunctionMuxes( 
  DdManager * dd, 
  DdNode * Func, 
  char ** pNames, 
  char * OutputName, 
  char * FileName )
{
    int i;
    FILE * pFile;
    int nInputCounter;
    static int s_pVarMask[MAXINPUTS];

    // create the variable mask
    Extra_SupportArray( dd, Func, s_pVarMask );
//  Extra_VectorSupportArray( dd, bFuncs, 2, s_pVarMask );
    nInputCounter = 0;
    for ( i = 0; i < dd->size; i++ )
        if ( s_pVarMask[i] )
            nInputCounter++;

    // start the file
    pFile = fopen( FileName, "w" );
    fprintf( pFile, ".model %s\n", FileName );

    fprintf( pFile, ".inputs" );
    if ( pNames )
    {
        for ( i = 0; i < dd->size; i++ ) // go through vars
            if ( s_pVarMask[i] ) // if this var is present
                fprintf( pFile, " %s", pNames[i] ); // print its name
    }
    else
    {
        for ( i = 0; i < dd->size; i++ )
            if ( s_pVarMask[i] ) // if this var is present
                fprintf( pFile, " x%d", i );
    }

    fprintf( pFile, "\n" );
    fprintf( pFile, ".outputs %s", OutputName );
    fprintf( pFile, "\n" );

    // write the DD into the file
    extraWriteFunctionMuxes( pFile, Func, "F", "", pNames );

    fprintf( pFile, ".end\n" );
    fclose( pFile );
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [ZDD traversal function, which write ZDD into the BLIF file.]

  Description [Enumerates the cubes of zCover and writes them into s_pVerValues.
  Upon reaching the bottom level, writes the cube into the FILE.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void 
extraWriteBlifNode( 
  FILE * pFile,        // the output file
  DdManager * dd,      // the DD manager
  DdNode * zCover,     // the ZDD representing the cube cover
  int * pVarMap,       // the mapping of the variable number in the cube into the variable index in the manager
  int nVars,           // the number of variables in the cube
  int levPrev,         // the level from which this function is called
  int * pVarValues )   // the current array of levels
{
    int TopLevel;
    int lev, var;

    if ( zCover == z0 )
        return;
    if ( zCover == z1 )
    {
        // fill in the remaining variables 
        for ( lev = levPrev + 1; lev < dd->sizeZ; lev++ )
            pVarValues[ dd->invpermZ[ lev ] ] = 0;
        // write the cube 
        for ( var = 0; var < nVars; var++ )
            if      (  pVarValues[ pVarMap[var]*2 ] && !pVarValues[ pVarMap[var]*2+1 ] ) // the value in ZDD is 1 
                fprintf( pFile, "1" );
            else if ( !pVarValues[ pVarMap[var]*2 ] &&  pVarValues[ pVarMap[var]*2+1 ] ) // the value in ZDD is 0
                fprintf( pFile, "0" );
            else if (  pVarValues[ pVarMap[var]*2 ] &&  pVarValues[ pVarMap[var]*2+1 ] ) // the value in ZDD is wrong
                fprintf( pFile, "@" );
            else
                fprintf( pFile, "-" );
        // write the end of the line
        fprintf( pFile, " 1\n" );
        return;
    }
    else
    {
        // find the level of the top variable 
        TopLevel = dd->permZ[ zCover->index ];

        // fill in the remaining variables 
        for ( lev = levPrev + 1; lev < TopLevel; lev++ )
            pVarValues[ dd->invpermZ[ lev ] ] = 0;

        // fill in this variable 
        // the given var has negative polarity 
        pVarValues[ zCover->index ] = 0;
        // iterate through the else branch
        extraWriteBlifNode( pFile, dd, cuddE(zCover), pVarMap, nVars, TopLevel, pVarValues );
        
        // the given var has positive polarity
        pVarValues[ zCover->index ] = 1;
        // iterate through the then branch
        extraWriteBlifNode( pFile, dd, cuddT(zCover), pVarMap, nVars, TopLevel, pVarValues );
    }
} /* end of extraWriteBlifNode */


/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraWriteBlifNodeUsingGates( FILE * pFile, DdManager * dd, DdNode * zCover, char * pInputNames[], char * pOutputName, int fCascade )
{
    char Buffer[100];
    int * pPolars;
    char ** pNamesOr;
    char ** pNamesAnd;
    DdNode * zCoverCur;
    DdNode * zCube, * zTemp;
    int nCubes;
    int c, v;

    if ( cuddIsConstant(zCover) )
    {
        fprintf( pFile, ".names %s\n", pOutputName );
        if ( zCover == z1 )
            fprintf( pFile, " 1\n" );
        return;
    }

    nCubes   = Cudd_zddCount( dd, zCover );
    pNamesOr = ALLOC( char *, nCubes );
    // assign the cube names
    for ( c = 0; c < nCubes; c++ )
    {
        sprintf( Buffer, "%s_c%d", pOutputName, c );
        pNamesOr[c] = util_strsav( Buffer );
    }

    // write the OR gate
    if ( fCascade )
        Extra_WriteBlifGateCascade( pFile, pNamesOr, NULL, nCubes, pOutputName, EXTRA_GATE_TYPE_OR );
    else
        Extra_WriteBlifGate( pFile, pNamesOr, NULL, nCubes, pOutputName, EXTRA_GATE_TYPE_OR );


    // write the cubes
    pNamesAnd = ALLOC( char *, dd->size );
    pPolars   = ALLOC( int, dd->size );
    c = 0;
    zCoverCur = zCover;   Cudd_Ref( zCoverCur );
    while ( zCoverCur != z0 )
    {
        // get the cube
        zCube = Extra_zddSelectOneCube( dd, zCoverCur );  Cudd_Ref( zCube );
        // collect the input names and polarities present in this cube
        v = 0;
        for ( zTemp = zCube; zTemp != z1; zTemp = cuddT(zTemp) )
        {
            pNamesAnd[v] = pInputNames[zTemp->index/2];
            pPolars  [v] = (zTemp->index%2 == 0);
            v++;
        }
        if ( fCascade )
        {
/*
            fprintf( pFile, "\n\n\n" );
            fprintf( stdout, "\n\n\n" );
            Extra_WriteBlifGate( pFile, pNamesAnd, pPolars, v, pNamesOr[c], EXTRA_GATE_TYPE_AND );
            Extra_WriteBlifGate( stdout, pNamesAnd, pPolars, v, pNamesOr[c], EXTRA_GATE_TYPE_AND );
            fprintf( pFile, "\n" );
            fprintf( stdout, "\n" );
if ( strcmp( pNamesOr[c], "[5]_c0" ) == 0 )
{
    int i = 0;
}
            Extra_WriteBlifGateCascade( pFile, pNamesAnd, pPolars, v, pNamesOr[c], EXTRA_GATE_TYPE_AND );
            Extra_WriteBlifGateCascade( stdout, pNamesAnd, pPolars, v, pNamesOr[c], EXTRA_GATE_TYPE_AND );
            fprintf( pFile, "\n\n\n" );
            fprintf( stdout, "\n\n\n" );
*/
            Extra_WriteBlifGateCascade( pFile, pNamesAnd, pPolars, v, pNamesOr[c], EXTRA_GATE_TYPE_AND );
        }
        else
            Extra_WriteBlifGate( pFile, pNamesAnd, pPolars, v, pNamesOr[c], EXTRA_GATE_TYPE_AND );
        
        // substract this cube
        zCoverCur = Cudd_zddDiff( dd, zTemp = zCoverCur, zCube );  Cudd_Ref( zCoverCur );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zCube );
        c++;
    }
    Cudd_RecursiveDerefZdd( dd, zCoverCur );
    for ( c = 0; c < nCubes; c++ )
        free( pNamesOr[c] );
    free( pPolars );
    free( pNamesOr );
    free( pNamesAnd );
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraWriteBlifGateCascade( FILE * pFile, char * pInputNames[], int Polars[], int nInputs, char * pOutputName, int GateType, int OutputSigNum, int * pSigNum )
{
    int nInputs1, nInputs2;
    int nSigNum1, nSigNum2;
    int fComp0, fComp1;

    assert( nInputs > 1 );
    if ( nInputs == 2 )
    {
        // write the names line
        fprintf( pFile, ".names" );
        // write the first input name
        fprintf( pFile, " %s", pInputNames[0] );
        // write the second input name
        fprintf( pFile, " %s", pInputNames[1] );
        // write the output name
        if ( OutputSigNum == 0 ) // this is the first call 
            fprintf( pFile, " %s\n", pOutputName );
        else
            fprintf( pFile, " %s_%d\n", pOutputName, OutputSigNum );
        // write the gate
        fComp0 = (Polars && Polars[0] == 0)? 1: 0;
        fComp1 = (Polars && Polars[1] == 0)? 1: 0;
        if ( GateType == EXTRA_GATE_TYPE_AND )
            fprintf( pFile, "%d%d 1\n", 1^fComp0, 1^fComp1 );
        else if ( GateType == EXTRA_GATE_TYPE_NOR )
            fprintf( pFile, "%d%d 1\n", 0^fComp0, 0^fComp1 );
        else if ( GateType == EXTRA_GATE_TYPE_OR )
            fprintf( pFile, "%d- 1\n-%d 1\n", 1^fComp0, 1^fComp1 );
        else if ( GateType == EXTRA_GATE_TYPE_NAND )
            fprintf( pFile, "%d- 1\n-%d 1\n", 0^fComp0, 0^fComp1 );
        else if ( GateType == EXTRA_GATE_TYPE_EXOR )
            fprintf( pFile, "01 1\n10 1\n" );
        else if ( GateType == EXTRA_GATE_TYPE_NEXOR )
            fprintf( pFile, "00 1\n11 1\n" );
        else
        {
            assert( 0 );
        }
        return;
    }
    // nInputs > 2

    nInputs1 = (1<<(Extra_Base2Log(nInputs)-1));
    nInputs2 = nInputs - nInputs1;
    assert( nInputs1 > 1 );
    assert( nInputs2 > 0 );
    nSigNum1 = (*pSigNum)++;
    nSigNum2 = (*pSigNum)++;

    // write the names line
    fprintf( pFile, ".names" );
    // write the first input name
    fprintf( pFile, " %s_%d", pOutputName, nSigNum1 );
    // write the second input name
    if ( nInputs2 == 1 )
        fprintf( pFile, " %s", pInputNames[nInputs1] );
    else
        fprintf( pFile, " %s_%d", pOutputName, nSigNum2 );
    // write the output name
    if ( OutputSigNum == 0 ) // this is the first call 
        fprintf( pFile, " %s\n", pOutputName );
    else
        fprintf( pFile, " %s_%d\n", pOutputName, OutputSigNum );
    // write the gate
    fComp0 = 0;
    fComp1 = (nInputs2 == 1 && Polars && Polars[nInputs1] == 0)? 1: 0;
    if ( GateType == EXTRA_GATE_TYPE_AND )
        fprintf( pFile, "%d%d 1\n", 1^fComp0, 1^fComp1 );
    else if ( GateType == EXTRA_GATE_TYPE_NOR )
        fprintf( pFile, "%d%d 1\n", 0^fComp0, 0^fComp1 );
    else if ( GateType == EXTRA_GATE_TYPE_OR )
        fprintf( pFile, "%d- 1\n-%d 1\n", 1^fComp0, 1^fComp1 );
    else if ( GateType == EXTRA_GATE_TYPE_NAND )
        fprintf( pFile, "%d- 1\n-%d 1\n", 0^fComp0, 0^fComp1 );
    else if ( GateType == EXTRA_GATE_TYPE_EXOR )
        fprintf( pFile, "01 1\n10 1\n" );
    else if ( GateType == EXTRA_GATE_TYPE_NEXOR )
        fprintf( pFile, "00 1\n11 1\n" );
    else
    {
        assert( 0 );
    }

    // call recursively for the first part
    extraWriteBlifGateCascade( pFile, pInputNames, Polars, nInputs1, pOutputName, GateType, nSigNum1, pSigNum );
    if ( nInputs2 > 1 )
        extraWriteBlifGateCascade( pFile, pInputNames + nInputs1, Polars? Polars + nInputs1: Polars, nInputs2, pOutputName, GateType, nSigNum2, pSigNum );
}



/**Function********************************************************************

  Synopsis    [Prints the cover represented by a ZDD.]

  Description [Takes the pointer to the output file stream, the manager, the cover 
  represented by a ZDD, the level from which this function was called (initially,
  this level should be -1), the number of levels that are considered when traversing 
  the cover, the suffix to append to the end of each line, and the variable mask 
  showing variables appearing in the cover. In the cubes that are printed, 
  the variables follow in the order in which they appear in the levels starting 
  from the topmost one.]

  SideEffects [None]

  SeeAlso     [Cudd_zddPrintCover]

******************************************************************************/
void extraWriteFunctionSop( 
    FILE * pFile, 
    DdManager * dd, 
    DdNode * zCover, 
    int levPrev,         // the previous level, from which this function has been called
    int nLevels,         // the number of levels traversed while printing starting from the top one
    const char * AddOn,  // the additional string to attache at the end
    int * VarMask )      // the mask showing which variables should be printed
{
    static char s_VarValueAtLevel[MAXINPUTS];

    if ( zCover == z1 )
    {
        int lev;
        // fill in the remaining variables
        for ( lev = levPrev + 1; lev < nLevels; lev++ )
//          s_VarValueAtLevel[ dd->invpermZ[ 2*lev ] / 2 ] = '-';
            s_VarValueAtLevel[ lev ] = '-';

        // write zero-delimiter
        s_VarValueAtLevel[nLevels] = 0;

        // print the cube
        if ( VarMask == NULL )
            fprintf( pFile, "%s %s\n", s_VarValueAtLevel, AddOn );
        else
        { 
            for ( lev = 0; lev < nLevels; lev++ )
                if ( VarMask[ dd->invperm[lev] ] ) // the variable on this level
                    fprintf( pFile, "%c", s_VarValueAtLevel[lev] );
            fprintf( pFile, " %s\n", AddOn );
        }
    }
    else
    {
        // find the level of the top variable
        int TopLevel = dd->permZ[zCover->index] / 2;
        int TopPol   = zCover->index % 2;
        int lev;

        // fill in the remaining variables
        for ( lev = levPrev + 1; lev < TopLevel; lev++ )
    //      s_VarValueAtLevel[ dd->invpermZ[ 2*lev ] / 2 ] = '-';
            s_VarValueAtLevel[ lev ] = '-';

        // fill in this variable and call for the low and high cofactors
        if ( TopPol == 0 ) 
        {  // the given var has positive polarity
    //      s_VarValueAtLevel[ dd->invpermZ[ 2*TopLevel ] / 2 ] = '1';
            s_VarValueAtLevel[ TopLevel ] = '1';
            extraWriteFunctionSop( pFile, dd, cuddT(zCover), TopLevel, nLevels, AddOn, VarMask );

            if ( cuddE(zCover) != z0 )
            {
    //          s_VarValueAtLevel[ dd->invpermZ[ 2*TopLevel ] / 2 ] = '-';
                s_VarValueAtLevel[ TopLevel ] = '-';
                extraWriteFunctionSop( pFile, dd, cuddE(zCover), TopLevel, nLevels, AddOn, VarMask );
            }
        }
        else
        {  // the given var has negative polarity
    //      s_VarValueAtLevel[ dd->invpermZ[ 2*TopLevel ] / 2 ] = '0';
            s_VarValueAtLevel[ TopLevel ] = '0';
            extraWriteFunctionSop( pFile, dd, cuddT(zCover), TopLevel, nLevels, AddOn, VarMask );

            if ( cuddE(zCover) != z0 )
            {
    //          s_VarValueAtLevel[ dd->invpermZ[ 2*TopLevel ] / 2 ] = '-';
                s_VarValueAtLevel[ TopLevel ] = '-';
                extraWriteFunctionSop( pFile, dd, cuddE(zCover), TopLevel, nLevels, AddOn, VarMask );
            }
        }
    }
}


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_WriteFunctionMuxes().]

  Description [Takes the output file stream, the {A,B}DD function, the name of the output, 
  the prefix attached to each intermediate signal to make it unique, and the array of input 
  variable names. This function is useful, when we need to write into a file the function, 
  for which it is impossible to derive the SOP. Some part of the code is borrowed 
  from Cudd_DumpDot()]

  SideEffects [None]

  SeeAlso     [Cudd_zddPrintCover]

******************************************************************************/
void extraWriteFunctionMuxes( 
  FILE * pFile, 
  DdNode * Func, 
  char * OutputName, 
  char * Prefix, 
  char ** InputNames )
{
    int i;
    st_table * visited;
    st_table * invertors;
    st_generator * gen = NULL;
    long refAddr, diff, mask;
    DdNode * Node, * Else, * ElseR, * Then;

    /* Initialize symbol table for visited nodes. */
    visited = st_init_table( st_ptrcmp, st_ptrhash );

    /* Collect all the nodes of this DD in the symbol table. */
    cuddCollectNodes( Cudd_Regular(Func), visited );

    /* Find how many most significant hex digits are identical
       ** in the addresses of all the nodes. Build a mask based
       ** on this knowledge, so that digits that carry no information
       ** will not be printed. This is done in two steps.
       **  1. We scan the symbol table to find the bits that differ
       **     in at least 2 addresses.
       **  2. We choose one of the possible masks. There are 8 possible
       **     masks for 32-bit integer, and 16 possible masks for 64-bit
       **     integers.
     */

    /* Find the bits that are different. */
    refAddr = ( long )Cudd_Regular(Func);
    diff = 0;
    gen = st_init_gen( visited );
    while ( st_gen( gen, ( char ** ) &Node, NULL ) )
    {
        diff |= refAddr ^ ( long ) Node;
    }
    st_free_gen( gen );
    gen = NULL;

    /* Choose the mask. */
    for ( i = 0; ( unsigned ) i < 8 * sizeof( long ); i += 4 )
    {
        mask = ( 1 << i ) - 1;
        if ( diff <= mask )
            break;
    }


    // write the buffer for the output
    fprintf( pFile, ".names %s%lx %s\n", Prefix, ( mask & (long)Cudd_Regular(Func) ) / sizeof(DdNode), OutputName ); 
    fprintf( pFile, "%s 1\n", (Cudd_IsComplement(Func))? "0": "1" );


    invertors = st_init_table( st_ptrcmp, st_ptrhash );

    gen = st_init_gen( visited );
    while ( st_gen( gen, ( char ** ) &Node, NULL ) )
    {
        if ( Node->index == CUDD_MAXINDEX )
        {
            // write the terminal node
            fprintf( pFile, ".names %s%lx\n", Prefix, ( mask & (long)Node ) / sizeof(DdNode) );
            fprintf( pFile, " %s\n", (cuddV(Node) == 0.0)? "0": "1" );
            continue;
        }

        Else  = cuddE(Node);
        ElseR = Cudd_Regular(Else);
        Then  = cuddT(Node);

        if ( InputNames )
        {
            assert( InputNames[Node->index] );
        }
        if ( Else == ElseR )
        { // no inverter
            if ( InputNames )
                fprintf( pFile, ".names %s", InputNames[Node->index] );
            else
                fprintf( pFile, ".names x%d", Node->index );

            fprintf( pFile, " %s%lx %s%lx %s%lx\n",
                              Prefix, ( mask & (long)ElseR ) / sizeof(DdNode),
                              Prefix, ( mask & (long)Then  ) / sizeof(DdNode),
                              Prefix, ( mask & (long)Node  ) / sizeof(DdNode)   );
            fprintf( pFile, "01- 1\n" );
            fprintf( pFile, "1-1 1\n" );
        }
        else
        { // inverter
            if ( InputNames )
                fprintf( pFile, ".names %s", InputNames[Node->index] );
            else
                fprintf( pFile, ".names x%d", Node->index );

            fprintf( pFile, " %s%lx_i %s%lx %s%lx\n", 
                              Prefix, ( mask & (long)ElseR ) / sizeof(DdNode),
                              Prefix, ( mask & (long)Then  ) / sizeof(DdNode),
                              Prefix, ( mask & (long)Node  ) / sizeof(DdNode)   );
            fprintf( pFile, "01- 1\n" );
            fprintf( pFile, "1-1 1\n" );

            if ( !st_find_or_add( invertors, (char*)ElseR, NULL ) )
            {
                // write the intertor
                fprintf( pFile, ".names %s%lx %s%lx_i\n",  
                                  Prefix, ( mask & (long)ElseR  ) / sizeof(DdNode),
                                  Prefix, ( mask & (long)ElseR  ) / sizeof(DdNode)   );
                fprintf( pFile, "0 1\n" );
            }
        }
    }
    st_free_gen( gen );
    gen = NULL;
    st_free_table( visited );
    st_free_table( invertors );
}

