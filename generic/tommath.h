/* LibTomMath, multiple-precision integer library -- Tom St Denis
 *
 * LibTomMath is a library that provides multiple-precision
 * integer arithmetic as well as number theoretic functionality.
 *
 * The library was designed directly after the MPI library by
 * Michael Fromberger but has been written from scratch with
 * additional optimizations in place.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@iahu.ca, http://math.libtomcrypt.org
 */
#ifndef BN_H_
#define BN_H_

#ifdef TCL_TOMMATH
#include <tclTomMath.h>
#endif
#ifndef TOMMATH_STORAGE_CLASS
#define TOMMATH_STORAGE_CLASS extern
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include <tommath_class.h>

#undef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#undef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))

#ifdef __cplusplus
extern "C" {

/* C++ compilers don't like assigning void * to mp_digit * */
#define  OPT_CAST(x)  (x *)

#else

/* C on the other hand doesn't care */
#define  OPT_CAST(x)

#endif


/* detect 64-bit mode if possible */
#if defined(__x86_64__) 
   #if !(defined(MP_64BIT) && defined(MP_16BIT) && defined(MP_8BIT))
      #define MP_64BIT
   #endif
#endif

/* some default configurations.
 *
 * A "mp_digit" must be able to hold DIGIT_BIT + 1 bits
 * A "mp_word" must be able to hold 2*DIGIT_BIT + 1 bits
 *
 * At the very least a mp_digit must be able to hold 7 bits
 * [any size beyond that is ok provided it doesn't overflow the data type]
 */
#ifdef MP_8BIT
#ifndef MP_DIGIT_DECLARED
   typedef unsigned char      mp_digit;
#define MP_DIGIT_DECLARED
#endif
   typedef unsigned short     mp_word;
#elif defined(MP_16BIT)
#ifndef MP_DIGIT_DECLARED
   typedef unsigned short     mp_digit;
#define MP_DIGIT_DECLARED
#endif
   typedef unsigned long      mp_word;
#elif defined(MP_64BIT)
   /* for GCC only on supported platforms */
#ifndef CRYPT
   typedef unsigned long long ulong64;
   typedef signed long long   long64;
#endif

#ifndef MP_DIGIT_DECLARED
   typedef unsigned long      mp_digit;
#define MP_DIGIT_DECLARED
#endif
   typedef unsigned long      mp_word __attribute__ ((mode(TI)));

   #define DIGIT_BIT          60
#else
   /* this is the default case, 28-bit digits */
   
   /* this is to make porting into LibTomCrypt easier :-) */
#ifndef CRYPT
   #if defined(_MSC_VER) || defined(__BORLANDC__) 
      typedef unsigned __int64   ulong64;
      typedef signed __int64     long64;
   #else
      typedef unsigned long long ulong64;
      typedef signed long long   long64;
   #endif
#endif

#ifndef MP_DIGIT_DECLARED
   typedef unsigned long      mp_digit;
#define MP_DIGIT_DECLARED
#endif
   typedef ulong64            mp_word;

#ifdef MP_31BIT   
   /* this is an extension that uses 31-bit digits */
   #define DIGIT_BIT          31
#else
   /* default case is 28-bit digits, defines MP_28BIT as a handy macro to test */
   #define DIGIT_BIT          28
   #define MP_28BIT
#endif   
#endif

/* define heap macros */
#ifndef CRYPT
   /* default to libc stuff */
   #ifndef XMALLOC 
       #define XMALLOC  malloc
       #define XFREE    free
       #define XREALLOC realloc
       #define XCALLOC  calloc
   #else
      /* prototypes for our heap functions */
      extern void *XMALLOC(size_t n);
      extern void *REALLOC(void *p, size_t n);
      extern void *XCALLOC(size_t n, size_t s);
      extern void XFREE(void *p);
   #endif
#endif


/* otherwise the bits per digit is calculated automatically from the size of a mp_digit */
#ifndef DIGIT_BIT
   #define DIGIT_BIT     ((int)((CHAR_BIT * sizeof(mp_digit) - 1)))  /* bits per digit */
#endif

#define MP_DIGIT_BIT     DIGIT_BIT
#define MP_MASK          ((((mp_digit)1)<<((mp_digit)DIGIT_BIT))-((mp_digit)1))
#define MP_DIGIT_MAX     MP_MASK

/* equalities */
#define MP_LT        -1   /* less than */
#define MP_EQ         0   /* equal to */
#define MP_GT         1   /* greater than */

#define MP_ZPOS       0   /* positive integer */
#define MP_NEG        1   /* negative */

#define MP_OKAY       0   /* ok result */
#define MP_MEM        -2  /* out of mem */
#define MP_VAL        -3  /* invalid input */
#define MP_RANGE      MP_VAL

#define MP_YES        1   /* yes response */
#define MP_NO         0   /* no response */

/* Primality generation flags */
#define LTM_PRIME_BBS      0x0001 /* BBS style prime */
#define LTM_PRIME_SAFE     0x0002 /* Safe prime (p-1)/2 == prime */
#define LTM_PRIME_2MSB_OFF 0x0004 /* force 2nd MSB to 0 */
#define LTM_PRIME_2MSB_ON  0x0008 /* force 2nd MSB to 1 */

typedef int           mp_err;

/* you'll have to tune these... */
extern int KARATSUBA_MUL_CUTOFF,
           KARATSUBA_SQR_CUTOFF,
           TOOM_MUL_CUTOFF,
           TOOM_SQR_CUTOFF;

/* define this to use lower memory usage routines (exptmods mostly) */
/* #define MP_LOW_MEM */

/* default precision */
#ifndef MP_PREC
   #ifndef MP_LOW_MEM
      #define MP_PREC                 64     /* default digits of precision */
   #else
      #define MP_PREC                 8      /* default digits of precision */
   #endif   
#endif

/* size of comba arrays, should be at least 2 * 2**(BITS_PER_WORD - BITS_PER_DIGIT*2) */
#define MP_WARRAY               (1 << (sizeof(mp_word) * CHAR_BIT - 2 * DIGIT_BIT + 1))

/* the infamous mp_int structure */
#ifndef MP_INT_DECLARED
#define MP_INT_DECLARED
typedef struct mp_int mp_int;
#endif
struct mp_int {
    int used, alloc, sign;
    mp_digit *dp;
};

/* callback for mp_prime_random, should fill dst with random bytes and return how many read [upto len] */
typedef int ltm_prime_callback(unsigned char *dst, int len, void *dat);


#define USED(m)    ((m)->used)
#define DIGIT(m,k) ((m)->dp[(k)])
#define SIGN(m)    ((m)->sign)

/* error code to char* string */
TOMMATH_STORAGE_CLASS char *mp_error_to_string(int code);

/* ---> init and deinit bignum functions <--- */
/* init a bignum */
TOMMATH_STORAGE_CLASS int mp_init(mp_int *a);

/* free a bignum */
TOMMATH_STORAGE_CLASS void mp_clear(mp_int *a);

/* init a null terminated series of arguments */
TOMMATH_STORAGE_CLASS int mp_init_multi(mp_int *mp, ...);

/* clear a null terminated series of arguments */
TOMMATH_STORAGE_CLASS void mp_clear_multi(mp_int *mp, ...);

/* exchange two ints */
TOMMATH_STORAGE_CLASS void mp_exch(mp_int *a, mp_int *b);

/* shrink ram required for a bignum */
TOMMATH_STORAGE_CLASS int mp_shrink(mp_int *a);

/* grow an int to a given size */
TOMMATH_STORAGE_CLASS int mp_grow(mp_int *a, int size);

/* init to a given number of digits */
TOMMATH_STORAGE_CLASS int mp_init_size(mp_int *a, int size);

/* ---> Basic Manipulations <--- */
#define mp_iszero(a) (((a)->used == 0) ? MP_YES : MP_NO)
#define mp_iseven(a) (((a)->used == 0 || (((a)->dp[0] & 1) == 0)) ? MP_YES : MP_NO)
#define mp_isodd(a)  (((a)->used > 0 && (((a)->dp[0] & 1) == 1)) ? MP_YES : MP_NO)

/* set to zero */
TOMMATH_STORAGE_CLASS void mp_zero(mp_int *a);

/* set to a digit */
TOMMATH_STORAGE_CLASS void mp_set(mp_int *a, mp_digit b);

/* set a 32-bit const */
TOMMATH_STORAGE_CLASS int mp_set_int(mp_int *a, unsigned long b);

/* get a 32-bit value */
unsigned long mp_get_int(mp_int * a);

/* initialize and set a digit */
TOMMATH_STORAGE_CLASS int mp_init_set (mp_int * a, mp_digit b);

/* initialize and set 32-bit value */
TOMMATH_STORAGE_CLASS int mp_init_set_int (mp_int * a, unsigned long b);

/* copy, b = a */
TOMMATH_STORAGE_CLASS int mp_copy(mp_int *a, mp_int *b);

/* inits and copies, a = b */
TOMMATH_STORAGE_CLASS int mp_init_copy(mp_int *a, mp_int *b);

/* trim unused digits */
TOMMATH_STORAGE_CLASS void mp_clamp(mp_int *a);

/* ---> digit manipulation <--- */

/* right shift by "b" digits */
TOMMATH_STORAGE_CLASS void mp_rshd(mp_int *a, int b);

/* left shift by "b" digits */
TOMMATH_STORAGE_CLASS int mp_lshd(mp_int *a, int b);

/* c = a / 2**b */
TOMMATH_STORAGE_CLASS int mp_div_2d(mp_int *a, int b, mp_int *c, mp_int *d);

/* b = a/2 */
TOMMATH_STORAGE_CLASS int mp_div_2(mp_int *a, mp_int *b);

/* c = a * 2**b */
TOMMATH_STORAGE_CLASS int mp_mul_2d(mp_int *a, int b, mp_int *c);

/* b = a*2 */
TOMMATH_STORAGE_CLASS int mp_mul_2(mp_int *a, mp_int *b);

/* c = a mod 2**d */
TOMMATH_STORAGE_CLASS int mp_mod_2d(mp_int *a, int b, mp_int *c);

/* computes a = 2**b */
TOMMATH_STORAGE_CLASS int mp_2expt(mp_int *a, int b);

/* Counts the number of lsbs which are zero before the first zero bit */
TOMMATH_STORAGE_CLASS int mp_cnt_lsb(mp_int *a);

/* I Love Earth! */

/* makes a pseudo-random int of a given size */
TOMMATH_STORAGE_CLASS int mp_rand(mp_int *a, int digits);

/* ---> binary operations <--- */
/* c = a XOR b  */
TOMMATH_STORAGE_CLASS int mp_xor(mp_int *a, mp_int *b, mp_int *c);

/* c = a OR b */
TOMMATH_STORAGE_CLASS int mp_or(mp_int *a, mp_int *b, mp_int *c);

/* c = a AND b */
TOMMATH_STORAGE_CLASS int mp_and(mp_int *a, mp_int *b, mp_int *c);

/* ---> Basic arithmetic <--- */

/* b = -a */
TOMMATH_STORAGE_CLASS int mp_neg(mp_int *a, mp_int *b);

/* b = |a| */
TOMMATH_STORAGE_CLASS int mp_abs(mp_int *a, mp_int *b);

/* compare a to b */
TOMMATH_STORAGE_CLASS int mp_cmp(mp_int *a, mp_int *b);

/* compare |a| to |b| */
TOMMATH_STORAGE_CLASS int mp_cmp_mag(mp_int *a, mp_int *b);

/* c = a + b */
TOMMATH_STORAGE_CLASS int mp_add(mp_int *a, mp_int *b, mp_int *c);

/* c = a - b */
TOMMATH_STORAGE_CLASS int mp_sub(mp_int *a, mp_int *b, mp_int *c);

/* c = a * b */
TOMMATH_STORAGE_CLASS int mp_mul(mp_int *a, mp_int *b, mp_int *c);

/* b = a*a  */
TOMMATH_STORAGE_CLASS int mp_sqr(mp_int *a, mp_int *b);

/* a/b => cb + d == a */
TOMMATH_STORAGE_CLASS int mp_div(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* c = a mod b, 0 <= c < b  */
TOMMATH_STORAGE_CLASS int mp_mod(mp_int *a, mp_int *b, mp_int *c);

/* ---> single digit functions <--- */

/* compare against a single digit */
TOMMATH_STORAGE_CLASS int mp_cmp_d(mp_int *a, mp_digit b);

/* c = a + b */
TOMMATH_STORAGE_CLASS int mp_add_d(mp_int *a, mp_digit b, mp_int *c);

/* c = a - b */
TOMMATH_STORAGE_CLASS int mp_sub_d(mp_int *a, mp_digit b, mp_int *c);

/* c = a * b */
TOMMATH_STORAGE_CLASS int mp_mul_d(mp_int *a, mp_digit b, mp_int *c);

/* a/b => cb + d == a */
TOMMATH_STORAGE_CLASS int mp_div_d(mp_int *a, mp_digit b, mp_int *c, mp_digit *d);

/* a/3 => 3c + d == a */
TOMMATH_STORAGE_CLASS int mp_div_3(mp_int *a, mp_int *c, mp_digit *d);

/* c = a**b */
TOMMATH_STORAGE_CLASS int mp_expt_d(mp_int *a, mp_digit b, mp_int *c);

/* c = a mod b, 0 <= c < b  */
TOMMATH_STORAGE_CLASS int mp_mod_d(mp_int *a, mp_digit b, mp_digit *c);

/* ---> number theory <--- */

/* d = a + b (mod c) */
TOMMATH_STORAGE_CLASS int mp_addmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* d = a - b (mod c) */
TOMMATH_STORAGE_CLASS int mp_submod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* d = a * b (mod c) */
TOMMATH_STORAGE_CLASS int mp_mulmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* c = a * a (mod b) */
TOMMATH_STORAGE_CLASS int mp_sqrmod(mp_int *a, mp_int *b, mp_int *c);

/* c = 1/a (mod b) */
TOMMATH_STORAGE_CLASS int mp_invmod(mp_int *a, mp_int *b, mp_int *c);

/* c = (a, b) */
TOMMATH_STORAGE_CLASS int mp_gcd(mp_int *a, mp_int *b, mp_int *c);

/* produces value such that U1*a + U2*b = U3 */
TOMMATH_STORAGE_CLASS int mp_exteuclid(mp_int *a, mp_int *b, mp_int *U1, mp_int *U2, mp_int *U3);

/* c = [a, b] or (a*b)/(a, b) */
TOMMATH_STORAGE_CLASS int mp_lcm(mp_int *a, mp_int *b, mp_int *c);

/* finds one of the b'th root of a, such that |c|**b <= |a|
 *
 * returns error if a < 0 and b is even
 */
TOMMATH_STORAGE_CLASS int mp_n_root(mp_int *a, mp_digit b, mp_int *c);

/* special sqrt algo */
TOMMATH_STORAGE_CLASS int mp_sqrt(mp_int *arg, mp_int *ret);

/* is number a square? */
TOMMATH_STORAGE_CLASS int mp_is_square(mp_int *arg, int *ret);

/* computes the jacobi c = (a | n) (or Legendre if b is prime)  */
TOMMATH_STORAGE_CLASS int mp_jacobi(mp_int *a, mp_int *n, int *c);

/* used to setup the Barrett reduction for a given modulus b */
TOMMATH_STORAGE_CLASS int mp_reduce_setup(mp_int *a, mp_int *b);

/* Barrett Reduction, computes a (mod b) with a precomputed value c
 *
 * Assumes that 0 < a <= b*b, note if 0 > a > -(b*b) then you can merely
 * compute the reduction as -1 * mp_reduce(mp_abs(a)) [pseudo code].
 */
TOMMATH_STORAGE_CLASS int mp_reduce(mp_int *a, mp_int *b, mp_int *c);

/* setups the montgomery reduction */
TOMMATH_STORAGE_CLASS int mp_montgomery_setup(mp_int *a, mp_digit *mp);

/* computes a = B**n mod b without division or multiplication useful for
 * normalizing numbers in a Montgomery system.
 */
TOMMATH_STORAGE_CLASS int mp_montgomery_calc_normalization(mp_int *a, mp_int *b);

/* computes x/R == x (mod N) via Montgomery Reduction */
TOMMATH_STORAGE_CLASS int mp_montgomery_reduce(mp_int *a, mp_int *m, mp_digit mp);

/* returns 1 if a is a valid DR modulus */
TOMMATH_STORAGE_CLASS int mp_dr_is_modulus(mp_int *a);

/* sets the value of "d" required for mp_dr_reduce */
TOMMATH_STORAGE_CLASS void mp_dr_setup(mp_int *a, mp_digit *d);

/* reduces a modulo b using the Diminished Radix method */
TOMMATH_STORAGE_CLASS int mp_dr_reduce(mp_int *a, mp_int *b, mp_digit mp);

/* returns true if a can be reduced with mp_reduce_2k */
TOMMATH_STORAGE_CLASS int mp_reduce_is_2k(mp_int *a);

/* determines k value for 2k reduction */
TOMMATH_STORAGE_CLASS int mp_reduce_2k_setup(mp_int *a, mp_digit *d);

/* reduces a modulo b where b is of the form 2**p - k [0 <= a] */
TOMMATH_STORAGE_CLASS int mp_reduce_2k(mp_int *a, mp_int *n, mp_digit d);

/* d = a**b (mod c) */
TOMMATH_STORAGE_CLASS int mp_exptmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* ---> Primes <--- */

/* number of primes */
#ifdef MP_8BIT
   #define PRIME_SIZE      31
#else
   #define PRIME_SIZE      256
#endif

/* table of first PRIME_SIZE primes */
extern const mp_digit ltm_prime_tab[];

/* result=1 if a is divisible by one of the first PRIME_SIZE primes */
TOMMATH_STORAGE_CLASS int mp_prime_is_divisible(mp_int *a, int *result);

/* performs one Fermat test of "a" using base "b".
 * Sets result to 0 if composite or 1 if probable prime
 */
TOMMATH_STORAGE_CLASS int mp_prime_fermat(mp_int *a, mp_int *b, int *result);

/* performs one Miller-Rabin test of "a" using base "b".
 * Sets result to 0 if composite or 1 if probable prime
 */
TOMMATH_STORAGE_CLASS int mp_prime_miller_rabin(mp_int *a, mp_int *b, int *result);

/* This gives [for a given bit size] the number of trials required
 * such that Miller-Rabin gives a prob of failure lower than 2^-96 
 */
TOMMATH_STORAGE_CLASS int mp_prime_rabin_miller_trials(int size);

/* performs t rounds of Miller-Rabin on "a" using the first
 * t prime bases.  Also performs an initial sieve of trial
 * division.  Determines if "a" is prime with probability
 * of error no more than (1/4)**t.
 *
 * Sets result to 1 if probably prime, 0 otherwise
 */
TOMMATH_STORAGE_CLASS int mp_prime_is_prime(mp_int *a, int t, int *result);

/* finds the next prime after the number "a" using "t" trials
 * of Miller-Rabin.
 *
 * bbs_style = 1 means the prime must be congruent to 3 mod 4
 */
TOMMATH_STORAGE_CLASS int mp_prime_next_prime(mp_int *a, int t, int bbs_style);

/* makes a truly random prime of a given size (bytes),
 * call with bbs = 1 if you want it to be congruent to 3 mod 4 
 *
 * You have to supply a callback which fills in a buffer with random bytes.  "dat" is a parameter you can
 * have passed to the callback (e.g. a state or something).  This function doesn't use "dat" itself
 * so it can be NULL
 *
 * The prime generated will be larger than 2^(8*size).
 */
#define mp_prime_random(a, t, size, bbs, cb, dat) mp_prime_random_ex(a, t, ((size) * 8) + 1, (bbs==1)?LTM_PRIME_BBS:0, cb, dat)

/* makes a truly random prime of a given size (bits),
 *
 * Flags are as follows:
 * 
 *   LTM_PRIME_BBS      - make prime congruent to 3 mod 4
 *   LTM_PRIME_SAFE     - make sure (p-1)/2 is prime as well (implies LTM_PRIME_BBS)
 *   LTM_PRIME_2MSB_OFF - make the 2nd highest bit zero
 *   LTM_PRIME_2MSB_ON  - make the 2nd highest bit one
 *
 * You have to supply a callback which fills in a buffer with random bytes.  "dat" is a parameter you can
 * have passed to the callback (e.g. a state or something).  This function doesn't use "dat" itself
 * so it can be NULL
 *
 */
TOMMATH_STORAGE_CLASS int mp_prime_random_ex(mp_int *a, int t, int size, int flags, ltm_prime_callback cb, void *dat);

/* ---> radix conversion <--- */
TOMMATH_STORAGE_CLASS int mp_count_bits(mp_int *a);

TOMMATH_STORAGE_CLASS int mp_unsigned_bin_size(mp_int *a);
TOMMATH_STORAGE_CLASS int mp_read_unsigned_bin(mp_int *a, unsigned char *b, int c);
TOMMATH_STORAGE_CLASS int mp_to_unsigned_bin(mp_int *a, unsigned char *b);

TOMMATH_STORAGE_CLASS int mp_signed_bin_size(mp_int *a);
TOMMATH_STORAGE_CLASS int mp_read_signed_bin(mp_int *a, unsigned char *b, int c);
TOMMATH_STORAGE_CLASS int mp_to_signed_bin(mp_int *a, unsigned char *b);

TOMMATH_STORAGE_CLASS int mp_read_radix(mp_int *a, const char *str, int radix);
TOMMATH_STORAGE_CLASS int mp_toradix(mp_int *a, char *str, int radix);
TOMMATH_STORAGE_CLASS int mp_toradix_n(mp_int * a, char *str, int radix, int maxlen);
TOMMATH_STORAGE_CLASS int mp_radix_size(mp_int *a, int radix, int *size);

TOMMATH_STORAGE_CLASS int mp_fread(mp_int *a, int radix, FILE *stream);
TOMMATH_STORAGE_CLASS int mp_fwrite(mp_int *a, int radix, FILE *stream);

#define mp_read_raw(mp, str, len) mp_read_signed_bin((mp), (str), (len))
#define mp_raw_size(mp)           mp_signed_bin_size(mp)
#define mp_toraw(mp, str)         mp_to_signed_bin((mp), (str))
#define mp_read_mag(mp, str, len) mp_read_unsigned_bin((mp), (str), (len))
#define mp_mag_size(mp)           mp_unsigned_bin_size(mp)
#define mp_tomag(mp, str)         mp_to_unsigned_bin((mp), (str))

#define mp_tobinary(M, S)  mp_toradix((M), (S), 2)
#define mp_tooctal(M, S)   mp_toradix((M), (S), 8)
#define mp_todecimal(M, S) mp_toradix((M), (S), 10)
#define mp_tohex(M, S)     mp_toradix((M), (S), 16)

/* lowlevel functions, do not call! */
TOMMATH_STORAGE_CLASS int s_mp_add(mp_int *a, mp_int *b, mp_int *c);
TOMMATH_STORAGE_CLASS int s_mp_sub(mp_int *a, mp_int *b, mp_int *c);
#define s_mp_mul(a, b, c) s_mp_mul_digs(a, b, c, (a)->used + (b)->used + 1)
TOMMATH_STORAGE_CLASS int fast_s_mp_mul_digs(mp_int *a, mp_int *b, mp_int *c, int digs);
TOMMATH_STORAGE_CLASS int s_mp_mul_digs(mp_int *a, mp_int *b, mp_int *c, int digs);
TOMMATH_STORAGE_CLASS int fast_s_mp_mul_high_digs(mp_int *a, mp_int *b, mp_int *c, int digs);
TOMMATH_STORAGE_CLASS int s_mp_mul_high_digs(mp_int *a, mp_int *b, mp_int *c, int digs);
TOMMATH_STORAGE_CLASS int fast_s_mp_sqr(mp_int *a, mp_int *b);
TOMMATH_STORAGE_CLASS int s_mp_sqr(mp_int *a, mp_int *b);
TOMMATH_STORAGE_CLASS int mp_karatsuba_mul(mp_int *a, mp_int *b, mp_int *c);
TOMMATH_STORAGE_CLASS int mp_toom_mul(mp_int *a, mp_int *b, mp_int *c);
TOMMATH_STORAGE_CLASS int mp_karatsuba_sqr(mp_int *a, mp_int *b);
TOMMATH_STORAGE_CLASS int mp_toom_sqr(mp_int *a, mp_int *b);
TOMMATH_STORAGE_CLASS int fast_mp_invmod(mp_int *a, mp_int *b, mp_int *c);
TOMMATH_STORAGE_CLASS int mp_invmod_slow (mp_int * a, mp_int * b, mp_int * c);
TOMMATH_STORAGE_CLASS int fast_mp_montgomery_reduce(mp_int *a, mp_int *m, mp_digit mp);
TOMMATH_STORAGE_CLASS int mp_exptmod_fast(mp_int *G, mp_int *X, mp_int *P, mp_int *Y, int mode);
TOMMATH_STORAGE_CLASS int s_mp_exptmod (mp_int * G, mp_int * X, mp_int * P, mp_int * Y);
TOMMATH_STORAGE_CLASS void bn_reverse(unsigned char *s, int len);

extern const char *mp_s_rmap;

#ifdef __cplusplus
   }
#endif

#endif


