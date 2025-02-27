
## Command line

```
cake [options] source1.c source2.c ...

SAMPLES

    cake source.c
    Compiles source.c and outputs /out/source.c

    cake -target=C11 source.c
    Compiles source.c and outputs C11 code at /out/source.c

cake file.c -o file.cc && cl file.cc
    Compiles file.c and outputs file.cc then use cl to compile file.cc

cake file.c -direct-compilation -o file.cc && cl file.cc
    Compiles file.c and outputs file.cc for direct compilation then use cl to compile file.cc

OPTIONS

  -I                    Adds a directory to the list of directories searched for include files
                        (On windows, if you run cake at the visual studio command prompt cake
                        uses the same include files used by msvc )

  -no-output            Cake will not generate ouput

  -D                    Defines a preprocessing symbol for a source file

  -E                    Copies preprocessor output to standard output

  -o name.c             Defines the ouput name. used when we compile one file

  -remove-comments      Remove all comments from the ouput file

  -direct-compilation   output without macros/preprocessor parts

  -target=standard      Output target C standard (c89, c99, c11, c2x, cxx)
                        C99 is the default and C89 (ANSI C) is the minimum target

  -std=standard         Assume that the input sources are for standard (c89, c99, c11, c2x, cxx)
                        (not implemented yet, input is considered C23)

  -fi                   Format input (format before language convertion)

  -fo                   Format output (format after language convertion, result parsed again)

  -no-discard           Makes [[nodiscard]] default implicitly

  -Wname -Wno-name      Enables or disable warning

  -sarif                Generates sarif files

```

One directory called **out** is created keeping the same directory structure 
of the input files.

For instance:

```c
cake c:\project\file1.c
```

output:

```
  c:\project
  ├── file1.c
  ├── out
      ├── file1.c
```

More files..

```
cake c:\project\file1.c c:\project\other\file2.c
```

output

```
  c:\project
  ├── file1.c
  ├── other
  │   ├── file2.c
  ├── out
      ├── file1.c
      ├── other
          ├── file2.c
```



## Pre-defined macros

```c
 #define __CAKE__ 202311L
 #define __STDC_VERSION__ 202311L
```

## Warnings

Warnings can be controlled with "-Wname" and "-Wno-name" and "-Wall".
When the same warning exists in gcc cakes uses the same name if possible.

See [warnings](warnings.html)

## C99 Transformations

C89 is the minimum target.

However the idea if C89 target is NOT support very old compilers, but generate code that can be compiled with C++.

C89 
https://port70.net/~nsz/c/c89/c89-draft.html

C99
https://open-std.org/JTC1/SC22/WG14/www/docs/n1124.pdf

```c
 #define __STDC_VERSION__ 199901L  //C99
```

### C99 restrict pointers

```c
void f(const char* restrict s);
```

Becomes in C89

```c
void f(const char* /*restrict*/ s);
```

The intended use of the restrict qualifier  is to promote optimization, removing it will not change the observable behavior.

C++ does not have standard support for restrict

N448

###  C99 Variable-length array (VLA) 

The idea is not implement variable length arrays with automatic storage duration. (\_\_STDC\_NO\_VLA\_\_ 1). 

But there are other uses of VLA.


```c
#include <stdlib.h>
#include <stdio.h>

int main() {
	int n = 2;
	int m = 3;
	int (*p)[n][m] = malloc(sizeof * p);

	printf("%zu\n", sizeof(*p));

	free(p);
}

```

Becomes C89 (not implemented)

```c
#include <stdlib.h>
#include <stdio.h>

int main() {
	int n = 2;
	int m = 3;
    
    /*these variables are created to store the dynamic size*/
    const int vla_1_n = n;
	const int vla_1_m = m;
    
	int (*p)[n][m] = malloc((vla_1_n*vla_1_m)*sizeof(int));

	printf("%zu\n", (vla_1_n*vla_1_m)*sizeof(int));

	free(p);
}

```

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n683.htm

### C99 Flexible array members

