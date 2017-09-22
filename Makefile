CXX = g++
CXXFLAGS = -g -Wall -MMD -std=c++0x # --coverage
OBJECTS = game.o negamax.o game_io.o

MAIN_OBJECTS = ${OBJECTS} main.o
MAIN_DEPENDS = ${MAIN_OBJECTS:.o=.d}
MAIN_EXEC = main

${MAIN_EXEC} : ${MAIN_OBJECTS}
	${CXX} ${MAIN_OBJECTS} -o ${MAIN_EXEC} ${CXXFLAGS}

TEST_OBJECTS = ${OBJECTS} tests/test.o tests/game_test.o
TEST_DEPENDS = ${TEST_OBJECTS:.o=.d}
TEST_EXEC = test

${TEST_EXEC} : ${TEST_OBJECTS}
	${CXX} ${TEST_OBJECTS} -o ${TEST_EXEC} ${CXXFLAGS}

JUDGE_OBJECTS = ${OBJECTS} judge.o
JUDGE_DEPENDS = ${JUDGE_OBJECTS:.o=.d}
JUDGE_EXEC = judge

${JUDGE_EXEC} : ${JUDGE_OBJECTS}
	${CXX} ${JUDGE_OBJECTS} -o ${JUDGE_EXEC} ${CXXFLAGS}

coverage: ${TEST_OBJECTS}
	${CXX} ${TEST_OBJECTS} -o ${TEST_EXEC} ${CXXFLAGS} --coverage
	./${TEST_EXEC}
	lcov -c -d . -o cov.info
	genhtml cov.info -o covout
	open covout/index.html

clean:
	rm -rf ${MAIN_OBJECTS} ${MAIN_DEPENDS} ${MAIN_EXEC}
	rm -rf ${TEST_OBJECTS} ${TEST_DEPENDS} ${TEST_EXEC}
	rm -rf ${JUDGE_OBJECTS} ${JUDGE_DEPENDS} ${JUDGE_EXEC}
	rm -rf *.gcda *.gcno
	rm -rf tests/*.gcda tests/*.gcno
	rm -rf cov.info covout

-include ${MAIN_DEPENDS} ${TEST_DEPENDS} ${JUDGE_DEPENDS}
