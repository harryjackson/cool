0xdaccaaa ;magic
0         ;major
0         ;minor
.constants
;imports need to be added here
def:globals:8:{
  db:1:S:"Test1"
  db:2:D:2.0
  db:3:D:100.0
  db:4:D:50.0
  db:5:I:1
}
.methods
main:(IS)(I) {
  ldk  ,   r2,  $2
  ldk  ,   r3,  $3    
  ldk  ,   r4,  $4
  div  ,   r5,  r3,  r2
  eq   ,   r1,  r5,  r4
  jmp  ,   7
  jmp  ,   8   
  halt ,   r1
  halt ,   r0
}
.bytecode
