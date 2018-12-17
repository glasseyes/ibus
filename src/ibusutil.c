/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2015 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2010-2017 Takao Fujiwara <takao.fujiwara1@gmail.com>
 * Copyright (C) 2008-2016 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <string.h>
#include "ibusxml.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#endif

/* gettext macro */
#define N_(t) t

static GHashTable *__languages_dict;

void get_detail_from_object(JsonObject *object, const gchar *member_name, gchar **detail)
{
    const gchar *value;
    JsonNode *member_node;

    member_node = json_object_get_member(object, member_name);
    if (member_node != NULL && JSON_NODE_HOLDS_VALUE (member_node)) {
        value = json_object_get_string_member (object, member_name);
        *detail = g_strdup(value);
    }
    else {
        *detail = NULL;
    }
}

void languages_foreach (JsonArray *array,
                     guint index_,
                     JsonNode *element_node,
                     gpointer user_data)
{
    // Get name, alpha_2 and alpha_3
    // insert into __languages_dict alpha_X : name
    JsonObject *language_object;
    gchar *name = NULL, *alpha_2 = NULL, *alpha_3 = NULL;

    if (JSON_NODE_HOLDS_OBJECT(element_node)) {
        language_object = json_node_get_object(element_node);

        get_detail_from_object(language_object, "name", &name);
        if (name) {
            get_detail_from_object(language_object, "alpha_2", &alpha_2);
            get_detail_from_object(language_object, "alpha_3", &alpha_3);

            if (alpha_3) {
                g_hash_table_insert (__languages_dict,
                                         (gpointer) g_strdup (alpha_3),
                                         (gpointer) g_strdup (name));
                g_free(alpha_3);
                alpha_3 = NULL;
            }
            if (alpha_2) {
                g_hash_table_insert (__languages_dict,
                                         (gpointer) g_strdup (alpha_2),
                                         (gpointer) g_strdup (name));
                g_free(alpha_2);
                alpha_2 = NULL;
            }
            g_free(name);
            name = NULL;
        }
    }
}

static gboolean
_iso_codes_parse_json (JsonParser *parser)
{
    GList *p;
    JsonNode *root, *corenode;
    JsonObject *root_object;
    JsonArray *languages;
    g_assert (parser);
    root = json_parser_get_root (parser);

    if (G_UNLIKELY (JSON_NODE_HOLDS_OBJECT(root) != TRUE)) {
        return FALSE;
    }

    root_object = json_node_get_object(root);
    corenode = json_object_get_member(root_object, "639-3");

    if (G_UNLIKELY (corenode == NULL)) {
        return FALSE;
    }
    if (G_UNLIKELY (JSON_NODE_HOLDS_ARRAY(corenode) != TRUE)) {
        return FALSE;
    }

    languages = json_node_get_array (corenode);
    if (G_UNLIKELY (languages == NULL)) {
        return FALSE;
    }

    json_array_foreach_element (languages, languages_foreach, NULL);

    return TRUE;
}

void
_load_lang()
{
    gchar *filename;
    JsonParser *parser;
    GError *error = NULL;
    struct stat buf;

#ifdef ENABLE_NLS
    bindtextdomain ("iso_639-3", LOCALEDIR);
    bind_textdomain_codeset ("iso_639-3", "UTF-8");
#endif

    __languages_dict = g_hash_table_new_full (g_str_hash,
            g_str_equal, g_free, g_free);
    filename = g_build_filename (ISOCODES_PREFIX,
                                 "share/iso-codes/json/iso_639-3.json",
                                 NULL);
    if (g_stat (filename, &buf) != 0) {
        g_warning ("Can not get stat of file %s", filename);
        g_free (filename);
        return;
    }

    parser = json_parser_new ();

    json_parser_load_from_file (parser, filename, &error);
    if (error) {
      g_warning ("Unable to parse `%s': %s\n", filename, error->message);
      g_free(filename);
      g_error_free (error);
      g_object_unref (parser);
      return;
    }

    _iso_codes_parse_json (parser);
    g_object_unref(parser);
}

const gchar *
ibus_get_untranslated_language_name (const gchar *_locale)
{
    const gchar *retval;
    gchar *p = NULL;
    gchar *lang = NULL;

    if (__languages_dict == NULL )
        _load_lang();
    if ((p = strchr (_locale, '_')) !=  NULL)
        p = g_strndup (_locale, p - _locale);
    else
        p = g_strdup (_locale);
    lang = g_ascii_strdown (p, -1);
    g_free (p);
    retval = (const gchar *) g_hash_table_lookup (__languages_dict, lang);
    g_free (lang);
    if (retval != NULL)
        return retval;
    else
        return "Other";
}

const gchar *
ibus_get_language_name (const gchar *_locale)
{
    const gchar *retval = ibus_get_untranslated_language_name (_locale);

#ifdef ENABLE_NLS
    if (g_strcmp0 (retval, "Other") == 0)
        return dgettext (GETTEXT_PACKAGE, N_("Other"));
    else
        return dgettext ("iso_639-3", retval);
#else
    return retval;
#endif
}

void
ibus_g_variant_get_child_string (GVariant *variant, gsize index, char **str)
{
    g_return_if_fail (str != NULL);

    g_free (*str);
    g_variant_get_child (variant, index, "s", str);
}
