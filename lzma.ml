(** OCaml bindings to the lzma library *)
(* Copyright (C) 2010  Florent Monnier
   Contact:  <fmonnier(AT-SIGN)linux-nantes(DOT-ORG)>
   This file is distributed under the terms of the MIT license.
   See the file LICENSE.txt for more details.
*)

(** You can get more documentation reading the comments in the files
    /usr/include/lzma/*.h
*)

exception EOF of int
(** end of file reached, but still [n] chars available in the buffer *)

exception MEM_LIMIT of int64
(** memory usage limit was reached,
    the minimum required mem-limit value is returned *)

let init () =
  Callback.register_exception "exn_lzma_eof" (EOF 0);
  Callback.register_exception "exn_lzma_memlimit" (MEM_LIMIT 0L);
;;
let () = init ()

type lzma_stream

external new_lzma_stream: unit -> lzma_stream = "new_lzma_stream"

external lzma_stream_total_in_out: strm:lzma_stream -> int64 * int64
  = "lzma_stream_total_in_out"
(** total number of bytes read/written by liblzma *)


(** {3 Integrity Check} *)

(** Type of the integrity check (Check ID)
  
    The .xz format supports multiple types of checks that are calculated from
    the uncompressed data. They vary in both speed and ability to detect errors.
*)
type lzma_check =
  | LZMA_CHECK_NONE
    (** No Check is calculated. (size of the Check field: 0 bytes) *)
  | LZMA_CHECK_CRC32
    (** CRC32 using the polynomial from the IEEE 802.3 standard
        (Size of the Check field: 4 bytes) *)
  | LZMA_CHECK_CRC64
    (** CRC64 using the polynomial from the ECMA-182 standard
        (Size of the Check field: 8 bytes) *)
  | LZMA_CHECK_SHA256
    (** SHA-256 (Size of the Check field: 32 bytes) *)


external lzma_check_is_supported: check:lzma_check -> bool
  = "caml_lzma_check_is_supported"
external lzma_check_size: check:lzma_check -> int = "caml_lzma_check_size"
external lzma_check_size_max: unit -> int = "caml_lzma_check_size_max"

external lzma_crc32: ?crc:int32 -> string -> int32 = "caml_lzma_crc32"
external lzma_crc64: ?crc:int64 -> string -> int64 = "caml_lzma_crc64"

external lzma_get_check: strm:lzma_stream -> lzma_check  = "caml_lzma_get_check"


(** {3 Initialise for decoding} *)

external lzma_auto_decoder: strm:lzma_stream -> memlimit:int64 ->
  check:lzma_check -> unit = "caml_lzma_auto_decoder"
(** decode .xz streams and .lzma streams with autodetection *)

(** {3 Initialise for encoding} *)

type lzma_preset =
  | LZMA_PRESET_DEFAULT
  | LZMA_PRESET_EXTREME
  | LZMA_PRESET_TEXT

external lzma_easy_encoder:
  strm:lzma_stream -> level:int -> preset:lzma_preset list ->
  check:lzma_check -> unit
  = "caml_lzma_easy_encoder"
(** initialise .xz stream encoder *)

type lzma_options
external new_lzma_options: unit -> lzma_options = "new_lzma_options_lzma"

external lzma_preset:
  options:lzma_options -> level:int -> preset_extreme:bool -> unit
  = "caml_lzma_lzma_preset"

external lzma_alone_encoder:
  strm:lzma_stream -> options:lzma_options -> unit
  = "caml_lzma_alone_encoder"
(** initialise .lzma stream encoder *)


(** {3 Running encoding/decoding} *)

type lzma_action =
  | LZMA_RUN
  | LZMA_SYNC_FLUSH
  | LZMA_FULL_FLUSH
  | LZMA_FINISH

external lzma_code: strm:lzma_stream -> action:lzma_action ->
  next_in:string -> next_out:string ->
  ofs_in:int -> ofs_out:int -> int * int
  = "caml_lzma_code_bytecode"
    "caml_lzma_code_native"
(** returns (avail_in, avail_out) *)


(** {3 Ending} *)

(* XXX maybe lzma_end could be a finaliser of the lzma_stream value,
   TODO investigate *)
external lzma_end: strm:lzma_stream -> unit = "caml_lzma_end"


(** {3 Single-call} *)

external lzma_stream_buffer_bound: uncompressed_size:int -> int
  = "caml_lzma_stream_buffer_bound"
(** Calculate output buffer size for single-call Stream encoder *)

external lzma_easy_buffer_encode:
  level:int -> preset:lzma_preset list -> check:lzma_check ->
  data:string -> buf:string -> ofs:int -> int
  = "caml_lzma_easy_buffer_encode_bytecode"
    "caml_lzma_easy_buffer_encode_native"
(** single-call .xz stream encoding *)

type decoder_flags =
  | LZMA_TELL_NO_CHECK
  | LZMA_TELL_UNSUPPORTED_CHECK
  | LZMA_CONCATENATED

external lzma_stream_buffer_decode: memlimit:int64 ->
  flags:decoder_flags list -> data:string -> data_ofs:int ->
  buf:string -> buf_ofs:int ->
  int * int
  = "caml_lzma_stream_buffer_decode_bytecode"
    "caml_lzma_stream_buffer_decode_native"
(** single-call .xz stream decoder
    returns (in_pos, out_pos) *)


(** {3 Memory Management} *)

external lzma_memusage: strm:lzma_stream -> int64 = "caml_lzma_memusage"
external lzma_memlimit_get: strm:lzma_stream -> int64 = "caml_lzma_memlimit_get"
external lzma_memlimit_set: strm:lzma_stream -> memlimit:int64 -> unit
  = "caml_lzma_memlimit_set"


(** {3 Version} *)

type stability =
  | Alpha
  | Beta
  | Stable

type version_kind =
  | Run_time
  | Compile_time

external lzma_version_number: version_kind -> int * int * int * stability
  = "caml_lzma_version_number"

external lzma_version_string: version_kind -> string
  = "caml_lzma_version_string"

