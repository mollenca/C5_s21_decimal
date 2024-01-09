#include "s21_decimal.h"

#include <math.h>

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  int res = OK;
  if (dst) {
    nuller(dst);
    if (src < 0) {
      src *= -1;
      set_sign(dst);
    }
    dst->bits[0] = src;
  } else {
    res = CONVERTING_ERROR;
  }
  return res;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  int res = 0;
  if (src == 1 / 0.0 && src == 0 / 0.0 && src == -1 / 0.0 && src == -0 / 0.0)
    res = 1;
  nuller(dst);
  if (src != 0) {
    nuller(dst);
    if (dst && !res) {
      int is_negative = 0;
      if (src < 0) {
        is_negative = 1;
        src *= -1;
      }
      double dbl = src;
      int exp = getbinexp((float)dbl);
      int scale = 0;
      while (scale < 28 && (int)dbl / (int)pow(2, 21) == 0) {
        dbl *= 10;
        scale++;
      }
      dbl = round(dbl);
      if (scale <= 28 && (exp > -94 && exp < 96)) {
        while (fmod(dbl, 10) == 0 && scale > 0) {
          dbl = dbl / 10;
          scale--;
        }
        exp = getbinexp((float)dbl);
        decsetbit(dst, exp);
        write_mantissa_to_decimal(dst, exp, (float)dbl);
        set_scale(dst, scale);
        is_negative ? set_sign(dst) : 1;
      } else {
        res = 1;
      }
    }
  }
  return res;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  int res = OK;
  double tmp = 0;
  int scale = 0;
  if (dst) {
    for (int i = 0; i < 96; i++)
      if (deccheckbit(src, i)) tmp += pow(2, i);
    for (int i = 16; i < 24; i++)
      if (checkbit(src.bits[3], i)) scale = setbit(scale, i - 16);
    if (scale > 0) {
      for (int i = scale; i > 0; i--) tmp /= 10.0;
    }
    *dst = (float)tmp;
    *dst *= src.bits[3] >> 31 ? -1 : 1;
  } else {
    res = CONVERTING_ERROR;
  }
  return res;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  *dst = 0;
  int res = s21_truncate(src, &src);
  if (res != OK || src.bits[1] || src.bits[2] || src.bits[0] > 2147483647u) {
    res = CONVERTING_ERROR;
  } else {
    *dst = src.bits[0];
    if (get_sign(src)) *dst *= -1;
  }
  return res;
}

int s21_is_equal(s21_decimal dec1, s21_decimal dec2) {
  int scale = 0;
  int result = 1;
  handle_scales_equality(&dec1, &dec2, &scale);
  if (dec1.bits[0] == 0 && dec1.bits[1] == 0 && dec1.bits[2] == 0 &&
      dec2.bits[0] == 0 && dec2.bits[1] == 0 && dec2.bits[2] == 0) {
    result = 1;
  } else if (get_sign(dec1) && !get_sign(dec2)) {
    result = 0;
  } else if (!get_sign(dec1) && get_sign(dec2)) {
    result = 0;
  } else if (get_sign(dec1) == get_sign(dec2)) {
    for (int i = 2; i >= 0; i--) {
      if (dec1.bits[i] > dec2.bits[i]) {
        result = 0;
        break;
      } else if (dec1.bits[i] < dec2.bits[i]) {
        result = 0;
        break;
      } else if (dec1.bits[i] == dec2.bits[i]) {
        continue;
      }
    }
  }
  return result;
}

int s21_truncate(s21_decimal dec1, s21_decimal *result) {
  nuller(result);
  s21_decimal ten = {{10, 0, 0, 0}};
  s21_decimal tmp = {{0, 0, 0, 0}};
  int sign_dec1 = get_sign(dec1);
  int scale = get_scale(dec1);
  if (!scale) {
    *result = dec1;
  } else {
    for (int i = scale; i > 0; i--) {
      div_only_bits(dec1, ten, &tmp, result);
      dec1 = *result;
    }
  }
  if (sign_dec1) set_sign(result);
  return OK;
}

int s21_is_greater(s21_decimal dec1, s21_decimal dec2) {
  int scale = 0;
  int result = 1;
  handle_scales_equality(&dec1, &dec2, &scale);
  if (get_sign(dec1) && !get_sign(dec2)) {
    result = 0;
  } else if (!get_sign(dec1) && get_sign(dec2)) {
    result = 1;
  } else if (get_sign(dec1) == get_sign(dec2)) {
    for (int i = 2; i >= 0; i--) {
      if (dec1.bits[i] > dec2.bits[i]) {
        result = 1;
        break;
      } else if (dec1.bits[i] < dec2.bits[i]) {
        result = 0;
        break;
      } else if (dec1.bits[i] == dec2.bits[i]) {
        result = 0;
        continue;
      }
    }
    if (get_sign(dec1)) {
      if (result == 1)
        result = 0;
      else
        result = 1;
    }
  }
  return result;
}

int s21_add(s21_decimal dec1, s21_decimal dec2, s21_decimal *result) {
  int res = 0;
  if (!result) {
    res = 1;
  } else {
    nuller(result);
    int fscale = 0;
    res = handle_scales_equality(&dec1, &dec2, &fscale);
    if (res == 0) {
      res = add_without_scale(dec1, dec2, result);
      set_scale(result, fscale);
    }
  }
  return res;
}

int s21_sub(s21_decimal dec1, s21_decimal dec2, s21_decimal *result) {
  int fscale = 0;
  int res = handle_scales_equality(&dec1, &dec2, &fscale);
  if (res == 0) {
    nuller(result);
    res = sub_without_scale(dec1, dec2, result);
    set_scale(result, fscale);
  }
  return res;
}

