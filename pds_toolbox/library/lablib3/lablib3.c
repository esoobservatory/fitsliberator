/*========================================================================*/
/*                                                                        */
/*                         PDS Label Library Lite                         */
/*                               (lablib3)                                */
/*                                                                        */
/*  Version:                                                              */
/*                                                                        */
/*      1.0Beta    Mar 31, 1994                                           */
/*      1.0        Jan 23, 1995                                           */
/*      1.1        Feb 23, 1995                                           */
/*      1.2        Jun 06, 1995  Preliminary                              */
/*                                                                        */
/*  Change History:                                                       */
/*                                                                        */
/*      03-31-94    Original code                                         */
/*      01-09-95    jsh - Changed OBJECT to OBJDESC                       */
/*      01-09-95    jsh - Corrected strcmp and strncmp == NULL            */
/*      02-16-95    jsh - Applied LASP changes (from OA dvlp - s. monk)   */
/*                      - Function Prototypes                             */
/*                      - Filename Units                                  */
/*                      - Filename for SUN/Unix                           */
/*                      - TB_MAX_BUFFER                                   */
/*                      - Several 0 -> NULL in function calls             */
/*      02-20-95    jsh - Added OdlPrintLine (reduced # fprintf)          */
/*      06-06-95    jsh/gmw - Stop gap for "/*" in text strings           */
/*      06-06-95    jsh/gmw - Allow SFDU without "= SFDU"                 */
/*      12-11-95    sm/LASP - Allow SFDU without "= SFDU"                 */
/*                            (Removed previous change, added new one.)   */
/*                          - Replaced 'class' by 'class_name' for C++    */ 
/*                            compatibility.                              */
/*                          - Added global variable odl_errno, and error  */
/*                            codes.                                      */
/*                          - Included oamalloc.h, and replaced 'malloc'  */
/*                            by 'OaMalloc'.                              */
/*                          - OdlValidElement: fixed dropping of trailing */
/*                            '>' from units expressions.                 */
/*                          - OdlDataType: '(' for ODL_SEQUENCE, '{' for  */
/*                            ODL_SET.                                    */
/*                          - OdlExpandLabelFile:                         */
/*                              - propagate 'suppress_messages' input     */
/*                                argument to OdlParseFile.               */
/*                              - move keywords resulting from expansions */
/*                                to immediately after the ^STRUCTURE or  */
/*                                ^CATALOG keyword, which is left as is.  */
/*                              - removed call to ExpandIsRecursive.      */
/*                              - added parameter to OdlGetFileSpec call. */
/*                          - OdlParseFile:
/*                              - added new input arg 'label_str', and    */
/*                                code to allow parsing a label string.   */
/*                              - fixed bug involving comments embedded   */
/*                                inside text strings.                    */
/*                          - OdlParseLabelString: modified to use new    */
/*                            OdlParseFile instead of creating tmp file.  */
/*                          - OdlCutObjDesc, OdlPasteObjDescBefore,       */
/*                            OdlPasteObjDescAfter: added robustness by   */
/*                            checking inputs for NULL.                   */
/*                          - OdlPrintKeywords: print SFDU keyword value  */
/*                            ("= SFDU_LABEL") if keyword value exists.   */
/*                          - OdlGetFileSpec: filled in the code for what */
/*                            was previously a stub;  added new function  */
/*                            OdlCheckFileSpec.                           */
/*                          - OdlPrintLabel: print label regardless of    */
/*                            value of odl_suppress_messages.             */
/*      02-13-95    sm/LASP - Changed TB_MAX_BUFFER in toolbox.h from     */
/*                            32767 to 30000 because some compilers don't */
/*                            allow more than 32K of local data.          */
/*      02-04-97    sm/LASP - Modified OdlParseFile and OdlPrintLabel to  */
/*                            parse, store and print pre-comments; added  */
/*                            keyword value alignment to OdlPrintKeywords */
/*      05-19-96    sm/LASP - Modified OdlGetFile to parse Macintosh-style*/
/*                            paths with spaces or other non-ASCII chars. */
/*      05-21-97    sm/LASP - added bug fix by CSC/John Kerich to do      */
/*                            start of line check in OdlWildCardCompare.  */
/*      01-29-98    sm/LASP - Added routines to get and set comments and  */
/*                            filename fields in OBJDESC and KEYWORD      */
/*                            structures.                                 */
/*      01-30-98    sm/LASP - Added argument 'options' to OdlPrintLabel   */
/*                            allow printing of expanded ^STRUCTURE nodes */
/*                            to a separate file, or to suppress printing */
/*                            of "END" statement.  Changed opening of     */
/*                            output file for append to opening for write.*/
/*                            Added user routine OdlChangeExpansionFile.  */
/*                            Added internal routines OdlChangeExpandFile */
/*                            and OdlExtractExpansionTree.                */
/*      02-07-03    DWS       Changed OdlGetFileSpec, see notes in        */
/*                            function.                                   */
/*		03-17-03    MDC		  Performed bug fixes and modifications to    */
/*							  OdlGetFileSpec. (See notes in function.)	  */
/*      06-10-03    MDC       Added a line in OdlExpandLabelFile. See     */
/*                            notes in function.                          */
/*      06-26-03    MDC       Modified OdlGetFileSpec routine. See notes  */
/*                            in function.                                */
/*      07-30-03    MDC       Modified OdlValidElement routine. See notes */
/*                            in function.                                */
/*      07-12-04    DWS       Added GROUP processing to the parser        */
/*      09-13-04    MDC       Modified OdlWildCardCompare. See notes in   */
/*                            function for details.                       */
/*      02-01-05    MDC       Modified OdlExpandLabelFile routine. See    */
/*                            notes.                                      */
/*      03-10-05    MDC       Modified OdlParseFile routine. See notes.   */
/*                                                                        */
/*========================================================================*/

#include "lablib3.h"

#ifdef XVT_DEF
#include "xvt.h"
#endif

long odl_message_count = {0};
short odl_suppress_messages = {FALSE};
int odl_errno;

#if defined(SUN_UNIX) || defined(OSX)
	char *pathSep = "/";
#endif
#ifdef MAC
	char *pathSep = ":";
#endif
#ifdef IBM_PC		
	char *pathSep = "\\";
#endif	


/*========================================================================*/
/*                                                                        */
/*                          Label Parse routines                          */
/*                                                                        */
/*========================================================================*/



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlParseLabelFile                                               */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      WARNING!  Do not use this routine unless you never plan to read */
/*      to read a variable-length record file - this routine will not   */
/*      work!   Use the Object Access Library routine OaParseLabelFile  */
/*      instead, as it handles all record types.                        */
/*                                                                      */
/*      This routine causes a file containing a PDS label to be parsed  */
/*      and its "^" keywords expanded.  It returns a pointer to the     */
/*      root of the OBJECT tree data structure.                         */
/*                                                                      */
/*      The "filespec" parameter is the full path and name of the       */
/*      label file to be parsed.                                        */
/*                                                                      */
/*      The "message_fname" parameter is the name of the file where     */
/*      parser error messages will be written.  If the file exists,     */
/*      the messages are appended, otherwise a new file is created.     */
/*      A NULL value passed in causes messages to be sent to stdout.    */
/*                                                                      */
/*      The "suppress_messages" parameter is a flag that tells the code */
/*      whether or not to print parser error messages.  A value of TRUE */
/*      (1) tells the code to supress all messages.  If this parameter  */
/*      is 1, it doesn't matter what you specified with the             */
/*      "message_fname".  Nothing will be written to that file.  If a   */
/*      zero (0) is passed in, then messages will be written to the     */
/*      file you specified with the "message_fname" parameter.          */
/*                                                                      */
/*      The expand parameter is a flag that controls whether or not     */
/*      ^STRUCTURE and ^CATALOG keywords are expanded.  To expand one   */
/*      of these keywords means to take the file name it points to,     */
/*      parse its contents, and insert the results into the tree right  */
/*      after the keyword.  The keyword itself is left unchanged.       */
/*      In other words, these keywords function just like include files */
/*      in "C".  These are the values that can be passed in:            */
/*                                                                      */
/*             ODL_EXPAND_STRUCTURE - expand ^STRUCTURE keywords only   */
/*             ODL_EXPAND_CATALOG   - expand ^CATALOG keywords only     */
/*             ODL_EXPAND_STRUCTURE | ODL_EXPAND_CATALOG - expand       */
/*                   both keywords (the "|" character is the logical    */
/*                   "or" of both values).                              */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*      WARNING:  The value returned by this routine points to memory   */
/*                allocated by this routine (sometimes quite a bit of   */
/*                memory!).  Be sure to deallocate it using the         */
/*                OdlFreeTree routine.                                  */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlParseLabelFile (filespec, message_fname, expand, suppress_messages)

char *filespec;
char *message_fname;
MASK expand;
unsigned short suppress_messages;

#else

OBJDESC *OdlParseLabelFile (char *filespec, char *message_fname, MASK expand, 
                           unsigned short suppress_messages)

#endif

{
    OBJDESC *root = {NULL};
    
    odl_suppress_messages = suppress_messages;
    root = (OBJDESC *) OdlParseFile(filespec,NULL,NULL,message_fname,NULL,
                                    suppress_messages,1,1,0);
    root = (OBJDESC *) OdlExpandLabelFile(root, message_fname, expand,
                                            suppress_messages);

    return(root);

}  /*  End:  "OdlParseLabelFile"  */




/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlParseLabelString                                             */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - Modified to use new OdlParseFile instead  */
/*                            of creating a temporary file.             */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine causes a character string containing an ODL        */
/*      statement to be parsed and its "^" keywords expanded.  It       */
/*      returns a pointer to the root of the OBJECT tree data structure.*/
/*                                                                      */
/*      WARNING:  The value returned by this routine points to memory   */
/*                allocated by this routine.  Be sure to deallocate it  */
/*                using the OdlFreeTree routine.                        */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlParseLabelString (odl_string, message_fname, 
                             expand, suppress_messages)

char *odl_string;
char *message_fname;
MASK expand;
unsigned short suppress_messages;

#else

OBJDESC *OdlParseLabelString (char *odl_string, char *message_fname, 
                             MASK expand, unsigned short suppress_messages)

#endif
{
    OBJDESC *root = {NULL};

    odl_suppress_messages = suppress_messages;

    root = (OBJDESC *) OdlParseFile(NULL,NULL,odl_string,message_fname,NULL,
                                    suppress_messages,1,1,0);
    root = (OBJDESC *) OdlExpandLabelFile(root, message_fname, expand,
                                            suppress_messages);
    return(root);

}  /*  End:  "OdlParseLabelString"  */




/*========================================================================*/
/*                                                                        */
/*                        Label Expand routines                           */
/*                                                                        */
/*========================================================================*/

/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlExpandLabelFile                                              */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - Pass suppress_messages to OdlParseFile.   */
/*      12-11-95    sm/LASP - Move keywords resulting from expansions   */
/*                            to immediately after the ^STRUCTURE or    */
/*                            ^CATALOG keyword, instead of after the    */
/*                            last keyword.                             */
/*                          - Removed call to ExpandIsRecursive.        */
/*                          - Added parameter to OdlGetFileSpec call.   */
/*      06-10-03    MDC - Added a line to transfer the child count      */
/*                        value to the actual tree we are building.     */
/*      02-01-05    MDC - Added preprocessor definition to print out an */
/*                        error msg to the user if FMT file cannot be   */
/*                        found                                         */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates "^STRUCTURE" and "^CATALOG" keywords and   */
/*      expands them.  To "expand" means to extract the file name       */
/*      pointed to by the keyword, parse its contents, and insert the   */
/*      resulting tree into the label right after the keyword.          */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlExpandLabelFile (object, message_fname, expand, suppress_messages)

OBJDESC *object;
char *message_fname;
MASK expand;
unsigned short suppress_messages;

#else

OBJDESC *OdlExpandLabelFile (OBJDESC *object, char *message_fname,
                             MASK expand, unsigned short suppress_messages)

#endif
{
    KEYWORD *kwd = {NULL};
    KEYWORD *new_kwd = {NULL};
    KEYWORD *save_kwd = {NULL};
    OBJDESC *temp_root = {NULL};
	OBJDESC *temp_ptr = {NULL};
    OBJDESC *new_obj = {NULL};
    OBJDESC *save_obj = {NULL};
    FILE *l_ptr = {NULL};
    unsigned long start_loc = {1};
    unsigned short loc_type = {ODL_RECORD_LOCATION};
    unsigned short done = {FALSE};
    char *fspec = {NULL};
    char *fname = {NULL};
    char *keyword_search_string = {NULL};
    char *fudge_string = {"_TEMPORARILY_RENAMED"};
    char error_message[5*(TB_MAXLINE + TB_MAXPATH + TB_MAXFNAME)];
	char prev_error_message = NULL;

    odl_suppress_messages = suppress_messages;

    /*  Let's expand all ^STRUCTURE keywords, shall we?  */
    if ((expand&ODL_EXPAND_STRUCTURE) == ODL_EXPAND_STRUCTURE)
    {
        expand -= ODL_EXPAND_STRUCTURE;
        CopyString(keyword_search_string, "^*STRUCTURE")
    }
    else
    /*  On second thought, let's expand all ^CATALOG keywords  */
    if ((expand&ODL_EXPAND_CATALOG) == ODL_EXPAND_CATALOG)
    {
        expand -= ODL_EXPAND_CATALOG;
        CopyString(keyword_search_string, "^*CATALOG")
    }
    else
    /*  Hmmm. I guess we have nothing left to expand.  */
    {
        expand = ODL_NOEXPAND;
        done = TRUE;
    }

    /*  Keep expanding until we can expand no more forever  */
    while (! done)
    {
        /*  Find the expand keyword wherever it my be hiding  */
        kwd = (KEYWORD *) OdlFindKwd(object, keyword_search_string,
                                     NULL, 1, ODL_RECURSIVE_DOWN);

        /*  We're done if there aren't any more keywords to expand  */
        if (kwd == NULL)
            done = TRUE;
        else
        {
            /*  Get the file name from the "^*STRUCTURE" or "^*CATALOG" keyword
                value, minus quotes and blanks, sans path  */
            fname = (char *) OdlGetFileName(kwd, &start_loc, &loc_type);
            /*  Figure out exactly where the file is located  */
            fspec = (char *) OdlGetFileSpec(fname, kwd->file_name,
                                            keyword_search_string);

            /*  We're in trouble if we can't find the file  */
            if (fspec == NULL)
            {
                sprintf(error_message, 
                        "Unable to locate %s file:  %s",
                        keyword_search_string, fname);
                odl_errno = 200;

				/* 02-01-05 MDC - Print out an error message to the user */
#ifdef XVT_DEF
				xvt_dm_post_note("Line %d: %s", kwd->line_number, error_message);
#else
                OdlPrintMessage(message_fname,NULL,kwd->line_number,
                                error_message);
#endif
            }
            else
            {
                l_ptr = (FILE *) OdlLocateStart(fspec, start_loc, loc_type);

   
                /*  Parse the file  */
                temp_root = (OBJDESC *) OdlParseFile(fspec,l_ptr,NULL,
                                                     message_fname,NULL,
                                                     suppress_messages,1,1,
                                                     1);
                    
                /*  Was there anything in the file to parse?  */
                if (temp_root != NULL)
                {
                    /*  Move keywords from the temporary root node to just
                        after the ^STRUCTURE or ^CATALOG keyword.  */
                    for (new_kwd=temp_root->last_keyword;
                             new_kwd != NULL; new_kwd = save_kwd)
                    {
                        save_kwd = new_kwd->left_sibling;
                        OdlPasteKwdAfter((KEYWORD *) OdlCutKwd(new_kwd),
                                          kwd);
                    }
					/* 02-09-05 MDC - We need to find out if there's any existing
					   COLUMN or CONTAINER objects outside of objects inside the STRUCTURE 
					   pointer file. For now, we assume that all objects found inside the pointer
					   file go before any of the COLUMN or CONTAINER objects we might find.
				    */
					if(kwd->parent->first_child != NULL)
					{
						temp_ptr = OdlFindObjDesc( kwd->parent->first_child, "COLUMN", NULL, NULL, 1, ODL_CHILDREN_ONLY); 
                        
						if(temp_ptr == NULL)
						{
							temp_ptr = OdlFindObjDesc( kwd->parent->first_child, "CONTAINER", NULL, NULL, 1, ODL_CHILDREN_ONLY);
						}
					}



                    /*  Move sub-objects of the temporary root node to
                        under the ^STRUCTURE or ^CATALOG keyword's node. */
                    for (new_obj=temp_root->first_child;
                         new_obj != NULL; new_obj = save_obj)
                    {
                        save_obj = new_obj->right_sibling;


						if(temp_ptr != NULL)
						{
							OdlPasteObjDescBefore( OdlCutObjDesc(new_obj), temp_ptr);
						}
						else
						{
							OdlPasteObjDesc( OdlCutObjDesc(new_obj),
                                         kwd->parent);
						}
                    }
					
					/*
					   06-10-03 MDC - Transfer the child_count value to
					   the actual tree. 
					*/
					kwd->parent->child_count = temp_root->child_count;

                    /*  Deallocate the temporary root  */
                    temp_root->first_keyword = NULL;
                    temp_root->first_child = NULL;
                    temp_root = (OBJDESC *) OdlFreeTree(temp_root);
					temp_ptr = NULL;
                }  /*  End:  "if (temp_root != NULL) ..."  */
    
                /*  Free the file spec storage  */
                LemmeGo(fspec)

            }  /*  End:  "if (fspec == NULL) ... else ..."  */

            /* Temporarily rename the ^STRUCTURE or ^CATALOG keyword so 
               it won't be found again at the top of the loop (and thus
               expanded again).  After the loop we'll change it back
               to its original value.  sm 2/4/97 */

            AppendString( kwd->name, fudge_string);

            /*  Free the file name storage  */
            LemmeGo(fname)
            
        }  /*  End:  "if (kwd == NULL) ... else ..."  */

    }  /*  End:  "while (! done) ..."  */

    /* Restore any keyword names changed above to their original values. */
    AppendString( keyword_search_string, fudge_string)
    while (1)
    {
        kwd = (KEYWORD *) OdlFindKwd(object, keyword_search_string,
                                     NULL, 1, ODL_RECURSIVE_DOWN);
        if (kwd == NULL)  break;

        kwd->name[ strlen( kwd->name) - strlen( fudge_string)] = '\0';
        strcpy( error_message, kwd->name);
        LemmeGo( kwd->name)
        CopyString( kwd->name, error_message)
    }

    /*  Free the keyword search string  */
    LemmeGo(keyword_search_string)

    /*  Repeat this whole process for the next type of EXPAND keyword,
        if another was specified.  E.g. we just expanded all the
        ^STRUCTURE keywords, now we'll expand all the ^CATALOG keywords.  */
    if (expand != ODL_NOEXPAND)
    {
        object = (OBJDESC *) OdlExpandLabelFile(object, message_fname, 
                                               expand, suppress_messages);
    }

    /*  Return the root of the expanded tree  */
    return(object);

}  /*  End:  "OdlExpandLabelFile"  */



/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static unsigned short ExpandIsRecursive (keyword, exp_fname)

KEYWORD *keyword;
char *exp_fname;

#else

static unsigned short ExpandIsRecursive (KEYWORD *keyword, char *exp_fname)

#endif
{
    OBJDESC *obj = {NULL};
    char *temp_fname = {NULL};
    unsigned short found = {FALSE};

    if ((keyword != NULL) && (exp_fname != NULL))
    {
#if (defined( VAX) || defined( ALPHA_VMS))
        UpperCase(exp_fname)
#endif

        CopyString(temp_fname, keyword->file_name)

#if (defined( VAX) || defined( ALPHA_VMS))
        UpperCase(temp_fname)
#endif

        found = (strcmp(temp_fname, exp_fname) == 0);
        LemmeGo(temp_fname)

        for (obj=keyword->parent; 
                  ((! found) && (obj != NULL)); obj=obj->parent)
        {
            CopyString(temp_fname, obj->file_name)

#if (defined( VAX) || defined( ALPHA_VMS))
            UpperCase(temp_fname)
#endif

            found = (strcmp(temp_fname, exp_fname) == 0);
            LemmeGo(temp_fname)
        }
    }

    return(found);

}  /*  End:  "ExpandIsRecursive"  */





/*========================================================================*/
/*                                                                        */
/*                     Object description routines                        */
/*                                                                        */
/*========================================================================*/

/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlFindObjDesc                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates an object within a parsed label by its     */
/*      class name (like TABLE), by its position (look for the seventh  */
/*      table object in the label), by a particular keyword present     */
/*      in the object (like NAME), or by a particular value that a      */
/*      particular keyword has (like START_BYTE = 76).                  */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlFindObjDesc(start_object, object_class, keyword_name, 
                       keyword_value, object_position, search_scope)

OBJDESC *start_object;
char *object_class;
char *keyword_name;
char *keyword_value;
unsigned long object_position;
unsigned short search_scope;

#else

OBJDESC *OdlFindObjDesc( OBJDESC *start_object, char *object_class, 
                        char *keyword_name, char *keyword_value, 
                        unsigned long object_position, 
                        unsigned short search_scope)

#endif
{
    OBJDESC *found_object = {NULL};
    OBJDESC *obj = {NULL};
    KEYWORD *kwd = {NULL};
    unsigned short found = {FALSE};
    unsigned short scope = {search_scope};
    unsigned long current_position = {0};

    for (obj=start_object;
          ((obj != NULL) && (! found));
            obj = (OBJDESC *) OdlNextObjDesc(obj, start_object->level, &scope))
    {
        if (object_class == NULL)
             found = TRUE;
        else
             found = OdlWildCardCompare(object_class, obj->class_name);

        if ((found) && (keyword_name != NULL))
        {
            kwd = (KEYWORD *) OdlFindKwd(obj, keyword_name, 
                                         NULL, 1, ODL_THIS_OBJECT);
            found = (kwd != NULL);
        }

        if ((found) && (keyword_value != NULL))
            found = OdlWildCardCompare(keyword_value, (char *) OdlGetKwdValue(kwd));

        if ((found) && (object_position > 0))
            found = ((++current_position) == object_position);

        if (found) found_object = obj;    

    }  /*  End:  "for (obj=start_object; ..."  */

    return(found_object);

}  /*  End:  "OdlFindObjDesc"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlNextObjDesc                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates the next object in the tree based on the   */
/*      search_scope passed in.                                         */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlNextObjDesc (object, root_level, search_scope)

