open Lzma

let load_file f =
  let ic = open_in f in
  let n = in_channel_length ic in
  let s = String.create n in
  really_input ic s 0 n;
  close_in ic;
  (s)

let filename = "./test_data.txt"

let buf_len = Sys.max_string_length

let () =
  let data = load_file filename in
  Printf.eprintf "data length: %d\n" (String.length data);
  let strm = new_lzma_stream() in
  let check = LZMA_CHECK_CRC32 in
  let preset = [(*LZMA_PRESET_EXTREME*)] in
  lzma_easy_encoder ~strm ~level:9 ~preset ~check;  (* select xz file format *)
  let buf = String.create buf_len in
  try
    let avail_in, avail_out =
      lzma_code ~strm ~action:LZMA_FINISH
                ~next_in:data
                ~next_out:buf
                ~ofs_in:0
                ~ofs_out:0
    in
    Printf.eprintf "avail_in: %d  avail_out: %d\n" avail_in avail_out;
  with EOF n ->
    lzma_end ~strm;
    let str = String.sub buf 0 (buf_len - n) in
    print_string str;
    flush stdout;
;;

