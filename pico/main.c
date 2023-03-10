/*
 * ========================================================================
 * Copyright 2006-2008 University of Washington
 * Copyright 2013-2022 Eduardo Chappa
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * ========================================================================
 */

/*
 * Program:	Main stand-alone Pine Composer routines
 *
 *
 * WEEMACS/PICO NOTES:
 *
 * 08 Jan 92 - removed PINE defines to simplify compiling
 *
 * 08 Apr 92 - removed PINE stub calls
 *
 */

#include	"headers.h"
#include	"../c-client/mail.h"
#include	"../c-client/rfc822.h"
#include	"../c-client/utf8.h"
#include	"../pith/osdep/collate.h"
#include	"../pith/charconv/filesys.h"
#include	"../pith/charconv/utf8.h"
#include	"../pith/conf.h"

/*
 * Useful internal prototypes
 */
#ifdef	_WINDOWS
int	pico_file_drop(int, int, char *);
#endif

/*
 * this isn't defined in the library, because it's a pine global
 * which we use for GetKey's timeout
 */
int	timeoutset = 0;


int	 my_timer_period = (300 * 1000);

/*
 * function key mappings
 */
static UCS fkm[12][2] = {
    { F1,  (CTRL|'G')},
    { F2,  (CTRL|'X')},
    { F3,  (CTRL|'O')},
    { F4,  (CTRL|'J')},
    { F5,  (CTRL|'R')},
    { F6,  (CTRL|'W')},
    { F7,  (CTRL|'Y')},
    { F8,  (CTRL|'V')},
    { F9,  (CTRL|'K')},
    { F10, (CTRL|'U')},
    { F11, (CTRL|'C')},
#ifdef	SPELLER
    { F12, (CTRL|'T')}
#else
    { F12, (CTRL|'D')}
#endif
};

