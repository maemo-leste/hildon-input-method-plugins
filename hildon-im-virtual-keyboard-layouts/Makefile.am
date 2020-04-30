SUFFIXES = .vkb .xml

vkbdir = $(datarootdir)/keyboards
vkb_DATA = be_BY.vkb bg_BG.vkb ca_AD.vkb cs_CZ.vkb da_DK.vkb \
	   de_DE.vkb dz_BT.vkb el_GR.vkb en_GB.vkb en_US.vkb \
	   es_ES.vkb es_MX.vkb fa_IR.vkb fi_FI.vkb fr_CA.vkb \
	   fr_FR.vkb he_IL.vkb hr_HR.vkb hu_HU.vkb hy_AM.vkb \
	   is_IS.vkb it_IT.vkb ka_GE.vkb km_KH.vkb mk_MK.vkb \
	   ne_NP.vkb nl_NL.vkb no_NO.vkb pa_IN.vkb pl_PL.vkb \
	   pt_PT.vkb ro_RO.vkb ru_RU.vkb sk_SK.vkb sl_SI.vkb \
	   sr_RS.vkb sv_SE.vkb th_TH.vkb tr_TR.vkb ug_CN.vkb \
	   vi_VN.vkb

.xml.vkb:
	gen_vkb $^

CLEANFILES = *.vkb

MAINTAINERCLEANFILES = Makefile.in
