/*****************************************************************************/
//
// 	Header for compilation of
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

#ifndef PUMA_H
#define PUMA_H

#include <fstream>
#include <vector>
#include <string>

//#define puma_graph

/*****************************************************************************/
//
//      >>>    MACRO DEFINITIONS and INCLUDE-FILES    <<<
//
/*****************************************************************************/

#define		uchar	   	unsigned char
#define	 	usint	   	unsigned short
#define	 	ulint	   	unsigned long int

#define	 	name_limit      50
#define	 	ut_max	  	14

#define	 	m_and(x)	(utnode*)((ulint)x & -2)

#include	"tc_time.h"


/*****************************************************************************/
//
//      >>>    GLOBAL VARIABLES     <<<
//
/*****************************************************************************/

enum {  //  >> Synthesis operation codes <<
	C_NIMF		=       2,      // ~(G =>F)     |
	C_NOT	   	=       3,      // ~ F	  |
	C_XOR	   	=       6,      //   F # G      |
	C_AND	   	=       8,      //   F * G      |
	C_OR	    	=       14,     //   F + G      |
	C_NOR	   	=       1,      // ~(F + G)
	C_NAND	  	=       7,      // ~(F * G)     |
	C_EQUI	  	=       9,      //  ~F # G      |
	C_IMPG	  	=       11,     //   F =>G      |
	C_ID	    	=       12,     //   F	  |
	C_IMPF	  	=       13,     //   G =>F      |
	//  >> Zero suppressed BDD operation codes <<
	ZS_Sub1	 	=       20,     //	      |
	ZS_Sub0	 	=       21,     //	      |
	ZS_Change       =       22,     //	      |
	ZS_Union	=       23,     //	      |
	ZS_Intsec       =       24,     //	      |
	ZS_Diff	 	=       25  };  //	      |

enum {  //  >> Decomposition types <<
	D_Shan	  	=       0,      //	      |
	D_posD	  	=       1,      //	      |
	D_negD	  	=       2   };  //	      |

enum {  //  >> Gate types <<
	Gate_lhshared	=	0,	//	      |
	Gate_lchshared	=	1,	//	      |
	Gate_lh		=	2,	//	      |
	Gate_lch	=	3,	//	      |
	Gate_idlhshared	=	4,	//	      |
	Gate_idlchshared=	5,	//	      |
	Gate_idlh	=	6,	//	      |
	Gate_idlch	=	7,	//	      |
	Gate_l0		=	8,	//	      |
	Gate_l1		=	9,	//	      |
	Gate_0h		=	10,	//	      |
	Gate_0ch	=	11,	//	      |
	Gate_1h		=	12,	//	      |
	Gate_1ch	=	13,	//	      |
	Gate_10		=	14,	//	      |
	Gate_01		=	15,	//	      |
	Gate_id1	=	16  };  //	      |

enum {  //  >> Return value of OKFDD_Type function <<
	Type_ZERO       =       0,      //	      |
	Type_ONE	=       1,      //	      |
	Type_S	  	=       2,      //	      |
	Type_P	  	=       4,      //	      |
	Type_Mix_S_P    =       6,      //	      |
	Type_N	  	=       8,      //	      |
	Type_Mix_S_N    =       10,     //	      |
	Type_Mix_P_N    =       12,     //	      |
	Type_Mix_S_P_N  =       14  };  //	      |

enum {  //  >> Operationcodes for OKFDD_GAC_4_DTL <<
	GAC_S_N_P       =       0,      //	      |
	GAC_S_P	 	=       1,      //	      |
	GAC_S_N	 	=       2,      //	      |
	GAC_P_N	 	=       3   };  //	      |

enum {  //  >> Traverse order <<
	T_Preorder      =       0,      //	      |
	T_Inorder       =       1,      //	      |
	T_Postorder     =       2   };  //	      |

enum {  //  >> Boolean value synonyms <<
	FALSE	   	=       0,      //	      |
	OFF	     	=       0,      //	      |
	NEGATIVE	=       0,      //	      |
	TRUE	    	=       1,      //	      |
	ON	      	=       1,      //	      |
	POSITIVE	=       1,      //	      |
	NONE	    	=       2,      //	      |
	DONTCARE	=       2   };  //	      |

enum {  //  >> Primary types (Input, Output, Temporary) <<
	PI	      	=       0,      //	      |
	PO	      	=       1,      //	      |
	TF	      	=       2,      //	      |
	ELSE	    	=       3   };  //	      |

enum {  //  >> Errorcodes in OKFDD_Error <<
	Err_No_Error    =       0,      // No Error
	Err_Error       =       1,      // General error if bit 0 is set
	Err_Unknown_Op  =       3,      // Unknown operation code for synthesis
	Err_NULL_Op     =       5,      // At least 1 operand is NULL
	Err_Namelength  =       7,      // Name too long
	Err_Linelength  =       9,      // Line too long
	Err_Chain       =       11,     // Node is not in chain
	Err_Hashpos     =       13,     // Hashposition is empty
	Err_Prime_ex    =       15,     // Prime already exists
	Err_Primelimit  =       17,     // Primary limit reached
	Err_Evaluation  =       19,     // Evaluation error
	Err_Outputmiss  =       21,     // Output with given name doesn't exist
	Err_Orderfile   =       23,     // Can't read orderfile
	Err_Levelusage  =       25,     // Level usage incorrect
	Err_Benchmark   =       27,     // Can't read benchmark
	Err_Overflow    =       29,     // Node limit reached
	Err_Prime_miss  =       31,     // Primary to remove doesn't exist
	Err_Dumporder   =       33  };  // Can't create orderfile

typedef enum { 	f_estimate,
		f_exact				} F_Mode;
typedef enum { 	f_not_equal_exact    = 1,
		f_not_equal_estimate = 2,
		f_equal_exact	     = 4,
		f_equal_estimate     = 8	} F_Result;
		

/*****************************************************************************/
//
//      >>>    CLASS STRUCTURES     <<<
//
/*****************************************************************************/

/* ========================================================================= */
// Class:       utnode
/* ========================================================================= */
class utnode
{
	friend  class   dd_man;	 	// dd_man has access
	friend  class   rcnode;	 	// rcnode has access

	public:

	void*	   	link;	   	// Association pointer
	ulint	   	idnum;		// ID-number
	utnode*	 	lo_p;	   	// Low-son of node
	utnode*	 	hi_p;	   	// High-son of node
	usint	    	label_i;	// Label of node

	// ....................................................................

	usint	   	ref_c;	  	// Reference counter

	usint		in_count;	// Counts ingoing edges
					// to get the right costs

 	protected:
	utnode*	 	next;	   	// Pointer to next chain element
	uchar		switch_flag;	// Flag for OKFDD_Switch_all routine
	uchar		flag;		// Flag for traversal
	usint		visFlag;	// visited flag for traversal

/**/ // utnode*	 	MSET;	   	// Activate only for usage with MORE
/**/ // utnode*	 	MNOM;	   	// Activate only for usage with MORE	
};
/* ========================================================================= */


/* ========================================================================= */
// Class:       names
/* ========================================================================= */
class names
{
	friend  class   dd_man;	 	// dd_man has access

	protected:

	names*	  	next;	   	// Pointer to next chain elm.

	public:

	void*		link;		// user-field	
	utnode*	 	root;	   	// PO only: root of node
	usint	   	label_i;	// Numerical identifier of variable
	uchar	   	type;	   	// PI-, PO- or TF-type variable
	char	      	name[name_limit];  // String identifier of variable
	usint		related_var;	// IBM-Adding
	int		group;		// IBM-Adding
	
