# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2010 Red Hat, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
# USA

__all__ = (
        "get_language_name",
    )

import json
import locale
import gettext

_ = lambda a: gettext.dgettext("ibus", a)
__languages_dict = {}

def get_language_name(_locale):
    lang = _locale.split("_")[0]
    lang = lang.lower()
    if lang in __languages_dict:
        lang = __languages_dict[lang]
        lang = gettext.dgettext("iso_639-3", lang)
    else:
        lang = _(u"Other")
        lang = gettext.dgettext("ibus", lang)
    return lang

def __load_lang():
    import os
    import _config
    iso_639_json = os.path.join(_config.ISOCODES_PREFIX, "share/iso-codes/json/iso_639-3.json")
    if os.path.isfile(iso_639_json):
        with open(iso_639_json, "r") as json_file:
            iso_639_data = json.load(json_file)
            for lang in iso_639_data["639-3"]:
                name = lang["name"]
                for attr_name in (u"alpha_2", u"alpha_3"):
                    if attr_name in lang:
                        attr_value = lang[attr_name]
                        __languages_dict[attr_value] = name
    else:
        print("Error json file", iso_639_json, "does not exist")

__load_lang()

if __name__ == "__main__":
    print get_language_name("mai")
    print get_language_name("zh")
    print get_language_name("ja")
    print get_language_name("ko")
