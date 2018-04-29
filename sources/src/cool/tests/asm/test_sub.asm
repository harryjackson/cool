0xdaccaaa ;magic
0         ;major
0         ;minor
.constants
;imports need to be added here
def:globals:8:{
  db:1:S:"Test1"
  db:2:I:2
  db:3:I:3
  db:4:I:1
  db:5:I:1
}
.methods





main:(IS)(I) {
  ldk  ,   r2,  $2      ; load 2.0
  ldk  ,   r3,  $3      ; load 3.0    
  ldk  ,   r4,  $4      ; load 3.0    
  sub  ,   r5,  r3,  r2
  eq   ,   r6,  r4,  r5
  jmp  ,   7
  jmp  ,   8   
  halt ,   r6
  halt ,   r0
}
.bytecode
