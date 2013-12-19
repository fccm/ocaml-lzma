open Lzma

let load_file f =
  let ic = open_in f in
  let n = in_channel_length ic in
  let s = String.create n in
  really_input ic s 0 n;
  close_in ic;
  (s)

let filename = "./test_data.txt"

let () =
  let data = load_file filename in
  let data_len = String.length data in
  let bound = lzma_stream_buffer_bound data_len in
  let buf = String.create bound in
  let ofs = 0 in
  let len =
    lzma_easy_buffer_encode
        ~level:9 ~preset:[] ~check:LZMA_CHECK_CRC32
        ~data ~buf ~ofs
  in
  let compressed = String.sub buf ofs (len - ofs) in
  Printf.printf "data_len: %d (bound: %d), compressed: %d\n" data_len bound len;
  let flags = [
    LZMA_TELL_NO_CHECK;
    LZMA_TELL_UNSUPPORTED_CHECK;
  ] in
  let buf = String.create data_len in
  let in_pos, out_pos =
    (* if the memory usage limit is reached, the minimum required
       memlimit value is returned with the exception MEM_LIMIT *)
    lzma_stream_buffer_decode ~memlimit:536870912L  (* 512 Mo *)
        ~flags ~data:compressed ~data_ofs:0
        ~buf ~buf_ofs:0
  in
  Printf.printf "in_pos: %d, out_pos: %d\n" in_pos out_pos;
  print_string buf;
;;

