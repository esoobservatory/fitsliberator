%{

/*****************************************************************************

 Description: This file contains the parser for the Object Description
              Language (ODL).  The parser is produced using Yacc and
              all changes to the parsing scheme should be made by
              modifying the Yacc input file rather than the
              C-language code produced from by Yacc.

 Author:  Randy Davis, University of Colorado LASP

 Creation Date: 17 April 1990
 Last Modified: 18 May 1991

 History:

   Creation - This module was introduced in the Version 1 ODLC library.

   Version 2.0 - 30 August 1990 - R. Davis, U. of Colorado LASP
     a) Modified to comply with ODL Version 2.  This includes adding
        support for GROUP statements.

   Version 2.0.1 - 26 November 1990 - R. Davis, U. of Colorado LASP
     a) Changed parsing scheme to provide better error reporting and
        recovery.

   Version 2.1 - 13 March 1991
     a) Modified calls to parser action routines to pass pointers to
        value structures rather than copying in the entire structure.

  Version 2.2 - 18 May 1991 - M. DeMore, Jet Propulsion Laboratory
    Added include file odlinter.h.

  Version 2.2 - 18 May 1991 - M. DeMore, Jet Propulsion Laboratory
    Added include file odlinter.h. Added pds_watch_ends flag for
    toolbox users, to eliminate the "missing END" warning.

  Version 2.3 - 10 July 1992 - M. DeMore, Jet Propulsion Laboratory
    Added handling for semicolons.

  Version 2.4 - 17 Oct 2003 - D. Schultz, Jet Propulsion Laboratory
    Added handling for NULL values and name space ie: MISSION:TARGET

*****************************************************************************/

/*****************************************************************************
 To allow the use of the resulting file on multiple platforms, the following
 changes should be performed on the resulting C file after its generation
 by the Sun Version of Yacc:

    1) Delete the line

          extern char *malloc(), *realloc();

*****************************************************************************/


#include "odldef.h"
#include "odlinter.h"

#ifdef PDS_TOOLBOX
    extern int pds_watch_ends;
#else
    int pds_watch_ends = TRUE;
#endif

/* A global flag telling whether we're inside a units expression. If so,
   we don't want "." interpreted as part of a value. Instead, it's a synomym
   for "*" (_multiply_operator). */

int in_units = 0;


/* A special lexer function that respects whether we're inside a units
   expression. */

extern int yylex_lvtool(int units_flag);

#define yylex() yylex_lvtool(in_units)

%}


%start        label

%union        {
               struct Value_Data  item;
               int                flag;
	      }

%token        _OBJECT
%token        _END_OBJECT
%token        _GROUP
%token        _END_GROUP
%token        _END

%token        _semi
%token        _sequence_opening
%token        _sequence_closing
%token        _set_opening
%token        _set_closing
%token        _units_opening
%token        _units_closing
%token        _list_separator
%token        _point_operator
%token        _assignment_operator
%token        _multiply_operator
%token        _alt_multiply_operator
%token        _divide_operator
%token        _exponentiate_operator
%token        _range_operator

%token <item> _date
%token <item> _date_time
%token <item> _date_timeV0
%token <item> _integer
%token <item> _name
%token <item> _real
%token <item> _symbol
%token <item> _text_string
%token <item> _time
%token <item> _null_str

%type  <item> integer_value
%type  <item> group_closing
%type  <item> object_closing
%type  <item> units
%type  <item> x_units
%type  <item> name_sequence
%type  <item> units_exponent
%type  <item> null_value
%type  <flag> units_mult_op


%%