```c
struct s {
    int n;
    double d[]; 
};
```

Becomes (not implemented)

```c
struct s {
    int n;
    double d[]; //?
};
```



### C99 static and type qualifiers in parameter array declarators

```c
#include <stdlib.h>

void F(int a[static 5]) {
}

int main() 
{    
    F(0);
    F(NULL);
    F(nullptr);

    int a[] = {1, 2, 3};    
    F(a);//error
    
    int b[] = { 1, 2, 3 , 4, 5};
    F(b); 

    int c[] = { 1, 2, 3 , 4, 5, 6};
    F(c);
}

```
  
`static` is removed when target is < c99.

Cakes verifies that the argument is an array of with equal or more elements.
  
Cakes extend this check for arrays without static as well.

### C99 Complex and imaginary support
TODO

### C99 Universal character names (\u and \U)
TODO

###  C99 Hexadecimal floating constants

```c
double d = 0x1p+1;
```
Becomes in C89

```c
double d = 2.000000;
```

>Cake converts the hexadecimal floating to decimal
>floating point using strtod then snprintf.
>That means this conversion is not precise.
>


### C99 Compound literals


```c
struct s {
  int i;
};

int f(void) {
  struct s * p = 0, * q;
  int j = 0;
  again:
    q = p, p = & ((struct s) { j++ });
  if (j < 2) goto again;
  return p == q && q -> i == 1;
}
```
Becomes in C89 (not implemented yet)

```c
struct s {
  int i;
};
int f(void) {
  struct s * p = 0, * q;
  int j = 0;
  again:
    struct s compound_literal_1 = { j++ };
    q = p, p = & compound_literal_1;
  if (j < 2) goto again;
  return p == q && q -> i == 1;
}
```
N716
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n716.htm

### C99 Designated initializers

```c
 int main()
 {
  int a[6] = {[4] = 29, [2] = 15 };

  struct point { int x, y; };

  struct point p = { .y = 2, .x = 3 }
 }

```

Becomes C89 (not implemented yet)

```c
int main()
{
  int a[6] = { 0, 0, 15, 0, 29, 0 };
  struct point { int x, y; };
  struct point p = { 3, 2 }
}
```

N494
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n494.pdf

### C99 Line comments
When compiling to C89 line comments are converted to comments.

### C99 inline functions
TODO
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n741.htm

### C99 _Pragma preprocessing operator
TODO 

### C99 \_\_func\_\_ predefined identifier
Parsed. C89 conversion not implemented yet.

###  C99 Variadic macros

Yes. We need to expand the macro when comping to C89.
This is covered by # macro expand.

Sample:

```c

#include <stdio.h>

#define debug(...) fprintf(stderr, __VA_ARGS__)
#pragma expand debug

int main()
{
  int x = 1;
  debug("X = %d\n", 1);
}

```
Becomes

```c
#include <stdio.h>

#define debug(...) fprintf(stderr, __VA_ARGS__)
#pragma expand debug

int main()
{
  int x = 1;
  fprintf(stderr, "X = %d\n", 1);
}

```

I am considering to mark the debug macro to be expanded automatically
if \_\_VA\_ARGS\_\_ is used. Then pragma expand will not be necessary.

N707
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n707.htm


###  C99 \_Bool
When compiling to C89 _Bool is replaced by unsigned char.

```c
//line comments
int main(void)
{
    _Bool b = 1;
    return 0;
}
```

Becomes in C89

```c
/*line comments*/
int main(void)
{
    int b = 1;
    return 0;
}
```

Alternative design - typedef ?
Considering C23 has bool and the objective of C89 version is to have a version that compiles in C++ the best option would be use bool, true, false.

## C11 Transformations

```c
#define __STDC_VERSION__ 201112L //C11
```


https://open-std.org/JTC1/SC22/WG14/www/docs/n1570.pdf

https://files.lhmouse.com/standards/ISO%20C%20N2176.pdf

###  C11 \_Static\_assert

