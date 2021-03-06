/* Copyright (C) 2010  Florent Monnier
   This file is distributed under the terms of the MIT license.
   See the file LICENSE.txt for more details.
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

#define Val_none Val_int(0)
#define Some_val(v) Field(v,0)

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
                                    caml_raise_with_arg(*caml_named_value("exn_lzma_eof"), \
                                           Val_long(Lzma_stream_val(strm)->avail_out)); \
        case LZMA_NO_CHECK:         caml_failwith(#func_name ": no check"); \
        case LZMA_UNSUPPORTED_CHECK:caml_failwith(#func_name ": unsupported check"); \
        case LZMA_GET_CHECK:        caml_failwith(#func_name ": get check"); \
        case LZMA_MEM_ERROR:        caml_failwith(#func_name ": mem error"); \
        case LZMA_MEMLIMIT_ERROR:   caml_failwith(#func_name ": memlimit error"); \
        case LZMA_FORMAT_ERROR:     caml_failwith(#func_name ": format error"); \
        case LZMA_OPTIONS_ERROR:    caml_failwith(#func_name ": options error"); \
        case LZMA_DATA_ERROR:       caml_failwith(#func_name ": data error"); \
        case LZMA_BUF_ERROR:        caml_failwith(#func_name ": buffer error"); \
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

static const lzma_check lzma_check_table[] = {
    LZMA_CHECK_NONE,
    LZMA_CHECK_CRC32,
    LZMA_CHECK_CRC64,
    LZMA_CHECK_SHA256
};
#define Lzma_check_val(v) (lzma_check_table[Long_val((v))])

CAMLprim value caml_lzma_auto_decoder(value strm, value memlimit, value ml_check)
{
    uint32_t flags = Lzma_check_val(ml_check);
    lzma_ret st = lzma_auto_decoder(
        Lzma_stream_val(strm), Int64_val(memlimit), flags);
    if (st != LZMA_OK) {
        if (st == LZMA_MEM_ERROR) caml_failwith("lzma_auto_decoder: cannot allocate memory");
        if (st == LZMA_OPTIONS_ERROR) caml_failwith("lzma_auto_decoder: unsupported flags");
        caml_failwith("lzma_auto_decoder");
    }
    return Val_unit;
}

#if !defined(LZMA_PRESET_TEXT)
  #define LZMA_PRESET_TEXT 0
#endif

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
CAMLprim value caml_lzma_easy_encoder(value strm, value level, value preset, value check)
{
    lzma_ret st = lzma_easy_encoder(Lzma_stream_val(strm),
            Long_val(level) | Lzma_preset_val(preset), Lzma_check_val(check));
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

CAMLprim value caml_lzma_stream_buffer_bound(value uncompressed_size) {
    return Val_long(lzma_stream_buffer_bound(Long_val(uncompressed_size)));
}

CAMLprim value caml_lzma_easy_buffer_encode_native(
        value level,
        value preset,
        value check,
        value in,
        value out,
        value ml_out_pos)
{
    size_t out_pos = Long_val(ml_out_pos);
    lzma_ret st = lzma_easy_buffer_encode(
            Long_val(level) | Lzma_preset_val(preset), Lzma_check_val(check),
            NULL, Conv_string(in), caml_string_length(in),
            Conv_string(out), &out_pos, caml_string_length(out));
    if (st != LZMA_OK) {
        if (st == LZMA_BUF_ERROR) caml_failwith("lzma_easy_buffer_encode: not enough output buffer space");
        if (st == LZMA_OPTIONS_ERROR) caml_failwith("lzma_easy_buffer_encode: options error");
        if (st == LZMA_MEM_ERROR) caml_failwith("lzma_easy_buffer_encode: mem error");
        if (st == LZMA_DATA_ERROR) caml_failwith("lzma_easy_buffer_encode: data error");
        if (st == LZMA_PROG_ERROR) caml_failwith("lzma_easy_buffer_encode: prog error");
        caml_failwith("lzma_easy_buffer_encode");
    }
    return Val_long(out_pos);
}
CAMLprim value caml_lzma_easy_buffer_encode_bytecode(value * argv, int argn) {
    return caml_lzma_easy_buffer_encode_native(argv[0], argv[1], argv[2],
                                               argv[3], argv[4], argv[5]);
}

static const uint32_t decoder_flags_table[] = {
    LZMA_TELL_NO_CHECK,
    LZMA_TELL_UNSUPPORTED_CHECK,
    LZMA_CONCATENATED
};
static inline uint32_t
Decoder_flags_val(value mask_list) {
    uint32_t c_mask = 0; 
    while (mask_list != Val_emptylist) {
        value head = Field(mask_list, 0);
        c_mask |= decoder_flags_table[Long_val(head)];
        mask_list = Field(mask_list, 1);
    }
    return c_mask;
}

CAMLprim value caml_lzma_stream_buffer_decode_native(
        value ml_memlimit,
        value flags,
        value in,
        value ml_in_pos,
        value out,
        value ml_out_pos)
{
    CAMLparam5(ml_memlimit, flags, in, ml_in_pos, out);
    CAMLxparam1(ml_out_pos);
    CAMLlocal1(ret);
    uint64_t memlimit = Int64_val(ml_memlimit);
    size_t in_pos = Long_val(ml_in_pos);
    size_t out_pos = Long_val(ml_out_pos);
    lzma_ret st = lzma_stream_buffer_decode(
            &memlimit, Decoder_flags_val(flags), NULL,
            Conv_string(in), &in_pos, caml_string_length(in),
            Conv_string(out), &out_pos, caml_string_length(out));
    if (st != LZMA_OK) {
        switch (st) {
        case LZMA_FORMAT_ERROR: caml_failwith("lzma_stream_buffer_decode: format error");
        case LZMA_OPTIONS_ERROR: caml_failwith("lzma_stream_buffer_decode: options error");
        case LZMA_DATA_ERROR: caml_failwith("lzma_stream_buffer_decode: data error");
        case LZMA_NO_CHECK: caml_failwith("lzma_stream_buffer_decode: no check");
        case LZMA_UNSUPPORTED_CHECK: caml_failwith("lzma_stream_buffer_decode: unsupported check");
        case LZMA_MEM_ERROR: caml_failwith("lzma_stream_buffer_decode: mem error");
        case LZMA_MEMLIMIT_ERROR: caml_raise_with_arg(*caml_named_value("exn_lzma_memlimit"),
                                                       caml_copy_int64(memlimit));
        case LZMA_BUF_ERROR: caml_failwith("lzma_stream_buffer_decode: output buffer was too small");
        case LZMA_PROG_ERROR: caml_failwith("lzma_stream_buffer_decode: prog error");
        case LZMA_STREAM_END:
        case LZMA_GET_CHECK: caml_failwith("lzma_stream_buffer_decode");
        case LZMA_OK:
        break;
        }
    }
    ret = caml_alloc(2, 0);
    Store_field(ret, 0, Val_long(in_pos));
    Store_field(ret, 1, Val_long(out_pos));
    CAMLreturn(ret);
}
CAMLprim value caml_lzma_stream_buffer_decode_bytecode(value * argv, int argn) {
    return caml_lzma_stream_buffer_decode_native(argv[0], argv[1], argv[2],
                                                 argv[3], argv[4], argv[5]);
}

CAMLprim value caml_lzma_version_number(value kind)
{
    CAMLparam1(kind);
    CAMLlocal1(ret);
    ret = caml_alloc(4, 0);
    if (kind == Val_int(0))
    {   /* run-time version */
        uint32_t v, major, minor, patch, stability;
        v = lzma_version_number();
        major = v / 10000000;
        minor = (v / 10000) - (major * 1000);
        patch = (v / 10) - (major * 1000000) - (minor * 1000);
        stability = v - (major * 10000000) - (minor * 10000) - (patch * 10);
        Store_field(ret, 0, Val_int(major) );
        Store_field(ret, 1, Val_int(minor) );
        Store_field(ret, 2, Val_int(patch) );
        Store_field(ret, 3, Val_int(stability) );
    }
    else
    {   /* compile-time version */
        Store_field(ret, 0, Val_int(LZMA_VERSION_MAJOR) );
        Store_field(ret, 1, Val_int(LZMA_VERSION_MINOR) );
        Store_field(ret, 2, Val_int(LZMA_VERSION_PATCH) );
        Store_field(ret, 3, Val_int(LZMA_VERSION_STABILITY) );
    }
    CAMLreturn(ret);
}

