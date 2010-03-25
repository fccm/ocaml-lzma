#  Copyright (C) 2010  Florent Monnier
#  Contact:  <fmonnier(AT-SIGN)linux-nantes(DOT-ORG)>
#  This file is distributed under the terms of the GNU Lesser General Public
#  License, with the special exception on linking described in file LICENSE.txt

OCAMLC := ocamlc.opt -g
OCAMLOPT := ocamlopt.opt -g
OCAMLMKLIB := ocamlmklib
OCAMLDOC := ocamldoc.opt
OCAML_PATH := $(shell ocamlc -where)
LZMA_LIBS := -llzma
LZMA_DIR := lzma
PREFIX := $(OCAML_PATH)/$(LZMA_DIR)
SO_PREFIX := $(PREFIX)
#SO_PREFIX := $(OCAML_PATH)/stublibs/
DOC_DIR := doc

all: cma cmxa cmxs
byte cma: lzma.cma
opt cmxa: lzma.cmxa
shared cmxs: lzma.cmxs
.PHONY: all byte cma opt cmxa

lzma.mli: lzma.ml
	$(OCAMLC) -i $< > $@

lzma.cmi: lzma.mli
	$(OCAMLC) -c $<

lzma.cmx: lzma.ml lzma.cmi
	$(OCAMLOPT) -c $<

lzma.cmo: lzma.ml lzma.cmi
	$(OCAMLC) -c $<

lzma_stubs.o: lzma_stubs.c
	$(OCAMLC) -c $<

dlllzma.so liblzma_stubs.a: lzma_stubs.o
	$(OCAMLMKLIB) -oc lzma_stubs $< $(LZMA_LIBS)

lzma.cmxa lzma.a: lzma.cmx dlllzma.so
	$(OCAMLMKLIB) -o lzma $< $(LZMA_LIBS) #-llzma_stubs

lzma.cma: lzma.cmo  dlllzma.so
	$(OCAMLC) -a -o $@ $< -cclib $(LZMA_LIBS) -dllib dlllzma_stubs.so

lzma.cmxs: lzma.cmxa
	$(OCAMLOPT) -shared -linkall -o $@ $<

doc:
	$(OCAMLDOC) lzma.ml -colorize-code -html -d $(DOC_DIR)

vim:
	vim lzma.ml lzma_stubs.c
.PHONY: doc vim test
test: test_decode.ml lzma.cma
	ocaml -I . lzma.cma $<

test_decode.byte: test_decode.ml lzma.cma
	$(OCAMLC) -o $@ -I . lzma.cma $<

test_decode.opt: test_decode.ml lzma.cmxa
	$(OCAMLOPT) -o $@ -I . lzma.cmxa -ccopt -llzma_stubs $<

DIST_FILES=           \
    liblzma_stubs.a   \
    lzma.a            \
    lzma.o            \
    lzma.cma          \
    lzma.cmi          \
    lzma.cmo          \
    lzma.cmx          \
    lzma.cmxa         \
    lzma.mli          \
#EOL
SO_DIST_FILES=        \
    dlllzma_stubs.so  \
    lzma.cmxs         \
#EOL

.PHONY: install uninstall
install: $(DIST_FILES)  $(SO_DIST_FILES) META
	if [ ! -d $(PREFIX) ]; then install -d $(PREFIX) ; fi
	for file in $(DIST_FILES);    do if [ -f $$file ]; then install -m 0644 $$file $(PREFIX)/; fi; done
	for file in $(SO_DIST_FILES); do if [ -f $$file ]; then install -m 0755 $$file $(SO_PREFIX)/; fi; done
	install -m 0644 META $(PREFIX)/
uninstall:
	rm $(PREFIX)/*
	rmdir $(PREFIX)/

.PHONY: clean cleandoc
clean:
	rm -f *.[oa] *.cm[ioxa] *.{so,cmxa,cmxs} *.{opt,byte}
cleandoc:
	rm -f $(DOC_DIR)/*
	rmdir $(DOC_DIR)