OBJDESC *object;
unsigned long root_level;
unsigned short *search_scope;

#else

OBJDESC *OdlNextObjDesc (OBJDESC *object, unsigned long root_level, 
                        unsigned short *search_scope)

#endif
{
    OBJDESC *next_object = {NULL};

    if (object != NULL)
    {
        switch (*search_scope)
        {
            /*  look only in the current object  */
            case ODL_THIS_OBJECT    :  next_object = NULL;
                                       break;

            /*  look at the current object's first child now, and its   */
            /*  child's right siblings in subsequent searches           */
            case ODL_CHILDREN_ONLY  :  next_object = object->first_child;
                                       *search_scope = ODL_SIBLINGS_ONLY;
                                       break;

            /*  look at the current object's right sibling  */
            case ODL_SIBLINGS_ONLY  :  next_object = object->right_sibling;
                                       break;

            /*  treat the current object as the root of a sub-tree  */
            case ODL_RECURSIVE_DOWN :  next_object = (OBJDESC *) OdlTraverseTree(object, root_level);
                                       break;

            /*  search children, then siblings, then move up to parent  */
            /*  keep going until the end of the label is reached        */
            default                 :  next_object = (OBJDESC *) 
                                                      OdlTraverseTree(object, 
                                                                     (unsigned long) 0);
                                       break;

        }  /*  End:  "switch (*search_scope) ..."  */

    }  /*  End:  "if (object != NULL) ..."  */

    return(next_object);

}  /*  End:  "OdlNextObjDesc"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlCutObjDesc                                                   */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - prevent crash when object has no parent.  */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine cuts an ODL object structure out of an ODL tree    */
/*      and returns a pointer to it.  All references to it in the tree  */
/*      are removed, and references within the object to its parent and */
/*      siblings in the original tree are removed.  The object retains  */
/*      its children, if any.                                           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlCutObjDesc (object)

OBJDESC *object;

#else

OBJDESC *OdlCutObjDesc (OBJDESC *object)

#endif
{
    if (object != NULL)
    {
        if (object->parent == NULL)
            return( object);

        if (object->right_sibling == NULL)
            object->parent->last_child = object->left_sibling;
        else
            object->right_sibling->left_sibling = object->left_sibling;

        if (object->left_sibling == NULL)
            object->parent->first_child = object->right_sibling;
        else
            object->left_sibling->right_sibling = object->right_sibling;

        object->parent = NULL;
        object->left_sibling = NULL;
        object->right_sibling = NULL;

    }  /*  End:  "if (object != NULL) ..."  */
    else
        odl_errno = 1;

    return(object);

}  /*  End routine:  "OdlCutObjDesc"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPasteObjDesc                                                 */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine adds an object to a tree as the last child of the  */
/*      parent_object.                                                  */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlPasteObjDesc (new_object, parent_object)

OBJDESC *new_object;
OBJDESC *parent_object;

#else

OBJDESC *OdlPasteObjDesc (OBJDESC *new_object, OBJDESC *parent_object)

#endif
{
    if ((new_object != NULL) && (parent_object != NULL))
    {
        new_object->left_sibling = parent_object->last_child;
        new_object->right_sibling = NULL;
        new_object->parent = parent_object;
    
        if (parent_object->first_child == NULL)
            parent_object->first_child = new_object;

        if (parent_object->last_child != NULL)
            parent_object->last_child->right_sibling = new_object;

        parent_object->last_child = new_object;

        OdlAdjustObjDescLevel(new_object);

    }  /*  End:  "if ((new_object != NULL) && ..."  */
    else
        odl_errno = 1;

    return(new_object);

}  /*  End routine:  "OdlPasteObjDesc"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPasteObjDescBefore                                           */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - Return NULL if old_object has no parent.  */
/*                                                                      */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine adds an object to a tree as the left sibling of    */
/*      the old_object.                                                 */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlPasteObjDescBefore (new_object, old_object)

OBJDESC *new_object;
OBJDESC *old_object;

#else

OBJDESC *OdlPasteObjDescBefore (OBJDESC *new_object, OBJDESC *old_object)

#endif
{
    if ((new_object != NULL) && (old_object != NULL))
    {
        if (old_object->parent == NULL) 
        {
            odl_errno = 10;
            return( NULL);
        }

        new_object->left_sibling = old_object->left_sibling;
        new_object->right_sibling = old_object;
        new_object->parent = old_object->parent;
    
        if (old_object->left_sibling == NULL)
            old_object->parent->first_child = new_object;
        else
            old_object->left_sibling->right_sibling = new_object;

        old_object->left_sibling = new_object;

        OdlAdjustObjDescLevel(new_object);

    }  /*  End:  "if ((new_object != NULL) && ..."  */
    else
        odl_errno = 1;
    
    return(new_object);

}  /*  End routine:  "OdlPasteObjDescBefore"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPasteObjDescAfter                                            */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - Return NULL if old_object has no parent.  */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine adds an object to a tree as the right sibling of   */
/*      the old_object.                                                 */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlPasteObjDescAfter (new_object, old_object)

OBJDESC *new_object;
OBJDESC *old_object;

#else

OBJDESC *OdlPasteObjDescAfter (OBJDESC *new_object, OBJDESC *old_object)

#endif
{
    if ((new_object != NULL) && (old_object != NULL))
    {
        if (old_object->parent == NULL)
        {
            odl_errno = 10;
            return( NULL);
        }

        new_object->right_sibling = old_object->right_sibling;
        new_object->left_sibling = old_object;
        new_object->parent = old_object->parent;
    
        if (old_object->right_sibling == NULL)
            old_object->parent->last_child = new_object;
        else
            old_object->right_sibling->left_sibling = new_object;

        old_object->right_sibling = new_object;

        OdlAdjustObjDescLevel(new_object);

    }  /*  End:  "if ((new_object != NULL) && ..."  */
    else
        odl_errno = 1;

    return(new_object);

}  /*  End routine:  "OdlPasteObjDescAfter"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlCopyObjDesc                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine makes a copy of an object and returns a pointer    */
/*      to the copy.  All fields are duplicated except for references   */
/*      to the original tree, which are removed.                        */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlCopyObjDesc (object)

OBJDESC *object;

#else

OBJDESC *OdlCopyObjDesc (OBJDESC *object)

#endif
{
    OBJDESC *new_object = {NULL};

    if (object != NULL)
    {
        new_object = OdlNewObjDesc(object->class_name, 
                               object->pre_comment, object->line_comment,
                               object->post_comment, object->end_comment,
                               object->file_name, object->is_a_group, 
                               object->line_number);
    }
    else
        odl_errno = 1;
    
    return(new_object);

}  /*  End routine:  "OdlCopyObjDesc"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlNewObjDesc                                                   */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - Replaced 'malloc' by 'OaMalloc'.          */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine creates a new object structure and initializes     */
/*      its fields with the values passed in.                           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlNewObjDesc (object_class, pre_comment, line_comment, post_comment, 
                       end_comment, file_name, is_a_group, line_number)
char *object_class;
char *pre_comment;
char *line_comment;
char *post_comment;
char *end_comment;
char *file_name;
short is_a_group;
long line_number;

#else

OBJDESC *OdlNewObjDesc (char *object_class, char *pre_comment, 
                       char *line_comment, char *post_comment, 
                       char *end_comment, char *file_name, short is_a_group,
                       long line_number)

#endif
{
    OBJDESC *new_object = {NULL};

    if ((new_object = (OBJDESC *)OaMalloc((long) sizeof(OBJDESC))) == NULL) 
    {
        odl_errno = 50;
        SayGoodbye()
    }
    else
    {
        CopyString(new_object->class_name, object_class)
        CopyString(new_object->pre_comment, pre_comment)
        CopyString(new_object->line_comment, line_comment)
        CopyString(new_object->post_comment, post_comment)
        CopyString(new_object->end_comment, end_comment)
        CopyString(new_object->file_name, file_name)

        new_object->is_a_group = is_a_group;
        new_object->child_count = 0;
        new_object->line_number = line_number;
        new_object->level = 0;
        new_object->parent = NULL;
        new_object->left_sibling = NULL;
        new_object->right_sibling = NULL;
        new_object->first_child = NULL;
        new_object->last_child = NULL;
        new_object->first_keyword = NULL;
        new_object->last_keyword = NULL;
        new_object->appl1 = NULL;
        new_object->appl2 = NULL;

    }  /*  End:  "if ((new_object = ... else ..."  */

    return(new_object);

}  /*  End routine:  "OdlNewObjDesc"  */




/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetLabelVersion                                              */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns a pointer to a character string containing */
/*      the ODL version of the label.  It looks for this information in */
/*      the ODL_VERSION_NUMBER keyword.                                 */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THIS ROUTINE.  THE RETURN   */
/*                VALUE IS A POINTER TO THE ACTUAL VALUE OF THE         */
/*                ODL_VERSION_NUMBER KEYWORD AND MUST NOT BE FREED.     */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlGetLabelVersion (object)

OBJDESC *object;

#else

char *OdlGetLabelVersion (OBJDESC *object)

#endif
{
    KEYWORD *kwd = {NULL};
    char *version = {NULL};

    if (object != NULL)
    {
        kwd = (KEYWORD *) OdlFindKwd(object, "PDS_VERSION_ID", 
                                     NULL, 1, ODL_THIS_OBJECT);
        if (kwd == NULL) 
        {
            kwd = (KEYWORD *) OdlFindKwd(object, "ODL_VERSION_NUMBER", 
                                         NULL, 1, ODL_THIS_OBJECT);
        }

        if (kwd != NULL) 
            version = (char *) OdlGetKwdValue(kwd);
    }
    else
        odl_errno = 1;

    return(version);

}  /*  End:  "OdlGetLabelVersion"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetObjDescClassName                                          */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns the class name of an object.               */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THIS ROUTINE.  THE RETURN   */
/*                VALUE IS A POINTER TO THE ACTUAL INFORMATION STORED   */
/*                IN THE ODL OBJECT STRUCTURE AND MUST NOT BE FREED.    */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlGetObjDescClassName (object)

OBJDESC *object;

#else

char *OdlGetObjDescClassName (OBJDESC *object)

#endif
{
    char *class_name = {NULL};

    if (object != NULL)
        class_name = object->class_name;
    else
        odl_errno = 1;

    return(class_name);

}  /*  End:  "OdlGetObjDescClassName"  */



/************************************************************************/
/*                                                                      */
/*  Components:                                                         */
/*                                                                      */
/*      OdlGetObjPreComment                                             */
/*      OdlGetObjLineComment                                            */
/*      OdlGetObjPostComment                                            */
/*      OdlGetObjEndComment                                             */
/*      OdlGetObjFilename                                               */
/*      OdlGetKwdPreComment                                             */
/*      OdlGetKwdLineComment                                            */
/*      OdlGetKwdFilename                                               */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      Steve Monk (Laboratory for Atmospheric and Space Physics)       */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    January 20, 1998                                         */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      01-20-98    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      These routines return a specific comment string or filename     */
/*      string from an object or keyword structure.                     */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THESE ROUTINES.  THE RETURN */
/*                VALUE OF EACH ROUTINE IS A POINTER TO THE ACTUAL      */
/*                INFORMATION STORED IN THE ODL OBJECT OR KEYWORD       */
/*                STRUCTURE AND MUST NOT BE FREED.                      */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO
char *OdlGetObjPreComment (object)
OBJDESC *object;
#else
char *OdlGetObjPreComment (OBJDESC *object)
#endif
{
    char *str = {NULL};
    if (object != NULL)
        str = object->pre_comment;
    else
        odl_errno = 1;
    return(str);
}  /*  End:  "OdlGetObjPreComment"  */

#ifdef _NO_PROTO
char *OdlGetObjLineComment (object)
OBJDESC *object;
#else
char *OdlGetObjLineComment (OBJDESC *object)
#endif
{
    char *str = {NULL};
    if (object != NULL)
        str = object->line_comment;
    else
        odl_errno = 1;
    return(str);
}  /*  End:  "OdlGetObjLineComment"  */

#ifdef _NO_PROTO
char *OdlGetObjPostComment (object)
OBJDESC *object;
#else
char *OdlGetObjPostComment (OBJDESC *object)
#endif
{
    char *str = {NULL};
    if (object != NULL)
        str = object->post_comment;
    else
        odl_errno = 1;
    return(str);
}  /*  End:  "OdlGetObjPostComment"  */


#ifdef _NO_PROTO
char *OdlGetObjEndComment (object)
OBJDESC *object;
#else
char *OdlGetObjEndComment (OBJDESC *object)
#endif
{
    char *str = {NULL};
    if (object != NULL)
        str = object->end_comment;
    else
        odl_errno = 1;
    return(str);
}  /*  End:  "OdlGetObjEndComment"  */



#ifdef _NO_PROTO
char *OdlGetObjFilename (object)
OBJDESC *object;
#else
char *OdlGetObjFilename (OBJDESC *object)
#endif
{
    char *str = {NULL};
    if (object != NULL)
        str = object->file_name;
    else
        odl_errno = 1;
    return(str);
}  /*  End:  "OdlGetObjFilename"  */


#ifdef _NO_PROTO
char *OdlGetKwdPreComment (keyword)
KEYWORD *keyword;
#else
char *OdlGetKwdPreComment (KEYWORD *keyword)
#endif
{
    char *str = {NULL};
    if (keyword != NULL)
        str = keyword->pre_comment;
    else
        odl_errno = 1;
    return(str);
}  /*  End:  "OdlGetKwdPreComment"  */

#ifdef _NO_PROTO
char *OdlGetKwdLineComment (keyword)
KEYWORD *keyword;
#else
char *OdlGetKwdLineComment (KEYWORD *keyword)
#endif
{
    char *str = {NULL};
    if (keyword != NULL)
        str = keyword->line_comment;
    else
        odl_errno = 1;
    return(str);
}  /*  End:  "OdlGetKwdLineComment"  */

#ifdef _NO_PROTO
char *OdlGetKwdFilename (keyword)
KEYWORD *keyword;
#else
char *OdlGetKwdFilename (KEYWORD *keyword)
#endif
{
    char *str = {NULL};
    if (keyword != NULL)
        str = keyword->file_name;
    else
        odl_errno = 1;
    return(str);
}  /*  End:  "OdlGetKwdFilename"  */



/************************************************************************/
/*                                                                      */
/*  Components:                                                         */
/*                                                                      */
/*      OdlSetObjPreComment                                             */
/*      OdlSetObjLineComment                                            */
/*      OdlSetObjPostComment                                            */
/*      OdlSetObjEndComment                                             */
/*      OdlSetObjFilename                                               */
/*      OdlSetKwdPreComment                                             */
/*      OdlSetKwdLineComment                                            */
/*      OdlSetKwdFilename                                               */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      Steve Monk (Laboratory for Atmospheric and Space Physics)       */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    January 20, 1998                                         */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      01-20-98    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      These routines set a specific comment string or filename string */
/*      in an object or keyword structure.  The input string is copied  */
/*      before setting it as the new value.  The old string is freed.   */
/*      If the input string is NULL, the corresponding string in the    */
/*      object or keyword structure is set to NULL after freeing the    */
/*      old string.                                                     */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO
void OdlSetObjPreComment (object, pre_comment)
OBJDESC *object;
char *pre_comment;
#else
void OdlSetObjPreComment (OBJDESC *object, char *pre_comment)
#endif
{
    char *str = {NULL};
    if (pre_comment != NULL)
        CopyString( str, pre_comment)
    LemmeGo( object->pre_comment)
    object->pre_comment = str;
    return;
}  /*  End:  "OdlSetObjPreComment"  */

#ifdef _NO_PROTO
void OdlSetObjLineComment (object, line_comment)
OBJDESC *object;
char *line_comment;
#else
void OdlSetObjLineComment (OBJDESC *object, char *line_comment)
#endif
{
    char *str = {NULL};
    if (line_comment != NULL)
        CopyString( str, line_comment)
    LemmeGo( object->line_comment)
    object->line_comment = str;
    return;
}  /*  End:  "OdlSetObjLineComment"  */

#ifdef _NO_PROTO
void OdlSetObjPostComment (object, post_comment)
OBJDESC *object;
char *post_comment;
#else
void OdlSetObjPostComment (OBJDESC *object, char *post_comment)
#endif
{
    char *str = {NULL};
    if (post_comment != NULL)
        CopyString( str, post_comment)
    LemmeGo( object->post_comment)
    object->post_comment = str;
    return;
}  /*  End:  "OdlSetObjPostComment"  */


#ifdef _NO_PROTO
void OdlSetObjEndComment (object, end_comment)
OBJDESC *object;
char *end_comment;
#else
void OdlSetObjEndComment (OBJDESC *object, char *end_comment)
#endif
{
    char *str = {NULL};
    if (end_comment != NULL)
        CopyString( str, end_comment)
    LemmeGo( object->end_comment)
    object->end_comment = str;
    return;
}  /*  End:  "OdlSetObjEndComment"  */

#ifdef _NO_PROTO
void OdlSetObjFilename (object, filename)
OBJDESC *object;
char *filename;
#else
void OdlSetObjFilename (OBJDESC *object, char *filename)
#endif
{
    char *str = {NULL};
    if (filename != NULL)
        CopyString( str, filename)
    LemmeGo( object->file_name)
    object->file_name = str;
    return;
}  /*  End:  "OdlSetObjFilename"  */

#ifdef _NO_PROTO
void OdlSetKwdPreComment (keyword, pre_comment)
KEYWORD *keyword;
char *pre_comment;
#else
void OdlSetKwdPreComment (KEYWORD *keyword, char *pre_comment)
#endif
{
    char *str = {NULL};
    if (pre_comment != NULL)
        CopyString( str, pre_comment)
    LemmeGo( keyword->pre_comment)
    keyword->pre_comment = str;
    return;
}  /*  End:  "OdlSetKwdPreComment"  */

#ifdef _NO_PROTO
void OdlSetKwdLineComment (keyword, line_comment)
KEYWORD *keyword;
char *line_comment;
#else
void OdlSetKwdLineComment (KEYWORD *keyword, char *line_comment)
#endif
{
    char *str = {NULL};
    if (line_comment != NULL)
        CopyString( str, line_comment)
    LemmeGo( keyword->line_comment)
    keyword->line_comment = str;
    return;
}  /*  End:  "OdlSetKwdLineComment"  */

#ifdef _NO_PROTO
void OdlSetKwdFilename (keyword, filename)
KEYWORD *keyword;
char *filename;
#else
void OdlSetKwdFilename (KEYWORD *keyword, char *filename)
#endif
{
    char *str = {NULL};
    if (filename != NULL)
        CopyString( str, filename)
    LemmeGo( keyword->file_name)
    keyword->file_name = str;
    return;
}  /*  End:  "OdlSetKwdFilename"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetObjComments                                               */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      Steve Monk (Laboratory for Atmospheric and Space Physics)       */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    January 20, 1998                                         */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      01-20-98    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns the comment strings from an object struct. */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THESE ROUTINES.             */
/*                EACH OUTPUT VALUE IS A POINTER TO THE ACTUAL          */
/*                INFORMATION STORED IN THE ODL STRUCTURE AND MUST NOT  */
/*                BE FREED.                                             */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO
void OdlGetObjComments (object, pre_comment, line_comment, post_comment,
                        end_comment)
OBJDESC *object;
char **pre_comment;
char **line_comment;
char **post_comment;
char **end_comment;
#else
void OdlGetObjComments (OBJDESC *object, char **pre_comment, 
                        char **line_comment, char **post_comment,
                        char **end_comment)
#endif
{
    *pre_comment = OdlGetObjPreComment( object);
    *line_comment = OdlGetObjLineComment( object);
    *post_comment = OdlGetObjPostComment( object);
    *end_comment = OdlGetObjEndComment( object);
    return;
}  /*  End:  "OdlGetObjComments"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlSetObjComments                                               */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      Steve Monk (Laboratory for Atmospheric and Space Physics)       */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    January 20, 1998                                         */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      01-20-98    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine sets the comment strings of an object structure to */
/*      the corresponding input strings.  Each input string is copied   */
/*      before setting it as a new comment value.  The old comment      */
/*      strings are freed.  If an input string is NULL, the             */
/*      corresponding comment string in the object structure is set to  */
/*      NULL.                                                           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO
void OdlSetObjComments (object, pre_comment, line_comment, post_comment,
                        end_comment)
OBJDESC *object;
char *pre_comment;
char *line_comment;
char *post_comment;
char *end_comment;
#else
void OdlSetObjComments (OBJDESC *object, char *pre_comment, 
                        char *line_comment, char *post_comment,
                        char *end_comment)
#endif
{
    OdlSetObjPreComment( object, pre_comment);
    OdlSetObjLineComment( object, line_comment);
    OdlSetObjPostComment( object, post_comment);
    OdlSetObjEndComment( object, end_comment);
    return;
}  /*  End:  "OdlGetObjComments"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetObjDescChildCount 
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns a count of the immediate children of an    */
/*      object.  It does not count children of children, etc.           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

int OdlGetObjDescChildCount (object)

OBJDESC *object;

#else

int OdlGetObjDescChildCount (OBJDESC *object)

#endif
{
    OBJDESC *obj = {NULL};
    int child_count = {0};

    if (object != NULL)
    {
        for (obj=object->first_child; obj != NULL; obj=obj->right_sibling)
            ++child_count;
    }
    else
        odl_errno = 1;

    return(child_count);

}  /*  End:  "OdlGetObjDescChildCount"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetObjDescLevel                                              */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns the nesting level of an object.  The ROOT  */
/*      object in a tree is always defined to be level 0.               */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

int OdlGetObjDescLevel (object)

OBJDESC *object;

#else

int OdlGetObjDescLevel (OBJDESC *object)

