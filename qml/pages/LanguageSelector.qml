// Parts of this file are generated by generate_language_gui.py

import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {

    id: main
    allowedOrientations : Orientation.All

    property string title
    property string comment
    property string note
    property string value

    property var callback

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingSmall
            anchors.margins: Theme.horizontalPageMargin

            DialogHeader {
                title: main.title
            }

            Label {
                text: main.comment
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                //font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            Label {
                text: main.note
                x: Theme.horizontalPageMargin
                width: parent.width-2*x
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
            }

            // Language switches
	    

            TextSwitch {
                id: input_af
                width: parent.width
                text: "Afrikaans (af, Afrikaans)"
            }

            TextSwitch {
                id: input_ar
                width: parent.width
                text: "العربية (ar, Arabic)"
            }

            TextSwitch {
                id: input_az
                width: parent.width
                text: "azərbaycan dili (az, Azerbaijani)"
            }

            TextSwitch {
                id: input_be
                width: parent.width
                text: "беларуская (be, Belarusian)"
            }

            TextSwitch {
                id: input_bg
                width: parent.width
                text: "български (bg, Bulgarian)"
            }

            TextSwitch {
                id: input_br
                width: parent.width
                text: "brezhoneg (br, Breton)"
            }

            TextSwitch {
                id: input_bs
                width: parent.width
                text: "bosanski (bs, Bosnian)"
            }

            TextSwitch {
                id: input_ca
                width: parent.width
                text: "català (ca, Catalan)"
            }

            TextSwitch {
                id: input_cs
                width: parent.width
                text: "čeština (cs, Czech)"
            }

            TextSwitch {
                id: input_cy
                width: parent.width
                text: "Cymraeg (cy, Welsh)"
            }

            TextSwitch {
                id: input_da
                width: parent.width
                text: "dansk (da, Danish)"
            }

            TextSwitch {
                id: input_de
                width: parent.width
                text: "Deutsch (de, German)"
            }

            TextSwitch {
                id: input_el
                width: parent.width
                text: "Ελληνικά (el, Greek)"
            }

            TextSwitch {
                id: input_en
                width: parent.width
                text: "English (en, English)"
            }

            TextSwitch {
                id: input_es
                width: parent.width
                text: "español (es, Spanish)"
            }

            TextSwitch {
                id: input_et
                width: parent.width
                text: "eesti (et, Estonian)"
            }

            TextSwitch {
                id: input_eu
                width: parent.width
                text: "euskara (eu, Basque)"
            }

            TextSwitch {
                id: input_fa
                width: parent.width
                text: "فارسی (fa, Persian)"
            }

            TextSwitch {
                id: input_fi
                width: parent.width
                text: "suomi (fi, Finnish)"
            }

            TextSwitch {
                id: input_fil
                width: parent.width
                text: "Filipino (fil, Filipino)"
            }

            TextSwitch {
                id: input_fr
                width: parent.width
                text: "français (fr, French)"
            }

            TextSwitch {
                id: input_ga
                width: parent.width
                text: "Gaeilge (ga, Irish)"
            }

            TextSwitch {
                id: input_gd
                width: parent.width
                text: "Gàidhlig (gd, Scottish Gaelic)"
            }

            TextSwitch {
                id: input_gl
                width: parent.width
                text: "galego (gl, Galician)"
            }

            TextSwitch {
                id: input_gsw
                width: parent.width
                text: "Schwiizertüütsch (gsw, Swiss German)"
            }

            TextSwitch {
                id: input_he
                width: parent.width
                text: "עברית (he, Hebrew)"
            }

            TextSwitch {
                id: input_hi
                width: parent.width
                text: "हिन्दी (hi, Hindi)"
            }

            TextSwitch {
                id: input_hr
                width: parent.width
                text: "hrvatski (hr, Croatian)"
            }

            TextSwitch {
                id: input_hu
                width: parent.width
                text: "magyar (hu, Hungarian)"
            }

            TextSwitch {
                id: input_id
                width: parent.width
                text: "Indonesia (id, Indonesian)"
            }

            TextSwitch {
                id: input_is
                width: parent.width
                text: "íslenska (is, Icelandic)"
            }

            TextSwitch {
                id: input_it
                width: parent.width
                text: "italiano (it, Italian)"
            }

            TextSwitch {
                id: input_ja
                width: parent.width
                text: "日本語 (ja, Japanese)"
            }

            TextSwitch {
                id: input_ka
                width: parent.width
                text: "ქართული (ka, Georgian)"
            }

            TextSwitch {
                id: input_ko
                width: parent.width
                text: "한국어 (ko, Korean)"
            }

            TextSwitch {
                id: input_lb
                width: parent.width
                text: "Lëtzebuergesch (lb, Luxembourgish)"
            }

            TextSwitch {
                id: input_lt
                width: parent.width
                text: "lietuvių (lt, Lithuanian)"
            }

            TextSwitch {
                id: input_lv
                width: parent.width
                text: "latviešu (lv, Latvian)"
            }

            TextSwitch {
                id: input_ms
                width: parent.width
                text: "Bahasa Melayu (ms, Malay)"
            }

            TextSwitch {
                id: input_mt
                width: parent.width
                text: "Malti (mt, Maltese)"
            }

            TextSwitch {
                id: input_nb
                width: parent.width
                text: "norsk bokmål (nb, Norwegian Bokmål)"
            }

            TextSwitch {
                id: input_nl
                width: parent.width
                text: "Nederlands (nl, Dutch)"
            }

            TextSwitch {
                id: input_oc
                width: parent.width
                text: "occitan (oc, Occitan)"
            }

            TextSwitch {
                id: input_pl
                width: parent.width
                text: "polski (pl, Polish)"
            }

            TextSwitch {
                id: input_pt
                width: parent.width
                text: "português (pt, Portuguese)"
            }

            TextSwitch {
                id: input_ro
                width: parent.width
                text: "română (ro, Romanian)"
            }

            TextSwitch {
                id: input_ru
                width: parent.width
                text: "русский (ru, Russian)"
            }

            TextSwitch {
                id: input_si
                width: parent.width
                text: "සිංහල (si, Sinhala)"
            }

            TextSwitch {
                id: input_sk
                width: parent.width
                text: "slovenčina (sk, Slovak)"
            }

            TextSwitch {
                id: input_sl
                width: parent.width
                text: "slovenščina (sl, Slovenian)"
            }

            TextSwitch {
                id: input_sr
                width: parent.width
                text: "српски (sr, Serbian)"
            }

            TextSwitch {
                id: input_sv
                width: parent.width
                text: "svenska (sv, Swedish)"
            }

            TextSwitch {
                id: input_th
                width: parent.width
                text: "ไทย (th, Thai)"
            }

            TextSwitch {
                id: input_tr
                width: parent.width
                text: "Türkçe (tr, Turkish)"
            }

            TextSwitch {
                id: input_uk
                width: parent.width
                text: "українська (uk, Ukrainian)"
            }

            TextSwitch {
                id: input_ur
                width: parent.width
                text: "اردو (ur, Urdu)"
            }

            TextSwitch {
                id: input_vi
                width: parent.width
                text: "Tiếng Việt (vi, Vietnamese)"
            }

            TextSwitch {
                id: input_zh
                width: parent.width
                text: "中文 (zh, Chinese)"
            }

        }

        VerticalScrollDecorator {}
    }

    function add(a)
    {
        if (value.length < 1)
            value += a
        else
            value += ", " + a
    }

    onAccepted: {
        value = ""
	
        if (input_af.checked) add("af")
        if (input_ar.checked) add("ar")
        if (input_az.checked) add("az")
        if (input_be.checked) add("be")
        if (input_bg.checked) add("bg")
        if (input_br.checked) add("br")
        if (input_bs.checked) add("bs")
        if (input_ca.checked) add("ca")
        if (input_cs.checked) add("cs")
        if (input_cy.checked) add("cy")
        if (input_da.checked) add("da")
        if (input_de.checked) add("de")
        if (input_el.checked) add("el")
        if (input_en.checked) add("en")
        if (input_es.checked) add("es")
        if (input_et.checked) add("et")
        if (input_eu.checked) add("eu")
        if (input_fa.checked) add("fa")
        if (input_fi.checked) add("fi")
        if (input_fil.checked) add("fil")
        if (input_fr.checked) add("fr")
        if (input_ga.checked) add("ga")
        if (input_gd.checked) add("gd")
        if (input_gl.checked) add("gl")
        if (input_gsw.checked) add("gsw")
        if (input_he.checked) add("he")
        if (input_hi.checked) add("hi")
        if (input_hr.checked) add("hr")
        if (input_hu.checked) add("hu")
        if (input_id.checked) add("id")
        if (input_is.checked) add("is")
        if (input_it.checked) add("it")
        if (input_ja.checked) add("ja")
        if (input_ka.checked) add("ka")
        if (input_ko.checked) add("ko")
        if (input_lb.checked) add("lb")
        if (input_lt.checked) add("lt")
        if (input_lv.checked) add("lv")
        if (input_ms.checked) add("ms")
        if (input_mt.checked) add("mt")
        if (input_nb.checked) add("nb")
        if (input_nl.checked) add("nl")
        if (input_oc.checked) add("oc")
        if (input_pl.checked) add("pl")
        if (input_pt.checked) add("pt")
        if (input_ro.checked) add("ro")
        if (input_ru.checked) add("ru")
        if (input_si.checked) add("si")
        if (input_sk.checked) add("sk")
        if (input_sl.checked) add("sl")
        if (input_sr.checked) add("sr")
        if (input_sv.checked) add("sv")
        if (input_th.checked) add("th")
        if (input_tr.checked) add("tr")
        if (input_uk.checked) add("uk")
        if (input_ur.checked) add("ur")
        if (input_vi.checked) add("vi")
        if (input_zh.checked) add("zh")

	
        if (typeof callback == "function")
           callback(value);
     }

    Component.onCompleted:
    {
        var langs = value.split(',');
        for (var i=0; i < langs.length; ++i)
        {
            var l = langs[i].trim()
	    
            if (l === "af") input_af.checked = true
            if (l === "ar") input_ar.checked = true
            if (l === "az") input_az.checked = true
            if (l === "be") input_be.checked = true
            if (l === "bg") input_bg.checked = true
            if (l === "br") input_br.checked = true
            if (l === "bs") input_bs.checked = true
            if (l === "ca") input_ca.checked = true
            if (l === "cs") input_cs.checked = true
            if (l === "cy") input_cy.checked = true
            if (l === "da") input_da.checked = true
            if (l === "de") input_de.checked = true
            if (l === "el") input_el.checked = true
            if (l === "en") input_en.checked = true
            if (l === "es") input_es.checked = true
            if (l === "et") input_et.checked = true
            if (l === "eu") input_eu.checked = true
            if (l === "fa") input_fa.checked = true
            if (l === "fi") input_fi.checked = true
            if (l === "fil") input_fil.checked = true
            if (l === "fr") input_fr.checked = true
            if (l === "ga") input_ga.checked = true
            if (l === "gd") input_gd.checked = true
            if (l === "gl") input_gl.checked = true
            if (l === "gsw") input_gsw.checked = true
            if (l === "he") input_he.checked = true
            if (l === "hi") input_hi.checked = true
            if (l === "hr") input_hr.checked = true
            if (l === "hu") input_hu.checked = true
            if (l === "id") input_id.checked = true
            if (l === "is") input_is.checked = true
            if (l === "it") input_it.checked = true
            if (l === "ja") input_ja.checked = true
            if (l === "ka") input_ka.checked = true
            if (l === "ko") input_ko.checked = true
            if (l === "lb") input_lb.checked = true
            if (l === "lt") input_lt.checked = true
            if (l === "lv") input_lv.checked = true
            if (l === "ms") input_ms.checked = true
            if (l === "mt") input_mt.checked = true
            if (l === "nb") input_nb.checked = true
            if (l === "nl") input_nl.checked = true
            if (l === "oc") input_oc.checked = true
            if (l === "pl") input_pl.checked = true
            if (l === "pt") input_pt.checked = true
            if (l === "ro") input_ro.checked = true
            if (l === "ru") input_ru.checked = true
            if (l === "si") input_si.checked = true
            if (l === "sk") input_sk.checked = true
            if (l === "sl") input_sl.checked = true
            if (l === "sr") input_sr.checked = true
            if (l === "sv") input_sv.checked = true
            if (l === "th") input_th.checked = true
            if (l === "tr") input_tr.checked = true
            if (l === "uk") input_uk.checked = true
            if (l === "ur") input_ur.checked = true
            if (l === "vi") input_vi.checked = true
            if (l === "zh") input_zh.checked = true

        }
    }
}


