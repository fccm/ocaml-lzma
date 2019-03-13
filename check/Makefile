#  Copyright (C) 2010  Florent Monnier
#  This file is distributed under the terms of the MIT license.
#  See the file LICENSE.txt for more details.

OCAML := ocaml
OCAMLC := ocamlc -g
OCAMLOPT := ocamlopt -g
OCAMLRUN := ocamlrun
MLLZ_DIR := ../src
#MLLZ_DIR := +lzma

all: test

EDITOR := vim
.PHONY: edit
edit:
	$(EDITOR) \
	  test_decode.ml \
	  test_encode_lzma.ml \
	  test_encode_xz.ml \
	  test_singlecall.ml \
	  test_data.txt \
	  #End

.PHONY: test test_opt test_byte

test: test_decode.ml
	$(OCAML) -I $(MLLZ_DIR) lzma.cma $<

test_opt: test_decode.opt
	./$<

test_byte: test_decode.byte
	$(OCAMLRUN) -I $(MLLZ_DIR) $<

test_decode.byte: test_decode.ml
	$(OCAMLC) -o $@ -I $(MLLZ_DIR) lzma.cma $<

test_decode.opt: test_decode.ml
	$(OCAMLOPT) -o $@ -I $(MLLZ_DIR) lzma.cmxa $<

