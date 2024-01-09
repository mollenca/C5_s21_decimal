#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

#define CONVERTING_ERROR 1
#define OK 0
#define SIGN 0x80000000
#define TRUE 1
#define FALSE 0
#define INF 1
#define NINF 2
#define NAN1 3

#define SUCCESS 0

typedef struct {
  unsigned int bits[4];
} s21_decimal;

void set_sign(s21_decimal *dst);
void reset_sign(s21_decimal *dst);
int checkbit(int number, int index);
int setbit(int number, int index);
int resetbit(int number, int index);
void nuller(s21_decimal *num);
int getbinexp(float src);
void decsetbit(s21_decimal *dst, int index);
int deccheckbit(s21_decimal dst, int index);
void write_mantissa_to_decimal(s21_decimal *dst, int exp, float src);
void set_scale(s21_decimal *dst, int scale);
int getexpfromdecimal(s21_decimal src);
int mantlastind(s21_decimal src);
int sign_minus(s21_decimal a);
int get_scale(s21_decimal a);
int get_sign(s21_decimal src);
int get_highest_bit(s21_decimal dec);
void minus_scale(s21_decimal *a);
void swap_values(s21_decimal *dec1, s21_decimal *dec2);
void bits_copy(s21_decimal src, s21_decimal *dest);

int handle_scales_equality(s21_decimal *dec1, s21_decimal *dec2,
                           int *final_scale);
int shift_left(s21_decimal *dec, int shift);
s21_decimal binary_div(s21_decimal dec1, s21_decimal dec2,
                       s21_decimal *reminder, int *fail);
int equalize_to_lower(s21_decimal *dec, int scale);
int equalize_to_bigger(s21_decimal *dec, int scale);
int equalize_scales(s21_decimal *dec1, s21_decimal *dec2, int scale);
int handle_scales_equality(s21_decimal *dec1, s21_decimal *dec2,
                           int *final_scale);
void scale_equalize(s21_decimal *dec1, s21_decimal *dec2);

int bit_add(s21_decimal dec1, s21_decimal dec2, s21_decimal *result);
void mul_only_bits(s21_decimal dec1, s21_decimal dec2, s21_decimal *result);
void sub_only_bits(s21_decimal dec1, s21_decimal dec2, s21_decimal *result);
void div_only_bits(s21_decimal dec1, s21_decimal dec2, s21_decimal *buf,
                   s21_decimal *result);
int sub_without_scale(s21_decimal value1, s21_decimal value2,
                      s21_decimal *result);
int add_without_scale(s21_decimal dec1, s21_decimal dec2, s21_decimal *result);

int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);

int s21_add(s21_decimal dec1, s21_decimal dec2, s21_decimal *result);
int s21_sub(s21_decimal dec1, s21_decimal dec2, s21_decimal *result);
int s21_mul(s21_decimal dec1, s21_decimal dec2, s21_decimal *result);
int s21_div(s21_decimal divident, s21_decimal divisor, s21_decimal *result);
int s21_mod(s21_decimal number_1, s21_decimal number_2, s21_decimal *result);

int s21_is_equal(s21_decimal dec1, s21_decimal dec2);
int s21_is_greater(s21_decimal dec1, s21_decimal dec2);
int s21_is_less(s21_decimal dec1, s21_decimal dec2);
int s21_is_less_or_equal(s21_decimal dec1, s21_decimal dec2);
int s21_is_greater_or_equal(s21_decimal dec1, s21_decimal dec2);
int s21_is_not_equal(s21_decimal dec1, s21_decimal dec2);

int s21_truncate(s21_decimal value, s21_decimal *result);
int s21_negate(s21_decimal dec, s21_decimal *result);
int s21_round(s21_decimal value, s21_decimal *result);
int s21_floor(s21_decimal value, s21_decimal *result);

#endif  // SRC_S21_DECIMAL_H_
