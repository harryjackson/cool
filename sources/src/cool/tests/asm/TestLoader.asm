0xdaccaaa ;magic
0         ;major
0         ;minor
.constants
def:globals:8:{
  db:1:C:TestLoader              ; Object
  db:2:F:TestLoader->Add:(II)(I) ; Function:this is an address
  db:3:F:Math->Add:(II)(I)       ; Function:this is also an address
  db:4:D:1.0                     ; Double
  db:5:I:1                       ; Integer
  db:6:S:"ASDASDA"               ; String
}
.methods
; I'm deliberately using the same signature in this 
; as the one in the Math.asm file.
Add:(II)(I) {
  ldk  ,   r1 ,   $5
  ldk  ,   r2 ,   $5
  arg  ,   1  ,   r1
  arg  ,   2  ,   r1 ;add  ,   r1 ,   r1 ,   r2
  send ,   $3
  ret  ,   r1
}
main:(IS)(I) {
  call ,   $2
  halt ,   r1
}
.bytecode
