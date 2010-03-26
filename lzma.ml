(** OCaml bindings to the lzma library *)
(* Copyright (C) 2010  Florent Monnier
   Contact:  <fmonnier(AT-SIGN)linux-nantes(DOT-ORG)>
   This file is distributed under the terms of the GNU Lesser General Public
   License, with the special exception on linking described in file LICENSE.txt
*)

(** You can get more documentation reading the comments in the files
    /usr/include/lzma/*.h
*)

exception EOF of int

let init() =
  Callback.register_exception "lzma_eof" (EOF 0);
;;
let () = init() ;;

type lzma_stream
external new_lzma_stream: unit -> lzma_stream = "new_lzma_stream"

external lzma_stream_total_in_out: strm:lzma_stream -> int64 * int64 = "lzma_stream_total_in_out"
(** total number of bytes read/written by liblzma *)


(** {3 Initialise for decoding} *)

external lzma_auto_decoder: strm:lzma_stream -> memlimit:int64 -> flags:int32 -> unit = "caml_lzma_auto_decoder"
(** decode .xz streams and .lzma streams with autodetection *)

(** {3 Initialise for encoding} *)

type lzma_preset =
  | LZMA_PRESET_DEFAULT
  | LZMA_PRESET_EXTREME
  | LZMA_PRESET_TEXT


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


external lzma_easy_encoder: strm:lzma_stream -> level:int -> preset:lzma_preset list ->
  check:lzma_check -> unit = "caml_lzma_easy_encoder"
(** initialise .xz stream encoder *)

type lzma_options
external new_lzma_options: unit -> lzma_options = "new_lzma_options_lzma"
external lzma_preset: options:lzma_options -> level:int -> preset_extreme:bool -> unit = "caml_lzma_lzma_preset"
external lzma_alone_encoder: strm:lzma_stream -> options:lzma_options -> unit = "caml_lzma_alone_encoder"
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

(* XXX maybe lzma_end could be a finaliser of the lzma_stream value, TODO investigate *)
external lzma_end: strm:lzma_stream -> unit = "caml_lzma_end"


(** {3 Single-call} *)

external lzma_stream_buffer_bound: uncompressed_size:int -> int = "caml_lzma_stream_buffer_bound"
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
(** single-call .xz stream decoder *)