CAMLprim value caml_lzma_version_string(value kind)
{
    CAMLparam1(kind);
    CAMLlocal1(ret);
    if (kind == Val_int(0))
    {   /* run-time version */
        ret = caml_copy_string(lzma_version_string());
    }
    else
    {   /* compile-time version */
        ret = caml_copy_string(LZMA_VERSION_STRING);
    }
    CAMLreturn(ret);
}

/* Integrity Check */

CAMLprim value caml_lzma_check_is_supported(value check)
{
    lzma_bool b = lzma_check_is_supported(Lzma_check_val(check));
    return Val_bool(b);
}

CAMLprim value caml_lzma_check_size(value check)
{
    uint32_t size = lzma_check_size(Lzma_check_val(check));
    if (size == UINT32_MAX) caml_invalid_argument("lzma_check_size");
    return Val_long(size);
}

CAMLprim value caml_lzma_check_size_max(value unit)
{
    return Val_long(LZMA_CHECK_SIZE_MAX);
}

CAMLprim value caml_lzma_crc32(value prev_crc, value buf)
{
    uint32_t crc;
    if (prev_crc == Val_none) crc = 0;
    else crc = Int32_val(Some_val(prev_crc));
    uint32_t ret = lzma_crc32(Conv_string(buf), caml_string_length(buf), crc);
    return caml_copy_int32(ret);
}

