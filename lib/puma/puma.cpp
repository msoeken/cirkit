/****************************************************************************/
//
//    	Source for compilation of
//
//      PUMA
//
//    	OKFDD-Package
//
//   	Copyright (C) 1994-96
//
//      Author: Andreas Hett
//      email:  hett@informatik.uni-freiburg.de
//      Web:    www.informatik.uni-freiburg.de/FREAK/hett.html
//
//      All rights reserved
//
/*****************************************************************************/



/*****************************************************************************/
//
//	>>>     INCLUDE FILES     <<<
//
/*****************************************************************************/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tc_time.h"
#include "puma.h"

using namespace std;


/*****************************************************************************/
//
//	>>>    MACRO DEFINITIONS     <<<
//
/*****************************************************************************/

// Loop definitions
// ----------------
#define	fui(x,y,z) 		for(short x=y; x<=z; x++)
#define	fdi(x,y,z)  		for(short x=y; x>=z; x--)
#define	fu(x,y,z)    		for(      x=y; x<=z; x++)
#define	fd(x,y,z)	       	for(      x=y; x>=z; x--)
#define	fu1(x,y,z)	      	for(      x=y; x<z;  x++)

// Bit manipulators
// ----------------
#define	m_orh(x,y)	      	(utnode*)((ulint)x |  (2*y))
#define	m_andh(x)	       	(utnode*)((ulint)x & -3)
#define	m_or(x,y)	       	(utnode*)((ulint)x |  y)
#define	m_xor(x,y)	      	(utnode*)((ulint)x ^  y)
#define	m_selh(x)	       	((ulint)x & 2) >> 1
#define	m_sel(x)		((ulint)x & 1)

#define	um_selh(x)	       	(x & 2) >> 1
#define	um_orh(x,y)	      	(x | (2*y))
#define um_and(x)               (x & 254)
#define	um_andh(x)	       	(x & 253)
#define	um_or(x,y)	       	(x | y)
#define	um_xor(x,y)	      	(x ^ y)
#define	um_sel(x)		(x & 1)

#define	gnc(z,o,f,g,r,n)	if (cc_last==NULL) z = new ctnode(); else cc_last=(ctnode*)(z=cc_last)->R;                fcpz(z,o,f,g,r,n)
#define	fcpz(z,o,f,g,r,n)      	z->Code = (((ulint)f & 1) << 4) + (((ulint)g & 1) << 3) + ((m_and(f))->switch_flag << 2); fcpx(z,o,f,g,r,n)
#define	fcpx(z,o,f,g,r,n)	z->Code += ((m_and(g))->switch_flag << 1) + (m_and(r))->switch_flag;	                  fcpy(z,o,f,g,r,n)
#define	fcpy(z,o,f,g,r,n)	z->Code = (o << 5) + z->Code;                                                             fcp2(z,o,f,g,r,n)
#define	fcp2(z,o,f,g,r,n)      	z->F_idnum = (m_and(f))->idnum; z->G_idnum = (m_and(g))->idnum;	                          fcp3(z,o,f,g,r,n)
#define	fcp3(z,o,f,g,r,n)      	z->R_idnum = (m_and(r))->idnum; z->R = r; z->next = n

#define	gnus(z,a,b,c,d,e)      	if (uc_last==NULL) z = new utnode(); else uc_last=(utnode*)(z=uc_last)->link;             fups(z,a,b,c,d,e)
#define	fups(z,a,b,c,d,e)      	z->label_i = a; z->hi_p = b; 				                                  fupx(z,a,b,c,d,e)
#define	gnuz(z,a,b,c,d,e)      	if (uc_last==NULL) z = new utnode(); else uc_last=(utnode*)(z=uc_last)->link;             fupz(z,a,b,c,d,e)
#define	fupz(z,a,b,c,d,e)      	OKFDD_No_UTNodes_Per_Lvl[a]++; OKFDD_No_UTNodes++; z->label_i = a; z->hi_p = b;           fupx(z,a,b,c,d,e)
#define	fupx(z,a,b,c,d,e)      	z->lo_p = c; z->idnum = d; z->ref_c = e; z->next = NULL; z->link = NULL; z->switch_flag = 0; z->flag = 0

/* To use 'MORE' the next two assignments must be added to the 'fupx'-macro:    z->MSET = z->MNOM = NULL */

#define	CHECK_MEM	        if_cond1 if_cond2 { if_cond3 { warning } Overflow = TRUE; OKFDD_Error = Err_Overflow; }
#define	if_cond1		if (memory_loop++ % 10 == 0)
#define	if_cond2		if ((OKFDD_No_UTNodes >= ML_nodes) || (OKFDD_No_UTNodes*Size_of_utnodes+OKFDD_No_CTNodes*Size_of_utnodes+Overhead >= ML_MB*1000000))
#define	if_cond3		if (Overflow == FALSE)

#define	Prozentbalken(z,n,b)   	if (OKFDD_Outputflags & ros_p) {pbkt1(z,n) pbkt2(b) pbkt3}
#define	pbkt1(z,n)	      	opos = (int)((float)(OKFDD_Dots_Out_Limit*(z))/(n));cout << "\r<";
#define	pbkt2(b)		for (sift = 0; sift < opos; sift++) cout << b;for (sift = opos; sift < OKFDD_Dots_Out_Limit; sift++) cout << "-";
#define	pbkt3		   	cout << "> \tSize " << OKFDD_Now_size_i << "   \t";cout.flush();

#define	resize1(x,z)	    	OKFDD_Now_size_i += (z * (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[x]] + OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[x+1]]))
#define	resize2(x,y,z)	  	if (x > y) { J0=x; x=y; y=J0; } for(J0=x; J0<=y; J0++) OKFDD_Now_size_i += (z * OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[J0]])

#define up(x)		   	if (x->ref_c != 0) (x->ref_c++)
#define do(x)		   	if (x->ref_c != 0) (x->ref_c--)
#define ups(x)		  	if ( ( (utnode*)((ulint)x & -2) )->ref_c != 0) (( (utnode*)((ulint)x & -2) )->ref_c++)
#define dos(x)		  	if ( ( (utnode*)((ulint)x & -2) )->ref_c != 0) (( (utnode*)((ulint)x & -2) )->ref_c--)

// Output definitions
// ------------------
#define cout(x)			if (OKFDD_Outputflags & x) cout
#define breakline	       	cout << "--------------------------------------------------------------------------------\n"
#define breakline1	      	cout(1) << "--------------------------------------------------------------------------------\n"
#define nextline		cout << "\n"
#define warning			cout(2) << "\nFOAUT: WARNING ! All following results may be incorrect due to MEMORY_LIMIT_OVERFLOW ...\n";
#define Version			">> PUMA * OKFDD_Manager V2.22 Copyright (C)'96 by Andreas HETT <<"

#define DUMMY		   	999
#define line_limit	      	5000
#define line_max		30000
#define bucket_max	      	20    // Setting is ONLY useful for OKFDD_Blocksize '2' -> Allows '2^(bucket_max-1)' lines in BLIF
#define c0		      	'\0'
#define c1		      	'\1'
#define pi_hashsize	     	1021
#define Overheadit	      	Overhead = pi_limit*(OH_Gap+37)+4*pi_hashsize+5*2*ct_hashsize+sizeof(rcnode)*rc_cachesize+OH_Basis
#define OH_Basis		1000000
#define OH_Gap		  	5
#define ros_y		   	131072
#define ros_a		   	262144
#define ros_b		   	524288
#define dumpflag		1048576
#define ros_p		   	2097152
#define pno		     	4194304

#define setfromlaba(x)		K3 = x; hrhelp = NULL
#define setfromlabb(x)		K3 = x


/*****************************************************************************/
//
//	>>>    GLOBAL VARIABLES    <<<
//
/*****************************************************************************/

usint	   	K0,K1,K2,K3,K4;		//	global usint counters
short	   	J0,J1,J2,J3,J4;	        //	global short counters
ulint	   	I0,I1,I2,I3,I4,I5;	//	global ulint counters

ulint Measuremax;
ulint Measureact;

char	    	gl_name[name_limit];
char	    	line1[line_limit];
char		line[line_limit];
// msoeken
std::string gl_input_names, gl_output_names;

names*		nameshelp;
hnode*		hrhelp;
ctnode*		ctlast;
ctnode*		cthelp;
rcnode*		rchelp;
utnode*		twin;
utnode*		hz;
utnode*		Pure_lo;
utnode*		Pure_hi;
utnode*		utlast;
utnode*		uthelp;
utnode**	ut_switch;
utnode*		uth1;
utnode*		newlo;
utnode*		newhi;
son_ptr*	VL_help;
son_ptr*	VL_first;
son_ptr*	VL_last;
son_ptr*	hhelp;
ofstream	streamhelp;


/*****************************************************************************/
//
//	>>>     CONSTRUCTORS    <<<
//
/*****************************************************************************/

/* ========================================================================= */
// Constructor:	 rcnode::rcnode				       		     //
/* ========================================================================= */
rcnode::rcnode(utnode* root_init)
{
	root_init	= m_and(root_init);

	idnum	   	= root_init->idnum;
	root	    	= root_init;
	next	    	= NULL;
};
/* ========================================================================= */


/* ========================================================================= */
// Constructor:	 dd_man::dd_man				       		     //
/* ========================================================================= */
dd_man::dd_man( uchar   ut_hashsize_init,
		ulint   ct_hashsize_init,
		ulint   rc_cachesize_init,
		uchar   ct_searchlen_init,
		usint   pi_limit_init,
		ulint   ML_nodes_init,
		ulint   ML_MB_init)
{
	// Set extern parameters
	// ---------------------
	ct_hashsize     	= ct_hashsize_init;
	rc_cachesize    	= rc_cachesize_init;
	ct_searchlen    	= ct_searchlen_init;
	pi_limit		= pi_limit_init;
	ML_nodes		= ML_nodes_init;
	ML_MB	   		= ML_MB_init;

	// Set intern parameters
	// ---------------------
	OKFDD_Subst_Method      = FALSE;
	OKFDD_Use_Order	 	= FALSE;
	OKFDD_Version_Wait      = TRUE;
	OKFDD_DTL_Default       = D_Shan;
	OKFDD_Interleaving      = FALSE;
	OKFDD_Temproutine       = 0;
	OKFDD_Tempfactor	= 1.5;
	OKFDD_Siftfactor	= 3;
	OKFDD_Siftbase	  	= 10000;
	OKFDD_Prof_Out_Limit    = 42;
	OKFDD_Dots_Out_Limit    = 39;
	OKFDD_Blocksize	 	= 2;
	OKFDD_Explodefactor     = 21;
	OKFDD_Implodefactor     = 5;

	memory_loop             = 0;

	// Turn on Computed Table
	// ----------------------
	OKFDD_CT	   	= TRUE;
	
	// Set Outputflags
	// ---------------
	OKFDD_Outputflags       = 2097191;

	// Set Interleaving values
	// -----------------------
	OKFDD_Min_FANIN	 	= 255;
	OKFDD_Mulend	    	= 100;
	OKFDD_Hiend	     	= .75;
	OKFDD_Bitend	    	= 16416;

	// Set external Shell-Function
	// ---------------------------
	OKFDD_Shell_Extension   = NULL;

	field		   	= new char*[line_max];

	// ID-Counter starts above DUMMY
	// -----------------------------
	counter		 	= DUMMY + 1;

	// Store ut_hashsizes in ut_sizes
	// ------------------------------
	ulint ut_hash[ut_max+1] = {	3,	7,	17,	31,
				   	61,	127,	257,	509,
				       	1021,	2053,	4099,	8191,
					16381,	32771,	65521		};

	fu(I0,0,ut_max) ut_sizes[I0] = ut_hash[I0];

	// Initialize Overflow values
	// --------------------------
	Overflow	= FALSE;
	Size_of_utnodes = sizeof(utnode);
	Size_of_ctnodes = sizeof(ctnode);
	Overheadit;

	// No ZSBDDs at startup time
	// -------------------------
	ZSBDD_Init_done = FALSE;

	// Define some outputchars
	// -----------------------
	dtl[0]	  	= 'S';
	dtl[1]	  	= 'P';
	dtl[2]	  	= 'N';
	outchar[0]      = '0';
	outchar[1]      = '1';
	outchar[2]      = '-';

	// Initialize ct-statistic fields
	// ------------------------------
/**/    ctentry = ctlookup = ctlookuphit = ctlookuphitno = 0;

/**/	OKFDD_No_CTNodes = 0;

	// Allocate table for ut hashsizes
	// -------------------------------
	ut_hashsize_d   = ut_hashsize_init; ut_hashsize = new uchar[pi_limit];

	// Init unique / computed table caches
	// -----------------------------------
	uc_last = NULL; cc_last = NULL;
	
	fu(I0,1,10000)
	{
		utlast 		= new utnode();
		utlast->idnum 	= DUMMY;
		utlast->link 	= uc_last;
		uc_last 	= utlast;
	}

	fu(I0,1,ct_hashsize*ct_searchlen)
	{
		ctlast 		= new ctnode();
		ctlast->R 	= (utnode*)cc_last;
		cc_last 	= ctlast;
	}

	// Create hashtable for primary inputs, outputs and temporary funtions
	// -------------------------------------------------------------------
	prime   = new names*[pi_hashsize];
	toprime = new names*[pi_limit];

	fu1(I0,0,pi_hashsize) prime[I0]   = NULL;
	fu1(I0,0,pi_limit)    toprime[I0] = NULL;

	// Create primary storage tables
	// -----------------------------
	All_Prime       = new usint[pi_limit];
	Act_Prime       = new usint[pi_limit];
	T_Prime	 	= new usint[pi_limit];

	ZSBDD_equals    = new usint[pi_limit];

	fu1(I0,0,pi_limit) All_Prime[I0]    = 0;
	fu1(I0,0,pi_limit) Act_Prime[I0]    = 0;
	fu1(I0,0,pi_limit) T_Prime[I0]      = 0;
	fu1(I0,0,pi_limit) ZSBDD_equals[I0] = 0;

	// Allocate memory for reordering fields
	// -------------------------------------
	mindtl	       	= new   uchar[pi_limit];
	aa	      	= new   short[pi_limit];
	min_pi_table    = new   usint[pi_limit];
	twin_pi_table	= new   usint[pi_limit];

	fu1(I0,0,pi_limit) mindtl[I0]	  	= 0;
	fu1(I0,0,pi_limit) aa[I0]	      	= 0;
	fu1(I0,0,pi_limit) min_pi_table[I0]	= 0;
	fu1(I0,0,pi_limit) twin_pi_table[I0]   	= 0;

	// Allocate memory for SAT fields
	// ------------------------------
	pfada	   	= new   uchar[pi_limit];
	pfads	   	= new   uchar[pi_limit];
	lsf   		= new   uchar[pi_limit];
	pi_level	= new   usint[pi_limit];

	fu1(I0,0,pi_limit) pfada[I0]	   	= 0;
	fu1(I0,0,pi_limit) pfads[I0]		= 0;
	fu1(I0,0,pi_limit) lsf[I0]	  	= 0;
	fu1(I0,0,pi_limit) pi_level[I0]		= 0;

	// Create help tables
	// ------------------
	OKFDD_Result	   	= new    usint[pi_limit];
	OKFDD_PI_Order_Table   	= new    usint[pi_limit];
	OKFDD_Result_2	 	= new long int[pi_limit];

	fu1(I0,0,pi_limit) OKFDD_Result[I0]	   	= 0;
	fu1(I0,0,pi_limit) OKFDD_PI_Order_Table[I0]	= 0;
	fu1(I0,0,pi_limit) OKFDD_Result_2[I0]	 	= 0;
	
	// Set initial values of PI, PO and Temp.Fcts counter
	// --------------------------------------------------
	OKFDD_P_I = OKFDD_P_O = T_F = 0;

	// Create decomp. table, order table and counter for number of utnodes
	// -------------------------------------------------------------------
	OKFDD_PI_DTL_Table       = new uchar[pi_limit];
	OKFDD_PI_Level_Table     = new short[pi_limit];
	OKFDD_No_UTNodes_Per_Lvl = new ulint[pi_limit];

	fu1(I0,0,pi_limit) OKFDD_PI_DTL_Table[I0]       = 0;
	fu1(I0,0,pi_limit) OKFDD_PI_Level_Table[I0]     = 0;
	fu1(I0,0,pi_limit) OKFDD_No_UTNodes_Per_Lvl[I0] = 0;

	// ONE & ZERO with label_i 0 need special entries for top var searches
	// -------------------------------------------------------------------
	OKFDD_PI_Level_Table[0] = pi_limit+1; OKFDD_PI_DTL_Table[0] = 0;

	// Init unique-hash-tab and computed-hash-tab
	// ------------------------------------------
	ut  = new utnode**[pi_limit];
	ct  = new ctnode*[ct_hashsize*2];
	ctl = new uchar[ct_hashsize*2];

	fu1(I0,0,pi_limit)      ut[I0]  = NULL;
	fu1(I0,0,ct_hashsize*2) ct[I0]  = NULL;
	fu1(I0,0,ct_hashsize*2) ctl[I0] = 0;

	// Create ONE and (faked ZERO which exists only for ptr test purposes)
	// -------------------------------------------------------------------
	gnuz(OKFDD_ONE,0,NULL,NULL,1,0); OKFDD_ZERO = m_xor(OKFDD_ONE,1);

	ZSBDD_Empty = OKFDD_ZERO; ZSBDD_Base = OKFDD_ONE;

	gnuz(OKFDD_DUMMY,0,OKFDD_ZERO,OKFDD_ONE,1,0);

	OKFDD_No_UTNodes = 0;

	// Create dummys for rc_cache to avoid time consuming special checks
	// -----------------------------------------------------------------
	rchelp = new rcnode(OKFDD_ONE); rc_cache_last = rchelp;

	fu(I0,0,rc_cachesize-2)
	{
		rc_cache_last->next = new rcnode(OKFDD_ONE);
		rc_cache_last 	    = rc_cache_last->next;
	}

	rc_cache_last->next = rchelp;

	// Set first label
	// ---------------
	OKFDD_Next_Label = 1;
	
	// The following section is NOT needed to run properly ->
	// It's used in combination with the multiple-OR routine 'MORE'
	// MORE isn't activated because of the problems with Davio types ->
	// A Shannon-only package will MIGHTILY improve with it
	// The unfortunate problems with Davio types are shown in our descrip.
/*
	// Build 32 connectors (label 1 - 32 with OKFDD_PI_Level_Table 2 to 64)
	// --------------------------------------------------------------------
	fu(I0,1,32)
	{
	     get_name(PI,OKFDD_Next_Label);

	     nameshelp				      = Make_Prime(PI);
	     OKFDD_PI_Level_Table[nameshelp->label_i] = 2 * nameshelp->label_i;
	     OKFDD_PI_DTL_Table[nameshelp->label_i]   = D_Shan;
	}
*/
};
/* ========================================================================= */


/* ========================================================================= */
// Initialize gate costs vector with linecosts values			     //
/* ========================================================================= */
const usint dd_man::line_costs [3][17] = { { 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 }, 
					    { 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1 }, 
					    { 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1 } };
/* ========================================================================= */


/* ========================================================================= */
// Initialize gate costs vector with quantumcosts values		     //
/* ========================================================================= */
const usint dd_man::quantum_costs [3][17] = { { 11, 12, 6, 11, 0, 2, 0, 1, 6, 7, 5, 6, 6, 5, 1, 1, 0 }, 
		       { 6, 7, 5, 6, 6, 7, 6, 7, 0, 2, 0, 0, 5, 6, 0, 0, 1 }, 
		       { 7, 8, 6, 7, 5, 6, 5, 6, 0, 2, 0, 0, 6, 7, 0, 0, 1 } };
/* ========================================================================= */


/* ========================================================================= */
// Constructor:	 names::names						     //
/* ========================================================================= */
names::names(   uchar   type_init,
		usint   label_init)
{
	uchar loop; fu1(loop,0,name_limit) name[loop] = gl_name[loop];

	label_i = label_init;   type    = type_init;
	root    = NULL;	 	next    = NULL;
	link	= NULL;		group	= 0;		related_var = 0;
};
/* ========================================================================= */


/* ========================================================================= */
// Constructor:	 hnode::hnode	       					     //
/* ========================================================================= */
hnode::hnode(   usint   label_init,
		uchar   type_init       )
{
	label_i = label_init;   type    = type_init;    next    = NULL;
	FANIN   = 0;	    	ref_c   = 0;	    	weight  = 0;
	depth   = 0;	    	no_nodes= 0;
	flag    = 0;	    	from    = 0;
};
/* ========================================================================= */


/* ========================================================================= */
// Constructor:	 son_ptr::son_ptr	       				     //
/* ========================================================================= */
son_ptr::son_ptr(	usint	   son_init,
			son_ptr*   next_init = NULL)
{
	son_label = son_init;   next = next_init;
};
/* ========================================================================= */



/*****************************************************************************/
//
//	>>>     SYNTHESIS OPERATIONS    <<<
//
/*****************************************************************************/

/* ========================================================================= */
// Function:    dd_man::OKFDD_Synthesis // Synthesis function	   	     //
/* ========================================================================= */
utnode* dd_man::OKFDD_Synthesis(usint   Code,
				utnode* F,
				utnode* G)
{
	// Check for NULL-Operands
	// -----------------------
	if ((F == NULL) || (G == NULL))
	{
		OKFDD_Error = Err_NULL_Op; return OKFDD_Root[0] = NULL;
	}
	else 	OKFDD_Error = Err_No_Error;

	// Check for default setting of operand 2
	// --------------------------------------
	if ((long)G == 1) G = OKFDD_ONE;

	// Save operands (Increase reference counters)
	// -------------------------------------------
	ups(F); ups(G);

	// Call synthesis procedure
	// ------------------------
	switch (Code)
	{
		case C_ID:	OKFDD_Root[0] = F;
				break;    //   F
		case C_NOT:	OKFDD_Root[0] = m_xor(F,1);
				break; 	  // ~ F
		case C_NIMF:	OKFDD_Root[0] = m_xor(CO_OR(F,m_xor(G,1)),1);
				break;	  // ~(G =>F)
		case C_XOR:	OKFDD_Root[0] = CO_XOR(F,G);
				break;	  //   F # G
		case C_OR:	OKFDD_Root[0] = CO_OR(F,G);
				break;	  //   F + G
		case C_NOR:	OKFDD_Root[0] = m_xor(CO_OR(F,G),1);
				break;	  // ~(F + G)
		case C_NAND:	OKFDD_Root[0] = CO_OR(m_xor(F,1),m_xor(G,1));
				break;	  // ~(F * G)
		case C_EQUI:	OKFDD_Root[0] = m_xor(CO_XOR(F,G),1);
				break;	  // ~(F # G)
		case C_IMPG:	OKFDD_Root[0] = CO_OR(m_xor(F,1),G);
				break;	  //   F =>G
		case C_IMPF:	OKFDD_Root[0] = CO_OR(F,m_xor(G,1));
				break;	  //   G =>F
		case C_AND:	OKFDD_Root[0] = m_xor(CO_OR(m_xor(F,1),
						  	    m_xor(G,1)),1 );
				break;	  //   F * G

		default:	dos(F); dos(G); OKFDD_Error = Err_Unknown_Op;
				return OKFDD_Root[0] = NULL;
	}

	// Reset operands and save result
	// ------------------------------
	dos(F); dos(G); ups(OKFDD_Root[0]);

	return OKFDD_Root[0];
};
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_ITE // returns pointer to F*G+~F*H	     //
/* ========================================================================= */
utnode* dd_man::OKFDD_ITE(      utnode* F,
				utnode* G,
				utnode* H)
{
	// Check for NULL-Operands
	// -----------------------
	if ((F == NULL) || (G == NULL) || (H == NULL))
	{
		OKFDD_Error = Err_NULL_Op; return OKFDD_Root[0] = NULL;
	}
	else 	OKFDD_Error = Err_No_Error;

	utnode* help1 = m_xor(CO_OR(m_xor(F,1),m_xor(G,1)),1);
	ups(help1);	        //  F*G
	utnode* help2 = m_xor(CO_OR(F,m_xor(H,1)),1);
	ups(help2);	        // ~F*H
	OKFDD_Root[0] = CO_OR(help1,help2);
	ups(OKFDD_Root[0]);     // ~F*H + F*G = ITE(F,G,H)

	OKFDD_Free_Node(help1); OKFDD_Free_Node(help2);

	return OKFDD_Root[0];
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::CO_OR // OR function				     //
/* ========================================================================= */
utnode* dd_man::CO_OR(  utnode* F,
			utnode* G)
{
	usint	 v;
	utnode*	 low;
	utnode*	 high;
	utnode*	 R;

	utnode*	 uthelp2;
	utnode*	 uthelp3;
	utnode*	 uthelp4;

	// Check terminal cases
	// --------------------
	if (F == G)	     return F;
	if (F == OKFDD_ONE)  return OKFDD_ONE;
	if (F == OKFDD_ZERO) return G;
	if (G == OKFDD_ONE)  return OKFDD_ONE;
	if (G == OKFDD_ZERO) return F;

	utnode* Pure_F  = m_and(F);
	utnode* Pure_G  = m_and(G);
	uchar   flag_F  = m_sel(F);
	uchar   flag_G  = m_sel(G);

	if (Pure_F == Pure_G) if (flag_F != flag_G) return OKFDD_ONE;

	// ------------------------------
	// Search entry in computed table
	// ------------------------------
	if (Pure_F->label_i == Pure_G->label_i) J0 = ct_hashsize;
	else				    	J0 = 0;

	if ((R = CTLO(C_OR,F,G)) != NULL) return R;

	// --------------------------
	// Calculate low and high son
	// --------------------------
	if (OKFDD_PI_Level_Table[Pure_F->label_i] ==
	    OKFDD_PI_Level_Table[Pure_G->label_i])
	{
		if (OKFDD_PI_DTL_Table[v = Pure_G->label_i] == D_Shan)
		{
			low  = CO_OR(m_xor(Pure_F->lo_p,flag_F),
				     m_xor(Pure_G->lo_p,flag_G)); ups(low);
			high = CO_OR(m_xor(Pure_F->hi_p,flag_F),
				     m_xor(Pure_G->hi_p,flag_G)); dos(low);

			if (low == high) return low;
		}
		else
		{
			low  = CO_OR(m_xor(Pure_F->lo_p,flag_F),
				     m_xor(Pure_G->lo_p,flag_G)); ups(low);

			if (m_xor(Pure_F->lo_p,flag_F) == Pure_F->hi_p)
			{
				uthelp2 = CO_XOR(m_xor(Pure_G->lo_p,flag_G),
						 Pure_G->hi_p);   ups(uthelp2);
				high    = CO_XOR(low,uthelp2);	  ups(high);
				
				OKFDD_Free_Node(uthelp2);
			}
			else if (m_xor(Pure_G->lo_p,flag_G) == Pure_G->hi_p)
			{
				uthelp2 = CO_XOR(m_xor(Pure_F->lo_p,flag_F),
						 Pure_F->hi_p);   ups(uthelp2);
				high    = CO_XOR(low,uthelp2);	  ups(high);
				
				OKFDD_Free_Node(uthelp2);
			}
			else
			{
				uthelp3 = CO_XOR(m_xor(Pure_F->lo_p,flag_F),
						 Pure_F->hi_p);   ups(uthelp3);
				uthelp4 = CO_XOR(m_xor(Pure_G->lo_p,flag_G),
						 Pure_G->hi_p);   ups(uthelp4);
				uthelp2 = CO_OR(uthelp3,uthelp4); ups(uthelp2);

				OKFDD_Free_Node(uthelp3);
				OKFDD_Free_Node(uthelp4);

				high    = CO_XOR(low,uthelp2);	  ups(high);
				
				OKFDD_Free_Node(uthelp2);
			}

			dos(low); dos(high);
			
			if (high == OKFDD_ZERO) return low;
		}
	}
	else
	{
		if (OKFDD_PI_Level_Table[Pure_F->label_i] <
		    OKFDD_PI_Level_Table[Pure_G->label_i])
		{
			if (OKFDD_PI_DTL_Table[v = Pure_F->label_i] == D_Shan)
			{
				low  = CO_OR(m_xor(Pure_F->lo_p,flag_F),
					     G);		  ups(low);
				high = CO_OR(m_xor(Pure_F->hi_p,flag_F),
					     G);		  dos(low);

				if (low == high) return low;
			}
			else
			{
				if (m_xor(Pure_F->lo_p,flag_F) == Pure_F->hi_p)
				{
					low  =
					CO_OR(m_xor(Pure_F->lo_p,flag_F),G);
					ups(low);
					high = CO_XOR(low,G);
					dos(low);

					if (high == OKFDD_ZERO) return low;
				}
				else
				{
					low  =
					CO_OR(m_xor(Pure_F->lo_p,flag_F),G);
					ups(low);
					high = CO_OR(m_xor(Pure_F->hi_p,1),G);
					dos(low);

					if (high == OKFDD_ONE) return low;
					
					high = m_xor(high,1);
				}
			}
		}
		else
		{
			if (OKFDD_PI_DTL_Table[v = Pure_G->label_i] == D_Shan)
			{
				low  = CO_OR(F,m_xor(Pure_G->lo_p,flag_G));
				ups(low);
				high = CO_OR(F,m_xor(Pure_G->hi_p,flag_G));
				dos(low);

				if (low == high) return low;
			}
			else
			{
				low  = CO_OR(F,m_xor(Pure_G->lo_p,flag_G));
				ups(low);
				high = CO_OR(F,m_xor(Pure_G->hi_p,1));
				dos(low);

				if (high == OKFDD_ONE) return low;
				
				high = m_xor(high,1);
			}
		}
	}

	// Find or add (v,high,low) in unique table
	// ----------------------------------------
	R = FOAUT(v,high,low);

	// Insert (Code,F,G,R) in computed table
	// -------------------------------------
	if (Pure_F->label_i == Pure_G->label_i) J0 = ct_hashsize;
	else				    	J0 = 0;

	ICT(C_OR,F,G,R);

	return R;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::CO_XOR // XOR function				     //
/* ========================================================================= */
utnode* dd_man::CO_XOR( utnode* F,
			utnode* G)
{
	usint	 v;
	utnode*	 low;
	utnode*	 high;
	utnode*	 R;

	// Check terminal cases
	// --------------------
	if (F == G)	     return OKFDD_ZERO;
	if (G == OKFDD_ZERO) return F;
	if (F == OKFDD_ZERO) return G;
	if (F == OKFDD_ONE)  return m_xor(G,1);
	if (G == OKFDD_ONE)  return m_xor(F,1);

	utnode*	 Pure_F  = m_and(F);
	utnode*	 Pure_G  = m_and(G);
	uchar	 flag_F  = m_sel(F);
	uchar	 flag_G  = m_sel(G);

	if (Pure_F == Pure_G) if (flag_F != flag_G) return OKFDD_ONE;

	// ------------------------------
	// Search entry in computed table
	// ------------------------------
	if (Pure_F->label_i == Pure_G->label_i) J0 = ct_hashsize;
	else				    	J0 = 0;

	if ((R = CTLO(C_XOR,F,G)) != NULL) return R;

	// --------------------------
	// Calculate low and high son
	// --------------------------
	if (OKFDD_PI_Level_Table[Pure_F->label_i] ==
	    OKFDD_PI_Level_Table[Pure_G->label_i])
	{
		if (OKFDD_PI_DTL_Table[v = Pure_G->label_i] == D_Shan)
		{
			low  = CO_XOR(m_xor(Pure_F->lo_p,flag_F),
				      m_xor(Pure_G->lo_p,flag_G)); ups(low);
			high = CO_XOR(m_xor(Pure_F->hi_p,flag_F),
				      m_xor(Pure_G->hi_p,flag_G)); dos(low);

			if (low == high) return low;
		}
		else
		{
			low  = CO_XOR(m_xor(Pure_F->lo_p,flag_F),
				      m_xor(Pure_G->lo_p,flag_G)); ups(low);
			high = CO_XOR(Pure_F->hi_p,Pure_G->hi_p);  dos(low);

			if (high == OKFDD_ZERO) return low;
		}
	}
	else
	{
		if (OKFDD_PI_Level_Table[Pure_F->label_i] <
		    OKFDD_PI_Level_Table[Pure_G->label_i])
		{
			if (OKFDD_PI_DTL_Table[v = Pure_F->label_i] == D_Shan)
			{
				low  = CO_XOR(m_xor(Pure_F->lo_p,flag_F),G);
				ups(low);
				high = CO_XOR(m_xor(Pure_F->hi_p,flag_F),G);
				dos(low);

				if (low == high) return low;
			}
			else
			{
				low  = CO_XOR(m_xor(Pure_F->lo_p,flag_F),G);
				high = Pure_F->hi_p;
			}
		}
		else
		{
			if (OKFDD_PI_DTL_Table[v = Pure_G->label_i] == D_Shan)
			{
				low  = CO_XOR(F,m_xor(Pure_G->lo_p,flag_G));
				ups(low);
				high = CO_XOR(F,m_xor(Pure_G->hi_p,flag_G));
				dos(low);

				if (low == high) return low;
			}
			else
			{
				low  = CO_XOR(F,m_xor(Pure_G->lo_p,flag_G));
				high = Pure_G->hi_p;
			}
		}
	}

	// Find or add (v,high,low) in unique table
	// ----------------------------------------
	R = FOAUT(v,high,low);

	// Insert (Code,F,G,R) in computed table
	// -------------------------------------
	if (Pure_F->label_i == Pure_G->label_i) J0 = ct_hashsize;
	else				    	J0 = 0;

	ICT(C_XOR,F,G,R);

	return R;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Exists // Returns ptr on F(lab = 0)+F(lab = 1) //
/* ========================================================================= */
utnode* dd_man::OKFDD_Exists(   utnode* F,
				usint   lab)
{
	// Get cofactors F0 and F1
	// -----------------------
	OKFDD_Cofactor(F,lab);

	// Check for NULL-Operand
	// ----------------------
	if (OKFDD_Error != Err_No_Error) return OKFDD_Root[0] = NULL;

	// Calculate F0 * F1
	// -----------------
	utnode* result = OKFDD_Synthesis(C_OR,OKFDD_Root[0],OKFDD_Root[1]);

	// Free F0 and F1 (cofactors)
	// --------------------------
	OKFDD_Free_Node(OKFDD_Root[0]); OKFDD_Free_Node(OKFDD_Root[1]);

	return OKFDD_Root[0] = result;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Forall // Returns ptr on F(lab = 0)*F(lab = 1) //
/* ========================================================================= */
utnode* dd_man::OKFDD_Forall(   utnode* F,
				usint   lab)
{
	// Get cofactors F0 and F1
	// -----------------------
	OKFDD_Cofactor(F,lab);

	// Check for NULL-Operand
	// ----------------------
	if (OKFDD_Error != Err_No_Error) return OKFDD_Root[0] = NULL;

	// Calculate F0 * F1
	// -----------------
	utnode* result = OKFDD_Synthesis(C_AND,OKFDD_Root[0],OKFDD_Root[1]);

	// Free F0 and F1 (cofactors)
	// --------------------------
	OKFDD_Free_Node(OKFDD_Root[0]); OKFDD_Free_Node(OKFDD_Root[1]);

	return OKFDD_Root[0] = result;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Depend // Returns (F(lab = 0) xor F(lab = 1))  //
/* ========================================================================= */
utnode* dd_man::OKFDD_Depend(   utnode* F,
				usint   lab)
{
	// Get cofactors F0 and F1
	// -----------------------
	OKFDD_Cofactor(F,lab);

	// Check for NULL-Operand
	// ----------------------
	if (OKFDD_Error != Err_No_Error) return OKFDD_Root[0] = NULL;

	// Calculate F0 * F1
	// -----------------
	utnode* result = OKFDD_Synthesis(C_XOR,OKFDD_Root[0],OKFDD_Root[1]);

	// Free F0 and F1 (cofactors)
	// --------------------------
	OKFDD_Free_Node(OKFDD_Root[0]); OKFDD_Free_Node(OKFDD_Root[1]);

	return OKFDD_Root[0] = result;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Cofactor // Calculates F(lab = 0), F(lab = 1)  //
/* ========================================================================= */
void dd_man::OKFDD_Cofactor(    utnode* F,
				usint   lab)
{
	utnode* F0;
	utnode* F1;
	usint   loop;
	usint   Ld;
	usint   Lu = 0;

	OKFDD_Root[0] = OKFDD_Root[1] = NULL;

	// Check for NULL-Operand
	// ----------------------
	if (F == NULL) { OKFDD_Error = Err_NULL_Op; return; }
	else             OKFDD_Error = Err_No_Error;

	// Get level of F->label_i and lab
	// -------------------------------
	Ld = pi_limit; I0 = 0;

	while(OKFDD_PI_Order_Table[I0] != 0)
	{
		if (OKFDD_PI_Order_Table[I0] == lab                ) Ld = I0;
                if (OKFDD_PI_Order_Table[I0] == (m_and(F))->label_i) Lu = I0;
		I0++;
	}

	// lab in support list ? / No -> F0 and F1 equal F
	// -----------------------------------------------
	if (Ld == pi_limit)
	{
		cout(2) << "OKFDD_Cofactor: Label not supported ...\n";
		F0 = F1 = F; ups(F0); ups(F1);
	}
	else
	{
		// Switch label in top level (when needed)
		// ---------------------------------------
		fd(loop,Ld,Lu+1) OKFDD_Levelexchange(loop-1);

		// lab in support list of tree ? No -> F0 and F1 equal F
		// -----------------------------------------------------
		if ((m_and(F))->label_i != lab)
		{
			cout(2) << "OKFDD_Cofactor: Label not supported ...\n";
			F0 = F1 = F; ups(F0); ups(F1);
		}
		else
		{
			// -------------------
			// Calculate F0 and F1
			// -------------------
			F0 = F1 = NULL;

			switch(OKFDD_PI_DTL_Table[(m_and(F))->label_i])
			{
			   case D_Shan: F0 = m_xor((m_and(F))->lo_p,m_sel(F));
					F1 = m_xor((m_and(F))->hi_p,m_sel(F));
			                ups(F0); ups(F1); break;
			   case D_posD: F0 = m_xor((m_and(F))->lo_p,m_sel(F));
					F1 = OKFDD_Synthesis(C_XOR,F0,
					     (m_and(F))->hi_p); ups(F0); break;
			   case D_negD: F1 = m_xor((m_and(F))->lo_p,m_sel(F));
					F0 = OKFDD_Synthesis(C_XOR,F1,
					     (m_and(F))->hi_p); ups(F1); break;
			}
		}

		// Switch label in old level
		// -------------------------
		fu1(loop,Lu,Ld) OKFDD_Levelexchange(loop);
	}

	// Store results in global table
	// -----------------------------
	OKFDD_Root[0] = F0; OKFDD_Root[1] = F1;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Substitution // Substitutes lab in F with G    //
/* ========================================================================= */
utnode* dd_man::OKFDD_Substitution(     utnode* F,
					usint   lab,
					utnode* G)
{
	utnode* F0;
	utnode* F1;
	usint   loop;
	usint   Ld;
	usint   Lu = 0;

	// Check for NULL-Operand
	// ----------------------
	if ((F == NULL) || (G == NULL))
        {
	        OKFDD_Error = Err_NULL_Op; return NULL;
	}
	else    OKFDD_Error = Err_No_Error;

	// Get level of F->label_i and lab
	// -------------------------------
	Ld = pi_limit; I0 = 0;

	while(OKFDD_PI_Order_Table[I0] != 0)
	{
		if (OKFDD_PI_Order_Table[I0] == lab                ) Ld = I0;
		if (OKFDD_PI_Order_Table[I0] == (m_and(F))->label_i) Lu = I0;
		I0++;
	}

	// lab in support list ? / No -> F0 and F1 equal F
	// -----------------------------------------------
	if (Ld == pi_limit)
	{
		cout(2) << "OKFDD_Substitution: Label not supported ...\n";
		F0 = F1 = F; ups(F0); ups(F1);
	}
	else
	{
		// Switch label in top level (when needed)
		// ---------------------------------------
		fd(loop,Ld,Lu+1) OKFDD_Levelexchange(loop-1);

		// lab in support list of tree ? No -> F0 and F1 equal F
		// -----------------------------------------------------
		if ((m_and(F))->label_i != lab)
		{
		   cout(2) << "OKFDD_Substitution: Label not supported ...\n";
		   F0 = F1 = F; ups(F0); ups(F1);
		}
		else
		{
			// ----------------------
			// Calculate Substitution
			// ----------------------
			F0 = m_xor((m_and(F))->lo_p,m_sel(F));

			if (OKFDD_PI_DTL_Table[lab] == D_Shan)
			{
				F1 = m_xor((m_and(F))->hi_p,m_sel(F));
				OKFDD_Root[0] = CO_OR( m_xor(CO_OR(m_xor(F0,1),
						G),1),m_xor(CO_OR(m_xor(F1,1),
						m_xor(G,1)),1) );
			}
			else
			{
				F1 = (m_and(F))->hi_p;
				OKFDD_Root[0] = CO_XOR( F0,
						m_xor(CO_OR(m_xor(F1,1),
						m_xor(G,1)),1) );
			}

			// Switch label in old level
			// -------------------------
			fu1(loop,Lu,Ld) OKFDD_Levelexchange(loop);
		}
	}

	ups(OKFDD_Root[0]); return OKFDD_Root[0];
};
/* ========================================================================= */



/*****************************************************************************/
//
//	        >>>     STRUCTURE MAINTAINING FUNCTIONS     <<<
//
/*****************************************************************************/

/* ========================================================================= */
// Function:    dd_man::OKFDD_Read_BLIF	// Build OKFDD out of benchfile      //
/* ========================================================================= */
void dd_man::OKFDD_Read_BLIF(char*      FILEX_init,
			     char*      ORDER_init,
			     travex     functex   ,
			     usint*     Label_Tab ,
           uchar*     DTL_Tab ,
           std::string *input_labels,
           std::string *output_labels  )
{
	FILEX	   = FILEX_init;
	ORDER	   = ORDER_init;
	OKFDD_Use_Order = FALSE;

	if (ORDER != NULL)
	{
		cout(1) << "\nReading order ...\n";

		OKFDD_Use_Order = TRUE;

		// Open Order-File for input
		// -------------------------
		ifstream o(ORDER);

		if (!o)
                {
                        cout(2) << "OKFDD_Read_BLIF: Can`t open orderfile ";
                        cout(2) << "... (using default)\n";
                        OKFDD_Use_Order = FALSE; OKFDD_Error = Err_Orderfile;
                }
		else
		{
			// Clean entry tables
			// ------------------
			fu1(I0,0,pi_limit)
                        {
                                OKFDD_Result_2[I0]       = I0 + 1;
                                OKFDD_PI_Order_Table[I0] = ELSE;
                        }

			// Read header then ignore it
			// --------------------------
			do
                        {
                                o.getline(line,1000);
                        }
                        while((line[0]=='#') || (line[0]==' ') ||
                              (line[0]== 0 ) ||
                              (strncmp(line,".order",6) == 0));

			I0 = 0;

			while((!o.eof()) && (strncmp(line,".e",2) != 0))
			{
				// Read level
				// ----------
				position = 0; get_word(line,name_limit);

                                J0 = atoi(gl_name);

				if (OKFDD_PI_Order_Table[I0] != ELSE)
				{
					cout(2) << "OKFDD_Read_BLIF: Level ";
                                        cout(2) << "usage incorrect ... ";
                                        cout(2) << "(using default order)\n";
					OKFDD_Use_Order = FALSE;
                                        OKFDD_Error = Err_Orderfile;
                                        return;
				}

				// Read DTL-Type
				// -------------
				get_word(line,name_limit);

				switch (gl_name[0])
				{
					case 'S': J1 = D_Shan;  break;
					case 'P': J1 = D_posD;  break;
					case 'N': J1 = D_negD;  break;

					default :
                                        {
                                           cout(2) << "OKFDD_Read_BLIF: ";
                                           cout(2) << "Wrong DTL-Type in ";
                                           cout(2) << "line <" << J0 << " \t";
                                           cout(2) << gl_name[0] << "> ... ";
                                           cout(2) << "(using Shannon)\n";
                                           J1 = D_Shan;
                                        }
				}

				// Make entries according previous decisions
				// -----------------------------------------
				OKFDD_Result_2[I0]         = J0;
                                OKFDD_PI_Order_Table[I0++] = J1;

				// Read next line
				// --------------
				o.getline(line,1000);
			}

			// In case of incomplete order definition ->
                        // Set default decomposition types to Shannon
                        // (Levels are already set)
			// ------------------------------------------
			fu1(I1,I0,pi_limit) OKFDD_PI_Order_Table[I1] = 0;

			o.close();
		}
	}
	else if ((Label_Tab != NULL) && (DTL_Tab != NULL))
	{
		cout(1) << "\nCopying order ...\n";

		OKFDD_Use_Order = TRUE;

	        // Clean entry tables
	        // ------------------
		fu1(I0,0,pi_limit)
                {
                        OKFDD_Result_2[I0]       = I0 + 1;
                        OKFDD_PI_Order_Table[I0] = ELSE;
                }

		I0 = 0;

		while((J0 = Label_Tab[I0]) != 0)
		{
		        if (OKFDD_PI_Order_Table[I0] != ELSE)
			{
				cout(2) << "OKFDD_Read_BLIF: Level ";
                                cout(2) << "usage incorrect ... ";
                                cout(2) << "(using default order)\n";
				OKFDD_Use_Order = FALSE;
                                OKFDD_Error = Err_Orderfile;
                                return;
			}

			if ((J1 = DTL_Tab[I0]) > 2)
			{
			        cout(2) << "OKFDD_Read_BLIF: ";
                                cout(2) << "Wrong DTL-Type at ";
                                cout(2) << "position <" << J0 << " \t";
                                cout(2) << (int)(DTL_Tab[I0]) << "> ... ";
                                cout(2) << "(using Shannon)\n";
                                J1 = D_Shan;
                        }

			// Make entries according previous decisions
			// -----------------------------------------
		        OKFDD_Result_2[I0]         = J0;
                        OKFDD_PI_Order_Table[I0++] = J1;
		}

		// In case of incomplete order definition ->
                // Set default decomposition types to Shannon
                // (Levels are already set)
		// ------------------------------------------
		fu1(I1,I0,pi_limit) OKFDD_PI_Order_Table[I1] = 0;
	}


	// Open BLIF-File for input
	// ------------------------
	ifstream f(FILEX);

	if (!f)
        {
                 cout(2) << "OKFDD_Read_BLIF: Can`t open benchmark ... ";
                 cout(2) << "(shutdown)\n";
                 OKFDD_Error = Err_Benchmark;
                 return;
        }
	else
	{
		// Decide which Builder is used (2_Lvl- or M_Lvl_Format)
		// -----------------------------------------------------
		do { get_line(f,line1); } while(strncmp(line,".i",2) != 0);

		f.close();

		if (line[2] == 'n')  R_M_LVL(functex);
		else		     R_2_LVL();

    if ( input_labels )
    {
      *input_labels = gl_input_names;
    }

    if ( output_labels )
    {
      *output_labels = gl_output_names;
    }
	}

	// Refresh primary input order in OKFDD_PI_Order_Table
	// ---------------------------------------------------
	Support_complete();

	init();
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::R_M_LVL	       	 // Build OKFDD for given benchfile  //
/* ========================================================================= */
void dd_man::R_M_LVL(travex functex)
{
	uchar	   set;
	usint	   in_lab;
	usint	   in_number;
	usint	   lab;
	usint	   lc;
	usint	   ix;
	usint	   i;
	short	   min;
	short	   order[100];
	short	   minpos = 0;
        short	   last_x = 0;
	names*	   nhelp;
	son_ptr*   shelp;
	utnode*	   actual_path_p;
	utnode*	   last_actual;
	utnode*	   last_actual_x;
       	utnode*	   uthelp2;
	utnode*	   uthelp3 = OKFDD_ZERO;


	// Clear 2-lvl-description field
	// -----------------------------
	fu1(I0,0,line_max) field[I0] = NULL;

	// Open BLIF-File for input
	// ------------------------
	ifstream f(FILEX);

        if (!f)
        {
	      	cout(2) << "R_M_LVL: Can`t open benchmark ... (shutdown)\n";
	  	OKFDD_Error = Err_Benchmark;
	  	return;
	}

	do { get_line(f,line1); }
	while((line[0] == '#') || (line[0] == ' ') || (line[0] ==  0 ) ||
	      (line[0] ==  13) || (strncmp(line,".model",6) == 0));

	// Init hierarchy construction and make pseudo-node
	// (It`s job is the connection of all primary output roots)
	// --------------------------------------------------------
	gl_list = new hnode*[pi_limit]; fu1(I0,0,pi_limit) gl_list[I0] = NULL;
	gl_list[0] = new hnode(0,ELSE);


	// ----------------------------------------------------------
	// Read header ".inputs ..." and create primes for input list
	// ----------------------------------------------------------
	if (strncmp(line,".inputs",7) != 0)
	{
	  	cout(2) << "R_M_LVL: '.inputs' not standard ... (shutdown)\n";
	  	OKFDD_Error = Err_Benchmark;
	  	return;
	}

	maxi_i   = 0;

	do
	{
	
	position = 7; while(line[++position] == ' ') { }

	do
	{
		get_word(line,name_limit);

		Act_Prime[maxi_i] = lab = (Make_Prime(PI))->label_i;

		// Break if no more primaries are allowed
		// --------------------------------------
		if (OKFDD_Error != Err_No_Error) return;

		// If primary input doens`t exist -> Initialize pi inverse
		// and decomposition type for it
		// -------------------------------------------------------
		if (OKFDD_Use_Order == FALSE)
		{
		  if (OKFDD_PI_Level_Table[lab] == 0)
		  {
		    OKFDD_PI_Level_Table[lab] = 2 * lab;
		    OKFDD_PI_DTL_Table[lab]   = OKFDD_DTL_Default;
		  }
		}
		else
		{
		  if (OKFDD_PI_Level_Table[lab] == 0)
		  {
		    OKFDD_PI_Level_Table[lab] = 2 * OKFDD_Result_2[maxi_i];
		    OKFDD_PI_DTL_Table[lab]   = OKFDD_PI_Order_Table[maxi_i];
		  }
		  else
		  {
		    if (OKFDD_PI_Level_Table[lab] != 2*OKFDD_Result_2[maxi_i])
		    {
		      cout(2) << "R_M_LVL: Order conflict with existing label";
		      cout(2) << " ... (shutdown)\n";
                      OKFDD_Error = Err_Benchmark;
		      return;
		    }
		  }
		}

		// Create hierarchy node for actual label (if it doesn't exist)
		// ------------------------------------------------------------
		if (gl_list[lab] == NULL) gl_list[lab] = new hnode(lab,PI);

		maxi_i++;
	}
	while(last == FALSE);

	get_line(f,line1);

        }
        while(strncmp(line,".inputs",7) == 0);

        while((line[0] == 0) && (!f.eof())) { get_line(f,line1); }

	// ------------------------------------------------------------
	// Read header ".outputs ..." and create primes for output list
	// ------------------------------------------------------------
	if (strncmp(line,".outputs",8) != 0)
        {
          	cout(2) << "R_M_LVL: '.outputs' not standard ... (shutdown)\n";
          	OKFDD_Error = Err_Benchmark;
          	return;
        }

	maxo_i   = 0;

	do
	{

	position = 8; while(line[++position] == ' ') { }

	do
	{
		get_word(line,name_limit);

		// ----------------------------------------------------------
		// Check if PO is already defined as PI (This mystery is real
                // for example in C2670.blif) -> Chip thru_line
		// ----------------------------------------------------------
		if ((nhelp = find_prime()) == NULL)
		{
			Act_Prime[pi_limit-1-maxo_i] = lab =
                           (Make_Prime(PO))->label_i; maxo_i++;

			// Break if no more primaries are allowed
			// --------------------------------------
			if (OKFDD_Error != Err_No_Error) return;

			OKFDD_PI_DTL_Table[lab] = D_posD;
		}
		else
		{
			// ------------------------------------------
			// Thru line detected -> Make copy of primary
			// ------------------------------------------
			in_lab = nhelp->label_i;

			// No two primaries must have the same names
                        // -> Change ending of PO
			// -----------------------------------------
			fu1(J3,0,name_limit) if (gl_name[J3] == ' ') break;
                        gl_name[J3++] = '_'; gl_name[J3] = 'T';

			Act_Prime[pi_limit-1-maxo_i] = lab =
                           (Make_Prime(PO))->label_i; maxo_i++;

			// Break if no more primaries are allowed
			// --------------------------------------
			if (OKFDD_Error != Err_No_Error) return;

			OKFDD_PI_DTL_Table[lab] = D_posD;

			// Create hierarchy node for actual label
                        // (if it doesn't exist) then fill it
			// --------------------------------------
			if (gl_list[lab] == NULL)
                           gl_list[lab] = new hnode(lab,PO);

			gl_list[lab]->FANIN     = 1;
			gl_list[lab]->lc	= line_max - 3;
                        /* lc references emulated latch fct */
                        /* in lower part of routine         */

			shelp = gl_list[lab]->next = new son_ptr(in_lab);
                        gl_list[in_lab]->ref_c++;
		}
	}
	while(last == FALSE);

	get_line(f,line1);

        }
        while(strncmp(line,".outputs",8) == 0);


	cout(1) << "\n";

	maxt_i = 0;

	// ------------------------------------
	// Check for latches and integrate them
	// ------------------------------------
	if (strncmp(line,".wire",5) == 0)
	{
		cout(1) << "\nANALYSATION of latches is in progress ...\n";

		get_line(f,line1);

		while(strncmp(line,".latch",6) == 0)
		{
			// Input of latch will be of TF-type
			// ---------------------------------
			position = 0; get_word(line,name_limit);
                        get_word(line,name_limit);
                        /* Correct position and remove 1 get_word */

			T_Prime[maxt_i++] = in_lab = (Make_Prime(TF))->label_i;
           		/* This part forbids PIs as latch inputs  */

			// Break if no more primaries are allowed
			// --------------------------------------
			if (OKFDD_Error != Err_No_Error) return;

			OKFDD_PI_DTL_Table[in_lab] = D_posD;

			// Create hierarchy node for actual label
                        // (if it doesn't exist)
			// --------------------------------------
			if (gl_list[in_lab] == NULL)
                           gl_list[in_lab] = new hnode(in_lab,TF);


			get_word(line,name_limit);

			nhelp = find_prime();

			if (nhelp != NULL)
                        /* Why is this condition so different ?        */
                        /* Only slight changes to reduce code, right ? */
			{
				// Modified output of latch will be of PO-type
				// -------------------------------------------
				fu1(J3,0,name_limit)
                                if (nhelp->name[J3] == ' ') break;
                                nhelp->name[J3++] = '_'; nhelp->name[J3] = 'L';

				/* Where is the change of type to PO ? */
                                /* Is it always PO when found ?        */
                                /* And what about DTL-type ?           */

				lab = nhelp->label_i;

				// Create hierarchy node for actual label
                                // (if it doesn't exist) then fill it
				// --------------------------------------
				if (gl_list[lab] == NULL)
                                   gl_list[lab] = new hnode(lab,PO);

				gl_list[lab]->FANIN     = 1;
				gl_list[lab]->lc	= line_max - 3;

				shelp = gl_list[lab]->next =
                                   new son_ptr(in_lab);
                                gl_list[in_lab]->ref_c++;


				// Original output of latch will be of PI-type
				// -------------------------------------------
				Act_Prime[maxi_i++] = lab =
                                   (Make_Prime(PI))->label_i;

				// Break if no more primaries are allowed
				// --------------------------------------
				if (OKFDD_Error != Err_No_Error) return;

				if (OKFDD_Use_Order == FALSE)
				{
					if (OKFDD_PI_Level_Table[lab] == 0)
					{
					   OKFDD_PI_Level_Table[lab] =
                                              2 * lab;
                                           OKFDD_PI_DTL_Table[lab]   =
                                              OKFDD_DTL_Default;
					}
				}
				else
				{
					if (OKFDD_PI_Level_Table[lab] == 0)
					{
					    OKFDD_PI_Level_Table[lab] =
                                               2 * OKFDD_Result_2[maxi_i-1];
					    OKFDD_PI_DTL_Table[lab]   =
                                               OKFDD_PI_Order_Table[maxi_i-1];
					}
					else
					{
					    if (OKFDD_PI_Level_Table[lab] !=
                                               2 * OKFDD_Result_2[maxi_i-1])
					    {
					       cout(2) << "R_M_LVL: Order con";
                                               cout(2) << "flict with existin";
                                               cout(2) << "g label ... (shutd";
                                               cout(2) << "own)\n"; return;
					    }
					}
				}

				if (gl_list[lab] == NULL)
                                   gl_list[lab] = new hnode(lab,PI);
			}
			else
			{
				// Original output of latch will be of PI-type
				// -------------------------------------------
				Act_Prime[maxi_i++] = lab =
                                   (Make_Prime(PI))->label_i;

				if (OKFDD_Use_Order == FALSE)
				{
					if (OKFDD_PI_Level_Table[lab] == 0)
					{
					   OKFDD_PI_Level_Table[lab] =
                                              2 * lab;
                                           OKFDD_PI_DTL_Table[lab]   =
                                              OKFDD_DTL_Default;
					}
				}
				else
				{
					if (OKFDD_PI_Level_Table[lab] == 0)
					{
					   OKFDD_PI_Level_Table[lab] =
                                              2 * OKFDD_Result_2[maxi_i-1];
					   OKFDD_PI_DTL_Table[lab]   =
                                              OKFDD_PI_Order_Table[maxi_i-1];
					}
					else
					{
					   if (OKFDD_PI_Level_Table[lab] !=
                                              2 * OKFDD_Result_2[maxi_i-1])
					   {
					      cout(2) << "R_M_LVL: Order conf";
                                              cout(2) << "lict with existing ";
                                              cout(2) << "label ... (shutdown";
                                              cout(2) << ")\n"; return;
					   }
					}
				}

				// Create hierarchy node for actual label
                                // (if it doesn't exist)
				// --------------------------------------
				if (gl_list[lab] == NULL) gl_list[lab] =
                                   new hnode(lab,PI);


				// Modified output of latch will be of PO-type
				// -------------------------------------------
				fu1(J3,0,name_limit)
                                if (gl_name[J3] == ' ') break;
                                gl_name[J3++] = '_'; gl_name[J3] = 'L';

				Act_Prime[pi_limit-1-maxo_i] = lab =
                                   (Make_Prime(PO))->label_i; maxo_i++;

				// Break if no more primaries are allowed
				// --------------------------------------
				if (OKFDD_Error != Err_No_Error) return;

				OKFDD_PI_DTL_Table[lab] = D_posD;


				// Create hierarchy node for actual label
                                // (if it doesn't exist) then fill it
				// --------------------------------------
				if (gl_list[lab] == NULL) gl_list[lab] =
                                   new hnode(lab,PO);

				gl_list[lab]->FANIN = 1;
				gl_list[lab]->lc    = line_max - 3;

				shelp = gl_list[lab]->next =
                                   new son_ptr(in_lab);
                                gl_list[in_lab]->ref_c++;
			}

			get_line(f,line1);
		}
	}

	// Emulate latch function
	// ----------------------
//	field[line_max - 2] = new char[6]; field[line_max - 2] = "1 1\0";
//	field[line_max - 1] = new char[9]; field[line_max - 1] = ".names\0";
  strcpy( field[line_max - 2], "1 1" );
  strcpy( field[line_max - 1], ".names" );

	// ....................................................................


	cout(1) << "\nCONSTRUCTION of hierarchy in progress ...\n";

	lc = 0;

	uchar get_2_lvl;

	ulint lo1 = 0;

	// --------------------------------------------------------------------
	// Analyse Blif-File and create hierarchy nodes for all unknown
        // encountered primaries -> the result is a hierarchy tree
	// --------------------------------------------------------------------
	while(!f.eof())
	{
		get_2_lvl = TRUE;
		
		if (strncmp(line,".names",6) == 0)
		{			
			// ----------------------------------------------------
       			// Check for simple_a and simple_b notation:
                        // '.names <output> (CR 1)'
       			// ----------------------------------------------------
			position  = 6; while(line[++position] == ' ') { }
			in_number = 0;

			get_word(line,name_limit);
			
       			if (last == TRUE)
       			{
				if ((nhelp = find_prime()) == NULL)
				{
				   cout(2) << "R_M_LVL: Thru-Line not in ";
                                   cout(2) << ".outputs ... (shutdown)\n";
                                   return;
				}
				
       				ulint posi = 0;
					
	       			get_line(f,line1);
                                if (line[0] == '1')
                                   { posi++; get_line(f,line1); }
	       			
	       			toprime[(OKFDD_Result_2[lo1++] =
                                   nhelp->label_i)]->root =
                                   m_xor(OKFDD_ZERO,posi);
				
				cout << "\n>> Detected ";
				
				if (posi == 1) cout << "POSITIVE";
				else           cout << "NEGATIVE";
				
/**/ //				cout << " thru-line ...\n";

				cout << " thru-line ...(not supported ";
                                cout << "/ shutdown)\n";
				OKFDD_Error = Err_Benchmark;
				return;
				
				get_2_lvl = FALSE; 
	       		}
			else
			{				
				// Logic gate detected
                                // -> Analyse inputs and output
				// ----------------------------
				position  = 6; while(line[++position]==' ') { }
				in_number = 0;
			
			    	do
				{
					get_word(line,name_limit);

					// If input doesn`t exist
                                        // -> Make temporary entry
                                        //    Make node in hierarchy tree for
                                        //    input
					// ----------------------------------
					if ((nhelp = find_prime()) == NULL)
					{
					   nhelp = Make_Prime(TF);

					   // Break if no more primaries
                                           // are allowed
					   // --------------------------
					   if (OKFDD_Error!=Err_No_Error)
                                              return;

					   T_Prime[maxt_i++] = nhelp->label_i;
					   OKFDD_PI_DTL_Table[nhelp->label_i]=
                                              D_posD;
					}

					lab = nhelp->label_i;

					// Create hierarchy node for actual
                                        // label (if it doesn't exist)
					// --------------------------------
					if (gl_list[lab] == NULL) gl_list[lab]=
                                           new hnode(lab,nhelp->type);

					// If output of gate detected
                                        // -> integrate gate in hierarchy and
                                        //    fill it
					// ----------------------------------
					if (last == TRUE)
					{
					   gl_list[lab]->FANIN= in_number;
					   gl_list[lab]->lc   = lc;

					   // Link sons to hnode
					   // ------------------
					   shelp = gl_list[lab]->next = new
                                              son_ptr(OKFDD_PI_Order_Table[0]);
					   gl_list[OKFDD_PI_Order_Table[0]]->ref_c++;

					   if (in_number > 1)
                                           fu1(I0,1,in_number)
					   {
					      shelp = shelp->next = new
                                              son_ptr(OKFDD_PI_Order_Table[I0]);
					      gl_list[OKFDD_PI_Order_Table[I0]]->ref_c++;
					   }
					}
					else OKFDD_PI_Order_Table[in_number++]=
                                                lab;
				}
				while(last == FALSE);
/**/			}
		}

       		// Get 2-Lvl-Format line from BLIF-File
       		// ------------------------------------
		if (get_2_lvl == TRUE)
		{		
			do
			{
				get_line(f,line1); lc++;

				I0 = 0; do { } while(line[I0++] != 0);
                                field[lc] = new char[I0];
                                fu1(J0,0,I0) field[lc][J0] = line[J0];
			}
			while((strncmp(line,".names",6) != 0) && (!f.eof()));
		}
	}

	f.close();

/**/ //	fu1(I0,0,maxo_i) cout << Act_Prime[pi_limit-1-I0] << " "; cout << "\n";

	// ------------------------
	// Set thru-line POs at end
	// ------------------------
	I3 = 0;

	fu1(I0,0,maxo_i)
	{
		// Collect POs that aren't thru-lines
		// ----------------------------------
		I2 = 0;
                fu1(I1,0,lo1)
                if (OKFDD_Result_2[I1] == Act_Prime[pi_limit-1-I0]) I2++;

		if (I2 == 0)
                   Act_Prime[pi_limit-1-I3++] = Act_Prime[pi_limit-1-I0];
	}
	fu1(I1,0,lo1) Act_Prime[pi_limit-1-I3++] = OKFDD_Result_2[I1];
	
/**/ //	fu1(I0,0,maxo_i) cout << Act_Prime[pi_limit-1-I0] << " "; cout << "\n";
				
	maxo_i -= lo1;
									

	// Make links from pseudo_node to all primary output roots
	// -------------------------------------------------------
	shelp = gl_list[0]->next = new son_ptr(Act_Prime[pi_limit-1]);

	if (maxo_i > 1) fu1(I0,1,maxo_i)
	{
		// Detect POs that aren't thru-lines
		// ---------------------------------
/**/ //		I2 = 0;
/**/ //         fu1(I1,0,lo1)
/**/ //         if (OKFDD_Result_2[I1] == Act_Prime[pi_limit-1-I0]) I2++;

		shelp = shelp->next = new son_ptr(Act_Prime[pi_limit-1-I0]);
	}
					
/**/ //	gl_list[0]->FANIN = maxo_i - (lo1-1);

	// ....................................................................


	if (OKFDD_Subst_Method == TRUE)
	{
		OKFDD_Subst_Method = FALSE;
		
		cout << "R_M_LVL: Substitution method currently not ";
                cout << "permitted ... (using Synthesis)\n";
	}

	// ----------------------------------------------
	// Strategy SUBSTITUION or SYNTHESIS CONSTRUCTION
	// ----------------------------------------------
	if (OKFDD_Subst_Method == TRUE)
	{
		// -------------------
		// SUBSTITUTION METHOD
		// -------------------
		cout(1) << "\nANALYSATIONS of hierarchy in progress ...\n";

		// Traverse hierarchy tree to determine useful order
                // (inverse pi) / Key - Algo is ATO /
                // Breadth-First-Search - Mix
		// -------------------------------------------------
		cout(4) << "\n";

		// Interleave (if allowed)
		// -----------------------
		if (OKFDD_Interleaving == TRUE) Interleave(gl_list[0]);

		// Traverse hierarchy tree to determine order of substitutions
		// -----------------------------------------------------------
		first_minus = -1; I2 = maxt_i + maxo_i - 1; I3 = I2 + 1;
                DFS(gl_list[0]);

		// ............................................................


		Support_complete();

		init();

/*	        fu1(I0,0,maxo_i)
                {
                cout << Act_Prime[pi_limit-1-I0] << "[";
                cout << OKFDD_PI_Level_Table[Act_Prime[pi_limit-1-I0]] << "] ";
                }
	        cout << "\n";
	        fu1(I0,0,maxt_i)
                {
                cout << T_Prime[I0] << "[";
                cout << OKFDD_PI_Level_Table[T_Prime[I0] << "] ";
                }
	        cout << "\n";
	        fu1(I0,0,maxi_i)
                {
                cout << Act_Prime[I0] << "[";
                cout << OKFDD_PI_Level_Table[Act_Prime[I0]] << "] ";
                }
	        cout << "\n";
*/
		cout(8) << "\nOKFDD_PI_Order_Table:\n---------\n"; I0 = 0;

		do
		{
		   cout(8) << OKFDD_PI_Order_Table[I0] << "[";
                   cout(8) << OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[I0]];
                   cout(8) << "] ";
		}
		while(OKFDD_PI_Order_Table[I0++] != 0); cout(8) << "\n";

		// ............................................................


		// Creation starts -> Hit the clock
		// --------------------------------
		timer.start();

		// Initialize primary outputs for substitution
		// -------------------------------------------
		usint loop;

		// Init all primary outputs with base node
		// ---------------------------------------
		fu1(loop,0,maxo_i)
		{
		   toprime[Act_Prime[pi_limit-1-loop]]->root =
                      FOAUT(Act_Prime[pi_limit-1-loop],OKFDD_ONE,OKFDD_ZERO);
		   ups(toprime[Act_Prime[pi_limit-1-loop]]->root);
		}


		last_actual_x = OKFDD_ZERO;

		cout(1) << "\nSUBSTITUTION of hierarchy in progress ...\n";

		do
		{
			lab = 1;

			// What substitution is next one ? -> Label in lab
			// -----------------------------------------------
			fu1(K2,0,maxo_i)
                        if (OKFDD_PI_Level_Table[K0 =
                        (m_and(toprime[Act_Prime[pi_limit-1-K2]]->root))->
                        label_i] < OKFDD_PI_Level_Table[lab])
                           { K3 = K2; lab = K0; }


			// All temporary labels substituted ?
                        // -> Break substition loop
			// ----------------------------------
			if (OKFDD_PI_Level_Table[lab] > 0) break;

			// Show some infos
			// ---------------
			if (OKFDD_Outputflags & 16)
			{
			   ulint OKFDD_Total_nodes = 0;
                           fu1(I0,1,pi_limit)
                             OKFDD_Total_nodes += OKFDD_No_UTNodes_Per_Lvl[I0];
			   cout << "Sub of <";
                           fu1(i,0,name_limit) cout << toprime[lab]->name[i];
			   cout << "> " << lab << " \t";
                           cout << OKFDD_PI_Level_Table[lab];
			   cout << " \t\t\tNodes in manager: ";
                           cout << OKFDD_Total_nodes << "\n";
			}
			else if (OKFDD_Outputflags & 32)
			{
			   cout << "Sub of <";
                           fu1(i,0,name_limit) cout << toprime[lab]->name[i];
			   cout << "> " << lab << " \t";
                           cout << OKFDD_PI_Level_Table[lab] << "\n";
			}


			// Extract all inputs of gate
                        // and store them in OKFDD_Result
			// ------------------------------
			hhelp = gl_list[lab]->next; I0 = 0;
                        do { OKFDD_Result[I0++] = hhelp->son_label; }
                        while((hhelp = hhelp->next) != NULL);

			// ----------------------------------------------------
			// Sort input list of actual gate in descending
                        // pi_inv-order -> Needed to create Micro-DD
			// ----------------------------------------------------
			fu1(K1,0,gl_list[lab]->FANIN)
			{
			   min = pi_limit;

			   fu1(K0,0,gl_list[lab]->FANIN)
                           if (OKFDD_PI_Level_Table[OKFDD_Result[K0]] < min)
			   {
			      if (K1 == 0)
                              {
                                 min = OKFDD_PI_Level_Table[OKFDD_Result[K0]];
                                 minpos = K0;
                              }
			      else
			      {
				 if (last_x <
                                 OKFDD_PI_Level_Table[OKFDD_Result[K0]])
				 {
				    min=OKFDD_PI_Level_Table[OKFDD_Result[K0]];
                                    minpos = K0;
				 }
			      }
			   }

			   order[K1] = minpos;
			   last_x    = min;
			}

			lc	      = gl_list[lab]->lc + 1;
			set	      = NONE;
			last_actual_x = OKFDD_ZERO;

			do
			{
				// Show actual line of BLIF (if allowed)
				// -------------------------------------
				if (OKFDD_Outputflags & 16)
				{
					cout << "Reading line: "; i = 0;
                                        while(field[lc][i]!=0)
                                           cout << field[lc][i++];
                                        cout << "\n";
				}

				// Check for 1, 0 or - output
                                // -> First 0 or 1 determines table as ON-Set
                                //    or OFF-Set declaration
				// ------------------------------------------
				position = 0; get_word(field[lc],line_max);

				if (((field[lc][position]=='1') && (set != OFF)
                                   ) ||
                                   ((field[lc][position]=='0') && (set != ON)))
				{
					if (field[lc][position]=='1') set = ON;
                                        else if (field[lc][position]=='0')
                                           set = OFF;

					actual_path_p = OKFDD_ONE;

					// -----------------------------
					// Construct Path of 2-Lvl-Table
					// -----------------------------
					fdi(j,(gl_list[lab]->FANIN)-1,0)
					{
					   short k = OKFDD_Result[order[j]];

					   if (field[lc][order[j]]=='0')
                                           switch (OKFDD_PI_DTL_Table[k])
					   {
					      // -----------------------
					      // Path type: a1 .... an 0
					      // -----------------------

					      // ai is primary input -> get
                                              // base node of it / Else -> get
                                              // root of temporary fct
					      // -----------------------------
					      case D_Shan:    actual_path_p = FOAUT(k,OKFDD_ZERO,actual_path_p);      break;
					      case D_posD:    actual_path_p = FOAUT(k,actual_path_p,actual_path_p);   break;
					      case D_negD:    actual_path_p = FOAUT(k,actual_path_p,OKFDD_ZERO);
					   }
					   else if (field[lc][order[j]]=='1')
                                           switch (OKFDD_PI_DTL_Table[k])
					   {
					      // -----------------------
					      // Path type: a1 .... an 1
					      // -----------------------

					      // ai is primary input -> get
                                              // base node of it / Else -> get
                                              // root of temporary fct
					      // -----------------------------
					      case D_Shan:    actual_path_p = FOAUT(k,actual_path_p,OKFDD_ZERO);      break;
					      case D_posD:    actual_path_p = FOAUT(k,actual_path_p,OKFDD_ZERO);      break;
					      case D_negD:    actual_path_p = FOAUT(k,actual_path_p,actual_path_p);
					   }
					}
				}
				else
                                {
                                        cout(2) << "R_M_LVL: Wrong set ...\n";
                                        return;
                                }

				ups(actual_path_p);

				utnode* uth1;

				// OR full path with last path / free old one
				// ------------------------------------------
				last_actual_x = CO_OR(actual_path_p,uth1 =
                                   last_actual_x);
                                ups(last_actual_x);
				OKFDD_Free_Node(actual_path_p);
                                OKFDD_Free_Node(uth1);
			}
			while((field[++lc][0] != '.') && (field[lc][0] != 0));

			// If table wasn`t ON-Set -> Negate function
			// -----------------------------------------
			if (set == OFF) last_actual_x = m_xor(last_actual_x,1);

			usint loopy;

			// ----------------------------------------------------
			// All primary outputs with actual label lab need
                        // subsitution
			// ----------------------------------------------------
			fu1(loopy,0,maxo_i)
                        if ((m_and(toprime[Act_Prime[pi_limit-1-loopy]]->
                        root))->label_i == lab)
			{
				// Building of Micro-OKFDD finished
                                // -> Now substitute top label
				// --------------------------------
				uthelp2 = toprime[Act_Prime[pi_limit-1-loopy]]
                                ->root;

				// Calculate Substitution / Top label is of
                                // pos. Davio type:
                                // last_actual = ~(~g + ~F2) # F0 = F0 # g*F2
				// ------------------------------------------
				uthelp3 = CO_OR(m_xor(last_actual_x,1),
                                   m_xor((m_and(uthelp2))->hi_p,1));
                                ups(uthelp3);
				uthelp3 = m_xor(uthelp3,1);

				last_actual = CO_XOR( m_xor((m_and(uthelp2))->
                                   lo_p,m_sel(uthelp2) ),uthelp3);
                                ups(last_actual);

				OKFDD_Free_Node(uthelp3);
				OKFDD_Free_Node(uthelp2);

				toprime[Act_Prime[pi_limit-1-loopy]]->root =
                                   last_actual;
			}

			// Free last Micro_OKFDD
			// ---------------------
			OKFDD_Free_Node(last_actual_x);

		}
		while(TRUE);

		// Creation ends -> Hit the clock
		// ------------------------------
		timer_stop = timer.stop();

		// FREE temporary labels
		// ---------------------
		ix = T_F; fu1(loop,0,ix) OKFDD_Remove_Prime(T_Prime[loop]);
	}
	else
	{
		// ----------------------
		// SYNTHESIS CONSTRUCTION
		// ----------------------

		cout(1) << "\nANALYSATIONS of hierarchy in progress ...\n";

		// Traverse hierarchy tree to determine useful order
                // (inverse pi) / Key - Algo is Depth First Search
		// -------------------------------------------------
		cout(4) << "\n";

		// Interleave (if allowed)
		// -----------------------
		if (OKFDD_Interleaving == TRUE) Interleave(gl_list[0]);

		// Traverse hierarchy tree DFS-like to determine order of
                // synthesis operations
		// ------------------------------------------------------
		I2 = 0; first_minus = -1; DFS(gl_list[0]);

		OKFDD_Max_Gate_Number = I2;

		// ............................................................


		cout(1) << "\nSYNTHESIS OP.s of DDs now in progress ...\n";

		// Fill OKFDD_PI_Order_Table with all PIs
                // (OKFDD_PI_Order_Table holds info for sifting proc.s) then
                // initialize sift fields
		// ---------------------------------------------------------
		Support_complete();

		init(); OKFDD_Total_nodes = OKFDD_Now_size_i;


/*              fu1(I0,0,maxo_i)
                {
                cout << Act_Prime[pi_limit-1-I0] << "[";
                cout << OKFDD_PI_Level_Table[Act_Prime[pi_limit-1-I0]] << "] ";
                }
                cout << "\n";
                fu1(I0,0,maxt_i)
                {
                cout << T_Prime[I0] << "[";
                cout << OKFDD_PI_Level_Table[T_Prime[I0]] << "] ";
                }
                cout << "\n";
                fu1(I0,0,maxi_i)
                {
                cout << Act_Prime[I0] << "[";
                cout << OKFDD_PI_Level_Table[Act_Prime[I0]] << "] ";
                }
                cout << "\n";
*/
		cout(8) << "\nOKFDD_PI_Order_Table:\n---------\n"; I0 = 0;

		do
		{
		   cout(8) << OKFDD_PI_Order_Table[I0] << "[";
                   cout(8) << OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[I0]];
                   cout(8) << "] ";
		}
		while(OKFDD_PI_Order_Table[I0++] != 0); cout(8) << "\n";

		cout(48) << "\n";

		// Creation starts -> Hit the clock
		// --------------------------------
		timer.start();

		fu1(OKFDD_Act_Gate_Number,0,OKFDD_Max_Gate_Number)
		{
Measureact = OKFDD_No_UTNodes;
			// Get actual gate label and startline of 2-lvl-table
			// --------------------------------------------------
			lab = OKFDD_Result_2[OKFDD_Act_Gate_Number];
                        lc = gl_list[lab]->lc + 1;

			// Extract all inputs of gate and store them in
                        // OKFDD_Result
			// --------------------------------------------
			hhelp = gl_list[lab]->next; I0 = 0;
                        do { OKFDD_Result[I0++] = hhelp->son_label; }
                        while((hhelp = hhelp->next) != NULL);

			set	    = NONE;
			last_actual = OKFDD_ZERO;

			do
			{
				// Show lines of BLIF (if allowed)
				// -------------------------------
				if (OKFDD_Outputflags & 16)
				{
				  cout << "Reading line: ";
                                        
                                  int i = 0;
                                  while(field[lc][i]!=0) cout <<field[lc][i++];
                                  nextline;
				}

				// ------------------------------------------
				// Check for 1, 0 or - output
                                // -> First 0 or 1 determines table as ON-Set
                                // or OFF-Set declared
				// ------------------------------------------
				position = 0; get_word(field[lc],line_max);

                                actual_path_p = OKFDD_ONE;

				if (((field[lc][position]=='1')&&(set != OFF))
                                   ||
                                   ((field[lc][position]=='0')&&(set != ON)))
				{
				   if      (field[lc][position]=='1') set =ON;
                                   else if (field[lc][position]=='0') set =OFF;

				   // -----------------------------
				   // Construct Path of 2-Lvl-Table
				   // -----------------------------
				   fdi(j,(gl_list[lab]->FANIN)-1,0)
				   {
				      short k = OKFDD_Result[j];

				      if (field[lc][j]=='1')
				      {
					 // -----------------------
					 // Path type: a1 .... an 1
					 // -----------------------

					 // ai is primary input -> get base
                                         // node of it / Else -> get root of
                                         // temporary fct
					 // --------------------------------
					 if (toprime[k]->type == PI)
					 {
					    switch (OKFDD_PI_DTL_Table[k])
					    {
					       case D_Shan:    uthelp3 = FOAUT(k,OKFDD_ONE,OKFDD_ZERO); break;
					       case D_posD:    uthelp3 = FOAUT(k,OKFDD_ONE,OKFDD_ZERO); break;
					       case D_negD:    uthelp3 = FOAUT(k,OKFDD_ONE,OKFDD_ONE);
					    }
					 }
					 else  uthelp3 = toprime[k]->root;

					 ups(uthelp3);

					 // AND path with last result
					 // -------------------------
					 uthelp2       =actual_path_p;

					 actual_path_p =CO_OR(m_xor(uthelp3,1),
                                            m_xor(uthelp2,1));

					 actual_path_p =m_xor(actual_path_p,1);

					 // Free last result
					 // ----------------
					 ups(actual_path_p);
					 OKFDD_Free_Node(uthelp2);
                                         OKFDD_Free_Node(uthelp3);
				      }
				      else if (field[lc][j]=='0')
				      {
					 // -----------------------
					 // Path type: a1 .... an 0
					 // -----------------------

					 // ai is primary input -> get base
                                         // node of it / Else -> get root of
                                         // temporary fct
					 // --------------------------------
					 if (toprime[k]->type == PI)
					 {
					    switch (OKFDD_PI_DTL_Table[k])
					    {
					       case D_Shan:    uthelp3 = FOAUT(k,OKFDD_ONE,OKFDD_ZERO); break;
					       case D_posD:    uthelp3 = FOAUT(k,OKFDD_ONE,OKFDD_ZERO); break;
					       case D_negD:    uthelp3 = FOAUT(k,OKFDD_ONE,OKFDD_ONE);
					    }
					 }
					 else  uthelp3 = toprime[k]->root;

					 ups(uthelp3);


					 // AND path with last result
					 // -------------------------
					 uthelp2       =actual_path_p;

					 actual_path_p =CO_OR(uthelp3,
                                            m_xor(uthelp2,1));

					 actual_path_p =m_xor(actual_path_p,1);

					 // Free last result
					 // ----------------
					 ups(actual_path_p);
					 OKFDD_Free_Node(uthelp2);
                                         OKFDD_Free_Node(uthelp3);
				      }
				   }
				}
				else
                                {
                                   if (field[lc][position]=='-')
                                      actual_path_p = OKFDD_ZERO;
                                   else
                                   {
                                      cout(2) << "R_M_LVL: Set {1,0} not ";
                                      cout(2) << "standard ...\n";
                                      return;
                                   }
                                }

				utnode* uth1;

				// OR full path with last path
                                // then free old one
				// ---------------------------

				last_actual = CO_OR(actual_path_p,uth1 =
                                   last_actual);

                                ups(last_actual);
				OKFDD_Free_Node(actual_path_p);
                                OKFDD_Free_Node(uth1);
			}
			while((field[++lc][0] != '.') && (field[lc][0] != 0));


			// If table wasn`t ON-Set -> Negate function
			// -----------------------------------------
			if (set == OFF) last_actual = m_xor(last_actual,1);

			// Store root of the new Micro-OKFDD
			// ---------------------------------
			toprime[lab]->root = last_actual;

if ((OKFDD_No_UTNodes > Measureact) && (OKFDD_No_UTNodes - Measureact > Measuremax))
  Measuremax = OKFDD_No_UTNodes - Measureact;
			
			// Only perform action if in sift mode
			// -----------------------------------
			if (OKFDD_Temproutine != 0)
			{
			   // Free rc_cache -> Reduce number of nodes
                           // for reordering procedures
			   // ---------------------------------------
			   OKFDD_Free_Fct_Cache();
			}


			// Count actual size of OKFDDs
			// ---------------------------
			OKFDD_Now_size_i = 0;
                        fu1(I0,0,OKFDD_Maxidx) OKFDD_Now_size_i +=
                           OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[I0]];

			// --------------------
			// Show some statistics
			// --------------------
			if (OKFDD_Outputflags & 16)
			{
			   cout << "Label " << lab << " \twith FANIN ";
                           cout << gl_list[lab]->FANIN << "  \t";
			   usint l;
                           fu1(l,0,gl_list[lab]->FANIN)
                              cout << OKFDD_Result[l] << " "; cout << "\n";
			}
			else if (OKFDD_Outputflags & 32)
			{
			   J1 = (short)((float)(OKFDD_Dots_Out_Limit*
                                OKFDD_Act_Gate_Number)/
                                (OKFDD_Max_Gate_Number-1));
                           J2 = OKFDD_Dots_Out_Limit - J1;

			   cout << "\r<"; fu1(J0,0,J1) cout << "m";
                           fu1(J0,0,J2) cout << "~";
                           cout << "> \tGate " << OKFDD_Act_Gate_Number;
                           cout << "   \t";
			   cout << "Size " << OKFDD_Now_size_i << "   \t";
                           cout.flush();
			}

			// --------------------
			// Perform extra action
			// --------------------
			if (functex != NULL) (*functex)(this,NULL);
		}

		// Construction finished -> Hit the clock
		// --------------------------------------
		timer_stop = timer.stop();

		cout(48) << "\n";

		// Free all help fields
		// --------------------
		fu1(I0,0,pi_limit)
                if (gl_list[I0] != NULL) delete gl_list[I0]; delete[] gl_list;
/*              {
                   son_ptr *p = gl_list[I0]->next, *q;
                   while (p != NULL)
                   {
                       q = p;
                       p = p->next;
                       delete q;
                   }
                   delete gl_list[I0];
                }
                delete[] gl_list;
*/
		// Free all temporary functions
		// ----------------------------
		if (maxt_i > 0)
                fu1(I1,0,maxt_i) OKFDD_Free_Node(toprime[T_Prime[I1]]->root);
                maxt_i = T_F = 0;
	}

	fu1(I0,0,line_max-2)
	{
/**/ //	   cout << I0 << "[" << &field[I0][0] << "]\n";
	   if (field[I0] != NULL) delete[] field[I0];
	}

cout << "\nMaximal expansion during a gate: " << Measuremax; nextline; nextline;

	// Free all dirty trees
	// --------------------
	OKFDD_Free_Fct_Cache();
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::R_2_LVL // Build OKFDD out of benchfile	       	     //
/* ========================================================================= */
void dd_man::R_2_LVL()
{
	names*  nhelp;
	usint   lab;

	// Open BLIF-File for input
	// ------------------------
	ifstream f(FILEX);
        
        if (!f)
        {
           cout(2) << "R_2_LVL: Can`t open benchmark ... (shutdown)\n";
           return;
        }

	// Read header ".i maxi_i" and ".o maxo_i"
        // No match -> No 2-Lvl-Format
	// ---------------------------------------
	do { get_line(f,line1); } while(strncmp(line,".i",2) != 0);
        
        maxi_i = atoi(&(line[3]));
	
	do { get_line(f,line1); } while(strncmp(line,".o",2) != 0);
        
        maxo_i = atoi(&(line[3]));

  // msoeken: read input and output names
  do { get_line( f, line1 ); } while ( strncmp( line, ".ilb", 4 ) != 0 );
  gl_input_names = std::string( &line[5] );

  do { get_line( f, line1 ); } while ( strncmp( line, ".ob", 3 ) != 0 );
  gl_output_names = std::string( &line[4] );

	do { get_line(f,line1); }
        while((line[0] != '0') && (line[0] != '1') && (line[0] != '-'));

	// Calculate new string names for primary inputs and make entries
	// --------------------------------------------------------------
	fu1(I0,0,maxi_i)
	{
		get_name(PI,OKFDD_Next_Label);

		nhelp		   = Make_Prime(PI);
		Act_Prime[I0] = lab     = nhelp->label_i;

		// If primary input doens`t exist
                // -> Initialize pi inverse and decomposition type for it
		// ------------------------------------------------------
		if (OKFDD_Use_Order == FALSE)
		{
			if (OKFDD_PI_Level_Table[lab] == 0)
			{
			   OKFDD_PI_Level_Table[lab]= 2 * lab;
                           OKFDD_PI_DTL_Table[lab]  = OKFDD_DTL_Default;
			}
		}
		else
		{
			if (OKFDD_PI_Level_Table[lab] == 0)
			{
			   OKFDD_PI_Level_Table[lab]= 2 * OKFDD_Result_2[I0];
			   OKFDD_PI_DTL_Table[lab]  = OKFDD_PI_Order_Table[I0];
			}
			else if (OKFDD_PI_Level_Table[lab] !=
                        2 * OKFDD_Result_2[I0])
			{
			   cout(2) << "R_2_LVL: Order conflict ... (using ";
                           cout(2) << "existing values)\n";
			}
		}
	}

	// Fill up (in this routine useless) T_Prime
        // -> Needed during contruction
	// -----------------------------------------
	fu1(I0,0,maxi_i) T_Prime[I0] = I0;


	// Bubble-Sort primary inputs according ascending pi inverse
	// ---------------------------------------------------------
	if (maxi_i > 1)
	{
		fdi(j,maxi_i-1,1) fui(i,0,j-1)

		if (OKFDD_PI_Level_Table[Act_Prime[i]] >
                OKFDD_PI_Level_Table[Act_Prime[i+1]])
		{
		   J2 = Act_Prime[i]; Act_Prime[i] = Act_Prime[i+1];
                   Act_Prime[i+1] = J2;
		   J2 = T_Prime[i];   T_Prime[i]   = T_Prime[i+1];
                   T_Prime[i+1]   = J2;
		}
	}

	cout(1) << "\n";

	// Calculate new string names for primary outputs and make entries
	// ---------------------------------------------------------------
	fu1(I0,0,maxo_i)
        {
           get_name(PO,OKFDD_Next_Label); nhelp = Make_Prime(PO);
           Act_Prime[pi_limit-1-I0] = nhelp->label_i;
        }

        breakline1;

	maxt_i  = 0;

	short     instart  = 0;
	short     outstart = instart + maxi_i +1;

	utnode*   actual_path_p;
	utnode*   last_actual;

	utnode*** actual_fct_p;
	uchar**   bucket;
	uchar**   bucketlevel;
	usint*    act_bucket;

	// Allocate and clear buckets
	// --------------------------
	actual_fct_p = new utnode**[bucket_max];
	bucket	     = new   uchar*[bucket_max];
	bucketlevel  = new   uchar*[bucket_max];

	fu1(I0,0,bucket_max)
	{
	   actual_fct_p[I0] = new utnode*[maxo_i];
	   bucket[I0]       = new   uchar[maxo_i];
	   bucketlevel[I0]  = new   uchar[maxo_i];
	   act_bucket       = new   usint[maxo_i];
	}

	fu1(I0,0,maxo_i) fu1(I1,0,bucket_max)
        {
           actual_fct_p[I1][I0] = OKFDD_ZERO;
           bucket[I1][I0] = bucketlevel[I1][I0] = 0;
        }

	fu1(I0,0,maxo_i) act_bucket[I0] = 0;


	I1 = 0; I2 = 50;

	// Creation starts -> Hit the clock
	// --------------------------------
	timer.start();

	while(!f.eof())
	{
                if ((strncmp(line,".e",2) == 0) || (f.eof()))
                { cout(1) << "\n"; break; }

		if (OKFDD_Outputflags & 16)
                {
                   cout << "Reading line: ";
                   fui(i,0,maxi_i+maxo_i) cout << line[i]; nextline;
                }
		else if (OKFDD_Outputflags & 32)
                {
                   if (I2 == 50)
                   { cout << "\nLine: " << I1*50 << " \t "; I1++; I2 = 0; }
                   cout << "."; I2++;
                }

		fu1(K0,0,maxo_i)
		{
			if (line[outstart+K0] == '1')
			{
				actual_path_p = OKFDD_ONE;

				// Construct path (one line of BLIF)
				// ---------------------------------
				fdi(j,maxi_i-1,0)
				{
					usint k = Act_Prime[j];

					if (line[T_Prime[j]] == '0')
					{
					   switch (OKFDD_PI_DTL_Table[k])
					   {
					      case D_Shan:    actual_path_p = FOAUT(k,OKFDD_ZERO,actual_path_p);    break;
					      case D_posD:    actual_path_p = FOAUT(k,actual_path_p,actual_path_p); break;
					      case D_negD:    actual_path_p = FOAUT(k,actual_path_p,OKFDD_ZERO);
					   }
					}
					else if (line[T_Prime[j]] == '1')
					{
					   switch (OKFDD_PI_DTL_Table[k])
					   {

					      case D_Shan:    actual_path_p = FOAUT(k,actual_path_p,OKFDD_ZERO);    break;
					      case D_posD:    actual_path_p = FOAUT(k,actual_path_p,OKFDD_ZERO);    break;
					      case D_negD:    actual_path_p = FOAUT(k,actual_path_p,actual_path_p);
					   }
					}
				}

				ups(actual_path_p);


				fui(l,0,maxo_i-1) if (line[outstart+l] == '1')
				{
				   bucket[act_bucket[l]][l]++;

/*				   cout << "Path to ";
                                   cout << bucket[act_bucket[l]][l];
                                   cout << "[" << l << "]\n";
*/
				   last_actual = actual_fct_p[bucket[
                                      act_bucket[l]][l]][l];
				   actual_fct_p[bucket[act_bucket[l]]
                                      [l]][l] = actual_path_p;

				   // Increase ref. counter to avoid
                                   // instant removal when caching
                                   // actual_fct_p[.][l]
				   // ------------------------------
				   ups(actual_fct_p[bucket[
                                      act_bucket[l]][l]][l]);

				   // Bucket overflow ? -> Increment next
                                   // bucket counter, OR actual bucket
                                   // contents,
				   // clean bucket then inspect next
				   // -----------------------------------
				   if (++bucketlevel[act_bucket[l]][l] ==
                                   OKFDD_Blocksize)
				   {
				      do
				      {
					 fdi(k,bucket[act_bucket[l]][l],
                                         bucket[act_bucket[l]][l]-
                                         OKFDD_Blocksize+2)
					 {
/*					    cout << k << "[" << l << "] to ";
					    cout << bucket[act_bucket[l]][l]-
                                            OKFDD_Blocksize+1 << "[" << l;
                                            cout << "]\n";
*/
					    actual_fct_p[bucket[act_bucket[l]]
                                            [l]-OKFDD_Blocksize+1][l] =
					    CO_OR(actual_fct_p[k][l],
					    last_actual = actual_fct_p[bucket[
                                            act_bucket[l]][l]-
                                            OKFDD_Blocksize+1][l]);

					    // Inc ref counter to avoid
                                            // instant removal when caching
                                            // actual_fct_p[.][l]
					    // ----------------------------
					    ups(actual_fct_p[bucket[act_bucket[
                                            l]][l]-OKFDD_Blocksize+1][l]);

					    // Now it's time to cache
                                            // last_actual & actual_fct_p[k][l]
					    // --------------------------------
					    OKFDD_Free_Node(actual_fct_p[k]
                                               [l]);
					    OKFDD_Free_Node(last_actual);
					 }

					 bucket[++act_bucket[l]][l]++;
				      }
				      while(++bucketlevel[act_bucket[l]][l]
                                      == OKFDD_Blocksize);

				      // Refresh buckets
				      // ---------------
				      fdi(k,act_bucket[l]-1,0)
				      {
					 bucket[k][l] = 
                                         bucket[act_bucket[l]][l];
					 bucketlevel[k][l] = 0;
				      }

				      // Clear actual bucket
				      // -------------------
				      act_bucket[l] = 0;
				   }
				}

				// Now it's time to cache actual_path_p
				// ------------------------------------
				OKFDD_Free_Node(actual_path_p);

				break;
			}
		}
                get_line(f,line1);
	}

	// DD construction before completion. Now it`s time to collect all
        // index entries up to the last value
	// ---------------------------------------------------------------
	fui(l,0,maxo_i-1)
	{
		fdi(k,bucket[act_bucket[l]][l],2)
		{
/**/ //		   cout << k << "[" << l << "] to " << 1 << "[" << l << "]\n";

		   actual_fct_p[1][l] = CO_OR(actual_fct_p[k][l],
                   last_actual = actual_fct_p[1][l]);

		   ups(actual_fct_p[1][l]);

		   // Now it's time to cache last_actual and actual_fct_p[k][l]
		   // ---------------------------------------------------------
		   OKFDD_Free_Node(actual_fct_p[k][l]);
		   OKFDD_Free_Node(last_actual);
		}
	}

	// Creation ends -> Hit the clock
	// ------------------------------
	timer_stop = timer.stop();

	// Close BLIF
	// ----------
	f.close();

	// Free all dirty trees
	// --------------------
	OKFDD_Free_Fct_Cache();

	// Store results (roots of constructed DDs) in POs
	// -----------------------------------------------
	cout(1) << "\n\n";

	fu1(I0,0,maxo_i)
	{
		nhelp	    = toprime[Act_Prime[pi_limit-1-I0]];
		nhelp->root = actual_fct_p[1][I0];

		if (OKFDD_Outputflags & 1)
		{
		   cout << "Root destination <";
		   fu1(I1,0,16) cout(1) << nhelp->name[I1]; cout << "...>";
                   nextline;
		}
	}

	cout(1) << "\n";
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::FOAUT // returns pointer to utnode with (v,high,low) //
/* ========================================================================= */
utnode* dd_man::FOAUT(  usint   v,
			utnode* hi,	     // Complemented pointer
			utnode* lo)	     // Complemented pointer
{
	// ------------------------------------------------
	// Explosion / Implosion  needed for unique table ?
	// ------------------------------------------------
	if (OKFDD_No_UTNodes_Per_Lvl[v] * 10 >
	    ut_sizes[ut_hashsize[v]] * OKFDD_Explodefactor)
	{
		// Enlarge hashtable ?
		// -------------------
		if (ut_hashsize[v] < ut_max)
		{
			OKFDD_New_Table_Size = ut_hashsize[v]+1;
			OKFDD_Resize_ut(v);
		}
	}
	else
	{
		if (OKFDD_No_UTNodes_Per_Lvl[v] * 10 <
		    ut_sizes[ut_hashsize[v]] * OKFDD_Implodefactor)
		{
			// Reduce size of hashtable ?
			// --------------------------
			if (ut_hashsize[v] > 0)
			{
				OKFDD_New_Table_Size = ut_hashsize[v]-1;
				OKFDD_Resize_ut(v);
			}
		}
	}

	// Normalization -> Switch complement marks if needed
	// --------------------------------------------------
	if ( m_sel(lo) == 1)
	{
		lo = m_xor(lo,1); flag =1;
		
		if (OKFDD_PI_DTL_Table[v] == D_Shan) hi = m_xor(hi,1);
	}
	else 	flag = 0;

	// Extract pure pointer
	// --------------------
	Pure_hi = m_and(hi); Pure_lo = m_and(lo);

	// Calculate hashposition in table
	// -------------------------------
	hashnr = (Pure_hi->idnum + Pure_lo->idnum) % ut_sizes[ut_hashsize[v]];

	
	// -------------------------------------------------------
	// Check if chain already exists / No -> create first node
	// Yes -> inspect chain and add new node if needed
	// -------------------------------------------------------
	if (ut[v][hashnr] != NULL)
	{
		uthelp = ut[v][hashnr];

		do
		{
			// Check for desired values / match -> return node
			// No match -> inspect next chain element
			// -----------------------------------------------
			if ((m_and(uthelp->hi_p))->idnum == Pure_hi->idnum)
			if (uthelp->lo_p->idnum	  	 == Pure_lo->idnum)
			if (uthelp->hi_p		 == hi	    )
			if (uthelp->lo_p		 == lo	    )
			return m_xor(uthelp,flag);

			utlast = uthelp;
		}
		while((uthelp = utlast->next) != NULL);

		// Increase references of sons and create node
		// -------------------------------------------
		up(Pure_hi); up(Pure_lo);
		gnuz(utlast->next,v,hi,lo,counter++,1);

		// Check memory limits every 10. allocation due to performance
		// -----------------------------------------------------------
		CHECK_MEM;
		
		return m_xor(utlast->next,flag);
	}
	else
	{
		up(Pure_hi); up(Pure_lo);
		gnuz(ut[v][hashnr],v,hi,lo,counter++,1);


		// Check memory limits every 10. allocation due to performance
		// -----------------------------------------------------------
		CHECK_MEM;
		
		return m_xor(ut[v][hashnr],flag);
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Free_Node	// Caches masked Dirty-Tree          //
/* ========================================================================= */
void dd_man::OKFDD_Free_Node(utnode* tocache)
{
	// Make new entry while removing old one
	// -------------------------------------
	rc_cache_last	   	= rc_cache_last->next;
	uthelp		  	= rc_cache_last->root;
	I0		      	= rc_cache_last->idnum;
	rc_cache_last->root     = m_and(tocache);
	rc_cache_last->idnum    = rc_cache_last->root->idnum;

	// If allowed call freedom with pure pointer
	// -----------------------------------------
	if (uthelp->ref_c != 0) OKFDD_Free_Node_Slave(uthelp,I0);
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Free_Node_Slave // Frees root node recursively //
/* ========================================================================= */
void dd_man::OKFDD_Free_Node_Slave(     utnode* F,
					ulint   F_idnum)
{
	// Freedom allowed ?
	// -----------------
	if (--F->ref_c == 1)
	{
		// Extract pure pointer
		// --------------------
		utnode* Pure_hi = m_and(F->hi_p); Pure_lo = m_and(F->lo_p);

		// Calculate hashposition
		// ----------------------
		hashnr = (Pure_hi->idnum + Pure_lo->idnum) %
			  ut_sizes[ut_hashsize[F->label_i]];

		// ------------------------------
		// Search utnode in correct chain
		// ------------------------------
		if ((uthelp = ut[F->label_i][hashnr]) != NULL)
		{
			// Check first node for idnum / Match -> free node
			// No match -> inspect chain
			// -----------------------------------------------
			if (uthelp->idnum == F_idnum)
			{
				ut[F->label_i][hashnr] 	= uthelp->next;
				utlast 			= uthelp;
			}
			else
			{
				while((utlast = uthelp->next) != NULL)
				{
					// Check for idnum
					// Match -> return node
					// No match -> inspect next element
					// --------------------------------
					if (utlast->idnum == F_idnum)
					{
						uthelp->next = utlast->next;
						break;
					}

					uthelp = utlast;
				}

				// If next line is true, something's damaged
				// the data structure (this error should'nt
				// be possible)
				// -----------------------------------------
				if (utlast == NULL)
				if (OKFDD_Outputflags & 64)
				{
   					cout << "OKFDD_Free_Node: Node ";
					cout << (m_and(F))->idnum;
					cout << " is not in chain ...\n";
					OKFDD_Error = Err_Chain; return;
				}
			}

			// Well, if next line is true,
			// there's a big cake of trouble
			// -----------------------------	       	       
			if (F != utlast)
			{
				cout(2) << "OKFDD_Free_Node: * Alarm *\n"; }

			// ----------------------
			// Node found -> Cache it
			// ----------------------
			OKFDD_No_UTNodes_Per_Lvl[F->label_i]--;
		   	OKFDD_No_UTNodes--;
			utlast->idnum = DUMMY;
		   	utlast->link  = uc_last; uc_last = utlast;

			// Free sons recursively
			// ---------------------
			if (Pure_lo->ref_c != 0)
		  	OKFDD_Free_Node_Slave(Pure_lo,Pure_lo->idnum);
			if (Pure_hi->ref_c != 0)
			OKFDD_Free_Node_Slave(Pure_hi,Pure_hi->idnum);
		}
		else
		{
	       		if (OKFDD_Outputflags & 64)
	       		{
   	       			cout << "OKFDD_Free_Node: Node ";
	       			cout << (m_and(F))->idnum;
	       			cout << " unreal (Hashpos is empty) ...\n";
	       			OKFDD_Error = Err_Hashpos; return;
	       		}
		}
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Free_Fct_Cache // Frees all Dirty-Trees	     //
/* ========================================================================= */
void dd_man::OKFDD_Free_Fct_Cache()
{
	cout(128) << "OKFDD_Free_Fct_Cache: Freeing function cache ... \n";

	usint loop; fu1(loop,0,rc_cachesize) OKFDD_Free_Node(OKFDD_ONE);
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::ICT // Insert (Code,F,G,R) in computed table	     //
/* ========================================================================= */
void dd_man::ICT(       usint   Code,
	      		utnode* F,
			utnode* G,
			utnode* R       )
{
	// Computed table activated?
	// -------------------------
	if (OKFDD_CT == FALSE) return;
	
/**/ // ctentry++;

	// Memory limit reached ?
	// ----------------------
	if (Overflow == TRUE) return;

	// First argument always gets root with smaller address
	// ----------------------------------------------------
	if ((ulint)F > (ulint)G) { uth1 = F; F = G; G = uth1; }

	hashnr = (( (ulint)F + (ulint)G ) % ct_hashsize) + J0;

	if (ctl[hashnr] == 0)
	{
		// Create ctnode with next pointer on itself
		// -----------------------------------------
		gnc(ct[hashnr],Code,F,G,R,NULL); ct[hashnr]->next = ct[hashnr];
		ctl[hashnr]++; OKFDD_No_CTNodes++;
	}
	else if (ctl[hashnr] < ct_searchlen)
	{
		// Append next chain element
		// ----------------------
		cthelp = ct[hashnr]->next;
		gnc(ct[hashnr]->next,Code,F,G,R,cthelp);
		ctl[hashnr]++; OKFDD_No_CTNodes++;
	}
	else
	{
		// Chain is full -> Overwrite next chain element
		// ---------------------------------------------
		cthelp = ct[hashnr]->next;
		gnc(ct[hashnr]->next,Code,F,G,R,cthelp->next);
		cthelp->R = (utnode*)cc_last; cc_last = cthelp;
		
		// Added: 10.08.95
/**/		// cthelp->R_idnum = DUMMY;
      	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::CTLO // Computed table lookup-function		     //
/* ========================================================================= */
utnode* dd_man::CTLO(   usint   Code,
			utnode* F,
			utnode* G       )
{
	// Computed table activated?
	// -------------------------
	if (OKFDD_CT == FALSE) return NULL;
	
	// First argument always gets root with smaller address
	// ----------------------------------------------------
	if ((ulint)F > (ulint)G) { uth1 = F; F = G; G = uth1; }

	static uchar j;
	static uchar next;

/**/ // ctlookup++; ctlookuphitno++;

	hashnr = (( (ulint)F + (ulint)G ) % ct_hashsize) + J0;

	// Calculate masked Code
	// switch_flag of result R (Bit 0) is ignored here
	// -----------------------------------------------
	Code = (Code << 5) + ((m_sel(F) << 4) +
			      (m_sel(G) << 3) +
		  	     ((m_and(F))->switch_flag << 2) +
		             ((m_and(G))->switch_flag << 1) );

	// Check for chain / If chain exists -> check it for desired entry
	// ---------------------------------------------------------------
	if (ctl[hashnr] != 0)
	{
		// Init chain search with chain length and pointer on element
		// ----------------------------------------------------------
		ctlast = ct[hashnr]; cthelp = ctlast->next; j = ctl[hashnr];

		next = TRUE;

		do
		{
			// Check chain element for desired entries (Code,F,G)
			// No match -> go to next chain element
			// --------------------------------------------------
			if (cthelp->F_idnum == (m_and(F))->idnum)
			if (cthelp->G_idnum == (m_and(G))->idnum)
			// Check Code without Bit 0 which holds the
			// switch_flag of result R
			// ----------------------------------------
			if (((cthelp->Code) & 1022) == Code)
			{
				// Entries okay, but what about the result node
				// (Still the wanted info in the node?)
				// --------------------------------------------
				if (cthelp->R_idnum==(m_and(cthelp->R))->idnum)
				{
/**/ //				 ctlookuphit++;

				     	// Set chain entry on predecessor
				     	// of match
				     	// ------------------------------
				     	ct[hashnr] = ctlast;

					// Is switch_flag still the same in R?
					// -----------------------------------
				     	if ((m_and(cthelp->R))->switch_flag ==
					    (uchar)(cthelp->Code & 1))
					{
						return cthelp->R;
					}
					// The else case should increase
					// performance and still work correct
					// ----------------------------------
				     	else	return m_xor(cthelp->R,1);
				}

				// Refresh_cycles don`t match ->
				// Entry is useless -> delete it
				// -----------------------------
				if (ctl[hashnr]-- == 1)
				{
					ct[hashnr]->R = (utnode*)cc_last;
					cc_last = ct[hashnr];
					OKFDD_No_CTNodes--;
					
					// Added: 10.08.95
/**/					//ct[hashnr]->R_idnum = DUMMY;
					
					return NULL;
				}
				else
				{
					ctlast->next = cthelp->next;
					cthelp->R = (utnode*)cc_last;
					cc_last = cthelp; OKFDD_No_CTNodes--;

					// Added: 10.08.95
/**/					// cthelp->R_idnum = DUMMY;

					ct[hashnr] = cthelp = ctlast->next;

					// pointer is now on next chain element
					// -> don`t go any further in next step
					// ------------------------------------
					next = FALSE;
				}
			}

			// Go to next chain element (when needed)
			// --------------------------------------
			if (next == TRUE)
			{
				ctlast = cthelp; cthelp = ctlast->next;
			}
			else 	next = TRUE;

/**/ //		 ctlookuphitno++;
		}
		while(j-- > 1);

/**/ //	 ctlookuphitno--;
	}
	return NULL;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::CTLO // Computed table lookup-function		     //
/* ========================================================================= */
void dd_man::OKFDD_Show_CT_Chain(ulint Anchor)
{
	// Check if chain exists
	// ---------------------
	if (Anchor < ct_hashsize*2)
	if (ctl[Anchor] != 0)
	{
		breakline;
	        cout << "OKFDD_Show_CT_Chain: Anchorposition ";
		cout.width(6); cout.fill('0'); cout << Anchor << ":\n";
		breakline;
		
		cthelp = ct[Anchor];

		// Show ctnode entries for each chain element
		// ------------------------------------------
		fu1(I0,0,(ulint)ctl[Anchor])
		{
			cout << "Node O1-<";
			
			cout.width(9); cout.fill('.'); cout << cthelp->F_idnum;
			
			if (cthelp->Code & 16) cout << "*"; else cout << " ";
			if (cthelp->Code &  4) cout << "x"; else cout << " ";

			cout << "> O2-<";
			
			cout.width(9); cout.fill('.'); cout << cthelp->G_idnum;
			
			if (cthelp->Code & 8)  cout << "*"; else cout << " ";
			if (cthelp->Code & 2)  cout << "x"; else cout << " ";

			cout << "> Res-<";
			
			cout.width(9); cout.fill('.'); cout << cthelp->R_idnum;

      			if (m_sel(cthelp->R))  cout << "*"; else cout << " ";
			if (cthelp->Code & 1)  cout << "x"; else cout << " ";

			cout.width(2); cout.fill('0');
			cout << "> Code-<" << (cthelp->Code >> 5);
       			cout << "> at " << cthelp << "\n";;

			cthelp = cthelp->next;
		}
		
		breakline;
		
		return;
	}
	
       	// Show output if allowed
       	// ----------------------
       	if (OKFDD_Outputflags & 2)
       	{
	       	cout << "OKFDD_Show_CT_Chain: No chain at position ";
	       	cout.width(6); cout.fill('0'); cout << Anchor << ".\n";	
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Resize_ut	// Resize ut[v] to new size	     //
/* ========================================================================= */
void dd_man::OKFDD_Resize_ut(usint v)
{
	// If allowed show action
	// ----------------------
	if (OKFDD_Outputflags & 256)
	{
		cout << "OKFDD_Resize_ut: Changing ut[";
		cout.width(5); cout.fill('.');
		cout << v << "] ";
		cout.width(6); cout.fill('.');
		cout << ut_sizes[ut_hashsize[v]] << " -> ";
		cout.width(6); cout.fill('.');
		cout << ut_sizes[OKFDD_New_Table_Size] << "\n";
	}

	// Allocate memory for new table
	// -----------------------------
	ut_switch = new utnode*[ut_sizes[OKFDD_New_Table_Size]];
	fu1(I0,0,ut_sizes[OKFDD_New_Table_Size]) ut_switch[I0] = NULL;

	// ---------------------------------------------------------------
	// Rearrange each chain by moving it from the old table to the new
	// one (ut[v][...] -> ut_switch[...])
	// ---------------------------------------------------------------
	fu1(I0,0,ut_sizes[ut_hashsize[v]]) if ((uthelp = ut[v][I0]) != NULL)
	{
		do
		{
			// Calculate hashposition in new table
			// -----------------------------------
			hashnr = ((m_and(uthelp->hi_p))->idnum +
				  (m_and(uthelp->lo_p))->idnum) %
				   ut_sizes[OKFDD_New_Table_Size];

			// Remove node from old chain head
			// -------------------------------
			utlast = uthelp; uthelp = utlast->next;

			// Place node on new chain head
			// ----------------------------
			utlast->next 	  = ut_switch[hashnr];
			ut_switch[hashnr] = utlast;
		}
		while(uthelp != NULL);
	}

	// Make new ut to actual ut
	// ------------------------
	delete[] ut[v];
	ut[v] = ut_switch; ut_hashsize[v] = OKFDD_New_Table_Size;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Cachecut // Set size of uc_cache to given %    //
/* ========================================================================= */
void dd_man::OKFDD_Cachecut(uchar percentage)
{
	// Do not change cachesize if % is out of bounds (100 is nonsens too)
	// ------------------------------------------------------------------
	if ((percentage < 1) || (percentage > 99)) return;

	// Counts elements
	// ---------------
	I0 = 0; uthelp = uc_last;
	while(uthelp != NULL) { uthelp = (utnode*)uthelp->link; I0++; }
	if (I0 == 0) return;

	// Set cachesize to new value
	// --------------------------
	I1 = I0 * percentage / 100;

	uthelp = uc_last;
	fu(I2,1,I1)
	{
		uc_last = (utnode*)uthelp->link; delete uthelp;
		uthelp  = uc_last;
	}

	// If allowed show action
	// ----------------------
	if (OKFDD_Outputflags & 256)
	{
		cout << "OKFDD_Cachecut: Reducing size from ";
		cout.width(7); cout.fill('0'); cout << I0 << " to ";
		cout.width(7); cout.fill('0'); cout << I0-I1; nextline;
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::get_word  // Extract next word in given string	     //
/* ========================================================================= */
void dd_man::get_word(  char*   string	  = NULL,
			usint   searchmax = name_limit)
{
	usint i 	= 0;
	OKFDD_Error 	= Err_No_Error;
	last		= FALSE;

	// Extract next word (string -> gl_name)
	// -------------------------------------
	while((string[position] != ' ') && (string[position] != 0))
	{
		if (i == searchmax)
		{
			OKFDD_Error = Err_Namelength; return;
		}
		gl_name[i++] = string[position++];
	}

	// Clear rest of word field
	// ------------------------
	fu1(I3,i,name_limit) gl_name[I3] = ' ';

	// Continue reading till start of next word
	// ----------------------------------------
	while(string[position++] == ' ') { }

	// String end reached -> last = TRUE
	// ---------------------------------
	if (string[--position] == 0) last = TRUE;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::get_line // Copy next line from file to field 'line' //
/* ========================================================================= */
void dd_man::get_line(  ifstream & file,
			char*	   string)
{
        do
        {

	OKFDD_Error = Err_No_Error;

	I3 = I4 = 0;

	file.getline(string,line_limit);

	while(TRUE)
	{
		// Continuation line ?
		// -------------------
		if (string[I4] == '\\')
		{
			I4 = 0; file.getline(string,line_limit);
		}

		// Copy character in line / End of string ? -> Quit
		// ------------------------------------------------
		if ((line[I3] = string[I4++]) == 0) break;

		// Line limit reached ? -> Error
		// -----------------------------
		if (++I3 == line_limit)
		{
			OKFDD_Error = Err_Linelength; return;
		}
	}
	line[I3] = 0;

        }
	while((line[0] == 0) && (!file.eof()));
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::get_name // Get name of label count		     //
/* ========================================================================= */
void dd_man::get_name(  uchar   type,
			usint   count)
{
	// Set type on position 7 in gl_name
	// ---------------------------------
	switch (type)
	{
		case PI: gl_name[6] = 'I'; break;
		case PO: gl_name[6] = 'O'; break;
		case TF: gl_name[6] = 'T';
	}

	// First 5 position are reserved for number
	// ----------------------------------------
	short i;
	fd(i,4,0)
	{
		if (count >= 10)
		{
			gl_name[i] = count%10 + 48;
			count 	   = (count - count%10)/10;
		}
		else if (count >0)
		{
			gl_name[i] = count%10 + 48; count = 0;
		}
		else  { gl_name[i] = 48; }
	}

	// Position 6 gets underscore and rest is empty
	// --------------------------------------------
	gl_name[5] = '_'; fu(i,7,name_limit-1) gl_name[i] = ' ';
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Make_Prime // FOA label for gl_name / return ptr     //
/* ========================================================================= */
names* dd_man::Make_Prime(uchar type)
{
	// New primary construction allowed ?
	// ----------------------------------
	if (type == TF)
	{
		if ( T_F > pi_limit-2)
                {
                     OKFDD_Error = Err_Primelimit; return NULL;
                }
                else OKFDD_Error = Err_No_Error;
	}
	else
	{
		if ((OKFDD_P_I+OKFDD_P_O) > pi_limit-2)
                {
                     OKFDD_Error = Err_Primelimit; return NULL;
                }
                else OKFDD_Error = Err_No_Error;
	}

	hashnr = (gl_name[6]*7 + gl_name[4]*5 + gl_name[2]*3) % pi_hashsize;

	// --------------------------------------------------------------------
	// Chain exists -> Insert new entry in chain / Else -> Make first entry
	// --------------------------------------------------------------------
	if (prime[hashnr] != NULL)
	{
		names* nhelp = prime[hashnr]; names* nlast;

		do
		{
			// primary exists ? -> Mysterious
			// ------------------------------
			if (nhelp->label_i == OKFDD_Next_Label)
                        /* Maybe label is to weak               */
                        /* -> add 'if (nhelp->type == type)' !? */
			{
			   cout(2) << "Make_Prime: Entry <";
                           fui(i,0,16) cout(2) << gl_name[i];
			   cout(2) << "...> already exists ...\n";

			   OKFDD_Error = Err_Prime_ex; return nhelp;
			}

			nlast = nhelp;
		}
		while ((nhelp = nhelp->next) != NULL);

		toprime[OKFDD_Next_Label] = nlast->next =
                   new names(type,OKFDD_Next_Label);
	}
	else toprime[OKFDD_Next_Label] = prime[hashnr] =
                new names(type,OKFDD_Next_Label);


	// Show construction (when allowed)
	// --------------------------------
	if (OKFDD_Outputflags & 512)
	{
		cout << "Making prime for <";
		fui(i,0,16) cout << gl_name[i];
                cout << "...> as IOT " << (short)type << " for label ";
                cout << OKFDD_Next_Label << "\n";
	}

	// Increase global counters
	// ------------------------
	switch (type)
	{
		case PI: All_Prime[OKFDD_P_I]            = OKFDD_Next_Label;
                         OKFDD_P_I++;    break;
		case PO: All_Prime[pi_limit-1-OKFDD_P_O] = OKFDD_Next_Label;
                         OKFDD_P_O++;    break;
		case TF: T_Prime[T_F]			 = OKFDD_Next_Label;
                         T_F++;
	}

	// --------------------------------------------------------------------
	// If unique table of primary with OKFDD_Next_Label doesn`t exist
        // -> create it
	// --------------------------------------------------------------------
	if (ut[OKFDD_Next_Label] == NULL)
	{
	   cout(1024) << "Make_Prime: Creating unique table for label ";
           cout(1024) << OKFDD_Next_Label << " ...\n";

	   // Set initial table size then allocate table and clear it
	   // -------------------------------------------------------
	   ut_hashsize[OKFDD_Next_Label] = ut_hashsize_d;
           ut[OKFDD_Next_Label] = new utnode*[ut_sizes[ut_hashsize[
              OKFDD_Next_Label]]];

	   ulint loop;
           fu1(loop,0,ut_sizes[ut_hashsize[OKFDD_Next_Label]])
              ut[OKFDD_Next_Label][loop] = NULL;
	}

	// If ZSBDDs are in usage -> Each new primary input gets copies
	// ------------------------------------------------------------
	if (ZSBDD_Init_done == TRUE) if (type == PI) ZSBDD_Copy(OKFDD_P_I-1);

	return toprime[OKFDD_Next_Label++];
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Remove_Prime // Remove TF or PO completely     //
/* ========================================================================= */
void dd_man::OKFDD_Remove_Prime(usint label)
{
	// Don't remove primary inputs
	// ---------------------------
	if ((nameshelp = toprime[label])->type == PI)
        {
             OKFDD_Error = Err_Error; return;
        }
        else OKFDD_Error = Err_No_Error;

	// If root exists -> Free tree
	// ---------------------------
	if ((uthelp = nameshelp->root) != NULL) OKFDD_Free_Node(uthelp);

	// Correct All_Prime-Table if primary is of type PO
        // (TFs are removed elsewhere)
	// ------------------------------------------------
	if (nameshelp->type == PO)
	{
		// Get position of label in All_Prime
		// ----------------------------------
		fu1(I0,0,OKFDD_P_O) if (All_Prime[pi_limit-1-I0]==label) break;

		if (I0 == OKFDD_P_O) { OKFDD_Error = Err_Error; return; }

		// Shift labels close
		// ------------------
		fu1(I1,I0,OKFDD_P_O-1)
                   All_Prime[pi_limit-1-I1] = All_Prime[pi_limit-1-(I1+1)];

		All_Prime[pi_limit-OKFDD_P_O] = 0;

		OKFDD_P_O--;
	}
	else    T_F--;
        /* Cause of counted out PIs in first line of routine */
        /* -> TF is the only alternative in this case        */

	toprime[label] = NULL;

	fu1(I0,0,name_limit) gl_name[I0] = nameshelp->name[I0];

	hashnr = (gl_name[6]*7 + gl_name[4]*5 + gl_name[2]*3) % pi_hashsize;

	// ------------------------------------------------------
	// Chain exists -> Free correct entry / No Chain -> Error
	// ------------------------------------------------------
	if (prime[hashnr] != NULL)
	{
		names* nhelp = prime[hashnr]; names* nlast = NULL;

		// Search chain
		// ------------
		I1 = 0;
                do { if (nhelp->label_i == label) break; nlast = nhelp; I1++; }
                while ((nhelp = nhelp->next) != NULL);

		// -------------------------------
		// Make new links and remove entry
		// -------------------------------
		if (I1 == 0) prime[hashnr] = nhelp->next;
                else         nlast->next   = nhelp->next;

		delete nhelp;
	}
	else    OKFDD_Error = Err_Prime_miss;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::find_prime // Find label for gl_name / return ptr    //
/* ========================================================================= */
names* dd_man::find_prime()
{
	uchar correct;

	hashnr = (gl_name[6]*7 + gl_name[4]*5 + gl_name[2]*3) % pi_hashsize;

	// Chain not empty ?
	// -----------------
	if (prime[hashnr] != NULL)
	{
		names* nhelp = prime[hashnr]; names* nlast;

		// -----------------------
		// Search primary in chain
		// -----------------------
		do
		{
			// Check all characters in name / Not all correct
                        // -> denied / Else -> return pointer to primary
			// ----------------------------------------------
			correct = TRUE;
                        fui(i,0,name_limit-1) if (nhelp->name[i]!=gl_name[i])
                        { nlast = nhelp; correct = FALSE; }

			if (correct == TRUE) return nhelp;
		}
		while ((nhelp = nhelp->next) != NULL);
	}

	return NULL;
};
/* ========================================================================= */

/* ========================================================================= */
// Function:    dd_man::Interleave // Calculate good var order for PIs       //
/* ========================================================================= */
void dd_man::Interleave(hnode* node)
{
	// Well, if order is already determined -> Say goodbye to interleaving
	// -------------------------------------------------------------------
	if (OKFDD_Use_Order == TRUE) return;

	// Init FANIN countertable
	// -----------------------
	fu1(I0,0,pi_limit) OKFDD_Result_2[I0] = 0; OKFDD_Hierarchy_Depth = 0;

	// Weight gates according #nodes beneath and depth
	// -----------------------------------------------
	Weight_watcher(gl_list[0]);

	// Show FANINs
	// -----------
	if (OKFDD_Outputflags & 4)
	{
		cout << "Depth " << OKFDD_Hierarchy_Depth << "\n\n";

		fu1(I0,0,pi_limit) if (OKFDD_Result_2[I0] != 0)
		{
			cout << "FANIN " <<    I0  << ": ";
			cout << OKFDD_Result_2[I0] << "\n";
		}
	}

	// Interleave variable order
	// -------------------------
	VL_first = VL_last = new son_ptr(0);
	fu1(I0,0,pi_limit) OKFDD_Result_2[I0] = 0; hrhelp = NULL;

	cout(1) << "\nInterleaving ...\n";

	// Bubblesort primary outputs according descending weights
	// -------------------------------------------------------
	fd(I4,maxo_i-1,1) fu1(I3,0,I4)
	if (gl_list[Act_Prime[pi_limit-1-I3]]->weight <
	    gl_list[Act_Prime[pi_limit-2-I3]]->weight )
	{
		I0 			 = Act_Prime[pi_limit-1-I3];
		Act_Prime[pi_limit-1-I3] = Act_Prime[pi_limit-1-(I3+1)];
		Act_Prime[pi_limit-2-I3] = I0;
	}

	// Interleave for all primary outputs according descending weights
	// ---------------------------------------------------------------
	fu1(I0,0,maxo_i)
	{
		// Set from label to actual PO
		// ---------------------------
		setfromlaba(Act_Prime[pi_limit-1-I0]);
		
		Interleave_slave(gl_list[Act_Prime[pi_limit-1-I0]],K3);
	}

	// --------------------------------------------------------------
	// Extract PI order from created VL-list (while list is unzipped)
	// --------------------------------------------------------------
	hhelp = VL_first->next; delete VL_first; K0 = 2;

	do
	{
		// Entry in list is primary input ?
		// --------------------------------
		if (hhelp->son_label != 0)
		if (gl_list[K1 = hhelp->son_label]->type == PI)
		{
			if(OKFDD_Outputflags & 2048)
			{
				cout << "PI " << K1 << "   \twith pi ";
				cout << "inverse " << K0 << "\n";
			}
			
			OKFDD_PI_Level_Table[K1] = K0; K0 += 2;
		}

		VL_first = hhelp; hhelp = hhelp->next; delete VL_first;
	}
	while(hhelp != NULL);
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Setfroms // Sets froms for gate dependency	     //
/* ========================================================================= */
void dd_man::Setfroms(hnode* node)
{
	son_ptr* hhelp1;

	node->from = K3;

	if ((hhelp1 = node->next) != NULL)
	{
		do
		{
			if (gl_list[hhelp1->sort_label]->from != K3)
			{
				Setfroms(gl_list[hhelp1->sort_label]);
			}
		}
		while((hhelp1 = hhelp1->next) != NULL);
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Interleave_slave // Calculate good var order for PIs //
/* ========================================================================= */
void dd_man::Interleave_slave(hnode* node,
			      usint  from_lab)
{
	son_ptr* hhelp1;

	// Gate already visited (from another primary output) -> Position is
	// next entry reference for interleaving list
	// -----------------------------------------------------------------
	if (node->flag == 2)
	{
		if (node->from != from_lab)
		{
			if ((OKFDD_PIs_Only != TRUE) ||
			    (gl_list[node->label_i]->type == PI))
			hrhelp = node;
			
			node->from = from_lab;
		}
		return;
	}

	// Notice for this node (gate), we were here
	// -----------------------------------------
	node->flag = 2; node->from = from_lab;

	// ----------------------------------------------
	// For each input of node -> Interleave
	// (If no chain (no input) -> node is of type PI)
	// ----------------------------------------------
	if ((hhelp1 = node->next) != NULL)
	{
		do
		{
			setfromlabb(from_lab);
			
			if (node->FANIN >= OKFDD_Min_FANIN)
		     	if (node->depth > (OKFDD_Hierarchy_Depth*OKFDD_Hiend))
		       	{
				setfromlabb(hhelp1->sort_label);
			}

			Interleave_slave(gl_list[hhelp1->sort_label],K3);
		}
		while((hhelp1 = hhelp1->next) != NULL);
	}

	if (node->FANIN >= OKFDD_Min_FANIN)
	if (node->depth > (OKFDD_Hierarchy_Depth*OKFDD_Hiend))
	{
		setfromlabb(from_lab); Setfroms(node);
	}

	
       	// ----------------------------
       	// Insert node in VL if allowed
       	// ----------------------------
       	if ((OKFDD_PIs_Only != TRUE) || (gl_list[node->label_i]->type == PI))
	{
		
		if (hrhelp == NULL)
		{
/**/ //	 cout << "Lab/From/Last: " << node->label_i << " " << node->from;
/**/ //  cout << " " << VL_last->son_label << " -> ";

		  // Insert node into top of VL
		  // --------------------------
		  VL_first->next = new son_ptr(node->label_i,VL_first->next);

		  if (VL_last == VL_first) VL_last = VL_first->next;
		
		  OKFDD_Result_2[node->label_i] = (ulint)VL_first->next;
		}
		else
		{
/**/ //	 cout << "Lab/From/Last: " << node->label_i << " " << node->from;
/**/ //  cout << " " << hrhelp->label_i << " -> ";

		  // Insert node next to hrhelp in VL
		  // --------------------------------
		  VL_help       = (son_ptr*)OKFDD_Result_2[hrhelp->label_i];
		  VL_help->next = new son_ptr(node->label_i,VL_help->next);

		  OKFDD_Result_2[node->label_i] = (ulint)(VL_help->next);
		
		  if (VL_help == VL_last) VL_last = VL_help->next;
		}

		hrhelp = node;
   	}
	
	VL_help = VL_first->next;

	// Show some informations (when allowed)
	// -------------------------------------
	if (OKFDD_Outputflags & 4096)
	{
		cout << "From_label: " << from_lab << " ";
		do
		{
			cout << VL_help->son_label << " [";
			cout << gl_list[VL_help->son_label]->from << "] ";
		}
		while((VL_help = VL_help->next) != NULL); cout << "\n";
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Weight_watcher // Weights hierarchy nodes            //
/* ========================================================================= */
void dd_man::Weight_watcher(hnode* node)
{
	// Primary input reached ? / Yes -> return initial weights
	// -------------------------------------------------------
	son_ptr* hhelp1 = node->next;
	
	if (hhelp1 == NULL) { K0 = K1 = 1; return; }

	/* Some ideas how to measure PIs for ALL TFs & POs efficiently ? */

	// -----------------------
	// Traverse hierarchy tree
	// -----------------------
	do
	{
		// Input gate already visited ?
		// Yes -> Use weights to calculate actual gate's weight
		// ----------------------------------------------------
		if (gl_list[K2 = hhelp1->son_label]->flag != 1)
		{
			Weight_watcher(gl_list[K2]);
		}
		else
		{
			K0 = gl_list[K2]->depth + 1;
			K1 = gl_list[K2]->no_nodes;
		}

		// Correct depth (always maximum of all son-depths plus 1
	  	// Number of nodes is sum of all gates under sons + sons
		// ------------------------------------------------------
		if (node->depth < K0) node->depth = K0; node->no_nodes += K1;
	}
	while((hhelp1 = hhelp1->next) != NULL);


	/* EVERY multiple usage of PIs (or TFs) is measured in amount of it */
	/* -> So size is >= act. number of hier_nodes but this is good      */
	/* On the other side -> Should take care of shared nodes !?         */

	// If gate is NO primary input -> Make some notes
	// ----------------------------------------------
	if ((node->type == TF) || (node->type == PO))
	{
		// Notes already taken -> Do nothing
		// ---------------------------------
		if (node->flag != 1)
		{
			node->flag = 1; node->no_nodes++;

/**/		    OKFDD_Result_2[node->FANIN]++;

/**/		    if (node->depth > OKFDD_Hierarchy_Depth)
			OKFDD_Hierarchy_Depth = node->depth;

			// ---------------------------------------------------
			// Judge the weights /* Now, THIS is the most
			// important part of the routine -> So judge wisely */
			// ---------------------------------------------------
			node->weight =  OKFDD_Mulend *
			(
			(node->no_nodes * node->no_nodes		) * 
			 (OKFDD_Bitend & 1)			+
			(node->no_nodes				 	) * 
			 (OKFDD_Bitend & 2)	       	/2      +
			(node->depth    * node->depth		   	) * 
			 (OKFDD_Bitend & 4)	      	/4      +
			(node->depth				    	) * 
			 (OKFDD_Bitend & 8)	      	/8      +
			(node->no_nodes * node->depth		   	) * 
			 (OKFDD_Bitend & 16)	   	/16     +
			(node->no_nodes * node->no_nodes * node->depth  ) * 
			 (OKFDD_Bitend & 32)	 	/32     +
			(node->no_nodes * node->depth * node->depth     ) * 
			 (OKFDD_Bitend & 64)	 	/64
			)
			/
			(
			(node->no_nodes * node->no_nodes		) * 
			 (OKFDD_Bitend & 128)		/128    +
			(node->no_nodes				 	) * 
			 (OKFDD_Bitend & 256)		/256    +
			(node->depth    * node->depth		   	) * 
			 (OKFDD_Bitend & 512)		/512    +
			(node->depth				    	) * 
			 (OKFDD_Bitend & 1024)       	/1024   +
			(node->no_nodes * node->depth		   	) * 
			 (OKFDD_Bitend & 2048)       	/2048   +
			(node->no_nodes * node->no_nodes * node->depth  ) * 
			 (OKFDD_Bitend & 4096)       	/4096   +
			(node->no_nodes * node->depth * node->depth     ) * 
			 (OKFDD_Bitend & 8192)       	/8192   +
			(1					      	) * 
			 (OKFDD_Bitend & 16384)      	/16384
			);

/**/ //	cout << node->weight << " ";

			// Bubble-Sort inputs according descending priority
			// ------------------------------------------------
			if ((I1 = node->FANIN) > 1)
			{
				// Extract all inputs of gate and store them
				// in OKFDD_Result
				// -----------------------------------------
				hhelp1 = node->next;
				
				fu1(I0,0,I1)
				{
				      OKFDD_Result[I0] = hhelp1->son_label;
				      hhelp1 		 = hhelp1->next;
				}

				fd(I4,I1-1,1) fu1(I3,0,I4)
				if (gl_list[OKFDD_Result[I3]]->weight < 
				    gl_list[OKFDD_Result[I3+1]]->weight)
				{
				      I0 		 = OKFDD_Result[I3];
				      OKFDD_Result[I3]   = OKFDD_Result[I3+1];
				      OKFDD_Result[I3+1] = I0;
				}

				// Store new order in son chain
				// ----------------------------
				hhelp1 = node->next;
				
				fu1(I0,0,I1)
				{
					hhelp1->sort_label = OKFDD_Result[I0];
					hhelp1 		   = hhelp1->next;
				}
			}
			else    node->next->sort_label = node->next->son_label;
		}
	}

	// Increase depth and set size
	// ---------------------------
	K0 = node->depth + 1; K1 = node->no_nodes;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::DFS // Traverse hierarchy tree in Depth-First-Search //
/* ========================================================================= */
void dd_man::DFS(hnode* node)
{
	// Check if gate has FANIN / No -> Primary inputs reached
	// ------------------------------------------------------
	son_ptr* hhelp1 = node->next; if (hhelp1 == NULL) return;

	// Check each input of gate if already visited / No -> Jump to it
	// --------------------------------------------------------------
	do
        {
           if (gl_list[hhelp1->son_label]->flag != 3)
           DFS(gl_list[hhelp1->son_label]);
        }
        while((hhelp1 = hhelp1->next) != NULL);

	// Here we are, returned from ALL inputs (if there are any)
        // If gate isn't primary input make some notes
	// --------------------------------------------------------
	if ((node->type == TF) || (node->type == PO))
	{
		// Notes already taken (This node (gate) is a reconvergence
                // node) ? / Yes -> Do nothing
		// --------------------------------------------------------
		if (node->flag != 3)
		{
			node->flag = 3;

			cout(2048) << "Entry for label " << node->label_i;
                        cout(2048) << "  \twith pi inverse " << first_minus;
                        cout(2048) << "  \tin " << I2 << "\n";

			// Notice order for SUBSTITUTION or SYNTHESIS routines
			// ---------------------------------------------------
			if (OKFDD_Subst_Method == FALSE)
                           OKFDD_Result_2[I2++] = node->label_i;
			else
                           OKFDD_Result_2[I2--] = node->label_i;

			// Notice position in variable order
			// ---------------------------------
			OKFDD_PI_Level_Table[node->label_i] = first_minus--;
			/* TFs don`t need expanded OKFDD_PI_Level_Table */
                        /* -> No '2 * ...' needed !                     */
		}
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Store_Root // Stores root of given OKFDD as PO //
/* ========================================================================= */
usint dd_man::OKFDD_Store_Root(utnode* root)
{
	// Get new name and label for primary output
	// -----------------------------------------
	get_name(PO,OKFDD_Next_Label); nameshelp = Make_Prime(PO);
        nameshelp->root = root;

	if (OKFDD_Error == Err_No_Error) return (OKFDD_Next_Label-1);
        else                             return 0;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Get_Lo_Son // Return Low-Son and incr. ref     //
/* ========================================================================= */
utnode* dd_man::OKFDD_Get_Lo_Son(utnode* Root)
{
     utnode* help = (m_and(Root))->lo_p;
     ups(help);
     return help;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Get_Hi_Son // Return High-Son and incr. ref    //
/* ========================================================================= */
utnode* dd_man::OKFDD_Get_Hi_Son(utnode* Root)
{
     utnode* help = (m_and(Root))->hi_p;
     ups(help);
     return help;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Get_Variable // Rtrn root of lab with giv.name //
/* ========================================================================= */
utnode* dd_man::OKFDD_Get_Variable(     char*   name,
					uchar   which)
{
	names*  nhelp;
	usint   lab;

	// Make a copy of input name in global name field
	// ----------------------------------------------
	fu1(I0,0,name_limit) gl_name[I0] = name[I0];

	// Check desired input / No match
        // -> Create primary input and update OKFDD_PI_Order_Table
	// -------------------------------------------------------
	if ((nhelp = find_prime()) == NULL)
	{
	   nhelp = Make_Prime(PI);
	   OKFDD_PI_DTL_Table[nhelp->label_i] = OKFDD_DTL_Default;
	   OKFDD_PI_Level_Table[nhelp->label_i] = 2 * nhelp->label_i;
	   Support_complete();
	}

	lab = nhelp->label_i;

	// Search and find or create new base node with label lab
	// ------------------------------------------------------
	if (which != POSITIVE)
	{
	   switch (OKFDD_PI_DTL_Table[lab])
	   {
	      case D_Shan:    uthelp = FOAUT(lab,OKFDD_ZERO,OKFDD_ONE); break;
	      case D_posD:    uthelp = FOAUT(lab,OKFDD_ONE,OKFDD_ONE);	break;
	      case D_negD:    uthelp = FOAUT(lab,OKFDD_ONE,OKFDD_ZERO);
	   }
	}
	else
	{
	   switch (OKFDD_PI_DTL_Table[lab])
	   {
	      case D_Shan:    uthelp = FOAUT(lab,OKFDD_ONE,OKFDD_ZERO); break;
	      case D_posD:    uthelp = FOAUT(lab,OKFDD_ONE,OKFDD_ZERO); break;
	      case D_negD:    uthelp = FOAUT(lab,OKFDD_ONE,OKFDD_ONE);
	   }
	}

	ups(uthelp); nhelp->root = uthelp; return uthelp;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Get_Root // Return root with given name	     //
/* ========================================================================= */
utnode* dd_man::OKFDD_Get_Root(char* name)
{
	OKFDD_Error = Err_No_Error;

	names* nhelp;

	// Make a copy of output name in global name field
	// -----------------------------------------------
	fu1(I0,0,name_limit) gl_name[I0] = name[I0];

	// Check desired output / No match
        // -> return NULL with error message / Match -> return root of output
	// ------------------------------------------------------------------
	if ((nhelp = find_prime()) != NULL) if (nhelp->root != NULL)
        {
           ups(nhelp->root); return nhelp->root;
        }

	// Show errormessage
	// -----------------
	if (OKFDD_Outputflags & 2)
	{
	   cout << "OKFDD_Get_Root: Output <";
           fu1(I0,0,name_limit) cout << gl_name[I0];
           cout << "> doesn`t exist ...\n";
	}

	OKFDD_Error = Err_Outputmiss; return NULL;
};
/* ========================================================================= */



/*****************************************************************************/
//
//			  >>>     MISCELLANEOUS       <<<
//
/*****************************************************************************/

/* ========================================================================= */
// Function:    OKFDD_Init // Initializes OKFDD-Manager			     //
/* ========================================================================= */
dd_man* OKFDD_Init(     uchar   ut_hashsize_init,
			ulint   ct_hashsize_init,
			ulint   rc_cachesize_init,
			uchar   ct_searchlen_init,
			usint   pi_limit_init,
			ulint   ML_nodes_init,
			ulint   ML_MB_init	)

{
  //	breakline; cout << Version << "\n"; breakline;

	return new dd_man(ut_hashsize_init ,
                          ct_hashsize_init ,
                          rc_cachesize_init,
                          ct_searchlen_init,
			  pi_limit_init    ,
                          ML_nodes_init    ,
                          ML_MB_init	  );
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Quit // Closes manager			     //
/* ========================================================================= */
void dd_man::OKFDD_Quit()
{
/**/ // int check1 = getchar();

	cout(1) << "Deallocation of OKFDD_Manager in progress ... \n";

	// Free caches
	// -----------
	while(uc_last != NULL)
        { uc_last = (utnode*)(uthelp = uc_last)->link; delete uthelp; }
	while(cc_last != NULL)
        { cc_last = (ctnode*)(cthelp = cc_last)->R;    delete cthelp; }

	// Free prime table (Care for PIs would be nice)
	// ---------------------------------------------
	fd(J4,OKFDD_P_O-1,0) OKFDD_Remove_Prime(All_Prime[pi_limit-1-J4]);

	delete[] prime;
	delete[] toprime;
	delete[] All_Prime;
	delete[] Act_Prime;
	delete[] T_Prime;
	delete[] ZSBDD_equals;
	delete[] OKFDD_Result;
	delete[] OKFDD_PI_Order_Table;
	delete[] OKFDD_Result_2;
	delete[] mindtl;
	delete[] aa;
	delete[] min_pi_table;
	delete[] twin_pi_table;
	delete[] lsf;
	delete[] pfada;
	delete[] pfads;
	delete[] pi_level;
	delete[] OKFDD_PI_DTL_Table;
	delete[] OKFDD_PI_Level_Table;
	delete[] OKFDD_No_UTNodes_Per_Lvl;
	delete[] field;

	// Free computed table entries
	// ---------------------------
	fu1(I0,0,ct_hashsize*2)
	{
	   ctlast = ct[I0];
           if (ctl[I0] != 0) fu(I1,1,ctl[I0])
           { cthelp = ctlast; ctlast = ctlast->next; delete cthelp; }
	}

	delete[] ct;
	delete[] ctl;

	// Free unique table entries
	// -------------------------
	fu1(I0,0,pi_limit)
	{
	   if (ut[I0] != NULL) fu1(I2,0,ut_sizes[ut_hashsize[I0]])
	   {
	      utlast = ut[I0][I2];
              while(utlast != NULL)
              { uthelp = utlast; utlast = utlast->next; delete uthelp; }
	   }

	   delete[] ut[I0];
	}

	delete[] ut;
	delete[] ut_hashsize;

	// Free rc_cache
	// -------------
	fu1(I0,0,rc_cachesize)
        {
             rchelp = rc_cache_last; rc_cache_last = rc_cache_last->next;
             delete rchelp;
        }

//      int check2 = getchar();

        delete OKFDD_ONE;
	delete OKFDD_DUMMY;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Version // Output OKFDD_Version / date	     //
/* ========================================================================= */
void dd_man::OKFDD_Version()
{
/**/ // breakline; cout << Version << "\n";
	breakline;

	cout << "ut_hashsize        : " << ut_sizes[ut_hashsize_d]  << "\n";
	cout << "ct_hashsize        : " << ct_hashsize	            << "\n";
	cout << "ct_searchlength    : " << (usint)ct_searchlen      << "\n";
	cout << "rc_cachesize       : " << rc_cachesize	            << "\n";
	cout << "memory_limit_nodes : " << ML_nodes		    << "\n";
	cout << "memory_limit_MB    : " << ML_MB		    << "\n";

	cout << "Readline output    : ";

	if (OKFDD_Outputflags & 16)      cout << "Readline\n";
        else if (OKFDD_Outputflags & 32) cout << "Dots\n";
        else                             cout << "off \n";
        breakline;
	if (OKFDD_Version_Wait == TRUE)
        { cout << "\n< waiting for keystroke >\n"; I0 = getchar(); }
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Set_act_to_min // Reset min.tables to act.vals //
/* ========================================================================= */
void dd_man::OKFDD_Set_act_to_min()
{
	fu1(I0,0,OKFDD_Maxidx)
        {
           min_pi_table[I0] = OKFDD_PI_Order_Table[I0];
           mindtl[I0]       = OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[I0]];
        }
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Support_complete				     //
/* ========================================================================= */
void dd_man::Support_complete()
{
	// Bubble-Sort primary inputs according ascending pi inverse
	// ---------------------------------------------------------
	fu1(I0,0,OKFDD_P_I) OKFDD_Result[I0] = All_Prime[I0];

	if (OKFDD_P_I > 1)
	{
	   fdi(j,OKFDD_P_I-1,1) fui(i,0,j-1)

	   if (OKFDD_PI_Level_Table[OKFDD_Result[i]] >
           OKFDD_PI_Level_Table[OKFDD_Result[i+1]])
	   {
	      K0                = OKFDD_Result[i];
              OKFDD_Result[i]   = OKFDD_Result[i+1];
              OKFDD_Result[i+1] = K0;
	   }
	}

	fu1(I0,0,OKFDD_P_I) OKFDD_PI_Order_Table[I0] = OKFDD_Result[I0];
        OKFDD_PI_Order_Table[OKFDD_P_I] = 0;
	OKFDD_Maxidx = OKFDD_P_I;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Dumporder // Writes a dump of the act.order    //
/* ========================================================================= */
void dd_man::OKFDD_Dumporder(char* dumpfilename = NULL)
{
	char outofpath[50]  = "";
	char outoforder[10] = "dump.ord";

	OKFDD_Error = Err_No_Error;

	// Sort labels (ascending)
	// -----------------------
	fu1(I0,0,OKFDD_Maxidx) OKFDD_Result[I0] = OKFDD_PI_Order_Table[I0];

	fu1(I0,0,OKFDD_Maxidx) fd(I1,OKFDD_Maxidx-1,1)
        if (OKFDD_Result[I1] < OKFDD_Result[I1-1])
	{
	   J0                 = OKFDD_Result[I1];
           OKFDD_Result[I1]   = OKFDD_Result[I1-1];
           OKFDD_Result[I1-1] = J0;
	}

	// Show order
	// ----------
	if (OKFDD_Outputflags & 8192)
	{
	   cout << "\n.order :\n";

	   fu1(I0,0,OKFDD_Maxidx)
           {
              cout << (OKFDD_PI_Level_Table[OKFDD_Result[I0]]/2) << " <";
              cout << dtl[OKFDD_PI_DTL_Table[OKFDD_Result[I0]]] << ">  ";
           }
           nextline;
	}

	// ----------------
	// Create orderfile
	// ----------------
	if (dumpfilename != NULL) strcat(outofpath,dumpfilename);
        else                      strcat(outofpath,outoforder);

	ofstream forder(outofpath);
	if (!forder)
        {
           cout(2) << "OKFDD_Dumporder: Can`t create orderfile ...\n";
           OKFDD_Error = Err_Dumporder; return;
        }

	// Write Header (Name of benchmark, # of PIs and # of POs)
	// -------------------------------------------------------
	forder.write("# PUMA * OKFDD_Manager V2.22 Copyright (C)'96 by Andreas HETT\n",62);
	forder.write("# \n",3);
	forder.write("# ORDER_file for CIRCUIT < ",27);

	I0 = 0; do { if (FILEX[I0++] == '\0') break; } while (TRUE);
        forder.write(FILEX,I0-1); forder.write(" >\n",3);

	forder.write("\n# Number of PIs: ",18);
	J0 = OKFDD_P_I;
        fu1(K1,0,5)
        { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }
	fd(J1,4,0) forder.put(line[J1]);

	forder.write("\n# Number of POs: ",18);
	J0 = OKFDD_P_O;
        fu1(K1,0,5)
        { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }
	fd(J1,4,0) forder.put(line[J1]);

	// -----------
	// Write Order
	// -----------
	forder.write("\n\n.order\n",9);

	fu1(I0,0,OKFDD_Maxidx)
	{
	   J0 = OKFDD_PI_Level_Table[OKFDD_Result[I0]]/2;
           fu1(K1,0,5)
           { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }

	   fd(J1,4,0) forder.put(line[J1]);

	   forder.write(" ",1);
           forder.put(dtl[OKFDD_PI_DTL_Table[OKFDD_Result[I0]]]);
           forder.write("\n",1);
	}

	forder.write(".end",4);

	forder.close();
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::repro // Erzeugen eines Doppelggs.v.vater (inv. T1R) //
/* ========================================================================= */
utnode* dd_man::repro( utnode* vater)
{
	gnus(twin,vater->label_i,vater->hi_p,vater->lo_p,counter++,2);
	CHECK_MEM;
	do(vater);
	up(vater->lo_p);
	ups(vater->hi_p);
   	return twin;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::redlight // Red.d.Soehne nach OKFDD_Levelexchange    //
/* ========================================================================= */
void dd_man::redlight(  utnode* opa)
{
   	utnode* vater=opa->lo_p;	
   
	// Typ 2 Reduktion
	// ---------------
	if (OKFDD_PI_DTL_Table[vater->label_i] == D_Shan)
	{
		if (vater->lo_p == vater->hi_p)
		{
			opa->lo_p 	= vater->lo_p;
			do(vater->lo_p);
			vater->idnum    = DUMMY;
			vater->link     = uc_last; 
		   	uc_last 	= vater;
		}
	   	else opa->lo_p = red_t1(vater);

	   	vater=opa->hi_p;
		if (vater->lo_p == vater->hi_p)
		{
			opa->hi_p 	= vater->lo_p;
			do(vater->lo_p);
			vater->idnum    = DUMMY;
			vater->link     = uc_last; 
		   	uc_last 	= vater;
		}
	   	else opa->hi_p = (red_t1(vater));
	}

	// Typ 3 Reduktion
	// ---------------
	else
	{
		if (vater->hi_p == OKFDD_ZERO)
		{
			opa->lo_p 	= vater->lo_p;
			vater->idnum    = DUMMY;
			vater->link     = uc_last; 
		   	uc_last 	= vater;
		}
	   	else opa->lo_p = red_t1(vater);
	   
	   	vater=opa->hi_p;
		if (vater->hi_p == OKFDD_ZERO)
		{
			opa->hi_p 	= vater->lo_p;
			vater->idnum    = DUMMY;
			vater->link     = uc_last; 
		   	uc_last 	= vater;
		}
	   	else opa->hi_p = (red_t1(vater));
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::red_t2 // Red.d.Soehne nach OKFDD_Levelexchange	     //
/* ========================================================================= */
utnode* dd_man::red_t2(utnode* vater)
{   
	// Typ 2 Reduktion
	// ---------------
       	if (vater->lo_p == vater->hi_p)
        {
       		do(vater->lo_p);
       		vater->idnum    = DUMMY;
       		vater->link     = uc_last; 
       	   	uc_last 	= vater;
       		return vater->lo_p;
       	}
       	else    return red_t1(vater);
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::red_t3 // Red.d.Soehne nach OKFDD_Levelexchange	     //
/* ========================================================================= */
utnode* dd_man::red_t3(utnode* vater)
{
	// Typ 3 Reduktion
	// ---------------
       	if (vater->hi_p == OKFDD_ZERO)
        {
       		vater->idnum    = DUMMY;
       		vater->link     = uc_last; 
       	   	uc_last 	= vater;
       		return  vater->lo_p;
       	}
       	else    return red_t1(vater);
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::red_t1 // Red.d.Soehne nach OKFDD_Levelexchange	     //
/* ========================================================================= */
utnode* dd_man::red_t1(utnode* vater)
{
    	// Typ 1 Reduktion (Kontrolle ob Hashposition bereits besetzt,
        // Kontolle der Kettenglieder)
       	// -----------------------------------------------------------
       	hashnr = ((m_and(vater->hi_p))->idnum + vater->lo_p->idnum) %
           ut_sizes[ut_hashsize[vater->label_i]];

       	// Neueintrag
       	// ----------
       	if (ut[vater->label_i][hashnr] == NULL)
       	{
	       	vater->next = NULL;
	       	ut[vater->label_i][hashnr] = vater;
	       	OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
	       	OKFDD_No_UTNodes++;
	       	return vater;
       	}
       	else
       	{
       		// Kette durchsuchen
       		// -----------------
       		hz = ut[vater->label_i][hashnr];
       		while (hz != NULL)
       		{
       			if (hz->lo_p != vater->lo_p || hz->hi_p != vater->hi_p)
       			{
       				hz = hz->next;
       				continue;
       			}

	       		// Doppelgaenger gefunden -> Typ 1
	       		// -------------------------------
	       		up(hz);
	       		do(hz->lo_p);
	       		dos(hz->hi_p);
	       		vater->idnum    = DUMMY;
	       		vater->link     = uc_last; uc_last = vater;
	       	   	return hz;
	       	}
	       	// neues Element in Liste einfuegen
	       	// --------------------------------
               	vater->next		     	= ut[vater->label_i][hashnr];
               	ut[vater->label_i][hashnr]      = vater;
	       	OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
	       	OKFDD_No_UTNodes++;
	       	return vater;
       	}
	
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::cut_off_ut // Ausschn.d.Soehne aus UT im unt.Level   //
/* ========================================================================= */
void dd_man::cut_off_ut(utnode* vater)
{
	// Neue Nummer vergeben, da sich Fkt aendert
        // -> Eintrag in ct wird ungueltig
	// -----------------------------------------
	vater->idnum = counter++;
	OKFDD_No_UTNodes_Per_Lvl[vater->label_i]--;
	OKFDD_No_UTNodes--;
	hashnr = ((m_and(vater->hi_p))->idnum + vater->lo_p->idnum) %
           ut_sizes[ut_hashsize[vater->label_i]];

	// vater ist erster oder einziger Knoten der Kette
	// -----------------------------------------------
	if (ut[vater->label_i][hashnr] == vater)
           ut[vater->label_i][hashnr] = vater->next;
	else
	{
		// vater in kette suchen und herausnehmen
		// --------------------------------------
		hz = ut[vater->label_i][hashnr];
		while (hz->next != NULL)
		{
		   if (hz->next != vater)  hz = hz->next;
		   else	{ hz->next = vater->next; vater->next = NULL; return; }
		}
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::mutation                                             //
// Tauschen d.inn.Enkel, Aufruf d.Reduktion f.Soehne, Eintr. d.Vaters in UT  //
/* ========================================================================= */
void dd_man::mutation(utnode* vater)
{
	usint   lln, lhn;
	uchar   marke;
	utnode* twin_hi_p;

	// Initialisierung und Wertzuweisung
	// ---------------------------------
	lln	    = vater->lo_p->label_i;
	twin_hi_p   = vater->hi_p;
	marke	    = m_sel(vater->hi_p);
	vater->hi_p = m_and(vater->hi_p);
	lhn	    = vater->hi_p->label_i;

	// linker Sohn befindet sich im unteren Level
	// ------------------------------------------
	if (lln == u_level)
	{
		// linker Sohn besitzt Referenzzaehler > 1
		// ---------------------------------------
		if (vater->lo_p->ref_c > 2) vater->lo_p=repro(vater->lo_p);
		else cut_off_ut(vater->lo_p);

		// rechter Sohn befindet sich auch im unteren Level
		// ------------------------------------------------
		if (lhn == u_level)
		{
			// rechter Sohn besitzt Referenzzaehler > 1
			// ----------------------------------------
			if (vater->hi_p->ref_c > 2)
                           vater->hi_p=repro(vater->hi_p);
			else cut_off_ut(vater->hi_p);

			// evlt. Marken runterziehen
			// -------------------------
		   	if (marke)
                        {      
			   if (OKFDD_PI_DTL_Table[vater->hi_p->label_i] ==
                           D_Shan) vater->hi_p->hi_p =
                              m_xor(vater->hi_p->hi_p,marke);
			   vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
			}
			   
			// Vertauschen der inneren Enkel
			// -----------------------------
			hz		= vater->lo_p->hi_p;
			vater->lo_p->hi_p = vater->hi_p->lo_p;
			vater->hi_p->lo_p = hz;

			// Vertauschen der Label
			// ---------------------
			vater->hi_p->label_i    = vater->label_i;
			vater->label_i	  = vater->lo_p->label_i;
			vater->lo_p->label_i    = vater->hi_p->label_i;
		}
		else
		{
			// rechten Sohn erzeugen, in Abhaengigkeit des DTs
			// -----------------------------------------------
			if (OKFDD_PI_DTL_Table[vater->lo_p->label_i] == D_Shan)
			{
			      gnus(hz,vater->label_i,twin_hi_p,
                              vater->lo_p->hi_p,counter++,2);
			      up(vater->hi_p);
			}
			else
                        {
                              gnus(hz,vater->label_i,OKFDD_ZERO,
                              vater->lo_p->hi_p,counter++,2);
                        }
			CHECK_MEM;

			// Label tauschen
			// --------------
			vater->label_i       = vater->lo_p->label_i;
			vater->lo_p->label_i = hz->label_i;

			// Zeiger aktualisieren
			// --------------------
			vater->lo_p->hi_p = twin_hi_p;
			vater->hi_p       = hz;
		}
	}
	else
	{
		// rechter Sohn besitzt Referenzzaehler > 1
		// ----------------------------------------
		if (vater->hi_p->ref_c > 2) vater->hi_p=repro(vater->hi_p);
		else cut_off_ut(vater->hi_p);

		// evlt. Marken runterziehen
		// -------------------------
		if (marke)
                {      
		   if (OKFDD_PI_DTL_Table[vater->hi_p->label_i] == D_Shan)
                      vater->hi_p->hi_p=m_xor(vater->hi_p->hi_p,marke);
		   vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
		}
		   
		// linken Sohn erzeugen, in Abhaengigkeit des Zerlegungstyps
		// ---------------------------------------------------------
		gnus(hz,vater->label_i,vater->hi_p->lo_p,
                   vater->lo_p,counter++,2);
		CHECK_MEM;
		if (OKFDD_PI_DTL_Table[vater->hi_p->label_i] == D_Shan)
		{
			up(vater->lo_p);
			vater->hi_p->lo_p = vater->lo_p;
		}
		else    vater->hi_p->lo_p = OKFDD_ZERO;

		// Label tauschen
		// --------------
		vater->label_i       = vater->hi_p->label_i;
		vater->hi_p->label_i = hz->label_i;

		// Zeiger aktualisieren
		// --------------------
		vater->lo_p = hz;
	}

	// Marken anpassen
	// ---------------
	marke = m_sel(vater->hi_p->lo_p);
   	if (marke)
        { 
	   if (OKFDD_PI_DTL_Table[vater->hi_p->label_i] == D_Shan)
              vater->hi_p->hi_p=m_xor(vater->hi_p->hi_p,marke);
	   vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
	}
	   
	// Reduktionsroutinen fuer Soehne aufrufen
	// ---------------------------------------
	redlight(vater);

	// Vater in ut eintragen
	// ---------------------
	hashnr = (vater->hi_p->idnum + vater->lo_p->idnum) %
           ut_sizes[ut_hashsize[vater->label_i]];
	vater->hi_p = m_xor(vater->hi_p,marke);
	vater->next = ut[vater->label_i][hashnr];
	ut[vater->label_i][hashnr] = vater;
	OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
	OKFDD_No_UTNodes++;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::mutation_DD                                          //
// Tauschen d.inn.Enkel, Aufruf d.Reduktion f.Soehne, Eintr. d.Vaters in UT  //
/* ========================================================================= */
void dd_man::mutation_DD(utnode* vater)
{
	usint   lln, lhn;
	uchar   marke;
	utnode* twin_hi_p;

	// Initialisierung und Wertzuweisung
	// ---------------------------------
	lln	    = vater->lo_p->label_i;
	twin_hi_p   = vater->hi_p;
	marke	    = m_sel(vater->hi_p);
	vater->hi_p = m_and(vater->hi_p);
	lhn	    = vater->hi_p->label_i;

	// linker Sohn befindet sich im unteren Level
	// ------------------------------------------
	if (lln == u_level)
	{
		// linker Sohn besitzt Referenzzaehler > 1
		// ---------------------------------------
		if (vater->lo_p->ref_c > 2) vater->lo_p=repro(vater->lo_p);
		else cut_off_ut(vater->lo_p);

		// rechter Sohn befindet sich auch im unteren Level
		// ------------------------------------------------
		if (lhn == u_level)
		{
			// rechter Sohn besitzt Referenzzaehler > 1
			// ----------------------------------------
			if (vater->hi_p->ref_c > 2)
                           vater->hi_p=repro(vater->hi_p);
			else cut_off_ut(vater->hi_p);

			// evlt. Marken runterziehen
			// -------------------------
		   	vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
			   
			// Vertauschen der inneren Enkel
			// -----------------------------
			hz		= vater->lo_p->hi_p;
			vater->lo_p->hi_p = vater->hi_p->lo_p;
			vater->hi_p->lo_p = hz;

			// Vertauschen der Label
			// ---------------------
			vater->hi_p->label_i = vater->label_i;
			vater->lo_p->label_i = vater->label_i;
			vater->label_i	     = lln;
		}
		else
		{
	       		// Label tauschen
	       		// --------------
			vater->lo_p->label_i = vater->label_i;
			vater->label_i       = lln;

			// Zeiger aktualisieren
			// --------------------
			vater->hi_p       = vater->lo_p->hi_p;
			vater->lo_p->hi_p = twin_hi_p;
			vater->lo_p 	  = red_t1(vater->lo_p);
			   
			// Vater in ut eintragen
			// ---------------------
			hashnr = ((m_and(vater->hi_p))->idnum +
                           vater->lo_p->idnum) % ut_sizes[ut_hashsize[
                           vater->label_i]];
			vater->next = ut[vater->label_i][hashnr];
			ut[vater->label_i][hashnr] = vater;
			OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
			OKFDD_No_UTNodes++;
			return;
		}
	}
	else
	{
		// rechter Sohn besitzt Referenzzaehler > 1
		// ----------------------------------------
		if (vater->hi_p->ref_c > 2) vater->hi_p=repro(vater->hi_p);
		else cut_off_ut(vater->hi_p);
	   
		// evlt. Marken runterziehen
		// -------------------------
		vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
	   
	   	if (vater->hi_p->lo_p==OKFDD_ZERO) {
		   
	       		// Label tauschen
	       		// --------------
			vater->hi_p->label_i = vater->label_i;
			vater->label_i       = lhn;

			// Zeiger aktualisieren
			// --------------------
		   	vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
			vater->hi_p 	  = m_xor(red_t1(vater->hi_p),marke);
			   
			// Vater in ut eintragen
			// ---------------------
			hashnr = ((m_and(vater->hi_p))->idnum +
                           vater->lo_p->idnum) %
                           ut_sizes[ut_hashsize[vater->label_i]];
			vater->next = ut[vater->label_i][hashnr];
			ut[vater->label_i][hashnr] = vater;
			OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
			OKFDD_No_UTNodes++;
			return;
		}
	   
       		// linken Sohn erzeugen, in Abhaengigkeit des Zerlegungstyps
       		// ---------------------------------------------------------
       		gnus(hz,vater->label_i,vater->hi_p->lo_p,vater->lo_p,
                   counter++,2);
       		CHECK_MEM;
       		vater->hi_p->lo_p = OKFDD_ZERO;
	
       		// Label tauschen
       		// --------------
       		vater->hi_p->label_i = vater->label_i;
       		vater->label_i       = lhn;

       		// Zeiger aktualisieren
       		// --------------------
       		vater->lo_p = hz;
	}

	// Marken anpassen
	// ---------------
	marke = m_sel(vater->hi_p->lo_p);
	vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
	   
	// Reduktionsroutinen fuer Soehne aufrufen
	// ---------------------------------------
	vater->lo_p = red_t3(vater->lo_p);
	vater->hi_p = m_xor(red_t3(vater->hi_p),marke);

	// Vater in ut eintragen
	// ---------------------
	hashnr = ((m_and(vater->hi_p))->idnum + vater->lo_p->idnum) %
           ut_sizes[ut_hashsize[vater->label_i]];
	vater->next = ut[vater->label_i][hashnr];
	ut[vater->label_i][hashnr] = vater;
	OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
	OKFDD_No_UTNodes++;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::mutation_DS                                          //
// Tauschen d.inn.Enkel, Aufruf d.Reduktion f.Soehne, Eintr. d.Vaters in UT  //
/* ========================================================================= */
void dd_man::mutation_DS(utnode* vater)
{
	usint   lln, lhn;
	uchar   marke;
	utnode* twin_hi_p;
   
	// Initialisierung und Wertzuweisung
	// ---------------------------------
	lln	    = vater->lo_p->label_i;
	twin_hi_p   = vater->hi_p;
	marke	    = m_sel(vater->hi_p);
	vater->hi_p = m_and(vater->hi_p);
	lhn	    = vater->hi_p->label_i;

	// linker Sohn befindet sich im unteren Level
	// ------------------------------------------
	if (lln == u_level)
	{
		// linker Sohn besitzt Referenzzaehler > 1
		// ---------------------------------------
		if (vater->lo_p->ref_c > 2) vater->lo_p=repro(vater->lo_p);
		else cut_off_ut(vater->lo_p);

		// rechter Sohn befindet sich auch im unteren Level
		// ------------------------------------------------
		if (lhn == u_level)
		{
			// rechter Sohn besitzt Referenzzaehler > 1
			// ----------------------------------------
			if (vater->hi_p->ref_c > 2)
                           vater->hi_p=repro(vater->hi_p);
			else cut_off_ut(vater->hi_p);

			// evlt. Marken runterziehen
			// -------------------------
		   	if (marke)
                        {      
			   vater->hi_p->hi_p = m_xor(vater->hi_p->hi_p,1);
			   vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,1);
			}
			   
			// Vertauschen der inneren Enkel
			// -----------------------------
			hz		  = vater->lo_p->hi_p;
			vater->lo_p->hi_p = vater->hi_p->lo_p;
			vater->hi_p->lo_p = hz;

			// Vertauschen der Label
			// ---------------------
			vater->lo_p->label_i = vater->label_i;
			vater->hi_p->label_i = vater->label_i;
			vater->label_i	     = lln;
		}
		else
		{
			// rechten Sohn erzeugen, in Abhaengigkeit des DTs
			// -----------------------------------------------
		        gnus(hz,vater->label_i,twin_hi_p,vater->lo_p->hi_p,
                           counter++,2);
			up(vater->hi_p);
			CHECK_MEM;

			// Label tauschen
			// --------------
			vater->lo_p->label_i = vater->label_i;
			vater->label_i       = lln;

			// Zeiger aktualisieren
			// --------------------
			vater->lo_p->hi_p = twin_hi_p;
			vater->hi_p       = hz;
		}
	}
	else
	{
		// rechter Sohn besitzt Referenzzaehler > 1
		// ----------------------------------------
		if (vater->hi_p->ref_c > 2) vater->hi_p=repro(vater->hi_p);
		else cut_off_ut(vater->hi_p);

		// evlt. Marken runterziehen
		// -------------------------
		if (marke)  {      
			vater->hi_p->hi_p = m_xor(vater->hi_p->hi_p,1);
			vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,1);
		}
	   
	   	if (vater->hi_p->lo_p==OKFDD_ZERO) {
		   		      
	       		// Label tauschen
	       		// --------------
			vater->hi_p->label_i = vater->label_i;
			vater->label_i       = lhn;

			// Zeiger aktualisieren
			// --------------------
			vater->hi_p->lo_p = vater->lo_p;
			vater->hi_p	  = red_t1(vater->hi_p);
	       		up(vater->lo_p);
			   
			// Vater in ut eintragen
			// ---------------------
			hashnr = (vater->hi_p->idnum + vater->lo_p->idnum) %
                           ut_sizes[ut_hashsize[vater->label_i]];
			vater->next = ut[vater->label_i][hashnr];
			ut[vater->label_i][hashnr] = vater;
			OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
			OKFDD_No_UTNodes++;
			return;
		}
	   
	   	if (vater->hi_p->hi_p==OKFDD_ZERO)
                {	   		   
	       		// Label tauschen
	       		// --------------
			vater->hi_p->label_i = vater->label_i;
			vater->label_i       = lhn;

			// Zeiger aktualisieren
			// --------------------
			vater->hi_p->hi_p = vater->hi_p->lo_p;
			vater->hi_p->lo_p = vater->lo_p;
			vater->lo_p 	  = red_t1(vater->hi_p);
			vater->hi_p   	  = vater->lo_p->lo_p;
	       		up(vater->hi_p);
			   
			// Vater in ut eintragen
			// ---------------------
			hashnr = (vater->hi_p->idnum + vater->lo_p->idnum) %
                           ut_sizes[ut_hashsize[vater->label_i]];
			vater->next = ut[vater->label_i][hashnr];
			ut[vater->label_i][hashnr] = vater;
			OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
			OKFDD_No_UTNodes++;
			return;
		}
		   
		// linken Sohn erzeugen, in Abhaengigkeit des Zerlegungstyps
		// ---------------------------------------------------------
		gnus(hz,vater->label_i,vater->hi_p->lo_p,vater->lo_p,
                   counter++,2);
		CHECK_MEM;
		up(vater->lo_p);
		vater->hi_p->lo_p = vater->lo_p;

		// Label tauschen
		// --------------
		vater->hi_p->label_i = vater->label_i;
		vater->label_i       = lhn;

		// Zeiger aktualisieren
		// --------------------
		vater->lo_p = hz;
	}

	// Marken anpassen
	// ---------------
	marke = m_sel(vater->hi_p->lo_p);
	vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
	   
	// Reduktionsroutinen fuer Soehne aufrufen
	// ---------------------------------------
	vater->lo_p = red_t3(vater->lo_p);
	vater->hi_p = m_xor(red_t3(vater->hi_p),marke);

	// Vater in ut eintragen
	// ---------------------
	hashnr = ((m_and(vater->hi_p))->idnum + vater->lo_p->idnum) %
           ut_sizes[ut_hashsize[vater->label_i]];
	vater->next = ut[vater->label_i][hashnr];
	ut[vater->label_i][hashnr] = vater;
	OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
	OKFDD_No_UTNodes++;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::mutation_SD	                                     //
// Tauschen d.inn.Enkel, Aufruf d.Reduk. f.Soehne, Eintragen d.Vaters in UT  //
/* ========================================================================= */
void dd_man::mutation_SD(utnode* vater)
{
	usint   lln, lhn;
	uchar   marke;
	utnode* twin_hi_p;

	// Initialisierung und Wertzuweisung
	// ---------------------------------
	lln	    = vater->lo_p->label_i;
	twin_hi_p   = vater->hi_p;
	marke	    = m_sel(vater->hi_p);
	vater->hi_p = m_and(vater->hi_p);
	lhn	    = vater->hi_p->label_i;

	// linker Sohn befindet sich im unteren Level
	// ------------------------------------------
	if (lln == u_level)
	{
		// linker Sohn besitzt Referenzzaehler > 1
		// ---------------------------------------
		if (vater->lo_p->ref_c > 2) vater->lo_p=repro(vater->lo_p);
		else cut_off_ut(vater->lo_p);

		// rechter Sohn befindet sich auch im unteren Level
		// ------------------------------------------------
		if (lhn == u_level)
		{
			// rechter Sohn besitzt Referenzzaehler > 1
			// ----------------------------------------
			if (vater->hi_p->ref_c > 2)
                          vater->hi_p=repro(vater->hi_p);
			else cut_off_ut(vater->hi_p);

			// evlt. Marken runterziehen
			// -------------------------
			vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
			   
			// Vertauschen der inneren Enkel
			// -----------------------------
			hz		= vater->lo_p->hi_p;
			vater->lo_p->hi_p = vater->hi_p->lo_p;
			vater->hi_p->lo_p = hz;

			// Vertauschen der Label
			// ---------------------
			vater->lo_p->label_i    = vater->label_i;
			vater->hi_p->label_i    = vater->label_i;
			vater->label_i	  = lln;
		}
		else
		{
		   	if (twin_hi_p==vater->lo_p->lo_p)
                        {
		       	   // Label tauschen
	       		   // --------------
			   vater->lo_p->label_i = vater->label_i;
			   vater->label_i       = lln;

			   // Zeiger aktualisieren
			   // --------------------
			   vater->lo_p->lo_p    = vater->lo_p->hi_p;
			   vater->lo_p->hi_p    = OKFDD_ZERO;
			   marke = m_sel(vater->lo_p->lo_p);
   			   if (marke)
                           { 
			      vater->lo_p->hi_p = m_xor(vater->lo_p->hi_p,1);
			      vater->lo_p->lo_p = m_xor(vater->lo_p->lo_p,1);
			   }
			   vater->hi_p = m_xor(red_t1(vater->lo_p),marke);
			   vater->lo_p = twin_hi_p;
			   do(vater->lo_p);
			   
			   // Vater in ut eintragen
			   // ---------------------
			   hashnr = ((m_and(vater->hi_p))->idnum +
                                    vater->lo_p->idnum) %
                                    ut_sizes[ut_hashsize[vater->label_i]];
			   vater->next = ut[vater->label_i][hashnr];
			   ut[vater->label_i][hashnr] = vater;
			   OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
			   OKFDD_No_UTNodes++;
			   return;
			}
		   
			// Rechten Sohn erzeugen, in Abhg.keit des DTs
			// -------------------------------------------
			gnus(hz,vater->label_i,OKFDD_ZERO,vater->lo_p->hi_p,
                          counter++,2); 
			CHECK_MEM;

			// Label tauschen
			// --------------
			vater->lo_p->label_i = vater->label_i;
			vater->label_i       = lln;

			// Zeiger aktualisieren
			// --------------------
			vater->lo_p->hi_p = twin_hi_p;
			vater->hi_p       = hz;
		}
	}
	else
	{
		// rechter Sohn besitzt Referenzzaehler > 1
		// ----------------------------------------
		if (vater->hi_p->ref_c > 2) vater->hi_p=repro(vater->hi_p);
		else cut_off_ut(vater->hi_p);
	   
		// evlt. Marken runterziehen
		// -------------------------
		vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,marke);
	   
	   	if (vater->hi_p->lo_p==vater->lo_p) {
		   		   
	       		// Label tauschen
	       		// --------------
			vater->hi_p->label_i = vater->label_i;
			vater->label_i       = lhn;

			// Zeiger aktualisieren
			// --------------------
			vater->hi_p->lo_p = OKFDD_ONE;
			vater->hi_p->hi_p = m_xor(vater->hi_p->hi_p,1);
			vater->hi_p	  = m_xor(red_t1(vater->hi_p),1);
		   	do(vater->lo_p);
			   
			// Vater in ut eintragen
			// ---------------------
			hashnr = ((m_and(vater->hi_p))->idnum +
                                 vater->lo_p->idnum) %
                                 ut_sizes[ut_hashsize[vater->label_i]];
			vater->next = ut[vater->label_i][hashnr];
			ut[vater->label_i][hashnr] = vater;
			OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
			OKFDD_No_UTNodes++;
			return;
		}
		   
		// Linken Sohn erzeugen, in Abhg.keit des DTs
		// ------------------------------------------
		gnus(hz,vater->label_i,vater->hi_p->lo_p,vater->lo_p,counter++,
                  2);
		CHECK_MEM;
		vater->hi_p->lo_p = OKFDD_ZERO;

		// Label tauschen
		// --------------
		vater->hi_p->label_i = vater->label_i;
		vater->label_i       = lhn;

		// Zeiger aktualisieren
		// --------------------
		vater->lo_p = hz;
	}

	// Marken anpassen
	// ---------------
	marke = m_sel(vater->hi_p->lo_p);
   	if (marke)
        { 
	   vater->hi_p->hi_p = m_xor(vater->hi_p->hi_p,1);
	   vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,1);
	}
	   
	// Reduktionsroutinen fuer Soehne aufrufen
	// ---------------------------------------
	vater->lo_p = red_t2(vater->lo_p);
	vater->hi_p = m_xor(red_t2(vater->hi_p),marke);

	// Vater in ut eintragen
	// ---------------------
	hashnr = ((m_and(vater->hi_p))->idnum + vater->lo_p->idnum) %
                 ut_sizes[ut_hashsize[vater->label_i]];
	vater->next = ut[vater->label_i][hashnr];
	ut[vater->label_i][hashnr] = vater;
	OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
	OKFDD_No_UTNodes++;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::mutation_SS	                                     //
// Tauschen d.inn.Enkel, Aufruf d.Reduk.f.Soehne, Eintragen d.Vaters in UT   //
/* ========================================================================= */
void dd_man::mutation_SS(utnode* vater)
{

	usint   lln, lhn;
	uchar   marke;
	utnode* twin_hi_p;

	// Initialisierung und Wertzuweisung
	// ---------------------------------
	lln	    = vater->lo_p->label_i;
	twin_hi_p   = vater->hi_p;
	marke	    = m_sel(vater->hi_p);
	vater->hi_p = m_and(vater->hi_p);
	lhn	    = vater->hi_p->label_i;

	// linker Sohn befindet sich im unteren Level
	// ------------------------------------------
	if (lln == u_level)
	{
		// linker Sohn besitzt Referenzzaehler > 1
		// ---------------------------------------
		if (vater->lo_p->ref_c > 2) vater->lo_p=repro(vater->lo_p);
		else cut_off_ut(vater->lo_p);

		// rechter Sohn befindet sich auch im unteren Level
		// ------------------------------------------------
		if (lhn == u_level)
		{
			// rechter Sohn besitzt Referenzzaehler > 1
			// ----------------------------------------
			if (vater->hi_p->ref_c > 2)
                          vater->hi_p=repro(vater->hi_p);
			else cut_off_ut(vater->hi_p);

			// evlt. Marken runterziehen
			// -------------------------
		   	if (marke)
                        {      
			   vater->hi_p->hi_p = m_xor(vater->hi_p->hi_p,1);
			   vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,1);
			}
			   
			// Vertauschen der inneren Enkel
			// -----------------------------
			hz		  = vater->lo_p->hi_p;
			vater->lo_p->hi_p = vater->hi_p->lo_p;
			vater->hi_p->lo_p = hz;

			// Vertauschen der Label
			// ---------------------
			vater->lo_p->label_i = vater->label_i;
			vater->hi_p->label_i = vater->label_i;
			vater->label_i	     = lln;
		}
		else
		{
		   	if (twin_hi_p==vater->lo_p->hi_p)
                        {   		   
		       	   // Label tauschen
	       		   // --------------
			   vater->lo_p->label_i = vater->label_i;
			   vater->label_i       = lln;

			   // Zeiger aktualisieren
			   // --------------------
			   vater->hi_p = twin_hi_p;
			   vater->lo_p = red_t1(vater->lo_p);
			   
			   // Vater in ut eintragen
			   // ---------------------
			   hashnr = ((m_and(vater->hi_p))->idnum +
                                    vater->lo_p->idnum) %
                                    ut_sizes[ut_hashsize[vater->label_i]];
			   vater->next = ut[vater->label_i][hashnr];
			   ut[vater->label_i][hashnr] = vater;
			   OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
			   OKFDD_No_UTNodes++;
			   return;
			}
		   	if (twin_hi_p==vater->lo_p->lo_p)
                        {	   		   
		       	   // Label tauschen
	       		   // --------------
			   vater->lo_p->label_i = vater->label_i;
			   vater->label_i       = lln;

			   // Zeiger aktualisieren
			   // --------------------
			   vater->lo_p->lo_p = vater->lo_p->hi_p;
			   vater->lo_p->hi_p = twin_hi_p;
			   marke = m_sel(vater->lo_p->lo_p);
   			   if (marke)
                           { 
			      vater->lo_p->hi_p = m_xor(vater->lo_p->hi_p,1);
			      vater->lo_p->lo_p = m_xor(vater->lo_p->lo_p,1);
			   }
			   vater->hi_p = m_xor(red_t1(vater->lo_p),marke);
			   vater->lo_p = twin_hi_p;
			   
			   // Vater in ut eintragen
			   // ---------------------
			   hashnr = ((m_and(vater->hi_p))->idnum +
                                    vater->lo_p->idnum) %
                                    ut_sizes[ut_hashsize[vater->label_i]];
			   vater->next = ut[vater->label_i][hashnr];
			   ut[vater->label_i][hashnr] = vater;
			   OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
			   OKFDD_No_UTNodes++;
			   return;
			}
		   
			// Rechten Sohn erzeugen, in Abhaengigkeit des DTs
			// -----------------------------------------------
			gnus(hz,vater->label_i,twin_hi_p,vater->lo_p->hi_p,
                          counter++,2);
			up(vater->hi_p);
			CHECK_MEM;

			// Label tauschen
			// --------------
			vater->lo_p->label_i = vater->label_i;
			vater->label_i       = lln;

			// Zeiger aktualisieren
			// --------------------
			vater->lo_p->hi_p = twin_hi_p;
			vater->hi_p       = hz;
		}
	}
	else
	{
		// rechter Sohn besitzt Referenzzaehler > 1
		// ----------------------------------------
		if (vater->hi_p->ref_c > 2) vater->hi_p=repro(vater->hi_p);
		else cut_off_ut(vater->hi_p);

		// evlt. Marken runterziehen
		// -------------------------
		if (marke)
                {      
		   vater->hi_p->hi_p = m_xor(vater->hi_p->hi_p,1);
		   vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,1);
		}
	   
	   	if (vater->hi_p->lo_p==vater->lo_p)
                {	   		   
	       	   // Label tauschen
	       	   // --------------
		   vater->hi_p->label_i = vater->label_i;
		   vater->label_i       = lhn;

		   // Zeiger aktualisieren
		   // --------------------
		   vater->hi_p = red_t1(vater->hi_p);
			   
		   // Vater in ut eintragen
		   // ---------------------
		   hashnr = (vater->hi_p->idnum + vater->lo_p->idnum) %
                            ut_sizes[ut_hashsize[vater->label_i]];
		   vater->next = ut[vater->label_i][hashnr];
		   ut[vater->label_i][hashnr] = vater;
		   OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
		   OKFDD_No_UTNodes++;
		   return;
		}
	   	if (vater->hi_p->hi_p==vater->lo_p)
                {	   		   
	       	   // Label tauschen
	       	   // --------------
		   vater->hi_p->label_i = vater->label_i;
		   vater->label_i       = lhn;

		   // Zeiger aktualisieren
		   // --------------------
		   vater->hi_p->hi_p = vater->hi_p->lo_p;
		   vater->hi_p->lo_p = vater->lo_p;
		   vater->lo_p 	     = red_t1(vater->hi_p);
		   vater->hi_p       = vater->lo_p->lo_p;
			   
		   // Vater in ut eintragen
		   // ---------------------
		   hashnr = (vater->hi_p->idnum + vater->lo_p->idnum) %
                            ut_sizes[ut_hashsize[vater->label_i]];
		   vater->next = ut[vater->label_i][hashnr];
		   ut[vater->label_i][hashnr] = vater;
		   OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
		   OKFDD_No_UTNodes++;
		   return;
		}
		   
		// Linken Sohn erzeugen, in Abhaengigkeit des DTs
		// ----------------------------------------------
		gnus(hz,vater->label_i,vater->hi_p->lo_p,vater->lo_p,counter++,
                   2);
		CHECK_MEM;
		up(vater->lo_p);
		vater->hi_p->lo_p = vater->lo_p;
	   
		// Label tauschen
		// --------------
		vater->hi_p->label_i = vater->label_i;
		vater->label_i       = lhn;

		// Zeiger aktualisieren
		// --------------------
		vater->lo_p = hz;
	}

	// Marken anpassen
	// ---------------
	marke = m_sel(vater->hi_p->lo_p);
   	if (marke)
        { 
	   vater->hi_p->hi_p = m_xor(vater->hi_p->hi_p,1);
	   vater->hi_p->lo_p = m_xor(vater->hi_p->lo_p,1);
	}
	   
	// Reduktionsroutinen fuer Soehne aufrufen
	// ---------------------------------------
	vater->lo_p = red_t2(vater->lo_p);
	vater->hi_p = m_xor(red_t2(vater->hi_p),marke);

	// Vater in ut eintragen
	// ---------------------
	hashnr = ((m_and(vater->hi_p))->idnum + vater->lo_p->idnum) %
                 ut_sizes[ut_hashsize[vater->label_i]];
	vater->next = ut[vater->label_i][hashnr];
	ut[vater->label_i][hashnr] = vater;
	OKFDD_No_UTNodes_Per_Lvl[vater->label_i]++;
	OKFDD_No_UTNodes++;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::gesamtausgabe // Ausgabe aller Groessen- u.DTL-Infos //
/* ========================================================================= */
void dd_man::gesamtausgabe()
{
        short i;
        uchar limiter;

	if (min_size_i < OKFDD_Now_size_i)
	{
	   cout << "\nMinimum was ...\n";

	   limiter = 0;

	   for (i = 0; i < OKFDD_Maxidx; i++)
	   {
	      if (limiter == 10) { cout << "\n"; limiter = 0; }
	      cout.width(5); cout.fill(' ');
	      cout << min_pi_table[i];
	      cout << " " << dtl[mindtl[min_pi_table[i]]];
	      cout << " ";
	      limiter++;
	   }

	   cout << "\nMinimal size " << min_size_i << " occured ";
           cout << min_count << " times\n";
	   cout << "Actual settings are ...";

	}
	cout <<"\n";

	limiter = 0;

	for (i = 0; i < OKFDD_Maxidx; i++)
	{
	   if (limiter == 10) { cout << "\n"; limiter = 0; }
	   cout.width(5); cout.fill(' ');
	   cout << OKFDD_PI_Order_Table[i];
	   cout << " " << dtl[OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]]];
	   cout << " ";
	   limiter++;
	}
	cout << "\nActual size is " << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::kosten                                               //
// Vergl., ob mom.Kosten minimaler / evtl.saven d.Reihenf.o.bei = zaehlen    //
/* ========================================================================= */
void dd_man::kosten(usint von,
		    usint bis)
{
	int i;

	if (min_size_i >= OKFDD_Now_size_i)
	{
	   if (OKFDD_Outputflags & dumpflag)
	   {
	      for (i = 0; i < OKFDD_Maxidx; i++)
               cout << dtl[OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]]] << " ";
              cout << "\n";
	      for (i = 0; i < OKFDD_Maxidx; i++)
               cout << OKFDD_PI_Order_Table[i] << " ";
              cout << "=> " << OKFDD_Now_size_i << "\n";
	   }
	   for (i = von; i <= bis; i++)
            min_pi_table[i] = OKFDD_PI_Order_Table[i];

	   if (min_size_i == OKFDD_Now_size_i) min_count++;
	   else { min_count = 1; min_size_i = OKFDD_Now_size_i; }
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::gesamtkosten                                         //
// Vergl., ob mom.Kosten minimaler / evtl.saven d.Reihenf.u.DTL o.bei = zae. //
/* ========================================================================= */
void dd_man::gesamtkosten(usint von,
			  usint bis)
{
	int i;

	if (min_size_i >= OKFDD_Now_size_i)
	{
	   if (OKFDD_Outputflags & dumpflag)
	   {
	      for (i = 0; i < OKFDD_Maxidx; i++)
               cout << dtl[OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]]] << " ";
              cout << "\n";
	      for (i = 0; i < OKFDD_Maxidx; i++)
               cout << OKFDD_PI_Order_Table[i] << " ";
              cout << "=> " << OKFDD_Now_size_i << "\n";
	   }
	   for (i = von; i <= bis; i++)
	   {
	      min_pi_table[i] = OKFDD_PI_Order_Table[i];
	      mindtl[OKFDD_PI_Order_Table[i]] =
               OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]];
	   }

	   if (min_size_i == OKFDD_Now_size_i) min_count++;
	   else { min_count = 1; min_size_i = OKFDD_Now_size_i; }
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::gesamtkosten                                         //
// Vergl., ob mom.Kosten minimaler / evtl.saven d.Reihenf.u.DTL o.bei = zae. //
/* ========================================================================= */
void dd_man::gesamtkosten2(usint von,
			   usint bis)
{
	int i;

	if (min_size_i > OKFDD_Now_size_i)
	{
	   if (OKFDD_Outputflags & dumpflag)
	   {
	      for (i = 0; i < OKFDD_Maxidx; i++)
               cout << dtl[OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]]] << " ";
              cout << "\n";
	      for (i = 0; i < OKFDD_Maxidx; i++)
               cout << OKFDD_PI_Order_Table[i] << " ";
              cout << "=> " << OKFDD_Now_size_i << "\n";
	   }
	   for (i = von; i <= bis; i++)
	   {
	      min_pi_table[i] = OKFDD_PI_Order_Table[i];
	      mindtl[OKFDD_PI_Order_Table[i]] =
               OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]];
	   }

	   if (min_size_i == OKFDD_Now_size_i) min_count++;
	   else { min_count = 1; min_size_i = OKFDD_Now_size_i; }
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_UT_Statistic // Unique-table-Statistiken	     //
/* ========================================================================= */
void dd_man::OKFDD_UT_Statistic()
{
	int     var;
	int     i;
	int     kiut;

	int     ketten  = 0;
	int     knoten  = 0;
	int     anker   = 0;
	float   load1   = 0.0;
	float   load2   = 0.0;

	OKFDD_Maxidx = 0;
        while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0) OKFDD_Maxidx++;

	cout << "\nUnique Table Statistic:\n\n";

	for (var = 0; var < OKFDD_Maxidx; var++)
	{
	   anker   += ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[var]]];
	   knoten  += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]];

	   kiut    =  0;

	   for (i=0; i<ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[var]]];i++)
	   {
	      if (ut[OKFDD_PI_Order_Table[var]][i] != NULL) kiut++;
	   }

	   ketten  += kiut;

	   cout << "UT[";
	   cout.width(5); cout.fill('0');
           cout << OKFDD_PI_Order_Table[var] << "][";
	   cout.width(5); cout.fill('.');
           cout << ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[var]]];
           cout << "] #No ";
	   cout.width(6); cout.fill(' ');
           cout << OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]];
           cout << " #Ch ";
	   cout.width(5); cout.fill(' '); cout << kiut << " with avg ";
	   cout.width(6); cout.fill(' '); cout.precision(3);
           cout << (float)OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]]/
            kiut << " ";
	   cout.width(6); cout.fill('0'); cout.precision(3);
           cout << "Load [v/r] ";
	   cout.width(6); cout.fill('0'); cout.precision(3);
           cout << (float)OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]]/
            ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[var]]] << " / ";
	   cout.width(6); cout.fill('0'); cout.precision(3);
           cout << (float)kiut/
            ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[var]]] << "\n";
	   load1 += (float)OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]]/
            ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[var]]];
	   load2 += (float)kiut/
            ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[var]]];
	}

	cout << "\nTotal values are ...\n\n";
	cout.width(6); cout.fill(' ');
        cout << "Supported vars	" << OKFDD_Maxidx <<"\n";
	cout.width(6); cout.fill(' ');
        cout << "Number of anchors     " << anker  <<"\n";
	cout.width(6); cout.fill(' ');
        cout << "Number of ut_nodes    " << knoten <<"\n";
	cout.width(6); cout.fill(' ');
        cout << "Number of chains      " << ketten <<"\n";
	cout.width(6); cout.fill(' '); cout.precision(3);
        cout << "Average chain length  " << (float)knoten/ketten <<"\n";
	cout.width(6); cout.fill(' '); cout.precision(3);
        cout << "Load factor (virtual) " << load1/OKFDD_Maxidx <<"\n";
	cout.width(6); cout.fill(' '); cout.precision(3);
        cout << "Load factor (real)    " << load2/OKFDD_Maxidx <<"\n\n";
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::level_stat	                                     //
// Levelstatistik (potentielle Reduktionsfaelle beim Leveltausch)	     //
/* ========================================================================= */
void dd_man::levelstat()
{
	int     var, i;
	utnode* vater;

/*      for (var = 0; var < OKFDD_Maxidx; var++)
          lsf[OKFDD_PI_Order_Table[var]] =
          OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]];
*/
	for (var = 0; var < OKFDD_Maxidx; var++)
          lsf[OKFDD_PI_Order_Table[var]] = 0;

	for (var = 0; var < OKFDD_Maxidx; var++)
        {
	for (i = 0; i < ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[var]]];i++)
        {
	if (ut[OKFDD_PI_Order_Table[var]][i] != NULL)
	{
	   vater = ut[OKFDD_PI_Order_Table[var]][i];
	   while (vater!=NULL)
	   {
	/*
	      if (vater->lo_p->ref_c > 2)
	      {
		 lsf[vater->lo_p->label_i]--;
		 lsf[vater->label_i]--;
	      }
	      if ((m_and(vater->hi_p))->ref_c > 2)
	      {
		 lsf[vater->lo_p->label_i]--;
		 lsf[vater->label_i]--;
	      }
	*/
	      if (vater->lo_p->label_i == (m_and(vater->hi_p))->label_i)
	      {
		 if (OKFDD_PI_DTL_Table[vater->label_i] == D_Shan)
                 {
		 if (vater->hi_p != OKFDD_ONE && vater->lo_p != OKFDD_ONE)
                 {
		    if ( (m_xor((m_and(vater->hi_p))->lo_p,
                       m_sel(vater->hi_p)) == vater->lo_p->lo_p) ||
		       (m_xor((m_and(vater->hi_p))->hi_p,m_sel(vater->hi_p)) 
                       == vater->lo_p->hi_p)  )
		    {
		       lsf[vater->label_i]++;
		       lsf[vater->lo_p->label_i]++;
		    }
                 }
                 }
	         else
	         {
		    if (vater->hi_p != OKFDD_ONE)
		    {
		       if (m_xor((m_and(vater->hi_p))->lo_p,m_sel(vater->hi_p))
                          == OKFDD_ZERO)
		       {
		          lsf[vater->lo_p->label_i]++;
		          lsf[vater->label_i]++;
		       }
		    }
	         }
	      }
/*
	      else 
              {
		 lsf[vater->lo_p->label_i]--;
		 lsf[vater->label_i]--;
	      }
*/
	      vater = vater->next;
	   }
	}}}

/*      cout <<(int)OKFDD_Outputflags<<" "<<(int)dumpflag<<" ";
        cout<<(int)(OKFDD_Outputflags && dumpflag)<< "\n";
*/
	if (OKFDD_Outputflags & ros_y)
        {
	   int summe = 0;
	   for (var = 0; var < OKFDD_Maxidx; var++)
	   {
	      if (lsf[OKFDD_PI_Order_Table[var]])
	      {
		 summe += lsf[OKFDD_PI_Order_Table[var]];
                 cout << (int) lsf[OKFDD_PI_Order_Table[var]] << " ";
	      }
	      else cout << ". ";
	   }
	   cout << " => " << summe << "\n";
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::bbr			                             //
 // Bestimmung (keine Transformation) der besten Reihenfolge fuer geg. DTL   //
/* ========================================================================= */
void dd_man::bbr(usint von,
		 usint bis)
{
	int sift, old_min_count, old_min_size_i;

	// best. der besten Reihenfolge
	// ----------------------------
	old_min_count  = min_count;
	old_min_size_i = min_size_i;
	l	       = von;

	for (sift = bis; sift >= von; sift--) aa[sift] = 0;

	permut(von+1,von,bis);
	OKFDD_Levelexchange(von);
	if (old_min_count != min_count || old_min_size_i != min_size_i)
	{
	   for (sift = 0; sift < OKFDD_Maxidx; sift++)
             mindtl[OKFDD_PI_Order_Table[sift]] =
               OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[sift]];
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::kwk // Routine zur Ueberprfg.erfuellender Beleg.     //
/* ========================================================================= */
void dd_man::kwk(utnode* vater)
{
	// Falls Shannonzerlegung vorliegt
	// -------------------------------
	if (OKFDD_PI_DTL_Table[vater->label_i] == D_Shan)
	{
	   // Verzweige in LOW- und HIGH-Seite gleichermassen,
           // Terminale XOR verknuepfen
	   // ------------------------------------------------
	   if (pfads[pi_level[vater->label_i]] == 0)
	   {
	      pfada[pi_level[vater->label_i]] = 0;
	      if(vater->lo_p != NULL) kwk(vater->lo_p);
	      else if (vater == OKFDD_ONE) erg^=1;
	   }
	   else
	   {
	      pfada[pi_level[vater->label_i]] = 1;
	      if (m_and(vater->hi_p) != NULL)
	      {
		 erg^=m_sel(vater->hi_p);
		 kwk(m_and(vater->hi_p));
	      }
	      else if (vater == OKFDD_ONE) erg^=1;
	   }
	}
	else
	{
	   // Bei Davio-Zerlegung komponentenweise kleinere Terme beachten
	   // ------------------------------------------------------------
	   pfada[pi_level[vater->label_i]] = 0;
	   if (vater->lo_p != NULL) kwk(vater->lo_p);
	   else if (vater == OKFDD_ONE) erg^=1;

	   if (pfads[pi_level[vater->label_i]] == 1)
	   {
	      pfada[pi_level[vater->label_i]] = 1;
	      if (m_and(vater->hi_p) != NULL)
	      {
		 erg^=m_sel(vater->hi_p);
		 kwk(m_and(vater->hi_p));
	      }
	      else if (vater == OKFDD_ONE) erg^=1;
	   }
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Shell  // Interactive Interface (x = EXIT)     //
/* ========================================================================= */
void dd_man::OKFDD_Shell()
{
	short   loop;

	uchar   rel      = 'a', ndt = 51, art = 'v', sart = '3', scam = '3';
	usint   von      = 0,   bis = 0,  len = 5, offset = 2, auswahl = 1;
	float   faktor   = OKFDD_Siftfactor;
	ulint   anchor   = 0;
	
	uchar   smurf    = FALSE;
	uchar   so;
	OKFDD_Now_size_i = OKFDD_Maxidx = 0;

	OKFDD_Outputflags|=ros_p;
	OKFDD_Outputflags|=ros_b;

	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0) OKFDD_Now_size_i += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[OKFDD_Maxidx++]];
        bis = OKFDD_Maxidx-1;
	min_size_i = OKFDD_Now_size_i;

	while (smurf!='x')
	{
		timer.start();

		switch (smurf)
		{     
		        case 'E' :      if (OKFDD_Shell_Extension != NULL) (*OKFDD_Shell_Extension)(this,NULL);		 break;
      case '3' :      OKFDD_Traverse_all(T_Preorder,&dd_man::Trav_Show,NULL);					 break;
			case 'A' :      OKFDD_SAT_all(toprime[All_Prime[pi_limit-1]]->root,ndt-48);			     break;
			case 'B' :      cout << "\nTransformation (->BDD)\n"; OKFDD_DD_to_BDD(von,bis,1); gesamtausgabe();      break;
			case 'C' :      if (scam-48) OKFDD_SAT_all_n(All_Prime[pi_limit-1],scam-48,NULL);
					else OKFDD_SAT_Count(toprime[All_Prime[pi_limit-1]]->root);			     break;
			case 'F' :      cout << "\nTransformation (->FDD)\n"; OKFDD_DD_to_FDD(von,bis,1); gesamtausgabe();      break;
			case 'G' :      gesamtausgabe();									break;
			case 'L' :      OKFDD_Levelswap(von,bis);							       break;
			case '7' :      levelstat();									    break;
			case 'N' :      cout << "\nTransformation (->NDD)\n"; OKFDD_DD_to_NDD(von,bis,1); gesamtausgabe();      break;
			case 'P' :      if (bis-von<7) {cout<<"\nDTL-Permutation\n"; OKFDD_DTL_Permutation(von,bis);
					gesamtausgabe();}								       break;
			case 'Q' :      cout << "\nXOR-DTL-Sifting\n"; dtlsift_XOR(von,bis,faktor,rel,art); gesamtausgabe();    break;
			case 'R' :      cout << "\nDTL-Rotation\n"; OKFDD_Rotation(von,bis,1); gesamtausgabe();		 break;
			case 'S' :      cout << "\nDTL-Sifting\n"; OKFDD_DTL_Sifting(von,bis,faktor,rel,art); gesamtausgabe();  break;
			case 'T' :      cout << "\nDD-Transformation\n"; OKFDD_Sort(von,bis,ndt-48); gesamtausgabe();	   break;
			case 'W' :      if (len<10 && len>1) {cout << "\nWIN-Permutation\n";
					OKFDD_Winpermutation(von,bis,len,offset); gesamtausgabe();}			     break;
#ifdef puma_graph
 		        case 'X' :      XShowDD_all();						 break;
#endif
			case 'Y' :      cout<<"\nDTL-Friedman\n";
					if (bis-von<15) OKFDD_DTL_Friedman(von,bis);
					else	    OKFDD_DTL_Friedman_X(von,bis);
					gesamtausgabe();
					break;
/*			case 'X' :      cout<<"\nPS-Friedman\n";
					if (bis-von<15) OKFDD_DTL_Friedman_PS(von,bis);
					gesamtausgabe();
					break;
*/
			case 'b' :      cout << "\nTransformation (->BDD) (ungeordnet)\n";OKFDD_DD_to_BDD(von,bis,0);
					gesamtausgabe();									break;
			case 'c' :      OKFDD_DTL_Chg_Bottom(von,D_Shan);						       break;
			case 'd' :      OKFDD_Set_Dumpflag();								   break;
			case 'f' :      cout << "\nTransformation (->FDD) (ungeordnet)\n";OKFDD_DD_to_FDD(von,bis,0);
					gesamtausgabe();									break;
			case 'g' :      OKFDD_Setthis(min_pi_table,mindtl);						     break;
			case 'h' :      helpme();									       break;
			case 'i' :      cout << "\nInversion\n"; OKFDD_Inversion(von,bis); gesamtausgabe();		     break;
			case 'l' :      cout << "\nSiftlight\n"; OKFDD_Siftlight(von,bis,art); gesamtausgabe();		 break;
			case 'm' :      OKFDD_Rotate_Max_Lvl(bis,1);							    break;
			case 'n' :      cout << "\nTransformation (->NDD) (ungeordnet)\n";OKFDD_DD_to_NDD(von,bis,0);
					gesamtausgabe();									break;
			case 'o' :      cout << "Dumping order ...\n"; OKFDD_Dumporder(NULL);				   break;
			case 'p' :      if (bis-von<10) { cout << "\nPermutation\n"; OKFDD_Permutation(von,bis);
					gesamtausgabe(); }								      break;
			case 'q' :      cout << "\nWIN-Sifting\n"; OKFDD_Winsift(von,bis,len,offset); gesamtausgabe();	  break;
			case 'r' :      cout << "\nDTL-Rotation (invers)\n"; OKFDD_Rotation(von,bis,-1); gesamtausgabe();       break;
			case 's' :      cout << "\nSifting\n"; OKFDD_Sifting(von,bis,faktor,rel,art); gesamtausgabe();	  break;
			case 't' :      cout << "\nTour\n"; OKFDD_Tour(von,bis); gesamtausgabe();			       break;
			case 'u' :      OKFDD_UT_Statistic();			       						   break;
		 	case 'v' :	cout << "Enter Anchor: ";
					cin >> anchor;
					nextline;
					OKFDD_Show_CT_Chain(anchor);
					break;
			case 'y' :      cout<<"\nFriedman\n";
					if (bis-von<15) OKFDD_Friedman(von,bis);
					else	    	OKFDD_Friedman_X(von,bis);
					gesamtausgabe();
					break;
			case 'z' :      cout<<"\nSCRAMBLE\n";OKFDD_scramble(von,bis,sart-48); gesamtausgabe();		  break;
			case '0' :      nextline;
					cout << "All:  ";   OKFDD_Profile_all(TRUE);    nextline;
					cout << "Vars: " << OKFDD_P_I;		  nextline;
					cout << "Outs: " << OKFDD_P_O;		  nextline;
					cout << "Size: " << OKFDD_Size_all();	   nextline;
					cout << "M-No: " << OKFDD_No_UTNodes;	   nextline;
					cout << "D-No: " << OKFDD_No_Dead_UTNodes();    nextline;
					break;

			case '1' :      cout << "All:  ";   OKFDD_Support_all(TRUE);    nextline; break;
			case '2' :      cout << "All:  ";   OKFDD_DTL_all(TRUE);	nextline; break;

			case 'a' :      cout << "Enter arguments for \n 1) window sifting / window permutation\n 2) ";
					cout << "increasefactor for sifting\n 3) new decomposition type for dd\n 4) ";
					cout << "level select routine for sifting\n 5) sift results output status\n 6) ";
					cout << "size of levelwindow\n 7) SAT methods\n 8) Scramble methods\n : ";
					cin >> auswahl;

					switch (auswahl)
					{
						case 1 :
							cout << "Enter winsift/winper arguments (Old values are size/offset " << len;
							cout << "/" << offset << ")\n";
							cout << "size   : "; cin >> len;
							if ((len<1) || (len>OKFDD_Maxidx)) len = 1;
							cout << "offset : "; cin >> offset;
							if ((offset<1) || (offset>OKFDD_Maxidx-offset-1)) offset = len;
							break;
						case 2 :
							cout << "\nEnter increasefactor for sifting (Old value " << faktor;
							cout << ") : "; cout.flush(); cin >> faktor;
							cout << "\nrelative <r> or absolute <a> (Old value " << rel << ") : ";
							cout.flush(); cin >> rel;
							break;
						case 3 :
							cout << "\nEnter new decomposition type \n< 0 : Shannon, 1 : pos. Davio,";
							cout << "2 : neg. Davio, 3 : old type > (Old value " << (int)(ndt-48) << ") : ";
							cout.flush(); cin >> ndt;
							break;
						case 4 :
							cout << "\nEnter level select routine for sifting\n< g : size oriented, ";
							cout << "v : look ahead, l : loser first, i : initial, r : random >\n(Old ";
							cout << "value " << art << ") : "; cout.flush(); cin >> art;
							break;
						case 5 :
							cout << "\nEnter results output status/moment\n< b : before , a : after , ";
							cout << "n : never , p : proportional > "; cout.flush(); cin >> so;
							switch (so)
							{
								case 'n' : OKFDD_Outputflags&=(!(ros_b|ros_a|ros_y|ros_p));     break;
								case 'b' : OKFDD_Outputflags^=ros_b;			    break;
								case 'a' : OKFDD_Outputflags^=ros_a;			    break;
								case 'v' : OKFDD_Outputflags^=ros_y;
								case 'p' : OKFDD_Outputflags^=ros_p;			    break;
							}
							break;
						case 6 :
							cout << "Enter window [1..." << OKFDD_Maxidx << "] (Old values are upper/lower " << (von+1) << "/";
							cout << (bis+1) << ")\n";
							cout << "Upper border : "; cin>>von; if ((von<1)||(von>OKFDD_Maxidx)) von=1; von--;
							cout << "Lower border : "; cin>>bis; if ((bis<1)||(bis>OKFDD_Maxidx)) bis=OKFDD_Maxidx; bis--;
							break;
						case 7 :
							cout << "Enter SAT method \n";
							cout << "0) SAT count (DD -> BDD -> 'count' -> DD)\n";
							cout << "1) SAT count (path generation)\n";
							cout << "2) SAT all   (path generation)\n";
							cout << "3) 1) + 2)   (path generation)\n";
							cout << "(old setting : " <<scam<<") : "; cin>>scam;
							break;
						case 8 :
							cout << "Scramble \n";
							cout << "1) order only\n";
							cout << "2) decomposition type only\n";
							cout << "3) order and decomposition type\n";
							cout << "(old setting : " <<sart<<") : "; cin>>sart;
							break;

					}
					break;

			case '4' :      fu(loop,0,OKFDD_P_I-1) OKFDD_DTL_Chg_XOR(loop,D_Shan);
					OKFDD_Now_size_i	= OKFDD_Maxidx = 0;
					while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
					OKFDD_Now_size_i += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[OKFDD_Maxidx++]];
					bis = OKFDD_Maxidx-1;
					break;

			case '5' :      fu(loop,0,OKFDD_P_I-1) OKFDD_DTL_Chg_XOR(loop,D_posD);
					OKFDD_Now_size_i	= OKFDD_Maxidx = 0;
					while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
					OKFDD_Now_size_i += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[OKFDD_Maxidx++]];
					bis = OKFDD_Maxidx-1;
					break;

			case '6' :      fu(loop,0,OKFDD_P_I-1) OKFDD_DTL_Chg_XOR(loop,D_negD);
					OKFDD_Now_size_i	= OKFDD_Maxidx = 0;
					while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
					OKFDD_Now_size_i += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[OKFDD_Maxidx++]];
					bis = OKFDD_Maxidx-1;
		}

		timer_stop = timer.stop();

		cout << "SHELL-Time: "<< (float)(timer_stop)/100 << " sec\n";

		breakline; cout << "\nCommand > "; cin >> smurf;

	}
	breakline;
}
/* ========================================================================= */



/*****************************************************************************/
//
//		  >>>     STRUCTURE ANALYSING FUNCTIONS       <<<
//
/*****************************************************************************/

/* ========================================================================= */
// Function:    dd_man::OKFDD_Type // Returns type of OKFDD    		     //
/* ========================================================================= */
uchar dd_man::OKFDD_Type(utnode* root)
{
	// Check for NULL-Operands
	// -----------------------
	if (root == NULL)
        {
             OKFDD_Error = Err_NULL_Op; return 0;
        }
        else OKFDD_Error = Err_No_Error;

	if (root == OKFDD_ZERO) return Type_ZERO;
	if (root == OKFDD_ONE)  return Type_ONE;

	OKFDD_DTL(root,FALSE);

	/* I0 holds number of supported labels stored in OKFDD_Result */
        /* (OKFDD_Result_2 holds corresponding DTL-types)             */

	uchar type = 0;

	fu1(I1,0,I0)
	{
		switch (OKFDD_Result_2[I1])
		{
			case D_Shan:    type = type | Type_S; break;
			case D_posD:    type = type | Type_P; break;
			case D_negD:    type = type | Type_N;
		}
	}

	return type;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Identity // Equivalence check of 2 OKFDD 	     //
/* ========================================================================= */
uchar dd_man::OKFDD_Identity(   utnode* F,
				utnode* G)
{
	// Check for NULL-Operands
	// -----------------------
	if ((F == NULL) || (G == NULL))
        {
             OKFDD_Error = Err_NULL_Op; return 0;
        }
        else OKFDD_Error = Err_No_Error;

	if (F == G) return TRUE; else return FALSE;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Satisfiability                                 //
// Returns primary setting to reach terminal node ONE                        //
/* ========================================================================= */
void dd_man::OKFDD_Satisfiability(      utnode* F,
					usint*  labels,
					uchar*  settings)
{
	if (F == NULL)
        {
             OKFDD_Error = Err_NULL_Op; labels[0] = 0;
        }
        else OKFDD_Error = Err_No_Error;

	// Check terminal cases
	// --------------------
	if ((m_and(F))->label_i < 1) { labels[0] = 0; return; }

	usint k;

	I0 = 0;

	// Follow path till reaching terminal node
	// ---------------------------------------
	while(TRUE)
	{
		while(TRUE)
		{
			// Get actual label
			// ----------------
			k = (m_and(F))->label_i;

			labels[I0] = k; settings[I0++] = 0;

			// Get mark-corrected low-son
			// --------------------------
			uthelp = m_xor((m_and(F))->lo_p,m_sel(F));

			// Is low-son terminal node ?
                        // Yes -> leave left-following-mode
			// --------------------------------
			if ( (m_and(uthelp))->label_i < 1) break;

			if (OKFDD_PI_DTL_Table[k] == D_negD) settings[I0-1]= 1;

			// Follow left path
			// ----------------
			F = uthelp;
		}

		// ----------------------------------------------
		// Last level reached ? / No -> follow path right
		// ----------------------------------------------
		if (uthelp == OKFDD_ONE)
                {
                   if (OKFDD_PI_DTL_Table[k] == D_negD) settings[I0-1] = 1;
                   break; /* break anyway */
                }

		// Not last level ?
                // -> Follow right path (move mark if Shannon type)
		// ------------------------------------------------
		switch (OKFDD_PI_DTL_Table[k])
		{
		   case D_Shan: F = m_xor((m_and(F))->hi_p,m_sel(F));
                                settings[I0-1] = 1;     break;
		   case D_posD: F = (m_and(F))->hi_p;
                                settings[I0-1] = 1;     break;
		   case D_negD: F = (m_and(F))->hi_p;
		}

		if (F == OKFDD_ONE) break;
                /* Terminal ZERO isn't possible here */
	}

	labels[I0] = 0;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Evaluate // Evaluate OKFDD		     //
/* ========================================================================= */
uchar dd_man::OKFDD_Evaluate(   utnode* F,
				usint*  labels)
{
	if (F == NULL)
        {
             OKFDD_Error = Err_NULL_Op; return 0;
        }
        else OKFDD_Error = Err_No_Error;

	// Check terminal cases
	// --------------------
	if (F == OKFDD_ZERO) return FALSE;
	if (F == OKFDD_ONE)  return TRUE;

	// Copy field of labels in OKFDD_PI_Order_Table for direct access
	// --------------------------------------------------------------
	fu1(I0,0,pi_limit) OKFDD_Result[I0] = FALSE; I0 = 0;
        while(labels[I0] != 0) { OKFDD_Result[labels[I0]] = TRUE; I0++; }

	// Analyse OKFDD
	// -------------
	uthelp = Evaluate_slave(F);

	// Check terminal cases
	// --------------------
	if (uthelp == OKFDD_ZERO) return FALSE;
	if (uthelp == OKFDD_ONE)  return TRUE;

	/* This line can't be reached (under normal conditions) */

	OKFDD_Error = Err_Evaluation; return 0;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Evaluate_slave // Evaluate OKFDD		     //
/* ========================================================================= */
utnode* dd_man::Evaluate_slave(utnode* R)
{
	usint k; if ((k = (m_and(R))->label_i) < 1) return R;

	// Check for setting of label (ZERO or ONE)
	// ----------------------------------------
	if (OKFDD_Result[k] == FALSE)
	{
	   switch (OKFDD_PI_DTL_Table[k])
	   {
	      case D_Shan: return Evaluate_slave(m_xor((m_and(R))->lo_p,
                                  m_sel(R)));
	      case D_posD: return Evaluate_slave(m_xor((m_and(R))->lo_p,
                                  m_sel(R)));
	      case D_negD: return CO_XOR(Evaluate_slave(m_xor((m_and(R))->lo_p,
                                  m_sel(R))),Evaluate_slave((m_and(R))->hi_p));
	   }
	}
	else if (OKFDD_Result[k] == TRUE)
	{
	   switch (OKFDD_PI_DTL_Table[k])
	   {
	      case D_Shan: return Evaluate_slave(m_xor((m_and(R))->hi_p,
                                  m_sel(R)));
	      case D_posD: return CO_XOR(Evaluate_slave(m_xor((m_and(R))->lo_p,
                                  m_sel(R))),Evaluate_slave((m_and(R))->hi_p));
	      case D_negD: return Evaluate_slave(m_xor((m_and(R))->lo_p,
                                  m_sel(R)));
	   }
	}

	return NULL; /* This case isn't activated <- No idea how to use it */
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Check_Refs                                     //
// Checks Reference-Counter of all existing nodes	                     //
/* ========================================================================= */
ulint dd_man::OKFDD_Check_Refs(uchar output)
{
	if (output == TRUE)
        cout << "\nCROSS_CHECKING data structure (Please standby ...)\n\n";

	I4 = 0;

	fu1(I0,0,OKFDD_P_I) fu1(I1,0,ut_sizes[ut_hashsize[All_Prime[I0]]])
        if ((uthelp = ut[All_Prime[I0]][I1]) != NULL)
	{
	   do
	   {
	      // Output percentage of finished checks
	      // ------------------------------------
	      if (output == TRUE)
	      {
		 J1 = (short)((float)(OKFDD_Dots_Out_Limit * I0)/(OKFDD_P_I - 
                 1)); J2 = OKFDD_Dots_Out_Limit - J1;

		 cout << "\r<"; fu1(J0,0,J1) cout << "m";
                 fu1(J0,0,J2) cout << "~";
                 cout << "> \tLevel " << I0 << "   \t";
		 cout.flush();
	      }

	      J4 = 0;

	      fu1(I2,0,OKFDD_P_I)
              fu1(I3,0,ut_sizes[ut_hashsize[All_Prime[I2]]])
              if ((utlast = ut[All_Prime[I2]][I3]) != NULL)
	      {
		 do
		 {
		    if (m_and(utlast->lo_p) == uthelp) J4++;
		    if (m_and(utlast->hi_p) == uthelp) J4++;
		 }
		 while((utlast = utlast->next) != NULL);
	      }

	      fu1(I2,0,OKFDD_P_O) if ((m_and(toprime[All_Prime[pi_limit-1-
              I2]]->root)) == uthelp) J4++;

	      if (uthelp->ref_c != (J4 + 1))
	      {
	         if (output == TRUE)
		 {
		    cout << "(ID:Stored_Ref:Checked_Ref) ";
                    cout << uthelp->idnum << ":" << (J4 + 1);
		    cout << ":" << uthelp->ref_c << "\n";
		 }

		 I4++;
	      }
	   }
	   while ((uthelp = uthelp->next) != NULL);
	}

	// Output number of errors (when allowed)
	// --------------------------------------
	if (output == TRUE)
        {
           if (I4 == 0) cout << "\n> No mismatch detected\n";
           else cout << "\n> " << I4 << " errors detected\n";
        }

	// Set global Errorstatusfield
	// ---------------------------
	if (I4 != 0) OKFDD_Error = Err_Error; else OKFDD_Error = Err_No_Error;

	return I4;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_SAT_Count	// okfdd => obdd, zaehlen, => okfdd  //
/* ========================================================================= */
void dd_man::OKFDD_SAT_Count(utnode* wurzel)
{
	ulint sum = 0;
	int   i;

	// Retten der urspruenglichen Daten
	// --------------------------------
	for (i = 0; i < OKFDD_Maxidx; i++)
	{
		pi_level[OKFDD_PI_Order_Table[i]] = i;
		twin_pi_table[i] = OKFDD_PI_Order_Table[i];
		OKFDD_Result[i] = OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]];
	}
	pi_level[0] = OKFDD_Maxidx;

	// Transformation in bdd
	// ---------------------
	OKFDD_DD_to_BDD(0,OKFDD_Maxidx-1,0);

	// Zaehlen der Pfade
	// -----------------
	sum = bdd_count(wurzel);

	cout << " Number or SAT count paths : " << sum << "\n";

	// OKFDD wiederherstellen
	// ----------------------
	for (i = 0; i < OKFDD_Maxidx; i++)
	{
		min_pi_table[i]	= twin_pi_table[i];
		mindtl[min_pi_table[i]] = OKFDD_Result[i];
	}

	OKFDD_Setthis(min_pi_table,mindtl);
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::bdd_count // Rekursive Zaehlroutine		     //
/* ========================================================================= */
ulint dd_man::bdd_count(utnode* F)
{
	if (F == OKFDD_ZERO) return 0;
	if (F == OKFDD_ONE)  return 1 << OKFDD_Maxidx;

	return ( bdd_count(m_xor((m_and(F))->lo_p,m_sel(F))) +
                 bdd_count(m_xor((m_and(F))->hi_p,m_sel(F))) ) >> 1;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_SAT_all // Zaehlen, Ausgeben d.Pfade e.OKFDD   //
/* ========================================================================= */
void dd_man::OKFDD_SAT_all(     utnode* wurzel,
				char    fkt   )
{
	usint   zeiger;
	char    flag, carry;
	ulint   asum = 0, csum = 0, tsum;
	int     i;

	// Initialisierung
	// ---------------
	for (i = 0; i < OKFDD_Maxidx; i++)
	{
		pi_level[OKFDD_PI_Order_Table[i]] = i;
		pfads[i]			  = 0;
	}

	// Alle Kombinationen durchspielen
	// -------------------------------
	flag = 1;
	while (flag)
	{
		// Ausgabepfad mit don't cares initialisieren
		// ------------------------------------------
		for (i = 0; i < OKFDD_Maxidx; i++) pfada[i] = 2;
		erg = 0;

		// rek. Routine aufrufen
		// ---------------------
		erg^=m_sel(wurzel);
		kwk(m_and(wurzel));

		// Falls erfuellende Belegung gefunden
		// -----------------------------------
		if (erg)
		{
		   // Ergebnisse und Steuerpfad aktualisieren +
                   // evtl. Ausgabe
		   // ----------------------------------------------------
		   asum++;
		   tsum = 1;
		   for (i = 0; i < OKFDD_Maxidx; i++)
		   {
		      if (pfada[i] == 2) pfads[i] = 1;
		      if (fkt>1) cout((ros_b||ros_a)) << outchar[pfada[i]];
		      if (fkt!=2) if (pfada[i]) tsum *= pfada[i];
		   }
		   if (fkt>1) cout((ros_b||ros_a)) << "\n";
		   if (fkt!=2) csum += tsum;
		}

		// Naechsten Steuerpfad berechnen
		// ------------------------------
		carry  = 1;
		zeiger = OKFDD_Maxidx-1;
		while (carry)
		{
			pfads[zeiger] ^= 1;
			if (pfads[zeiger] == 0)
			{
			   if (zeiger == 0) carry = flag = 0;
			   else zeiger--;
			}
			else carry = 0;
		}
	}

	// Ausgabe der Ergebnisse
	// ----------------------
	cout((ros_b||ros_a)) << "\n";

	if (fkt>1)
           cout((ros_b||ros_a)) << " #SAT all paths:   " << asum << "\n";
	if (fkt!=2)
           cout((ros_b||ros_a)) << " #SAT count paths: " << csum << "\n";
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_SAT_all_n	// Zaehlen, Ausgeben d.Pfade e.OKFDD //
/* ========================================================================= */
void dd_man::OKFDD_SAT_all_n(   usint   label,
				char    fkt   ,
				char*   dumpfilename = NULL)
{
	char outofpath[50] = "";
	char outofsat[10]  = "dump.sat";

	usint   zeiger;
	char    flag, carry;
	ulint   asum = 0, csum = 0, tsum;
	int     i;
	utnode* wurzel;

	OKFDD_Error = Err_No_Error;

	// Sort labels (ascending)
	// -----------------------
	fu1(I0,0,OKFDD_Maxidx) OKFDD_Result[I0] = OKFDD_PI_Order_Table[I0];

	fu1(I0,0,OKFDD_Maxidx) fd(I1,OKFDD_Maxidx-1,1)
        if (OKFDD_Result[I1] < OKFDD_Result[I1-1])
	{
	   J0 = OKFDD_Result[I1]; 
           OKFDD_Result[I1] = OKFDD_Result[I1-1]; OKFDD_Result[I1-1] = J0;
	}

	// Show order
	// ----------
	if (OKFDD_Outputflags & 8192)
	{
	   cout << "\n.order :\n";

	   fu1(I0,0,OKFDD_Maxidx) cout << (OKFDD_PI_Level_Table[OKFDD_Result[
             I0]]/2) << " <" << dtl[OKFDD_PI_DTL_Table[OKFDD_Result[I0]]]
             << ">  "; nextline;
	}

	// --------------
	// Create satfile
	// --------------
	if (dumpfilename != NULL) strcat(outofpath,dumpfilename);
        else                      strcat(outofpath,outofsat);

	ofstream forder(outofpath);

	if (!forder)
        {
          cout(2) << "OKFDD_SAT_all_n: Can`t create file ...\n";
          OKFDD_Error = Err_Dumporder; return;
        }

	// Write Header (Name of benchmark, # of PIs and # of POs)
	// -------------------------------------------------------
	forder.write("# PUMA * OKFDD_Manager V2.22 Copyright (C)'96 by Andreas HETT\n",62);
	forder.write("# \n",3);
	forder.write("# SAT_all_file for CIRCUIT < ",29);

	I0 = 0;
        do { if (FILEX[I0++] == '\0') break; } while (TRUE);
        forder.write(FILEX,I0-1); forder.write(" >\n",3);

	forder.write("\n# Number of PIs: ",18);
	J0 = OKFDD_P_I;
        fu1(K1,0,5)
        { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }
	fd(J1,4,0) forder.put(line[J1]);

	forder.write("\n# Number of POs: ",18);
	J0 = OKFDD_P_O;
        fu1(K1,0,5)
        { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }
	fd(J1,4,0) forder.put(line[J1]);

	// -----------
	// Write Order
	// -----------
	wurzel =  toprime[label]->root;
	forder.write("\n\n.order\n",9);

	fu1(I0,0,OKFDD_Maxidx)
	{
	   J0 = OKFDD_PI_Level_Table[OKFDD_Result[I0]]/2;
           fu1(K1,0,5)
           { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }
           fd(J1,4,0) forder.put(line[J1]);
	   forder.write(" ",1);
           forder.put(dtl[OKFDD_PI_DTL_Table[OKFDD_Result[I0]]]);
           forder.write("\n",1);
	}

	forder.write(".end",4);

	forder.write("\n\n.sat_all for < ",17);
	fu(J1,0,16) forder.put(toprime[label]->name[J1]);
	forder.write("... > with label ",17);
	J0 = label;
        fu1(K1,0,5)
        { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }
	fd(J1,4,0) forder.put(line[J1]);
	forder.write("\n",1);

	// Initialisierung
	// ---------------
	wurzel =  toprime[label]->root;
	for (i = 0; i < OKFDD_Maxidx; i++)
	{
	   pi_level[OKFDD_PI_Order_Table[i]] = i;
	   pfads[i]			     = 0;
	}

	// Alle Kombinationen durchspielen
	// -------------------------------
	flag = 1;
	while (flag)
	{
		// Ausgabepfad mit don't cares initialisieren
		// ------------------------------------------
		for (i = 0; i < OKFDD_Maxidx; i++) pfada[i] = 2;
		erg = 0;

		// rek. Routine aufrufen
		// ---------------------
		erg^=m_sel(wurzel);
		kwk(m_and(wurzel));

		// Falls erfuellende Belegung gefunden
		// -----------------------------------
		if (erg)
		{
			// Ergebnisse und Steuerpfad aktualisieren +
                        // evtl. Ausgabe
			// -----------------------------------------
			asum++;
			tsum = 1;
			for (i = 0; i < OKFDD_Maxidx; i++)
			{
				if (pfada[i] == 2) pfads[i] = 1;
				if (fkt>1)
                                   cout((ros_b||ros_a)) << outchar[pfada[i]];
				if (fkt!=2) if (pfada[i]) tsum *= pfada[i];
				forder.put(outchar[pfada[i]]);
			}
			if (fkt>1) cout((ros_b||ros_a)) << "\n";
			if (fkt!=2) csum += tsum;
			forder.write("\n",1);
		}

		// Naechsten Steuerpfad berechnen
		// ------------------------------
		carry  = 1;
		zeiger = OKFDD_Maxidx-1;
		while (carry)
		{
			pfads[zeiger] ^= 1;
			if (pfads[zeiger] == 0)
			{
				if (zeiger == 0) carry = flag = 0;
				else zeiger--;
			}
			else carry = 0;
		}
	}

	// Ausgabe der Ergebnisse
	// ----------------------
	cout((ros_b||ros_a)) << "\n";

	if (fkt>1)
          cout((ros_b||ros_a)) << " #SAT all paths:   " << asum << "\n";
	if (fkt!=2)
          cout((ros_b||ros_a)) << " #SAT count paths: " << csum << "\n";

	forder.write(".sat_all_paths ",15);
	J0 = asum;
        fu1(K1,0,5)
        { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }
	fd(J1,4,0) forder.put(line[J1]);
	forder.write("\n.end_sat_all",13);

	forder.close();
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_Set_Dumpflag                                   //
// Aktivierung der Ausgabe best. Statusinformationen	                     //
/* ========================================================================= */
void dd_man::OKFDD_Set_Dumpflag()
{
	OKFDD_Outputflags^=dumpflag;

	cout << "OKFDD_Dumpflag = " << (short)((float)(OKFDD_Outputflags & 
        dumpflag)/dumpflag) << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::init // Initialisierung der Groesse und Anzahl	     //
/* ========================================================================= */
void dd_man::init()
{
	OKFDD_Now_size_i = OKFDD_Maxidx = 0;

	while (OKFDD_PI_Order_Table[OKFDD_Maxidx] != 0)
          OKFDD_Now_size_i += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[
          OKFDD_Maxidx++]];
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::helpme // Liste der moeglichen Shell-Kommandos	     //
/* ========================================================================= */
void dd_man::helpme()
{
	cout<< "0 :  Shows profile of all DDs				                  \n";
	cout<< "1 :  Shows support-List of all DDs					  \n";
	cout<< "2 :  Shows DTL-List of all DDs						  \n";
	cout<< "3 :  Traverse all DDs with Trav_Show (shows node informations)		  \n";
	cout<< "a :  Set window arguments (size, offset ...)				  \n";
	cout<< "B :  DD_to_BDD								  \n";
	cout<< "b :  DD_to_BDD Fast-version (ignores initial order)			  \n";
	cout<< "C :  SAT_all and SAT_count (select arguments with 'a')			  \n";
	cout<< "c :  Change first DTL-Type to Shannon					  \n";
	cout<< "d :  Set Dumpflag							  \n";
	cout<< "E :  Call external function (if provided)				  \n";
	cout<< "F :  DD_to_FDD								  \n";
	cout<< "f :  DD_to_FDD Fast-version (ignores initial order)			  \n";
	cout<< "G :  Shows gathered output information					  \n";
	cout<< "g :  Goto last found mininal ordering and DTL-type settings		  \n";
	cout<< "h :  This helptext							  \n";
	cout<< "i :  Inversion				  [upper_border,lower_border]     \n";
	cout<< "L :  Levelswap				  [upper_border,lower_border]     \n";
	cout<< "l :  Siftlight				  [upper_border,lower_border]     \n";
	cout<< "N :  DD_to_NDD								  \n";
	cout<< "n :  DD_to_NDD Fast-version (ignores initial order)			  \n";
	cout<< "o :  Dump order								  \n";
	cout<< "P :  DTL-Permutation			  [upper_border,lower_border]     \n";
	cout<< "p :  Permutation		          [upper_border,lower_border]     \n";
	cout<< "R :  Rotate DTL-Types S->P->N->S	  [upper_border,lower_border]     \n";
	cout<< "r :  Rotate DTL-Types S<-P<-N<-S	  [upper_border,lower_border]     \n";
	cout<< "S :  DTL-Sifting			  [upper_border,lower_border]     \n";
	cout<< "s :  Sifting				  [upper_border,lower_border]     \n";
	cout<< "T :  Transform DDs to standard order					  \n";
	cout<< "t :  Tour				  [upper_border,lower_border]     \n";
	cout<< "u :  Shows UT-Statistics					          \n";
	cout<< "v :  Shows Computed Table chain at giving anchorposition                  \n";
	cout<< "W :  Windowpermutation			  [upper_border,lower_border]     \n";
	cout<< "X :  Shows graphical visualization of the DDs	       	                  \n";
	cout<< "Y :  DTL-Friedman			  [upper_border,lower_border]     \n";
	cout<< "y :  Friedman				  [upper_border,lower_border]     \n";
	cout<< "z :  Scrambles ordering and DTL-types					  \n";
}
/* ========================================================================= */



/*****************************************************************************/
//
//			 >>>     TRAVERSE ALGORITHMS     <<<
//
/*****************************************************************************/

/* ========================================================================= */
// Function:    dd_man::OKFDD_Profile                                        //
// Collects support vars nodes` of OKFDD and DTL                             //
/* ========================================================================= */
void dd_man::OKFDD_Profile(     utnode* root,
				uchar   output)
{
	utnode** root_table = new utnode*[2];
        root_table[0] = root; root_table[1] = NULL;

	OKFDD_Profile_mult(root_table,output);

	delete[] root_table;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Profile_mult		                     //
// Collects support vars nodes` of OKFDD and DTL                             //
/* ========================================================================= */
void dd_man::OKFDD_Profile_mult(utnode**	root_table,
				uchar	   output	  )
{
	fu1(I0,0,pi_limit) OKFDD_Result_2[I0] = 0;

	OKFDD_Traverse_mult(root_table,T_Preorder,&dd_man::Trav_Prof,NULL);

	// Squeeze OKFDD_Result_2 to closed block
	// --------------------------------------
	I0 = 0; fu1(I1,0,pi_limit) if (OKFDD_Result_2[I1] != 0)
	{ OKFDD_Result_2[I0] = OKFDD_Result_2[I1]; OKFDD_Result[I0++] = I1; }

	OKFDD_Result[I0] = 0; OKFDD_Result_2[I0] = 0;

	OKFDD_Supported_Vars = I0;


	// Bubble-Sort primary inputs according ascending pi inverse
        // (when number of elements > 1)
	// ---------------------------------------------------------
	if (I0 > 1)
	{
	   fdi(j,I0-1,1) fui(i,0,j)
           if (OKFDD_PI_Level_Table[OKFDD_Result[i]] >
           OKFDD_PI_Level_Table[OKFDD_Result[i+1]])
	   {
	      I2 = OKFDD_Result[i]; OKFDD_Result[i] = OKFDD_Result[i+1];
              OKFDD_Result[i+1] = I2;
	      I2 = OKFDD_Result_2[i]; OKFDD_Result_2[i] = OKFDD_Result_2[i+1];
	      OKFDD_Result_2[i+1] = I2;
	   }
	}

	if (output == TRUE)
	{
		if (OKFDD_Result[0] == 0)
                  cout << "PROFILE_LIST is empty ...\n";
		else
		{
			cout << "PROFILE_LIST:\n\n";

			int i = 0;

			// Search maximal level
			// --------------------
			I1 = 0; while(OKFDD_Result[i] != 0)
                        {
                          if (I1 < OKFDD_Result_2[i]) I1 = OKFDD_Result_2[i];
                          i++;
                        }

			float factor = (float)(OKFDD_Prof_Out_Limit)/I1;

			// ---------------------------------------------------
			// Show all levels with name, label, number of utnodes
                        // and histogram
			// ---------------------------------------------------
			i = 0;

			while(OKFDD_Result[i] != 0)
			{
				usint j;

				cout.width(4); cout.fill('.');
                                cout << (i+1) << ": ";

				if (OKFDD_Outputflags & pno)
				{
				   cout << "<";
                                   fu1(j,0,16)
                                   cout << toprime[OKFDD_Result[i]]->name[j];
				   cout << "...> with label ";
				} else cout << "Label ";

				cout.width(5); cout.fill('0');
                                cout << OKFDD_Result[i]; cout << " has ";
				cout.width(5); cout.fill(' ');
                                cout << OKFDD_Result_2[i]; cout << " nodes   ";

				K0 = (usint)(factor*OKFDD_Result_2[i]);

				fu(j,1,(OKFDD_Prof_Out_Limit - K0) / 2)
                                   cout << " ";

				fu(j,0,K0)
                                  cout << dtl[OKFDD_PI_DTL_Table[OKFDD_Result[
                                  i]]]; cout << "\n";

				i++;
			}
		}
		nextline;
                cout << "Number of supported vars: " << OKFDD_Supported_Vars;
                cout << "\n\n";
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Profile_all	                             //
// Collects support vars nodes` of all OKFDDs	                             //
/* ========================================================================= */
void dd_man::OKFDD_Profile_all(uchar output)
{
	utnode** root_table = new utnode*[OKFDD_P_O + 1];

	fu1(I0,0,OKFDD_P_O)
          root_table[I0] = toprime[All_Prime[pi_limit-1-I0]]->root;
        root_table[OKFDD_P_O] = NULL;

	OKFDD_Profile_mult(root_table,output);

	delete[] root_table;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL	                                     //
// Collects support vars of OKFDD and DTL		                     //
/* ========================================================================= */
void dd_man::OKFDD_DTL( utnode* root,
			uchar   output)
{
	utnode** root_table = new utnode*[2];
        root_table[0] = root; root_table[1] = NULL;

	OKFDD_DTL_mult(root_table,output);

	delete[] root_table;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_mult					     //
// Collects support vars of OKFDD and DTL		                     //
/* ========================================================================= */
void dd_man::OKFDD_DTL_mult(    utnode**	root_table,
				uchar	   output	  )
{
	OKFDD_Support_mult(root_table,FALSE);

	fu1(I1,0,I0) OKFDD_Result_2[I1] = OKFDD_PI_DTL_Table[OKFDD_Result[I1]];

	if (output == TRUE)
	{
		if (OKFDD_Result[0] == 0) cout << "DTL_LIST is empty ...\n";
		else
		{
			cout << "DTL_LIST:\n\n";

			// ---------------------------------------------
			// Show all levels with name, label and DTL-type
			// ---------------------------------------------
			int i = 0;

			while(OKFDD_Result[i] != 0)
			{
				cout.width(4); cout.fill('.');
                                cout << (i+1) << ": ";

				if (OKFDD_Outputflags & pno)
				{
				   cout << "<";
                                   fui(j,0,15)
                                     cout << toprime[OKFDD_Result[i]]->name[j];
				   cout << "...> with label ";
				} else  cout << "Label ";

				cout.width(5); cout.fill('0');
                                cout << OKFDD_Result[i];
                                cout << " is of type ";

				switch (OKFDD_Result_2[i++])
				{
				   case D_Shan: cout<< "Shannon       "; break;
				   case D_posD: cout<< "positive_Davio"; break;
				   case D_negD: cout<< "negative_Davio";
				}
				cout << "\n";
			}
			OKFDD_Supported_Vars = i;
		}
		nextline;
                cout << "Number of supported vars: " << OKFDD_Supported_Vars;
                cout << "\n\n";
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_all					     //
// Collects support vars of all OKFDDs		                             //
/* ========================================================================= */
void dd_man::OKFDD_DTL_all(uchar output)
{
	utnode** root_table = new utnode*[OKFDD_P_O + 1];

	fu1(I0,0,OKFDD_P_O)
          root_table[I0] = toprime[All_Prime[pi_limit-1-I0]]->root;
        root_table[OKFDD_P_O] = NULL;

	OKFDD_DTL_mult(root_table,output);

	delete[] root_table;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Support					     //
// Collects support vars of OKFDD			                     //
/* ========================================================================= */
void dd_man::OKFDD_Support(     utnode* root,
				uchar   output)
{
	utnode** root_table = new utnode*[2];
        root_table[0] = root; root_table[1] = NULL;

	OKFDD_Support_mult(root_table,output);

	delete[] root_table;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Support_mult				     //
// Collects support vars of OKFDDs		                             //
/* ========================================================================= */
void dd_man::OKFDD_Support_mult(utnode**	root_table,
				uchar	   output	  )
{
	fu1(I0,0,pi_limit) OKFDD_Result_2[I0] = 0;

	OKFDD_Traverse_mult(root_table,T_Postorder,&dd_man::Trav_Supp,NULL);

	I0 = 0; fu1(I1,0,pi_limit) if (OKFDD_Result_2[I1] != 0)
	{ OKFDD_Result_2[I0] = OKFDD_Result_2[I1]; OKFDD_Result[I0++] = I1; }

	OKFDD_Result[I0] = 0; OKFDD_Result_2[I0] = 0;

	OKFDD_Supported_Vars = I0;


	// Bubble-Sort primary inputs according ascending pi inverse
        // (when number of elements > 1)
	// ---------------------------------------------------------
	if (I0 > 1)
	{
	   fdi(j,I0-1,1) fui(i,0,j)
           if (OKFDD_PI_Level_Table[OKFDD_Result[i]] >
           OKFDD_PI_Level_Table[OKFDD_Result[i+1]])
	   {
		   I2 = OKFDD_Result[i]; OKFDD_Result[i] = OKFDD_Result[i+1];
                   OKFDD_Result[i+1] = I2;
		   I2 = OKFDD_Result_2[i];
                   OKFDD_Result_2[i] = OKFDD_Result_2[i+1];
		   OKFDD_Result_2[i+1] = I2;
	   }
	}

	if (output == TRUE)
	{
	   if (OKFDD_Result[0] == 0) cout << "SUPPORT_LIST is empty ...\n";
	   else
	   {
		   cout << "SUPPORT_LIST:\n\n";

		   // -----------------------------------
		   // Show all levels with name and label
		   // -----------------------------------
		   int i = 0;

		   while(OKFDD_Result[i] != 0)
		   {
		      cout.width(4); cout.fill('.'); cout << (i+1) << ": ";

		      if (OKFDD_Outputflags & pno)
		      {
			 cout << "<";
                         fui(j,0,15) cout << toprime[OKFDD_Result[i]]->name[j];
			 cout << "...> with label ";
		      }
                      else  cout << "Label ";

		      cout.width(5); cout.fill('0');
                      cout << OKFDD_Result[i++]; cout << "\n";
		   }
		   OKFDD_Supported_Vars = i;
		}
		nextline;
                cout << "Number of supported vars: " << OKFDD_Supported_Vars;
                cout << "\n\n";
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Support_all				     //
// Collects support vars of all OKFDDs		                             //
/* ========================================================================= */
void dd_man::OKFDD_Support_all(uchar output)
{
	utnode** root_table = new utnode*[OKFDD_P_O + 1];

	fu1(I0,0,OKFDD_P_O)
          root_table[I0] = toprime[All_Prime[pi_limit-1-I0]]->root;
        root_table[OKFDD_P_O] = NULL;

	OKFDD_Support_mult(root_table,output);

	delete[] root_table;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Size					     //
// Calculates size of OKFDD			                             //
/* ========================================================================= */
ulint dd_man::OKFDD_Size(utnode* root)
{
	utnode** root_table = new utnode*[2];
        root_table[0] = root; root_table[1] = NULL;

	I0 = 0;
  OKFDD_Traverse_mult(root_table,T_Postorder,&dd_man::Trav_Size ,NULL);
  delete[] root_table;
  return I0;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Size_mult					     //
// Calculates total size of OKFDDs		                             //
/* ========================================================================= */
ulint dd_man::OKFDD_Size_mult(utnode** root_table)
{
	I0 = 0;
  OKFDD_Traverse_mult(root_table,T_Postorder,&dd_man::Trav_Size,NULL);
  return I0;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Size_all					     //
// Calculates total size of nodes			                     //
/* ========================================================================= */
ulint dd_man::OKFDD_Size_all()
{
	utnode** root_table = new utnode*[OKFDD_P_O + 1];

	fu1(I0,0,OKFDD_P_O)
          root_table[I0] = toprime[All_Prime[pi_limit-1-I0]]->root;
        root_table[OKFDD_P_O] = NULL;

	I0 = 0;
  OKFDD_Traverse_mult(root_table,T_Postorder,&dd_man::Trav_Size,NULL);
  delete[] root_table;
  return I0;
};
/* ========================================================================= */




/* ========================================================================= */
// Function:    dd_man::OKFDD_1_PC_Slave // Return no. of 1 paths in OKFDD   //
/* ========================================================================= */
ulint dd_man::OKFDD_1_PC_Slave(utnode* F)
{
	if (F == OKFDD_ZERO) return 0;
	if (F == OKFDD_ONE ) return 1;

	if (OKFDD_PI_DTL_Table[(m_and(F))->label_i] == D_Shan)
	{
	   return (OKFDD_1_PC_Slave(m_xor((m_and(F))->lo_p,m_sel(F))) +
             OKFDD_1_PC_Slave(m_xor((m_and(F))->hi_p,m_sel(F))));
	}
	else
	{
	   if (m_xor((m_and(F))->lo_p,m_sel(F)) == (m_and(F))->hi_p)
             return 2 * OKFDD_1_PC_Slave((m_and(F))->hi_p);

	   return (OKFDD_1_PC_Slave(m_xor((m_and(F))->lo_p,m_sel(F))) +
             OKFDD_1_PC_Slave((m_and(F))->hi_p));
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_1_PC // Returns no. of 1 paths in OKFDD  	     //
/* ========================================================================= */
ulint dd_man::OKFDD_1_PC(utnode*	root,
			 char*	  dumpfilename)
{
	if (dumpfilename != NULL)
	{
	     utnode** root_table = new utnode*[2]; root_table[0] = root;
             root_table[1] = NULL;

	     ulint elements = OKFDD_1_PC_mult(root_table,TRUE,dumpfilename);
             delete[] root_table; return elements;
	}
	else return OKFDD_1_PC_Slave(root);
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_1_PC_mult	// Returns no. of 1 paths in OKFDDs  //
/* ========================================================================= */
ulint dd_man::OKFDD_1_PC_mult(  utnode**	root_table,
				uchar	   Shared,
				char*	   dumpfilename)
{
	usint   loop		  = 0;
	ulint   elements	  = 0;
	ulint   unshared_elements = 0;
	flag			  = FALSE;

	// ----------------
	// Create 1_PC_file
	// ----------------
	if (dumpfilename != NULL)
	{
/**/ //	   char outofpath[50] = ""; strcat(outofpath,dumpfilename);

	   streamhelp.open(dumpfilename); flag = TRUE;

	   if (!streamhelp)
           {
              cout(2) << "OKFDD_1_PC_mult: Can`t create file ...\n";
              OKFDD_Error = Err_Dumporder; return 0;
           }

	   // Write Header (Name of benchmark, # of PIs and # of POs)
	   // -------------------------------------------------------
	   streamhelp.write("# PUMA * OKFDD_Manager V2.22 Copyright (C)'96 by Andreas HETT\n",62);
	   streamhelp.write("# \n",3);
	   streamhelp.write("# 1_PC_file for CIRCUIT < ",26);

	    I0 = 0; do { if (FILEX[I0++] == '\0') break; } while (TRUE);
           streamhelp.write(FILEX,I0-1); streamhelp.write(" >\n",3);

	   streamhelp.write("\n# Number of PIs: ",18);
	   J0 = OKFDD_P_I;
           fu1(K1,0,5)
           { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }
	   fd(J1,4,0) streamhelp.put(line[J1]);

	   streamhelp.write("\n# Number of POs: ",18);
	   J0 = OKFDD_P_O;
           fu1(K1,0,5)
           { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }
	   fd(J1,4,0) streamhelp.put(line[J1]);

	   // Sort labels (ascending)
	   // -----------------------
	   fu1(I0,0,OKFDD_Maxidx) OKFDD_Result[I0] = OKFDD_PI_Order_Table[I0];

	   fu1(I0,0,OKFDD_Maxidx) fd(I1,OKFDD_Maxidx-1,1)
           if (OKFDD_Result[I1] < OKFDD_Result[I1-1])
	   {
	      J0 = OKFDD_Result[I1]; OKFDD_Result[I1] = OKFDD_Result[I1-1];
              OKFDD_Result[I1-1] = J0;
	   }

	   // -----------
	   // Write Order
	   // -----------
	   streamhelp.write("\n\n.order\n",9);

	   fu1(I0,0,OKFDD_Maxidx)
	   {
	      J0 = OKFDD_PI_Level_Table[OKFDD_Result[I0]]/2;
              fu1(K1,0,5)
              { line[K1] = 48 + J0 % 10; if (J0 > 0) J0 = (J0 - J0 % 10)/10; }

	      fd(J1,4,0) streamhelp.put(line[J1]);

	      streamhelp.write(" ",1);
              streamhelp.put(dtl[OKFDD_PI_DTL_Table[OKFDD_Result[I0]]]);
              streamhelp.write("\n",1);
	   }

	   streamhelp.write(".end",4);

	   streamhelp.write("\n\n.exor\n",8);

	   /* Counting is always SHARED (with elements) */
	   /* and UNSHARED (with unshared_elements)     */

	   // Initialize Path table
	   // ---------------------
	   fu1(I0,0,OKFDD_P_I) OKFDD_Result_2[All_Prime[I0]] = DONTCARE;

	   // Count trees in root_table then traverse them
	   // --------------------------------------------
	   I5 = 0;
	   J2 = 0; while(root_table[J2++] != NULL) { } J2--;
	   J3 = 0;
           while(root_table[J3]   != NULL)
           {
             I4 = 0; K4 = J3; Trav_1_PC(root_table,root_table[J3]);
             elements += I4; J3++;
           }

	   // -----------------------------------------------------
	   // Check all trees for calculates 1-paths and write them
	   // -----------------------------------------------------
	   fu1(I0,0,I5)
	   {
	      // Don't check 1-paths if only one tree exists
	      // -------------------------------------------
	      if (J2 > 1)
	      {

		 J3 = 0; while((uthelp = root_table[J3]) != NULL)
		 {
		    fu1(I1,0,OKFDD_P_I)
		    {
		       OKFDD_Result[All_Prime[I1]]   = '-';
		       OKFDD_Result_2[All_Prime[I1]] = field[I0][I1];
		    }

		    K3 = FALSE;

		    while((m_and(uthelp)) != OKFDD_ONE)
		    {
		       switch (OKFDD_PI_DTL_Table[(m_and(uthelp))->label_i])
		       {

		       case D_Shan:

			 switch (OKFDD_Result_2[(m_and(uthelp))->label_i])
			 {
			    case '0': OKFDD_Result[(m_and(uthelp))->label_i] =
                                        '0';
				      uthelp = m_xor((m_and(uthelp))->lo_p,
                                        m_sel(uthelp));
				      break;
			    case '1': OKFDD_Result[(m_and(uthelp))->label_i] =
                                        '1';
				      uthelp = m_xor((m_and(uthelp))->hi_p,
                                        m_sel(uthelp));
				      break;
			    default : K3 = TRUE;
			 }
			 break;

		       case D_posD:

			 switch (OKFDD_Result_2[(m_and(uthelp))->label_i])
			 {
			    case '-': OKFDD_Result[(m_and(uthelp))->label_i] =
                                        '-';
				      uthelp = m_xor((m_and(uthelp))->lo_p,
                                        m_sel(uthelp));
				      break;
			    case '1': OKFDD_Result[(m_and(uthelp))->label_i] =
                                        '1';
				      uthelp = (m_and(uthelp))->hi_p;
				      break;
			    default : K3 = TRUE;
			 }
			 break;

		       case D_negD:

			 switch (OKFDD_Result_2[(m_and(uthelp))->label_i])
			 {
			    case '-': OKFDD_Result[(m_and(uthelp))->label_i] =
                                        '-';
				      uthelp = m_xor((m_and(uthelp))->lo_p,
                                        m_sel(uthelp));
				      break;
			    case '0': OKFDD_Result[(m_and(uthelp))->label_i] =
                                        '0';
				      uthelp = (m_and(uthelp))->hi_p;
				      break;
			    default : K3 = TRUE;
			 }
		      }

		      if (K3 == TRUE) break;
		   }

		   if (uthelp == OKFDD_ONE)
		   {
		      K3 = FALSE;

		      fu1(I1,0,OKFDD_P_I) if (OKFDD_Result[All_Prime[I1]] !=
                      OKFDD_Result_2[All_Prime[I1]])
		      { K3 = TRUE; break; }

		      if (K3 == FALSE)
                      { field[I0][OKFDD_P_I+1+J3] = '1'; unshared_elements++; }
		   }
		   J3++;
		}
	     }

	     streamhelp.write(field[I0],OKFDD_P_I+1+J2);
	     streamhelp.write("\n",1);
	     delete[] field[I0];
	  }

	  if ((J2 > 1) && (Shared == FALSE)) elements = unshared_elements;

	  streamhelp.write(".paths ",7);
	  I0 = elements;
          fu(K1,0,9)
          { line[K1] = 48 + I0 % 10; if (I0 > 0) I0 = (I0 - I0 % 10)/10; }
	  fd(J1,9,0) streamhelp.put(line[J1]);

	  // ---------------
	  // Close 1_PC_file
	  // ---------------
	  streamhelp.write("\n.end_exor",10); streamhelp.close();
	}
	else
	{
	  /* No dumpfile output */

	  // Count elements separately or shared
          // -> Result is in elements and will be returned
	  // ---------------------------------------------
	  if (Shared == TRUE)
	  {
	     // Count first tree
	     // ----------------
	     elements += OKFDD_1_PC_Slave(root_table[0]);

	     // Initialize Path table
	     // ---------------------
	     fu1(I0,0,OKFDD_P_I) OKFDD_Result_2[All_Prime[I0]] = DONTCARE;

	     // Count trees in root_table
	     // -------------------------
	     loop = 1;
             while(root_table[loop] != NULL)
             {
                I4 = 0; K4 = loop; Trav_1_PC(root_table,root_table[loop]);
                elements += I4; loop++;
             }
	   }
	   else
	   {
	     loop = 0;
             while(root_table[loop] != NULL)
             { elements += OKFDD_1_PC_Slave(root_table[loop]); loop++; }
	   }
	}

	return elements;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Trav_1_PC // Slave of OKFDD_1_PC_mult		     //
/* ========================================================================= */
void dd_man::Trav_1_PC( utnode**	root_table,
			utnode*	 root      )
{
	utnode* mroot = m_and(root);

	// -----------------------------------------------------------
	// Path finished -> If terminal ONE reached search path in all
        // visited trees / Not found -> Count it
	// -----------------------------------------------------------
	if (mroot == OKFDD_ONE)
	{
	   if (root == OKFDD_ONE)
	   {
	   utnode* mcroot;
	   utnode* croot;

	   I3 = 0;

	   fu1(K0,0,K4)
	   {
	      // Get root of visited tree
	      // ------------------------
	      croot = root_table[K0];

	      K3 = FALSE;

	      fu1(I1,0,OKFDD_P_I) OKFDD_Result[All_Prime[I1]] = DONTCARE;

	      while((mcroot = m_and(croot)) != OKFDD_ONE)
	      {
		 switch (OKFDD_PI_DTL_Table[mcroot->label_i])
		 {
		 case D_Shan:
		   switch (OKFDD_Result_2[mcroot->label_i])
		   {
		   case NEGATIVE:  croot = m_xor(mcroot->lo_p,m_sel(croot));
				   OKFDD_Result[mcroot->label_i] = NEGATIVE;
				   break;
		   case POSITIVE:  croot = m_xor(mcroot->hi_p,m_sel(croot));
				   OKFDD_Result[mcroot->label_i] = POSITIVE;
				   break;
		   default:	   K3 = TRUE;
		   }
		   break;

		 case D_posD:
		   switch (OKFDD_Result_2[mcroot->label_i])
		   {
		   case DONTCARE:  croot = m_xor(mcroot->lo_p,m_sel(croot));
			           OKFDD_Result[mcroot->label_i] = DONTCARE;
				   break;
		   case POSITIVE:  croot = mcroot->hi_p;
				   OKFDD_Result[mcroot->label_i] = POSITIVE;
				   break;
		   default:	   K3 = TRUE;
		   }
		   break;

		 case D_negD:
		   switch (OKFDD_Result_2[mcroot->label_i])
		   {
		   case DONTCARE:  croot = m_xor(mcroot->lo_p,m_sel(croot));
				   OKFDD_Result[mcroot->label_i] = DONTCARE;
				   break;
		   case NEGATIVE:  croot = mcroot->hi_p;
				   OKFDD_Result[mcroot->label_i] = NEGATIVE;
				   break;
		   default:	   K3 = TRUE;
				   }
		   }

		   if (K3 == TRUE) break;
		 }

		 // End of path reached / ONE -> Path exists in this tree
                 // -> Don't count it again
		 // -----------------------------------------------------
		 if (croot == OKFDD_ONE)
		 {
		    K3 = FALSE;

		    fu1(I1,0,OKFDD_P_I) if (OKFDD_Result[All_Prime[I1]] !=
                    OKFDD_Result_2[All_Prime[I1]])
		    { K3 = TRUE; break; }

		    // Path is not alone ?
		    // -------------------
		    if (K3 == FALSE) I3++;
		 }

		 // At least one duplicate of path exists ?
                 // -> Don't count again
		 // ---------------------------------------
		 if (I3 != 0) break;
	      }

	      // Path is unique -> Count it
	      // --------------------------
	      if (I3 == 0)
	      {
		 I4++;

		 // Write path (if allowed)
		 // -----------------------
		 if (flag != FALSE)
		 {
		    field[I5] = new char[OKFDD_P_I+1+J2];
		    field[I5][OKFDD_P_I] = ' ';
		    fu1(I1,0,J2) field[I5][OKFDD_P_I+1+I1] = '0';
		    field[I5][OKFDD_P_I+1+J3] = '1';
		    fu1(I1,0,OKFDD_P_I)
                      field[I5][I1] = outchar[OKFDD_Result_2[All_Prime[I1]]];
		    I5++;
		 }
	      }
	   }

	   return;
	}

	utnode* son[2];
	uchar   val[2];

	son[0] = m_xor(mroot->lo_p,m_sel(root));

	switch (OKFDD_PI_DTL_Table[mroot->label_i])
	{
	case D_Shan:

		val[0] = NEGATIVE;
		son[1] = m_xor(mroot->hi_p,m_sel(root));
		val[1] = POSITIVE;
		break;

	case D_posD:

		val[0] = DONTCARE;
		son[1] = mroot->hi_p;
		val[1] = POSITIVE;
		break;

	case D_negD:

		val[0] = DONTCARE;
		son[1] = mroot->hi_p;
		val[1] = NEGATIVE;
	}

	usint loop1;

	fu(loop1,0,1)
	{
	   // Store direction and follow it
	   // -----------------------------
	   OKFDD_Result_2[mroot->label_i] = val[loop1];
           Trav_1_PC(root_table,son[loop1]);
	}

	OKFDD_Result_2[mroot->label_i] = DONTCARE;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_1_PC_all                                       //
// Returns no. of 1 paths in all existing OKFDDs	                     //
/* ========================================================================= */
ulint dd_man::OKFDD_1_PC_all(uchar      Shared,
			     char*      dumpfilename)
{
	ulint elements;

	utnode** root_table = new utnode*[OKFDD_P_O + 1];

	fu1(I0,0,OKFDD_P_O)
          root_table[I0] = toprime[All_Prime[pi_limit-1-I0]]->root;
        root_table[OKFDD_P_O] = NULL;

	elements = OKFDD_1_PC_mult(root_table,Shared,dumpfilename);
        delete[] root_table;
        return elements;
};
/* ========================================================================= */





/* ========================================================================= */
// Function:    dd_man::OKFDD_Traverse // Traverses OKFDD with given order   //
/* ========================================================================= */
void dd_man::OKFDD_Traverse(    utnode* root    = NULL,
				uchar   order   = T_Preorder,
				travin  functin = NULL,
				travex  functex = NULL)
{
	utnode** root_table = new utnode*[2];
	
	root_table[0] = root;
	root_table[1] = NULL;

	OKFDD_Traverse_mult(root_table,order,functin,functex);

	delete[] root_table;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Traverse_mult // Traverses hierarchy tree      //
/* ========================================================================= */
void dd_man::OKFDD_Traverse_mult(       utnode**  root_table   	= NULL,
					uchar	  order	   	= T_Preorder,
					travin	  functin	= NULL,
					travex	  functex	= NULL)
{
	short entry;

	entry = 0; flagx = TRUE;

	while((uthelp = root_table[entry++]) != NULL)
	{
		if (um_selh((m_and(uthelp))->flag) != flagx    )
		if ((m_and(uthelp))                != OKFDD_ONE)
		Traverse_slave(uthelp,order,functin,functex);
	}


	entry = 0; flagx = FALSE;

	while((uthelp = root_table[entry++]) != NULL)
	{
		if (um_selh((m_and(uthelp))->flag) != flagx    )
		if ((m_and(uthelp))                != OKFDD_ONE)
      Traverse_slave(uthelp,order,&dd_man::Trav_Reset,NULL);
	}
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Traverse_all // Traverses all OKFDDs	     //
/* ========================================================================= */
void dd_man::OKFDD_Traverse_all(uchar   order   = T_Preorder,
				travin  functin = NULL,
				travex  functex = NULL      )
{
	utnode** root_table = new utnode*[OKFDD_P_O + 1];

	fu1(I0,0,OKFDD_P_O)
	{
		root_table[I0] = toprime[All_Prime[pi_limit-1-I0]]->root;
	}
	
       	root_table[OKFDD_P_O] = NULL;
	
	OKFDD_Traverse_mult(root_table,order,functin,functex);

	delete[] root_table;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Traverse_slave // Traverses hierarchy tree           //
/* ========================================================================= */
void dd_man::Traverse_slave(    utnode* root    = NULL,
				uchar   order   = T_Preorder,
				travin  functin = NULL,
				travex  functex = NULL      )
{
	utnode* mroot = m_and(root);

	switch(order)
	{
	case T_Preorder:

		if (functin != NULL) (this->*functin)(root);
		if (functex != NULL) (*functex)(this,root);

		if (um_selh((m_and(mroot->lo_p))->flag) != flagx    )
		if ((m_and(mroot->lo_p))                != OKFDD_ONE)
		Traverse_slave(mroot->lo_p,order,functin,functex);
		
		if (um_selh((m_and(mroot->hi_p))->flag) != flagx    )
		if ((m_and(mroot->hi_p))                != OKFDD_ONE)
		Traverse_slave(mroot->hi_p,order,functin,functex);

		break;

	case T_Inorder:

		if (um_selh((m_and(mroot->lo_p))->flag) != flagx    )
		if ((m_and(mroot->lo_p))                != OKFDD_ONE)
		Traverse_slave(mroot->lo_p,order,functin,functex);

		if (functin != NULL) (this->*functin)(root);
		if (functex != NULL) (*functex)(this,root);

		if (um_selh((m_and(mroot->hi_p))->flag) != flagx    )
		if ((m_and(mroot->hi_p))                != OKFDD_ONE)
		Traverse_slave(mroot->hi_p,order,functin,functex);

		break;

	case T_Postorder:

		if (um_selh((m_and(mroot->lo_p))->flag) != flagx    )
		if ((m_and(mroot->lo_p))                != OKFDD_ONE)
		Traverse_slave(mroot->lo_p,order,functin,functex);
		
		if (um_selh((m_and(mroot->hi_p))->flag) != flagx    )
		if ((m_and(mroot->hi_p))                != OKFDD_ONE)
		Traverse_slave(mroot->hi_p,order,functin,functex);

		if (functin != NULL) (this->*functin)(root);
		if (functex != NULL) (*functex)(this,root);
	}

	mroot->flag = um_orh(um_andh(mroot->flag),flagx);
};
/* ========================================================================= */

/* ========================================================================= */
// Function:    dd_man::Traverse_slave2	// Slave of OKFDD_Switch_all	     //
/* ========================================================================= */
void dd_man::Traverse_slave2(   utnode* root    = NULL,
				uchar   order   = T_Preorder,
				travin  functin = NULL,
				travex  functex = NULL      )
{
	utnode* mroot = m_and(root);

	switch(order)
	{
	case T_Preorder:

		if (functin != NULL) (this->*functin)(root);
		if (functex != NULL) (*functex)(this,root);

		if (um_sel((m_and(mroot->lo_p))->flag) != flag     )
		if ((m_and(mroot->lo_p))               != OKFDD_ONE)
		Traverse_slave2(mroot->lo_p,order,functin,functex);
		
		if (um_sel((m_and(mroot->hi_p))->flag) != flag     )
		if ((m_and(mroot->hi_p))               != OKFDD_ONE)
		Traverse_slave2(mroot->hi_p,order,functin,functex);

		break;

	case T_Inorder:

		if (um_sel((m_and(mroot->lo_p))->flag) != flag     )
		if ((m_and(mroot->lo_p))               != OKFDD_ONE)
		Traverse_slave2(mroot->lo_p,order,functin,functex);

		if (functin != NULL) (this->*functin)(root);
		if (functex != NULL) (*functex)(this,root);

		if (um_sel((m_and(mroot->hi_p))->flag) != flag     )
		if ((m_and(mroot->hi_p))               != OKFDD_ONE)
		Traverse_slave2(mroot->hi_p,order,functin,functex);

		break;

	case T_Postorder:

		if (um_sel((m_and(mroot->lo_p))->flag) != flag     )
		if ((m_and(mroot->lo_p))               != OKFDD_ONE)
		Traverse_slave2(mroot->lo_p,order,functin,functex);
		
		if (um_sel((m_and(mroot->hi_p))->flag) != flag     )
		if ((m_and(mroot->hi_p))               != OKFDD_ONE)
		Traverse_slave2(mroot->hi_p,order,functin,functex);

		if (functin != NULL) (this->*functin)(root);
		if (functex != NULL) (*functex)(this,root);
	}

	mroot->flag = um_or(um_and(mroot->flag),flag);
};
/* ========================================================================= */


/* ========================================================================= */
// Some one-liners: (Used as subroutines of Traverse_slave)		     //
//							       		     //
// Function:    dd_man::Trav_Reset // Reset second flag bit (in next)	     //
// Function:    dd_man::Trav_Size  // Increase global node counter	     //
// Function:    dd_man::Trav_Prof  // Increase counter for node level        //
// Function:    dd_man::Trav_Supp  // Mark level as used (-> Support)	     //
// Function:    dd_man::Trav_Mark  (ONLY needed with MORE)  // Reset Marks   //
/* ========================================================================= */

void dd_man::Trav_Reset (utnode* root)
{
	(m_and(root))->flag = um_andh((m_and(root))->flag);
};
void dd_man::Trav_Reset2(utnode* root)
{
	(m_and(root))->next = m_and((m_and(root))->next);
};
void dd_man::Trav_Size  (utnode* root)
{
	I0++;
};
void dd_man::Trav_Prof  (utnode* root)
{
	OKFDD_Result_2[(m_and(root))->label_i]++;
};
void dd_man::Trav_Supp  (utnode* root)
{
	OKFDD_Result_2[(m_and(root))->label_i] = TRUE;
};
/*
void dd_man::Trav_Mark (utnode* root)
{
	(m_and(root))->MSET = NULL; (m_and(root))->MNOM = NULL;
};
*/
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Trav_Show // Show node on screen		     //
/* ========================================================================= */
void dd_man::Trav_Show(utnode*  root)
{
	utnode* mroot = m_and(root);

	cout << "Node <"; cout.width(9); cout.fill('.');

	cout << mroot->idnum   << "-"; cout.width(5); cout.fill('0');
	cout << mroot->label_i << " ";

	switch (OKFDD_PI_DTL_Table[mroot->label_i])
	{
		case D_Shan: cout << "S"; break;
		case D_posD: cout << "P"; break;
		case D_negD: cout << "N";
	}

	cout << " >";

	cout << "   Lo ["; cout.width(9); cout.fill('.');

	cout << (m_and(mroot->lo_p))->idnum   << "-";
	cout.width(5); cout.fill('0');
	cout << (m_and(mroot->lo_p))->label_i << " ";

	if ((m_and(mroot->lo_p)) == OKFDD_ONE) cout << "T";
	else
	{
		switch (OKFDD_PI_DTL_Table[(m_and(mroot->lo_p))->label_i])
		{
			case D_Shan: cout << "S"; break;
			case D_posD: cout << "P"; break;
			case D_negD: cout << "N"; break;
		}
	}

	cout << " ";
	
	if (m_sel(mroot->lo_p) == 1) cout << "*]"; else cout << " ]";

	cout << " - Hi ["; cout.width(9); cout.fill('.');

	cout << (m_and(mroot->hi_p))->idnum   << "-";
	cout.width(5); cout.fill('0');
	cout << (m_and(mroot->hi_p))->label_i << " ";

	if ((m_and(mroot->hi_p)) == OKFDD_ONE) cout << "T";
	else
	{
		switch (OKFDD_PI_DTL_Table[(m_and(mroot->hi_p))->label_i])
		{
			case D_Shan: cout << "S"; break;
			case D_posD: cout << "P"; break;
			case D_negD: cout << "N"; break;
		}
	}

	cout << " ";
	
	if (m_sel(mroot->hi_p) == 1) cout << "*]"; else cout << " ]";

	cout << "\n";
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Switch_all // Rec. CE-switching for S,P -> N   //
/* ========================================================================= */
void dd_man::OKFDD_Switch_all()
{
	// ----------------------------------------------
	// Switch marks recursively (for all POs and TFs)
	// ----------------------------------------------
	fu1(I0,0,OKFDD_P_O)
	{
		if ((uthelp = toprime[All_Prime[pi_limit-1-I0]]->root) != NULL)
		{
			if (um_sel((m_and(uthelp))->flag) == FALSE    )
			if ((m_and(uthelp))               != OKFDD_ONE)
			Switch_slave(uthelp);

			// If returned from Switch_slave (or never visited)
			// to_node_edge maybe in need of switch -> So do it
			// ------------------------------------------------
			if (m_sel((m_and(uthelp))->next) == TRUE)
			{
			     toprime[All_Prime[pi_limit-1-I0]]->root =
			     m_xor(toprime[All_Prime[pi_limit-1-I0]]->root,1);
			}
		}
	}

	fu1(I0,0,maxt_i)
	{
		if ((uthelp = toprime[T_Prime[I0]]->root) != NULL)
		{
			if (um_sel((m_and(uthelp))->flag) == FALSE    )
			if ((m_and(uthelp))               != OKFDD_ONE)
			Switch_slave(uthelp);

			// If returned from Switch_slave (or never visited)
			// to_node_edge maybe in need of switch -> So do it
			// ------------------------------------------------
			if (m_sel((m_and(uthelp))->next) == TRUE)
			{
			     toprime[T_Prime[I0]]->root =
			     m_xor(toprime[T_Prime[I0]]->root,1);
			}
		}
	}

	// ------------------------------------
	// Reset control flags in visited nodes
	// ------------------------------------
	fu1(I0,0,OKFDD_P_O)
	{
		if ((uthelp = toprime[All_Prime[pi_limit-1-I0]]->root) != NULL)
		{
			flag = FALSE;

			if (um_sel((m_and(uthelp))->flag) != FALSE    )
			if ((m_and(uthelp))               != OKFDD_ONE)
        Traverse_slave2(uthelp,T_Postorder,&dd_man::Trav_Reset2);
		}
	}

	fu1(I0,0,maxt_i)
	{
		if ((uthelp = toprime[T_Prime[I0]]->root) != NULL)
		{
			flag = FALSE;

			if (um_sel((m_and(uthelp))->flag) != FALSE    )
			if ((m_and(uthelp))               != OKFDD_ONE)
        Traverse_slave2(uthelp,T_Postorder,&dd_man::Trav_Reset2);
		}
	}

// Added:   11.08.95
// Removed: 19.08.95

/*   Clear Computed Table

	fu1(I0,0,ct_hashsize*2)
     	{
	  	cthelp = ct[I0];

	  	fui(anchor,1,ctl[I0])
	  	{
	       		cthelp->R = (utnode*)cc_last; cc_last = cthelp;
	       		cthelp = cthelp->next;
	  	}

	  	OKFDD_No_CTNodes -= ctl[I0];
	  	ctl[I0] = 0;
     	}
*/

// Recplacement for upper part in Switch_slave
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::Switch_slave // Trav. hierarchy tree to switch marks //
/* ========================================================================= */
void dd_man::Switch_slave(utnode* root)
{
	root = m_and(root);

	if (um_sel((m_and(root->lo_p))->flag) == FALSE    )
	if ((m_and(root->lo_p))               != OKFDD_ONE)
	Switch_slave(m_and(root->lo_p));
	
	if (um_sel((m_and(root->hi_p))->flag) == FALSE    )
	if ((m_and(root->hi_p))               != OKFDD_ONE)
	Switch_slave(m_and(root->hi_p));

	if (m_sel((m_and(root->lo_p))->next) == TRUE)
	root->lo_p = m_xor(root->lo_p,1);
	if (m_sel((m_and(root->hi_p))->next) == TRUE)
	root->hi_p = m_xor(root->hi_p,1);

	// Normalization -> Switch complement marks if needed
	// --------------------------------------------------
	if (m_sel(root->lo_p) == 1)
	{
		root->lo_p = m_xor(root->lo_p,1);
		root->next =  m_or(root->next,1);

		if (OKFDD_PI_DTL_Table[root->label_i] == D_Shan)
		root->hi_p = m_xor(root->hi_p,1);
		
		// Following lines added: 19.08.95
		
		// Switch switch_flag to guarantee correct computed table
		// entries without tracing the wrong ones
		// ------------------------------------------------------
		root->switch_flag = root->switch_flag ^ 1;
	}

	root->flag = um_or(root->flag,1);
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Change_DTL // Switch DT for given variable     //
/* ========================================================================= */
utnode* dd_man::OKFDD_Change_DTL(utnode* root)
{
	utnode* mroot;
	utnode* lo;
	utnode* hi;

	// Below label of interest ?
	// -------------------------
	if (OKFDD_PI_Level_Table[(mroot = m_and(root))->label_i] >
        OKFDD_PI_Level_Table[J2]) return root;


	// -------------------------------------------
	// Get sons of equivalent node with changed DT
        // (Move marks for old DTL-Interpretation)
	// -------------------------------------------
	lo = OKFDD_Change_DTL(m_xor(mroot->lo_p,m_sel(root)));

	// ----------------------------------------
	// Label of interest reached -> Change type
	// ----------------------------------------
	if (mroot->label_i == (usint)J2)
	{
	   // Get high son (using oldtype interpretation)
	   // -------------------------------------------
	   if (J3 == D_Shan) hi = OKFDD_Change_DTL(m_xor(mroot->hi_p,
                                  m_sel(root)));
	   else	             hi = OKFDD_Change_DTL(mroot->hi_p);

	   // Remember: newtype must be (and is) unequal to actual type of J2
           // (So everything is fine and complete)
	   // J3 holds old type of J2 / J4 holds new type of J2
           // (which is already set)
	   // ---------------------------------------------------------------
	   switch (J3)
	   {
	      case D_Shan: if (J4 == D_negD)
                           { newlo = hi; newhi = CO_XOR(lo,hi); }
			   else { newlo = lo; newhi = CO_XOR(lo,hi); }
			   break;

	      case D_posD: if (J4 == D_Shan)
                           { newlo = lo; newhi = CO_XOR(lo,hi); }
			   else	{ newhi = hi; newlo = CO_XOR(lo,hi); }
			   break;

	      case D_negD: if (J4 == D_Shan)
                           { newhi = lo; newlo = CO_XOR(lo,hi); }
			   else	{ newhi = hi; newlo = CO_XOR(lo,hi); }
	   }
	   return root = FOAUT((usint)J2,newhi,newlo);
	}
	else
	{
	   if (OKFDD_PI_DTL_Table[mroot->label_i] == D_Shan)
           hi = OKFDD_Change_DTL(m_xor(mroot->hi_p,m_sel(root)));
	   else  hi = OKFDD_Change_DTL(mroot->hi_p);

	   return root = FOAUT(mroot->label_i,hi,lo);
	}
};
/* ========================================================================= */



/*****************************************************************************/
//
//		     >>>     Zero-Suppressed-BDD Routines    <<<
//
/*****************************************************************************/

/* ========================================================================= */
// Function:    dd_man::OKFDD_to_ZSBDD // Transforms OKFDD to ZSBDD	     //
/* ========================================================================= */
utnode* dd_man::OKFDD_to_ZSBDD(utnode* root)
{
	usint loop;
	usint limit = OKFDD_P_I;

	// Check for initialization of ZSBDD usage
        // No -> Make copy of all existings PIs plus modified labels for
        // each ex. Shannon-label
	// -------------------------------------------------------------
	if (ZSBDD_Init_done != TRUE)
	{
	   // Make (pos / neg) copies of each label
	   // -------------------------------------
	   fu1(loop,0,limit)
           { ZSBDD_Copy(loop); if (OKFDD_Error != Err_No_Error) return NULL; }

	   ZSBDD_Init_done = TRUE; Support_complete();
	}

	// All labels exist -> Now traverse Postorder given OKFDD and
        // construct ZSBDD version
	// ----------------------------------------------------------
	uthelp = OKFDD_to_ZSBDD_Slave(NULL,root); ups(uthelp); return uthelp;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::ZSBDD_Copy() // Inits ZSBDD usage		     //
/* ========================================================================= */
void dd_man::ZSBDD_Copy(usint prime_index)
{
	usint lab;

	OKFDD_Error = Err_No_Error;

	// Modify name of existing variable (ending is + or -) for new label
	// -----------------------------------------------------------------
	fu1(J3,0,name_limit) gl_name[J3] = ' ';
	fu1(J3,0,name_limit)
        if ((gl_name[J3] = toprime[All_Prime[prime_index]]->name[J3]) == ' ')
          break;
        gl_name[J3] = '+';

	lab = (Make_Prime(PI))->label_i;

	// Check error flag
	// ----------------
	if (OKFDD_Error != Err_No_Error)
	{
	   cout(2) << "ZSBDD_Copy: Primary limit reached -> ";
           cout(2) << "Unable to create new labels for ZSBDD tree ...\n";
           return;
	}

	OKFDD_PI_Level_Table[lab] = 2 * lab; OKFDD_PI_DTL_Table[lab] = D_posD;

	// Link from original to copied variable needed
	// --------------------------------------------
	ZSBDD_equals[All_Prime[prime_index]] = lab;

	// -------------------
	// Create modified one
	// -------------------
	gl_name[J3] = '-';      // J3 stills holds index of last name character

	lab = (Make_Prime(PI))->label_i;

	// Check error flag
	// ----------------
	if (OKFDD_Error != Err_No_Error)
	{
	   cout(2) << "ZSBDD_Copy: Primary limit reached -> ";
           cout(2) << "Unable to create new labels for ZSBDD tree ...\n";
           return;
	}

	OKFDD_PI_Level_Table[lab] = 2 * ZSBDD_equals[All_Prime[prime_index]]-1;
        OKFDD_PI_DTL_Table[lab] = D_posD;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_to_ZSBDD_Slave // Traverse hierarchy tree	     //
/* ========================================================================= */
utnode* dd_man::OKFDD_to_ZSBDD_Slave(utnode* mfather,utnode* root)
{
	long	 l2;
	long	 l3;
	utnode*	 mroot;
	utnode*	 lo;
	utnode*	 hi;
	static   short   loop;

	// No terminal case ?
	// ------------------
	if ((mroot = m_and(root)) != OKFDD_ONE)
	{
	   // -----------------------------------------------------------
	   // Calculate sons of equivalent node with ZSBDD interpretation
	   // -----------------------------------------------------------
	   lo = OKFDD_to_ZSBDD_Slave(mroot,m_xor(mroot->lo_p,m_sel(root)));

	   if (OKFDD_PI_DTL_Table[mroot->label_i] == D_Shan)
	   {
	      hi = OKFDD_to_ZSBDD_Slave(mroot,m_xor(mroot->hi_p,m_sel(root)));

	      if (hi == OKFDD_ZERO)
	      {
		 if (lo == OKFDD_ZERO) root = OKFDD_ONE;
		 else root = FOAUT(ZSBDD_equals[mroot->label_i]+1,lo,
                   OKFDD_ONE);
	      }
	      else
	      {
		 if (lo == OKFDD_ZERO) root = FOAUT(ZSBDD_equals[
                   mroot->label_i],hi,OKFDD_ONE);
		 else
		 {
		    uthelp  = FOAUT(ZSBDD_equals[mroot->label_i],hi,OKFDD_ONE);
		    root    = FOAUT(ZSBDD_equals[mroot->label_i]+1,lo,uthelp);
		 }
	      }
	      root = m_xor(root,1);
	    }
	    else
	    {
	      hi = OKFDD_to_ZSBDD_Slave(mroot,mroot->hi_p);

	      root = m_xor(FOAUT(ZSBDD_equals[mroot->label_i],hi,
                m_xor(lo,m_sel(lo))),m_sel(lo));
	    }
	}

	// --------------------------------------------------------------
	// root holds root of constructed subtree / Now check for missing
        // labels between his root and root's father (and build bridge)
	// --------------------------------------------------------------
	if (root == OKFDD_ZERO) return OKFDD_ZERO;

	// Get position of father label in OKFDD_PI_Order_Table
        // (level of variable) then position of root label
	// ----------------------------------------------------
	if (mfather == NULL) l2 = -1;
        else fu1(l2,0,OKFDD_P_I) if (OKFDD_PI_Order_Table[l2] ==
               mfather->label_i) break;
	fu1(l3,l2+1,OKFDD_P_I)
        if (OKFDD_PI_Order_Table[l3] == mroot->label_i) break;

	/* mroot is the root of the old tree (its label must be searched) */

	// ----------------------------------------------------------------
	// Some vars missing (difference > 1) -> Check OKFDD_PI_Order_Table
        // between father and root label for Shannon (and insert them)
	// ----------------------------------------------------------------
	if ((l3 - l2) > 1) fd(loop,l3-1,l2+1)
        if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[loop]] == D_Shan)
	{
	   uthelp = FOAUT(ZSBDD_equals[OKFDD_PI_Order_Table[loop]],root,
             OKFDD_ZERO);
	   root   = FOAUT(ZSBDD_equals[OKFDD_PI_Order_Table[loop]]+1,root,
             uthelp);
	}

	return root;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::ZSBDD_Multi_Op // ZSBDD Operation launcher	     //
/* ========================================================================= */
utnode* dd_man::ZSBDD_Multi_Op( usint   Code,
				utnode* F,
				utnode* G       )
{
	// Check for NULL-Operands
	// -----------------------
	if ((F == NULL) || (G == NULL))
           { OKFDD_Error = Err_NULL_Op; return OKFDD_Root[0] = NULL; }
        else OKFDD_Error = Err_No_Error;

	// Save operands (Increase reference counters)
	// -------------------------------------------
	ups(F); ups(G);

	// Call synthesis procedure
	// ------------------------
	switch (Code)
	{
	   case ZS_Union:  OKFDD_Root[0] = ZS_CO_M(Code,F,G);      break;
	   case ZS_Intsec: OKFDD_Root[0] = ZS_CO_M(Code,F,G);      break;
	   case ZS_Diff:   OKFDD_Root[0] = ZS_CO_M(Code,F,G);      break;

	   default:	   dos(F); dos(G); OKFDD_Error = Err_Unknown_Op;
                           return OKFDD_Root[0] = NULL;
	}

	// Reset operands, save result and return it
	// -----------------------------------------
	dos(F); dos(G); ups(OKFDD_Root[0]); return OKFDD_Root[0];
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::ZSBDD_Single_Op	// ZSBDD Operation launcher	     //
/* ========================================================================= */
utnode* dd_man::ZSBDD_Single_Op(usint   Code,
				usint   v,
				utnode* F       )
{
	// Check for NULL-Operands
	// -----------------------
	if ((v == 0) || (F == NULL))
           { OKFDD_Error = Err_NULL_Op; return OKFDD_Root[0] = NULL; }
        else OKFDD_Error = Err_No_Error;

	// Save operands (Increase reference counters)
	// -------------------------------------------
	ups(F);

	// Call synthesis procedure
	// ------------------------
	switch (Code)
	{
	   case ZS_Sub1:   OKFDD_Root[0] = ZS_CO_S(Code,v,F);      break;
	   case ZS_Sub0:   OKFDD_Root[0] = ZS_CO_S(Code,v,F);      break;
	   case ZS_Change: OKFDD_Root[0] = ZS_CO_S(Code,v,F);      break;

	   default:	   dos(F); OKFDD_Error = Err_Unknown_Op;
                           return OKFDD_Root[0] = NULL;
	}

	// Reset operands, save result and return it
	// -----------------------------------------
	dos(F); ups(OKFDD_Root[0]); return OKFDD_Root[0];
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::ZS_CO_S	// Zero suppressed BDD operations w. 1 arg.  //
/* ========================================================================= */
utnode* dd_man::ZS_CO_S(usint   Code,
			usint   v,
			utnode* F       )
{
	utnode* low;
	utnode* high;
	utnode* R;

	utnode* Pure_F  = m_and(F);
	uchar   flag_F  = m_sel(F);

	J0 = 0; if ((R = CTLO(Code,F,OKFDD_ZERO)) != NULL) return R;

	switch (Code)
	{
	case ZS_Change: // Return F when variable given in v is inverted
			// ---------------------------------------------

	      if (OKFDD_PI_Level_Table[Pure_F->label_i] ==
                OKFDD_PI_Level_Table[v])
	      {
		 low  = Pure_F->hi_p;
		 high = m_xor(Pure_F->lo_p,flag_F);
                 if (high == OKFDD_ZERO) return low;
	      }
	      else
	      {
		 if (OKFDD_PI_Level_Table[Pure_F->label_i] >
                   OKFDD_PI_Level_Table[v]) { low = OKFDD_ZERO; high = F; }
		 else
		 {
		    v    = Pure_F->label_i;
		    low  = ZS_CO_S(Code,v,m_xor(Pure_F->lo_p,flag_F));
		    high = ZS_CO_S(Code,v,Pure_F->hi_p);
		    if (high == OKFDD_ZERO) return low;
		 }
	      }
	      break;

	case ZS_Sub1:   // Return the subset of F such as variable v is ONE
			// ------------------------------------------------

	      if (OKFDD_PI_Level_Table[Pure_F->label_i] ==
                OKFDD_PI_Level_Table[v])
              { return Pure_F->hi_p; }
	      else
	      {
		 if (OKFDD_PI_Level_Table[Pure_F->label_i] >
                   OKFDD_PI_Level_Table[v]) { return OKFDD_ZERO; }
		 else
		 {
		    v    = Pure_F->label_i;
		    low  = ZS_CO_S(Code,v,m_xor(Pure_F->lo_p,flag_F));
		    high = ZS_CO_S(Code,v,Pure_F->hi_p);
		    if (high == OKFDD_ZERO) return low;
		 }
	      }
	      break;

	case ZS_Sub0:   // Return the subset of F such as variable v is ZERO
			// -------------------------------------------------

	      if (OKFDD_PI_Level_Table[Pure_F->label_i] ==
                OKFDD_PI_Level_Table[v])
              { return m_xor(Pure_F->lo_p,flag_F); }
	      else
	      {
		 if (OKFDD_PI_Level_Table[Pure_F->label_i] >
                   OKFDD_PI_Level_Table[v]) { return F; }
		 else
		 {
		    v    = Pure_F->label_i;
		    low  = ZS_CO_S(Code,v,m_xor(Pure_F->lo_p,flag_F));
		    high = ZS_CO_S(Code,v,Pure_F->hi_p);
		    if (high == OKFDD_ZERO) return low;
		 }
	      }
	      break;

	default:
            
              low = high = OKFDD_ZERO;
              /* Case can't be reached if called from ZSBDD_Single_Op */
	}

	// find or add (v,high,low) in unique table
	// ----------------------------------------
	R = FOAUT(v,high,low);

	// insert (Code,F,G,R) in computed table
	// -------------------------------------
	J0 = 0; ICT(Code,F,OKFDD_ZERO,R);

	return R;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::ZS_CO_M	// Zero suppressed BDD operations w. 2 args  //
/* ========================================================================= */
utnode* dd_man::ZS_CO_M(usint   Code,
			utnode* F,
			utnode* G       )
{
	usint   v;
	utnode* low;
	utnode* high;
	utnode* R;

	utnode* Pure_F  = m_and(F);
	utnode* Pure_G  = m_and(G);

	uchar   flag_F  = m_sel(F);
	uchar   flag_G  = m_sel(G);


	// Determine terminal cases according operation code (Code)
	// --------------------------------------------------------
	switch (Code)
	{
	case ZS_Union:  // Calculates union of F and G
			// ---------------------------

	      if (F == OKFDD_ZERO) return G;
	      if (G == OKFDD_ZERO) return F;
	      if (F == G)	   return F;

	      if (Pure_F->label_i == Pure_G->label_i) J0 = ct_hashsize;
	      else				      J0 = 0;

	      if ((R = CTLO(Code,F,G)) != NULL) return R;

	      if (OKFDD_PI_Level_Table[Pure_F->label_i] ==
                OKFDD_PI_Level_Table[v = Pure_G->label_i])
	      {
		 low  = ZS_CO_M(Code,m_xor(Pure_F->lo_p,flag_F),
                   m_xor(Pure_G->lo_p,flag_G));     ups(low);
		 high = ZS_CO_M(Code,Pure_F->hi_p,Pure_G->hi_p); dos(low);
                 if (high == OKFDD_ZERO) return low;
	      }
	      else
	      {
		 if (OKFDD_PI_Level_Table[Pure_F->label_i] >
                   OKFDD_PI_Level_Table[Pure_G->label_i])
		 {
		    low  = ZS_CO_M(Code,F,m_xor(Pure_G->lo_p,flag_G));
		    high = Pure_G->hi_p;
		 }
		 else
		 {
		    v    = Pure_F->label_i;
		    low  = ZS_CO_M(Code,m_xor(Pure_F->lo_p,flag_F),G);
		    high = Pure_F->hi_p;
		 }
	      }
	      break;

	case ZS_Intsec: // Calculates intersection of F and G
			// ----------------------------------

	      if (F == OKFDD_ZERO)    return OKFDD_ZERO;
	      if (G == OKFDD_ZERO)    return OKFDD_ZERO;
	      if (F == G)	     return F;

	      if (Pure_F->label_i == Pure_G->label_i) J0 = ct_hashsize;
	      else				      J0 = 0;

	      if ((R = CTLO(Code,F,G)) != NULL) return R;

	      if (OKFDD_PI_Level_Table[v = Pure_F->label_i] ==
                OKFDD_PI_Level_Table[Pure_G->label_i])
	      {
		 low  = ZS_CO_M(Code,m_xor(Pure_F->lo_p,flag_F),
                   m_xor(Pure_G->lo_p,flag_G)); ups(low);
		 high = ZS_CO_M(Code,Pure_F->hi_p,Pure_G->hi_p); dos(low);

		 if (high == OKFDD_ZERO) return low;
	      }
	      else
	      {
		 if (OKFDD_PI_Level_Table[Pure_F->label_i] >
                   OKFDD_PI_Level_Table[Pure_G->label_i])
		 {
		    return ZS_CO_M(Code,F,m_xor(Pure_G->lo_p,flag_G));
		 }
		 else
		 {
		    return ZS_CO_M(Code,m_xor(Pure_F->lo_p,flag_F),G);
		 }
	      }
	      break;

	case ZS_Diff:   // Calculates F minus G
			// --------------------

	      if (F == OKFDD_ZERO)    return OKFDD_ZERO;
	      if (G == OKFDD_ZERO)    return F;
	      if (F == G)	      return OKFDD_ZERO;

	      if (Pure_F->label_i == Pure_G->label_i) J0 = ct_hashsize;
	      else				      J0 = 0;

	      if ((R = CTLO(Code,F,G)) != NULL) return R;

	      if (OKFDD_PI_Level_Table[v = Pure_F->label_i] ==
                OKFDD_PI_Level_Table[Pure_G->label_i])
	      {
		 low  = ZS_CO_M(Code,m_xor(Pure_F->lo_p,flag_F),
                   m_xor(Pure_G->lo_p,flag_G)); ups(low);
		 high = ZS_CO_M(Code,Pure_F->hi_p,Pure_G->hi_p); dos(low);

		 if (high == OKFDD_ZERO) return low;
	      }
	      else
	      {
		 if (OKFDD_PI_Level_Table[Pure_F->label_i] > 
                   OKFDD_PI_Level_Table[Pure_G->label_i])
		 {
		    return ZS_CO_M(Code,F,m_xor(Pure_G->lo_p,flag_G));
		 }
		 else
		 {
		    low  = ZS_CO_M(Code,m_xor(Pure_F->lo_p,flag_F),G);
		    high = Pure_F->hi_p;
		 }
	      }
	      break;

	default:

              v = 0; low = high = OKFDD_ZERO;
              /* Case can't be reached if called from ZSBDD_Multi_Op */
	}

	// find or add (v,high,low) in unique table
	// ----------------------------------------
	R = FOAUT(v,high,low);

	// insert (Code,F,G,R) in computed table
	// -------------------------------------
	if (Pure_F->label_i == Pure_G->label_i) J0 = ct_hashsize;
	else				        J0 = 0;

	ICT(Code,F,G,R);

	return R;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::ZSBDD_1_PC // Returns #elements in combination set   //
/* ========================================================================= */
ulint dd_man::ZSBDD_1_PC(utnode* F)
{
	if (F == OKFDD_ZERO) return 0;
	if (F == OKFDD_ONE ) return 1;

	return ( ZSBDD_1_PC(m_xor((m_and(F))->lo_p,m_sel(F))) +
                 ZSBDD_1_PC((m_and(F))->hi_p) );
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::ZSBDD_1_PC_mult // Returns #elements in comb. sets   //
/* ========================================================================= */
ulint dd_man::ZSBDD_1_PC_mult(  utnode** root_table,
				uchar	 Shared    )
{
	usint   loop	 = 0;
	ulint   elements = 0;

	utnode* secure;
	utnode* R	 = ZSBDD_Empty;

	// Count elements separately or shared ->
        // Result is in elements and will be returned
	// ------------------------------------------
	if (Shared == TRUE)
	{
	   while(root_table[loop] != NULL)
	   { R = ZS_CO_M(ZS_Union,secure = R,root_table[loop]);
           ups(R); OKFDD_Free_Node(secure); loop++; }

	   elements = ZSBDD_1_PC(R); OKFDD_Free_Node(R);
	}
	else
	{
	   while(root_table[loop] != NULL)
	   { elements += ZSBDD_1_PC(root_table[loop]); loop++; }
	}

	return elements;
};
/* ========================================================================= */



/*****************************************************************************/
//
//		>>>     MORE (Multiple OR-Synthesis) ROUTINES   <<<
//
/*****************************************************************************/

/* ========================================================================= */
// function:    dd_man::MORE // Mult. OR (based on levelswaps & connectors)  //
/* ========================================================================= */
void dd_man::MORE(utnode** root_table)

// I0 holds number of entries ( > 1 !) ( <- searching is time consuming)
{
	// First connector has label 16
	// ----------------------------
	J1 = 32;

	// Build complete tree of connectors above root_table
	// --------------------------------------------------
	while(J0 > 1)
	{
	   // Normalization is done in FOAUT (so don`t waste time for it,
           // just connect both functions)
	   // -----------------------------------------------------------
	   J2 = 0;
           do
           {
              root_table[J2 >> 1] = FOAUT(J1,root_table[J2+1],root_table[J2]);
              J2++; J2++;
           }
           while(J2 < J0);

	   root_table[J0 = (J2 >> 1)] = OKFDD_ZERO; J1--;
	}

	J1++;

	ups(root_table[0]);

	// Create OKFDD_PI_Order_Table for levelswaps and make a copy of
        // level table (for recreating of levels)
	// -------------------------------------------------------------
	short connectors = 32 - J1 + 1;

	I1 = 0; fu1(I0,0,connectors) { OKFDD_PI_Order_Table[I1] = J1++; I1++; }

/*      fu1(I0,0,OKFDD_P_I)
        {
          OKFDD_Result_2[I0] = OKFDD_PI_Level_Table[I_Prime[I0]];
          OKFDD_PI_Order_Table[I1] = All_Prime[I0]; I1++;
        }
*/
	OKFDD_PI_Order_Table[OKFDD_Maxidx = I1] = 0;
        /* Upper line needs change when using arbitrary start order */

	utnode* secure;

	// Do Multiple OR by levelswaps
	// ----------------------------
	fd(I4,connectors,1)
	{
		if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[I4-1]] != 0)
		{
			// Shift connector to last level
			// -----------------------------
			OKFDD_Levelshift(I4-1,OKFDD_P_I+I4-1);

			I3 = OKFDD_PI_Order_Table[OKFDD_P_I+I4-1];

			// Perform reduction
			// -----------------
			secure = Reduction(root_table[0]); ups(secure);

			// Reset marks
			// -----------
/**/    //	        OKFDD_Traverse(root_table[0],&Trav_Mark,T_Postorder);
/**/    //	        OKFDD_Traverse(secure,&Trav_Mark,T_Postorder);

			// Free old DD immediately
			// -----------------------
			OKFDD_Free_Node(root_table[0]); root_table[0] = secure;
			OKFDD_Free_Fct_Cache();
		}
	}

	// Recreate pi inverse
	// -------------------
	fu1(I0,0,OKFDD_P_I)
          OKFDD_PI_Level_Table[All_Prime[I0]] = OKFDD_Result_2[I0];
	/* Upper line deeds change when using arbitrary start order */

	cout << "RESULT:\n";

	OKFDD_Traverse(root_table[0],T_Preorder,&dd_man::Trav_Show);
	OKFDD_Profile(root_table[0],TRUE);

	cout << "Size: " << OKFDD_Size(root_table[0]) << "\n";
};
/* ========================================================================= */


/* ========================================================================= */
// function:    dd_man::Reduction // Reduce OKFDD with connector label 'I3'  //
/* ========================================================================= */
utnode* dd_man::Reduction(utnode* root)
{

/*
	utnode* secure[2];
	utnode* mroot;
	utnode* lo;
	utnode* hi;

	// Check terminal cases
	// --------------------
	if ((mroot = m_and(root)) == OKFDD_ONE) return root;
	if ( mroot->label_i       == I3 )       return OKFDD_ONE;

	// ----------------------------------------------------------
	// Check if node was visited before with same mark setting ->
        // Get pointer and do northing else
	// ----------------------------------------------------------
	secure[0] = m_xor(mroot->lo_p,m_sel(root));

	if (OKFDD_PI_DTL_Table[mroot->label_i] == D_Shan)
             secure[1] = m_xor(mroot->hi_p,m_sel(root));
        else secure[1] = mroot->hi_p;

	// Check if low son is already caluculated for this mark setting
	// -------------------------------------------------------------
	if (m_sel(secure[0]) == 1) uthelp = (m_and(secure[0]))->MSET;
        else                       uthelp = (m_and(secure[0]))->MNOM;

	if (uthelp == NULL) lo = Reduction(secure[0]); else lo = uthelp;

	// Check if high son is already caluculated for this mark setting
	// --------------------------------------------------------------
	if (m_sel(secure[1]) == 1) uthelp = (m_and(secure[1]))->MSET;
        else                       uthelp = (m_and(secure[1]))->MNOM;

	if (uthelp == NULL) hi = Reduction(secure[1]); else hi = uthelp;

	// Normalization
	// -------------
	if (OKFDD_PI_DTL_Table[mroot->label_i] == D_Shan)
	{
	   if (lo == hi) uthelp = lo;    // Minimize next line
	   else	         uthelp = m_xor(FOAUT(mroot->label_i,
                           m_xor(hi,m_sel(lo)),m_xor(lo,m_sel(lo))),m_sel(lo));
	}
	else
	{
	   if (hi == OKFDD_ZERO) uthelp = lo;    // Minimize next line
	   else		         uthelp = m_xor(FOAUT(mroot->label_i,hi,
                                   m_xor(lo,m_sel(lo))),m_sel(lo));
	}

	if (m_sel(root) == 1) return mroot->MSET = uthelp;
	else		      return mroot->MNOM = uthelp;
*/

	return root;    // Remove it when using Reduction
};
/* ========================================================================= */



/*****************************************************************************/
//
//		       >>>     REORDERING ROUTINES     <<<
//
/*****************************************************************************/


/* ========================================================================= */
// Function:    dd_man::OKFDD_Levelexchange                                  //
// Vertauschen von Level und Nachfolgelevel		                     //
/* ========================================================================= */
void dd_man::OKFDD_Levelexchange(usint varpos)
{
	int     var, i;
	utnode* vater;
	utnode* head=NULL;
	utnode* next;

	// Falls Knoten in beiden Leveln vorhanden sind
	// --------------------------------------------
	if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos]] &&
          OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos+1]])
	{
	   // Sohnlevelposition und Gesamtgroesse best.
	   // -----------------------------------------
	   u_level = OKFDD_PI_Order_Table[varpos+1];
	   OKFDD_Now_size_i -=
             (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos]] +
	     OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos+1]]);

	   // Herausloesen der Leveltausch-Kandidaten (Vaeter) aus ut
	   // -------------------------------------------------------
	   for (i=0; i<ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[varpos]]];i++)
	   {
	      // Ist der Ankerplatz belegt ???
	      // -----------------------------
	      if (ut[OKFDD_PI_Order_Table[varpos]][i] != NULL)
	      {
		 // vater ist Erster oder Einziger in der Kette
		 // -------------------------------------------
		 vater = ut[OKFDD_PI_Order_Table[varpos]][i];
		 // Kette aufteilen in :
		 // --------------------
		 ut[OKFDD_PI_Order_Table[varpos]][i] = NULL;
		 while (vater != NULL)
		 {
		    // Vaeter mit Soehnen im unt.Level => OKFDD_Levelexchange()
		    // --------------------------------------------------------
		    if (vater->lo_p->label_i	  == u_level ||
		      (m_and(vater->hi_p))->label_i == u_level)
		    {
		       OKFDD_No_UTNodes_Per_Lvl[vater->label_i]--;
		       OKFDD_No_UTNodes--;
		       next	   = vater->next;
		       vater->next = head;
		       head	   = vater;
		       vater	   = next;
		    }

		    // Vaeter d.keine Soehne im unt.Level besitzen =>
                    // unbehandelt
		    // ----------------------------------------------
		    else
		    {
		       next	   = vater->next;
		       vater->next = ut[OKFDD_PI_Order_Table[varpos]][i];
		       ut[OKFDD_PI_Order_Table[varpos]][i] = vater;
		       vater	   = next;
		    }
		}
	     }
	  }

	  // entstandene Kette mutieren
	  // --------------------------
	  vater = head;
	  if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[varpos]] == D_Shan)
          {
	     if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[varpos+1]] == D_Shan)
             {
		while (vater != NULL && Overflow == FALSE)
                {
		   next = vater->next;
		   mutation_SS(m_and(vater));
		   vater = next;
		}
	     }
	     else
             {
	        while (vater != NULL && Overflow == FALSE)
                {
		   next = vater->next;
		   mutation_SD(m_and(vater));
		   vater = next;
		}
	     }
	  }
	  else
          {
	     if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[varpos+1]] == D_Shan)
             {
		while (vater != NULL && Overflow == FALSE)
                {
		   next = vater->next;
		   mutation_DS(m_and(vater));
		   vater = next;
		}
	     }
	     else
             {
		while (vater != NULL && Overflow == FALSE)
                {
		   next = vater->next;
		   mutation_DD(m_and(vater));
		   vater = next;
		}
	     }
	  }

	  // Neue Gesamtgroesse best.
	  // ------------------------
	  OKFDD_Now_size_i +=
            (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos]] +
	    OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos+1]]);
	}

	// Reihenfolgenlisten anpassen
	// ---------------------------
	var			       = OKFDD_PI_Order_Table[varpos];
	OKFDD_PI_Order_Table[varpos]   = OKFDD_PI_Order_Table[varpos+1];
	OKFDD_PI_Order_Table[varpos+1] = var;

	var = OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[varpos]];
	OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[varpos]] =
         OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[varpos+1]];
	OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[varpos+1]] = var;


	// ut-Groessen an geaenderte Knotenzahlen anpassen
	// -----------------------------------------------
	if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos]]*10 
	  > ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[varpos]]] *
          OKFDD_Explodefactor &&
          ut_hashsize[OKFDD_PI_Order_Table[varpos]] < ut_max)
        {
/*      if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos]] >
          ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[varpos]]] * 2)
        {
*/
//      if (ut_hashsize[OKFDD_PI_Order_Table[varpos]] < ut_max)
	{
	   K0 = ut_hashsize[OKFDD_PI_Order_Table[varpos]]+1;
	   OKFDD_Resize_ut(OKFDD_PI_Order_Table[varpos]);
	} }
	else if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos]]*10 
	       < ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[varpos]]] *
               OKFDD_Implodefactor &&
	       ut_hashsize[OKFDD_PI_Order_Table[varpos]] > 0)
        {
/*      else if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos]]*2 <
               ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[varpos]]])
        {
*/
//      if (ut_hashsize[OKFDD_PI_Order_Table[varpos]] > 0)
	{
	   K0 = ut_hashsize[OKFDD_PI_Order_Table[varpos]]-1;
	   OKFDD_Resize_ut(OKFDD_PI_Order_Table[varpos]);
	} }

	if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos+1]]*10 
	  > ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[varpos+1]]] *
          OKFDD_Explodefactor &&
	  ut_hashsize[OKFDD_PI_Order_Table[varpos+1]] < ut_max)
        {
/*      if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos+1]] >
          ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[varpos+1]]] * 2)
        {
*/
//      if(ut_hashsize[OKFDD_PI_Order_Table[varpos+1]] < ut_max)
	{
	   K0 = ut_hashsize[OKFDD_PI_Order_Table[varpos+1]]+1;
	   OKFDD_Resize_ut(OKFDD_PI_Order_Table[varpos+1]);
	} }
	else if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos+1]]*10 
	       < ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[varpos+1]]] *
               OKFDD_Implodefactor &&
	       ut_hashsize[OKFDD_PI_Order_Table[varpos+1]] > 0)
        {
/*      else if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[varpos+1]]*2 <
               ut_sizes[ut_hashsize[OKFDD_PI_Order_Table[varpos+1]]])
        {
*/
//      if(ut_hashsize[OKFDD_PI_Order_Table[varpos+1]] > 0)
	{
	   K0 = ut_hashsize[OKFDD_PI_Order_Table[varpos+1]]-1;
	   OKFDD_Resize_ut(OKFDD_PI_Order_Table[varpos+1]);
	} }
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_Levelswap	// Vert. zweier bel. Level im DD     //
/* ========================================================================= */
void dd_man::OKFDD_Levelswap(   usint level1,
				usint level2)
{
	int sift;

	min_count = 1;
	OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	// Variableninhalte vertauschen falls level1 groesser als level2
	// -------------------------------------------------------------
	if (level1 > level2)
	{
	   sift   = level1;
	   level1 = level2;
	   level2 = sift;
	}

	// Level vertauschen
	// -----------------
	for (sift = level1  ; sift < level2 ;sift++) OKFDD_Levelexchange(sift);
	for (sift = level2-2; sift >= level1;sift--) OKFDD_Levelexchange(sift);
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Levelshift // Verschiebt 'von' -> 'nach'	     //
/* ========================================================================= */
void dd_man::OKFDD_Levelshift(  usint von,
				usint nach)
{
	int sift;

	if (von < nach) for (sift = von  ; sift < nach  ; sift++)
          OKFDD_Levelexchange(sift);
	else for (sift = von-1; sift >= nach ; sift--)
          OKFDD_Levelexchange(sift);
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Blockshift                                     //
// Verschiebt Block der Laenge len 'von' -> 'nach'	                     //
/* ========================================================================= */
void dd_man::OKFDD_Blockshift(  usint von,
				usint nach,
				usint len )
{
	int var, sift, nacha, vona;

	// Falls von kleiner als nach => neue Blockparameter best.
	// -------------------------------------------------------
	if (von > nach)
	{
	   vona  = von;
	   nacha = nach;
	   von   = nacha;
	   nach  = vona + len - 1;
	   len   = vona - nacha;
	}

	// Block verschieben
	// -----------------
	for (var = von+len-1 ; var >= von ; var--)
        for (sift = var ; sift < nach+var-von-len+1 ; sift++)
          OKFDD_Levelexchange(sift);
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DD_to_BDD	                                     //
// Transformation bel. DDs in BDD / 1 = alte Ord. , 0 = zufaellige Ord.	     //
/* ========================================================================= */
void dd_man::OKFDD_DD_to_BDD(   usint von,
				usint bis,
				char order)
{
	int var, i;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b|ros_a)).flush();
//      if (OKFDD_Outputflags & (ros_b|ros_a)) cout.flush();

	// Initialisierung der Hilfsvariablen
	// ----------------------------------
	min_count =OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	// alte Ordnung
	// ------------
	if (order)
	{
	   for (var = von; var <= bis; var++)
	   {
	      if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_posD)
              {
		for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		P2S(OKFDD_Maxidx-1);
	        for (i = OKFDD_Maxidx-2; i >= var; i--) OKFDD_Levelexchange(i);
	      }
	      else if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_negD)
              {
		for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		N2S(OKFDD_Maxidx-1);
		for (i = OKFDD_Maxidx-2; i >= var; i--) OKFDD_Levelexchange(i);
	      }

	      gesamtkosten(0,OKFDD_Maxidx-1);
	   }
	}

	// Zufaellige Ordnung
	// ------------------
	else
        {
	   for (var = bis; var >= von ; var--)
	   {
	      if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_posD)
              {
	        for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		P2S(OKFDD_Maxidx-1);
	      }
	      else if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_negD)
              {
		for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		N2S(OKFDD_Maxidx-1);
	      }

	      gesamtkosten(0,OKFDD_Maxidx-1);
	   }
	}
	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DD_to_FDD		                             //
// Transformation bel. DDs in FDD / 1 = alte Ord., 0 = zufaellige Ord.       //
/* ========================================================================= */
void dd_man::OKFDD_DD_to_FDD(   usint von,
				usint bis,
				char order)
{
	int var, i;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Initialisierung der Hilfsvariablen
	// ----------------------------------
	min_count =OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	// alte Ordnung
	// ------------
	if (order)
	{
	   for (var = von; var <= bis ; var++)
	   {
	      if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_Shan)
              {
		for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
	        S2P(OKFDD_Maxidx-1);
		for (i = OKFDD_Maxidx-2; i >= var; i--) OKFDD_Levelexchange(i);
	      }
	      else if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_negD)
              {
		for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
	        N2P(OKFDD_Maxidx-1);
		for (i = OKFDD_Maxidx-2; i >= var; i--) OKFDD_Levelexchange(i);
	      }

	      gesamtkosten(0,OKFDD_Maxidx-1);
	   }
	}

	// Zufaellige Ordnung
	// ------------------
	else
        {
	   for (var = bis; var >= von ; var--)
	   {
	      if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_Shan)
              {
		 for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		 S2P(OKFDD_Maxidx-1);
	      }
	      else if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_negD)
              {
		 for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		 N2P(OKFDD_Maxidx-1);
	      }

	      gesamtkosten(0,OKFDD_Maxidx-1);
	   }
	}
	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DD_to_NDD		                             //
// Transformation bel. DDs in NDD / 1 = alte Ord., 0 = zufaellige Ord.       //
/* ========================================================================= */
void dd_man::OKFDD_DD_to_NDD(   usint von,
				usint bis,
				char order)
{
	int var, i;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Initialisierung der Hilfsvariablen
	// ----------------------------------
	min_count =OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	// alte Ordnung
	// ------------
	if (order)
	{
	   for (var = von; var <= bis ; var++)
	   {
	      if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_posD)
              {
		for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		P2N(OKFDD_Maxidx-1);
		for (i = OKFDD_Maxidx-2; i >= var; i--) OKFDD_Levelexchange(i);
	      }
	      else if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_Shan)
              {
		for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		S2N(OKFDD_Maxidx-1);
		for (i = OKFDD_Maxidx-2; i >= var; i--) OKFDD_Levelexchange(i);
	      }

	      gesamtkosten(0,OKFDD_Maxidx-1);
	   }
	}

	// Zufaellige Ordnung
	// ------------------
	else
        {
	   for (var = bis; var >= von ; var--)
	   {
	      if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_posD)
              {
		 for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		 P2N(OKFDD_Maxidx-1);
	      }
	      else if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]]==D_Shan)
              {
		 for (i = var; i < OKFDD_Maxidx-1; i++) OKFDD_Levelexchange(i);
		 S2N(OKFDD_Maxidx-1);
	      }

	      gesamtkosten(0,OKFDD_Maxidx-1);
	   }
	}
	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_scramble	                                     //
// Veraenderung d.Var.Reihenf. waehrend des Aufbaus	                     //
/* ========================================================================= */
void dd_man::OKFDD_scramble(    usint von,
				usint bis,
				char  art)
{
	int    sift, var, opos;
	uchar* newdtl = new uchar[OKFDD_Maxidx]; // help field f.storage o.DTLs

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// aktualisiere Hilfsfelder
	// ------------------------
	min_count = 1;
	for (var = 0; var < OKFDD_Maxidx; var++)
	{
	   min_pi_table[var] = twin_pi_table[var] = OKFDD_PI_Order_Table[var];
	   newdtl[OKFDD_PI_Order_Table[var]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]];
	}
	min_size_i = OKFDD_Now_size_i;

	// chaotische Positionen bestimmen
	// -------------------------------
	for (var = von; var <= bis; var++)
	{
	   if (art>1) newdtl[twin_pi_table[var]] = rand()%3;
	   if (art!=2)
           {
	      sift		  = bis-(rand()%(bis-var+1));
	      opos		  = twin_pi_table[var];
	      twin_pi_table[var]  = twin_pi_table[sift];
	      twin_pi_table[sift] = opos;
	   }
	}
	OKFDD_Setthis(twin_pi_table,newdtl);

	cout(ros_a) << OKFDD_Now_size_i << "\n";

        delete[] newdtl;
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_Siftlight			                     //
// Veraenderung der Variablenreihenfolge waehrend des Aufbaus	             //
/* ========================================================================= */
void dd_man::OKFDD_Siftlight(   usint von,
				usint bis,
				char  art)
{
	int	     sift, var, opos , belegt, frei;
	unsigned int old_no_ut[pi_limit];

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Schiebe belegte unique tables zusammen
	// --------------------------------------
	frei = von;
        while (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[frei]] &&
         frei <= bis) frei++;

	for (belegt = frei+1; belegt <= bis; belegt ++)
	{
	   if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[belegt]])
	   {
	      sift = OKFDD_PI_Order_Table[frei];
	      OKFDD_PI_Order_Table[frei] = OKFDD_PI_Order_Table[belegt];
	      OKFDD_PI_Order_Table[belegt] = sift;
	      var = OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]] =
                OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]] = var;
	      frei++;
	   }
	}
	bis = frei-1;

	// aktualisiere Hilfsfelder
	// ------------------------
	min_count = 0;
//      for (var = von; var <= bis; var++)
	for (var = 0; var < OKFDD_Maxidx; var++)
	{
	   old_no_ut[OKFDD_PI_Order_Table[var]] =
             OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]];
	   twin_pi_table[var] = OKFDD_PI_Order_Table[var];
	}
	min_size_i = OKFDD_Now_size_i;

	// suche ersten Kandidaten
	// -----------------------
	switch (art)
	{
	   case 'r':  // bestimme zufaellige Levelreihenfolge
		      // ------------------------------------
		      for (var = von; var <= bis; var++)
		      {
			 sift                = bis-(rand()%(bis-var+1));
			 opos                = twin_pi_table[var];
			 twin_pi_table[var]  = twin_pi_table[sift];
		         twin_pi_table[sift] = opos;
		      }
		      break;

	   case 'g':  // sortiere Level nach absteigender Breite
		      // ---------------------------------------
		      for (var = von; var <= bis; var++)
		      {
			 for (sift = var+1; sift <= bis; sift++)
			 {
			    if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[var]] 
			      < OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]])
			    {
			       opos                = twin_pi_table[var];
			       twin_pi_table[var]  = twin_pi_table[sift];
			       twin_pi_table[sift] = opos;
			    }
			 }
		      }
		      break;

	   case 'l':  // suche max. Level
		      // ----------------
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]] 
		        > OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]])
                        opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
		      break;

	   case 'v':  // suche vielversprechendes Level
		      // ------------------------------
		      levelstat();
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (lsf[twin_pi_table[sift]] > lsf[twin_pi_table[opos]])
                         opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
	}

	// Suche lokales Minimum fuer jede Variable
	// ----------------------------------------
	for (var = von; var <= bis; var++)
	{
	   Prozentbalken(var-von+1,bis-von+1,'l')

	   opos = von;
           while (OKFDD_PI_Order_Table[opos]!=twin_pi_table[var]) opos++;

	   // Schiebe nach oben solange Verbesserung auftritt
	   // -----------------------------------------------
	   for (sift = opos - 1; sift >= von; sift--)
	   {
	      OKFDD_Levelexchange(sift);
	      if (min_size_i < OKFDD_Now_size_i) break;
	      else min_size_i = OKFDD_Now_size_i;
	   }
	   if (sift < von) sift = von;
	   opos = sift;

	   // Schiebe zurueck
	   // --------------
	   for (sift = opos; sift < bis; sift++)
	   {
	      OKFDD_Levelexchange(sift);
	      if (min_size_i < OKFDD_Now_size_i)
              {
                   OKFDD_Levelexchange(sift); break;
              }
	      else min_size_i = OKFDD_Now_size_i;
	   }
	   kosten(von,bis);

	   // suche nächstes Level für Tausch
	   // -------------------------------
	   switch (art)
	   {
	      case 'l':  // suche max. Level
			 // ----------------
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if ((OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]]-
                           old_no_ut[twin_pi_table[sift]]) > 
			   (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]]-
                           old_no_ut[twin_pi_table[opos]])) opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
			 break;

	      case 'v':  // suche vielversprechendes Level
			 // ------------------------------
			 levelstat();
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if (lsf[twin_pi_table[sift]] >
                           lsf[twin_pi_table[opos]]) opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
	   }
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Sort	                                     //
// Sortierung d.bestehenden DDs in Reihenfolge 1-n und evtl. DTL Normierung  //
/* ========================================================================= */
void dd_man::OKFDD_Sort(	usint von,
				usint bis,
				uchar ndt)
{
	int sift, var, opos;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i<< " =>\n";
	cout(ros_b).flush();

	// aktualisiere Hilfsfelder
	// ------------------------
	min_count = 0;

	for (var = 0; var < OKFDD_Maxidx; var++)
//      for (var = von; var <= bis; var++)
	{
	   twin_pi_table[var] = min_pi_table[var]  = OKFDD_PI_Order_Table[var];
	   mindtl[OKFDD_PI_Order_Table[var]]       =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]];
	}
	min_size_i = OKFDD_Now_size_i;


	// sortiere Level nach aufsteigendem Label
	// ---------------------------------------
	for (var = von; var <= bis; var++)
	{
	   opos = var;
	   for (sift = var+1; sift <= bis; sift++) if (twin_pi_table[opos] >
             twin_pi_table[sift])
             opos = sift;

	   if (opos != var)
	   {
	      sift		  = twin_pi_table[var];
	      twin_pi_table[var]  = twin_pi_table[opos];
	      twin_pi_table[opos] = sift;
	   }
	}

	// Fuer alle Variablen vorgegebene Sortierung und DTL einnehmen
	// ------------------------------------------------------------
	for (var = von; var <= bis; var++)
	{
	   Prozentbalken(var-von+1,bis-von+1,'T')

	   opos = von;
           while (OKFDD_PI_Order_Table[opos]!=twin_pi_table[var]) opos++;

	   for (sift = opos ; sift < bis ; sift++) OKFDD_Levelexchange(sift);

	   if (ndt!=3 && OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[bis]]!=ndt)
	   {
	      for (sift = bis; sift < OKFDD_Maxidx-1; sift++)
                OKFDD_Levelexchange(sift);

	      switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	      {
		 case D_Shan :   if (ndt==D_posD) S2P(OKFDD_Maxidx-1);
				 else	          S2N(OKFDD_Maxidx-1);
				 break;
		 case D_posD :   if (ndt==D_negD) P2N(OKFDD_Maxidx-1);
				 else	          P2S(OKFDD_Maxidx-1);
				 break;
		 case D_negD :   if (ndt==D_Shan) N2S(OKFDD_Maxidx-1);
				 else	          N2P(OKFDD_Maxidx-1);
	      }
	      for (sift = OKFDD_Maxidx-2; sift >= bis; sift--)
                OKFDD_Levelexchange(sift);
	   }
	}

	// Entstandenen DD bewerten
	// ------------------------
	gesamtkosten(von,bis);
	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Sifting // Sifting			     //
/* ========================================================================= */
void dd_man::OKFDD_Sifting(     usint von,
				usint bis,
				float faktor,
				char  rel,
				char  art   )
{
	int	     sift, var, opos , mitte, abb, old_size, frei, belegt;
	unsigned int old_no_ut[pi_limit];

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Schiebe belegte unique tables zusammen
	// --------------------------------------
	frei = von;
        while (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[frei]] && frei <=
          bis) frei++;

	for (belegt = frei+1; belegt <= bis; belegt ++)
	{
	   if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[belegt]])
	   {
	      sift = OKFDD_PI_Order_Table[frei];
	      OKFDD_PI_Order_Table[frei] = OKFDD_PI_Order_Table[belegt];
	      OKFDD_PI_Order_Table[belegt] = sift;
	      var = OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]] =
                OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]] = var;
	      frei++;
	   }
	}
	bis = frei-1;

	// aktualisiere Hilfsfelder
	// ------------------------
	if (faktor < 1.01) faktor = 1.01;
	min_count = 0;
	for (var = 0; var < OKFDD_Maxidx; var++)
//      for (var = von; var <= bis; var++)
	{
	   old_no_ut[OKFDD_PI_Order_Table[var]] =
             OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]];
	   twin_pi_table[var] = min_pi_table[var] = OKFDD_PI_Order_Table[var];
	   mindtl[OKFDD_PI_Order_Table[var]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]];
	}
	old_size = min_size_i = OKFDD_Now_size_i;

	// suche ersten Kandidaten
	// -----------------------
	switch (art)
	{
	   case 'r':  // bestimme zufaellige Levelreihenfolge
		      // ------------------------------------
		      for (var = von; var <= bis; var++)
		      {
			 sift                = bis-(rand()%(bis-var+1));
			 opos		     = twin_pi_table[var];
			 twin_pi_table[var]  = twin_pi_table[sift];
			 twin_pi_table[sift] = opos;
		      }
		      break;

	   case 'g':  // sortiere Level nach absteigender Breite
		      // ---------------------------------------
		      for (var = von; var <= bis; var++)
		      {
			 for (sift = var+1; sift <= bis; sift++)
			 {
			    if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[var]] 
			      < OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]])
			    {
			       opos		   = twin_pi_table[var];
			       twin_pi_table[var]  = twin_pi_table[sift];
			       twin_pi_table[sift] = opos;
			    }
			 }
		      }
		      break;

	   case 'k':  // suche max. Level
		      // ----------------
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]] 
			> OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]])
                        opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
		      break;

	   case 'l':  // suche max. Level
		      // ----------------
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]] 
			> OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]])
                        opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
		      break;

	   case 'v':  // suche vielversprechendes Level
		      // -------------------------------
		      levelstat();
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (lsf[twin_pi_table[sift]] > lsf[twin_pi_table[opos]])
                        opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
	}

	// Suche lokales Minimum fuer jede Variable
	// ----------------------------------------
	mitte = OKFDD_Maxidx/2;
	for (var = von; var <= bis; var++)
	{

	   Prozentbalken(var-von+1,bis-von+1,'s')

	   // Vergroesserungsfaktor relativieren
	   // ----------------------------------
	   if (rel=='r') old_size = OKFDD_Now_size_i;

	   opos = von;
           while (OKFDD_PI_Order_Table[opos]!=twin_pi_table[var]) opos++;

	   // Variable steht in der oberen Haelfte des DDs =>
           // zuerst nach oben schieben
	   // -----------------------------------------------
	   if (opos < mitte)
	   {
	      // Schiebe Level in Richtung von unter Beachtung des
              // Abbruchfaktors
	      // -------------------------------------------------
	      for (sift = opos - 1; sift >= von && OKFDD_Now_size_i <
                old_size*faktor; sift--)
	      {
		 OKFDD_Levelexchange(sift);
		 kosten(von,bis);
	      }
	      abb = sift+1;

	      // zurueckstellen
	      //---------------
	      for (sift = abb; sift < opos; sift++) OKFDD_Levelexchange(sift);

	      // Schiebe Level in Richtung bis unter Beachtung des
              // Abbruchfaktors
	      // -------------------------------------------------
	      for (sift = opos; sift < bis && OKFDD_Now_size_i <
                old_size*faktor; sift++)
	      {
		 OKFDD_Levelexchange(sift);
		 kosten(von,bis);
	      }

	      // Optimale Position bestimmen und einnehmen
	      // -----------------------------------------
	      opos = abb; abb = sift-1;
	      while (min_pi_table[opos]!=twin_pi_table[var]) opos++;
	      for (sift = abb; sift >= opos; sift--) OKFDD_Levelexchange(sift);
	   }

	   // Variable steht in der unteren Haelfte des DDs => zuerst nach
           // unten schieben
	   // ------------------------------------------------------------
	   else
	   {
	      // Schiebe Level in Richtung bis unter Beachtung des
              // Abbruchfaktors
	      // -------------------------------------------------
	      for (sift = opos ; sift < bis && OKFDD_Now_size_i <
                old_size*faktor; sift++)
	      {
		 OKFDD_Levelexchange(sift);
		 kosten(von,bis);
	      }

	      // zurueckstellen
	      //---------------
	      abb = sift-1;
	      for (sift = abb; sift >= opos; sift--) OKFDD_Levelexchange(sift);

	      // Schiebe Level in Richtung von unter Beachtung des
              // Abbruchfaktors
	      // -------------------------------------------------
	      for (sift = opos - 1; sift >= von && OKFDD_Now_size_i <
                old_size*faktor; sift--)
	      {
		 OKFDD_Levelexchange(sift);
		 kosten(von,bis);
	      }
	      abb = sift+1;

	      // optimale Position bestimmen und einnehmen
	      // -----------------------------------------
	      opos = abb; abb = sift+1;
	      while (min_pi_table[opos]!=twin_pi_table[var]) opos++;
	      for (sift = abb; sift < opos; sift++) OKFDD_Levelexchange(sift);
	   }
	   kosten(von,bis);

	   // suche nächstes Level für Tausch
	   // -------------------------------
	   switch (art)
	   {
	      case 'l':  // suche max. Level
			 // ----------------
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if ((OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]]-
                           old_no_ut[twin_pi_table[sift]]) > 
			   (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]]-
                           old_no_ut[twin_pi_table[opos]]))
                           opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
			 break;

	      case 'k':  // suche max. Level
			 // ----------------
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if ((OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]]-
                           old_no_ut[twin_pi_table[sift]]) >= 
			   (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]]-
                           old_no_ut[twin_pi_table[opos]]))
                         {
			    if ((OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]]-
                              old_no_ut[twin_pi_table[sift]]) == 
			      (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]]-
                              old_no_ut[twin_pi_table[opos]]))
                            {
			       if (OKFDD_No_UTNodes_Per_Lvl[
                                 twin_pi_table[sift]]>
                                 OKFDD_No_UTNodes_Per_Lvl[
                                 twin_pi_table[opos]])
                                 opos = sift;
			    }
			    else opos=sift;
			 }
			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
			 break;

	      case 'v':  // suche vielversprechendes Level
			 // -------------------------------
			 levelstat();
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if (lsf[twin_pi_table[sift]] > 
                           lsf[twin_pi_table[opos]])
                           opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
	   }
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::dtlsift_XOR			                     //
// Sifting mit gleichzeitiger Veraenderung des Zerlegungstyps	             //
/* ========================================================================= */
void dd_man::dtlsift_XOR(usint von,
			 usint bis,
			 float faktor,
			 char  rel,
			 char  art   )
{
	int	     sift, var, opos, old_size;
	unsigned int old_no_ut[pi_limit];

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// aktualisiere Hilfsfelder
	// ------------------------
	if (faktor < 1.01) faktor = 1.01;
	min_count = 0;
	for (var = 0; var < OKFDD_Maxidx; var++)
//      for (var = von; var <= bis; var++)
	{
	   old_no_ut[OKFDD_PI_Order_Table[var]] =
             OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]];
	   twin_pi_table[var] = min_pi_table[var] = OKFDD_PI_Order_Table[var];
	   mindtl[OKFDD_PI_Order_Table[var]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]];
	}
	old_size =min_size_i = OKFDD_Now_size_i;

	// suche ersten Kandidaten
	// -----------------------
	switch (art)
	{
	   case 'r':  // Bestimme zufaellige Levelreihenfolge
		      // ------------------------------------
		      for (var = von; var <= bis; var++)
		      {
			 sift                = bis-(rand()%(bis-var+1));
			 opos		     = twin_pi_table[var];
			 twin_pi_table[var]  = twin_pi_table[sift];
			 twin_pi_table[sift] = opos;
		      }
		      break;

	   case 'g':  // sortiere Level nach absteigender Breite
		      // ---------------------------------------
		      for (var = von; var <= bis; var++)
		      {
			 for (sift = var+1; sift <= bis; sift++)
			 {
			    if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[var]] 
			      < OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]])
			    {
			       opos		   = twin_pi_table[var];
			       twin_pi_table[var]  = twin_pi_table[sift];
			       twin_pi_table[sift] = opos;
			    }
			 }
		      }
		      break;

	   case 'l':  // Suche max. Level
		      // ----------------
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]] 
			> OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]])
                        opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
		      break;

	   case 'v':  // Suche vielversprechendes Level
		      // -------------------------------
		      levelstat();
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (lsf[twin_pi_table[sift]] > lsf[twin_pi_table[opos]])
                        opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
	}

	for (var = von; var <= bis; var++)
	{

	   Prozentbalken(var-von+1,bis-von+1,'X')

	   // Vergroesserungsfaktor relativieren
	   // ----------------------------------
	   if (rel == 'r') old_size = OKFDD_Now_size_i;

	   // Variable fuer momentanen Zerlegungstyp durch DD schieben
	   // --------------------------------------------------------
	   opos = von;
           while (OKFDD_PI_Order_Table[opos]!=twin_pi_table[var]) opos++;

	   if (opos > von)
	   {
	      for (sift = opos - 1; sift >= von ; sift--)
	      {
		 OKFDD_Levelexchange(sift);
		 gesamtkosten(von,bis);
	      }
	      for (sift = von; sift < opos; sift++) OKFDD_Levelexchange(sift);
	   }

	   for (sift = opos; sift < bis ; sift++)
	   {
	      OKFDD_Levelexchange(sift);
	      gesamtkosten(von,bis);
	   }

	   // Zerlegungstyp rotieren
	   // ---------------------
	   switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[bis]])
	   {
	      case D_Shan : OKFDD_DTL_Chg_XOR(bis,D_posD); break;
	      case D_posD : OKFDD_DTL_Chg_XOR(bis,D_negD); break;
	      case D_negD : OKFDD_DTL_Chg_XOR(bis,D_Shan);
	   }
	   OKFDD_Now_size_i=0;
           for (sift = 0; sift < OKFDD_Maxidx ; sift++)
             OKFDD_Now_size_i +=
               OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[sift]];


	   // Variable fuer momentanen Zerlegungstyp durch DD schieben
	   // --------------------------------------------------------
	   gesamtkosten(von,bis);

	   for (sift = bis-1; sift >= von ; sift--)
	   {
	      OKFDD_Levelexchange(sift);
	      gesamtkosten(von,bis);
	   }

	   // Zerlegngstyp rotieren
	   // ---------------------
	   switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[von]])
	   {
	      case D_Shan : OKFDD_DTL_Chg_XOR(von,D_posD); break;
	      case D_posD : OKFDD_DTL_Chg_XOR(von,D_negD); break;
	      case D_negD : OKFDD_DTL_Chg_XOR(von,D_Shan);
	   }
	   OKFDD_Now_size_i=0;
           for (sift = 0; sift < OKFDD_Maxidx ; sift++)
             OKFDD_Now_size_i +=
               OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[sift]];

	   // Variable fuer momentanen Zerlegungstyp durch DD schieben
	   // --------------------------------------------------------
	   min_size_i++;
	   gesamtkosten2(von,bis);
	   for (sift = von; sift < bis ; sift++)
	   {
	      OKFDD_Levelexchange(sift);
	      gesamtkosten2(von,bis);
	   }

	   // Evtl. Typ aendern
	   // -----------------
	   OKFDD_DTL_Chg_XOR(bis,mindtl[OKFDD_PI_Order_Table[bis]]);
	   OKFDD_Now_size_i=0;
           for (sift = 0; sift < OKFDD_Maxidx ; sift++)
             OKFDD_Now_size_i +=
               OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[sift]];

	   // Optimale Position bestimmen und einnehmen
	   // -----------------------------------------
	   opos = von; while (min_pi_table[opos]!=twin_pi_table[var]) opos++;
	   if (opos<bis) for (sift = bis-1; sift >= opos; sift--)
             OKFDD_Levelexchange(sift);
	   gesamtkosten(von,bis);

	   // Suche nächstes Level für Tausch
	   // -------------------------------
	   switch (art)
	   {
	      case 'l':  // Suche max. Level
			 // ----------------
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if ((OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]]-
                           old_no_ut[twin_pi_table[sift]]) > 
			   (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]]-
                           old_no_ut[twin_pi_table[opos]]))
                             opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
			 break;

	      case 'v':  // Suche vielversprechendes Level
			 // -------------------------------
			 levelstat();
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if (lsf[twin_pi_table[sift]] > 
                           lsf[twin_pi_table[opos]])
                             opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
		}
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Sifting		                     //
// Sifting mit gleichzeitiger Veraenderung des Zerlegungstyps	             //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Sifting( usint von,
				usint bis,
				float faktor,
				char  rel,
				char  art   )
{
	int	     sift, var, opos , belegt, frei, abb, old_size;
	char	     abbflag = 0;
	unsigned int old_no_ut[pi_limit];

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Schiebe belegte unique tables zusammen
	// --------------------------------------
	frei = von;
        while (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[frei]] &&
          frei <= bis)
            frei++;

	for (belegt = frei+1; belegt <= bis; belegt ++)
	{
	   if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[belegt]])
	   {
	      sift = OKFDD_PI_Order_Table[frei];
	      OKFDD_PI_Order_Table[frei] = OKFDD_PI_Order_Table[belegt];
	      OKFDD_PI_Order_Table[belegt] = sift;
	      var = OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]] =
                OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]] = var;
	      frei++;
	    }
	}
	bis = frei-1;

	// aktualisiere Hilfsfelder
	// ------------------------
	if (faktor < 1.01) faktor = 1.01;
	min_count = 0;
//      for (var = von; var <= bis; var++)
	for (var = 0; var < OKFDD_Maxidx; var++)
	{
	   old_no_ut[OKFDD_PI_Order_Table[var]] =
             OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]];
	   twin_pi_table[var] = min_pi_table[var] = OKFDD_PI_Order_Table[var];
	   mindtl[OKFDD_PI_Order_Table[var]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]];
	}
	old_size =min_size_i = OKFDD_Now_size_i;

	// suche ersten Kandidaten
	// -----------------------
	switch (art)
	{
	   case 'r':  // Bestimme zufaellige Levelreihenfolge
		      // ------------------------------------
		      for (var = von; var <= bis; var++)
		      {
			 sift                = bis-(rand()%(bis-var+1));
			 opos		     = twin_pi_table[var];
			 twin_pi_table[var]  = twin_pi_table[sift];
			 twin_pi_table[sift] = opos;
		      }
		      break;

	   case 'g':  // Sortiere Level nach absteigender Breite
		      // ---------------------------------------
		      for (var = von; var <= bis; var++)
		      {
		         for (sift = var+1; sift <= bis; sift++)
			 {
			    if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[var]] 
			      < OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]])
			    {
			       opos		   = twin_pi_table[var];
			       twin_pi_table[var]  = twin_pi_table[sift];
			       twin_pi_table[sift] = opos;
			    }
			 }
		      }
		      break;

	   case 'l':  // Suche max. Level
		      // ----------------
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]] 
			> OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]])
                          opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
		      break;

	   case 'v':  // Suche vielversprechendes Level
		      // -------------------------------
		      levelstat();
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (lsf[twin_pi_table[sift]] >
                        lsf[twin_pi_table[opos]])
                          opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
	}

	for (var = von; var <= bis; var++)
	{

	   Prozentbalken(var-von+1,bis-von+1,'S')

	   // Vergroesserungsfaktor relativieren
	   // ----------------------------------
	   if (rel == 'r') old_size = OKFDD_Now_size_i;

	   // Variable fuer momentanen Zerlegungstyp durch DD schieben
	   // --------------------------------------------------------
	   opos = von;
           while (OKFDD_PI_Order_Table[opos]!=twin_pi_table[var]) opos++;

	   if (opos > von)
	   {
	      for (sift = opos - 1; sift >= von && OKFDD_Now_size_i <
                old_size*faktor; sift--)
	      {
		 OKFDD_Levelexchange(sift);
		 gesamtkosten(von,bis);
	      }
	      abb = sift+1;
	      for (sift = abb; sift < opos; sift++) OKFDD_Levelexchange(sift);
	   }

	   for (sift = opos; sift < bis && OKFDD_Now_size_i <
             old_size*faktor; sift++)
	   {
	      OKFDD_Levelexchange(sift);
	      gesamtkosten(von,bis);
	   }
	   abb = sift;

	   if (abb < bis) abbflag = 1;
	   else
	   {
	      // Zerlegungstyp rotieren
	      // ---------------------
	      for (sift = bis; sift < OKFDD_Maxidx-1 && OKFDD_Now_size_i <
                old_size*faktor; sift++)
                  OKFDD_Levelexchange(sift);
	      abb = sift-1;
	      if (abb != OKFDD_Maxidx-2)
	      {
		 for (sift = abb; sift >= bis; sift--)
                   OKFDD_Levelexchange(sift);
		 abb = bis;
		 abbflag = 1;
	      }
	      else
	      {
		 switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                   1]])
		 {
		    case D_Shan : S2P(OKFDD_Maxidx-1); break;
		    case D_posD : P2N(OKFDD_Maxidx-1); break;
		    case D_negD : N2S(OKFDD_Maxidx-1);
		 }
		 for (sift = abb; sift >= bis && OKFDD_Now_size_i <
                   old_size*faktor; sift--) OKFDD_Levelexchange(sift);
		 abb = sift+1;
	      }

	      if (abb != bis)
	      {
		 abbflag = 1;
		 for (sift = abb; sift < OKFDD_Maxidx-1; sift++)
                   OKFDD_Levelexchange(sift);

		 switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                   1]])
		 {
		    case D_Shan : S2N(OKFDD_Maxidx-1); break;
		    case D_posD : P2S(OKFDD_Maxidx-1); break;
		    case D_negD : N2P(OKFDD_Maxidx-1);
		 }

		 for (sift = OKFDD_Maxidx-2; sift >= bis; sift--)
                   OKFDD_Levelexchange(sift);
				abb = bis;
	      }
	   }

	   if (abbflag)
	   {

	      opos = von;
              while (min_pi_table[opos]!=twin_pi_table[var]) opos++;

	      for (sift = abb-1; sift >= opos; sift--)
                OKFDD_Levelexchange(sift);
	      gesamtkosten(0,OKFDD_Maxidx-1);
	   }
	   else
	   {
	      // Variable fuer momentanen Zerlegungstyp durch DD schieben
	      // --------------------------------------------------------
	      gesamtkosten(0,OKFDD_Maxidx-1);
	      for (sift = bis-1; sift >= von && OKFDD_Now_size_i <
                old_size*faktor; sift--)
	      {
		 OKFDD_Levelexchange(sift);
		 gesamtkosten(von,bis);
	      }
	      abb = sift+1;

	      for (sift = abb; sift < OKFDD_Maxidx-1; sift++)
                OKFDD_Levelexchange(sift);

	      // Zerlegungstyp rotieren
	      // ---------------------
	      switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	      {
		 case D_Shan : S2P(OKFDD_Maxidx-1); break;
		 case D_posD : P2N(OKFDD_Maxidx-1); break;
		 case D_negD : N2S(OKFDD_Maxidx-1);
	      }

	      for (sift = OKFDD_Maxidx-2; sift >= bis && OKFDD_Now_size_i <
                old_size*faktor; sift--)
                  OKFDD_Levelexchange(sift);
	      abb = sift+1;

	      if (abb!=bis)
	      {
		 for (sift = abb; sift < OKFDD_Maxidx-1; sift++)
                   OKFDD_Levelexchange(sift);

		 switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                   1]])
		 {
		    case D_Shan : S2N(OKFDD_Maxidx-1); break;
		    case D_posD : P2S(OKFDD_Maxidx-1); break;
		    case D_negD : N2P(OKFDD_Maxidx-1);
		 }

		 for (sift = OKFDD_Maxidx-2; sift >= bis; sift--)
                   OKFDD_Levelexchange(sift);
		 abb = bis;
	      }
	      else
	      {
		 // Variable fuer momentanen Zerlegungstyp durch DD schieben
		 // --------------------------------------------------------
		 gesamtkosten(0,OKFDD_Maxidx-1);
		 for (sift = bis-1; sift >= von && OKFDD_Now_size_i <
                   old_size*faktor; sift--)
		 {
		    OKFDD_Levelexchange(sift);
		    gesamtkosten(von,bis);
		 }
		 abb = sift+1;
	      }

	      // Falls momentaner Zerlegungstyp der betrachteten Variable
              // der beste ist
	      // --------------------------------------------------------
	      if (mindtl[OKFDD_PI_Order_Table[abb]] ==
                OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[abb]])
	      {
		 opos = von;
                 while (min_pi_table[opos]!=twin_pi_table[var]) opos++;

		 if (abb < opos) for (sift = abb; sift < opos; sift++)
                   OKFDD_Levelexchange(sift);
		 else for (sift = abb-1; sift >= opos; sift--)
                   OKFDD_Levelexchange(sift);
		 gesamtkosten(0,OKFDD_Maxidx-1);
	      }

	      // Sonst Typ aendern
	      // -----------------
	      else
	      {
		 for (sift = abb; sift < OKFDD_Maxidx-1; sift++)
                   OKFDD_Levelexchange(sift);

		 if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]]
                   == D_Shan)
		 {
		    if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]]==D_posD)
                      S2P(OKFDD_Maxidx-1);
		    if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]]==D_negD)
                      S2N(OKFDD_Maxidx-1);
		 }
		 else
		 {
		    if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                      1]] == D_posD)
		    {
		       if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]] ==
                         D_Shan) P2S(OKFDD_Maxidx-1);
		       if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]] ==
                         D_negD) P2N(OKFDD_Maxidx-1);
		    }
		    else
		    {
		       if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[
                         OKFDD_Maxidx-1]] == D_negD)
		       {
		       if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]] ==
                         D_Shan) N2S(OKFDD_Maxidx-1);
		       if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]] ==
                         D_posD) N2P(OKFDD_Maxidx-1);
		       }
		    }
		 }

	         // Optimale Position bestimmen und einnehmen
		 // -----------------------------------------
		 opos = von;
                 while (min_pi_table[opos]!=twin_pi_table[var]) opos++;

		 for (sift = OKFDD_Maxidx-2; sift >= opos; sift--)
                   OKFDD_Levelexchange(sift);
		 gesamtkosten(0,OKFDD_Maxidx-1);
	      }
	   }

	   // Suche nächstes Level für Tausch
	   // -------------------------------
	   abbflag = 0;

	   switch (art)
	   {
	      case 'l':  // Suche max. Level
			 // ----------------
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if ((OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]]-
                           old_no_ut[twin_pi_table[sift]]) >
			   (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]]-
                           old_no_ut[twin_pi_table[opos]]))
                             opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
			 break;

	      case 'v':  // Suche vielversprechendes Level
			 // -------------------------------
			 levelstat();
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if (lsf[twin_pi_table[sift]] >
                           lsf[twin_pi_table[opos]])
                             opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
		}
/*	if( var == bis-1 )
	  {
	    int shannon = 0, pos = 0, neg = 0;
	    printf( "\n" );

	    for( int n = 0; n < OKFDD_P_I; n++ )
	    {
	      if( OKFDD_PI_DTL_Table[ n ] == 0 )
	      {
		shannon += 1;
	      }
	      else if( OKFDD_PI_DTL_Table[ n ] == 1 )
	      {
		pos += 1;
	      }
	      else
	      {
		neg += 1;
	      }
	    }
	    printf( "Shannon -> %i; PosD -> %i; NegD -> %i\n", shannon, pos, neg );
	  }*/
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Sifting_PS		                     //
// Sifting mit gleichzeitiger Veraenderung des Zerlegungstyps	             //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Sifting_PS(      usint von,
					usint bis,
					float faktor,
					char  rel,
					char  art   )
{
	int	     sift, var, opos , belegt, frei, abb, old_size, sdul;
	char	     abbflag = 0;
	unsigned int old_no_ut[pi_limit];

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Schiebe belegte unique tables zusammen
	// --------------------------------------
	frei = von;
        while (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[frei]] && frei <=
          bis) frei++;

	for (belegt = frei+1; belegt <= bis; belegt ++)
	{
	   if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[belegt]])
	   {
	      sift = OKFDD_PI_Order_Table[frei];
	      OKFDD_PI_Order_Table[frei] = OKFDD_PI_Order_Table[belegt];
	      OKFDD_PI_Order_Table[belegt] = sift;
	      var = OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]] =
                OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]] = var;
	      frei++;
	   }
	}
	bis = frei-1;

	// aktualisiere Hilfsfelder
	// ------------------------
	if (faktor < 1.01) faktor = 1.01;
	min_count = sdul = 0;
	for (var = bis+1; var < OKFDD_Maxidx; var++)
          sdul += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]];

	for (var = 0; var < OKFDD_Maxidx; var++)
//      for (var = von; var <= bis; var++)
	{
	   old_no_ut[OKFDD_PI_Order_Table[var]] =
             OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[var]];
	   twin_pi_table[var] = min_pi_table[var] = OKFDD_PI_Order_Table[var];
	   mindtl[OKFDD_PI_Order_Table[var]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[var]];
	}
	old_size = min_size_i = OKFDD_Now_size_i;

	// suche ersten Kandidaten
	// -----------------------
	switch (art)
	{
	   case 'r':  // Bestimme zufaellige Levelreihenfolge
		      // ------------------------------------
		      for (var = von; var <= bis; var++)
		      {
			 sift                = bis-(rand()%(bis-var+1));
			 opos		     = twin_pi_table[var];
			 twin_pi_table[var]  = twin_pi_table[sift];
			 twin_pi_table[sift] = opos;
		      }
		      break;

	   case 'g':  // Sortiere Level nach absteigender Breite
		      // ---------------------------------------
		      for (var = von; var <= bis; var++)
		      {
			 for (sift = var+1; sift <= bis; sift++)
			 {
			    if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[var]]
			      < OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]])
			    {
			       opos		   = twin_pi_table[var];
			       twin_pi_table[var]  = twin_pi_table[sift];
			       twin_pi_table[sift] = opos;
			    }
			 }
		      }
		      break;

	   case 'l':  // Suche max. Level
		      // ----------------
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]]
			> OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]])
                          opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
		      break;

	   case 'v':  // Suche vielversprechendes Level
		      // -------------------------------
		      levelstat();
		      opos = von;
		      for (sift = von+1; sift <= bis; sift++)
		      if (lsf[twin_pi_table[sift]] > lsf[twin_pi_table[opos]])
                        opos = sift;

		      if (opos != von)
		      {
			 sift		     = twin_pi_table[opos];
			 twin_pi_table[opos] = twin_pi_table[von];
			 twin_pi_table[von]  = sift;
		      }
	}

//      for (var = von; var <= bis; var++)
	for (var = von; var <= bis; var++) /* Only for test purposes */
	{

	   Prozentbalken(var-von+1,bis-von+1,'S')

	   // Vergroesserungsfaktor relativieren
	   // ----------------------------------
	   if (rel == 'r') old_size = OKFDD_Now_size_i;

	   // Variable fuer momentanen Zerlegungstyp durch DD schieben
	   // --------------------------------------------------------
	   opos = von;
           while (OKFDD_PI_Order_Table[opos]!=twin_pi_table[var]) opos++;
	   for (sift = opos - 1; sift >= von && OKFDD_Now_size_i <
             old_size*faktor; sift--)
	   {
	      OKFDD_Levelexchange(sift);
	      gesamtkosten(von,bis);
	   }
	   abb = sift+1;

	   for (sift = abb; sift < opos; sift++) OKFDD_Levelexchange(sift);
	   for (sift = opos; sift < bis && OKFDD_Now_size_i < old_size*faktor;
              sift++)
	   {
	      OKFDD_Levelexchange(sift);
	      gesamtkosten(von,bis);
	   }
	   opos = abb; abb = sift-1;

	   if (abb < bis-1) abbflag = 1;
	   else
	   {
	      // Zerlegngstyp aendern (unter Beachtung der Faktors)
	      // --------------------------------------------------
	      if (sdul)
	      {
		 for (sift = bis; sift < OKFDD_Maxidx-1 && OKFDD_Now_size_i <
                   old_size*faktor; sift++)
                     OKFDD_Levelexchange(sift);
		 abb = sift-1;

		 if (sift != OKFDD_Maxidx-1)
		 {
		    for (sift = abb; sift >= bis; sift--)
                      OKFDD_Levelexchange(sift);
		    abb = bis;
		    abbflag = 1;
		 }
		 else
		 {
		    if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                      1]]==D_Shan)
                         S2P(OKFDD_Maxidx-1);
		    else P2S(OKFDD_Maxidx-1);
		    for (sift = abb; sift >= bis && OKFDD_Now_size_i <
                      old_size*faktor; sift--)
                        OKFDD_Levelexchange(sift);
		    abb = sift;
		 }

		 if (sift >= bis)
		 {
		    abbflag = 1;

		    for (sift = abb+1; sift < OKFDD_Maxidx-1; sift++)
                      OKFDD_Levelexchange(sift);
		    if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                      1]]==D_Shan)
                         S2P(OKFDD_Maxidx-1);
		    else P2S(OKFDD_Maxidx-1);
		    for (sift = OKFDD_Maxidx-2; sift >= bis; sift--)
                      OKFDD_Levelexchange(sift);
		 }
	      }
	      else
	      {
		 if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[bis]]==D_Shan)
                      S2P(bis);
		 else P2S(bis);
	      }
	   }

	   if (abbflag)
	   {
	      while (min_pi_table[opos]!=twin_pi_table[var]) opos++;

	      for (sift = abb; sift >= opos; sift--) OKFDD_Levelexchange(sift);
	      gesamtkosten(von,bis);
	   }
	   else
	   {
	      // Variable fuer momentanen Zerlegungstyp durch DD schieben
	      // --------------------------------------------------------
	      gesamtkosten(von,bis);
	      for (sift = bis-1; sift >= von && OKFDD_Now_size_i <
                old_size*faktor; sift--)
	      {
		 OKFDD_Levelexchange(sift);
		 gesamtkosten(von,bis);
	      }
	      abb = sift+1;

	      // Falls momentaner Zerlegungstyp der betrachteten Variable
              // der beste ist
	      // --------------------------------------------------------
	      if (mindtl[OKFDD_PI_Order_Table[abb]]==OKFDD_PI_DTL_Table[
                OKFDD_PI_Order_Table[abb]])
	      {
		 opos = abb;
                 while (min_pi_table[opos]!=twin_pi_table[var]) opos++;
		 for (sift = abb; sift < opos; sift++)
                   OKFDD_Levelexchange(sift);
		 gesamtkosten(von,bis);
	      }

	      // Sonst Typ aendern
	      // -----------------
	      else
	      {
		 if (sdul)
		 {
		    for (sift = abb; sift < OKFDD_Maxidx-1; sift++)
                      OKFDD_Levelexchange(sift);
		    if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                      1]]==D_Shan &&
		      mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]]==D_posD)
                        S2P(OKFDD_Maxidx-1);
		    else if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[
                      OKFDD_Maxidx-1]]==D_posD &&
		      mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]]==D_Shan)
                 	P2S(OKFDD_Maxidx-1);

		    for (sift = OKFDD_Maxidx-2; sift >= bis; sift--)
                      OKFDD_Levelexchange(sift);
		 }
		 else
		 {
		    for (sift = abb; sift < bis; sift++)
                      OKFDD_Levelexchange(sift);
		    if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[bis]]==D_Shan
                      && mindtl[OKFDD_PI_Order_Table[bis]]==D_posD)
                        S2P(bis);
		    else if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[bis]]==
                      D_posD && mindtl[OKFDD_PI_Order_Table[bis]]==D_Shan)
                        P2S(bis);
		 }

		 // Optimale Position bestimmen und einnehmen
		 // -----------------------------------------
		 opos = von;
                 while (min_pi_table[opos]!=twin_pi_table[var]) opos++;
		 for (sift = bis-1; sift >= opos; sift--)
                   OKFDD_Levelexchange(sift);
		   gesamtkosten(von,bis);
	      }
	   }

	   // Suche nächstes Level für Tausch
	   // -------------------------------
	   abbflag = 0;
	   switch (art)
	   {
	      case 'l':  // Suche max. Level
			 // ----------------
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if ((OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[sift]]-
                           old_no_ut[twin_pi_table[sift]]) >
			   (OKFDD_No_UTNodes_Per_Lvl[twin_pi_table[opos]]-
                           old_no_ut[twin_pi_table[opos]]))
                             opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
		            twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
			 break;

	      case 'v':  // Suche vielversprechendes Level
			 // -------------------------------
		         levelstat();
			 opos = var+1;
			 for (sift = var+2; sift <= bis; sift++)
			 if (lsf[twin_pi_table[sift]] >
                           lsf[twin_pi_table[opos]])
                             opos = sift;

			 if (opos != var+1)
			 {
			    sift		 = twin_pi_table[opos];
			    twin_pi_table[opos]  = twin_pi_table[var+1];
			    twin_pi_table[var+1] = sift;
			 }
		}
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Tour // Rotation der Variablenreihenfolge	     //
/* ========================================================================= */
void dd_man::OKFDD_Tour(usint von,
			usint bis)
{
	int sift, var, opos, i;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	min_count = OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   twin_pi_table[OKFDD_Maxidx] = min_pi_table[OKFDD_Maxidx] =
             OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	// Variablenreihenfolge rotieren
	// -----------------------------
	for (var = von; var <= bis; var++)
	{
	   Prozentbalken(var-von+1,bis-von+1,'t')
	   for (i = von; i < bis; i++) OKFDD_Levelexchange(i);
	   kosten(von,bis);
	}

	// optimale Positionen bestimmen und einnehmen
	// -------------------------------------------
	for (var = bis; var >= von; var--)
	{
	   opos = von;
           while (min_pi_table[var]!=OKFDD_PI_Order_Table[opos]) opos++;
	   for (sift = opos; sift < var; sift++) OKFDD_Levelexchange(sift);
	}
	kosten(von,bis);

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Inversion	// Invertiere Variablenreihenfolge   //
/* ========================================================================= */
void dd_man::OKFDD_Inversion(   usint von,
				usint bis)
{
	int var, i, opos, sift;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	min_count = OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	// Variablenreihenfolge invertieren
	// --------------------------------
	kosten(von,bis);
	for (var = von; var <= bis; var++)
	{
	   Prozentbalken(var-von+1,bis-von+1,'i')
	   for (i = bis-1; i >= var; i--) OKFDD_Levelexchange(i);
	   kosten(von,bis);
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Rotation	                                     //
// Rotation der Zerlegungstypen im DD im bestimmten Bereich und Richtung     //
/* ========================================================================= */
void dd_man::OKFDD_Rotation(    usint von,
				usint bis,
				char  dir)
{
	int sift, var, opos;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	min_count = 1;
	OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	for (var = von; var <= bis; var++)
	{

	   Prozentbalken(var-von+1,bis-von+1,'r')

	   // Variable in unterstes Level schieben
	   // ------------------------------------
	   for (sift = bis; sift < OKFDD_Maxidx-1; sift++)
             OKFDD_Levelexchange(sift);

	   // Vorwaertsrotation
	   // -----------------
	   if (dir == 1)
	   {
	      switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	      {
		 case D_Shan : S2P(OKFDD_Maxidx-1); break;
		 case D_posD : P2N(OKFDD_Maxidx-1); break;
		 case D_negD : N2S(OKFDD_Maxidx-1);
	      }
	   }

	   // Rueckwaertsrotation
	   // -------------------
	   else
	   {
	      switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	      {
		 case D_Shan : S2N(OKFDD_Maxidx-1); break;
		 case D_posD : P2S(OKFDD_Maxidx-1); break;
		 case D_negD : N2P(OKFDD_Maxidx-1);
	      }
	   }

	   // Variable in urspruengliches Level schieben
	   // ------------------------------------------
	   for (sift = OKFDD_Maxidx-2; sift >= von; sift--)
             OKFDD_Levelexchange(sift);
	   gesamtkosten(von,bis);
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Winsift		                             //
// Sifting fuer best. Bereich, best Fenstergroessen und best. Ueberlappung   //
/* ========================================================================= */
void dd_man::OKFDD_Winsift(     usint von,
				usint bis,
				usint len,
				usint offset)
{
	int win, sift, var, opos ;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	min_count = OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   twin_pi_table[OKFDD_Maxidx] = min_pi_table[OKFDD_Maxidx] =
             OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	win = von;
	if (win > (bis-len+1)) len = bis-von+1;

	while (win < bis)
	{
	   for (var = win; var <= win+len-1; var++)
	   {
	      opos = win;
              while (OKFDD_PI_Order_Table[opos]!=twin_pi_table[var]) opos++;

	      for (sift = opos - 1; sift >= win; sift--)
	      {
		 OKFDD_Levelexchange(sift);
		 kosten(von,bis);
	      }
	      for (sift = win; sift < opos; sift++) OKFDD_Levelexchange(sift);
	      for (sift = opos; sift < win+len-1; sift++)
	      {
		 OKFDD_Levelexchange(sift);
		 kosten(von,bis);
	      }

	      // Optimale Position bestimmen und einnehmen
	      // -----------------------------------------
	      opos = win;
              while (min_pi_table[opos]!=twin_pi_table[var]) opos++;

	      for (sift = win+len-2; sift >= opos; sift--)
                OKFDD_Levelexchange(sift);
	      kosten(von,bis);
	   }

	   for (var = win; var <= win+len-1; var++)
             twin_pi_table[var] = OKFDD_PI_Order_Table[var];
	   win +=offset;
	   if (win > (bis-len+1)) len = bis-win+1;
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::permut		                                     //
// Rekursive Routine zur Erzeugung v. Permutationen per Levelvertauschungen  //
/* ========================================================================= */
void dd_man::permut(usint t,
		    usint von,
		    usint bis)
{
	int ii;

	if (aa[t] == 1)
	{
		l--;
		for (ii = 0; ii <= t-von; ii++)
		{
			if (t < bis) permut(t+1,von,bis);
			if (ii < t-von)
			{
				OKFDD_Levelexchange(l+ii);
				kosten(von,bis);
			}
			aa[t+1] = (aa[t+1]+1)%2;
		}
	}
	else
	{
		for (ii = t-von; ii >= 0; ii--)
		{
			if (t < bis) permut(t+1,von,bis);
			if (ii > 0)
			{
				OKFDD_Levelexchange(l+ii-1);
				kosten(von,bis);
			}
			aa[t+1] = (aa[t+1]+1)%2;
		}
		l++;
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Permutation			             //
// Bestimmung des absoluten Min. einer Fkt. fuer feste DTL !!!	             //
/* ========================================================================= */
void dd_man::OKFDD_Permutation( usint von,
				usint bis)
{      
	int i , sift, opos;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	min_count = OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	l = von;

	// Laufzeittechnische Einschraenkung
	// ---------------------------------
	if ((bis-von)>1) permut(von+1,von,bis);
	OKFDD_Levelexchange(von);
	kosten(von,bis);

	// optimale Positionen bestimmen und einnehmen
	// -------------------------------------------
	for (i = bis; i >= von; i--)
	{
	   opos = von;
           while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos++;
	   for (sift = opos; sift < i; sift++) OKFDD_Levelexchange(sift);
	}
	kosten(von,bis);

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Winpermutation	                             //
// Permutation f.best. Bereich, best. Fenstergroessen + best. Ueberlappung   //
/* ========================================================================= */
void dd_man::OKFDD_Winpermutation(      usint von,
					usint bis,
					usint len,
					usint offset)
{     
	int win, i, sift, var, opos , belegt, frei;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	min_count = OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	// Schiebe belegte unique tables zusammen
	// --------------------------------------
	frei = von;
        while (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[frei]] && frei
          <= bis)
            frei++;

	for (belegt = frei+1; belegt <= bis; belegt ++)
	{
	   if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[belegt]])
	   {
	      sift = OKFDD_PI_Order_Table[frei];
	      OKFDD_PI_Order_Table[frei] = OKFDD_PI_Order_Table[belegt];
	      OKFDD_PI_Order_Table[belegt] = sift;
	      var = OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[frei]] =
                OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]];
	      OKFDD_PI_Level_Table[OKFDD_PI_Order_Table[belegt]] = var;
	      frei++;
	   }
	}
	bis = frei-1;
	win = von;
	if (win > (bis-len+1)) len = bis-von+1;

	while (win < bis)
	{
	   l = win;
	   permut(win+1,win,(win+len-1));

	   // Optimale Position bestimmen und einnehmen
	   // -----------------------------------------
	   for (i = 0; i < OKFDD_Maxidx; i++) aa[i] = 0;
	   for (i = (win+len-1); i >= win; i--)
	   {
//	       aa[i] = 0;
//	       opos = von;
//             while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos++;
	       opos = win;
               while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos++;
	       for (sift = opos; sift < i; sift++) OKFDD_Levelexchange(sift);
	   }

	   // Kosten und naechstes Fenster bestimmen
	   // --------------------------------------
	   kosten(win,win+len-1);
//	   kosten(von,bis);
	   win +=offset;
	   if (win > (bis-len+1)) len=bis-win+1;
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_GAC_4_DTL	// Generiere alle DTL-Kombinationen  //
/* ========================================================================= */
void dd_man::OKFDD_GAC_4_DTL(   usint   von,
				usint   bis,
				char    fkt,
				travin  functin,
				travex  functex)
{
	int   opos, i, sift, zeiger, x_hoch_n=1;
	char* zeile = new char[OKFDD_Maxidx];
        char* spalte = new char[OKFDD_Maxidx];
        char  potenz=3;

	// Initialisierung
	// ---------------
	if (fkt==3) OKFDD_DD_to_FDD(von,bis,1);
	else	OKFDD_DD_to_BDD(von,bis,1);

	if (fkt) potenz = 2;

	for (i = von; i <= bis; i++)
	{
	   x_hoch_n	   *= potenz;
	   spalte[i]	    = 0;
//	   zeile[i]	    = OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]];
	   zeile[i]	    = 0;
	   twin_pi_table[i] = OKFDD_PI_Order_Table[i];
	}

	// Funktionsaufruf
	// ---------------
	if (functin != NULL) (this->*functin)(NULL);
	if (functex != NULL) (*functex)(this,NULL);

	// Durchlaufe alle Zerlegungstypen im 'GRAY-Code'
	// ----------------------------------------------
	for (i = 1; i < x_hoch_n; i++)
	{
		// Veraenderung des Zerlegungstyps
		// -------------------------------
		zeiger = bis;
		spalte[zeiger]++;
		while (spalte[zeiger] == potenz)
		{
			spalte[zeiger] = 0;
			zeile[zeiger]++;
			if (zeile[zeiger] == potenz) zeile[zeiger] = 0;
			spalte[--zeiger]++;
		}

		opos = von;
		while (twin_pi_table[zeiger]!=OKFDD_PI_Order_Table[opos])
                  opos++;
		for (sift = opos; sift < OKFDD_Maxidx-1; sift++)
                  OKFDD_Levelexchange(sift);

		switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                  1]])
		{
		        case D_Shan:    if (fkt==1) S2P(OKFDD_Maxidx-1);
					else	S2N(OKFDD_Maxidx-1); break;
			case D_posD:    if (fkt==3) P2N(OKFDD_Maxidx-1);
					else	P2S(OKFDD_Maxidx-1); break;
			case D_negD:    if (fkt==2) N2S(OKFDD_Maxidx-1);
					else	N2P(OKFDD_Maxidx-1);
		}

		for (sift = OKFDD_Maxidx-2; sift >= opos; sift--)
                  OKFDD_Levelexchange(sift);
		gesamtkosten(von,bis);

		// Funktionsaufruf
		// ---------------
		if (functin != NULL) (this->*functin)(NULL);
		if (functex != NULL) (*functex)(this,NULL);
	}

	delete[] zeile;
	delete[] spalte;
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_Friedman	                                     //
// Bestimmung des absoluten Min. einer Fkt. fuer alle DTLs (GRAY-Code)!!!    //
/* ========================================================================= */
void dd_man::OKFDD_Friedman(    usint von,
				usint bis)
{
	int     i,j, opos, sift, zeiger, zwei_hoch_n=1;
	ulint   bbggg=1;
	usint   tma,tmac;
	usint   adr[pi_limit];
	uchar   flag_p=0, flag_a=0, flag_b=0;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Vorverdichtung
	// --------------
	if (OKFDD_Outputflags & ros_p) { OKFDD_Outputflags^=ros_p; flag_p=1; }
	if (OKFDD_Outputflags & ros_a) { OKFDD_Outputflags^=ros_a; flag_a=1; }
	if (OKFDD_Outputflags & ros_b) { OKFDD_Outputflags^=ros_b; flag_b=1; }

	OKFDD_Siftlight(von,bis,'v');
	OKFDD_Siftlight(von,bis,'v');
	OKFDD_Siftlight(von,bis,'v');

	if(flag_p==1) OKFDD_Outputflags^=ros_p;
	if(flag_a==1) OKFDD_Outputflags^=ros_a;
	if(flag_b==1) OKFDD_Outputflags^=ros_b;

	// Initialisierung
	// ---------------
	min_count = 0;
	for (i = 0; i < OKFDD_Maxidx; i++)
        {
	   twin_pi_table[i] = min_pi_table[i] = OKFDD_PI_Order_Table[i];
	   mindtl[OKFDD_PI_Order_Table[i]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]];
	}
	min_size_i = OKFDD_Now_size_i;

	for (i = von; i <= bis; i++)
        {
	   adr[OKFDD_PI_Order_Table[i]]   = zwei_hoch_n;
	   zwei_hoch_n *= 2;
	   bbggg += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[i]];
	}

	usint*   label     = new usint[zwei_hoch_n];
	usint*   teilmenge = new usint[zwei_hoch_n];
	ulint*   breite    = new ulint[zwei_hoch_n];

	teilmenge[0] = 0;
	breite[0]    = 0;
	label[0]     = 0;


	// Konstruktion aller Teilmengen
	// -----------------------------
	for (tma = 1; tma < zwei_hoch_n; tma++)
        {
		Prozentbalken(tma,zwei_hoch_n-1,'f')

		// Teilmenge in oberen Bereich schieben
		// ------------------------------------
		zeiger = von-1;
		tmac   = tma;
		for (sift = von; sift <= bis && tmac; sift++)
                {
		   if (tmac&1)
                   {
		      zeiger++;
		      opos = zeiger;
		      while (twin_pi_table[sift]!=OKFDD_PI_Order_Table[opos])
                        opos++;
		      if (opos!=zeiger) for (j = opos-1; j >= zeiger; j--)
                        OKFDD_Levelexchange(j);
		   }
		   tmac >>=1;
		}

		// Beste Bottom-Variable bestimmen
		// -------------------------------
		breite[tma] = bbggg;
		for (sift = zeiger; sift >= von; sift--)
                {
		   if (breite[tma-adr[OKFDD_PI_Order_Table[sift]]] <
                     breite[tma])
                   {
		      for (j = sift; j < zeiger; j++) OKFDD_Levelexchange(j);
		      if (breite[tma] > breite[tma-adr[OKFDD_PI_Order_Table[
                        zeiger]]] +
			OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[zeiger]])
                      {
			 teilmenge[tma] = tma-
                           adr[OKFDD_PI_Order_Table[zeiger]];
			 breite[tma]    = breite[teilmenge[tma]] +
                           OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[
                           zeiger]];
			 label[tma]     = OKFDD_PI_Order_Table[zeiger];
		      }
		   }
		}
	}

	// Best. der besten Reihenfolge
	// ----------------------------
	min_pi_table[bis] = label[zwei_hoch_n-1];
	min_size_i	  = breite[zwei_hoch_n-1];
	tma		  = teilmenge[zwei_hoch_n-1];

	for (i = bis-1; i >= von; i--)
        {
		min_pi_table[i] = label[tma];
		tma		= teilmenge[tma];
	}

	// Optimale Position bestimmen und einnehmen
	// -----------------------------------------
	for (i = von; i <= bis; i++)
	{
	   opos = bis;
           while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos--;
	   for (sift = opos-1; sift >= i; sift--) OKFDD_Levelexchange(sift);
	}
	gesamtkosten(von,bis);
	min_size_i = OKFDD_Now_size_i;

	cout(ros_a) << OKFDD_Now_size_i << "\n";

	delete[] label;
	delete[] teilmenge;
	delete[] breite;
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_Friedman_X	                             //
// Bestimmung des absoluten Min. einer Fkt. fuer alle DTLs (GRAY-Code)!!!    //
/* ========================================================================= */
void dd_man::OKFDD_Friedman_X(  usint von,
				usint bis)
{
        if (bis-von>30)
        {
	  cout << "Runtime too expensive (more than 30 variables) ... (exiting)\n";
	  return;
	}

	int     i,j, opos, sift, zeiger;
	ulint   zwei_hoch_n=1;
	ulint   bbggg=1;
	ulint   tma,tmac;
	ulint   adr[pi_limit];
	uchar   flag_p=0, flag_a=0, flag_b=0;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Vorverdichtung
	// --------------
	if (OKFDD_Outputflags & ros_p) { OKFDD_Outputflags^=ros_p; flag_p=1; }
	if (OKFDD_Outputflags & ros_a) { OKFDD_Outputflags^=ros_a; flag_a=1; }
	if (OKFDD_Outputflags & ros_b) { OKFDD_Outputflags^=ros_b; flag_b=1; }

	OKFDD_Siftlight(von,bis,'r');
	OKFDD_Siftlight(von,bis,'l');
	OKFDD_Siftlight(von,bis,'v');

	if(flag_p==1) OKFDD_Outputflags^=ros_p;
	if(flag_a==1) OKFDD_Outputflags^=ros_a;
	if(flag_b==1) OKFDD_Outputflags^=ros_b;

	// Initialisierung
	// ---------------
	min_count = 0;
	for (i = 0; i < OKFDD_Maxidx; i++)
        {
	   twin_pi_table[i] = min_pi_table[i] = OKFDD_PI_Order_Table[i];
	   mindtl[OKFDD_PI_Order_Table[i]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]];
	}
	min_size_i = OKFDD_Now_size_i;

	for (i = von; i <= bis; i++)
        {
	   adr[OKFDD_PI_Order_Table[i]] = zwei_hoch_n;
	   zwei_hoch_n *= 2;
	   bbggg += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[i]];
	}

	ulint* label     = new ulint[zwei_hoch_n];
	ulint* teilmenge = new ulint[zwei_hoch_n];
	ulint* breite    = new ulint[zwei_hoch_n];

	teilmenge[0] = 0;
	breite[0]    = 0;
	label[0]     = 0;


	// Konstruktion aller Teilmengen
	// -----------------------------
	for (tma = 1; tma < zwei_hoch_n; tma++)
        {
	   Prozentbalken(tma,zwei_hoch_n-1,'f')

	   // Teilmenge in oberen Bereich schieben
	   // ------------------------------------
	   zeiger = von-1;
	   tmac   = tma;
	   for (sift = von; sift <= bis && tmac; sift++)
           {
	      if (tmac&1)
              {
		 zeiger++;
		 opos = zeiger;
		 while (twin_pi_table[sift]!=OKFDD_PI_Order_Table[opos])
                   opos++;
		 if (opos!=zeiger) for (j = opos-1; j >= zeiger; j--)
                   OKFDD_Levelexchange(j);
	      }
	      tmac >>=1;
	   }

	   // Beste Bottom-Variable bestimmen
	   // -------------------------------
	   breite[tma]    = bbggg;
	   for (sift = zeiger; sift >= von; sift--)
           {
	      if (breite[tma-adr[OKFDD_PI_Order_Table[sift]]] < breite[tma])
              {
	         for (j = sift; j < zeiger; j++) OKFDD_Levelexchange(j);
		 if (breite[tma] > breite[tma-
                   adr[OKFDD_PI_Order_Table[zeiger]]] +
		   OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[zeiger]])
                 {
		    teilmenge[tma] = tma-adr[OKFDD_PI_Order_Table[zeiger]];
		    breite[tma]    = breite[teilmenge[tma]] +
                      OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[zeiger]];
		    label[tma]     = OKFDD_PI_Order_Table[zeiger];
		 }
	      }
	   }
	}

	// best. der besten Reihenfolge
	// ----------------------------
	min_pi_table[bis] = label[zwei_hoch_n-1];
	min_size_i	  = breite[zwei_hoch_n-1];
	tma		  = teilmenge[zwei_hoch_n-1];
	for (i = bis-1; i >= von; i--)
        {
		min_pi_table[i] = label[tma];
		tma		= teilmenge[tma];
	}

	// optimale Position bestimmen und einnehmen
	// -----------------------------------------
	for (i = von; i <= bis; i++)
	{
	   opos = bis;
           while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos--;
	   for (sift = opos-1; sift >= i; sift--) OKFDD_Levelexchange(sift);
	}
	gesamtkosten(von,bis);
	min_size_i = OKFDD_Now_size_i;

	cout(ros_a) << OKFDD_Now_size_i << "\n";

	delete[] label;
	delete[] teilmenge;
	delete[] breite;
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Friedman                                   //
// Bestimmung des absoluten Min. einer Fkt. fuer alle DTLs (GRAY-Code)!!!    //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Friedman(	usint von,
					usint bis)
{
	int    z,i,j, opos, sift, zeiger, zwei_hoch_n=1, drei_hoch_n=1, grenze;
	char*  zeile = new char[OKFDD_Maxidx];
        char*  spalte = new char[OKFDD_Maxidx];
	ulint  bbggg=1;
	usint  tma,tmac;
	usint  adr[pi_limit];
	uchar  flag_p=0, flag_a=0, flag_b=0;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Vorverdichtung
	// --------------
	if (OKFDD_Outputflags & ros_p) { OKFDD_Outputflags^=ros_p; flag_p=1; }
	if (OKFDD_Outputflags & ros_a) { OKFDD_Outputflags^=ros_a; flag_a=1; }
	if (OKFDD_Outputflags & ros_b) { OKFDD_Outputflags^=ros_b; flag_b=1; }

	OKFDD_Siftlight(von,bis,'v');
	OKFDD_Siftlight(von,bis,'v');
	OKFDD_Siftlight(von,bis,'v');

	if(flag_p==1) OKFDD_Outputflags^=ros_p;
	if(flag_a==1) OKFDD_Outputflags^=ros_a;
	if(flag_b==1) OKFDD_Outputflags^=ros_b;

	// Initialisierung
	// ---------------
	min_count = 0;
//      for (i = von; i <= bis; i++) {
	for (i = 0; i < OKFDD_Maxidx; i++)
        {
	   spalte[i] = 0;
	   twin_pi_table[i] = min_pi_table[i] = OKFDD_PI_Order_Table[i];
	   zeile[i]  = mindtl[OKFDD_PI_Order_Table[i]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]];
	}
	min_size_i = OKFDD_Now_size_i;

	for (i = von; i <= bis; i++)
        {
		drei_hoch_n *= 3;
		adr[OKFDD_PI_Order_Table[i]] = zwei_hoch_n;
		zwei_hoch_n *= 2;
		bbggg += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[i]];
	}

	usint* label     = new usint[zwei_hoch_n];
	ulint* breite    = new ulint[zwei_hoch_n];
	usint* teilmenge = new usint[zwei_hoch_n];

	teilmenge[0] = 0;
	breite[0]    = 0;
	label[0]     = 0;
	grenze       = von;

	// Durchlaufe alle Zerlegungstypen im 'GRAY-Code'
	// ----------------------------------------------
	for (z = 0; z < drei_hoch_n; z++)
        {
	   Prozentbalken(z,drei_hoch_n-1,'F')

	   for (tma = adr[twin_pi_table[grenze]]; tma < zwei_hoch_n; tma++)
           {
	      if ((tma & adr[twin_pi_table[grenze]])||!z)
              {
		 zeiger = von-1;
		 tmac   = tma;
		 for (sift = von; sift <= bis && tmac; sift++)
                 {
		    if (tmac&1)
		    {
		       zeiger++;
		       opos = zeiger;
		       while (twin_pi_table[sift]!=OKFDD_PI_Order_Table[opos])
                         opos++;
		       if (opos!=zeiger) for (j = opos-1; j >= zeiger; j--)
                         OKFDD_Levelexchange(j);
		    }
		    tmac >>=1;
		 }

		 // Beste Bottom-Variable bestimmen
		 // -------------------------------
		 breite[tma]    = bbggg;
		 for (sift = zeiger; sift >= von; sift--)
                 {
		    if (breite[tma-adr[OKFDD_PI_Order_Table[sift]]] <
                      breite[tma])
                    {
		       for (j = sift; j < zeiger; j++) OKFDD_Levelexchange(j);
		       if (breite[tma] > breite[tma-adr[OKFDD_PI_Order_Table[
                         zeiger]]] + OKFDD_No_UTNodes_Per_Lvl[
                         OKFDD_PI_Order_Table[zeiger]])
                       {
			  teilmenge[tma] = tma-
                            adr[OKFDD_PI_Order_Table[zeiger]];
			  breite[tma]    = breite[teilmenge[tma]] +
                            OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[
                            zeiger]];
			  label[tma]     = OKFDD_PI_Order_Table[zeiger];
		       }
		    }
		 }
	      }
           }

	   // Best. der besten Reihenfolge
	   // ----------------------------
	   if (min_size_i > breite[zwei_hoch_n-1])
           {
	      tma   = zwei_hoch_n-1;
	      bbggg = min_size_i = breite[tma];
	      for (i = bis; i >= von; i--)
              {
		 min_pi_table[i]    = label[tma];
		 mindtl[label[tma]] = OKFDD_PI_DTL_Table[label[tma]];
		 tma		    = teilmenge[tma];
	      }
	   }

	   // Veraenderung des Zerlegungstyps
	   // -------------------------------
	   zeiger = bis;
	   spalte[zeiger]++;
	   while (spalte[zeiger] == 3)
	   {
	      spalte[zeiger] = 0;
	      zeile[zeiger]++;
	      if (zeile[zeiger] == 3) zeile[zeiger] = 0;
	      if (zeiger > von) spalte[--zeiger]++;
	   }

	   opos = von;
	   while (twin_pi_table[zeiger]!=OKFDD_PI_Order_Table[opos]) opos++;
	   for (sift = opos; sift < OKFDD_Maxidx-1; sift++)
             OKFDD_Levelexchange(sift);

	   switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	   {
	      case D_Shan : S2N(OKFDD_Maxidx-1); break;
	      case D_negD : N2P(OKFDD_Maxidx-1); break;
	      case D_posD : P2S(OKFDD_Maxidx-1);
	   }

	   for (sift = OKFDD_Maxidx-2; sift >= opos; sift--)
             OKFDD_Levelexchange(sift);
	   grenze = zeiger;
	}

	// Optimale Position bestimmen und einnehmen
	// -----------------------------------------
	for (i = von; i <= bis; i++)
	{
	   opos = bis;
           while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos--;

	   if (mindtl[min_pi_table[i]]==OKFDD_PI_DTL_Table[
             OKFDD_PI_Order_Table[opos]])
             for (sift = opos-1; sift >= i; sift--)
               OKFDD_Levelexchange(sift);
	   else
	   {
	      for (sift = opos; sift < OKFDD_Maxidx-1; sift++)
                OKFDD_Levelexchange(sift);

	      switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	      {
		 case D_Shan :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_posD)
                                      S2P(OKFDD_Maxidx-1);
				 else S2N(OKFDD_Maxidx-1);
				 break;
		 case D_posD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      P2S(OKFDD_Maxidx-1);
				 else P2N(OKFDD_Maxidx-1);
				 break;
		 case D_negD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      N2S(OKFDD_Maxidx-1);
				 else N2P(OKFDD_Maxidx-1);
	      }

	      for (sift = OKFDD_Maxidx-2; sift >= i; sift--)
                OKFDD_Levelexchange(sift);
	   }
	}
	gesamtkosten(0,OKFDD_Maxidx-1);
	min_size_i = OKFDD_Now_size_i;

	cout(ros_a) << OKFDD_Now_size_i << "\n";

	delete[] zeile;
	delete[] spalte;
	delete[] label;
	delete[] teilmenge;
	delete[] breite;
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Friedman_X	                             //
// Bestimmung des absoluten Min. einer Fkt. fuer alle DTLs (GRAY-Code)!!!    //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Friedman_X(      usint von,
					usint bis)
{
        if (bis-von>30)
        {
	  cout << "Runtime too expensive (more than 30 variables) ... (exiting)\n";
	  return;
	}

	int    z,i,j, opos, sift, zeiger;
	ulint  zwei_hoch_n=1, drei_hoch_n=1, grenze;
	char*  zeile = new char[OKFDD_Maxidx];
	char*  spalte = new char[OKFDD_Maxidx];
	ulint  bbggg=1;
	ulint  tma,tmac;
	ulint  adr[pi_limit];
	uchar  flag_p=0, flag_a=0, flag_b=0;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Vorverdichtung
	// --------------
	if (OKFDD_Outputflags & ros_p) { OKFDD_Outputflags^=ros_p; flag_p=1; }
	if (OKFDD_Outputflags & ros_a) { OKFDD_Outputflags^=ros_a; flag_a=1; }
	if (OKFDD_Outputflags & ros_b) { OKFDD_Outputflags^=ros_b; flag_b=1; }

	OKFDD_Siftlight(von,bis,'v');
	OKFDD_Siftlight(von,bis,'v');
	OKFDD_Siftlight(von,bis,'v');

	if(flag_p==1) OKFDD_Outputflags^=ros_p;
	if(flag_a==1) OKFDD_Outputflags^=ros_a;
	if(flag_b==1) OKFDD_Outputflags^=ros_b;

	// Initialisierung
	// ---------------
	min_count = 0;
//      for (i = von; i <= bis; i++) {
	for (i = 0; i < OKFDD_Maxidx; i++)
        {
	   spalte[i] = 0;
	   twin_pi_table[i] = min_pi_table[i] = OKFDD_PI_Order_Table[i];
	   zeile[i] = mindtl[OKFDD_PI_Order_Table[i]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]];
	}
	min_size_i = OKFDD_Now_size_i;

	for (i = von; i <= bis; i++)
        {
		drei_hoch_n *= 3;
		adr[OKFDD_PI_Order_Table[i]]   = zwei_hoch_n;
		zwei_hoch_n *= 2;
		bbggg += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[i]];
	}

	ulint* label     = new ulint[zwei_hoch_n];
	ulint* breite    = new ulint[zwei_hoch_n];
	ulint* teilmenge = new ulint[zwei_hoch_n];

	teilmenge[0] = 0;
	breite[0]    = 0;
	label[0]     = 0;
	grenze       = von;

	// Durchlaufe alle Zerlegungstypen im 'GRAY-Code'
	// ----------------------------------------------
	for (z = 0; z < drei_hoch_n; z++)
        {
	   Prozentbalken(z,drei_hoch_n-1,'F')

	   for (tma = adr[twin_pi_table[grenze]]; tma < zwei_hoch_n; tma++)
           {
	      if ((tma & adr[twin_pi_table[grenze]])||!z)
              {
		 zeiger = von-1;
		 tmac   = tma;
		 for (sift = von; sift <= bis && tmac; sift++)
                 {
		    if (tmac&1)
                    {
		       zeiger++;
		       opos = zeiger;
		       while (twin_pi_table[sift]!=OKFDD_PI_Order_Table[opos])
                         opos++;
		       if (opos!=zeiger) for (j = opos-1; j >= zeiger; j--)
                         OKFDD_Levelexchange(j);
		    }
		    tmac >>=1;
		 }

		 // Beste Bottom-Variable bestimmen
		 // -------------------------------
		 breite[tma] = bbggg;
		 for (sift = zeiger; sift >= von; sift--)
                 {
		    if (breite[tma-adr[OKFDD_PI_Order_Table[sift]]] <
                      breite[tma])
                    {
		       for (j = sift; j < zeiger; j++) OKFDD_Levelexchange(j);
		       if (breite[tma] > breite[tma-adr[OKFDD_PI_Order_Table[
                         zeiger]]] + OKFDD_No_UTNodes_Per_Lvl[
                         OKFDD_PI_Order_Table[zeiger]])
                       {
			  teilmenge[tma] = tma-
                            adr[OKFDD_PI_Order_Table[zeiger]];
			  breite[tma]    = breite[teilmenge[tma]] +
                            OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[
                            zeiger]];
			  label[tma]     = OKFDD_PI_Order_Table[zeiger];
		       }
		    }
		 }
	      }
	   }

	   // Best. der besten Reihenfolge
	   // ----------------------------
	   if (min_size_i > breite[zwei_hoch_n-1])
           {
	      tma   = zwei_hoch_n-1;
	      bbggg = min_size_i = breite[tma];
	      for (i = bis; i >= von; i--)
              {
		 min_pi_table[i]    = label[tma];
		 mindtl[label[tma]] = OKFDD_PI_DTL_Table[label[tma]];
		 tma		    = teilmenge[tma];
	      }
	   }

	   // Veraenderung des Zerlegungstyps
	   // -------------------------------
	   zeiger = bis;
	   spalte[zeiger]++;
	   while (spalte[zeiger] == 3)
	   {
	      spalte[zeiger] = 0;
	      zeile[zeiger]++;
	      if (zeile[zeiger] == 3) zeile[zeiger] = 0;
	      if (zeiger > von) spalte[--zeiger]++;
	   }

	   opos = von;
	   while (twin_pi_table[zeiger]!=OKFDD_PI_Order_Table[opos]) opos++;
	   for (sift = opos; sift < OKFDD_Maxidx-1; sift++)
             OKFDD_Levelexchange(sift);

	   switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	   {
	      case D_Shan : S2N(OKFDD_Maxidx-1); break;
	      case D_negD : N2P(OKFDD_Maxidx-1); break;
	      case D_posD : P2S(OKFDD_Maxidx-1);
	   }

	   for (sift = OKFDD_Maxidx-2; sift >= opos; sift--)
             OKFDD_Levelexchange(sift);
	   grenze = zeiger;
	}

	// Optimale Position bestimmen und einnehmen
	// -----------------------------------------
	for (i = von; i <= bis; i++)
	{
	   opos = bis;
           while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos--;

	   if (mindtl[min_pi_table[i]]==OKFDD_PI_DTL_Table[
             OKFDD_PI_Order_Table[opos]])
             for (sift = opos-1; sift >= i; sift--)
               OKFDD_Levelexchange(sift);
	   else
	   {
	      for (sift = opos; sift < OKFDD_Maxidx-1; sift++)
                OKFDD_Levelexchange(sift);

	      switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	      {
		 case D_Shan :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_posD)
                                      S2P(OKFDD_Maxidx-1);
				 else S2N(OKFDD_Maxidx-1);
				 break;
	         case D_posD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      P2S(OKFDD_Maxidx-1);
				 else P2N(OKFDD_Maxidx-1);
				 break;
		 case D_negD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      N2S(OKFDD_Maxidx-1);
				 else N2P(OKFDD_Maxidx-1);
	      }

	      for (sift = OKFDD_Maxidx-2; sift >= i; sift--)
                OKFDD_Levelexchange(sift);
	   }
	}
	gesamtkosten(0,OKFDD_Maxidx-1);
	min_size_i = OKFDD_Now_size_i;

	cout(ros_a) << OKFDD_Now_size_i << "\n";

	delete[] zeile;
	delete[] spalte;
	delete[] label;
	delete[] teilmenge;
	delete[] breite;
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Friedman                                   //
// Bestimmung des absoluten Min. einer Fkt. fuer alle DTLs (GRAY-Code)!!!    //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Friedman_PS(	usint von,
					usint bis)
{
	int    z,i,j, opos, sift, zeiger, zwei_hoch_n=1, drei_hoch_n=1, grenze;
	char*  zeile = new char[OKFDD_Maxidx];
	char*  spalte = new char[OKFDD_Maxidx];
	ulint  bbggg=1;
	usint  tma,tmac;
	usint  adr[pi_limit];
	uchar  flag_p=0, flag_a=0, flag_b=0;

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Vorverdichtung
	// --------------
	if (OKFDD_Outputflags & ros_p) { OKFDD_Outputflags^=ros_p; flag_p=1; }
	if (OKFDD_Outputflags & ros_a) { OKFDD_Outputflags^=ros_a; flag_a=1; }
	if (OKFDD_Outputflags & ros_b) { OKFDD_Outputflags^=ros_b; flag_b=1; }

	OKFDD_DD_to_BDD(von,bis,1);
	OKFDD_Siftlight(von,bis,'v');
	OKFDD_Siftlight(von,bis,'v');
	OKFDD_Siftlight(von,bis,'v');

	if(flag_p==1) OKFDD_Outputflags^=ros_p;
	if(flag_a==1) OKFDD_Outputflags^=ros_a;
	if(flag_b==1) OKFDD_Outputflags^=ros_b;

	// Initialisierung
	// ---------------
	min_count = 0;
//      for (i = von; i <= bis; i++) {
	for (i = 0; i < OKFDD_Maxidx; i++)
        {
	   spalte[i] = 0;
	   twin_pi_table[i] = min_pi_table[i] = OKFDD_PI_Order_Table[i];
	   zeile[i] = mindtl[OKFDD_PI_Order_Table[i]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]];
	}
	min_size_i = OKFDD_Now_size_i;

	for (i = von; i <= bis; i++)
        {
		drei_hoch_n *= 3;
		adr[OKFDD_PI_Order_Table[i]] = zwei_hoch_n;
		zwei_hoch_n *= 2;
		bbggg += OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[i]];
	}

	usint* label     = new usint[zwei_hoch_n];
	ulint* breite    = new ulint[zwei_hoch_n];
	usint* teilmenge = new usint[zwei_hoch_n];

	teilmenge[0] = 0;
	breite[0]    = 0;
	label[0]     = 0;
	grenze       = von;

	// Durchlaufe die Zerlegungstypen P,S im 'GRAY-Code'
	// -------------------------------------------------
	for (z = 0; z < zwei_hoch_n; z++)
        {
	   Prozentbalken(z,zwei_hoch_n-1,'F')

	   for (tma = adr[twin_pi_table[grenze]]; tma < zwei_hoch_n; tma++)
           {
	      if ((tma & adr[twin_pi_table[grenze]])||!z)
              {
		 zeiger = von-1;
		 tmac   = tma;
		 for (sift = von; sift <= bis && tmac; sift++)
                 {
		    if (tmac&1)
                    {
		       zeiger++;
		       opos = zeiger;
		       while (twin_pi_table[sift]!=OKFDD_PI_Order_Table[opos])
                         opos++;
		       if (opos!=zeiger) for (j = opos-1; j >= zeiger; j--)
                         OKFDD_Levelexchange(j);
		     }
		     tmac >>=1;
		 }

		 // Beste Bottom-Variable bestimmen
		 // -------------------------------
		 breite[tma] = bbggg;
		 for (sift = zeiger; sift >= von; sift--)
                 {
		    if (breite[tma-adr[OKFDD_PI_Order_Table[sift]]] <
                      breite[tma])
                    {
		       for (j = sift; j < zeiger; j++) OKFDD_Levelexchange(j);
		       if (breite[tma] > breite[tma-
                         adr[OKFDD_PI_Order_Table[zeiger]]] +
			 OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[
                         zeiger]])
                       {
			  teilmenge[tma] = tma-
                            adr[OKFDD_PI_Order_Table[zeiger]];
			  breite[tma]    = breite[teilmenge[tma]] +
                            OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[
                            zeiger]];
			  label[tma]     = OKFDD_PI_Order_Table[zeiger];
		       }
		    }
		 }
	      }
	   }

	   // Best. der besten Reihenfolge
	   // ----------------------------
	   if (min_size_i > breite[zwei_hoch_n-1])
           {
	      tma   = zwei_hoch_n-1;
	      bbggg = min_size_i = breite[tma];
	      for (i = bis; i >= von; i--)
              {
		 min_pi_table[i]    = label[tma];
		 mindtl[label[tma]] = OKFDD_PI_DTL_Table[label[tma]];
		 tma		    = teilmenge[tma];
	      }
	   }

	   // Veraenderung des Zerlegungstyps
	   // -------------------------------
	   zeiger = bis;
	   spalte[zeiger]++;
	   while (spalte[zeiger] == 2)
	   {
	      spalte[zeiger] = 0;
	      zeile[zeiger]++;
	      if (zeile[zeiger] == 2) zeile[zeiger] = 0;
	      if (zeiger > von) spalte[--zeiger]++;
	   }

	   opos = von;
	   while (twin_pi_table[zeiger]!=OKFDD_PI_Order_Table[opos]) opos++;
	   for (sift = opos; sift < OKFDD_Maxidx-1; sift++)
             OKFDD_Levelexchange(sift);

	   switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	   {
	      case D_Shan : S2P(OKFDD_Maxidx-1); break;
	      case D_posD : P2S(OKFDD_Maxidx-1);
	   }

	   for (sift = OKFDD_Maxidx-2; sift >= opos; sift--)
             OKFDD_Levelexchange(sift);
	   grenze = zeiger;
	}
	// optimale Position bestimmen und einnehmen
	// -----------------------------------------
	for (i = von; i <= bis; i++)
	{
	   opos = bis;
           while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos--;

	   if (mindtl[min_pi_table[i]]==OKFDD_PI_DTL_Table[
             OKFDD_PI_Order_Table[opos]])
             for (sift = opos-1; sift >= i; sift--)
               OKFDD_Levelexchange(sift);
	   else
	   {
	      for (sift = opos; sift < OKFDD_Maxidx-1; sift++)
                OKFDD_Levelexchange(sift);

	      switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	      {
		 case D_Shan :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_posD)
                                      S2P(OKFDD_Maxidx-1);
				 else S2N(OKFDD_Maxidx-1);
				 break;
		 case D_posD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      P2S(OKFDD_Maxidx-1);
				 else P2N(OKFDD_Maxidx-1);
				 break;
		 case D_negD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      N2S(OKFDD_Maxidx-1);
				 else N2P(OKFDD_Maxidx-1);
	      }

	      for (sift = OKFDD_Maxidx-2; sift >= i; sift--)
                OKFDD_Levelexchange(sift);
	   }
	}
	gesamtkosten(0,OKFDD_Maxidx-1);
	min_size_i = OKFDD_Now_size_i;

	cout(ros_a) << OKFDD_Now_size_i << "\n";

	delete[] zeile;
	delete[] spalte;
	delete[] label;
	delete[] teilmenge;
	delete[] breite;
}
/* ========================================================================= */



/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Permutation	                             //
// Bestimmung des absoluten Min. einer Fkt. fuer alle DTLs (GRAY-Code)!!     //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Permutation(     usint von,
					usint bis)
{
	int     i, opos, sift, zeiger, drei_hoch_n=1;
	char*   zeile = new char[OKFDD_Maxidx];
	char*   spalte = new char[OKFDD_Maxidx];

	cout(ros_b) << "DD-SIZE : " << OKFDD_Now_size_i << " =>\n";
	cout(ros_a) << " => ";
	cout((ros_b||ros_a)).flush();

	// Initialisierung
	// ---------------
	min_count = OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   spalte[OKFDD_Maxidx]	= 0;
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   zeile[OKFDD_Maxidx] = mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	for (i = von; i <= bis; i++) drei_hoch_n*=3;

	// best. der besten Reihenfolge (fuer init. DTL)
	// ---------------------------------------------
	bbr(von,bis);

	// Durchlaufe alle Zerlegungstypen im 'GRAY-Code'
	// ----------------------------------------------
	for (i = 1; i < drei_hoch_n; i++)
	{
	   Prozentbalken(i,drei_hoch_n-1,'P')

	   // Veraenderung des Zerlegungstyps
	   // -------------------------------
	   zeiger = bis;
	   spalte[zeiger]++;
	   while (spalte[zeiger] == 3)
	   {
	      spalte[zeiger] = 0;
	      zeile[zeiger]++;
	      if (zeile[zeiger] == 3) zeile[zeiger] = 0;
	      spalte[--zeiger]++;
	   }

	   for (sift = zeiger; sift < OKFDD_Maxidx-1; sift++)
             OKFDD_Levelexchange(sift);

	   switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	   {
	      case D_Shan : S2N(OKFDD_Maxidx-1); break;
	      case D_negD : N2P(OKFDD_Maxidx-1); break;
	      case D_posD : P2S(OKFDD_Maxidx-1);
	   }

	   for (sift = OKFDD_Maxidx-2; sift >= zeiger; sift--)
             OKFDD_Levelexchange(sift);
		gesamtkosten(von,bis);

	   // Best. der besten Reihenfolge
	   // ----------------------------
	   bbr(von,bis);
	}

	// Optimale Position bestimmen und einnehmen
	// -----------------------------------------
	for (i = von; i <= bis; i++)
	{
	   opos = bis; 
           while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos--;

	   if (mindtl[min_pi_table[i]]==OKFDD_PI_DTL_Table[
             OKFDD_PI_Order_Table[opos]])
	     for (sift = opos-1; sift >= i; sift--)
               OKFDD_Levelexchange(sift);
	   else
	   {
	      for (sift = opos; sift < OKFDD_Maxidx-1; sift++)
                OKFDD_Levelexchange(sift);

	      switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	      {
		 case D_Shan :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_posD)
                                      S2P(OKFDD_Maxidx-1);
				 else S2N(OKFDD_Maxidx-1);
				 break;
		 case D_posD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      P2S(OKFDD_Maxidx-1);
				 else P2N(OKFDD_Maxidx-1);
				 break;
		 case D_negD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      N2S(OKFDD_Maxidx-1);
				 else N2P(OKFDD_Maxidx-1);
	      }

	      for (sift = OKFDD_Maxidx-2; sift >= i; sift--)
                OKFDD_Levelexchange(sift);
	   }
	}
	gesamtkosten(von,bis);

	cout(ros_a) << OKFDD_Now_size_i << "\n";

	delete[] zeile;
	delete[] spalte;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Sethhis			                     //
// Nimmt Minimum ein, dass bereits vorher bestimmt wurde		     //
/* ========================================================================= */
void dd_man::OKFDD_Setthis(usint* min_pi_table,
			   uchar* mindtl      )
{
	int i, opos, sift;

	if (min_pi_table == NULL) for (i = OKFDD_Maxidx-1; i >=0 ; i--)
          twin_pi_table[i] = OKFDD_PI_Order_Table[i];
	else for (i = OKFDD_Maxidx-1; i >=0 ; i--)
          twin_pi_table[i] = min_pi_table[i];

	if (mindtl != NULL)
	{
	   for (i = OKFDD_Maxidx-1; i >=0 ; i--)
	   {
	      Prozentbalken(OKFDD_Maxidx-i,OKFDD_Maxidx,'D')
	      if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[i]]!=
                mindtl[OKFDD_PI_Order_Table[i]])
	      {
		 for (sift = i ; sift < OKFDD_Maxidx-1 ; sift++)
                   OKFDD_Levelexchange(sift);

		 switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                   1]])
		 {
		    case D_Shan :   if (mindtl[OKFDD_PI_Order_Table[
                                      OKFDD_Maxidx-1]]==D_posD)
                                        S2P(OKFDD_Maxidx-1);
				    else if (mindtl[OKFDD_PI_Order_Table[
                                      OKFDD_Maxidx-1]]==D_negD)
                                        S2N(OKFDD_Maxidx-1);
				    break;
		    case D_posD :   if (mindtl[OKFDD_PI_Order_Table[
                                      OKFDD_Maxidx-1]]==D_Shan)
                                        P2S(OKFDD_Maxidx-1);
				    else if (mindtl[OKFDD_PI_Order_Table[
                                      OKFDD_Maxidx-1]]==D_negD)
                                        P2N(OKFDD_Maxidx-1);
				    break;
		    case D_negD :   if (mindtl[OKFDD_PI_Order_Table[
                                      OKFDD_Maxidx-1]]==D_Shan)
                                        N2S(OKFDD_Maxidx-1);
				    else if (mindtl[OKFDD_PI_Order_Table[
                                      OKFDD_Maxidx-1]]==D_posD)
                                        N2P(OKFDD_Maxidx-1);
				    break;
		 }

		 for (sift = OKFDD_Maxidx-2; sift >= i ; sift--)
                   OKFDD_Levelexchange(sift);
	      }
	      gesamtkosten(0,OKFDD_Maxidx-1);
	   }
	   cout(ros_p) <<"\n";
	}

	for (i = OKFDD_Maxidx-1; i >=0 ; i--)
	{
	   Prozentbalken(OKFDD_Maxidx-i,OKFDD_Maxidx,'O')
	   if (twin_pi_table[i]!=OKFDD_PI_Order_Table[i])
           {
	      opos = 0;
              while (twin_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos++;
	      for (sift = opos; sift < i; sift++) OKFDD_Levelexchange(sift);
	      gesamtkosten(0,OKFDD_Maxidx-1);
	   }
	}
	cout(ros_p) <<"\n";

}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Rotate_Max_Lvl			             //
// Rotation des Zerlegungstyps im untersten Level		             //
/* ========================================================================= */
void dd_man::OKFDD_Rotate_Max_Lvl(      usint maxpos,
					char  dir   )
{
	// Vorwaerts
	// ---------
	if (dir == 1)
	{
		switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[maxpos]])
		{
			case D_Shan : S2P(maxpos); break;
			case D_posD : P2N(maxpos); break;
			case D_negD : N2S(maxpos);
		}
	}

	// Rueckwaerts
	// -----------
	else if (dir == -1)
	{
		switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[maxpos]])
		{
			case D_Shan : S2N(maxpos); break;
			case D_posD : P2S(maxpos); break;
			case D_negD : N2P(maxpos);
		}
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Chg_XOR		                     //
// Aenderung des Zerlegungstyps einer Variable in bel. Hoehe	             //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Chg_XOR( usint level,
				uchar newtype)
{
//      cout(1) << "Changing level: " << level << "\n";

	// Only perform if needed
	// ----------------------
	if ((J4 = newtype) ==
          (J3 = OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[level]])) return;
	else OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[level]] = J4;

	J2 = OKFDD_PI_Order_Table[level];

	// Calculate all OKFDDs
	// --------------------
	fui(loop,0,OKFDD_P_O-1)
	{
	   // Calculate new OKFDD
	   // -------------------
	   uth1 = OKFDD_Change_DTL(toprime[All_Prime[pi_limit-1-loop]]->root);
           ups(uth1);

	   // Remove old one
	   // --------------
	   OKFDD_Free_Node(toprime[All_Prime[pi_limit-1-loop]]->root);

	   // Store new one
	   // -------------
	   toprime[All_Prime[pi_limit-1-loop]]->root = uth1;
	}

	// Remove all dirty trees
	// ----------------------
	OKFDD_Free_Fct_Cache();

	return;
};
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Chg_Bottom		                     //
// Aenderung des Zerlegungstyps einer Variable in bel. Hoehe	             //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Chg_Bottom(      usint level,
					uchar newtype)
{
	int sift;

	// Initialisierung der Groessen, Reihenfolgen und Zerlegungstypen
	// --------------------------------------------------------------
	min_count = OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	min_size_i = OKFDD_Now_size_i;

	// Level nach unten schieben
	// -------------------------
	for (sift = level; sift < OKFDD_Maxidx-1; sift++)
	{
		OKFDD_Levelexchange(sift);
		gesamtkosten(sift,OKFDD_Maxidx-1);
	}

	// Zerlegungstyp aendern
	// ---------------------
	if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]]==D_Shan)
	{
		if (newtype==D_posD) S2P(OKFDD_Maxidx-1);
		if (newtype==D_negD) S2N(OKFDD_Maxidx-1);
	}
	else
	{
		if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]]==
                  D_posD)
		{
			if (newtype==D_Shan) P2S(OKFDD_Maxidx-1);
			if (newtype==D_negD) P2N(OKFDD_Maxidx-1);
		}
		else if (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                  1]]==D_negD)
		{
			if (newtype==D_Shan) N2S(OKFDD_Maxidx-1);
			if (newtype==D_posD) N2P(OKFDD_Maxidx-1);
		}
	}

	// Zrueckschieben
	// --------------
	for (sift = OKFDD_Maxidx-2; sift >= level; sift--)
	{
		OKFDD_Levelexchange(sift);
		gesamtkosten(sift,OKFDD_Maxidx-1);
	}

	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::S2P // Typkonvertierung im untersten Level S->P      //
/* ========================================================================= */
void dd_man::S2P(usint maxpos)
{
	// Zerlegungstyp anpassen
	// ----------------------
	OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[maxpos]] = D_posD;

	// Zeiger anpassen
	// ---------------
	if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[maxpos]])
          ut[OKFDD_PI_Order_Table[maxpos]][2]->hi_p = OKFDD_ONE;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::S2N // Typkonvertierung im untersten Level S->N      //
/* ========================================================================= */
void dd_man::S2N(usint maxpos)
{
	// Zerlegungstyp anpassen
	// ----------------------
	OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[maxpos]] = D_negD;

	if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[maxpos]])
	{
		// Zeiger anpassen
		// ---------------
		ut[OKFDD_PI_Order_Table[maxpos]][2]->hi_p = OKFDD_ONE;
		ut[OKFDD_PI_Order_Table[maxpos]][2]->lo_p = OKFDD_ZERO;

		// ID des untersten Knotens speichern
		// ----------------------------------
		I4 = ut[OKFDD_PI_Order_Table[maxpos]][2]->idnum;

		// Marke auf Knoten anpassen !!!
		// -----------------------------
		OKFDD_Switch_all();
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::P2S // Typkonvertierung im untersten Level P->S      //
/* ========================================================================= */
void dd_man::P2S(usint maxpos)
{
	// Zerlegungstyp anpassen
	// ----------------------
	OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[maxpos]] = D_Shan;

	// Zeiger anpassen
	// ---------------
	if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[maxpos]])
          ut[OKFDD_PI_Order_Table[maxpos]][2]->hi_p = OKFDD_ZERO;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::P2N // Typkonvertierung im untersten Level P->N      //
/* ========================================================================= */
void dd_man::P2N(usint maxpos)
{
	// Zerlegungstyp anpassen
	// ----------------------
	OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[maxpos]] = D_negD;

	if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[maxpos]])
	{
		// Zeiger anpassen
		// ---------------
		ut[OKFDD_PI_Order_Table[maxpos]][2]->lo_p = OKFDD_ZERO;

		// ID des untersten Knotens speichern
		// ----------------------------------
		I4 = ut[OKFDD_PI_Order_Table[maxpos]][2]->idnum;

		// Marke auf Knoten anpassen !!!
		// -----------------------------
		OKFDD_Switch_all();
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::N2S // Typkonvertierung im untersten Level N->S      //
/* ========================================================================= */
void dd_man::N2S(usint maxpos)
{
	// Zerlegungstyp anpassen
	// ----------------------
	OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[maxpos]] = D_Shan;

	if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[maxpos]])
	{
		// Zeiger anpassen
		// ---------------
		ut[OKFDD_PI_Order_Table[maxpos]][2]->lo_p = OKFDD_ZERO;

		// ID des untersten Knotens speichern
		// ----------------------------------
		I4 = ut[OKFDD_PI_Order_Table[maxpos]][2]->idnum;

		// Marke auf Knoten anpassen !!!
		// -----------------------------
		OKFDD_Switch_all();
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::N2P // Typkonvertierung im untersten Level N->P      //
/* ========================================================================= */
void dd_man::N2P(usint maxpos)
{
	// Zerlegungstyp anpassen
	// ----------------------
	OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[maxpos]] = D_posD;

	if (OKFDD_No_UTNodes_Per_Lvl[OKFDD_PI_Order_Table[maxpos]])
	{
		// Zeiger anpassen
		// ---------------
		ut[OKFDD_PI_Order_Table[maxpos]][2]->lo_p = OKFDD_ZERO;

		// ID des untersten Knotens speichern
		// ----------------------------------
		I4 = ut[OKFDD_PI_Order_Table[maxpos]][2]->idnum;

		// Marke auf Knoten anpassen !!!
		// -----------------------------
		OKFDD_Switch_all();
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    INTERHAND                                                    //
// Determines what to do after each gate construction in R_M_LVL	     //
/* ========================================================================= */
void INTERHAND(dd_man* dd_manager, utnode* root)
{
	// If entry condition reached -> Start reordering process
	// ------------------------------------------------------
	if ((float)(dd_manager->OKFDD_Now_size_i) /
          (dd_manager->OKFDD_Total_nodes + 1) > dd_manager->OKFDD_Tempfactor)
	{
	   if (dd_manager->OKFDD_Now_size_i > dd_manager->OKFDD_Siftbase)
	   {
	      // Store actual size
	      // -----------------
	      ulint check = dd_manager->OKFDD_Now_size_i;

	      if (dd_manager->OKFDD_Outputflags & 1) cout << "\n";

	      // INTERRUPT: Perform action
	      // -------------------------
	      switch(dd_manager->OKFDD_Temproutine)
	      {
		 case 1: dd_manager->OKFDD_Siftlight(0,
                           dd_manager->OKFDD_Maxidx-1,'v');
			 break;
		 case 2: dd_manager->OKFDD_Sifting(0,
                           dd_manager->OKFDD_Maxidx-1,
                           dd_manager->OKFDD_Siftfactor,'a','v');
			 break;
		 case 3: dd_manager->OKFDD_DTL_Sifting(0,
                           dd_manager->OKFDD_Maxidx-1,
                           dd_manager->OKFDD_Siftfactor,'a','v');
			 break;
		 case 4: dd_manager->OKFDD_DTL_Sifting_PS(0,
                           dd_manager->OKFDD_Maxidx-1,
                           dd_manager->OKFDD_Siftfactor,'a','v');
			 break;
		 case 5: dd_manager->OKFDD_Winpermutation(0,
                           dd_manager->OKFDD_Maxidx-1,5,5);
			 break;
	      }

	      // New size acceptable
	      // -------------------
	      if (dd_manager->OKFDD_Now_size_i < check)
                dd_manager->OKFDD_Total_nodes = dd_manager->OKFDD_Now_size_i;
	   }
	}
}
/* ========================================================================= */





















/* ================================================================================ */
// Function:    dd_man::quantumkosten                                               //
// Vergl., ob mom.Quantumkosten minimaler / evtl.saven d.Reihenf.o.bei = zaehlen    //
/* ================================================================================ */
void dd_man::quantumkosten(usint von,
            usint bis)
{
  OKFDD_Now_cost = 0;
  OKFDD_Line_cost = 0;

  for( int i = 0; i < OKFDD_P_O; i++ )
  {
    reset_flags( OKFDD_PO_Root_Table( OKFDD_PO_Table( i ) ) );
  }

  for( int i = 0; i < OKFDD_P_O; i++ )
  {
    if( ( ( ulint )( m_and( OKFDD_PO_Root_Table( OKFDD_PO_Table( i ) ) ) )->hi_p & 1 ) == 1 )
    {
      OKFDD_Now_cost += 1;
    }
    berechne_quantumkosten( OKFDD_PO_Root_Table( OKFDD_PO_Table( i ) ) );
  }
  OKFDD_Line_cost += OKFDD_P_I;

  if( ( siftingType == 0 ? min_cost_quantum > OKFDD_Now_cost : min_cost > OKFDD_Line_cost ) )
//  if( min_cost > OKFDD_Line_cost )
//      || ( min_cost == OKFDD_Line_cost && min_cost_quantum > OKFDD_Now_cost ) )
  {
    for( int i = von; i <= bis; i++ )
    {
      min_pi_table[ i ] = OKFDD_PI_Order_Table[ i ];
      mindtl[ OKFDD_PI_Order_Table[ i ] ] = OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ i ] ];
    }
    min_cost = OKFDD_Line_cost;
    min_cost_quantum = OKFDD_Now_cost;
  }
}
/* ========================================================================= */


/* ================================================================================ */
// Function:    dd_man::reset_flags                                                 //
// Flags der Knoten im OKFDD zuruecksetzen                                          //
/* ================================================================================ */
void dd_man::reset_flags( utnode* node )
{
  utnode *highNode, *lowNode;

  S_lhshared = 0;
  S_lchshared = 0;
  S_lh = 0;
  S_lch = 0;
  S_idlh = 0;
  S_id1 = 0;
  S_idlchshared = 0;
  S_idlch = 0;
  S_l0 = 0;
  S_l1 = 0;
  S_0h = 0;
  S_0ch = 0;
  S_1h = 0;
  S_1ch = 0;
  S_10 = 0;
  S_01 = 0;

  //return if node is correctly set (correct in_count and unset visFlag)
  if( node->visFlag == 0 
    && ( ( node->ref_c == 0 && node->in_count == 1 )
    || ( node->in_count == node->ref_c - 1 ) ) )
  {
    return;
  }
  else
  {
    node->visFlag = 0;

    if( node->ref_c == 0 )
      node->in_count = 1;
    else
      node->in_count = node->ref_c - 1;
  }

  if( node == OKFDD_ONE || node == OKFDD_ZERO )
  {
    return;
  }
  // recursive calls
  lowNode  = ( m_and( ( m_and( node ) )->lo_p ) );
  highNode = ( m_and( ( m_and( node ) )->hi_p ) );

  if( !( highNode == OKFDD_ONE || highNode == OKFDD_ZERO ) )
  {
    reset_flags( highNode );
  }
  if( !( lowNode == OKFDD_ONE || lowNode == OKFDD_ZERO ) )
  {
    reset_flags( lowNode );
  }
}


/* ================================================================================ */
// Function:    dd_man::berechne_quantumkosten                                      //
// Kosten den OKFDD's ermitteln und in OKFDD_Now_size_i speichern                   //
/* ================================================================================ */
void dd_man::berechne_quantumkosten( utnode* node )
{
  utnode *lowNode, *highNode;

  lowNode  = ( m_and( ( m_and( node ) )->lo_p ) );
  highNode = ( m_and( ( m_and( node ) )->hi_p ) );

  //return if node is not marked as unvisited
  if( node->visFlag != 0 )
  {
    return;
  }
  //mark as visited
  node->visFlag = 1;

  //return if constant
  if( node == OKFDD_ONE || node == OKFDD_ZERO )
  {
    return;
  }

  // recursive calls
  if( !( highNode == OKFDD_ONE || highNode == OKFDD_ZERO ) )
  {
    berechne_quantumkosten( highNode );
  }
  if( !( lowNode == OKFDD_ONE || lowNode == OKFDD_ZERO ) )
  {
    berechne_quantumkosten( lowNode );
  }

  bool complement = ( ( ulint )( m_and( node ) )->hi_p & 1 ) == 1;
  unsigned short dtl = ( unsigned short )OKFDD_PI_DTL_Table[ OKFDD_Label( node ) ];
  debug_on = false;

  //add costs for current type of node
  if( complement )
  {
    if( lowNode == OKFDD_ZERO )
    {
      if( highNode == OKFDD_ONE )
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_01 ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_01 ];

	if( debug_on )
	  printf( "S_01" );
      }
      else
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_0ch ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_0ch ];

	if( debug_on )
	  printf( "S_0ch" );
      }
    }
    else if( lowNode == OKFDD_ONE )
    {
      if( highNode == OKFDD_ZERO )
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_10 ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_10 ];

	if( debug_on )
	  printf( "S_10" );
      }
      else
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_1ch ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_1ch ];

	if( debug_on )
	  printf( "S_1ch" );
      }
    }
    else if( lowNode != highNode )
    {
      if( highNode == OKFDD_ONE )
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_l1 ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_l1 ];

	if( debug_on )
	  printf( "S_l1" );
      }
      else if( highNode == OKFDD_ZERO )
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_l0 ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_l0 ];

	if( debug_on )
	  printf( "S_l0" );
      }
      else
      {
	if( lowNode->in_count >= 2 || highNode->in_count >= 2 )
	{
	  OKFDD_Line_cost += line_costs[ dtl ][ Gate_lchshared ];
	  OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_lchshared ];

	  if( debug_on )
	    printf( "S_lchshared" );
	}
	else
	{
	  OKFDD_Line_cost += line_costs[ dtl ][ Gate_lch ];
	  OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_lch ];

	  if( debug_on )
	    printf( "S_lch" );
	}
      }
    }
    else
    {
      if( lowNode->in_count >= 3 || highNode->in_count >= 3 )
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_idlchshared ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_idlchshared ];

	if( debug_on )
	  printf( "S_idlchshared" );
      }
      else
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_idlch ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_idlch ];

	if( debug_on )
	  printf( "S_idlch" );
      }
    }
  }
  else
  {
    if( lowNode == OKFDD_ZERO )
    {
      if( highNode == OKFDD_ONE)
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_01 ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_01 ];

	if( debug_on )
	  printf( "S_01" );
      }
      else
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_0h ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_0h ];

	if( debug_on )
	  printf( "S_0h" );
      }
    }
    else if( lowNode == OKFDD_ONE )
    {
      if( highNode == OKFDD_ZERO )
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_10 ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_10 ];

	if( debug_on )
	  printf( "S_10" );
      }
      else if( highNode == OKFDD_ONE )
      {
      // both terminals are one
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_id1 ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_id1 ];

	if( debug_on )
	  printf( "n_id1" );
      }
      else
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_1h ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_1h ];

	if( debug_on )
	  printf( "S_1h" );
      }
    }
    else if( highNode == OKFDD_ONE )
    {
      OKFDD_Line_cost += line_costs[ dtl ][ Gate_l1 ];
      OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_l1 ];

      if( debug_on )
	printf( "S_l1" );
    }
    else if( highNode == OKFDD_ZERO )
    {
      OKFDD_Line_cost += line_costs[ dtl ][ Gate_l0 ];
      OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_l0 ];

      if( debug_on )
	printf( "S_l0" );
    }
    else if( lowNode != highNode )
    {
      if( lowNode->in_count >= 2 || highNode->in_count >= 2 )
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_lhshared ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_lhshared ];

	if( debug_on )
	  printf( "S_lhshared" );
      }
      else
      {
	OKFDD_Line_cost += line_costs[ dtl ][ Gate_lh ];
	OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_lh ];

	if( debug_on )
	  printf( "S_lh" );
      }
    }
    else
    {
    // nodell nD || nodell shared nD
      OKFDD_Line_cost += line_costs[ dtl ][ Gate_idlh ];
      OKFDD_Now_cost += quantum_costs[ dtl ][ Gate_idlh ];

      if( debug_on )
	printf( "n_idlh" );
    }
  }
  lowNode->in_count -= 1;
  highNode->in_count -= 1;

  if( debug_on )
  {
    printf( " -> momentane Kosten (lines) %lu\n", OKFDD_Line_cost );
    printf( " -> momentane Kosten (quantum) %lu\n", OKFDD_Now_cost );
  }
}


/* ========================================================================= */
// Function:    dd_man::OKFDD_Sifting_Quantum // Sifting		     //
/* ========================================================================= */
void dd_man::OKFDD_Sifting_Quantum( usint von,
                usint bis,
                float faktor,
                char  rel,
                char  art )
{
  int          sift, var, opos, mitte, abb, old_size, frei, belegt, old_cost;
  unsigned int old_no_ut[pi_limit];

  // Schiebe belegte unique tables zusammen
  // --------------------------------------
  frei = von;

  while( OKFDD_No_UTNodes_Per_Lvl[ OKFDD_PI_Order_Table[ frei ] ] && frei <= bis )
    frei++;

  for( belegt = frei + 1; belegt <= bis; belegt++ )
  {
    if( OKFDD_No_UTNodes_Per_Lvl[ OKFDD_PI_Order_Table[ belegt ] ] )
    {
      sift = OKFDD_PI_Order_Table[ frei ];
      OKFDD_PI_Order_Table[ frei ] = OKFDD_PI_Order_Table[ belegt ];
      OKFDD_PI_Order_Table[ belegt ] = sift;
      var = OKFDD_PI_Level_Table[ OKFDD_PI_Order_Table[ frei ] ];
      OKFDD_PI_Level_Table[ OKFDD_PI_Order_Table[ frei ] ] = OKFDD_PI_Level_Table[ OKFDD_PI_Order_Table[ belegt ] ];
      OKFDD_PI_Level_Table[ OKFDD_PI_Order_Table[ belegt ] ] = var;
      frei++;
    }
  }
  bis = frei - 1;

  // aktualisiere Hilfsfelder
  // ------------------------
  if( faktor < 1.01 )
    faktor = 1.01;
  min_count = 0;

  for( var = 0; var < OKFDD_Maxidx; var++ )
  {
    old_no_ut[ OKFDD_PI_Order_Table[ var ] ] = OKFDD_No_UTNodes_Per_Lvl[ OKFDD_PI_Order_Table[ var ] ];
    twin_pi_table[ var ] = min_pi_table[ var ] = OKFDD_PI_Order_Table[ var ];
    mindtl[ OKFDD_PI_Order_Table[ var ] ] = OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ var ] ];
  }
  old_size = min_size_i = OKFDD_Now_size_i;
  old_cost = min_cost = OKFDD_Now_cost;

  // suche ersten Kandidaten
  // -----------------------
  switch( art )
  {
    case 'r':  // bestimme zufaellige Levelreihenfolge
               // ------------------------------------
      for( var = von; var <= bis; var++ )
      {
        sift                  = bis - ( rand() % ( bis-var+1 ) );
        opos		          = twin_pi_table[ var ];
        twin_pi_table[ var ]  = twin_pi_table[ sift ];
        twin_pi_table[ sift ] = opos;
      }
      break;

    case 'g':  // sortiere Level nach absteigender Breite
               // ---------------------------------------
      for (var = von; var <= bis; var++)
      {
        for (sift = var+1; sift <= bis; sift++)
        {
          if( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ var ] ] < OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] )
          {
            opos	              = twin_pi_table[ var ];
            twin_pi_table[ var ]  = twin_pi_table[ sift ];
            twin_pi_table[ sift ] = opos;
          }
        }
      }
      break;

    case 'k':  // suche max. Level
               // ----------------
      opos = von;

      for( sift = von + 1; sift <= bis; sift++ )
        if( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] > OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ opos ] ] )
          opos = sift;

      if( opos != von )
      {
        sift		          = twin_pi_table[ opos ];
        twin_pi_table[ opos ] = twin_pi_table[ von ];
        twin_pi_table[ von ]  = sift;
      }
      break;

    case 'l':  // suche max. Level
               // ----------------
      opos = von;

      for( sift = von + 1; sift <= bis; sift++ )
        if( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] > OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ opos ] ] )
          opos = sift;

      if( opos != von )
      {
        sift		          = twin_pi_table[ opos ];
        twin_pi_table[ opos ] = twin_pi_table[ von ];
        twin_pi_table[ von ]  = sift;
      }
      break;

    case 'v':  // suche vielversprechendes Level
               // -------------------------------
      levelstat();
      opos = von;

      for( sift = von + 1; sift <= bis; sift++ )
        if( lsf[ twin_pi_table[ sift ] ] > lsf[ twin_pi_table[ opos ] ] )
          opos = sift;

      if( opos != von )
      {
        sift		          = twin_pi_table[ opos ];
        twin_pi_table[ opos ] = twin_pi_table[ von ];
        twin_pi_table[ von ]  = sift;
      }
  }

  // Calculate quantumcosts at the start of the sifting
  // and initialize variables
  // ----------------------------------------
  mitte = OKFDD_Maxidx/2;

  debug_on = false;
  debug_costs = false;

  quantumkosten( von, bis );
  
  old_cost = min_cost = OKFDD_Line_cost;
  min_cost_quantum = OKFDD_Now_cost;

  if( debug_on || debug_costs )
  {
    printf( "Kosten zu Beginn %lu\n", OKFDD_Now_cost );
  }
  
  // Search for local minimum of each variable
  // ----------------------------------------
  for( var = von; var <= bis; var++ )
  {
    // Vergroesserungsfaktor relativieren
    // ----------------------------------
    if( rel == 'r' )
      old_cost = OKFDD_Line_cost;

    opos = von;

    // Position der aktuellen Variable bestimmen
    while( OKFDD_PI_Order_Table[ opos ] != twin_pi_table[ var ] )
      opos++;

    // Variable steht in der oberen Haelfte des DDs => zuerst nach oben schieben
    // -----------------------------------------------
    if( opos < mitte )
    {
      // Schiebe Level in Richtung "von" unter Beachtung des Abbruchfaktors
      // -------------------------------------------------
      for( sift = opos - 1; sift >= von && OKFDD_Line_cost < old_cost * faktor; sift-- )
      {
        OKFDD_Levelexchange( sift );
        quantumkosten( von, bis );
      }
      abb = sift + 1;

      // zurueckstellen
      //---------------
      for( sift = abb; sift < opos; sift++ )
        OKFDD_Levelexchange( sift );

      // Schiebe Level in Richtung "bis" unter Beachtung des Abbruchfaktors
      // -------------------------------------------------
      for( sift = opos; sift < bis && OKFDD_Line_cost < old_cost * faktor; sift++ )
      {
        OKFDD_Levelexchange( sift );
        quantumkosten( von, bis );
      }

      // Optimale Position bestimmen und einnehmen
      // -----------------------------------------
      opos = abb;
      abb = sift - 1;

      while( min_pi_table[ opos ] != twin_pi_table[ var ] )
	opos++;

      for( sift = abb; sift >= opos; sift--)
	OKFDD_Levelexchange( sift );
    }

    // Variable steht in der unteren Haelfte des DDs => zuerst nach
    // unten schieben
    // ------------------------------------------------------------
    else
    {
      // Schiebe Level in Richtung bis unter Beachtung des
      // Abbruchfaktors
      // -------------------------------------------------
      for( sift = opos; sift < bis && OKFDD_Line_cost < old_cost * faktor; sift++ )
      {
        OKFDD_Levelexchange( sift );
        quantumkosten( von, bis );
      }

      // zurueckstellen
      //---------------
      abb = sift - 1;

      for( sift = abb; sift >= opos; sift-- )
        OKFDD_Levelexchange( sift );

      // Schiebe Level in Richtung von unter Beachtung des
      // Abbruchfaktors
      // -------------------------------------------------
      for( sift = opos - 1; sift >= von && OKFDD_Line_cost < old_cost * faktor; sift-- )
      {
        OKFDD_Levelexchange( sift );
        quantumkosten( von, bis );
      }
      abb = sift+1;

      // optimale Position bestimmen und einnehmen
      // -----------------------------------------
      opos = abb;
      abb = sift + 1;

      while( min_pi_table[ opos ] != twin_pi_table[ var ] )
	opos++;

      for( sift = abb; sift < opos; sift++ )
	OKFDD_Levelexchange( sift );
    }
    quantumkosten( von, bis );

    if( debug_on )
    {
      printf( "Kosten nach Durchlauf %lu\n", OKFDD_Line_cost );
    }

    // suche nächstes Level für Tausch
    // -------------------------------
    switch( art )
    {
      case 'l':  // suche max. Level
                 // ----------------
        opos = var + 1;

        for( sift = var + 2; sift <= bis; sift++ )
          if( ( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] - old_no_ut[ twin_pi_table[ sift ] ] ) >
            ( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ opos ] ] - old_no_ut[ twin_pi_table[ opos ] ] ) )
            opos = sift;

        if( opos != var + 1 )
        {
          sift		               = twin_pi_table[ opos ];
          twin_pi_table[ opos ]    = twin_pi_table[ var + 1 ];
          twin_pi_table[ var + 1 ] = sift;
        }
        break;

      case 'k':  // suche max. Level
                 // ----------------
        opos = var + 1;

        for( sift = var + 2; sift <= bis; sift++ )
          if( ( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] - old_no_ut[ twin_pi_table[ sift ] ] ) >=
            ( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ opos ] ] - old_no_ut[ twin_pi_table[ opos ] ] ) )
          {
            if( ( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] - old_no_ut[ twin_pi_table[ sift ] ] ) ==
              ( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ opos ] ] - old_no_ut[ twin_pi_table[ opos ] ] ) )
            {
              if( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] > OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ opos ] ] )
                opos = sift;
            }
            else
              opos = sift;
          }

        if( opos != var + 1 )
        {
          sift		               = twin_pi_table[ opos ];
          twin_pi_table[ opos ]    = twin_pi_table[ var + 1 ];
          twin_pi_table[ var + 1 ] = sift;
        }
        break;

      case 'v':  // suche vielversprechendes Level
                 // -------------------------------
        levelstat();
        opos = var + 1;

        for( sift = var + 2; sift <= bis; sift++ )
          if( lsf[ twin_pi_table[ sift ] ] > lsf[ twin_pi_table[ opos ] ] )
            opos = sift;

        if( opos != var + 1 )
        {
          sift		               = twin_pi_table[ opos ];
          twin_pi_table[ opos ]    = twin_pi_table[ var + 1 ];
          twin_pi_table[ var + 1 ] = sift;
        }
    }
  }
  debug_on = false;
  debug_gatetypes = false;

  printf( "--------------------------------------------\n" );
  quantumkosten( von, bis );

  if( debug_on || debug_costs )
  {
    printf( "Kosten am Ende %lu\n", OKFDD_Now_cost );
  }

  if( debug_gatetypes )
  {
    printf( "Anzahl low high shared : %u\n", S_lhshared );
    printf( "Anzahl low compl.high shared : %u\n", S_lchshared );
    printf( "Anzahl low high : %u\n", S_lh );
    printf( "Anzahl low compl.high : %u\n", S_lch );
    printf( "Anzahl low = high normal/shared : %u\n", S_idlh );
    printf( "Anzahl low = high = 1 : %u\n", S_id1 );
    printf( "Anzahl low = compl.high shared : %u\n", S_idlchshared );
    printf( "Anzahl low = compl.high : %u\n", S_idlch );
    printf( "Anzahl low 0 : %u\n", S_l0 );
    printf( "Anzahl low 1 : %u\n", S_l1 );
    printf( "Anzahl 0 high : %u\n", S_0h );
    printf( "Anzahl 0 compl.high : %u\n", S_0ch );
    printf( "Anzahl 1 high : %u\n", S_1h );
    printf( "Anzahl 1 compl.high : %u\n", S_1ch );
    printf( "Anzahl 1 0 : %u\n", S_10 );
    printf( "Anzahl 0 1 : %u\n", S_01 );
  }
}
/* ========================================================================= */











/* ========================================================================= */
// Function:    dd_man::permut_quantum		                             //
// Rekursive Routine zur Erzeugung v. Permutationen per Levelvertauschungen  //
/* ========================================================================= */
void dd_man::permut_quantum	( usint t,
				  usint von,
				  usint bis )
{
	int ii;

	if( aa[ t ] == 1 )
	{
	  l--;
	  for( ii = 0; ii <= t - von; ii++ )
	  {
	    if( t < bis )
	      permut_quantum( t + 1, von, bis);

	    if( ii < t - von )
	    {
	      OKFDD_Levelexchange( l + ii );
	      quantumkosten( von, bis );
	    }
	    aa[ t + 1 ] = ( aa[ t + 1 ] + 1 ) % 2;
	  }
	}
	else
	{
	  for( ii = t - von; ii >= 0; ii-- )
	  {
	    if( t < bis )
	      permut_quantum( t + 1, von, bis );

	    if( ii > 0 )
	    {
	      OKFDD_Levelexchange( l + ii - 1 );
	      quantumkosten( von, bis );
	    }
	    aa[ t + 1 ] = ( aa[ t + 1 ] + 1 ) % 2;
	  }
	  l++;
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Permutation_Quantum			     //
// Bestimmung des absoluten Min. einer Fkt. fuer feste DTL !!!	             //
/* ========================================================================= */
void dd_man::OKFDD_Permutation_Quantum( usint von,
					usint bis)
{
	int i , sift, opos;

	min_count = OKFDD_Maxidx = 0;

	while( OKFDD_PI_Order_Table[ OKFDD_Maxidx ] != 0 )
	{
	  min_pi_table[ OKFDD_Maxidx ] = OKFDD_PI_Order_Table[ OKFDD_Maxidx ];
	  mindtl[ OKFDD_PI_Order_Table[ OKFDD_Maxidx ] ] = OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ OKFDD_Maxidx ] ];
	  OKFDD_Maxidx++;
	}
	quantumkosten( von, bis );
	min_cost = OKFDD_Line_cost;
	min_cost_quantum = OKFDD_Now_cost;

	l = von;

	// Laufzeittechnische Einschraenkung
	// ---------------------------------
	if( ( bis - von ) > 1 )
	  permut_quantum( von + 1, von, bis );

	OKFDD_Levelexchange( von );
	quantumkosten( von, bis );

	// optimale Positionen bestimmen und einnehmen
	// -------------------------------------------
	for( i = bis; i >= von; i-- )
	{
	  opos = von;

	  while( min_pi_table[ i ] != OKFDD_PI_Order_Table[ opos ] )
	    opos++;

	  for( sift = opos; sift < i; sift++ )
	    OKFDD_Levelexchange( sift );
	}
	quantumkosten( von, bis );
//printf( "Kosten am Ende -> %lu\n", OKFDD_Line_cost );
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Permutation_Quantum                        //
// Bestimmung des absoluten Min. einer Fkt. fuer alle DTLs (GRAY-Code)!!     //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Permutation_Quantum(     usint von,
					usint bis)
{
	int     i, opos, sift, zeiger, drei_hoch_n=1;
	char*   zeile = new char[OKFDD_Maxidx];
	char*   spalte = new char[OKFDD_Maxidx];

	// Initialisierung
	// ---------------
	min_count = OKFDD_Maxidx = 0;
	while (OKFDD_PI_Order_Table[OKFDD_Maxidx]!=0)
	{
	   spalte[OKFDD_Maxidx]	= 0;
	   min_pi_table[OKFDD_Maxidx] = OKFDD_PI_Order_Table[OKFDD_Maxidx];
	   zeile[OKFDD_Maxidx] = mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx]] =
             OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx]];
	   OKFDD_Maxidx++;
	}
	debug_on = false;
	quantumkosten( von, bis );
	min_cost = OKFDD_Line_cost;
	min_cost_quantum = OKFDD_Now_cost;

	for (i = von; i <= bis; i++) drei_hoch_n*=3;

	// best. der besten Reihenfolge (fuer init. DTL)
	// ---------------------------------------------
	bbr(von,bis);

	// Durchlaufe alle Zerlegungstypen im 'GRAY-Code'
	// ----------------------------------------------
	for (i = 1; i < drei_hoch_n; i++)
	{
	   // Veraenderung des Zerlegungstyps
	   // -------------------------------
	   zeiger = bis;
	   spalte[zeiger]++;
	   while (spalte[zeiger] == 3)
	   {
	      spalte[zeiger] = 0;
	      zeile[zeiger]++;
	      if (zeile[zeiger] == 3) zeile[zeiger] = 0;
	      spalte[--zeiger]++;
	   }

	   for (sift = zeiger; sift < OKFDD_Maxidx-1; sift++)
             OKFDD_Levelexchange(sift);

	   switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	   {
	      case D_Shan : S2N(OKFDD_Maxidx-1); break;
	      case D_negD : N2P(OKFDD_Maxidx-1); break;
	      case D_posD : P2S(OKFDD_Maxidx-1);
	   }

	   for (sift = OKFDD_Maxidx-2; sift >= zeiger; sift--)
             OKFDD_Levelexchange(sift);
		quantumkosten(von,bis);

	   // Best. der besten Reihenfolge
	   // ----------------------------
	   bbr(von,bis);
	}

	// Optimale Position bestimmen und einnehmen
	// -----------------------------------------
	for (i = von; i <= bis; i++)
	{
	   opos = bis; 
           while (min_pi_table[i]!=OKFDD_PI_Order_Table[opos]) opos--;

	   if (mindtl[min_pi_table[i]]==OKFDD_PI_DTL_Table[
             OKFDD_PI_Order_Table[opos]])
	     for (sift = opos-1; sift >= i; sift--)
               OKFDD_Levelexchange(sift);
	   else
	   {
	      for (sift = opos; sift < OKFDD_Maxidx-1; sift++)
                OKFDD_Levelexchange(sift);

	      switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	      {
		 case D_Shan :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_posD)
                                      S2P(OKFDD_Maxidx-1);
				 else S2N(OKFDD_Maxidx-1);
				 break;
		 case D_posD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      P2S(OKFDD_Maxidx-1);
				 else P2N(OKFDD_Maxidx-1);
				 break;
		 case D_negD :   if (mindtl[OKFDD_PI_Order_Table[OKFDD_Maxidx-
                                   1]]==D_Shan)
                                      N2S(OKFDD_Maxidx-1);
				 else N2P(OKFDD_Maxidx-1);
	      }

	      for (sift = OKFDD_Maxidx-2; sift >= i; sift--)
                OKFDD_Levelexchange(sift);
	   }
	}
	quantumkosten(von,bis);

	delete[] zeile;
	delete[] spalte;
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::bbr			                             //
 // Bestimmung (keine Transformation) der besten Reihenfolge fuer geg. DTL   //
/* ========================================================================= */
void dd_man::bbr_quantum(usint von,
		 usint bis)
{
	int sift, old_min_cost;

	// best. der besten Reihenfolge
	// ----------------------------
//	old_min_count  = min_count;
	old_min_cost = min_cost;
	l	       = von;

	for (sift = bis; sift >= von; sift--) aa[sift] = 0;

	permut_quantum( von + 1, von, bis );
	OKFDD_Levelexchange(von);
	if (old_min_cost != min_cost)//old_min_count != min_count || old_min_cost != min_cost)
	{
	   for (sift = 0; sift < OKFDD_Maxidx; sift++)
             mindtl[OKFDD_PI_Order_Table[sift]] =
               OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[sift]];
	}
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_DTL_Sifting_Quantum		                     //
// Sifting mit gleichzeitiger Veraenderung des Zerlegungstyps	             //
/* ========================================================================= */
void dd_man::OKFDD_DTL_Sifting_Quantum( usint von,
                usint bis,
                float faktor,
                char  rel,
                char  art,
		bool  siftType )
{
	int	     sift, var, opos , belegt, frei, abb, old_cost;
	char	     abbflag = 0;
	unsigned int old_no_ut[ pi_limit ];

	// Schiebe belegte unique tables zusammen
	// --------------------------------------
	frei = von;
        while( OKFDD_No_UTNodes_Per_Lvl[ OKFDD_PI_Order_Table[ frei ] ] && frei <= bis )
	  frei++;

	for( belegt = frei + 1; belegt <= bis; belegt++ )
	{
	  if( OKFDD_No_UTNodes_Per_Lvl[ OKFDD_PI_Order_Table[ belegt ] ] )
	  {
	    sift = OKFDD_PI_Order_Table[ frei ];
	    OKFDD_PI_Order_Table[ frei ] = OKFDD_PI_Order_Table[ belegt ];
	    OKFDD_PI_Order_Table[ belegt ] = sift;
	    var = OKFDD_PI_Level_Table[ OKFDD_PI_Order_Table[ frei ] ];
	    OKFDD_PI_Level_Table[ OKFDD_PI_Order_Table[ frei ] ] = OKFDD_PI_Level_Table[ OKFDD_PI_Order_Table[ belegt ] ];
	    OKFDD_PI_Level_Table[ OKFDD_PI_Order_Table[ belegt ] ] = var;
	    frei++;
	  }
	}
	bis = frei - 1;

	// aktualisiere Hilfsfelder
	// ------------------------
	if( faktor < 1.01 )
	  faktor = 1.01;
	min_count = 0;

	for( var = 0; var < OKFDD_Maxidx; var++ )
	{
	  old_no_ut[ OKFDD_PI_Order_Table[ var ] ] = OKFDD_No_UTNodes_Per_Lvl[ OKFDD_PI_Order_Table[ var ] ];
	  twin_pi_table[ var ] = min_pi_table[ var ] = OKFDD_PI_Order_Table[ var ];
	  mindtl[ OKFDD_PI_Order_Table[ var ] ] = OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ var ] ];
	}
	quantumkosten( von, bis );

	siftingType = siftType;
	old_cost = ( siftingType == 0 ? OKFDD_Now_cost : OKFDD_Line_cost );
	min_cost = OKFDD_Line_cost;
	min_cost_quantum = OKFDD_Now_cost;

	// suche ersten Kandidaten
	// -----------------------
	switch (art)
	{
	  case 'r':  // Bestimme zufaellige Levelreihenfolge
		     // ------------------------------------
	    for( var = von; var <= bis; var++ )
	    {
	      sift		    = bis - ( rand() % ( bis - var + 1 ) );
	      opos		    = twin_pi_table[ var ];
	      twin_pi_table[ var ]  = twin_pi_table[ sift ];
	      twin_pi_table[ sift ] = opos;
	    }
	    break;

	  case 'g':  // Sortiere Level nach absteigender Breite
		     // ---------------------------------------
	    for( var = von; var <= bis; var++ )
	    {
	      for( sift = var + 1; sift <= bis; sift++ )
	      {
		if( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ var ] ]
		  < OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] )
		{
		  opos			= twin_pi_table[ var ];
		  twin_pi_table[ var ]  = twin_pi_table[ sift ];
		  twin_pi_table[ sift ] = opos;
		}
	      }
	    }
	    break;

	  case 'l':  // Suche max. Level
		     // ----------------
	    opos = von;
	    for( sift = von + 1; sift <= bis; sift++ )
	    {
	      if( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] 
		> OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ opos ] ] )
		opos = sift;
	    }
	    if( opos != von )
	    {
	      sift		    = twin_pi_table[ opos ];
	      twin_pi_table[ opos ] = twin_pi_table[ von ];
	      twin_pi_table[ von ]  = sift;
	    }
	    break;

	  case 'v':  // Suche vielversprechendes Level
		     // -------------------------------
	    levelstat();
	    opos = von;

	    for( sift = von + 1; sift <= bis; sift++ )
	    {
	      if( lsf[ twin_pi_table[ sift ] ] 
		> lsf[ twin_pi_table[ opos ] ] )
		opos = sift;
	    }
	    if( opos != von )
	    {
	      sift		    = twin_pi_table[ opos ];
	      twin_pi_table[ opos ] = twin_pi_table[ von ];
	      twin_pi_table[ von ]  = sift;
	    }
	}

	for( var = von; var <= bis; var++ )
	{
	  Prozentbalken( var - von + 1, bis - von + 1, 'S' )

	  // Vergroesserungsfaktor relativieren
	  // ----------------------------------
	  if( rel == 'r' )
	    old_cost = ( siftingType == 0 ? OKFDD_Now_cost : OKFDD_Line_cost );

	  // Variable fuer momentanen Zerlegungstyp durch DD schieben
	  // --------------------------------------------------------
	  opos = von;
	  while( OKFDD_PI_Order_Table[ opos ] != twin_pi_table[ var ] )
	    opos++;

	  if( opos > von )
	  {
	    for( sift = opos - 1; sift >= von && ( siftingType == 0 ? OKFDD_Now_cost : OKFDD_Line_cost ) 
	      < old_cost * faktor; sift-- )
	    {
	      OKFDD_Levelexchange( sift );
	      quantumkosten( von, bis );
	    }
	    abb = sift + 1;

	    for( sift = abb; sift < opos; sift++ )
	      OKFDD_Levelexchange( sift );
	  }

	  for( sift = opos; sift < bis && ( siftingType == 0 ? OKFDD_Now_cost : OKFDD_Line_cost ) 
	    < old_cost * faktor; sift++ )
	  {
	    OKFDD_Levelexchange( sift );
	    quantumkosten( von, bis );
	  }
	  abb = sift;

	  if( abb < bis )
	    abbflag = 1;
	  else
	  {
	    // Zerlegungstyp rotieren
	    // ---------------------
	    for( sift = bis; sift < OKFDD_Maxidx - 1 && ( siftingType == 0 ? OKFDD_Now_cost : OKFDD_Line_cost ) 
	      < old_cost * faktor; sift++ )
	      OKFDD_Levelexchange( sift );
	    abb = sift - 1;

	    if( abb != OKFDD_Maxidx - 2 )
	    {
	      for( sift = abb; sift >= bis; sift-- )
		OKFDD_Levelexchange( sift );
	      abb = bis;
	      abbflag = 1;
	    }
	    else
	    {
	      switch( OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] )
	      {
		case D_Shan : S2P( OKFDD_Maxidx - 1 );
		  break;
		case D_posD : P2N( OKFDD_Maxidx - 1 );
		  break;
		case D_negD : N2S( OKFDD_Maxidx - 1 );
	      }

	      for( sift = abb; sift >= bis && ( siftingType == 0 ? OKFDD_Now_cost : OKFDD_Line_cost ) 
		< old_cost * faktor; sift-- )
		OKFDD_Levelexchange( sift );
	      abb = sift + 1;
	    }

	    if( abb != bis )
	    {
	      abbflag = 1;

	      for( sift = abb; sift < OKFDD_Maxidx - 1; sift++ )
		OKFDD_Levelexchange( sift );

	      switch( OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] )
	      {
		case D_Shan : S2N( OKFDD_Maxidx - 1 );
		  break;
		case D_posD : P2S( OKFDD_Maxidx - 1 );
		  break;
		case D_negD : N2P( OKFDD_Maxidx - 1 );
	      }

	      for( sift = OKFDD_Maxidx - 2; sift >= bis; sift-- )
		OKFDD_Levelexchange( sift );
	      abb = bis;
	    }
	  }

	  if( abbflag )
	  {
	    opos = von;

	    while( min_pi_table[ opos ] != twin_pi_table[ var ] )
	      opos++;

	    for( sift = abb - 1; sift >= opos; sift-- )
              OKFDD_Levelexchange( sift );
	    quantumkosten( 0, OKFDD_Maxidx - 1 );
	  }
	  else
	  {
	    // Variable fuer momentanen Zerlegungstyp durch DD schieben
	    // --------------------------------------------------------
	    quantumkosten( 0, OKFDD_Maxidx - 1 );

	    for( sift = bis - 1; sift >= von && ( siftingType == 0 ? OKFDD_Now_cost : OKFDD_Line_cost ) 
	      < old_cost * faktor; sift-- )
	    {
	      OKFDD_Levelexchange( sift );
	      quantumkosten( von, bis );
	    }
	    abb = sift + 1;

	    for( sift = abb; sift < OKFDD_Maxidx - 1; sift++ )
              OKFDD_Levelexchange( sift );

	    // Zerlegungstyp rotieren
	    // ---------------------
	    switch (OKFDD_PI_DTL_Table[OKFDD_PI_Order_Table[OKFDD_Maxidx-1]])
	    {
	      case D_Shan : S2P( OKFDD_Maxidx - 1 );
		break;
	      case D_posD : P2N( OKFDD_Maxidx - 1 );
		break;
	      case D_negD : N2S( OKFDD_Maxidx - 1 );
	    }

	    for( sift = OKFDD_Maxidx - 2; sift >= bis && ( siftingType == 0 ? OKFDD_Now_cost : OKFDD_Line_cost ) 
	      < old_cost * faktor; sift-- )
              OKFDD_Levelexchange( sift );
	    abb = sift + 1;

	    if( abb != bis )
	    {
	      for( sift = abb; sift < OKFDD_Maxidx - 1; sift++ )
                OKFDD_Levelexchange( sift );

	      switch( OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] )
	      {
		case D_Shan : S2N( OKFDD_Maxidx - 1 );
		  break;
		case D_posD : P2S( OKFDD_Maxidx - 1 );
		  break;
		case D_negD : N2P( OKFDD_Maxidx - 1 );
	      }

	      for( sift = OKFDD_Maxidx - 2; sift >= bis; sift-- )
		OKFDD_Levelexchange( sift );
	      abb = bis;
	    }
	    else
	    {
	    // Variable fuer momentanen Zerlegungstyp durch DD schieben
	    // --------------------------------------------------------
	      quantumkosten( 0, OKFDD_Maxidx - 1 );

	      for( sift = bis - 1; sift >= von && ( siftingType == 0 ? OKFDD_Now_cost : OKFDD_Line_cost ) 
		< old_cost * faktor; sift-- )
	      {
		OKFDD_Levelexchange( sift );
		quantumkosten( von, bis );
	      }
	      abb = sift + 1;
	    }

	    // Falls momentaner Zerlegungstyp der betrachteten Variable
            // der beste ist
	    // --------------------------------------------------------
	    if( mindtl[ OKFDD_PI_Order_Table[ abb ] ] == OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ abb ] ] )
	    {
	      opos = von;
	      while( min_pi_table[ opos ] != twin_pi_table[ var ] )
		opos++;

	      if( abb < opos )
	      {
		for( sift = abb; sift < opos; sift++ )
		  OKFDD_Levelexchange( sift );
	      }
	      else
	      {
		for( sift = abb - 1; sift >= opos; sift-- )
		  OKFDD_Levelexchange( sift );
	      }
	      quantumkosten( 0, OKFDD_Maxidx - 1 );
	    }

	    // Sonst Typ aendern
	    // -----------------
	    else
	    {
	      for( sift = abb; sift < OKFDD_Maxidx - 1; sift++ )
		OKFDD_Levelexchange( sift );

	      if( OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] == D_Shan )
	      {
		if( mindtl[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] == D_posD )
                  S2P( OKFDD_Maxidx - 1 );
		if( mindtl[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] == D_negD )
                  S2N( OKFDD_Maxidx - 1 );
	      }
	      else
	      {
		if( OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] == D_posD )
		{
		  if( mindtl[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] == D_Shan )
		    P2S( OKFDD_Maxidx - 1 );
		  if( mindtl[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] == D_negD )
		    P2N( OKFDD_Maxidx - 1 );
		}
		else
		{
		  if( OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] == D_negD )
		  {
		    if( mindtl[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] == D_Shan )
		      N2S( OKFDD_Maxidx - 1 );
		    if( mindtl[ OKFDD_PI_Order_Table[ OKFDD_Maxidx - 1 ] ] == D_posD )
		      N2P( OKFDD_Maxidx - 1 );
		  }
		}
	      }

	      // Optimale Position bestimmen und einnehmen
	      // -----------------------------------------
	      opos = von;
	      while( min_pi_table[ opos ] != twin_pi_table[ var ] )
		opos++;

	      for( sift = OKFDD_Maxidx - 2; sift >= opos; sift-- )
		OKFDD_Levelexchange( sift );
	      quantumkosten( 0, OKFDD_Maxidx - 1 );
	    }
	  }

	  // Suche nächstes Level für Tausch
	  // -------------------------------
	  abbflag = 0;

	  switch( art )
	  {
	    case 'l': // Suche max. Level
		      // ----------------
	      opos = var + 1;
	      for(sift = var + 2; sift <= bis; sift++ )
	      {
	      if( ( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ sift ] ] - old_no_ut[ twin_pi_table[ sift ] ] ) 
		> ( OKFDD_No_UTNodes_Per_Lvl[ twin_pi_table[ opos ] ] - old_no_ut[ twin_pi_table[ opos ] ] ) )
		opos = sift;
	      }
	      if( opos != var + 1 )
	      {
		sift			 = twin_pi_table[ opos ];
		twin_pi_table[ opos ]	 = twin_pi_table[ var + 1 ];
		twin_pi_table[ var + 1 ] = sift;
	      }
	      break;

	    case 'v': // Suche vielversprechendes Level
		      // -------------------------------
	      levelstat();
	      opos = var + 1;
	      for( sift = var + 2; sift <= bis; sift++ )
	      {
		if( lsf[ twin_pi_table[ sift ] ] > lsf[ twin_pi_table[ opos ] ] )
		  opos = sift;
	      }
	      if( opos != var + 1 )
	      {
		sift			 = twin_pi_table[ opos ];
		twin_pi_table[ opos ]	 = twin_pi_table[ var + 1 ];
		twin_pi_table[ var + 1 ] = sift;
	      }
	  }
/*	  if( var == bis-1 )
	  {
	    int shannon = 0, pos = 0, neg = 0;
	    printf( "\n" );

	    for( int n = 0; n < OKFDD_P_I; n++ )
	    {
	      if( OKFDD_PI_DTL_Table[ n ] == 0 )
	      {
		shannon += 1;
	      }
	      else if( OKFDD_PI_DTL_Table[ n ] == 1 )
	      {
		pos += 1;
	      }
	      else
	      {
		neg += 1;
	      }
	    }
	    printf( "Shannon -> %i; PosD -> %i; NegD -> %i\n", shannon, pos, neg );
	  }*/
	}
	//printf( "Kosten am Ende -> %lu\n", OKFDD_Line_cost );
	cout(ros_a) << OKFDD_Now_size_i << "\n";
}
/* ========================================================================= */


/* ========================================================================= */
// Function:    dd_man::OKFDD_Friedman_Quantum	                             //
// Bestimmung des absoluten Min. einer Fkt. fuer alle DTLs (GRAY-Code)!!!    //
/* ========================================================================= */
void dd_man::OKFDD_Friedman_Quantum(    usint von,
					usint bis )
{
	int     i, j, opos, sift, zeiger, zwei_hoch_n=1;
	ulint   bbggg = 1;
	usint   tma, tmac;
	usint   adr[ pi_limit ];

	// Vorverdichtung
	// --------------
	OKFDD_Siftlight( von, bis, 'v');
	OKFDD_Siftlight( von, bis, 'v');
	OKFDD_Siftlight( von, bis, 'v');

	// Initialisierung
	// ---------------
	min_count = 0;

	for( i = 0; i < OKFDD_Maxidx; i++ )
        {
	  twin_pi_table[ i ] = min_pi_table[ i ] = OKFDD_PI_Order_Table[ i ];
	  mindtl[ OKFDD_PI_Order_Table[ i ] ] 
	    = OKFDD_PI_DTL_Table[ OKFDD_PI_Order_Table[ i ] ];
	}
	quantumkosten( von, bis );
	min_cost = OKFDD_Line_cost;
	min_cost_quantum = OKFDD_Now_cost;

	for( i = von; i <= bis; i++ )
        {
	  adr[ OKFDD_PI_Order_Table[ i ] ] = zwei_hoch_n;
	  zwei_hoch_n *= 2;
	  bbggg += OKFDD_No_UTNodes_Per_Lvl[ OKFDD_PI_Order_Table[ i ] ];
	}
	usint* label     = new usint[ zwei_hoch_n ];
	usint* teilmenge = new usint[ zwei_hoch_n ];
	ulint* breite    = new ulint[ zwei_hoch_n ];

	teilmenge[ 0 ] = 0;
	breite[ 0 ]    = 0;
	label[ 0 ]     = 0;

	// Konstruktion aller Teilmengen
	// -----------------------------
	for( tma = 1; tma < zwei_hoch_n; tma++ )
        {
	  Prozentbalken( tma, zwei_hoch_n - 1, 'f' )

	  // Teilmenge in oberen Bereich schieben
	  // ------------------------------------
	  zeiger = von - 1;
	  tmac   = tma;

	  for( sift = von; sift <= bis && tmac; sift++ )
	  {
	    if( tmac & 1 )
	    {
	      zeiger++;
	      opos = zeiger;

	      while( twin_pi_table[ sift ] != OKFDD_PI_Order_Table[ opos ] )
		opos++;

	      if( opos != zeiger )
	      {
		for( j = opos - 1; j >= zeiger; j-- )
		  OKFDD_Levelexchange( j );
	      }
	    }
	    tmac >>=1;
	  }

	  // Beste Bottom-Variable bestimmen
	  // -------------------------------
	  breite[ tma ] = bbggg;

	  for( sift = zeiger; sift >= von; sift-- )
	  {
	    if( breite[ tma - adr[ OKFDD_PI_Order_Table[ sift ] ] ] < breite[ tma ] )
	    {
	      for( j = sift; j < zeiger; j++ )
		OKFDD_Levelexchange( j );

	      if( breite[ tma ] > breite[ tma - adr[ OKFDD_PI_Order_Table[ zeiger ] ] ] 
		+ OKFDD_No_UTNodes_Per_Lvl[ OKFDD_PI_Order_Table[ zeiger ] ] )
	      {
		teilmenge[ tma ] = tma - adr[ OKFDD_PI_Order_Table[ zeiger ] ];
		breite[ tma ]    = breite[ teilmenge[ tma ] ] 
		  + OKFDD_No_UTNodes_Per_Lvl[ OKFDD_PI_Order_Table[ zeiger ] ];
		label[ tma ]     = OKFDD_PI_Order_Table[ zeiger ];
	      }
	    }
	  }
	}

	// Best. der besten Reihenfolge
	// ----------------------------
	min_pi_table[ bis ] = label[ zwei_hoch_n - 1 ];
	min_size_i	    = breite[ zwei_hoch_n - 1 ];
	tma		    = teilmenge[ zwei_hoch_n - 1 ];

	for( i = bis - 1; i >= von; i-- )
	{
	  min_pi_table[ i ] = label[ tma ];
	  tma		    = teilmenge[ tma ];
	}

	// Optimale Position bestimmen und einnehmen
	// -----------------------------------------
	for( i = von; i <= bis; i++ )
	{
	   opos = bis;

	   while( min_pi_table[ i ] != OKFDD_PI_Order_Table[ opos ] )
	     opos--;

	   for( sift = opos - 1; sift >= i; sift-- )
	     OKFDD_Levelexchange( sift );
	}
	quantumkosten( von, bis );
	min_cost = OKFDD_Now_cost;

	delete[] label;
	delete[] teilmenge;
	delete[] breite;
}
/* ========================================================================= */