	// ....................................................................

	public:			 	// Functions for public use

	names	   	(uchar,usint);  // Construct node
};
/* ========================================================================= */


/* ========================================================================= */
// Class:       son_ptr
/* ========================================================================= */
class son_ptr
{
	friend  class   dd_man;	 	// dd_man has access

	protected:

	son_ptr*	next;	   	// Pointer to next chain ele.
	usint	   	son_label;      // Label of son
	usint	   	sort_label;     // Label in sorted order
					// (needed for Interleaving)

	// ....................................................................

	public:			 	// Functions for public use

	son_ptr	 	(usint,son_ptr*);  // Construct node
};
/* ========================================================================= */


/* ========================================================================= */
// Class:       hnode
/* ========================================================================= */
class hnode
{
	friend  class   dd_man;	 	// dd_man has access

	protected:

	son_ptr*	next;	   	// Pointer to first son_ptr
	ulint	   	weight;	 	// Weigth of gate
					// (needed for Interleaving)
	ulint	   	no_nodes;       // Number of nodes under gate
	usint	   	label_i;	// Numerical identifier of variable
	usint	   	lc;	     	// Line of gate-definition in BLIF
	usint	   	ref_c;	  	// Reference counter of hierarchy node
	usint	   	from;	   	// Used for interleaving process
	usint	   	FANIN;	  	// FANIN of gate
	usint	   	depth;	  	// Depth of gate
	uchar	   	type;	   	// PI-, PO- or TF-type variable
	uchar	   	flag;	   	// Help flag

	// ....................................................................

	public:			 	// Functions for public use

	hnode	   	(usint,uchar);  // Construct node
};
/* ========================================================================= */


/* ========================================================================= */
// Class:       rcnode
/* ========================================================================= */
class rcnode
{
	friend  class   dd_man;	 	// dd_man has access

	protected:

	utnode*	 	root;	   	// Pointer to root node
	rcnode*	 	next;	   	// Pointer to next chain elm.
	ulint	   	idnum;	  	// ID-number of root node
					// (when stored in rc_cache)

	// ....................................................................

	public:			 	// Functions for public use

	rcnode	  	(utnode*);      // Construct node
};
/* ========================================================================= */


/* ========================================================================= */
// Class:       ctnode
/* ========================================================================= */
class ctnode
{
	friend  class   dd_man;	 	// dd_man has access

	protected:

	ulint	   	F_idnum;	// ID-number of 1. operand
	ulint	   	G_idnum;	// ID-number of 2. operand
	ulint	   	R_idnum;	// ID-number of result
	utnode*	 	R;	      	// Pointer to root-node of OKFDD
	ctnode*	 	next;	   	// Pointer to next chain elmement
	usint	   	Code;	   	// Operation code
};
/* ========================================================================= */


/*****************************************************************************/
//
//      >>>    TYPEDEFINITIONS     <<<
//
/*****************************************************************************/

class dd_man;

// Travelling type for dd_man member functions
typedef void (dd_man::*travin)(utnode*);

// Travelling type for external functions
typedef void (*travex)(dd_man*,utnode*);

/*****************************************************************************/
//
//      >>>    GLOBAL FUNCTIONS     <<<
//
/*****************************************************************************/

dd_man* OKFDD_Init(     uchar   ut_hashsize_init	= 0,
				// Unique Tables initial ut_sizes-index

			ulint   ct_hashsize_init	= 5003,
				// Computed Table size

			ulint   rc_cachesize_init       = 1000,
				// Recycling Cache size

			uchar   ct_searchlen_init       = 3,
				// Computed Table searchlength

			usint   pi_limit_init	   	= 5000,
				// Limit of primaries

			ulint   ML_nodes_init	   	= 1000000000,
				// Memory limit for Unique Table nodes

			ulint   ML_MB_init	      	= 10000     );
				// Memory limit in MegaBytes

// Standard procedure after gate construction
void    INTERHAND(      dd_man*,
			utnode*     );

/*****************************************************************************/
//
//      >>>    MANAGER CLASS    <<<
//
/*****************************************************************************/

/* ========================================================================= */
// Class:       dd_man
/* ========================================================================= */
class dd_man
{
	public:

        char DDCV_background;
	char DDCV_lowedge;
	char DDCV_highedge;
	char DDCV_nbackground;
	char DDCV_nbackpdav;
	char DDCV_nbackndav;
	char DDCV_coderback;
	char DDCV_nodeframe;
	char DDCV_mark;
	char DDCV_terminal;
	char DDCV_text;
        char DDCV_topmark;
	char DDCV_topedge;
	
	usint S_lhshared;
	usint S_lchshared;
	usint S_lh;
	usint S_lch;
	usint S_idlh;
	usint S_id1;
	usint S_idlchshared;
	usint S_idlch;
	usint S_l0;
	usint S_l1;
	usint S_0h;
	usint S_0ch;
	usint S_1h;
	usint S_1ch;
	usint S_10;
	usint S_01;

	bool debug_on;
	bool debug_gatetypes;
	bool debug_costs;
	bool checkreset;
	bool testflag;

	static const usint line_costs [3][17];
	static const usint quantum_costs [3][17];

	names**	 	toprime;
			// Direct access table to primes

	usint		OKFDD_New_Table_Size;
			// Global parameter for routine OKFDD_Resize_ut
	uchar		OKFDD_CT;
	
	stop_watch      timer;
			// Global timer
	long	    	timer_stop;
			// Global timer value in msecs

	uchar	   	OKFDD_Prof_Out_Limit;
			// Gives breadth of profile output

	uchar	   	OKFDD_Dots_Out_Limit;
			// Gives breadth of percentage-meter

	uchar	   	OKFDD_Version_Wait;
			// Waits for keypress after version_output

	uchar	   	OKFDD_DTL_Default;
			// Sets DTL-default value for construction

	uchar	   	OKFDD_Blocksize;
			// Blocksize of Two-Lvl-Fmt-OKFDD-Builder

	uchar	   	OKFDD_Subst_Method;
			// Turns on substitution-based
			// Multi-Lvl-Fmt-OKFDD-Builder

	uchar	   	OKFDD_Explodefactor;
			// Explodefactor of unique tables

	uchar	   	OKFDD_Implodefactor;
			// Implodefactor of unique tables

	uchar	   	OKFDD_Interleaving;
			// Turns on interleaving

	short	   	OKFDD_Temproutine;
			// Chooses action to perform if tempfactor says so

	float	   	OKFDD_Tempfactor;
			// Sets growing factor to determine next time to ...

	float	   	OKFDD_Siftfactor;
			// Sets growing limit during sift processes

	ulint	   	OKFDD_Siftbase;
			// Minimal size of OKFDDs to init a temproutine

	ulint	   	OKFDD_Outputflags;
			// Bitselects outputs

	float	   	OKFDD_Hiend;
			// Depth for Pseudo-POS in Interleaving

	ulint	   	OKFDD_Mulend;
			// Multiplier for weight-calculations

	ulint	   	OKFDD_Bitend;
			// Formula-selector for Interleaving

	uchar	   	OKFDD_PIs_Only;
			// Flag for interleaving VL-list

	usint	   	OKFDD_Min_FANIN;
			// Minimal FANIN for Pseudo-POs

	ulint	   	OKFDD_Joker;
			// Joker field
			// Coding table will follow