#endif
{
    OBJDESC *obj = {NULL};
    int level = {0};

    if (object != NULL)
    {
        for (obj=object->parent; obj != NULL; obj=obj->parent)
            ++level;
    }
    else
        odl_errno = 1;

    return(level);

}  /*  End:  "OdlGetObjDescLevel"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlAdjustObjDescLevel                                           */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine changes the nesting level of an object and all of  */
/*      its subobjects so they fit in with their place in the overall   */
/*      ODL tree.  This is particularly useful when objects are cut     */
/*      from one tree and pasted into another tree, perhaps higher or   */
/*      lower in the nesting hierarchy then they were in the original   */
/*      tree.                                                           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

void OdlAdjustObjDescLevel (object)

OBJDESC *object;

#else

void OdlAdjustObjDescLevel (OBJDESC *object)

#endif
{
    OBJDESC *obj = {NULL};
    unsigned short scope = {ODL_RECURSIVE_DOWN};

    if (object == NULL) 
    {
        odl_errno = 1;
        return;
    }

    for (obj=object; obj != NULL; 
             obj = (OBJDESC *) OdlNextObjDesc(obj, object->level, &scope))
    {
        obj->level = (obj->parent == NULL) ? 0 : (1 + obj->parent->level);
    }

    return;

}  /*  End routine:  "OdlAdjustObjDescLevel"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetObjDescParent                                             */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns a pointer to an object's parent.           */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THIS ROUTINE.  THE RETURN   */
/*                VALUE IS A POINTER TO AN EXISTING ODL OBJECT AND      */
/*                AND MUST NOT BE FREED.                                */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlGetObjDescParent (object)

OBJDESC *object;

#else

OBJDESC *OdlGetObjDescParent (OBJDESC *object)

#endif
{
    OBJDESC *parent = {NULL};

    if (object != NULL)
        parent = object->parent;

    return(parent);

}  /*  End:  "OdlGetObjDescParent"  */




/*========================================================================*/
/*                                                                        */
/*                           Keyword routines                             */
/*                                                                        */
/*========================================================================*/


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlFindKwd                                                      */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates the keyword in a label that satisfies the  */
/*      requirements passed in:  The object where the search is to      */
/*      begin, the name of the keyword, a particular value that the     */
/*      keyword must have, which version of the keyword we want (if     */
/*      there are duplicates), and the search scope we want to use to   */
/*      limit the objects searched.                                     */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlFindKwd (start_object, keyword_name, keyword_value, 
                         keyword_position, search_scope)
OBJDESC *start_object;
char *keyword_name;
char *keyword_value;
unsigned long keyword_position;
unsigned short search_scope;

#else

KEYWORD *OdlFindKwd (OBJDESC *start_object, char *keyword_name, 
                     char *keyword_value, unsigned long keyword_position, 
                     unsigned short search_scope)

#endif
{
    OBJDESC *obj = {NULL};
    KEYWORD *kwd = {NULL};
    KEYWORD *found_kwd = {NULL};
    unsigned short found = {FALSE};
    unsigned short scope = {search_scope};
    unsigned long current_position = {0};

    if (start_object == NULL)
    {
        odl_errno = 1;
        return( NULL);
    }

    for (obj=start_object;
            ((obj != NULL) && (! found));
                obj = (OBJDESC *) OdlNextObjDesc(obj, start_object->level, &scope))
    {
        for (kwd=obj->first_keyword; ((kwd != NULL) && (! found)); kwd=kwd->right_sibling)
        {
            if (keyword_name == NULL)
                found = TRUE;
            else
                found = OdlWildCardCompare(keyword_name, kwd->name);

            if ((found) && (keyword_value != NULL))
                found = OdlWildCardCompare(keyword_value, (char *) OdlGetKwdValue(kwd));
    
            if ((found) && (keyword_position > 0))
                found = ((++current_position) == keyword_position);
    
            if (found) found_kwd = kwd;    

        }  /*  End:  "for (kwd=obj-> ..."  */

    }  /*  End:  "for (obj=start_object; ..."  */

    return(found_kwd);

}  /*  End:  "OdlFindKwd"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlNextKwd                                                      */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates the keyword in a label that satisfies the  */
/*      requirements passed in:  The object where the search is to      */
/*      begin, the name of the keyword, a particular value that the     */
/*      keyword must have, which version of the keyword we want (if     */
/*      there are duplicates), and the search scope we want to use to   */
/*      limit the objects searched.                                     */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlNextKwd (start_keyword, keyword_name, keyword_value, 
                     keyword_position, search_scope)

KEYWORD *start_keyword;
char *keyword_name;
char *keyword_value;
unsigned long keyword_position;
unsigned short search_scope;

#else

KEYWORD *OdlNextKwd (KEYWORD *start_keyword, char *keyword_name, 
                     char *keyword_value, unsigned long keyword_position, 
                     unsigned short search_scope)

#endif
{
    OBJDESC *start_object = {NULL};
    OBJDESC *obj = {NULL};
    KEYWORD *kwd = {NULL};
    KEYWORD *found_kwd = {NULL};
    unsigned short found = {FALSE};
    unsigned short scope = {search_scope};
    unsigned long current_position = {0};

    if (start_keyword != NULL)
    {
        start_object = start_keyword->parent;
        obj = start_object;
        kwd = start_keyword; 
    
        do
        {
            for ( ; ((kwd != NULL) && (! found)); kwd=kwd->right_sibling)
            {
                if (keyword_name == NULL)
                    found = TRUE;
                else
                    found = OdlWildCardCompare(keyword_name, kwd->name);
        
                if ((found) && (keyword_value != NULL))
                    found = OdlWildCardCompare(keyword_value, (char *) OdlGetKwdValue(kwd));
        
                if ((found) && (keyword_position > 0))
                    found = ((++current_position) == keyword_position);
        
                if (found) found_kwd = kwd;    
        
            }  /*  End:  "for (kwd=start_keyword; ..."  */
    
            if (! found)
            {
                obj = (OBJDESC *) OdlNextObjDesc(obj, start_object->level, &scope);
                kwd = (KEYWORD *) OdlGetFirstKwd(obj);
            }
    
        }  while ((obj != NULL) && (! found));

    }  /*  End:  "if (start_keyword != NULL) ..."  */
    else
        odl_errno = 1;

    return(found_kwd);

}  /*  End:  "OdlNextKwd"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlCutKwd                                                       */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine removes a keyword from an object and returns a     */
/*      pointer to it.  All references to the object within the keyword */
/*      are removed, and all references to the keyword within the       */
/*      object are removed.                                             */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THIS ROUTINE.  THE RETURN   */
/*                VALUE IS A POINTER TO AN EXISTING KEYWORD STRUCTURE   */
/*                AND MUST NOT BE FREED.                                */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlCutKwd (keyword)

KEYWORD *keyword;

#else

KEYWORD *OdlCutKwd (KEYWORD *keyword)

#endif
{
    if (keyword != NULL)
    {
        if (keyword->left_sibling != NULL)
            keyword->left_sibling->right_sibling = keyword->right_sibling;

        if (keyword->right_sibling != NULL)
            keyword->right_sibling->left_sibling = keyword->left_sibling;

        if (keyword->parent->first_keyword == keyword)
            keyword->parent->first_keyword = keyword->right_sibling;

        if (keyword->parent->last_keyword == keyword)
            keyword->parent->last_keyword = keyword->left_sibling;

        keyword->parent = NULL;
        keyword->left_sibling = NULL;
        keyword->right_sibling = NULL;

    }  /*  End:  "if ((keyword != NULL) && ..."  */
    else
        odl_errno = 1;

    return(keyword);

}  /*  End routine:  "OdlCutKwd"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPasteKwd                                                     */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine adds a keyword to the end of an object's keyword   */
/*      list.                                                           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlPasteKwd (keyword, object)

KEYWORD *keyword;
OBJDESC *object;

#else

KEYWORD *OdlPasteKwd (KEYWORD *keyword, OBJDESC *object)

#endif
{
    if ((keyword != NULL) && (object != NULL))
    {
        keyword->parent = object;
        keyword->left_sibling = object->last_keyword;
        keyword->right_sibling = NULL;

        if (object->first_keyword == NULL)
            object->first_keyword = keyword;

        if (object->last_keyword != NULL)
            object->last_keyword->right_sibling = keyword;

        object->last_keyword = keyword;

    }  /*  End:  "if ((keyword != NULL) && ..."  */
    else
        odl_errno = 1;

    return(keyword);

}  /*  End routine:  "OdlPasteKwd"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPasteKwdBefore 
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine adds a keyword to an object as the left sibling of */
/*      the old_keyword.                                                */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlPasteKwdBefore (new_keyword, old_keyword)

KEYWORD *new_keyword;
KEYWORD *old_keyword;

#else

KEYWORD *OdlPasteKwdBefore (KEYWORD *new_keyword, KEYWORD *old_keyword)

#endif
{
    if ((new_keyword != NULL) && (old_keyword != NULL))
    {
        new_keyword->parent = old_keyword->parent;
        new_keyword->left_sibling = old_keyword->left_sibling;
        new_keyword->right_sibling = old_keyword;

        if (old_keyword->left_sibling == NULL)
            old_keyword->parent->first_keyword = new_keyword;
        else
            old_keyword->left_sibling->right_sibling = new_keyword;

        old_keyword->left_sibling = new_keyword;

    }  /*  End:  "if ((new_keyword != NULL) && ..."  */
    else
        odl_errno = 1;

    return(new_keyword);

}  /*  End routine:  "OdlPasteKwdBefore"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPasteKwdAfter                                                */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine adds a keyword to an object as the right sibling   */
/*      of the old_keyword.                                             */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlPasteKwdAfter (new_keyword, old_keyword)

KEYWORD *new_keyword;
KEYWORD *old_keyword;

#else

KEYWORD *OdlPasteKwdAfter (KEYWORD *new_keyword, KEYWORD *old_keyword)

#endif
{
    if ((new_keyword != NULL) && (old_keyword != NULL))
    {
        new_keyword->parent = old_keyword->parent;
        new_keyword->right_sibling = old_keyword->right_sibling;
        new_keyword->left_sibling = old_keyword;

        if (old_keyword->right_sibling == NULL)
            old_keyword->parent->last_keyword = new_keyword;
        else
            old_keyword->right_sibling->left_sibling = new_keyword;

        old_keyword->right_sibling = new_keyword;

    }  /*  End:  "if ((new_keyword != NULL) && ..."  */
    else
        odl_errno = 1;

    return(new_keyword);

}  /*  End routine:  "OdlPasteKwdAfter"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlCopyKwd                                                      */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine makes a copy of a keyword and returns a pointer to */
/*      it.  All of the keyword's fields are duplicated except for      */
/*      references to the parent object, which are removed.             */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlCopyKwd (keyword)

KEYWORD *keyword;

#else

KEYWORD *OdlCopyKwd (KEYWORD *keyword)

#endif
{
    KEYWORD *new_keyword = {NULL};

    if (keyword != NULL)
    {
        new_keyword = OdlNewKwd(keyword->name, keyword->value, 
                                 keyword->pre_comment, keyword->line_comment,
                                 keyword->file_name, keyword->line_number);
    }
    else
        odl_errno = 1;

    return(new_keyword);

}  /*  End routine:  "OdlCopyKwd"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlNewKwd                                                       */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - Replaced 'malloc' by 'OaMalloc'.          */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine creates a new keyword structure, initializes       */
/*      its fields with the values passed in, and returns a pointer     */
/*      to it.                                                          */
/*                                                                      */      
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlNewKwd (keyword_name, value_text, pre_comment, 
                    line_comment, file_name, line_number)

char *keyword_name;
char *value_text;
char *pre_comment;
char *line_comment;
char *file_name;
long line_number;

#else

KEYWORD *OdlNewKwd (char *keyword_name, char *value_text, char *pre_comment, 
                    char *line_comment, char *file_name, long line_number)

#endif
{
    KEYWORD *new_keyword = {NULL};

    if ((new_keyword = (KEYWORD *)OaMalloc((long) sizeof(KEYWORD))) == NULL)
    {
        odl_errno = 50;
        SayGoodbye()
    }
    else
    {
        CopyString(new_keyword->name, keyword_name)
        CopyString(new_keyword->pre_comment, pre_comment)
        CopyString(new_keyword->line_comment, line_comment)
        CopyString(new_keyword->file_name, file_name)
        CopyString(new_keyword->value, value_text)

        new_keyword->is_a_pointer = (keyword_name == NULL) ? FALSE : (*keyword_name == '^');

        if (value_text == NULL) 
        {
            new_keyword->size = 0;
            new_keyword->is_a_list = FALSE;
        }
        else
        {
            new_keyword->size = (unsigned long) strlen(new_keyword->value);
            new_keyword->is_a_list = ((*value_text == '{') || (*value_text == '('));
        }

        new_keyword->line_number = line_number;
        new_keyword->parent = NULL;
        new_keyword->left_sibling = NULL;
        new_keyword->right_sibling = NULL;
        new_keyword->appl1 = NULL;
        new_keyword->appl2 = NULL;

    }  /*  End:  "if ((new_keyword = ... else ..."  */

    return(new_keyword);

}  /*  End routine:  "OdlNewKwd"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetFirstKwd                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns a pointer to the first keyword data        */
/*      structure in an object definition structure.                    */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THIS ROUTINE.  THE RETURN   */
/*                VALUE IS A POINTER TO THE ACTUAL INFORMATION STORED   */
/*                IN THE OBJECT DATA STRUCTURE AND MUST NOT BE FREED.   */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlGetFirstKwd (object)

OBJDESC *object;

#else

KEYWORD *OdlGetFirstKwd (OBJDESC *object)

#endif
{
    KEYWORD *kwd = {NULL};

    if (object != NULL)
        kwd = object->first_keyword;
    else
        odl_errno = 1;

    return(kwd);

}  /*  End:  "OdlGetFirstKwd"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetNextKwd                                                   */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns a pointer to the next keyword data         */
/*      structure in an object definition's list of keyword structures. */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THIS ROUTINE.  THE RETURN   */
/*                VALUE IS A POINTER TO THE ACTUAL INFORMATION STORED   */
/*                IN THE OBJECT DATA STRUCTURE AND MUST NOT BE FREED.   */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlGetNextKwd (keyword)

KEYWORD *keyword;

#else

KEYWORD *OdlGetNextKwd (KEYWORD *keyword)

#endif
{
    KEYWORD *kwd = {NULL};

    if (keyword != NULL)
        kwd = keyword->right_sibling;
    else
        odl_errno = 1;

    return(kwd);

}  /*  End:  "OdlGetNextKwd"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetKwdValue                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns a pointer to a keyword's value.            */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THIS ROUTINE.  THE RETURN   */
/*                VALUE IS A POINTER TO THE ACTUAL INFORMATION STORED   */
/*                IN THE KEYWORD DATA STRUCTURE AND MUST NOT BE FREED.  */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlGetKwdValue (keyword)

KEYWORD *keyword;

#else

char *OdlGetKwdValue (KEYWORD *keyword)

#endif
{
    char *value = {NULL};

    if (keyword != NULL)
        value = keyword->value;
    else
        odl_errno = 1;

    return(value);

}  /*  End:  "OdlGetKwdValue"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetAllKwdValues                                              */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine extracts each individual value from a set or       */
/*      sequence of values of a keyword, stores these values in a       */
/*      linked list, and returns a pointer to this list.                */
/*                                                                      */
/*      For example, if a keyword has this combination of sets and      */
/*      sequences as its value:                                         */
/*                                                                      */
/*         {red, (green, blue), {17, (("book.lbl", 345), orange)}}      */
/*                                                                      */
/*      Then the TB_STRING_LIST returned would contain:                 */
/*                                                                      */
/*         red                                                          */
/*         green                                                        */
/*         blue                                                         */
/*         17                                                           */
/*         "book.lbl"                                                   */
/*         345                                                          */
/*         orange                                                       */
/*                                                                      */
/*      WARNING:  The string list must be freed using the               */
/*                RemoveStringList macro (look in toolbox.h).           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

TB_STRING_LIST *OdlGetAllKwdValues (keyword)

KEYWORD *keyword;

#else

TB_STRING_LIST *OdlGetAllKwdValues (KEYWORD *keyword)

#endif
{
    TB_STRING_LIST *value_list = {NULL};
    char *val_start = {NULL};
    char *val_stop = {NULL};
    char save_ch;

    if (keyword != NULL)
    {
        if (keyword->value != NULL)
        {
            for (val_start=(char *)OdlValueStart(keyword->value);
                     *val_start != '\0'; 
                         val_start=(char *)OdlValueStart(val_stop+1))
            {
                val_stop = (char *) OdlValueEnd(val_start);
                save_ch = *(val_stop + 1); *(val_stop + 1) = '\0';
                AddStringToList(val_start, value_list)
                *(val_stop + 1) = save_ch;
            }

        }  /*  End:  "if (keyword->value != NULL) ..."  */

    }  /*  End:  "if (keyword != NULL) ..."  */
    else
        odl_errno = 1;

    return(value_list);

}  /*  End:  "OdlGetAllKwdValues"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetKwdValueType                                              */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine determines the data type of a keyword's value and  */
/*      returns a data type symbolic id.  Possible symbolic ids are:    */
/*                                                                      */
/*         ODL_UNKNOWN   (can't tell what the heck it is)               */
/*         ODL_INTEGER   (handles optional leading plus or minus)       */
/*         ODL_REAL      (handles optional leading plus or minus,       */
/*                        scientific notation, and real exponents)      */
/*         ODL_SYMBOL    (unqouted or single quoted string of           */
/*                        characters)                                   */
/*         ODL_TEXT      (double quoted string of characters)           */
/*         ODL_DATE      (yyyy-mm-dd or yyyy-ddd)                       */
/*         ODL_DATE_TIME (yyyy-mm-ddThh:mm:ss.h)                        */
/*         ODL_SEQUENCE  (starts with a paren character "(")            */
/*         ODL_SET       (starts with a brace character "{")            */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

unsigned short OdlGetKwdValueType (keyword)

KEYWORD *keyword;

#else

unsigned short OdlGetKwdValueType (KEYWORD *keyword)

#endif
{
    unsigned short type = {ODL_UNKNOWN};
    if (keyword != NULL)
         type = (unsigned short) OdlDataType(keyword->value);
    else
         odl_errno = 1;

    return(type);

}  /*  End:  "OdlGetKwdValueType"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetKwdUnit                                                   */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates the units part of a keyword's value,       */
/*      extracts it, stores it in a new character string, and returns   */
/*      a pointer to this new character string.                         */
/*                                                                      */
/*      WARNING:  This routine allocates memory for the return value    */
/*                that must be freed.                                   */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlGetKwdUnit (keyword)

KEYWORD *keyword;

#else

char *OdlGetKwdUnit (KEYWORD *keyword)

#endif
{
    char *c = {NULL};
    char *unit = {NULL};

    /*  If we were given a keyword to use  */
    if (keyword != NULL)
    {
        /*  Attempt to locate the units string  */
        c = (char *) strchr(keyword->value, '<');

        if (c != NULL)
        {
            /*  We found it!  Now copy it and make it upper case  */
            CopyString(unit, c)
            UpperCase(unit)
    
            /*  Close off the units string  */
            c = (char *) strchr(unit, '>');
            if (c != NULL) *(c + 1) = '\0';
        }

    }  /*  End:  "if (keyword != NULL) ..."  */
    else
        odl_errno = 1;

    return(unit);

}  /*  End:  "OdlGetKwdUnit"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetKwdName                                                   */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns the name of a keyword.                     */
/*                                                                      */
/*      WARNING:  NO MEMORY IS ALLOCATED BY THIS ROUTINE.  THE RETURN   */
/*                VALUE IS A POINTER TO THE ACTUAL INFORMATION STORED   */
/*                IN THE KEYWORD DATA STRUCTURE AND MUST NOT BE FREED.  */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlGetKwdName (keyword)

KEYWORD *keyword;

#else

char *OdlGetKwdName (KEYWORD *keyword)

#endif
{
    char *name = {NULL};

    if (keyword != NULL)
        name = keyword->name;
    else
        odl_errno = 1;

    return(name);

}  /*  End:  "OdlGetKwdName"  */





/*========================================================================*/
/*                                                                        */
/*                    Memory deallocation routines                        */
/*                                                                        */
/*========================================================================*/

/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlFreeTree                                                     */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine frees all memory used by an ODL tree. The return   */
/*      value is always NULL.                                           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlFreeTree (object)

OBJDESC *object;

#else

OBJDESC *OdlFreeTree (OBJDESC *object)

#endif
{
    if (object != NULL)
    {
        OdlFreeTree(object->first_child);
        OdlFreeTree(object->right_sibling);
        OdlFreeAllKwds(object);
        LemmeGo(object->class_name)      
        LemmeGo(object->pre_comment)
        LemmeGo(object->line_comment)
        LemmeGo(object->post_comment)
        LemmeGo(object->end_comment)
        LemmeGo(object->file_name)
        LemmeGo(object)
    }               
    else
        odl_errno = 1;
                    
    return(object);
                    
}  /*  End:  "OdlFreeTree"  */
                    
        


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlFreeAllKwds                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine frees all memory used by an object's keywords.     */
/*      When it's finished, all references to keywords are gone from    */
/*      the object.  The return value is always NULL.                   */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlFreeAllKwds (object)

OBJDESC *object;

#else

KEYWORD *OdlFreeAllKwds (OBJDESC *object)

#endif
{
    KEYWORD *kwd = {NULL};

    if (object != NULL)
    {
        for (kwd=object->first_keyword; kwd != NULL; 
                    kwd=(KEYWORD *) OdlFreeKwd(kwd)) ;

        object->first_keyword = NULL;
        object->last_keyword = NULL;
    }               
    else
        odl_errno = 1;                    

    return(kwd);
                    
}  /*  End:  "OdlFreeAllKwds"  */
                    
        



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlFreeKwd                                                      */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine frees the memory used by a keyword.  The return    */
/*      value is always a pointer to the right sibling of the keyword.  */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

KEYWORD *OdlFreeKwd (keyword)

KEYWORD *keyword;

#else

KEYWORD *OdlFreeKwd (KEYWORD *keyword)

#endif
{
    KEYWORD *next_kwd = {NULL};

    if (keyword != NULL)
    {
        next_kwd = keyword->right_sibling;
        LemmeGo(keyword->name)        
        LemmeGo(keyword->file_name)   
        LemmeGo(keyword->value)       
        LemmeGo(keyword->pre_comment) 
        LemmeGo(keyword->line_comment)            
        LemmeGo(keyword)
    }               
    else
        odl_errno = 1;
                    
    return(next_kwd);
                    
}  /*  End:  "OdlFreeKwd"  */
                    
        


