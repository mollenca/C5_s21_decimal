#include "s21_decimal.h"

int checkbit(int number, int index) { return (number & (1 << index)) != 0; }

int setbit(int number, int index) { return number | (1 << index); }

int resetbit(int number, int index) { return number & (~(1 << index)); }

void set_sign(s21_decimal *dst) { dst->bits[3] = setbit(dst->bits[3], 31); }

void reset_sign(s21_decimal *dst) { dst->bits[3] = resetbit(dst->bits[3], 31); }

void nuller(s21_decimal *num) {
  for (int i = 0; i < 4; i++) {
    num->bits[i] = 0;
  }
}

int getbinexp(float src) {
  unsigned int res = 0;
  unsigned int ftoint = *((unsigned *)&src);
  for (int i = 23; i < 31; i++) {
    if (checkbit(ftoint, i)) res = setbit(res, i);
  }
  return (res >> 23) - 127;
}

void decsetbit(s21_decimal *dst, int index) {
  int byteind = index / 32;
  int bitind = index % 32;
  dst->bits[byteind] = setbit(dst->bits[byteind], bitind);
}

int deccheckbit(s21_decimal dst, int index) {
  int byteind = index / 32;
  int bitind = index % 32;
  return (dst.bits[byteind] & (1 << bitind)) != 0;
}

void write_mantissa_to_decimal(s21_decimal *dst, int exp, float src) {
  unsigned tmp = *((unsigned *)&src);
  for (int i = exp - 1, j = 22; i >= 0 && j >= 0; i--, j--) {
    if (checkbit(tmp, j)) {
      decsetbit(dst, i);
    }
  }
}

void set_scale(s21_decimal *dst, int scale) {
  for (int i = 16, j = 0; i < 24; i++, j++) {
    if (checkbit(scale, j)) dst->bits[3] = setbit(dst->bits[3], i);
  }
}

int get_sign(s21_decimal src) { return checkbit(src.bits[3], 31); }

int get_scale(const s21_decimal a) { return (char)(a.bits[3] >> 16); }

int shift_left(s21_decimal *dec, int shift) {
  int res = 0;
  int highest_bit = get_highest_bit(*dec);
  int last_low_byte_bit = 0;
  int last_mid_byte_bit = 0;
  if (highest_bit + shift > 95) {
    res = 1;
  } else {
    for (int i = 0; i < shift; i++) {
      last_low_byte_bit = deccheckbit(*dec, 31);
      last_mid_byte_bit = deccheckbit(*dec, 63);
      dec->bits[0] = dec->bits[0] << 1;
      dec->bits[1] = dec->bits[1] << 1;
      dec->bits[2] = dec->bits[2] << 1;
      if (last_low_byte_bit) decsetbit(dec, 32);
      if (last_mid_byte_bit) decsetbit(dec, 64);
    }
  }
  if (get_sign(*dec) && res) res = 2;
  return res;
}

s21_decimal binary_div(s21_decimal dec1, s21_decimal dec2,
                       s21_decimal *reminder, int *fail) {
  nuller(reminder);
  s21_decimal result = {{0, 0, 0, 0}};
  int result_scale = 0;
  *fail = handle_scales_equality(&dec1, &dec2, &result_scale);
  for (int i = get_highest_bit(dec1); i >= 0; i--) {
    if (deccheckbit(dec1, i)) decsetbit(reminder, 0);
    if (s21_is_greater_or_equal(*reminder, dec2)) {
      *fail = sub_without_scale(*reminder, dec2, reminder);
      if (i != 0) *fail = shift_left(reminder, 1);
      if (deccheckbit(dec1, i - 1)) decsetbit(reminder, 0);
      *fail = shift_left(&result, 1);
      decsetbit(&result, 0);
    } else {
      *fail = shift_left(&result, 1);
      if (i != 0) *fail = shift_left(reminder, 1);
      if (i - 1 >= 0 && deccheckbit(dec1, i - 1)) decsetbit(reminder, 0);
    }
  }
  set_scale(reminder, result_scale);
  return result;
}