char *pico_args(int, char **, int *, int *, int *);
void  pico_args_help(void);
void  pico_vers_help(void);
void  pico_display_args_err(char *, char **, int);
#ifndef _WINDOWS
PCOLORS *pico_set_global_colors(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
void  display_color_codes(void);
#endif /* ! _WINDOWS */

/* TRANSLATORS: An error message about an unknown flag (option)
   on the command line */
char  args_pico_missing_flag[]  = N_("unknown flag \"%c\"");
/* TRANSLATORS: error message about command line */
char  args_pico_missing_arg[]   = N_("missing or empty argument to \"%c\" flag");
char  args_pico_missing_arg_s[] = N_("missing or empty argument to \"%s\" flag");
char  args_pico_missing_num[]   = N_("non numeric argument for \"%c\" flag");
char  args_pico_missing_color[] = N_("missing color for \"%s\" flag");
char  args_pico_missing_charset[] = N_("missing character set for \"%s\" flag");
char  args_pico_input_charset[] = N_("input character set \"%s\" is unsupported");
char  args_pico_output_charset[] = N_("output character set \"%s\" is unsupported");

char *args_pico_args[] = {
/* TRANSLATORS: little help printed out when incorrect arguments
   are given to pico program. */
N_("Possible Starting Arguments for Pico editor:"),
"",
N_("\tArgument\t\tMeaning"),
N_("\t -e \t\tComplete - allow file name completion"),
N_("\t -k \t\tCut - let ^K cut from cursor position to end of line"),
N_("\t -a \t\tShowDot - show dot files in file browser"),
N_("\t -j \t\tGoto - allow 'Goto' command in file browser"),
N_("\t -g \t\tShow - show cursor in file browser"),
N_("\t -m \t\tMouse - turn on mouse support"),
N_("\t -x \t\tNoKeyhelp - suppress keyhelp"),
N_("\t -p \t\tPreserveStartStop - preserve \"start\"(^Q) and \"stop\"(^S) characters"),
N_("\t -q \t\tTermdefWins - termcap or terminfo takes precedence over defaults"),
N_("\t -Q <quotestr> \tSet quote string (eg. \"> \") esp. for composing email"),
N_("\t -d \t\tRebind - let delete key delete current character"),
N_("\t -f \t\tKeys - force use of function keys"),
N_("\t -b \t\tReplace - allow search and replace"),
N_("\t -h \t\tHelp - give this list of options"),
N_("\t -r[#cols] \tFill - set fill column to #cols columns, default=72"),
N_("\t -n[#s] \tMail - notify about new mail every #s seconds, default=180"),
N_("\t -s <speller> \tSpeller - specify alternative speller"),
N_("\t -t \t\tShutdown - enable special shutdown mode"),
N_("\t -o <dir>\tOperation - specify the operating directory"),
N_("\t -z \t\tSuspend - allow use of ^Z suspension"),
N_("\t -w \t\tNoWrap - turn off word wrap"),
N_("\t -W <wordseps> \tSet word separators other than whitespace"),
#ifndef	_WINDOWS
N_("\t -dcs <display_character_set> \tdefault uses LANG or LC_CTYPE from environment"),
N_("\t -kcs <keyboard_character_set> \tdefaults to display_character_set"),
N_("\t -syscs\t\tuse system-supplied translation routines"),
#endif	/* ! _WINDOWS */
#ifdef	_WINDOWS
N_("\t -dict \"dict1,dict2\" a comma separated list of dictionaries, e.g. en_US, de_DE, es_ES, etc."),
N_("\t -cnf color \tforeground color"),
N_("\t -cnb color \tbackground color"),
N_("\t -crf color \treverse foreground color"),
N_("\t -crb color \treverse background color"),
#endif	/* _WINDOWS */
#ifndef _WINDOWS
N_("\t -color_code  \tdisplay number codes for different colors"),
N_("\t -ncolors number \tnumber of colors for screen (8, 16, or 256)"),
N_("\t -ntfc number \tnumber of color of foreground text"),
N_("\t -ntbc number \tnumber of color of the background"),
N_("\t -rtfc number \tnumber of color of reverse text"),
N_("\t -rtbc number \tnumber of color of reverse background"),
N_("\t -tbfc number \tnumber of color of foreground (text) of the title bar"),
N_("\t -tbbc number \tnumber of color of background of the title bar"),
N_("\t -klfc number \tnumber of color of foreground (text) of the key label"),
N_("\t -klbc number \tnumber of color of background of the key label"),
N_("\t -knfc number \tnumber of color of foreground (text) of the key name"),
N_("\t -knbc number \tnumber of color of background of the key name"),
N_("\t -stfc number \tnumber of color of foreground (text) of the status line"),
N_("\t -stbc number \tnumber of color of background of the status line"),
N_("\t -prfc number \tnumber of color of foreground (text) of a prompt"),
N_("\t -prbc number \tnumber of color of background of a prompt"),
N_("\t -q1fc number \tnumber of color of foreground (text) of level one of quoted text"),
N_("\t -q1bc number \tnumber of color of background of level one of quoted text"),
N_("\t -q2fc number \tnumber of color of foreground (text) of level two of quoted text"),
N_("\t -q2bc number \tnumber of color of background of level two of quoted text"),
N_("\t -q3fc number \tnumber of color of foreground (text) of level three of quoted text"),
N_("\t -q3bc number \tnumber of color of background of level three of quoted text"),
N_("\t -sbfc number \tnumber of color of foreground of signature block text"),
N_("\t -sbbc number \tnumber of color of background of signature block text"),
#endif /* !_WINDOWS */
N_("\t +[line#] \tLine - start on line# line, default=1"),
N_("\t -v \t\tView - view file"),
N_("\t -no_setlocale_collate\tdo not do setlocale(LC_COLLATE)"),
N_("\t -version\tPico version number"),
"", 
N_("\t All arguments may be followed by a file name to display."),
"",
NULL
};


/*
 * main standalone pico routine
 */
#ifdef _WINDOWS
int
app_main (int argc, char *argv[])
#else
int
main(int argc, char *argv[])
#endif
{
    UCS      c;
    register int    f;
    register int    n;
    register BUFFER *bp;
    int	     viewflag = FALSE;		/* are we starting in view mode?*/
    int	     starton = 0;		/* where's dot to begin with?	*/
    int      setlocale_collate = 1;
    char     bname[NBUFN];		/* buffer name of file to read	*/
    char    *file_to_edit = NULL;
    char    *display_charmap = NULL, *dc;
    char    *keyboard_charmap = NULL;
    int      use_system = 0;
    char    *err = NULL;

#ifndef _WINDOWS
    utf8_parameters(SET_UCS4WIDTH, (void *) pith_ucs4width);
#else  /* _WINDOWS */
    chosen_dict = -1;			/* do not commit any dictionary when starting */
#endif /* _WINDOWS */

    set_input_timeout(600);
    Pmaster = NULL;     		/* turn OFF composer functionality */
    km_popped = 0;

    /*
     * Read command line flags before initializing, otherwise, we never
     * know to init for f_keys...
     */
    file_to_edit = pico_args(argc, argv, &starton, &viewflag, &setlocale_collate);

    set_collation(setlocale_collate, 1);

#define cpstr(s) strcpy((char *)fs_get(1+strlen(s)), s)

#ifdef	_WINDOWS	
    init_utf8_display(1, NULL);
#else	/* UNIX */


    if(display_character_set)
      display_charmap = cpstr(display_character_set);
#if   HAVE_LANGINFO_H && defined(CODESET)
    else if((dc = nl_langinfo_codeset_wrapper()) != NULL)
      display_charmap = cpstr(dc);
#endif

    if(!display_charmap)
      display_charmap = cpstr("US-ASCII");

    if(keyboard_character_set)
      keyboard_charmap = cpstr(keyboard_character_set);
    else
      keyboard_charmap = cpstr(display_charmap);


    if(use_system_translation){
#if	PREREQ_FOR_SYS_TRANSLATION
	use_system++;
	/* This modifies its arguments */
	if(setup_for_input_output(use_system, &display_charmap, &keyboard_charmap,
				  &input_cs, &err) == -1){
	    fprintf(stderr, "%s\n", err ? err : "trouble with character set");
	    exit(1);
	}
	else if(err){
	    fprintf(stderr, "%s\n", err);
	    fs_give((void **) &err);
	}
#endif
    }

    if(!use_system){
	if(setup_for_input_output(use_system, &display_charmap, &keyboard_charmap,
				  &input_cs, &err) == -1){
	    fprintf(stderr, "%s\n", err ? err : "trouble with character set");
	    exit(1);
	}
	else if(err){
	    fprintf(stderr, "%s\n", err);
	    fs_give((void **) &err);
	}
    }

    if(keyboard_charmap){
	set_locale_charmap(keyboard_charmap);
	free((void *) keyboard_charmap);
    }

    if(display_charmap)
      free((void *) display_charmap);

#endif	/* UNIX */

    /*
     * There are a couple arguments that we need to be sure
     * are converted for internal use.
     */
    if(alt_speller)
      alt_speller = cpstr(fname_to_utf8(alt_speller));

    if(opertree && opertree[0]){
	strncpy(opertree, fname_to_utf8(opertree), sizeof(opertree));
	opertree[sizeof(opertree)-1] = '\0';
    }

    if(glo_quote_str_orig)
      glo_quote_str = utf8_to_ucs4_cpystr(fname_to_utf8(glo_quote_str_orig));

    if(glo_wordseps_orig)
      glo_wordseps = utf8_to_ucs4_cpystr(fname_to_utf8(glo_wordseps_orig));

    if(file_to_edit)
      file_to_edit = cpstr(fname_to_utf8(file_to_edit));

#undef cpstr

#if	defined(DOS) || defined(OS2)
    if(file_to_edit){			/* strip quotes? */
	int   l;

	if(strchr("'\"", file_to_edit[0])
	   && (l = strlen(file_to_edit)) > 1
	   && file_to_edit[l-1] == file_to_edit[0]){
	    file_to_edit[l-1] = '\0';	/* blat trailing quote */
	    file_to_edit++;		/* advance past leading quote */
	}
    }
#endif

    if(!vtinit())			/* Displays.            */
	exit(1);

    strncpy(bname, "main", sizeof(bname));		/* default buffer name */
    bname[sizeof(bname)-1] = '\0';
    edinit(bname);			/* Buffers, windows.   */

    update();				/* let the user know we are here */
    sgarbf = TRUE;			/* next update, repaint all */

#ifdef	_WINDOWS
    mswin_setwindow(NULL, NULL, NULL, NULL, NULL, NULL);
    mswin_showwindow();
    mswin_showcaret(1);			/* turn on for main window */
    mswin_allowpaste(MSWIN_PASTE_FULL);
    mswin_setclosetext("Use the ^X command to exit Pico.");
    mswin_setscrollcallback (pico_scroll_callback);
#endif

#if	defined(USE_TERMCAP) || defined(USE_TERMINFO) || defined(VMS)
    if(kbesc == NULL){			/* will arrow keys work ? */
	(*term.t_putchar)('\007');
	emlwrite("Warning: keypad keys may be non-functional", NULL);
    }
#endif	/* USE_TERMCAP/USE_TERMINFO/VMS */

    if(file_to_edit){			/* Any file to edit? */

	makename(bname, file_to_edit);	/* set up a buffer for this file */

	bp = curbp;			/* read in first file */
	makename(bname, file_to_edit);
	strncpy(bp->b_bname, bname, sizeof(bp->b_bname));
	bp->b_bname[sizeof(bp->b_bname)-1] = '\0';

	if(strlen(file_to_edit) >= NFILEN){
	    char buf[128];

	    snprintf(buf, sizeof(buf), "Filename \"%.10s...\" too long", file_to_edit);
	    emlwrite(buf, NULL);
	    file_to_edit = NULL;
	}
	else{
	    strncpy(bp->b_fname, file_to_edit, sizeof(bp->b_fname));
	    bp->b_fname[sizeof(bp->b_fname)-1] = '\0';
	    if (((gmode&MDTREE) && !in_oper_tree(file_to_edit)) ||
		readin(file_to_edit, (viewflag==FALSE), TRUE) == ABORT) {
		if ((gmode&MDTREE) && !in_oper_tree(file_to_edit)){
		    EML eml;
		    eml.s = opertree;
		    emlwrite(_("Can't read file from outside of %s"), &eml);
		}

		file_to_edit = NULL;
	    }
	}

	if(!file_to_edit){
	    strncpy(bp->b_bname, "main", sizeof(bp->b_bname));
	    bp->b_bname[sizeof(bp->b_bname)-1] = '\0';
	    strncpy(bp->b_fname, "", sizeof(bp->b_fname));
	    bp->b_fname[sizeof(bp->b_fname)-1] = '\0';
	}

	bp->b_dotp = bp->b_linep;
	bp->b_doto = 0;

	if (viewflag)			/* set the view mode */
	  bp->b_mode |= MDVIEW;
    }

    /* setup to process commands */
    lastflag = 0;			/* Fake last flags.     */
    curbp->b_mode |= gmode;		/* and set default modes*/

    curwp->w_flag |= WFMODE;		/* and force an update	*/

    if(timeoutset){
	EML eml;

	eml.s = comatose(get_input_timeout());
	emlwrite(_("Checking for new mail every %s seconds"), &eml);
    }


    forwline(0, starton - 1);		/* move dot to specified line */

    while(1){

	if(Pcolors)
	   pico_set_colorp(Pcolors->ntcp, PSC_NONE);

	if(km_popped){
	    km_popped--;
	    if(km_popped == 0) /* cause bottom three lines to be repainted */
	      curwp->w_flag |= WFHARD;
	}

	if(km_popped){  /* temporarily change to cause menu to be painted */
	    term.t_mrow = 2;
	    curwp->w_ntrows -= 2;
	    curwp->w_flag |= WFMODE;
	    movecursor(term.t_nrow-2, 0); /* clear status line, too */
	    peeol();
	}

	update();			/* Fix up the screen    */
	if(km_popped){
	    term.t_mrow = 0;
	    curwp->w_ntrows += 2;
	}

#ifdef	MOUSE
#ifdef  EX_MOUSE
	/* New mouse function for real mouse text selection. */
	register_mfunc(mouse_in_pico, 2, 0, term.t_nrow - (term.t_mrow + 1),
		       term.t_ncol);
#else
	mouse_in_content(KEY_MOUSE, -1, -1, 0, 0);
	register_mfunc(mouse_in_content, 2, 0, term.t_nrow - (term.t_mrow + 1),
		       term.t_ncol);
#endif
#endif
#ifdef	_WINDOWS
	mswin_setdndcallback (pico_file_drop);
	mswin_mousetrackcallback(pico_cursor);
#endif
	c = GetKey();
#ifdef	MOUSE
#ifdef  EX_MOUSE
	clear_mfunc(mouse_in_pico);
#else
	clear_mfunc(mouse_in_content);
#endif
#endif
#ifdef	_WINDOWS
	mswin_cleardndcallback ();
	mswin_mousetrackcallback(NULL);
#endif

	if(timeoutset && (c == NODATA || time_to_check())){
	    if(pico_new_mail())
	      emlwrite(_("You may possibly have new mail."), NULL);
	}

	if(km_popped)
	  switch(c){
	    case NODATA:
	    case (CTRL|'L'):
	      km_popped++;
	      break;
	    
	    default:
	      /* clear bottom three lines */
	      mlerase();
	      break;
	  }

	if(c == NODATA)
	  continue;

	if(mpresf){			/* erase message line? */
	    if(mpresf++ > MESSDELAY)
	      mlerase();
	}

	f = FALSE;
	n = 1;

#ifdef	MOUSE
	clear_mfunc(mouse_in_content);
#endif
					/* Do it.               */
	execute(normalize_cmd(c, fkm, 1), f, n);
    }
}

#ifndef _WINDOWS
void
display_color_codes(void)
{
#define SPACES "   "
  int i, k, l, a;
  int ncolors;
  COLOR_PAIR *lastc = NULL, *newcp;

  /* this is the format "SPACE COLORED_SPACES = CODE SPACE" */
  vtterminalinfo(gmode & MDTCAPWINS);
  (*term.t_open)();
  (*term.t_rev)(FALSE);

  pico_toggle_color(0);
  if((ncolors = pico_count_in_color_table()) <= 0){
    fprintf(stderr, "%s", "Your screen does not support colors\r\n");
    exit(1);
  }

  if(term.t_ncol < 62 || term.t_nrow < 23){
    fprintf(stderr, "%s", "Screen must have at least 24 rows and 63 columns\r\n");
    exit(1);
  }

  if(ncolors & 1) ncolors--;	/* eliminate transparent colors at this time */

  fprintf(stdout, "%s", "The code of a color is the number in the same row plus\r\n");
  fprintf(stdout, "%s", "the number in the same column as that color.\r\n\r\n");

  lastc = pico_get_cur_color();
  switch(ncolors){
      case   8: pico_set_color_options(COLOR_ANSI8_OPT|COLOR_TRANS_OPT); break;
      case  16: pico_set_color_options(COLOR_ANSI16_OPT|COLOR_TRANS_OPT); break;
      case 256: pico_set_color_options(COLOR_ANSI256_OPT|COLOR_TRANS_OPT); break;
      default : fprintf(stderr, "Unknown number of colors %d\n", ncolors); 
	        exit(1);
	        break;
  }

  pico_toggle_color(1);
  if(ncolors < 256){
      for(k = -1; 16*k < ncolors; k++){
	for(l = -1; l < ncolors; l++){
	   if(k == -1){
		if(l == -1)
		   fprintf(stdout, "%s", "    ");
		else
		   fprintf(stdout, "%3d", l);
	   }
	   else{
		if(l == -1){
		    pico_toggle_color(1);
		    fprintf(stdout, "%3d ", k);
		}
	   	else{
		    if(ncolors || l < 8){
		      i = 16*k + l;
		      newcp = new_color_pair(colorx(0), colorx(i));
		      pico_set_colorp(newcp, PSC_NONE);
		      fprintf(stdout, "%s", SPACES);
		      pico_set_colorp(lastc, PSC_NONE);
		    }
		}
	   }
	}
	pico_toggle_color(0);
	if(k == -1)
	  fprintf(stdout, " (%d colors)", ncolors);
	fprintf(stdout, "%s", "\r\n");
      }
  } else {
      fprintf(stdout, "%s", "Codes for terminal with 256 colors:\r\n");
      a = 16;
      for(k = -1; 36*k < ncolors; k++){
	for(l = -1; l < ncolors && l < 16; l++){
	   if(k == -1){
		if(l == -1)
		    fprintf(stdout, "%s", "    ");
	   	else 
		    fprintf(stdout, "%3d", l);
	   }
	   else{
		if(l == -1){
		    pico_toggle_color(1);
		    fprintf(stdout, "%3d ", 36*k);
		}
	   	else{
		    i = 36*k + l;
		    newcp = new_color_pair(colorx(0), colorx(i));
		    pico_set_colorp(newcp, PSC_NONE);
		    fprintf(stdout, "%s", SPACES);
		    pico_set_colorp(lastc, PSC_NONE);
		}
	   }
	}
	pico_toggle_color(0);
	fprintf(stdout, "%s", "\r\n");
      }
      a = 20;
      for(k = -1; 16 + 36*k < ncolors; k++){
	for(l = -1; l < ncolors && l < a; l++){
	   if(k == -1){
		if(l == -1)
		    fprintf(stdout, "%s", "    ");
	   	else 
		    fprintf(stdout, "%3d", l);
	   }
	   else{
		if(l == -1){
		    pico_toggle_color(1);
		    fprintf(stdout, "%3d ", 16 + 36*k);
		}
	   	else{
		    i = 16 + 36*k + l;
		    newcp = new_color_pair(colorx(0), colorx(i));
		    pico_set_colorp(newcp, PSC_NONE);
		    fprintf(stdout, "%s", SPACES);
		    pico_set_colorp(lastc, PSC_NONE);
		}
	   }
	}
	pico_toggle_color(0);
	fprintf(stdout, "%s", "\r\n");
      }
  }
  pico_set_colorp(lastc, PSC_NONE);
}
#endif /* ! _WINDOWS */


/*
 *  Parse the command line args.
 *
 * Args      ac
 *           av
 *      starton -- place to return starton value
 *     viewflag -- place to return viewflag value
 *
 * Result: command arguments parsed
 *       possible printing of help for command line
 *       various global flags set
 *       returns the name of any file to open, else NULL
 */
char *
pico_args(int ac, char **av, int *starton, int *viewflag, int *setlocale_collate)
{
    int   c, usage = 0;
    char *str;
    char  tmp_1k_buf[1000];     /* tmp buf to contain err msgs  */ 
#ifndef _WINDOWS
    int ncolors, ntfc, ntbc, rtfc, rtbc;
    int tbfc, tbbc, klfc, klbc, knfc, knbc, stfc, stbc, prfc, prbc;
    int q1fc, q1bc, q2fc, q2bc, q3fc, q3bc, sbfc, sbbc;

    ncolors = 0;
    ntfc = ntbc = rtfc = rtbc = tbfc = tbbc = klfc = klbc = knfc = 
    knbc = stfc = stbc = prfc = prbc = q1fc = q1bc = q2fc = q2bc = 
    q3fc = q3bc = sbfc = sbbc = -1;
#endif /* ! _WINDOWS */

Loop:
    /* while more arguments with leading - or + */
    while(--ac > 0 && (**++av == '-' || **av == '+')){
      if(**av == '+'){
	if(*++*av)
	  str = *av;
	else if(--ac)
	  str = *++av;
	else{
	  snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_arg), '+');
	  pico_display_args_err(tmp_1k_buf, NULL, 1);
	  usage++;
	  goto Loop;
	}

	if(!isdigit((unsigned char)str[0])){
	  snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_num), '+');
	  pico_display_args_err(tmp_1k_buf, NULL, 1);
	  usage++;
	}

	if(starton)
	  *starton = atoi(str);

	goto Loop;
      }
      
      /* while more chars in this argument */
      else while(*++*av){

	if(strcmp(*av, "version") == 0){
	    pico_vers_help();
	}
#ifndef	_WINDOWS     /* color configuration disabled in Windows  at this time */
	else if(strcmp(*av, "ntfc") == 0
		|| strcmp(*av, "ntbc") == 0
		|| strcmp(*av, "rtfc") == 0
		|| strcmp(*av, "rtbc") == 0
		|| strcmp(*av, "tbfc") == 0
		|| strcmp(*av, "tbbc") == 0
		|| strcmp(*av, "klfc") == 0
		|| strcmp(*av, "klbc") == 0
		|| strcmp(*av, "knfc") == 0
		|| strcmp(*av, "knbc") == 0
		|| strcmp(*av, "stfc") == 0
		|| strcmp(*av, "stbc") == 0
		|| strcmp(*av, "prfc") == 0
		|| strcmp(*av, "prbc") == 0
		|| strcmp(*av, "q1fc") == 0
		|| strcmp(*av, "q1bc") == 0
		|| strcmp(*av, "q2fc") == 0
		|| strcmp(*av, "q2bc") == 0
		|| strcmp(*av, "q3fc") == 0
		|| strcmp(*av, "q3bc") == 0
		|| strcmp(*av, "sbfc") == 0
		|| strcmp(*av, "sbbc") == 0
		|| strcmp(*av, "ncolors") == 0){
	  str = *av;
	  if(--ac)
	    ++av;
	  else{
	    snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_arg), '-');
	    pico_display_args_err(tmp_1k_buf, NULL, 1);
	    usage++;
	    goto Loop;
	  }

	  if(!isdigit((unsigned char)**av)){
	    snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_num), '-');
	    pico_display_args_err(tmp_1k_buf, NULL, 1);
	    usage++;
	  }
	  if(strcmp(str, "ntfc") == 0) ntfc = atoi(*av);
	  else if (strcmp(str, "ntbc") == 0) ntbc = atoi(*av);
	  else if (strcmp(str, "rtfc") == 0) rtfc = atoi(*av);
	  else if (strcmp(str, "rtbc") == 0) rtbc = atoi(*av);
	  else if (strcmp(str, "tbfc") == 0) tbfc = atoi(*av);
	  else if (strcmp(str, "tbbc") == 0) tbbc = atoi(*av);
	  else if (strcmp(str, "klfc") == 0) klfc = atoi(*av);
	  else if (strcmp(str, "klbc") == 0) klbc = atoi(*av);
	  else if (strcmp(str, "knfc") == 0) knfc = atoi(*av);
	  else if (strcmp(str, "knbc") == 0) knbc = atoi(*av);
	  else if (strcmp(str, "stfc") == 0) stfc = atoi(*av);
	  else if (strcmp(str, "stbc") == 0) stbc = atoi(*av);
	  else if (strcmp(str, "prfc") == 0) prfc = atoi(*av);
	  else if (strcmp(str, "prbc") == 0) prbc = atoi(*av);
	  else if (strcmp(str, "q1fc") == 0) q1fc = atoi(*av);
	  else if (strcmp(str, "q1bc") == 0) q1bc = atoi(*av);
	  else if (strcmp(str, "q2fc") == 0) q2fc = atoi(*av);
	  else if (strcmp(str, "q2bc") == 0) q2bc = atoi(*av);
	  else if (strcmp(str, "q3fc") == 0) q3fc = atoi(*av);
	  else if (strcmp(str, "q3bc") == 0) q3bc = atoi(*av);
	  else if (strcmp(str, "sbfc") == 0) sbfc = atoi(*av);
	  else if (strcmp(str, "sbbc") == 0) sbbc = atoi(*av);
	  else if (strcmp(str, "ncolors") == 0) ncolors = atoi(*av);
	  if(!strcmp(str, "ncolors")){
	     switch(ncolors){
		case   8: pico_set_color_options(COLOR_ANSI8_OPT|COLOR_TRANS_OPT); break;
		case  16: pico_set_color_options(COLOR_ANSI16_OPT|COLOR_TRANS_OPT); break;
		case 256: pico_set_color_options(COLOR_ANSI256_OPT|COLOR_TRANS_OPT); break;
		default : snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _("Unsupported number of colors: %d"), ncolors);
			  pico_display_args_err(tmp_1k_buf, NULL, 1);
			  exit(1);
			  break;
	     }
	  }
	  goto Loop;
	}
	else if(strcmp(*av, "color_code") == 0){
	   display_color_codes();
	   exit(0);
	}
