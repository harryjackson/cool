0xdaccaaa ;magic
0         ;major
0         ;minor
.constants
;imports need to be added here
def:globals:8:{
  db:1:S:"Test1"
  db:2:D:2.0
  db:3:D:3.0
  db:4:D:6.0
  db:5:I:1
}
.methods
main:(IS)(I) {
  ldk  ,   r1,  $2      ; load 2.0
  ldk  ,   r2,  $3      ; load 3.0    
  ldk  ,   r3,  $4      ; load 3.0    
  mul  ,   r4,  r1,  r2
  eq   ,   r5,  r3,  r4
  jmp  ,   7
  jmp  ,   8   
  halt ,   r1
  halt ,   r0
}
.bytecode