When compiling to versions C89, C99 \_Static\_Assert is removed.

Alternative design - macro ?

### C11 Anonymous structures and unions
TODO

### C11 \_Noreturn

```c
_Noreturn void f () {
  abort(); // ok
}
```

Expected in C99, C89 (not implemented yet)

```c
/*[[noreturn]]*/ void f () {
  abort(); // ok
}
```

Alternative design - macro ?

Note: 
C23 attribute [[noreturn]] provides similar semantics. The _Noreturn function specifier is 
 an obsolescent feature



###  C11 Thread_local/Atomic
Parsed but not transformed.

###  C11 type-generic expressions (\_Generic)

When compiling to C99, C89 we keep the expression that matches the type.

For instance:

The expression that matches the argument 1.0 is **cbrtl**.

The result of \_Generic in C99 will be cbrtl. Because this is inside a macro we need to tell the transpiler to expand that macro using pragma expand.

N1441
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1441.htm

```c
#include <math.h>

#define cbrt(X) _Generic((X),    \
                  double: cbrtl, \
                  float: cbrtf , \
                  default: cbrtl \
              )(X)

#pragma expand cbrt

int main(void)
{
    cbrt(1.0);
}

```

Becomes in C99, C89

```c

#include <math.h>

#define cbrt(X) _Generic((X),    \
                  double: cbrtl, \
                  float: cbrtf , \
                  default: cbrtl \
              )(X)

#pragma expand cbrt

int main(void)
{
     cbrtl(1.0);
}

```

###  C11 u8"literals"

u8 literals are converted to escape sequences.

```c
char * s1 = u8"maçã";
char * s2 = u8"maca";
```

Becomes in C99, C89

```c
char * s1 = "ma\xc3\xa7\xc3\xa3";
char * s2 = "maca";
```

N1488
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1488.htm

### C11 _Alignof or C23 alignof
When compiling to C99 or C89 it is replaced by the equivalent constant.

```c
 int main()
 {
   int align = alignof(int);
 }
```

becomes

```c
 int main()
 {
   int align = 4;
 }
```

### C11 _Alignas or C23 alignas

Not implemented. 

## C23 Transformations

https://open-std.org/JTC1/SC22/WG14/www/docs/n3096.pdf

```c
#define __STDC_VERSION__ 201710L  //C17
#define __STDC_VERSION__ 202311L  //C23
```

###  C23 \_Decimal32, \_Decimal64, and \_Decimal128
Not implemented (maybe parsed?)

### C23 static\_assert / single-argument static_assert
In C23 static\_assert is a keyword and the text message is optional.

Whe comping to C11, static\_assert is replaced by it C11 version \_Static\_assert. If the static\_assert has only one argument the text becomes "error".

N1330
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1330.pdf

```c
int main()
{    
    static_assert(1 == 1, "error message");
    static_assert(1 == 1);
}

```
Becomes in C11

```c
int main()
{    
    _Static_assert(1 == 1, "error message");
    _Static_assert(1 == 1, "error");
}
```
In C99, C89 it is replaced by one space;

###  C23 u8 character prefix
not implemented yet.

https://open-std.org/JTC1/SC22/WG14/www/docs/n2418.pdf

### C23 No function declarators without prototypes
https://www.open-std.org/JTC1/SC22/WG14/www/docs/n2841.htm

Yes

```c
int main(){
    func(); //this is an error in C23
}
```

Before C23 this was a warning.

See also Remove support for function definitions with identifier lists
https://open-std.org/JTC1/SC22/WG14/www/docs/n2432.pdf



### C23 Improved Tag Compatibility
No.
To transform this to previous version we could generate a typedef.

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3037.pdf

### C23 Unnamed parameters in function definitions
  
```c
int f(int );

int f(int ) {
}
```

https://open-std.org/JTC1/SC22/WG14/www/docs/n2480.pdf

TODO: But we need to add a dummy variable when compiling to versions C < 23.

### C23 Digit separators

```c
int main()
{
    int a = 1000'00;
}
```
Becomes in C11, C99, C89