#endif /* ! _WINDOWS */
	else if(strcmp(*av, "no_setlocale_collate") == 0){
	    *setlocale_collate = 0;
	    goto Loop;
	}
#ifndef	_WINDOWS
	else if(strcmp(*av, "syscs") == 0){
	    use_system_translation = !use_system_translation;
	    goto Loop;
	}
#endif	/* ! _WINDOWS */
#ifdef	_WINDOWS
        else if(strcmp(*av, "dict") == 0){
	   char *cmd = *av; /* save it to use below */
	   str = *++av;
	   if(--ac){
	     int i = 0;
	     char *s;
#define MAX_DICTIONARY	10
	     while(str && *str){
	        if(dictionary == NULL){
		   dictionary = fs_get((MAX_DICTIONARY + 1)*sizeof(char *));
		   memset((void *) dictionary, 0, (MAX_DICTIONARY+1)*sizeof(char *));
		   if(dictionary == NULL)
		     goto Loop;		/* get out of here */
		}
		if((s = strpbrk(str, " ,")) != NULL)
		   *s++ = '\0';
		dictionary[i] = fs_get(strlen(str) + 1);
		strcpy(dictionary[i++], str);
		if(s != NULL)
		   for(; *s && (*s == ' ' || *s == ','); s++);
		else
		   goto Loop;
		if(i == MAX_DICTIONARY + 1)
		  goto Loop;
		else
		  str = s;
	     }
	   }
	   else{
		snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_arg_s), cmd);
		pico_display_args_err(tmp_1k_buf, NULL, 1);
	        usage++;
	   }

	   goto Loop;
	}
	else if(strcmp(*av, "cnf") == 0
	   || strcmp(*av, "cnb") == 0
	   || strcmp(*av, "crf") == 0
	   || strcmp(*av, "crb") == 0){

	    char *cmd = *av; /* save it to use below */

	    if(--ac){
		str = *++av;
		if(cmd[1] == 'n'){
		    if(cmd[2] == 'f')
		      pico_nfcolor(str);
		    else if(cmd[2] == 'b')
		      pico_nbcolor(str);
		}
		else if(cmd[1] == 'r'){
		    if(cmd[2] == 'f')
		      pico_rfcolor(str);
		    else if(cmd[2] == 'b')
		      pico_rbcolor(str);
		}
	    }
	    else{
		snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_color), cmd);
		pico_display_args_err(tmp_1k_buf, NULL, 1);
	        usage++;
	    }

	    goto Loop;
	}