label                  : statement_list
                          {
                            /* End-of-file hit before END statement found */
                            ODLEndLabel ();
                            if (pds_watch_ends)
                            {
                               yyerror ("END statement is missing");
                               YYABORT;
			    }
                            else
                               YYACCEPT;
                          }

 statement_list        : statement 
                       | statement_list statement

  statement            : aggregation_stmt
                       | assignment_stmt
                       | end_statement
                       | error statement

   aggregation_stmt    : object_stmt
                       | group_stmt
                       ;

   assignment_stmt     : attribute_stmt
                       | pointer_stmt
                       | attribute_stmt _semi
                       | pointer_stmt _semi
                       ;

   end_statement       
                       : _END
                          {
                            /* This is the normal termination of parsing */
                            if (ODLEndLabel ())
                              {
                                YYACCEPT;
                              }
                            else
                              {
                                YYABORT;
                              }
                          }  

    object_stmt        : object_opening
                       | object_closing
                       | object_opening _semi
                       | object_closing _semi
                       ;

    object_opening     : _OBJECT
                           { yyerror ("Missing '=' operator after OBJECT"); }
                       | _OBJECT  _assignment_operator  _name
                           { ODLBeginAggregate (KA_OBJECT, &$3); }

    object_closing     : _END_OBJECT
                           { 
                             $$.value.string = NULL;
                             ODLEndAggregate (KA_OBJECT, &$$);
                           }
                       | _END_OBJECT  _assignment_operator  _name
                           { ODLEndAggregate (KA_OBJECT, &$3); }


   group_stmt          : group_opening
                       | group_closing
                       | group_opening _semi
                       | group_closing _semi
                       ;

    group_opening      : _GROUP
                           { yyerror ("Missing '=' operator after GROUP"); }
                       | _GROUP  _assignment_operator  _name
                           { ODLBeginAggregate (KA_GROUP, &$3); }

    group_closing      : _END_GROUP
                           { 
                             $$.value.string = NULL;
                             ODLEndAggregate (KA_GROUP, &$$);
                           }
                       | _END_GROUP  _assignment_operator  _name
                           { ODLEndAggregate (KA_GROUP, &$3); }
               

  attribute_stmt       : _name  _assignment_operator
                           { ODLBeginParameter (KP_ATTRIBUTE, &$1); }
                         value
                       | _name  _assignment_operator  error
                           { 
                             yyerror ("Bad value in assignment statement");
                             yyclearin;
                           }
                       | _name error
                           { 
                              yyerror ("Expected '=' after name");
                              free ($1.value.string);
                           }


  pointer_stmt         : _point_operator  _name  _assignment_operator
                           { ODLBeginParameter (KP_POINTER, &$2); }
                         value
                       ;

   value               : scalar_value
                            { ODLMarkParameter (KV_SCALAR); }
                       | sequence_value
                            { ODLMarkParameter (KV_SEQUENCE); }
                       | set_value
                            { ODLMarkParameter (KV_SET); }
                       | range_value
                            { ODLMarkParameter (KV_SEQUENCE); }
                       ;

    scalar_value       : integer_value
                       | real_value
                       | date_time_value
                       | symbolic_value
                       | text_string_value
                       | null_value
                       ;
                       
     integer_value     : _integer
                            { ODLStoreValue (&$1); }
                         units_part
                       ;

     real_value        : _real
                            { ODLStoreValue (&$1); }
                         units_part
                       ;
                       
     null_value	       : _null_str
                            { ODLStoreValue (&$1); }
                         units_part
                        ;
     				

      units_part       :       
                       | units_expression
                       ;

      units_expression : _units_opening
                           { in_units = 1; }
                         units_expr
                         _units_closing
                           { in_units = 0; }
                       ;

       units_expr      : units_factor
                       | units_expr  units_mult_op  x_units_factor
                           { ODLMarkUnits ($2); }

        units_factor   : units
                           { ODLStoreUnits1 (&$1); }
                       | units  units_exp_op  units_exponent
                           { ODLStoreUnits2 (&$1, &$3); }
                       | _sequence_opening  units_expr  _sequence_closing
                       ;

         units         : name_sequence
                       | error  units_expression
                           { yyerror ("Units designator must be a name (alphanumeric)"); }

        /* Same as units_factor, but used for the right-hand-side of
           a multiplicative expression, and allows integers, so that
           units like <LOCALDAY/24> are supported. */
        x_units_factor : x_units
                           { ODLStoreUnits1 (&$1); }
                       | x_units  units_exp_op  units_exponent
                           { ODLStoreUnits2 (&$1, &$3); }
                       | _sequence_opening  units_expr  _sequence_closing
                       ;

         /* Same as units, but allows integers. See x_units_factor, above. */
         x_units       : name_sequence
                       | _integer
                           {
                            /* We must convert the integer value to
                               a string value. Our temporary buffer
                               must be big enough to hold the largest
                               possible integer. */

                            char value[20];
                            sprintf(value, "%ld", $1.value.integer.number);
                            $$ = ODLConvertSymbol(value, strlen(value), 1);
                           }
                       | error  units_expression
                           { yyerror ("Units designator must be a name (alphanumeric) or integer"); }
                       ;

