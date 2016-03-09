\mainpage 

Cool Virtual Machine

\section Introduction

I'm still trying to figure out Doxygen at the moment. This means this page will
likely look a little odd as I learn it's markup etc.

Most of this is randmom musings ie thoughts about language design etc. Some
items might not even be possible.


\section Aims

The aim of this is education at this point. In particular I want to learn more 
about

1. Compilers, Interpreters and Virtual Machines
2. [Communicating Sequential Process](https://en.wikipedia.org/wiki/Communicating_sequential_processes)

I've read a lot about VM's and compilers but knowledge != understanding. Anyone
that's hacked on any of the other popular VM's will probably see a lot of
similarities here except what's here is likely to be in very bad condition.

As I've been building the VM this I've realised that it's becaming a single
CPU/Core ie there needs to be something that manages the cores. The actual VM
is a layer of abstraction above where my thinking is/was. Considering that I'd
like to play with concurrency issues my "VM" as it stands is at the wrong layer
of abstraction.

The Cloud where we have thousands of nodes/machines running is a case in point.
It should be possible today to map the cloud into some sort of Virtual Cloud ie
instead of a Virtual Machine you have a Cloud Machine. The fact that the cloud
is distributed among machines makes the word virtual redundant it's all virtual
because it's all managed using software but there's no [operating
system](https://en.wikipedia.org/wiki/Distributed_operating_system)

I'm not attempting anything so bold as a distributed OS but communication of 
independant processes in a network is something I'd like to achieve even if 
it's only doing simple tasks.. 

\section Installation

The entire project is written in C.

I use Cmake where the src directory is ./src and build is ./build.  Cmake
generates an XCode project for me that I then use for all development. I'm 
using an out of source build so I can compeltely blast everythign under 
./build and start from scratch. 


To generate an XCode project do this...

````
cd ./build
cmake -G Xcode ../
````

Then open the project in Xcode and you should be able to run some of the tests 
etc

\section Cool Assembly Language (casm)

\subsection Casm Intro

I think the name Casm is perfect because there's a casm between Casm and real
assembler. The VM willl load casm files in ascii or binary. I've chosen ascii
to work on now because you can write test files using Vim and it's much easier
to think about the machine when you can read what it reads in plain text. 

The frontend and backend API is entirely defined by Casm. I'm trying to avoid
coupling the backend to a particular language at this time because I believe it
will lead to a cleaner overall design and make designing the actual FE and it's
semantics easier to reason about if the BE has a clean separation.

The Casm file format belongs to the backend.

\subsection Casm Features

Some  things are very difficult to impplement, in particular anything to do
with the VM working in a network environment with other VM's in some sort of
distributed OS. I believe error handling might be an unsolvable problem ie
there are two many tradeoffs with any of the popular methods that make [none of
them great.](https://en.wikipedia.org/wiki/Perfect_is_the_enemy_of_good)

\subsection Must have features

Some of these sound simple enough. I've listed them here so anyone can quckly 
see what exists and what doesn't.

<table>
<caption id="multi_row">Basic Features</caption>
<tr>
  <th>Feature</th>
  <th>Sub Feature</th>
  <th>Not Started</th>
  <th>Partial</th>
  <th>Done</th>
</tr>
  <tr><td rowspan="2">Basic Math</td><td>Add/Sub/Mul/Div    </td><td>.</td><td/><td style="background-color:#008080">Done</td></tr>
  <tr>                           <td>Pow/Mod                </td><td>.</td><td/><td style="background-color:#008080">Done</td></tr>
  <tr><td>Function call     </td><td>Nested and Single      </td><td>.</td><td/><td style="background-color:#008080">Done</td></tr>
  <tr><td>Comparisons        </td><td>LT/EQ/LE               </td><td>.</td><td/><td style="background-color:#008080">Done</td></tr>
  <tr><td>Jump              </td><td>Unconditional          </td><td>.</td><td/><td style="background-color:#008080">Done</td></tr>
  <tr><td>Load Constants    </td><td>From constant table    </td><td>.</td><td/><td style="background-color:#008080">Done</td></tr>
  <tr><td>Sets and Movs     </td><td>From Main memory       </td><td>.</td><td style="background-color:#ffffc2">Partial</td><td>.</td></tr>
  <tr><td>String Ops        </td><td>Concat / Find          </td><td style="background-color:#ff5d59">Not Started</td><td/><td></td></tr>
  <tr><td>Load Object Files </td><td>import                 </td><td style="background-color:#ff5d59">Not Started</td><td/><td></td></tr>
  <tr><td>Objects           </td><td>New Delete             </td><td style="background-color:#f75d59">Not Started</td><td/><td></td></tr>
</table>


\subsection Base Wish list

The basic list does not imply simplest to develop it just sets the base set of
features required to enable me to play with problems in concurrency. Having
multiple cores is more important than green threads because we can play with
concurrency issues using a VM per OS thread. If I manage to design a simple VM
with these feature I'd consider it a success ie I'd have learned an awful lot
just by doing this because at the outset how do so this was purely theoretical.

1. Import Objects. Sounds trivial enough but this is inherently tricky. It's
   also really important ie Java's import, Perl's use etc define a large part
   of how the runtime works and behaves ie who's not fought with CLASSPATH in
   java.
2. Multicore VM ie the VM should have multiple cores and this should be
   supported by Casm.
3. Error handling by the VM ie we should do our best to provide decent errors 
   that make it easy to see where the problem is. 
4. Design a simple Frontend Language that targets Casm and supports concurrency 
   ins some form. This is vague because I don't know what method I'll use yet.

\subsection Larger Wish list

Some other features that would be great to add

0. Green threads.
1. Closures.
2. Debugging should be builtin ie the ability to call "BREAK" in Casm and have
   a debugger appear on a port number that can then be queried.
3. Events/Signals. These should be native. Not sure what this means yet but it
   should be easy to implement events loops etc in the Frontend. I'd also like
   to add things like events on functions etc.
4. Garbage collection?  Is there any benefit to supporting both explicit and
   implicit garbage collection? Garbage collection is a massive topic and I've
   found some great reading on it?
5. Tail Calls. This is not easy in C but doable. Not having tail calls will
   make it hard to support functional languages. Note, I've already made our
   stack a list of stack frames as opposed to a fixed block of memory to help
   with things like this.
6. Benchmarkingi/Profiling. While I've read a lot of material saying that
   performance is not something to worry about when designing a language it's
   how almost all language get ranked.
7. Testing. Testing should also be builtin. I like how golang does it.
8. Support a REPL?
9. Compile Casm to C
10. Error handling? Open question. I think this is probably the single hardest
    thing to get right because there is no correct way to do it. Exceptions
    suck, passing errors around suck.


Garbage collection is tough and I think I could spend years on it. I don't
think I'll be able to work on any serious garbage collector so I might
implement a half way house. The [Hans Boem Collector](http://hboehm.info/gc/)
may be the way to go. I think Guile is using the Hans Boem collector. Mark and
sweep may be the simplest with it's obvious issues.

I read recently of a real time OS (RTOS) that would allocate frames for time periods
and when the time ran out it would remove the whole thread and start again.
This means the OS was in charge of garbage collection. I love this idea. Theres
something inherently robust about it which is what you'd expect from a RTOS.

Another idea would be to provide memory pools that can be used and deallocated
all at once ie in a function we create a memory pool. All subsequent calls
would allocate from this pool and when complete would deallocate the whole
thing.  This would impose limits on threading etc ie what pool to choose from
but if every thread is a new VM this might work ok.  Note, I'm not hiding this
from the Frontend I think the FE would need to 

The memory pool idea would allow me to get to a half way house ie some memory
collection in the FE language or the FE language specifies which memory pool
the object lives in and some memory pools are collected using different methods
or not at all. As an example using fake annotations we could do it like.


````
newIO(String file) (IO) {
  String realfile = file ++ '.txt';//declared on stack
  #@permpool
  IO file = IO(realfile); // declared in permanenet pool
  //The file object has been added to the permanent pool and will never be
  //garbage collected. The realfile object will be deallocated on return from
  //the function.
  return file;
}
````

Problems

````C
IO newFile(String file) {
  #@permpool
  IO file = IO(file);
  return file;
}

void parent() {
  IO file = newIO("/tmp/foo");
  file.unlink();
  //we have a memory leak here because the object 
  //was added to the permanent pool.
  // we'd have to xplicity free it ie 
  file.delete();
  return;
}
````

Would it be possible to have garbage collection and the ability to free object
explicitly. This might be a way to avoid expensive stop the world collections.
This is one area that's really interesting. I think I may need to invest in the
following book before embarking on some half assed attempt that's been put to
bed as a bad idea by people smarter than me.

[The Garbage Collection Handbook](http://amzn.com/1420082795)


\subsection Casm Calling Conventions

This has caused me no end of consternation. A fixed convvention is easy to write but 
it's in part starting to limit the frontend. I like the idea of a fixed set because 
it's simple to implement and easy to explain to people.



\section Class Loader 
The class loaded is where we load each file into the ...

\section Scoping

I think wars have been fought over this. Here are some question that need 
answering

1. Should each Object be completely private ie no mutable state exported unless
it's in a function. Personally YES!. Objects expose interfaces not data.
2. Should exporting be process/thread based ie you don't call a function you send 
it a message 
3. Import, should you be able to import the whole library or make it explicit what 
gets pulled in ie You must explicitly import anything you want ie you cannnot 
"import math" and get "cos/sin/tan" etc. You need to expilicitly import each 
one. I'd imagine this would be a PITA but it might make things run faster. 
import math.cos;
import math.sin;
import math.tan;

vs

import math.\*;

Personally most time I see wildcards imports is laziness, and laziness is not 
always a bad thing.

Or we offload that to the compiler and it only loads what's needed. This would
be difficult to implement if we allow first class functions ie tracking what
has and has not been loaded is hard.


4. Should we use "link" and "use" ie we can "link" to a library and "use" 
its exported functions.

5. A function that's imported does not pollute the namespace ie "import math.sin;"
does not mean you can use it like "var f = sin(foo);" ie namespaces are never 
polluted.

\section Error Handling

For the record I don't like Exceptions in Java/C++ etc. I prefer explicit error 
handling even though it peppers the code full of boiler plate.

Interesting ways to implemetn error handling

1. Result objects in place of normal return types.
2. Multiple return types
3. Code at the end of the function that gets called on 
error detection from any function called.


Great read on Erlangs philosophy on errors.
[Zen of Erlang](http://ferd.ca/the-zen-of-erlang.html)

Joe Duffy covers a lot of ground on various languages error handling here.
[The Error Model](http://joeduffyblog.com/2016/02/07/the-error-model/)

[Andrew Binstock, some thought son error handling](http://www.drdobbs.com/architecture-and-design/the-scourge-of-error-handling/240143878)

Some pseudo code

Go's error handling

````Go
f, err := os.Open("filename.ext")
if err != nil {
    log.Fatal(err)
    }
    //do something with the open *File f
//.....
if err := dec.Decode(&val); err != nil {
    if serr, ok := err.(*json.SyntaxError); ok {
        line, col := findLine(f, serr.Offset)
        return fmt.Errorf("%s:%d:%d: %v", f.Name(), line, col, err)
    }
    return err
}
````
I think Rust's error handling is also interesting.
[Rusts error handline](https://doc.rust-lang.org/book/error-handling.html)


````Rust
// Searches `haystack` for the Unicode character `needle`. If one is found, the
// byte offset of the character is returned. Otherwise, `None` is returned.
fn find(haystack: &str, needle: char) -> Option<usize> {
    for (offset, c) in haystack.char_indices() {
        if c == needle {
            return Some(offset);
        }
    }
    None
}

fn main() {
    let file_name = "foobar.rs";
    match find(file_name, '.') {
        None => println!("No file extension found."),
        Some(i) => println!("File extension: {}", &file_name[i+1..]),
    }
}

use std::num::ParseIntError;

fn double\_number(number\_str: &str) -> Result<i32, ParseIntError> {
    match number_str.parse::<i32>() {
        Ok(n) => Ok(2 * n),
        Err(err) => Err(err),
    }
}

fn main() {
    match double_number("10") {
        Ok(n) => assert_eq!(n, 20),
        Err(err) => println!("Error: {:?}", err),
    }
}


````
func fileOpener(string filepathbad)
// propogate the error
file := File(filepathbad).err(){
  return Error("Bad file path" ++ filepathbad);
};

//Handle the error
file := try:File("/badfilepath.txt").(/dev/null);


````



\section Language Design Reading

Where I find interesting articles I'll drop them in here...

[Great synospis of the different C++ compilers etc.](http://www.agner.org/optimize/calling_conventions.pdf)

[Preprocessor reading](http://www.keithschwarz.com/cs106l/spring2009/handouts/080_Preprocessor_2.pdf)

[Agner Fogs Optizations pages](http://www.agner.org/optimize/)

[C-- Interesting intermediate representation, I think it was adopted by Haskell](http://www.cs.tufts.edu/~nr/c--/extern/man2.pdf)

C-- is a classic example of why adopting something like it can lead to all sorts of 
problems ie The cminusminus.org site has been hijacked by a spam website and other than 
haskell it seems to have died a death. I did find some code on github that mentions Quick 
C--.

[Calling Conventions Some work by Norman Ramsey and  Christian Lindig here](https://www.cs.tufts.edu/~nr/pubs/custom.pdf)

Anything by Walter Bright is worth reading
[C's biggest mistake](http://www.drdobbs.com/architecture-and-design/cs-biggest-mistake/228701625)
[So you want to write your own language](http://www.drdobbs.com/architecture-and-design/so-you-want-to-write-your-own-language/240165488?queryText=walter)
[Speeding up the D Compiler. I loved how he tacked malloc](http://www.drdobbs.com/cpp/increasing-compiler-speed-by-over-75/240158941?queryText=walter)

[Research in Programming Languages](http://tagide.com/blog/academia/research-in-programming-languages/)

Good reading on C2. I think the important ones are the list where the flaws
actually got fixed ie they were considered bad enough to have something done
about them.
[Java Design Flaws](http://c2.com/cgi/wiki?JavaDesignFlaws)

[Worse if better in software](http://www.dreamsongs.com/RiseOfWorseIsBetter.html)

[Floating Point Numbers][http://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html]