```c
int main()
{
    int a = 100000;
}x  
```

This transformation uses only tokens. 
So even preprocessor and inactive blocks are transformed.

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2626.pdf

### C23 Binary literals 

```c
#define X  0b1010

int main()
{
    int a = X;
    int b = 0B1010;
}

```
Becomes in C11, C99, C89

```c
#define X  0xa

int main()
{
    int a = X;
    int b = 0xa;
}

```

>This transformation uses only tokens. So even preprocessor and inactive
>code is transformed.


### C23 Introduce the nullptr constant

```c

int main()
{
  void * p = nullptr;
  auto p2 = nullptr;
  typeof(nullptr) p3 = nullptr;
}

```

Becomes in < C23

```
int main()
{
  void * p = ((void*)0);
  void  * p2 = ((void*)0);
  void  * p3 = ((void*)0);
}

```

https://open-std.org/JTC1/SC22/WG14/www/docs/n3042.htm

### C23 Make false and true first-class language features

When compiling to C89 bool is replaced by int true by 1 and false 0. 
(I am considering adding an extra header file in C89 with bool definition.)

When compiling to C99 and C11 bool is replaced with **_Bool**, true is replaced with `((_Bool)1)` 
and false with **(_Bool)0)**


https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2935.pdf

###  C23 {} empty initializer

```c

int main()
{
    struct X {
        int i;
    } x = {};

    x = (struct X) {};

    struct Y
    {
        struct X x;
    } y = { {} };
}  

```

Becomes in C11, C99, C89

```c

int main()
{
    struct X {
        int i;
    } x = {0};

    x = (struct X) {0};

    struct Y
    {
        struct X x;
    } y = { {0} };
}

```

> Note: Cake code is 100% equivalent because it does not make padding bit zero.

###  C23 auto


```c
static auto a = 3.5;
auto p = &a;

double A[3] = { 0 };
auto pA = A;
auto qA = &A;

```
Expected result (not implemented yet)

```c
static double a = 3.5;
double * p = &a;

double A[3] = { 0 };
double* pA = A; //TODO
auto qA = &A; //TODO?
```

https://open-std.org/JTC1/SC22/WG14/www/docs/n3007.htm


###  C23 typeof / typeof_unqual


```c

#define SWAP(a, b) \
  do {\
    typeof(a) temp = a; a = b; b = temp; \
  } while (0)

#pragma expand SWAP

int main()
{
    /*simple case*/
    int a = 1;
    typeof(a) b = 1;

    /*pay attention to the pointer*/
    typeof(int*) p1, p2;

    /*let's expand this macro and see inside*/
    SWAP(a, b);

    /*for anonymous structs we insert a tag*/
    struct { int i; } x;
    typeof(x) x2;
    typeof(x) x3;

   /*Things get a little more complicated*/
   int *array[2];
   typeof(array) a1, a2;
   
   typeof(array) a3[3];
   typeof(array) *a4[4];

   /*abstract declarator*/
   int k = sizeof(typeof(array));


   /*new way to declare pointer to functions?*/
   typeof(void (int)) * pf = NULL;
}

```
Becomes in C11, C99, C89

```c


#define SWAP(a, b) \
  do {\
    typeof(a) temp = a; a = b; b = temp; \
  } while (0)

#pragma expand SWAP

int main()
{
    /*simple case*/
    int a = 1;
    int  b = 1;

    /*pay attention to the pointer*/
    int  *p1,  *p2;

    /*let's expand this macro and see inside*/
     do {int temp = a; a = b; b = temp; } while (0);

    /*for anonymous structs we insert a tag*/
    struct _anonymous_struct_0 { int i; } x;
    struct _anonymous_struct_0  x2;
    struct _anonymous_struct_0  x3;

   /*Things get a little more complicated*/
   int *array[2];
   int  *a1[2],  *a2[2];
   
   int  *(a3[3])[2];
   int  *(*a4[4])[2];

   /*abstract declarator*/
   int k = sizeof(int*[2]);


   /*new way to declare pointer to functions?*/
   void  (*pf)(int) = ((void*)0);
}


```