/*========================================================================*/
/*                                                                        */
/*                    File and File Name routines                         */
/*                                                                        */
/*========================================================================*/

/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlOpenMessageFile                                              */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns a pointer to an opened file based on       */
/*      what is passed in.  If message_fptr is not NULL, the we assume  */
/*      that the file is already open and return message_fptr.  If      */
/*      message_fname is NULL, or we can't open message_fname, then     */
/*      we return stdout.  If message_fname can be opened, then we      */
/*      return a pointer to the newly opened file.                      */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

FILE *OdlOpenMessageFile (message_fname, message_fptr)

char *message_fname;
FILE *message_fptr;

#else

FILE *OdlOpenMessageFile (char *message_fname, FILE *message_fptr)

#endif
{
    FILE *fptr = {stdout};

    if (message_fptr != NULL)
        fptr = message_fptr;
    else
        if (message_fname != NULL && ! odl_suppress_messages)
        {
            if ((fptr = (FILE *) fopen(message_fname, "a")) == NULL)
            {
                fptr = stdout;
                odl_errno = 200;
                OdlPrintMessage(NULL, NULL, 0,
                    "Unable to open the output file.  Messages will be written to the terminal");
            }
        }

    return(fptr);

}  /*  End routine:  "OdlOpenMessageFile"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetFileName                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      05-19-96    Modified parsing to allow Macintosh-style paths     */
/*                  with spaces or other non-ASCII chars.  SM           */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine extracts the file name from a "^" keyword,         */
/*      allocates storage for it, and returns a pointer to this new     */
/*      character string.  It also returns information about where      */
/*      the data actually begins in the file.                           */
/*                                                                      */
/*      For example, lets say we're looking at a label in a file        */
/*      called test.lbl, and we want to get the file name assocated     */
/*      the FNAME keyword.  Here are the possible values this keyword   */
/*      might have, and what information would be returned for each     */
/*      possibility:                                                    */
/*                                                                      */
/*            ^FNAME = 17                                               */
/*                                                                      */
/*               file name            : test.lbl (attached)             */
/*               *start_location      : 17                              */
/*               *start_location type : ODL_RECORD_LOCATION             */
/*                                                                      */
/*          ^FNAME = 29 <RECORD>                                        */
/*                                                                      */
/*             file name            : test.lbl (attached)               */
/*             *start_location      : 29                                */
/*             *start_location type : ODL_RECORD_LOCATION               */
/*                                                                      */
/*          ^FNAME = 197 <RECORDS>                                      */
/*                                                                      */
/*             file name            : test.lbl (attached)               */
/*             *start_location      : 197                               */
/*             *start_location type : ODL_RECORD_LOCATION               */
/*                                                                      */
/*          ^FNAME = 346 <BYTE>                                         */
/*                                                                      */
/*             file name            : test.lbl (attached)               */
/*             *start_location      : 346                               */
/*             *start_location type : ODL_BYTE_LOCATION                 */
/*                                                                      */
/*          ^FNAME = 2189 <BYTES>                                       */
/*                                                                      */
/*             file name            : test.lbl (detached)               */
/*             *start_location      : 2189                              */
/*             *start_location type : ODL_BYTE_LOCATION                 */
/*                                                                      */
/*          ^FNAME = "file_name.dat"                                    */
/*                                                                      */
/*             file name            : file_name.dat (detached)          */
/*             *start_location      : 1                                 */
/*             *start_location type : ODL_RECORD_LOCATION               */
/*                                                                      */
/*          ^FNAME = ("file_name.dat", 17)                              */
/*                                                                      */
/*             file name            : file_name.dat (detached)          */
/*             *start_location      : 17                                */
/*             *start_location type : ODL_RECORD_LOCATION               */
/*                                                                      */
/*          ^FNAME = ("file_name.dat", 29 <RECORD>)                     */
/*                                                                      */
/*             file name            : file_name.dat (detached)          */
/*             *start_location      : 29                                */
/*             *start_location type : ODL_RECORD_LOCATION               */
/*                                                                      */
/*          ^FNAME = ("file_name.dat", 197 <RECORDS>)                   */
/*                                                                      */
/*             file name            : file_name.dat (detached)          */
/*             *start_location      : 197                               */
/*             *start_location type : ODL_RECORD_LOCATION               */
/*                                                                      */
/*          ^FNAME = ("file_name.dat", 346 <BYTE>)                      */
/*                                                                      */
/*             file name            : file_name.dat (detached)          */
/*             *start_location      : 346                               */
/*             *start_location type : ODL_BYTE_LOCATION                 */
/*                                                                      */
/*          ^FNAME = ("file_name.dat", 2189 <BYTES>)                    */
/*                                                                      */
/*             file name            : file_name.dat (detached)          */
/*             *start_location      : 2189                              */
/*             *start_location type : ODL_BYTE_LOCATION                 */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlGetFileName (keyword, start_location, start_location_type)

KEYWORD *keyword;
unsigned long *start_location;
unsigned short *start_location_type;

#else

char *OdlGetFileName (KEYWORD *keyword, unsigned long *start_location, 
                      unsigned short *start_location_type)

#endif
{
    char *fname = {NULL};
    char *text = {NULL};
    char *unit = {NULL};
    char *first_word = {NULL};
    char *second_word = {NULL};

    if (keyword != NULL)
    {
        /*  Make a copy of the keyword's value  */
        CopyString(text, keyword->value)
    
        /*  Get rid of parens, braces, and commas  */
        ReplaceChar(text, '(', ' ')
        ReplaceChar(text, ')', ' ')
        ReplaceChar(text, '{', ' ')
        ReplaceChar(text, '}', ' ')
        ReplaceChar(text, ',', ' ')
    
        /*  Locate the units string  */
        unit = (char *) strchr(text, '<');
    
        /*  Remove the units string if it's there  */
        if (unit != NULL) *unit = '\0';
    
        /*  Find the first word  */
        first_word = (char *) OdlFirstWord(text);
    
        /*  If the first word is quoted, then it's a file name  */
        if ((*first_word == '"') || (*first_word == '\''))
        {
            /*  Look for a second word after the close quote of the first word,
                so that Macintosh-style file/directory names with spaces or
                other non-ASCII characters aren't mistaken for the end of 
                first_word.  */
                
            second_word = (char *) OdlNextWord( strchr( first_word+1, *first_word));
    
            /*  If we can't find one, then the location is record 1  */
            if (*second_word == '\0')
                *start_location = 1;
            else
            {
                /*  Otherwise, the second word is the location  */
                *start_location = atoi(second_word);
                *(second_word - 1) = '\0';
            }
    
            /*  Copy and clean up the file name  */
            CopyString(fname, (first_word+1))
            ReplaceChar(fname, '"', ' ');
            ReplaceChar(fname, '\'', ' ');
            StripTrailing(fname, ' ')
        }
        else
        {
            /*  Since the first word isn't quoted, we assume that it's a     */
            /*  location, and that the file name is the one associated with  */
            /*  the keyword itself (e.g., we're looking at attached data)    */
            *start_location = atoi(first_word);
            CopyString(fname, keyword->file_name)

        }  /*  End:  "if ((*first_word == '"') || ... else ..."  */
    
        /*  No unit string means a record location  */
        if (unit == NULL)
            *start_location_type = ODL_RECORD_LOCATION;
        else
        {
            /*  Otherwise, find out what kind of units string we have  */
            UpperCase(unit)
            *unit = '<';  /* Bug fix SM 10/24/94 */
            if (strncmp(unit, "<BYTE", 5) == 0)
                *start_location_type = ODL_BYTE_LOCATION;
            else
                *start_location_type = ODL_RECORD_LOCATION;
        }
    
        LemmeGo(text)

    }  /*  End:  "if (keyword != NULL) ..."  */
    else
        odl_errno = 1;

    return(fname);

}  /*  End:  "OdlGetFileName"  */




/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlGetFileSpec                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath / Steve Monk                                   */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - Changed input parameters and wrote the    */
/*                            actual code; was originally a 5-line stub.*/
/*                            Also wrote OdlCheckFileSpec.              */
/*      02-07-03    DWS       Original code found first slash and       */
/*                            appended LABEL or CATALOG after it.       */
/*			      Change was to find last slash and delete  */
/*			      it and then delete the directory befor it */
/*			      and then add LABEL or CATALOG to the end  */
/*			      of the string.                            */
/*                                                                      */
/*     03-17-03    MDC	      Modified code to append a '/' instead of  */
/*			      a '\' so that it properly creates a path  */
/*			      for UNIX systems. Also modified the code  */
/*		       	      to lowercase LABEL or CATALOG if it could */
/*			      not find the file using uppercase LABEL   */
/*			      or CATALOG (for UNIX systems).  		*/
/*    06-28-03    MDC         Changed a call to strstr.                 */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine checks for the existence of a file, specifically a */
/*      ^STRUCTURE or ^CATALOG include file, by following the rules for */
/*      resolving pointer statements, as documented on pg 14-3 in the   */
/*      August 1994 PDS Standards Document.                             */
/*                                                                      */
/*      WARNING:  This routine allocates memory to hold the file spec   */
/*                that is returned.  This memory will have to be freed. */
/*                                                                      */
/*      Example (UNIX platform):                                        */
/*      img.lbl contains the line:   ^STRUCTURE = MY_FILE.FMT           */
/*      label_spec = "/device/vol/dir/img.lbl"                          */
/*      filename = "MY_FILE.FMT"                                        */
/*      The code will look for the following file specs:                */
/*                                                                      */
/*      MY_FILE.FMT                                                     */
/*      my_file.fmt                                                     */
/*      MY_FILE.FMT;1                                                   */
/*      my_file.fmt;1                                                   */
/*      /device/vol/dir/MY_FILE.FMT                                     */
/*      /device/vol/dir/my_file.fmt                                     */
/*      /device/vol/dir/MY_FILE.FMT;1                                   */
/*      /device/vol/dir/my_file.fmt;1                                   */
/*      and:                                                            */
/*      /device/vol/DIR/MY_FILE.FMT                                     */
/*      /device/vol/DIR/my_file.fmt                                     */
/*      /device/vol/DIR/MY_FILE.FMT;1                                   */
/*      /device/vol/DIR/my_file.fmt;1                                   */
/*      where DIR = LABEL if expand_type ends with "LABEL", or "CATALOG"*/
/*      if expand_type ends with "CATALOG".                             */
/*                                                                      */
/*      If a full path name is not given in label_spec, then the code   */
/*      can't be guaranteed to work;  it does handle the case where the */
/*      user's default directory is already set to /device/vol, and     */
/*      label_spec = "dir/img.lbl".  In this case the code looks for:   */
/*                                                                      */
/*      MY_FILE.FMT                                                     */
/*      my_file.fmt                                                     */
/*      MY_FILE.FMT;1                                                   */
/*      my_file.fmt;1                                                   */
/*      dir/MY_FILE.FMT                                                 */
/*      dir/my_file.fmt                                                 */
/*      dir/MY_FILE.FMT;1                                               */
/*      dir/my_file.fmt;1                                               */
/*      DIR/MY_FILE.FMT                                                 */
/*      DIR/my_file.fmt                                                 */
/*      DIR/MY_FILE.FMT;1                                               */
/*      DIR/my_file.fmt;1                                               */
/*                                                                      */
/*      Note that it is not a good idea to set default to a CD-ROM,     */
/*      because OdlWriteMessage, called often when parsing a label,     */
/*      will not be able to open a message file (assuming that's what   */
/*      the default routing is set to) and messages will be re-routed   */
/*      to standard output, e.g. your screen.                           */
/*                                                                      */
/*      Similar file specs are looked for on other platforms, with      */
/*      the path delimitors changed accordingly.                        */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlGetFileSpec (filename, label_spec, expand_type)

char *filename;
char *label_spec;
char *expand_type;

#else

char *OdlGetFileSpec (char *filename, char *label_spec, char *expand_type)

#endif
{

    char *filespec = {NULL};
    char *ptr = {NULL};
    char *expand_dir_name = {NULL};
	char *label_dir = NULL;
	char *test_dir = NULL;
	char *temp = NULL;
	int goUp = 5;
	int i = 0;


    if (filename == NULL)
    {
        odl_errno = 1;
        return( NULL);
    }

    /* Try the filename as is.  */
    if ((filespec = OdlCheckFileSpec( NULL, filename)) != NULL)
        return( filespec);

    /* Check if the label_spec has a path component before the filename.  */
    CopyString(label_dir, label_spec);
    ptr = OdlExtractFilename(label_dir);
    if (ptr != label_dir)  /* There's a directory before the label filename */
    {                      /* so erase the filename part of the path.       */
        *ptr = '\0';

        /* Try filename appended to the directory component of the label path. */
        if ((filespec = OdlCheckFileSpec(label_dir, filename)) != NULL)
            return( filespec);

        /* If expand_type ends in "LABEL" or "CATALOG", try looking in the LABEL
           (or CATALOG) directory if a device (disk) name or VMS logical is part
           of the path.

           Under UNIX, look for three slashes in something like: 
           "/device/vol/dir/img.lbl", and replace it by "/device/vol/LABEL/"
           If not found, assume user's default directory is set to the root
           of the device, and replace by "LABEL/".

           Under VMS, look for the colon/left bracket pair in something like:
           "diska:[my_dir]img.lbl"  or  "my_logical:[my_dir]img.lbl", and 
           replace it by "diska:[LABEL]" or "my_logical:[LABEL]".  
           If not found, assume user's default directory is set and replace
           by "[LABEL]".
           Note: my_logical:img.lbl won't work.

           Under Macintosh, look for a space, as in "VG_0001 dir img.lbl"
           and replace by "VG_0001 LABEL ".

           Under DOS or Windows, look for the colon/back-slash in:
           C:\DIR\IMG.LBL, and replace it by C:\LABEL\.
        */

        if (expand_type != NULL)
        {
            expand_dir_name = (char *) strstr( expand_type, "STRUCTURE");        
            if (expand_dir_name != NULL)
                expand_dir_name = "LABEL";
            else
                expand_dir_name = "CATALOG";

/* Keep this conditional statement for VAX just in case...*/
#if (defined( VAX) || defined( ALPHA_VMS))
			char tempBuf[256]={0};
            strcpy(tempBuf, label_spec);
            ptr = (char *) strstr(tempBuf, ":[");
            if (ptr != NULL)
            {
                *ptr = '\0';
                sprintf( tempBuf + strlen(tempBuf), ":[%s]", 
                         expand_dir_name);
            }
            else
                sprintf(tempBuf, "[%s]", expand_dir_name);

            if (strlen(tempBuf) > (size_t) 0) {
                if ((filespec = OdlCheckFileSpec( label_dir, filename)) != NULL)
      				return( filespec);
			}
			odl_errno = 200;
			return(NULL);
#endif

			for(i=0; ((i < goUp) && (label_dir != NULL)); i++) {
				 temp = OdlGetParentDir(label_dir);
				 LemmeGo(label_dir);
				 label_dir = temp;

				if(label_dir != NULL) {
                    CopyString(test_dir, label_dir);
					AppendString(test_dir, expand_dir_name);
			        AppendString(test_dir, pathSep);

					if ((filespec = OdlCheckFileSpec(test_dir, filename)) != NULL) {
						LemmeGo(label_dir);
						LemmeGo(test_dir);
      					return( filespec);
					}
				/*------------------------------------------------------*/
				/*For UNIX, lowercase LABEL or CATALOG in the path name */
				/*and perform the search again.		                    */
				/*------------------------------------------------------*/
#if (defined(SUN_UNIX) || defined(LINUX))  
					ptr = (char *) strstr(test_dir, expand_dir_name);
					if(ptr != NULL) {
						LowerCase(ptr);
						if ((filespec = OdlCheckFileSpec(test_dir, filename)) != NULL) {
							LemmeGo(label_dir);
							LemmeGo(test_dir);
                            return( filespec);
						}
					}
#endif
					LemmeGo(test_dir);
				}
			} /* End for loop */
		} /* End if(expand_type != NULL) loop */
	}
	LemmeGo(label_dir);
    odl_errno = 200;
    return( NULL);

}  /*  End:  "OdlGetFileSpec"  */


char *OdlGetParentDir(char *dir)
{
	char *parent = NULL;
	char *lastChar = NULL;
	char *dirCopy = NULL;
	char *ptr = NULL;

	if(dir == NULL) {
		return NULL;
	}
	else {
        CopyString(dirCopy, dir);
	}

	/* First strip out any trailing path separator	*/
	StripTrailing(dirCopy,*pathSep);
	ptr = strrchr(dirCopy, *pathSep);
	if(ptr != NULL) {
		++ptr;
		*ptr = NULL;
		parent = dirCopy;
	}

	return parent;
}

/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

char *OdlCheckFileSpec( vol_dir, filename)

char *vol_dir;
char *filename;

#else

char *OdlCheckFileSpec( char *vol_dir, char *filename)

#endif
{
    char path[256];
    FILE *fd = {NULL};
    char *filespec = {NULL};
    char *ptr = {NULL};

    if (vol_dir == NULL)
    {
        strcpy( path, filename);
        ptr = path;
    }
    else
    {
        sprintf( path, "%s%s", vol_dir, filename);
        ptr = path + strlen( vol_dir);
    }

    /* Try the path/filename as is.  */
    if ((fd = fopen( path, "r")) != NULL)
    {
        fclose( fd);
        CopyString(filespec, path);
        return(filespec);
    }

    /* Try lower-case.  */
    LowerCase( ptr);

    if ((fd = fopen( path, "r")) != NULL)
    {
        fclose( fd);
        CopyString(filespec, path);
        return(filespec);
    }

    /* Try appending ";1".  A typical discrepancy on CD-ROM's is that the
        actual filename has a ";1" extension, but the label has the filename
        without the extension.  */
    strcpy( ptr, filename);
    if (strstr( ptr, ";1") == NULL)
    {
        strcat( ptr, ";1");

        if ((fd = fopen( path, "r")) != NULL)
        {
            fclose( fd);
            CopyString(filespec, path);
            return(filespec);
        }

        /* Try ";1" with lower-case.  */
        LowerCase( ptr);

        if ((fd = fopen( path, "r")) != NULL)
        {
            fclose( fd);
            CopyString(filespec, path);
            return(filespec);
        }
    }

    return( NULL);

}  /*  End:  "OdlCheckFileSpec"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlExtractFilename                                              */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      Steve Monk (University of Colorado LASP)                        */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      06-10-97    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine finds the beginning of the file name in a path.    */
/*      The path may contain volume and directory information, or a     */
/*      VMS logical.  It returns a pointer to the start of the filename */
/*      part of the path (which may be the beginning of the path string,*/
/*      if it's just a file name).                                      */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlExtractFilename (path)

char *path;

#else

char *OdlExtractFilename (char *path)

#endif
{
    char *ptr;

    /* Look for the last character of a directory specification or VMS logical. */

    ptr = NULL;
    ptr = strrchr( path, '/');
#if (defined( VAX) || defined( ALPHA_VMS))
    ptr = strrchr( path, ']');                   /* Dir name?     */
    if (ptr == NULL) ptr = strrchr( path, ':');  /* Logical name? */
#endif
#ifdef IBM_PC
    ptr = strrchr( path, '\\');                   /* Dir name?     */
    if (ptr == NULL) ptr = strrchr( path, '/');   /* Alternate way.*/
#endif
#ifdef MAC
    ptr = strrchr( path, ':');
#endif
    if (ptr != NULL) 
      ptr++;
    else
      ptr = path;
    return( ptr);
}  /*  End:  "OdlExtractFilename"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlLocateStart                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine opens the filespec passed in, and attempts to      */
/*      find the start of the data by using the start_location and      */
/*      location_type that were passed in.  It returns a pointer to     */
/*      the start of the data within the file.                          */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

FILE *OdlLocateStart (filespec, start_location, start_location_type)

char *filespec;
unsigned long start_location;
unsigned short start_location_type;

#else

FILE *OdlLocateStart (char *filespec, unsigned long start_location, 
                      unsigned short start_location_type)

#endif
{
    FILE *fptr = {NULL};
    unsigned short reached_the_end = {FALSE};
    char buffer [TB_MAX_BUFFER];  /* Bug fix 11/2/94 SM:                     */
                                  /* Was TB_MAX_BUFFER + 1, which won't      */
                                  /* compile on platforms with 2-byte ints,  */
                                  /* because it's 1 bigger than than MAX_INT.*/
    unsigned long i;

    if (filespec != NULL)
    {
        if (start_location_type == ODL_BYTE_LOCATION)
        {
            fptr = (FILE *) fopen(filespec, "rb");
            if ((fptr != NULL) && (start_location > 1))
                reached_the_end = (fseek(fptr,start_location,0) != 0);
        }
        else
        {
            fptr = (FILE *) fopen(filespec, "r");
            if (fptr != NULL)
            {
                for (i=1; ((i < start_location) && (! reached_the_end)); ++i)
                {
                    if (! fgets(buffer, TB_MAX_BUFFER, fptr))
                        reached_the_end = TRUE;
                }
            }

        }  /*  End:  "if (start_location_type == ... else ..."  */
 
        if (reached_the_end) 
        {
            odl_errno = 200;
            CloseMe(fptr)
        }

    }  /*  End:  "if (filespec != NULL) ..."  */
    else
        odl_errno = 1;

    return(fptr);

}  /*  End:  "OdlGetFileSpec"  */






/*========================================================================*/
/*                                                                        */
/*                            Print Routines                              */
/*                                                                        */
/*========================================================================*/

/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPrintMessage                                                 */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine prints a formatted message either to stdout or to  */
/*      a message file, depending on what was passed in.  Messages are  */
/*      formatted to look like this:                                    */
/*                                                                      */
/*         <line number> -- <message text>                              */
/*                                                                      */
/*      If the line_number is zero, then just the message text is       */
/*      printed, with no formatting.                                    */
/*                                                                      */
/*      If the odl_suppress_messages global flag is set to TRUE (1)     */
/*      then nothing is printed.                                        */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

short OdlPrintMessage (message_fname, message_fptr, line_number, text)

