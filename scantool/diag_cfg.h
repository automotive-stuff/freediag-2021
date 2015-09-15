#ifndef DIAG_CFG_H
#define DIAG_CFG_H

/* freediag
 * API for configurable items
 *
 * (c) fenugrec 2015
 * GPLv3
 *
 *
 */

#include <stdint.h>
#include <stdbool.h>

/* This struct describes one configurable param,
 * including description,type, *value, etc.
 *
 * Typically an L0 driver will alloc a linked-list of (struct cfgi) items
 */
struct cfgi {
	const char *descr;		//description; not mallocd
	const char *shortname;		//for CLI use, must be unique in any array of cfgi. Not mallocd.
	int type;		//indicate type of *val
		#define CFGT_U8		1	//uint8_t, *val is a static buf
		#define CFGT_INT	2	//int, *val is a static buf
		#define CFGT_STR	3	//const char *; generic string; re-alloc/manage *val every time
		//#define CFGT_TTY	4	//const char *; special string ?
		//#define CFGT_ENUM	5	//redundant with ((type==CFGT_INT) && (numopts >0) )?
		#define CFGT_BOOL	6
	void *val;		//actual param
	bool dyn_val;	//if *val must be free'd

	int numopts;		//if > 0 : number of predefined string options / enum values. If ==0 : value set directly.
	char **opt;	//description for each predefined option, i.e. numopts==1 means
							//*opt[0]=="option_id 0 descr", etc.
							// given { const char opt0_descr[]="option_id 0 descr"; const char *opt0_table[]={opt0_descr, opt1_descr}; }
							// use { cfg_param->opt = opt0_table; }
	bool dyn_opt;	//if *opt[] must be free'd (recursively)

	void *dval;		//default value;  used for reset()
	bool dyn_dval;	//dval needs to be free'd

	struct cfgi *next;	//single-linked list

	/* do not call these directly */
	void (*refresh)(struct cfgi *_this);	//called by diag_cfg_refresh()
		//  Possible problem with refresh() if numopts>0; and refresh() makes *val invalid / illegal !
	void (*reset)(struct cfgi *_this);	//called by diag_cfg_reset()
};

void diag_cfg_refresh(struct cfgi *cfgp);	//Optional func to refresh opt[] and numopts (for tty, J2534, etc), doesn't change *val
void diag_cfg_reset(struct cfgi *cfgp);	//Optional: func to reset *val to default; doesn't call refresh()

//set config value for a param; caller must use the right func ... not super efficient
int diag_cfg_setstr(struct cfgi *cfgp, const char *str);
int diag_cfg_setbool(struct cfgi *cfgp, bool val);
int diag_cfg_setu8(struct cfgi *cfgp, uint8_t val);
int diag_cfg_setint(struct cfgi *cfgp, int val);

//set config value to one of the predefined options. Ret 0 if ok. Note: optid is 0-based
int diag_cfg_setopt(struct cfgi *cfgp, int optid);

//directly set param value (caller knows correct type and handles mem management, etc) BAD
//void diag_cfg_setraw(struct cfgi *cfgp, void *val);

//get param value: generates new string to be free'd by caller
char * diag_cfg_getstr(struct cfgi *cfgp);

//free contents of *cfgp but not the struct itself
void diag_cfg_clear(struct cfgi *cfgp);

/****** re-usable, typical configurable params ******/
/* after alloc'ing a new struct cfgi, calling these funcs will prepare the struct */
/* Ret 0 if ok. Some members of the struct may need to be filled after calling these. */

//new TTY / serial port config item
int diag_cfgn_tty(struct cfgi *cfgp);

//serial link speed;
int diag_cfgn_bps(struct cfgi *cfgp, int *val, int *def);

//ordinary int param using caller's &val, and *dev as default value for reset().
//Doesn't fill descr and shortname
int diag_cfgn_int(struct cfgi *cfgp, int *val, int *def);

//ordinary u8 param (copy of _int code); use dval as default value for reset(). Don't fill descr and shortname
int diag_cfgn_u8(struct cfgi *cfgp, uint8_t *val, uint8_t *def);

//ordinary string, copies *def for its default value; sets descr and shortname ptrs
int diag_cfgn_str(struct cfgi *cfgp, const char *def, const char *descr, const char *sn);

//ordinary bool (copy of _int code)
int diag_cfgn_bool(struct cfgi *cfgp, bool *val, bool *def);

#endif // DIAG_CFG_H
