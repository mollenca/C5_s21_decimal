all: s21_decimal.a test

clean:
	rm -rf *.o *.g* *.info *.out report *.a *.dSYM

rebuild: clean all

test: s21_decimal.c s21_decimal.h sup.c tests.c
	gcc -Wall -Wextra -Werror s21_decimal.c s21_decimal.h sup.c tests.c -lcheck
	./a.out

gcov_report:
	rm -f *.g*
	gcc -Wall -Wextra -Werror --coverage s21_decimal.c sup.c tests.c -lcheck
	./a.out
	lcov -t a.out -o rep.info -c -d .
	genhtml -o report rep.info
	open ./report/index.html
	rm -rf *.gcda *.gcno *.info

s21_decimal.a: s21_decimal.c s21_decimal.h sup.c
	gcc -c -Wall -Wextra -Werror s21_decimal.c sup.c
	ar rc s21_decimal.a *.o
	ranlib s21_decimal.a
	cp s21_decimal.a libs21_decimal.a
	rm -rf *.o

check:
	cp ../materials/linters/CPPLINT.cfg .
	python3 ../materials/linters/cpplint.py --extensions=c *.c *.h
