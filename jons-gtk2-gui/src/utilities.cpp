/*
 * Licq GTK GUI Plugin
 *
 * Copyright (C) 2000, Jon Keating <jon@licq.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <gtk/gtk.h>
#include <cstring>
#include <string>

char *
convert_to_utf8(const char *input_text, const char *input_enc)
{
	if (input_text == NULL)
  	return NULL;
  
  if (input_text[0] == 0)
  	return g_strdup(input_text);
    
  size_t len = strlen(input_text);
  if (!g_utf8_validate(input_text, len, NULL)) {
  	gsize b_in, b_out;
		if (input_enc != NULL && *input_enc != 0 && 
    		strcasecmp(input_enc, "UTF-8") != 0)
			return g_convert(input_text, len, "UTF-8", input_enc, &b_in, &b_out,
      		NULL);
		else {
			const char *cs;
			if (g_get_charset(&cs)) 
      	// locale is already utf8 so conversion won't help - we use 
        // fallback character set - iso8859-1
				return g_convert(input_text, len, "UTF-8", "ISO8859-1", &b_in, &b_out,
        		NULL);
			else
				return g_convert(input_text, len, "UTF-8", cs, &b_in, &b_out, NULL);
		}
	}
	
	return g_strdup(input_text);
}

std::string
s_convert_to_utf8(const char *input_text, const char *input_enc)
{
	if (input_text == NULL || input_text[0] == '\0')
		return std::string("");
	char *p = convert_to_utf8(input_text, input_enc);
	std::string ret("");
	if (p != NULL) {
		ret = p;
		g_free(p);
	}
	return ret;
}

std::string
entry_get_chars(GtkWidget *w)
{
	char *et = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);
	if (et != NULL) {
		std::string s(et);
		g_free(et);
		return s;
	}
	return std::string();
}

