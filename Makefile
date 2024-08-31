TARGET = cadb_0021_final

SRC_DIR = ./src/
INC_DIR = ./include/

INC = -I${INC_DIR}

SRC = $(wildcard $(SRC_DIR)*.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

CXX = g++
CXXFLAGS = -O3
LDFLAGS = -pthread

${TARGET}: ${OBJ}
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${OBJ} ${INC} -o ${TARGET}

$(SRC_DIR)%.o: $(SRC_DIR)%.cpp
	${CXX} ${CXXFLAGS} ${INC} -o $@ -c $<

.PHONY: clean
clean:
	rm -f core *~ ${SRC_DIR}*.o ${TARGET}