char *message_fname;
FILE *message_fptr;
long line_number;
char *text;

#else

short OdlPrintMessage (char *message_fname, FILE *message_fptr, 
                       long line_number, char *text)

#endif
{
    FILE *m_ptr = {NULL};
    char line_prompt[20];
    char *line_out = {NULL};

    ++odl_message_count;

    if (! odl_suppress_messages)
    {
        m_ptr = (message_fptr != NULL) ? message_fptr :
                                         (FILE *) OdlOpenMessageFile(message_fname, message_fptr);
    
        if (line_number == 0)
            strcpy(line_prompt, "");
        else
            sprintf(line_prompt, " Line %d -- ", line_number);
    
        if (text == NULL)
        {
            NewString(line_out, (20 + strlen(line_prompt)))
            odl_errno = 1;
            sprintf(line_out, "%s Unknown error", line_prompt);
        }
        else
        {
            NewString(line_out, (20 + strlen(line_prompt) + strlen(text)))
            sprintf(line_out, "%s%s", line_prompt, text);
        }
    
        line_out = OdlFormatMessage(line_out);
        fprintf(m_ptr, "%s", line_out);
        LemmeGo(line_out)
    
        /*  if we opened the message file in this routine then close it  */
        if ((m_ptr != stdout) && (message_fptr == NULL))
            CloseMe(m_ptr)

    }  /*  End:  "if (! odl_suppress_messages) ..."  */
    
    return(FALSE);

}  /*  End routine:  OdlPrintMessage  */

/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPrintLine                                                    */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      J. S. Hughes (Jet Propulsion Laboratory)                        */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    February 20, 1995                                        */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      02-20-95    jsh - a copy of OdlPrintMessage cleaned up for      */
/*                  simple output of lines to message file              */
/*                  NOTE: '\n' is not appended or assumed               */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine prints a simple  line either to stdout or to       */
/*      a message file, depending on what was passed in.                */
/*                                                                      */
/*      If the odl_suppress_messages global flag is set to TRUE (1)     */
/*      then nothing is printed.                                        */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

short OdlPrintLine (message_fname, message_fptr, text)

char *message_fname;
FILE *message_fptr;
char *text;

#else

short OdlPrintLine (char *message_fname, FILE *message_fptr, 
                       char *text)

#endif
{
    FILE *m_ptr = {NULL};

    if (! odl_suppress_messages)
    {
        m_ptr = (message_fptr != NULL) ? message_fptr :
                                         (FILE *) OdlOpenMessageFile(message_fname, message_fptr);
    
        if (text == NULL)
        {
            odl_errno = 1;
            fprintf(m_ptr, "%s", "Unknown error\n");
        }
        else
        {
            fprintf(m_ptr, "%s", text);
        }
    
        /*  if we opened the message file in this routine then close it  */
        if ((m_ptr != stdout) && (message_fptr == NULL))
            CloseMe(m_ptr)

    }  /*  End:  "if (! odl_suppress_messages) ..."  */
    
    return(FALSE);

}  /*  End routine:  OdlPrintLine  */


/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static char *OdlFormatMessage (text)
                                                      
char *text;

#else

static char *OdlFormatMessage (char *text)

#endif
{
    char *new_text = {NULL};
    char *first_char = {NULL};
    char *last_char = {NULL};
    char *dashes = {NULL};
    char *blanks = {NULL};
    char *c = {NULL};
    char save_it = {'\0'};
    long report_indent = {0};
    long report_width = {75};
    long line_size = {0};
    long len = {0};
    long n_chars = {0};
    long i = {0};

    /* IF a message was passed in THEN                                     */
    if (text != NULL)
    {
        n_chars = 1;
        NewString(new_text, n_chars)

        /* Find the double dash delimiter thingy in the message.           */
        /*     Messages will look something like this:                     */
        /*         WARNING: Line 123 -- BANDS: Not in data dictionary.     */
        /*     We are using the location of the " -- " characters to       */
        /*     figure out how far the wrapped part of the line should      */
        /*     be indented.                                                */

        if ((dashes = strstr(text, " -- ")) != NULL)
            report_indent = 4 + ((long) (dashes - text));

        if (report_indent >= (report_width - 2))
            report_indent = 0;

        /* Initialize the string of blanks used for indentation.           */
        NewString(blanks, report_indent + 1)
        for (i=0; i < report_indent; ++i)
            *(blanks+i) = ' ';
        *(blanks+i) = '\0';

        /* Figure out the size of the wrapped parts of the line.           */
        line_size = report_width - report_indent;

        /* Now that we have all that out of the way, we can LOOP through   */
        /*         the string until we have wrapped and written the        */
        /*         whole thing.                                            */
        for (first_char=text; *first_char != '\0'; first_char = last_char)
        {
            /* Find the length of the remaining part of the string.        */
            len = (long) strlen(first_char);

            /* IF we are at the beginning of the string THEN               */
            /*     Use the total width of the report to figure out where   */
            /*         the end of the line should be.                      */
            /* ELSE                                                        */
            /*     Write the blanks to the report file and use the space   */
            /*         left over after indentation to figure out where     */
            /*         the end of the line should be.                      */
            /* ENDIF                                                       */

            if (first_char == text) 
            {
                if (len > report_width)
                    last_char = (char *) (first_char + report_width);
                else
                    last_char = (char *) (first_char + len);
            }
            else
            {
                AppendString(new_text, blanks)

                if (len > line_size)
                    last_char = (char *) (first_char + line_size);
                else
                    last_char = (char *) (first_char + len);

            }  /*  End:  "if (first_char == text) ... else ..."  */

            /* IF the current part of the message is still too large to    */
            /*        fit without wrapping THEN                            */
            /*     Find the last blank in the line and wrap there.         */
            /* ENDIF                                                       */

            if (*last_char != '\0')
            {
                for (c = last_char; ((c >= first_char) && (*c != ' ')); --c) ;
                           
                if (c > first_char)
                    last_char = c;

            }  /*  End: "if (*last_char != '\0') ..."  */

            /* Append the current part of the message onto the new string  */
            save_it = *last_char;
            *last_char = '\0';
            AppendString(new_text, first_char)
            AppendString(new_text, "\n")
            *last_char = save_it;

            /* Bypass the last blank character.                            */
            if (*last_char == ' ')
                ++last_char;

        }  /*  End:  "for (first_char = text; ..."  */

        /* Deallocate local storage.                                       */
        LemmeGo(blanks)

    }  /*  End:  "if ((text != NULL) && ..."  */
    else
        odl_errno = 1;
       
    LemmeGo(text)

    return(new_text);

}  /*  End routine:  "OdlFormatMessage"  */




/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPrintHierarchy                                               */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine prints the object hierarchy to a message file.     */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

void OdlPrintHierarchy (object, message_fname, message_fptr)

OBJDESC *object;
char *message_fname;
FILE *message_fptr;

#else

void OdlPrintHierarchy (OBJDESC *object, char *message_fname, 
                        FILE *message_fptr)

#endif
{
    OBJDESC *obj = {NULL};
    KEYWORD *kwd = {NULL};
    FILE *m_ptr = {NULL};
    char *format = {NULL};
    char *no_name = {"<no name>"};
    char msgtext [TB_MAXLINE + 1];
    long n_chars = {0};

    if (object == NULL)
        odl_errno = 1;

    m_ptr = (message_fptr != NULL) ? message_fptr :
                                     (FILE *) OdlOpenMessageFile(message_fname,
                                                                 message_fptr);

    n_chars = TB_MAXLINE + 1;
    for (obj=object; obj != NULL; obj=(OBJDESC *) OdlTraverseTree(obj, 
                                                                object->level))
    {
        NewString(format, n_chars)

        kwd = (KEYWORD *)OdlFindKwd(obj, "NAME", NULL,1, ODL_THIS_OBJECT);

        if ((kwd == NULL) || (kwd->value == NULL))
        {
            sprintf(format, " Line %-5d %%%dd %%%ds", 
                    obj->line_number, (obj->level + 1),
                    (2*(obj->level) + strlen(no_name)));

            sprintf(msgtext, format, obj->level, no_name);
            OdlPrintLine(message_fname, m_ptr, msgtext);
        }
        else
        {
            sprintf(format, " Line %-5d %%%dd %%%ds", 
                    obj->line_number, (obj->level + 1),
                    (2*(obj->level) + strlen(kwd->value)));
            sprintf(msgtext, format, obj->level, kwd->value);
            OdlPrintLine(message_fname, m_ptr, msgtext);
        }

        if (obj->class_name == NULL)
            OdlPrintLine(message_fname, m_ptr, "  --  <no class>\n");
        else {
            sprintf(msgtext, "  --  %s\n", obj->class_name);
            OdlPrintLine(message_fname, m_ptr, msgtext);
        }

        LemmeGo(format)

    }  /*  End:  "for (obj=object; ..."  */

    /*  if we opened the message file in this routine then close it  */
    if ((m_ptr != stdout) && (message_fptr == NULL))
        CloseMe(m_ptr)
    
    return;

}  /*  End routine:  "OdlPrintHierarchy"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlChangeExpansionFile                                          */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      Steve Monk (Laboratory for Atmospheric and Space Physics)       */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.1  January 30, 1998                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      01-30-98    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine identifies keywords and ODL tree nodes which       */
/*      resulted from a ^STRUCTURE or ^CATALOG expansion, and changes   */
/*      the filename field of each KEYWORD or OBJDESC structure to the  */
/*      input filename.  The input filename can be a full path; it is   */
/*      used as the new filename field, and the file name part of the   */
/*      path, converted to upper-case, is used as the new ^STRUCTURE    */
/*      keyword value.  The routine traverses the sub-tree whose root   */
/*      is the input object node, and compares each keyword or ODL tree */
/*      node structure's filename field (file name part only if it's a  */
/*      path) with the file name specified in the input expand_kwd      */
/*      value.  If they match, then the routine changes the filename    */
/*      field to the new file name (path).  If an OBJDESC's file_name   */
/*      doesn't match the expand_kwd value, then its children are NOT   */
/*      searched, since they could not have originated from the same    */
/*      ^STRUCTURE expansion.  The routine changes the input expand_kwd */
/*      keyword value to the filename and returns 0.  If bad inputs     */
/*      were given, the routine returns 1.                              */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

int OdlChangeExpansionFile (expand_kwd, filename)

KEYWORD *expand_kwd;
char *filename;

#else

int OdlChangeExpansionFile (KEYWORD *expand_kwd, char *filename)

#endif
{
    unsigned long start_loc = {0};
    unsigned short loc_type = {0};
    char *old_expand_fname = {NULL};
    char buf[128];

    if (expand_kwd == NULL)
    {
        return (1);
    }
    if (expand_kwd->parent == NULL)
    {
        return (1);
    }

    /* Get the file name as it appears in the ^STRUCTURE keyword.  */
    old_expand_fname = OdlGetFileName( expand_kwd, &start_loc, &loc_type);
    if (old_expand_fname == NULL)
    {
        return (1);
    }
    UpperCase( old_expand_fname);

    OdlChangeExpandFile( expand_kwd->parent, expand_kwd, 
                         old_expand_fname, filename);

    /* Set ^STRUCTURE keyword value to new file name.  */
    LemmeGo( expand_kwd->value);
    sprintf( buf, "\"%s\"", OdlExtractFilename( filename));
    CopyString( expand_kwd->value, buf)
    expand_kwd->size = (unsigned long) strlen( expand_kwd->value);
    LemmeGo( old_expand_fname)
    return(0);
}


/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

void OdlChangeExpandFile (object, expand_kwd, old_expand_fname,
                          new_expand_path)

OBJDESC *object;
KEYWORD *expand_kwd;
char *old_expand_fname;
char *new_expand_path;

#else

void OdlChangeExpandFile (OBJDESC *object, KEYWORD *expand_kwd,
                          char *old_expand_fname, char *new_expand_path)

#endif
{
    char fname[256];
    KEYWORD *kwd = {NULL};
    OBJDESC *odltreenode = {NULL};

    /* If the input ODL tree node is the node the ^STRUCTURE keyword is in,
       start processing keywords after the ^STRUCTURE keyword;  otherwise
       process all the keywords in the node.  */

    if (object == expand_kwd->parent)
        kwd = OdlNextKwd( expand_kwd, "*", "*", (unsigned long) 0, 
                          (unsigned short) ODL_THIS_OBJECT);
    else
        kwd = OdlGetFirstKwd( object);

    for (; kwd != NULL; kwd = kwd->right_sibling)
    {
        if (kwd->file_name == NULL)
        {
            continue;
        }
        strcpy( fname, OdlExtractFilename( kwd->file_name));
        UpperCase( fname)
        if (strcmp( old_expand_fname, fname) == 0)
        {        
            OdlSetKwdFilename( kwd, new_expand_path);
        }
    }  /* End: for (; kwd != NULL; kwd = OdlNextKwd())...  */

    /* Process all the children of the input object.  If a child's file_name
       field matches the old_expand_fname, the set it's file_name field to the
       new one, and recursively process its keywords and its children.  If a
       child's file_name field DOESN'T match the old_expand_fname, then don't
       process its keywords or its children, since they could not have 
       originated from the same expansion.  The sequence of node with matching
       file_names may be broken up by non-matching nodes, if those were
       included by another, nested ^STRUCTURE expansion (actually depends on
       where the parser places expanded nodes, may change...), so process ALL
       the children.  */

    for (odltreenode = object->first_child; odltreenode != NULL;
         odltreenode = odltreenode->right_sibling)
    {
        if (odltreenode->file_name == NULL)
            continue;
        strcpy( fname, OdlExtractFilename( odltreenode->file_name));
        UpperCase( fname)
        if (strcmp( old_expand_fname, fname) == 0)
        {
            OdlSetObjFilename( odltreenode, new_expand_path);
            OdlChangeExpandFile( odltreenode, expand_kwd, old_expand_fname,
                                 new_expand_path);
        }
    }
}  /*  End routine:  "OdlChangeExpandFile"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlExtractExpansionTree                                         */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      Steve Monk (Laboratory for Atmospheric and Space Physics)       */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.1  January 30, 1998                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      01-30-98    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine identifies keywords and nodes which resulted from  */
/*      a ^STRUCTURE or ^CATALOG expansion, and moves them to a new     */
/*      tree containing only such nodes and keywords.  The routine      */
/*      does this by comparing the file name part of a keyword or ODL   */
/*      tree node's filename field (which may be a full path) with the  */
/*      file name refered to in the ^STRUCTURE keyword.  Note that if   */
/*      a child node of the node containing the ^STRUCTURE keyword      */
/*      satisfies this comparison, then the entire sub-tree whose root  */
/*      is the child node is cut and added to the new tree.  There may  */
/*      still be nodes and keywords under the child node which have     */
/*      a different filename if these were due to another, nested       */
/*      ^STRUCTURE keyword.  If desired, additional call(s) to this     */
/*      routine can extract these expansion trees.                      */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlExtractExpansionTree (expand_kwd)

KEYWORD *expand_kwd;

#else

OBJDESC *OdlExtractExpansionTree (KEYWORD *expand_kwd)

#endif
{
    char *expand_fname = {NULL};
    char fname[256];
    KEYWORD *kwd = {NULL};
    KEYWORD *next_kwd = {NULL};
    unsigned long start_loc = {0};
    unsigned short loc_type = {0};
    OBJDESC *root_node = {NULL};
    OBJDESC *odltreenode = {NULL};
    OBJDESC *next_odltreenode = {NULL};

    if (expand_kwd == NULL)
    {
        return (NULL);
    }

    /* Get the file name as it appears in the ^STRUCTURE keyword.  */
    expand_fname = OdlGetFileName( expand_kwd, &start_loc, &loc_type);
    
    if (expand_fname == NULL)
        return (NULL);

    UpperCase( expand_fname);
    root_node = OdlNewObjDesc( "ROOT", NULL, NULL, NULL, NULL, NULL, 0, 0);

    /* Process all keywords after the ^STRUCTURE keyword.  */

    kwd = expand_kwd->right_sibling;
    while (kwd != NULL)
    {
        if (kwd->file_name == NULL)
        {
            kwd = kwd->right_sibling;
            continue;
        }
        strcpy( fname, OdlExtractFilename( kwd->file_name));
        UpperCase( fname)
        if (strcmp( expand_fname, fname) == 0)
        {
            next_kwd = kwd->right_sibling;
            OdlPasteKwd( OdlCutKwd( kwd), root_node);
            kwd = next_kwd;
        }
        else
        {
            kwd = kwd->right_sibling;
        }
    }

    /* Process the sub-tree whose root is the node the ^STRUCTURE keyword is
       in.  Note that this sub-tree may contain nested ^STRUCTURE keywords,
       with keywords and nodes which resulted from their expansion;  any sub-
       tree whose root is a child node of the ^STRUCTURE keyword node with a
       matching file_name, is cut out of the main tree and added to the
       extracted tree.  Any further nested ^STRUCTURE expansion keywords and
       nodes may be extracted by additional calls to this routine.  */

    odltreenode = expand_kwd->parent->first_child;
    while (odltreenode != NULL)
    {
        if (odltreenode->file_name == NULL)
        {
            odltreenode = odltreenode->right_sibling;
            continue;
        }
        strcpy( fname, OdlExtractFilename( odltreenode->file_name));
        UpperCase( fname)
        if (strcmp( expand_fname, fname) == 0)
        {
            next_odltreenode = odltreenode->right_sibling;
            OdlPasteObjDesc( OdlCutObjDesc( odltreenode), root_node);
            odltreenode = next_odltreenode;
        }
        else
        {
            odltreenode = odltreenode->right_sibling;
        }
    }

    if (root_node->first_child != NULL)
      OdlSetObjFilename( root_node, root_node->first_child->file_name);
    if (root_node->first_keyword != NULL)
      OdlSetObjFilename( root_node, root_node->first_keyword->file_name);

    /*  Free the file name storage  */
    LemmeGo( expand_fname)

    if (root_node->file_name == NULL)
    {
      OdlFreeTree( root_node);
      return( NULL);
    }
    return( root_node);

}  /*  End routine:  "OdlExtractExpansionTree"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlPrintLabel                                                   */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - removed checking of global                */
/*                            odl_suppress_messages, since it applies   */
/*                            to error messages, not to writing a label.*/
/*      02-04-97    sm/LASP - Added code to print pre-comments.         */
/*      01-30-98    sm/LASP - Added new input argument 'options', to    */
/*                            allow printing of expanded ^STRUCTURE     */
/*                            nodes to separate file or to suppress     */
/*                            "END" statement.  When message_fname is   */
/*                            specified, OdlPrintLabel now overwrites   */
/*                            any existing file, instead of appending.  */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine prints the ODL tree to a file, in ODL format.      */
/*      The input message_fptr can be an open file descriptor;          */
/*      the label is appended, and the file is left open upon return.   */
/*      If message_fptr is NULL, then the routine opens message_fname,  */
/*      prints the label to it, and closes it upon return.  The input   */
/*      root_level should normally be set to 0 to enable printing of    */
/*      the ODL tree below object, which doesn't have to be (but        */
/*      normally is) the actual root of the ODL tree.                   */
/*      Setting root_level to object->level - 1 will print the partial  */
/*      ODL tree whose root is object, AND all the partial ODL trees    */
/*      whose roots are nodes to the right of object.                   */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

void OdlPrintLabel (object, message_fname, message_fptr, root_level, options)

OBJDESC *object;
char *message_fname;
FILE *message_fptr;
unsigned long root_level;
MASK options;

#else

void OdlPrintLabel (OBJDESC *object, char *message_fname, FILE *message_fptr, 
                    unsigned long root_level, MASK options)