https://open-std.org/JTC1/SC22/WG14/www/docs/n2927.htm
https://open-std.org/JTC1/SC22/WG14/www/docs/n2930.pdf

### C23 Improved Normal Enumerations

https://open-std.org/JTC1/SC22/WG14/www/docs/n3029.htm

###  C23 constexpr 

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3018.htm

Cake convert constexpr declarator with a cast and the value.
addressof constexpr declarator is not implemented.

```c
#include <stdio.h>

constexpr int c = 123;

int a[c];

constexpr double PI = 3.14;

static_assert(PI + 1 == 3.14 + 1.0);

int main()
{
   printf("%f", PI);
}

```
Becomes

```c
#include <stdio.h>

const int c = 123;

int a[((int)123)];

const double PI = 3.14;



int main()
{
   printf("%f", ((double)3.140000));
}


```

###  C23 Enhancements to Enumerations


```c
enum X : short {
  A
};

int main() {
   enum X x = A;   
}
```

Becomes

```c
enum X {
  A
};

int main() {
   short x = ((short)A);   
}
```

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3030.htm


###  C23 Attributes


Conversion to C11, C99, C89 will just remove the attributes.

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2335.pdf
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2554.pdf

Related: Standard Attributes in C and C++ - Timur Doumler - ACCU 2023
https://youtu.be/EpAEFjbTh3I


### C23 fallthrough attribute
Not implemented

https://open-std.org/JTC1/SC22/WG14/www/docs/n2408.pdf

### C23 deprecated attribute

Partially implemented
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2334.pdf

### C23 maybe_unused attribute

Implemented
https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2270.pdf

### C23 nodiscard attribute
Partially implemented

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2267.pdf

https://open-std.org/JTC1/SC22/WG14/www/docs/n2448.pdf

### C23 [[unsequenced]] and [[reproducible]]

Parsed.

https://open-std.org/JTC1/SC22/WG14/www/docs/n2956.htm


###  C23 \_\_has\_attribute

It is implemented in C23.

Conversion to C11, C99 C89 not defined yet.


###  C23 \_\_has\_include

```c

#if __has_include(<stdio.h>)
#warning  YES
#endif

#if __has_include(<any.h>)
#warning  YES
#else
#warning  NO
#endif

```

Becomes in C11, C99, C89.

```c
Not defined yet
```


###  C23 #warning
When compiling to versions < 23 it is commented out.

The compiler also outputs the message on stderr.

```c
int main()
{
  #warning my warning message  
}
```
Becomes

```c
int main()
{
  //#warning my warning message  
}
```

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2686.pdf

###  C23 #embed

```c
#include <stdio.h>

int main()
{
  static const char file_txt[] = {
   #embed "stdio.h"
   ,0
  };

  printf("%s\n", file_txt);

}
```
Becomes in C11, C99, C89

```c

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3017.htm

#include <stdio.h>

int main()
{
  static const char file_txt[] = {
    35,112,114,/*lot more here ...*/ 13,10
   ,0
  };

  printf("%s\n", file_txt);

}

```

I am considering add an option to generate a file with a suffix
like "embed_stdio.h" then the equivalent code will be:

Becomes in C11, C99, C89

```c
#include <stdio.h>

int main()
{
  static const char file_txt[] = {
   #include "embed_stdio.h"
   ,0
  };

  printf("%s\n", file_txt);

}

```



###  C23 #elifdef #elifndef

```c
#define Y

#ifdef X
#define VERSION 1
#elifdef  Y
#define VERSION 2
#else
#define VERSION 3
#endif
```

Becomes C11, C99, C89

```c
#define Y

#ifdef X
#define VERSION 1
#elif defined   Y
#define VERSION 2
#else
#define VERSION 3
#endif

```


###  C23 \_\_VA_OPT\_\_

Requires #pragma expand. (TODO make the expansion automatic)