#endif	/* _WINDOWS */
#ifndef	_WINDOWS
	else if(strcmp(*av, "dcs") == 0 || strcmp(*av, "kcs") == 0){
	    char *cmd = *av;

	    if(--ac){
		if(strcmp(*av, "dcs") == 0){
		    display_character_set = *++av;
		    if(!output_charset_is_supported(display_character_set)){
			snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_output_charset), display_character_set);
			pico_display_args_err(tmp_1k_buf, NULL, 1);
			usage++;
		    }
		}
		else{
		    keyboard_character_set = *++av;
		    if(!input_charset_is_supported(keyboard_character_set)){
			snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_input_charset), keyboard_character_set);
			pico_display_args_err(tmp_1k_buf, NULL, 1);
			usage++;
		    }
		}
	    }
	    else{
		snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_charset), cmd);
		pico_display_args_err(tmp_1k_buf, NULL, 1);
	        usage++;
	    }

	    goto Loop;
	}
#endif	/* ! _WINDOWS */

	/*
	 * Single char options.
	 */
	switch(c = **av){
	  /*
	   * These don't take arguments.
	   */
          case 'a':
            gmode ^= MDDOTSOK;          /* show dot files */
            break;
	  case 'b':
#ifdef notdef
    /*
     * Make MDREPLACE always on instead
     * Don't leave this to allow turning off of MDREPLACE because the
     * polarity of -b will have changed. Don't think anybody wants to
     * turn it off.
     */
	    gmode ^= MDREPLACE;  /* -b for replace string in where is command */
#endif
	    break;
	  case 'd':			/* -d for rebind delete key */
	    bindtokey(0x7f, forwdel);
	    break;
	  case 'e':			/* file name completion */
	    gmode ^= MDCMPLT;
	    break;
	  case 'f':			/* -f for function key use */
	    gmode ^= MDFKEY;
	    break;
	  case 'g':			/* show-cursor in file browser */
	    gmode ^= MDSHOCUR;
	    break;
	  case 'h':
	    usage++;
	    break;
	  case 'j':			/* allow "Goto" in file browser */
	    gmode ^= MDGOTO;
	    break;
	  case 'k':			/* kill from dot */
	    gmode ^= MDDTKILL;
	    break;
	  case 'm':			/* turn on mouse support */
	    gmode ^= MDMOUSE;
	    break;
	  case 'p':
	    preserve_start_stop = 1;
	    break;
	  case 'q':			/* -q for termcap takes precedence */
	    gmode ^= MDTCAPWINS;
	    break;
	  case 't':			/* special shutdown mode */
	    gmode ^= MDTOOL;
	    rebindfunc(wquit, quickexit);
	    break;
	  case 'v':			/* -v for View File */
	  case 'V':
	    *viewflag = !*viewflag;
	    break;			/* break back to inner-while */
	  case 'w':			/* -w turn off word wrap */
	    gmode ^= MDWRAP;
	    break;
	  case 'x':			/* suppress keyhelp */
	    sup_keyhelp = !sup_keyhelp;
	    break;
	  case 'z':			/* -z to suspend */
	    gmode ^= MDSSPD;
	    break;

	  /*
	   * These do take arguments.
	   */
	  case 'r':			/* set fill column */
	  case 'n':			/* -n for new mail notification */
	  case 's' :			/* speller */
	  case 'o' :			/* operating tree */
	  case 'Q' :                    /* Quote string */
	  case 'W' :                    /* Word separators */
	    if(*++*av)
	      str = *av;
	    else if(--ac)
	      str = *++av;
	    else{
	      if(c == 'r')
		str= "72";
	      else if(c == 'n')
		str = "180";
	      else{
		  snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_arg), c);
		  pico_display_args_err(tmp_1k_buf, NULL, 1);
		  usage++;
		  goto Loop;
	      }
	    }

	    switch(c){
	      case 's':
	        alt_speller = str;
		break;
	      case 'o':
		strncpy(opertree, str, NLINE);
		gmode ^= MDTREE;
		break;
	      case 'Q':
		glo_quote_str_orig = str;
		break;
	      case 'W':
		glo_wordseps_orig = str;
		break;

	/* numeric args */
	      case 'r':
	      case 'n':
		if(!isdigit((unsigned char)str[0])){
		  snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_num), c);
		  pico_display_args_err(tmp_1k_buf, NULL, 1);
		  usage++;
		}

		if(c == 'r'){
		    if((userfillcol = atoi(str)) < 1)
		      userfillcol = 72;
		}
		else{
		    timeoutset = 1;
		    set_input_timeout(180);
		    if(set_input_timeout(atoi(str)) < 30)
		      set_input_timeout(180);
		}
		
		break;
	    }

	    goto Loop;

	  default:			/* huh? */
	    snprintf(tmp_1k_buf, sizeof(tmp_1k_buf), _(args_pico_missing_flag), c);
	    pico_display_args_err(tmp_1k_buf, NULL, 1);
	    usage++;
	    break;
	}
      }
    }

    if(usage)
      pico_args_help();

