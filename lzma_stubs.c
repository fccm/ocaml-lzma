/* Copyright (C) 2010  Florent Monnier
   Contact:  <fmonnier(AT-SIGN)linux-nantes(DOT-ORG)>
   This file is distributed under the terms of the GNU Lesser General Public
   License, with the special exception on linking described in file LICENSE.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/custom.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>

#include <lzma.h>

static struct custom_operations lzma_stream_custom_ops = {
    identifier: "lzma_stream custom_operations",
    finalize:    custom_finalize_default,
    compare:     custom_compare_default,
    hash:        custom_hash_default,
    serialize:   custom_serialize_default,
    deserialize: custom_deserialize_default
};

static struct custom_operations lzma_options_lzma_custom_ops = {
    identifier: "lzma_options_lzma custom_operations",
    finalize:    custom_finalize_default,
    compare:     custom_compare_default,
    hash:        custom_hash_default,
    serialize:   custom_serialize_default,
    deserialize: custom_deserialize_default
};

static const lzma_stream lzma_stream_init = LZMA_STREAM_INIT;
#define Lzma_stream_val(v) ((lzma_stream *) Data_custom_val(v))
CAMLprim value new_lzma_stream(value unit)
{
    CAMLparam1(unit);
    CAMLlocal1(v);
    v = caml_alloc_custom(&lzma_stream_custom_ops, sizeof(lzma_stream), 0, 1);
    memcpy(Data_custom_val(v), &lzma_stream_init, sizeof(lzma_stream));
    CAMLreturn(v);
}

#define Lzma_options_lzma_val(v) ((lzma_options_lzma *) Data_custom_val(v))
CAMLprim value new_lzma_options_lzma(value unit)
{
    CAMLparam1(unit);
    CAMLlocal1(v);
    v = caml_alloc_custom(&lzma_options_lzma_custom_ops, sizeof(lzma_options_lzma), 0, 1);
    bzero(Data_custom_val(v), sizeof(lzma_options_lzma));
    CAMLreturn(v);
}

#define Conv_string(v) ((uint8_t *)(v))

CAMLprim value lzma_stream_total_in_out(value strm)
{
    CAMLparam1(strm);
    CAMLlocal1(ret);
    ret = caml_alloc(2, 0);
    Store_field(ret, 0, caml_copy_int64(Lzma_stream_val(strm)->total_in) );
    Store_field(ret, 1, caml_copy_int64(Lzma_stream_val(strm)->total_out) );
    CAMLreturn(ret);
}

#define lzma_status_check(st, func_name) \
    if (st != LZMA_OK) { \
        switch (st) { \
        case LZMA_STREAM_END:       /*caml_raise_end_of_file();*/ \
                                    caml_raise_with_arg(*caml_named_value("lzma_eof"), \
                                           Val_long(Lzma_stream_val(strm)->avail_out)); \
        case LZMA_NO_CHECK:         caml_failwith(#func_name ": no check"); \
        case LZMA_UNSUPPORTED_CHECK:caml_failwith(#func_name ": unsupported check"); \
        case LZMA_GET_CHECK:        caml_failwith(#func_name ": get check"); \
        case LZMA_MEM_ERROR:        caml_failwith(#func_name ": mem error"); \
        case LZMA_MEMLIMIT_ERROR:   caml_failwith(#func_name ": memlimit error"); \
        case LZMA_FORMAT_ERROR:     caml_failwith(#func_name ": format error"); \
        case LZMA_OPTIONS_ERROR:    caml_failwith(#func_name ": options error"); \
        case LZMA_DATA_ERROR:       caml_failwith(#func_name ": data error"); \
        case LZMA_BUF_ERROR:        caml_failwith(#func_name ": buf error"); \
        case LZMA_PROG_ERROR:       caml_failwith(#func_name ": prog error"); \
        case LZMA_OK: break; \
        } \
    }

static const lzma_action lzma_action_table[] = {
    LZMA_RUN,
    LZMA_SYNC_FLUSH,
    LZMA_FULL_FLUSH,
    LZMA_FINISH
};
#define Lzma_action_val(v) (lzma_action_table[Long_val((v))])

CAMLprim value caml_lzma_code_native(
        value strm,
        value action,
        value strm_next_in,
        value strm_next_out,
        value ofs_in,
        value ofs_out)
{
    CAMLparam5(strm, action, strm_next_in, strm_next_out, ofs_in);
    CAMLxparam1(ofs_out);
    CAMLlocal1(ret);

    Lzma_stream_val(strm)->next_in = Conv_string(strm_next_in) + Long_val(ofs_in);
    Lzma_stream_val(strm)->next_out = Conv_string(strm_next_out) + Long_val(ofs_out);
    Lzma_stream_val(strm)->avail_in = caml_string_length(strm_next_in) - Long_val(ofs_in);
    Lzma_stream_val(strm)->avail_out = caml_string_length(strm_next_out) - Long_val(ofs_out);

    lzma_ret st = lzma_code(Lzma_stream_val(strm), Lzma_action_val(action));
    lzma_status_check(st, lzma_code)

    ret = caml_alloc(2, 0);
    Store_field(ret, 0, Val_long(Lzma_stream_val(strm)->avail_in) );
    Store_field(ret, 1, Val_long(Lzma_stream_val(strm)->avail_out) );
    CAMLreturn(ret);
}
CAMLprim value caml_lzma_code_bytecode(value * argv, int argn) {
    return caml_lzma_code_native(argv[0], argv[1], argv[2],
                                 argv[3], argv[4], argv[5]);
}

CAMLprim value caml_lzma_end(value strm) {
    lzma_end(Lzma_stream_val(strm));
    return Val_unit;
}

CAMLprim value caml_lzma_auto_decoder(value strm, value memlimit, value ml_flags)
{
    uint32_t flags = Int32_val(ml_flags)/*LZMA_CHECK_CRC32*/;
    lzma_ret st = lzma_auto_decoder(
        Lzma_stream_val(strm), Int64_val(memlimit), flags);
    if (st != LZMA_OK) {
        if (st == LZMA_MEM_ERROR) caml_failwith("lzma_auto_decoder: cannot allocate memory");
        if (st == LZMA_OPTIONS_ERROR) caml_failwith("lzma_auto_decoder: unsupported flags");
    }
    return Val_unit;
}

static const uint32_t lzma_preset_table[] = {
    LZMA_PRESET_DEFAULT,
    LZMA_PRESET_EXTREME,
    LZMA_PRESET_TEXT
};
static inline uint32_t
Lzma_preset_val(value mask_list) {
    uint32_t c_mask = 0; 
    while (mask_list != Val_emptylist) {
        value head = Field(mask_list, 0);
        c_mask |= lzma_preset_table[Long_val(head)];
        mask_list = Field(mask_list, 1);
    }
    return c_mask;
}
CAMLprim value caml_lzma_easy_encoder(value strm, value level, value preset/*, value check*/)
{
    lzma_check check = LZMA_CHECK_CRC32;
    lzma_ret st = lzma_easy_encoder(Lzma_stream_val(strm), Long_val(level) | Lzma_preset_val(preset), check);
    lzma_status_check(st, lzma_easy_encoder)
    return Val_unit;
}

CAMLprim value caml_lzma_lzma_preset(value options, value level, value preset_extreme)
{
    uint32_t preset = (Long_val(preset_extreme) ? LZMA_PRESET_EXTREME : 0);
    lzma_bool b = lzma_lzma_preset(Lzma_options_lzma_val(options), Long_val(level) | preset);
    if (b) caml_failwith("lzma_lzma_preset");
    return Val_unit;
}

CAMLprim value caml_lzma_alone_encoder(value strm, value options)
{
    lzma_ret st = lzma_alone_encoder(
            Lzma_stream_val(strm), Lzma_options_lzma_val(options));
    lzma_status_check(st, lzma_alone_encoder)
    return Val_unit;
}