	travex	  	OKFDD_Shell_Extension;
			// External function for OKFDD_Shell


	/* public variables for READ_ONLY */

	usint*	  	OKFDD_PI_Order_Table;
			// Holds actual ordering of primary inputs

	usint*	  	OKFDD_Result;
			// Result table

	long int*       OKFDD_Result_2;
			// Result table

	ulint	  	OKFDD_No_UTNodes;
			// Global number of unqiue table nodes
			// (sum of all tables)

	ulint*	  	OKFDD_No_UTNodes_Per_Lvl;
			// Counter for number of unique table nodes
			// for each level

	ulint	   	OKFDD_No_CTNodes;
			// Counter for number of CT nodes

	usint	   	OKFDD_P_I;
			// Actual number of primary inputs

	usint	   	OKFDD_P_O;
			// Actual number of primary outputs

	uchar*	  	OKFDD_PI_DTL_Table;
			// Decomposition type table

	short*	  	OKFDD_PI_Level_Table;
			// Relative Level Table

	utnode*	 	OKFDD_ONE;
			// Terminal node one

	utnode*	 	OKFDD_ZERO;
			// Terminal node zero

	utnode*	 	ZSBDD_Empty;
			// Combination set {}

	utnode*	 	ZSBDD_Base;
			// Combination set {0}

	utnode*	 	OKFDD_Root[3];
			// Pointer for some procedures with return type utnode*

	ulint	      	OKFDD_Error;
			// Global errorstatus of some functions

	usint	   	OKFDD_Hierarchy_Depth;
			// Maximal Depth of hierarchy tree

	ulint	   	OKFDD_Act_Gate_Number;
			// Currently working with this gate

	ulint	   	OKFDD_Max_Gate_Number;
			// Maximal number of gates in hierarchy tree

	usint	   	OKFDD_Max_Gate_FANIN;
			// Maximal FANIN of all gates in hierarchy tree

	ulint	   	OKFDD_Now_size_i;
			// actual size of OKFDDs

	ulint		OKFDD_Now_cost;
			// actual cost of OKFDDs regarding the quantum costs

	ulint		OKFDD_Line_cost;
			// actual cost of KFDDs regarding the number of lines

	ulint	   	OKFDD_Total_nodes;
			// Needed for size comparison in Siftroutine

	ulint	   	OKFDD_Maxidx;
			// Number of variables in OKFDD_PI_Level_Table

	uchar	   	Size_of_utnodes;
			// sizeof(utnode)

	uchar	   	Size_of_ctnodes;
			// sizeof(ctnode)

	ulint	   	OKFDD_Supported_Vars;
			// # of supported vars
			// (return value of support procedures)

	usint	   	OKFDD_Next_Label;
			// Holds next label to use for new primary

/**/    ulint	   	ctentry;
			// Number of entries made in ct

/**/    ulint	   	ctlookup;
			// Number of lookups performed on ct

/**/    ulint	   	ctlookuphit;
			// Number of matches in lookups

/**/    ulint	   	ctlookuphitno;
			// Number of comparisons to get matches

	//.....................................................................

	protected:

	utnode*	 	OKFDD_DUMMY;
			// Dummy node

	uchar	   	OKFDD_Use_Order;
			// Flag to determine usage of given orderfile

	uchar	   	ut_hashsize_d;
			// Default unique table hashsize index

	uchar	   	ct_searchlen;
			// Computed Table searchlength

	ulint	   	counter;
			// ID-number counter

	ulint	   	hashnr;
			// Last hashnumber

	ulint	   	ct_hashsize;
			// Computed Table size

	ulint	   	rc_cachesize;
			// Recycling Cache size

	uchar*	  	ut_hashsize;
			// Unique Table size

	uchar*	  	ctl;
			// Counts lengths of computed table chains

	utnode***     	ut;
			// Unique tables

	ctnode**	ct;
			// Computed table

	names**	 	prime;
			// Hashtable for PIs, POs, TFs

	usint*	 	All_Prime;
			// Holds all primaries

	usint*	  	Act_Prime;
			// Hold actual primaries

	usint*	  	T_Prime;
			// Holds all temporaray functions

	usint	  	maxi_i;
			// Actual number of primary inputs

	usint	   	maxo_i;
			// Actual number of primary outputs

	usint	   	maxt_i;
			// Actual number of temporary functions

	rcnode*	 	rc_cache_last;
			// Pointer to last rcnode

	utnode*	 	uc_last;
			// Next element in uc_cache

	ctnode*	 	cc_last;
			// Next element in cc_cache

	usint	   	T_F;
			// Actual number of temporary functions

	char*	   	FILEX;
			// Pointer on last benchmarkname

	char*	   	ORDER;
			// Pointer on last ordername

	usint	   	pi_limit;
			// Maximal number of primaries

	usint	   	pi_hashsize;
			// Size of prime hashtable

	uchar	   	Overflow;
			// Memory overflow flag

	ulint	   	Overhead;
			// Amount of memory overhead

	ulint	   	ML_MB;
			// Memory limit in MBs

	ulint	   	ML_nodes;
			// Memory limit in utnodes

	ulint	   	memory_loop;
			// Memory limit help counter

	usint*	  	ZSBDD_equals;
			// List to find copies of primary inputs (for ZSBDDs)

	uchar	  	ZSBDD_Init_done;
			// True signals copies of primary inputs made

	uchar	   	flag;
			// Help flag

	uchar	   	flagx;
			// Help flag during traversal

	usint	   	position;
			// Holds position in string line read from BLIF

	short	   	first_minus;
			// Negative counter for next level table

	char**	  	field;
			// Help field in Multi-Level-Construction

	uchar	  	last;
			// TRUE if word was last in given string

	hnode**	 	gl_list;
			// direct access to hierarchy nodes

	char	    	r;
			// help flag

	int	     	l;
			// help counter

	ulint	   	min_size_i;
			// minimal size of OKFDDs

	bool		siftingType;
			//current siftingtype (determines the sifting lines / quantum)

	ulint		min_cost;
			// minimal cost of OKFDDs

	ulint		min_cost_quantum;
			// minimal quantumcost of OKFDDs

	ulint	   	min_count;
			// minimal size counter

	uchar*	  	mindtl;
			// help field for storage of DTLs

	short*	  	aa;
			// help field

	usint*	  	min_pi_table;
			// OKFDD_PI_Level_Table for minimal setting

	usint*	  	twin_pi_table;
			// OKFDD_PI_Level_Table copy

	short	   	level;
			// help counter

	usint	   	u_level;
			// help counter

	char	    	dtl[3];
			// holds S, P and N

	char	    	outchar[3];
			// holds 0, 1 and -

	uchar	   	erg;
			// SAT help field

	usint*	  	pi_level;
			// help table

	uchar*	  	pfads;
			// SAT help table

	uchar*	  	pfada;
			// SAT help table

	uchar*	  	lsf;
			// help table

	ulint	  	ut_sizes[ut_max + 1];
			// Unique Tables hashsizes
			// { 3, 7, 17, 31, 61, 127, 257, 509, 1021,
			// 2053, 4099,8191, 16381, 32771, 65521 };

	// ....................................................................


	public:	 // Functions for public use

	// --------------------------------------------------------------------
	// Construction of OKFDD-Manager
	// --------------------------------------------------------------------
	dd_man	  	(     	uchar    UT_Hashsize_Startindex,
		   		ulint    CT_Hashsize,
				ulint    Recycling_Cachesize,
				uchar    CT_Searchlength,
				usint    Primary_Limit,
				ulint	 MemoryBoundary_in_Nodes,
				ulint	 MemoryBoundary_in_MByes 	);