int get_highest_bit(s21_decimal dec) {
  int i = 95;
  while (!deccheckbit(dec, i) && i >= 0) {
    i--;
  }
  return i;
}

int equalize_to_lower(s21_decimal *dec, int scale) {
  int ret = 0;
  while (scale--) {
    s21_decimal reminder;
    s21_decimal ten = {{10, 0, 0, 0}};
    *dec = binary_div(*dec, ten, &reminder, &ret);
    if (ret != 0) break;
  }
  return ret;
}

int equalize_to_bigger(s21_decimal *dec, int scale) {
  int res = 0;
  while (scale--) {
    s21_decimal buf1 = *dec, buf2 = *dec;
    res = shift_left(&buf2, 3);
    if (res != 0) break;
    res = shift_left(&buf1, 1);
    if (res != 0) break;
    res = s21_add(buf2, buf1, dec);
    if (res != 0) break;
  }
  return res;
}

int equalize_scales(s21_decimal *dec1, s21_decimal *dec2, int scale) {
  int scale1 = get_scale(*dec1);
  int scale2 = get_scale(*dec2);
  int res = 0;
  int sign1 = get_sign(*dec1);
  dec1->bits[3] = 0;
  int sign2 = get_sign(*dec2);
  dec2->bits[3] = 0;
  if (scale1 > scale)
    res = equalize_to_lower(dec1, scale1 - scale);
  else if (scale1 < scale)
    res = equalize_to_bigger(dec1, scale - scale1);
  if (scale2 > scale)
    res = equalize_to_lower(dec2, scale2 - scale);
  else if (scale2 < scale)
    res = equalize_to_bigger(dec2, scale - scale2);
  set_scale(dec1, scale);
  set_scale(dec2, scale);
  if (sign1) set_sign(dec1);
  if (sign2) set_sign(dec2);
  return res;
}

int handle_scales_equality(s21_decimal *dec1, s21_decimal *dec2,
                           int *final_scale) {
  s21_decimal tmp1 = *dec1;
  s21_decimal tmp2 = *dec2;
  int scale1 = get_scale(tmp1);
  int scale2 = get_scale(tmp2);
  int res = 0;
  int fscale = scale1;
  if (scale1 > scale2) {
    res = equalize_scales(&tmp1, &tmp2, scale1);
    fscale = scale1;
    if (res != 0) {
      fscale = scale2;
      tmp1 = *dec1;
      tmp2 = *dec2;
      res = equalize_scales(&tmp1, &tmp2, scale2);
    }
  } else if (scale1 < scale2) {
    res = equalize_scales(&tmp1, &tmp2, scale2);
    fscale = scale2;
    if (res != 0) {
      fscale = scale1;
      tmp1 = *dec1;
      tmp2 = *dec2;
      res = equalize_scales(&tmp1, &tmp2, scale1);
    }
  }
  *final_scale = fscale;
  *dec1 = tmp1;
  *dec2 = tmp2;
  return res;
}

void sub_only_bits(s21_decimal dec1, s21_decimal dec2, s21_decimal *result) {
  nuller(result);
  if (!s21_is_equal(dec1, dec2)) {
    int dec1_last_bit = get_highest_bit(dec1);
    int buffer = 0;
    int dec1_curbit = 0;
    int dec2_curbit = 0;
    for (int i = 0; i <= dec1_last_bit; i++) {
      dec1_curbit = deccheckbit(dec1, i);
      dec2_curbit = deccheckbit(dec2, i);
      if (!dec1_curbit && !dec2_curbit) {
        if (buffer) {
          buffer = 1;
          decsetbit(result, i);
        }
      } else if (dec1_curbit && !dec2_curbit) {
        if (buffer) {
          buffer = 0;
        } else {
          decsetbit(result, i);
        }
      } else if (!dec1_curbit && dec2_curbit) {
        if (buffer) {
          buffer = 1;
        } else {
          buffer = 1;
          decsetbit(result, i);
        }
      } else if (dec1_curbit && dec2_curbit) {
        if (buffer) {
          buffer = 1;
          decsetbit(result, i);
        }
      }
    }
  }
}

