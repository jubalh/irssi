/*
 recode.c : irssi

    Copyright (C) 1999-2000 Timo Sirainen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "module.h"
#include "settings.h"
#include "lib-config/iconfig.h"

#ifdef HAVE_GLIB2
static gboolean recode_get_charset(const char **charset)
{
	*charset = settings_get_str("term_charset");
	if (**charset)
		/* we use the same test as in src/fe-text/term.c:123 */
			return !g_strcasecmp(*charset, "utf-8");
			
	return g_get_charset(charset);
}
#endif

char *recode_in(const char *str, const char *target)
{
#ifdef HAVE_GLIB2
	const char *from = NULL;
	const char *to = NULL;
	char *recoded = NULL;
	gboolean term_is_utf8;
	gboolean str_is_utf8;
	gboolean translit;
	int len;
	
	if (!str)
		return g_strdup(str);
	
	len = strlen(str);
	
	str_is_utf8 = g_utf8_validate(str, len, NULL);

	translit = settings_get_bool("recode_transliterate");
	
	/* check if the str is UTF-8 encoded as first */
	if (target && !str_is_utf8)
		from = iconfig_get_str("conversions", target, NULL);
	else if (target && str_is_utf8)
		from = "UTF-8";

	term_is_utf8 = recode_get_charset(&to);
	
	if (translit)
		to = g_strdup_printf("%s//TRANSLIT", to);
	
	if (from)
		recoded = g_convert(str, len, to, from, NULL, NULL, NULL);

	if (!recoded) {
		if (term_is_utf8) {
			if (!str_is_utf8)
				from = settings_get_str("recode_fallback");
				
		} else if (str_is_utf8)
			from = "UTF-8";
		
		if (from)
			recoded = g_convert(str, len, to, from, NULL, NULL, NULL);

		if (!recoded)
			recoded = g_strdup(str);
	}
	return recoded;
#else
	return g_strdup(str);
#endif
}

char *recode_out(const char *str, const char *target)
{
#ifdef HAVE_GLIB2
	char *recoded = NULL;
	const char *from = NULL;
	gboolean translit;
	gboolean term_is_utf8;
	int len;
	
	if (!str)
		return g_strdup(str);
	
	len = strlen(str);
	
	translit = settings_get_bool("recode_transliterate");
	if (target) {
		const char *to = NULL;

		to = iconfig_get_str("conversions", target, NULL);
		if (!to)
			/* default outgoing charset if no conversion is set */
			to = settings_get_str("recode_out_default_charset"); 
		if (to) {
			if (translit)
				to = g_strdup_printf("%s//TRANSLIT", to);
				
			term_is_utf8 = recode_get_charset(&from);
			recoded = g_convert(str, len, to, from, NULL, NULL, NULL);
		}
	}
	if (!recoded)
		recoded = g_strdup(str);

	return recoded;
#else
	return g_strdup(str);
#endif
}

void recode_init(void)
{
	settings_add_str("misc", "recode_fallback", "ISO8859-1");
	settings_add_str("misc", "recode_out_default_charset", "");
	settings_add_bool("misc", "recode_transliterate", TRUE);
}

void recode_deinit(void)
{
	settings_remove("recode_fallback");
	settings_remove("recode_out_default_charset");
	settings_remove("recode_transliterate");
}