open Lzma

let load_file f =
  let ic = open_in_bin f in
  let n = in_channel_length ic in
  let s = String.create n in
  really_input ic s 0 n;
  close_in ic;
  (s)

let filename = "./test_data.txt.lzma"

let buf_len = 16384

let () =
  Lzma.init();
  let data = load_file filename in
  let data_len = String.length data in
  let strm = new_lzma_stream() in
  lzma_auto_decoder ~strm ~check:LZMA_CHECK_NONE ~memlimit:536_870_912L;  (* 512 * 1024 * 1024 *)
  let buf = String.create buf_len in
  let ofs = ref 0 in
  begin
    try
      while true do
        let avail_in, avail_out =
          lzma_code ~strm ~action:LZMA_RUN
                    ~next_in:data ~next_out:buf
                    ~ofs_in:!ofs
                    ~ofs_out:0
        in
        ofs := data_len - avail_in;
        print_string buf;
      done
    with EOF n ->
      let str_end = String.sub buf 0 (buf_len - n) in
      print_string str_end;
      lzma_end ~strm;
  end;
;;