CAMLprim value caml_lzma_crc64(value prev_crc, value buf)
{
    uint64_t crc;
    if (prev_crc == Val_none) crc = 0;
    else crc = Int64_val(Some_val(prev_crc));
    uint64_t ret = lzma_crc64(Conv_string(buf), caml_string_length(buf), crc);
    return caml_copy_int64(ret);
}

CAMLprim value caml_lzma_get_check(value strm)
{
    lzma_check check = lzma_get_check(Lzma_stream_val(strm));
    switch (check) {
        case LZMA_CHECK_NONE:   return Val_int(0);
        case LZMA_CHECK_CRC32:  return Val_int(1);
        case LZMA_CHECK_CRC64:  return Val_int(2);
        case LZMA_CHECK_SHA256: return Val_int(3);
    }
    caml_failwith("lzma_get_check: unrecognised return value");
    return Val_int(0);
}

/* memory management */

CAMLprim value caml_lzma_memusage(value strm)
{
    uint64_t ret = lzma_memusage(Lzma_stream_val(strm));
    return caml_copy_int64(ret);
}

CAMLprim value caml_lzma_memlimit_get(value strm)
{
    uint64_t ret = lzma_memlimit_get(Lzma_stream_val(strm));
    return caml_copy_int64(ret);
}

CAMLprim value caml_lzma_memlimit_set(value strm, value memlimit)
{
    lzma_ret st = lzma_memlimit_set(Lzma_stream_val(strm), Int64_val(memlimit));
    if (st != LZMA_OK) {
      if (st == LZMA_MEMLIMIT_ERROR) caml_failwith("lzma_memlimit_set: the new limit is too small");
      if (st == LZMA_PROG_ERROR) caml_invalid_argument("lzma_memlimit_set");
    }
    return Val_unit;
}