void div_only_bits(s21_decimal dec1, s21_decimal dec2, s21_decimal *buf,
                   s21_decimal *result) {
  nuller(buf);
  nuller(result);
  for (int i = get_highest_bit(dec1); i >= 0; i--) {
    if (deccheckbit(dec1, i)) decsetbit(buf, 0);
    if (s21_is_greater_or_equal(*buf, dec2)) {
      sub_only_bits(*buf, dec2, buf);
      if (i != 0) shift_left(buf, 1);
      if (deccheckbit(dec1, i - 1)) decsetbit(buf, 0);
      shift_left(result, 1);
      decsetbit(result, 0);
    } else {
      shift_left(result, 1);
      if (i != 0) shift_left(buf, 1);
      if ((i - 1) >= 0 && deccheckbit(dec1, i - 1)) decsetbit(buf, 0);
    }
  }
}

int sub_without_scale(s21_decimal dec1, s21_decimal dec2, s21_decimal *result) {
  nuller(result);
  int ret = 0;
  int reminder = 0;
  if (!get_sign(dec1) && get_sign(dec2)) {
    s21_negate(dec2, &dec2);
    ret = add_without_scale(dec1, dec2, result);
  } else if (get_sign(dec1) && !get_sign(dec2)) {
    set_sign(&dec2);
    ret = add_without_scale(dec1, dec2, result);
  } else {
    if (get_sign(dec1) && get_sign(dec2)) {
      s21_negate(dec2, &dec2);
      s21_negate(dec1, &dec1);
      swap_values(&dec1, &dec2);
    }
    if (!s21_is_greater_or_equal(dec1, dec2)) {
      set_sign(result);
      swap_values(&dec1, &dec2);
    }
    for (int i = 0; i < 96; i++) {
      if (deccheckbit(dec1, i) - deccheckbit(dec2, i) == 0) {
        if (reminder == 1) {
          decsetbit(result, i);
        }
      } else if (deccheckbit(dec1, i) - deccheckbit(dec2, i) == 1) {
        if (reminder == 0)
          decsetbit(result, i);
        else
          reminder = 0;
      } else if (deccheckbit(dec1, i) - deccheckbit(dec2, i) == -1) {
        if (reminder != 1) {
          decsetbit(result, i);
          reminder = 1;
        } else {
          reminder = 1;
        }
      }
      if (i == 95 && reminder) {
        if (get_sign(*result))
          ret = 2;
        else
          ret = 1;
      }
    }
  }
  return ret;
}

int add_without_scale(s21_decimal dec1, s21_decimal dec2, s21_decimal *result) {
  int res = 0;
  if (get_sign(dec1) != get_sign(dec2)) {
    s21_decimal dec1tmp = dec1;
    s21_decimal dec2tmp = dec2;
    if (get_sign(dec1))
      s21_negate(dec1, &dec1tmp);
    else
      s21_negate(dec2, &dec2tmp);
    res = sub_without_scale(dec1tmp, dec2tmp, result);
    if (get_sign(dec1) && s21_is_greater(dec1, dec2))
      s21_negate(*result, result);
    else if (s21_is_greater(dec2, dec1))
      s21_negate(*result, result);
  } else {
    int check_minus;
    int buffer = 0;
    s21_decimal temp = {{0, 0, 0, 0}};
    if (get_sign(dec1)) set_sign(&temp);
    for (int i = 0; i < 96; i++) {
      if (deccheckbit(dec1, i) && deccheckbit(dec2, i)) {
        if (buffer) decsetbit(&temp, i);
        buffer = 1;
      } else if (deccheckbit(dec1, i) || deccheckbit(dec2, i)) {
        if (!buffer) decsetbit(&temp, i);
      } else if (buffer) {
        decsetbit(&temp, i);
        buffer = 0;
      }
      if (i == 95 && buffer) {
        check_minus = get_sign(temp);
        if (check_minus == 1)
          res = 2;
        else
          res = 1;
      }
    }
    *result = temp;
  }
  return res;
}

void swap_values(s21_decimal *dec1, s21_decimal *dec2) {
  s21_decimal tmp = *dec2;
  *dec2 = *dec1;
  *dec1 = tmp;
}
