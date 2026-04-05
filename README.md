# malbolge-c

i spent some time writing a malbolge interpreter in pure c89. no external libs, no modern c features, just the standard library and some ternary math that probably shouldn't exist.

if you don't know malbolge, count yourself lucky. it was designed in 1998 to be the literal "hell" of programming languages. it's self-modifying, uses base-3 (ternary) logic, and encrypts its own instructions after every single cycle. writing a "hello world" in it took the original creator two years. 

anyway, i spent some time in the dark with this one. here's what i've got.

### the flex
*   **pure ansi c (c89):** compiled with `-ansi -pedantic -Wall -Werror`. no `//` comments, no inline variable declarations. it's 1989 in here.
*   **zero dependencies:** just `<stdio.h>`, `<stdlib.h>`, and some bit-shifting.
*   **o(1) crazy op:** most interpreters loop 10 times for every "crazy" ternary operation. i precomputed a 5-trit half-word lookup table ($3^5 = 243$) to make it actually fast. 
*   **verbose trace:** if you're brave enough to try and debug malbolge, run it with `-v`. it'll show you the registers (a, c, d) and the decoded opcodes as they happen.

### how to build
just use `gcc`. or `clang`. or whatever you have.

```bash
gcc -ansi -pedantic -Wall malbolge.c -o malbolge
```

### running it
to run a script:
```bash
./malbolge hello.mb
```

if you want to see the vm actually suffering (the trace mode):
```bash
./malbolge -v hello.mb
```

### the internals
*   **mem:** 59,049 words (exactly $3^{10}$).
*   **registers:** `A` (accumulator), `C` (code pointer), `D` (data pointer).
*   **encryption:** handled via a 94-character translation string that scrambles memory after every execution.

it's not pretty, and the math is kind of a headache, but it works. if you find a bug, honestly, just let it be. malbolge was meant to be broken.

- z
