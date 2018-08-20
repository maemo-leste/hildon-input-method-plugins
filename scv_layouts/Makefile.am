scvvkbdir = $(datarootdir)/scv_layouts
scvvkb_DATA = ch.vkb cz.vkb dano.vkb de.vkb fise.vkb fr.vkb it.vkb pl.vkb ptes.vkb ru.vkb us.vkb

%.vkb: %.xml
	gen_vkb $^

CLEANFILES = *.vkb

MAINTAINERCLEANFILES = Makefile.in