#ifndef _WINDOWS
    Pcolors = pico_set_global_colors(ncolors, ntfc, ntbc, rtfc, rtbc,
		tbfc, tbbc, klfc, klbc, knfc, knbc, stfc, stbc, prfc, prbc,
		q1fc, q1bc, q2fc, q2bc, q3fc, q3bc, sbfc, sbbc);

    if(Pcolors)
       pico_toggle_color(1);
#endif /* ! _WINDOWS */

    /* return the first filename for editing */
    if(ac > 0)
      return(*av);
    else
      return(NULL);
}


#ifndef _WINDOWS
PCOLORS *
pico_set_global_colors(int nc, int ntfg, int ntbg, int rtfg, int rtbg,
	int tbfg, int tbbg, int klfg, int klbg,
	int knfg, int knbg, int stfg, int stbg, int prfg, int prbg,
	int q1fg, int q1bg, int q2fg, int q2bg, int q3fg, int q3bg,
	int sbfg, int sbbg)
{
  PCOLORS *pcolors = NULL;
  char *fg, *bg;

  if(nc != 0 && nc != 8 && nc != 16 && nc != 256){
    fprintf(stderr, "number of colors must be either 8, 16 or 256\n");
    exit(1);
  }

  if(nc == 0){
     if(ntfg != -1 || ntbg != -1 || tbfg != -1 || tbbg != -1 || 
	klfg != -1 || klbg != -1 || knfg != -1 || knbg != -1 || 
	stfg != -1 || stbg != -1 || prfg != -1 || prbg != -1 ||
	q1fg != -1 || q1bg != -1 || q2fg != -1 || q1bg != -1 ||
	q3fg != -1 || q3bg != -1 || sbfg != -1 || sbbg != -1 ||
	rtfg != -1 || rtbg != -1){
	fprintf(stderr, "can not specify color numbers without specifying number of colors\n");
	exit(1);
     }
     else
	return pcolors;		/* no colors used */
  }

  if(ntfg >= nc || ntbg >= nc || tbfg >= nc || tbbg >= nc 
	|| klfg >= nc || klbg >= nc || knfg >= nc || knbg >= nc 
	|| stfg >= nc || stbg >= nc || prfg >= nc || prbg >= nc 
	|| q1fg >= nc || q1bg >= nc || q2fg >= nc || q2bg >= nc 
	|| q3fg >= nc || q3bg >= nc || sbfg >= nc || sbbg >= nc
	|| rtfg >= nc || rtbg >= nc){
	fprintf(stderr, "Error: specified a color bigger than number of colors\n");
	exit(1);
  }

  /* Finally, we set up colors */
  pico_toggle_color(0);
  switch(nc){
    case   8: pico_set_color_options(COLOR_ANSI8_OPT|COLOR_TRANS_OPT); break;
    case  16: pico_set_color_options(COLOR_ANSI16_OPT|COLOR_TRANS_OPT); break;
    case 256: pico_set_color_options(COLOR_ANSI256_OPT|COLOR_TRANS_OPT); break;
    default : break;	/* impossible case */
  }
  pico_toggle_color(1);

  pcolors = (PCOLORS *) malloc(sizeof(PCOLORS));
  memset((void *)pcolors, 0, sizeof(PCOLORS));
  /* ignore bad pair settings, also we set tbcp backwards because it will
   * be reversed later in the modeline function, so tbcp->fg is actually 
   * tbcp->bg and vice versa.
   */
  if(ntfg >= 0 && ntbg >= 0){	/* set normal text color */
        fg = colorx(ntfg); bg = colorx(ntbg);
	pcolors->ntcp = new_color_pair(fg, bg);
	pico_nfcolor(fg);
	pico_nbcolor(bg);
  }
  /* reverse means reverse of normal text */
  if((rtfg < 0 || rtbg < 0) && (ntfg >= 0 && ntbg >= 0)){
	rtfg = ntbg;
	rtbg = ntfg;
  }
  if(rtfg >= 0 && rtbg >= 0){	/* set reverse text color */
        fg = colorx(rtfg); bg = colorx(rtbg);
	pcolors->rtcp = new_color_pair(fg, bg);
	pico_rfcolor(fg);
	pico_rbcolor(bg);
  }
  /* If the user does not specify this, we will set up all 
   * backgrounds for text to the normal text background
   */
  if(ntbg >= 0){
    if(knbg < 0) knbg = ntbg;
    if(q1bg < 0) q1bg = ntbg;
    if(q2bg < 0) q2bg = ntbg;
    if(q3bg < 0) q3bg = ntbg;
    if(sbbg < 0) sbbg = ntbg;
  }
  if(rtfg >= 0 && rtbg >= 0){   /* set default reverse */
    if(tbfg < 0 || tbbg < 0){	/* set titlebar colors to reverse color if any missing*/
	tbfg = rtfg; tbbg = rtbg;
    }
    if(klfg < 0 || klbg < 0){	/* set key label colors */
	klfg = rtfg; klbg = rtbg;
    }
    if(stfg < 0 || stbg < 0){	/* set status colors */
	stfg = rtfg; stbg = rtbg;
    }
    if(prfg >= 0 && prbg >= 0){	/* set prompt colors */
	prfg = rtfg; prbg = rtbg;
    }
  }
  if(tbfg >= 0 && tbbg >= 0)	/* set titlebar colors */
     pcolors->tbcp = new_color_pair(colorx(tbbg), colorx(tbfg));
  if(klfg >= 0 && klbg >= 0)	/* set key label colors */
     pcolors->klcp = new_color_pair(colorx(klfg), colorx(klbg));
  if(knfg >= 0 && knbg >= 0)	/* set key name colors */
     pcolors->kncp = new_color_pair(colorx(knfg), colorx(knbg));
  if(stfg >= 0 && stbg >= 0)	/* set status colors */
     pcolors->stcp = new_color_pair(colorx(stfg), colorx(stbg));
  if(prfg >= 0 && prbg >= 0)	/* set prompt colors */
     pcolors->prcp = new_color_pair(colorx(prfg), colorx(prbg));
  if(q1fg >= 0 && q1bg >= 0)	/* set quote level 1 colors */
     pcolors->qlcp = new_color_pair(colorx(q1fg), colorx(q1bg));
  if(q2fg >= 0 && q2bg >= 0)	/* set quote level 2 colors */
     pcolors->qllcp = new_color_pair(colorx(q2fg), colorx(q2bg));
  if(q3fg >= 0 && q3bg >= 0)	/* set quote level 3 colors */
     pcolors->qlllcp = new_color_pair(colorx(q3fg), colorx(q3bg));
  if(sbfg >= 0 && sbbg >= 0)	/* set signature block colors */
     pcolors->sbcp = new_color_pair(colorx(sbfg), colorx(sbbg));

  if(pico_usingcolor()) 
     pico_set_normal_color();

  return pcolors;
}
#endif /* ! _WINDOWS */