	// --------------------------------------------------------------------
	// Outputs OKFDD_Version of sourcecode
	// --------------------------------------------------------------------
	void
	OKFDD_Version   (					       );

	// --------------------------------------------------------------------
	// Synthesis operations launcher
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Synthesis (       usint  	 Operationcode,
				utnode*	 Operand_1,
				utnode*	 Operand_2 = (utnode*)1  	);

	// --------------------------------------------------------------------
	// IF-THEN-ELSE: Returns pointer to root of (F * G) + (~F * H)
	// --------------------------------------------------------------------
	utnode*
	OKFDD_ITE       (       utnode*	 F_IF,
				utnode*	 G_THEN,
				utnode*	 H_ELSE		  		);

	// --------------------------------------------------------------------
	// Returns unique ID-number of given node
	// --------------------------------------------------------------------
	ulint
	OKFDD_ID	(       utnode*	 Node		    )
			{ return (m_and(Node))->idnum; }

	// --------------------------------------------------------------------
	// Returns label of given node
	// --------------------------------------------------------------------
	usint
	OKFDD_Label     (       utnode*	 Node		    )
			{ return (m_and(Node))->label_i; }

	// --------------------------------------------------------------------
	// Calculates amount of used nodes in given OKFDD
	// --------------------------------------------------------------------
	ulint
	OKFDD_Size      (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Calculates amount of used nodes in given OKFDDs
	// --------------------------------------------------------------------
	ulint
	OKFDD_Size_mult (       utnode** Root_Table_NT	   		);

	// --------------------------------------------------------------------
	// Calculates amount of used nodes in all existing OKFDDs
	// --------------------------------------------------------------------
	ulint
	OKFDD_Size_all  (					       	);

	// --------------------------------------------------------------------
	// Calculates amount of dead nodes in manager
	// --------------------------------------------------------------------

	ulint
	OKFDD_No_Dead_UTNodes(					  )
			{ return OKFDD_No_UTNodes - OKFDD_Size_all(); }

	// --------------------------------------------------------------------
	// Gives list of all needed variables (l.o.a.n.v) in given OKFDD
	// --------------------------------------------------------------------
	void
	OKFDD_Support   (       utnode*	 Root,
				uchar	 Show_Results	    		);

	// --------------------------------------------------------------------
	// Gives l.o.a.n.v and their DTL-types in given OKFDD
	// --------------------------------------------------------------------
	void
	OKFDD_DTL       (       utnode*	 Root,
				uchar	 Show_Results	    		);

	// --------------------------------------------------------------------
	// Gives l.o.a.n.v and their amount in given OKFDD
	// --------------------------------------------------------------------
	void
	OKFDD_Profile   (       utnode*	 Root,
				uchar	 Show_Results	    		);

	// --------------------------------------------------------------------
	// Gives list of all needed variables in given OKFDDs
	// --------------------------------------------------------------------
	void
	OKFDD_Support_mult(     utnode** Root_Table_NT,
				uchar	 Show_Results	    		);

	// --------------------------------------------------------------------
	// Gives l.o.a.n.v and their DTL-types in given OKFDDs
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_mult  (       utnode** Root_Table_NT,
				uchar	 Show_Results	    		);

	// --------------------------------------------------------------------
	// Gives l.o.a.n.v and their amount in given OKFDDs
	// --------------------------------------------------------------------
	void
	OKFDD_Profile_mult(     utnode** Root_Table_NT,
				uchar	 Show_Results	    		);

	// --------------------------------------------------------------------
	// Gives list of all needed variables in all existing OKFDDs
	// --------------------------------------------------------------------
	void
	OKFDD_Support_all(      uchar	 Show_Results	    		);

	// --------------------------------------------------------------------
	// Gives l.o.a.n.v and their DTL-type in all existing OKFDDs
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_all   (       uchar	 Show_Results	    		);

	// --------------------------------------------------------------------
	// Gives l.o.a.n.v and their amount in all existing OKFDDs
	// --------------------------------------------------------------------
	void
	OKFDD_Profile_all(      uchar	 Show_Results	    		);

	// --------------------------------------------------------------------
	// Checks identity of two nodes and returns TRUE or FALSE
	// --------------------------------------------------------------------
	uchar
	OKFDD_Identity  (       utnode*	 Operand_1,
				utnode*	 Operand_2	       		);

	// --------------------------------------------------------------------
	// Returns type of given OKFDD
	// --------------------------------------------------------------------
	uchar
	OKFDD_Type      (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Store root in recycling cache
	// --------------------------------------------------------------------
	void
	OKFDD_Free_Node (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Free all OKFDDs in recycling cache
	// --------------------------------------------------------------------
	void
	OKFDD_Free_Fct_Cache(					   	);

	// --------------------------------------------------------------------
	// Returns Root-OKFDD(Label = 0), Root-OKFDD(Label = 1)
	// in field OKFDD_Root[0] and OKFDD_Root[1]
	// --------------------------------------------------------------------
	void
	OKFDD_Cofactor  (       utnode*	 Root,
				usint	 Label		   		);

	// --------------------------------------------------------------------
	// Returns pointer on Root-OKFDD(Label = 0) OR Root-OKFDD(Label = 1)
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Exists    (       utnode*	 Root,
				usint	 Label		   		);

	// --------------------------------------------------------------------
	// Returns pointer on Root-OKFDD(Label = 0) AND Root-OKFDD(Label = 1)
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Forall    (       utnode*	 Root,
				usint	 Label		   		);

	// --------------------------------------------------------------------
	// Returns pointer on Root-OKFDD(Label = 0) XOR Root-OKFDD(Label = 1)
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Depend    (       utnode*	 Root,
				usint	 Label		   		);

	// --------------------------------------------------------------------
	// OKFDD-Traversal (DFS) through Root-OKFDD with function call
	// at each node
	// --------------------------------------------------------------------
	void
	OKFDD_Traverse  (       utnode*	 Root,
				uchar	 Order,
				travin	 Member_Function,
				travex	 External_Function       	);

	// --------------------------------------------------------------------
	// OKFDD-Traversal (DFS) through set of OKFDDs with function call
	// at each node
	// --------------------------------------------------------------------
	void
	OKFDD_Traverse_mult(    utnode** Root_Table_NT,
				uchar	 Order,
				travin	 Member_Function,
				travex	 External_Function       	);

	// --------------------------------------------------------------------
	// OKFDD-Traversal (DFS) through all existing OKFDDs with function call
	// at each node
	// --------------------------------------------------------------------
	void
	OKFDD_Traverse_all(     uchar	 Order,
				travin	 Member_Function,
				travex	 External_Function       	);

	// --------------------------------------------------------------------
	// Checks data structure for reference counters
	// (Returns number of errors)
	// --------------------------------------------------------------------
	ulint
	OKFDD_Check_Refs(       uchar	 Show_Output	     		);

	// --------------------------------------------------------------------
	// Shows each node of traversed OKFDDs once
	// --------------------------------------------------------------------
	void
	Trav_Show       (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Resize unique table of given label to size given in global counter K0
	// (Set this before call)
	// --------------------------------------------------------------------
	void
	OKFDD_Resize_ut (       usint	 Label		   		);


	// --------------------------------------------------------------------
	// Evaluates OKFDD for given labels
	// (all given labels are assumed to be set to ONE)
	// --------------------------------------------------------------------
	uchar
	OKFDD_Evaluate  (       utnode*	 Root,
				usint*	 Label_Table_NT	  		);

	// --------------------------------------------------------------------
	// Returns labels and settings to reach terminal node ONE
	// --------------------------------------------------------------------
	void
	OKFDD_Satisfiability(   utnode*	 Root,
				usint*	 Label_Table_NT,
				uchar*	 Setting_Table_NT		);

	// --------------------------------------------------------------------
	// Returns root of output with given name
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Get_Root  (       char*	 PO_Name		 	);

	// --------------------------------------------------------------------
	// Makes primary output of given root
	// --------------------------------------------------------------------
	usint
	OKFDD_Store_Root(       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Header for OKFDD-Builder-Routines / Reads given order too
	// (External_function is called after each gate construction)
	// --------------------------------------------------------------------
  // msoeken (adding labels)
	void
	OKFDD_Read_BLIF (       char*	 Filename          = NULL,
				char*	 Ordername         = NULL,
				travex	 External_Function = NULL,
			        usint*   Label_Table       = NULL,
                          uchar*   DTL_Table         = NULL,
                          std::string *input_labels = 0, std::string *output_lables = 0 );

	// --------------------------------------------------------------------
	// Returns root of pos. or neg. label with given name
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Get_Variable(     char*	 Primary_Name,
				uchar	 Positive_Negative       	);

	// --------------------------------------------------------------------
	// Returns number of elements in combination set of given ZSBDD
	// --------------------------------------------------------------------
	ulint
	ZSBDD_1_PC      (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Returns number of elements in combination sets of given ZSBDDs
	// --------------------------------------------------------------------
	ulint
	ZSBDD_1_PC_mult (       utnode** Root_Table_NT,
				uchar	 Unshared_Shared	 	);

	// --------------------------------------------------------------------
	// ZSBDD operations launcher for Two-operand-operations
	// --------------------------------------------------------------------
	utnode*
	ZSBDD_Multi_Op  (       usint	 Operationcode,
				utnode*	 Operand_1,
				utnode*	 Operand_2	       		);

	// --------------------------------------------------------------------
	// ZSBDD operations launcher for Single-operand-operations
	// --------------------------------------------------------------------
	utnode*
	ZSBDD_Single_Op (       usint	 Operationscode,
				usint	 Label,
				utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Completely eliminate PO with given label
	// --------------------------------------------------------------------
	void
	OKFDD_Remove_Prime(     usint	 Label		   		);

	// --------------------------------------------------------------------
	// Return PI number x with [0 ... x ... OKFDD_P_I - 1]
	// --------------------------------------------------------------------
	usint
	OKFDD_PI_Table  (       usint	   Index		   )
			{ return All_Prime[Index]; }

	// --------------------------------------------------------------------
	// Return PO number x with [0 ... x ... OKFDD_P_O - 1]
	// --------------------------------------------------------------------
	usint
	OKFDD_PO_Table  (       usint	   Index		   )
			{ return All_Prime[pi_limit-1-Index]; }

	// --------------------------------------------------------------------
	// Return root of PO with given label
	// --------------------------------------------------------------------
	utnode*
	OKFDD_PO_Root_Table(    usint	   Label		   )
			{ return toprime[Label]->root; }

	// --------------------------------------------------------------------
	// Return names-pointer of PO with given label
	// --------------------------------------------------------------------
	names*
	OKFDD_PO_toprime(       usint	   Label		   )
			{ return toprime[Label]; }

	// --------------------------------------------------------------------
	// Initializes actual settings to minimum fields
	// --------------------------------------------------------------------
	void
	OKFDD_Set_act_to_min(					   	);

	// --------------------------------------------------------------------
	// Returns number of 1-paths in given OKFDD
	// --------------------------------------------------------------------
	ulint
	OKFDD_1_PC      (       utnode*	 Root,
				char*	 Dumpfilename = NULL     	);

	// --------------------------------------------------------------------
	// Returns number of 1-paths in given OKFDDs
	// --------------------------------------------------------------------
	ulint
	OKFDD_1_PC_mult (       utnode** Root_Table_NT,
				uchar	 Unshared_Shared,
				char*	 Dumpfilename = NULL     	);

	// --------------------------------------------------------------------
	// Returns number of 1-paths in all existing OKFDDs
	// --------------------------------------------------------------------
	ulint
	OKFDD_1_PC_all  (       uchar	 Unshared_Shared,
				char*	 Dumpfilename = NULL     	);

	// --------------------------------------------------------------------
	// Shell for online-studying of function behaviours (Just a toy)
	// --------------------------------------------------------------------
	void
	OKFDD_Shell     (					       	);

	// --------------------------------------------------------------------
	// Sets given ordering and DTL-types
	// --------------------------------------------------------------------
	void
	OKFDD_Setthis   (       usint*	 Label_Table_NT,
				uchar*	 DTL_Table_NT	    		);

	// --------------------------------------------------------------------
	// Shows order and creates orderfile for given name
	// (default is dump.ord)
	// --------------------------------------------------------------------
	void
	OKFDD_Dumporder (       char*	 Dumpfilename	    		);

	// --------------------------------------------------------------------
	// Deallocates all fields and closes manager
	// --------------------------------------------------------------------
	void
	OKFDD_Quit      (					       	);

	// --------------------------------------------------------------------
	// Converts given OKFDD to ZSBDD
	// --------------------------------------------------------------------
	utnode*
	OKFDD_to_ZSBDD  (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Sets size of uc_cache on given percentage
	// --------------------------------------------------------------------
	void
	OKFDD_Cachecut  (       uchar	 Percentage	      		);

	// --------------------------------------------------------------------
	// Substitutes Label in Root_OKFDD with Sub_Root_OKFDD
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Substitution(     utnode*	 Root,
				usint	 Label,
				utnode*	 Sub_Root		  	);

	// --------------------------------------------------------------------
	// Generate all combinations (order and code-dependant DTL-settings)
	// in given Window (For each combination call functions)
	// --------------------------------------------------------------------
	void
	OKFDD_GAC_4_DTL (       usint	 Window_Start,
				usint	 Window_Stop,
				char	 Operationcode,
				travin	 Member_Function,
				travex	 External_Function       	);

	// --------------------------------------------------------------------
	// SAT_All in given OKFDD
	// --------------------------------------------------------------------
	void
	OKFDD_SAT_all   (       utnode*	 Root,
				char	 Method_Count_Show       	);

	// --------------------------------------------------------------------
	// SAT_All in given OKFDD
	// --------------------------------------------------------------------
	void
	OKFDD_SAT_all_n (       usint	 Label,
				char	 Method_Count_Show,
				char*	 Dumpfilename	    		);

	// --------------------------------------------------------------------
	// SAT_Count in given OKFDD
	// --------------------------------------------------------------------
	void
	OKFDD_SAT_Count (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Exchange of Level with Level + 1
	// --------------------------------------------------------------------
	void
	OKFDD_Levelexchange(    usint	 Level		   		);

	// --------------------------------------------------------------------
	// Shifts label in Start_Level to End_Level
	// --------------------------------------------------------------------
	void
	OKFDD_Levelshift(       usint	 Start_Level,
				usint	 End_Level	       		);

	// --------------------------------------------------------------------
	// Exchange Level_1 with Level_2
	// --------------------------------------------------------------------
	void
	OKFDD_Levelswap (       usint	 Level_1,
				usint	 Level_2		 	);

	// --------------------------------------------------------------------
	// Shifts a block of adjacent levels with Blocklength
	// from Start_Level to End_Level
	// --------------------------------------------------------------------
	void
	OKFDD_Blockshift(       usint	 Start_Level,
				usint	 End_Level,
				usint	 Blocklength	     		);

	// --------------------------------------------------------------------
	// Change label in given Level to new DTL_type
	// (using Bottom_change-Algorithm)
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_Chg_Bottom(   usint	 Level,
				uchar	 DTL_type			);

	// --------------------------------------------------------------------
	// Change label in given Level to new DTL_type (using XOR-Algorithm)
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_Chg_XOR(      usint	 Level,
				uchar	 DTL_type			);

	// --------------------------------------------------------------------
	// Conversion of OKFDD to Shannon-DD in given window
	// --------------------------------------------------------------------
	void
	OKFDD_DD_to_BDD (       usint	 Window_Start,
				usint	 Window_Stop,
				char	 Old_Order	       		);

	// --------------------------------------------------------------------
	// Conversion of OKFDD to pos.Davio-DD in given window
	// --------------------------------------------------------------------
	void
	OKFDD_DD_to_FDD (       usint	 Window_Start,
				usint	 Window_Stop,
				char	 Old_Order	       		);

	// --------------------------------------------------------------------
	// Conversion of OKFDD to neg.Davio-DD in given window
	// --------------------------------------------------------------------
	void
	OKFDD_DD_to_NDD (       usint	 Window_Start,
				usint	 Window_Stop,
				char	 Old_Order	       		);

	// --------------------------------------------------------------------
	// Sifting in given window for given subwindow sizes and overlapping
	// --------------------------------------------------------------------
	void
	OKFDD_Winsift   (       usint	 Window_Start,
				usint	 Window_Stop,
				usint	 Window_Size,
				usint	 Window_Overlapping      	);

	// --------------------------------------------------------------------
	// Permutation in given window for given subwindow sizes and overlapping
	// --------------------------------------------------------------------
	void
	OKFDD_Winpermutation(   usint	 Window_Start,
				usint	 Window_Stop,
				usint	 Window_Size,
				usint	 Window_Overlapping      	);

	// --------------------------------------------------------------------
	// Permutation in given window
	// --------------------------------------------------------------------
	void
	OKFDD_Permutation(      usint	 Window_Start,
				usint	 Window_Stop	    	      	);

	// --------------------------------------------------------------------
	// DTL-Permutation in given window
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_Permutation(  usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Sorts OKFDDs to set order [1 ... OKFDD_PI] and given DTL-type
	// --------------------------------------------------------------------
	void
	OKFDD_Sort      (       usint	 Window_Start,
				usint	 Window_Stop,
				uchar	 New_DTL_Type	    		);

	// --------------------------------------------------------------------
	// Improved Sifting
	// --------------------------------------------------------------------
	void
	OKFDD_Sifting   (       usint	 Window_Start,
				usint	 Window_Stop,
				float	 Siftfactor,
				char	 Relative_Absolute,
				char	 Method		  		);

	// --------------------------------------------------------------------
	// Faster variation of sifting
	// (works with strongly monotonuos decisions)
	// --------------------------------------------------------------------
	void
	OKFDD_Siftlight (       usint	 Window_Start,
				usint	 Window_Stop,
				char	 Method		  		);

	// --------------------------------------------------------------------
	// Scrambles order and DTL-types in given window
	// --------------------------------------------------------------------
	void
	OKFDD_scramble  (       usint	 Window_Start,
				usint	 Window_Stop,
				char	 Method		  		);

	// --------------------------------------------------------------------
	// Improved DTL-Sifting
	// --------------------------------------------------------------------
	void
	dtlsift_XOR     (       usint	 Window_Start,
				usint	 Window_Stop,
				float	 Siftfactor,
				char	 Relative_Absolute,
				char	 Method		  		);

	// --------------------------------------------------------------------
	// Improved DTL-Sifting
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_Sifting(      usint	 Window_Start,
				usint	 Window_Stop,
				float	 Siftfactor,
				char	 Relative_Absolute,
				char	 Method		  		);

	// --------------------------------------------------------------------
	// Improved DTL-Sifting for OKFDD-construction (doesn't try neg.Davio)
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_Sifting_PS(   usint	 Window_Start,
				usint	 Window_Stop,
				float	 Siftfactor,
				char	 Relative_Absolute,
				char	 Method		  		);

	// --------------------------------------------------------------------
	// Changes DTL-types for labels in given window levels
	// (Direction 1 means forward: S->P->N->S)
	// --------------------------------------------------------------------
	void
	OKFDD_Rotation  (       usint	 Window_Start,
				usint	 Window_Stop,
				char	 Direction	       		);

	// --------------------------------------------------------------------
	// Rotates levels in given window
	// --------------------------------------------------------------------
	void
	OKFDD_Tour      (       usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Inverts order in given window
	// --------------------------------------------------------------------
	void
	OKFDD_Inversion (       usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Turns dumpflag on / off
	// --------------------------------------------------------------------
	void
	OKFDD_Set_Dumpflag(					     	);

	// --------------------------------------------------------------------
	// Friedman-Minimization (extended version)
	// --------------------------------------------------------------------
	void
	OKFDD_Friedman_X(       usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// DTL-Friedman-Minimization (extended version)
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_Friedman_X(   usint	 Window_Start,
				usint	 Window_Stop	     		);
	void
	OKFDD_DTL_Friedman_PS(     usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Friedman-Minimization
	// --------------------------------------------------------------------
	void
	OKFDD_Friedman  (       usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// DTL-Friedman-Minimization
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_Friedman(     usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Improved Sifting
	// --------------------------------------------------------------------
	void
	OKFDD_Sifting_Quantum	(       usint	 Window_Start,
					usint	 Window_Stop,
					float	 Siftfactor,
					char	 Relative_Absolute,
					char	 Method		  	);

	// --------------------------------------------------------------------
	// Improved DTL-Sifting
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_Sifting_Quantum(      usint	 Window_Start,
					usint	 Window_Stop,
					float	 Siftfactor,
					char	 Relative_Absolute,
					char	 Method,
					bool	 SiftingType		);

	// --------------------------------------------------------------------
	// Permutation in given window
	// --------------------------------------------------------------------
	void
	OKFDD_Permutation_Quantum(      usint	 Window_Start,
				usint	 Window_Stop	    	      	);

	// --------------------------------------------------------------------
	// Permutation in given window
	// --------------------------------------------------------------------
	void
	OKFDD_DTL_Permutation_Quantum(      usint	 Window_Start,
				usint	 Window_Stop	    	      	);

	// --------------------------------------------------------------------
	// Friedman-Minimization
	// --------------------------------------------------------------------
	void
	OKFDD_Friedman_Quantum  (       usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// DTL-Friedman-Minimization
	// --------------------------------------------------------------------
//	void
//	OKFDD_DTL_Friedman_Quantum(     usint	 Window_Start,
//				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Multiple OR
	// (based on levelswaps, connectors and existential quantification)
	// --------------------------------------------------------------------
	void
	MORE	    	(       utnode** Root_Table_NT	   		);

	// --------------------------------------------------------------------
	// Reduces OKFDD after MORE
	// --------------------------------------------------------------------
	utnode*
	Reduction       (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Show some unique table statistics
	// --------------------------------------------------------------------
	void
	OKFDD_UT_Statistic(					     	);

	// ....................................................................

	protected:	// Private functions

	// --------------------------------------------------------------------
	// Show help table for OKFDD_Shell
	// --------------------------------------------------------------------
	void
	helpme	  	(				      		);

	// --------------------------------------------------------------------
	// Internal init routine (sets some sizes)
	// --------------------------------------------------------------------
	void
	init	    	(				      		);

	// --------------------------------------------------------------------
	// Slave of OKFDD_DTL_Permutation
	// --------------------------------------------------------------------
	void
	bbr	     	(       usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Slave of OKFDD_DTL_Permutation
	// --------------------------------------------------------------------
	void
	bbr_quantum    	(       usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Conversion Shannon to pos.Davio in bottom level
	// --------------------------------------------------------------------
	void
	S2P	     	(       usint	 Level		   		);

	// --------------------------------------------------------------------
	// Conversion Shannon to neg.Davio in bottom level
	// --------------------------------------------------------------------
	void
	S2N	     	(       usint	 Level		   		);

	// --------------------------------------------------------------------
	// Conversion pos.Davio to Shannon in bottom level
	// --------------------------------------------------------------------
	void
	P2S	     	(       usint	 Level		   		);

	// --------------------------------------------------------------------
	// Conversion pos.Davio to neg.Davio bottom level
	// --------------------------------------------------------------------
	void
	P2N	     	(       usint	 Level		   		);

	// --------------------------------------------------------------------
	// Conversion neg.Davio to pos.Davio in bottom level
	// --------------------------------------------------------------------
	void
	N2P	     	(       usint	 Level		   		);

	// --------------------------------------------------------------------
	// Conversion neg.Davio to Shannon in bottom level
	// --------------------------------------------------------------------
	void
	N2S	     	(       usint	 Level		   		);

	// --------------------------------------------------------------------
	// Output of Sizes and DTL-Infos
	// --------------------------------------------------------------------
	void
	gesamtausgabe   (					       	);

	// --------------------------------------------------------------------
	// Compares actual costs with minimal ones in given window
	// (and store order result)
	// --------------------------------------------------------------------
	void
	kosten	  	(       usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Compares actual costs with minimal ones in given window
	// (and store order result)
	// --------------------------------------------------------------------
	void
	quantumkosten	  	(       usint	 Window_Start,
					usint	 Window_Stop		);

	// --------------------------------------------------------------------
	// Calculates actual quantumcost
	// --------------------------------------------------------------------
	void
	berechne_quantumkosten( utnode* node );

	// --------------------------------------------------------------------
	// Resets flags of utnodes
	// --------------------------------------------------------------------
	void
	reset_flags( utnode* node );

	// --------------------------------------------------------------------
	// Compares actual costs with minimal ones in given window
	// (and store order and DTL results)
	// --------------------------------------------------------------------
	void
	gesamtkosten    (       usint	 Window_Start,
				usint	 Window_Stop	     		);


	// --------------------------------------------------------------------
	// Compares actual costs with minimal ones in given window
	// (and store order and DTL results)
	// --------------------------------------------------------------------
	void
	gesamtkosten2   (       usint	 Window_Start,
				usint	 Window_Stop	     		);

	// --------------------------------------------------------------------
	// Slave of OKFDD_SAT_Count
	// --------------------------------------------------------------------
	ulint
	bdd_count       (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Cut off nodes in bottom level (needed for OKFDD_Levelexchange
	// --------------------------------------------------------------------
	void
	cut_off_ut      (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Slave of OKFDD_SAT_all
	// --------------------------------------------------------------------
	void
	kwk	     	(       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Perform inverse Type 1 Reduction
	// --------------------------------------------------------------------
	utnode*
	repro	   	(       utnode*	 Father);

	// --------------------------------------------------------------------
	// Reduction of sons after OKFDD_Levelexchange
	// --------------------------------------------------------------------
	void
	redlight	(       utnode*	 Father);
					  
	utnode*
	red_t1		(       utnode*	 Father);
	utnode*
	red_t2		(       utnode*	 Father);
	utnode*
	red_t3		(       utnode*	 Father);

	// --------------------------------------------------------------------
	// Exchange of inner grandsons, call of son-reduction,
	// store root in unique table
	// --------------------------------------------------------------------
	void
	mutation	(       utnode*	 Root		    		);
	void
	mutation_SS	(       utnode*	 Root		    		);
	void
	mutation_SD	(       utnode*	 Root		    		);
	void
	mutation_DS	(       utnode*	 Root		    		);
	void
	mutation_DD	(       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Creates permutations with levelexchanges
	// (Slave of permutation algorithms)
	// --------------------------------------------------------------------
	void
	permut	  	(       usint    Depth,
				usint    Window_Start,
				usint    Window_Stop		     	);

	// --------------------------------------------------------------------
	// Creates permutations with levelexchanges
	// (Slave of permutation algorithms)
	// --------------------------------------------------------------------
	void
	permut_quantum	(	usint	t,
				usint	Window_Start,
				usint	Window_Stop		     	);

	// --------------------------------------------------------------------
	// Determines candidates for reduction cases in levelexchanges
	// --------------------------------------------------------------------
	void
	levelstat       (					       	);

	// --------------------------------------------------------------------
	// Changes DTL-type of given variable to given type
	// (J2 holds label of interest,
	//  J3 it's old DTL-type and
	//  J4 it's new DTL-type)
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Change_DTL(       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Slave of OKFDD_1_PC_mult in Shared case
	// --------------------------------------------------------------------
	void
	Trav_1_PC       (       utnode** Root_Table_NT,
				utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Constructs new primary name in field gl_name
	// (needed for routine Make_Prime)
	// --------------------------------------------------------------------
	void
	get_name	(       uchar	 Primary_Type,
				usint	 Primary_Label_Counter      	);

	// --------------------------------------------------------------------
	// Creates new primary entry in prime-table
	// --------------------------------------------------------------------
	names*
	Make_Prime      (       uchar	 Primary_Type	    		);

	// --------------------------------------------------------------------
	// Searches for given primary in prime-table
	// --------------------------------------------------------------------
	names*
	find_prime      (					       	);

	// --------------------------------------------------------------------
	// Extracts next word from given string to gl_name-field
	// --------------------------------------------------------------------
	void
	get_word	(       char*	 Sourcestring,
				usint	 Max_wordlength	  		);

	// --------------------------------------------------------------------
	// Reads next (complete line) in field line
	// --------------------------------------------------------------------
	void
  get_line	(       std::ifstream &File_handle,
				char*	 Stringbuffer	    		);

	// --------------------------------------------------------------------
	// Slave routine of all traversal algorithms
	// --------------------------------------------------------------------
	void
	Traverse_slave  (       utnode*	 Root,
				uchar	 Order,
				travin	 Member_Function,
				travex	 External_Function       	);

	// --------------------------------------------------------------------
	// Slave routine of all traversal algorithms
	// --------------------------------------------------------------------
	void
	Traverse_slave2 (       utnode*	 Root,
				uchar	 Order,
				travin	 Member_Function,
				travex	 External_Function       	);

	// --------------------------------------------------------------------
	// Resets utnode-flags in traversed OKFDDs
	// --------------------------------------------------------------------
	void
	Trav_Reset      (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Resets utnode-flags in traversed OKFDDs
	// --------------------------------------------------------------------
	void
	Trav_Reset2     (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Counts size of traversed OKFDDs
	// --------------------------------------------------------------------
	void
	Trav_Size       (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Gets infos for profile of traversed OKFDDs
	// --------------------------------------------------------------------
	void
	Trav_Prof       (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Gets infos for support of traversed OKFDDs
	// --------------------------------------------------------------------
	void
	Trav_Supp       (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Makes copies of PI in All_Prime at given index
	// --------------------------------------------------------------------
	void
	ZSBDD_Copy      (       usint	 Index		   		);

	// --------------------------------------------------------------------
	// Conversion slave for OKFDD_to_ZSBDD
	// --------------------------------------------------------------------
	utnode*
	OKFDD_to_ZSBDD_Slave(   utnode*	 Roots_Father,
				utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Builds OKFDDs out of Multi-Lvl-Benchfile
	// (External_function is called after each gate construction)
	// --------------------------------------------------------------------
	void
	R_M_LVL	 	(       travex	 External_Function       	);

	// --------------------------------------------------------------------
	// ZSBDD operations with Single-operand-operations
	// --------------------------------------------------------------------
	utnode*
	ZS_CO_S	 	(       usint	 Operationcode,
				usint	 Label,
				utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// ZSBDD operations with Two-operand-operations
	// --------------------------------------------------------------------
	utnode*
	ZS_CO_M	 	(       usint	 Operationcode,
				utnode*	 Operand_1,
				utnode*	 Operand_2	       		);

	// --------------------------------------------------------------------
	// Builds OKFDDs out of Two-Level-Benchfile
	// --------------------------------------------------------------------
	void
	R_2_LVL	 	(					       	);

	// --------------------------------------------------------------------
	// Slave of OKFDD_Switch_all
	// --------------------------------------------------------------------
	void
	Switch_slave    (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Synthesis operation F # G
	// --------------------------------------------------------------------
	utnode*
	CO_XOR	  	(       utnode*	 Operand_1,
				utnode*	 Operand_2	       		);

	// --------------------------------------------------------------------
	// Synthesis operation F + G
	// --------------------------------------------------------------------
	utnode*
	CO_OR	   	(       utnode*	 Operand_1,
				utnode*	 Operand_2	       		);

	// --------------------------------------------------------------------
	// Evaluation slave of OKFDD_Evaluate
	// --------------------------------------------------------------------
	utnode*
	Evaluate_slave  (       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Interleaves variable start order
	// --------------------------------------------------------------------
	void
	Interleave      (       hnode*	 Hierarchy_Root	  		);

	// --------------------------------------------------------------------
	// Interleave-Slave
	// --------------------------------------------------------------------
	void
	Interleave_slave(       hnode*	 Hierarchy_Node,
				usint	 From_Label	      		);

	// --------------------------------------------------------------------
	// Interleaves variable start order
	// --------------------------------------------------------------------
	void
	Setfroms	(       hnode*	 Hierarchy_Node	  		);


	// --------------------------------------------------------------------
	// Weights hierarchy tree for interleaving
	// --------------------------------------------------------------------
	void
	Weight_watcher  (       hnode*	 Hierarchy_Node	  		);

	// --------------------------------------------------------------------
	// Traverses hierarchy tree to determine order of TFs
	// --------------------------------------------------------------------
	void
	DFS	     	(       hnode*	 Hierarchy_Node	  		);

	// --------------------------------------------------------------------
	// Slave of OKFDD_Free_Node
	// --------------------------------------------------------------------
	void
	OKFDD_Free_Node_Slave(  utnode*	 Root,
				ulint	 Root_idnum	      		);

	// --------------------------------------------------------------------
	// Returns pointer to unique table node (Label,High_son,Low_son)
	// --------------------------------------------------------------------
	utnode*
	FOAUT	   	(       usint	 Label,
				utnode*	 High_son,
				utnode*	 Low_son		 	);

	// --------------------------------------------------------------------
	// Inserts calculations of last synthesis operation in computed table
	// --------------------------------------------------------------------
	void
	ICT	     	(       usint	 Operationcode,
				utnode*	 Operand_1,
				utnode*	 Operand_2,
				utnode*	 Result		  		);

	// --------------------------------------------------------------------
	// Searchs entry for actual synthesis operation in computed table
	// and return it (when possible)
	// --------------------------------------------------------------------
	utnode*
	CTLO	    	(       usint	 Operationcode,
				utnode*	 Operand_1,
				utnode*	 Operand_2	       		);

	// --------------------------------------------------------------------
	// Corrects marks after certain DTL-switches
	// --------------------------------------------------------------------
	void
	OKFDD_Switch_all(					       	);

	// --------------------------------------------------------------------
	// Refresh primary inputs order in OKFDD_PI_Order_Table
	// --------------------------------------------------------------------
	void
	Support_complete(					       	);

	// --------------------------------------------------------------------
	// Changes DTL-type for label in Level
	// (Direction 1 means forward: S->P->N->S)
	// --------------------------------------------------------------------
	void
	OKFDD_Rotate_Max_Lvl(   usint	 Level,
				char	 Direction	       		);

	// --------------------------------------------------------------------
	// Returns number of 1-paths in given OKFDD (Slave of OKFDD_1_PC(_mult))
	// --------------------------------------------------------------------
	ulint
	OKFDD_1_PC_Slave(       utnode*	 Root		    		);

	// --------------------------------------------------------------------
	// Reset MSET, MNOM of traversed OKFDDs (needed for MORE-routine)
	//
	// --------------------------------------------------------------------
/**/ // void
/**/ // Trav_Mark       (       utnode*	 Root		    );

	public:

	// --------------------------------------------------------------------
	// Returns low son of Root
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Get_Lo_Son    ( 	utnode*  Root 				);

	// --------------------------------------------------------------------
	// Returns high son of Root
	// --------------------------------------------------------------------
	utnode*
	OKFDD_Get_Hi_Son    ( 	utnode*  Root 				);

	// --------------------------------------------------------------------
	// Returns number of 1-paths in given OKFDD (Slave of OKFDD_1_PC(_mult))
	// --------------------------------------------------------------------
	void
	OKFDD_Show_CT_Chain ( 	ulint    Anchor 			);

	// --------------------------------------------------------------------
	// Stores pointer on user-information for given variable
	// --------------------------------------------------------------------
	void
	OKFDD_Set_Var_Info  (	usint	 Variable,
			        void*	 Userfield			)
			{ toprime[Variable]->link = Userfield; }

	// --------------------------------------------------------------------
	// Returns pointer of user-information for given variable
	// --------------------------------------------------------------------
	void*
	OKFDD_Get_Var_Info  (	usint	 Variable			)
			{ return toprime[Variable]->link; }


	// --------------------------------------------------------------------
	// Checks for utnode*
	// --------------------------------------------------------------------
	int
	OKFDD_utnode_ptr    (	utnode*	 Root				)
			{ return ((m_and(Root)) == NULL); }

	// --------------------------------------------------------------------
	// Returns hashed ID-Number
	// --------------------------------------------------------------------
	unsigned
	OKFDD_Hash          (	utnode*	 Root,
			        unsigned Max				)
			{ return ((m_and(Root))->idnum / Max); }

#ifdef puma_graph

void XShowDD_mult(utnode** root_table);
void XShowDD_all ();
void XShowDD     (utnode* root);

void eventloop(utnode** root_table);

void Trav_XInspect(utnode* root);
void Trav_XPerform(utnode* root);
#endif

};
/* ========================================================================= */

#endif
