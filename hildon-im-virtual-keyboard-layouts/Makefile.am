SUFFIXES = .vkb .xml

vkbdir = $(datarootdir)/keyboards
vkb_DATA = cs_CZ.vkb da_DK.vkb de_DE.vkb el_GR.vkb en_GB.vkb en_US.vkb \
	   es_ES.vkb es_MX.vkb fi_FI.vkb fr_CA.vkb fr_FR.vkb it_IT.vkb \
	   nl_NL.vkb no_NO.vkb pl_PL.vkb pt_PT.vkb ru_RU.vkb sv_SE.vkb


.xml.vkb:
	gen_vkb $^

CLEANFILES = *.vkb

MAINTAINERCLEANFILES = Makefile.in