name_sequence          : _name
                       | name_sequence _name
                           {
                            /* Concatenate the names together, separated by
                               a space, up to a maximum of 1000 characters. */
                            char temp[1000+1];
                            strncpy(temp, $1.value.string, sizeof(temp)-1);
                            temp[sizeof(temp)-1] = '\0';
                            strncat(temp, " ", sizeof(temp)-1-strlen(temp));
                            strncat(temp, $2.value.string,
                                    sizeof(temp)-1-strlen(temp));
                            $$ = ODLConvertSymbol(temp, strlen(temp), 1);
                           }
                       ;

         units_mult_op : _multiply_operator
                           { $$ = 1; }
                       | _alt_multiply_operator
                           { $$ = 1; }
                       | _divide_operator
                           { $$ = -1; }
                       | error  units_expression
                           { yyerror ("Expected a '*', '/' or '**' operator");}
  
         units_exp_op  : _exponentiate_operator
                       ;

         units_exponent: _integer
                       | error  units_expression
                         { yyerror("Exponent in units expression must be integer");}

     date_time_value   : _date
                            { ODLStoreValue (&$1); } 
                       | _time
                            { ODLStoreValue (&$1); } 
                       | _date_time
                            { ODLStoreValue (&$1); } 
                       | _date_timeV0 time_zoneV0
                            { ODLStoreValue (&$1); }

      time_zoneV0      :
                       | _units_opening _name _units_closing
                       ;

     symbolic_value    : _name
                            { ODLStoreValue (&$1); }
                       | _symbol
                            { ODLStoreValue (&$1); }

     text_string_value : _text_string
                            { ODLStoreValue (&$1); }
 
    sequence_value     : sequence_1D
                       | sequence_2D
                       ;

     sequence_1D       : _sequence_opening              _sequence_closing
                           { yyerror("Sequences with no values not allowed"); }
                       | _sequence_opening  value_list  _sequence_closing
                           { ODLCheckSequence (); }
                       | _sequence_opening  value_list  error
                           { 
                             yyerror("')' at end of a sequence is missing");
                             ODLCheckSequence ();
                           }

      value_list       : scalar_value
                       | value_list  _list_separator  scalar_value
                       | error
                          { yyerror ("Error in value list"); }

     sequence_2D       : _sequence_opening  sequence_1D_list  _sequence_closing
                       ;

      sequence_1D_list : sequence_1D
                       | sequence_1D_list  list_separator_opt  sequence_1D
                       ;

list_separator_opt     : /*empty*/
                       | _list_separator
                       ;

   set_value           : _set_opening              _set_closing
                       | _set_opening  value_list  _set_closing
                       | _set_opening  value_list  error
                           { yyerror ("The '}' is missing at end of set"); }


   range_value         : integer_value _range_operator integer_value
                            { ODLCheckRange (&$1, &$3); }
                       ;

%%


/* Error handling routine.  This routine is called, explicitly or
   implicitly, whenever a syntax error is detected.                         */

yyerror (error_msg)
  char *error_msg;                 /* Error message text                    */

{
  ODLPrintError (error_msg);

  return;
}