int s21_is_less(s21_decimal dec1, s21_decimal dec2) {
  return (s21_is_greater(dec2, dec1));
}

int s21_is_less_or_equal(s21_decimal dec1, s21_decimal dec2) {
  return (s21_is_less(dec1, dec2) || s21_is_equal(dec1, dec2));
}

int s21_is_greater_or_equal(s21_decimal dec1, s21_decimal dec2) {
  return (s21_is_greater(dec1, dec2) || s21_is_equal(dec1, dec2));
}

int s21_is_not_equal(s21_decimal dec1, s21_decimal dec2) {
  return (!s21_is_equal(dec1, dec2));
}

int s21_negate(s21_decimal dec, s21_decimal *result) {
  *result = dec;
  int sign = get_sign(*result);
  if (sign) {
    reset_sign(result);
  } else {
    set_sign(result);
  }
  return OK;
}

int s21_round(s21_decimal value, s21_decimal *result) {
  nuller(result);
  int res = OK;
  int sign = get_sign(value);
  reset_sign(&value);
  s21_decimal tmp = {{0}};
  s21_truncate(value, &tmp);
  s21_decimal tmp_copy = tmp;
  s21_sub(value, tmp, &tmp);
  s21_decimal five = {{5, 0, 0, 0}};
  s21_decimal one = {{1, 0, 0, 0}};
  set_scale(&five, 1);
  if (s21_is_greater_or_equal(tmp, five)) {
    res = s21_add(tmp_copy, one, result);
  } else {
    *result = tmp_copy;
  }
  if (sign) set_sign(result);
  return res;
}

int s21_floor(s21_decimal value, s21_decimal *result) {
  nuller(result);
  int res = OK;
  s21_truncate(value, result);
  s21_decimal one = {{1, 0, 0, 0}};
  if (get_sign(value)) {
    res = s21_sub(*result, one, result);
  }
  return res;
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int res = 0;
  int scale_1 = get_scale(value_1);
  int scale_2 = get_scale(value_2);
  int sign = 0;
  if ((get_sign(value_1) && !get_sign(value_2)) ||
      (!get_sign(value_1) && get_sign(value_2)))
    sign = 1;
  value_1.bits[3] = 0;
  value_2.bits[3] = 0;
  nuller(result);
  for (int i = 0; i <= get_highest_bit(value_2); i++) {
    int bit_2 = deccheckbit(value_2, i);
    if (bit_2) {
      s21_decimal tmp = value_1;
      res = shift_left(&tmp, i);
      if (res) break;
      res = add_without_scale(*result, tmp, result);
      if (res) break;
    }
  }
  int scale_res = scale_1 + scale_2;
  if (scale_res > 28)
    res = 2;
  else
    set_scale(result, scale_res);
  if (sign && !res) set_sign(result);
  if (res == 1 && sign) {
    res = 2;
  }
  return res;
}

int s21_div(s21_decimal dec1, s21_decimal dec2, s21_decimal *result) {
  int ret_val = 0;
  nuller(result);
  s21_decimal zero = {{0, 0, 0, 0}};
  if (s21_is_equal(dec2, zero)) {
    ret_val = NAN1;
  } else {
    int sign_1 = get_sign(dec1);
    int sign_2 = get_sign(dec2);
    int scale_1 = get_scale(dec1);
    int scale_2 = get_scale(dec2);
    dec1.bits[3] = 0;
    dec2.bits[3] = 0;
    s21_decimal reminder = {{0, 0, 0, 0}}, dec_int_part = {{0, 0, 0, 0}};
    int division_counter = 0;
    dec_int_part = binary_div(dec1, dec2, &reminder, &ret_val);
    *result = dec_int_part;
    while (!s21_is_equal(reminder, zero) && division_counter <= 10) {
      equalize_to_bigger(&reminder, 1);
      division_counter++;
      dec_int_part = binary_div(reminder, dec2, &reminder, &ret_val);
      set_scale(&dec_int_part, division_counter);
      ret_val = s21_add(*result, dec_int_part, result);
      if (ret_val != 0) break;
    }
    if (sign_1 != sign_2) decsetbit(result, 127);
    int scale_result = scale_1 - scale_2;
    if (scale_result < 0 && !ret_val) {
      ret_val = equalize_to_bigger(result, 0 - scale_result);
    }
    if (get_sign(dec1) != get_sign(dec2)) set_sign(result);
    if (s21_is_equal(*result, zero) && !s21_is_equal(dec1, zero)) ret_val = 2;
  }
  return ret_val;
}

int s21_mod(s21_decimal number_1, s21_decimal number_2, s21_decimal *result) {
  nuller(result);
  int res = OK;
  s21_decimal zero = {{0, 0, 0, 0}};
  if (s21_is_equal(number_2, zero)) {
    res = NAN1;
  } else {
    if (!get_sign(number_1) && !get_sign(number_2)) {
      while (s21_is_greater_or_equal(number_1, number_2)) {
        s21_sub(number_1, number_2, &number_1);
      }
    } else if (!get_sign(number_1) && get_sign(number_2)) {
      reset_sign(&number_2);
      while (s21_is_greater_or_equal(number_1, number_2)) {
        s21_sub(number_1, number_2, &number_1);
      }
    } else if (get_sign(number_1) && !get_sign(number_2)) {
      reset_sign(&number_1);
      while (s21_is_greater_or_equal(number_1, number_2)) {
        s21_sub(number_1, number_2, &number_1);
      }
      set_sign(&number_1);
    } else if (get_sign(number_1) && get_sign(number_2)) {
      reset_sign(&number_1);
      reset_sign(&number_2);
      while (s21_is_greater_or_equal(number_1, number_2)) {
        s21_sub(number_1, number_2, &number_1);
      }
      set_sign(&number_1);
    }
    *result = number_1;
  }
  return res;
}