#ifdef	_WINDOWS
/*
 *
 */
int
pico_file_drop(int x, int y, char *filename)
{
    /*
     * if current buffer is unchanged
     * *or* "new buffer" and no current text
     */
    if(((curwp->w_bufp->b_flag & BFCHG) == 0)
       || (curwp->w_bufp->b_fname[0] == '\0'
	   && curwp->w_bufp->b_linep == lforw(curwp->w_bufp->b_linep)
	   && curwp->w_doto == 0)){
	register BUFFER *bp = curwp->w_bufp;
	char     bname[NBUFN];

	makename(bname, filename);
	strncpy(bp->b_bname, bname, sizeof(bp->b_bname));
	bp->b_bname[sizeof(bp->b_bname)-1] = '\0';
	strncpy(bp->b_fname, filename, sizeof(bp->b_fname));
	bp->b_fname[sizeof(bp->b_fname)-1] = '\0';
	bp->b_flag &= ~BFCHG;	/* turn off change bit */
	if (readin(filename, 1, 1) == ABORT) {
	    strncpy(bp->b_bname, "", sizeof(bp->b_bname));
	    bp->b_bname[sizeof(bp->b_bname)-1] = '\0';
	    strncpy(bp->b_fname, "", sizeof(bp->b_fname));
	    bp->b_fname[sizeof(bp->b_fname)-1] = '\0';
	}

	bp->b_dotp = bp->b_linep;
	bp->b_doto = 0;
    }
    else{
	EML eml;

	ifile(filename);
	curwp->w_flag |= WFHARD;
	update();
	eml.s = filename;
	emlwrite("Inserted dropped file \"%s\"", &eml);
    }

    curwp->w_flag |= WFHARD;
    update();			/* restore cursor */
    return(1);
}
#endif