```c

#define F(...) f(0 __VA_OPT__(,) __VA_ARGS__)
#define G(X, ...) f(0, X __VA_OPT__(,) __VA_ARGS__)
#define SDEF(sname, ...) S sname __VA_OPT__(= { __VA_ARGS__ })
#define EMP

/*maybe this could be automatic if <C23*/
#pragma expand F
#pragma expand G
#pragma expand SDEF
#pragma expand EMP

void f(int i, ...) {}


int main()
{
  int a = 1;
  int b = 2;
  int c = 3;
  
  F(a, b, c);
  F();
  F(EMP);
  G(a, b, c);
  G(a, );
  G(a);

}

```


Becomes in < C23

```c

#define F(...) f(0 __VA_OPT__(,) __VA_ARGS__)
#define G(X, ...) f(0, X __VA_OPT__(,) __VA_ARGS__)
#define SDEF(sname, ...) S sname __VA_OPT__(= { __VA_ARGS__ })
#define EMP

/*maybe this could be automatic if <C23*/
#pragma expand F
#pragma expand G
#pragma expand SDEF
#pragma expand EMP

void f(int i, ...) {}


int main()
{
  int a = 1;
  int b = 2;
  int c = 3;
  
   f(0, a, b, c);
   f(0 );
   f(0);
   f(0, a, b, c);
   f(0, a );
   f(0, a );

}
```

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3033.htm

###  C23 BitInt(N))

Not implemented


### C23 Compound Literals with storage specifier

```c
void F(int *p){}

int main()
{
   F((static int []){1, 2, 3, 0})
}
```

becomes (not implemented yet)

```c
void F(int *p){}

int main()
{
    static int _compound_1[] = {1, 2, 3, 0};
    F(_compound_1);
x   }
```

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3038.htm

### C23 Variably-modified (VM) types

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2778.pdf


## Extensions (Not in C23)

###  Extension - try catch throw


```
   try-statement:
      try secondary-block
      try secondary-block catch secondary-block   
```

```
jump-statement:
  throw;
```

try catch is a external block that we can jump off.

try catch is a LOCAL jump only and this is on purpose not a limitation.


catch block is optional.

```c
try
{
   for (int i = 0 ; i < 10; i++) {
      for (int j = 0 ; j < 10; j++) {
        ... 
        if (error) throw;
        ...
      }
   }
}
catch
{
}
```

###  Extension - defer

*defer* will call the defer statement before the block exit at inverse order of declaration.

```
     defer-statement:
        defer secondary-block
```

For instance:

```c
#include <stdio.h>

int main() {
  do {
     FILE* f = fopen("in.txt", "r");
     if (f == NULL) break;
     defer fclose(f);

     FILE* f2 = fopen("out.txt", "w");
     if (f2 == NULL) break;
     defer fclose(f2);
     //...    
  }
  while(0);
}
```

Becomes in C23, C11, C99, C89

```c
#include <stdio.h>

int main() {
  do {
     FILE* f = fopen("in.txt", "r");
     if (f == ((void*)0)) break;

     FILE* f2 = fopen("out.txt", "w");
     if (f2 == ((void*)0)) {  fclose(f); break;}
     
     fclose(f2); fclose(f);
   }
  while(0);
}
```
I guess everthing is working including **goto** jumps.

###  Extension - if with initializer

No idea why C++ 17 if with initializer was not proposed for C23!
But in cake it is implemented.

```c
#include <stdio.h>
int main()
{
   int size = 10;
   if (FILE* f = fopen("file.txt", "r"); f)
   {
     /*...*/
     fclose(f);
   }
}
```
Becomes

```c
#include <stdio.h>

int main()
{
   int size = 10;
   {FILE* f = fopen("file.txt", "r");if ( f)
   {
     /*...*/
     fclose(f);
   }}
}
```
An extension if + initializer + defer expression was considered but not implemented yet.

###  Extension Literal function - lambdas

Lambdas without capture where implemented using a syntax similar of 
compound literal for function pointer.

