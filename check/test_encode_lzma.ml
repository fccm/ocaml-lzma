open Lzma

let load_file f =
  let ic = open_in f in
  let n = in_channel_length ic in
  let s = Bytes.create n in
  really_input ic s 0 n;
  close_in ic;
  (s)

let filename = "./test_data.txt"

let buf_len = Sys.max_string_length

let () =
  let data = load_file filename in
  Printf.eprintf "data length: %d\n" (Bytes.length data);
  let strm = new_lzma_stream() in

  let options = new_lzma_options() in
  lzma_preset ~options ~level:9 ~preset_extreme:true;
  lzma_alone_encoder ~strm ~options;  (* select lzma file format *)

  let buf = Bytes.create buf_len in
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
    let str = Bytes.sub buf 0 (buf_len - n) in
    print_bytes str;
    flush stdout;
;;