#endif
{
    FILE *m_ptr = {NULL};
    char *blanks = {NULL};
    unsigned long i;
    char msgtext [TB_MAXLINE + 1];
    MASK remaining_options = {0};
    char *kwd_name = {NULL};
    KEYWORD *expand_kwd = {NULL};
    OBJDESC *root_node = {NULL};

    odl_suppress_messages = FALSE;

    /* Don't use OdlOpenMessageFile to open the file, because it opens for
       appending (meant to be used only on error message files).  */
    m_ptr = (message_fptr != NULL) ? message_fptr :
            (FILE *) fopen (message_fname, "w");
    if (m_ptr == NULL)
    {
        fprintf( stderr, "OdlPrintLabel: Couldn't open output file!\n");
        return;
    }

    if (object != NULL)
    {
        /* If unexpansion is specified, find all ^STRUCTURE or ^CATALOG
           keywords and extract all the nodes that resulted from each
           expansion, and print them to a separate file.  */
        remaining_options = options & (ODL_UNEXPAND_STRUCTURE |
                                       ODL_UNEXPAND_CATALOG);
        while (remaining_options > 0)
        {
            if ((remaining_options & ODL_UNEXPAND_STRUCTURE) != 0)
            {
                kwd_name = "^*STRUCTURE";
                remaining_options -= ODL_UNEXPAND_STRUCTURE;
            }
            else
            {
                kwd_name = "^*CATALOG";
                remaining_options -= ODL_UNEXPAND_CATALOG;
            }
            expand_kwd = OdlFindKwd( object, kwd_name, "*", (unsigned long) 0,
                                     (unsigned short) ODL_THIS_OBJECT);
            while (expand_kwd != NULL)
            {
                root_node = OdlExtractExpansionTree( expand_kwd);
                if (root_node != NULL)
                {
                    OdlPrintLabel( root_node, root_node->file_name, NULL,
                                   0, options | ODL_SUPPRESS_END_STATEMENT);
                    OdlFreeTree( root_node);
                }
                expand_kwd = OdlNextKwd( expand_kwd->right_sibling, kwd_name,
                                         "*", (unsigned long) 0,
                                         (unsigned short) ODL_THIS_OBJECT);
            }  /*  End: "while (expand_kwd != NULL) ..."  */
        }  /*  End: "while (unexpand > 0) ..."  */

        NewString(blanks, (4*object->level))
        for (i=1; i < object->level; ++i) strcat(blanks, "  ");
    
        if (object->pre_comment != NULL)
            if (strlen( object->pre_comment) > 0)
            {
                OdlPrintLine(message_fname, m_ptr, object->pre_comment);

                /* If pre-comment wasn't all newlines (representing blank 
                   lines in the label), then add another one. */
                for (i=0; i<strlen( object->pre_comment); i++)
                    if (object->pre_comment[i] != '\n')
                    {
                        OdlPrintLine(message_fname, m_ptr, "\n");
                        break;
                    }
            }    

        if (object->parent != NULL)
        {
            if (object->class_name == NULL) {
                sprintf(msgtext, "%sOBJECT", blanks);
                OdlPrintLine(message_fname, m_ptr, msgtext);
            }
            else {
                sprintf(msgtext, "%sOBJECT = %s", blanks, object->class_name);
                OdlPrintLine(message_fname, m_ptr, msgtext);
            }
    
            if (object->line_comment != NULL) {
                OdlPrintLine(message_fname, m_ptr, " ");
                OdlPrintLine(message_fname, m_ptr, object->line_comment);
            }
    
            OdlPrintLine(message_fname, m_ptr, "\n");
    
        }  /*  End:  "if (object->parent != NULL) ..."  */
    
        OdlPrintKeywords(object, NULL, m_ptr);
        OdlPrintLabel(object->first_child, NULL, m_ptr, root_level, options);
    
        if (object->post_comment != NULL)
            if (strlen( object->post_comment) > 0)
            {
            OdlPrintLine(message_fname, m_ptr, object->post_comment);

            /* If post-comment wasn't all newlines (representing blank 
               lines in the label), then add another one. */
            for (i=0; i<strlen( object->post_comment); i++)
                if (object->post_comment[i] != '\n')
                {
                    OdlPrintLine(message_fname, m_ptr, "\n");
                    break;
                }
            }

        if (object->parent != NULL)
        {
            if (object->class_name == NULL)
                sprintf(msgtext, "%sEND_OBJECT", blanks);
            else
                sprintf(msgtext, "%sEND_OBJECT = %s", blanks,
                        object->class_name);
            OdlPrintLine(message_fname, m_ptr, msgtext);
    
            if (object->end_comment != NULL) {
                OdlPrintLine(message_fname, m_ptr, " ");
                OdlPrintLine(message_fname, m_ptr, object->end_comment);
            }
    
            OdlPrintLine(message_fname, m_ptr, "\n");
   
        }  /*  End:  "if (object->parent != NULL) ..."  */
    
        if (object->level > root_level)
            OdlPrintLabel(object->right_sibling, 0, m_ptr, root_level, options);
    
        LemmeGo(blanks)
    
        if ((object->parent == NULL) &&
            ((options & ODL_SUPPRESS_END_STATEMENT) == 0))
            OdlPrintLine(message_fname, m_ptr, "END\n");
    
    }  /*  End:  "if (object != NULL) ..."  */
    else
        odl_errno = 1;
    
    /*  if we opened the message file in this routine then close it  */
    if ((m_ptr != stdout) && (message_fptr == NULL))
        CloseMe(m_ptr)
    
    return;

}  /*  End routine:  "OdlPrintLabel"  */



/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static void OdlPrintKeywords (object, message_fname, message_fptr)

OBJDESC *object;
char *message_fname;
FILE *message_fptr;

#else

static void OdlPrintKeywords (OBJDESC *object, char *message_fname, 
                              FILE *message_fptr)

#endif
{
    KEYWORD *keyword = {NULL};
    FILE *m_ptr = {NULL};
    short sfdu_only = {FALSE};
    char blanks[ TB_MAXLINE + 1];
    int i, len, n_blanks, kwd_name_column, max_kwd_name_len = {0};
    int kwd_equals_sign_column;

    for (i=0; i<(TB_MAXLINE+1); i++)  blanks[i] = ' ';
    m_ptr = (message_fptr != NULL) ? message_fptr :
                                     (FILE *) OdlOpenMessageFile(message_fname, message_fptr);

    if (object != NULL)
    {
        kwd_name_column = (int) object->level * 2;

        /* Get length of longest keyword name whose length is <= 20,
           and align the equals sign and values of all keywords whose
           keyword name length is < 20 on the same column.  sm 2/4/97  */
        for (keyword=object->first_keyword; keyword != NULL; 
                 keyword = keyword->right_sibling)
            if (keyword->name != NULL)
            {
                len = (int) strlen( keyword->name);
                if ((len <= 20) && (len > max_kwd_name_len))
                    max_kwd_name_len = len;
            }
        if (max_kwd_name_len == 0)  max_kwd_name_len = 20;
        kwd_equals_sign_column = kwd_name_column + max_kwd_name_len + 1;

        for (keyword=object->first_keyword; keyword != NULL; 
                 keyword = keyword->right_sibling)
        {
            if (keyword->pre_comment != NULL)
                if (strlen( keyword->pre_comment) > 0)
                {
                    OdlPrintLine(message_fname, m_ptr, keyword->pre_comment);

                    /* If pre-comment was all newlines, then when printed out
                       they make the desired number of blank lines.  Otherwise
                       add another newline after the pre-comment to get to the
                       next line.  sm 2/4/97  */
                    for (i=0; i< (int) strlen( keyword->pre_comment); i++)
                        if (keyword->pre_comment[i] != '\n')
                        {
                            OdlPrintLine(message_fname, m_ptr, "\n");
                            break;
                        }
                }
            blanks[ kwd_name_column] = '\0';
            OdlPrintLine(message_fname, m_ptr, blanks);
            blanks[ kwd_name_column] = ' ';

            sfdu_only = FALSE;
            if (keyword->name == NULL)
            {
                OdlPrintLine(message_fname, m_ptr, "unknown_keyword");
                len = (int) strlen( "unknown_keyword");
            }
            else
            { 
                OdlPrintLine(message_fname, m_ptr, keyword->name);
                len = (int) strlen( keyword->name);
                sfdu_only = (((strncmp(keyword->name, "NJPL", 4) == 0) ||
                              (strncmp(keyword->name, "CCSD", 4) == 0)) &&
                              (strlen(keyword->value) == 0));
            }

            if ((keyword->value != NULL) && (! sfdu_only)) {
                n_blanks = kwd_equals_sign_column - (kwd_name_column + len+1);
                if ((n_blanks > 0) && ((kwd_name_column + len + n_blanks + 3 +
                                        strlen( keyword->value)) < 78))
                {
                    blanks[ n_blanks] = '\0';
                    OdlPrintLine(message_fname, m_ptr, blanks);
                    blanks[ n_blanks] = ' ';
                }
                OdlPrintLine(message_fname, m_ptr, " = ");
                OdlPrintLine(message_fname, m_ptr, keyword->value);
            }

            if (keyword->line_comment != NULL) {
                OdlPrintLine(message_fname, m_ptr, " ");
                OdlPrintLine(message_fname, m_ptr, keyword->line_comment);
            }

            OdlPrintLine(message_fname, m_ptr, "\n");

        }  /*  End:  "for (keyword=object ..."  */

    }  /*  End:  "if (object != NULL) ..."  */
    else
        odl_errno = 1;

    /*  if we opened the message file in this routine then close it  */
    if ((m_ptr != stdout) && (message_fptr == NULL))
        CloseMe(m_ptr)
    
    return;

}  /*  End routine:  "OdlPrintKeywords"  */




/*========================================================================*/
/*                                                                        */
/*                       Parser-specific routines                         */
/*                                                                        */
/*========================================================================*/


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlParseFile                                                    */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - Allow SFDU without "= SFDU" to be parsed  */
/*                            into a keyword (with a zero-length value).*/
/*      12-11-95    sm/LASP - Added new input parameter 'label_str' to  */
/*                            allow parsing a label string (without     */
/*                            creating a temporary file).               */
/*      12-11-95    sm/LASP - Fixed bug involving comments embedded     */
/*                            inside text strings.                      */
/*      02-04-97    sm/LASP - Added code to handle pre-comments.        */
/*      03-10-05    MDC     - Added code to recognize GROUP objects     */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine actually does the parsing of a label file (or      */
/*      label string) and returns a pointer to the root object of the   */
/*      tree.                                                           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlParseFile(label_fname, label_fptr, label_str, message_fname, message_fptr,
                     suppress_messages, suppress_metrics, suppress_hierarchy,
                     ignore_missing_end)

char *label_fname;
FILE *label_fptr;
char *label_str;
char *message_fname;
FILE *message_fptr;
unsigned short suppress_messages;
unsigned short suppress_metrics;
unsigned short suppress_hierarchy;
unsigned short ignore_missing_end;

#else

OBJDESC *OdlParseFile( char *label_fname, FILE *label_fptr, char *label_str, 
                      char *message_fname, FILE *message_fptr,
                      unsigned short suppress_messages, 
                      unsigned short suppress_metrics, 
                      unsigned short suppress_hierarchy,
                      unsigned short ignore_missing_end)

#endif        

{
    OBJDESC *root = {NULL};
    OBJDESC *curr_object = {NULL};
    KEYWORD *curr_keyword = {NULL};
    FILE *m_ptr = {NULL};
    FILE *l_ptr = {NULL};
    char *left_part = {NULL};
    char *equals = {NULL};
    char *right_part = {NULL};
    char *comment = {NULL};
    char *c = {NULL};
    char *tc = {NULL};
    char *tintext = {NULL};
    char *text = {NULL};
    char line_comment [TB_MAXLINE + 1];
    char intext [TB_MAXLINE + 1];
    char msgtext [TB_MAXLINE + 1];
    char *current_label_ptr = {NULL};
    long line_number = {0};
    long value_list_line_number = {0};
    long object_count = {0};
    long end_object_count = {0};
    long keyword_count = {0};
    long comment_count = {0};
    long brace_nesting = {0};
    long paren_nesting = {0};
    long chars_read    = {0};
    long n_chars       = {0};
    short end_found = {FALSE};
    short equals_found = {FALSE};
    short val_found = {FALSE};
    short balanced = {FALSE};
    short oddquotes = {FALSE};
    short is_a_group = 0; /* 03-10-05 MDC - Added to flag GROUP objects in a label */
    odl_message_count = 0;
    odl_suppress_messages = suppress_messages;

    /*  either use the file pointer passed in or open the message file  */
    m_ptr = (message_fptr != NULL) ? message_fptr :
                                     (FILE *) OdlOpenMessageFile(message_fname, message_fptr);
    /*  opening remarks  */
    if (label_fname == NULL)
        sprintf(msgtext, "Parsing File:  (no file name provided)");
    else
        sprintf(msgtext, "Parsing File:  %s", label_fname);

    OdlPrintLine(message_fname, m_ptr, "\n--------------------------------------------------------------------------\n");
    OdlPrintLine(message_fname, m_ptr, msgtext);
    OdlPrintLine(message_fname, m_ptr, "\n--------------------------------------------------------------------------\n\n");

    /*  if no label_str was specified, either use the file pointer passed in
        or open the label file  */
    if (label_str == NULL)
        l_ptr = (label_fptr != NULL) ? label_fptr :
                ((label_fname == NULL) ? NULL :
                (FILE *) fopen(label_fname,"r"));
    else
        current_label_ptr = label_str;
    
    if ((l_ptr == NULL) && (label_str == NULL))
    {
        odl_errno = 200;
        OdlPrintMessage(message_fname, m_ptr, 0,
            "Unable to open the label file.  Parsing cannot continue");
    }
    else
    {
        n_chars = 1;
        NewString(comment, n_chars)

        /*  Initialize a ROOT object to start the tree  */
        curr_object = root = OdlNewObjDesc("ROOT",0,0,0,0,label_fname,0,0);

        
        while (! end_found)
        {
            /*  read a line from file or extract a line from label_str  */
            if (label_str == NULL)
            {
                chars_read = (long) fgets(intext, TB_MAXLINE, l_ptr);
                if (chars_read == 0) break;
            }
            else
            {
                while (((*current_label_ptr == '\r') || 
                        (*current_label_ptr == '\n')) &&
                        (*current_label_ptr != '\0'))
	        {
                    if ((*current_label_ptr == '\r') &&
                        (*(current_label_ptr + 1) == '\n'))
                        line_number--;
                    current_label_ptr++;
                    line_number++;
   	        }
                if (*current_label_ptr == '\0')
                    break;
                chars_read = (long) strcspn( current_label_ptr, "\r\n");
                current_label_ptr[ chars_read] = '\0';
                strcpy( intext, current_label_ptr);
                current_label_ptr += (chars_read+1);
            }
            ++line_number;
            StripUnprintables(intext)       /*  removes linefeeds and such  */
            ReplaceChar(intext, '\t', ' ')  /*  turns TABs into blanks      */
            StripTrailing(intext, ' ')      /*  removes trailing blanks     */

#ifdef OA_DEBUG2
            printf( "intext = %s\n", intext);
#endif

            /*  locate, save, and remove comment text from the line    */
            line_comment[0] = '\0';
            tintext = intext;
         
			while ((c = strstr(tintext, "/*")) != NULL)
			{
		        /* Count occurrences of dbl quotes preceeding the comment to
                   see if comment is embedded in a quoted string. */
                for (tc = tintext; tc < c; tc++)
				{
                    if (*tc == '"')
                        if (oddquotes) oddquotes = FALSE;
                        else oddquotes = TRUE;
				}
                if (! oddquotes)  /* Rest of line (or whole line) is comment.*/
                {
                    ++comment_count;
                    strcpy(line_comment, c);
                    *c = '\0';
					StripTrailing(tintext, ' ');
                    break;
 				}
				else                /* Comment is inside dbl quotes.  */
				{
					tintext = c+1;  /* Point to the '*' in "/*".      */

					/*********************************************************/
					/* 02-03-03 MDC											 */
					/* Check to see if you are still within the quotes. If   */
					/* you are, then point to the character after the '"'    */
					/* since we don't care about anything inside the quotes. */
					/*********************************************************/
					if((tintext = strchr(tintext, '"')) != NULL)
					{
						tintext = tintext+1;
						oddquotes = FALSE;
					}
				}
            }  /* End while tintext has "/*" in it  */

            c = OdlFirstWord(intext);

            /* Append current line (without comment) to text stored from
               previous line, if any.  */
            if ((text != NULL) || (*c != '\0'))
            {
                AppendString(text, intext)                
                c = OdlFirstWord(text);

                if (strcmp(c, "END") == 0)
                {
                    balanced = TRUE;
                    end_found = TRUE;
                    break;
                }
                else
                if (strcmp(c, "END_OBJECT") == 0)
                {
                    balanced = TRUE;
                }
				else
                if (strcmp(c, "END_GROUP") == 0)
                {
                    balanced = TRUE;
                }
                else
                {
                    if (! equals_found)
                        equals_found = (strchr(text, '=') != NULL);
    
                    if (! val_found && equals_found)
                    {
                        c = (char *) LastChar(text);
                        val_found = (*c != '=');
                    }

                    if (val_found && equals_found)
                        balanced = CheckBalance(text);

                    if (! balanced)
  	                AppendString(text, "\n")                

                    /* If a SFDU is on the first line, then force balance to
                       TRUE, even though the "= SFDU_LABEL" may be missing, so
                       that we'll enter the next code section.
                       A keyword with a name but no value will be created,
                       which is what we want.  sm 12/11/95  */
                    if ((line_number == 1L) && ((! strncmp(text, "CCSD",4)) || 
                                                (! strncmp(text, "NJPL",4))))
                        balanced = TRUE;
                }
	    }
            else
            /* Line has no ODL statement on it;  if there was no line comment
               (blank line) then save the blank line by adding newline.
               If there was a line comment, then add it, preceeded by a newline
               if the last character in the previous line's comment (if any) 
               wasn't a newline.  sm 2/4/97  */
            {
                if (strlen( line_comment) == 0)
                {
                    AppendString( comment, "\n")
                }
                else
                {
                    if (strlen( comment) > 0)
                        if (comment[ strlen( comment)-1] != '\n')
                            AppendString( comment, "\n")
                    AppendString( comment, line_comment)
                }
                line_comment[0] = '\0';
            }

#ifdef OA_DEBUG2
                    printf( "new intext = %s\n", intext);
                    printf( "comment = %s\n", comment);
                    printf( "line_comment = %s\n", line_comment);
#endif

            if (balanced)
            {
                /*  locate the keyword, the equals sign, and the value  */
                left_part = OdlFirstWord(text);

                if ((equals = (char *)strchr(left_part, '=')) != NULL)
                    right_part = OdlFirstWord(equals+1);
                else
                {
                    equals = text + strlen(text);
                    right_part = equals;
                }

        /*------------------------------------------------------------------*/
        /*  Here's where the parsing begins.  First, we take care of three  */
        /*  special cases:  multi-line quoted values, multi-line value      */
        /*  lists, and blank lines.  If the current line isn`t one          */
        /*  of these than it's either an OBJECT statement, an END_OBJECT    */
        /*  statement, the END of the label, or a new KEYWORD.              */
        /*------------------------------------------------------------------*/

                /*  we've discovered the beginning of a new object  */
                if ((strncmp(left_part, "OBJECT ", 7) == 0) || 
                     (strcmp(left_part, "OBJECT") == 0)
					 ||(strncmp(left_part, "GROUP ", 6) == 0) || 
                   (strcmp(left_part, "GROUP") == 0)
					)
                {
                    ++object_count;
                    ++(curr_object->child_count);
    
					/* 03-10-05 MDC - Check to see if we've found a GROUP object */
					if( (strncmp(left_part, "GROUP ", 6) == 0) ||
						(strcmp(left_part, "GROUP") == 0) )
					{
						is_a_group = 1;
					}
					else
						is_a_group = 0;

                    /*  validate the new object's class identifier  */
                    OdlValidObjDesc(equals, right_part, message_fname,
                                    m_ptr, line_number);
    
                    /*  make the new object a child of the current object  */
       /*           curr_object = OdlPasteObjDesc(OdlNewObjDesc(right_part,
                                                  comment,line_comment,0,0,
                                                  label_fname,0,line_number),
                                                curr_object);
	 */
					/* 03-10-05 MDC - Because we've flagged a GROUP object, we
					  should pass this into the routine below to record it in the
					  tree
				    */
					curr_object = OdlPasteObjDesc(OdlNewObjDesc(right_part,
												  comment,line_comment,0,0,
												  label_fname,is_a_group,line_number),
											    curr_object);
    
                    /*  reset the comment text string  */
                    LemmeGo(comment)
                    n_chars = 1;
                    NewString(comment, n_chars)
                }
                else
        /*------------------------------------------------------------------*/
                /*  we've discovered the end of the current object  */
                if ((strncmp(left_part, "END_OBJECT ", 11) == 0) ||
                     (strcmp(left_part, "END_OBJECT") == 0)
					 ||(strncmp(left_part, "END_GROUP ", 10) == 0) ||
                     (strcmp(left_part, "END_GROUP") == 0)
					 )
                {
                    ++end_object_count;
    
                    /*  validate the end_object's class identifier  */
                    OdlValidEndObjDesc(curr_object, equals, right_part,
                                   message_fname, m_ptr, line_number);
    
                    /*  set the current object's remaining comment fields  */
                    CopyString(curr_object->post_comment, comment)
                    CopyString(curr_object->end_comment, line_comment)
    
                    /*  make curr object's parent the new current object  */
                    if (curr_object->parent != NULL)
                        curr_object = curr_object->parent;
    
                    /*  reset the comment text string  */
                    LemmeGo(comment)
                    n_chars = 1;
                    NewString(comment, n_chars)
                }
                else
        /*------------------------------------------------------------------*/
                /*  we've reached the end of the label  */
                if ((strncmp(left_part, "END ", 4) == 0) || 
                     (strcmp(left_part, "END") == 0))
                {
                    end_found = TRUE;
                    CopyString(curr_object->post_comment, comment)
                }
                else
        /*------------------------------------------------------------------*/
                /*  We've discovered a keyword and its value  */
                {
                    ++keyword_count;
    
                    /*  validate the keyword and its values  */
                    OdlValidKwd(curr_object, left_part, equals, right_part,
                                message_fname, m_ptr, line_number);
    
                    /*  Add the keyword to the current object  */
                    curr_keyword = OdlPasteKwd(OdlNewKwd(left_part, right_part,
                                                     comment, line_comment, 
                                                     label_fname, line_number),
                                                     curr_object);
    
                    /*  we've got a potential multi-line value list if the 
                        first character of the value is either an open 
                        brace, '{', or an open paren, '('
                    */

                    if (curr_keyword->is_a_list == TRUE)
                    {
                        /*  validate that the braces and parens are correct  */
                        OdlValidBraces(curr_keyword->value,
                                    brace_nesting, paren_nesting,
                                        message_fname, m_ptr, 
                                        value_list_line_number);


                    }  /*  End:  "if ((curr_keyword->is_a_list) == TRUE)"    */

                    /*  reset the comment text string  */
                    LemmeGo(comment)
                    n_chars = 1;
                    NewString(comment, n_chars)
    
                }  /*  End:  "if ((strncmp(left_part, ... else ... else ..." */
    /*------------------------------------------------------------------*/

                equals_found = FALSE;
                val_found = FALSE;
                balanced = FALSE;
                oddquotes = FALSE;
                LemmeGo(text)

	    }  /*  End:  "if (balanced)"  */
    /*------------------------------------------------------------------*/
        }  /*  End:  "while (fgets(text, ..."  */

        /* if we're not sitting at the root then not enough END_OBJECTs found */
        if (curr_object->parent != NULL)
        {
            odl_errno = 300;
            OdlPrintMessage(message_fname, m_ptr, line_number,
        "Not enough END_OBJECT statements.  Some objects may be incomplete");
        }

        /*  hey, we didn't find an end statement!  */
        if ((! end_found) && (! ignore_missing_end))
        {
            odl_errno = 301;
            OdlPrintMessage(message_fname, m_ptr, line_number,
                "END statement is missing");
        }

        /*  oops, there was nothing in the label file to parse  */
        if (line_number == 0) 
        {
            root = OdlFreeTree(root);
            odl_errno = 302;
        }

        LemmeGo(comment)

    }  /*  End:  "if (l_ptr == NULL) ... else ..."  */

    /*  how'd we do?  */
    if (! suppress_metrics && ! suppress_messages) 
    {
        OdlPrintLine(message_fname, m_ptr, "\n");
        OdlPrintLine(message_fname, m_ptr, "           |-------------------------------|\n");
        OdlPrintLine(message_fname, m_ptr, "           | Parsing Metrics:              |\n");
        OdlPrintLine(message_fname, m_ptr, "           |                               |\n");
        sprintf(msgtext, "           | %7d Syntax Messages       |\n", odl_message_count);
        OdlPrintLine(message_fname, m_ptr, msgtext);
        OdlPrintLine(message_fname, m_ptr, "           |                               |\n");
        sprintf(msgtext, "           | %7d OBJECT Statements     |\n", object_count);
        OdlPrintLine(message_fname, m_ptr, msgtext);
        sprintf(msgtext, "           | %7d END_OBJECT Statements |\n", end_object_count);
        OdlPrintLine(message_fname, m_ptr, msgtext);
        sprintf(msgtext, "           | %7d Keywords              |\n", keyword_count);
        OdlPrintLine(message_fname, m_ptr, msgtext);
        sprintf(msgtext, "           | %7d Comments              |\n", comment_count);
        OdlPrintLine(message_fname, m_ptr, msgtext);
        OdlPrintLine(message_fname, m_ptr, "           |-------------------------------|\n\n");

    }  /*  End:  "if (! suppress_metrics) ..."  */

    /*  display the object hierarchy  */
    if (! suppress_hierarchy && ! suppress_messages)
    {
        if (label_fname == NULL)
            sprintf(msgtext, "Object Hierarchy in File:  (no file name provided)");
        else
            sprintf(msgtext, "Object Hierarchy in File:  %s", label_fname);

        OdlPrintLine(message_fname, m_ptr, "\n--------------------------------------------------------------------------\n");
        OdlPrintLine(message_fname, m_ptr, msgtext);
        OdlPrintLine(message_fname, m_ptr, "\n--------------------------------------------------------------------------\n\n");

        OdlPrintHierarchy(root, message_fname, m_ptr);

    }  /*  End:  "if (suppress_hierarchy) ..."  */

    /*  closing remarks  */



    if (label_fname == NULL)
        sprintf(msgtext, "End of Parsing File:  (no file name provided)");
    else
        sprintf(msgtext, "End of Parsing File:  %s", label_fname);

    OdlPrintLine(message_fname, m_ptr, "\n--------------------------------------------------------------------------\n");
    OdlPrintLine(message_fname, m_ptr, msgtext);
    OdlPrintLine(message_fname, m_ptr, "\n--------------------------------------------------------------------------\n\n");

    /*  if we opened the label file in this routine then close it  */
    if (label_fptr == NULL)
        CloseMe(l_ptr)

    /*  if we opened the message file in this routine then close it  */
    if ((m_ptr != stdout) && (message_fptr == NULL))
        CloseMe(m_ptr)
    
    LemmeGo(text)

    return (root);

}  /*  End routine:  OdlParseFile  */


