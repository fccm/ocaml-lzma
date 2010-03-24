open Lzma

let buf_len = Sys.max_string_length
let data = "\

    ALZMAL       ALZMALZMALZ  ALZMA       ALZMA        ALZ       
    MALZMA       MALZMALZMAL  MALZM       MALZM       ZMALZ      
      ALZ        ZMA    LZMA    ALZM      ZMA         LZMAL      
      MA               MAL      MALZM    ALZM        MALZMA      
      ZM              LZM       ZMALZ    M LZ        ZMA ZMA     
      LZ             MAL        LZMALZ  LZ AL       ALZMALZMA    
      ALZ           LZM         AL MALZ AL MA       MALZMALZM    
     ZMAL    ZM    MAL          MA ZMALZM  ZMA     LZMA   ALZM   
      ZMA    LZ   LZM     LZ    ZMA ZMALZ ALZM     ALZ     ALZ   
    MALZMALZMAL  MALZMALZMAL  MALZMA ZMA ZMALZM  LZMALZ   ZMALZM 
    ZMALZMALZMA  ZMALZMALZMA   MALZM LZ  LZMALZ  ALZMAL   LZMALZ 

      amlOCa       OCaml          OCa       OCaml       amlOCamlOCa      
     lOCamlOC    CamlOCamlO      amlOC      mlOCa       OCamlOCamlO      
    Caml  amlO  mlO   lOCam      OCaml        mlOC      mlO    OCa       
    lOC    Cam OCa     mlOC     amlOCa        CamlO    OCam    ml        
   Caml    lOCamlO              OCa lOC       lOCam    m OC    Ca        
   lOCa    amlOCa              amlOCamlO      amlOCa  OC ml    lO        
    mlO    OCa lO              OCamlOCam      OC mlOC ml Ca    aml       
    Cam    mlO am       Ca    amlO   lOCa     ml CamlOC  lOC  lOCa    am 
    lOCa  OCam OCa     mlO    OCa     mlO     Cam OCaml Caml   mlO    OC 
     mlOCamlO   lOCamlOCam  CamlOC   OCamlO amlOCa lOC mlOCamlOCamlOCaml 
      amlOCa     mlOCaml    lOCaml   mlOCam  CamlO am  CamlOCamlOCamlOCa 


   ngsbinding   ndings indin   inding  indings     ngsbin ingsb   ingsbi    gsbin        ding  i 
   gsbindingsb  dingsb nding   ndings  ndingsbin   gsbind ngsbi   ngsbin   gsbindings  ndingsbind
   sbind   sbi   ngsb   ingsb   ings   dingsbindi   bind   sbind   sbin   gsbi  ings  nding  indi
     ndi   bin    sbi   ngsbin  ngs    ings   ding   ndi   bindin  bin   gsbi    gsb  dings      
     dingsbind   sbi    gsbindi gsb     gsb   ings  ndi    indings ind   sbi           ngsbi     
     ingsbindin   ind   sbindin sbi     sbi    gsb   ing   ndingsb ndi   bin              indin  
     ngsbindings  nd    bin ingsbin     bin   gsbi   ng    din sbindin   ind   sbindi      ding  
     gsb    ngsb  di    ind ngsbind    bind  gsbin   gs    ing binding    di    indin       ngsb 
     sbindingsbi ding   ndi  sbindi   bindingsbind  gsbi   ngs  ndings    ing   ndin  bind  gsbi 
   gsbindingsbi dingsb nding  indin   indingsbind  gsbind ngsbi  ingsb     gsbindin   indingsbin 
   sbindingsbi  ingsbindings   ding   ndingsbin    sbindingsbin   gsbi      bindings  ndingsbin  

"

let () =
  Printf.eprintf "data length: %d\n" (String.length data);
  let strm = new_lzma_stream() in
  lzma_easy_encoder ~strm ~level:9 ~preset:[(*LZMA_PRESET_EXTREME*)];  (* select xz file format *)
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