Lambdas are the most complex code transformation so far because sometimes function scope 
types needs to be transformed to file scope. This is important because manual lambda capture
is something we want to use in function scope.

For instance:

```c
extern char* strdup(const char* s);
void create_app(const char* appname)
{
  struct capture {
     char * name;
  } capture = { .name = strdup(appname) };

  (void (void* p)) {
    struct capture* capture = p;    
  }(&capture); 
}
```
Because struct capture was in function scope and the lambda function will be created
at file scope the type **struct capture** had to be moved from function scope to file scope.

```c
extern char* strdup(const char* s);

struct _capture0 {
     char * name;
  };
  
void _lit_func_0(void *p) {
    struct _capture0* capture = p;    
  }

void create_app(const char* appname)
{
  struct _capture0  capture = { .name = strdup(appname) };
  _lit_func_0(&capture);  
}
```

###  Extension Repeat

```
iteration-statement:
        ...
        "repeat" secondary-block
```

Repeat merely executes the given statement forever unless a break statement is encountered, 
or a goto passes control to a statement outside the loop.

```c
  repeat {
       if (condition) 
         break;
  }
```

Repeat is the same of `for(;;)`. In cake's source for instance I have 5 occurrences of `for(;;)`.


### Extension #pragma expand

pragma expand tells the C back-end to not hide macro expansions. This is necessary when
the compiler makes changes inside the macro code.

Sample:


```c

#define SWAP(a, b) \
    do { \
      typeof(a) temp = a; a = b; b = temp; \
    } while(0)

#pragma expand SWAP

int main()
{
   int a = 1;
   typeof(a) b = 2;
   SWAP(a, b);
   return 1;
}
```

Becomes

```c
#define SWAP(a, b) \
    do { \
      typeof(a) temp = a; a = b; b = temp; \
    } while(0)

#pragma expand SWAP

int main()
{
   int a = 1;
   int b = 2;
    do {int temp = a; a = b; b = temp; } while(0);
   return 1;
}

```


### Type traits

We have some compile time functions to infer properties of types.
They are based on C++ https://en.cppreference.com/w/cpp/header/type_traits

```c

_is_char (TODO)
The three types char, signed char, and unsigned char are collectively called the character types.

_is_pointer
Pointer to object or function

_is_array
Array type

_is_function
A function type describes a function with specified return type. 

_is_floating_point
float, double, and long double return true

_is_integral
The standard signed integer types and standard unsigned integer types are collectively called the
standard integer types;

_is_arithmetic
Integer and floating types are collectively called arithmetic types. 

_is_scalar
Arithmetic types, pointer types, and the nullptr_t type are collectively called scalar types

_is_same(T, U)
If T and U name the same type (taking into account const/volatile qualifications returns 1

```



###  Extension type-expression


Syntax:

``` 
  cast-expression:
    unary-expression
    ( type-name ) cast-expression
    ( type-name )                  //<- creates a type expression
```

Sample

```c
    int a[2];
    static_assert( a == (int[2]) );


    /* is array of ints? */    
    static_assert( _is_array(a) && a[0] == (int) );

int main()
{
    int a;
    
    int t1 = 
       a == (int) ? 1 : a == (double) ? 2 : 0;
    
    int t2 = 
       _Generic(a, int: 1, double: 2, default: 0);

    //allowing typeof on this type of value we can create a way to select types in runtim!
    typeof( 1 ? (int) : (double)) a;
}
```

We can use != and == with void value to compare types.

### Extension - Ownership checks

Under development...
See [ownership](ownership.html)

### Extension typename on _Generic


```c
 int main()
{
    const int * const p;
    static_assert(_Generic(p, const int *: 1));

    /*extension*/
    static_assert(_Generic(int, int : 1));
    static_assert(_Generic(typeof(p), const int * const: 1));
}

```

and the lvalue conversion will not happen, allowing 
more precise (with qualifiers) type match.

## Versions

0.5.1 Versioning started