/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static short OdlNestingLevel (text, brace_nesting, paren_nesting)
char *text;
long *brace_nesting;
long *paren_nesting;

#else

static short OdlNestingLevel (char *text, long *brace_nesting, 
                              long *paren_nesting)

#endif
{
    char *c = {NULL};

    for (c=text; *c != '\0'; ++c)
    {
        if (*c == '{') 
            ++(*brace_nesting);
        else
        if (*c == '}') 
            --(*brace_nesting);
        else
        if (*c == '(') 
            ++(*paren_nesting);
        else
        if (*c == ')') 
            --(*paren_nesting);
    }

    return((*brace_nesting == 0) && (*paren_nesting == 0));

}  /*  End routine:  "OdlNestingLevel"  */


/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static short OdlValidBraces (text, brace_nesting, paren_nesting,
                          message_fname, message_fptr, line_number)

char *text;
long brace_nesting;
long paren_nesting;
char *message_fname;
FILE *message_fptr;
long line_number;

#else

static short OdlValidBraces (char *text, long brace_nesting, 
                             long paren_nesting, char *message_fname, 
                             FILE *message_fptr, long line_number)

#endif
{
    char *c = {NULL};
    char *sp = {NULL};
    char *nesting_stack = {NULL};
    short status = {TRUE};

    /*  allocate storage for the nesting stack  */
    NewString(nesting_stack, strlen(text))

    /*  validate that all braces and parens are correctly nested  */
    for (c=text,sp=(nesting_stack-1); ((*c != '\0') && (status == TRUE)); ++c)
    {
        /*  push brace or paren onto the nesting stack  */
        if ((*c == '{') || (*c == '('))
            *(++sp) = *c;
        else
        /*  nesting is ok so far, pop the nesting stack  */
        if (((*c == '}') && (*sp == '{')) || ((*c == ')') && (*sp == '(')))
            --sp;
        else
        /*  found a right brace that doesn't have a matching left one  */
        if ((*c == '}') && (*sp != '{'))
        {
            odl_errno = 303;
            status = OdlPrintMessage(message_fname,message_fptr,line_number, 
                         "Bad nesting in VALUE LIST.  Expected a right parenthesis and found a brace instead.");
        }
        else
        /*  found a right paren that doesn't have a matching left one  */
        if ((*c == ')') && (*sp != '('))
        {
            odl_errno = 303;
            status = OdlPrintMessage(message_fname,message_fptr,line_number, 
                         "Bad nesting in VALUE LIST.  Expected a right brace and found a parenthesis instead.");
        }

        /*  we've reached nesting level zero before reaching the end  */
        if ((sp < nesting_stack) && (*(c+1) != '\0'))
        {
            odl_errno = 303;
            status = OdlPrintMessage(message_fname,message_fptr,line_number, 
                         "VALUE LIST not properly enclosed in braces or parentheses");
        }

    }  /*  End:  "for (c=text,sp=(nesting_stack-1); ..."  */

    LemmeGo(nesting_stack)

    if (brace_nesting < 0)
    {
        odl_errno = 303;
        status = OdlPrintMessage(message_fname, message_fptr, line_number, 
                     "Too many right braces in VALUE LIST");
    }
    else
    if (brace_nesting > 0)
    {
        odl_errno = 303;
        status = OdlPrintMessage(message_fname, message_fptr, line_number, 
                     "Too many left braces in VALUE LIST");
    }

    if (paren_nesting < 0)
    {
        odl_errno = 303;
        status = OdlPrintMessage(message_fname, message_fptr, line_number, 
                     "Too many right parentheses in VALUE LIST");
    }
    else
    if (paren_nesting > 0)
    {
        odl_errno = 303;
        status = OdlPrintMessage(message_fname, message_fptr, line_number, 
                     "Too many left parentheses in VALUE LIST");
    }

    return(status);

}  /*  End routine:  "OdlValidBraces"  */


/************************************************************************/
/*  Local Routine                                                       */
/*                                                                      */
/* Change History:                                                      */
/*      07-30-03   MDC     Commented out line to strip trailing blanks  */
/*                         off of values with units expression.         */ 
/************************************************************************/

#ifdef _NO_PROTO

static short OdlValidElement (text, message_fname, message_fptr, line_number, 
                           element_number)
char *text;
char *message_fname;
FILE *message_fptr;
long line_number;
long element_number;

#else

static short OdlValidElement (char *text, char *message_fname, 
                              FILE *message_fptr, long line_number, 
                              long element_number)

#endif
{
    char *message = NULL;
    char element_prompt[TB_MAXLINE + 1];
    char *save_units = 0;
    char *first_char = {NULL};
    char *last_char = {NULL};
    char *units_start = {NULL};
    char *units_end = {NULL};
    char *single_quote = {NULL};
    char *double_quote = {NULL};
    short status = {TRUE};

    if (element_number <= 0)
       strcpy(element_prompt, "");
    else
       sprintf(element_prompt, " LIST element %d", element_number);

    single_quote = (char *) strchr(text+1, (int) '\'');
    double_quote = (char *) strchr(text+1, (int) '"');
    first_char = text;
    last_char = (char *) LastChar(text);

    NewString(message, TB_MAXLINE+strlen(text))

    /*  double quote found in the middle of the value  */
    if ((double_quote > first_char) && (double_quote < last_char))
    {
        odl_errno = 304;
        sprintf(message, "Embedded double quote in VALUE%s", element_prompt);
        status = OdlPrintMessage(message_fname, message_fptr, line_number, message);
    }
    else
    /*  value is double quoted - everything is okay  */
    if (*first_char == '"')
    {
        status = TRUE;
    }
    else
    /*  single quote found in the middle of the value  */
    if ((single_quote > first_char) && (single_quote < last_char))
    {
        odl_errno = 304;
        sprintf(message, "Embedded single quote in VALUE%s", element_prompt);
        status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
    }
    else
    /*  value is single quoted - fine if not just a quote  */
    if ((*first_char == '\'') && (*last_char == '\''))
    {
        if (first_char == last_char)
        {
            odl_errno = 304;
            sprintf(message, "Unpaired single quote in VALUE%s", element_prompt);
            status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
        }
    }
    else
    /*  value is missing a closing single quote  */
    if ((*first_char == '\'') && (*last_char != '\''))
    {
        odl_errno = 304;
        sprintf(message, "Unpaired single quote in VALUE%s", element_prompt);
        status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
    }
    else
    /*  value is missing an opening single quote  */
    if ((*first_char != '\'') && (*last_char == '\''))
    {
        odl_errno = 304;
        sprintf(message, "Unpaired single quote in VALUE%s", element_prompt);
        status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
    }
    else
    /*  value is missing an opening double quote  */
    if ((*first_char != '"') && (*last_char == '"'))
    {
        odl_errno = 304;
        sprintf(message, "Unpaired double quote in VALUE%s", element_prompt);
        status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
    }
    else
    /*  current value list element is just a double quote  */
    if ((element_number > 0) && 
        (first_char == (last_char-1)) && (*first_char == '"'))
    {
        odl_errno = 304;
        sprintf(message, "Unpaired double quote in VALUE%s", element_prompt);
        status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
    }
    else
    /*  current value list element is missing a closing double quote  */
    if ((element_number > 0) && 
        (*first_char == '"') && (*(last_char-1) != '"'))
    {
        odl_errno = 304;
        sprintf(message, "Unpaired double quote in VALUE%s", element_prompt);
        status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
    }
    else
    /*  value is unquoted  */
    if ((*first_char != '\'') && (*last_char != '\''))
    {
        /*  check the value only if it isn't N/A  */
        if ((strcmp(first_char, "n/a") != 0) && (strcmp(first_char, "N/A") != 0))
        {
            /*  we can't have multiple underscores in an unquoted value  */
            if (strstr(first_char, "__") != NULL)
            {
                odl_errno = 305;
                sprintf(message, "Multiple underscores in VALUE%s", element_prompt);
                status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
            }
    
            /*  an unquoted value cannot begin with an underscore  */
            if (*first_char == '_')
            {
                odl_errno = 304;
                sprintf(message, "First character is an underscore in VALUE%s", element_prompt);
                status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
            }
    
            /*  an unquoted value cannot end with an underscore  */
            if (*last_char == '_')
            {
                odl_errno = 304;
                sprintf(message, "Last character is an underscore in VALUE%s", element_prompt);
                status = OdlPrintMessage(message_fname,message_fptr, line_number, message);
            }

            /*  the value may have a units expression  */
            if ((units_start = (char *) strchr(text, (int) '<')) != NULL)
	    {
                CopyString(save_units, units_start)
                *units_start = '\0';
				/* 07-29-03 MDC - Don't need to strip trailing blanks. The call
				   to OdlDataType does this already.
				*/
/*		StripTrailing(text, ' ') */
            }
            
            if (OdlDataType(text) == ODL_UNKNOWN)
            {
                odl_errno = 306;
                sprintf(message, "Unable to determine the data type of VALUE%s: \"%s\"", 
                                 element_prompt, first_char);
                status = OdlPrintMessage(message_fname, message_fptr, line_number, message);
            }

            /*  validate the units expression, if any  */
            if (units_start != NULL)
            {
                /*  only one '<' char allowed in a units expression  */
                if (strchr(units_start+1, (int) '<') != NULL)
                {
                    odl_errno = 307;
                    sprintf(message, "Embedded '<' character found in the UNITS expression: \"<%s\", for VALUE%s:  \"%s\"", 
                                     units_start+1, element_prompt, first_char);
                    status = OdlPrintMessage(message_fname, message_fptr, line_number, message);
                }

                /*  find the closing char for the units expression  */
                units_end = (char *) strchr(units_start+1, (int) '>');

                /*  missing the closing '>' char in the units expression  */
                if (units_end == NULL)
                {
                    odl_errno = 307;
                    sprintf(message, "Missing the closing '>' character in the UNITS expression: \"<%s\", for VALUE%s:  \"%s\"", 
                                     units_start+1, element_prompt, first_char);
                    status = OdlPrintMessage(message_fname, message_fptr, line_number, message);
                }
                else
                /*  characters found after the closing '>' in the units exp  */
                if (units_end != last_char)
                {
                    odl_errno = 307;
                    sprintf(message, "Extraneous characters found after the closing '>' character in the UNITS expression: \"<%s\", for VALUE%s:  \"%s\"", 
                                     units_start+1, element_prompt, first_char);
                    status = OdlPrintMessage(message_fname, message_fptr, line_number, message);
                }
  
                /*  restore the value exactly as it was, since caller saved
                    its length. */
                strcat(text, save_units);
                LemmeGo(save_units)

            }  /*  End:  "if (units_start != NULL) ..."  */

        }  /*  End:  "if ((strcmp(first_char, "n/a") != 0) && ..."  */

    }  /*  End:  "if ((double_quote > ... else ... else ..."  */
    
    LemmeGo(message)

    return(status);

}  /*  End routine:  "OdlValidElement"  */


/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static short OdlValidEndObjDesc (curr_object, equals, right_part, 
                             message_fname, message_fptr, line_number)
OBJDESC *curr_object;
char *equals;
char *right_part;
char *message_fname;
FILE *message_fptr;
long line_number;

#else

static short OdlValidEndObjDesc (OBJDESC *curr_object, char *equals, 
                                 char *right_part, char *message_fname, 
                                 FILE *message_fptr, long line_number)

#endif
{
    short status = {TRUE};

    if (curr_object->parent == NULL)
    {
        odl_errno = 300;
        status = OdlPrintMessage(message_fname, message_fptr, line_number,
                     "Extra END_OBJECT encountered");
    }

    if (*equals != '\0')
    {
        if (*right_part != '\0')
        {
            if (strcmp(curr_object->class_name, right_part) != 0)
            {
                odl_errno = 300;
                status = OdlPrintMessage(message_fname, message_fptr, line_number,
                            "OBJECT and END_OBJECT class identifiers do not match");
            }
        }

        status = OdlValidIdentifier(right_part, "END_OBJECT class", 
                     message_fname, message_fptr,line_number) && status;

    }  /*  End:  "if (*equals != '\0') ..."  */

    return(status);

}  /*  End routine:  "OdlValidEndObjDesc"  */


/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static short OdlValidIdentifier (id_name, id_type, message_fname, message_fptr, line_number)

char *id_name;
char *id_type;
char *message_fname;
FILE *message_fptr;
long line_number;

#else

static short OdlValidIdentifier (char *id_name, char *id_type, 
                                 char *message_fname, FILE *message_fptr, 
                                 long line_number)

#endif
{
    char *message = NULL;
    char *c = {NULL};
    int i;
    short status = {TRUE};

    NewString(message, TB_MAXLINE+strlen(id_name)+strlen(id_type))

    if (id_name == NULL)
    {
        odl_errno = 308;
        sprintf(message, "%s identifier is missing", id_type);
        status = OdlPrintMessage(message_fname, message_fptr, line_number,message);
    }
    else
    {
        StripUnprintables(id_name)

        if (*id_name == '\0')
        {
            odl_errno = 308;
            sprintf(message, "%s identifier is missing", id_type);
            status = OdlPrintMessage(message_fname, message_fptr, line_number, message);
        }
        else
        {
            if (! isalpha(*id_name))
            {
                sprintf(message, 
                        "%s identifier:  \"%s\"  does not begin with a letter",
                        id_type, id_name);
                odl_errno = 308;
                status = OdlPrintMessage(message_fname, message_fptr, line_number, message);
            }
        
            for (c=id_name,i=0; *c != '\0'; ++c)
            {
                if ((*c != '_') && (! isalnum(*c))) ++i;
            }
        
            if (i > 0)
            {
                sprintf(message, 
                        "%s identifier:  \"%s\"  contains %d embedded non-alphanumeric or \"_\" character", 
                        id_type, id_name, i);
                odl_errno = 308;
                if (i > 1) strcat(message, "s");
                status = OdlPrintMessage(message_fname, message_fptr, line_number, message);
            }

        }  /*  End:  "if (*id_name == '\0') ... else ..."  */

    }  /*  End:  "if (id_name == NULL) ... else ..."  */
    
    LemmeGo(message)

    return(status);

}  /*  End routine:  "OdlValidIdentifier"  */


/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static short OdlValidKwd (curr_object, left_part, equals, right_part, 
                           message_fname, message_fptr, line_number)
OBJDESC *curr_object;
char *left_part;
char *equals;
char *right_part;
char *message_fname;
FILE *message_fptr;
long line_number;

#else

static short OdlValidKwd (OBJDESC *curr_object, char *left_part, char *equals,
                          char *right_part, char *message_fname, 
                          FILE *message_fptr, long line_number)

#endif
{
    KEYWORD *keyword = {NULL};
    char *key = {NULL};
    char *message = NULL;
    short status = {TRUE};
    short sfdu_only = {FALSE};
    short found_keyword = {FALSE};

    NewString(message, TB_MAXLINE+strlen(left_part)+strlen(right_part))

    if (*left_part == '=')
    {
        *left_part = '\0';
        odl_errno = 309;
        status = OdlPrintMessage(message_fname, message_fptr, line_number, 
                     "KEYWORD identifier is missing");
    }
    else
    {
        if (*equals == '\0')
        {
            sfdu_only = ((strncmp(left_part, "NJPL", 4) == 0) ||
                            (strncmp(left_part, "CCSD", 4) == 0));

            if (! sfdu_only)
            {
                odl_errno = 309;                
                sprintf(message, 
                       "Missing equals sign after KEYWORD identifier:  \"%s\"",
                       left_part);
                status = OdlPrintMessage(message_fname, message_fptr,
                                         line_number, message);
	    }
        }
        else
        {
            *equals = '\0';
            StripTrailing(left_part, ' ')
        }

        /*  ignore the first character if the keyword is a pointer  */
        key = (*left_part != '^') ? left_part : left_part + 1;

        status = OdlValidIdentifier(key, "KEYWORD", message_fname, 
                                    message_fptr, line_number) && status;

        for (keyword=curr_object->first_keyword;
                ((keyword != NULL) && (! found_keyword));
                    keyword=keyword->right_sibling)
        {
            if (keyword->name != NULL)
                found_keyword = (strcmp(keyword->name, left_part) == 0);
        }

        if (found_keyword)
        {
            sprintf(message, 
                    "Duplicate KEYWORD identifier:  \"%s\"", left_part);
            odl_errno = 309;
            status = OdlPrintMessage(message_fname, message_fptr, line_number,message);
        }
        
    }  /*  End:  "if (*left_part == '=') ... else ..."  */

    if (*right_part != '\0')
    {
        /*  what sort of value do we have?  */
        if ((*right_part != '{') && (*right_part != '('))
        {
            /*  we have a single element */
            status = OdlValidElement(right_part, message_fname, message_fptr, 
                                  line_number, 0) && status;
        }
        else
        {
            /*  we have a value list  */
            status = OdlValidValueList(right_part, message_fname, message_fptr,
                                    line_number) && status;
        }
    }
    else
    if (! sfdu_only)
    {
        sprintf(message, 
                "KEYWORD identifier:  \"%s\"  is missing a VALUE",
                left_part);
        odl_errno = 309;
        status = OdlPrintMessage(message_fname, message_fptr, line_number,message);
    }

    LemmeGo(message)

    return(status);

}  /*  End routine:  "OdlValidKwd"  */


/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static short OdlValidObjDesc (equals, right_part, message_fname,
                              message_fptr, line_number)

char *equals;
char *right_part;
char *message_fname;
FILE *message_fptr;
long line_number;

#else

static short OdlValidObjDesc (char *equals, char *right_part,
                              char *message_fname, FILE *message_fptr,
                              long line_number)

#endif
{
    short status = {TRUE};

    if (*equals == '\0')
    {
        odl_errno = 310;
        status = OdlPrintMessage(message_fname, message_fptr, line_number, 
                     "Missing equals sign after OBJECT statement");
    }

    StripTrailing(right_part, ' ')

    status = OdlValidIdentifier(right_part, "OBJECT class", 
                 message_fname, message_fptr,line_number) && status;

    return(status);

}  /*  End routine:  "OdlValidObjDesc"  */



/*******************/
/*  Local Routine  */
/*******************/

#ifdef _NO_PROTO

static short OdlValidValueList (text, message_fname, message_fptr, line_number)

char *text;
char *message_fname;
FILE *message_fptr;
long line_number;

#else

static short OdlValidValueList (char *text, char *message_fname, 
                                FILE *message_fptr, long line_number)

#endif
{
    char *first_char = {NULL};
    char *last_char = {NULL};
    char save_c;
    int i;
    short status = {TRUE};

    for (i=1,first_char=OdlValueStart(text); *first_char != '\0'; ++i)
    {
        /*  find the end of the current element  */
        last_char = OdlValueEnd(first_char);

        /*  save the next character and terminate the string  */
        save_c = *(++last_char);
        *last_char = '\0';

        /*  validate the current element  */
        OdlValidElement(first_char,message_fname,message_fptr,line_number,i);

        /*  restore the character that was overwritten by the terminator  */
        *last_char = save_c;

        /*  find the start of the next element  */
        first_char = OdlValueStart(last_char);

    }  /*  End:  "for (i=1, ..."  */

    return(status);

}  /*  End routine:  "OdlValidValueList"  */




/*========================================================================*/
/*                                                                        */
/*                        Miscellaneous routines                          */
/*                                                                        */
/*========================================================================*/


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlWildCardCompare                                              */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      05-21-97    sm/LASP - added bug fix by CSC/John Kerich to do    */
/*                            start of line check.                      */
/*      09-13-04    MDC - Removed malloc statements from temp_str and   */
/*                        temp_str2. CopyString does that for you.      */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine compares two text strings, one of which may have   */
/*      wildcard characters in it ('*').                                */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

