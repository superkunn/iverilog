/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: a_fetch_value.c,v 1.1 2003/04/12 18:57:14 steve Exp $"
#endif

# include  <acc_user.h>
# include  <vpi_user.h>
# include  "priv.h"
# include  <assert.h>

static char* fetch_struct_value(handle obj, s_acc_value*value)
{
      struct t_vpi_value val;

      switch (value->format) {

	  case accScalarVal:
	    val.format = vpiScalarVal;
	    vpi_get_value(obj, &val);
	    value->value.scalar = val.value.scalar;
	    break;

	  case accIntVal:
	    val.format = vpiIntVal;
	    vpi_get_value(obj, &val);
	    value->value.integer = val.value.integer;
	    break;

	  case accRealVal:
	    val.format = vpiRealVal;
	    vpi_get_value(obj, &val);
	    value->value.real = val.value.real;
	    break;

	  default:
	    vpi_printf("XXXX acc_fetch_value(..., \"%%%%\", <%d>);\n",
		       value->format);
	    value->value.str = "<string value>";
	    break;
      }

      return 0;
}

static char* fetch_strength_value(handle obj)
{
      struct t_vpi_value val;

      val.format = vpiStrengthVal;
      vpi_get_value(obj, &val);

      vpi_printf("XXXX acc_fetch_value(..., \"%%v\")\n");
      return __acc_newstring("StX");
}

char* acc_fetch_value(handle obj, const char*fmt, s_acc_value*value)
{
      if (strcmp(fmt, "%%") == 0)
	    return fetch_struct_value(obj, value);

      vpi_printf("XXXX acc_fetch_value(..., \"%s\", ...)\n", fmt);
      return "<acc_fetch_value>";
}


/*
 * $Log: a_fetch_value.c,v $
 * Revision 1.1  2003/04/12 18:57:14  steve
 *  More acc_ function stubs.
 *
 */

