0xdaccaaa ;magic
0         ;major
0         ;minor
.constants
;imports need to be added here
;
def:globals:8:{
  db:1:S:"Test1"
  db:2:S:"argc"
  db:3:S:"argv"
  db:4:S:"a"
  db:5:D:1.0
  db:6:D:2.0
  db:7:D:5.0
  db:8:D:1000.0
  db:9:D:2.0
}
.methods
mul:(D)(D) {
  ldk  ,   r2,  $6
  mul  ,   r1,  r1,   r2
  ret  ,   r1
}
inc:(D)(D) {
  arg  ,   1,   r1
  call , mul:(D)(D)
  ldk  ,   r2,  $5
  add  ,   r1,  r1,   r2
  ret  ,   r1
}
main:(IS)(I) {
  ldk  ,   r1,  $7       ; ldk Load constant from constant table into register, this limits us to 2^16-1 constants :(
                         ; I'll likely add a new load operation to load into a default register constant whose index is >= 2^16
  ldk  ,   r2,  $8     
  lt   ,   r3,  r1,  r2  ; if(r1 < r2) pc += 1 ie execute next instruction
  jmp  ,   7             ; Jump if false 
  arg  ,   1,   r1
  call ,   inc:(D)(D)
  jmp  ,   2   
  halt ,   r1
}
.bytecode