unsigned short OdlWildCardCompare (wildcard_text, plain_text)

char *wildcard_text;
char *plain_text;

#else

unsigned short OdlWildCardCompare (char *wildcard_text, char *plain_text)

#endif
{
    char *c = {NULL};
    char *substr = {NULL};
    char *tmp_str = {NULL};
    char *tmp_str2 = {NULL};
    char *text_start = {NULL};
    char save_it;
    unsigned long len; 
    unsigned short allrightythen = {FALSE};
    int do_start_of_line_check = 1;
	int str_len = 0; /*added so that we could get string lengths and */
				     /*alloc mem for temp_str and temp_str2*/

    /*  see if we have anything to compare  */
    if ((wildcard_text != NULL) && (plain_text != NULL))
    {
		/* 09-13-04 MDC - No need for malloc statements here. CopyString
		   allocates memory for you.
	    */
/*		str_len = strlen(wildcard_text);
		tmp_str = OaMalloc(sizeof(char) * str_len * 2);
		str_len = strlen(wildcard_text);
		tmp_str = OaMalloc(sizeof(char) * str_len * 2);
		str_len = strlen(plain_text);
		tmp_str2 = OaMalloc(sizeof(char) * str_len * 2);
*/
        /*  all righty then, let's initialize some local variables  */
        allrightythen = TRUE;

        /*  copy the wildcard text  */
        CopyString(tmp_str, wildcard_text)

        /*  strip off leading and trailing quotes  */
        save_it = *tmp_str;
        if ((save_it == '\'') || (save_it == '"'))
        {
            StripLeading(tmp_str, save_it)
            StripTrailing(tmp_str, save_it)
            StripLeading(tmp_str, ' ')
            StripTrailing(tmp_str, ' ')
        }

        /*  copy the plain text  */
        CopyString(tmp_str2, plain_text)

        /*  strip off leading and trailing quotes  */
        save_it = *tmp_str2;
        if ((save_it == '\'') || (save_it == '"'))
        {
            StripLeading(tmp_str2, save_it)
            StripTrailing(tmp_str2, save_it)
            StripLeading(tmp_str2, ' ')
            StripTrailing(tmp_str2, ' ')
        }

        substr = tmp_str;
        if (*substr == '*')  do_start_of_line_check = 0;
        text_start = tmp_str2;
       
        if (strchr(substr, '*') == NULL)
            allrightythen = (strcmp(text_start, substr) == 0);
        else
        {

            /*  we're going to break out the chunks of text between the */
            /*  wildcard caracters and try to find them one-by-one in   */
            /*  the plain text string.                                      */
            do
            {
                /*  locate the start of next substring  */
                for ( ; *substr == '*'; ++substr) ;
                if (*substr == '\0') break;

                /*  locate the end of the substring and save that address  */
                for (c=substr; ((*c != '*') && (*c != '\0')); ++c) ;
                save_it = *c;
                *c = '\0';
    
                /*  look for the substring in the un-wildcarded text  */
                if ((c = (char *)strstr(text_start, substr)) == NULL)
                {
                    allrightythen = FALSE;
                    break;
                }
                else if (do_start_of_line_check)
                {
                    if (c != text_start)
                    {
                        allrightythen = FALSE;
                        break;
                    }
                    do_start_of_line_check = 0;
                }

                /*  prepare for the next search  */
                len = (unsigned long) strlen(substr);
                substr += len;
                *substr = save_it;
                text_start = c + len;
                if ((*substr == '\0') && (*text_start != '\0'))
                {
                    allrightythen = FALSE;
                    break;
                }

            } while(TRUE);

	}  /*  End:  "if (strchr(substr, '*') == NULL) ... else ..."  */

        LemmeGo(tmp_str)
        LemmeGo(tmp_str2)

    }  /*  End:  "if ((wildcard_text != NULL) && ..."  */

    return(allrightythen);

}  /*  End:  "OdlWildCardCompare"  */




/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlTraverseTree                                                 */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates the next object in an ODL tree, stopping   */
/*      when it has traversed the entire tree as defined by the         */
/*      root_level parameter.                                           */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlTraverseTree (curr_object, root_level)

OBJDESC *curr_object;
unsigned long root_level;

#else

OBJDESC *OdlTraverseTree (OBJDESC *curr_object, unsigned long root_level)

#endif
{
    OBJDESC *obj = {NULL};
    OBJDESC *next_object = {NULL};

    if (curr_object != NULL)
    {
        /*  start search with current object's children  */
        if (curr_object->first_child != NULL)
            next_object = curr_object->first_child;
        else
        /*  start search with current object's right sibling  */
        if (curr_object->right_sibling != NULL)
            next_object = curr_object->right_sibling;
        else
        /*  move up parent list until we find one with a right sibling  */
        {
            for (next_object=NULL,obj=curr_object->parent; 
                    (obj != NULL); obj=obj->parent)
            {
                if (obj->level <= root_level) 
                    break;
                else
                if (obj->right_sibling != NULL)
                {
                    next_object = obj->right_sibling;
                    break;    
                }
            }

        }  /*  End:  "if (curr_object->first_child ... else ..."  */

    }  /*  End:  "if (curr_object != NULL) ..."  */
    else
        odl_errno = 1;

    return(next_object);

}  /*  End routine:  "OdlTraverseTree"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlFirstWord                                                    */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns a pointer to the first word in a string    */
/*      of text.  A word is anything that begins with a printable       */
/*      ASCII character.                                                */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlFirstWord(text)

char *text;

#else

char *OdlFirstWord(char *text)

#endif
{
    char *c = {NULL};
    for (c=text; 
          ((c != NULL) && ((*c <= ' ') || (*c > '~')) && (*c != '\0')); ++c) ;
    return(c);

}  /*  End routine:  "OdlFirstWord"  */


	
/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlNextWord                                                     */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates the next word in a string of text.  It     */
/*      locates the end of the current word and skips over whitespace   */
/*      to find the start of the next.  A word is anything that begins  */
/*      with a printable ASCII character.                               */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlNextWord(text)

char *text;

#else

char *OdlNextWord( char *text)

#endif
{
    char *c = {NULL};
    for (c=text; 
          ((c != NULL) && (*c > ' ') && (*c <= '~') && (*c != '\0')); ++c) ;
    return(OdlFirstWord(c));

}  /*  End routine:  "OdlNextWord"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlValueStart                                                   */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates the start of a value within a set or       */
/*      sequence.  A value is anything that doesn't begin with a brace, */
/*      comma, paren, or blank.                                         */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlValueStart(text)

char *text;

#else

char *OdlValueStart(char *text)

#endif
{
    /*  find a character that is not a brace, paren, comma, or blank  */
    return(text + strspn(text, "{}(), \n"));

}  /*  End routine:  "OdlValueStart"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlValueEnd                                                     */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine finds the end of a value within a set or sequence. */
/*      A value is anything that doesn't begin with a brace, comma,     */
/*      paren, or blank.  It finds the end by locating a comma, paren,  */
/*      or brace, and backing up over trailing whitespace.              */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlValueEnd(text)

char *text;

#else

char *OdlValueEnd( char *text)

#endif
{
    char *c = {NULL};

    /*  find a character that is a brace, paren, or comma  */
    c = strpbrk(text, "{}(),");

    /*  backup over any trailing blanks  */
    for (--c; ((c > text) && ((*c == ' ') || (*c == '\0'))); --c) ;

    return(c);

}  /*  End routine:  "OdlValueEnd"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlValueRowStart                                                */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates the start of a set or sequence within a    */
/*      set or sequence.  A set begins with a brace and a sequence      */
/*      begins with a paren.                                            */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlValueRowStart(text)
char *text;

#else

char *OdlValueRowStart( char *text)

#endif
{
    /*  find a character that is not a brace or paren  */
    return(text + strspn(text, "{}()\n"));

}  /*  End routine:  "OdlValueRowStart"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlValueRowEnd                                                  */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine locates the end of a set or sequence within a      */
/*      set or sequence.  A set begins with a brace and a sequence      */
/*      begins with a paren.  It finds a closing brace or paren and     */
/*      backs up over any whitespace.                                   */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlValueRowEnd(text)
char *text;	

#else

char *OdlValueRowEnd( char *text)

#endif
{
    char *c = {NULL};

    /*  find a character that is a brace or paren  */
    c = strpbrk(text, "{}()\n");

    /*  backup over any trailing blanks or commas  */
    for (--c; ((c > text) && ((*c == ' ') || (*c == ',') || (*c == '\0'))); --c) ;

    return(c);

}  /*  End routine:  "OdlValueRowEnd"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlDataType                                                     */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      12-11-95    sm/LASP - '(' for ODL_SEQUENCE, '{' for ODL_SET.    */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine determines the ODL data type of a text string.     */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

unsigned short OdlDataType (text)

char *text;

#else

unsigned short OdlDataType (char *text)

#endif
{
    char *c = {NULL};
    char *u = {NULL};
    char *tempstr = {NULL};
    char *last_pound = {NULL};
    int base;
    unsigned long decimal_count = {0};
    unsigned long hyphen_count = {0};
    unsigned long colon_count = {0};
    unsigned long t_count = {0};
    unsigned short has_sign = {FALSE};
    unsigned short type = {ODL_UNKNOWN};
    unsigned short exponent_type = {ODL_UNKNOWN};
    static char *valid_chars[17] = {
        "",   /* base 0 */
        "0",   /* base 1 */
        "01",   /* base 2 */
        "012",   /* base 3 */
        "0123",   /* base 4 */
        "01234",   /* base 5 */
        "012345",   /* base 6 */
        "0123456",   /* base 7 */
        "01234567",   /* base 8 */
        "012345678",   /* base 9 */
        "0123456789",   /* base 10 */
        "0123456789A",   /* base 11 */
        "0123456789AB",   /* base 12 */
        "0123456789ABC",   /* base 13 */
        "0123456789ABCD",   /* base 14 */
        "0123456789ABCDE",   /* base 15 */
        "0123456789ABCDEF"};  /* base 16 */

    if (text != NULL)
    {
        /*  make a copy of the string  */
        CopyString(tempstr, text)

        /*  remove any units  */
        if ((u = (char *)strchr(tempstr, '<')) != NULL) *u = '\0';

        /*  clean up blanks and convert to upper case  */
        StripLeading(tempstr, ' ')
        StripTrailing(tempstr, ' ')
        UpperCase(tempstr)

        c = tempstr;

        type = ODL_SYMBOL;

        /*  See if we have a double quoted text value  */
        if (*c == '"')
            type = ODL_TEXT;
        else
        /*  See if we have a single quoted text value  */
        if (*c == '\'')
            type = ODL_SYMBOL;
        else
        /*  See if we have a sequence of values  */
        if (*c == '(')
            type = ODL_SEQUENCE;
        else
        /*  See if we have a set of values  */
        if (*c == '{')
            type = ODL_SET;
        else
        /*  See if we have any embedded blanks  */
        if (strchr(c, ' ') != NULL)
            type = ODL_UNKNOWN;
        else
        /*  See if we have an integer, real, date, or date-time value  */
        {
            /*  we're going to check each character  */
            for ( ; *c != '\0'; ++c)
            {
                /*  may have a number  */
                if (isdigit(*c))
                {
                    if (c == tempstr)
                        type = ODL_INTEGER;
                    else
                    if (has_sign && (c == (tempstr + 1)))
                        type = ODL_INTEGER;
                }
                else
                /*  we may have a real number or a date-time  */
                if (*c == '.')
                {
                    /*  we may have a real number  */
                    if (type == ODL_INTEGER)
                        type = ODL_REAL;
                    else
                    /*  date-times can only have one decimal point  */
                    if (type == ODL_DATE_TIME)
                    {
                        if ((++decimal_count) > 1)
                            type = ODL_UNKNOWN;
		    }
                    else
                    /*  we may have a real number  */
                    if (c == tempstr)
                        type = ODL_REAL;
                    else
                    /*  we may have a signed real number  */
                    if (has_sign && (c == (tempstr + 1)))
                        type = ODL_REAL;
                    else
                        type = ODL_UNKNOWN;
                }
                else
                /*  we may have a real number in scientific notation */
                if (*c == 'E')
                {
                    /*  only valid if we thought we had an int or real  */
                    if ((type == ODL_INTEGER) || (type == ODL_REAL))
                    {
                        /*  check out the exponent  */
                        exponent_type = (unsigned short) OdlDataType((c+1));

                        /*  we have a real number only if the exponent  */
                        /*  is real or int                              */
                        if ((exponent_type == ODL_REAL) || 
                            (exponent_type == ODL_INTEGER))
                            type = ODL_REAL;
                        else
                            type = ODL_UNKNOWN;

                        break;
                    }
                    else
                        if (type != ODL_SYMBOL)
                            type = ODL_UNKNOWN;
                }
                else
                /*  we may have a signed number  */
                if (*c == '+')
                {
                    /*  this had better be the first character  */
                    if (c != tempstr)
                        type = ODL_UNKNOWN;
                    else
                        has_sign = TRUE;
                }
                else
                /*  we may have a date or a signed number */
                if (*c == '-')
                {
                    /*  this had better be the first character  */
                    if (c == tempstr)
                        has_sign = TRUE;
                    else
                    /*  a date can have at most two hyphens  */
                    if ((++hyphen_count) > 2)
                        type = ODL_UNKNOWN;
                    else
                    /*  we thought we had an integer  */
                    if (type == ODL_INTEGER)
                        type = ODL_DATE;
                    else
                    /*  if it wasn't an int and it wasn't a date ...  */
                    if (type != ODL_DATE)
                        type = ODL_UNKNOWN;
                }
                else
                /*  we may have a date-time  */
                if (*c == 'T')
                {
                    /*  we thought we had a date  */
                    if (type == ODL_DATE)
                        type = ODL_DATE_TIME;
                    else
                    /*  a date-time may only have one 'T'  */
                    if (type == ODL_DATE_TIME)
                    {
                        if ((++t_count) > 1)
                            type = ODL_UNKNOWN;
		    }
                    else
                    /*  must be a symbol  */
                    if (type != ODL_SYMBOL)
                        type = ODL_UNKNOWN;
                }
                else
                /*  we may have a date-time  */
                if (*c == 'Z')
                {
                    /*  only a date time may contain a 'Z'  */
                    if (type == ODL_DATE_TIME)
                    {
                        /*  it had better be the last char in the string  */
                        if (*(c + 1) != '\0')
                            type = ODL_UNKNOWN;
		    }
                    else
                    if (type != ODL_SYMBOL)
                        type = ODL_UNKNOWN;
                }
                else
                /*  we may have a date-time  */
                if (*c == ':')
                {
                    /*  only a date-time may contain colons  */
                    if (type != ODL_DATE_TIME)
                        type = ODL_UNKNOWN;
                    else
                    /*  there can't be more than two of them  */
                    if ((++colon_count) > 2)
                        type = ODL_UNKNOWN;
                    else
                    /*  characters on either side must be digits  */
                    if ((! isdigit(*(c-1))) || (! isdigit(*(c+1))))
                        type = ODL_UNKNOWN;
                    else
                    /*  decimal points can't occur before the last colon  */
                    if (decimal_count > 0)
                        type = ODL_UNKNOWN;
                }
                else
                /*  we may have a non-decimal integer  */
                if (*c == '#')
                {
                    /*  we didn't think it WAS an integer  */
                    if (type != ODL_INTEGER)
                        type = ODL_UNKNOWN;
                    else
                    /*  the base can't be signed  */
                    if (has_sign)
                        type = ODL_UNKNOWN;
                    else
                    /*  missing the closing '#' character  */
                    if ((last_pound = (char *)strchr(c+1, '#')) == NULL)
                        type = ODL_UNKNOWN;
                    else
                    /*  closing '#' char is not at the end  */
                    if (*(last_pound + 1) != '\0')
                        type = ODL_UNKNOWN;
                    else
                    {
                        /*  looks good so far, but we have to make sure  */
                        /*  stuff between the '#' characters is valid    */
                        /*  with the specified base                      */

                        /*  isolate the base  */
                        *c = '\0'; 
                        base = atoi(tempstr);

                        /*  ignore the integer's sign  */
                        ++c;
                        if ((*c == '+') || (*c == '-')) ++c;

                        /*  isolate the number part  */
                        *last_pound = '\0';

                        /*  valid bases are 2 through 16, inclusive  */
                        if ((base < 2) || (base > 16))
                            type = ODL_UNKNOWN;
                        else
                        /*  look for invalid digits for the specified base  */
                        if (c[strspn(c, valid_chars[base])] != '\0')
                            type = ODL_UNKNOWN;
                    }

                    if (type != ODL_UNKNOWN) 
                    {
                        type = ODL_INTEGER;
                        break;
		    }
                }
                else
                /*  we may have an unquoted symbol  */
                if (isalpha(*c) || (*c == '_'))
                {
                    if (type != ODL_SYMBOL)
                        type = ODL_UNKNOWN;
                }
                else
                /*  we havn't got a clue  */
                    type = ODL_UNKNOWN;

                if (type == ODL_UNKNOWN)
                {
                    odl_errno = 311;
                    break;
		}

            }  /*  End:  "for ( ; ..."  */

            if (has_sign && (type != ODL_INTEGER) && (type != ODL_REAL))
                type = ODL_UNKNOWN;

        }  /*  End:  "if (*c == '(') ... else ..."  */

		
        LemmeGo(tempstr); 

    }  /*  End:  "if (text != NULL) ..."  */

    return(type);

}  /*  End:  "OdlDataType"  */


/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlTypeString                                                   */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine determines the ODL data type of a text string.     */
/*                                                                      */
/************************************************************************/

#ifdef _NO_PROTO

char *OdlTypeString (type, type_string)

unsigned short type;
char *type_string;

#else

char *OdlTypeString (unsigned short type, char *type_string)

#endif
{
    static char local_type_string [TB_MAXLINE];

    switch (type)
    {
        case ODL_INTEGER   : strcpy(local_type_string, "INTEGER");
                             break;

        case ODL_REAL      : strcpy(local_type_string, "REAL");
                             break;

        case ODL_SYMBOL    : strcpy(local_type_string, "SYMBOL");
                             break;

        case ODL_TEXT      : strcpy(local_type_string, "TEXT");
                             break;

        case ODL_DATE      : strcpy(local_type_string, "DATE");
                             break;

        case ODL_DATE_TIME : strcpy(local_type_string, "DATE-TIME");
                             break;

        case ODL_SEQUENCE  : strcpy(local_type_string, "SEQUENCE");
                             break;

        case ODL_SET       : strcpy(local_type_string, "SET");
                             break;

        default            : strcpy(local_type_string, "UNKNOWN");
                             odl_errno = 311;
                             break;

    }  /*  End:  "switch (type) ..."  */

    if (type_string != NULL) strcpy(type_string, local_type_string);

    return(local_type_string);

}  /*  End:  "OdlTypeString"  */



/************************************************************************/
/*                                                                      */
/*  Component:                                                          */
/*                                                                      */
/*      OdlTempFname                                                    */
/*                                                                      */
/*  Author:                                                             */
/*                                                                      */
/*      David P. Bernath (Jet Propulsion Laboratory)                    */
/*                                                                      */
/*  Version:                                                            */
/*                                                                      */
/*      1.0    March 31, 1994                                           */
/*                                                                      */
/*  Change History:                                                     */
/*                                                                      */
/*      03-31-94    Original code                                       */
/*      11-02-94    LASP/Steve Monk - Added more #ifdefs for additional */
/*                                    systems.                          */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*  Description:                                                        */
/*                                                                      */
/*      This routine returns a unique file name, depending on the       */
/*      system on which the code was compiled.                          */
/*                                                                      */
/************************************************************************/

char *OdlTempFname()
{
    FILE *fptr = {NULL};
    char *fname = {NULL};
    char temp_str  [TB_MAXPATH + TB_MAXFNAME];
    char base_name [TB_MAXPATH + TB_MAXFNAME];
#if defined( MSDOS) || defined( IBM_PC)
    time_t t;
#endif

    strcpy(base_name, "tmp.tmp");

#if defined( SUN_UNIX) || defined( SUN4) || defined( SGI) || defined( ULTRIX) || defined( ALPHA_OSF)
    tmpnam(temp_str);
    strcpy( base_name, temp_str);  /* Bug fix 11/2/94 SM                     */
                                   /* Was:    sprintf(base_name, "~/%s.tmp", */
                                   /*                 temp_str);             */
#endif

#if (defined( VAX) || defined( ALPHA_VMS))
    tmpnam(temp_str);
    sprintf(base_name, "sys$login:%s.tmp", temp_str);
#endif

#if defined( MAC_THINK) || defined( MAC)
    tmpnam(temp_str);
    sprintf(base_name, "%s.tmp", temp_str);
#endif

#if defined( MSDOS) || defined( IBM_PC)
    {
        temp_str[0] = '\0';  /* Variable not used; prevent compiler warning. */
        t = (time_t) time(NULL);
        sprintf(base_name, "C:\\%ld", t);
        base_name[8] = '\0';
        strcat(base_name, ".tmp");
    }
#endif

    CopyString(fname, base_name)

    if ((fptr = (FILE *) fopen(fname, "w")) == NULL)
        LemmeGo(fname)
    else
        CloseMe(fptr)

    return(fname);

}  /*  End:  "OdlTempFname"  */


#ifdef _NO_PROTO

short CheckBalance(text)
char *text;

#else

short CheckBalance( char *text)

#endif
{
    long quote_nesting = 0;
    long brace_nesting = 0;
    long paren_nesting = 0;
    char *c = {NULL};
    char *c1 = {NULL};

    c1 = (char *) strchr(text, '=');
    c = OdlFirstWord(c1 + 1);

    if ((*c == '(') || (*c == '{'))
        OdlNestingLevel(c,&brace_nesting,&paren_nesting);
    else
        if (*c == '"')
        {
            for (; *c != '\0'; ++c)
            {
                if (*c == '"') 
                {
                    if (quote_nesting == 0)
                        quote_nesting = 1;
                    else
                        quote_nesting = 0;
	        }
            }
        }

    return((brace_nesting + paren_nesting + quote_nesting) == 0);
}

/*========================================================================*/
/*                                                                        */
/*                       End of lablib 3.0 stuff                          */
/*                                                                        */
/*========================================================================*/