/*----------------------------------------------------------------------
    print a few lines of help for command line arguments

  Args:  none

 Result: prints help messages
  ----------------------------------------------------------------------*/
void
pico_args_help(void)
{
    char **a;
    char *pp[2];

    pp[1] = NULL;

    /**  print out possible starting arguments... **/

    for(a=args_pico_args; a && *a; a++){
	pp[0] = _(*a);
	pico_display_args_err(NULL, pp, 0);
    }

    exit(1);
}


void
pico_vers_help(void)
{
    char v0[100];
    char *v[2];

    snprintf(v0, sizeof(v0), "Pico %.50s", version);
    v[0] = v0;
    v[1] = NULL;

    pico_display_args_err(NULL, v, 0);
    exit(1);
}


/*----------------------------------------------------------------------
   write argument error to the display...

  Args:  none

 Result: prints help messages
  ----------------------------------------------------------------------*/
void
pico_display_args_err(char *s, char **a, int err)
{
    char  errstr[256], *errp;
    FILE *fp = err ? stderr : stdout;
#ifdef _WINDOWS
    char         tmp_20k_buf[SIZEOF_20KBUF];
#endif


    if(err && s)
      snprintf(errp = errstr, sizeof(errstr), _("Argument Error: %.200s"), s);
    else
      errp = s;

#ifdef  _WINDOWS
    if(errp)
      mswin_messagebox(errp, err);

    if(a && *a){
        strncpy(tmp_20k_buf, *a++, SIZEOF_20KBUF);
	tmp_20k_buf[SIZEOF_20KBUF-1] = '\0';
        while(a && *a){
            strncat(tmp_20k_buf, "\n", SIZEOF_20KBUF-strlen(tmp_20k_buf)-1);
	    tmp_20k_buf[SIZEOF_20KBUF-1] = '\0';
            strncat(tmp_20k_buf, *a++, SIZEOF_20KBUF-strlen(tmp_20k_buf)-1);
	    tmp_20k_buf[SIZEOF_20KBUF-1] = '\0';
        }

        mswin_messagebox(tmp_20k_buf, err);
    }
#else
    if(errp)
      fprintf(fp, "%s\n", errp);

    while(a && *a)
      fprintf(fp, "%s\n", *a++);
#endif
}
